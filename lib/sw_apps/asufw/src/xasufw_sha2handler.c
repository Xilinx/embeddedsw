/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasufw_sha2handler.h"
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
static s32 XAsufw_Sha2ResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_ShaContext XAsufw_Sha2Context; /**< ASUFW SHA2 common context */

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

	/** The XAsufw_Sha2Cmds array contains the list of commands supported by SHA2 module. */
	static const XAsufw_ModuleCmd XAsufw_Sha2Cmds[XASU_SHA_MAX_CMDS] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_ShaOperation),
		[XASU_SHA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_ShaKatOperation),
	};

	/** The XAsufw_Sha2ResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_Sha2ResourcesBuf[XASU_SHA_MAX_CMDS] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
		[XASU_SHA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
	};

	/** The XAsufw_Sha2AccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_Sha2AccessPermBuf[XASU_SHA_MAX_CMDS] = {
		[XASU_SHA_OPERATION_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_SHA_OPERATION_CMD_ID),
		[XASU_SHA_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_SHA_KAT_CMD_ID),
	};

	XAsufw_Sha2Context.Module.Id = XASU_MODULE_SHA2_ID;
	XAsufw_Sha2Context.Module.Cmds = XAsufw_Sha2Cmds;
	XAsufw_Sha2Context.Module.ResourcesRequired = XAsufw_Sha2ResourcesBuf;
	XAsufw_Sha2Context.Module.CmdCnt = XASU_SHA_MAX_CMDS;
	XAsufw_Sha2Context.Module.ResourceHandler = XAsufw_Sha2ResourceHandler;
	XAsufw_Sha2Context.Module.AsuDmaPtr = NULL;
	XAsufw_Sha2Context.Module.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAsufw_Sha2Context.Module.AccessPermBufferPtr = XAsufw_Sha2AccessPermBuf;
	XAsufw_Sha2Context.CmdStage = XSHA_NON_BLOCKING_CMD_STAGE_INIT;

	/** Register SHA2 module. */
	Status = XAsufw_ModuleRegister(&XAsufw_Sha2Context.Module);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA2_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Initialize the SHA2 crypto engine. */
	Status = XSha_CfgInitialize(XAsufw_Sha2Context.Module.ShaPtr);
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
		((Cmd->OperationFlags & (XASU_UPDATE | XASU_FINISH)) != 0U)) ||
		(CmdId == XASU_SHA_KAT_CMD_ID)) {
		XAsufw_Sha2Context.Module.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_SHA2, ReqId);
		if (XAsufw_Sha2Context.Module.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
	}
	XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_SHA2, ReqId);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/** @} */
