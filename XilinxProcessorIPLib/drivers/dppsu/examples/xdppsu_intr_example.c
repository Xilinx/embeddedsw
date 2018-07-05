/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * @file xdppsu_intr_example.c
 *
 * Contains a design example using the XDpPsu driver with interrupts. Upon Hot-
 * Plug-Detect (HPD - DisplayPort cable is plugged/unplugged or the monitor is
 * turned on/off), the main link will be trained.
 *
 * @note	This example requires an interrupt controller connected to the
 *		processor and the DisplayPort TX core in the system.
 * @note	For this example to display output, the user will need to
 *		select the Test Pattern Generator or initialize and select an
 *		appropriate source.
 * @note	The functions DpPsu_PlatformInit and DpPsu_StreamSrc* are
 *		declared extern in xdppsu_common_example.h and are left up to the
 *		user to implement.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad 09/04/17 Initial creation.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdppsu_common_example.h"
#include "xscugic.h"


/**************************** Constant Definitions ****************************/


#define DP_INTERRUPT_ID		151
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID


/****************************** Type Definitions ******************************/

#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_DeviceInterruptHandler

/**************************** Function Prototypes *****************************/

u32 DpPsu_IntrExample(XDpPsu *InstancePtr, u16 DeviceId, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XDpPsu_HpdEventHandler HpdEventHandler,
		XDpPsu_HpdPulseHandler HpdPulseHandler);
static u32 DpPsu_SetupInterruptHandler(XDpPsu *InstancePtr, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XDpPsu_HpdEventHandler HpdEventHandler,
		XDpPsu_HpdPulseHandler HpdPulseHandler);
static void DpPsu_HpdEventHandler(void *InstancePtr);
static void DpPsu_HpdPulseHandler(void *InstancePtr);

/**************************** Variable Definitions ****************************/

INTC IntcInstance; /* The interrupt controller instance. */

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDpPsu interrupt example. If the
 * DpPsuIntrExample function, which sets up the system succeeds, this function
 * will wait for interrupts. Once a connection event or pulse is detected, link
 * training will commence (if needed) and a video stream will start being sent
 * over the main link.
 *
 * @param	None.
 *
 * @return
 *		- XST_FAILURE if the interrupt example was unsuccessful - system
 *		  setup failed.
 *
 * @note	Unless setup failed, main will never return since
 *		DpPsuIntrExample is blocking (it is waiting on interrupts for
 *		Hot-Plug-Detect (HPD) events.
 *
*******************************************************************************/
int main(void)
{
	/* Run the XDpPsu interrupt example. */
	DpPsu_IntrExample(&DpPsuInstance, DPPSU_DEVICE_ID,
				&IntcInstance, INTC_DEVICE_ID, DP_INTERRUPT_ID,
				&DpPsu_HpdEventHandler, &DpPsu_HpdPulseHandler);

	return XST_FAILURE;
}

/******************************************************************************/
/**
 * The main entry point for the interrupt example using the XDpPsu driver. This
 * function will set up the system with interrupts and set up the Hot-Plug-Event
 * (HPD) handlers.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 * @param	IntcPtr is a pointer to the interrupt instance.
 * @param	IntrId is the unique device ID of the interrupt controller.
 * @param	DpIntrId is the interrupt ID of the DisplayPort TX connection to
 *		the interrupt controller.
 * @param	HpdEventHandler is a pointer to the handler called when an HPD
 *		event occurs.
 * @param	HpdPulseHandler is a pointer to the handler called when an HPD
 *		pulse occurs.
 *
 * @return
 *		- XST_FAILURE if the system setup failed.
 *		- XST_SUCCESS should never return since this function, if setup
 *		  was successful, is blocking.
 *
 * @note	If system setup was successful, this function is blocking in
 *		order to illustrate interrupt handling taking place for HPD
 *		events.
 *
*******************************************************************************/
u32 DpPsu_IntrExample(XDpPsu *InstancePtr, u16 DeviceId, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XDpPsu_HpdEventHandler HpdEventHandler,
		XDpPsu_HpdPulseHandler HpdPulseHandler)
{
	u32 Status;

	/* Do platform initialization here. This is hardware system specific -
	 * it is up to the user to implement this function. */
	DpPsu_PlatformInit();
	/******************/

	Status = DpPsu_SetupExample(InstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		xil_printf("----------------------\n");

		return XST_FAILURE;
	}

	/* Setup interrupt handling in the system. */
	Status = DpPsu_SetupInterruptHandler(InstancePtr, IntcPtr, IntrId,
	DpIntrId, HpdEventHandler, HpdPulseHandler);
	if (Status != XST_SUCCESS) {
		xil_printf("=======================\n");

		return XST_FAILURE;
	}
	/* Do not return in order to allow interrupt handling to run. HPD events
	 * (connect, disconnect, and pulse) will be detected and handled. */
	while (1);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets up the interrupt system such that interrupts caused by
 * Hot-Plug-Detect (HPD) events and pulses are handled. This function is
 * application-specific for systems that have an interrupt controller connected
 * to the processor. The user should modify this function to fit the
 * application.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 * @param	IntcPtr is a pointer to the interrupt instance.
 * @param	IntrId is the unique device ID of the interrupt controller.
 * @param	DpIntrId is the interrupt ID of the DisplayPort TX connection to
 *		the interrupt controller.
 * @param	HpdEventHandler is a pointer to the handler called when an HPD
 *		event occurs.
 * @param	HpdPulseHandler is a pointer to the handler called when an HPD
 *		pulse occurs.
 *
 * @return
 *		- XST_SUCCESS if the interrupt system was successfully set up.
 *		- XST_FAILURE otherwise.
 *
 * @note	An interrupt controller must be present in the system, connected
 *		to the processor and the DisplayPort TX core.
 *
*******************************************************************************/
static u32 DpPsu_SetupInterruptHandler(XDpPsu *InstancePtr, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XDpPsu_HpdEventHandler HpdEventHandler,
		XDpPsu_HpdPulseHandler HpdPulseHandler)
{
	u32 Status;
	u32 IntrMask = XDPPSU_INTR_HPD_IRQ_MASK | XDPPSU_INTR_HPD_EVENT_MASK;

	/* Set the HPD interrupt handlers. */
	XDpPsu_SetHpdEventHandler(InstancePtr, HpdEventHandler, InstancePtr);
	XDpPsu_SetHpdPulseHandler(InstancePtr, HpdPulseHandler, InstancePtr);

	/* Initialize interrupt controller driver. */

	XScuGic_Config *IntcConfig;

	IntcConfig = XScuGic_LookupConfig(IntrId);
	Status = XScuGic_CfgInitialize(IntcPtr, IntcConfig,
	IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XScuGic_SetPriorityTriggerType(IntcPtr, DpIntrId, 0x0, 0x3);


	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device. */

	Status = XScuGic_Connect(IntcPtr, DpIntrId,
		(Xil_InterruptHandler)XDpPsu_HpdInterruptHandler, InstancePtr);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Start the interrupt controller. */

	XScuGic_Enable(IntcPtr, DpIntrId);

	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception table. */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)INTC_HANDLER, 0);

	/* Enable exceptions. */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	Xil_ExceptionEnable();

	/* Enable DP interrupts. */
	XScuGic_Enable(IntcPtr,DpIntrId);
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_INTR_EN, IntrMask);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function is called when a Hot-Plug-Detect (HPD) event is received by the
 * DisplayPort TX core. The XDPPSU_INTERRUPT_STATUS_HPD_EVENT_MASK bit of the
 * core's XDPPSU_INTERRUPT_STATUS register indicates that an HPD event has
 * occurred.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	Use the XDpPsu_SetHpdEventHandler driver function to set this
 *		function as the handler for HPD pulses.
 *
*******************************************************************************/
static void DpPsu_HpdEventHandler(void *InstancePtr)
{
	XDpPsu *XDpPsu_InstancePtr = (XDpPsu *)InstancePtr;

	if (XDpPsu_IsConnected(XDpPsu_InstancePtr)) {
		xil_printf("+===> HPD connection event detected.\n");

		DpPsu_Run(XDpPsu_InstancePtr);
	}
	else {
		xil_printf("+===> HPD disconnection event detected.\n\n");
	}
}

/******************************************************************************/
/**
 * This function is called when a Hot-Plug-Detect (HPD) pulse is received by the
 * DisplayPort TX core. The XDPPSU_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK bit
 * of the core's XDPPSU_INTERRUPT_STATUS register indicates that an HPD event has
 * occurred.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	Use the XDpPsu_SetHpdPulseHandler driver function to set this
 *		function as the handler for HPD pulses.
 *
*******************************************************************************/
static void DpPsu_HpdPulseHandler(void *InstancePtr)
{
	XDpPsu *XDpPsu_InstancePtr = (XDpPsu *)InstancePtr;

	xil_printf("===> HPD pulse detected.\n");

	DpPsu_Run(XDpPsu_InstancePtr);
}
