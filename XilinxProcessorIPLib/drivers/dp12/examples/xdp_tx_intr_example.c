/*******************************************************************************
 *
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
 * @file xdp_tx_intr_example.c
 *
 * Contains a design example using the XDp driver (operating in TX mode) with
 * interrupts. Upon Hot-Plug-Detect (HPD - DisplayPort cable is
 * plugged/unplugged or the monitor is turned on/off), the main link will be
 * trained.
 *
 * @note	This example requires an interrupt controller connected to the
 *		processor and the DisplayPort TX core in the system.
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
#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xintc.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#endif /* XPAR_INTC_0_DEVICE_ID */

/**************************** Constant Definitions ****************************/

/* The following constants map to the XPAR parameters created in the
 * xparameters.h file. */
#ifdef XPAR_INTC_0_DEVICE_ID
#define DP_INTERRUPT_ID		XPAR_AXI_INTC_1_DISPLAYPORT_0_AXI_INT_INTR
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
#define DP_INTERRUPT_ID		XPAR_FABRIC_DISPLAYPORT_0_AXI_INT_INTR
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */

/****************************** Type Definitions ******************************/

/* Depending on whether the system is a MicroBlaze or ARM/Zynq SoC system,
 * different drivers and associated types will be used. */
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

/**************************** Function Prototypes *****************************/

u32 Dptx_IntrExample(XDp *InstancePtr, u16 DeviceId, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XDp_IntrHandler HpdEventHandler,
		XDp_IntrHandler HpdPulseHandler);
static u32 Dptx_SetupInterruptHandler(XDp *InstancePtr, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XDp_IntrHandler HpdEventHandler,
		XDp_IntrHandler HpdPulseHandler);
static void Dptx_HpdEventHandler(void *InstancePtr);
static void Dptx_HpdPulseHandler(void *InstancePtr);

/**************************** Variable Definitions ****************************/

INTC IntcInstance; /* The interrupt controller instance. */

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDptx interrupt example. If the
 * DptxIntrExample function, which sets up the system succeeds, this function
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
 *		DptxIntrExample is blocking (it is waiting on interrupts for
 *		Hot-Plug-Detect (HPD) events.
 *
*******************************************************************************/
int main(void)
{
	/* Run the XDptx interrupt example. */
	Dptx_IntrExample(&DpInstance, DPTX_DEVICE_ID,
				&IntcInstance, INTC_DEVICE_ID, DP_INTERRUPT_ID,
				&Dptx_HpdEventHandler, &Dptx_HpdPulseHandler);

	xil_printf("dp_tx_intr Example Failed\r\n");
	return XST_FAILURE;
}

/******************************************************************************/
/**
 * The main entry point for the interrupt example using the XDp driver. This
 * function will set up the system with interrupts and set up the Hot-Plug-Event
 * (HPD) handlers.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 Dptx_IntrExample(XDp *InstancePtr, u16 DeviceId, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XDp_IntrHandler HpdEventHandler,
		XDp_IntrHandler HpdPulseHandler)
{
	u32 Status;

	/* Use single-stream transport (SST) mode for this example. */
	XDp_TxMstCfgModeDisable(InstancePtr);

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

	/* Setup interrupt handling in the system. */
	Status = Dptx_SetupInterruptHandler(InstancePtr, IntcPtr, IntrId,
	DpIntrId, HpdEventHandler, HpdPulseHandler);
	if (Status != XST_SUCCESS) {
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
 * @param	InstancePtr is a pointer to the XDp instance.
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
static u32 Dptx_SetupInterruptHandler(XDp *InstancePtr, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XDp_IntrHandler HpdEventHandler,
		XDp_IntrHandler HpdPulseHandler)
{
	u32 Status;

	/* Set the HPD interrupt handlers. */
	XDp_TxSetHpdEventHandler(InstancePtr, HpdEventHandler, InstancePtr);
	XDp_TxSetHpdPulseHandler(InstancePtr, HpdPulseHandler, InstancePtr);

	/* Initialize interrupt controller driver. */
#ifdef XPAR_INTC_0_DEVICE_ID
	Status = XIntc_Initialize(IntcPtr, IntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#else
	XScuGic_Config *IntcConfig;

	IntcConfig = XScuGic_LookupConfig(IntrId);
	Status = XScuGic_CfgInitialize(IntcPtr, IntcConfig,
	IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XScuGic_SetPriorityTriggerType(IntcPtr, DpIntrId, 0xA0, 0x1);
#endif /* XPAR_INTC_0_DEVICE_ID */

	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device. */
#ifdef XPAR_INTC_0_DEVICE_ID
	Status = XIntc_Connect(IntcPtr, DpIntrId,
		(XInterruptHandler)XDp_InterruptHandler, InstancePtr);
#else
	Status = XScuGic_Connect(IntcPtr, DpIntrId,
		(Xil_InterruptHandler)XDp_InterruptHandler, InstancePtr);
#endif /* XPAR_INTC_0_DEVICE_ID */
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Start the interrupt controller. */
#ifdef XPAR_INTC_0_DEVICE_ID
	Status = XIntc_Start(IntcPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XIntc_Enable(IntcPtr, DpIntrId);
#else
	XScuGic_Enable(IntcPtr, DpIntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */

	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception table. */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)INTC_HANDLER, IntcPtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function is called when a Hot-Plug-Detect (HPD) event is received by the
 * DisplayPort TX core. The XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK bit of the
 * core's XDP_TX_INTERRUPT_STATUS register indicates that an HPD event has
 * occurred.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	Use the XDp_TxSetHpdEventHandler driver function to set this
 *		function as the handler for HPD pulses.
 *
*******************************************************************************/
static void Dptx_HpdEventHandler(void *InstancePtr)
{
	XDp *XDp_InstancePtr = (XDp *)InstancePtr;

	if (XDp_TxIsConnected(XDp_InstancePtr)) {
		xil_printf("+===> HPD connection event detected.\n");

		Dptx_Run(XDp_InstancePtr);
	}
	else {
		xil_printf("+===> HPD disconnection event detected.\n\n");
	}
}

/******************************************************************************/
/**
 * This function is called when a Hot-Plug-Detect (HPD) pulse is received by the
 * DisplayPort TX core. The XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK bit
 * of the core's XDP_TX_INTERRUPT_STATUS register indicates that an HPD event
 * has occurred.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	Use the XDp_TxSetHpdPulseHandler driver function to set this
 *		function as the handler for HPD pulses.
 *
*******************************************************************************/
static void Dptx_HpdPulseHandler(void *InstancePtr)
{
	XDp *XDp_InstancePtr = (XDp *)InstancePtr;

	xil_printf("===> HPD pulse detected.\n");

	Dptx_Run(XDp_InstancePtr);
}
