/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_hmachandler.c
*
* This file contains the HMAC module commands supported by ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog 01/02/25 Initial release
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
#include "xasufw_hmachandler.h"
#include "xhmac.h"
#include "xsha_hw.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xkeymanager.h"
#include "xasu_sharedmem.h"

#ifdef XASU_HMAC_ENABLE
/************************************ Function Prototypes ****************************************/
static s32 XAsufw_HmacKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_HmacResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_HmacComputeSha(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_HmacModule; /**< ASUFW HMAC Module ID and commands array */

/************************************** Type Definitions *****************************************/

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the HMAC module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if HMAC module initialization is successful.
 * 	- XASUFW_HMAC_MODULE_REGISTRATION_FAILED, if HMAC module registration fails.
 * 	- XASUFW_HMAC_INIT_FAILED, if HMAC init fails.
 *
 *************************************************************************************************/
s32 XAsufw_HmacInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XHmac *HmacPtr = XHmac_GetInstance();

	/** The XAsufw_HmacCmds array contains the list of commands supported by HMAC module. */
	static const XAsufw_ModuleCmd XAsufw_HmacCmds[XASU_HMAC_MAX_CMDS] = {
		[XASU_HMAC_COMPUTE_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HmacComputeSha),
		[XASU_HMAC_COMPUTE_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HmacComputeSha),
		[XASU_HMAC_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HmacKat),
	};

	/** The XAsufw_HmacResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_HmacResourcesBuf[XASU_HMAC_MAX_CMDS] = {
		[XASU_HMAC_COMPUTE_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
		[XASU_HMAC_COMPUTE_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_HMAC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK,
	};

	/** The XAsufw_HmacAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_HmacAccessPermBuf[XASU_HMAC_MAX_CMDS] = {
		[XASU_HMAC_COMPUTE_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_HMAC_COMPUTE_SHA2_CMD_ID),
		[XASU_HMAC_COMPUTE_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_HMAC_COMPUTE_SHA3_CMD_ID),
		[XASU_HMAC_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_HMAC_KAT_CMD_ID),
	};

	XAsufw_HmacModule.Id = XASU_MODULE_HMAC_ID;
	XAsufw_HmacModule.Cmds = XAsufw_HmacCmds;
	XAsufw_HmacModule.ResourcesRequired = XAsufw_HmacResourcesBuf;
	XAsufw_HmacModule.CmdCnt = XASU_HMAC_MAX_CMDS;
	XAsufw_HmacModule.ResourceHandler = XAsufw_HmacResourceHandler;
	XAsufw_HmacModule.AsuDmaPtr = NULL;
	XAsufw_HmacModule.ShaPtr = NULL;
	XAsufw_HmacModule.AccessPermBufferPtr = XAsufw_HmacAccessPermBuf;

	/** Register HMAC module. */
	Status = XAsufw_ModuleRegister(&XAsufw_HmacModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_HMAC_MODULE_REGISTRATION_FAILED;
		goto END;
	}

	/** Initialize HMAC instance. */
	Status = XHmac_CfgInitialize(HmacPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_INIT_FAILED);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for HMAC module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_HmacResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	if ((CmdId == XASU_HMAC_COMPUTE_SHA2_CMD_ID) || (CmdId == XASU_HMAC_KAT_CMD_ID)) {
		ResourceId = XASUFW_SHA2;
		XAsufw_HmacModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	} else {
		ResourceId = XASUFW_SHA3;
		XAsufw_HmacModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	}

	/**
	 * Allocate DMA, HMAC and SHA2/3 resource based on Command ID.
	 */
	XAsufw_HmacModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_HMAC, ReqId);
	if (XAsufw_HmacModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}
	XAsufw_AllocateResource(XASUFW_HMAC, XASUFW_HMAC, ReqId);
	XAsufw_AllocateResource(ResourceId, XASUFW_HMAC, ReqId);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform HMAC operation using SHA2/SHA3 engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if SHA2 hash operation is successful.
 * 	- XASUFW_FAILURE, in case of failure.
 * 	- XASUFW_SHA_INVALID_SHA_MODE, if input sha mode is invalid
 * 	- XASUFW_HMAC_INITIALIZATION_FAILED, if HMAC initialisation fails.
 * 	- XASUFW_HMAC_UPDATE_FAILED, if HMAC update fails.
 * 	- XASUFW_HMAC_FINAL_FAILED, if HMAC final fails.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_CMD_IN_PROGRESS, if HMAC update is in progress(non-blocking).
 * 	- XASUFW_INVALID_CMD_STAGE, if command stage is invalid.
 *
 *************************************************************************************************/
static s32 XAsufw_HmacComputeSha(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_HmacParams *HmacParamsPtr = (const XAsu_HmacParams *)ReqBuf->Arg;
	XHmac *HmacPtr = XHmac_GetInstance();
	u32 *ResponseBufferPtr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
				 XASUFW_RESP_DATA_OFFSET;
	static u32 HmacCmdStage = XHMAC_NON_BLOCKING_CMD_STAGE_INIT;
	XAsu_KdfHmacKeyObject KeyObject;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u32 SubsystemId = 0U;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_HmacParams);

	KeyObject = HmacParamsPtr->KeyObject;

	switch (HmacCmdStage) {
	case XHMAC_NON_BLOCKING_CMD_STAGE_INIT:
		if ((HmacParamsPtr->OperationFlags & XASU_INIT) == XASU_INIT) {
#ifdef XASU_KEYMANAGER_ENABLE
			/** Get subsystem ID from IPI mask. */
			SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
			if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
				Status = XASUFW_INVALID_SUBSYSTEM_ID;
				goto END;
			}

			if (HmacParamsPtr->KeyObject.KeyId != 0U) {
				Status = XKeyManager_UpdateKeyObjFromVault(
						XAsufw_HmacModule.AsuDmaPtr,
						HmacParamsPtr->KeyObject.KeyId,
						(u64)(UINTPTR)&KeyObject,
						SubsystemId,
						XKEYMANAGER_KDF_HMAC_HMAC_USE_CASE,
						XKEYMANAGER_RSA_OP_NONE);
				if (Status != XASUFW_SUCCESS) {
					Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
					goto END;
				}
			}
#else
	(void)SubsystemId;
	(void)IpiMask;
#endif /* XASU_KEYMANAGER_ENABLE */

			/**
			 * Else, use the direct key address provided by the caller.
			 * Perform HMAC init with the resolved or direct key.
			 */
			Status = XHmac_Init(HmacPtr, XAsufw_HmacModule.AsuDmaPtr,
					XAsufw_HmacModule.ShaPtr, KeyObject.KeyInAddr,
					KeyObject.KeyInLen, HmacParamsPtr->ShaMode,
					HmacParamsPtr->HmacLen);
			if (Status != XST_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_HMAC_INITIALIZATION_FAILED);
				goto END;
			}
		}
		/* fall through */
	case XHMAC_NON_BLOCKING_CMD_STAGE_UPDATE_IN_PROGRESS:
		HmacCmdStage = XHMAC_NON_BLOCKING_CMD_STAGE_INIT;
		if ((HmacParamsPtr->OperationFlags & XASU_UPDATE) == XASU_UPDATE) {
			/**
			 * If operation flags include HMAC UPDATE, perform HMAC update operation.
			 */
			Status = XHmac_Update(HmacPtr, XAsufw_HmacModule.AsuDmaPtr,
					HmacParamsPtr->MsgBufferAddr, HmacParamsPtr->MsgLen,
					(u32)HmacParamsPtr->IsLast);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				HmacCmdStage = XHMAC_NON_BLOCKING_CMD_STAGE_UPDATE_IN_PROGRESS;
				XAsufw_DmaCfgNonBlocking(XAsufw_HmacModule.AsuDmaPtr,
							 XASUDMA_SRC_CHANNEL, ReqBuf, ReqId,
							 XASUFW_BLOCK_DMA);
				break;
			} else if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
								  XASUFW_HMAC_UPDATE_FAILED);
				goto END;
			} else {
				/* Do nothing */
			}
		}

		if ((HmacParamsPtr->OperationFlags & XASU_FINISH) == XASU_FINISH) {
			/** If operation flags include HMAC FINAL, perform HMAC final operation. */
			Status = XHmac_Final(HmacPtr, XAsufw_HmacModule.AsuDmaPtr,
					     ResponseBufferPtr);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_FINAL_FAILED);
				goto END;
			}
		} else {
			if (XAsufw_HmacModule.AsuDmaPtr != NULL) {
				/**
				 * If HMAC_FINAL is not set in operation flags, release DMA resource
				 * and idle resources allocated for HMAC operation.
				 */
				Status = XAsufw_ReleaseDmaResource(XAsufw_HmacModule.AsuDmaPtr,
								   ReqId);
				if (Status != XASUFW_SUCCESS) {
					Status = XAsufw_UpdateErrorStatus(Status,
							XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
				}
			}
			XAsufw_IdleResource(XASUFW_HMAC);
		}
		break;
	default:
		Status = XASUFW_INVALID_CMD_STAGE;
		break;
	}

END:
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		XAsufw_HmacModule.AsuDmaPtr = NULL;
		if ((Status != XASUFW_SUCCESS) ||
		    ((HmacParamsPtr->OperationFlags & XASU_FINISH) == XASU_FINISH)) {
			/**
			 * Release resources upon any failure or after HMAC operation is complete.
			 */
			if (XAsufw_ReleaseResource(XASUFW_HMAC, ReqId) != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for HMAC KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code, returned when XAsufw_HmacOperationKat API fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_HmacKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform HMAC KAT with known inputs. */
	Status = XAsufw_HmacOperationKat(XAsufw_HmacModule.AsuDmaPtr);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_HMAC, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_HmacModule.AsuDmaPtr = NULL;
	XAsufw_HmacModule.ShaPtr = NULL;

	return Status;
}
#endif  /* XASU_HMAC_ENABLE */
/** @} */
