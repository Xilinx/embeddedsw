/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sha2handler.c
 *
 * This file contains the SHA2 module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   04/18/24 Initial release
 *       ma   05/20/24 Add error code for module registration and SHA initialization failure
 *       ma   06/14/24 Add support for SHAKE256 XOF and also modify SHA APIs to take DMA pointer
 *                     for every update
 *       ma   07/08/24 Add task based approach at queue level
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *                     Added support for DMA non-blocking wait
 *                     Updated copying the hash to response buffer
 *       ma   12/23/24 Allocate SHA resource for SHA start and SHA finish as well
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
#include "xasufw_sha2handler.h"
#include "xasufw_modules.h"
#include "xasufw_status.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xasufw_util.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xasu_shainfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_Sha2Kat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_Sha2Operation(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_Sha2ResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_Sha2Module; /**< ASUFW SHA2 Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the SHA2 module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if SHA2 module initialization is successful.
 * 	- XASUFW_SHA2_MODULE_REGISTRATION_FAILED, if SHA2 module registration fails.
 * 	- XASUFW_SHA2_INIT_FAILED, if SHA2 init fails.
 *
 *************************************************************************************************/
s32 XAsufw_Sha2Init(void)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);

	/** The XAsufw_Sha2Cmds array contains the list of commands supported by SHA2 module. */
	static const XAsufw_ModuleCmd XAsufw_Sha2Cmds[] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha2Operation),
		[XASU_SHA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha2Kat),
	};

	/** The XAsufw_Sha2ResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_Sha2ResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_Sha2Cmds)] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
		[XASU_SHA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
	};

	XAsufw_Sha2Module.Id = XASU_MODULE_SHA2_ID;
	XAsufw_Sha2Module.Cmds = XAsufw_Sha2Cmds;
	XAsufw_Sha2Module.ResourcesRequired = XAsufw_Sha2ResourcesBuf;
	XAsufw_Sha2Module.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_Sha2Cmds);
	XAsufw_Sha2Module.ResourceHandler = XAsufw_Sha2ResourceHandler;
	XAsufw_Sha2Module.AsuDmaPtr = NULL;

	/** Register SHA2 module. */
	Status = XAsufw_ModuleRegister(&XAsufw_Sha2Module);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Initialize the SHA2 crypto engine. */
	Status = XSha_CfgInitialize(XAsufw_Sha2);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_INIT_FAILED);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for SHA2 module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha2ResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	const XAsu_ShaOperationCmd *Cmd = (const XAsu_ShaOperationCmd *)ReqBuf->Arg;

	/**
	 * Allocate DMA and SHA2 resource if CmdId is SHA Operation with Update or Finish flag
	 * or if CmdId is SHA KAT.
	 */
	if (((CmdId == XASU_SHA_OPERATION_CMD_ID) &&
		((Cmd->OperationFlags & (XASU_SHA_UPDATE | XASU_SHA_FINISH)) != 0U)) ||
		(CmdId == XASU_SHA_KAT_CMD_ID)) {
		XAsufw_Sha2Module.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_SHA2, ReqId);
		if (XAsufw_Sha2Module.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
	}
	XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_SHA2, ReqId);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for SHA2 operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if SHA2 hash operation is successful.
 * 	- XASUFW_SHA2_START_FAILED, if SHA2 start fails.
 * 	- XASUFW_SHA2_UPDATE_FAILED, if SHA2 update fails.
 * 	- XASUFW_SHA2_FINISH_FAILED, if SHA2 finish fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha2Operation(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	const XAsu_ShaOperationCmd *Cmd = (const XAsu_ShaOperationCmd *)ReqBuf->Arg;
	static u32 CmdStage = 0x0U;
	u32 *HashAddr;

	/** Jump to SHA_STAGE_UPDATE_DONE if SHA update is in progress. */
	if (CmdStage != 0x0U) {
		goto SHA_STAGE_UPDATE_DONE;
	}

	if ((Cmd->OperationFlags & XASU_SHA_START) == XASU_SHA_START) {
		/** If operation flags include SHA START, perform SHA2 start operation. */
		Status = XSha_Start(XAsufw_Sha2, Cmd->ShaMode);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_START_FAILED);
			goto END;
		}
	}

	if ((Cmd->OperationFlags & XASU_SHA_UPDATE) == XASU_SHA_UPDATE) {
		/** If operation flags include SHA UPDATE perform SHA2 update operation. */
		Status = XSha_Update(XAsufw_Sha2, XAsufw_Sha2Module.AsuDmaPtr, Cmd->DataAddr,
					Cmd->DataSize, Cmd->IsLast);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = SHA_UPDATE_DONE;
			XAsufw_DmaNonBlockingWait(XAsufw_Sha2Module.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
						ReqBuf, ReqId, XASUFW_RELEASE_DMA);
			XAsufw_Sha2Module.AsuDmaPtr = NULL;
			goto DONE;
		} else if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_UPDATE_FAILED);
			goto END;
		} else {
			/* Do nothing */
		}
	}

SHA_STAGE_UPDATE_DONE:
	CmdStage = 0x0U;

	if ((Cmd->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) {
		/** If operation flags include SHA FINISH, perform SHA2 finish operation. */
		HashAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;
		Status = XSha_Finish(XAsufw_Sha2, XAsufw_Sha2Module.AsuDmaPtr, HashAddr,
						Cmd->HashBufSize, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_FINISH_FAILED);
			goto END;
		}
	} else {
		if (XAsufw_Sha2Module.AsuDmaPtr != NULL) {
			/**
			 * If SHA_FINISH is not set in operation flags and SHA2 update is complete,
			 * release DMA resource and Idle SHA2 resource.
			 */
			Status = XAsufw_ReleaseDmaResource(XAsufw_Sha2Module.AsuDmaPtr, ReqId);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
			XAsufw_Sha2Module.AsuDmaPtr = NULL;
		}
		XAsufw_IdleResource(XASUFW_SHA2);
	}

END:
	if ((Status != XASUFW_SUCCESS) ||
		((Cmd->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH)) {
		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_SHA2, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XSha_Reset(XAsufw_Sha2);
		XAsufw_Sha2Module.AsuDmaPtr = NULL;
	}

DONE:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for SHA2 KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error code from XAsufw_ShaKat API, if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha2Kat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	u32 Sha2Mode = *((u32 *)ReqBuf->Arg);

	if ((Sha2Mode != XASU_SHA_MODE_256) && (Sha2Mode != XASU_SHA_512_HASH_LEN)) {
		Status = XASUFW_SHA_INVALID_SHA_MODE;
		goto END;
	}

	/** Perform SHA2 KAT. */
	Status = XAsufw_ShaKat(XAsufw_Sha2, XAsufw_Sha2Module.AsuDmaPtr, XASUFW_SHA2, Sha2Mode);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_SHA2, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_Sha2Module.AsuDmaPtr = NULL;

	return Status;
}
/** @} */
