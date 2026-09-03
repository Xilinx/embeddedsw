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
* Client buffers passed to ASUFW must reside in memory visible to both client and server.
* See the linker-script notes in xasu_sha2_example.c for OCM/shared-memory placement.
*
* IPI command flow demonstrated:
* 1. XAsu_ClientInit initializes the mailbox and client layer.
* 2. For each parameter set, XAsu_KmGenerateSlhdsaKeyPair sends keygen parameters
*    (SkOutAddr, PkOutAddr, ParamSet) to the key manager and waits for the IPI callback.
* 3. XAsu_SlhdsaSign sends sign parameters (message, secret key, signature output,
*    context) and waits for completion.
* 4. XAsu_SlhdsaSignVerify sends verify parameters with matching CtxLen and ParamSet.
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

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_CACHE_DISABLE	/**< Disables cache maintenance calls in this example. */
#define XASU_SLHDSA_EXAMPLE_VARIANT_COUNT	(4U)	/**< Number of FIPS 205 parameter sets exercised by this example. */
#define XASU_SLHDSA_EXAMPLE_MAX_SIG_SIZE	XASU_SLHDSA_SHA2_256S_SIG_SIZE	/**< Largest signature size across the exercised parameter sets. */
#define XASU_SLHDSA_EXAMPLE_MSG_SIZE		((u32)sizeof(XAsu_SlhdsaMessage))	/**< Test message size in bytes. */

/************************************ Function Prototypes ****************************************/
static void XAsu_SlhdsaCallBackRef(void *CallBackRef, u32 Status);
static s32 XAsu_SlhdsaWaitForCompletion(void);
static s32 XAsu_SlhdsaKeyGenExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaSignExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaSignVerifyExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaRunVariantExample(XAsu_ClientParams *ClientParamsPtr, u8 ParamSet);
static s32 XAsu_SlhdsaExample(void);

/************************************ Variable Definitions ***************************************/
static const u8 XAsu_SlhdsaMessage[] __attribute__ ((
		section (".data.XAsu_SlhdsaMessage"))) = {
	'S', 'L', 'H', '-', 'D', 'S', 'A', ' ',
	'A', 'S', 'U', ' ',
	'c', 'l', 'i', 'e', 'n', 't', ' ',
	'e', 'x', 'a', 'm', 'p', 'l', 'e'
};
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
static volatile u8 Notify = 0U;
static volatile u32 ErrorStatus = XST_FAILURE;

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
	KeyGenParams.PkOutAddr = (u64)(UINTPTR)XAsu_SlhdsaPublicKey;
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
	XAsu_SlhdsaSignParams SignParams = {0U};
	u32 SigSize = XAsu_SlhdsaGetSigSize(ParamSet);

	/**
	 * Set up sign parameters. SkId is zero because the secret key is passed by address
	 * rather than fetched from the key vault, and SignatureLen must be exactly the
	 * signature size of the parameter set or validation rejects the request.
	 */
	ErrorStatus = XST_FAILURE;
	SignParams.SkObj.SkAddr = (u64)(UINTPTR)XAsu_SlhdsaSecretKey;
	SignParams.SkObj.SkId = 0U;
	SignParams.MsgAddr = (u64)(UINTPTR)XAsu_SlhdsaMessage;
	SignParams.SignatureAddr = (u64)(UINTPTR)XAsu_SlhdsaSignature;
	SignParams.MsgLen = XASU_SLHDSA_EXAMPLE_MSG_SIZE;
	SignParams.SignatureLen = SigSize;
	/**
	 * A zero context address with zero length selects the FIPS empty context, and the same
	 * pair must be used at verification time.
	 */
	SignParams.CtxAddr = 0U;
	/* SignMode defaults to XASU_SLHDSA_SIGN_MODE_HEDGED (0): fresh add_rand from TRNG. */
	SignParams.CtxLen = 0U;
	SignParams.ParamSet = ParamSet;

	/** Send the sign request to ASUFW. */
	Status = XAsu_SlhdsaSign(ClientParamsPtr, &SignParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n SLH-DSA sign request failed with Status = %08x", Status);
		goto END;
	}

	/**
	 * Wait for the callback. Signing takes seconds to tens of seconds depending on the
	 * parameter set, so this busy-wait is long lived by design.
	 */
	Status = XAsu_SlhdsaWaitForCompletion();

END:
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
	XAsu_SlhdsaSignVerifyParams VerifyParams = {0U};
	u32 PkSize = XAsu_SlhdsaGetPkSize(ParamSet);
	u32 SigSize = XAsu_SlhdsaGetSigSize(ParamSet);

	/**
	 * Set up verify parameters, repeating the message, context and parameter set used at
	 * signing time. AdditionalStatus is cleared as well as ErrorStatus because it carries
	 * the verification verdict, and a stale value from an earlier variant must not be read
	 * as this signature verifying.
	 */
	ErrorStatus = XST_FAILURE;
	ClientParamsPtr->AdditionalStatus = XST_FAILURE;
	VerifyParams.PkObj.PubKeyAddr = (u64)(UINTPTR)XAsu_SlhdsaPublicKey;
	VerifyParams.PkObj.PubKeyId = 0U;
	VerifyParams.PkObj.PubKeyLen = PkSize;
	VerifyParams.MsgAddr = (u64)(UINTPTR)XAsu_SlhdsaMessage;
	VerifyParams.SignatureAddr = (u64)(UINTPTR)XAsu_SlhdsaSignature;
	VerifyParams.MsgLen = XASU_SLHDSA_EXAMPLE_MSG_SIZE;
	VerifyParams.SignatureLen = SigSize;
	VerifyParams.CtxAddr = 0U;
	VerifyParams.CtxLen = 0U;
	VerifyParams.ParamSet = ParamSet;

	/** Send the signature verification request to ASUFW. */
	Status = XAsu_SlhdsaSignVerify(ClientParamsPtr, &VerifyParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n SLH-DSA verify request failed with Status = %08x", Status);
		goto END;
	}

	/**
	 * Treat the signature as valid only when the server status is success and
	 * AdditionalStatus reports XASU_SLHDSA_SIGNATURE_VERIFIED, which is what distinguishes
	 * a verified signature from a command that merely completed.
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
	Notify = 1U;
}

/** @} */
