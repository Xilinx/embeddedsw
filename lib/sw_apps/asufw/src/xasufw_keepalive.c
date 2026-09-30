/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_keepalive.c
 *
 * This file contains ASUFW keep-alive counter support.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------------------------
 * 1.0   vm   07/14/2026 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_keepalive.h"
#include "xasu_sharedmem.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xtask.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_KEEP_ALIVE_INTERVAL_MS		(100U)
						/**< ASUFW counter increment period (100 ms); PLM polls every 200 ms */

/************************************ Variable Definitions ***************************************/
static XTask_TaskNode *XAsufwKeepAliveTask;	/**< Periodic task for RTCA counter increments */

/************************** Function Prototypes ******************************/
static s32 XAsufw_KeepAliveTaskHandler(void *Arg);

/*************************************************************************************************/
/**
 * @brief	This function initializes the ASUFW keep-alive counter support.
 *
 * @return
 *	- XASUFW_SUCCESS on success.
 *	- XASUFW_TASK_INVALID_HANDLER on failure.
 *
 *************************************************************************************************/
s32 XAsufw_KeepAliveInit(void)
{
	s32 Status = XASUFW_FAILURE;

	XAsufw_WriteReg(XASU_RTCA_KEEP_ALIVE_COUNTER_ADDR, 0U);

	/** Create a periodic task to increment the keep-alive counter. */
	XAsufwKeepAliveTask = XTask_Create(XTASK_PRIORITY_0, XAsufw_KeepAliveTaskHandler,
					   NULL, XASUFW_KEEP_ALIVE_INTERVAL_MS);
	if (XAsufwKeepAliveTask == NULL) {
		Status = XASUFW_TASK_INVALID_HANDLER;
		goto END;
	}

	/** Kick first increment before the 100 ms periodic schedule starts. */
	XTask_TriggerNow(XAsufwKeepAliveTask);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function increments the ASUFW keep-alive counter.
 *
 * @param	Arg	Unused.
 *
 * @return
 *	- XASUFW_SUCCESS (required by XTask_Handler_t).
 *
 *************************************************************************************************/
static s32 XAsufw_KeepAliveTaskHandler(void *Arg)
{
	u32 Counter;

	(void)Arg;

	/** Increment the keep-alive counter. */
	Counter = XAsufw_ReadReg(XASU_RTCA_KEEP_ALIVE_COUNTER_ADDR);
	Counter++;
	XAsufw_WriteReg(XASU_RTCA_KEEP_ALIVE_COUNTER_ADDR, Counter);

	return XASUFW_SUCCESS;
}

/** @} */
