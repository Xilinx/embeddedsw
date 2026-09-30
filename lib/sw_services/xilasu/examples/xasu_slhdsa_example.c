/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_slhdsa_example.c
* @addtogroup xasu_slhdsa_client_example XilAsu SLH-DSA API Example Generic Usage
* @{
* @note
* This example demonstrates SLH-DSA key generation, message signing, and
* signature verification using the xilasu client APIs over IPI to ASUFW.
*
* The flow is exercised for all four supported FIPS 205 parameter sets:
* SHAKE-256s, SHAKE-128s, SHA2-256s, and SHA2-128s.
*
* For each parameter set the example:
* 1. Generates an SLH-DSA key pair.
* 2. Signs a test message with the generated secret key.
* 3. Verifies the signature with the generated public key.
*
* SHAKE-128s can also run three extra sign/verify sequences on the same key pair:
* a non-empty FIPS context, HashSLH-DSA (SHA-256 PH(M)), and deterministic
* signing. SHA2-256s can additionally run HashSLH-DSA with SHA-512 PH(M).
* SHAKE-256s can queue an AES-GCM encrypt immediately after the default sign so that
* ASUFW can pick AES on a yield slice and finish AES before sign completes.
* Those extra paths are off by default; set the matching XASU_SLHDSA_EXAMPLE_*_ENABLE
* macros to 1U to turn them on.
*
* Client buffers passed to ASUFW must reside in memory visible to both client and server.
* See the linker-script notes in xasu_sha2_example.c for OCM/shared-memory placement.
*
* IPI command flow demonstrated:
* 1. XAsu_ClientInit initializes the mailbox and client layer.
* 2. For each parameter set, XAsu_KmGenerateSlhdsaKeyPair sends keygen parameters
*    (SkOutAddr, SkOutLen, PkOutAddr, PkOutLen, ParamSet) to the key manager and waits
*    for the IPI callback.
* 3. XAsu_SlhdsaSign sends sign parameters (message, secret key, signature output,
*    context. When AES overlap is enabled, SHAKE-256s immediately queues an
*    AES-GCM encrypt on the same high-priority queue so ASUFW can pick AES during
*    a sign yield slice.
* 4. XAsu_SlhdsaSignVerify sends verify parameters with matching CtxLen and ParamSet.
* 5. When extra modes are enabled, SHAKE-128s runs three further sign/verify
*    sequences on the same key pair: non-empty context, HashSLH-DSA SHA-256, and
*    deterministic signing.
* 6. When SHA-512 pre-hash is enabled, SHA2-256s signs and verifies HashSLH-DSA
*    with SHA-512 PH(M).
*
* DMA buffer requirements:
* - Secret key, public key, message, and signature buffers are placed in dedicated
*   linker sections (.data.XAsu_Slhdsa*) for shared-memory visibility.
* - Buffer sizes use XASU_SLHDSA_MAX_* constants; per-variant sizes come from
*   XAsu_SlhdsaGetSkSize, XAsu_SlhdsaGetPkSize, and XAsu_SlhdsaGetSigSize.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   kk     07/08/26 Initial Release
* 1.1   kk     09/17/26 Optional extra-mode, SHA-512 pre-hash and AES overlap
*                       coverage, disabled by default
*
* </pre>
**************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_slhdsa.h"
#include "xasu_slhdsa_common.h"
#include "xasu_keymanager.h"
#include "xasu_sha2.h"
#include "xasu_aes.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_CACHE_DISABLE	/**< Disables cache maintenance calls in this example. */
#define XASU_SLHDSA_EXAMPLE_VARIANT_COUNT	(4U)
			/**< Number of FIPS 205 parameter sets exercised by this example. */
#define XASU_SLHDSA_EXAMPLE_MAX_SIG_SIZE	XASU_SLHDSA_SHA2_256S_SIG_SIZE
			/**< Largest signature size across the exercised parameter sets. */
#define XASU_SLHDSA_EXAMPLE_MSG_SIZE		((u32)sizeof(XAsu_SlhdsaMessage))
			/**< Test message size in bytes. */
#define XASU_SLHDSA_EXAMPLE_EXTRA_PARAM_SET	XASU_SLHDSA_PARAM_SHAKE_128S
			/**< Parameter set used for context, HashSLH-DSA SHA-256 and
			 *   deterministic coverage. */
#define XASU_SLHDSA_EXAMPLE_SHA512_PARAM_SET	XASU_SLHDSA_PARAM_SHA2_256S
			/**< Parameter set used for HashSLH-DSA SHA-512 coverage. */
#define XASU_SLHDSA_EXAMPLE_CTX_SIZE		(11U)
			/**< FIPS context length used by the extra SHAKE-128s sequences. */
#define XASU_SLHDSA_EXAMPLE_AES_OVERLAP_PARAM_SET	XASU_SLHDSA_PARAM_SHAKE_256S
			/**< Parameter set used to overlap AES with a yielding sign. */
#define XASU_SLHDSA_EXAMPLE_EXTRA_MODES_ENABLE	(0U)
			/**< Set to 1U to run context, HashSLH-DSA SHA-256 and deterministic
			 *   sign on SHAKE-128s. */
#define XASU_SLHDSA_EXAMPLE_SHA512_PREHASH_ENABLE	(0U)
			/**< Set to 1U to run HashSLH-DSA SHA-512 on SHA2-256s. */
#define XASU_SLHDSA_EXAMPLE_AES_OVERLAP_ENABLE	(0U)
			/**< Set to 1U to queue AES-GCM during SHAKE-256s sign. */
#define XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN	(32U)
			/**< AES-GCM payload length used by the yield-overlap encrypt. */
#define XASU_SLHDSA_EXAMPLE_AES_AAD_LEN		(16U)
			/**< AES-GCM AAD length used by the yield-overlap encrypt. */
#define XASU_SLHDSA_EXAMPLE_AES_KEY_LEN		(32U)
			/**< AES-256 key length used by the yield-overlap encrypt. */
#define XASU_SLHDSA_EXAMPLE_AES_IV_LEN		(12U)
			/**< AES-GCM IV length used by the yield-overlap encrypt. */
#define XASU_SLHDSA_EXAMPLE_AES_TAG_LEN		(16U)
			/**< AES-GCM tag length used by the yield-overlap encrypt. */

/************************************ Function Prototypes ****************************************/
static void XAsu_SlhdsaCallBackRef(void *CallBackRef, u32 Status);
static void XAsu_SlhdsaAesCallBackRef(void *CallBackRef, u32 Status);
static s32 XAsu_SlhdsaWaitForCompletion(void);
static s32 XAsu_SlhdsaSubmitAesGcmEncrypt(void);
static s32 XAsu_SlhdsaCheckAesGcmEncrypt(void);
static s32 XAsu_SlhdsaKeyGenExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaSignExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaSignVerifyExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaRunVariantExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaRunExtraModesExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaRunSha512PreHashExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaSha2Digest(XAsu_ClientParams *ClientParamsPtr, u8 ShaMode, u32 HashLen);
static s32 XAsu_SlhdsaSignOnce(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet,
			      u64 MsgAddr, u32 MsgLen, u64 CtxAddr, u32 CtxLen,
			      u8 PreHashMode, u8 HashId, u8 SignMode, u8 OverlapAes);
static s32 XAsu_SlhdsaVerifyOnce(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet,
				u64 MsgAddr, u32 MsgLen, u64 CtxAddr, u32 CtxLen,
				u8 PreHashMode, u8 HashId);
static const char *XAsu_SlhdsaGetParamSetName(u8 ParamSet);
static s32 XAsu_SlhdsaExample(void);

/************************************ Variable Definitions ***************************************/
static const u8 XAsu_SlhdsaMessage[] __attribute__ ((
		section (".data.XAsu_SlhdsaMessage"))) = {
	'S', 'L', 'H', '-', 'D', 'S', 'A', ' ',
	'A', 'S', 'U', ' ',
	'c', 'l', 'i', 'e', 'n', 't', ' ',
	'e', 'x', 'a', 'm', 'p', 'l', 'e'
};
static const u8 XAsu_SlhdsaContext[] __attribute__ ((
		section (".data.XAsu_SlhdsaContext"))) = {
	'S', 'L', 'H', '-', 'D', 'S', 'A', '-', 'C', 'T', 'X'
};
_Static_assert(sizeof(XAsu_SlhdsaContext) == XASU_SLHDSA_EXAMPLE_CTX_SIZE,
	       "SLH-DSA example context length must match XASU_SLHDSA_EXAMPLE_CTX_SIZE");
static const u8 XAsu_SlhdsaExampleParamSets[XASU_SLHDSA_EXAMPLE_VARIANT_COUNT] = {
	XASU_SLHDSA_PARAM_SHAKE_256S,
	XASU_SLHDSA_PARAM_SHAKE_128S,
	XASU_SLHDSA_PARAM_SHA2_256S,
	XASU_SLHDSA_PARAM_SHA2_128S,
};
static u8 XAsu_SlhdsaSecretKey[XASU_SLHDSA_MAX_SK_SIZE] __attribute__ ((
		section (".data.XAsu_SlhdsaSecretKey")));
static u8 XAsu_SlhdsaPublicKey[XASU_SLHDSA_MAX_PK_SIZE] __attribute__ ((
		section (".data.XAsu_SlhdsaPublicKey")));
static u8 XAsu_SlhdsaSignature[XASU_SLHDSA_EXAMPLE_MAX_SIG_SIZE] __attribute__ ((
		section (".data.XAsu_SlhdsaSignature")));
static u8 XAsu_SlhdsaPreHash[XASU_SLHDSA_PREHASH_SHA512_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaPreHash")));
static u8 XAsu_SlhdsaAesInput[XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaAesInput"))) = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};
static u8 XAsu_SlhdsaAesAad[XASU_SLHDSA_EXAMPLE_AES_AAD_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaAesAad"))) = {
	0x9AU, 0x7BU, 0x86U, 0xE7U, 0x82U, 0xCCU, 0xAAU, 0x6AU,
	0xB2U, 0x21U, 0xBDU, 0x03U, 0x47U, 0x0BU, 0xDCU, 0x2EU
};
static u8 XAsu_SlhdsaAesKey[XASU_SLHDSA_EXAMPLE_AES_KEY_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaAesKey"))) = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};
static u8 XAsu_SlhdsaAesIv[XASU_SLHDSA_EXAMPLE_AES_IV_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaAesIv"))) = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U,
	0x62U, 0x98U, 0xDFU, 0xEAU, 0x5AU, 0xB2U
};
static u8 XAsu_SlhdsaAesExpCt[XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaAesExpCt"))) = {
	0x59U, 0x8CU, 0xD1U, 0x9FU, 0x16U, 0x83U, 0xB4U, 0x1BU,
	0x4CU, 0x59U, 0xE1U, 0xC1U, 0x57U, 0xD4U, 0x15U, 0x01U,
	0xA3U, 0xC0U, 0x89U, 0x02U, 0xF0U, 0xEAU, 0x3AU, 0x37U,
	0x6AU, 0x8BU, 0x0DU, 0x99U, 0x88U, 0xCFU, 0xF8U, 0xC1U
};
static u8 XAsu_SlhdsaAesExpTag[XASU_SLHDSA_EXAMPLE_AES_TAG_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaAesExpTag"))) = {
	0xADU, 0xCEU, 0xFEU, 0x2FU, 0x6EU, 0xE4U, 0xC7U, 0x06U,
	0x0EU, 0x44U, 0xAAU, 0x5EU, 0xDFU, 0x0DU, 0xBEU, 0xBCU
};
static u8 XAsu_SlhdsaAesEncData[XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaAesEncData")));
static u8 XAsu_SlhdsaAesTag[XASU_SLHDSA_EXAMPLE_AES_TAG_LEN] __attribute__ ((
		section (".data.XAsu_SlhdsaAesTag")));
static XAsu_AesKeyObject XAsu_SlhdsaAesKeyObj __attribute__ ((
		section (".data.XAsu_SlhdsaAesKeyObj")));
static XAsu_ClientParams XAsu_SlhdsaAesClientParams;
static volatile u8 Notify = 0U;
static volatile u32 ErrorStatus = XST_FAILURE;
static volatile u8 AesNotify = 0U;
static volatile u32 AesErrorStatus = XST_FAILURE;
static volatile u32 CompletionSeq = 0U;
static volatile u32 AesSeq = 0U;
static volatile u32 SignSeq = 0U;
static volatile u8 TrackCompletionOrder = 0U;

/*************************************************************************************************/
/**
* @brief	Main function to call the SLH-DSA client example.
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

	/** Disable the data cache so shared buffers need no explicit flush or invalidate. */
#ifdef XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	/** Initialize the mailbox instance used to carry IPI requests to ASU. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x \r\n", Status);
		goto END;
	}

	/** Initialize the client layer, which must precede any SLH-DSA client API call. */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	xil_printf("\r\n--- SLH-DSA Client Example ---\r\n");

	Status = XAsu_SlhdsaExample();
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SLH-DSA client example failed with Status = %08x", Status);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Runs SLH-DSA key generation, sign, and verify for all parameter sets.
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaExample(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	u32 VariantIdx;

	/**
	 * Set up client parameters shared by every request. XASU_CMD_SECURE is required because
	 * ASUFW registers keygen and sign as secure-access-only commands.
	 */
	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_SlhdsaCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;
	ClientParams.AdditionalStatus = XST_FAILURE;

	/**
	 * Run the full keygen, sign and verify sequence for each supported parameter set,
	 * stopping at the first failure.
	 */
	for (VariantIdx = 0U; VariantIdx < XASU_SLHDSA_EXAMPLE_VARIANT_COUNT; VariantIdx++) {
		Status = XAsu_SlhdsaRunVariantExample(&ClientParams,
						      XAsu_SlhdsaExampleParamSets[VariantIdx]);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** Invalidate the key buffers so the final values written by ASUFW are visible. */
#ifndef XASU_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaSecretKey, XASU_SLHDSA_MAX_SK_SIZE);
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaPublicKey, XASU_SLHDSA_MAX_PK_SIZE);
#endif

	xil_printf("\r\n Successfully ran SLH-DSA client example for all parameter sets");

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Returns a human-readable name for an SLH-DSA parameter set selector.
*
* @param	ParamSet	SLH-DSA parameter set selector (XASU_SLHDSA_PARAM_*).
*
* @return
*		Pointer to a static variant name string.
*
*************************************************************************************************/
static const char *XAsu_SlhdsaGetParamSetName(u8 ParamSet)
{
	const char *Name = "Unknown";

	switch (ParamSet) {
		case XASU_SLHDSA_PARAM_SHAKE_256S: {
			Name = "SHAKE-256s";
			break;
		}
		case XASU_SLHDSA_PARAM_SHAKE_128S: {
			Name = "SHAKE-128s";
			break;
		}
		case XASU_SLHDSA_PARAM_SHA2_256S: {
			Name = "SHA2-256s";
			break;
		}
		case XASU_SLHDSA_PARAM_SHA2_128S: {
			Name = "SHA2-128s";
			break;
		}
		default: {
			break;
		}
	}

	return Name;
}

/*************************************************************************************************/
/**
* @brief	Runs keygen, sign, and verify for one SLH-DSA parameter set.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ParamSet	SLH-DSA parameter set selector.
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaRunVariantExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet)
{
	s32 Status = XST_FAILURE;
	const char *VariantName = XAsu_SlhdsaGetParamSetName(ParamSet);
#ifndef XASU_CACHE_DISABLE
	u32 SkSize;
	u32 PkSize;
	u32 SigSize;
#endif

	/** Validate input pointer. */
	if (ClientParamsPtr == NULL) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	xil_printf("\r\n--- SLH-DSA Variant: %s ---\r\n", VariantName);

	/**
	 * Step 1: generate a key pair into the shared buffers consumed by the sign and verify
	 * stages below, so this stage must run first.
	 */
	xil_printf("\r\n SLH-DSA Key Generation started (%s)\r\n", VariantName);
	Status = XAsu_SlhdsaKeyGenExample(ClientParamsPtr, ParamSet);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SLH-DSA Key Generation failed (%s)\r\n", VariantName);
		goto END;
	}
	xil_printf("\r\n SLH-DSA Key Generation completed (%s)\r\n", VariantName);

	/**
	 * Invalidate the key buffers, which ASUFW wrote by DMA, over the per-variant sizes
	 * rather than the maximum buffer sizes.
	 */
#ifndef XASU_CACHE_DISABLE
	SkSize = XAsu_SlhdsaGetSkSize(ParamSet);
	PkSize = XAsu_SlhdsaGetPkSize(ParamSet);
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaSecretKey, SkSize);
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaPublicKey, PkSize);
#endif

	/** Step 2: sign the test message with the secret key just generated. */
	xil_printf("\r\n SLH-DSA Sign started (%s)\r\n", VariantName);
	Status = XAsu_SlhdsaSignExample(ClientParamsPtr, ParamSet);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SLH-DSA Sign failed (%s)\r\n", VariantName);
		goto END;
	}
	xil_printf("\r\n SLH-DSA Sign completed (%s)\r\n", VariantName);

	/** Invalidate the signature buffer before it is passed back for verification. */
#ifndef XASU_CACHE_DISABLE
	SigSize = XAsu_SlhdsaGetSigSize(ParamSet);
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaSignature, SigSize);
#endif

	/** Step 3: verify the signature with the matching public key. */
	xil_printf("\r\n SLH-DSA Verify started (%s)\r\n", VariantName);
	Status = XAsu_SlhdsaSignVerifyExample(ClientParamsPtr, ParamSet);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SLH-DSA Verify failed (%s)\r\n", VariantName);
		goto END;
	}
	xil_printf("\r\n SLH-DSA Verify completed (%s)\r\n", VariantName);

	/**
	 * SHAKE-128s is the cheapest extra sign, so the remaining FIPS modes (non-empty
	 * context, HashSLH-DSA and deterministic signing) are exercised on this key pair only
	 * when extra-mode coverage is enabled.
	 */
	if ((XASU_SLHDSA_EXAMPLE_EXTRA_MODES_ENABLE != 0U) &&
	    (ParamSet == XASU_SLHDSA_EXAMPLE_EXTRA_PARAM_SET)) {
		Status = XAsu_SlhdsaRunExtraModesExample(ClientParamsPtr, ParamSet);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/**
	 * SHA2-256s requires SHA-512 as PH(M) (FIPS 205 footnote 18, n = 32), so HashSLH-DSA
	 * SHA-512 is exercised on this key pair only when that coverage is enabled.
	 */
	if ((XASU_SLHDSA_EXAMPLE_SHA512_PREHASH_ENABLE != 0U) &&
	    (ParamSet == XASU_SLHDSA_EXAMPLE_SHA512_PARAM_SET)) {
		Status = XAsu_SlhdsaRunSha512PreHashExample(ClientParamsPtr, ParamSet);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Generates an SLH-DSA key pair using ASUFW.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ParamSet	SLH-DSA parameter set selector.
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaKeyGenExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet)
{
	s32 Status = XST_FAILURE;
	XAsu_SlhdsaKeyGenParams KeyGenParams = {0U};

	/**
	 * Set up key generation parameters. ErrorStatus is reset first so the value read after
	 * the wait can only have come from this request's callback.
	 */
	ErrorStatus = XST_FAILURE;
	KeyGenParams.SkOutAddr = (u64)(UINTPTR)XAsu_SlhdsaSecretKey;
	KeyGenParams.SkOutLen = XAsu_SlhdsaGetSkSize(ParamSet);
	KeyGenParams.PkOutAddr = (u64)(UINTPTR)XAsu_SlhdsaPublicKey;
	KeyGenParams.PkOutLen = XAsu_SlhdsaGetPkSize(ParamSet);
	KeyGenParams.ParamSet = ParamSet;

	/** Send the key generation request to the ASUFW key manager. */
	Status = XAsu_KmGenerateSlhdsaKeyPair(ClientParamsPtr, &KeyGenParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n SLH-DSA key generation request failed with Status = %08x",
			      Status);
		goto END;
	}

	/**
	 * Wait for the callback and return the server status. A successful send only means the
	 * request was queued, so the outcome is only known here.
	 */
	Status = XAsu_SlhdsaWaitForCompletion();

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Signs a test message using the generated SLH-DSA secret key.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ParamSet	SLH-DSA parameter set selector.
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaSignExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet)
{
	s32 Status = XST_FAILURE;

	/**
	 * Sign the test message as pure SLH-DSA with an empty context and hedged add_rand.
	 * This is the FIPS 205 default path and is run for every parameter set.
	 */
	Status = XAsu_SlhdsaSignOnce(ClientParamsPtr, ParamSet,
				     (u64)(UINTPTR)XAsu_SlhdsaMessage,
				     XASU_SLHDSA_EXAMPLE_MSG_SIZE, 0U, 0U,
				     XASU_SLHDSA_MSG_PURE, 0U,
				     XASU_SLHDSA_SIGN_MODE_HEDGED,
				     ((XASU_SLHDSA_EXAMPLE_AES_OVERLAP_ENABLE != 0U) &&
				      (ParamSet == XASU_SLHDSA_EXAMPLE_AES_OVERLAP_PARAM_SET)) ?
				     1U : 0U);

	return Status;
}

/*************************************************************************************************/
/**
* @brief	Verifies the SLH-DSA signature for the test message.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ParamSet	SLH-DSA parameter set selector.
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaSignVerifyExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet)
{
	s32 Status = XST_FAILURE;

	/**
	 * Verify with the same empty context and pure message mode used at signing time.
	 */
	Status = XAsu_SlhdsaVerifyOnce(ClientParamsPtr, ParamSet,
				       (u64)(UINTPTR)XAsu_SlhdsaMessage,
				       XASU_SLHDSA_EXAMPLE_MSG_SIZE, 0U, 0U,
				       XASU_SLHDSA_MSG_PURE, 0U);

	return Status;
}

/*************************************************************************************************/
/**
* @brief	Runs the extra FIPS 205 modes on one SHAKE-128s key pair.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ParamSet	SLH-DSA parameter set selector (SHAKE-128s).
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaRunExtraModesExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet)
{
	s32 Status = XST_FAILURE;
	u64 MsgAddr = (u64)(UINTPTR)XAsu_SlhdsaMessage;
	u64 CtxAddr = (u64)(UINTPTR)XAsu_SlhdsaContext;
	u64 DigestAddr = (u64)(UINTPTR)XAsu_SlhdsaPreHash;

	/** Validate input pointer. */
	if (ClientParamsPtr == NULL) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Stage 1: pure SLH-DSA with a non-empty context. Sign and verify must use the same
	 * CtxAddr and CtxLen, because the context is bound into the message digest.
	 */
	xil_printf("\r\n SLH-DSA extra: pure sign with context (SHAKE-128s)\r\n");
	Status = XAsu_SlhdsaSignOnce(ClientParamsPtr, ParamSet, MsgAddr,
				     XASU_SLHDSA_EXAMPLE_MSG_SIZE, CtxAddr,
				     XASU_SLHDSA_EXAMPLE_CTX_SIZE,
				     XASU_SLHDSA_MSG_PURE, 0U,
				     XASU_SLHDSA_SIGN_MODE_HEDGED, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XAsu_SlhdsaVerifyOnce(ClientParamsPtr, ParamSet, MsgAddr,
				       XASU_SLHDSA_EXAMPLE_MSG_SIZE, CtxAddr,
				       XASU_SLHDSA_EXAMPLE_CTX_SIZE,
				       XASU_SLHDSA_MSG_PURE, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\r\n SLH-DSA extra: pure context sign/verify completed\r\n");

	/**
	 * Stage 2: HashSLH-DSA. PH(M) is SHA-256 of the test message and is computed on the
	 * client; ASUFW hashes 0x01 || ctx_len || ctx || OID || PH(M).
	 */
	xil_printf("\r\n SLH-DSA extra: HashSLH-DSA SHA-256 (SHAKE-128s)\r\n");
	Status = XAsu_SlhdsaSha2Digest(ClientParamsPtr, XASU_SHA_MODE_256,
				      XASU_SLHDSA_PREHASH_SHA256_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XAsu_SlhdsaSignOnce(ClientParamsPtr, ParamSet, DigestAddr,
				     XASU_SLHDSA_PREHASH_SHA256_LEN, CtxAddr,
				     XASU_SLHDSA_EXAMPLE_CTX_SIZE,
				     XASU_SLHDSA_MSG_PREHASH,
				     XASU_SLHDSA_HASHID_SHA256,
				     XASU_SLHDSA_SIGN_MODE_HEDGED, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XAsu_SlhdsaVerifyOnce(ClientParamsPtr, ParamSet, DigestAddr,
				       XASU_SLHDSA_PREHASH_SHA256_LEN, CtxAddr,
				       XASU_SLHDSA_EXAMPLE_CTX_SIZE,
				       XASU_SLHDSA_MSG_PREHASH,
				       XASU_SLHDSA_HASHID_SHA256);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\r\n SLH-DSA extra: HashSLH-DSA sign/verify completed\r\n");

	/**
	 * Stage 3: deterministic signing. add_rand is derived from PK.seed rather than the
	 * TRNG, with the default empty context so this path is distinct from stage 1.
	 */
	xil_printf("\r\n SLH-DSA extra: deterministic sign (SHAKE-128s)\r\n");
	Status = XAsu_SlhdsaSignOnce(ClientParamsPtr, ParamSet, MsgAddr,
				     XASU_SLHDSA_EXAMPLE_MSG_SIZE, 0U, 0U,
				     XASU_SLHDSA_MSG_PURE, 0U,
				     XASU_SLHDSA_SIGN_MODE_DETERMINISTIC, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XAsu_SlhdsaVerifyOnce(ClientParamsPtr, ParamSet, MsgAddr,
				       XASU_SLHDSA_EXAMPLE_MSG_SIZE, 0U, 0U,
				       XASU_SLHDSA_MSG_PURE, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\r\n SLH-DSA extra: deterministic sign/verify completed\r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Runs HashSLH-DSA with SHA-512 PH(M) on one SHA2-256s key pair.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ParamSet	SLH-DSA parameter set selector (SHA2-256s).
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaRunSha512PreHashExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet)
{
	s32 Status = XST_FAILURE;
	u64 DigestAddr = (u64)(UINTPTR)XAsu_SlhdsaPreHash;

	/** Validate input pointer. */
	if (ClientParamsPtr == NULL) {
		Status = (s32)XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * PH(M) is SHA-512 of the test message. HashId must be SHA-512 for 256s sets so that
	 * PH(M) is at least 2n bytes. The empty context keeps this path distinct from the
	 * SHAKE-128s HashSLH-DSA sequence, which already used a non-empty context.
	 */
	xil_printf("\r\n SLH-DSA extra: HashSLH-DSA SHA-512 (SHA2-256s)\r\n");
	Status = XAsu_SlhdsaSha2Digest(ClientParamsPtr, XASU_SHA_MODE_512,
				      XASU_SLHDSA_PREHASH_SHA512_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XAsu_SlhdsaSignOnce(ClientParamsPtr, ParamSet, DigestAddr,
				     XASU_SLHDSA_PREHASH_SHA512_LEN, 0U, 0U,
				     XASU_SLHDSA_MSG_PREHASH,
				     XASU_SLHDSA_HASHID_SHA512,
				     XASU_SLHDSA_SIGN_MODE_HEDGED, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XAsu_SlhdsaVerifyOnce(ClientParamsPtr, ParamSet, DigestAddr,
				       XASU_SLHDSA_PREHASH_SHA512_LEN, 0U, 0U,
				       XASU_SLHDSA_MSG_PREHASH,
				       XASU_SLHDSA_HASHID_SHA512);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\r\n SLH-DSA extra: HashSLH-DSA SHA-512 sign/verify completed\r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Computes SHA-256 or SHA-512 PH(M) of the test message for HashSLH-DSA.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ShaMode		XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	HashLen		Digest length in bytes (32 or 64).
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaSha2Digest(XAsu_ClientParams *ClientParamsPtr, u8 ShaMode, u32 HashLen)
{
	s32 Status = XST_FAILURE;
	XAsu_ShaOperationCmd ShaParams = {0U};

	/** Reset the callback status so the wait observes this SHA2 request only. */
	ErrorStatus = XST_FAILURE;
#ifndef XASU_CACHE_DISABLE
	Xil_DCacheFlushRange((UINTPTR)XAsu_SlhdsaMessage, XASU_SLHDSA_EXAMPLE_MSG_SIZE);
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaPreHash, HashLen);
#endif
	ShaParams.DataAddr = (u64)(UINTPTR)XAsu_SlhdsaMessage;
	ShaParams.HashAddr = (u64)(UINTPTR)XAsu_SlhdsaPreHash;
	ShaParams.DataSize = XASU_SLHDSA_EXAMPLE_MSG_SIZE;
	ShaParams.HashBufSize = HashLen;
	ShaParams.ShaMode = ShaMode;
	ShaParams.IsLast = TRUE;
	ShaParams.OperationFlags = (XASU_INIT | XASU_UPDATE | XASU_FINISH);

	/** Hash the test message; the digest is the HashSLH-DSA PH(M). */
	Status = XAsu_Sha2Operation(ClientParamsPtr, &ShaParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n SLH-DSA SHA2 digest request failed with Status = %08x",
			      Status);
		goto END;
	}

	Status = XAsu_SlhdsaWaitForCompletion();
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaPreHash, HashLen);
#endif

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Sends one SLH-DSA sign request and waits for completion.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ParamSet	SLH-DSA parameter set selector.
* @param	MsgAddr		Address of the message or PH(M) digest.
* @param	MsgLen		Length of the message or digest in bytes.
* @param	CtxAddr		Address of the FIPS context string, or 0 for empty context.
* @param	CtxLen		Context length in bytes (0..255).
* @param	PreHashMode	XASU_SLHDSA_MSG_PURE or XASU_SLHDSA_MSG_PREHASH.
* @param	HashId		Pre-hash HashId; unused in pure mode.
* @param	SignMode	Hedged or deterministic signing.
* @param	OverlapAes	When non-zero, queue AES-GCM encrypt immediately after sign.
*
* @return
*		- XST_SUCCESS on success.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaSignOnce(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet,
			      u64 MsgAddr, u32 MsgLen, u64 CtxAddr, u32 CtxLen,
			      u8 PreHashMode, u8 HashId, u8 SignMode, u8 OverlapAes)
{
	s32 Status = XST_FAILURE;
	s32 AesStatus = XST_FAILURE;
	XAsu_SlhdsaSignParams SignParams = {0U};
	u32 SigSize = XAsu_SlhdsaGetSigSize(ParamSet);
	u32 SkSize = XAsu_SlhdsaGetSkSize(ParamSet);

	/**
	 * Fill sign parameters. SkId stays zero because the secret key is passed by address.
	 * SignatureLen and SkLen must match the parameter set or validation rejects the request.
	 */
	ErrorStatus = XST_FAILURE;
	AesNotify = 0U;
	AesErrorStatus = XST_FAILURE;
	CompletionSeq = 0U;
	AesSeq = 0U;
	SignSeq = 0U;
	TrackCompletionOrder = OverlapAes;
	SignParams.SkObj.SkAddr = (u64)(UINTPTR)XAsu_SlhdsaSecretKey;
	SignParams.SkObj.SkId = 0U;
	SignParams.SkObj.SkLen = SkSize;
	SignParams.MsgAddr = MsgAddr;
	SignParams.SignatureAddr = (u64)(UINTPTR)XAsu_SlhdsaSignature;
	SignParams.MsgLen = MsgLen;
	SignParams.SignatureLen = SigSize;
	SignParams.CtxAddr = CtxAddr;
	SignParams.CtxLen = CtxLen;
	SignParams.ParamSet = ParamSet;
	SignParams.PreHashMode = PreHashMode;
	SignParams.HashId = HashId;
	SignParams.SignMode = SignMode;

	/** Send the sign request to ASUFW. */
	Status = XAsu_SlhdsaSign(ClientParamsPtr, &SignParams);
	if (Status != XST_SUCCESS) {
		TrackCompletionOrder = 0U;
		XilAsu_Printf("\r\n SLH-DSA sign request failed with Status = %08x", Status);
		goto END;
	}

	/**
	 * Queue AES immediately so the server can pick it on the next yield slice. AES uses a
	 * second unique ID and callback; it must be HIGH priority so it shares the P0 queue.
	 */
	if (OverlapAes != 0U) {
		xil_printf("\r\n AES GCM encrypt queued immediately after SLH-DSA sign\r\n");
		AesStatus = XAsu_SlhdsaSubmitAesGcmEncrypt();
		if (AesStatus != XST_SUCCESS) {
			XilAsu_Printf("\r\n AES overlap request failed with Status = %08x",
				      AesStatus);
		}
	}

	/**
	 * Wait for the callback. Signing takes seconds to tens of seconds depending on the
	 * parameter set, so this busy-wait is long lived by design. When AES was queued,
	 * wait for both completions so their order can be compared.
	 */
	if ((OverlapAes != 0U) && (AesStatus == XST_SUCCESS)) {
		while ((Notify == 0U) || (AesNotify == 0U)) {
			;
		}
	} else {
		while (Notify == 0U) {
			;
		}
	}

	Status = (s32)ErrorStatus;
	Notify = 0U;
	TrackCompletionOrder = 0U;

	if (OverlapAes != 0U) {
		if (AesStatus != XST_SUCCESS) {
			Status = AesStatus;
			goto END;
		}
		if (AesSeq > SignSeq) {
			xil_printf("\r\n AES did not complete before SLH-DSA sign "
				   "(AES seq %u, sign seq %u)\r\n", AesSeq, SignSeq);
			Status = XST_FAILURE;
			goto END;
		}
		AesStatus = XAsu_SlhdsaCheckAesGcmEncrypt();
		if (AesStatus != XST_SUCCESS) {
			Status = AesStatus;
			goto END;
		}
		xil_printf("\r\n AES completed before SLH-DSA sign (AES seq %u, sign seq %u)\r\n",
			   AesSeq, SignSeq);
	}

	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaSignature, SigSize);
#endif

END:
	AesNotify = 0U;
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Queues an AES-GCM encrypt on the high-priority IPI queue.
*
* @return
*		- XST_SUCCESS if the request was queued.
*		- Error code on failure.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaSubmitAesGcmEncrypt(void)
{
	s32 Status = XST_FAILURE;
	XAsu_AesParams AesParams = {0U};

	XAsu_SlhdsaAesClientParams.Priority = XASU_PRIORITY_HIGH;
	XAsu_SlhdsaAesClientParams.SecureFlag = XASU_CMD_SECURE;
	XAsu_SlhdsaAesClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_SlhdsaAesCallBackRef);
	XAsu_SlhdsaAesClientParams.CallBackRefPtr = (void *)&XAsu_SlhdsaAesClientParams;
	XAsu_SlhdsaAesClientParams.AdditionalStatus = XST_FAILURE;

	AesParams.EngineMode = XASU_AES_GCM_MODE;
	AesParams.OperationFlags = (XASU_INIT | XASU_UPDATE | XASU_FINISH);
	AesParams.IsLast = TRUE;
	AesParams.OperationType = XASU_AES_ENCRYPT_OPERATION;

	XAsu_SlhdsaAesKeyObj.KeyAddress = (u64)(UINTPTR)XAsu_SlhdsaAesKey;
	XAsu_SlhdsaAesKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	XAsu_SlhdsaAesKeyObj.KeySrc = XASU_AES_USER_KEY_0;
	XAsu_SlhdsaAesKeyObj.KeyId = 0U;
	AesParams.KeyObjectAddr = (u64)(UINTPTR)&XAsu_SlhdsaAesKeyObj;
	AesParams.IvObj.IvAddr = (u64)(UINTPTR)XAsu_SlhdsaAesIv;
	AesParams.IvObj.IvLen = XASU_SLHDSA_EXAMPLE_AES_IV_LEN;
	AesParams.IvObj.IvId = 0U;
	AesParams.InputDataAddr = (u64)(UINTPTR)XAsu_SlhdsaAesInput;
	AesParams.OutputDataAddr = (u64)(UINTPTR)XAsu_SlhdsaAesEncData;
	AesParams.DataLen = XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN;
	AesParams.AadAddr = (u64)(UINTPTR)XAsu_SlhdsaAesAad;
	AesParams.AadLen = XASU_SLHDSA_EXAMPLE_AES_AAD_LEN;
	AesParams.TagAddr = (u64)(UINTPTR)XAsu_SlhdsaAesTag;
	AesParams.TagLen = XASU_SLHDSA_EXAMPLE_AES_TAG_LEN;

	Status = XAsu_AesOperation(&XAsu_SlhdsaAesClientParams, &AesParams);

	return Status;
}

/*************************************************************************************************/
/**
* @brief	Checks that the overlapped AES-GCM encrypt completed with the expected tag and
*		ciphertext.
*
* @return
*		- XST_SUCCESS if AES succeeded and the ciphertext and tag match.
*		- XST_FAILURE otherwise.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaCheckAesGcmEncrypt(void)
{
	s32 Status = XST_FAILURE;

	if ((AesErrorStatus != (u32)XST_SUCCESS) ||
	    (XAsu_SlhdsaAesClientParams.AdditionalStatus != (u32)XASU_AES_TAG_READ)) {
		XilAsu_Printf("\r\n AES overlap failed from server with error %08x and "
			      "additional error of %08x", AesErrorStatus,
			      XAsu_SlhdsaAesClientParams.AdditionalStatus);
		goto END;
	}

#ifndef XASU_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaAesEncData,
				  XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN);
	Xil_DCacheInvalidateRange((UINTPTR)XAsu_SlhdsaAesTag, XASU_SLHDSA_EXAMPLE_AES_TAG_LEN);
#endif

	Status = Xil_SMemCmp_CT(XAsu_SlhdsaAesExpCt, XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN,
				XAsu_SlhdsaAesEncData, XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN,
				XASU_SLHDSA_EXAMPLE_AES_PAYLOAD_LEN);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n AES overlap failed at ciphertext comparison");
		goto END;
	}

	Status = Xil_SMemCmp_CT(XAsu_SlhdsaAesExpTag, XASU_SLHDSA_EXAMPLE_AES_TAG_LEN,
				XAsu_SlhdsaAesTag, XASU_SLHDSA_EXAMPLE_AES_TAG_LEN,
				XASU_SLHDSA_EXAMPLE_AES_TAG_LEN);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n AES overlap failed at tag comparison");
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Sends one SLH-DSA verify request and waits for a verified verdict.
*
* @param	ClientParamsPtr	Pointer to client parameters.
* @param	ParamSet	SLH-DSA parameter set selector.
* @param	MsgAddr		Address of the message or PH(M) digest used at signing time.
* @param	MsgLen		Length of the message or digest in bytes.
* @param	CtxAddr		Address of the FIPS context string used at signing time.
* @param	CtxLen		Context length used at signing time.
* @param	PreHashMode	Message mode used at signing time.
* @param	HashId		Pre-hash HashId used at signing time.
*
* @return
*		- XST_SUCCESS if the server reports XASU_SLHDSA_SIGNATURE_VERIFIED.
*		- Error code or XST_FAILURE otherwise.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaVerifyOnce(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet,
				u64 MsgAddr, u32 MsgLen, u64 CtxAddr, u32 CtxLen,
				u8 PreHashMode, u8 HashId)
{
	s32 Status = XST_FAILURE;
	XAsu_SlhdsaSignVerifyParams VerifyParams = {0U};
	u32 PkSize = XAsu_SlhdsaGetPkSize(ParamSet);
	u32 SigSize = XAsu_SlhdsaGetSigSize(ParamSet);

	/**
	 * Repeat the message, context and message mode used at signing time. AdditionalStatus
	 * is cleared so a stale verified verdict from an earlier sequence cannot be reused.
	 */
	ErrorStatus = XST_FAILURE;
	ClientParamsPtr->AdditionalStatus = XST_FAILURE;
	VerifyParams.PkObj.PubKeyAddr = (u64)(UINTPTR)XAsu_SlhdsaPublicKey;
	VerifyParams.PkObj.PubKeyId = 0U;
	VerifyParams.PkObj.PubKeyLen = PkSize;
	VerifyParams.MsgAddr = MsgAddr;
	VerifyParams.SignatureAddr = (u64)(UINTPTR)XAsu_SlhdsaSignature;
	VerifyParams.MsgLen = MsgLen;
	VerifyParams.SignatureLen = SigSize;
	VerifyParams.CtxAddr = CtxAddr;
	VerifyParams.CtxLen = CtxLen;
	VerifyParams.ParamSet = ParamSet;
	VerifyParams.PreHashMode = PreHashMode;
	VerifyParams.HashId = HashId;

	/** Send the signature verification request to ASUFW. */
	Status = XAsu_SlhdsaSignVerify(ClientParamsPtr, &VerifyParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n SLH-DSA verify request failed with Status = %08x", Status);
		goto END;
	}

	/**
	 * Treat the signature as valid only when the server status is success and
	 * AdditionalStatus reports XASU_SLHDSA_SIGNATURE_VERIFIED.
	 */
	Status = XAsu_SlhdsaWaitForCompletion();
	if ((Status != XST_SUCCESS) ||
	    (ClientParamsPtr->AdditionalStatus != XASU_SLHDSA_SIGNATURE_VERIFIED)) {
		Status = XST_FAILURE;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Waits for the registered callback to complete an ASUFW request.
*
* @return
*		- Server status returned through the callback.
*
*************************************************************************************************/
static s32 XAsu_SlhdsaWaitForCompletion(void)
{
	/** Busy-wait for the callback to set the notify flag. */
	while (!Notify) {
		;
	}

	/** Clear the flag so the next request in the sequence waits on its own callback. */
	Notify = 0U;

	return (s32)ErrorStatus;
}

/*************************************************************************************************/
/**
* @brief	Callback registered with xilasu to notify request completion.
*
* @param	CallBackRef	Pointer to the callback reference.
* @param	Status		Status returned from ASUFW.
*
*************************************************************************************************/
static void XAsu_SlhdsaCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	/** Record the status before raising Notify, so the waiter never sees a stale status. */
	ErrorStatus = Status;
	if (TrackCompletionOrder != 0U) {
		CompletionSeq++;
		SignSeq = CompletionSeq;
	}
	Notify = 1U;
}

/*************************************************************************************************/
/**
* @brief	Callback registered for the AES-GCM request overlapped with SLH-DSA sign.
*
* @param	CallBackRef	Pointer to the AES client parameters.
* @param	Status		Status returned from ASUFW.
*
*************************************************************************************************/
static void XAsu_SlhdsaAesCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	AesErrorStatus = Status;
	if (TrackCompletionOrder != 0U) {
		CompletionSeq++;
		AesSeq = CompletionSeq;
	}
	AesNotify = 1U;
}

/** @} */
