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
#ifdef VERSAL_AIEPG2
#include "xsecure_plat_defs.h"
#endif

/************************** Constant Definitions *****************************/
#define XSECURE_INVALID_RESOURCE_ID (0xFFU) /**< Invalid resource */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XSecure_TriggerIpiEvent(XPlmi_CoreType Core);
static int XPlmi_FreeResourceTask(void *Data);

/************************** Variable Definitions *****************************/
static struct metal_list XSecure_IpiEventsQueue[XPLMI_MAX_CORE];
static XSecure_PartialPdiEventParams *XSecure_PpdiEventParamsPtr;

/************************** Function Definitions ********************************/

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
static int XSecure_TriggerIpiEvent(XPlmi_CoreType Core)
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

	/** Free resource based on core recieved from private data */
	switch ((XPlmi_CoreType)Data)
	{
	case XPLMI_SHA3_CORE:
#ifdef VERSAL_AIEPG2
	case XPLMI_SHA2_CORE:
#endif
		Status = XSecure_MakeShaFree((XPlmi_CoreType)Data);
		break;
#ifndef PLM_SECURE_EXCLUDE
	case XPLMI_AES_CORE:
		Status = XSecure_MakeAesFree();
		break;
#endif
	case XPLMI_MAX_CORE:
	default:
		Status = (int)XSECURE_INVALID_RESOURCE;
		break;
	}

	return Status;
}

/********************************************************************************/
/**
* @brief	The function is used to get the state of AES
*
* @param	AesState is the pointer to the variable in which AES state is stored
*
* @return
* 			- XST_SUCCESS on success
*           - XST_INVALID_PARAM on failure
*
**********************************************************************************/
int XSecure_GetAesState(XSecure_AesState *AesState)
{
	int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	/** Input parameters validation */
	if ((NULL == AesState) || (XSecureAesInstPtr == NULL)){
		Status = XST_INVALID_PARAM;
		goto END;
	}
	*AesState = XSecureAesInstPtr->AesState;
	Status = XST_SUCCESS;
END:
	return Status;
}

/********************************************************************************/
/**
* @brief	The function is used to get the state of SHA
*
* @param	ShaState is the pointer to the variable in which SHA state is stored
*
* @return
* 			- XST_SUCCESS on success
*           - XST_INVALID_PARAM on failure
*
**********************************************************************************/
int XSecure_GetShaState(XSecure_ShaState *ShaState, XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;
	XSecure_Sha *XSecureShaInstPtr = NULL;

	/** Input parameters validation */
	if ((NULL == ShaState) || (Core >= XPLMI_MAX_CORE)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSecureShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_0_DEVICE_ID);
#ifdef VERSAL_AIEPG2
	if (Core == XPLMI_SHA2_CORE)
	{
		XSecureShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_1_DEVICE_ID);
	}
#endif
	if (XSecureShaInstPtr == NULL) {
		goto END;
	}

	/** Read SHA state */
	*ShaState = XSecureShaInstPtr->ShaState;
	Status = XST_SUCCESS;
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
static int XSecure_NotifyIpiEvent(u32 BufIndex, XPlmi_CoreType Core)
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

#ifndef PLM_SECURE_EXCLUDE
/********************************************************************************/
/**
* @brief	The function frees AES resource
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_MakeAesFree(void)
{
	int Status = XST_FAILURE;
	XSecure_ShaState Sha3State;
#ifdef VERSAL_AIEPG2
	XSecure_ShaState Sha2State;
#endif

	/** Get AES instance */
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	if (XSecureAesInstPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Free AES by setting its state to initial state and clearing timeout */
	XSecureAesInstPtr->IpiMask = XSECURE_IPI_MASK_DEF_VAL;
	Status = XPlmi_LoadResourceTimeout(XPLMI_AES_CORE, XSECURE_TIMEOUT_CLEAR);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Check state of SHA if partial PDI event is pending */
	if (XSecure_PpdiEventParamsPtr->PartialPdiEventSts == XSECURE_EVENT_PENDING) {
		Status = XSecure_GetShaState(&Sha3State, XPLMI_SHA3_CORE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
#ifdef VERSAL_AIEPG2
		Status = XSecure_GetShaState(&Sha2State, XPLMI_SHA2_CORE);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** Trigger partial PDI event if SHA is free */
		if ((Sha3State == XSECURE_SHA_INITIALIZED) && (Sha2State == XSECURE_SHA_INITIALIZED)) {
			Status = XSecure_PpdiEventParamsPtr->TriggerPartialPdiEvent();
		}
#else
		/** Trigger partial PDI event if SHA is free */
		if (Sha3State == XSECURE_SHA_INITIALIZED) {
			Status = XSecure_PpdiEventParamsPtr->TriggerPartialPdiEvent();
		}
#endif
		/** Return if SHA is not free */
	}
	/** Trigger AES IPI event if partial PDI event is not pending */
	else {
		Status = XSecure_TriggerIpiEvent(XPLMI_AES_CORE);
	}

END:
	return Status;
}

/********************************************************************************/
/**
* @brief	The function handles the AES IPI events
*
* @param    Cmd is the pointer to received IPI command
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_AesIpiEventHandling(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	XSecure_Aes *AesInstPtr = XSecure_GetAesInstance();

	/** Input parameters validation */
	if (Cmd == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (AesInstPtr == NULL) {
		goto END;
	}

    /** Notify AES IPI event if the operation is in progress and if IPI mask gets mismatched
	 * and the recieved command is 1st command of sequence
	 */
	if (AesInstPtr->IpiMask != XSECURE_IPI_MASK_DEF_VAL) {
		/** Check received command in case of IPI mask mismatch */
		if (AesInstPtr->IpiMask != Cmd->IpiMask) {
		/** Notify IPI event if received command is first command of AES sequence */
			if (((Cmd->CmdId & XSECURE_API_ID_MASK) == XSECURE_API(XSECURE_API_AES_INIT)) ||
				((Cmd->CmdId & XSECURE_API_ID_MASK) == XSECURE_API(XSECURE_API_AES_OP_INIT)) ||
				((Cmd->CmdId & XSECURE_API_ID_MASK) == XSECURE_API(XSECURE_API_KAT)) ||
			    ((Cmd->CmdId & XSECURE_API_ID_MASK) == XSECURE_API(XSECURE_API_AES_WRITE_KEY))) {
				Status = XSecure_NotifyIpiEvent(Cmd->BufIndex, XPLMI_AES_CORE);
				if (Status == XST_SUCCESS) {
					Status = (int)XPLMI_CMD_IN_PROGRESS;
				}
			}
			/**
			* Throw incorrect AES sequence error if received command is not
			* first command of AES sequence
			*/
			else {
				Status = (int)XSECURE_AES_INCORRECT_SEQUENCE;
			}
			goto END;
		}
	}

	/** Load current IPI mask */
	AesInstPtr->IpiMask = Cmd->IpiMask;

	/** Load AES Timeout */
	Status = XPlmi_LoadResourceTimeout(XPLMI_AES_CORE, XSECURE_2SEC_INTREMSOF_10MSEC);

	Status = XST_SUCCESS;

END:
	return Status;
}
#endif

/********************************************************************************/
/**
* @brief	The function frees SHA resource
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_MakeShaFree(XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;
	XSecure_AesState AesState;
#ifdef VERSAL_AIEPG2
	XSecure_ShaState OtherShaCoreState;
#endif
	XSecure_Sha *XSecureShaInstPtr = NULL;

	/** Get SHA instance */
	if (Core >= XPLMI_MAX_CORE) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSecureShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_0_DEVICE_ID);
#ifdef VERSAL_AIEPG2
	if (Core == XPLMI_SHA2_CORE) {
		XSecureShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_1_DEVICE_ID);
	}
#endif
	if (XSecureShaInstPtr == NULL) {
		goto END;
	}

	/** Free SHA by setting its state to initial state and clearing timeout */
	XSecureShaInstPtr->IpiMask = XSECURE_IPI_MASK_DEF_VAL;
	Status = XPlmi_LoadResourceTimeout(Core, XSECURE_TIMEOUT_CLEAR);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Check state of AES if partial PDI event is pending */
	if (XSecure_PpdiEventParamsPtr->PartialPdiEventSts == XSECURE_EVENT_PENDING) {
		Status = XSecure_GetAesState(&AesState);
		if (Status != XST_SUCCESS) {
			goto END;
		}

#ifdef VERSAL_AIEPG2
		if (Core == XPLMI_SHA3_CORE) {
			Status = XSecure_GetShaState(&OtherShaCoreState, XPLMI_SHA2_CORE);
		}
		else if (Core == XPLMI_SHA2_CORE) {
			Status = XSecure_GetShaState(&OtherShaCoreState, XPLMI_SHA3_CORE);
		}
		/** Trigger partial PDI event if AES is free */
		if ((AesState == XSECURE_AES_INITIALIZED) &&
		    (OtherShaCoreState == XSECURE_SHA_INITIALIZED)) {
#else
		if (AesState == XSECURE_AES_INITIALIZED) {
#endif
			Status = XSecure_PpdiEventParamsPtr->TriggerPartialPdiEvent();
		}
		/** Return if SHA is not free */
	}
	/** Trigger SHA IPI event if partial PDI event is not pending */
	else {
		Status = XSecure_TriggerIpiEvent(Core);
	}

END:
	return Status;
}

/********************************************************************************/
/**
* @brief	The function initializes the queues of SHA & AES IPI events and
*           free resource task
*
* @param    PpdiEventParamsPtr is the pointer which contains the functions
*          related to partial PDI event
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

	/** Input parameter validation */
	if (PpdiEventParamsPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSecure_PpdiEventParamsPtr = PpdiEventParamsPtr;

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
	Status = XST_SUCCESS;

END:
	 return Status;
}

/********************************************************************************/
/**
* @brief	The function handles the SHA IPI events
*
* @param    Cmd is the pointer to received IPI command
*
* @return
* 			- XST_SUCCESS on success
*           - error code on failure
*
**********************************************************************************/
int XSecure_ShaIpiEventHandling(XPlmi_Cmd *Cmd, XPlmi_CoreType Core)
{
	int Status = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = NULL;
#ifdef VERSAL_AIEPG2
	u32 ShaInitApiId = XSECURE_API(XSECURE_API_SHA_INIT);
#endif

	/** Input parameters validation */
	if ((Cmd == NULL) || (Core >= XPLMI_MAX_CORE)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

#ifdef VERSAL_AIEPG2
	if (Core == XPLMI_SHA3_CORE) {
		ShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_0_DEVICE_ID);
	}
	else if (Core == XPLMI_SHA2_CORE) {
		ShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_1_DEVICE_ID);
		ShaInitApiId = XSECURE_API(XSECURE_API_SHA2_INIT);
	}
	else {
		Status = (int)XSECURE_INVALID_RESOURCE;
		goto END;
	}
#else
	ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
#endif
	if (ShaInstPtr == NULL) {
		goto END;
	}

	/** Notify SHA IPI event if the operation is in progress and if IPI mask gets mismatched
	 * and the recieved command is 1st Pkt
	 */
	if (ShaInstPtr->IpiMask != XSECURE_IPI_MASK_DEF_VAL) {
		/** Check received command in case of IPI mask mismatch */
		if (ShaInstPtr->IpiMask != Cmd->IpiMask) {
				/** Notify IPI event if received command is first command of SHA sequence */
#ifdef VERSAL_AIEPG2
			if (((Cmd->CmdId & XSECURE_API_ID_MASK) == XSECURE_API(XSECURE_API_KAT)) ||
				((Cmd->CmdId & XSECURE_API_ID_MASK) == ShaInitApiId)) {
#else
			if ((((Cmd->CmdId & XSECURE_API_ID_MASK) == XSECURE_API(XSECURE_API_SHA3_UPDATE)) &&
				((Cmd->Payload[2U] & XSECURE_IPI_FIRST_PACKET_MASK) != 0U)) ||
				((Cmd->CmdId & XSECURE_API_ID_MASK) == XSECURE_API(XSECURE_API_KAT))) {
#endif
				Status = XSecure_NotifyIpiEvent(Cmd->BufIndex, Core);
				if (Status == XST_SUCCESS) {
					Status = (int)XPLMI_CMD_IN_PROGRESS;
				}
			}
			/**
			 * Throw incorrect SHA sequence error if received command is not
			 * first command of SHA sequence
			 */
			else {
				Status = (int)XSECURE_SHA_INCORRECT_SEQUENCE;
			}
			goto END;
		}
	}

	/** Load current IPI mask */
	ShaInstPtr->IpiMask = Cmd->IpiMask;

	/** Load SHA timeout */
	Status = XPlmi_LoadResourceTimeout(Core, XSECURE_2SEC_INTREMSOF_10MSEC);

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
