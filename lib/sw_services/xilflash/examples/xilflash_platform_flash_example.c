/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xilflash_platform_flash_example.c
*
*
* This file contains a design example using the Flash Library with the Xilinx
* Platform Flash XL device. This example tests Unlock, Erase, Read and Write
* features. The Page is unlocked, erased and data is written to the page. The
* data is read back and compared with the data written for correctness.
* This example also shows usage of XFlash_DeviceControl to set the Platform
* Flash XL device in the Async/Sync Mode.
*
* @note		None
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a ksu  06/16/09 First release
* 2.00a ktn  12/04/09 Updated to use the HAL processor APIs/macros
* 3.00a sdm  03/03/11 Updated to pass BaseAddress and Flash Width to _Initialize
*		      API, as required by the new version of the library
* 4.7	akm  07/23/19 Initialized Status variable to XST_FAILURE.
* 4.10	akm  07/14/23 Added support for system device-tree flow.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include <xilflash.h>
#include <xil_types.h>

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
#define START_ADDRESS		0x100000

/*
 * The Sync and Async mode address
 */
#define ASYNC_ADDR  0x17BBE /* Config Register value for setting Async mode */
#define SYNC_ADDR   0x07BBE /* Config Register value for setting Sync mode */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int PlatformFlashReadWriteExample(void);

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
* Main function to execute the Platform Flash Read/Write example.
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

	xil_printf(" Flash platform Read/Write Test \r\n");
	Status = PlatformFlashReadWriteExample();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Flash platform Read/Write Test \r\n");
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
int PlatformFlashReadWriteExample(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	DeviceCtrlParam IoctlParams;

	/*
	 * Initialize the Flash Library. The Platform Flash XL device is
	 * set into Async Mode during the initialization.
	 */
	Status = XFlash_Initialize(&FlashInstance, FLASH_BASE_ADDRESS,
				   FLASH_MEM_WIDTH, 1);
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
	 * Set ASYNC flash read mode. This call is for showing the
	 * usage of the XFlash_DeviceControl to set the Platform Flash XL
	 * device in Async Mode.
	 */
	IoctlParams.ConfigRegParam.Value = ASYNC_ADDR;
	Status = XFlash_DeviceControl(&FlashInstance,
				XFL_DEVCTL_SET_CONFIG_REG, &IoctlParams);
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
	Status = XFlash_Unlock(&FlashInstance, START_ADDRESS, 0);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
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
		WriteBuffer[Index] = Index + 0x5;
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

	/*
	 * Set the Platform Flash XL device to Sync mode. This call is
	 * for showing the usage of the XFlash_DeviceControl to set the
	 * Platform Flash XL device in Sync Mode.
	 */
	IoctlParams.ConfigRegParam.Value = SYNC_ADDR;
	Status = XFlash_DeviceControl(&FlashInstance,
				XFL_DEVCTL_SET_CONFIG_REG, &IoctlParams);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
