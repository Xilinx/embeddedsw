/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xilflash_readwrite_example.c
*
*
* This file contains a design example using the Flash Library.
* This example tests the Erase, Read and Write features. The Page is erased
* and data is written to the page. The data is read back and compared with
* the data written for correctness.
*
* @note		The Erase operation on a CFI compliant Flash device will result
*		in an error if the block is locked.
*		This example performs an Unlock operation before Erase operation
*		only on the Intel CFI compliant Flash device. The AMD CFI
*		compliant Flash devices require the user to apply a 12V DC
*		voltage on the RP pin while performing the Lock and Unlock
*		operations. This example will not work with the AMD compliant
*		Flash devices if the block is locked and the user doesn't
*		perform an Unlock operation.
*		Change the value of XFL_TO_ASYNCMODE to 1, inorder to operate
*		the Micron Flash in async mode,if it was set to sync mode.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a mta  10/09/07 First release
* 1.01a ksu  04/10/08 Added support for AMD CFI Interface
* 1.01a sdm  02/24/09 a) Added code to perform an Unlock operation before the
*                        Erase operation for Intel CFI compliant Flash devices
*                     b) Updated the notes with information about using the
*                        example with AMD CFI compliant Flash devices
* 2.00a ktn  12/04/09 Updated to use the HAL processor APIs/macros
* 3.00a sdm  03/03/11 Updated to pass BaseAddress and Flash Width to _Initialize
*		      API, as required by the new version of the library
* 4.2   nsk  01/07/16 Added Support to change Flash to Async Mode, if it was
*                     set to sync mode.
*                     Modified FLASH_BASE_ADDRESS to canonical name.
* 4.2   nsk  01/27/16 Added check to unlock the Micron G18 Flash.
* 4.7	akm  07/23/19 Initialized Status variable to XST_FAILURE.
* 4.10	akm  07/14/23 Added support for system device-tree flow.
*
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include <xilflash.h>
#include <stdio.h>
#include <xil_types.h>
#include "xil_io.h"
/************************** Constant Definitions *****************************/

/*
 * The following constants define the baseaddress and width the flash memory.
 * These constants map to the XPAR parameters created in the xparameters.h file.
 * They are defined here such that a user can easily change all the needed
 * parameters in one place.
 */
#ifndef SDT
#define FLASH_BASE_ADDRESS	XPAR_EMC_0_S_AXI_MEM0_BASEADDR
#else
#define FLASH_BASE_ADDRESS	XPAR_XEMC_0_BASEADDR
#endif

/*
 * The following constant defines the total byte width of the flash memory. The
 * user needs to update this width based on the flash width in the design/board.
 * The total flash width on some of the Xilinx boards is listed below.
 * -------------------------------
 * Board		Width
 * -------------------------------
 * ML403		4 (32 bit)
 * ML5xx		2 (16 bit)
 * Spartan3S 1600E	2 (16 bit)
 * Spartan-3A DSP	2 (16 bit)
 * Spartan-3A		2 (16 bit)
 * Spartan-3AN		2 (16 bit)
 * ML605		2 (16 bit)
 * SP605		2 (16 bit)
 * SP601		1 (8 bit)
 */
#define FLASH_MEM_WIDTH		2

#define FLASH_TEST_SIZE		4048
#define START_ADDRESS		0x060000
#define BLOCK_OFFSET_ADDR	0x068000

/*
 * The following constant defines the commands and register offsets inorder
 * to change the flash to Async Mode.
 */
#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
	#define XFL_TO_ASYNCMODE	0
	#define ASYNC_ADDR		0x17BBE
	#define SYNC_ADDR		0x07BBE
	#define INTEL_CMD_CONFIG_REG_SETUP	0x60606060
	#define INTEL_CMD_CONFIG_REG_CONFIRM	0x03030303
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FlashReadWriteExample(void);
/************************** Variable Definitions *****************************/
XFlash FlashInstance; /* XFlash Instance. */

/*
 * Buffers used during read and write transactions.
 */
u8 ReadBuffer[FLASH_TEST_SIZE]; /* Buffer used to store the data read. */
u8 WriteBuffer[FLASH_TEST_SIZE]; /* Write buffer. */

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* Main function to execute the Flash read write example.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("Flash Read/Write Test \r\n");
	Status = FlashReadWriteExample();
	if(Status != XST_SUCCESS) {
		xil_printf("Flash Read/Write Test Fail\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran Flash Read/Write Test\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes, reads, and verifies the data to the Flash device.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashReadWriteExample(void)
{
	int Status = XST_FAILURE;
	u32 Index;

	#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
		#if XFL_TO_ASYNCMODE
		/*
		 * Set Flash to Async mode.
		 */
		if (FLASH_MEM_WIDTH == 1) {
			WRITE_FLASH_8(FLASH_BASE_ADDRESS + ASYNC_ADDR, 0x60);
			WRITE_FLASH_8(FLASH_BASE_ADDRESS + ASYNC_ADDR, 0x03);
		} else if (FLASH_MEM_WIDTH == 2) {
			WRITE_FLASH_16(FLASH_BASE_ADDRESS + ASYNC_ADDR,
					INTEL_CMD_CONFIG_REG_SETUP);
			WRITE_FLASH_16(FLASH_BASE_ADDRESS + ASYNC_ADDR,
					INTEL_CMD_CONFIG_REG_CONFIRM);
		}
		#endif
	#endif

	/*
	 * Initialize the Flash Library.
	 */
	Status = XFlash_Initialize(&FlashInstance, FLASH_BASE_ADDRESS,
				   FLASH_MEM_WIDTH, 0);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Reset the Flash Device. This clears the Status registers and puts
	 * the device in Read mode.
	 */
	Status = XFlash_Reset(&FlashInstance);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform an unlock operation before the erase operation for the Intel
	 * Flash. The erase operation will result in an error if the block is
	 * locked.
	 */
	if ((FlashInstance.CommandSet == XFL_CMDSET_INTEL_STANDARD) ||
	    (FlashInstance.CommandSet == XFL_CMDSET_INTEL_EXTENDED) ||
            (FlashInstance.CommandSet == XFL_CMDSET_INTEL_G18)) {
		Status = XFlash_Unlock(&FlashInstance, BLOCK_OFFSET_ADDR, 0);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/*
	 * Perform the Erase operation.
	 */
	Status = XFlash_Erase(&FlashInstance, START_ADDRESS, FLASH_TEST_SIZE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the write buffer. Fill in the data need to be written into
	 * Flash Device.
	 */
	for(Index = 0; Index < FLASH_TEST_SIZE; Index++) {
		WriteBuffer[Index] = Index + 1;
	}

	/*
	 * Perform the Write operation.
	 */
	Status = XFlash_Write(&FlashInstance, START_ADDRESS, FLASH_TEST_SIZE,
								WriteBuffer);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform the read operation.
	 */
	Status = XFlash_Read(&FlashInstance, START_ADDRESS, FLASH_TEST_SIZE,
								ReadBuffer);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data Written.
	 */
	for(Index = 0; Index < FLASH_TEST_SIZE; Index++) {
		if(ReadBuffer[Index] != WriteBuffer[Index]) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
