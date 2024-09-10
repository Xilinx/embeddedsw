/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_ecc_client_example.c
* @addtogroup xasufw_ecc_client_example ECC API Example Generic Usage
* @{
*
* @note
* This example illustrates the usage of ASU ECC APIs using below tests
* 	Generate signature to the NIST P-256 curve using the private key and hash
*	provided and verifies the generated signature.
*
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------------------------------
* 1.0   yog    08/19/24 Initial Release
*
* </pre>
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_ecc.h"
#include "xasu_client.h"

/************************************ Constant Definitions ***************************************/
#define XASU_CACHE_DISABLE
#define XASU_ECC_P256_SIZE_IN_BYTES		(32U) /**< Size of NIST P-256 curve in bytes */
#define XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES	(64U) /**< 2 * Size of NIST P-256 curve in bytes */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

static const u8 PrivKey[] __attribute__ ((section (".data.PrivKey"))) = {
	0xC4U, 0x77U, 0xF9U, 0xF6U, 0x5CU, 0x22U, 0xCCU, 0xE2U,
	0x06U, 0x57U, 0xFAU, 0xA5U, 0xB2U, 0xD1U, 0xD8U, 0x12U,
	0x23U, 0x36U, 0xF8U, 0x51U, 0xA5U, 0x08U, 0xA1U, 0xEDU,
	0x04U, 0xE4U, 0x79U, 0xC3U, 0x49U, 0x85U, 0xBFU, 0x96U
};

static const u8 Hash[] __attribute__ ((section (".data.Hash"))) = {
	0xA4U, 0x1AU, 0x41U, 0xA1U, 0x2AU, 0x79U, 0x95U, 0x48U,
	0x21U, 0x1CU, 0x41U, 0x0CU, 0x65U, 0xD8U, 0x13U, 0x3AU,
	0xFDU, 0xE3U, 0x4DU, 0x28U, 0xBDU, 0xD5U, 0x42U, 0xE4U,
	0xB6U, 0x80U, 0xCFU, 0x28U, 0x99U, 0xC8U, 0xA8U, 0xC4U
};

static const u8 PubKey[] __attribute__ ((section (".data.PubKey"))) = {
	0xB7U, 0xE0U, 0x8AU, 0xFDU, 0xFEU, 0x94U, 0xBAU, 0xD3U,
	0xF1U, 0xDCU, 0x8CU, 0x73U, 0x47U, 0x98U, 0xBAU, 0x1CU,
	0x62U, 0xB3U, 0xA0U, 0xADU, 0x1EU, 0x9EU, 0xA2U, 0xA3U,
	0x82U, 0x01U, 0xCDU, 0x08U, 0x89U, 0xBCU, 0x7AU, 0x19U,
	0x36U, 0x03U, 0xF7U, 0x47U, 0x95U, 0x9DU, 0xBFU, 0x7AU,
	0x4BU, 0xB2U, 0x26U, 0xE4U, 0x19U, 0x28U, 0x72U, 0x90U,
	0x63U, 0xADU, 0xC7U, 0xAEU, 0x43U, 0x52U, 0x9EU, 0x61U,
	0xB5U, 0x63U, 0xBBU, 0xC6U, 0x06U, 0xCCU, 0x5EU, 0x09U
};

static u8 Sign[XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES] __attribute__ ((section (".data.Sign")));

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
*
* @brief	Main function to call the XAsu_EccGenSign() and XAsu_EccVerifySign() API's
*
* @param	None
*
* @return
*		- XST_SUCCESS if example runs successfully
*		- ErrorCode if the example fails.
*
**************************************************************************************************/
s32 main(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_EccParams EccParams;
	u32 Index = 0U;
	u32 CurveType = 0U;
	u32 CurveLength = 0U;

#ifdef XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	CurveType = XASU_ECC_NIST_P256;
	CurveLength = XASU_ECC_P256_SIZE_IN_BYTES;

	Status = XAsu_ClientInit(0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	EccParams.CurveType = CurveType;
	EccParams.DigestAddr = (u64)Hash;
	EccParams.KeyAddr = (u64)PrivKey;
	EccParams.SignAddr = (u64)Sign;
	EccParams.DigestLen = CurveLength;
	EccParams.KeyLen = CurveLength;

	Status = XAsu_EccGenSign(&ClientParams, &EccParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\r\n Generated Sign: ");
	for (Index = 0; Index < XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES; Index++) {
		xil_printf("%02x", Sign[Index]);
	}

	EccParams.CurveType = CurveType;
	EccParams.DigestAddr = (u64)Hash;
	EccParams.KeyAddr = (u64)PubKey;
	EccParams.SignAddr = (u64)Sign;
	EccParams.DigestLen = CurveLength;
	EccParams.KeyLen = CurveLength;

	Status = XAsu_EccVerifySign(&ClientParams, &EccParams);

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n ECC client example failed with Status = %08x", Status);
	} else {
		xil_printf("\r\n Successfully ran ECC client example ");
	}

	return Status;
}
