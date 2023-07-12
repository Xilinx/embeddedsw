/******************************************************************************
* Copyright (C) 2006 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiic_low_level_dynamic_eeprom_example.c
*
* This file consists of a polled mode design example which uses the Xilinx
* IIC device in dynamic mode and low-level driver to exercise the EEPROM.
*
* The XIic_DynSend() API is used to transmit the data and XIic_DynRecv() API
* is used to receive the data.
*
* The example is tested on ML300/ML310/ML403/ML501/ML507/ML510 Xilinx boards.
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
* The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML410/ML403/
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
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a mta  03/20/06 Created.
* 2.00a sdm  09/22/09 Converted all register accesses to 32 bit access.
* 2.01a ktn  03/17/10 Updated the information about the EEPROM's used on
*		      ML605/SP601/SP605 boards. Updated the example so that it
*		      can be used to access the entire IIC EEPROM for devices
*		      like M24C04/M24C08 that use LSB bits of the IIC device
*		      select code (IIC slave address) to specify the higher
*		      address bits of the EEPROM internal address.
* 3.3   sk    06/18/16 checked bytes written with the input byte count and
*                      returns error if the value is not matched.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Modified Comment lines in functions to
*                     recognize it as documentation block for doxygen
*                     generation.
* 3.10  gm   07/09/23 Added SDT support.
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

#define IIC_BASE_ADDRESS	XPAR_AXI_IIC_0_BASEADDR

/*
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_TEST_START_ADDRESS	0x80

/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 * The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
 * ML501/ML505/ML507/ML510 boards is 0x50. The 7 bit IIC Slave address of the IIC
 * EEPROM on the ML605/SP601/SP605 boards is 0x54.
 * Please refer the User Guide's of the respective boards for further
 * information about the IIC slave address of IIC EEPROM's.
 */
#define EEPROM_ADDRESS		0x50

/*
 * The page size determines how much data should be written at a time.
 * The ML310/ML300 board supports a page size of 32 and 16.
 * The write function should be called with this as a maximum byte count.
 */
#define PAGE_SIZE	16

#define IIC_SLAVE_ADDRESS	1

/**************************** Type Definitions *******************************/

/*
 * The AddressType for ML300/ML310/ML410/ML510 boards should be u16 as the address
 * pointer in the on board EEPROM is 2 bytes.
 * The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605 boards should
 * be u8 as the address pointer in the on board EEPROM is 1 bytes.
 */
typedef u8 AddressType;

u8 EepromIicAddr;		/* Variable for storing Eeprom IIC address */

/************************** Function Prototypes ******************************/

int IicLowLevelDynEeprom();

u8 EepromReadByte(u8 *BufferPtr, u8 ByteCount);
u8 EepromWriteByte(u8 *BufferPtr, u8 ByteCount);

/************************** Variable Definitions *****************************/

u8 WriteBuffer[PAGE_SIZE];	/* Write buffer for writing a page. */

u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page.  */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Main function to call the low level Dynamic EEPROM example.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Perform the Read and Write operation.
	 */
	Status = IicLowLevelDynEeprom();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC lowlevel dynamic eeprom Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran IIC lowlevel dynamic eeprom Example\r\n");
	return XST_SUCCESS;
}

/******************************************************************************
/**
*
* The function uses the low level driver of IIC to read from the IIC EEPROM on
* the ML300/ML310 board. The addresses tested starts from 128.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int IicLowLevelDynEeprom()
{
	u8 BytesRead;
	u8 BytesWritten;
	u32 StatusReg;
	u8 Index;
	int Status;
	EepromIicAddr = EEPROM_ADDRESS;

	/*
	 * Initialize the IIC Core.
	 */
	Status = XIic_DynInit(IIC_BASE_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Make sure all the Fifo's are cleared and Bus is Not busy.
	 */
	while (((StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS,
					  XIIC_SR_REG_OFFSET)) &
		(XIIC_SR_RX_FIFO_EMPTY_MASK |
		 XIIC_SR_TX_FIFO_EMPTY_MASK |
		 XIIC_SR_BUS_BUSY_MASK)) !=
	       (XIIC_SR_RX_FIFO_EMPTY_MASK |
		XIIC_SR_TX_FIFO_EMPTY_MASK)) {

	}

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
	BytesWritten = EepromWriteByte(WriteBuffer, PAGE_SIZE);
	if (BytesWritten != PAGE_SIZE) {
		return XST_FAILURE;
	}

	/*
	 * Read from the EEPROM.
	 */
	BytesRead = EepromReadByte(ReadBuffer, PAGE_SIZE);
	if (BytesRead != PAGE_SIZE) {
		return XST_FAILURE;
	}

	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < PAGE_SIZE; Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index]) {
			return XST_FAILURE;
		}

		ReadBuffer[Index] = 0;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of bytes to the IIC serial EEPROM.
*
* @param	BufferPtr contains the address of the data to write.
* @param	ByteCount contains the number of bytes in the buffer to be
*		written. Note that this should not exceed the page size of the
*		EEPROM as noted by the constant PAGE_SIZE.
*
* @return	The number of bytes written, a value less than that which was
*		specified as an input indicates an error.
*
* @note		one.
*
******************************************************************************/
u8 EepromWriteByte(u8 *BufferPtr, u8 ByteCount)
{
	u8 SentByteCount;
	AddressType Address = EEPROM_TEST_START_ADDRESS;
	u8 WriteBuffer[sizeof(Address) + PAGE_SIZE];
	u8 Index;

	/*
	 * A temporary write buffer must be used which contains both the address
	 * and the data to be written, put the address in first based upon the
	 * size of the address for the EEPROM
	 */
	if (sizeof(AddressType) == 2) {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
	} else if (sizeof(AddressType) == 1) {
		WriteBuffer[0] = (u8) (Address);
		EepromIicAddr |= (EEPROM_TEST_START_ADDRESS >> 8) & 0x7;
	}

	/*
	 * Put the data in the write buffer following the address.
	 */
	for (Index = 0; Index < ByteCount; Index++) {
		WriteBuffer[sizeof(Address) + Index] = BufferPtr[Index];
	}

	/*
	 * Write a page of data at the specified address to the EEPROM.
	 */
	SentByteCount = XIic_DynSend(IIC_BASE_ADDRESS, EepromIicAddr,
				     WriteBuffer, sizeof(Address) + PAGE_SIZE,
				     XIIC_STOP);

	/*
	 * Return the number of bytes written to the EEPROM.
	 */
	return SentByteCount - sizeof(Address);
}

/******************************************************************************
/**
*
* This function reads a number of bytes from the IIC serial EEPROM into a
* specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*		This value is constrained by the page size of the device such
*		that up to 64K may be read in one call.
*
* @return	The number of bytes read. A value less than the specified input
*		value indicates an error.
*
* @note		None.
*
******************************************************************************/
u8 EepromReadByte(u8 *BufferPtr, u8 ByteCount)
{
	u8 ReceivedByteCount;
	u8 SentByteCount;
	u16 StatusReg;
	AddressType Address = EEPROM_TEST_START_ADDRESS;

	/*
	 * Position the Read pointer to specific location in the EEPROM.
	 */
	do {
		StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS, XIIC_SR_REG_OFFSET);
		if (!(StatusReg & XIIC_SR_BUS_BUSY_MASK)) {
			SentByteCount = XIic_DynSend(IIC_BASE_ADDRESS,
						     EepromIicAddr,
						     (u8 *) &Address,
						     sizeof(Address),
						     XIIC_STOP);
		}

	} while (SentByteCount != sizeof(Address));

	StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS, XIIC_SR_REG_OFFSET);

	while ((StatusReg & XIIC_SR_BUS_BUSY_MASK)) {
		StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS, XIIC_SR_REG_OFFSET);
	}

	do {
		/*
		 * Receive the data.
		 */
		ReceivedByteCount = XIic_DynRecv(IIC_BASE_ADDRESS,
						 EepromIicAddr, BufferPtr,
						 ByteCount);
	} while (ReceivedByteCount != ByteCount);

	/*
	 * Return the number of bytes received from the EEPROM.
	 */
	return ReceivedByteCount;
}
