/******************************************************************************
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xnandps_example.c
*
* This file contains a design example using the NAND driver (XNandPs).
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
* 1.00  nm   12/10/2010  First release.
* 1.01a nm   28/02/2012  Modified the test offsets.
*       ms   04/10/17    Modified Comment lines to follow doxygen rules
* 2.8  akm   07/06/23    Add support for system device-tree flow.
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
#ifndef SDT
#define NAND_DEVICE_ID		XPAR_XNANDPS_0_DEVICE_ID
#endif
/* Test parameters */
#define NAND_TEST_START_BLOCK	64	/**< Starting block to test */
#define NAND_TEST_NUM_BLOCKS	16	/**< Number of blocks to test */
#define NAND_TEST_BLOCK_SIZE	0x20000	/**< Test Block Size, must be same as
					  the flash block size */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
int NandReadWriteExample(u32 NandDeviceId);
#else
int NandReadWriteExample(UINTPTR BaseAddress);
#endif

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
#ifndef SDT
	Status = NandReadWriteExample(NAND_DEVICE_ID);
#else
	Status = NandReadWriteExample(XPAR_XNANDPS_0_BASEADDR);
#endif

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
#ifndef SDT
int NandReadWriteExample(u32 NandDeviceId)
#else
int NandReadWriteExample(UINTPTR BaseAddress)
#endif
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
#ifndef SDT
	ConfigPtr = XNandPs_LookupConfig(NandDeviceId);
#else
	ConfigPtr = XNandPs_LookupConfig(BaseAddress);
#endif
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
		Status = XNandPs_Write(NandInstPtr, Offset, Length, WriteBuffer, NULL);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/*
		 * Perform the read operation.
		 */
		Status = XNandPs_Read(NandInstPtr, Offset, Length, ReadBuffer, NULL);
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
