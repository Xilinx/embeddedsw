/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_kdfhandler.c
*
* This file contains the KDF module commands supported by ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ma  01/15/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_kdfhandler.h"
#include "xkdf.h"
#include "xasufw_status.h"
#include "xasufw_resourcemanager.h"
#include "xsha_hw.h"
#include "xasufw_util.h"
#include "xasufw_kat.h"
#include "xkeymanager.h"
#include "xasu_sharedmem.h"
#include "xil_sutil.h"
#include "xasufw_aeshandler.h"
#ifdef XASU_ENABLE_CAVP_SUPPORT
#include "xhkdf.h"
#endif /* XASU_ENABLE_CAVP_SUPPORT */

#ifdef XASU_KDF_ENABLE
/************************************ Function Prototypes ****************************************/
static s32 XAsufw_KdfResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KdfGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KdfKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#ifdef XASU_ENABLE_CAVP_SUPPORT
static s32 XAsufw_CmacKdfGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_HkdfGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif /* XASU_ENABLE_CAVP_SUPPORT */

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_KdfModule; /**< ASUFW KDF Module ID and commands array */

/************************************** Type Definitions *****************************************/

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the KDF module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KDF module initialization is successful.
 * 	- XASUFW_KDF_MODULE_REGISTRATION_FAILED, if KDF module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_KdfInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_KdfCmds array contains the list of commands supported by KDF module. */
	static const XAsufw_ModuleCmd XAsufw_KdfCmds[XASU_KDF_MAX_CMDS] = {
		[XASU_KDF_GENERATE_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KdfGenerate),
		[XASU_KDF_GENERATE_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KdfGenerate),
		[XASU_KDF_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KdfKat),
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_KDF_CMAC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_CmacKdfGenerate),
		[XASU_KDF_HKDF_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HkdfGenerate),
		[XASU_KDF_HKDF_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HkdfGenerate),
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	/** The XAsufw_KdfResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_KdfResourcesBuf[XASU_KDF_MAX_CMDS] = {
		[XASU_KDF_GENERATE_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK,
		[XASU_KDF_GENERATE_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK,
		[XASU_KDF_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK,
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_KDF_CMAC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK |
		XASUFW_KDF_RESOURCE_MASK,
		[XASU_KDF_HKDF_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK,
		[XASU_KDF_HKDF_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK |
		XASUFW_SHA3_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK,
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	/** The XAsufw_KdfAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_KdfAccessPermBuf[XASU_KDF_MAX_CMDS] = {
		[XASU_KDF_GENERATE_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KDF_GENERATE_SHA2_CMD_ID),
		[XASU_KDF_GENERATE_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KDF_GENERATE_SHA3_CMD_ID),
		[XASU_KDF_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KDF_KAT_CMD_ID),
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_KDF_CMAC_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KDF_CMAC_CMD_ID),
		[XASU_KDF_HKDF_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KDF_HKDF_SHA2_CMD_ID),
		[XASU_KDF_HKDF_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KDF_HKDF_SHA3_CMD_ID),
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	/** Initialize the KDF module structure. */
	XAsufw_KdfModule.Id = XASU_MODULE_KDF_ID;
	XAsufw_KdfModule.Cmds = XAsufw_KdfCmds;
	XAsufw_KdfModule.ResourcesRequired = XAsufw_KdfResourcesBuf;
	XAsufw_KdfModule.CmdCnt = XASU_KDF_MAX_CMDS;
	XAsufw_KdfModule.ResourceHandler = XAsufw_KdfResourceHandler;
	XAsufw_KdfModule.AsuDmaPtr = NULL;
	XAsufw_KdfModule.ShaPtr = NULL;
	XAsufw_KdfModule.AccessPermBufferPtr = XAsufw_KdfAccessPermBuf;

	/** Register KDF module. */
	Status = XAsufw_ModuleRegister(&XAsufw_KdfModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KDF_MODULE_REGISTRATION_FAILED;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for KDF module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_AES_CONTEXT_SAVE_FAIL, if AES context save fails (CMAC-KDF only).
 *
 *************************************************************************************************/
static s32 XAsufw_KdfResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	if ((CmdId == XASU_KDF_GENERATE_SHA2_CMD_ID) || (CmdId == XASU_KDF_KAT_CMD_ID) ||
	    (CmdId == XASU_KDF_HKDF_SHA2_CMD_ID)) {
		ResourceId = XASUFW_SHA2;
		XAsufw_KdfModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	} else if ((CmdId == XASU_KDF_GENERATE_SHA3_CMD_ID) || (CmdId == XASU_KDF_HKDF_SHA3_CMD_ID)) {
		ResourceId = XASUFW_SHA3;
		XAsufw_KdfModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	} else if (CmdId == XASU_KDF_CMAC_CMD_ID) {
		ResourceId = XASUFW_AES;
		/** Check and save the AES context if resource is not busy. */
		Status = XAsufw_AesCheckAndSaveContext(ReqId);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_CONTEXT_SAVE_FAIL);
			goto END;
		}
		XAsufw_KdfModule.AesPtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	} else {
		/* Do Nothing */
	}

	/**
	 * Allocate DMA resource.
	 */
	XAsufw_KdfModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_KDF, ReqId);
	if (XAsufw_KdfModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	/**
	 * Allocate HMAC and SHA2/3 resources for KDF and HKDF commands, or
	 * AES resource for CMAC-KDF command, based on ResourceId.
	 */
	if ((ResourceId == XASUFW_SHA2) || (ResourceId == XASUFW_SHA3)) {
		XAsufw_AllocateResource(XASUFW_HMAC, XASUFW_KDF, ReqId);
	}
	XAsufw_AllocateResource(ResourceId, XASUFW_KDF, ReqId);
	XAsufw_AllocateResource(XASUFW_KDF, XASUFW_KDF, ReqId);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform KDF operation using SHA2/SHA3 engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KDF operation is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error codes from XKdf_Generate API, if any failure occurs.
 *
 *************************************************************************************************/
static s32 XAsufw_KdfGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_KdfParams KdfParams;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u32 SubsystemId = 0U;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KdfParams);

	/** Copy KDF parameters from request buffer. */
	Status = Xil_SMemCpy(&KdfParams, sizeof(XAsu_KdfParams),
			ReqBuf->Arg, sizeof(XAsu_KdfParams), sizeof(XAsu_KdfParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** If key vault ID is provided, resolve key object from vault. */
	if (KdfParams.KeyObject.KeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_KdfModule.AsuDmaPtr,
				KdfParams.KeyObject.KeyId, (u64)(UINTPTR)&KdfParams.KeyObject,
				SubsystemId,
				XASU_KEYMANAGER_KDF_HMAC_KDF_USE_CASE,
				XKEYMANAGER_RSA_OP_NONE);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
			goto END;
		}
		/* Clear KeyId since keyaddress is fetched. */
		KdfParams.KeyObject.KeyId = 0U;
	}
#else
	(void)SubsystemId;
	(void)IpiMask;
#endif /* XASU_KEYMANAGER_ENABLE */

	/**
	 * Else, use the direct key address provided by the caller.
	 * Perform KDF generate with the resolved or direct key.
	 */
	Status = XKdf_Generate(XAsufw_KdfModule.AsuDmaPtr, XAsufw_KdfModule.ShaPtr, &KdfParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_GENERATE_FAILED);
	}

END:
	/** Clear DMA and SHA pointers. */
	XAsufw_KdfModule.AsuDmaPtr = NULL;
	XAsufw_KdfModule.ShaPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KDF, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for KDF KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error codes from XAsufw_KdfOperationKat API, if any failure occurs.
 *
 *************************************************************************************************/
static s32 XAsufw_KdfKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform KDF KAT. */
	Status = XAsufw_KdfOperationKat(XAsufw_KdfModule.AsuDmaPtr);

	/** Clear DMA and SHA pointers. */
	XAsufw_KdfModule.AsuDmaPtr = NULL;
	XAsufw_KdfModule.ShaPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KDF, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

#ifdef XASU_ENABLE_CAVP_SUPPORT
/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform CMAC-KDF operation using AES engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KDF operation is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_INVALID_PARAM, if any argument is invalid.
 * 	- Error codes from XKdf_CmacGenerate API, if any failure occurs.
 *
 *************************************************************************************************/
static s32 XAsufw_CmacKdfGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_CmacKdfParams *CmacKdfParams = (const XAsu_CmacKdfParams *)ReqBuf->Arg;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_CmacKdfParams);

	/** Perform KDF generate. */
	Status = XKdf_CmacGenerate(XAsufw_KdfModule.AsuDmaPtr, XAsufw_KdfModule.AesPtr,
			&CmacKdfParams->KdfParams, CmacKdfParams->AesKeySrc);

END:
	/** Clear DMA and AES pointers. */
	XAsufw_KdfModule.AsuDmaPtr = NULL;
	XAsufw_KdfModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KDF, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform HKDF operation using SHA2/SHA3 engine
 * 		as per RFC 5869.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KDF operation is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_INVALID_PARAM, if any argument is invalid.
 * 	- Error codes from XHkdf_Generate API, if any failure occurs.
 *
 *************************************************************************************************/
static s32 XAsufw_HkdfGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_HkdfParams *HKdfParams = (const XAsu_HkdfParams *)ReqBuf->Arg;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_HkdfParams);

	/** Perform HKDF generate. */
	Status = XHkdf_Generate(XAsufw_KdfModule.AsuDmaPtr, XAsufw_KdfModule.ShaPtr, HKdfParams);

END:
	/** Clear DMA and SHA pointers. */
	XAsufw_KdfModule.AsuDmaPtr = NULL;
	XAsufw_KdfModule.ShaPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KDF, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}
#endif /* XASU_ENABLE_CAVP_SUPPORT */
#endif /* XASU_KDF_ENABLE */
/** @} */
