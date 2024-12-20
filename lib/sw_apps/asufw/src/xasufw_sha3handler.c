/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sha3handler.c
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
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *                     Added support for DMA non-blocking wait
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
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

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_Sha3Kat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_Sha3GetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_Sha3Operation(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_Sha3ResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_Sha3Module; /**< ASUFW SHA3 Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the SHA3 module.
 *
 * @return
 * 	- On successful initialization of SHA3 module, it returns XASUFW_SUCCESS.
 * 	- XASUFW_SHA3_MODULE_REGISTRATION_FAILED, if SHA3 module registration fails.
 * 	- XASUFW_SHA3_INIT_FAILED, if SHA3 init fails.
 *
 *************************************************************************************************/
s32 XAsufw_Sha3Init(void)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);

	/** Contains the array of ASUFW SHA3 commands. */
	static const XAsufw_ModuleCmd XAsufw_Sha3Cmds[] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha3Operation),
		[XASU_SHA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha3Kat),
		[XASU_SHA_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_Sha3GetInfo),
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_Sha3ResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_Sha3Cmds)] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_SHA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_SHA_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_Sha3Module.Id = XASU_MODULE_SHA3_ID;
	XAsufw_Sha3Module.Cmds = XAsufw_Sha3Cmds;
	XAsufw_Sha3Module.ResourcesRequired = XAsufw_Sha3ResourcesBuf;
	XAsufw_Sha3Module.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_Sha3Cmds);
	XAsufw_Sha3Module.ResourceHandler = XAsufw_Sha3ResourceHandler;
	XAsufw_Sha3Module.AsuDmaPtr = NULL;

	/** Register SHA3 module. */
	Status = XAsufw_ModuleRegister(&XAsufw_Sha3Module);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Initialize SHA3 instance. */
	Status = XSha_CfgInitialize(XAsufw_Sha3);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_INIT_FAILED);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for SHA3 module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha3ResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	const XAsu_ShaOperationCmd *Cmd = (const XAsu_ShaOperationCmd *)ReqBuf->Arg;

	/**
	 * Allocate DMA and SHA3 resource if CmdId is SHA Operation with Update or if CmdId is SHA
	 * KAT.
	 */
	if (((CmdId == XASU_SHA_OPERATION_CMD_ID) &&
		((Cmd->OperationFlags & XASU_SHA_UPDATE) == XASU_SHA_UPDATE)) ||
		(CmdId == XASU_SHA_KAT_CMD_ID)) {
		XAsufw_Sha3Module.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_SHA3, ReqId);
		if (XAsufw_Sha3Module.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
		XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_SHA3, ReqId);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for SHA3 operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS - if SHA3 hash operation is successful.
 * 	- XASUFW_SHA3_START_FAILED - if SHA3 start fails.
 * 	- XASUFW_SHA3_UPDATE_FAILED - if SHA3 update fails.
 * 	- XASUFW_SHA3_FINISH_FAILED - if SHA3 finish fails.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED - if DMA resource allocation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED - upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha3Operation(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	const XAsu_ShaOperationCmd *Cmd = (const XAsu_ShaOperationCmd *)ReqBuf->Arg;
	static u32 CmdStage = 0x0U;

	/** Jump to SHA_STAGE_UPDATE_DONE if the CmdStage is SHA_UPDATE_DONE. */
	if (CmdStage != 0x0U) {
		goto SHA_STAGE_UPDATE_DONE;
	}

	if ((Cmd->OperationFlags & XASU_SHA_START) == XASU_SHA_START) {
		/**
		 * If operation flags include SHA START, check SHA2 resource availability,
		 * allocate it and perform SHA start operation.
		 */
		Status = XSha_Start(XAsufw_Sha3, Cmd->ShaMode);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_START_FAILED);
			goto END;
		}
	}

	if ((Cmd->OperationFlags & XASU_SHA_UPDATE) == XASU_SHA_UPDATE) {
		/**
		 * If operation flags include SHA UPDATE, check DMA resource availability,
		 * allocate it and perform SHA update operation.
		 */
		Status = XSha_Update(XAsufw_Sha3, XAsufw_Sha3Module.AsuDmaPtr, Cmd->DataAddr,
					Cmd->DataSize, Cmd->IsLast);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = SHA_UPDATE_DONE;
			XAsufw_DmaNonBlockingWait(XAsufw_Sha3Module.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
						ReqBuf, ReqId);
			XAsufw_Sha3Module.AsuDmaPtr = NULL;
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
		/** If operation flags include SHA FINISH, perform SHA finish operation. */
		Status = XSha_Finish(XAsufw_Sha3, Cmd->HashAddr, Cmd->HashBufSize, Cmd->ShakeReserved);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_FINISH_FAILED);
			goto END;
		}
		if ((Cmd->ShaMode == XASU_SHA_MODE_SHAKE256) &&
		    (Cmd->ShakeReserved != XASU_SHA_NEXT_XOF_ENABLE_MASK)) {
			if (XAsufw_ReleaseResource(XASUFW_SHA3, ReqId) != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
		}
	} else {
		if (XAsufw_Sha3Module.AsuDmaPtr != NULL) {
			/**
			 * If SHA_FINISH is not set in operation falgs, release DMA resource and Idle SHA3
			 * resource.
			 */
			Status = XAsufw_ReleaseDmaResource(XAsufw_Sha3Module.AsuDmaPtr, ReqId);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
			XAsufw_Sha3Module.AsuDmaPtr = NULL;
		}
		XAsufw_IdleResource(XASUFW_SHA3);
	}

END:
	if (Status != XASUFW_SUCCESS) {
		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_SHA3, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_Sha3Module.AsuDmaPtr = NULL;
	}

DONE:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for SHA3 KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Reqest Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code, returned when XAsufw_ShaKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha3Kat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);

	(void)ReqBuf;

	/** Perform SHA3 KAT. */
	Status = XAsufw_ShaKat(XAsufw_Sha3, XAsufw_Sha3Module.AsuDmaPtr, XASUFW_SHA3);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_SHA3, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_Sha3Module.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for SHA3 Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS on successful execution of the command.
 * 	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_Sha3GetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;
	(void)ReqId;

	/** TODO: Add SHA3 Get Info command */
	return Status;
}
/** @} */
