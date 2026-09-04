/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_keywraphandler.c
*
* This file contains the key wrap unwrap module commands supported by ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   02/24/25 Initial release
*       rmv  08/17/26 Added validation of client provided buffers
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
#include "xasufw_keywraphandler.h"
#include "xkeywrap.h"
#include "xkeymanager.h"
#include "xaes_hw.h"
#include "xsha_hw.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_memvalidation.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xasufw_aeshandler.h"
#include "xasu_sharedmem.h"
#include "xil_sutil.h"
#include "xoverlay.h"
#include "xasufw_memory.h"

#ifdef XASU_KEYWRAP_ENABLE
/************************************ Function Prototypes ****************************************/
static s32 XAsufw_KeyWrapKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyWrapResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyWrap(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyUnwrap(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyWrapValidateBuffers(u32 SubsystemId,
					 const XAsu_KeyWrapParams *KeyWrapParams,
					 u32 ExpoCompSize, u32 KeyCompSize);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_KeyWrapModule; /**< ASUFW Key wrap unwp Module ID and commands array */

/************************************** Type Definitions *****************************************/

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the Key wrap unwrap module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if Key wrap unwrap module initialization is successful.
 * 	- XASUFW_KEYWRAP_MODULE_REGISTRATION_FAILED, if module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_KeyWrapInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * The XAsufw_KeyWrapCmds array contains the list of commands supported by Key wrap unwrap
	 * module.
	 */
	static const XAsufw_ModuleCmd XAsufw_KeyWrapCmds[XASU_KEYWRAP_MAX_CMDS] = {
		[XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyWrap),
		[XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyWrap),
		[XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyUnwrap),
		[XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyUnwrap),
		[XASU_KEYWRAP_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyWrapKat),
	};

	/** The XAsufw_KeyWrapResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_KeyWrapResourcesBuf[XASU_KEYWRAP_MAX_CMDS] = {
		[XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_KEYWRAP_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
	};

	/** The XAsufw_KeyWrapAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_KeyWrapAccessPermBuf[XASU_KEYWRAP_MAX_CMDS] = {
		[XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID),
		[XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID),
		[XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID),
		[XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID),
		[XASU_KEYWRAP_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KEYWRAP_KAT_CMD_ID),
	};

	XAsufw_KeyWrapModule.Id = XASU_MODULE_KEYWRAP_ID;
	XAsufw_KeyWrapModule.Cmds = XAsufw_KeyWrapCmds;
	XAsufw_KeyWrapModule.ResourcesRequired = XAsufw_KeyWrapResourcesBuf;
	XAsufw_KeyWrapModule.CmdCnt = XASU_KEYWRAP_MAX_CMDS;
	XAsufw_KeyWrapModule.ResourceHandler = XAsufw_KeyWrapResourceHandler;
	XAsufw_KeyWrapModule.AsuDmaPtr = NULL;
	XAsufw_KeyWrapModule.ShaPtr = NULL;
	XAsufw_KeyWrapModule.AesPtr = NULL;
	XAsufw_KeyWrapModule.AccessPermBufferPtr = XAsufw_KeyWrapAccessPermBuf;

	/** Register Key wrap unwrap module. */
	Status = XAsufw_ModuleRegister(&XAsufw_KeyWrapModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYWRAP_MODULE_REGISTRATION_FAILED;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for key wrap module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_RESOURCE_UNAVAILABLE, if the AES context cannot be saved yet, so that the
 * 	  scheduler re-queues the request.
 * 	- XASUFW_AES_CONTEXT_SAVE_FAIL, if AES context save fails.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_OVERLAY_ACTIVATE_FAIL, if overlay activation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyWrapResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ReqResources;

	/** Check and save the AES context if resource is not busy. */
	Status = XAsufw_AesCheckAndSaveContext(ReqId);
	if (Status == XASUFW_RESOURCE_UNAVAILABLE) {
		/** Propagate the deferral as-is so the scheduler re-queues the request. */
		goto END;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_CONTEXT_SAVE_FAIL);
		goto END;
	}

	/** Allocate resources for the module except for Get_Info command. */
	/** Allocate DMA resource. */
	XAsufw_KeyWrapModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_KEYWRAP, ReqId);
	if (XAsufw_KeyWrapModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	/** Allocate AES resource. */
	XAsufw_AllocateResource(XASUFW_AES, XASUFW_KEYWRAP, ReqId);
	XAsufw_KeyWrapModule.AesPtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);

	/** Allocate Key wrap resource. */
	XAsufw_AllocateResource(XASUFW_KEYWRAP, XASUFW_KEYWRAP, ReqId);

	/** Allocate overlay resource. */
	XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_KEYWRAP, ReqId);

	/** Allocate TRNG resource. */
	XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_KEYWRAP, ReqId);

	/** Allocate SHA2/SHA3 resource for commands which are dependent on SHA2/SHA3 HW. */
	ReqResources = XAsufw_KeyWrapModule.ResourcesRequired[CmdId];
	if ((ReqResources & XASUFW_SHA2_RESOURCE_MASK) == XASUFW_SHA2_RESOURCE_MASK) {
			XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_KEYWRAP, ReqId);
			XAsufw_KeyWrapModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	} else if ((ReqResources & XASUFW_SHA3_RESOURCE_MASK) == XASUFW_SHA3_RESOURCE_MASK) {
			XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_KEYWRAP, ReqId);
			XAsufw_KeyWrapModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	} else {
		/* Do nothing */
	}

	/** Activate RSA-ECC overlay. */
	Status = XOverlay_Activate(XOVERLAY_ALGO_RSA_ECC, XAsufw_KeyWrapModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		if (XAsufw_ReleaseResource(XASUFW_KEYWRAP, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the client provided buffers that the key wrap and unwrap
 *		operations share.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	KeyWrapParams	Pointer to the key wrap parameters.
 * @param	ExpoCompSize	Size of the pre-calculated exponentiation values in bytes.
 * @param	KeyCompSize	Size of the RSA key component structure in bytes.
 *
 * @return
 *	- XASUFW_SUCCESS, if all the provided buffers are accessible to the subsystem.
 *	- An error code from XAsufw_ValidateMemRegnAccess, if any buffer is not accessible.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyWrapValidateBuffers(u32 SubsystemId,
					 const XAsu_KeyWrapParams *KeyWrapParams,
					 u32 ExpoCompSize, u32 KeyCompSize)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the client provided data and label buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, KeyWrapParams->InputDataAddr,
				   KeyWrapParams->InputDataLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, KeyWrapParams->OutputDataAddr,
				   KeyWrapParams->OutputDataLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, KeyWrapParams->OptionalLabelAddr,
				   KeyWrapParams->OptionalLabelSize);

	/** Validate the client provided pre-calculated exponentiation values. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, KeyWrapParams->ExpoCompAddr,
				   ExpoCompSize);

	/** Validate the client provided RSA key buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, KeyWrapParams->KeyCompAddr,
				   KeyCompSize);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform key wrap operation using SHA2/SHA3
 * 		engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if operation is successful.
 * 	- XASUFW_KEYWRAP_GEN_WRAPPED_KEY_OPERATION_FAIL, if operation fails
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyWrap(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_KeyWrapParams KeyWrapParams;
	u32 *OutLenAddr;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	XAsu_AesKeyObject AesKeyObj;
	const XAsu_AesKeyObject *AesKeyObjPtr = NULL;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyWrapParams);

	/** Copy key wrap parameters from request buffer. */
	Status = Xil_SMemCpy(&KeyWrapParams, sizeof(XAsu_KeyWrapParams),
			ReqBuf->Arg, sizeof(XAsu_KeyWrapParams), sizeof(XAsu_KeyWrapParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided buffers. */
	Status = XAsufw_KeyWrapValidateBuffers(SubsystemId, &KeyWrapParams,
					       sizeof(XAsu_RsaRRModN),
					       sizeof(XAsu_RsaPubKeyComp));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	/** Resolve RSA public key from vault if key ID is provided. */
	if (KeyWrapParams.RsaKeyId != 0U) {
		KeyWrapParams.KeyCompAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK +
						XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_KeyWrapModule.AsuDmaPtr,
						KeyWrapParams.RsaKeyId,
						KeyWrapParams.KeyCompAddr, SubsystemId,
						XASU_KEYMANAGER_RSA_PUB_KEY_TRANSPORT_USE_CASE,
						XKEYMANAGER_RSA_OP_NONE,
						XASU_RSA_PUB_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
			goto END;
		}
		/* Clear KeyId since keyaddress is fetched. */
		KeyWrapParams.RsaKeyId = 0U;
	}

	/** Resolve AES key from vault if key ID is provided. */
	if (KeyWrapParams.AesKeyId != 0U) {
		AesKeyObj.KeyId = KeyWrapParams.AesKeyId;
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_KeyWrapModule.AsuDmaPtr,
						AesKeyObj.KeyId, (u64)(UINTPTR)&AesKeyObj,
						SubsystemId,
						XASU_KEYMANAGER_AES_KEY_WRAP_USE_CASE,
						XKEYMANAGER_RSA_OP_NONE,
						XASU_AES_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
			goto END;
		}
		AesKeyObjPtr = &AesKeyObj;
	}
#else
	(void)AesKeyObj;
#endif

	/** Perform Key wrap operation using given SHA crypto engine. */
	OutLenAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;
	Status = XKeyWrap(&KeyWrapParams, XAsufw_KeyWrapModule.AsuDmaPtr,
				XAsufw_KeyWrapModule.ShaPtr, XAsufw_KeyWrapModule.AesPtr,
				OutLenAddr, AesKeyObjPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYWRAP_GEN_WRAPPED_KEY_OPERATION_FAIL);
	}

END:
	XAsufw_KeyWrapModule.AsuDmaPtr = NULL;
	XAsufw_KeyWrapModule.ShaPtr = NULL;
	XAsufw_KeyWrapModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYWRAP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform key unwrap operation using SHA2/SHA3
 * 		engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if operation is successful.
 * 	- XASUFW_KEYWRAP_GEN_UNWRAPPED_KEY_OPERATION_FAIL, if operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyUnwrap(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_KeyWrapParams KeyWrapParams;
	u32 *OutLenAddr;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyWrapParams);

	/** Copy key unwrap parameters from request buffer. */
	Status = Xil_SMemCpy(&KeyWrapParams, sizeof(XAsu_KeyWrapParams),
			ReqBuf->Arg, sizeof(XAsu_KeyWrapParams), sizeof(XAsu_KeyWrapParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided buffers. */
	Status = XAsufw_KeyWrapValidateBuffers(SubsystemId, &KeyWrapParams,
					       sizeof(XAsu_RsaRModN),
					       sizeof(XAsu_RsaPvtKeyComp));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	/** Resolve RSA private key from vault if key ID is provided. */
	if (KeyWrapParams.RsaKeyId != 0U) {
		KeyWrapParams.KeyCompAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK +
						XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_KeyWrapModule.AsuDmaPtr,
						KeyWrapParams.RsaKeyId,
						KeyWrapParams.KeyCompAddr, SubsystemId,
						XASU_KEYMANAGER_RSA_PVT_KEY_TRANSPORT_USE_CASE,
						XKEYMANAGER_RSA_OP_NONCRT,
						XASU_RSA_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
			goto END;
		}
		/* Clear KeyId since keyaddress is fetched. */
		KeyWrapParams.RsaKeyId = 0U;
	}
#endif /* XASU_KEYMANAGER_ENABLE */

	/** Perform Key unwrap operation using given SHA crypto engine. */
	OutLenAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;
	Status = XKeyUnwrap(&KeyWrapParams, XAsufw_KeyWrapModule.AsuDmaPtr,
				XAsufw_KeyWrapModule.ShaPtr, XAsufw_KeyWrapModule.AesPtr,
				OutLenAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYWRAP_GEN_UNWRAPPED_KEY_OPERATION_FAIL);
	}

END:
	XAsufw_KeyWrapModule.AsuDmaPtr = NULL;
	XAsufw_KeyWrapModule.ShaPtr = NULL;
	XAsufw_KeyWrapModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYWRAP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key wrap unwrap KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- Error codes from XAsufw_KeyWrapOperationKat API, if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyWrapKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform Key wrap unwrap KAT. */
	Status = XAsufw_KeyWrapOperationKat(XAsufw_KeyWrapModule.AsuDmaPtr);

	XAsufw_KeyWrapModule.AsuDmaPtr = NULL;
	XAsufw_KeyWrapModule.ShaPtr = NULL;
	XAsufw_KeyWrapModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYWRAP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}
#endif /* XASU_KEYWRAP_ENABLE */
/** @} */
