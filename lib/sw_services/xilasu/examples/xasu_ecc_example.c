/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       ss     09/19/24 Added print for client init failure
*       am     09/24/24 Added SDT support
*       yog    11/27/24 Handling the error returned from server
*       am     03/05/25 Integrated performance measurement macro
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
#define XASU_CURVE_TYPE		XASU_ECC_NIST_P256
#define XASU_CURVE_LENGTH	XASU_ECC_P256_SIZE_IN_BYTES

/************************************ Function Prototypes ****************************************/
static void XAsu_EccCallBackRef(void *CallBackRef, u32 Status);

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
static u8 PubKeyOut[XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES] __attribute__ ((section (".data.PubKeyOut")));

volatile u8 Notify = 0U; /**< To notify the call back from client library. */
volatile u32 ErrorStatus = XST_FAILURE; /**< Status variable to store the error returned from server. */

/************************************ Function Definitions ***************************************/
static void XAsu_GenSign(void);
static void XAsu_VerifySign(void);
static void XAsu_GenPubKey(void);

/*************************************************************************************************/
/**
 * @brief	Main function to call the XAsu_EccGenSign() and XAsu_EccVerifySign() API's.
 *
 * @return
 *		- XST_SUCCESS if example runs successfully.
 *		- ErrorCode if the example fails.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XMailbox MailboxInstance;

#ifdef XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed: %08x \r\n", Status);
		goto END;
	}

	/* Initialize the client instance */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	/** Generate signature operation. */
	XAsu_GenSign();

	/** Verify signature operation. */
	XAsu_VerifySign();

	/** Generate public key operation. */
	XAsu_GenPubKey();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is to perform the generate signature operation.
 *
 *************************************************************************************************/
static void XAsu_GenSign(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XAsu_EccParams EccParams;
	u32 Index = 0U;

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_EccCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;

	ErrorStatus = XST_FAILURE;
	EccParams.CurveType = XASU_CURVE_TYPE;
	EccParams.DigestAddr = (u64)(UINTPTR)Hash;
	EccParams.KeyAddr = (u64)(UINTPTR)PrivKey;
	EccParams.SignAddr = (u64)(UINTPTR)Sign;
	EccParams.DigestLen = XASU_CURVE_LENGTH;
	EccParams.KeyLen = XASU_CURVE_LENGTH;

	/* Measure start time. */
	XAsu_StartTiming();
	Status = XAsu_EccGenSign(&ClientParams, &EccParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	while(!Notify);
	Notify = 0U;
	if ((ErrorStatus != XST_SUCCESS)) {
		goto END;
	}
	/* Compute execution time in milliseconds. */
	XAsu_EndTiming("XAsu_EccGenSign");

	xil_printf("\r\n Generated Sign: ");
	for (Index = 0; Index < XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES; Index++) {
		xil_printf("%02x", Sign[Index]);
	}

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n ECC sign generation operation failed with Status = %08x", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n ECC sign generation operation failed with error from server = %08x",
				ErrorStatus);
	} else {
		xil_printf("\r\n Successfully ran ECC sign generation operation ");
	}
}

/*************************************************************************************************/
/**
 * @brief	This function is to perform the verify signature operation.
 *
 *************************************************************************************************/
static void XAsu_VerifySign(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_EccParams EccParams;

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_EccCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;

	ErrorStatus = XST_FAILURE;
	EccParams.CurveType = XASU_CURVE_TYPE;
	EccParams.DigestAddr = (u64)(UINTPTR)Hash;
	EccParams.KeyAddr = (u64)(UINTPTR)PubKey;
	EccParams.SignAddr = (u64)(UINTPTR)Sign;
	EccParams.DigestLen = XASU_CURVE_LENGTH;
	EccParams.KeyLen = XASU_CURVE_LENGTH;

	Status = XAsu_EccVerifySign(&ClientParams, &EccParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	while(!Notify);
	Notify = 0U;

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n ECC sign verification operation failed with Status = %08x", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n ECC sign verification operation failed with error from server = %08x",
				ErrorStatus);
	} else {
		xil_printf("\r\n Successfully ran ECC sign verification operation ");
	}
}

/*************************************************************************************************/
/**
 * @brief	This function is to perform the generate public key operation.
 *
 *************************************************************************************************/
static void XAsu_GenPubKey(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams;
	XAsu_EccKeyParams EccKeyParams;

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_EccCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;

	ErrorStatus = XST_FAILURE;
	EccKeyParams.CurveType = XASU_CURVE_TYPE;
	EccKeyParams.KeyLen = XASU_CURVE_LENGTH;
	EccKeyParams.PvtKeyAddr = (u64)(UINTPTR)PrivKey;
	EccKeyParams.PubKeyAddr = (u64)(UINTPTR)PubKeyOut;

	Status = XAsu_EccGenPubKey(&ClientParams, &EccKeyParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	while(!Notify);
	Notify = 0U;
	if ((ErrorStatus != XST_SUCCESS)) {
		goto END;
	}

	/** Compare the generated public key with the expected. */
	Status = Xil_SMemCmp_CT(PubKeyOut, XAsu_DoubleCurveLength(XASU_CURVE_LENGTH), PubKey,
			XAsu_DoubleCurveLength(XASU_CURVE_LENGTH), XAsu_DoubleCurveLength(XASU_CURVE_LENGTH));
	if (Status != XST_SUCCESS) {
		xil_printf("ASU ECC Example Failed at generated public key data Comparison \r\n");
	}

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n ECC public key generation operation failed with Status = %08x", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n ECC public key generation operation failed with error from server = %08x",
				ErrorStatus);
	} else {
		xil_printf("\r\n Successfully ran ECC public key generation operation ");
	}
}

/*************************************************************************************************/
/**
 * @brief	Call back function which will be registered with library to notify the completion
 * 		of request
 *
 * @param	CallBackRef	Pointer to the call back reference.
 * @param	Status		Status of the request will be passed as an argument during callback
 * 			- 0 Upon success
 * 			- Error code from ASUFW application upon any error
 *
 *************************************************************************************************/
static void XAsu_EccCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;
	/* Update the variable to notify the callback */
	Notify = 1U;
}
