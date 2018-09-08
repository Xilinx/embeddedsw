/*******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_tx_timer_example.c
 *
 * Contains a design example using the XDp driver (operating in TX mode) with a
 * user-defined hook for delay. The reasoning behind this is that MicroBlaze
 * sleep is not very accurate without a hardware timer. For systems that have a
 * hardware timer, the user may override the default MicroBlaze sleep with a
 * function that will use the hardware timer.
 *
 * @note	This example requires an AXI timer in the system.
 * @note	For this example to display output, the user will need to
 *		implement initialization of the system (Dptx_PlatformInit) and,
 *		after training is complete, implement configuration of the video
 *		stream source in order to provide the DisplayPort core with
 *		input (Dptx_StreamSrc* - called in xdp_tx_example_common.c). See
 *		XAPP1178 for reference.
 * @note	The functions Dptx_PlatformInit and Dptx_StreamSrc* are declared
 *		extern in xdp_tx_example_common.h and are left up to the user to
 *		implement.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial creation.
 * 5.1   ms   01/23/17 Added xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp_tx_example_common.h"
#include "xtmrctr.h"
#include "xil_printf.h"

/**************************** Function Prototypes *****************************/

u32 Dptx_TimerExample(XDp *InstancePtr, u16 DeviceId,
		XTmrCtr *TimerCounterPtr, XDp_TimerHandler UserSleepFunc);
static void Dptx_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);

/*************************** Variable Declarations ****************************/

XTmrCtr TimerCounterInst; /* The timer counter instance. */

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDptx timer example.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if the timer example finished successfully.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
int main(void)
{
	u32 Status;

	/* Run the XDptx timer example. */
	Status = Dptx_TimerExample(&DpInstance, DPTX_DEVICE_ID,
					&TimerCounterInst, &Dptx_CustomWaitUs);
	if (Status != XST_SUCCESS) {
		xil_printf("dp_tx_timer Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran dp_tx_timer Example\r\n");
	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * The main entry point for the timer example using the XDp driver. This
 * function will set up the system and the custom sleep handler. If this is
 * successful, link training will commence and a video stream will start being
 * sent over the main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 * @param	TimerCounterPtr is a pointer to the timer instance.
 * @param	UserSleepFunc is a pointer to the custom handler for sleep.
 *
 * @return
 *		- XST_SUCCESS if the system was set up correctly and link
 *		  training was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dptx_TimerExample(XDp *InstancePtr, u16 DeviceId,
		XTmrCtr *TimerCounterPtr, XDp_TimerHandler UserSleepFunc)
{
	u32 Status;

	/* Use single-stream transport (SST) mode for this example. */
	XDp_TxMstCfgModeDisable(InstancePtr);

	/* Do platform initialization here. This is hardware system specific -
	 * it is up to the user to implement this function. */
	Dptx_PlatformInit();
	/*******************/

	/* Set a custom timer handler for improved delay accuracy on MicroBlaze
	 * systems since the driver does not assume/have a dependency on the
	 * system having a timer in the FPGA.
	 * Note: This only has an affect for MicroBlaze systems since the Zynq
	 * ARM SoC contains a timer, which is used when the driver calls the
	 * delay function. */
	XDp_SetUserTimerHandler(InstancePtr, UserSleepFunc, TimerCounterPtr);

	Status = Dptx_SetupExample(InstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XDp_TxEnableTrainAdaptive(InstancePtr, TRAIN_ADAPTIVE);
	XDp_TxSetHasRedriverInPath(InstancePtr, TRAIN_HAS_REDRIVER);

	/* A sink monitor must be connected at this point. See the polling or
	 * interrupt examples for how to wait for a connection event. */
	Status = Dptx_Run(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function is used to override the driver's default sleep functionality.
 * For MicroBlaze systems, the XDp_WaitUs driver function's default behavior
 * is to use the MB_Sleep function from microblaze_sleep.h, which is implemented
 * in software and only has millisecond accuracy. For this reason, using a
 * hardware timer is preferrable. For ARM/Zynq SoC systems, the SoC's timer is
 * used - XDp_WaitUs will ignore this custom timer handler.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	Use the XDp_SetUserTimerHandler driver function to set this
 *		function as the handler for when the XDp_WaitUs driver
 *		function is called.
 *
*******************************************************************************/
static void Dptx_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
	XDp *XDp_InstancePtr = (XDp *)InstancePtr;
	u32 TimerVal;

	XTmrCtr_Start(XDp_InstancePtr->UserTimerPtr, 0);

	/* Wait specified number of useconds. */
	do {
		TimerVal = XTmrCtr_GetValue(XDp_InstancePtr->UserTimerPtr, 0);
	}
	while (TimerVal < (MicroSeconds *
			(XDp_InstancePtr->Config.SAxiClkHz / 1000000)));

	XTmrCtr_Stop(XDp_InstancePtr->UserTimerPtr, 0);
}
