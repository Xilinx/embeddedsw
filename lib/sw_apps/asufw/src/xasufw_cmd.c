/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
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

/*************************************************************************************************/
/**
 * @brief	This function calls the registered command handler based on the QueueInfo.
 *
 * @param	QueueBuf	Pointer to the XAsu_ChannelQueueBuf structure.
 * @param	QueueId		Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, On successful execution of command.
 * 	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, when module is not registered.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_CommandQueueHandler(XAsu_ChannelQueueBuf *QueueBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = QueueBuf->ReqBuf.Header & XASU_COMMAND_ID_MASK;
	u32 ModuleId = (QueueBuf->ReqBuf.Header & XASU_MODULE_ID_MASK) >> XASU_MODULE_ID_SHIFT;
	const XAsufw_Module *Module = NULL;
	u32 IpiMask = QueueId >> XASUFW_IPI_MASK_SHIFT;

	/** Get module ID. */
	Module = XAsufw_GetModule(ModuleId);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	QueueBuf->ReqBufStatus = XASU_COMMAND_IN_PROGRESS;
	Status = Module->Cmds[CmdId].CmdHandler(&QueueBuf->ReqBuf, QueueId);

	/**
	 * Update command status in the request queue, the response in response queue and trigger an
	 * interrupt to the requested channel.
	 */
	QueueBuf->ReqBufStatus = XASU_COMMAND_EXECUTION_COMPLETE;
	XAsufw_CommandResponseHandler(QueueBuf, Status);
	XAsufw_WriteReg(IPI_ASU_TRIG, IpiMask);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function writes the given response to the corresponding response buffer.
 *
 * @param	QueueBuf	Pointer	to the XAsu_ChannelQueueBuf structure.
 * @param	Response	Status of the executed command.
 *
 *************************************************************************************************/
void XAsufw_CommandResponseHandler(XAsu_ChannelQueueBuf *QueueBuf, s32 Response)
{
	/** Update response to the response buffer. */
	QueueBuf->RespBuf.Header = QueueBuf->ReqBuf.Header;
	QueueBuf->RespBuf.Arg[XASU_RESPONSE_STATUS_INDEX] = (u32)Response;
	QueueBuf->RespBufStatus = XASU_RESPONSE_IS_PRESENT;
	XAsufw_Printf(DEBUG_GENERAL, "Command response: 0x%x\r\n", Response);
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

	/** Get module ID. */
	Module = XAsufw_GetModule(ModuleId);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	/** Check if Cmd ID is greater than the max supported commands */
	if (CmdId >= Module->CmdCnt) {
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
 * @brief	This function checks if the required resources are available for the command or not.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID
 *
 * @return
 *	- XASUFW_SUCCESS, if the required resources are available.
 *	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, when module is not registered.
 *	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_CheckResources(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ModuleId = XAsufw_GetModuleId(ReqBuf->Header);
	const XAsufw_Module *Module = NULL;
	u16 ReqResources;

	/** Get module ID. */
	Module = XAsufw_GetModule(ModuleId);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	/**
	 * If resources required array is registered with module, check if required resources are
	 * available.
	 */
	if (Module->ResourcesRequired != NULL) {
		ReqResources = Module->ResourcesRequired[CmdId];
		Status = XAsufw_CheckResourceAvailability(ReqResources, QueueId);
	}

END:
	return Status;
}
/** @} */
