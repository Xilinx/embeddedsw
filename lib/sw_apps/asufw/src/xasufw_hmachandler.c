/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* @addtogroup xasufw_application ASUFW Functionality
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
#include "xasu_hmacinfo.h"

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_HmacKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_HmacGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
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
 * 	- On successful initialization of HMAC module, it returns XASUFW_SUCCESS.
 * 	- XASUFW_HMAC_MODULE_REGISTRATION_FAILED, if HMAC module registration fails.
 * 	- XASUFW_HMAC_INIT_FAILED, if HMAC init fails.
 *
 *************************************************************************************************/
s32 XAsufw_HmacInit(void)
{
	volatile s32 Status = XASUFW_FAILURE;
	XHmac *HmacPtr = XHmac_GetInstance();

	/** Contains the array of ASUFW HMAC commands. */
	static const XAsufw_ModuleCmd XAsufw_HmacCmds[] = {
		[XASU_HMAC_COMPUTE_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HmacComputeSha),
		[XASU_HMAC_COMPUTE_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HmacComputeSha),
		[XASU_HMAC_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HmacKat),
		[XASU_HMAC_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HmacGetInfo),
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_HmacResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_HmacCmds)] = {
		[XASU_HMAC_COMPUTE_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
		[XASU_HMAC_COMPUTE_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_HMAC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK,
		[XASU_HMAC_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_HmacModule.Id = XASU_MODULE_HMAC_ID;
	XAsufw_HmacModule.Cmds = XAsufw_HmacCmds;
	XAsufw_HmacModule.ResourcesRequired = XAsufw_HmacResourcesBuf;
	XAsufw_HmacModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_HmacCmds);
	XAsufw_HmacModule.ResourceHandler = XAsufw_HmacResourceHandler;
	XAsufw_HmacModule.AsuDmaPtr = NULL;
	XAsufw_HmacModule.ShaPtr = NULL;

	/** Register HMAC module. */
	Status = XAsufw_ModuleRegister(&XAsufw_HmacModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Initialize HMAC instance. */
	Status = XHmac_CfgInitialize(HmacPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_MODULE_REGISTRATION_FAILED);
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
	volatile s32 Status = XASUFW_FAILURE;
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
	if (CmdId != XASU_HMAC_GET_INFO_CMD_ID) {
		XAsufw_HmacModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_HMAC, ReqId);
		if (XAsufw_HmacModule.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
		XAsufw_AllocateResource(XASUFW_HMAC, XASUFW_HMAC, ReqId);
		XAsufw_AllocateResource(ResourceId, XASUFW_HMAC, ReqId);
	}

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
 * 	- XASUFW_SUCCESS - if SHA2 hash operation is successful.
 * 	- XASUFW_SHA_INVALID_SHA_MODE - if input sha mode is invalid
 * 	- XASUFW_HMAC_INITIALIZATION_FAILED - if HMAC initialisation fails.
 * 	- XASUFW_HMAC_UPDATE_FAILED - if HMAC update fails.
 * 	- XASUFW_HMAC_FINAL_FAILED - if HMAC final fails.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED - if DMA resource allocation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED - upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_HmacComputeSha(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	const XAsu_HmacParams *HmacParamsPtr = (const XAsu_HmacParams *)ReqBuf->Arg;
	XHmac *HmacPtr = XHmac_GetInstance();
	u32 *ResponseBufferPtr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
				 XASUFW_RESP_DATA_OFFSET;
	static u32 HmacCmdStage = 0x0U;

	/** Jump to HMAC_STAGE_UPDATE_DONE if the HmacCmdStage is HMAC_UPDATE_DONE. */
	if (HmacCmdStage != 0x0U) {
		goto HMAC_STAGE_UPDATE_IN_PROGRESS;
	}

	if ((HmacParamsPtr->OperationFlags & XASU_HMAC_INIT) == XASU_HMAC_INIT) {
		Status = XHmac_Init(HmacPtr, XAsufw_HmacModule.AsuDmaPtr, XAsufw_HmacModule.ShaPtr,
				    HmacParamsPtr->KeyAddr, HmacParamsPtr->KeyLen,
				    HmacParamsPtr->ShaMode, HmacParamsPtr->HmacLen);
		if (Status != XST_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_HMAC_INITIALIZATION_FAILED);
			goto END;
		}
	}

HMAC_STAGE_UPDATE_IN_PROGRESS:
	HmacCmdStage = 0x0U;

	if ((HmacParamsPtr->OperationFlags & XASU_HMAC_UPDATE) == XASU_HMAC_UPDATE) {
		/**
		 * If operation flags include HMAC UPDATE, perform HMAC update operation.
		 */
		Status = XHmac_Update(HmacPtr, XAsufw_HmacModule.AsuDmaPtr,
				      HmacParamsPtr->MsgBufferAddr, HmacParamsPtr->MsgLen,
				      (u32)HmacParamsPtr->IsLast);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			HmacCmdStage = HMAC_UPDATE_IN_PROGRESS;
			XAsufw_DmaNonBlockingWait(XAsufw_HmacModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
						  ReqBuf, ReqId, XASUFW_BLOCK_DMA);
			goto DONE;
		} else if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_UPDATE_FAILED);
			goto END;
		} else {
			/* Do nothing */
		}
	}

	if ((HmacParamsPtr->OperationFlags & XASU_HMAC_FINAL) == XASU_HMAC_FINAL) {
		/** If operation flags include HMAC FINAL, perform HMAC final operation. */
		Status = XHmac_Final(HmacPtr, XAsufw_HmacModule.AsuDmaPtr, ResponseBufferPtr);
		if (Status != XST_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_FINAL_FAILED);
			goto END;
		}
		if (XAsufw_ReleaseResource(XASUFW_HMAC, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	} else {
		if (XAsufw_HmacModule.AsuDmaPtr != NULL) {
			/**
			 * If HMAC_FINAL is not set in operation falgs, release DMA resource and
			 * Idle HMAC and allocated SHA resource.
			 */
			Status = XAsufw_ReleaseDmaResource(XAsufw_HmacModule.AsuDmaPtr, ReqId);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
		}
		XAsufw_IdleResource(XASUFW_HMAC);
	}
END:
	XAsufw_HmacModule.AsuDmaPtr = NULL;

	if (Status != XASUFW_SUCCESS) {
		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_HMAC, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}
DONE:
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
 *
 *************************************************************************************************/
static s32 XAsufw_HmacKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	Status = XAsufw_HmacOperationKat(XAsufw_HmacModule.AsuDmaPtr);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_HMAC, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_HmacModule.AsuDmaPtr = NULL;
	XAsufw_HmacModule.ShaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for HMAC Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 *	- Returns XASUFW_SUCCESS on successful execution of the command.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_HmacGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;
	(void)ReqId;

	/* TODO: Implement XAsufw_HmacGetInfo */
	return Status;
}
/** @} */
