/**************************************************************************************************
* Copyright (c) 2024 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_aeshandler.c
 *
 * This file contains the AES module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   08/01/24 Initial release
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *       am   01/20/25 Added AES CCM support
 *       am   04/03/25 Removed redundant release of AES resource
 *       am   04/14/25 Added support for DMA non-blocking wait
 * 1.2   am   05/18/25 Fixed implicit conversion of operands
 *       am   07/18/25 Added core reset support for single glitch recovery
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_aeshandler.h"
#include "xaes.h"
#include "xasufw_modules.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xfih.h"
#include "xasu_aes_common.h"
#include "xkeymanager.h"
#include "xasu_sharedmem.h"
#include "xasufw_memory.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XAES_NON_BLOCKING_CMD_STAGE_INIT	(0x0U) /**< Initial Command stage value for AES operations */
#define XAES_NON_BLOCKING_AAD_UPDATE_IN_PROGRESS	(0x1U) /**< AES AAD update in progress
							stage for DMA non-blocking wait */
#define XAES_NON_BLOCKING_DATA_UPDATE_INPROGRESS	(0x2U) /**< AES data update done stage for
							DMA non-blocking wait */
/************************************ Function Prototypes ****************************************/
static s32 XAsufw_AesOperation(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_AesKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_AesResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_AesModule; /**< ASUFW AES Module ID and commands array */
static u32 AesCtxSaveSupported = XASU_STATUS_FAIL; /**< AES context save supported (DDR reserved size check). */

/*************************************************************************************************/
/**
 * @brief	This function initializes the AES module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization of AES module is successful.
 * 	- XASUFW_AES_MODULE_REGISTRATION_FAILED, if AES module registration fails.
 * 	- XASUFW_AES_INIT_FAILED, if AES init fails.
 *
 *************************************************************************************************/
s32 XAsufw_AesInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XAes *XAsufw_Aes = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);

	/** The XAsufw_AesCmds array contains the list of commands supported by AES module. */
	static const XAsufw_ModuleCmd XAsufw_AesCmds[XASU_AES_MAX_CMDS] = {
		[XASU_AES_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_AesOperation),
		[XASU_AES_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_AesKat),
	};

	/** The XAsufw_AesResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_AesResourcesBuf[XASU_AES_MAX_CMDS] = {
		[XASU_AES_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK,
		[XASU_AES_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK,
	};

	/** The XAsufw_AesAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_AesAccessPermBuf[XASU_AES_MAX_CMDS] = {
		[XASU_AES_OPERATION_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_AES_OPERATION_CMD_ID),
		[XASU_AES_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_AES_KAT_CMD_ID),
	};

	XAsufw_AesModule.Id = XASU_MODULE_AES_ID;
	XAsufw_AesModule.Cmds = XAsufw_AesCmds;
	XAsufw_AesModule.ResourcesRequired = XAsufw_AesResourcesBuf;
	XAsufw_AesModule.CmdCnt = XASU_AES_MAX_CMDS;
	XAsufw_AesModule.ResourceHandler = XAsufw_AesResourceHandler;
	XAsufw_AesModule.AsuDmaPtr = NULL;
	XAsufw_AesModule.AccessPermBufferPtr = XAsufw_AesAccessPermBuf;

	/** Register AES module. */
	Status = XAsufw_ModuleRegister(&XAsufw_AesModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_MODULE_REGISTRATION_FAILED;
		goto END;
	}

	/** Initialize the AES crypto engine. */
	Status = XAes_CfgInitialize(XAsufw_Aes);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
	}

	/** Check if AES context can be saved successfully. */
	if (XAsufw_ReadReg(XASU_RTCA_DDR_SIZE_ADDR) >= XASUFW_DDR_RSVD_SIZE) {
		AesCtxSaveSupported = XASU_STATUS_PASS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for AES module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_AesResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Check and save the context if resource is not busy. */
	Status = XAsufw_AesCheckAndSaveContext(ReqId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_CONTEXT_SAVE_FAIL);
		goto END;
	}

	/** Allocate AES and DMA resources for AES operation and AES KAT commands. */
	XAsufw_AesModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_AES, ReqId);
	if (XAsufw_AesModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	XAsufw_AllocateResource(XASUFW_AES, XASUFW_AES, ReqId);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for AES operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if encryption/decryption operation is successful.
 * 	- XASUFW_AES_WRITE_KEY_FAILED, if Key write to AES USER key register fails.
 * 	- XASUFW_AES_INIT_FAILED, if initialization of AES engine fails.
 * 	- XASUFW_AES_UPDATE_FAILED, if update of data to AES engine fails.
 * 	- XASUFW_AES_FINAL_FAILED, if completion of final AES operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_AesOperation(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAes *XAsufw_Aes = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	const XAsu_AesParams *AesParamsPtr = (const XAsu_AesParams *)ReqBuf->Arg;
	u8 EngineMode = XASUFW_AES_INVALID_ENGINE_MODE;
	static u32 CmdStage = XAES_NON_BLOCKING_CMD_STAGE_INIT;
	u8 AadIsLast;
	u8 KeyUsecase = 0U;
	u64 LocalIvAddr = 0U;
	u32 LocalIvLen = 0U;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	XAsu_AesKeyObject KeyObject;
	XAsu_AesIvObject IvObject;

	/**
	 * If the DMA non-blocking transfer was initiated previously,
	 * - In case of AES AAD update is in progress:
	 *   - Jump to XAES_STAGE_DATA_UPDATE, to clear AAD configuration and proceed with
	 *     the data update.
	 * - In case of AES data update is in progress, jump to XAES_STAGE_DATA_UPDATE_DONE,
	 *   to complete the update.
	 */
	if (CmdStage == XAES_NON_BLOCKING_AAD_UPDATE_IN_PROGRESS) {
		goto XAES_STAGE_DATA_UPDATE;
	}
	else if (CmdStage == XAES_NON_BLOCKING_DATA_UPDATE_INPROGRESS) {
		goto XAES_STAGE_DATA_UPDATE_DONE;
	}

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_AesParams);

#ifdef XASU_KEYMANAGER_ENABLE
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}
#else
	(void)SubsystemId;
	(void)IpiMask;
#endif /* XASU_KEYMANAGER_ENABLE */

	/**
	 * For AES CCM mode, AadLen and DataLen in INIT encode the B0 block, so they must
	 * be the total sizes for the complete operation. INIT|UPDATE without FINAL is not
	 * allowed as AadLen/DataLen would be per-chunk sizes, producing an incorrect B0
	 * encoding. INIT|UPDATE|FINAL (single-shot) is allowed.
	 */
	if ((AesParamsPtr->EngineMode == XASU_AES_CCM_MODE) &&
		((AesParamsPtr->OperationFlags & XASU_INIT) == XASU_INIT) &&
		((AesParamsPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) &&
		((AesParamsPtr->OperationFlags & XASU_FINISH) != XASU_FINISH)) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_AES_CCM_INVALID_OPERATION_FLAGS);
		goto END;
	}
#ifdef XASU_KEYMANAGER_ENABLE
	if ((AesParamsPtr->EngineMode == XASU_AES_CMAC_MODE) || (AesParamsPtr->EngineMode == XASU_AES_GHASH_MODE)) {
		KeyUsecase = XASU_KEYMANAGER_AES_AUTH_USE_CASE;
	} else if (AesParamsPtr->OperationType == XASU_AES_ENCRYPT_OPERATION) {
		KeyUsecase = XASU_KEYMANAGER_AES_ENC_USE_CASE;
	} else if (AesParamsPtr->OperationType == XASU_AES_DECRYPT_OPERATION) {
		KeyUsecase = XASU_KEYMANAGER_AES_DEC_USE_CASE;
	} else {
		Status = XASUFW_AES_INVALID_OPERATION_TYPE;
		goto END;
	}
#endif /* XASU_KEYMANAGER_ENABLE */

	if (((AesParamsPtr->OperationFlags & XASU_INIT) == XASU_INIT) &&
	      (AesParamsPtr->EngineMode != XASU_AES_CMAC_MODE) &&
	      (AesParamsPtr->EngineMode != XASU_AES_ECB_MODE)) {
		LocalIvAddr = AesParamsPtr->IvAddr;
		LocalIvLen = AesParamsPtr->IvLen;

#ifdef XASU_KEYMANAGER_ENABLE
		if (AesParamsPtr->IvId != 0U) {
			IvObject.IvId = AesParamsPtr->IvId;
			Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_AesModule.AsuDmaPtr,
							IvObject.IvId, (u64)(UINTPTR)&IvObject,
							SubsystemId, KeyUsecase,
							XKEYMANAGER_RSA_OP_NONE);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
				goto END;
			}
			LocalIvAddr = IvObject.IvAddr;
			LocalIvLen = IvObject.IvLen;
		}
#endif /* XASU_KEYMANAGER_ENABLE */
	}

	if ((AesParamsPtr->OperationFlags & XASU_INIT) == XASU_INIT) {
		/** Copy KeyObject structure from 64-bit address space to local structure using ASU DMA. */
		Status = XAsufw_DmaXfr(XAsufw_AesModule.AsuDmaPtr, AesParamsPtr->KeyObjectAddr,
			(u64)(UINTPTR)&KeyObject, sizeof(XAsu_AesKeyObject), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}

#ifdef XASU_KEYMANAGER_ENABLE
		if (KeyObject.KeyId != 0U) {
			Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_AesModule.AsuDmaPtr,
							KeyObject.KeyId, (u64)(UINTPTR)&KeyObject,
							SubsystemId, KeyUsecase,
							XKEYMANAGER_RSA_OP_NONE);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
				goto END;
			}
		}
#endif /* XASU_KEYMANAGER_ENABLE */

		/** Write the given user key to the specified AES USER KEY register. */
		Status = XAes_WriteKey(XAsufw_Aes, XAsufw_AesModule.AsuDmaPtr, &KeyObject);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_WRITE_KEY_FAILED);
			goto END;
		}

		/** Initialize the AES engine for the given AES operation. */
		Status = XAes_Init(XAsufw_Aes, XAsufw_AesModule.AsuDmaPtr, LocalIvAddr,
				LocalIvLen, AesParamsPtr->EngineMode,
				AesParamsPtr->OperationType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
			goto END;
		}

		/** For AES CCM mode, format and transmit the CCM B0 header (nonce block and
		 * AAD length encoding) to the AES engine. This is a one-time initialization
		 * step; actual AAD data chunks are pushed via XAes_Update calls.
		 */
		if (AesParamsPtr->EngineMode == XASU_AES_CCM_MODE) {
			Status = XAes_CcmSendB0Header(XAsufw_Aes, XAsufw_AesModule.AsuDmaPtr,
					LocalIvAddr, (u8)LocalIvLen, AesParamsPtr->AadLen,
					AesParamsPtr->DataLen, (u8)AesParamsPtr->TagLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_AES_CCM_AAD_FORMATTING_FAILED);
				goto END;
			}
		}
	}

	if ((AesParamsPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) {
		/* Fetch the actual engine mode from the initialized AES instance */
		EngineMode = XAes_GetEngineMode(XAsufw_Aes);
		if (EngineMode == (u8)XASUFW_AES_INVALID_ENGINE_MODE) {
			Status = XASUFW_AES_INVALID_ENGINE_MODE;
			goto END;
		}

		/**
		 * Update AAD data to AES engine.
		 * User can push data in one go(entire AAD and Plaintext data at once) to the
		 * AES engine or in multiple chunks like, UPDATE(AAD data1), UPDATE(AAD data 2),
		 * UPDATE(Plaintext data 1) and so on...
		 * During single/multiple update of AAD data, address of AAD data will be non-zero.
		 * User should pass AAD address as zero for AES standard modes(CBC, ECB, CFB, OFB, CTR)
		 */
		if ((AesParamsPtr->AadAddr != 0U) &&
				((EngineMode == XASU_AES_GCM_MODE) ||
				 (EngineMode == XASU_AES_CMAC_MODE) ||
				 (EngineMode == XASU_AES_CCM_MODE))) {
			/**
			 * IsLast flag behaviour for AAD based on AES mode and data:
			 * For AES-GCM/CCM mode:
			 * (1) If only AAD is provided (no payload), IsLast should be set to true
			 *     as a part of last update of AAD.
			 * (2) If payload follows AAD, IsLast is not set during AAD update.
			 * For CMAC mode:
			 * IsLast is set for the last block of both AAD and Plaintext.
			 */
			AadIsLast = AesParamsPtr->IsLast;
			if (((EngineMode == XASU_AES_GCM_MODE) ||
					(EngineMode == XASU_AES_CCM_MODE)) &&
					(AesParamsPtr->InputDataAddr != 0U) &&
					(AesParamsPtr->DataLen != 0U)) {
				AadIsLast = XASU_FALSE;
			}
			Status = XAes_Update(XAsufw_Aes, XAsufw_AesModule.AsuDmaPtr, AesParamsPtr->AadAddr, 0U,
				AesParamsPtr->AadLen, AadIsLast);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				CmdStage = XAES_NON_BLOCKING_AAD_UPDATE_IN_PROGRESS;
				XAsufw_DmaCfgNonBlocking(XAsufw_AesModule.AsuDmaPtr,
							 XASUDMA_SRC_CHANNEL, ReqBuf, ReqId,
							 XASUFW_BLOCK_DMA);
				goto DONE;
			} else if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
				goto END;
			}
		}


XAES_STAGE_DATA_UPDATE:
		/**
		 * Update payload data to AES engine.
		 * Updates can be single updates or multiple chunks.
		 * For authentication only modes like CMAC, plaintext is not required.
		 */
		if ((AesParamsPtr->InputDataAddr != 0U) && (EngineMode != XASU_AES_CMAC_MODE)) {
			Status = XAes_Update(XAsufw_Aes, XAsufw_AesModule.AsuDmaPtr,
					AesParamsPtr->InputDataAddr, AesParamsPtr->OutputDataAddr,
					AesParamsPtr->DataLen, AesParamsPtr->IsLast);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				CmdStage = XAES_NON_BLOCKING_DATA_UPDATE_INPROGRESS;
				XAsufw_DmaCfgNonBlocking(XAsufw_AesModule.AsuDmaPtr,
							 XASUDMA_DST_CHANNEL, ReqBuf, ReqId,
							 XASUFW_BLOCK_DMA);
				goto DONE;
			} else if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
				goto END;
			}
		}
	}

XAES_STAGE_DATA_UPDATE_DONE:
	CmdStage = XAES_NON_BLOCKING_CMD_STAGE_INIT;

	if ((AesParamsPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) {
		/** Complete the AES operation. */
		Status = XAes_Final(XAsufw_Aes, XAsufw_AesModule.AsuDmaPtr, AesParamsPtr->TagAddr,
					AesParamsPtr->TagLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_FINAL_FAILED);
			goto END;
		}
	} else {
		if (XAsufw_AesModule.AsuDmaPtr != NULL) {
			Status = XAsufw_ReleaseDmaResource(XAsufw_AesModule.AsuDmaPtr, ReqId);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
			XAsufw_AesModule.AsuDmaPtr = NULL;
		}
		XAsufw_IdleResource(XASUFW_AES);
	}

END:
	/** Release the resource in the event of failure or after a successful final operation. */
	if ((Status != XASUFW_SUCCESS) || ((AesParamsPtr->OperationFlags & XASU_FINISH) ==
			XASU_FINISH)) {
		if (XAsufw_ReleaseResource(XASUFW_AES, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAes_SetReset(XAsufw_Aes);
		Status = XAsufw_UpdateErrorStatus(Status,
			XAes_KeyClear(XAsufw_Aes, XASU_AES_EXPANDED_KEYS));
		XAsufw_AesModule.AsuDmaPtr = NULL;
	}

DONE:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function executes AES Known Answer Tests (KATs) using the user-specified mode.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if AES KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 * 	- XASUFW_FAILURE, upon failure.
 *
 *************************************************************************************************/
static s32 XAsufw_AesKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const u32 AesKatMode = *((const u32 *)ReqBuf->Arg);

	/** Run KAT based on user specified KAT mode. */
	switch (AesKatMode) {
		case XASU_AES_ECB_MODE:
		case  XASU_AES_GCM_MODE:
			Status = XAsufw_AesOperationKat(XAsufw_AesModule.AsuDmaPtr, (u8)AesKatMode);
			break;
#ifdef XASU_AES_CM_ENABLE
		case XASU_AES_KAT_DPA_MODE:
			Status = XAsufw_AesDpaCmKat(XAsufw_AesModule.AsuDmaPtr);
			break;
#endif
		default:
			Status = XASUFW_AES_INVALID_ENGINE_MODE;
			break;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_AES, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_AesModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks if AES resource is busy and saves the intermediate context
 * 		if required.
 *
 * @param        ReqId		Request Unique ID for resource tracking.
 *
 * @return
 * 	- XASUFW_SUCCESS, if context save is successful or not required.
 * 	- XASUFW_RESOURCE_UNAVAILABLE, if resource is busy in ECB mode.
 * 	- XASUFW_FAILURE, if context save operation fails.
 *
 *************************************************************************************************/
s32 XAsufw_AesCheckAndSaveContext(u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAes *AesInstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	XAes_ContextInfo *Ctx = NULL;
	u8 EngineMode = XAes_GetEngineMode(AesInstancePtr);

	if (XAes_GetContextDdrPtr(&Ctx) == XASUFW_SUCCESS) {
		if (!(XAsufw_IsResourceBusy(XASUFW_AES, ReqId)) && (Ctx->IsContextSaved != XASU_TRUE) &&
				(XAes_GetAndValidateInternalState(AesInstancePtr))) {
			/** AES-ECB and AES-CCM mode doesn't support context switching. */
			if ((EngineMode == XASU_AES_ECB_MODE) ||
					(EngineMode == XASU_AES_CCM_MODE)) {
				Status = XASUFW_RESOURCE_UNAVAILABLE;
				goto END;
			}

			Status = XAes_SaveContext(AesInstancePtr);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
		}
		else {
			Ctx->ReqId = ReqId;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Check if AES context save is supported.
 *
 * @return
 * 	- XASU_STATUS_PASS, if AES context save is supported.
 * 	- XASU_STATUS_FAIL, if AES context save is not supported.
 *
 *************************************************************************************************/
u32 XAsufw_IsAesContextSaveSupported(void)
{
	return AesCtxSaveSupported;
}
/** @} */
