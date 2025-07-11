/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_cmd.c
 *
 * This file contains the code for commands handling in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/02/24 Initial release
 *       ma   03/16/24 Added error codes at required places
 *       ma   04/18/24 Moved command handling related APIs from xasufw_sharedmem.c to this file
 *       ma   05/18/24 Added API to check resources availability before executing a command
 *       ma   07/08/24 Add task based approach at queue level
 *       ss   09/26/24 Fixed doxygen comments
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *       ma   02/28/25 Move IPI triggering to remote processor to xasufw_queuescheduler.c
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_cmd.h"
#include "xasufw_status.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_util.h"
#include "xasufw_debug.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/
/**
 * @brief	This function returns module ID from the command header.
 *
 * @param	Header	Command Header.
 *
 * @return
 * 	- Returns module ID.
 *
 *************************************************************************************************/
static inline u32 XAsufw_GetModuleId(u32 Header)
{
	return ((Header & XASU_MODULE_ID_MASK) >> XASU_MODULE_ID_SHIFT);
}

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
s32 ReturnStatus = XASUFW_FAILURE; /**< Redundant variable holds non-zero success value helps to
		detect any glitch attacks */

/*************************************************************************************************/
/**
 * @brief	This function calls the registered command handler based on the QueueInfo.
 *
 * @param	QueueBuf	Pointer to the XAsu_ChannelQueueBuf structure.
 * @param	ReqId		Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if command execution is successful.
 * 	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, if module is not registered.
 * 	- XASUFW_FAILURE, if there is any failure.
 * 	- XASUFW_CMD_IN_PROGRESS, if command is in progress (This status is returned if the DMA
 * 	  non-blocking operation is initiated).
 *
 *************************************************************************************************/
s32 XAsufw_CommandQueueHandler(XAsu_ChannelQueueBuf *QueueBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = QueueBuf->ReqBuf.Header & XASU_COMMAND_ID_MASK;
	u32 ModuleId = (QueueBuf->ReqBuf.Header & XASU_MODULE_ID_MASK) >> XASU_MODULE_ID_SHIFT;
	const XAsufw_Module *Module = NULL;

	/** Get module from the module ID received in the command header. */
	Module = XAsufw_GetModule(ModuleId);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	/** Change request buffer status to command in progress and call the command handler. */
	QueueBuf->ReqBufStatus = XASU_COMMAND_IN_PROGRESS;
	Status = Module->Cmds[CmdId].CmdHandler(&QueueBuf->ReqBuf, ReqId);

	/**
	 * If the command execution is complete, call the command response handler with command
	 * execution status.
	 */
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		XAsufw_CommandResponseHandler(&QueueBuf->ReqBuf, Status);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function writes the given response to the corresponding response buffer.
 *
 * @param	ReqBuf		Pointer	to the request buffer.
 * @param	Response	Status of the executed command.
 *
 *************************************************************************************************/
void XAsufw_CommandResponseHandler(XAsu_ReqBuf *ReqBuf, s32 Response)
{
	XAsu_ChannelQueueBuf *QueueBuf = XLinkList_ContainerOf(ReqBuf, XAsu_ChannelQueueBuf, ReqBuf);

	/**
	 * Update command status with execution complete in the request queue.
	 * Update response in the response queue.
	 */
	QueueBuf->ReqBufStatus = XASU_COMMAND_EXECUTION_COMPLETE;
	QueueBuf->RespBuf.Header = QueueBuf->ReqBuf.Header;
	QueueBuf->RespBuf.Arg[XASU_RESPONSE_STATUS_INDEX] = (u32)Response;
	QueueBuf->RespBuf.AdditionalStatus = (u32)ReturnStatus;
	ReturnStatus = XASUFW_FAILURE;
	QueueBuf->RespBufStatus = XASU_RESPONSE_IS_PRESENT;
	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "Command response: 0x%x\r\n", Response);
}

/*************************************************************************************************/
/**
 * @brief	This function checks if the received command is valid and has required access
 * 		permissions or not and returns status accordingly.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 *
 * @return
 *	- XASUFW_SUCCESS, if the command validation is successful.
 *	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, when module is not registered.
 *	- XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED, when invalid command ID is received.
 *	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_ValidateCommand(const XAsu_ReqBuf *ReqBuf)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ModuleId = XAsufw_GetModuleId(ReqBuf->Header);
	const XAsufw_Module *Module = NULL;

	/** Get module from the module ID received in the command header and validate. */
	Module = XAsufw_GetModule(ModuleId);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	/**
	 * Check if command ID received in the command header is greater than the max supported
	 * commands by the module.
	 */
	if (CmdId >= Module->CmdCnt) {
		Status = XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED;
		goto END;
	}

	/**
	 * Checks if the command with the specified CmdId in the Module's Cmds array is NULL.
	 */
	if (Module->Cmds[CmdId].CmdHandler == NULL) {
		Status = XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED;
		goto END;
	}

	/* TODO: Access permissions check should happen here */
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks for the availability of required resources for the command. If
 * the required resources are available and the corresponding module had registered for the
 * resource handler, this function allocates the necessary resources for the command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID
 *
 * @return
 *	- XASUFW_SUCCESS, if the required resources are available.
 *	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, if module is not registered.
 *	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_CheckAndAllocateResources(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ModuleId = XAsufw_GetModuleId(ReqBuf->Header);
	const XAsufw_Module *Module = NULL;
	u16 ReqResources;

	/** Get module from the module ID received in the command header. */
	Module = XAsufw_GetModule(ModuleId);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	/** Check for the resource availability if ResourcesRequired is registered by the module. */
	if (Module->ResourcesRequired != NULL) {
		ReqResources = Module->ResourcesRequired[CmdId];
		Status = XAsufw_CheckResourceAvailability(ReqResources, ReqId, ReqBuf);
		/**
		 * If required resources are available, allocate the resource.
		 * Otherwise, return error code.
		 */
		if ((Status == XASUFW_SUCCESS) && (Module->ResourceHandler != NULL)) {
			Status = Module->ResourceHandler(ReqBuf, ReqId);
		}
	}

END:
	return Status;
}
/** @} */
