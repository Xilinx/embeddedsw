/******************************************************************************
* Copyright (C) 2008 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xspi_atmel_flash_example.c
*
* This file contains a design example using the SPI driver (XSpi) and
* hardware device with an Atmel Serial Flash Device (AT45XX series).
* This example erases the Page, writes to the Page, reads back from the Page and
* compares the data.
*
* This example works for an Atmel AT45DB161D. The bytes per page
* (ATMEL_PAGE_SIZE) in this device is 528 bytes for default addressing mode and
* 512 bytes in Power-of-2 addressing mode.
* For further details of device refer to the Atmel Datasheet of AT45DB161D
* device.
*
* The ATMEL_PAGE_SIZE constant need to be updated by the user according to the
* Device used.
*
* The ATMEL_FLASH_TEST_ADDRESS constant need to be updated by the user according
* to the serial flash device used, there is no error checking done in the
* example for the address specified by the user.
*
* This example also works with the In-System Flash(ISF) in the S3AN devices.
* The ATMEL_PAGE_SIZE and ATMEL_FLASH_TEST_ADDRESS need to be defined properly
* based on the device used. For further details of the ISF refer to the
* Spartan-3AN FPGA In-System Flash User Guide (UG333).
*
* This example assumes that the underlying processor is MicroBlaze and default
* addressing mode is used in the Flash Device.
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
* 1.00a sdn  02/26/08 First release
* 1.00a sdn  07/02/08 Changed the initialization so that the SPI
*		      Master works in Spi Mode 3 as the In-System Flash
*		      works only in Spi Mode 3
* 3.00a ktn  10/22/09 Converted all register accesses to 32 bit access.
*		      Updated to use the HAL APIs/macros. Replaced call to
*		      XSpi_Initialize API with XSpi_LookupConfig and
*		      XSpi_CfgInitialize.
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Modified Comment lines to follow doxygen rules.
* 4.11  sb   07/11/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#ifndef SDT
#include "xintc.h"		/* Interrupt controller device driver */
#else
#include "xinterrupt_wrap.h"
#endif
#include "xspi.h"		/* SPI device driver */
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define SPI_INTR_ID		XPAR_INTC_0_SPI_0_VEC_ID
#endif

/*
 * The following constant defines the slave select signal that is used to
 * to select the Flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device.
 */
#define ATMEL_SPI_SELECT 0x01


/*
 * Definition of the commands
 */
#define ATMEL_COMMAND_READ		0x03 /* Read command */
#define ATMEL_COMMAND_WRITE		0x82 /* Write command */
#define ATMEL_COMMAND_STATUSREG_READ	0xD7 /* Status Register Read command */
#define ATMEL_COMMAND_PAGE_ERASE	0x81 /* Erase command */

/*
 * The following definition specify the EXTRA bytes in the command
 * transactions. This count includes Command byte, address bytes and any
 * don't care bytes needed.
 */
#define ATMEL_READ_WRITE_EXTRA_BYTES	0x4 /* Read/Write extra bytes */
#define ATMEL_PAGE_ERASE_BYTES		0x4 /* Page erase extra bytes */
#define ATMEL_STATUS_READ_BYTES		0x2 /* Status read bytes count */

/*
 * The following constants define the offsets for command and data.
 * Note that the read data offset is not the same as the write data
 * because the SPI driver is designed to allow full  duplex transfers
 * such that the number of bytes received is the number sent and received.
 */
#define ATMEL_COMMAND_OFFSET      	0
#define ATMEL_ADDRESS_BYTE1_OFFSET	1
#define ATMEL_ADDRESS_BYTE2_OFFSET  	2
#define ATMEL_ADDRESS_BYTE3_OFFSET	3

/*
 * The following definitions specify the status register bit definitions.
 */
#define ATMEL_FLASH_SR_IS_READY_MASK	0x80

#define ATMEL_PAGE_SIZE		264  /* Page Size */

/*
 * Address of the page to perform Erase, Write and Read operations.
 */
#define ATMEL_FLASH_TEST_ADDRESS	0x259800

/*
 * Byte offset value written to Flash. This needs to redefined for writing
 * different patterns of data to the Flash device.
 */
#define ATMEL_TEST_BYTE		0x10 /* Test value written to Flash */


#define ATMEL_DUMMYBYTE			0xFF
#define ATMEL_INITBYTE			0x00

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
static int SetupInterruptSystem(XSpi *SpiPtr);
#endif
void SpiHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);
int SpiAtmelFlashRead(XSpi *SpiPtr, u32 Address, u16 ByteCount);
int SpiAtmelFlashWrite(XSpi *SpiPtr, u32 Address, u16 ByteCount);
int SpiAtmelFlashPageErase(XSpi *SpiPtr, u32 Address);
static int SpiAtmelFlashWaitForFlashNotBusy(XSpi *SpiPtr);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
#ifndef SDT
static XIntc InterruptController;
#endif
static XSpi Spi;

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile static int TransferInProgress;

/*
 * The following variable tracks any errors that occur during interrupt
 * processing.
 */
int ErrorCount;

/*
 * Buffers used during read and write transactions.
 */
u8 ReadBuffer[ATMEL_PAGE_SIZE + ATMEL_READ_WRITE_EXTRA_BYTES];
u8 WriteBuffer[ATMEL_PAGE_SIZE + ATMEL_READ_WRITE_EXTRA_BYTES];

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* Main function to execute the Atmel Flash example.
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
	 * Initialize the SPI driver so that it is  ready to use.
	 */
#ifndef SDT
	ConfigPtr = XSpi_LookupConfig(SPI_DEVICE_ID);
#else
	ConfigPtr = XSpi_LookupConfig(XPAR_XSPI_0_BASEADDR);
#endif
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
#ifndef SDT
	Status = SetupInterruptSystem(&Spi);
#else
	Status = XSetupInterruptSystem(&Spi, &XSpi_InterruptHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
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
				 XSP_MANUAL_SSELECT_OPTION |
				 XSP_CLK_PHASE_1_OPTION |
				 XSP_CLK_ACTIVE_LOW_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Select the slave on the SPI bus so that the Atmel Flash device can be
	 * read and written using the SPI bus.
	 */
	Status = XSpi_SetSlaveSelect(&Spi, ATMEL_SPI_SELECT);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that interrupts and the device are enabled.
	 */
	XSpi_Start(&Spi);


	/*
	 * Specify the address in the flash device for the Erase/Write/Read
	 * operations.
	 */
	Address = ATMEL_FLASH_TEST_ADDRESS;


	/*
	 * Erase the Page.
	 */
	Status = SpiAtmelFlashPageErase(&Spi, Address);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the current Erase command is executed in the Flash
	 * and the Flash is ready for the next command.
	 */
	Status = SpiAtmelFlashWaitForFlashNotBusy(&Spi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write the data to the Page.
	 */
	Status = SpiAtmelFlashWrite(&Spi, Address, ATMEL_PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the current Write command is executed in the Flash
	 * and the Flash is ready for the next command.
	 */
	Status = SpiAtmelFlashWaitForFlashNotBusy(&Spi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read the data from the Page.
	 */
	Status = SpiAtmelFlashRead(&Spi, Address, ATMEL_PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the current Write command is executed in the Flash
	 * and the Flash is ready for the next command.
	 */
	Status = SpiAtmelFlashWaitForFlashNotBusy(&Spi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Compare the data read with what is written.
	 */
	for (Index = ATMEL_READ_WRITE_EXTRA_BYTES; Index < (ATMEL_PAGE_SIZE +
			ATMEL_READ_WRITE_EXTRA_BYTES); Index++) {
		if (ReadBuffer[Index] != (u8)(Index + ATMEL_TEST_BYTE)) {
			return XST_FAILURE;
		}
	}

	xil_printf("Successfully ran Spi atmel flash Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads data from the Atmel Flash device connected to the SPI
* interface.
*
* @param	SpiPtr is a pointer to the SPI driver component to use.
* @param	Address is the address from which the data is to be read from
*		the Flash.
* @param	ByteCount is the number of bytes to read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiAtmelFlashRead(XSpi *SpiPtr, u32 Address, u16 ByteCount)
{
	u16 Index;

	/*
	 * Setup the read command with the specified address and data for the
	 * Atmel Flash.
	 */
	WriteBuffer[ATMEL_COMMAND_OFFSET]	= ATMEL_COMMAND_READ;
	WriteBuffer[ATMEL_ADDRESS_BYTE1_OFFSET] = (u8) (Address >> 16);
	WriteBuffer[ATMEL_ADDRESS_BYTE2_OFFSET] = (u8) (Address >> 8);
	WriteBuffer[ATMEL_ADDRESS_BYTE3_OFFSET] = (u8) Address;

	/*
	 * Prepare the write buffer. Fill in some dummy data.
	 */
	for (Index = 4; Index < (ByteCount + ATMEL_READ_WRITE_EXTRA_BYTES);
	     Index++) {
		WriteBuffer[Index] = ATMEL_DUMMYBYTE;
	}

	/*
	 * Prepare the Read Buffer. Fill in some initialization data into the
	 * the buffer.
	 */
	for (Index = 0; Index < (ByteCount +
				 ATMEL_READ_WRITE_EXTRA_BYTES); Index++) {
		ReadBuffer[Index] = ATMEL_INITBYTE;
	}

	/*
	 * Send the read command to the Atmel Flash to read the specified number
	 * of bytes.
	 */
	TransferInProgress = TRUE;
	XSpi_Transfer(SpiPtr, WriteBuffer, ReadBuffer,
		      ByteCount + ATMEL_READ_WRITE_EXTRA_BYTES);

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction.
	 */
	while (TransferInProgress);
	if (ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the Atmel Flash device connected to the SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver component to use.
* @param	Address is the address to which the data is written.
* @param	ByteCount contains the number of bytes to write.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiAtmelFlashWrite(XSpi *SpiPtr, u32 Address, u16 ByteCount)
{
	u16 Index;

	/*
	 * Setup the write command with the specified address, and data to be
	 * written to the flash.
	 */
	WriteBuffer[ATMEL_COMMAND_OFFSET]     = ATMEL_COMMAND_WRITE;
	WriteBuffer[ATMEL_ADDRESS_BYTE1_OFFSET] = (u8) (Address >> 16);
	WriteBuffer[ATMEL_ADDRESS_BYTE2_OFFSET] = (u8) (Address >> 8);
	WriteBuffer[ATMEL_ADDRESS_BYTE3_OFFSET] = (u8) (Address);

	/*
	 * Prepare the write buffer. Fill in the data that is to be written into
	 * the Flash.
	 */
	for (Index = 4; Index < (ByteCount + ATMEL_READ_WRITE_EXTRA_BYTES);
	     Index++) {
		WriteBuffer[Index] = (u8)(ATMEL_TEST_BYTE + Index);
	}

	/*
	 * Send the write command, address, and data to the Flash.
	 * No receive buffer is specified since there is nothing to receive.
	 */
	TransferInProgress = TRUE;
	XSpi_Transfer(SpiPtr, WriteBuffer, NULL,
		      ByteCount + ATMEL_READ_WRITE_EXTRA_BYTES)
	;

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction.
	 */
	while (TransferInProgress);
	if (ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases the contents of the specified Page in the Flash.
*
* @param	SpiPtr is a pointer to the SPI driver component to use.
* @param	Address contains the address of the page to be erased.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiAtmelFlashPageErase(XSpi *SpiPtr, u32 Address)
{

	/*
	 * Prepare the Write Buffer.
	 */
	WriteBuffer[ATMEL_COMMAND_OFFSET] = ATMEL_COMMAND_PAGE_ERASE;
	WriteBuffer[ATMEL_ADDRESS_BYTE1_OFFSET] = (u8) (Address >> 16);
	WriteBuffer[ATMEL_ADDRESS_BYTE2_OFFSET] = (u8) (Address >> 8);
	WriteBuffer[ATMEL_ADDRESS_BYTE3_OFFSET] = ATMEL_DUMMYBYTE;

	/*
	 * Send the Erase command and address to the flash.
	 * No receive buffer is specified since there is nothing to
	 * receive.
	 */
	TransferInProgress = TRUE;
	XSpi_Transfer(SpiPtr, WriteBuffer, NULL, ATMEL_PAGE_ERASE_BYTES);

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction.
	 */
	while (TransferInProgress);
	if (ErrorCount != 0) {
		ErrorCount = 0;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits till the flash is ready to accept next command.
*
* @param	SpiPtr is a pointer to the SPI driver component to use.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int SpiAtmelFlashWaitForFlashNotBusy(XSpi *SpiPtr)
{
	u8 StatusReg;

	/*
	 * Prepare the Write Buffer.
	 */
	WriteBuffer[ATMEL_COMMAND_OFFSET] = ATMEL_COMMAND_STATUSREG_READ;
	WriteBuffer[ATMEL_ADDRESS_BYTE1_OFFSET] = ATMEL_DUMMYBYTE;

	/*
	 * Prepare the Read Buffer.
	 */
	ReadBuffer[0] = ATMEL_INITBYTE;
	ReadBuffer[1] = ATMEL_INITBYTE;

	while (1) {

		/*
		 * Transmit the data.
		 */
		TransferInProgress = TRUE;
		XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer,
			      ATMEL_STATUS_READ_BYTES);

		/*
		 * Wait for the transmission to be complete and
		 * check if there are any errors in the transaction.
		 */
		while (TransferInProgress);
		if (ErrorCount != 0) {
			ErrorCount = 0;
			return XST_FAILURE;
		}

		StatusReg = ReadBuffer[1];

		if ((StatusReg & ATMEL_FLASH_SR_IS_READY_MASK)) {
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
#ifndef SDT
static int SetupInterruptSystem(XSpi *SpiPtr)
{

	int Status;

	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device
	 */
	Status = XIntc_Connect(&InterruptController,
			       SPI_INTR_ID,
			       (XInterruptHandler)XSpi_InterruptHandler,
			       (void *)SpiPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the SPI can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the SPI.
	 */
	XIntc_Enable(&InterruptController, SPI_INTR_ID);


	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler) XIntc_InterruptHandler,
				     &InterruptController);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif
