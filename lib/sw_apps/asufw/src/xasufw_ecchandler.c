/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_ecchandler.c
 *
 * This file contains the ECC module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/19/24 Initial release
 *       am   09/13/24 Fixed array initialization error for cpp compiler
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ss   12/02/24 Added support for ECDH
 *       ma   12/12/24 Updated resource allocation logic
 *       yog  03/25/25 Added support for public key generation.
 *       yog  07/11/25 Updated code related to curve type value updation.
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#include "xasufw_ecchandler.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_cmd.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xrsa_ecc.h"
#include "xecc.h"
#include "xasu_eccinfo.h"
#include "xasufw_trnghandler.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_EccKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EccGenSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EccVerifySign(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EcdhGenSharedSecret(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EcdhKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EccResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EccGenPubKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_EccModule; /**< ASUFW ECC Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the ECC module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECC module initialization is successful.
 * 	- XASUFW_ECC_MODULE_REGISTRATION_FAILED, if ECC module registration fails.
 * 	- XASUFW_ECC_INIT_FAILED, if ECC init fails.
 *
 *************************************************************************************************/
s32 XAsufw_EccInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);

	/** The XAsufw_EccCmds array contains the list of commands supported by ECC module. */
	static const XAsufw_ModuleCmd XAsufw_EccCmds[] = {
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccGenSign),
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccVerifySign),
		[XASU_ECC_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccKat),
		[XASU_ECDH_SHARED_SECRET_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EcdhGenSharedSecret),
		[XASU_ECDH_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EcdhKat),
		[XASU_ECC_GEN_PUBKEY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccGenPubKey),
	};

	/** The XAsufw_EccResourcesBuf contains the required resources for each supported command. */
	/**
	 * For XASU_ECC_GEN_SIGNATURE_CMD_ID, XASU_ECC_VERIFY_SIGNATURE_CMD_ID and
	 * XASU_ECC_GEN_PUBKEY_CMD_ID, XASUFW_ECC_RESOURCE_MASK checks for the availability of ECC
	 * or RSA core based on the curve type received.
	 * For XASU_ECC_KAT_CMD_ID, both RSA and ECC cores are required. So, XASUFW_ECC_RESOURCE_MASK
	 * checks for the availability of ECC core and XASUFW_RSA_RESOURCE_MASK checks for the
	 * availability of RSA core.
	 */
	static XAsufw_ResourcesRequired XAsufw_EccResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_EccCmds)] = {
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_SHA_RESOURCE_MASK,
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_SHA_RESOURCE_MASK,
		[XASU_ECC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_RSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_ECDH_SHARED_SECRET_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_ECDH_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_ECC_GEN_PUBKEY_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_SHA_RESOURCE_MASK,
	};

	XAsufw_EccModule.Id = XASU_MODULE_ECC_ID;
	XAsufw_EccModule.Cmds = XAsufw_EccCmds;
	XAsufw_EccModule.ResourcesRequired = XAsufw_EccResourcesBuf;
	XAsufw_EccModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_EccCmds);
	XAsufw_EccModule.ResourceHandler = XAsufw_EccResourceHandler;
	XAsufw_EccModule.AsuDmaPtr = NULL;
	XAsufw_EccModule.ShaPtr = NULL;
	XAsufw_EccModule.AesPtr = NULL;

	/** Register ECC module. */
	Status = XAsufw_ModuleRegister(&XAsufw_EccModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Initialize the ECC crypto engine. */
	Status = XEcc_Initialize(XAsufw_Ecc);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_INIT_FAILED);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for ECC module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_EccResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	/** Allocate DMA resource for ECC module commands except for Get_Info command. */
	ResourceId = XAsufw_GetEccMaskResourceId(ReqBuf);
	XAsufw_EccModule.AsuDmaPtr = XAsufw_AllocateDmaResource(ResourceId, ReqId);
	if (XAsufw_EccModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}
	XAsufw_AllocateResource(ResourceId, ResourceId, ReqId);
	XAsufw_AllocateResource(XASUFW_TRNG, ResourceId, ReqId);
	if (CmdId == XASU_ECC_KAT_CMD_ID) {
		XAsufw_AllocateResource(XASUFW_RSA, ResourceId, ReqId);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC sign generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if sign generation operation is successful.
 * 	- XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL, if TRNG fails to generate random number.
 * 	- XASUFW_ECC_GEN_SIGN_OPERATION_FAIL, if generate signature operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGenSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u8 EphemeralKey[XASU_ECC_P521_SIZE_IN_BYTES];
	u64 PvtKeyAddr = EccParamsPtr->KeyAddr;

	if ((EccParamsPtr->CurveType == XASU_ECC_NIST_P256) ||
		(EccParamsPtr->CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
	} else {
		ResourceId = XASUFW_RSA;
	}

	/** Generate ephemeral key using TRNG. */
	Status = XAsufw_TrngGetRandomNumbers(EphemeralKey, EccParamsPtr->KeyLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL;
		XFIH_GOTO(END);
	}

	/**
	 * Generate signature using core API XEcc_GenerateSignature or XRsa_EccGenerateSignature
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_GenerateSignature(EccInstancePtr, XAsufw_EccModule.AsuDmaPtr,
				EccParamsPtr->CurveType, EccParamsPtr->KeyLen, PvtKeyAddr,
				EphemeralKey, EccParamsPtr->DigestAddr,	EccParamsPtr->DigestLen,
				EccParamsPtr->SignAddr);
	} else {
		Status = XRsa_EccGenerateSignature(XAsufw_EccModule.AsuDmaPtr, EccParamsPtr->CurveType,
			EccParamsPtr->KeyLen, PvtKeyAddr, EphemeralKey, EccParamsPtr->DigestAddr,
			EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_GEN_SIGN_OPERATION_FAIL);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(ResourceId, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC sign verification operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if sign verification operation is successful.
 * 	- XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL, if verify signature operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_EccVerifySign(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u64 PubKeyAddr = EccParamsPtr->KeyAddr;

	if ((EccParamsPtr->CurveType == XASU_ECC_NIST_P256) ||
		(EccParamsPtr->CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
	} else {
		ResourceId = XASUFW_RSA;
	}

	/**
	 * Verify signature using core API XEcc_VerifySignature or XRsa_EccVerifySignature
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_VerifySignature(EccInstancePtr, XAsufw_EccModule.AsuDmaPtr,
				EccParamsPtr->CurveType, EccParamsPtr->KeyLen, PubKeyAddr,
				EccParamsPtr->DigestAddr, EccParamsPtr->DigestLen,
				EccParamsPtr->SignAddr);
	} else {
		Status = XRsa_EccVerifySignature(XAsufw_EccModule.AsuDmaPtr, EccParamsPtr->CurveType,
				EccParamsPtr->KeyLen, PubKeyAddr, EccParamsPtr->DigestAddr,
				EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(ResourceId, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC public key generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public key generation operation is successful.
 * 	- XASUFW_ECC_GEN_PUB_KEY_OPERATION_FAIL, if generate public key operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGenPubKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccKeyParams *EccParamsPtr = (const XAsu_EccKeyParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	if ((EccParamsPtr->CurveType == XASU_ECC_NIST_P256) ||
		(EccParamsPtr->CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
	} else {
		ResourceId = XASUFW_RSA;
	}

	/**
	 * Generate the public key using core API XEcc_GeneratePublicKey or XRsa_EccGeneratePubKey
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_GeneratePublicKey(EccInstancePtr, XAsufw_EccModule.AsuDmaPtr,
				EccParamsPtr->CurveType, EccParamsPtr->KeyLen,
				EccParamsPtr->PvtKeyAddr, EccParamsPtr->PubKeyAddr);
	} else {
		Status = XRsa_EccGeneratePubKey(XAsufw_EccModule.AsuDmaPtr, EccParamsPtr->CurveType,
			EccParamsPtr->KeyLen, EccParamsPtr->PvtKeyAddr, EccParamsPtr->PubKeyAddr);
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_GEN_PUB_KEY_OPERATION_FAIL);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(ResourceId, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECDH secret generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if command execution is successful.
 *	- XASUFW_ECDH_GEN_SECRET_OPERATION_FAIL, if shared secret generation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_EcdhGenSharedSecret(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_EcdhParams *EcdhParamsPtr = (const XAsu_EcdhParams *)ReqBuf->Arg;

	/** Generate shared secret using public key and private key based on curve type. */
	Status = XRsa_EcdhGenSharedSecret(XAsufw_EccModule.AsuDmaPtr, EcdhParamsPtr->CurveType,
					EcdhParamsPtr->KeyLen, EcdhParamsPtr->PvtKeyAddr,
					EcdhParamsPtr->PubKeyAddr, EcdhParamsPtr->SharedSecretAddr,
					EcdhParamsPtr->SharedSecretObjIdAddr);

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECDH_GEN_SECRET_OPERATION_FAIL);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs) utilizing both ECC and RSA cores.
 *
 * @param	ReqBuf	Pointer to XAsu_ReqBuf structure.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal release resource is requested.
 * 	- Error code from XAsufw_EccCoreKat or XAsufw_RsaEccKat, if any failure occurs.

 *
 *************************************************************************************************/
static s32 XAsufw_EccKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const u32 CurveType = *((const u32 *)ReqBuf->Arg);

	(void)ReqBuf;

	/** Perform KAT on P-256 curve using ECC core. */
	Status = XAsufw_EccCoreKat(XAsufw_EccModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Perform KAT on P-256 curve using RSA core. */
	Status = XAsufw_RsaEccKat(XAsufw_EccModule.AsuDmaPtr, (u8)CurveType);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_ECC, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs) utilizing RSA cores.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 		- XASUFW_SUCCESS, if command execution is successful.
 *		- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *		- Error codes from XAsufw_P256EcdhKat, if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_EcdhKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;

	/** Perform KAT on P-256 curve using RSA core. */
	Status = XAsufw_P256EcdhKat(XAsufw_EccModule.AsuDmaPtr);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to get the resource ID when XASUFW_ECC_RESOURCE_MASK is included.
 * For XASU_ECC_GEN_SIGNATURE_CMD_ID, XASU_ECC_VERIFY_SIGNATURE_CMD_ID and XASU_ECC_GEN_PUBKEY_CMD_ID,
 * 	- XASUFW_ECC_RESOURCE_MASK checks for the availability of ECC or RSA core based on the
 *	  curve type received.
 * For XASU_ECC_KAT_CMD_ID, both RSA and ECC cores are required. So, XASUFW_ECC_RESOURCE_MASK
 * checks for the availability of ECC core and XASUFW_RSA_RESOURCE_MASK checks for the
 * availability of RSA core.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 *
 * @return
 *	- ECC or RSA resource ID.
 *
 *************************************************************************************************/
XAsufw_Resource XAsufw_GetEccMaskResourceId(const XAsu_ReqBuf *ReqBuf)
{
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Resource Resource = XASUFW_INVALID;

	if (((CmdId == XASU_ECC_GEN_SIGNATURE_CMD_ID) ||
	    (CmdId == XASU_ECC_VERIFY_SIGNATURE_CMD_ID) ||
	    (CmdId == XASU_ECC_GEN_PUBKEY_CMD_ID)) &&
	    ((EccParamsPtr->CurveType == XASU_ECC_NIST_P256) ||
	    (EccParamsPtr->CurveType == XASU_ECC_NIST_P384))) {
		Resource = XASUFW_ECC;
	} else if (CmdId == XASU_ECC_KAT_CMD_ID) {
		Resource = XASUFW_ECC;
	} else {
		Resource = XASUFW_RSA;
	}

	return Resource;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to get the resource ID when XASUFW_RSA_SHA_RESOURCE_MASK is
 * 		included. For XASU_ECC_GEN_SIGNATURE_CMD_ID, XASU_ECC_VERIFY_SIGNATURE_CMD_ID and
 * 		XASU_ECC_GEN_PUBKEY_CMD_ID, XASUFW_RSA_SHA_RESOURCE_MASK checks for the
 * 		availability of SHA2 or SHA3 for Ed25519/Ed25519PH or Ed448/Ed448PH respectively.
 * 		If the curve type is other than these, no resource availability is checked.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 *
 * @return
 *	- SHA2, SHA3 or RSA resource ID.
 *
 *************************************************************************************************/
XAsufw_Resource XAsufw_GetRsaShaMaskResourceId(const XAsu_ReqBuf *ReqBuf)
{
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Resource Resource = XASUFW_INVALID;

	if ((EccParamsPtr->CurveType == XASU_ECC_NIST_ED25519) ||
		(EccParamsPtr->CurveType == XASU_ECC_NIST_ED25519_PH)) {
		Resource = XASUFW_SHA2;
	} else if ((EccParamsPtr->CurveType == XASU_ECC_NIST_ED448) ||
		(EccParamsPtr->CurveType == XASU_ECC_NIST_ED448_PH)) {
		Resource = XASUFW_SHA3;
	} else {
		Resource = XASUFW_NONE;
	}

	return Resource;
}
/** @} */
