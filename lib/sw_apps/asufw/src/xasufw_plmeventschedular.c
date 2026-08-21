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
 *       vm    07/30/26 Preserve pending PLM events across in-place update
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
#include "xasufw_update_ds.h"
#include "xil_error_node.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_NOTIFICATION_TASK_DELAY	(100U)	/**< PLM to ASU notification task handler delay */
#define XASUFW_EVENT_PENDING		(0x1U)	/**< Event pending flag */
#define XASUFW_PLM_EVENT_DS_ID		(0x1U)	/**< Data structure ID of the pending events */
#define XASUFW_PLM_EVENT_DS_VERSION	(0x1U)	/**< Pending events data structure version */
#define XASUFW_PLM_EVENT_DS_LCVERSION	(0x1U)	/**< Pending events lowest compatible version */

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_PlmEventTaskHandler(void *Arg);
static void XAsufw_SetPendingPlmEvent(const XAsufw_Module *Module, u32 CmdId);
static void XAsufw_SanitizeRestoredPlmEvents(void);
static u32 XAsufw_IsPlmCmdRegistered(const XAsufw_Module *Module, u32 CmdId);
static void XAsufw_LogDroppedPlmEvent(const XAsufw_Module *Module, u32 CmdId);

/************************************ Variable Definitions ***************************************/
static XTask_TaskNode *XAsufwPlmNotificationTask;	/**< Pointer to PLM notification task */
static u32 XAsufw_Event[XASUFW_PLM_EVT_MAX_CMDS];		/**< Event for respective command ID */

/*
 * The pending events are held in ASU RAM, which is overwritten when the new firmware image is
 * loaded during in-place ASU update. Exporting them to the update database keeps the events which
 * are not served yet across the update, so that they can be served once the update is complete.
 * The restore path matches only the module ID and the data structure ID, hence the data structure
 * ID has to be changed whenever the layout or the meaning of this array changes.
 */
EXPORT_ASUFW_DS(XAsufw_Event, XASU_MODULE_PLM_ID, XASUFW_PLM_EVENT_DS_ID,
		XASUFW_PLM_EVENT_DS_VERSION, XASUFW_PLM_EVENT_DS_LCVERSION,
		sizeof(XAsufw_Event), (u32)(UINTPTR)XAsufw_Event);

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
 * @note	This function does not return a status. A failure while reading the message from
 *		PLM or a missing notification task is logged before returning.
 *
 *************************************************************************************************/
void XAsufw_HandlePlmEvent(void)
{
	s32 Status = XASUFW_FAILURE;
	const XAsufw_Module *Module = NULL;
	u32 Payload[XASUFW_IPI_MAX_MSG_LEN] = {0U};
	u32 EventMask;

	/** Read IPI message form PLM. */
	Status = XAsufw_ReadIpiMsgFromPlm(Payload, XASUFW_IPI_MAX_MSG_LEN);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_READ_IPI_MSG_FROM_PLM_FAIL);
		goto END;
	}
	EventMask = Payload[XASUFW_BUFFER_INDEX_TWO];

	Module = XAsufw_GetModule(XASU_MODULE_PLM_ID);
	if ((EventMask & XIL_EVENT_ERROR_MASK_OCP_SUBSYS_UPDATE) ==
			XIL_EVENT_ERROR_MASK_OCP_SUBSYS_UPDATE) {
		/** Update the pending event for a given command. */
		XAsufw_SetPendingPlmEvent(Module, XASUFW_OCP_GEN_DEV_KEYS_CMD_ID);
	}

	if ((EventMask & XIL_EVENT_ERROR_MASK_ASU_UPDATE) == XIL_EVENT_ERROR_MASK_ASU_UPDATE) {
		/** Update the pending event for ASU update command. */
		XAsufw_SetPendingPlmEvent(Module, XASUFW_ASU_UPDATE_CMD_ID);
	}

	/** Trigger PLM notification task. */
	if (XAsufwPlmNotificationTask == NULL) {
		Status = XASUFW_TASK_INVALID_HANDLER;
		goto END;
	}

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
 *	- XASUFW_SUCCESS, if every pending event is served or skipped, or if the retry of an
 *	  event which is left pending is armed.
 *	- XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, if module with given ID is not registered.
 *	- XASUFW_PLM_EVENT_HANDLING_FAIL, if PLM event handling is failed.
 *	- XASUFW_TASK_INVALID_HANDLER, if the retry of a pending event cannot be armed and no
 *	  command handler failed.
 *
 * @note	Serving stops at the first command handler failure, and every event which is
 *		left pending gets its retry armed before the failure is reported, whether it was
 *		waiting for its resources or had not been reached yet. A command handler failure
 *		is the reported error when the retry cannot be armed either, and the retry failure
 *		is printed instead of being returned.
 *
 *************************************************************************************************/
static s32 XAsufw_PlmEventTaskHandler(void *Arg)
{
	s32 Status = XASUFW_FAILURE;
	s32 HandlerStatus = XASUFW_SUCCESS;
	const XAsufw_Module *Module = NULL;
	XAsu_ReqBuf ReqBuf = {0U};
	u32 *EventMask = (u32 *)Arg;
	u32 CmdCnt;
	u32 Idx;

	/** Get module from the module ID received in the command header and validate. */
	Module = XAsufw_GetModule(XASU_MODULE_PLM_ID);
	if (Module == NULL) {
		Status = XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED;
		goto END;
	}

	/** The event array holds one flag per command, so it bounds the commands served here. */
	CmdCnt = Module->CmdCnt;
	if (CmdCnt > XASUFW_PLM_EVT_MAX_CMDS) {
		CmdCnt = XASUFW_PLM_EVT_MAX_CMDS;
	}

	for (Idx = 0U; Idx < CmdCnt; Idx++) {
		if (EventMask[Idx] != XASUFW_EVENT_PENDING) {
			/** No event to process, continue to next command. */
			continue;
		}

		/** Drop the event when its command is not registered in this build. */
		if (XAsufw_IsPlmCmdRegistered(Module, Idx) != XASU_TRUE) {
			EventMask[Idx] = 0U;
			XAsufw_LogDroppedPlmEvent(Module, Idx);
			continue;
		}

		/** Prepare minimum request buffer to mimic user request. */
		ReqBuf.Header = ((((u32)XASU_MODULE_PLM_ID) << XASU_MODULE_ID_SHIFT) &
				 XASU_MODULE_ID_MASK) | ((((u32)XASUFW_REQUEST_ID_PLM_EVENT) <<
				 XASU_UNIQUE_REQ_ID_SHIFT) & XASU_UNIQUE_REQ_ID_MASK) |
				 (Idx & XASU_COMMAND_ID_MASK);

		/** Check availability of resources. */
		Status = XAsufw_CheckAndAllocateResources(&ReqBuf, XASUFW_REQUEST_ID_PLM_EVENT);
		if (Status != XASUFW_SUCCESS) {
			continue;
		}

		/** Clear the event mask bits which are to be served. */
		EventMask[Idx] = 0U;

		/** Call the command handler for the requested command. */
		Status = Module->Cmds[Idx].CmdHandler(&ReqBuf, XASUFW_REQUEST_ID_PLM_EVENT);
		if (Status != XASUFW_SUCCESS) {
			HandlerStatus = XAsufw_UpdateErrorStatus(Status,
					XASUFW_PLM_EVENT_HANDLING_FAIL);
			break;
		}
	}

	/** Every pending event is either served or skipped unless the scan below finds one. */
	Status = XASUFW_SUCCESS;

	/**
	 * An event is left pending when its resources were not available, and when a command
	 * handler failed before the loop reached it, so the array is scanned instead of tracking
	 * the reason. The retry is armed on the command handler failure path as well, otherwise
	 * the pending event waits for an unrelated PLM event to trigger this task again.
	 */
	for (Idx = 0U; Idx < CmdCnt; Idx++) {
		if (EventMask[Idx] != XASUFW_EVENT_PENDING) {
			continue;
		}

		Status = XTask_TriggerAfterDelay(XAsufwPlmNotificationTask,
						 XASUFW_NOTIFICATION_TASK_DELAY);
		if (Status != XASUFW_SUCCESS) {
			XAsufw_Printf(DEBUG_GENERAL,
				"PLM event retry not armed. Error: 0x%x\r\n", Status);
		}
		break;
	}

	/** The command handler failure is reported once the pending event has its retry armed. */
	if (HandlerStatus != XASUFW_SUCCESS) {
		Status = HandlerStatus;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function triggers the PLM event task after in-place ASU update when an event
 *		restored from the update database is still waiting to be served.
 *
 * @note	The event which requested the in-place ASU update is cleared before the update
 *		starts, hence it is never replayed here.
 *
 *************************************************************************************************/
void XAsufw_PostUpdatePlmEvents(void)
{
	u32 Idx;

	XAsufw_SanitizeRestoredPlmEvents();

	if (XAsufwPlmNotificationTask == NULL) {
		goto END;
	}

	/** Trigger the PLM notification task if any restored event is still pending. */
	for (Idx = 0U; Idx < XASUFW_PLM_EVT_MAX_CMDS; Idx++) {
		if (XAsufw_Event[Idx] == XASUFW_EVENT_PENDING) {
			XAsufw_Printf(DEBUG_GENERAL,
				"Serving PLM event preserved across ASU update\r\n");
			XTask_TriggerNow(XAsufwPlmNotificationTask);
			break;
		}
	}

END:
	return;
}

/*************************************************************************************************/
/**
 * @brief	This function marks the event of a command as pending when that command is
 *		registered in this build.
 *
 * @param	Module	Pointer to the PLM module.
 * @param	CmdId	ID of the command whose event has to be marked as pending.
 *
 *************************************************************************************************/
static void XAsufw_SetPendingPlmEvent(const XAsufw_Module *Module, u32 CmdId)
{
	if (XAsufw_IsPlmCmdRegistered(Module, CmdId) == XASU_TRUE) {
		XAsufw_Event[CmdId] = XASUFW_EVENT_PENDING;
	} else {
		XAsufw_LogDroppedPlmEvent(Module, CmdId);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function records that the event of a command is dropped. PLM has already
 *		raised the event, so it is recorded instead of letting PLM wait for its shutdown
 *		timeout without any trace on the ASU side.
 *
 * @param	Module	Pointer to the PLM module.
 * @param	CmdId	ID of the command whose event is dropped.
 *
 *************************************************************************************************/
static void XAsufw_LogDroppedPlmEvent(const XAsufw_Module *Module, u32 CmdId)
{
	if (Module == NULL) {
		XAsufw_Printf(DEBUG_GENERAL,
			"Dropped PLM event of command %u, PLM event module is not registered\r\n",
			CmdId);
	} else {
		XAsufw_Printf(DEBUG_GENERAL,
			"Dropped PLM event of unregistered command %u\r\n", CmdId);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether a PLM command is registered in this build.
 *
 * @param	Module	Pointer to the PLM module.
 * @param	CmdId	ID of the command to be checked.
 *
 * @return
 *	- XASU_TRUE, if the command is within the supported range and has a handler.
 *	- XASU_FALSE, if the module is not registered, the command ID is out of range or the
 *	  command has no handler.
 *
 *************************************************************************************************/
static u32 XAsufw_IsPlmCmdRegistered(const XAsufw_Module *Module, u32 CmdId)
{
	u32 Registered = XASU_FALSE;

	if ((Module != NULL) && (CmdId < XASUFW_PLM_EVT_MAX_CMDS) && (CmdId < Module->CmdCnt) &&
			(Module->Cmds[CmdId].CmdHandler != NULL)) {
		Registered = XASU_TRUE;
	}

	return Registered;
}

/*************************************************************************************************/
/**
 * @brief	This function normalizes the PLM event flags restored from the update database.
 *		A flag is kept only when it holds the pending value and its command is registered
 *		in this build, so that every reader of the array agrees on which events are
 *		pending.
 *
 *************************************************************************************************/
static void XAsufw_SanitizeRestoredPlmEvents(void)
{
	const XAsufw_Module *Module = NULL;
	u32 Idx;

	Module = XAsufw_GetModule(XASU_MODULE_PLM_ID);

	for (Idx = 0U; Idx < XASUFW_PLM_EVT_MAX_CMDS; Idx++) {
		/** Any value other than the pending value is not an event. */
		if (XAsufw_Event[Idx] != XASUFW_EVENT_PENDING) {
			XAsufw_Event[Idx] = 0U;
			continue;
		}

		/** Drop the event when the new image does not register its command. */
		if (XAsufw_IsPlmCmdRegistered(Module, Idx) != XASU_TRUE) {
			XAsufw_Event[Idx] = 0U;
			XAsufw_LogDroppedPlmEvent(Module, Idx);
		}
	}
}
/** @} */
