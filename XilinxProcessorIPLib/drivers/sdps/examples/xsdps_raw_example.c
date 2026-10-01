/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdps_raw_example.c
* @brief Tests read and write transfers on the SD/eMMC interface.
*
* Running this example modifies the card contents and erases the file system
* information in the card. The card needs to be re-formatted.
*
* Modify the offset and Size macros to test different SD memory offset and
* size.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date     Changes
* ----- --- -------- ---------------------------------------------
* 3.9 	mn  12/02/19 First release
* 3.10	mn  09/17/20 Fix sector offset issue with Non-HCS SD cards
* 4.2   ro     06/12/23 Added support for system device-tree flow.
* 4.7   vn     07/30/26 Fix Doxygen warnings
* 4.7   tb     10/01/26 Align Doxygen comments with AMD documentation standards
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsdps.h"		/* SD device driver */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int SdpsRawTest(void);

/************************** Variable Definitions *****************************/

#ifdef __ICCARM__
#pragma data_alignment = 32
u8 DestinationAddress[10 * 1024];	/**< Buffer for the read data. */
#pragma data_alignment = 32
u8 SourceAddress[10 * 1024];	/**< Buffer for the write data. */
#else
/** Buffer for the read data. */
u8 DestinationAddress[10 * 1024] __attribute__ ((aligned(32)));
/** Buffer for the write data. */
u8 SourceAddress[10 * 1024] __attribute__ ((aligned(32)));
#endif

#define TEST 7	/**< Base value of the test data pattern. */
#define NUM_BLOCKS 16	/**< Number of SD blocks to test. */
#define SECTOR_OFFSET 204800	/**< Sector offset to test. */

/*****************************************************************************/
/**
*
* @brief Calls the SD example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("SD Raw Read/ Write Test \r\n");

	Status = SdpsRawTest();
	if (Status != XST_SUCCESS) {
		xil_printf("SD Raw Read/ Write Test failed \r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SD Raw Read/ Write Test \r\n");

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* @brief Performs the SD Raw Read/ Write Test.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
static int SdpsRawTest(void)
{
	static XSdPs SdInstance;
	XSdPs_Config *SdConfig;
	int Status;
	u32 BuffCnt;
	/*
	 * Since block size is 512 bytes. File Size is 512*BlockCount.
	 */
	u32 FileSize = (512 * NUM_BLOCKS); /* File Size is only up to 2MB */
	u32 Sector = SECTOR_OFFSET;

	for (BuffCnt = 0; BuffCnt < FileSize; BuffCnt++) {
		SourceAddress[BuffCnt] = TEST + BuffCnt;
	}

	/*
	 * Initialize the host controller
	 */
#ifndef SDT
	SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
#else
	SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_BASEADDR);
#endif
	if (NULL == SdConfig) {
		return XST_FAILURE;
	}

	Status = XSdPs_CfgInitialize(&SdInstance, SdConfig,
				     SdConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSdPs_CardInitialize(&SdInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write data to SD/eMMC.
	 */
	if (!(SdInstance.HCS)) {
		Sector *= XSDPS_BLK_SIZE_512_MASK;
	}
	Status = XSdPs_WritePolled(&SdInstance, Sector, NUM_BLOCKS,
				   SourceAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read data from SD/eMMC.
	 */
	Status  = XSdPs_ReadPolled(&SdInstance, Sector, NUM_BLOCKS,
				   DestinationAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Data verification
	 */
	for (BuffCnt = 0; BuffCnt < FileSize; BuffCnt++) {
		if (SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
