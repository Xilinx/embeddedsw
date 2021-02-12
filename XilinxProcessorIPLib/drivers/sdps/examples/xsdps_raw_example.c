/******************************************************************************
* Copyright (C) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdps_raw_example.c
*
* This example is used to test read and write transfers on SD/eMMC interface.
*
* Please note that running this example will modify the card contents and
* file system information will be erased in the card. Card will need to be
* re-formatted.
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
u8 DestinationAddress[10*1024];
#pragma data_alignment = 32
u8 SourceAddress[10*1024];
#else
u8 DestinationAddress[10*1024] __attribute__ ((aligned(32)));
u8 SourceAddress[10*1024] __attribute__ ((aligned(32)));
#endif

#define TEST 7
/* Number of SD blocks to test */
#define NUM_BLOCKS 16
/* Sector offset to test */
#define SECTOR_OFFSET 204800

/*****************************************************************************/
/**
*
* Main function to call the SD example.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
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
* This function performs the SD Raw Read/ Write Test.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
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
	u32 FileSize = (512*NUM_BLOCKS); /* File Size is only up to 2MB */
	u32 Sector = SECTOR_OFFSET;

	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		SourceAddress[BuffCnt] = TEST + BuffCnt;
	}

	/*
	 * Initialize the host controller
	 */
	SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
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
	if (!(SdInstance.HCS)) Sector *= XSDPS_BLK_SIZE_512_MASK;
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
	if (Status!=XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Data verification
	 */
	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		if(SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]){
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
