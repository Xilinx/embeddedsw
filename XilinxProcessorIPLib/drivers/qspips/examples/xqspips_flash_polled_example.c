/******************************************************************************
*
* Copyright (C) 2010 - 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xqspips_flash_polled_example.c
*
*
* This file contains a design example using the QSPI driver (XQspiPs) in
* polled mode with a serial FLASH device. This examples performs
* some transfers in Auto mode and Manual start mode, to illustrate the modes
* available. It is recommended to use Manual CS + Auto start for
* best performance.
* The hardware which this example runs on. must have a serial FLASH (Numonyx
* N25Q, Winbond W25Q, Spansion S25FL, ISSI IS25WP) for it to run. This example
* has been tested with the Numonyx Serial Flash (N25Q128) and IS25WP series
* flash parts.
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
* 1.00  sdm 11/25/10 First release
* 1.01  srt 06/12/12 Changed to meet frequency requirements of READ command
*                    for CR 663787
* 2.00a	kka 22/08/12 Updated the example as XQspiPs_PolledTransfer API has
*		     changed. Changed the prescalar to use divide by 8.
*		     The user can change the prescalar to a maximum of
*		     divide by 2 based on the reference clock in the
*		     system.
* 	 	     Set the Holdb_dr bit in the configuration register using
*		     XQSPIPS_HOLD_B_DRIVE_OPTION. Setting this bit
*		     drives the HOLD bit of the QSPI controller.
*		     This is required for QSPI to be used in Non QSPI boot
*		     mode else there needs to be an external pullup on this
*		     line.
*		     See http://www.xilinx.com/support/answers/47596.htm.
* 2.01a sg  02/03/13 Created a function FlashReadID. Removed multiple
*		     initialization using SetOptions.
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block for doxygen
*                    generation.
* 3.5	tjs 07/16/18 Added support for low density ISSI flash parts.
*		     Added FlashQuadEnable API to enable quad mode in flash.
* 3.6   akm 04/15/19 Modified FlashQuadEnable, FlashWrie and FlashErase APIs,
*                    to wait for the on going operation to complete before
*                    performing the next operation.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xqspips.h"		/* QSPI device driver */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID

/*
 * The following constants define the commands which may be sent to the FLASH
 * device.
 */
#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD		0x02
#define READ_CMD		0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define	SEC_ERASE_CMD		0xD8
#define READ_ID			0x9F

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* FLASH instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define DATA_OFFSET		4 /* Start of Data for Read/Write */
#define DUMMY_OFFSET		4 /* Dummy byte offset for fast, dual and quad
				   * reads
				   */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast, dual and
				   * quad reads
				   */
#define RD_ID_SIZE		4 /* Read ID command + 3 bytes ID response */
#define BULK_ERASE_SIZE		1 /* Bulk Erase command size */
#define SEC_ERASE_SIZE		4 /* Sector Erase command + Sector address */

/*
 * The following constants specify the extra bytes which are sent to the
 * FLASH on the QSPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4

/*
 * The following constants specify the page size, sector size, and number of
 * pages and sectors for the FLASH.  The page size specifies a max number of
 * bytes that can be written to the FLASH with a single transfer.
 */
#define SECTOR_SIZE		0x10000
#define NUM_SECTORS		0x100
#define NUM_PAGES		0x10000
#define PAGE_SIZE		256

/* Number of flash pages to be written.*/
#define PAGE_COUNT		16

/* Flash address to which data is ot be written.*/
#define TEST_ADDRESS		0x00055000
#define UNIQUE_VALUE		0x05
/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA		(PAGE_COUNT * PAGE_SIZE)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);

void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);

void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);

int FlashReadID(void);

void FlashQuadEnable(XQspiPs *QspiPtr);

int QspiFlashPolledExample(XQspiPs *QspiInstancePtr, u16 QspiDeviceId);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XQspiPs QspiInstance;

/*
 * The following variable allows a test value to be added to the values that
 * are written to the FLASH such that unique values can be generated to
 * guarantee the writes to the FLASH were successful
 */
int Test = 5;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];
u8 WriteBuffer[PAGE_SIZE + DATA_OFFSET];

/*****************************************************************************/
/**
*
* Main function to call the QSPI Flash example.
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

	xil_printf("QSPI FLASH Polled Example Test \r\n");

	/* Run the Qspi Interrupt example.*/
	Status = QspiFlashPolledExample(&QspiInstance, QSPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("QSPI FLASH Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPI FLASH Polled Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XQspiPs
* device driver in polled mode. This function writes and reads data
* from a serial FLASH.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
int QspiFlashPolledExample(XQspiPs *QspiInstancePtr, u16 QspiDeviceId)
{
	int Status;
	u8 *BufferPtr;
	u8 UniqueValue;
	int Count;
	int Page;
	XQspiPs_Config *QspiConfig;

	/* Initialize the QSPI driver so that it's ready to use*/
	QspiConfig = XQspiPs_LookupConfig(QspiDeviceId);
	if (QspiConfig == NULL) {
		return XST_FAILURE;
	}

	Status = XQspiPs_CfgInitialize(QspiInstancePtr, QspiConfig,
					QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Perform a self-test to check hardware build*/
	Status = XQspiPs_SelfTest(QspiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the write buffer for a pattern to write to the FLASH
	 * and the read buffer to zero so it can be verified after the read,
	 * the test value that is added to the unique value allows the value
	 * to be changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PAGE_SIZE;
	     Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue + Test);
	}
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/*
	 * Set Manual Start and Manual Chip select options and drive HOLD_B
	 * pin high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_MANUAL_START_OPTION |
			XQSPIPS_FORCE_SSELECT_OPTION |
			XQSPIPS_HOLD_B_DRIVE_OPTION);

	/* Set the prescaler for QSPI clock*/
	XQspiPs_SetClkPrescaler(QspiInstancePtr, XQSPIPS_CLK_PRESCALE_8);

	/* Assert the FLASH chip select.*/
	XQspiPs_SetSlaveSelect(QspiInstancePtr);


	FlashReadID();

	FlashQuadEnable(QspiInstancePtr);

	/* Erase the flash.*/
	FlashErase(QspiInstancePtr, TEST_ADDRESS, MAX_DATA);

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(QspiInstancePtr, (Page * PAGE_SIZE) + TEST_ADDRESS,
			   PAGE_SIZE, WRITE_CMD);
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Normal Read
	 * command. Change the prescaler as the READ command operates at a
	 * lower frequency.
	 */
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Fast Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, FAST_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Dual Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, DUAL_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Quad Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, QUAD_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Initialize the write buffer for a pattern to write to the FLASH
	 * and the read buffer to zero so it can be verified after the read,
	 * the test value that is added to the unique value allows the value
	 * to be changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PAGE_SIZE;
	     Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue + Test);
	}
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/*
	 * Set Auto Start and Manual Chip select options and drive HOLD_B
	 * pin high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_FORCE_SSELECT_OPTION |
			XQSPIPS_HOLD_B_DRIVE_OPTION);

	/* Erase the flash.*/
	FlashErase(QspiInstancePtr, TEST_ADDRESS, MAX_DATA);

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(QspiInstancePtr, (Page * PAGE_SIZE) + TEST_ADDRESS,
			   PAGE_SIZE, WRITE_CMD);
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Normal Read
	 * command. Change the prescaler as the READ command operates at a
	 * lower frequency.
	 */
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Fast Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, FAST_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Dual Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, DUAL_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Quad Read
	 * command
	 */
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MAX_DATA, QUAD_READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET + DUMMY_SIZE];

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial FLASH connected to the QSPI interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to write data to in the FLASH.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. QSPI
*		device supports only Page Program command to write data to the
*		flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];

	/*
	 * Send the write enable command to the FLASH so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));


	/*
	 * Setup the write command with the specified address and data for the
	 * FLASH
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	/*
	 * Send the write command, address, and data to the FLASH to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
				ByteCount + OVERHEAD_SIZE);

	/*
	 * Wait for the write command to the FLASH to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the FLASH to determine when it
		 * completes, by sending a read status command and receiving the
		 * status byte
		 */
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
					sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		FlashStatus[1] |= FlashStatus[0];
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}
}

/*****************************************************************************/
/**
*
* This function reads from the  serial FLASH connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to read data from in the FLASH.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash. QSPI
*		device supports one of the Read, Fast Read, Dual Read and Fast
*		Read commands to read data from the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	/*
	 * Setup the write command with the specified address and data for the
	 * FLASH
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
	    (Command == QUAD_READ_CMD)) {
		ByteCount += DUMMY_SIZE;
	}
	/*
	 * Send the read command to the FLASH to read the specified number
	 * of bytes from the FLASH, send the read command and address and
	 * receive the specified number of bytes of data in the data buffer
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, ReadBuffer,
				ByteCount + OVERHEAD_SIZE);
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial FLASH connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];
	int Sector;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command
	 */
	if (ByteCount == (NUM_SECTORS * SECTOR_SIZE)) {
		/*
		 * Send the write enable command to the FLASH so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the erase
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/* Setup the bulk erase command*/
		WriteBuffer[COMMAND_OFFSET]   = BULK_ERASE_CMD;

		/*
		 * Send the bulk erase command; no receive buffer is specified
		 * since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					BULK_ERASE_SIZE);

		/* Wait for the erase command to the FLASH to be completed*/
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting; if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			FlashStatus[1] |= FlashStatus[0];
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		return;
	}

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */
	for (Sector = 0; Sector < ((ByteCount / SECTOR_SIZE) + 1); Sector++) {
		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
					sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and data
		 * for the FLASH
		 */
		WriteBuffer[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		WriteBuffer[ADDRESS_1_OFFSET] = (u8)(Address >> 16);
		WriteBuffer[ADDRESS_2_OFFSET] = (u8)(Address >> 8);
		WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					SEC_ERASE_SIZE);

		/*
		 * Wait for the sector erse command to the
		 * FLASH to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			FlashStatus[1] |= FlashStatus[0];
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		Address += SECTOR_SIZE;
	}
}

/*****************************************************************************/
/**
*
* This function reads serial FLASH ID connected to the SPI interface.
*
* @param	None.
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashReadID(void)
{
	int Status;

	/* Read ID in Auto mode.*/
	WriteBuffer[COMMAND_OFFSET]   = READ_ID;
	WriteBuffer[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	WriteBuffer[ADDRESS_2_OFFSET] = 0x08;
	WriteBuffer[ADDRESS_3_OFFSET] = 0x09;

	Status = XQspiPs_PolledTransfer(&QspiInstance, WriteBuffer, ReadBuffer,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[1], ReadBuffer[2],
		   ReadBuffer[3]);

	return XST_SUCCESS;
}

/*****************************************************************************/

/**
 *
 * This function enables quad mode in the serial flash connected to the
 * SPI interface.
 *
 * @param	QspiPtr is a pointer to the QSPI driver component to use.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void FlashQuadEnable(XQspiPs *QspiPtr)
{
	u8 WriteEnableCmd = {WRITE_ENABLE_CMD};
	u8 ReadStatusCmd[] = {READ_STATUS_CMD, 0};
	u8 QuadEnableCmd[] = {WRITE_STATUS_CMD, 0};
	u8 FlashStatus[2];


	if (ReadBuffer[1] == 0x9D) {

		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
					FlashStatus,
					sizeof(ReadStatusCmd));

		QuadEnableCmd[1] = FlashStatus[1] | 1 << 6;

		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		XQspiPs_PolledTransfer(QspiPtr, QuadEnableCmd, NULL,
					sizeof(QuadEnableCmd));
		while (1) {
			/*
			 * Poll the status register of the FLASH to determine when
			 * Quad Mode is enabled and the device is ready, by sending
			 * a read status command and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
					sizeof(ReadStatusCmd));
			/*
			 * If 6th bit is set & 0th bit is reset, then Quad is Enabled
			 * and device is ready.
			 */
			if ((FlashStatus[0] == 0x40) && (FlashStatus[1] == 0x40)) {
				break;
			}
		}
	}
}
