/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sha3handler.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the SHA3 module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   04/18/24 Initial release
 *       ma   05/20/24 Add error code for module registration and SHA initialization failure
 *       vns  06/07/24 Splitted Data address low and high
 *       ma   06/14/24 Add support for SHAKE256 XOF and also modify SHA APIs to take DMA pointer
 *                     for every update
 *       ma   07/08/24 Add task based approach at queue level
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasufw_sha3handler.h"
#include "xsha_hw.h"
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
static s32 XAsufw_Sha3Kat(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_Sha3GetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_Sha3Module; /**< ASUFW SHA3 Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief   This function initialized the XAsufw_Sha3Cmds structure with supported commands and
 * initializes SHA3 instance.
 *
 * @return
 * 			- On successful initialization of SHA3 module, it returns XASUFW_SUCCESS.
 *            Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XAsufw_Sha3Init(void)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);

	/* Contains the array of ASUFW SHA3 commands */
	static const XAsufw_ModuleCmd XAsufw_Sha3Cmds[] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha3Operation),
		[XASU_SHA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha3Kat),
		[XASU_SHA_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha3GetInfo),
	};

	/* Contains the required resources for each supported command */
	static XAsufw_ResourcesRequired XAsufw_Sha3ResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_Sha3Cmds)] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_SHA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_SHA_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_Sha3Module.Id = XASU_MODULE_SHA3_ID;
	XAsufw_Sha3Module.Cmds = XAsufw_Sha3Cmds;
	XAsufw_Sha3Module.ResourcesRequired = XAsufw_Sha3ResourcesBuf;
	XAsufw_Sha3Module.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_Sha3Cmds);

	Status = XAsufw_ModuleRegister(&XAsufw_Sha3Module);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_MODULE_REGISTRATION_FAILED);
		XFIH_GOTO(END);
	}

	Status = XSha_CfgInitialize(XAsufw_Sha3);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_INIT_FAILED);
		XFIH_GOTO(END);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function is a handler for SHA3 operation command.
 *
 * @param   ReqBuf	Pointer to the request buffer.
 * @param   QueueId	Queue Unique ID.
 *
 * @return
 * 			- Returns XASUFW_SUCCESS on successful execution of the command.
 *          - Otherwise, returns an error code.
 *
 *************************************************************************************************/
s32 XAsufw_Sha3Operation(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	XAsu_ShaOperationCmd *Cmd = (XAsu_ShaOperationCmd *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	if ((Cmd->OperationFlags & XASU_SHA_START) == XASU_SHA_START) {
		XAsufw_AllocateResource(XASUFW_SHA3, QueueId);
		Status = XSha_Start(XAsufw_Sha3, Cmd->ShaMode);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_START_FAILED);
			XFIH_GOTO(END);
		}
	}

	if ((Cmd->OperationFlags & XASU_SHA_START) == XASU_SHA_START) {
		AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_SHA3, QueueId);
		if (AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			XFIH_GOTO(END);
		}
		Status = XSha_Update(XAsufw_Sha3, AsuDmaPtr, Cmd->DataAddr, Cmd->DataSize, Cmd->IsLast);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_UPDATE_FAILED);
			XFIH_GOTO(END);
		}
	}

	if ((Cmd->OperationFlags & XASU_SHA_FINISH) == XASU_SHA_FINISH) {
		Status = XSha_Finish(XAsufw_Sha3, Cmd->HashAddr, Cmd->HashBufSize, Cmd->ShakeReserved);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_FINISH_FAILED);
			XFIH_GOTO(END);
		}
		if ((Cmd->ShaMode == XASU_SHA_MODE_SHAKE256) &&
		    (Cmd->ShakeReserved != XASU_SHA_NEXT_XOF_ENABLE_MASK)) {
			if (XAsufw_ReleaseResource(XASUFW_SHA3, QueueId) != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
		}
	}

END:
	if (Status != XASUFW_SUCCESS) {
		/* Set SHA2/3 under reset */
		XSha_SetReset(XAsufw_Sha3);
		if (XAsufw_ReleaseResource(XASUFW_SHA3, QueueId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function is a handler for SHA3 KAT command.
 *
 * @param   ReqBuf	Pointer to the request buffer.
 * @param   QueueId	Queue Unique ID.
 *
 * @return
 * 			- Returns XASUFW_SUCCESS on successful execution of the command.
 *          - Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha3Kat(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);

	Status = XAsufw_ShaKat(XAsufw_Sha3, QueueId, XASUFW_SHA3);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function is a handler for SHA3 Get Info command.
 *
 * @param   ReqBuf	Pointer to the request buffer.
 * @param   QueueId	Queue Unique ID.
 *
 * @return
 * 			- Returns XASUFW_SUCCESS on successful execution of the command.
 *          - Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha3GetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	return Status;
}
/** @} */
