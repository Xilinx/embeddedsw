/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       rmv  08/17/26 Added validation of client provided buffers
 *       vm   09/15/26 Allocated the ECC core for key manager commands
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
#include "xasufw_memvalidation.h"
#include "xasufw_cmd.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xrsa_ecc.h"
#include "xecc.h"
#include "xoverlay.h"
#include "xasu_eccinfo.h"
#include "xasufw_trnghandler.h"
#include "xfih.h"
#include "xkeymanager.h"
#include "xasu_sharedmem.h"

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
#ifdef XASU_ENABLE_CAVP_SUPPORT
static s32 XAsufw_EccValidatePubKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif /* XASU_ENABLE_CAVP_SUPPORT */

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_EccModule; /**< ASUFW ECC Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the ECC module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECC module initialization is successful.
 * 	- XASUFW_ECC_MODULE_REGISTRATION_FAILED, if ECC module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_EccInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_EccCmds array contains the list of commands supported by ECC module. */
	static const XAsufw_ModuleCmd XAsufw_EccCmds[XASU_ECC_MAX_CMDS] = {
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccGenSign),
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccVerifySign),
		[XASU_ECC_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccKat),
		[XASU_ECDH_SHARED_SECRET_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EcdhGenSharedSecret),
		[XASU_ECDH_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EcdhKat),
		[XASU_ECC_GEN_PUBKEY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccGenPubKey),
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_ECC_VAL_PUBKEY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccValidatePubKey),
#endif /* XASU_ENABLE_CAVP_SUPPORT */
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
	static XAsufw_ResourcesRequired XAsufw_EccResourcesBuf[XASU_ECC_MAX_CMDS] = {
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_SHA_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_SHA_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_ECC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_RSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_ECDH_SHARED_SECRET_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_ECDH_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_ECC_GEN_PUBKEY_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_SHA_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_ECC_VAL_PUBKEY_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_SHA_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	/** The XAsufw_EccAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_EccAccessPermBuf[XASU_ECC_MAX_CMDS] = {
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_ECC_GEN_SIGNATURE_CMD_ID),
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_ECC_VERIFY_SIGNATURE_CMD_ID),
		[XASU_ECC_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_ECC_KAT_CMD_ID),
		[XASU_ECDH_SHARED_SECRET_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_ECDH_SHARED_SECRET_CMD_ID),
		[XASU_ECDH_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_ECDH_KAT_CMD_ID),
		[XASU_ECC_GEN_PUBKEY_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_ECC_GEN_PUBKEY_CMD_ID),
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_ECC_VAL_PUBKEY_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_ECC_VAL_PUBKEY_CMD_ID),
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	XAsufw_EccModule.Id = XASU_MODULE_ECC_ID;
	XAsufw_EccModule.Cmds = XAsufw_EccCmds;
	XAsufw_EccModule.ResourcesRequired = XAsufw_EccResourcesBuf;
	XAsufw_EccModule.CmdCnt = XASU_ECC_MAX_CMDS;
	XAsufw_EccModule.ResourceHandler = XAsufw_EccResourceHandler;
	XAsufw_EccModule.AsuDmaPtr = NULL;
	XAsufw_EccModule.ShaPtr = NULL;
	XAsufw_EccModule.AesPtr = NULL;
	XAsufw_EccModule.AccessPermBufferPtr = XAsufw_EccAccessPermBuf;

	/** Register ECC module. */
	Status = XAsufw_ModuleRegister(&XAsufw_EccModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_MODULE_REGISTRATION_FAILED;
		goto END;
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
 * 	- XASUFW_OVERLAY_ACTIVATE_FAIL, if overlay activation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource fails.
 * 	- XASUFW_RESOURCE_INVALID, if an invalid resource is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_EccResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	/** Allocate DMA resource for ECC module commands except for Get_Info command. */
	ResourceId = XAsufw_GetEccMaskResourceId(ReqBuf);
	if (ResourceId == XASUFW_INVALID) {
		Status = XASUFW_RESOURCE_INVALID;
		goto END;
	}
	XAsufw_EccModule.AsuDmaPtr = XAsufw_AllocateDmaResource(ResourceId, ReqId);
	if (XAsufw_EccModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}
	XAsufw_AllocateResource(ResourceId, ResourceId, ReqId);
	XAsufw_AllocateResource(XASUFW_TRNG, ResourceId, ReqId);
	XAsufw_AllocateResource(XASUFW_OVERLAY, ResourceId, ReqId);
	if (CmdId == XASU_ECC_KAT_CMD_ID) {
		XAsufw_AllocateResource(XASUFW_RSA, ResourceId, ReqId);
	}

	/** Activate RSA-ECC overlay. */
	Status = XOverlay_Activate(XOVERLAY_ALGO_RSA_ECC, XAsufw_EccModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		if (XAsufw_ReleaseResource(ResourceId, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

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
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGenSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	XAsu_EccKeyObject EccKeyObj;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_EccParams);

	/** Copy key object from request */
	EccKeyObj = EccParamsPtr->Key;

	if ((EccKeyObj.CurveType == XASU_ECC_NIST_P256) ||
		(EccKeyObj.CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
	} else {
		ResourceId = XASUFW_RSA;
	}
	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided private key buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccKeyObj.KeyAddr,
				   EccKeyObj.KeyLen);

#ifdef XASU_KEYMANAGER_ENABLE
	/** If KeyAddr is zero and KeyId is provided, retrieve private key from vault. */
	if (EccKeyObj.KeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_EccModule.AsuDmaPtr,
			EccKeyObj.KeyId, (u64)(UINTPTR)&EccKeyObj, SubSystemId,
			XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
			XASU_ECC_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

	/** Validate the client provided digest and signature buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccParamsPtr->DigestAddr,
				   EccParamsPtr->DigestLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccParamsPtr->SignAddr,
				   XASUFW_DOUBLE_VALUE(EccKeyObj.KeyLen));

	/**
	 * Generate signature using core API XEcc_GenerateSignature or XRsa_EccGenerateSignature
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_GenerateSignature(EccInstancePtr, XAsufw_EccModule.AsuDmaPtr,
				EccKeyObj.CurveType, EccKeyObj.KeyLen, EccKeyObj.KeyAddr,
				NULL, EccParamsPtr->DigestAddr,	EccParamsPtr->DigestLen,
				EccParamsPtr->SignAddr);
	} else {
		Status = XRsa_EccGenerateSignature(XAsufw_EccModule.AsuDmaPtr, EccKeyObj.CurveType,
			EccKeyObj.KeyLen, EccKeyObj.KeyAddr, NULL, EccParamsPtr->DigestAddr,
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
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_EccVerifySign(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	XAsu_EccKeyObject EccKeyObj;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_EccParams);

	/** Copy key object from request */
	EccKeyObj = EccParamsPtr->Key;

	if ((EccKeyObj.CurveType == XASU_ECC_NIST_P256) ||
		(EccKeyObj.CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
	} else {
		ResourceId = XASUFW_RSA;
	}
	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided public key buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccKeyObj.KeyAddr,
				   XASUFW_DOUBLE_VALUE(EccKeyObj.KeyLen));

#ifdef XASU_KEYMANAGER_ENABLE
	/** If KeyAddr is zero and KeyId is provided, retrieve public key from vault. */
	if (EccKeyObj.KeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_EccModule.AsuDmaPtr,
			EccKeyObj.KeyId, (u64)(UINTPTR)&EccKeyObj, SubSystemId,
			XASU_KEYMANAGER_ECC_PUB_SIGN_VER_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
			XASU_ECC_PUB_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

	/** Validate the client provided digest and signature buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccParamsPtr->DigestAddr,
				   EccParamsPtr->DigestLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccParamsPtr->SignAddr,
				   XASUFW_DOUBLE_VALUE(EccKeyObj.KeyLen));

	/**
	 * Verify signature using core API XEcc_VerifySignature or XRsa_EccVerifySignature
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_VerifySignature(EccInstancePtr, XAsufw_EccModule.AsuDmaPtr,
				EccKeyObj.CurveType, EccKeyObj.KeyLen, EccKeyObj.KeyAddr,
				EccParamsPtr->DigestAddr, EccParamsPtr->DigestLen,
				EccParamsPtr->SignAddr);
	} else {
		Status = XRsa_EccVerifySignature(XAsufw_EccModule.AsuDmaPtr, EccKeyObj.CurveType,
				EccKeyObj.KeyLen, EccKeyObj.KeyAddr, EccParamsPtr->DigestAddr,
				EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL);
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
 * @brief	This function is a handler for ECC public key generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public key generation operation is successful.
 * 	- XASUFW_ECC_GEN_PUB_KEY_OPERATION_FAIL, if generate public key operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGenPubKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccKeyParams *EccParamsPtr = (const XAsu_EccKeyParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	XAsu_EccKeyObject EccPvtKeyObj;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_EccKeyParams);

	/** Copy key object from request */
	EccPvtKeyObj = EccParamsPtr->PvtKey;

	if ((EccPvtKeyObj.CurveType == XASU_ECC_NIST_P256) ||
		(EccPvtKeyObj.CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
	} else {
		ResourceId = XASUFW_RSA;
	}
	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided private key buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccPvtKeyObj.KeyAddr,
				   EccPvtKeyObj.KeyLen);

#ifdef XASU_KEYMANAGER_ENABLE
	if (EccPvtKeyObj.KeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_EccModule.AsuDmaPtr,
			EccPvtKeyObj.KeyId, (u64)(UINTPTR)&EccPvtKeyObj, SubSystemId,
			XASU_KEYMANAGER_ECC_PVT_PUB_KEY_GEN_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
			XASU_ECC_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

	/** Validate the client buffer receiving the generated public key. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccParamsPtr->PubKeyAddr,
				   XASUFW_DOUBLE_VALUE(EccPvtKeyObj.KeyLen));

	/**
	 * Generate the public key using core API XEcc_GeneratePublicKey or XRsa_EccGeneratePubKey
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_GeneratePublicKey(EccInstancePtr, XAsufw_EccModule.AsuDmaPtr,
				EccPvtKeyObj.CurveType, EccPvtKeyObj.KeyLen,
				EccPvtKeyObj.KeyAddr, EccParamsPtr->PubKeyAddr);
	} else {
		Status = XRsa_EccGeneratePubKey(XAsufw_EccModule.AsuDmaPtr, EccPvtKeyObj.CurveType,
			EccPvtKeyObj.KeyLen, EccPvtKeyObj.KeyAddr, EccParamsPtr->PubKeyAddr);
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_GEN_PUB_KEY_OPERATION_FAIL);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(ResourceId, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

#ifdef XASU_ENABLE_CAVP_SUPPORT
/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC public key validation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public key validation operation is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 * 	- XASUFW_INVALID_PARAM, if any argument is invalid.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_EccValidatePubKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccKeyParams *EccParamsPtr = (const XAsu_EccKeyParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	if ((EccParamsPtr->PvtKey.CurveType == XASU_ECC_NIST_P256) ||
		(EccParamsPtr->PvtKey.CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
	} else {
		ResourceId = XASUFW_RSA;
	}

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_EccKeyParams);

	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided public key buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, EccParamsPtr->PubKeyAddr,
				   XASUFW_DOUBLE_VALUE(EccParamsPtr->PvtKey.KeyLen));

	/**
	 * Validate the public key using core API XEcc_ValidatePublicKey or XRsa_EccValidatePubKey
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_ValidatePublicKey(EccInstancePtr, XAsufw_EccModule.AsuDmaPtr,
				EccParamsPtr->PvtKey.CurveType, EccParamsPtr->PvtKey.KeyLen, EccParamsPtr->PubKeyAddr);
	} else {
		Status = XRsa_EccValidatePubKey(XAsufw_EccModule.AsuDmaPtr, EccParamsPtr->PvtKey.CurveType,
			EccParamsPtr->PvtKey.KeyLen, EccParamsPtr->PubKeyAddr);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(ResourceId, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_ENABLE_CAVP_SUPPORT */

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
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *	- XASUFW_INVALID_PARAM, if any argument is invalid.
 *
 *************************************************************************************************/
static s32 XAsufw_EcdhGenSharedSecret(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_EcdhParams *EcdhParamsPtr = (const XAsu_EcdhParams *)ReqBuf->Arg;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	XAsu_EccKeyObject PvtKeyObj;
	XAsu_EccKeyObject PubKeyObj;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_EcdhParams);

	/** Validate whether the curve type of the private key matches the public key's curve type. */
	if (EcdhParamsPtr->PvtKey.CurveType != EcdhParamsPtr->PubKey.CurveType) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Copy key object from request */
	PvtKeyObj = EcdhParamsPtr->PvtKey;
	PubKeyObj = EcdhParamsPtr->PubKey;

	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided key buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, PvtKeyObj.KeyAddr,
				   PvtKeyObj.KeyLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, PubKeyObj.KeyAddr,
				   XASUFW_DOUBLE_VALUE(PubKeyObj.KeyLen));

#ifdef XASU_KEYMANAGER_ENABLE
	if (PvtKeyObj.KeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_EccModule.AsuDmaPtr,
			PvtKeyObj.KeyId, (u64)(UINTPTR)&PvtKeyObj, SubSystemId,
			XASU_KEYMANAGER_ECC_PVT_KEY_AGREEMENT_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
			XASU_ECC_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}

	if (PubKeyObj.KeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_EccModule.AsuDmaPtr,
			PubKeyObj.KeyId, (u64)(UINTPTR)&PubKeyObj, SubSystemId,
			XASU_KEYMANAGER_ECC_PUB_KEY_AGREEMENT_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
			XASU_ECC_PUB_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

	/**
	 * Validate the client buffer receiving the shared secret. The secret is written either
	 * to the shared secret buffer or, when that buffer is not provided, as an object ID.
	 */
	if (EcdhParamsPtr->SharedSecretAddr != 0U) {
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId,
					   EcdhParamsPtr->SharedSecretAddr, PvtKeyObj.KeyLen);
	} else {
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId,
					   EcdhParamsPtr->SharedSecretObjIdAddr,
					   XASUFW_WORD_LEN_IN_BYTES);
	}

	/** Generate shared secret using public key and private key based on curve type. */
	Status = XRsa_EcdhGenSharedSecret(XAsufw_EccModule.AsuDmaPtr, PvtKeyObj.CurveType,
					PvtKeyObj.KeyLen, PvtKeyObj.KeyAddr,
					PubKeyObj.KeyAddr, EcdhParamsPtr->SharedSecretAddr,
					EcdhParamsPtr->SharedSecretObjIdAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECDH_GEN_SECRET_OPERATION_FAIL);
	}

END:
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
 * OCP and key manager commands drive the ECC core through the P-384 device identity key, so
 * XASUFW_ECC_RESOURCE_MASK checks for the availability of the ECC core for those modules.
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
	u32 ModuleId = ((ReqBuf->Header & XASU_MODULE_ID_MASK) >> XASU_MODULE_ID_SHIFT);
	u32 CurveType;
	XAsufw_Resource Resource = XASUFW_INVALID;

	/** Get CurveType based on command ID. */
	if ((CmdId == XASU_ECC_GEN_PUBKEY_CMD_ID) || (CmdId == XASU_ECC_VAL_PUBKEY_CMD_ID)) {
		CurveType = ((const XAsu_EccKeyParams *)ReqBuf->Arg)->PvtKey.CurveType;
	} else {
		CurveType = ((const XAsu_EccParams *)ReqBuf->Arg)->Key.CurveType;
	}

	if (ModuleId == XASU_MODULE_ECC_ID) {
		if (((CmdId == XASU_ECC_GEN_SIGNATURE_CMD_ID) ||
		    (CmdId == XASU_ECC_VERIFY_SIGNATURE_CMD_ID) ||
		    (CmdId == XASU_ECC_GEN_PUBKEY_CMD_ID) ||
		    (CmdId == XASU_ECC_VAL_PUBKEY_CMD_ID)) &&
		    ((CurveType == XASU_ECC_NIST_P256) ||
		    (CurveType == XASU_ECC_NIST_P384))) {
			Resource = XASUFW_ECC;
		} else if (CmdId == XASU_ECC_KAT_CMD_ID) {
			Resource = XASUFW_ECC;
		} else {
			Resource = XASUFW_RSA;
		}
	} else if ((ModuleId == XASU_MODULE_OCP_ID) || (ModuleId == XASU_MODULE_PROVISION_ID) ||
		   (ModuleId == XASU_MODULE_KEYMANAGER_ID)) {
		Resource = XASUFW_ECC;
	} else {
		Resource = XASUFW_INVALID;
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
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 CurveType;
	XAsufw_Resource Resource = XASUFW_INVALID;

	/** Get CurveType based on command ID. */
	if ((CmdId == XASU_ECC_GEN_PUBKEY_CMD_ID) || (CmdId == XASU_ECC_VAL_PUBKEY_CMD_ID)) {
		CurveType = ((const XAsu_EccKeyParams *)ReqBuf->Arg)->PvtKey.CurveType;
	} else {
		CurveType = ((const XAsu_EccParams *)ReqBuf->Arg)->Key.CurveType;
	}

	if ((CurveType == XASU_ECC_NIST_ED25519) ||
		(CurveType == XASU_ECC_NIST_ED25519_PH)) {
		Resource = XASUFW_SHA2;
	} else if ((CurveType == XASU_ECC_NIST_ED448) ||
		(CurveType == XASU_ECC_NIST_ED448_PH)) {
		Resource = XASUFW_SHA3;
	} else {
		Resource = XASUFW_NONE;
	}

	return Resource;
}
/** @} */
