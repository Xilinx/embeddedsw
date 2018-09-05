/******************************************************************************
*
* Copyright (C) 2002 - 2018 Xilinx, Inc.  All rights reserved.
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
* @file xiic_low_level_eeprom_example.c
*
* This file consists of a polled mode design example which uses the Xilinx
* IIC device and low-level driver to exercise the EEPROM.
*
* This example writes/reads from the lower 256 bytes of the IIC EEPROMS. Please
* refer to the datasheets of the IIC EEPROM's for details about the internal
* addressing and page size of these devices.
*
* The XIic_Send() API is used to transmit the data and XIic_Recv() API is used
* to receive the data.
*
* This example is tested on ML300/ML310/ML403/ML501/ML507/ML510/ML605/SP601 and
* SP605 Xilinx boards.
*
* The ML310/ML410/ML510 boards have a on-board 64 Kb serial IIC EEPROM
* (Microchip 24LC64A). The WP pin of the IIC EEPROM is hardwired to ground on
* this board.
*
* The ML300 board has an on-board 32 Kb serial IIC EEPROM(Microchip 24LC32A).
* The WP pin of the IIC EEPROM has to be connected to ground for this example.
* The WP is connected to pin Y3 of the FPGA.
*
* The ML403 board has an on-board 4 Kb serial IIC EEPROM(Microchip 24LC04A).
* The WP pin of the IIC EEPROM is hardwired to ground on this board.
*
* The ML501/ML505/ML507/ML605/SP601/SP605 boards have an on-board 8 Kb serial
* IIC EEPROM(STM M24C08). The WP pin of the IIC EEPROM is hardwired to
* ground on these boards.
*
* The AddressType for ML300/ML310/ML410/ML510 boards should be u16 as the
* address pointer in the on board EEPROM is 2 bytes.
*
* The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605 boards should
* be u8 as the address pointer for the on board EEPROM is 1 byte.
*
* The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
* ML501/ML505/ML507/ML510 boards is 0x50.
* The 7 bit IIC Slave address of the IIC EEPROM on the ML605/SP601/SP605 boards
* is 0x54.
* Refer to the User Guide's of the respective boards for further information
* about the IIC slave address of IIC EEPROM's.
*
* The define EEPROM_ADDRESS in this file needs to be changed depending on
* the board on which this example is to be run.
*
* This code assumes that no Operating System is being used.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a jhl  09/10/03 Created
* 1.00a sv   05/09/05 Minor changes to comply to Doxygen and coding guidelines
* 1.00a mta  03/09/06 Minor updates due to changes in the low level driver for
*		      supporting repeated start functionality.
* 2.00a sdm  09/22/09 Converted all register accesses to 32 bit access and minor
*		      modifications as per coding guidelines.
* 2.01a ktn  03/17/10 Updated the information about the EEPROM's used on
*		      ML605/SP601/SP605 boards. Updated the example so that it
*		      can be used to access the entire IIC EEPROM for devices
*		      like M24C04/M24C08 that use LSB bits of the IIC device
*		      select code (IIC slave address) to specify the higher
*		      address bits of the EEPROM internal address.
* 2.01a sdm  06/13/11 Updated the example to flush the Tx FIFO when waiting for
*		      the previous command to be completed for CR612546.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 3.5   sd   08/29/18 Update the fifo flush.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_io.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_BASE_ADDRESS	XPAR_IIC_0_BASEADDR

/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 * The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
 * ML501/ML505/ML507/ML510 boards is 0x50. The 7 bit IIC Slave address of the
 * IIC EEPROM on the ML605/SP601/SP605 boards is 0x54.
 * Please refer the User Guide's of the respective boards for further
 * information about the IIC slave address of IIC EEPROM's.
 */
#define EEPROM_ADDRESS	0x54	 /* 0xA0 as an 8 bit number */

/*
 * The page size determines how much data should be written at a time.
 * The ML300 board supports a page size of 32 and 16
 * The write function should be called with this as a maximum byte count.
 */
#define PAGE_SIZE	16

/*
 * The Starting address in the IIC EEPROM on which this test is performed
 */
#define EEPROM_TEST_START_ADDRESS	128


/**************************** Type Definitions *******************************/

/*
 * The AddressType for ML300/ML310/ML510 boards should be u16 as the address
 * pointer in the on board EEPROM is 2 bytes.
 * The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605 boards should
 * be u8 as the address pointer in the on board EEPROM is 1 bytes.
 */
typedef u8 AddressType;


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int IicLowLevelEeprom();

int ReadWriteVerify(AddressType Address);

unsigned EepromWriteByte(AddressType Address, u8 *BufferPtr, u16 ByteCount);

unsigned EepromReadByte(AddressType Address, u8 *BufferPtr, u16 ByteCount);

/************************** Variable Definitions **************************/

int ErrorCount;			  /* The Error Count */

u8 WriteBuffer[PAGE_SIZE];	  /* Write buffer for writing a page */
u8 ReadBuffer[PAGE_SIZE];	  /* Read buffer for reading a page */
u8 ReadBufferAll[PAGE_SIZE * 4];  /* Buffer used for reading all the data */

u8 EepromIicAddr;		  /* Variable for storing Eeprom IIC address */

/*****************************************************************************/
/**
* Main function to call the low level EEPROM example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Low Level EEPROM example.
	 */
	Status = IicLowLevelEeprom();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC lowlevel eeprom Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC lowlevel eeprom Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* The function uses the low level driver of IIC to read and write to the
* IIC EEPROM board. The addresses tested are from 128 to 192.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
****************************************************************************/
int IicLowLevelEeprom()
{
	int Status;
	unsigned BytesRead;
	EepromIicAddr = EEPROM_ADDRESS;

	/*
	 * Read, write and verify a page of data at the specified address.
	 */
	Status = ReadWriteVerify(EEPROM_TEST_START_ADDRESS);
	if (Status != XST_SUCCESS) {
		ErrorCount++;
	}

	/*
	 * Read, write and verify a page of data at the
	 * specified address + PAGE_SIZE.
	 */
	Status = ReadWriteVerify(EEPROM_TEST_START_ADDRESS + PAGE_SIZE);
	if (Status != XST_SUCCESS) {
		ErrorCount++;
	}

	/*
	 * Read, write and verify a page of data at the
	 * specified address + (PAGE_SIZE * 3).
	 */
	Status = ReadWriteVerify(EEPROM_TEST_START_ADDRESS + (PAGE_SIZE * 3));
	if (Status != XST_SUCCESS) {
		ErrorCount++;
	}

	/*
	 * Read, write and verify a page of data at the
	 * specified address + (PAGE_SIZE * 2).
	 */
	Status = ReadWriteVerify(EEPROM_TEST_START_ADDRESS + (PAGE_SIZE * 2));
	if (Status != XST_SUCCESS) {
		ErrorCount++;
	}

	/*
	 * Read all the locations that were written in a single read,
	 * this data is not verified, only read to show that a larger
	 * amount of data can be read.
	*/
	BytesRead = EepromReadByte(EEPROM_TEST_START_ADDRESS,
					ReadBufferAll,
					PAGE_SIZE * 4);
	if (BytesRead != PAGE_SIZE * 4) {
		ErrorCount++;
	}

	if (ErrorCount != 0x0) {
		Status = XST_FAILURE;
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function writes, reads, and verifies the read to the IIC EEPROM.  It
* does the write as a single page write, performs a buffered read, and also
* performs byte reads.
*
* @param	Address is the starting address of the page in the EEPROM device
*		to which the data is to be written.
*
* @return	 XST_FAILURE if the test fails, XST_SUCCESS if the test passes.
*
* @note 	None.
*
****************************************************************************/
int ReadWriteVerify(AddressType Address)
{
	unsigned BytesWritten;
	unsigned BytesRead;
	int Index;

	/*
	 * Initialize the data to written and the read buffer.
	 */
	for (Index = 0; Index < PAGE_SIZE; Index++) {
		WriteBuffer[Index] = Index;
		ReadBuffer[Index] = 0;
	}

	/*
	 * Write to the EEPROM.
	 */
	BytesWritten = EepromWriteByte(Address, WriteBuffer, PAGE_SIZE);
	if (BytesWritten != PAGE_SIZE) {
		return XST_FAILURE;
	}

	/*
	 * Read from the EEPROM.
	 */
	BytesRead = EepromReadByte(Address, ReadBuffer, PAGE_SIZE);
	if (BytesRead != PAGE_SIZE) {
		return XST_FAILURE;
	}

	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < PAGE_SIZE; Index++)
	{
		if (ReadBuffer[Index] != WriteBuffer[Index]) {
			return XST_FAILURE;
		}
		ReadBuffer[Index] = 0;
	}

	/*
	 * Read each byte one at a time and verify.
	 */
	for (Index = 0; Index < PAGE_SIZE; Index++)
	{
		BytesRead = EepromReadByte(Address + Index,
				&ReadBuffer[Index], 1);
		if (BytesRead != 1) {
			return XST_FAILURE;
		}

		if (ReadBuffer[Index] != WriteBuffer[Index]) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of bytes to the IIC serial EEPROM.
*
* @param	Address contains the address in the EEPROM to write to.
* @param	BufferPtr contains the address of the data to write.
* @param	ByteCount contains the number of bytes in the buffer to be written.
*		Note that this should not exceed the page size of the EEPROM as
*		noted by the constant PAGE_SIZE.
*
* @return	The number of bytes written, a value less than that which was
*		specified as an input indicates an error.
*
* @note		None.
*
****************************************************************************/
unsigned EepromWriteByte(AddressType Address, u8 *BufferPtr, u16 ByteCount)
{
	volatile unsigned SentByteCount;
	volatile unsigned AckByteCount;
	u8 WriteBuffer[sizeof(Address) + PAGE_SIZE];
	int Index;
	u32 CntlReg;

	/*
	 * A temporary write buffer must be used which contains both the address
	 * and the data to be written, put the address in first based upon the
	 * size of the address for the EEPROM.
	 */
	if (sizeof(AddressType) == 2) {
		WriteBuffer[0] = (u8)(Address >> 8);
		WriteBuffer[1] = (u8)(Address);
	} else if (sizeof(AddressType) == 1) {
		WriteBuffer[0] = (u8)(Address);
		EepromIicAddr |= (EEPROM_TEST_START_ADDRESS >> 8) & 0x7;
	}

	/*
	 * Put the data in the write buffer following the address.
	 */
	for (Index = 0; Index < ByteCount; Index++) {
		WriteBuffer[sizeof(Address) + Index] = BufferPtr[Index];
	}

	/*
	 * Set the address register to the specified address by writing
	 * the address to the device, this must be tried until it succeeds
	 * because a previous write to the device could be pending and it
	 * will not ack until that write is complete.
	 */
	do {
		SentByteCount = XIic_Send(IIC_BASE_ADDRESS,
					EepromIicAddr,
					(u8 *)&Address, sizeof(Address),
					XIIC_STOP);
		if (SentByteCount != sizeof(Address)) {

			/* Send is aborted so reset Tx FIFO */
			CntlReg = XIic_ReadReg(IIC_BASE_ADDRESS,
						XIIC_CR_REG_OFFSET);
			XIic_WriteReg(IIC_BASE_ADDRESS, XIIC_CR_REG_OFFSET,
					CntlReg | XIIC_CR_TX_FIFO_RESET_MASK);
			XIic_WriteReg(IIC_BASE_ADDRESS, XIIC_CR_REG_OFFSET,
					XIIC_CR_ENABLE_DEVICE_MASK);
		}

	} while (SentByteCount != sizeof(Address));

	/*
	 * Write a page of data at the specified address to the EEPROM.
	 */
	SentByteCount = XIic_Send(IIC_BASE_ADDRESS, EepromIicAddr,
				  WriteBuffer, sizeof(Address) + PAGE_SIZE,
				  XIIC_STOP);

	/*
	 * Wait for the write to be complete by trying to do a write and
	 * the device will not ack if the write is still active.
	 */
	do {
		AckByteCount = XIic_Send(IIC_BASE_ADDRESS, EepromIicAddr,
					(u8 *)&Address, sizeof(Address),
					XIIC_STOP);
		if (AckByteCount != sizeof(Address)) {

			/* Send is aborted so reset Tx FIFO */
			CntlReg = XIic_ReadReg(IIC_BASE_ADDRESS,
					XIIC_CR_REG_OFFSET);
			XIic_WriteReg(IIC_BASE_ADDRESS, XIIC_CR_REG_OFFSET,
					CntlReg | XIIC_CR_TX_FIFO_RESET_MASK);
			XIic_WriteReg(IIC_BASE_ADDRESS, XIIC_CR_REG_OFFSET,
					XIIC_CR_ENABLE_DEVICE_MASK);
		}

	} while (AckByteCount != sizeof(Address));


	/*
	 * Return the number of bytes written to the EEPROM
	 */
	return SentByteCount - sizeof(Address);
}

/*****************************************************************************/
/**
* This function reads a number of bytes from the IIC serial EEPROM into a
* specified buffer.
*
* @param	Address contains the address in the EEPROM to read from.
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*		This value is not constrained by the page size of the device
*		such that up to 64K may be read in one call.
*
* @return	The number of bytes read. A value less than the specified input
*		value indicates an error.
*
* @note		None.
*
****************************************************************************/
unsigned EepromReadByte(AddressType Address, u8 *BufferPtr, u16 ByteCount)
{
	volatile unsigned ReceivedByteCount;
	u16 StatusReg;
	u32 CntlReg;

	/*
	 * Set the address register to the specified address by writing
	 * the address to the device, this must be tried until it succeeds
	 * because a previous write to the device could be pending and it
	 * will not ack until that write is complete.
	 */
	do {
		StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS, XIIC_SR_REG_OFFSET);
		if(!(StatusReg & XIIC_SR_BUS_BUSY_MASK)) {
			ReceivedByteCount = XIic_Send(IIC_BASE_ADDRESS,
							EepromIicAddr,
							(u8 *)&Address,
							sizeof(Address),
							XIIC_STOP);

			if (ReceivedByteCount != sizeof(Address)) {

				/* Send is aborted so reset Tx FIFO */
				CntlReg = XIic_ReadReg(IIC_BASE_ADDRESS,
							XIIC_CR_REG_OFFSET);
				XIic_WriteReg(IIC_BASE_ADDRESS, XIIC_CR_REG_OFFSET,
						CntlReg | XIIC_CR_TX_FIFO_RESET_MASK);
				XIic_WriteReg(IIC_BASE_ADDRESS,
						XIIC_CR_REG_OFFSET,
						XIIC_CR_ENABLE_DEVICE_MASK);
			}
		}

	} while (ReceivedByteCount != sizeof(Address));

	/*
	 * Read the number of bytes at the specified address from the EEPROM.
	 */
	ReceivedByteCount = XIic_Recv(IIC_BASE_ADDRESS, EepromIicAddr,
					BufferPtr, ByteCount, XIIC_STOP);

	/*
	 * Return the number of bytes read from the EEPROM.
	 */
	return ReceivedByteCount;
}
