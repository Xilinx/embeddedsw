/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file	xilsecure_spartanup_sha_server_example.c
* @addtogroup xsecure_sha_example_apis XilSecure SHA SPARTANUP API Example Usage
* @{
* This example illustrates the SHA3_256 hash calculation.
* This example is supported for spartanup device.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   kpt    09/03/24 First Release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsecure_sha.h"
#include "xsecure_plat.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define SHA_HASH_LEN_IN_BYTES  (XSECURE_SHA3_256_HASH_LEN) /**< SHA hash length in bytes */
#define SHA_INPUT_DATA_LEN     8U /**< Input data length */
#define SHA_MODE               (XSECURE_SHA3_256) /**< SHA mode */

/************************** Function Prototypes ******************************/

static int SecureShaExample(void);
static int SecureShaCompareHash(u8 *Hash, u8 *ExpectedHash);
static void SecureShaPrintHash(u8 *Hash);

/************************** Variable Definitions *****************************/

static const char Data[SHA_INPUT_DATA_LEN + 1U] = "SHA3_256";
/**< Input data should be word aligned */

#if (SHA_MODE == XSECURE_SHA3_256)
static u8 ExpHash[SHA_HASH_LEN_IN_BYTES] = {
	0x88U, 0x8FU, 0xBDU, 0x5BU, 0x0EU, 0xB9U, 0xD7U, 0x7EU,
	0x15U, 0x1BU, 0xD3U, 0x83U, 0xD9U, 0x3BU, 0x16U, 0xEAU,
	0xF2U, 0xF1U, 0x4FU, 0xC7U, 0x06U, 0x8CU, 0x61U, 0x23U,
	0x09U, 0x57U, 0x9BU, 0x4BU, 0x79U, 0x91U, 0x6CU, 0xE2U
};
#elif (SHA_MODE == XSECURE_SHAKE_256)
static u8 ExpHash[SHA_HASH_LEN_IN_BYTES] = {
	0x12U, 0x80U, 0x23U, 0x88U, 0xF9U, 0xE0U, 0x26U, 0xF1U,
	0x1AU, 0xFFU, 0xA7U, 0xF8U, 0xDCU, 0xE1U, 0xA3U, 0xECU,
	0x79U, 0x91U, 0xB6U, 0xBCU, 0xD0U, 0x86U, 0x23U, 0x8BU,
	0xB4U, 0x18U, 0x6BU, 0x04U, 0x45U, 0x56U, 0xBDU, 0xA1U
};
#else
#error "Invaild SHA mode selected"
#endif

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

	Status = SecureShaExample();
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran SHA example");
	} else {
		xil_printf("SHA Example failed");
	}
	return Status;
}

/****************************************************************************/
/**
*
* This function sends string to SHA module for hashing.
* The purpose of this function is to illustrate how to use the SHA
* driver.
*
* @return
*		- XST_SUCCESS - hash successfully generated for given
*				input data string.
*		- XST_FAILURE - if the hash generation failed.
*
****************************************************************************/
/** //! [SHA3 example] */
static int SecureShaExample()
{
	int Status = XST_FAILURE;
	XSecure_Sha  Secure_Sha = {0U};
	XPmcDma PmcDma;
	XPmcDma_Config *Config;
	u8 Out[SHA_HASH_LEN_IN_BYTES];
	u32 Size = 0U;

	Size = Xil_Strnlen(Data, SHA_INPUT_DATA_LEN);
	if (Size != SHA_INPUT_DATA_LEN) {
		xil_printf("Provided data length is Invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}

	Config = XPmcDma_LookupConfig(0U);
	if (NULL == Config) {
		xil_printf("config failed\n\r");
		Status = XST_FAILURE;
		goto END;
	}

	Status = XPmcDma_CfgInitialize(&PmcDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/*
	 * Initialize the SHA driver so that it's ready to use
	 */
	Status = XSecure_ShaInitialize(&Secure_Sha, &PmcDma);
	if (Status != XST_SUCCESS) {
		xil_printf("SHA Initialization failed, Status = 0x%x \r\n",
			   Status);
		goto END;
	}

	Status = XSecure_ShaDigest(&Secure_Sha, SHA_MODE, (UINTPTR)Data, Size, (u64)(UINTPTR)Out,
				   SHA_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("SHA digest failed, Status = 0x%x \r\n",
			   Status);
		goto END;
	}

	xil_printf(" Calculated Hash \r\n ");
	SecureShaPrintHash(Out);

	Status = SecureShaCompareHash(Out, ExpHash);
END:
	return Status;
}

/****************************************************************************/
/**
* This function compares the given hash with the expected Hash
*
* @return
*		- XST_SUCCESS - if the expected hash is equal to the
*                               given hash
*		- XST_FAILURE - if the comparison fails.
*
****************************************************************************/
static int SecureShaCompareHash(u8 *Hash, u8 *ExpectedHash)
{
	int Status = XST_FAILURE;

	Status = Xil_SMemCmp(Hash, SHA_HASH_LEN_IN_BYTES, ExpectedHash,
			     SHA_HASH_LEN_IN_BYTES, SHA_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("Expected Hash \r\n");
		SecureShaPrintHash(ExpectedHash);
		xil_printf("SHA Example Failed at Hash Comparison \r\n");
	}

	return Status;
}

/****************************************************************************/
/**
* This function prints the given hash on the console
*
****************************************************************************/
static void SecureShaPrintHash(u8 *Hash)
{
	u32 Index;

	for (Index = 0U; Index < SHA_HASH_LEN_IN_BYTES; Index++) {
		xil_printf(" %0x ", Hash[Index]);
	}
	xil_printf(" \r\n ");
}
/** //! [SHA example] */
/** @} */
