/******************************************************************************
*
* Copyright (C) 2009 - 2015 Xilinx, Inc.  All rights reserved.
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
* @file xnandps_skip_example.c
*
* This file contains a design example using the NAND driver (XNandPs).
* This example tests the skip block method of erase/read/write operations.
* The skip block method is useful while reading/writing images on to the flash.
* The flash is erased and programming by considering the bad blocks. The data is
* read back and compared with the data written for correctness.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date         Changes
* ----- ---- ----------   -----------------------------------------------
* 1.00  nm   12/10/2010   First release
* 1.01a nm   28/02/2012   Modified the test offsets.
*       ms   04/10/17     Modified Comment lines to follow doxygen rules
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <xil_types.h>
#include <xil_printf.h>
#include <xparameters.h>
#include <xnandps.h>
#include <xnandps_bbm.h>
/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define NAND_DEVICE_ID		XPAR_XNANDPS_0_DEVICE_ID
/* Test parameters */
#define NAND_TEST_OFFSET	0x01000000	/**< Flash Test Offset */
#define NAND_TEST_LENGTH	0x00080000	/**< Test Length */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int NandSkipBlockExample(u32 NandDeviceId);

int XNandPs_SkipRead(XNandPs *InstancePtr, u64 Offset, u32 Length, void
		*DestPtr);

int XNandPs_SkipWrite(XNandPs *InstancePtr, u64 Offset, u32 Length, void
		*SrcPtr);

int XNandPs_SkipErase(XNandPs *InstancePtr, u64 Offset, u32 Length);

/************************** Variable Definitions *****************************/
XNandPs NandInstance; /* XNand Instance. */
XNandPs *NandInstPtr = &NandInstance;
/*
 * Buffers used during read and write transactions.
 */
u8 ReadBuffer[NAND_TEST_LENGTH];
u8 WriteBuffer[NAND_TEST_LENGTH];
/************************** Function Definitions ******************************/

/****************************************************************************/
/**
*
* Main function to execute the Skip Block based Nand read/write example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{
	int Status;

	xil_printf("Nand Flash Skip Block Method Example Test\r\n");

	/*
	 * Run the NAND read write example, specify the Base Address that
	 * is generated in xparameters.h .
	 */
	Status = NandSkipBlockExample(NAND_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Nand Flash Skip Block Method Example Test Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran Nand Flash Skip Block Method Example Test\r\n");

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the NAND flash device using the basic driver
* functions.
* The function does the following tasks:
*	- Initialize the driver.
*	- Erase the required length of bytes by taking bad blocks into account.
*	- Write the number of bytes from given offset by taking bad blocks
*		into account.
*	- Read the number of bytes from given offset by taking bad blocks
*		into account.
*	- Compare the data read against the data Written.
*
* @param	NandDeviceId is the XPAR_<NAND_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
****************************************************************************/
int NandSkipBlockExample(u32 NandDeviceId)
{
	int Status;
	u32 Index;
	XNandPs_Config *ConfigPtr;
	u64 Offset;
	u32 Length;

	/*
	 * Initialize the flash driver.
	 */
	ConfigPtr = XNandPs_LookupConfig(NandDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XNandPs_CfgInitialize(NandInstPtr, ConfigPtr,
			ConfigPtr->SmcBase,ConfigPtr->FlashBase);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Offset = NAND_TEST_OFFSET;
	Length = NAND_TEST_LENGTH;

	/*
	 * Prepare the write buffer. Fill in the data need to be written into
	 * Flash Device.
	 */
	for (Index = 0; Index < Length; Index++) {
		WriteBuffer[Index] = Index % 256;
	}

	/*
	 * Erase the blocks using skip block method
	 */
	Status = XNandPs_SkipErase(NandInstPtr, Offset, Length);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write into the flash offset.
	 */
	Status = XNandPs_SkipWrite(NandInstPtr, Offset, Length, WriteBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read back from the flash.
	 */
	Status = XNandPs_SkipRead(NandInstPtr, Offset, Length, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data Written.
	 */
	for (Index = 0; Index < Length; Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index]) {
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns the length including bad blocks from a given offset and
* length.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Offset is the flash data address to read from.
* @param	Length is number of bytes to read.
*
* @return
*		- Return actual length including bad blocks.
*
* @note		None.
*
******************************************************************************/
u32 XNandPs_CalculateLength(XNandPs *InstancePtr, u64 Offset, u32 Length)
{
	u32 BlockSize = InstancePtr->Geometry.BlockSize;
	u32 CurBlockLen;
	u32 CurBlock;
	u32 Status;
	u32 TempLen = 0;
	u32 ActLen = 0;

	while (TempLen < Length) {
		CurBlockLen = BlockSize - (Offset & (BlockSize - 1));
		CurBlock = (Offset & ~(BlockSize - 1))/BlockSize;

		/*
		 * Check if the block is bad
		 */
		Status = XNandPs_IsBlockBad(InstancePtr, CurBlock);
		if (Status != XST_SUCCESS) {
			/* Good Block */
			TempLen += CurBlockLen;
		}
		ActLen += CurBlockLen;
		Offset += CurBlockLen;
		if (Offset >= InstancePtr->Geometry.DeviceSize) {
			break;
		}
	}

	return ActLen;
}
/*****************************************************************************/
/**
*
* This function reads the data from the Flash device and copies it into the
* specified user buffer. This function considers bad blocks and skips them
* to read next blocks.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Offset is the flash data address to read from.
* @param	Length is number of bytes to read.
* @param	DestPtr is the destination address to copy data to.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None.
*
******************************************************************************/
int XNandPs_SkipRead(XNandPs *InstancePtr, u64 Offset, u32 Length, void
		*DestPtr)
{
	u32 ActLen;
	u32 BlockOffset;
	u32 Block;
	u32 Status;
	u32 BytesLeft = Length;
	u32 BlockSize = InstancePtr->Geometry.BlockSize;
	u8 *BufPtr = (u8 *)DestPtr;
	u32 ReadLen;
	u32 BlockReadLen;

	/*
	 * Calculate the actual length including bad blocks
	 */
	ActLen = XNandPs_CalculateLength(InstancePtr, Offset, Length);

	/*
	 *  Check if the actual length cross flash size
	 */
	if (Offset + ActLen > InstancePtr->Geometry.DeviceSize) {
		return XST_FAILURE;
	}

	while (BytesLeft > 0) {
		BlockOffset = Offset & (BlockSize - 1);
		Block = (Offset & ~(BlockSize - 1))/BlockSize;
		BlockReadLen = BlockSize - BlockOffset;

		Status = XNandPs_IsBlockBad(InstancePtr, Block);
		if (Status == XST_SUCCESS) {
			/* Move to next block */
			Offset += BlockReadLen;
			continue;
		}

		/*
		 * Check if we cross block boundary
		 */
		if (BytesLeft < BlockReadLen) {
			ReadLen = BytesLeft;
		} else {
			ReadLen = BlockReadLen;
		}

		/*
		 * Read from the NAND flash
		 */
		Status = XNandPs_Read(InstancePtr, Offset, ReadLen, BufPtr, NULL);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		BytesLeft -= ReadLen;
		Offset += ReadLen;
		BufPtr += ReadLen;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function programs the flash device(s) with data specified in the user
* buffer. This function considers bad blocks and skips them.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Offset is the flash data address to write to.
* @param	Length is number of bytes to write.
* @param	SrcPtr is the source address to write the data from.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
* @note		None.
*
******************************************************************************/
int XNandPs_SkipWrite(XNandPs *InstancePtr, u64 Offset, u32 Length, void
		*SrcPtr)
{
	u32 ActLen;
	u32 BlockOffset;
	u32 Block;
	u32 Status;
	u32 BytesLeft = Length;
	u32 BlockSize = InstancePtr->Geometry.BlockSize;
	u8 *BufPtr = (u8 *)SrcPtr;
	u32 WriteLen;
	u32 BlockWriteLen;

	/*
	 * Calculate the actual length including bad blocks
	 */
	ActLen = XNandPs_CalculateLength(InstancePtr, Offset, Length);

	/*
	 * Check if the actual length cross flash size
	 */
	if (Offset + ActLen > InstancePtr->Geometry.DeviceSize) {
		return XST_FAILURE;
	}
	while (BytesLeft > 0) {
		BlockOffset = Offset & (BlockSize - 1);
		Block = (Offset & ~(BlockSize - 1))/BlockSize;
		BlockWriteLen = BlockSize - BlockOffset;

		/*
		 * Check if the block is bad
		 */
		Status = XNandPs_IsBlockBad(InstancePtr, Block);
		if (Status == XST_SUCCESS) {
			/* Move to next block */
			Offset += BlockWriteLen;
			continue;
		}

		/*
		 *  Check if we cross block boundary
		 */
		if (BytesLeft < BlockWriteLen) {
			WriteLen = BytesLeft;
		} else {
			WriteLen = BlockWriteLen;
		}

		/*
		 * Read from the NAND flash
		 */
		Status = XNandPs_Write(InstancePtr, Offset, WriteLen, BufPtr, NULL);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		BytesLeft -= WriteLen;
		Offset += WriteLen;
		BufPtr += WriteLen;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases length bytes in the flash device from a given offset.
* The Offset and Length must be block aligned. This functions skips bad blocks.
*
* @param	InstancePtr is the pointer to the XNand instance.
* @param	Offset is the flash address to start erasing from.
* @param	Length is number of bytes to erase.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if error in erase.
*
* @note		None.
*
******************************************************************************/
int XNandPs_SkipErase(XNandPs *InstancePtr, u64 Offset, u32 Length)
{
	u32 StartBlock;
	u32 NumOfBlocks;
	u32 BlockSize;
	u32 Block;
	int Status;

	BlockSize = InstancePtr->Geometry.BlockSize;
	/*
	 * Start address must align on block boundary
	 */
	if (Offset & (BlockSize - 1)) {
		/* Unalinged offset */
		return XST_FAILURE;
	}

	/*
	 *  Length must align on block boundary
	 */
	if (Length & (BlockSize - 1)) {
		/* Length is not block aligned */
		return XST_FAILURE;
	}

	StartBlock = (Offset & ~(BlockSize - 1))/BlockSize;
	NumOfBlocks = (Length & ~(BlockSize - 1))/BlockSize;

	for (Block = StartBlock; Block < (StartBlock + NumOfBlocks)
				; Block++) {
		/*
		 * Check if the block is bad
		 */
		Status = XNandPs_IsBlockBad(InstancePtr, Block);
		if (Status == XST_SUCCESS) {
			NumOfBlocks++;
			if ((StartBlock + NumOfBlocks) >=
					InstancePtr->Geometry.NumBlocks) {
				return XST_FAILURE;
			}
			/* Increase the block count for skip block method */
			continue;
		}

		/*
		 * Erase the Nand flash block
		 */
		Status = XNandPs_EraseBlock(InstancePtr, Block);
		if (Status == XST_NAND_WRITE_PROTECTED) {
			/* Flash is write protected */
			return XST_FAILURE;
		}
		if (Status != XST_SUCCESS) {
			/* Erase operation error */
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
