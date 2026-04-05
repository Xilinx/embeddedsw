/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_shahandler_common.c
 *
 * This file contains common SHA functions for SHA2 and SHA3 handler modules in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  04/01/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_shahandler_common.h"
#include "xasufw_status.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xasufw_util.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xasu_shainfo.h"
#include "xasufw_dma.h"
#include "xasufw_queuescheduler.h"
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XAsufw_Resource XAsufw_GetShaResourceId(u32 ModuleId);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function returns the SHA resource ID for the given module ID.
 *
 * @param	ModuleId	SHA module identifier.
 *
 * @return
 * 	- XASUFW_SHA2, if module ID is XASU_MODULE_SHA2_ID.
 * 	- XASUFW_SHA3, if module ID is XASU_MODULE_SHA3_ID.
 * 	- XASUFW_INVALID, if module ID is invalid.
 *
 *************************************************************************************************/
static XAsufw_Resource XAsufw_GetShaResourceId(u32 ModuleId)
{
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	if (ModuleId == XASU_MODULE_SHA2_ID) {
		ResourceId = XASUFW_SHA2;
	} else if (ModuleId == XASU_MODULE_SHA3_ID) {
		ResourceId = XASUFW_SHA3;
	} else {
		/* Do nothing. */
	}

	return ResourceId;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler for SHA operation command for SHA2
 *		and SHA3 handler modules.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if SHA hash operation is successful.
 * 	- XASUFW_FAILURE, in case of failure.
 * 	- XASUFW_INVALID_PARAM, if invalid parameter is passed.
 * 	- XASUFW_RESOURCE_INVALID, if resource is invalid.
 * 	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, if module is not registered.
 * 	- XASUFW_SHA_START_FAILED, if SHA start fails.
 * 	- XASUFW_SHA_UPDATE_FAIL, if SHA update fails.
 * 	- XASUFW_SHA_FINISH_FAILED, if SHA finish fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_CMD_IN_PROGRESS, if SHA operation is in progress(non-blocking).
 * 	- XASUFW_INVALID_CMD_STAGE, if command stage is invalid.
 *
 *************************************************************************************************/
s32 XAsufw_ShaOperation(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_ShaOperationCmd *Cmd = (const XAsu_ShaOperationCmd *)ReqBuf->Arg;
	u32 ModuleId = (ReqBuf->Header & XASU_MODULE_ID_MASK) >> XASU_MODULE_ID_SHIFT;
	XAsufw_Module *ShaModulePtr = NULL;
	XAsufw_ShaContext *ShaCtxPtr = NULL;
	XSha *ShaInstancePtr = NULL;
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	/** Get SHA module. */
	ShaModulePtr = XAsufw_GetModule(ModuleId);
	if (ShaModulePtr == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	ShaInstancePtr = ShaModulePtr->ShaPtr;
	if (ShaInstancePtr == NULL) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Derive SHA context from module. */
	ShaCtxPtr = XAsufw_GetShaContext(ShaModulePtr, XAsufw_ShaContext, Module);
	if (ShaCtxPtr == NULL) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Get SHA resource ID. */
	ResourceId = XAsufw_GetShaResourceId(ShaModulePtr->Id);
	if ((ResourceId != XASUFW_SHA2) && (ResourceId != XASUFW_SHA3)) {
		Status = XASUFW_RESOURCE_INVALID;
		goto END;
	}

	switch (ShaCtxPtr->CmdStage) {
	case XSHA_NON_BLOCKING_CMD_STAGE_INIT:
		if ((Cmd->OperationFlags & XASU_SHA_START) == XASU_SHA_START) {
			/** If operation flags include SHA START, perform SHA start operation. */
			Status = XSha_Start(ShaInstancePtr, Cmd->ShaMode);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_START_FAILED);
				goto END;
			}
		}

		/** If operation flags include SHA UPDATE perform SHA update operation. */
		if ((Cmd->OperationFlags & XASU_SHA_UPDATE) == XASU_SHA_UPDATE) {
			Status = XSha_Update(ShaInstancePtr, ShaModulePtr->AsuDmaPtr,
					     Cmd->DataAddr, Cmd->DataSize, Cmd->IsLast);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				ShaCtxPtr->CmdStage = XSHA_NON_BLOCKING_CMD_STAGE_UPDATE_IN_PROGRESS;
				XAsufw_DmaCfgNonBlocking(ShaModulePtr->AsuDmaPtr,
					XASUDMA_SRC_CHANNEL, ReqBuf, ReqId, XASUFW_BLOCK_DMA);
				break;
			} else if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_UPDATE_FAIL);
				goto END;
			} else {
				/* Do nothing */
			}
		}
		/* fall through */
	case XSHA_NON_BLOCKING_CMD_STAGE_UPDATE_IN_PROGRESS:
		ShaCtxPtr->CmdStage = XSHA_NON_BLOCKING_CMD_STAGE_INIT;
		if ((Cmd->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) {
			/** If operation flags include SHA FINISH, perform SHA finish operation. */
			Status = XSha_Finish(ShaInstancePtr, ShaModulePtr->AsuDmaPtr,
					Cmd->HashAddr, Cmd->HashBufSize, XASU_FALSE);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
								  XASUFW_SHA_FINISH_FAILED);
				goto END;
			}
		} else {
			if (ShaModulePtr->AsuDmaPtr != NULL) {
				/**
				 * If SHA_FINISH is not set in operation flags and SHA update is
				 * complete, release DMA resource and Idle SHA resource.
				 */
				Status = XAsufw_ReleaseDmaResource(ShaModulePtr->AsuDmaPtr,
						ReqId);
				if (Status != XASUFW_SUCCESS) {
					Status = XAsufw_UpdateErrorStatus(Status,
							XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
				}
				ShaModulePtr->AsuDmaPtr = NULL;
			}
			XAsufw_IdleResource(ResourceId);
		}
		break;
	default:
		Status = XASUFW_INVALID_CMD_STAGE;
		break;
	}

END:
	if (((Status != XASUFW_SUCCESS) ||
	      ((Cmd->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH)) &&
	     (Status != XASUFW_CMD_IN_PROGRESS)) {
		/** Release resources. */
		if ((ResourceId != XASUFW_INVALID) &&
		    (XAsufw_ReleaseResource(ResourceId, ReqId) != XASUFW_SUCCESS)) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		/** Clear DMA pointer. */
		if (ShaModulePtr != NULL) {
			ShaModulePtr->AsuDmaPtr = NULL;
		}
		/** Reset SHA instance. */
		XSha_Reset(ShaInstancePtr);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler for SHA KAT command for SHA2 and
 *		SHA3 handler modules.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, if module is not registered.
 * 	- XASUFW_RESOURCE_INVALID, if resource is invalid.
 * 	- XASUFW_SHA_INVALID_SHA_MODE, if SHA mode is invalid for SHA2 KAT.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error code from XAsufw_ShaKat API, if any operation fails.
 *
 *************************************************************************************************/
s32 XAsufw_ShaKatOperation(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 ModuleId = (ReqBuf->Header & XASU_MODULE_ID_MASK) >> XASU_MODULE_ID_SHIFT;
	XAsufw_Module *ShaModulePtr = NULL;
	XSha *ShaInstancePtr = NULL;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 ShaMode = 0U;

	/** Get SHA module. */
	ShaModulePtr = XAsufw_GetModule(ModuleId);
	if (ShaModulePtr == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	ShaInstancePtr = ShaModulePtr->ShaPtr;
	if (ShaInstancePtr == NULL) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Get SHA resource ID. */
	ResourceId = XAsufw_GetShaResourceId(ShaModulePtr->Id);
	if ((ResourceId != XASUFW_SHA2) && (ResourceId != XASUFW_SHA3)) {
		Status = XASUFW_RESOURCE_INVALID;
		goto END;
	}

	/** Validate SHA mode. */
	if (ModuleId == XASU_MODULE_SHA2_ID) {
		ShaMode = *((u32 *)ReqBuf->Arg);
		if ((ShaMode != XASU_SHA_MODE_256) && (ShaMode != XASU_SHA_MODE_512)) {
			Status = XASUFW_SHA_INVALID_SHA_MODE;
			goto END;
		}
	} else if (ModuleId == XASU_MODULE_SHA3_ID) {
		ShaMode = XASU_SHA_MODE_256;
	} else {
		/* Do nothing. */
	}

	/** Perform SHA KAT. */
	Status = XAsufw_ShaKat(ShaInstancePtr, ShaModulePtr->AsuDmaPtr, ResourceId, ShaMode);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(ResourceId, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	if (ShaModulePtr != NULL) {
		ShaModulePtr->AsuDmaPtr = NULL;
	}

	return Status;
}
/** @} */
