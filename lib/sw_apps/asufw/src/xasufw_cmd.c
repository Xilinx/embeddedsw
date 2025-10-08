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
 *       rmv  09/09/25 Added bound checks for command ID and handler in XAsufw_CommandQueueHandler()
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
#include "xasufw_hw.h"
#include "xaes.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
static void XAsufw_CheckAndRestoreAesContext(void);

/*************************************************************************************************/
/**
 * @brief	This function triggers the IPI interrupt to the sender.
 *
 * @param	IpiMask		IPI Mask of the remote processor.
 *
 *************************************************************************************************/
static inline void XAsufw_InterruptRemoteProc(u32 IpiMask)
{
	XAsufw_WriteReg(IPI_ASU_TRIG, IpiMask);
}

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
static u32 XAsufw_GetReqType(u32 CmdHeader, u32 ChannelIndex);

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

	/**
	 * Check if command ID received in the command header is greater than the max supported
	 * commands by the module.
	 */
	if (CmdId >= Module->CmdCnt) {
		Status = XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED;
		goto END;
	}

	/**
	 * Check if the command with the specified CmdId in the Module's Cmds array is NULL.
	 */
	if (Module->Cmds[CmdId].CmdHandler == NULL) {
		Status = XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED;
		goto END;
	}

	/** Change request buffer status to command in progress and call the command handler. */
	QueueBuf->ReqBufStatus = XASU_COMMAND_IN_PROGRESS;
	Status = Module->Cmds[CmdId].CmdHandler(&QueueBuf->ReqBuf, ReqId);

	/** Check and restore the AES context if previous AES operation had saved the context. */
	XAsufw_CheckAndRestoreAesContext();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function writes the given response to the corresponding response buffer.
 *
 * @param	ReqBuf		Pointer	to the request buffer.
 * @param	ReqId		Request Unique ID.
 * @param	Response	Status of the executed command.
 *
 *************************************************************************************************/
void XAsufw_CommandResponseHandler(XAsu_ReqBuf *ReqBuf, u32 ReqId, s32 Response)
{
	XAsu_ChannelQueueBuf *QueueBuf = XLinkList_ContainerOf(ReqBuf, XAsu_ChannelQueueBuf, ReqBuf);
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

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

	/** Trigger interrupt to the sender after writing the response. */
	XAsufw_InterruptRemoteProc(IpiMask);
}

/*************************************************************************************************/
/**
 * @brief	This function checks IPI Command permission type and channel permissions and returns
 * IPI request type to the caller.
 *
 * @param	CmdHeader		IPI command header.
 * @param	ChannelIndex	IPI channel index.
 *
 * @return
 *	- XASU_CMD_SECURE, if command request type is secure.
 *	- XASU_CMD_NON_SECURE, if command request type is non-secure.
 *
 *************************************************************************************************/
static u32 XAsufw_GetReqType(u32 CmdHeader, u32 ChannelIndex)
{
	u32 CmdPerm = (CmdHeader & XASU_COMMAND_SECURE_FLAG_MASK) >> XASU_COMMAND_SECURE_FLAG_SHIFT;
	volatile u32 ChannelPerm = XASU_CMD_NON_SECURE;
	u32 ReqType = XASU_CMD_NON_SECURE;
	u32 IpiBitPos;

	/** If command permission is not secure, return non-secure. */
	if (CmdPerm != XASU_CMD_SECURE) {
		goto END;
	}

	/** Get IPI bit position from the channel's IPI bit mask. */
	IpiBitPos = (u32)(__builtin_ctz(XAsufw_GetIpiMask(ChannelIndex)));
	if (IpiBitPos == 0U) {
		goto END;
	}

	/** Get TrustZone status from the IPI channel's aperture permission register. */
	ChannelPerm = XAsufw_ReadReg(LPD_XPPU_APERPERM_49 + (IpiBitPos * XASUFW_WORD_LEN_IN_BYTES));
	ChannelPerm = (ChannelPerm & LPD_XPPU_APERPERM_49_TRUSTZONE_MASK) >>
						LPD_XPPU_APERPERM_49_TRUSTZONE_SHIFT;

	/** If command permission is secure and channel permission is secure, return secure. */
	if ((CmdPerm == XASU_CMD_SECURE) && (ChannelPerm == XASU_CMD_SECURE)) {
		ReqType = XASU_CMD_SECURE;
	}

END:
	return ReqType;
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
s32 XAsufw_ValidateCommand(const XAsu_ReqBuf *ReqBuf, u32 ChannelIndex)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ModuleId = XAsufw_GetModuleId(ReqBuf->Header);
	const XAsufw_Module *Module = NULL;
	u32 AccessPerm = XASUFW_NO_IPI_ACCESS;
	u32 ReqType = XASU_CMD_NON_SECURE;

	/** Validate channel index. */
	if (ChannelIndex >= XASUFW_MAX_CHANNELS_SUPPORTED) {
		Status = XASUFW_VALIDATE_CMD_INVALID_CHANNEL_INDEX;
		goto END;
	}

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
	 * Check if the command with the specified CmdId in the Module's Cmds array is NULL.
	 */
	if (Module->Cmds[CmdId].CmdHandler == NULL) {
		Status = XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED;
		goto END;
	}

	/** Determine request type based on secure bit in command header and channel TZ settings. */
	ReqType = XAsufw_GetReqType(ReqBuf->Header, ChannelIndex);

	/** Perform access permission checks */
	if (Module->AccessPermBufferPtr != NULL) {
		AccessPerm = Module->AccessPermBufferPtr[CmdId] >>
							(XASUFW_ACCESS_PERM_SHIFT * ChannelIndex);
		AccessPerm &= XASUFW_ACCESS_PERM_MASK;
		/** - If the requested command is not given IPI access, return error. */
		if (AccessPerm == XASUFW_NO_IPI_ACCESS) {
			Status = XASUFW_ERR_VALIDATE_IPI_NO_IPI_ACCESS;
			goto END;
		}
		/**
		 * - If the request type is Non-Secure and the requested API requires Secure access,
		 * return an error.
		 */
		if ((ReqType == XASU_CMD_NON_SECURE) && (AccessPerm == XASUFW_SECURE_IPI_ACCESS)) {
			Status = XASUFW_ERR_VALIDATE_IPI_NO_NONSECURE_ACCESS;
			goto END;
		}
		/**
		 * - If the request type is Non-Secure and the requested API requires Secure access,
		 * return an error.
		 */
		if ((ReqType == XASU_CMD_SECURE) && (AccessPerm == XASUFW_NON_SECURE_IPI_ACCESS)) {
			Status = XASUFW_ERR_VALIDATE_IPI_NO_SECURE_ACCESS;
			goto END;
		}
	}
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

/*************************************************************************************************/
/**
 * @brief	This function restores AES context if required and updates the resource owner of
 * 		restored context. when a previous AES operation had saved context.
 *
 * @return
 * 	- XASUFW_SUCCESS, if context restoration is successful or not required.
 * 	- XASUFW_FAILURE, if context restoration operation fails.
 *
 *************************************************************************************************/
static void XAsufw_CheckAndRestoreAesContext(void)
{
	s32 Status = XASUFW_FAILURE;
	const XAes_ContextInfo *Ctx = XAes_GetAesContext();
	XAes *AesInstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);

	if (Ctx->IsContextRestoreReq == XASU_TRUE) {
		Status = XAes_RestoreContext(AesInstancePtr);
		if (Status != XASUFW_SUCCESS) {
			XAsufw_Printf(DEBUG_PRINT_ALWAYS, "AES context restore failed: 0x%x\r\n", Status);
		} else {
			/** Update the resource owner to the restored context. */
			XAsufw_UpdateResourceOwner(XASUFW_AES, Ctx->ReqId);
		}
	}
}
/** @} */
