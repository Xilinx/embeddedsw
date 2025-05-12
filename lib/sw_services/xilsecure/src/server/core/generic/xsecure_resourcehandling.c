/******************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_resourcehandling.c
*
* This file contains the resource handling functions. This
* file will only be part of XilSecure when it is compiled for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   pre  03/02/2025 Initial release
*       pre  04/16/2025 Added core reset at resource freeing
*       pre  05/10/2025 Added AES and SHA events queuing mechanism under XPLMI_IPI_DEVICE_ID macro
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_resourcehandling.h"
#include "xplmi_task.h"
#include "xplmi_ipi.h"
#include "xplmi_status.h"
#ifdef VERSAL_2VE_2VM
#include "xsecure_plat_defs.h"
#endif

/************************** Variable Definitions *****************************/
static u32 XSecure_ResIpiMask[XPLMI_MAX_CORE];
#if (defined(PLM_ENABLE_SHA_AES_EVENTS_QUEUING) || defined(VERSAL_NET)\
     && defined(XPLMI_IPI_DEVICE_ID))
static struct metal_list XSecure_IpiEventsQueue[XPLMI_MAX_CORE];
static XSecure_PartialPdiEventParams *XSecure_PpdiEventParamsPtr;

/************************** Constant Definitions *****************************/
#define XSECURE_INVALID_RESOURCE_ID (0xFFU) /**< Invalid resource */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XPlmi_FreeResourceTask(void *Data);

/************************** Variable Definitions *****************************/



/************************** Function Definitions ********************************/

/*********************************************************************************/
/**
* @brief	This function is used to get the AES and SHA state
*
* @param	ResourceSts is pointer to variable to which status has to be written
*
* @return
* 			- XST_SUCCESS on success
* 			- Error code on failure
*
**********************************************************************************/
int XSecure_GetShaAndAesSts(XSecure_ResourceSts *ResourceSts)
{
	int Status = XST_FAILURE;

	/** Input parameter validation */
	if (ResourceSts == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Considered as busy if either of AES or SHA is busy */
	if ((XSecure_ResIpiMask[XPLMI_SHA3_CORE] != XSECURE_IPI_MASK_DEF_VAL)
#ifndef PLM_SECURE_EXCLUDE
		|| (XSecure_ResIpiMask[XPLMI_AES_CORE] != XSECURE_IPI_MASK_DEF_VAL)
#endif
#ifdef VERSAL_2VE_2VM
		|| (XSecure_ResIpiMask[XPLMI_SHA2_CORE] != XSECURE_IPI_MASK_DEF_VAL)
#endif
		) {
		*ResourceSts = XSECURE_RES_BUSY;
	}
	else {
		*ResourceSts = XSECURE_RES_FREE;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/********************************************************************************/
/**
* @brief	The function triggers SHA and AES IPI events that are notified when
*           SHA/AES resource is busy
*
* @param    Core is the core whose IPI events is being triggered
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_TriggerIpiEvent(XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;
	struct metal_list *TaskQueue;
	XPlmi_TaskNode *Task;

	/** Input parameters validation */
	if (Core >= XPLMI_MAX_CORE) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	TaskQueue = &XSecure_IpiEventsQueue[Core];
	/** Return success if queue is empty */
	if (metal_list_is_empty(TaskQueue) == (int)TRUE) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** Goto next node as first node is not proper */
	TaskQueue = TaskQueue->next;
	Task = metal_container_of(TaskQueue, XPlmi_TaskNode, TaskNode);
	/** Deleting node from resource(AES/SHA) queue and adding to task queue */
	metal_list_del(&Task->TaskNode);
	/**
	 * Disable interrupts and enable after triggering task to avoid racing condition
	 * since task queue is accessed in interrupts
	 */
	microblaze_disable_interrupts();
	XPlmi_TaskTriggerNow(Task);
	microblaze_enable_interrupts();

	Status = XST_SUCCESS;

END:
	return Status;
}

/********************************************************************************/
/**
* @brief	The function frees the given resource
*
* @param    Data is the core(AES/SHA) which is to be freed
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
static int XPlmi_FreeResourceTask(void *Data)
{
	int Status = XST_FAILURE;

	/** Input parameters validation */
	if ((XPlmi_CoreType)Data >= XPLMI_MAX_CORE) {
		Status = (int)XSECURE_INVALID_RESOURCE;
		goto END;
	}

	/** Free resource */
	Status = XSecure_MakeResFree((XPlmi_CoreType)Data);

END:
	return Status;
}

/********************************************************************************/
/**
* @brief	The function notifies SHA and AES IPI events
*
* @param	BufIndex is the index of IPI message buffer
* @param    Core is the core whose IPI events is being notified
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_NotifyIpiEvent(u32 BufIndex, XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;
	struct XPlmi_TaskNode *Event = NULL;
	struct metal_list *TaskQueue;

	/** Input parameters validation */
	if (Core >= XPLMI_MAX_CORE) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Get a free task node */
	Event = XPlmi_GetTaskInstance(XPlmi_IpiDispatchHandler, (void *)BufIndex, XPLMI_INVALID_INTR_ID);
	if (Event == NULL) {
		goto END;
	}

	/** Add task to task queue (SHA/AES)*/
	TaskQueue = &XSecure_IpiEventsQueue[Core];
	if (metal_list_is_empty(&Event->TaskNode) != (int)FALSE) {
		metal_list_add_tail(TaskQueue, &Event->TaskNode);
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/********************************************************************************/
/**
* @brief	The function initializes the queues of SHA & AES IPI events and
*           free resource task
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_QueuesAndTaskInit(XSecure_PartialPdiEventParams *PpdiEventParamsPtr)
{
	int Status = XST_FAILURE;
	XPlmi_TaskNode *Task = NULL;

	/** Input parameters validation */
	if (PpdiEventParamsPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Initialize queues of SHA and AES IPI events */
	for (u32 CoreType = 0; CoreType < (u32)XPLMI_MAX_CORE; CoreType++) {
		metal_list_init(&XSecure_IpiEventsQueue[CoreType]);
	}

	/** Create task for free resource task with defined INTR ID */
	Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_1, XPlmi_FreeResourceTask,
		(void *)XSECURE_INVALID_RESOURCE_ID);
	if (Task == NULL) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
		goto END;
	}
	Task->IntrId = XPLMI_FREE_RESOURCE_TASK_ID;
	XSecure_PpdiEventParamsPtr = PpdiEventParamsPtr;
	Status = XST_SUCCESS;

END:
	 return Status;
}

/********************************************************************************/
/**
* @brief	The function frees AES/SHA resource
*
* @param    Core is the resource which is to be freed
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_MakeResFree(XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;
#ifndef PLM_SECURE_EXCLUDE
	XSecure_Aes *AesInstPtr = XSecure_GetAesInstance();
#endif
	XSecure_Sha *XSecureShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	/** Reset core and get core state to default (initialized) */
	switch (Core) {
#ifdef VERSAL_2VE_2VM
		case XPLMI_SHA2_CORE:
		XSecureShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);
		XSecure_SetReset(XSecureShaInstPtr->BaseAddress, XSECURE_SHA_RESET_OFFSET);
		XSecureShaInstPtr->ShaState = XSECURE_SHA_INITIALIZED;
		break;
#endif
		case XPLMI_SHA3_CORE:
		XSecure_SetReset(XSecureShaInstPtr->BaseAddress, XSECURE_SHA3_RESET_OFFSET);
		XSecureShaInstPtr->ShaState = XSECURE_SHA_INITIALIZED;
		break;
#ifndef PLM_SECURE_EXCLUDE
		case XPLMI_AES_CORE:
		XSecure_SetReset(AesInstPtr->BaseAddress, XSECURE_AES_SOFT_RST_OFFSET);
		AesInstPtr->AesState = XSECURE_AES_INITIALIZED;
		break;
#endif
		case XPLMI_MAX_CORE:
		default:
		Status = XST_INVALID_PARAM;
		goto END;
		break;
	}

	/** Free resource by clearing its IPI mask and timeout */
	XSecure_ResIpiMask[Core] = XSECURE_IPI_MASK_DEF_VAL;
	Status = XPlmi_LoadResourceTimeout(Core, XSECURE_TIMEOUT_CLEAR);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Check state of all resources if partial PDI event is pending */
	if (XSecure_PpdiEventParamsPtr->PartialPdiEventSts == XSECURE_EVENT_PENDING) {
		/** Trigger partial PDI event if all resources are free */
		if ((XSecure_ResIpiMask[XPLMI_SHA3_CORE] == XSECURE_IPI_MASK_DEF_VAL)
#ifndef PLM_SECURE_EXCLUDE
		&& (XSecure_ResIpiMask[XPLMI_AES_CORE] == XSECURE_IPI_MASK_DEF_VAL)
#endif
#ifdef VERSAL_2VE_2VM
		&& (XSecure_ResIpiMask[XPLMI_SHA2_CORE] == XSECURE_IPI_MASK_DEF_VAL)
#endif
		) {
			Status = XSecure_PpdiEventParamsPtr->TriggerPartialPdiEvent();
		}
		/** Return if any of the resources is not free */
	}
	/** Trigger IPI event if partial PDI event is not pending */
	else {
		Status = XSecure_TriggerIpiEvent(Core);
	}

END:
	return Status;
}

/********************************************************************************/
/**
* @brief	The function handles the SHA/AES IPI events
*
* @param    Cmd is the pointer to received IPI command
* @param    Core is the resource whose IPI events are to be handled
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_IpiEventHandling(XPlmi_Cmd *Cmd, XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;

	/** Input parameters validation */
	if ((Cmd == NULL) || (Core >= XPLMI_MAX_CORE)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Notify IPI event if the operation is in progress and if IPI mask gets mismatched */
	if (XSecure_ResIpiMask[Core] != XSECURE_IPI_MASK_DEF_VAL) {
		/** Check received command in case of IPI mask mismatch */
		if (XSecure_ResIpiMask[Core] != Cmd->IpiMask) {
			Status = XSecure_NotifyIpiEvent(Cmd->BufIndex, Core);
			if (Status == XST_SUCCESS) {
				Status = (int)XPLMI_CMD_IN_PROGRESS;
				goto END;
			}
		}
	}

	/** Load current IPI mask */
	XSecure_ResIpiMask[Core] = Cmd->IpiMask;

	/** Load timeout */
	Status = XPlmi_LoadResourceTimeout(Core, XSECURE_2SEC_INTREMSOF_10MSEC);

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sets the status of data context of previous AES/SHA operation
 *
 * @param   Core is the SHA/AES source whose data context status has to be set
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_INVALID_PARAM  On failure
 *
 *************************************************************************************************/
int XSecure_SetDataContextLost(XPlmi_CoreType Core) {
	(void)Core;
	return XST_SUCCESS;
}

#else
static u32 XSecure_DataContextLost[XPLMI_MAX_CORE];

/************************** Constant Definitions *****************************/
#define XSECURE_DATACONTEXTLOST_SET (1U) /**< Set value of data context lost */
#define XSECURE_DATACONTEXTLOST_CLR (0U) /**< Clear value of data context lost */

/********************************************************************************/
/**
* @brief	The function frees AES/SHA resource
*
* @param    Core is the resource which is to be freed
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_MakeResFree(XPlmi_CoreType Core)
{
	/** Free resource by clearing its IPI mask */
	XSecure_ResIpiMask[Core] = XSECURE_IPI_MASK_DEF_VAL;

	return XST_SUCCESS;
}

/********************************************************************************/
/**
* @brief	The function handles the SHA/AES IPI events
*
* @param    Cmd is the pointer to received IPI command
* @param    Core is the resource whose IPI events are to be handled
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_IpiEventHandling(XPlmi_Cmd *Cmd, XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;

	/** Input parameters validation */
	if ((Cmd == NULL) || (Core >= XPLMI_MAX_CORE)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Notify IPI event if the operation is in progress and if IPI mask gets mismatched
	 * and the received command is 1st Pkt
	 */
	if (XSecure_ResIpiMask[Core] != XSECURE_IPI_MASK_DEF_VAL) {
		/** Check received command in case of IPI mask mismatch */
		if (XSecure_ResIpiMask[Core] != Cmd->IpiMask) {
			Status = XST_DEVICE_BUSY;
			goto END;
		}
		else {
			if (XSecure_DataContextLost[Core] == XSECURE_DATACONTEXTLOST_SET) {
				XSecure_DataContextLost[Core] = XSECURE_DATACONTEXTLOST_CLR;
				Status = XST_DATA_LOST;
				goto END;
			}
		}
	}

	/** Load current IPI mask */
	XSecure_ResIpiMask[Core] = Cmd->IpiMask;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sets the status of data context of previous AES/SHA operation
 *
 * @param   Core is the SHA/AES source whose data context status has to be set
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_INVALID_PARAM  On failure
 *
 *************************************************************************************************/
int XSecure_SetDataContextLost(XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
#ifndef PLM_SECURE_EXCLUDE
	XSecure_Aes *AesInstPtr = XSecure_GetAesInstance();
#endif

	/** Input parameters validation */
	if (Core >= XPLMI_MAX_CORE) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Set data context lost only when resource is busy */
	if (XSecure_ResIpiMask[Core] != XSECURE_IPI_MASK_DEF_VAL) {
		XSecure_DataContextLost[Core] = XSECURE_DATACONTEXTLOST_SET;
		/** Set SHA under reset */
		if (Core == XPLMI_SHA3_CORE) {
			XSecure_SetReset(XSecureSha3InstPtr->BaseAddress,
							XSECURE_SHA3_RESET_OFFSET);
			XSecureSha3InstPtr->ShaState = XSECURE_SHA_INITIALIZED;
		}
#ifndef PLM_SECURE_EXCLUDE
		/** Set AES under reset */
		else if(Core == XPLMI_AES_CORE) {
			XSecure_SetReset(AesInstPtr->BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);
				AesInstPtr->AesState = XSECURE_AES_INITIALIZED;
		}
#endif
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
#endif
/** @} */
