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
 *       vm   10/05/26 Added advancing the counter from a wait on hardware, once the periodic
 *                     task is running and the increment period has passed
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
#include "xpseudo_asm.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_KEEP_ALIVE_INTERVAL_MS		(100U)
						/**< ASUFW counter increment period (100 ms); PLM polls every 200 ms */

/************************************ Variable Definitions ***************************************/

/**
 * Written from task context only, because an advance is refused while interrupts are disabled,
 * so it does not need the volatile qualifier TaskTimeNow carries.
 */
static u32 XAsufwKeepAliveLastTime;		/**< Time in milliseconds of the last increment */

/************************** Function Prototypes ******************************/
static s32 XAsufw_KeepAliveTaskHandler(void *Arg);
static void XAsufw_KeepAliveIncrement(u32 Counter);

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
	XTask_TaskNode *KeepAliveTask;

	XAsufw_WriteReg(XASU_RTCA_KEEP_ALIVE_COUNTER_ADDR, 0U);

	/** Create a periodic task to increment the keep-alive counter. */
	KeepAliveTask = XTask_Create(XTASK_PRIORITY_0, XAsufw_KeepAliveTaskHandler,
				     NULL, XASUFW_KEEP_ALIVE_INTERVAL_MS);
	if (KeepAliveTask == NULL) {
		Status = XASUFW_TASK_INVALID_HANDLER;
		goto END;
	}

	/** Kick first increment before the 100 ms periodic schedule starts. */
	XTask_TriggerNow(KeepAliveTask);

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
	(void)Arg;

	/** Increment the keep-alive counter. */
	XAsufw_KeepAliveIncrement(XAsufw_ReadReg(XASU_RTCA_KEEP_ALIVE_COUNTER_ADDR));

	return XASUFW_SUCCESS;
}

/*************************************************************************************************/
/**
 * @brief	This function increments the ASUFW keep-alive counter which PLM samples and records
 *		when it was incremented.
 *
 * @param	Counter		Value the counter currently holds, read by the caller.
 *
 *************************************************************************************************/
static void XAsufw_KeepAliveIncrement(u32 Counter)
{
	XAsufw_WriteReg(XASU_RTCA_KEEP_ALIVE_COUNTER_ADDR, Counter + 1U);

	XAsufwKeepAliveLastTime = TaskTimeNow;
}

/*************************************************************************************************/
/**
 * @brief	This function increments the ASUFW keep-alive counter when the increment period has
 *		passed since the last increment.
 *
 * @note	A wait on hardware which holds the task dispatcher for longer than the window PLM
 *		allows calls this between its polls, so that the counter keeps moving while the
 *		periodic task cannot run. The period is measured from the last increment, whoever
 *		made it, so the counter advances exactly when PLM would otherwise see it stand
 *		still. Nothing is incremented while the counter still reads zero, because PLM
 *		skips its health check until the periodic task has made the first increment, and
 *		an advance from here would start that check early.
 *		Nothing is incremented while interrupts are disabled either, which keeps the
 *		counter's read-modify-write to task context, where it cannot be torn by an
 *		interrupt handler taking a wait of its own. Nothing is lost by that, because the
 *		milliseconds this decision rests on are maintained by an interrupt and so cannot
 *		advance there.
 *
 *************************************************************************************************/
void XAsufw_KeepAliveAdvanceIfDue(void)
{
	u32 MachineStatus = (u32)csrr(mstatus);
	u32 Counter = XAsufw_ReadReg(XASU_RTCA_KEEP_ALIVE_COUNTER_ADDR);

	if ((Counter != 0U) &&
	    ((MachineStatus & XREG_MSTATUS_MIE_MASK) != 0U) &&
	    ((TaskTimeNow - XAsufwKeepAliveLastTime) >= XASUFW_KEEP_ALIVE_INTERVAL_MS)) {
		XAsufw_KeepAliveIncrement(Counter);
	}
}

/** @} */
