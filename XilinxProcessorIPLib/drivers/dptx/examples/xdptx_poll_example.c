/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx_poll_example.c
 *
 * Contains a design example using the XDptx driver with polling. Once the
 * polling detects a Hot-Plug-Detect event (HPD - DisplayPort cable is plugged/
 * unplugged or the monitor is turned on/off), the main link will be trained.
 *
 * @note	For this example to display output, the user will need to
 *		implement initialization of the system (Dptx_PlatformInit) and,
 *		after training is complete, implement configuration of the video
 *		stream source in order to provide the DisplayPort core with
 *		input (Dptx_StreamSrc* - called in xdptx_example_common.c). See
 *		XAPP1178 for reference.
 * @note	The functions Dptx_PlatformInit and Dptx_StreamSrc* are declared
 *		extern in xdptx_example_common.h and are left up to the user to
 *		implement.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  06/17/14 Initial creation.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx_example_common.h"

/**************************** Function Prototypes *****************************/

u32 Dptx_PollExample(XDptx *InstancePtr, u16 DeviceId);
static void Dptx_HpdPoll(XDptx *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDptx polling example.
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
	/* Run the XDptx polling example. */
	Dptx_PollExample(&DptxInstance, DPTX_DEVICE_ID);

	return XST_FAILURE;
}

/******************************************************************************/
/**
 * The main entry point for the polling example using the XDptx driver. This
 * function will set up the system. If this is successful, this example will
 * begin polling the Hot-Plug-Detect (HPD) status registers for HPD events. Once
 * a connection event or a pulse is detected, link training will commence (if
 * needed) and a video stream will start being sent over the main link.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
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
u32 Dptx_PollExample(XDptx *InstancePtr, u16 DeviceId)
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

	XDptx_EnableTrainAdaptive(InstancePtr, TRAIN_ADAPTIVE);
	XDptx_SetHasRedriverInPath(InstancePtr, TRAIN_HAS_REDRIVER);

	/* Continuously poll for HPD events. */
	while (1) {
		Dptx_HpdPoll(InstancePtr);
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function polls the XDPTX_INTERRUPT_SIG_STATE and XDPTX_INTERRUPT_STATUS
 * registers for Hot-Plug-Detect (HPD) events and handles them accordingly. If a
 * connection or pulse event is detected, link training will begin (if required)
 * and a video stream will be initiated.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dptx_HpdPoll(XDptx *InstancePtr)
{
	u32 InterruptSignalState;
	u32 InterruptStatus;
	u32 HpdState;
	u32 HpdEvent;
	u32 HpdPulseDetected;
	u32 HpdDuration;

	/* Read interrupt registers. */
	InterruptSignalState = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
						XDPTX_INTERRUPT_SIG_STATE);
	InterruptStatus = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPTX_INTERRUPT_STATUS);

	/* Check for HPD events. */
	HpdState = InterruptSignalState &
				XDPTX_INTERRUPT_SIG_STATE_HPD_STATE_MASK;
	HpdEvent = InterruptStatus & XDPTX_INTERRUPT_STATUS_HPD_EVENT_MASK;
	HpdPulseDetected = InterruptStatus &
				XDPTX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK;
	if (HpdPulseDetected) {
		HpdDuration = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPTX_HPD_DURATION);
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
		XDptx_DisableMainLink(InstancePtr);
	}
}
