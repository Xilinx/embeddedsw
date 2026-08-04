/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xplmi_asufw_keepalive.c
*
* This file contains ASUFW keep-alive monitor.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ==============================================================================
* 1.0   vm   07/14/2026 Initial release
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xilplmi_server_apis XilPlmi server APIs
 * @{
 */

/*************************************** Include Files ********************************************/
#include "xplmi_asufw_keepalive.h"
#include "xplmi_asu_update.h"
#include "xplmi_modules.h"
#include "xplmi_scheduler.h"
#include "xplmi.h"
#include "xplmi_hw.h"
#include "xplmi_status.h"
#include "xplmi_task.h"
#include "xstatus.h"

/************************************ Constant Definitions ****************************************/
#define XPLMI_ASUFW_KEEP_ALIVE_TASK_ID	(0x123U) /**< Task Id for ASUFW keep-alive monitor */

/* Keep-alive status data structure identifiers */
#define XPLMI_ASU_KEEP_ALIVE_STS_DS_ID	(0x02U) /**< ASU keep-alive status DS Id */
#define XPLMI_ASU_KEEP_ALIVE_STS_VER	(1U) /**< ASU keep-alive status version */
#define XPLMI_ASU_KEEP_ALIVE_STS_LCVER	(1U) /**< ASU keep-alive status LC version */

/* ASUFW keep-alive status values */
#define XPLMI_ASUFW_ALIVE_INIT	(0U) /**< ASUFW keep-alive init */
#define XPLMI_ASUFW_ALIVE_ERR	(1U) /**< ASUFW keep-alive error */

#define XPLMI_ASUFW_KEEP_ALIVE_MAX_RETRIES	(3U)
						/**< Consecutive stall samples before
						 * reporting ASUFW not alive */

/************************************ Function Prototypes *****************************************/
static int XPlmi_AsufwKeepAliveTask(void *Arg);
static int XPlmi_RemoveAsufwKeepAliveTask(void);

/************************************ Variable Definitions ****************************************/
static u32 AsuKeepAliveStatus __attribute__ ((aligned(4U)))
		= XPLMI_ASUFW_ALIVE_INIT;	/**< ASUFW keep-alive status */

/**
 * Export AsuKeepAliveStatus for in-place PLM update.
 *
 * Exports keep-alive status so a latched ASUFW-not-alive error survives
 * the PLM image swap and prevents monitor restart after failure.
 */
EXPORT_DS(AsuKeepAliveStatus, XPLMI_MODULE_ASU_ID, XPLMI_ASU_KEEP_ALIVE_STS_DS_ID,
	XPLMI_ASU_KEEP_ALIVE_STS_VER, XPLMI_ASU_KEEP_ALIVE_STS_LCVER,
	sizeof(AsuKeepAliveStatus), (u32)(UINTPTR)&AsuKeepAliveStatus);
static u32 PrevAsufwCounterValue;		/**< Previous ASUFW counter value */
static u32 StallRetryCount;			/**< Consecutive stall sample count */

/**************************************************************************************************/
/**
 * @brief	This function runs the ASUFW keep-alive health check.
 *
 * @param	Arg is unused.
 *
 * @return
 * 		- XST_SUCCESS when the check completes normally.
 * 		- XPLM_ERR_ASUFW_NOT_ALIVE when the ASUFW counter did not change across
 * 		  three consecutive samples.
 *
 **************************************************************************************************/
static int XPlmi_AsufwKeepAliveTask(void *Arg)
{
	int Status = XST_FAILURE;
	u32 CurrCounterValue;
	u32 UpdateState;

	(void)Arg;

	/** Skip check when ASUFW is not present. */
	if ((u8)FALSE == XPlmi_AsuFwIsPresent()) {
		Status = XST_SUCCESS;
		goto END;
	}

	UpdateState = XPlmi_In32(XPLMI_ASU_UPDATE_STATE_REG);

	/** Skip check while PLM update is in progress. */
	if ((UpdateState != XPLMI_UPDATE_STATE_INIT) &&
	    (UpdateState != XPLMI_UPDATE_STATE_FINISHED)) {
		Status = XST_SUCCESS;
		goto END;
	}

	CurrCounterValue = XPlmi_In32(XPLMI_ASU_RTCA_KEEP_ALIVE_COUNTER_ADDR);

	/** Skip keep-alive check while counter is zero, waiting for ASUFW to start incrementing. */
	if (0U == CurrCounterValue) {
		PrevAsufwCounterValue = CurrCounterValue;
		StallRetryCount = 0U;
		Status = XST_SUCCESS;
		goto END;
	}

	/** Report failure after three consecutive samples with no counter change.
	 * ASUFW increments every 100 ms and PLM polls every 200 ms, so the counter
	 * must advance at least once between consecutive PLM samples.
	 */
	if (CurrCounterValue == PrevAsufwCounterValue) {
		StallRetryCount++;
		if (StallRetryCount >= XPLMI_ASUFW_KEEP_ALIVE_MAX_RETRIES) {
			PrevAsufwCounterValue = 0U;
			StallRetryCount = 0U;
			AsuKeepAliveStatus = XPLMI_ASUFW_ALIVE_ERR;
			Status = XPlmi_RemoveAsufwKeepAliveTask();
			Status = XPlmi_UpdateStatus(XPLM_ERR_ASUFW_NOT_ALIVE, Status);
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"%s ERROR: ASUFW is not alive\r\n", __func__);
		} else {
			Status = XST_SUCCESS;
		}
	} else {
		PrevAsufwCounterValue = CurrCounterValue;
		StallRetryCount = 0U;
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function creates the ASUFW keep-alive PLM task.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XPLM_ERR_ASUFW_KEEP_ALIVE_TASK_CREATE on failure.
 *
 **************************************************************************************************/
int XPlmi_AsufwKeepAliveInit(void)
{
	volatile int Status = XST_FAILURE;
	XPlmi_TaskNode *Task = NULL;

	/** Create ASUFW keep-alive task which will be triggered on start */
	Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_0, XPlmi_AsufwKeepAliveTask, NULL);
	if (Task == NULL) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_ASUFW_KEEP_ALIVE_TASK_CREATE,
				(int)XPLM_ERR_TASK_CREATE);
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"ASUFW keep-alive task creation failed\n\r");
		goto END;
	}

	/** Assign task ID for identification */
	Task->IntrId = XPLMI_ASUFW_KEEP_ALIVE_TASK_ID;

	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function starts the ASUFW keep-alive monitor.
 *
 * @note	Does not restart the monitor when keep-alive status is already error.
 *		Does not add a duplicate scheduler entry when the monitor is already
 *		running. Logs with DEBUG_PRINT_ALWAYS on failure.
 *
 **************************************************************************************************/
void XPlmi_StartAsufwKeepAliveMonitor(void)
{
	int Status = XST_SUCCESS;
	XPlmi_TaskNode *Task = NULL;

	/** Retrieve the pre-created ASUFW keep-alive task */
	Task = XPlmi_GetTaskInstance(NULL, NULL, XPLMI_ASUFW_KEEP_ALIVE_TASK_ID);
	if (Task == NULL) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_ASUFW_KEEP_ALIVE_TASK_CREATE,
				(int)XPLM_ERR_TASK_CREATE);
		goto END;
	}

	/** Set init and start monitor only when keep-alive status is not error. */
	if (XPLMI_ASUFW_ALIVE_ERR != AsuKeepAliveStatus) {
		PrevAsufwCounterValue = XPlmi_In32(XPLMI_ASU_RTCA_KEEP_ALIVE_COUNTER_ADDR);
		StallRetryCount = 0U;
		AsuKeepAliveStatus = XPLMI_ASUFW_ALIVE_INIT;

		/** Skip add when monitor is already scheduled. */
		if (metal_list_is_empty(&Task->TaskNode) == (int)FALSE) {
			Status = XST_SUCCESS;
			goto END;
		}

		/** Register periodic keep-alive health check with scheduler */
		Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_ASU_ID, XPlmi_AsufwKeepAliveTask,
				NULL, XPLMI_ASUFW_KEEP_ALIVE_INTERVAL_MS, XPLM_TASK_PRIORITY_0,
				NULL, XPLMI_PERIODIC_TASK);
		if (XST_SUCCESS != Status) {
			Status = XPlmi_UpdateStatus(XPLM_ERR_ASUFW_KEEP_ALIVE_TASK_CREATE,
							Status);
		}
	}

END:
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"ASUFW keep-alive monitor start failed. Error: 0x%x\r\n",
			(u32)Status);
	}
	return;
}

/**************************************************************************************************/
/**
 * @brief	This function stops the ASUFW keep-alive health check.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code from XPlmi_SchedulerRemoveTask() on failure.
 *
 **************************************************************************************************/
static int XPlmi_RemoveAsufwKeepAliveTask(void)
{
	int Status = XST_FAILURE;

	/** Remove ASUFW keep-alive health check from scheduler */
	Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_ASU_ID,
			XPlmi_AsufwKeepAliveTask, 0U, NULL);
	PrevAsufwCounterValue = 0U;
	StallRetryCount = 0U;

	return Status;
}

/** @} */
