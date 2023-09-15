/******************************************************************************
* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*
 * CONTENT
 * Timer peripheral driver. Code is mostly reused from
 * hello_ttc0_interrupt application.
 */

#include <xil_exception.h>
#include <xil_printf.h>

#include <xttcps.h>
#include "timer.h"
#include "gic_setup.h"
#include "pm_client.h"

volatile u32 TickCount = 0;
static XTtcPs timer0_inst;

/**
 * TickHandler() - interrupt handler for timer 0
 * @timer_inst:	pointer to the timer instance
 */
static void TickHandler(XTtcPs *timer_inst)
{
	u32 int_status = XTtcPs_GetInterruptStatus(timer_inst);

	int_status &= XTtcPs_ReadReg(timer_inst->Config.BaseAddress, XTTCPS_IER_OFFSET);
	XTtcPs_ClearInterruptStatus(timer_inst, int_status);
	TickCount++;
	pm_dbg("Timer0 interrupt handler, tick_count = %d\n", TickCount);
}

/**
 * TimerSetIntervalMode() - This function sets TTC into interval mode
 * @timer_inst	pointer to the timer instance
 * @sec		interval timeout in seconds
 */
static void TimerSetIntervalMode(XTtcPs *TimerInstPtr, u32 PeriodInSec)
{
	/* Stop the timer */
	XTtcPs_Stop(TimerInstPtr);
	/* Set Interval mode */
	XTtcPs_SetOptions(TimerInstPtr, XTTCPS_OPTION_INTERVAL_MODE);
	XTtcPs_SetInterval(TimerInstPtr, (PeriodInSec * COUNT_PER_SEC));
	XTtcPs_ResetCounterValue(TimerInstPtr);
	XTtcPs_SetPrescaler(TimerInstPtr, 15);
	/* Enable interrupt */
	XTtcPs_EnableInterrupts(TimerInstPtr, XTTCPS_IXR_INTERVAL_MASK);
}

/**
 * TimerInit() - initializes timer0 device
 * @timeout	period for the interval timer interrupt generation
 */
s32 TimerInit(u32 PeriodInSec)
{
	s32 status;
	XTtcPs_Config *timer_config = XTtcPs_LookupConfig(TTC0_0_DEVICE_ID);

	if (NULL == timer_config) {
		return XST_FAILURE;
	}

	status = XTtcPs_CfgInitialize(&timer0_inst, timer_config, timer_config->BaseAddress);
	if (XST_SUCCESS != status) {
		return status;
	}

	status = GicSetupInterruptSystem(TTC_INT_ID0,
		&timer0_inst, (Xil_ExceptionHandler) TickHandler);
	if (XST_SUCCESS == status) {
		TimerSetIntervalMode(&timer0_inst, PeriodInSec);
		XTtcPs_Start(&timer0_inst);
	}

	return status;
}

/**
 * TimerConfigure() - configure timer to generate periodic interrupts
 * timer_period		Time between two timer ticks
 *
 * @return		Status of configuration success
 */
s32 TimerConfigure(u32 Period)
{
	s32 ret_status = TimerInit(Period);

	switch (ret_status) {
	case XST_SUCCESS:
		pm_dbg("OK, configured timer\n");
		break;
	case XST_DEVICE_IS_STARTED:
		pm_dbg("WARNING, timer is already counting\n");
		break;
	case XST_FAILURE:
		pm_dbg("ERROR, failed to configure timer\n");
		break;
	default:
		pm_dbg("??? unhandled status %d\n", ret_status);
		break;
	}

	return ret_status;
}
