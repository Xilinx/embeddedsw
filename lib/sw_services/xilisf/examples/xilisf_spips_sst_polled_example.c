/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilisf_spips_flash_polled_example.c
*
* This file contains a design example using the XILISF Library in
* interrupt mode with a serial FLASH device. This examples performs
* some transfers in Auto mode and Manual start mode, to illustrate the modes
* available.
* The hardware which this example runs on, must have a serial FLASH (Numonyx
* N25Q, Winbond W25Q, SST or Spansion S25FL) for it to run. This example has
* been tested with the SST Flash.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.00  srt 06/20/12 First release
* 3.01  srt 03/03/13 Modified the flash write, erase and read logic.
*		     Ensured flash blocks are unprotected before a flash erase
*		     or write operation. (CR 703816)
* 3.02  srt 04/26/13 Modified Erase function to perform Write Enable operation
*		     for each sector erase.
* 5.4   sk  08/07/15 Modified the example to support on ZynqMP.
* 5.14  akm 08/01/19 Initialized Status variable to XST_FAILURE.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xscugic.h"		/* Interrupt controller device driver */
#include "xil_exception.h"
#include "xil_printf.h"
#include "xplatform_info.h"
#include <xilisf.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the SPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define DATA_OFFSET		4 /* Start of Data for Read/Write */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast, dual and
				     quad reads */

/*
 * The following constants specify the page size, sector size, and number of
 * pages and sectors for the FLASH.  The page size specifies a max number of
 * bytes that can be written to the FLASH with a single transfer.
 */
#define SECTOR_SIZE		0x1000
#define NUM_SECTORS		0x100

/*
 * Flash address to which data is to be written.
 */
#define TEST_ADDRESS		0x00
#define UNIQUE_VALUE		0x05
/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA		SECTOR_SIZE * NUM_SECTORS

/*
 * The following constant defines the slave select signal that is used to
 * to select the FLASH device on the SPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define FLASH_SPI_SELECT_1	0x01
#define FLASH_SPI_SELECT_0 	0x00

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FlashErase(XIsf *InstancePtr, u32 Address, u32 ByteCount);
int FlashWrite(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command);
int FlashRead(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command);
int WriteEnable(XIsf *InstancePtr);
int SpiFlashPolledExample(XSpiPs *SpiInstancePtr, u16 SpiDeviceId);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XSpiPs SpiInstance;
static XIsf Isf;

/*
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];
u8 WriteBuffer[MAX_DATA + DATA_OFFSET];
u8 IsfWriteBuffer[XISF_CMD_SEND_EXTRA_BYTES + 1];

/*****************************************************************************/
/**
*
* Main function to call the SPI Flash example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("SPI FLASH Polled Example Test \r\n");

	/*
	 * Run the Spi Interrupt example.
	 */
	Status = SpiFlashPolledExample(&SpiInstance,SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI FLASH Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SPI FLASH Polled Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************
*
* The purpose of this function is to illustrate how to use the XSpiPs
* device driver in interrupt mode. This function writes and reads data
* from a serial FLASH.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
* This function calls other functions which contain loops that may be infinite
* if interrupts are not working such that it may not return. If the device
* slave select is not correct and the device is not responding on bus it will
* read a status of 0xFF for the status register as the bus is pulled up.
*
*****************************************************************************/
void FlashWriteSPI(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command);
int SpiFlashPolledExample(XSpiPs *SpiInstancePtr,
			 u16 SpiDeviceId)
{
	u8 *BufferPtr;
	u8 UniqueValue;
	u32 Count;
	XSpiPs_Config *ConfigPtr;	/* Pointer to Configuration ROM data */
	u32 TempAddress;
	u32 MaxSize = MAX_DATA;
	u32 ChipSelect = FLASH_SPI_SELECT_1;

	if (XGetPlatform_Info() == XPLAT_ZYNQ_ULTRA_MP) {
		MaxSize = 1024 * 10;
		ChipSelect = FLASH_SPI_SELECT_0;	/* Device is on CS 0 */
	}

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
	ConfigPtr = XSpiPs_LookupConfig(SpiDeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	XSpiPs_CfgInitialize(SpiInstancePtr, ConfigPtr,
				  ConfigPtr->BaseAddress);

	/* Initialize the XILISF Library */
	XIsf_Initialize(&Isf, SpiInstancePtr, ChipSelect,
					   IsfWriteBuffer);

	memset(WriteBuffer, 0x00, sizeof(WriteBuffer));
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/* Unprotect Sectors */
	FlashWrite(&Isf, 0, 0, XISF_WRITE_STATUS_REG);

	FlashErase(&Isf, TEST_ADDRESS, MaxSize);

	/*
	 * Initialize the write buffer for a pattern to write to the FLASH
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	TempAddress = TEST_ADDRESS;
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxSize;
		 Count++, UniqueValue++, TempAddress++) {
		WriteBuffer[0] = (u8)(UniqueValue);
		FlashWrite(&Isf, TempAddress, 1, XISF_WRITE);
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Normal Read
	 * command
	 */
	FlashRead(&Isf, TEST_ADDRESS, MaxSize, XISF_READ);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxSize;
			 Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue)) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits till the Serial Flash is ready to accept next command.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This function reads the status register of the Serial Flash and
		waits till the WIP bit of the status register becomes 0.
*
******************************************************************************/
int IsfWaitForFlashNotBusy(void)
{
	int Status = XST_FAILURE;
	u8 StatusReg;

	while(1) {

		/*
		 * Get the Status Register.
		 */
		Status = XIsf_GetStatus(&Isf, ReadBuffer);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Check if the flash is ready to accept the next command.
		 * If so break.
		 */
		StatusReg = ReadBuffer[BYTE2];
		if((StatusReg & XISF_SR_IS_READY_MASK) == 0) {
			break;
		}
	}

	return XST_SUCCESS;
}

/******************************************************************************
*
*
* This function writes to the  serial FLASH connected to the SPI interface.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address to write data to in the FLASH.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashWrite(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command)
{
	XIsf_WriteParam WriteParam;

	int Status = XST_FAILURE;

	WriteEnable(InstancePtr);

	WriteParam.Address = Address;
	WriteParam.NumBytes = ByteCount;
	WriteParam.WritePtr = WriteBuffer;

	/*
	 * Perform the Write operation.
	 */
	Status = XIsf_Write(&Isf, Command, (void*) &WriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Flash is not Busy.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/******************************************************************************
*
* This function reads from the  serial FLASH connected to the
* SPI interface.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address to read data from in the FLASH.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashRead(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command)
{
	XIsf_ReadParam ReadParam;
	int Status = XST_FAILURE;

	/*
	 * Set the
	 * - Address in the Serial Flash where the data is to be read from.
	 * - Number of bytes to be read from the Serial Flash.
	 * - Read Buffer to which the data is to be read.
	 */
	ReadParam.Address = Address;
	ReadParam.NumBytes = ByteCount;
	ReadParam.ReadPtr = ReadBuffer;

	/*
	 * Perform the Read operation.
	 */
	Status = XIsf_Read(&Isf, Command, (void*) &ReadParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************
*
*
* This function erases the sectors in the  serial FLASH connected to the
* SPI interface.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashErase(XIsf *InstancePtr, u32 Address, u32 ByteCount)
{
	int Status = XST_FAILURE;
	int Sector;

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */
	for (Sector = 0; Sector < ((ByteCount / SECTOR_SIZE) + 1); Sector++) {

		/*
		 * Write enable instruction has to be executed prior to
		 * any Write operation.
		 */
		WriteEnable(InstancePtr);

		/*
		 * Perform the Sector Erase operation.
		 */
		Status = XIsf_Erase(InstancePtr, XISF_SECTOR_ERASE, Address);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait till the Flash is not Busy.
		 */
		Status = IsfWaitForFlashNotBusy();
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Address += SECTOR_SIZE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function performs the write enable operation.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
*
******************************************************************************/
int WriteEnable(XIsf *InstancePtr)
{
	int Status = XST_FAILURE;

	/*
	 * Perform the Write Enable operation.
	 */
	Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Flash is not Busy.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
