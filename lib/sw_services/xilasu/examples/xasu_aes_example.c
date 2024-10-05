/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aes_example.c
 * @addtogroup Overview
 * @{
 *
 * This example illustrates the usage of ASU AES client APIs for AES GCM and CTR engine modes.
 * Encrypt the input data with provided key and IV, the output result is then decrypted
 * back to get original data and verifies the GCM tag. The test fails, if decryption does not
 * produce the original data or mismatch in GCM tag.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory.
 * For this example to work, any data shared between client running on A78/R52/PL and server
 * running on ASU, should be placed in area which is acccessible to both client and server.
 *
 * Following is the procedure to compile the example on any memory region which can be accessed
 * by the server.
 *
 *      1. Open ASU application linker script(lscript.ld) and there will be an memory
 *         mapping section which should be updated to point all the required sections
 *         to shared memory using a memory region selection
 *
 *                                      OR
 *
 *      1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
 *              .sharedmemory : {
 *              . = ALIGN(4);
 *              __sharedmemory_start = .;
 *              *(.sharedmemory)
 *              *(.sharedmemory.*)
 *              *(.gnu.linkonce.d.*)
 *              __sharedmemory_end = .;
 *              } > Shared_memory_area
 *
 *      2. In this example ".data" section elements that are passed by reference to the server-side
 *         should be stored in the above shared memory section.
 *         Replace ".data" in attribute section with ".sharedmemory", as shown below-
 *      static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".data.Data")));
 *                              should be changed to
 *      static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".sharedmemory.Data")));
 *
 * To keep things simple, by default the cache is disabled in this example using
 * XASU_ENABLE_CACHE macro.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   07/10/24 Initial release
 *       am   08/25/24 Added XASU_DISABLE_CACHE macro
 *       am   09/24/24 Added SDT support
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_aes.h"

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_DISABLE_CACHE

#define XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES	(32U) /**< AES payload data length in bytes */
#define XASU_AES_AAD_LEN_IN_BYTES		(16U) /**< AES AAD length in bytes */
#define XASU_AES_KEY_LEN_IN_BYTES		(32U) /**< AES Key length in bytes */
#define XASU_AES_IV_LEN_IN_BYTES		(12U) /**< AES Iv length in bytes */
#define XASU_AES_TAG_LEN_IN_BYTES		(16U) /**< AES Tag length in bytes */

/************************************ Function Prototypes ****************************************/
static s32 XAsu_AesGcmExample(void);
static s32 XAsu_AesCtrExample(void);
static void XAsu_AesPrintData(const u8 *Data, u32 DataLen);
static void XAsu_AesCallBackRef(void *CallBackRef, u32 Status);

/************************************ Variable Definitions ***************************************/
#if defined (__GNUC__)
/* AES plaintext data */
static u8 XAsu_AesInputData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.XAsu_AesInputData"))) = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

/* AES AAD data */
static u8 XAsu_AesAad[XASU_AES_AAD_LEN_IN_BYTES] __attribute__ ((section (".data.XAsu_AesAad"))) = {
	0x9AU, 0x7BU, 0x86U, 0xE7U, 0x82U, 0xCCU, 0xAAU, 0x6AU,
	0xB2U, 0x21U, 0xBDU, 0x03U, 0x47U, 0x0BU, 0xDCU, 0x2EU
};

/* AES key */
static u8 XAsu_AesKey[XASU_AES_KEY_LEN_IN_BYTES] __attribute__ ((section (".data.XAsu_AesKey"))) = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};

/* AES IV */
static u8 XAsu_AesIv[XASU_AES_IV_LEN_IN_BYTES] __attribute__ ((section (".data.XAsu_AesIv"))) = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U,
	0x62U, 0x98U, 0xDFU, 0xEAU, 0x5AU, 0xB2U
};

/* AES-GCM expected CipherText */
static u8 XAsu_AesGcmExpCt[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.XAsu_AesGcmExpCt"))) = {
	0x59U, 0x8CU, 0xD1U, 0x9FU, 0x16U, 0x83U, 0xB4U, 0x1BU,
	0x4CU, 0x59U, 0xE1U, 0xC1U, 0x57U, 0xD4U, 0x15U, 0x01U,
	0xA3U, 0xC0U, 0x89U, 0x02U, 0xF0U, 0xEAU, 0x3AU, 0x37U,
	0x6AU, 0x8BU, 0x0DU, 0x99U, 0x88U, 0xCFU, 0xF8U, 0xC1U
};

/* AES-CTR expected CipherText */
static u8 XAsu_AesCtrExpCt[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES] __attribute__ ((
		section (".data.XAsu_AesCtrExpCt"))) = {
	0x40U, 0x67U, 0x04U, 0x6BU, 0x20U, 0x74U, 0x83U, 0xC5U,
	0x86U, 0x7FU, 0x3DU, 0x1DU, 0xFDU, 0x6BU, 0x27U, 0xF3U,
	0x5CU, 0x07U, 0x75U, 0xCEU, 0xEEU, 0x85U, 0x6DU, 0xE7U,
	0x0EU, 0xD8U, 0x20U, 0xBAU, 0xC3U, 0x72U, 0xBBU, 0x2FU
};

/* AES-GCM expected Tag */
static u8 XAsu_AesGcmExpTag[XASU_AES_TAG_LEN_IN_BYTES] __attribute__ ((
		section (".data.XAsu_AesGcmExpTag"))) = {
	0xADU, 0xCEU, 0xFEU, 0x2FU, 0x6EU, 0xE4U, 0xC7U, 0x06U,
	0x0EU, 0x44U, 0xAAU, 0x5EU, 0xDFU, 0x0DU, 0xBEU, 0xBCU
};

/* AES Tag */
static u8 XAsu_AesTag[XASU_AES_TAG_LEN_IN_BYTES]
__attribute__ ((section (".data.XAsu_AesTag")));

/* AES Encrypted data */
static u8 XAsu_AesEncData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES]
__attribute__ ((section (".data.XAsu_AesEncData")));

/* AES Decrypted data */
static u8 XAsu_AesDecData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES]
__attribute__ ((section (".data.XAsu_AesDecData")));
#elif defined (__ICCARM__)
/* AES plaintext data */
static u8 XAsu_AesInputData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES] = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

/* AES AAD data */
static u8 XAsu_AesAad[XASU_AES_AAD_LEN_IN_BYTES] = {
	0x9AU, 0x7BU, 0x86U, 0xE7U, 0x82U, 0xCCU, 0xAAU, 0x6AU,
	0xB2U, 0x21U, 0xBDU, 0x03U, 0x47U, 0x0BU, 0xDCU, 0x2EU
};

/* AES key */
static u8 XAsu_AesKey[XASU_AES_KEY_LEN_IN_BYTES] = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};

/* AES IV */
static u8 XAsu_AesIv[XASU_AES_IV_LEN_IN_BYTES] = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U,
	0x62U, 0x98U, 0xDFU, 0xEAU, 0x5AU, 0xB2U
};

/* AES-GCM expected CipherText */
static u8 XAsu_AesGcmExpCt[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES] = {
	0x59U, 0x8CU, 0xD1U, 0x9FU, 0x16U, 0x83U, 0xB4U, 0x1BU,
	0x4CU, 0x59U, 0xE1U, 0xC1U, 0x57U, 0xD4U, 0x15U, 0x01U,
	0xA3U, 0xC0U, 0x89U, 0x02U, 0xF0U, 0xEAU, 0x3AU, 0x37U,
	0x6AU, 0x8BU, 0x0DU, 0x99U, 0x88U, 0xCFU, 0xF8U, 0xC1U
};

/* AES-CTR expected CipherText */
static u8 XAsu_AesCtrExpCt[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES] = {
	0x40U, 0x67U, 0x04U, 0x6BU, 0x20U, 0x74U, 0x83U, 0xC5U,
	0x86U, 0x7FU, 0x3DU, 0x1DU, 0xFDU, 0x6BU, 0x27U, 0xF3U,
	0x5CU, 0x07U, 0x75U, 0xCEU, 0xEEU, 0x85U, 0x6DU, 0xE7U,
	0x0EU, 0xD8U, 0x20U, 0xBAU, 0xC3U, 0x72U, 0xBBU, 0x2FU
};

/* AES-GCM Expected tag */
static u8 XAsu_AesGcmExpTag[XASU_AES_TAG_LEN_IN_BYTES] = {
	0xADU, 0xCEU, 0xFEU, 0x2FU, 0x6EU, 0xE4U, 0xC7U, 0x06U,
	0x0EU, 0x44U, 0xAAU, 0x5EU, 0xDFU, 0x0DU, 0xBEU, 0xBCU
};

/* AES tag */
static u8 XAsu_AesTag[XASU_AES_TAG_LEN_IN_BYTES];

/* AES Encrypted data */
static u8 XAsu_AesEncData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES];

/* AES Decrypted data */
static u8 XAsu_AesDecData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES];
#endif

static u8 Notify = 0; /**< To notify the call back from client library */

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This  main function to call the XAsu_AesExample function to perform AES encryption
 *		and decryption operation on given payload data.
 *
 * @return
 *		- Upon successful encryption and decryption operation, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;

	xil_printf("ASU AES Client Example: \r\n");

#ifdef XASU_DISABLE_CACHE
	Xil_DCacheDisable();
#endif

	/* Initialize the client instance */
	Status = XAsu_ClientInit(XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU Client initialize failed: %08x \r\n", Status);
		goto END;
	}

	/* AES-CTR example */
	Status = XAsu_AesCtrExample();
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n ASU AES-CTR client example failed \r\n");
	}

	/* AES-GCM example */
	Status = XAsu_AesGcmExample();
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n ASU AES-GCM client example failed \r\n");
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This  function performs AES encryption and decryption operation on a given data
 *		using AES-GCM engine mode.
 *
 * @return
 *		- Upon successful encryption and decryption operation, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static s32 XAsu_AesGcmExample(void)
{
	s32 Status = XST_FAILURE;
	XAsu_AesKeyObject AesKeyObj;
	XAsu_ClientParams AesClientParams;
	Asu_AesParams AesParams;

	/* Set Queue priority */
	AesClientParams.Priority = XASU_PRIORITY_HIGH;
	AesClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_AesCallBackRef);
	AesClientParams.CallBackRefPtr = (void *)&AesClientParams;

	/* AES parameters structure initialization for encryption */
	AesParams.EngineMode = XASU_AES_GCM_MODE;
	AesParams.OperationFlags = (XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL);
	AesParams.IsLast = TRUE;
	AesParams.OperationType = XASU_AES_ENCRYPT_OPERATION;

	AesKeyObj.KeyAddress = (u64)XAsu_AesKey;
	AesKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_0;
	AesParams.KeyObjectAddr = (u64)&AesKeyObj;
	AesParams.IvAddr = (u64)XAsu_AesIv;
	AesParams.IvLen = XASU_AES_IV_LEN_IN_BYTES;

	AesParams.InputDataAddr = (u64)XAsu_AesInputData;
	AesParams.OutputDataAddr = (u64)XAsu_AesEncData;
	AesParams.DataLen = XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES;
	AesParams.AadAddr = (u64)XAsu_AesAad;
	AesParams.AadLen = XASU_AES_AAD_LEN_IN_BYTES;

	AesParams.TagAddr = (u64)XAsu_AesTag;
	AesParams.TagLen = XASU_AES_TAG_LEN_IN_BYTES;

	Status = XAsu_AesEncrypt(&AesClientParams, &AesParams);
	if (Status != XST_SUCCESS) {
		xil_printf("AES-GCM Encryption failed: %08x \r\n", Status);
		goto END;
	}
	while(!Notify);
	Notify = 0;

	/* Comparison of encrypted Data with expected ciphertext data */
	Status = Xil_SMemCmp_CT(XAsu_AesGcmExpCt, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, XAsu_AesEncData,
				XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES-GCM Example Failed at Encrypted data Comparison \r\n");
		goto END;
	}

	/* Comparison of GCM tag with the expected GCM tag */
	Status = Xil_SMemCmp_CT(XAsu_AesGcmExpTag, XASU_AES_TAG_LEN_IN_BYTES, XAsu_AesTag,
				XASU_AES_TAG_LEN_IN_BYTES, XASU_AES_TAG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES-GCM Example Failed at Encrypted data Comparison \r\n");
		goto END;
	}

	xil_printf("AES-GCM Encrypted data: \n\r");
	XAsu_AesPrintData((const u8 *)XAsu_AesEncData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);

	xil_printf("AES-GCM tag: \n\r");
	XAsu_AesPrintData((const u8 *)XAsu_AesTag, XASU_AES_TAG_LEN_IN_BYTES);

	/* AES decryption */
	AesClientParams.Priority = XASU_PRIORITY_HIGH;
	AesClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_AesCallBackRef);
	AesClientParams.CallBackRefPtr = (void *)&AesClientParams;

	/* AES parameters structure initialization for decryption */
	AesParams.EngineMode = XASU_AES_GCM_MODE;
	AesParams.OperationFlags = (XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL);
	AesParams.IsLast = TRUE;
	AesParams.OperationType = XASU_AES_DECRYPT_OPERATION;

	AesKeyObj.KeyAddress = (u64)XAsu_AesKey;
	AesKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_0;
	AesParams.KeyObjectAddr = (u64)&AesKeyObj;
	AesParams.IvAddr = (u64)XAsu_AesIv;
	AesParams.IvLen = XASU_AES_IV_LEN_IN_BYTES;

	AesParams.InputDataAddr = (u64)XAsu_AesEncData;
	AesParams.OutputDataAddr = (u64)XAsu_AesDecData;
	AesParams.DataLen = XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES;
	AesParams.AadAddr = (u64)XAsu_AesAad;
	AesParams.AadLen = XASU_AES_AAD_LEN_IN_BYTES;

	AesParams.TagAddr = (u64)XAsu_AesTag;
	AesParams.TagLen = XASU_AES_TAG_LEN_IN_BYTES;

	Status = XAsu_AesDecrypt(&AesClientParams, &AesParams);
	if (Status != XST_SUCCESS) {
		xil_printf("AES-GCM Decryption failed: %08x \r\n", Status);
		goto END;
	}
	while(!Notify);
	Notify = 0;

	xil_printf("AES-GCM Decrypted data: \n\r");
	XAsu_AesPrintData((const u8 *)XAsu_AesDecData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);

	/* Comparison of decrypted Data with expected input data */
	Status = Xil_SMemCmp_CT(XAsu_AesInputData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, XAsu_AesDecData,
				XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES-GCM Example Failed at Decrypted data Comparison \r\n");
		goto END;
	}
	xil_printf("Successfully ran ASU AES-GCM Example\n\r");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This  function performs AES encryption and decryption operation on a given data
 *		using AES-CTR engine mode.
 *
 * @return
 *		- Upon successful encryption and decryption operation, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static s32 XAsu_AesCtrExample(void)
{
	s32 Status = XST_FAILURE;
	XAsu_AesKeyObject AesKeyObj;
	XAsu_ClientParams AesClientParams;
	Asu_AesParams AesParams;

	/* Set Queue priority */
	AesClientParams.Priority = XASU_PRIORITY_HIGH;
	AesClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_AesCallBackRef);
	AesClientParams.CallBackRefPtr = (void *)&AesClientParams;

	/* AES parameters structure initialization for encryption */
	AesParams.EngineMode = XASU_AES_CTR_MODE;
	AesParams.OperationFlags = (XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL);
	AesParams.IsLast = TRUE;
	AesParams.OperationType = XASU_AES_ENCRYPT_OPERATION;

	AesKeyObj.KeyAddress = (u64)XAsu_AesKey;
	AesKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_0;
	AesParams.KeyObjectAddr = (u64)&AesKeyObj;
	AesParams.IvAddr = (u64)XAsu_AesIv;
	AesParams.IvLen = XASU_AES_IV_LEN_IN_BYTES;

	AesParams.InputDataAddr = (u64)XAsu_AesInputData;
	AesParams.OutputDataAddr = (u64)XAsu_AesEncData;
	AesParams.DataLen = XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES;

	/* No AAD and Tag for AES standard modes.(ECB, CBC, CFB, OFB, CTR). */
	AesParams.AadAddr = 0U;
	AesParams.AadLen = 0U;
	AesParams.TagAddr = 0U;
	AesParams.TagLen = 0U;

	Status = XAsu_AesEncrypt(&AesClientParams, &AesParams);
	if (Status != XST_SUCCESS) {
		xil_printf("AES-CTR Encryption failed: %08x \r\n", Status);
		goto END;
	}
	while(!Notify);
	Notify = 0;

	/* Comparison of encrypted Data with expected ciphertext data */
	Status = Xil_SMemCmp_CT(XAsu_AesCtrExpCt, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, XAsu_AesEncData,
				XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES-CTR Example Failed at Encrypted data Comparison \r\n");
		goto END;
	}

	xil_printf("AES-CTR Encrypted data: \n\r");
	XAsu_AesPrintData((const u8 *)XAsu_AesEncData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);

	AesClientParams.Priority = XASU_PRIORITY_HIGH;
	AesClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_AesCallBackRef);
	AesClientParams.CallBackRefPtr = (void *)&AesClientParams;
	/* AES parameters structure initialization for decryption */
	AesParams.EngineMode = XASU_AES_CTR_MODE;
	AesParams.OperationFlags = (XASU_AES_INIT | XASU_AES_UPDATE | XASU_AES_FINAL);
	AesParams.IsLast = TRUE;
	AesParams.OperationType = XASU_AES_DECRYPT_OPERATION;

	AesKeyObj.KeyAddress = (u64)XAsu_AesKey;
	AesKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_0;
	AesParams.KeyObjectAddr = (u64)&AesKeyObj;
	AesParams.IvAddr = (u64)XAsu_AesIv;
	AesParams.IvLen = XASU_AES_IV_LEN_IN_BYTES;

	AesParams.InputDataAddr = (u64)XAsu_AesEncData;
	AesParams.OutputDataAddr = (u64)XAsu_AesDecData;
	AesParams.DataLen = XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES;

	Status = XAsu_AesDecrypt(&AesClientParams, &AesParams);
	if (Status != XST_SUCCESS) {
		xil_printf("AES-CTR Decryption failed: %08x \r\n", Status);
		goto END;
	}

	while(!Notify);
	Notify = 0;

	xil_printf("Decrypted data: \n\r");
	XAsu_AesPrintData((const u8 *)XAsu_AesDecData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);

	/* Comparison of decrypted Data with expected input data */
	Status = Xil_SMemCmp_CT(XAsu_AesInputData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, XAsu_AesDecData,
				XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("ASU AES-CTR Example Failed at Decrypted data Comparison \r\n");
		goto END;
	}
	xil_printf("Successfully ran ASU AES-CTR Example\n\r");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the data from given array on the console
 *
 * @param	Data	Pointer to data array
 * @param	DataLen	Length of the data to be printed on console
 *
 *************************************************************************************************/
static void XAsu_AesPrintData(const u8 *Data, u32 DataLen)
{
	u32 Index;

	for (Index = 0U; Index < DataLen; Index++) {
		xil_printf("%02x ", Data[Index]);
	}
	xil_printf(" \r\n ");
}

/*************************************************************************************************/
/**
 * @brief	Call back function which will be registered with library to notify the completion
 *		of request
 *
 * @param	CallBackRef	Pointer to the call back reference.
 * @param	Status		Status of the request will be passed as an argument during callback
 * 			- 0 Upon success
 * 			- Error code from ASUFW application upon any error
 *
 *************************************************************************************************/
  static void XAsu_AesCallBackRef(void *CallBackRef, u32 Status)
 {
	(void)CallBackRef;

	xil_printf("Recieved response from library with Status = %x\n\r", Status);
	/* Update the variable to notify the callback */
	Notify = 1U;

 }
/** @} */