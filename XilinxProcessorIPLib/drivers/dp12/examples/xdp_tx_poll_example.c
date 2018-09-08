/*******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_tx_poll_example.c
 *
 * Contains a design example using the XDp driver (operating in TX mode) with
 * polling. Once the polling detects a Hot-Plug-Detect event (HPD - DisplayPort
 * cable is plugged/unplugged or the monitor is turned on/off), the main link
 * will be trained.
 *
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
#include "xil_printf.h"

/**************************** Function Prototypes *****************************/

u32 Dptx_PollExample(XDp *InstancePtr, u16 DeviceId);
static void Dptx_HpdPoll(XDp *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDp polling example.
 *
 * @param	None.
 *
 * @return
 *		- XST_FAILURE if the polling example was unsuccessful - system
 *		  setup failed.
 *
 * @note	Unless setup failed, main will never return since
 *		Dptx_PollExample is blocking (it is continuously polling for
 *		Hot-Plug-Detect (HPD) events.
 *
*******************************************************************************/
int main(void)
{
	/* Run the XDp polling example. */
	Dptx_PollExample(&DpInstance, DPTX_DEVICE_ID);

	xil_printf("dp_tx_poll Example Failed\r\n");
	return XST_FAILURE;
}

/******************************************************************************/
/**
 * The main entry point for the polling example using the XDp driver. This
 * function will set up the system. If this is successful, this example will
 * begin polling the Hot-Plug-Detect (HPD) status registers for HPD events. Once
 * a connection event or a pulse is detected, link training will commence (if
 * needed) and a video stream will start being sent over the main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 *
 * @return
 *		- XST_FAILURE if the system setup failed.
 *		- XST_SUCCESS should never return since this function, if setup
 *		  was successful, is blocking.
 *
 * @note	If system setup was successful, this function is blocking in
 *		order to illustrate polling taking place for HPD events.
 *
*******************************************************************************/
u32 Dptx_PollExample(XDp *InstancePtr, u16 DeviceId)
{
	u32 Status;

	/* Do platform initialization here. This is hardware system specific -
	 * it is up to the user to implement this function. */
	Dptx_PlatformInit();
	/******************/

	Status = Dptx_SetupExample(InstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XDp_TxEnableTrainAdaptive(InstancePtr, TRAIN_ADAPTIVE);
	XDp_TxSetHasRedriverInPath(InstancePtr, TRAIN_HAS_REDRIVER);

	/* Continuously poll for HPD events. */
	while (1) {
		Dptx_HpdPoll(InstancePtr);
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function polls the XDP_TX_INTERRUPT_SIG_STATE and
 * XDP_TX_INTERRUPT_STATUS registers for Hot-Plug-Detect (HPD) events and
 * handles them accordingly. If a connection or pulse event is detected, link
 * training will begin (if required) and a video stream will be initiated.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dptx_HpdPoll(XDp *InstancePtr)
{
	u32 InterruptSignalState;
	u32 InterruptStatus;
	u32 HpdState;
	u32 HpdEvent;
	u32 HpdPulseDetected;
	u32 HpdDuration;

	/* Read interrupt registers. */
	InterruptSignalState = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_SIG_STATE);
	InterruptStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_STATUS);

	/* Check for HPD events. */
	HpdState = InterruptSignalState &
				XDP_TX_INTERRUPT_SIG_STATE_HPD_STATE_MASK;
	HpdEvent = InterruptStatus & XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK;
	HpdPulseDetected = InterruptStatus &
				XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK;
	if (HpdPulseDetected) {
		HpdDuration = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_TX_HPD_DURATION);
	}

	/* HPD event handling. */
	if (HpdState && HpdEvent) {
		xil_printf("+===> HPD connection event detected.\n");

		/* Initiate link training. */
		Dptx_Run(InstancePtr);
	}
	else if (HpdState && HpdPulseDetected && (HpdDuration >= 250)) {
		xil_printf("===> HPD pulse detected.\n");

		/* Re-train if needed. */
		Dptx_Run(InstancePtr);
	}
	else if (!HpdState && HpdEvent) {
		xil_printf("+===> HPD disconnection event detected.\n\n");

		/* Disable main link. */
		XDp_TxDisableMainLink(InstancePtr);
	}
}
