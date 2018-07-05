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
* @file xspips_eeprom_polled_example.c
*
* This file contains a design example using the SPI driver (XSpiPs) in
* polled mode and hardware device with a serial EEPROM device.  The
* hardware which this example runs on must have a serial EEPROM (Microchip
* 25XX320 or 25XX160) for it to run.  This example has been tested with the
* SPI EEPROM on the EP4.5 ARM processor.
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
* 1.00  sdm  03/09/10 First release
* 1.00  sdm  10/25/11 Updated the chip select to be used to second chip select
*       ms   04/05/17 Modified Comment lines in functions to
*                     recognize it as documentation block for doxygen
*                     generation and also modified filename tag to include
*                     the file in doxygen examples.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
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
 * The following constants define the commands which may be sent to the EEPROM
 * device.
 */
#define WRITE_STATUS_CMD	1
#define WRITE_CMD		2
#define READ_CMD		3
#define WRITE_DISABLE_CMD	4
#define READ_STATUS_CMD		5
#define WRITE_ENABLE_CMD	6

/*
 * The following constants define the offsets within a EepromBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the SPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* EEPROM instruction */
#define ADDRESS_MSB_OFFSET	1 /* MSB of address to read or write */
#define ADDRESS_LSB_OFFSET	2 /* LSB of address to read or write */
#define DATA_OFFSET		3
#define WRITE_DATA_OFFSET	3  /* Start of data to write to the EEPROM */
#define READ_DATA_OFFSET	6  /* Start of data read from the EEPROM */

/*
 * The following constants specify the extra bytes which are sent to the
 * EEPROM on the SPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		3

/*
 * The following constants specify the page size and number of pages for the
 * EEPROM.  The page size specifies a max number of bytes that can be written
 * to the EEPROM with a single transfer using the SPI driver.
 */
#define PAGE_SIZE		16
#define PAGE_COUNT		128

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the EEPROM.
 */
#define MAX_DATA		PAGE_COUNT * PAGE_SIZE
#define BUFFER_SIZE		MAX_DATA + READ_DATA_OFFSET

/*
 * The following constant defines the slave select signal that is used to
 * to select the EEPROM device on the SPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define EEPROM_SPI_SELECT	0x01

/**************************** Type Definitions *******************************/

/*
 * The following data type is used to send and receive data to the serial
 * EEPROM device connected to the SPI interface.  It is an array of bytes
 * rather than a structure for portability avoiding packing issues.  The
 * application must setup the data to be written in this buffer and retrieve
 * the data read from it.
 */
typedef u8 EepromBuffer[BUFFER_SIZE];

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void EepromRead(XSpiPs *SpiPtr, u16 Address, int ByteCount,
		EepromBuffer Buffer);

void EepromWrite(XSpiPs *SpiPtr, u16 Address, u8 ByteCount,
		 EepromBuffer Buffer);

int SpiPsEepromPolledExample(XSpiPs *SpiInstancePtr, u16 SpiDeviceId);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.  They could be local
 * but should at least be static so they are zeroed.
 */
static XSpiPs SpiInstance;

/*
 * The following variable allows a test value to be added to the values that
 * are written to the EEPROM such that unique values can be generated to
 * guarantee the writes to the EEPROM were successful
 */
int Test;

/*
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
EepromBuffer ReadBuffer;
EepromBuffer WriteBuffer;

/*****************************************************************************/
/**
*
* Main function to call the Spi Eeprom example.
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
	int Status;

	xil_printf("SPI EEPROM Polled Mode Example Test \r\n");

	/*
	 * Run the Spi Interrupt example.
	 */
	Status = SpiPsEepromPolledExample(&SpiInstance, SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI EEPROM Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SPI EEPROM Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XSpiPs
* device driver in polled mode. This test writes and reads data from a
* serial EEPROM. The serial EEPROM part must be present in the hardware
* to use this example.
*
* @param	SpiInstancePtr is a pointer to the Spi Instance.
* @param	SpiDeviceId is the Device Id of Spi.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
* This function calls functions which contain loops that may be infinite
* if interrupts are not working such that it may not return. If the device
* slave select is not correct and the device is not responding on bus it will
* read a status of 0xFF for the status register as the bus is pulled up.
*
*****************************************************************************/
int SpiPsEepromPolledExample(XSpiPs *SpiInstancePtr, u16 SpiDeviceId)
{
	int Status;
	u8 *BufferPtr;
	u8 UniqueValue;
	int Count;
	int Page;
	XSpiPs_Config *SpiConfig;

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
	 * Set the Spi device as a master. External loopback is required.
	 */
	XSpiPs_SetOptions(SpiInstancePtr, XSPIPS_MASTER_OPTION |
			   XSPIPS_FORCE_SSELECT_OPTION);

	XSpiPs_SetClkPrescaler(SpiInstancePtr, XSPIPS_CLK_PRESCALE_64);

	/*
	 * Initialize the write buffer for a pattern to write to the EEPROM
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = 13, Count = 0; Count < MAX_DATA;
					Count++, UniqueValue++) {
		WriteBuffer[WRITE_DATA_OFFSET + Count] =
					(u8)(UniqueValue + Test);
		ReadBuffer[READ_DATA_OFFSET + Count] = 0xA5;
	}

	/*
	 * Assert the EEPROM chip select
	 */
	XSpiPs_SetSlaveSelect(SpiInstancePtr, EEPROM_SPI_SELECT);

	/*
	 * Write the data in the write buffer to the serial EEPROM a page at a
	 * time, read the data back from the EEPROM and verify it
	 */
	UniqueValue = 13;
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		EepromWrite(SpiInstancePtr, Page * PAGE_SIZE, PAGE_SIZE,
				&WriteBuffer[Page * PAGE_SIZE]);
		EepromRead(SpiInstancePtr, Page * PAGE_SIZE, PAGE_SIZE,
				ReadBuffer);

		BufferPtr = &ReadBuffer[READ_DATA_OFFSET];
		for (Count = 0; Count < PAGE_SIZE; Count++, UniqueValue++) {
			if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
				return XST_FAILURE;
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads from the serial EEPROM connected to the SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver component to use.
* @param	Address contains the address to read data from in the EEPROM.
* @param	ByteCount contains the number of bytes to read.
* @param	Buffer is a buffer to read the data into.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void EepromRead(XSpiPs *SpiPtr, u16 Address, int ByteCount,
		EepromBuffer Buffer)
{
	/*
	 * Setup the write command with the specified address and data for the
	 * EEPROM
	 */
	Buffer[COMMAND_OFFSET]     = READ_CMD;
	Buffer[ADDRESS_MSB_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	Buffer[ADDRESS_LSB_OFFSET] = (u8)(Address & 0x00FF);

	/*
	 * Send the read command to the EEPROM to read the specified number
	 * of bytes from the EEPROM, send the read command and address and
	 * receive the specified number of bytes of data in the data buffer
	 */
	XSpiPs_PolledTransfer(SpiPtr, Buffer, &Buffer[DATA_OFFSET],
				ByteCount + OVERHEAD_SIZE);
}

/*****************************************************************************/
/**
*
* This function writes to the serial EEPROM connected to the SPI interface.
* This function is not designed to be a driver to handle all
* the conditions of the EEPROM device.  The EEPROM contains a 32 byte write
* buffer which can be filled and then a write is automatically performed by
* the device.  All the data put into the buffer must be in the same page of
* the device with page boundaries being on 32 byte boundaries.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
* @param	Address contains the address to write data to in the EEPROM.
* @param	ByteCount contains the number of bytes to write.
* @param	Buffer is a buffer of data to write from.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void EepromWrite(XSpiPs *SpiPtr, u16 Address, u8 ByteCount,
		 EepromBuffer Buffer)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 EepromStatus[2];
	int DelayCount = 0;

	/*
	 * Send the write enable command to the SEEPOM so that it can be
	 * written to, this needs to be sent as a seperate transfer before
	 * the write
	 */
	XSpiPs_PolledTransfer(SpiPtr, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));

	/*
	 * Setup the write command with the specified address and data for the
	 * EEPROM
	 */
	Buffer[COMMAND_OFFSET]     = WRITE_CMD;
	Buffer[ADDRESS_MSB_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	Buffer[ADDRESS_LSB_OFFSET] = (u8)(Address & 0x00FF);

	/*
	 * Send the write command, address, and data to the EEPROM to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	XSpiPs_PolledTransfer(SpiPtr, Buffer, NULL, ByteCount + OVERHEAD_SIZE);

	/*
	 * Wait for a bit of time to allow the programming to occur as reading
	 * the status while programming causes it to fail because of noisy power
	 * on the board containing the EEPROM, this loop does not need to be
	 * very long but is longer to hopefully work for a faster processor
	 */
	while (DelayCount++ < 10000) {
	}

	/*
	 * Wait for the write command to the EEPROM to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the device to determine when it
		 * completes by sending a read status command and receiving the
		 * status byte
		 */
		XSpiPs_PolledTransfer(SpiPtr, ReadStatusCmd, EepromStatus,
					sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		if ((EepromStatus[1] & 0x03) == 0) {
			break;
		}
	}
}
