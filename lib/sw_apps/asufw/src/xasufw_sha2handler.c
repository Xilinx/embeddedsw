/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sha2handler.c
 * @addtogroup Overview
 * @{
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
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasufw_sha2handler.h"
#include "xasufw_modules.h"
#include "xasufw_status.h"
#include "xsha.h"
#include "xasufw_util.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xasu_shainfo.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_Sha2Kat(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_Sha2GetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_Sha2Module; /**< ASUFW SHA2 Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief   This function initialized the XAsufw_Sha2Cmds structure with supported commands and
 * initializes SHA2 instance.
 *
 * @return
 * 			- On successful initialization of SHA2 module, it returns XASUFW_SUCCESS.
 *            Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XAsufw_Sha2Init(void)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);

	/* Contains the array of ASUFW SHA2 commands */
	static const XAsufw_ModuleCmd XAsufw_Sha2Cmds[] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha2Operation),
		[XASU_SHA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha2Kat),
		[XASU_SHA_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha2GetInfo),
	};

	/* Contains the required resources for each supported command */
	static XAsufw_ResourcesRequired XAsufw_Sha2ResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_Sha2Cmds)] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
		[XASU_SHA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
		[XASU_SHA_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_Sha2Module.Id = XASU_MODULE_SHA2_ID;
	XAsufw_Sha2Module.Cmds = XAsufw_Sha2Cmds;
	XAsufw_Sha2Module.ResourcesRequired = XAsufw_Sha2ResourcesBuf;
	XAsufw_Sha2Module.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_Sha2Cmds);

	Status = XAsufw_ModuleRegister(&XAsufw_Sha2Module);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_MODULE_REGISTRATION_FAILED);
		XFIH_GOTO(END);
	}

	Status = XSha_CfgInitialize(XAsufw_Sha2);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_INIT_FAILED);
		XFIH_GOTO(END);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function is a handler for SHA2 operation command.
 *
 * @param   ReqBuf	Pointer to the request buffer.
 * @param   QueueId	Queue Unique ID.
 *
 * @return
 * 			- Returns XASUFW_SUCCESS on successful execution of the command.
 *          - Otherwise, returns an error code.
 *
 *************************************************************************************************/
s32 XAsufw_Sha2Operation(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAsu_ShaOperationCmd *Cmd = (XAsu_ShaOperationCmd *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	if ((Cmd->OperationFlags & XASU_SHA_START) == XASU_SHA_START) {
		XAsufw_AllocateResource(XASUFW_SHA2, QueueId);
		Status = XSha_Start(XAsufw_Sha2, Cmd->ShaMode);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_START_FAILED);
			XFIH_GOTO(END);
		}
	}

	if ((Cmd->OperationFlags & XASU_SHA_UPDATE) == XASU_SHA_UPDATE) {
		AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_SHA2, QueueId);
		if (AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			XFIH_GOTO(END);
		}
		Status = XSha_Update(XAsufw_Sha2, AsuDmaPtr, Cmd->DataAddr, Cmd->DataSize, Cmd->IsLast);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_UPDATE_FAILED);
			XFIH_GOTO(END);
		}
	}

	if ((Cmd->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) {
		Status = XSha_Finish(XAsufw_Sha2, Cmd->HashAddr, Cmd->HashBufSize, FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_FINISH_FAILED);
			XFIH_GOTO(END);
		}
		if (XAsufw_ReleaseResource(XASUFW_SHA2, QueueId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

END:
	if (Status != XASUFW_SUCCESS) {
		/* Release resources */
		if (XAsufw_ReleaseResource(XASUFW_SHA2, QueueId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function is a handler for SHA2 KAT command.
 *
 * @param   ReqBuf	Pointer to the request buffer.
 * @param   QueueId	Queue Unique ID.
 *
 * @return
 * 			- Returns XASUFW_SUCCESS on successful execution of the command.
 *          - Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha2Kat(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);

	Status = XAsufw_ShaKat(XAsufw_Sha2, QueueId, XASUFW_SHA2);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function is a handler for SHA2 Get Info command.
 *
 * @param   ReqBuf	Pointer to the request buffer.
 * @param   QueueId	Queue Unique ID.
 *
 * @return
 * 			- Returns XASUFW_SUCCESS on successful execution of the command.
 *          - Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha2GetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	return Status;
}
/** @} */
