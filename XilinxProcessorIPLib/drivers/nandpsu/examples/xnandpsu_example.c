/******************************************************************************
* Copyright (C) 2014 - 2022  Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/*****************************************************************************/
/**
* @file xnandpsu_example.c
*
* This file contains a design example using the NAND driver (XNandPsu).
* This example tests the erase, read and write features of the controller.
* The flash is erased and written. The data is read back and compared
* with the data written for correctness.
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
* 1.0  nm   05/06/2014  First release.
*      ms   04/10/17    Modified Comment lines to follow doxygen rules.
* 1.4  nsk  10/04/2018  Added support for ICCARM Compiler.
* 1.6  sd   31/03/2020  Fixed a gcc warning.
* 1.9  akm  07/15/2021  Switch to best supported Data interface and Timing mode.
* 1.12 akm  06/27/2023  Add support for system device-tree flow for example.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <xil_types.h>
#include <xil_printf.h>
#include <xparameters.h>
#include "xnandpsu.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define NAND_DEVICE_ID		0U
#endif
#define TEST_BUF_SIZE		0x8000U
#define TEST_PAGE_START		0x2U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
s32 NandReadWriteExample(u16 NandDeviceId);
#else
s32 NandReadWriteExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

XNandPsu NandInstance;			/* XNand Instance */
XNandPsu *NandInstPtr = &NandInstance;

/*
 * Buffers used during read and write transactions.
 */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
#ifdef __ICCARM__
u8 ReadBuffer[TEST_BUF_SIZE];
u8 WriteBuffer[TEST_BUF_SIZE];
#pragma pack(pop)
#else
u8 ReadBuffer[TEST_BUF_SIZE] __attribute__ ((aligned(64)));	/**< Block sized Read buffer */
u8 WriteBuffer[TEST_BUF_SIZE] __attribute__ ((aligned(64)));	/**< Block sized write buffer */
#endif

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
	int Status = XST_FAILURE;

	xil_printf("Nand Flash Read Write Example Test\r\n");
	/*
	 * Run the NAND read write example, specify the Base Address that
	 * is generated in xparameters.h .
	 */
#ifndef SDT
	Status = NandReadWriteExample(NAND_DEVICE_ID);
#else
	Status = NandReadWriteExample(XPAR_XNANDPSU_0_BASEADDR);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("Nand Flash Read Write Example Test Failed\r\n");
		goto Out;
	}

	Status = XST_SUCCESS;
	xil_printf("Successfully ran Nand Flash Read Write Example Test\r\n");
Out:
	return Status;
}

/****************************************************************************/
/**
*
* This function runs a test on the NAND flash device using the basic driver
* functions in polled mode.
* The function does the following tasks:
*	- Initialize the driver.
*	- Erase the flash.
*	- Write data to the flash.
*	- Read back the data from the flash.
*	- Compare the data read against the data Written.
*
* @param	NandDeviceId is is the XPAR_<NAND_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note
*		None
*
****************************************************************************/
#ifndef SDT
s32 NandReadWriteExample(u16 NandDeviceId)
#else
s32 NandReadWriteExample(UINTPTR BaseAddress)
#endif
{
	s32 Status = XST_FAILURE;
	XNandPsu_Config *Config;
	u32 Index;
	u64 Offset;
	u32 Length;

#ifndef SDT
	Config = XNandPsu_LookupConfig(NandDeviceId);
#else
	Config = XNandPsu_LookupConfig(BaseAddress);
#endif

	if (Config == NULL) {
		Status = XST_FAILURE;
		goto Out;
	}
	/*
	 * Initialize the flash driver.
	 */
	Status = XNandPsu_CfgInitialize(NandInstPtr, Config,
			Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	Status = XNandPsu_ChangeTimingMode(NandInstPtr,
					   NandInstPtr->DataInterface,
					   NandInstPtr->TimingMode);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	XNandPsu_EnableDmaMode(NandInstPtr);

	Offset = (u64)(TEST_PAGE_START * NandInstPtr->Geometry.BytesPerPage);
	Length = TEST_BUF_SIZE;

	/*
	 * Initialize the write buffer
	 */
	for (Index = 0; Index < Length;Index++) {
		WriteBuffer[Index] = (u8) (rand() % 256);
	}
	/*
	 * Erase the flash
	 */
	Status = XNandPsu_Erase(NandInstPtr, (u64)Offset, (u64)Length);
	if (Status != XST_SUCCESS) {
		goto Out;
	}
	/*
	 * Write to flash
	 */
	Status = XNandPsu_Write(NandInstPtr, (u64)Offset, (u64)Length,
						&WriteBuffer[0]);
	if (Status != XST_SUCCESS) {
		goto Out;
	}
	/*
	 * Read the flash after writing
	 */
	Status = XNandPsu_Read(NandInstPtr, (u64)Offset, (u64)Length,
						&ReadBuffer[0]);
	if (Status != XST_SUCCESS) {
		goto Out;
	}
	/*
	 * Compare the results
	 */
	for (Index = 0U; Index < Length;Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index]) {
			xil_printf("Index 0x%x: Read 0x%x != Write 0x%x\n",
						Index,
						ReadBuffer[Index],
						WriteBuffer[Index]);
			Status = XST_FAILURE;
			goto Out;
		}
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}
