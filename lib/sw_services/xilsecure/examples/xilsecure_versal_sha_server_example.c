/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilsecure_versal_sha_server_example.c
* @addtogroup xsecure_sha3_example_apis XilSecure SHA3 Versal API Example Usage
* @{
* This example illustrates the SHA3 hash calculation.
* This example is supported for Versal and Versal Net devices.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   kal    04/21/21 First Release
* 4.5   kal    04/21/21 Updated file version to sync with library version
* 4.7   kpt    12/01/21 Replaced library specific,standard utility functions
*                       with xilinx maintained functions
* 4.9   bm     07/06/22 Refactor versal and versal_net code
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsecure_sha.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define SHA3_HASH_LEN_IN_BYTES  48U
#define SHA3_INPUT_DATA_LEN     6U

/************************** Function Prototypes ******************************/

static int SecureSha3Example(void);
static int SecureSha3CompareHash(u8 *Hash, u8 *ExpectedHash);
static void SecureSha3PrintHash(u8 *Hash);

/************************** Variable Definitions *****************************/

static const char Data[SHA3_INPUT_DATA_LEN + 1U] = "XILINX";

static u8 ExpHash[SHA3_HASH_LEN_IN_BYTES] =
				     {0x70,0x69,0x77,0x35,0x0b,0x93,
				      0x92,0xa0,0x48,0x2c,0xd8,0x23,
				      0x38,0x47,0xd2,0xd9,0x2d,0x1a,
				      0x95,0x0c,0xad,0xa8,0x60,0xc0,
				      0x9b,0x70,0xc6,0xad,0x6e,0xf1,
				      0x5d,0x49,0x68,0xa3,0x50,0x75,
				      0x06,0xbb,0x0b,0x9b,0x03,0x7d,
				      0xd5,0x93,0x76,0x50,0xdb,0xd4};

/*****************************************************************************/
/**
*
* Main function to call the SecureShaExample
*
* @return
*		- XST_FAILURE if the SHA calculation failed.
*
******************************************************************************/
int main(void)
{
	int Status;

	Status = SecureSha3Example();
	if(Status == XST_SUCCESS) {
		xil_printf("Successfully ran SHA example");
	}
	else {
		xil_printf("SHA Example failed");
	}
	return Status;
}

/****************************************************************************/
/**
*
* This function sends 'XILINX' to SHA-3 module for hashing.
* The purpose of this function is to illustrate how to use the XSecure_Sha3
* driver.
*
*
* @return
*		- XST_SUCCESS - SHA-3 hash successfully generated for given
*				input data string.
*		- XST_FAILURE - if the SHA-3 hash failed.
*
****************************************************************************/
/** //! [SHA3 example] */
static int SecureSha3Example()
{
	/*
	 * It is required to zeroize sha instance before calling
	 * XSecure_Sha3Initialize in server mode.
	 */
	XSecure_Sha3 Secure_Sha3 = {0U};
	XCsuDma CsuDma;
	XCsuDma_Config *Config;
	u8 Out[SHA3_HASH_LEN_IN_BYTES];
	int Status = XST_FAILURE;
	u32 Size = 0U;

	Size = Xil_Strnlen(Data, SHA3_INPUT_DATA_LEN);
	if (Size != SHA3_INPUT_DATA_LEN) {
		xil_printf("Provided data length is Invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}

	Config = XCsuDma_LookupConfig(0);
	if (NULL == Config) {
		xil_printf("config failed\n\r");
		Status = XST_FAILURE;
		goto END;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/*
	 * Initialize the SHA-3 driver so that it's ready to use
	 */
	Status = XSecure_Sha3Initialize(&Secure_Sha3, &CsuDma);
	if (Status != XST_SUCCESS) {
		xil_printf("SHA Initialization failed, Status = 0x%x \r\n",
			Status);
		goto END;
	}

	XSecure_Sha3Digest(&Secure_Sha3, (UINTPTR)Data, Size, (XSecure_Sha3Hash*)Out);


	xil_printf(" Calculated Hash \r\n ");
	SecureSha3PrintHash(Out);

	Status = SecureSha3CompareHash(Out, ExpHash);
END:
	return Status;
}

/****************************************************************************/
/**
*
* This function compares the given hash with the expected Hash
*
* @return
*		- XST_SUCCESS - if the expected hash is equal to the
*                               given hash
*		- XST_FAILURE - if the comparison fails.
*
****************************************************************************/
static int SecureSha3CompareHash(u8 *Hash, u8 *ExpectedHash)
{
	int Status = XST_FAILURE;

	Status = Xil_SMemCmp(Hash, SHA3_HASH_LEN_IN_BYTES, ExpectedHash,
			SHA3_HASH_LEN_IN_BYTES, SHA3_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("Expected Hash \r\n");
		SecureSha3PrintHash(ExpectedHash);
		xil_printf("SHA Example Failed at Hash Comparison \r\n");
	}

	return Status;
}

/****************************************************************************/
/**
*
* This function prints the given hash on the console
*
****************************************************************************/
static void SecureSha3PrintHash(u8 *Hash)
{
	u32 Index;

	for (Index = 0U; Index < SHA3_HASH_LEN_IN_BYTES; Index++) {
		xil_printf(" %0x ", Hash[Index]);
	}
	xil_printf(" \r\n ");
 }
/** //! [SHA3 example] */
/** @} */
