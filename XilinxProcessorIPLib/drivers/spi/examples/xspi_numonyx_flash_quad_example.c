/******************************************************************************
* Copyright (C) 2012 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xspi_numonyx_flash_quad_example.c
*
* This file contains a design example using the SPI driver (XSpi) and axi_qspi
* device with a Numonyx quad serial flash device in the interrupt mode.
* This example erases a Sector, writes to a Page within the Sector, reads back
* from that Page and compares the data.
*
* This example  has been tested with an N25Q128 device on KC705 and ZC770
* board. The bytes per page (PAGE_SIZE) in N25Q128 is 256.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a bss  08/08/12 First release
* 3.04a bss  02/11/13 Modified to use ScuGic in case of Zynq (CR#683510)
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Modified Comment lines to follow doxygen rules.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xspi.h"		/* SPI device driver */
#include "xil_exception.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else  /* SCU GIC */
#include "xscugic.h"
#include "xil_printf.h"
#endif


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID			XPAR_SPI_0_DEVICE_ID

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
 #define SPI_INTR_ID		XPAR_INTC_0_SPI_0_VEC_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
 #define SPI_INTR_ID		XPAR_FABRIC_SPI_0_VEC_ID
#endif /* XPAR_INTC_0_DEVICE_ID */


#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		    static XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC			static XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

/*
 * The following constant defines the slave select signal that is used to
 * to select the Flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device.
 */
#define SPI_SELECT 			0x01

/*
 * Definitions of the commands shown in this example.
 */
#define COMMAND_PAGE_PROGRAM		0x02 /* Page Program command */
#define COMMAND_QUAD_WRITE		0x32 /* Quad Input Fast Program */
#define COMMAND_RANDOM_READ		0x03 /* Random read command */
#define COMMAND_DUAL_READ		0x3B /* Dual Output Fast Read */
#define COMMAND_DUAL_IO_READ		0xBB /* Dual IO Fast Read */
#define COMMAND_QUAD_READ		0x6B /* Quad Output Fast Read */
#define COMMAND_QUAD_IO_READ		0xEB /* Quad IO Fast Read */
#define	COMMAND_WRITE_ENABLE		0x06 /* Write Enable command */
#define COMMAND_SECTOR_ERASE		0xD8 /* Sector Erase command */
#define COMMAND_BULK_ERASE		0xC7 /* Bulk Erase command */
#define COMMAND_STATUSREG_READ		0x05 /* Status read command */

/**
 * This definitions specify the EXTRA bytes in each of the command
 * transactions. This count includes Command byte, address bytes and any
 * don't care bytes needed.
 */
#define READ_WRITE_EXTRA_BYTES		4 /* Read/Write extra bytes */
#define	WRITE_ENABLE_BYTES		1 /* Write Enable bytes */
#define SECTOR_ERASE_BYTES		4 /* Sector erase extra bytes */
#define BULK_ERASE_BYTES		1 /* Bulk erase extra bytes */
#define STATUS_READ_BYTES		2 /* Status read bytes count */
#define STATUS_WRITE_BYTES		2 /* Status write bytes count */

/*
 * Flash not busy mask in the status register of the flash device.
 */
#define FLASH_SR_IS_READY_MASK		0x01 /* Ready mask */

/*
 * Number of bytes per page in the flash device.
 */
#define PAGE_SIZE			256

/*
 * Address of the page to perform Erase, Write and Read operations.
 */
#define FLASH_TEST_ADDRESS		0x00

/*
 * Byte Positions.
 */
#define BYTE1				0 /* Byte 1 position */
#define BYTE2				1 /* Byte 2 position */
#define BYTE3				2 /* Byte 3 position */
#define BYTE4				3 /* Byte 4 position */
#define BYTE5				4 /* Byte 5 position */
#define BYTE6				5 /* Byte 6 position */
#define BYTE7				6 /* Byte 7 position */
#define BYTE8				7 /* Byte 8 position */

/*
 * The following definitions specify the number of dummy bytes to ignore in the
 * data read from the flash, through various Read commands. This is apart from
 * the dummy bytes returned in response to the command and address transmitted.
 */
/*
 * After transmitting Dual Read command and address on DIO0,the quad spi device
 * configures DIO0 and DIO1 in input mode and receives data on both DIO0 and
 * DIO1 for 8 dummy clock cycles. So we end up with 16 dummy bits in DRR. The
 * same logic applies Quad read command, so we end up with 4 dummy bytes in
 * that case.
 */
#define DUAL_READ_DUMMY_BYTES		2
#define QUAD_READ_DUMMY_BYTES		4

#define DUAL_IO_READ_DUMMY_BYTES	2
#define QUAD_IO_READ_DUMMY_BYTES	5

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int SpiFlashWriteEnable(XSpi *SpiPtr);
int SpiFlashWrite(XSpi *SpiPtr, u32 Addr, u32 ByteCount, u8 WriteCmd);
int SpiFlashRead(XSpi *SpiPtr, u32 Addr, u32 ByteCount, u8 ReadCmd);
int SpiFlashBulkErase(XSpi *SpiPtr);
int SpiFlashSectorErase(XSpi *SpiPtr, u32 Addr);
int SpiFlashGetStatus(XSpi *SpiPtr);
int SpiFlashQuadEnable(XSpi *SpiPtr);
int SpiFlashEnableHPM(XSpi *SpiPtr);
static int SpiFlashWaitForFlashReady(void);
void SpiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);
static int SetupInterruptSystem(XSpi *SpiPtr);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XSpi Spi;
INTC InterruptController;

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile static int TransferInProgress;

/*
 * The following variable tracks any errors that occur during interrupt
 * processing.
 */
static int ErrorCount;

/*
 * Buffers used during read and write transactions.
 */
static u8 ReadBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES + 4];
static u8 WriteBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];

/*
 * Byte offset value written to Flash. This needs to be redefined for writing
 * different patterns of data to the Flash device.
 */
static u8 TestByte = 0x20;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Main function to run the quad flash example.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;
	u32 Index;
	u32 Address;
	XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */

	/*
	 * Initialize the SPI driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h.
	 */
	ConfigPtr = XSpi_LookupConfig(SPI_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(&Spi, ConfigPtr,
				  ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the SPI driver to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(&Spi);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the SPI that will be called from the interrupt
	 * context when an SPI status occurs, specify a pointer to the SPI
	 * driver instance as the callback reference so the handler is able to
	 * access the instance data.
	 */
	XSpi_SetStatusHandler(&Spi, &Spi, (XSpi_StatusHandler)SpiHandler);

	/*
	 * Set the SPI device as a master and in manual slave select mode such
	 * that the slave select signal does not toggle for every byte of a
	 * transfer, this must be done before the slave select is set.
	 */
	Status = XSpi_SetOptions(&Spi, XSP_MASTER_OPTION |
				 XSP_MANUAL_SSELECT_OPTION);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Select the quad flash device on the SPI bus, so that it can be
	 * read and written using the SPI bus.
	 */
	Status = XSpi_SetSlaveSelect(&Spi, SPI_SELECT);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that interrupts and the device are enabled.
	 */
	XSpi_Start(&Spi);

	/*
	 * Specify address in the Quad Serial Flash for the Erase/Write/Read
	 * operations.
	 */
	Address = FLASH_TEST_ADDRESS;

	/*
	 * Perform the Write Enable operation.
	 */
	Status = SpiFlashWriteEnable(&Spi);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform the Sector Erase operation.
	 */
	Status = SpiFlashSectorErase(&Spi, Address);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Perform the Write Enable operation.
	 */
	Status = SpiFlashWriteEnable(&Spi);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write the data to the Page using Page Program command.
	 */
	Status = SpiFlashWrite(&Spi, Address, PAGE_SIZE, COMMAND_PAGE_PROGRAM);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Clear the read Buffer.
	 */
	for(Index = 0; Index < PAGE_SIZE + READ_WRITE_EXTRA_BYTES; Index++) {
		ReadBuffer[Index] = 0x0;
	}

	/*
	 * Read the data from the Page using Random Read command.
	 */
	Status = SpiFlashRead(&Spi, Address, PAGE_SIZE, COMMAND_RANDOM_READ);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data written.
	 */
	for(Index = 0; Index < PAGE_SIZE; Index++) {
		if(ReadBuffer[Index + READ_WRITE_EXTRA_BYTES] !=
					(u8)(Index + TestByte)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Clear the Read Buffer.
	 */
	for(Index = 0; Index < PAGE_SIZE + READ_WRITE_EXTRA_BYTES +
	    DUAL_READ_DUMMY_BYTES; Index++) {
		ReadBuffer[Index] = 0x0;
	}

	/*
	 * Read the data from the Page using Dual Output Fast Read command.
	 */
	Status = SpiFlashRead(&Spi, Address, PAGE_SIZE, COMMAND_DUAL_READ);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data written.
	 */
	for(Index = 0; Index < PAGE_SIZE; Index++) {
		if(ReadBuffer[Index + READ_WRITE_EXTRA_BYTES +
				DUAL_READ_DUMMY_BYTES] !=
					(u8)(Index + TestByte)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Clear the read Buffer.
	 */
	for(Index = 0; Index < PAGE_SIZE + READ_WRITE_EXTRA_BYTES +
			  QUAD_READ_DUMMY_BYTES; Index++) {
		ReadBuffer[Index] = 0x0;
	}

	/*
	 * Read the data from the Page using Quad Output Fast Read command.
	 */
	Status = SpiFlashRead(&Spi, Address, PAGE_SIZE, COMMAND_QUAD_READ);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
     	 * Compare the data read against the data written.
	 */
	for(Index = 0; Index < PAGE_SIZE; Index++) {
		if(ReadBuffer[Index + READ_WRITE_EXTRA_BYTES +
			QUAD_READ_DUMMY_BYTES] != (u8)(Index + TestByte)) {
				return XST_FAILURE;
			}
	}

	/*
 	 * Perform the Write Enable operation.
 	 */
	Status = SpiFlashWriteEnable(&Spi);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write the data to the next Page using Quad Fast Write command. Erase
	 * is not required since we are writing to next page with in the same
	 * erased sector
	 */
	TestByte = 0x09;
	Address += PAGE_SIZE;

	Status = SpiFlashWrite(&Spi, Address, PAGE_SIZE, COMMAND_QUAD_WRITE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait while the Flash is busy.
	 */
	Status = SpiFlashWaitForFlashReady();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Clear the read Buffer.
	 */
	for(Index = 0; Index < PAGE_SIZE + READ_WRITE_EXTRA_BYTES;
		Index++) {
		ReadBuffer[Index] = 0x0;
	}

	/*
	 * Read the data from the Page using Normal Read command.
	 */
	Status = SpiFlashRead(&Spi, Address, PAGE_SIZE, COMMAND_RANDOM_READ);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data written.
	 */
	for(Index = 0; Index < PAGE_SIZE; Index++) {
		if(ReadBuffer[Index + READ_WRITE_EXTRA_BYTES] !=
					(u8)(Index + TestByte)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Clear the read Buffer.
	 */
	for(Index = 0; Index < PAGE_SIZE + READ_WRITE_EXTRA_BYTES +
	    DUAL_IO_READ_DUMMY_BYTES; Index++) {
		ReadBuffer[Index] = 0x0;
	}


	/*
	 * Read the data from the Page using Dual IO Fast Read command.
	 */
	Status = SpiFlashRead(&Spi, Address, PAGE_SIZE, COMMAND_DUAL_IO_READ);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data written.
	 */
	for(Index = 0; Index < PAGE_SIZE; Index++) {
		if(ReadBuffer[Index + READ_WRITE_EXTRA_BYTES +
				DUAL_IO_READ_DUMMY_BYTES] !=
					(u8)(Index + TestByte)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Clear the read Buffer.
	 */
	for(Index = 0; Index < PAGE_SIZE + READ_WRITE_EXTRA_BYTES +
	    QUAD_IO_READ_DUMMY_BYTES; Index++) {
		ReadBuffer[Index] = 0x0;
	}

	/*
	 * Read the data from the Page using Quad IO Fast Read command.
	 */
	Status = SpiFlashRead(&Spi, Address, PAGE_SIZE, COMMAND_QUAD_IO_READ);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data written.
	 */
	for(Index = 0; Index < PAGE_SIZE; Index++) {
		if(ReadBuffer[Index + READ_WRITE_EXTRA_BYTES +
				QUAD_IO_READ_DUMMY_BYTES] !=
					(u8)(Index + TestByte)) {
			return XST_FAILURE;
		}
	}

	xil_printf("Successfully ran Spi numonyx flash quad Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function enables writes to the Numonyx Serial Flash memory.
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int SpiFlashWriteEnable(XSpi *SpiPtr)
{
	int Status;

	/*
	 * Wait while the Flash is busy.
	 */
	Status = SpiFlashWaitForFlashReady();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the WriteBuffer.
	 */
	WriteBuffer[BYTE1] = COMMAND_WRITE_ENABLE;

	/*
	 * Initiate the Transfer.
	 */
	TransferInProgress = TRUE;
	Status = XSpi_Transfer(SpiPtr, WriteBuffer, NULL,
				WRITE_ENABLE_BYTES);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction..
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes the data to the specified locations in the Numonyx Serial
* Flash memory.
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
* @param	Addr is the address in the Buffer, where to write the data.
* @param	ByteCount is the number of bytes to be written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int SpiFlashWrite(XSpi *SpiPtr, u32 Addr, u32 ByteCount, u8 WriteCmd)
{
	u32 Index;
	int Status;

	/*
	 * Wait while the Flash is busy.
	 */
	Status = SpiFlashWaitForFlashReady();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the WriteBuffer.
	 */
	WriteBuffer[BYTE1] = WriteCmd;
	WriteBuffer[BYTE2] = (u8) (Addr >> 16);
	WriteBuffer[BYTE3] = (u8) (Addr >> 8);
	WriteBuffer[BYTE4] = (u8) Addr;


	/*
	 * Fill in the TEST data that is to be written into the Numonyx Serial
	 * Flash device.
	 */
	for(Index = 4; Index < ByteCount + READ_WRITE_EXTRA_BYTES; Index++) {
		WriteBuffer[Index] = (u8)((Index - 4) + TestByte);
	}

	/*
	 * Initiate the Transfer.
	 */
	TransferInProgress = TRUE;
	Status = XSpi_Transfer(SpiPtr, WriteBuffer, NULL,
				(ByteCount + READ_WRITE_EXTRA_BYTES));
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction.
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the data from the Numonyx Serial Flash Memory
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
* @param	Addr is the starting address in the Flash Memory from which the
*		data is to be read.
* @param	ByteCount is the number of bytes to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int SpiFlashRead(XSpi *SpiPtr, u32 Addr, u32 ByteCount, u8 ReadCmd)
{
	int Status;

	/*
	 * Wait while the Flash is busy.
	 */
	Status = SpiFlashWaitForFlashReady();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the WriteBuffer.
	 */
	WriteBuffer[BYTE1] = ReadCmd;
	WriteBuffer[BYTE2] = (u8) (Addr >> 16);
	WriteBuffer[BYTE3] = (u8) (Addr >> 8);
	WriteBuffer[BYTE4] = (u8) Addr;

	if (ReadCmd == COMMAND_DUAL_READ) {
		ByteCount += DUAL_READ_DUMMY_BYTES;
	} else if (ReadCmd == COMMAND_DUAL_IO_READ) {
		ByteCount += DUAL_READ_DUMMY_BYTES;
	} else if (ReadCmd == COMMAND_QUAD_IO_READ) {
		ByteCount += QUAD_IO_READ_DUMMY_BYTES;
	} else if (ReadCmd==COMMAND_QUAD_READ) {
		ByteCount += QUAD_READ_DUMMY_BYTES;
	}

	/*
	 * Initiate the Transfer.
	 */
	TransferInProgress = TRUE;
	Status = XSpi_Transfer( SpiPtr, WriteBuffer, ReadBuffer,
				(ByteCount + READ_WRITE_EXTRA_BYTES));
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction.
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases the entire contents of the Numonyx Serial Flash device.
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The erased bytes will read as 0xFF.
*
******************************************************************************/
int SpiFlashBulkErase(XSpi *SpiPtr)
{
	int Status;

	/*
	 * Wait while the Flash is busy.
	 */
	Status = SpiFlashWaitForFlashReady();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the WriteBuffer.
	 */
	WriteBuffer[BYTE1] = COMMAND_BULK_ERASE;

	/*
	 * Initiate the Transfer.
	 */
	TransferInProgress = TRUE;
	Status = XSpi_Transfer(SpiPtr, WriteBuffer, NULL,
						BULK_ERASE_BYTES);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction..
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases the contents of the specified Sector in the Numonyx
* Serial Flash device.
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
* @param	Addr is the address within a sector of the Buffer, which is to
*		be erased.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The erased bytes will be read back as 0xFF.
*
******************************************************************************/
int SpiFlashSectorErase(XSpi *SpiPtr, u32 Addr)
{
	int Status;

	/*
	 * Wait while the Flash is busy.
	 */
	Status = SpiFlashWaitForFlashReady();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the WriteBuffer.
	 */
	WriteBuffer[BYTE1] = COMMAND_SECTOR_ERASE;
	WriteBuffer[BYTE2] = (u8) (Addr >> 16);
	WriteBuffer[BYTE3] = (u8) (Addr >> 8);
	WriteBuffer[BYTE4] = (u8) (Addr);

	/*
	 * Initiate the Transfer.
	 */
	TransferInProgress = TRUE;
	Status = XSpi_Transfer(SpiPtr, WriteBuffer, NULL,
					SECTOR_ERASE_BYTES);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction..
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the Status register of the Numonyx Flash.
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The status register content is stored at the second byte
*		pointed by the ReadBuffer.
*
******************************************************************************/
int SpiFlashGetStatus(XSpi *SpiPtr)
{
	int Status;

	/*
	 * Prepare the Write Buffer.
	 */
	WriteBuffer[BYTE1] = COMMAND_STATUSREG_READ;

	/*
	 * Initiate the Transfer.
	 */
	TransferInProgress = TRUE;
	Status = XSpi_Transfer(SpiPtr, WriteBuffer, ReadBuffer,
						STATUS_READ_BYTES);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction..
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* This function waits till the Numonyx serial Flash is ready to accept next
* command.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This function reads the status register of the Buffer and waits
*.		till the WIP bit of the status register becomes 0.
*
******************************************************************************/
int SpiFlashWaitForFlashReady(void)
{
	int Status;
	u8 StatusReg;

	while(1) {

		/*
		 * Get the Status Register. The status register content is
		 * stored at the second byte pointed by the ReadBuffer.
		 */
		Status = SpiFlashGetStatus(&Spi);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Check if the flash is ready to accept the next command.
		 * If so break.
		 */
		StatusReg = ReadBuffer[1];
		if((StatusReg & FLASH_SR_IS_READY_MASK) == 0) {
			break;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing for the SPI driver.
* It is called from an interrupt context such that the amount of processing
* performed should be minimized. It is called when a transfer of SPI data
* completes or an error occurs.
*
* This handler provides an example of how to handle SPI interrupts and
* is application specific.
*
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
* @param	StatusEvent is the event that just occurred.
* @param	ByteCount is the number of bytes transferred up until the event
*		occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void SpiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount)
{
	/*
	 * Indicate the transfer on the SPI bus is no longer in progress
	 * regardless of the status event.
	 */
	TransferInProgress = FALSE;

	/*
	 * If the event was not transfer done, then track it as an error.
	 */
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		ErrorCount++;
	}
}

/*****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the Spi device. This function is application specific since the actual
* system may or may not have an interrupt controller. The Spi device could be
* directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
static int SetupInterruptSystem(XSpi *SpiPtr)
{

	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
	 */
	Status = XIntc_Connect(&InterruptController,
				SPI_INTR_ID,
				(XInterruptHandler)XSpi_InterruptHandler,
				(void *)SpiPtr);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the SPI can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the SPI.
	 */
	XIntc_Enable(&InterruptController, SPI_INTR_ID);

#else /* SCUGIC */
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	XScuGic_SetPriorityTriggerType(&InterruptController, SPI_INTR_ID,
					0xA0, 0x3);


	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(&InterruptController, SPI_INTR_ID,
				 (Xil_InterruptHandler)XSpi_InterruptHandler,
					 (void *)SpiPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the SPI device.
	 */
	XScuGic_Enable(&InterruptController, SPI_INTR_ID);


#endif

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)INTC_HANDLER,
				&InterruptController);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
