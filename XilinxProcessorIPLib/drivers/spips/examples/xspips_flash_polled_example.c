/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xspips_flash_polled_example.c
*
*
* This file contains a design example using the SPI driver (XSpiPs) in
* polled mode with a Serial Flash device. This examples performs
* transfers in polled mode.
* The hardware which this example runs on, must have a Serial Flash
* for it to run. This example has been tested with SST25W080.
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
* 1.00  sg  1/30/13  First release
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block for doxygen
*                    generation and also modified filename tag to include
*                    the file in doxygen examples.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xplatform_info.h"
#include "xspips.h"		/* SPI device driver */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID

/*
 * The following constants define the commands which may be sent to the flash
 * device.
 */
#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD		0x02
#define READ_CMD		0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define CHIP_ERASE_CMD		0x60
#define BULK_ERASE_CMD		0xC7
#define	BLOCK_ERASE_64K_CMD	0xD8
#define READ_ID			0x90
#define SST_READ_ID		0x9F
#define AAI_WRITE_CMD		0xAD
/* Global Block-Protection Unlock register */
#define GLOBAL_BLK_PROT_UNLK	0x98

/* All SST flash parts have id as 0xBF */
#define SST_FLASH_ID		0xBF

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the SPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* Flash instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define DATA_OFFSET		4 /* Start of Data for Read/Write */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast read */
#define RD_ID_SIZE		4 /* Read ID command + 3 bytes ID response */

/*
 * The following constants specify the extra bytes which are sent to the
 * flash on the SPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4

#define UNIQUE_VALUE		0x05

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the flash.
 */
#define MAX_DATA		1024*1024

/*
 * The following constant defines the slave select signal that is used to
 * to select the flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define FLASH_SPI_SELECT_1	0x01
#define FLASH_SPI_SELECT_0	0x00

/* Flag stating sst flash or not */
static int is_sst;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void FlashErase(XSpiPs *SpiPtr);

static void FlashWrite(XSpiPs *SpiPtr, u32 Address, u32 ByteCount, u8 Command);

static void FlashRead(XSpiPs *SpiPtr, u32 Address, u32 ByteCount, u8 Command);

int SpiPsFlashPolledExample(XSpiPs *SpiInstancePtr, u16 SpiDeviceId);

static int FlashReadID(XSpiPs *SpiInstance);

static int SST_GlobalBlkProtectUnlk(XSpiPs *SpiInstancePtr);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
#ifndef TESTAPP_GEN
static XSpiPs SpiInstance;
#endif

/*
 * Write Address Location in Serial Flash.
 */
static int TestAddress;

/*
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];
u8 WriteBuffer[MAX_DATA + DATA_OFFSET];

/*****************************************************************************/
/**
*
* Main function to call the SPI Flash Polled Example.
*
* @param	None
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("SPI SerialFlash Polled Example Test \r\n");

	/*
	 * Run the Spi Polled example.
	 */
	Status = SpiPsFlashPolledExample(&SpiInstance,SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI SerialFlash Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SPI SerialFlash Polled Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XSpiPs
* device driver in polled mode. This function writes and reads data
* from a serial flash.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
*
* @param	SpiDeviceId is the Instance Id of SPI in the system.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note
*
* If the device slave select is not correct and the device is not responding
* on bus it will read a status of 0xFF for the status register as the bus
* is pulled up.
*
*****************************************************************************/
int SpiPsFlashPolledExample(XSpiPs *SpiInstancePtr,
			 u16 SpiDeviceId)
{
	int Status;
	u8 *BufferPtr;
	u8 UniqueValue;
	u32 Count;
	u32 MaxSize = MAX_DATA;
	u32 ChipSelect = FLASH_SPI_SELECT_1;
	XSpiPs_Config *SpiConfig;

	if (XGetPlatform_Info() == XPLAT_ZYNQ_ULTRA_MP) {
		MaxSize = 1024 * 10;
		ChipSelect = FLASH_SPI_SELECT_0;	/* Device is on CS 0 */
	}

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
	SpiConfig = XSpiPs_LookupConfig(SpiDeviceId);
	if (NULL == SpiConfig) {
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize(SpiInstancePtr, SpiConfig,
					SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XSpiPs_SelfTest(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the SPI device as a master with manual start and manual
	 * chip select mode options
	 */
	XSpiPs_SetOptions(SpiInstancePtr, XSPIPS_MANUAL_START_OPTION | \
			XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);

	/*
	 * Set the SPI device pre-scalar to divide by 8
	 */
	XSpiPs_SetClkPrescaler(SpiInstancePtr, XSPIPS_CLK_PRESCALE_8);

	memset(WriteBuffer, 0x00, sizeof(WriteBuffer));
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/*
	 * Initialize the write buffer for a pattern to write to the Flash
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxSize;
		 Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue);
	}

	/*
	 * Set the flash chip select
	 */
	XSpiPs_SetSlaveSelect(SpiInstancePtr, ChipSelect);

	/*
	 * Read the flash Id
	 */
	Status = FlashReadID(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI Flash Polled Example Read ID Failed\r\n");
		return XST_FAILURE;
	}

	if (is_sst == 1) {
		/* Unlock  the Global Block-Protection Unlock register bits */
		SST_GlobalBlkProtectUnlk(SpiInstancePtr);
		if (Status != XST_SUCCESS) {
			xil_printf("SPI Flash Polled Example Read ID Failed\r\n");
			return XST_FAILURE;
		}
	}

	/*
	 * Erase the flash
	 */
	FlashErase(SpiInstancePtr);

	TestAddress = 0x0;
	/*
	 * Write the data in the write buffer to TestAddress in serial flash
	 */
	FlashWrite(SpiInstancePtr, TestAddress, MaxSize, WRITE_CMD);

	/*
	 * Read the contents of the flash from TestAddress of size MAX_DATA
	 * using Normal Read command
	 */
	FlashRead(SpiInstancePtr, TestAddress, MaxSize, READ_CMD);

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

	memset(WriteBuffer, 0x00, sizeof(WriteBuffer));
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/*
	 * Initialize the write buffer for a pattern to write to the Flash
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxSize;
		 Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue);
	}

	/*
	 * Set the SPI device as a master with auto start and manual
	 * chip select mode options
	 */
	XSpiPs_SetOptions(SpiInstancePtr, XSPIPS_MASTER_OPTION | \
			XSPIPS_FORCE_SSELECT_OPTION);

	/*
	 * Erase the flash
	 */
	FlashErase(SpiInstancePtr);

	TestAddress = 0x0;
	/*
	 * Write the data in the write buffer to TestAddress in serial flash
	 */
	FlashWrite(SpiInstancePtr, TestAddress, MaxSize, WRITE_CMD);


	/*
	 * Read the contents of the flash from TestAddress of size MAX_DATA
	 * using Normal Read command
	 */
	FlashRead(SpiInstancePtr, TestAddress, MaxSize, READ_CMD);

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

/******************************************************************************
*
*
* This function writes to the desired address in serial flash connected to
* the SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
* @param	Address contains the address to write data to in the flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FlashWrite(XSpiPs *SpiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 WriteDisableCmd = { WRITE_DISABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];
	u32 Temp = 0;
	u32 TempAddress = Address;
	u8 TempBuffer[5];

	if (Command == WRITE_CMD) {
		for (Temp = 0; Temp < ByteCount ; Temp++, TempAddress++) {
			/*
			 * Send the write enable command to the flash so
			 * that it can be written to, this needs to be sent
			 * as a seperate transfer before the write
			 */
			XSpiPs_PolledTransfer(SpiPtr, &WriteEnableCmd, NULL,
						sizeof(WriteEnableCmd));

			/*
			 * Setup the write command with the specified address
			 * and data for the flash
			 */
			TempBuffer[COMMAND_OFFSET] = Command;
			TempBuffer[ADDRESS_1_OFFSET] =
					 (u8)((TempAddress & 0xFF0000) >> 16);
			TempBuffer[ADDRESS_2_OFFSET] =
					 (u8)((TempAddress & 0xFF00) >> 8);
			TempBuffer[ADDRESS_3_OFFSET] =
					 (u8)(TempAddress & 0xFF);
			TempBuffer[DATA_OFFSET] =
					 WriteBuffer[DATA_OFFSET + Temp];

			/*
			 * Send the write command, address, and data to the
			 * flash to be written, no receive buffer is specified
			 * since there is nothing to receive
			 */
			XSpiPs_PolledTransfer(SpiPtr, TempBuffer, NULL, 5);

			/*
			 * Wait for the write command to the flash to be ,
			 * completed it takes some time for the data to be
			 * written
			 */
			while (1) {
				/*
				 * Poll the status register of the flash to
				 * determine when it completes, by sending
				 * a read status command and receiving the
				 * status byte
				 */

				XSpiPs_PolledTransfer(SpiPtr, ReadStatusCmd,
					 FlashStatus, sizeof(ReadStatusCmd));

				/*
				 * If the status indicates the write is done,
				 * then stop waiting, if a value of 0xFF in
				 * the status byte is read from the device
				 * and this loop never exits, the device slave
				 * select is possibly incorrect such that the
				 * device status is not being read
				 */
				if ((FlashStatus[1] & 0x01) == 0) {
					break;
				}
			}

			XSpiPs_PolledTransfer(SpiPtr, &WriteDisableCmd,
					 NULL, sizeof(WriteDisableCmd));
		}
	}
}

/******************************************************************************
*
* This function reads from the  serial flash connected to the
* SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
* @param	Address contains the address to read data from in the flash.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash. SPI
*		device supports one of the Read, Fast Read commands to read
*		data from the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FlashRead(XSpiPs *SpiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	/*
	 * Setup the read command with the specified address and data for the
	 * flash
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	XSpiPs_PolledTransfer(SpiPtr, WriteBuffer, ReadBuffer,
			  ByteCount + OVERHEAD_SIZE);

}

/******************************************************************************
*
* This function Unlocks the Global Block-Protection Unlock register bits.
*
* @param        None.
*
* @return
*               - XST_SUCCESS if successful
*               - XST_FAILURE if not successful
*
* @note         None.
*
******************************************************************************/
static int SST_GlobalBlkProtectUnlk(XSpiPs *SpiInstancePtr)
{
	int Status;
	u8 WriteEnable[] = { WRITE_ENABLE_CMD };
	u8 ulbpr[] = { GLOBAL_BLK_PROT_UNLK };

	/* send wite enable */
	Status = XSpiPs_PolledTransfer(SpiInstancePtr, WriteEnable, NULL,sizeof(WriteEnable));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Unlock  the Global Block-Protection Unlock register bits */
	Status = XSpiPs_PolledTransfer(SpiInstancePtr, ulbpr, NULL, sizeof(ulbpr));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/******************************************************************************
*
* This function reads serial flash ID connected to the SPI interface.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note		None.
*
******************************************************************************/
static int FlashReadID(XSpiPs *SpiInstance)
{
	u8 Index;
	int Status;
	u8 ByteCount = 4;
	u8 SendBuffer[8];
	u8 RecvBuffer[8];

	SendBuffer[0] = READ_ID;
	SendBuffer[1] = 0;
	SendBuffer[2] = 0;
	SendBuffer[3] = 0;

	for(Index=0; Index < ByteCount; Index++) {
		SendBuffer[4 + Index] = 0x00;
	}

	Status = XSpiPs_PolledTransfer(SpiInstance, SendBuffer, RecvBuffer,
			 (4 + ByteCount));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if ((RecvBuffer[4] == 0xff) || (RecvBuffer[4] == 0x00)) {
		/* Use SST_READ_ID(0x9f) for reading id*/
		SendBuffer[0] = SST_READ_ID;
		Status = XSpiPs_PolledTransfer(SpiInstance, SendBuffer, RecvBuffer,
				(4 + ByteCount));

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (RecvBuffer[4] == SST_FLASH_ID) {
			/* SST flash part */
			is_sst = 1;
		}
	}

	for(Index=0; Index < ByteCount; Index++) {
		xil_printf("ID : %0x\r\n", RecvBuffer[4 + Index]);
	}

	return XST_SUCCESS;
}

/******************************************************************************
*
*
* This function erases the sectors in the  serial flash connected to the
* SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FlashErase(XSpiPs *SpiPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	/* must send 2 bytes */
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0x0 };
	/* must send 2 bytes */
	u8 WriteStatusCmd[] = { WRITE_STATUS_CMD, 0x0 };
	u8 FlashStatus[3];

	/*
	 * Send the write enable command to the flash so that it can be
	 * written to, this needs to be sent as a separate transfer
	 * before the erase
	 */
	XSpiPs_PolledTransfer(SpiPtr, &WriteEnableCmd, NULL,
			  sizeof(WriteEnableCmd));

	while (1) {
		XSpiPs_PolledTransfer(SpiPtr, ReadStatusCmd, FlashStatus,
				  sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write enabled, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x02) == 0x02) {
			break;
		}
	}

	/*
	 * Clear write protect bits using write status command to the flash
	 * so that it can be written to, this needs to be sent as a
	 * separate transfer before the erase
	 */
	XSpiPs_PolledTransfer(SpiPtr, WriteStatusCmd, NULL,
			  sizeof(WriteStatusCmd));
	while (1) {
		XSpiPs_PolledTransfer(SpiPtr, ReadStatusCmd, FlashStatus,
				  sizeof(ReadStatusCmd));
		/*
		 * If the status indicates the WP bits cleared, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x1C) == 0x0) {
			break;
		}
	}

	/*
	 * Send the write enable command to the flash so that it can be
	 * written to, this needs to be sent as a separate transfer
	 * before the erase
	 */
	XSpiPs_PolledTransfer(SpiPtr, &WriteEnableCmd, NULL,
			  sizeof(WriteEnableCmd));

	while (1) {
		XSpiPs_PolledTransfer(SpiPtr, ReadStatusCmd, FlashStatus,
				  sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write enabled, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x02) == 0x02) {
			break;
		}
	}

	/*
	 * Performs chip erase.
	 */
	if (is_sst == 0) {
		WriteBuffer[COMMAND_OFFSET] = CHIP_ERASE_CMD;
	} else {
		WriteBuffer[COMMAND_OFFSET] = BULK_ERASE_CMD;
	}

	XSpiPs_PolledTransfer(SpiPtr, WriteBuffer, NULL, 1);

	/*
	 * Wait for the erase command to the flash to be completed
	 */
	while (1) {
		XSpiPs_PolledTransfer(SpiPtr, ReadStatusCmd, FlashStatus,
				  sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}
}
