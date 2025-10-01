/**************************************************************************************************
* Copyright (c) 2024 -2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_ecdh_client_example.c
* @addtogroup xasufw_ecdh_client_example ECDH API Example Generic Usage
* @{
*
* @note
* This example illustrates the usage of ASU ECDH APIs using below tests
* 	Generate secret to the NIST P-256 curve using two key pairs
*	and verifies the generated secret are same for both key pairs.
*
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------------------------------
* 1.0   ss    12/02/24 Initial Release
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

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsu_EcdhCallBackRef(void *CallBackRef, u32 Status);

/************************************ Variable Definitions ***************************************/

static const u8 PrivKey[] __attribute__ ((section (".data.PrivKey"))) = {
        0xC9U, 0x80U, 0x68U, 0x98U, 0xA0U, 0x33U, 0x49U, 0x16U, 0xC8U, 0x60U, 0x74U, 0x88U, 0x80U,
        0xA5U, 0x41U, 0xF0U, 0x93U, 0xB5U, 0x79U, 0xA9U, 0xB1U, 0xF3U, 0x29U, 0x34U, 0xD8U, 0x6CU,
        0x36U, 0x3CU, 0x39U, 0x80U, 0x03U, 0x57U
};

static const u8 PubKey[] __attribute__ ((section (".data.PubKey"))) = {
        0XD0U, 0X72U, 0X0DU, 0XC6U, 0X91U, 0XAAU, 0X80U, 0X09U, 0X6BU, 0XA3U, 0X2FU, 0XEDU, 0X1CU,
        0XB9U, 0X7CU, 0X2BU, 0X62U, 0X06U, 0X90U, 0XD0U, 0X6DU, 0XE0U, 0X31U, 0X7BU, 0X86U, 0X18U,
        0XD5U, 0XCEU, 0X65U, 0XEBU, 0X72U, 0X8FU, 0X96U, 0X81U, 0XB5U, 0X17U, 0XB1U, 0XCDU, 0XA1U,
        0X7DU, 0X0DU, 0X83U, 0XD3U, 0X35U, 0XD9U, 0XC4U, 0XA8U, 0XA9U, 0XA9U, 0XB0U, 0XB1U, 0XB3U,
        0XC7U, 0X10U, 0X6DU, 0X8FU, 0X3CU, 0X72U, 0XBCU, 0X50U, 0X93U, 0XDCU, 0X27U, 0X5FU
};

static const u8 PrivKey1[] __attribute__ ((section (".data.PrivKey1"))) = {
        0X71U, 0X07U, 0X35U, 0XC8U, 0X38U, 0X8FU, 0X48U, 0XC6U, 0X84U, 0XA9U, 0X7BU, 0XD6U, 0X67U,
        0X51U, 0XCCU, 0X5FU, 0X5AU, 0X12U, 0X2DU, 0X6BU, 0X9AU, 0X96U, 0XA2U, 0XDBU, 0XE7U, 0X36U,
        0X62U, 0XF7U, 0X82U, 0X17U, 0X44U, 0X6DU
};

static const u8 PubKey1[] __attribute__ ((section (".data.PubKey1"))) = {
        0XF6U, 0X83U, 0X6AU, 0X8AU, 0XDDU, 0X91U, 0XCBU, 0X18U, 0X2DU, 0X8DU, 0X25U, 0X8DU, 0XDAU,
        0X66U, 0X80U, 0X69U, 0X0EU, 0XB7U, 0X24U, 0XA6U, 0X6DU, 0XC3U, 0XBBU, 0X60U, 0XD2U, 0X32U,
        0X25U, 0X65U, 0XC3U, 0X9EU, 0X4AU, 0XB9U, 0X1FU, 0X83U, 0X7AU, 0XA3U, 0X28U, 0X64U, 0X87U,
        0X0CU, 0XB8U, 0XE8U, 0XD0U, 0XACU, 0X2FU, 0XF3U, 0X1FU, 0X82U, 0X4EU, 0X7BU, 0XEDU, 0XDCU,
        0X4BU, 0XB7U, 0XADU, 0X72U, 0XC1U, 0X73U, 0XADU, 0X97U, 0X4BU, 0X28U, 0X9DU, 0XC2U
};
static const u8 ExpSharedSecret[] __attribute__ ((section (".data.ExpSharedSecret"))) = {
	0x1DU, 0xB8U, 0x09U, 0xC2U, 0x76U, 0xF2U, 0x16U, 0x10U, 0x79U, 0x11U, 0x68U, 0x52U, 0x8EU,
	0xFAU, 0x01U, 0x85U, 0x11U, 0x2EU, 0x78U, 0x65U, 0x50U, 0x36U, 0xAEU, 0xEDU, 0x87U, 0xC7U,
	0x15U, 0xA2U, 0x90U, 0x45U, 0xFDU, 0xFCU
};
static u8 SharedSecret[XASU_ECC_P256_SIZE_IN_BYTES]
        __attribute__ ((section (".data.SharedSecret")));

static u8 SharedSecret1[XASU_ECC_P256_SIZE_IN_BYTES]
        __attribute__ ((section (".data.SharedSecret1")));

volatile u8 Notify = 0; /**< To notify the call back from client library */
volatile u32 ErrorStatus = XST_FAILURE; /**< Status variable to store the error returned from
						server. */

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
*
* @brief	Main function to call the XAsu_EcdhGenSharedSecret() API
*
* @param	None
*
* @return
*		- XST_SUCCESS if example runs successfully
*		- ErrorCode if the example fails.
*
**************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XAsu_EcdhParams EcdhParams;
	u32 Index = 0U;
	u32 CurveType = 0U;
	u32 CurveLength = 0U;
	XMailbox MailboxInstance;

#ifdef XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	CurveType = XASU_ECC_NIST_P256;
	CurveLength = XASU_ECC_P256_SIZE_IN_BYTES;

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x \r\n", Status);
		goto END;
	}

	/* Initialize the client instance */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ASU Client initialize failed: %08x \r\n", Status);
		goto END;
	}

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_EcdhCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;
	ClientParams.AdditionalStatus = XST_FAILURE;

	ErrorStatus = XST_FAILURE;
	EcdhParams.CurveType = CurveType;
	EcdhParams.KeyLen = CurveLength;
	EcdhParams.SharedSecretObjIdAddr = 0U;
	EcdhParams.PvtKeyAddr = (u64)(UINTPTR)PrivKey;
	EcdhParams.PubKeyAddr = (u64)(UINTPTR)PubKey1;
	EcdhParams.SharedSecretAddr = (u64)(UINTPTR)SharedSecret;

	Status = XAsu_EcdhGenSharedSecret(&ClientParams, &EcdhParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Secret1 generation for ECDH failed, Status = %x \n\r", Status);
		goto END;
	}
	while(!Notify);
	Notify = 0;
	if ((ErrorStatus != XST_SUCCESS) ||
	   (ClientParams.AdditionalStatus != XASU_RSA_ECDH_SUCCESS)) {
		goto END;
	}

	Status = Xil_SMemCmp_CT(SharedSecret, XASU_ECC_P256_SIZE_IN_BYTES, ExpSharedSecret,
				XASU_ECC_P256_SIZE_IN_BYTES, XASU_ECC_P256_SIZE_IN_BYTES);
	XilAsu_Printf("\r\n Generated Secret1: ");
	for (Index = 0; Index < XASU_ECC_P256_SIZE_IN_BYTES; Index++) {
		XilAsu_Printf("%02x", SharedSecret[Index]);
	}
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Secret1 and Expected Secret Comparison failed, Status = %x \n\r",
				Status);
		goto END;
	}

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_EcdhCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;
	ClientParams.AdditionalStatus = XST_FAILURE;

	ErrorStatus = XST_FAILURE;
        EcdhParams.CurveType = CurveType;
        EcdhParams.KeyLen = CurveLength;
	EcdhParams.SharedSecretObjIdAddr = 0U;
	EcdhParams.PvtKeyAddr = (u64)(UINTPTR)PrivKey1;
	EcdhParams.PubKeyAddr = (u64)(UINTPTR)PubKey;
	EcdhParams.SharedSecretAddr = (u64)(UINTPTR)SharedSecret1;

        Status = XAsu_EcdhGenSharedSecret(&ClientParams, &EcdhParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Secret2 generation for ECDH failed, Status = %x \n\r", Status);
		goto END;
	}
	while(!Notify);
	Notify = 0;
	if ((ErrorStatus != XST_SUCCESS) ||
	   (ClientParams.AdditionalStatus != XASU_RSA_ECDH_SUCCESS)) {
		goto END;
	}

	Status = Xil_SMemCmp_CT(SharedSecret1, XASU_ECC_P256_SIZE_IN_BYTES, ExpSharedSecret,
				XASU_ECC_P256_SIZE_IN_BYTES, XASU_ECC_P256_SIZE_IN_BYTES);
	XilAsu_Printf("\r\n Generated Secret2: ");
	for (Index = 0; Index < XASU_ECC_P256_SIZE_IN_BYTES; Index++) {
		XilAsu_Printf("%02x", SharedSecret1[Index]);
	}
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Secret2 and Expected Secret Comparison failed, Status = %x \n\r",
				Status);
	}

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n ECDH client example failed with Status = %08x", Status);
	} else if ((ErrorStatus != XST_SUCCESS) ||
		  (ClientParams.AdditionalStatus != XASU_RSA_ECDH_SUCCESS)) {
		xil_printf("\r\n ECDH client example failed with error from server Status = %08x,"
			" Additional Status = %08x", ErrorStatus, ClientParams.AdditionalStatus);
	} else {
		xil_printf("\r\n Successfully ran ECDH client example ");
	}

	return Status;
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
  static void XAsu_EcdhCallBackRef(void *CallBackRef, u32 Status)
 {
	(void)CallBackRef;

	ErrorStatus = Status;
	/* Update the variable to notify the callback */
	Notify = 1U;
 }