/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasufw_plmeventhandler.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_status.h"
#include "xil_types.h"
#include "xasufw_util.h"
#include "xasufw_cmd.h"
#include "xasufw_ipi.h"
#include "xil_error_node.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_PLM_REQ_ID		(0x3FU)	/**< Fixed request ID for PLM request */
#define XASUFW_NOTIFICATION_TASK_DELAY	(100U)	/**< PLM to ASU notification task handler delay */
#define XASUFW_MAX_PLM_CMDS		(1U)	/**< Maximum PLM commands */

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_PlmEventTaskHandler(void *Arg);

/************************************ Variable Definitions ***************************************/
static XTask_TaskNode *XAsufwPlmNotificationTask;	/**< Pointer to PLM notification task */
static u32 XAsufw_Event[XASUFW_MAX_PLM_CMDS];		/**< Event for respective command ID */

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
						 (void *)XAsufw_Event, 0x0U);
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
 *************************************************************************************************/
void XAsufw_HandlePlmEvent(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 Payload[XASUFW_IPI_MAX_MSG_LEN] = {0U};
	u32 EventMask;

	/** Read IPI message form PLM. */
	Status = XAsufw_ReadIpiMsgFromPlm(Payload, XASUFW_IPI_MAX_MSG_LEN);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_READ_IPI_MSG_FROM_PLM_FAIL);
		goto END;
	}
	EventMask = Payload[XASUFW_BUFFER_INDEX_TWO];

	if ((EventMask & XIL_EVENT_ERROR_MASK_OCP_SUBSYS_UPDATE) == XIL_EVENT_ERROR_MASK_OCP_SUBSYS_UPDATE) {
		/** Update the pending event for a given command. */
		XAsufw_Event[XASU_OCP_GEN_DEV_KEYS_CMD_ID] = 0x1;
	}

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
	XAsu_ReqBuf ReqBuf = {0U};
	u32 *EventMask = (u32 *)Arg;
	u32 SchedulelTask = XASU_FALSE;
	u32 Idx;

	/** Get module from the module ID received in the command header and validate. */
	Module = XAsufw_GetModule(XASU_MODULE_PLM_ID);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	for (Idx = 0U; Idx < Module->CmdCnt; Idx++) {
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

		/** Check availability of resources. */
		Status = XAsufw_CheckAndAllocateResources(&ReqBuf, XASUFW_PLM_REQ_ID);
		if (Status != XASUFW_SUCCESS) {
			SchedulelTask = XASU_TRUE;
			continue;
		}

		/** Clear the event mask bits which are to be served. */
		EventMask[Idx] = 0U;

		/** Call the command handler for the requested command. */
		Status = Module->Cmds[Idx].CmdHandler(&ReqBuf, XASUFW_PLM_REQ_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PLM_EVENT_HANDLING_FAIL);
			goto END;
		}
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
