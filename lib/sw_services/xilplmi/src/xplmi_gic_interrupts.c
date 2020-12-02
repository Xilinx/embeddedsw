/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
static struct GicIntrHandlerTable
	g_GicPInterruptTable[XPLMI_GICP_SOURCE_COUNT][XPLMI_NO_OF_BITS_IN_REG] = {
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	[XPLMI_PMC_GIC_IRQ_GICP0][XPLMI_GICP0_SRC27] =
		{(void *)0U, XPlmi_IpiDispatchHandler},
#endif
};

/*****************************************************************************/
/**
 * @brief	This will register the GIC handler.
 *
 * @param	PlmIntrId is the interrupt ID
 * @param	Handler is the interrupt handler
 * @param	Data is the pointer to arguments to interrupt handler
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicRegisterHandler(u32 PlmIntrId, GicIntHandler_t Handler, void *Data)
{
	u32 GicPVal;
	u32 GicPxVal;

	/* Get the GicP mask */
	GicPVal = (PlmIntrId & XPLMI_GICP_MASK) >> 8U;
	GicPxVal = (PlmIntrId & XPLMI_GICPX_MASK) >> 16U;
	/** Register Handler */
	g_GicPInterruptTable[GicPVal][GicPxVal].GicHandler = Handler;
	g_GicPInterruptTable[GicPVal][GicPxVal].Data = Data;
}

/*****************************************************************************/
/**
 * @brief	This will clear the GIC interrupt.
 *
 * @param	PlmIntrId PLM interrupt Id
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrClearStatus(u32 PlmIntrId)
{
	u32 GicPVal;
	u32 GicPxVal;

	/* Get the GicP mask */
	GicPVal = (PlmIntrId & XPLMI_GICP_MASK) >> 8U;
	GicPxVal = (PlmIntrId & XPLMI_GICPX_MASK) >> 16U;
	/* Enable interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP_PMC_IRQ_STATUS,
		(u32)1U << GicPVal,
		(u32)1U << GicPVal);

	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_STATUS + (GicPVal * XPLMI_GICPX_LEN),
		(u32)1U << GicPxVal,
		(u32)1U << GicPxVal);
}

/*****************************************************************************/
/**
 * @brief	This will enable the GIC interrupt.
 *
 * @param	PlmIntrId is the interrupt Id
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrEnable(u32 PlmIntrId)
{
	u32 GicPVal;
	u32 GicPxVal;

	/* Get the GicP mask */
	GicPVal = (PlmIntrId & XPLMI_GICP_MASK) >> 8U;
	GicPxVal = (PlmIntrId & XPLMI_GICPX_MASK) >> 16U;
	/* Enable interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP_PMC_IRQ_ENABLE,
		(u32)1U << GicPVal,
		(u32)1U << GicPVal);

	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_ENABLE + (GicPVal * XPLMI_GICPX_LEN),
		(u32)1U << GicPxVal,
		(u32)1U << GicPxVal);
}

/*****************************************************************************/
/**
 * @brief	This will disable the GIC interrupt.
 *
 * @param	PlmIntrId  is the interrupt Id
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrDisable(u32 PlmIntrId)
{
	u32 GicPVal;
	u32 GicPxVal;

	/* Get the GicP mask */
	GicPVal = (PlmIntrId & XPLMI_GICP_MASK) >> 8U;
	GicPxVal = (PlmIntrId & XPLMI_GICPX_MASK) >> 16U;
	/* Disable interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_DISABLE + (GicPVal * XPLMI_GICPX_LEN),
		(u32)1U << GicPxVal, (u32)1U << GicPxVal);
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

	(void )CallbackRef;

	GicPIntrStatus = XPlmi_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS);
	XPlmi_Printf(DEBUG_DETAILED,
			"GicPIntrStatus: 0x%x\r\n", GicPIntrStatus);

	for (GicIndex = 0U; GicIndex < XPLMI_GICP_SOURCE_COUNT; GicIndex++) {
		if ((GicPIntrStatus & ((u32)1U << GicIndex)) != (u32)FALSE) {
			GicPNIntrStatus =
				XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_STATUS +
				(GicIndex * XPLMI_GICPX_LEN));
			GicPNIntrMask =
				XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_MASK +
				(GicIndex * XPLMI_GICPX_LEN));
			XPlmi_Printf(DEBUG_DETAILED,
				"GicP%u Intr Status: 0x%x\r\n",
				GicIndex, GicPNIntrStatus);

			for (GicPIndex = 0U;
				GicPIndex < XPLMI_NO_OF_BITS_IN_REG; GicPIndex++) {
				if (((GicPNIntrStatus & ((u32)1U << GicPIndex)) != (u32)FALSE) &&
					((GicPNIntrMask & ((u32)1U << GicPIndex)) == (u32)FALSE)) {
					if(g_GicPInterruptTable[GicIndex][GicPIndex].GicHandler != NULL) {
						XPlmi_GicIntrAddTask((GicIndex << 8U) |
							(GicPIndex << 16U));
						XPlmi_Out32(PMC_GLOBAL_GICP0_IRQ_DISABLE +
							(GicIndex * XPLMI_GICPX_LEN),
							(u32)1U << GicPIndex);
					} else {
						XPlmi_Printf(DEBUG_GENERAL,
						"%s: Error: Unhandled GIC"
						"interrupt received\n\r", __func__);
					}
					XPlmi_Out32(PMC_GLOBAL_GICP0_IRQ_STATUS +
						(GicIndex * XPLMI_GICPX_LEN),
						(u32)1U << GicPIndex);
				}
			}
			XPlmi_Out32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS, ((u32)1U << GicIndex));
		}
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
	XPlmi_TaskNode *Task;

	Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_0,
			XPlmi_GicTaskHandler, (void *)Index);
	if (Task == NULL) {
		XPlmi_Printf(DEBUG_GENERAL, "GIC Intr Task Creation Err\n\r");
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

	/*
	 * Call Gic interrupt handler. GIC interrupt handlers should follow
	 * the return values as followed by task handler.
	 * Interrupt clear and enable should be done by Handler only.
	 */
	Status = g_GicPInterruptTable[GicIndex][GicPIndex].GicHandler(
				g_GicPInterruptTable[GicIndex][GicPIndex].Data);

	return Status;
}
