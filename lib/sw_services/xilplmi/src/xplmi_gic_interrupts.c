/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_gic_interrupts.c
*
* This file is to handle the GIC interrupts
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  ma   10/08/2018 Initial release
* 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
* 1.02  bsv  04/04/2020 Code clean up
* 1.03  bm   10/14/2020 Code clean up
* 1.04  bm   04/03/2021 Move task creation out of interrupt context
* 1.05  ma   07/12/2021 Minor updates to task related code
*       bsv  08/02/2021 Code clean up to reduce code size
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_gic_interrupts.h"
#include "xplmi_hw.h"
#include "xplmi_util.h"
#include "xplmi_task.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_GicIntrAddTask(u32 Index);
static int XPlmi_GicTaskHandler(void *Arg);

/************************** Variable Definitions *****************************/
static struct GicIntrHandlerTable g_GicPInterruptTable[] = {
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC13) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC14) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC15) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC16) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC17) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC18) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC19) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC20) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC21) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC22) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC23) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC24) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC25) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC26) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP0_MASK | XPLMI_GICP0_SRC27) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC5) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC6) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC7) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC8) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC9) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC10) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC11) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC12) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC13) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC14) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC15) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC16) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC24) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC25) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC26) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC27) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC28) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC29) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC30) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP1_MASK | XPLMI_GICP1_SRC31) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP2_MASK | XPLMI_GICP2_SRC0) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP2_MASK | XPLMI_GICP2_SRC1) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP2_MASK | XPLMI_GICP2_SRC2) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP2_MASK | XPLMI_GICP2_SRC3) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP2_MASK | XPLMI_GICP2_SRC10) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP3_MASK | XPLMI_GICP3_SRC30) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP3_MASK | XPLMI_GICP3_SRC31) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP4_MASK | XPLMI_GICP4_SRC0) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP4_MASK | XPLMI_GICP4_SRC1) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP4_MASK | XPLMI_GICP4_SRC14) },
	{ NULL, NULL, (XPLMI_PMC_GIC_IRQ_GICP4_MASK | XPLMI_GICP4_SRC8) },
};

/*****************************************************************************/
/**
 * @brief	This will register the GIC handler.
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source
 * @param	Handler is the interrupt handler
 * @param	Data is the pointer to arguments to interrupt handler
 *
 * @return	None
 *
 *****************************************************************************/
int XPlmi_GicRegisterHandler(u32 GicPVal, u32 GicPxVal, GicIntHandler_t Handler,
	void *Data)
{
	int Status = XST_FAILURE;
	u8 GicIndex;
	u8 GicSource;
	XPlmi_TaskNode *Task = NULL;
	u32 PlmIntrId = (GicPVal << 8U) | (GicPxVal << 16U);

	Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_0,
			XPlmi_GicTaskHandler, (void *)PlmIntrId);
	if (Task == NULL) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
		XPlmi_Printf(DEBUG_GENERAL, "GIC Interrupt task creation "
			"error\n\r");
		goto END;
	}
	Task->IntrId = PlmIntrId | XPLMI_IOMODULE_PMC_GIC_IRQ;
	Task->State |= (u8)XPLMI_TASK_IS_PERSISTENT;

	/** Register Handler */
	GicSource = ((u8)GicPVal << XPLMI_GICP_SOURCE_SHIFT) | (u8)GicPxVal;
	for (GicIndex = 0U; GicIndex < XPLMI_ARRAY_SIZE(g_GicPInterruptTable);
		++GicIndex) {
		if (g_GicPInterruptTable[GicIndex].GicSource != GicSource) {
			continue;
		}
		g_GicPInterruptTable[GicIndex].GicHandler = Handler;
		g_GicPInterruptTable[GicIndex].Data = Data;
		Status = XST_SUCCESS;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This will clear the GIC interrupt.
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrClearStatus(u32 GicPVal, u32 GicPxVal)
{
	u32 GicPMask;
	u32 GicPxMask;

	/* Get the GicP mask */
	GicPMask = (u32)1U << GicPVal;
	GicPxMask = (u32)1U << GicPxVal;
	/* Clear interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP_PMC_IRQ_STATUS, GicPMask, GicPMask);
	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_STATUS + (GicPVal * XPLMI_GICPX_LEN),
		GicPxMask, GicPxMask);
}

/*****************************************************************************/
/**
 * @brief	This will enable the GIC interrupt.
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrEnable(u32 GicPVal, u32 GicPxVal)
{
	u32 GicPMask;
	u32 GicPxMask;

	/* Get the GicP mask */
	GicPMask = (u32)1U << GicPVal;
	GicPxMask = (u32)1U << GicPxVal;

	/* Enable interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP_PMC_IRQ_ENABLE, GicPMask, GicPMask);
	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_ENABLE + (GicPVal * XPLMI_GICPX_LEN),
		GicPxMask, GicPxMask);
}

/*****************************************************************************/
/**
 * @brief	This will disable the GIC interrupt.
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrDisable(u32 GicPVal, u32 GicPxVal)
{
	u32 GicPxMask;

	/* Get the GicP mask */
	GicPxMask = (u32)1U << GicPxVal;
	/* Disable interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_DISABLE + (GicPVal * XPLMI_GICPX_LEN),
		GicPxMask, GicPxMask);
}

/*****************************************************************************/
/**
 * @brief	This is handler for GIC interrupts.
 *
 * @param	CallbackRef is presently the interrupt number that is received
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrHandler(void *CallbackRef)
{
	u32 GicPIntrStatus;
	u32 GicPNIntrStatus;
	u32 GicPNIntrMask;
	u32 GicIndex;
	u32 GicPIndex;
	u32 GicOffset;
	u32 GicPMask;
	u32 GicMask;
	u32 GicTableIndex;
	u32 PlmIntrId;
	u32 Index;

	(void)CallbackRef;

	GicPIntrStatus = XPlmi_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS);
	XPlmi_Printf(DEBUG_DETAILED,
			"GicPIntrStatus: 0x%x\r\n", GicPIntrStatus);

	for (GicIndex = 0U; GicIndex < XPLMI_GICP_SOURCE_COUNT; GicIndex++) {
		GicMask = (u32)1U << GicIndex;
		if ((GicPIntrStatus & GicMask) == 0U) {
			continue;
		}
		GicOffset = GicIndex * XPLMI_GICPX_LEN;
		GicPNIntrStatus = XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_STATUS +
			GicOffset);
		GicPNIntrMask = XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_MASK +
			GicOffset);
		XPlmi_Printf(DEBUG_DETAILED, "GicP%u Intr Status: 0x%x\r\n",
			GicIndex, GicPNIntrStatus);

		for (GicPIndex = 0U; GicPIndex < XPLMI_NO_OF_BITS_IN_REG;
			GicPIndex++) {
			GicPMask = (u32)1U << GicPIndex;
			if ((GicPNIntrStatus & GicPMask) == 0U) {
				continue;
			}
			if ((GicPNIntrMask & GicPMask) != 0U) {
				continue;
			}
			GicTableIndex = (GicIndex << XPLMI_GICP_SOURCE_SHIFT) |
				GicPIndex;
			for (Index = 0U;
				Index < XPLMI_ARRAY_SIZE(g_GicPInterruptTable);
				++Index) {
				if (g_GicPInterruptTable[Index].GicSource ==
					GicTableIndex) {
					break;
				}
			}
			if (Index == XPLMI_ARRAY_SIZE(g_GicPInterruptTable)) {
				continue;
			}

			if(g_GicPInterruptTable[Index].GicHandler != NULL) {
				PlmIntrId = ((GicIndex << 8U) | (GicPIndex << 16U));
				XPlmi_GicIntrAddTask(PlmIntrId);
				XPlmi_Out32((PMC_GLOBAL_GICP0_IRQ_DISABLE + GicOffset), GicPMask);
			}
			else {
				XPlmi_Printf(DEBUG_GENERAL, "%s: Error: Unhandled GIC interrupt"
					" received\n\r", __func__);
			}
			XPlmi_Out32(PMC_GLOBAL_GICP0_IRQ_STATUS + GicOffset, GicPMask);
		}
		XPlmi_Out32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS, GicMask);
	}

	return;
}

/*****************************************************************************/
/**
 * @brief	This function adds the GiCTaskHandler to the TaskQueue.
 *
 * @param	Index is the interrupt index with GicPx and its corresponding
 *              bit details.
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_GicIntrAddTask(u32 Index)
{
	XPlmi_TaskNode *Task = NULL;
	u32 IntrId = Index | XPLMI_IOMODULE_PMC_GIC_IRQ;

	Task = XPlmi_GetTaskInstance(NULL, NULL, IntrId);
	if (Task == NULL) {
		XPlmi_Printf(DEBUG_GENERAL, "GIC Interrrupt add task error\n\r");
		goto END;
	}
	XPlmi_TaskTriggerNow(Task);

END:
	return;
}

/*****************************************************************************/
/**
 * @brief	This is GicTaskhandler with executes the registered handler. In
 * case of SUCCESS or error, IRQ status is cleared and enabled again.
 * If the task is in still progress, then interrupt is not cleared and Task
 * handler is called again from TaskDispatcher.
 *
 * @param	Arg is index of the interrupt handler.
 *
 * @return	None
 *
 *****************************************************************************/
static int XPlmi_GicTaskHandler(void *Arg)
{
	int Status = XST_FAILURE;
	u32 Index = (u32)Arg;
	u32 GicIndex = (Index & XPLMI_GICP_MASK) >> 8U;
	u32 GicPIndex = (Index & XPLMI_GICPX_MASK) >> 16U;
	u32 GicSource;

	GicSource = (GicIndex << XPLMI_GICP_SOURCE_SHIFT) | GicPIndex;

	/*
	 * Call Gic interrupt handler. GIC interrupt handlers should follow
	 * the return values as followed by task handler.
	 * Interrupt clear and enable should be done by Handler only.
	 */
	for (Index = 0U; Index < XPLMI_ARRAY_SIZE(g_GicPInterruptTable); ++Index) {
		if (g_GicPInterruptTable[Index].GicSource == GicSource) {
			Status = g_GicPInterruptTable[Index].GicHandler(
				g_GicPInterruptTable[Index].Data);
			break;
		}
	}

	return Status;
}
