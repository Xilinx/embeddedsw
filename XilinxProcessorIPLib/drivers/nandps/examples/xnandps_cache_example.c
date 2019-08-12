/******************************************************************************
*
* Copyright (C) 2013 - 2015 Xilinx, Inc.  All rights reserved.
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
* @file xnandps_cache_example.c
*
* This file contains a design example using the NAND driver (XNandPs).
* This example tests NAND page cache read and write commands. The page cache
* commands are not supported by OnDie ECC flash since ECC is enabled by
* default. Tested Spansion S34ML04G100TFI00 flash with this example.
*
* This example tests the block erase, block read and block write features.
* The flash blocks are erased and written. The data is read back and compared
* with the data written for correctness. The bad blocks are not erased/programmed.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ----------  -----------------------------------------------
* 1.00  nm   04/25/2013  First release.
*       ms   04/10/17    Modified Comment lines to follow doxygen rules
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
#define NAND_TEST_START_BLOCK	64	/**< Starting block to test */
#define NAND_TEST_NUM_BLOCKS	16	/**< Number of blocks to test */
#define NAND_TEST_BLOCK_SIZE	0x20000	/**< Test Block Size, must be same as
					  the flash block size */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int NandReadWriteCacheExample(u32 NandDeviceId);

/************************** Variable Definitions *****************************/
XNandPs NandInstance; /* XNand Instance. */
XNandPs *NandInstPtr = &NandInstance;
/*
 * Buffers used during read and write transactions.
 */
u8 ReadBuffer[NAND_TEST_BLOCK_SIZE];	/**< Block sized Read buffer */
u8 WriteBuffer[NAND_TEST_BLOCK_SIZE];	/**< Block sized write buffer */
/************************** Function Definitions ******************************/

/****************************************************************************/
/**
*
* Main function to execute the Nand Flash read write example.
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

	xil_printf("Nand Flash Read Write Example Test\r\n");
	/*
	 * Run the NAND read write example, specify the Base Address that
	 * is generated in xparameters.h .
	 */
	Status = NandReadWriteCacheExample(NAND_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Nand Flash Read Write Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Nand Flash Read Write Example Test\r\n");

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the NAND flash device using the basic driver
* functions.
* The function does the following tasks:
*	- Initialize the driver.
*	- Erase the blocks.
*	- Write in to all the blocks.
*	- Read back the data from the blocks.
*	- Compare the data read against the data Written.
*
* @param	NandDeviceId is is the XPAR_<NAND_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		When bad blocks are encountered, they are not erased and
*		programmed.
*
****************************************************************************/
int NandReadWriteCacheExample(u32 NandDeviceId)
{
	int Status;
	u32 Index;
	u32 BlockIndex;
	XNandPs_Config *ConfigPtr;
	u64 Offset;
	u32 Length;
	u32 StartBlock;
	u32 EndBlock;

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

	StartBlock = NAND_TEST_START_BLOCK;
	EndBlock = NAND_TEST_START_BLOCK + NAND_TEST_NUM_BLOCKS;
	Length = NAND_TEST_BLOCK_SIZE;
	/*
	 * Prepare the write buffer. Fill in the data need to be written into
	 * Flash Device.
	 */
	for (Index = 0; Index < Length; Index++) {
		WriteBuffer[Index] = Index % 256;
	}

	/*
	 * Erase the blocks in the flash
	 */
	for (BlockIndex = StartBlock; BlockIndex < EndBlock; BlockIndex++) {
		/*
		 * Don't erase bad blocks.
		 */
		if (XNandPs_IsBlockBad(NandInstPtr, BlockIndex) == XST_SUCCESS)
		{
			continue;
		}
		/*
		 * Perform erase operation.
		 */
		Status = XNandPs_EraseBlock(NandInstPtr, BlockIndex);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	/*
	 * Perform the read/write operation
	 */
	for (BlockIndex = StartBlock; BlockIndex < EndBlock; BlockIndex++) {
		/*
		 * Don't program bad blocks.
		 */
		if (XNandPs_IsBlockBad(NandInstPtr, BlockIndex) == XST_SUCCESS)
		{
			continue;
		}
		Offset = BlockIndex * NandInstPtr->Geometry.BlockSize;

		/*
		 * Perform the write operation.
		 */
		Status = XNandPs_WriteCache(NandInstPtr, Offset, Length, WriteBuffer, NULL);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/*
		 * Perform the read operation.
		 */
		Status = XNandPs_ReadCache(NandInstPtr, Offset, Length, ReadBuffer, NULL);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/*
		 * Compare the data read against the data Written.
		 */
		for (Index = 0; Index < Length; Index++) {
			if (ReadBuffer[Index] != WriteBuffer[Index]) {
				return XST_FAILURE;
			}
		}

		/*
		 * Clear the Receive buffer for next iteration
		 */
		memset(ReadBuffer, 0, Length);
	}

	return XST_SUCCESS;
}
