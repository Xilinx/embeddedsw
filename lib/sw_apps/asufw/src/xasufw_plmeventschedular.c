/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_plmeventschedular.c
 *
 * This file contains code for PLM event registration functionality in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv   08/07/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_def.h"
#include "xasufw_plmeventschedular.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_status.h"
#include "xil_types.h"
#include "xasufw_util.h"
#include "xasufw_cmd.h"
#include "xasufw_ipi.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_PLM_REQ_ID		(0x3FU)	/**< Fixed request ID for PLM request */
#define XASUFW_NOTIFICATION_TASK_DELAY	(100U)	/**< PLM to ASU notification task handler delay */
#define XASUFW_MAX_PLM_CMDS		(1U)	/**< Maximum PLM commands */
#define XASUFW_IPI_MESSAGE_LEN		(8U)	/**< Length of IPI message buffer */

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_PlmEventTaskHandler(void *Arg);

/************************************ Variable Definitions ***************************************/
static XTask_TaskNode *XAsufwPlmNotificationTask;	/**< Pointer to PLM notification task */
static u32 XAsufw_EventMask[XASUFW_MAX_PLM_CMDS];	/**< Event mask for respective
								command ID*/

/*************************************************************************************************/
/**
 * @brief	This function creates a task to handle PLM notification.
 *
 * @return
 *	- XASUFW_SUCCESS, if task created successfully.
 *	- XASUFW_FAILURE, if task is not created successfully.
 *	- XASUFW_TASK_INVALID_HANDLER, if task handler is invalid.
 *
 *************************************************************************************************/
s32 XAsufw_PlmEventInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Create a task to handle PLM notification. */
	XAsufwPlmNotificationTask = XTask_Create(0U, XAsufw_PlmEventTaskHandler,
						 (void *)XAsufw_EventMask, 0x0U);
	if (XAsufwPlmNotificationTask == NULL) {
		Status = XASUFW_TASK_INVALID_HANDLER;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;

}

/*************************************************************************************************/
/**
 * @brief	This function triggers task to handle event from PLM.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_READ_IPI_MSG_FROM_PLM_FAIL, if IPI message read is failed.
 *	- XASUFW_PLM_INVALID_CMD_HEADER, if header is not valid in IPI notification command.
 *	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, if module with given ID is not registered.
 *	- XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED, if command ID is not valid.
 *
 *************************************************************************************************/
void XAsufw_HandlePlmEvent(void)
{
	s32 Status = XASUFW_FAILURE;
	const XAsufw_Module *Module = NULL;
	u32 Payload[XASUFW_IPI_MESSAGE_LEN] = {0U};
	u32 ReqId;
	u32 ModuleId;
	u32 CmdId;

	/** Read IPI message form PLM. */
	Status = XAsufw_ReadIpiMsgFromPlm(Payload, XASUFW_IPI_MESSAGE_LEN);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_READ_IPI_MSG_FROM_PLM_FAIL);
		goto END;
	}

	/** Extract request ID from IPI command header. */
	ReqId = (Payload[XASUFW_BUFFER_INDEX_ZERO] & XASU_UNIQUE_REQ_ID_MASK) >>
		 XASU_UNIQUE_REQ_ID_SHIFT;
	if (ReqId != XASUFW_PLM_REQ_ID) {
		Status = XASUFW_PLM_INVALID_CMD_HEADER;
		goto END;
	}

	/** Extract module ID from IPI command header. */
	ModuleId = (Payload[XASUFW_BUFFER_INDEX_ZERO] & XASU_MODULE_ID_MASK) >>
		    XASU_MODULE_ID_SHIFT;
	if (ModuleId != XASU_MODULE_PLM_ID) {
		Status = XASUFW_PLM_INVALID_CMD_HEADER;
		goto END;
	}

	/** Get module from the module ID received in the command header and validate. */
	Module = XAsufw_GetModule(ModuleId);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	/** Extract command ID from IPI command header. */
	CmdId = (Payload[XASUFW_BUFFER_INDEX_ZERO] & XASU_COMMAND_ID_MASK);

	/**
	 * Check if command ID received in the command header is greater than the max supported
	 * commands by the module.
	 */
	if ((CmdId >= Module->CmdCnt) || (CmdId >= XASUFW_MAX_PLM_CMDS)) {
		Status = XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED;
		goto END;
	}

	/** Update the pending event mask for a given command. */
	XAsufw_EventMask[CmdId] |= Payload[XASUFW_BUFFER_INDEX_ONE];

	/** Trigger PLM notification task. */
	XTask_TriggerNow(XAsufwPlmNotificationTask);

END:
	if (Status != XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_GENERAL, "PLM event handling failed with error: %x\n", Status);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for PLM event task.
 *
 * @param	Arg	Pointer to the task private data.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, if module with given ID is not registered.
 *	- XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED, if command handler is not registered for the
 *	  requested command.
 *	- XASUFW_PLM_EVENT_HANDLING_FAIL, if PLM event handling is failed.
 *
 *************************************************************************************************/
static s32 XAsufw_PlmEventTaskHandler(void *Arg)
{
	s32 Status = XASUFW_FAILURE;
	const XAsufw_Module *Module = NULL;
	XAsu_ReqBuf ReqBuf;
	u32 *EventMask = (u32 *)Arg;
	u32 SchedulelTask = XASU_FALSE;
	u32 Idx;

	/** Get module from the module ID received in the command header and validate. */
	Module = XAsufw_GetModule(XASU_MODULE_PLM_ID);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	for (Idx = 0; Idx < Module->CmdCnt; Idx++) {
		if (EventMask[Idx] == 0U) {
			/** No event to process, continue to next command. */
			continue;
		}

		/**
		 * Checks if the command with the specified CmdId in the Module's Cmds array is
		 * NULL.
		 */
		if (Module->Cmds[Idx].CmdHandler == NULL) {
			Status = XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED;
			goto END;
		}

		/** Prepare minimum request buffer to mimic user request. */
		ReqBuf.Header = ((((u32)XASU_MODULE_PLM_ID) << XASU_MODULE_ID_SHIFT) &
				 XASU_MODULE_ID_MASK) | ((((u32)XASUFW_PLM_REQ_ID) <<
				 XASU_UNIQUE_REQ_ID_SHIFT) & XASU_UNIQUE_REQ_ID_MASK) |
				 (Idx & XASU_COMMAND_ID_MASK);
		ReqBuf.Arg[XASUFW_BUFFER_INDEX_ZERO] = EventMask[Idx];

		/** Check availability of resources. */
		Status = XAsufw_CheckAndAllocateResources(&ReqBuf, XASUFW_PLM_REQ_ID);
		if (Status != XASUFW_SUCCESS) {
			SchedulelTask = XASU_TRUE;
			continue;
		}

		/** Call the command handler for the requested command. */
		Status = Module->Cmds[Idx].CmdHandler(&ReqBuf, XASUFW_PLM_REQ_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PLM_EVENT_HANDLING_FAIL);
			goto END;
		}

		/** Clear the event mask bits which are served. */
		EventMask[Idx] &= ~(ReqBuf.Arg[XASUFW_BUFFER_INDEX_ZERO]);
	}

	if (SchedulelTask == XASU_TRUE) {
		/** If resource is not available, schedule task after some delay to handle event. */
		if (XAsufwPlmNotificationTask != NULL) {
			(void)XTask_TriggerAfterDelay(XAsufwPlmNotificationTask,
						      XASUFW_NOTIFICATION_TASK_DELAY);
		}
	}

END:
	return Status;
}
/** @} */
