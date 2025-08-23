/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_ecies_example.c
* @addtogroup xasu_ecies_example ECIES API Example Generic Usage
* @{
*
* @note
* This example illustrates the usage of ASU ECIES APIs using below tests
* Encrypt the provided input data using the ECIES algorithm. The result is then decrypted back
* to get the original data. This test fails, if the decrypted data is not matched with the
* input data.
*
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------------------------------
* 1.0   yog    02/21/25 Initial Release
*       LP     04/07/25 Added additional parameters to support HKDF
*
* </pre>
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_ecies.h"
#include "xasu_client.h"
#include "xasu_aesinfo.h"
#include "xasu_eccinfo.h"
#include "xasu_shainfo.h"

/************************************ Constant Definitions ***************************************/
#define XASU_CACHE_DISABLE
#define XASU_ECIES_INPUT_DATA_LENGTH		(36U) /**< Input data length */
#define XASU_ECIES_CONTEXT_LENGTH		(36U) /**< Input context length used for KDF */
#define XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES	(64U) /**< Double length of the curve P256 */
/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsu_EciesCallBackRef(void *CallBackRef, u32 Status);

/************************************ Variable Definitions ***************************************/

static const u8 RxPrivKey[] __attribute__ ((section (".data.RxPrivKey"))) = {
	0xC9U, 0x80U, 0x68U, 0x98U, 0xA0U, 0x33U, 0x49U, 0x16U,
	0xC8U, 0x60U, 0x74U, 0x88U, 0x80U, 0xA5U, 0x41U, 0xF0U,
	0x93U, 0xB5U, 0x79U, 0xA9U, 0xB1U, 0xF3U, 0x29U, 0x34U,
	0xD8U, 0x6CU, 0x36U, 0x3CU, 0x39U, 0x80U, 0x03U, 0x57U
};

static const u8 RxPubKey[] __attribute__ ((section (".data.RxPubKey"))) = {
	0xD0U, 0x72U, 0x0DU, 0xC6U, 0x91U, 0xAAU, 0x80U, 0x09U,
	0x6BU, 0xA3U, 0x2FU, 0xEDU, 0x1CU, 0xB9U, 0x7CU, 0x2BU,
	0x62U, 0x06U, 0x90U, 0xD0U, 0x6DU, 0xE0U, 0x31U, 0x7BU,
	0x86U, 0x18U, 0xD5U, 0xCEU, 0x65U, 0xEBU, 0x72U, 0x8FU,
	0x96U, 0x81U, 0xB5U, 0x17U, 0xB1U, 0xCDU, 0xA1U, 0x7DU,
	0x0DU, 0x83U, 0xD3U, 0x35U, 0xD9U, 0xC4U, 0xA8U, 0xA9U,
	0xA9U, 0xB0U, 0xB1U, 0xB3U, 0xC7U, 0x10U, 0x6DU, 0x8FU,
	0x3CU, 0x72U, 0xBCU, 0x50U, 0x93U, 0xDCU, 0x27U, 0x5FU
};

static const u8 Data[] __attribute__ ((section (".data.Data"))) = {
	0x48U, 0x65U, 0x6CU, 0x6CU, 0x6FU, 0x20U, 0x42U, 0x6FU,
	0x62U, 0x2CU, 0x20U, 0x74U, 0x68U, 0x69U, 0x73U, 0x20U,
	0x69U, 0x73U, 0x20U, 0x61U, 0x20U, 0x73U, 0x65U, 0x63U,
	0x72U, 0x65U, 0x74U, 0x20U, 0x6DU, 0x65U, 0x73U, 0x73U,
	0x61U, 0x67U, 0x65U, 0x21U
};

static const u8 Context[] __attribute__ ((section (".data.Context"))) = {
	0x27U, 0x1BU, 0x63U, 0x0BU, 0x3AU, 0x5DU, 0xD7U, 0x5FU,
	0x08U, 0x17U, 0xCDU, 0xE1U, 0xB8U, 0x97U, 0x8AU, 0x59U
};

static const u8 Iv[XASU_AES_IV_SIZE_96BIT_IN_BYTES] __attribute__ ((section (".data.Iv"))) = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U, 0x62U, 0x98U,
	0xDFU, 0xEAU, 0x5AU, 0xB2U
};

static u8 TxPubKey[XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES] __attribute__ ((section (".data.TxPubKey")));
static u8 Mac[XASU_AES_MAX_TAG_LENGTH_IN_BYTES] __attribute__ ((section (".data.Mac")));
static u8 OutEncData[XASU_ECIES_INPUT_DATA_LENGTH] __attribute__ ((section (".data.OutEncData")));
static u8 OutDecData[XASU_ECIES_INPUT_DATA_LENGTH] __attribute__ ((section (".data.OutDecData")));

volatile u8 Notify = 0; /**< To notify the call back from client library */
volatile u32 ErrorStatus = XST_FAILURE; /**< Status variable to store the error returned from
						server. */
/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
*
* @brief	Main function to call the XAsu_EciesEncrypt() API and XAsu_EciesDecrypt() API.
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
	XAsu_EciesParams EciesParams;
	u32 Index = 0U;
	XMailbox MailboxInstance;

#ifdef XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

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
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_EciesCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;

	EciesParams.EccCurveType = XASU_ECC_NIST_P256;
	EciesParams.ShaType = XASU_SHA2_TYPE;
	EciesParams.ShaMode = XASU_SHA_MODE_256;
	EciesParams.AesKeySize = XASU_AES_KEY_SIZE_256_BITS;
	EciesParams.DataLength = XASU_ECIES_INPUT_DATA_LENGTH;
	EciesParams.EccKeyLength = XASU_ECC_P256_SIZE_IN_BYTES;
	EciesParams.RxKeyAddr = (u64)(UINTPTR)RxPubKey;
	EciesParams.TxKeyAddr = (u64)(UINTPTR)TxPubKey;
	EciesParams.InDataAddr = (u64)(UINTPTR)Data;
	EciesParams.IvAddr = (u64)(UINTPTR)Iv;
	EciesParams.OutDataAddr = (u64)(UINTPTR)OutEncData;
	EciesParams.MacAddr = (u64)(UINTPTR)Mac;
	EciesParams.ContextAddr = (u64)(UINTPTR)Context;
	EciesParams.ContextLen = XASU_ECIES_CONTEXT_LENGTH;
	EciesParams.SaltAddr = (u64)(UINTPTR)0U;
	EciesParams.SaltLen = 0U;
	EciesParams.MacLength = XASU_AES_MAX_TAG_LENGTH_IN_BYTES;
	EciesParams.IvLength = XASU_AES_IV_SIZE_96BIT_IN_BYTES;

	Status = XAsu_EciesEncrypt(&ClientParams, &EciesParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Encrypt operation failed, Status = %x \n\r", Status);
		goto END;
	}
	while(!Notify);
	Notify = 0;
	if (ErrorStatus != XST_SUCCESS) {
		goto END;
	}

	XilAsu_Printf("\r\n Encrypted Output: ");
	for (Index = 0; Index < XASU_ECIES_INPUT_DATA_LENGTH; Index++) {
		XilAsu_Printf("%02x", OutEncData[Index]);
	}

	XilAsu_Printf("\r\n Generated Mac: ");
	for (Index = 0; Index < XASU_AES_MAX_TAG_LENGTH_IN_BYTES; Index++) {
		XilAsu_Printf("%02x", Mac[Index]);
	}

	XilAsu_Printf("\r\n Generated Iv: ");
	for (Index = 0; Index < XASU_AES_IV_SIZE_96BIT_IN_BYTES; Index++) {
		XilAsu_Printf("%02x", Iv[Index]);
	}

	ErrorStatus = XST_FAILURE;
	EciesParams.EccCurveType = XASU_ECC_NIST_P256;
	EciesParams.ShaType = XASU_SHA2_TYPE;
	EciesParams.ShaMode = XASU_SHA_MODE_256;
	EciesParams.AesKeySize = XASU_AES_KEY_SIZE_256_BITS;
	EciesParams.DataLength = XASU_ECIES_INPUT_DATA_LENGTH;
	EciesParams.EccKeyLength = XASU_ECC_P256_SIZE_IN_BYTES;
	EciesParams.RxKeyAddr = (u64)(UINTPTR)RxPrivKey;
	EciesParams.TxKeyAddr = (u64)(UINTPTR)TxPubKey;
	EciesParams.InDataAddr = (u64)(UINTPTR)OutEncData;
	EciesParams.IvAddr = (u64)(UINTPTR)Iv;
	EciesParams.OutDataAddr = (u64)(UINTPTR)OutDecData;
	EciesParams.MacAddr = (u64)(UINTPTR)Mac;
	EciesParams.ContextAddr = (u64)(UINTPTR)Context;
	EciesParams.ContextLen = XASU_ECIES_CONTEXT_LENGTH;
	EciesParams.SaltAddr = (u64)(UINTPTR)0U;
	EciesParams.SaltLen = 0U;
	EciesParams.MacLength = XASU_AES_MAX_TAG_LENGTH_IN_BYTES;
	EciesParams.IvLength = XASU_AES_IV_SIZE_96BIT_IN_BYTES;

	Status = XAsu_EciesDecrypt(&ClientParams, &EciesParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Decrypt operation failed, Status = %x \n\r", Status);
		goto END;
	}
	while(!Notify);
	Notify = 0;
	if (ErrorStatus != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCmp_CT(OutDecData, XASU_ECIES_INPUT_DATA_LENGTH, Data,
				XASU_ECIES_INPUT_DATA_LENGTH, XASU_ECIES_INPUT_DATA_LENGTH);
	XilAsu_Printf("\r\n Decrypted Output: ");
	for (Index = 0; Index < XASU_ECIES_INPUT_DATA_LENGTH; Index++) {
		XilAsu_Printf("%02x", OutDecData[Index]);
	}
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Decrypted Output comparision failed, Status = %x \n\r", Status);
	}

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n ECIES client example failed with Status = %08x", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n ECIES client example failed with error from server = %08x", ErrorStatus);
	} else {
		xil_printf("\r\n Successfully ran ECIES client example ");
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
static void XAsu_EciesCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;
	/* Update the variable to notify the callback */
	Notify = 1U;
}
