/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdprxss_intr_example.c
*
* This file contains a design example using the XDpRxSs driver in single stream
* (SST) transport or multi-stream transport (MST) mode with interrupts. Upon
* unplug event (DisplayPort cable is unplugged/plugged), DisplayPort RX
* Subsystem will program DP159 disconnect sequence or DP159 training pattern
* 1, 2, 3 sequences respectively.
*
* @note		This example requires an interrupt controller connected to the
*		processor and the DisplayPort RX Subsystem HIP is in the
*		system.
*		For this example to display output, the user need to implement
*		initialization of the system (DpRxSs_PlatformInit), Video Phy
*		(DpRxSs_VideoPhyInit), start DisplayPort RX subsystem
*		(XDpRxSs_Start) and DisplayPort RX Subsystem setup
*		(DpRxSs_Setup).
*		The input to the Subsystem is from RX (GT).
*		The functions DpRxSs_PlatformInit, DpRxSs_VideoPhyInit and
*		DpRxSs_Setup are declared and are left up to the user to
*		implement.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 07/29/15 Initial release.
* 2.00 sha 10/05/15 Changed DpTxSs_Setup --> DpRxSs_Setup.
*                   Removed HDCP callbacks registration and callbacks.
*                   Added HDCP and Timer Counter interrupt handler setup.
* 4.00 ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xintc.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#endif /* XPAR_INTC_0_DEVICE_ID */

/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#ifdef XPAR_INTC_0_DEVICE_ID
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_INTC_0_DPRXSS_0_DPRXSS_DP_IRQ_VEC_ID
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPRXSS_HDCP_INTERRUPT_ID \
	XPAR_INTC_0_DPRXSS_0_DPRXSS_HDCP_IRQ_VEC_ID
#define XINTC_DPRXSS_TMR_INTERRUPT_ID \
	XPAR_INTC_0_DPRXSS_0_DPRXSS_TIMER_IRQ_VEC_ID
#endif
#define XINTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define XINTC				XIntc
#define XINTC_HANDLER			XIntc_InterruptHandler
#else /* Else part */
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_INTC_0_DPRXSS_0_DPRXSS_DP_IRQ_VEC_ID
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPRXSS_HDCP_INTERRUPT_ID \
	XPAR_INTC_0_DPRXSS_0_DPRXSS_HDCP_IRQ_VEC_ID
#define XINTC_DPRXSS_TMR_INTERRUPT_ID \
	XPAR_INTC_0_DPRXSS_0_DPRXSS_TIMER_IRQ_VEC_ID
#endif
#define XINTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC				XScuGic
#define XINTC_HANDLER			XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

/* The unique device ID of the DisplayPort Receiver Subsystem HIP instance
 * to be used
 */
#define XDPRXSS_DEVICE_ID		XPAR_DPRXSS_0_DEVICE_ID

/* Example will run either in MST or SST mode based upon config parameters.
 * In MST mode, this example exposes maximum number of input and output ports
 * of DP RX that will be included while replying to sideband messages from TX.
 */
#define DPRXSS_LINK_RATE		XDPRXSS_LINK_BW_SET_540GBPS
#define DPRXSS_LANE_COUNT		XDPRXSS_LANE_COUNT_SET_4

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DpRxSs_IntrExample(u16 DeviceId);
u32 DpRxSs_PlatformInit(void);
u32 DpRxSs_VideoPhyInit(void);
u32 DpRxSs_Setup(void);


/* Interrupt helper functions */
u32 DpRxSs_SetupIntrSystem(void);

void DpRxSs_VideoModeHandler(void *InstancePtr);
void DpRxSs_PowerChangeHandler(void *InstancePtr);
void DpRxSs_NoVideoHandler(void *InstancePtr);
void DpRxSs_VerticalBlankHandler(void *InstancePtr);
void DpRxSs_TrainingLostHandler(void *InstancePtr);
void DpRxSs_VideoHandler(void *InstancePtr);
void DpRxSs_InfoPacketHandler(void *InstancePtr);
void DpRxSs_ExtPacketHandler(void *InstancePtr);
void DpRxSs_TrainingDoneHandler(void *InstancePtr);
void DpRxSs_BandwidthChangeHandler(void *InstancePtr);
void DpRxSs_DownReqestHandler(void *InstancePtr);
void DpRxSs_DownReplyHandler(void *InstancePtr);
void DpRxSs_AudioOverflowHandler(void *InstancePtr);
void DpRxSs_PayloadAllocationHandler(void *InstancePtr);
void DpRxSs_ActRxHandler(void *InstancePtr);
void DpRxSs_CrcTestHandler(void *InstancePtr);
void DpRxSs_UnplugHandler(void *InstancePtr);
void DpRxSs_LinkBandwidthHandler(void *InstancePtr);
void DpRxSs_PllResetHandler(void *InstancePtr);


/************************** Variable Definitions *****************************/

XDpRxSs DpRxSsInst;	/* The DPRX Subsystem instance.*/
XINTC IntcInst;		/* The interrupt controller instance. */

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs interrupt example. If the
* DpRxSs_IntrExample function which setup the system succeeds, this function
* will wait for the interrupts.
*
* @param	None.
*
* @return
*		- XST_FAILURE if the interrupt example was unsuccessful.
*
* @note		Unless setup failed, main will never return since
*		DpRxSs_IntrExample is blocking (it is waiting on interrupts).
*
******************************************************************************/
int main()
{
	u32 Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("DisplayPort RX Subsystem interrupt example\n\r");
	xil_printf("(c) 2015 by Xilinx\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

	Status = DpRxSs_IntrExample(XDPRXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort RX Subsystem interrupt example failed.");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DisplayPort RX Subsystem interrupt example\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the interrupt example using the
* XDpRxSs driver. This function will setup the system with interrupts handlers.
*
* @param	DeviceId is the unique device ID of the DisplayPort RX
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		  was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
*		order to illustrate interrupt handling taking place for
*		different types interrupts.
*		Refer xdprxss.h file for more info.
*
******************************************************************************/
u32 DpRxSs_IntrExample(u16 DeviceId)
{
	u32 Status;
	XDpRxSs_Config *ConfigPtr;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpRxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
	ConfigPtr = XDpRxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the DpRxSsInst's Config
	 * structure. */
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPRXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpRxSsInst.UsrOpt.MstSupport) {
		xil_printf("\n\rINFO:DPRXSS is MST enabled. DPRXSS can be "
			"switched to SST/MST\n\r\n\r");
	}
	else {
		xil_printf("\n\rINFO:DPRXSS is SST enabled. DPRXSS works "
			"only in SST mode.\n\r\n\r");
	}

	Status = DpRxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\n\r");
		return XST_FAILURE;
	}

	/* Set Link rate and lane count to maximum */
	XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

	/* Start DPRX Subsystem set */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return XST_FAILURE;
	}

	/* Setup Video Phy, left to the user for implementation */
	DpRxSs_VideoPhyInit();

	/* Setup DPRX SS, left to the user for implementation */
	DpRxSs_Setup();

	/* Do not return in order to allow interrupt handling to run. Different
	 * types of interrupts will be detected and handled.
	 */
	while (1);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required platform specific peripherals.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if required peripherals are initialized and
*		configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_PlatformInit(void)
{
	/* User is responsible to setup platform specific initialization */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures Video Phy.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if Video Phy configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_VideoPhyInit(void)
{
	/* User is responsible to setup Video Phy */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures DisplayPort RX Subsystem.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if DP RX Subsystem configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_Setup(void)
{
	/* User is responsible to setup Video Phy */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* DisplayPort RX Subsystem core. The function is application-specific since
* the actual system may or may not have an interrupt controller. The DPRX
* Subsystem core could be directly connected to a processor without an
* interrupt controller. The user should modify this function to fit the
* application.
*
* @param	None
*
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	/* Set callbacks for all the interrupts */
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VM_CHG_EVENT,
				DpRxSs_VideoModeHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,
				DpRxSs_PowerChangeHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_EVENT,
				DpRxSs_NoVideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_EVENT,
				DpRxSs_VerticalBlankHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TLOST_EVENT,
				DpRxSs_TrainingLostHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_EVENT,
				DpRxSs_VideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
				DpRxSs_InfoPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
				DpRxSs_ExtPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
				DpRxSs_TrainingDoneHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_BW_CHG_EVENT,
				DpRxSs_BandwidthChangeHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,
				DpRxSs_DownReqestHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REP_EVENT,
				DpRxSs_DownReplyHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_AUD_OVRFLW_EVENT,
				DpRxSs_AudioOverflowHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst,
			XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,
				DpRxSs_PayloadAllocationHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ACT_RX_EVENT,
				DpRxSs_ActRxHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,
				DpRxSs_CrcTestHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_UNPLUG_EVENT,
				DpRxSs_UnplugHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_LINKBW_EVENT,
				DpRxSs_LinkBandwidthHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_PLL_RESET_EVENT,
				DpRxSs_PllResetHandler, &DpRxSsInst);

#ifdef XPAR_INTC_0_DEVICE_ID
	/* Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstPtr, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\n\r");
		return XST_FAILURE;
	}

	/* Hook up interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
			(XInterruptHandler)XDpRxSs_DpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPRXSS_HDCP_INTERRUPT_ID,
			(XInterruptHandler)XDpRxSs_HdcpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS HDCP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstPtr, XINTC_DPRXSS_TMR_INTERRUPT_ID,
			(XInterruptHandler)XDpRxSs_TmrCtrIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS Timer Counter interrupt connect "
			"failed!\n\r");
		return XST_FAILURE;
	}

	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(IntcInstPtr, XINTC_DPRXSS_HDCP_INTERRUPT_ID);
	XIntc_Enable(IntcInstPtr, XINTC_DPRXSS_TMR_INTERRUPT_ID);
#endif

	/* Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#else
	/* The configuration parameters of the interrupt controller */
	XScuGic_Config *IntcConfig;

	/* Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(XINTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcConfig,
				IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
			(Xil_InterruptHandler)XDpRxSs_DpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Enable the interrupt for the DP device */
	XScuGic_Enable(IntcInstance, XINTC_DPRXSS_DP_INTERRUPT_ID);

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPRXSS_HDCP_INTERRUPT_ID,
			(Xil_InterruptHandler)XDpRxSs_HdcpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS HDCP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPRXSS_TMR_INTERRUPT_ID,
			(Xil_InterruptHandler)XDpRxSs_TmrCtrIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS Timer Counter interrupt connect "
			"failed!\n\r");
		return XST_FAILURE;
	}

	/* Enable the interrupt device */
	XScuGic_Enable(IntcInstance, XINTC_DPRXSS_HDCP_INTERRUPT_ID);
	XScuGic_Enable(IntcInstance, XINTC_DPRXSS_TMR_INTERRUPT_ID);
#endif
#endif
	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception
	 * table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XINTC_HANDLER, IntcInstPtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a video mode change interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_VideoModeHandler(void *InstancePtr)
{
	xil_printf("Interrupt: video mode change.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the power state interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_PowerChangeHandler(void *InstancePtr)
{
	xil_printf("Interrupt: power state change request.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a no video interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_NoVideoHandler(void *InstancePtr)
{
	xil_printf("Interrupt: no-video flags in the VBID field after active "
			"video has been received.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a vertical blank interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_VerticalBlankHandler(void *InstancePtr)
{
	xil_printf("Interrupt: vertical blank.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a training lost interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_TrainingLostHandler(void *InstancePtr)
{
	xil_printf("Interrupt: training done.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a valid video interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_VideoHandler(void *InstancePtr)
{
	xil_printf("Interrupt: valid video.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when an info packet interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_InfoPacketHandler(void *InstancePtr)
{
	xil_printf("Interrupt: info packet.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when arrival of external (audio)
* packet interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_ExtPacketHandler(void *InstancePtr)
{
	xil_printf("Interrupt: external/audio packet.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the training done interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_TrainingDoneHandler(void *InstancePtr)
{
	xil_printf("Interrupt: training done.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a bandwidth change interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_BandwidthChangeHandler(void *InstancePtr)
{
	xil_printf("Interrupt: bandwidth change request.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a down request interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_DownReqestHandler(void *InstancePtr)
{
	xil_printf("Interrupt: down request.\n\r");

	XDpRxSs_HandleDownReq(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a down reply interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_DownReplyHandler(void *InstancePtr)
{
	xil_printf("Interrupt: down reply.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when an audio overflow interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_AudioOverflowHandler(void *InstancePtr)
{
	xil_printf("Interrupt: audio packet overflow.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a payload allocation
* interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_PayloadAllocationHandler(void *InstancePtr)
{
	xil_printf("Interrupt: payload allocation.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when an ACT sequence interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_ActRxHandler(void *InstancePtr)
{
	xil_printf("Interrupt: ACT sequence received.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a CRC test start interrupt
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_CrcTestHandler(void *InstancePtr)
{
	xil_printf("Interrupt: CRC test started.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the unplug event occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_UnplugHandler(void *InstancePtr)
{
	xil_printf("Interrupt: unplug event.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the link bandwidth change
* occurs.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_LinkBandwidthHandler(void *InstancePtr)
{
	xil_printf("Interrupt: link bandwidth change request.\n\r");
}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpRxSs_PllResetHandler(void *InstancePtr)
{
	xil_printf("Interrupt: PLL reset request.\n\r");
}
