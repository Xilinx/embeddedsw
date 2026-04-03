/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       vns  06/07/24 Split Data address low and high
 *       ma   06/14/24 Add support for SHAKE256 XOF and also modify SHA APIs to take DMA pointer
 *                     for every update
 *       ma   07/08/24 Add task based approach at queue level
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *                     Added support for DMA non-blocking wait
 *                     Updated copying the hash to response buffer
 *       ma   12/23/24 Allocate SHA resource for SHA start and SHA finish as well
 *       am   07/18/25 Added core reset support for single glitch recovery
 *       rmv  04/01/26 Moved common SHA operation and KAT logic to xasufw_shahandler_common.c
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_sha3handler.h"
#include "xasufw_shahandler_common.h"
#include "xasufw_modules.h"
#include "xasufw_status.h"
#include "xsha.h"
#include "xasufw_resourcemanager.h"
#include "xasu_shainfo.h"
#include "xsha_hw.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_Sha3ResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_ShaContext XAsufw_Sha3Context; /**< ASUFW SHA3 common context */

/*************************************************************************************************/
/**
 * @brief	This function initializes the SHA3 module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if SHA3 module initialization is successful.
 * 	- XASUFW_SHA3_MODULE_REGISTRATION_FAILED, if SHA3 module registration fails.
 * 	- XASUFW_SHA3_INIT_FAILED, if SHA3 init fails.
 *
 *************************************************************************************************/
s32 XAsufw_Sha3Init(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_Sha3Cmds array contains the list of commands supported by SHA3 module. */
	static const XAsufw_ModuleCmd XAsufw_Sha3Cmds[XASU_SHA_MAX_CMDS] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_ShaOperation),
		[XASU_SHA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_ShaKatOperation),
	};

	/** The XAsufw_Sha3ResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_Sha3ResourcesBuf[XASU_SHA_MAX_CMDS] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_SHA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
	};

	/** The XAsufw_Sha3AccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_Sha3AccessPermBuf[XASU_SHA_MAX_CMDS] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_SHA_OPERATION_CMD_ID),
		[XASU_SHA_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_SHA_KAT_CMD_ID),
	};

	XAsufw_Sha3Context.Module.Id = XASU_MODULE_SHA3_ID;
	XAsufw_Sha3Context.Module.Cmds = XAsufw_Sha3Cmds;
	XAsufw_Sha3Context.Module.ResourcesRequired = XAsufw_Sha3ResourcesBuf;
	XAsufw_Sha3Context.Module.CmdCnt = XASU_SHA_MAX_CMDS;
	XAsufw_Sha3Context.Module.ResourceHandler = XAsufw_Sha3ResourceHandler;
	XAsufw_Sha3Context.Module.AsuDmaPtr = NULL;
	XAsufw_Sha3Context.Module.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	XAsufw_Sha3Context.Module.AccessPermBufferPtr = XAsufw_Sha3AccessPermBuf;
	XAsufw_Sha3Context.CmdStage = XSHA_NON_BLOCKING_CMD_STAGE_INIT;

	/** Register SHA3 module. */
	Status = XAsufw_ModuleRegister(&XAsufw_Sha3Context.Module);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA3_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Initialize the SHA3 crypto engine. */
	Status = XSha_CfgInitialize(XAsufw_Sha3Context.Module.ShaPtr);
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
	 * Allocate DMA and SHA3 resource if CmdId is SHA Operation with Update or Finish flag
	 * or if CmdId is SHA KAT.
	 */
	if (((CmdId == XASU_SHA_OPERATION_CMD_ID) &&
		((Cmd->OperationFlags & (XASU_SHA_UPDATE | XASU_SHA_FINISH)) != 0U)) ||
		(CmdId == XASU_SHA_KAT_CMD_ID)) {
		XAsufw_Sha3Context.Module.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_SHA3, ReqId);
		if (XAsufw_Sha3Context.Module.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
	}
	XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_SHA3, ReqId);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/** @} */
