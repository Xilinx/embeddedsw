/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_intr_example.c
*
* This file contains a design example using the XDpTxSs driver in single stream
* (SST) transport or multi-stream transport (MST) mode with interrupts. Upon
* Hot-Plug-Detect (HPD - DisplayPort cable is plugged/unplugged or the monitor
* is turned on/off), DisplayPort TX Subsystem will read the capability of RX
* device and re-start the subsystem.
*
* @note		This example requires an interrupt controller connected to the
*		processor and the DisplayPort TX Subsystem HIP in the system.
*		For this example to display output, the user need to implement
*		initialization of the system (DpTxSs_PlatformInit) and after
*		DisplayPort TX subsystem start (XDpTxSs_Start) is complete,
*		implement configuration of the video stream source in order to
*		provide the DisplayPort TX Subsystem HIP input.
*		The functions DpTxSs_PlatformInit and DpTxSs_StreamSrc are
*		declared and are left up to the user implement.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 07/01/15 Initial release.
* 2.00 sha 09/28/15 Added HDCP, Timer Counter interrupt handier registration.
*                   Added set MSA callback.
* 4.1  ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss.h"
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
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_DP_IRQ_VEC_ID
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPTXSS_HDCP_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_HDCP_IRQ_VEC_ID
#define XINTC_DPTXSS_TMR_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_TIMER_IRQ_VEC_ID
#endif
#define XINTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define XINTC				XIntc
#define XINTC_HANDLER			XIntc_InterruptHandler
#else /* Else part */
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_DP_IRQ_VEC_ID
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPTXSS_HDCP_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_HDCP_IRQ_VEC_ID
#define XINTC_DPTXSS_TMR_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_TIMER_IRQ_VEC_ID
#endif
#define XINTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC				XScuGic
#define XINTC_HANDLER			XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

/* The unique device ID of the DisplayPort Transmitter Subsystem HIP instance
 * to be used
 */
#define XDPTXSS_DEVICE_ID		XPAR_DPTXSS_0_DEVICE_ID

/* If set to 1, example will run in MST mode. Otherwise, in SST mode.
 * In MST mode, this example reads the EDID of RX devices if connected in
 * daisy-chain.
 */
#define DPTXSS_MST			1
#define DPTXSS_LINK_RATE		XDPTXSS_LINK_BW_SET_540GBPS
#define DPTXSS_LANE_COUNT		XDPTXSS_LANE_COUNT_SET_4

/* The video resolution from the display mode timings (DMT) table to use for
 * DisplayPort TX Subsystem. It can be set to use preferred video mode for
 * EDID of RX device.
 */
#define DPTXSS_VID_MODE			XVIDC_VM_USE_EDID_PREFERRED

/* The color depth (bits per color component) to use DisplayPort TX
 * Subsystem.
 */
#define DPTXSS_BPC			8

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DpTxSs_IntrExample(u16 DeviceId);
u32 DpTxSs_PlatformInit(void);
u32 DpTxSs_StreamSrc(u8 VerticalSplit);

/* Interrupt helper functions */
u32 DpTxSs_SetupIntrSystem(void);
void DpTxSs_HpdEventHandler(void *InstancePtr);
void DpTxSs_HpdPulseHandler(void *InstancePtr);
void DpTxSs_LaneCountChangeHandler(void *InstancePtr);
void DpTxSs_LinkRateChangeHandler(void *InstancePtr);
void DpTxSs_PeVsAdjustHandler(void *InstancePtr);
void DpTxSs_MsaHandler(void *InstancePtr);
static void DpTxSs_HpdEventCommon(XDpTxSs *InstancePtr, u8 Mode);

/************************** Variable Definitions *****************************/

XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
XINTC IntcInst;		/* The interrupt controller instance. */

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This is the main function for XDpTxSs interrupt example. If the
* DpTxSs_IntrExample function which sets up the system succeeds, this function
* will wait for the interrupts. Once a connection event or pulse is detected,
* DpTxSs will RX device capabilities and re-start the subsystem.
*
* @param	None.
*
* @return
*		- XST_FAILURE if the interrupt example was unsuccessful.
*
* @note		Unless setup failed, main will never return since
*		DpTxSs_IntrExample is blocking (it is waiting on interrupts
*		for Hot-Plug-Detect (HPD) events.
*
******************************************************************************/
int main()
{
	u32 Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("DisplayPort TX Subsystem interrupt example\n\r");
	xil_printf("(c) 2015 by Xilinx\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

	Status = DpTxSs_IntrExample(XDPTXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort TX Subsystem interrupt example failed.");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DisplayPort TX Subsystem interrupt example\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the interrupt example using the
* XDpTxSs driver. This function will set up the system with interrupts and
* set up Hot-Plug-Event (HPD) handlers.
*
* @param	DeviceId is the unique device ID of the DisplayPort TX
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		  was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
*		order to illustrate interrupt handling taking place for HPD
*		events.
*
******************************************************************************/
u32 DpTxSs_IntrExample(u16 DeviceId)
{
	u32 Status;
	XDpTxSs_Config *ConfigPtr;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpTxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

	/* Obtain the device configuration for the DisplayPort TX Subsystem */
	ConfigPtr = XDpTxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the DpTxSsInst's Config
	 * structure. */
	Status = XDpTxSs_CfgInitialize(&DpTxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPTXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpTxSsInst.UsrOpt.MstSupport) {
		xil_printf("\n\rINFO:DPTXSS is MST enabled. DPTXSS can be "
			"switched to SST/MST\n\r\n\r");
	}
	else {
		xil_printf("\n\rINFO:DPTXSS is  SST enabled. DPTXSS works "
			"only in SST mode.\n\r\n\r");
	}

	Status = DpTxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\n\r");
		return XST_FAILURE;
	}

	/* Do not return in order to allow interrupt handling to run. HPD events
	 * (connect, disconnect, and pulse) will be detected and handled.
	 */
	while (1);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required platform-specifc peripherals.
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
u32 DpTxSs_PlatformInit(void)
{
	/* User is responsible to setup platform specific initialization */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function setup stream source to input DisplayPort TX Subsystem.
*
* @param	VerticalSplit specifies whether to split video frame
*		vertically into two different vertical halves.
*		- 1 = Vertically split input frame
*		- 0 = No vertically split input frame.
*
* @return
*		- XST_SUCCESS if stream source is configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_StreamSrc(u8 VerticalSplit)
{
	/* User is responsible to setup stream source to input DPTXSS */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* DisplayPort TX Subsystem core. The function is application-specific since
* the actual system may or may not have an interrupt controller. The DPTX
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
u32 DpTxSs_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	/* Set interrupt handlers. */
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_HPD_EVENT,
				DpTxSs_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_HPD_PULSE,
				DpTxSs_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_LANE_COUNT_CHG,
				DpTxSs_LaneCountChangeHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_LINK_RATE_CHG,
				DpTxSs_LinkRateChangeHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_PE_VS_ADJUST,
				DpTxSs_PeVsAdjustHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, XDPTXSS_HANDLER_DP_SET_MSA,
				DpTxSs_MsaHandler, &DpTxSsInst);

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
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
			(XInterruptHandler)XDpTxSs_DpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID);

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	/* Hook up interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPTXSS_HDCP_INTERRUPT_ID,
			(XInterruptHandler)XDpTxSs_HdcpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Hook up interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPTXSS_TMR_INTERRUPT_ID,
			(XInterruptHandler)XDpTxSs_TmrCtrIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(IntcInstPtr, XINTC_DPTXSS_HDCP_INTERRUPT_ID);
	XIntc_Enable(IntcInstPtr, XINTC_DPTXSS_TMR_INTERRUPT_ID);
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
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
			(Xil_InterruptHandler)XDpTxSs_DpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	/* Enable the interrupt for the Pixel Splitter device */
	XScuGic_Enable(IntcInstance, XINTC_DPTXSS_DP_INTERRUPT_ID);

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	/* Hook up interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPTXSS_HDCP_INTERRUPT_ID,
			(XInterruptHandler)XDpTxSs_HdcpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Hook up interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPTXSS_TMR_INTERRUPT_ID,
			(XInterruptHandler)XDpTxSs_TmrCtrIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(IntcInstPtr, XINTC_DPTXSS_HDCP_INTERRUPT_ID);
	XIntc_Enable(IntcInstPtr, XINTC_DPTXSS_TMR_INTERRUPT_ID);
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
* This function is called when a Hot-Plug-Detect (HPD) event is received by
* the DisplayPort TX Subsystem core.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetCallback driver function to set this
*		function as the handler for HPD event.
*
******************************************************************************/
void DpTxSs_HpdEventHandler(void *InstancePtr)
{
	XDpTxSs *DpTxSsInstance = (XDpTxSs *)InstancePtr;
	u32 IntrMask;
	u32 Status;

	/* Read interrupt */
	IntrMask = XDpTxSs_ReadReg(DpTxSsInstance->DpPtr->Config.BaseAddr,
			(XDPTXSS_INTERRUPT_MASK));

	/* Disable HPD interrupts. */
	XDpTxSs_WriteReg(DpTxSsInstance->DpPtr->Config.BaseAddr,
			(XDPTXSS_INTERRUPT_MASK), IntrMask |
			(XDPTXSS_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK) |
			(XDPTXSS_INTERRUPT_MASK_HPD_EVENT_MASK));

	xil_printf("+===> HPD connection event detected.\n\r");
	if (XDpTxSs_IsConnected(DpTxSsInstance)) {
		Status = XDpTxSs_IsMstCapable(DpTxSsInstance);
		if ((Status == XST_SUCCESS) &&
				DpTxSsInstance->Config.MstSupport) {
			DpTxSs_HpdEventCommon(DpTxSsInstance, 1);
		}
		else if (Status == XST_NO_FEATURE) {
			DpTxSs_HpdEventCommon(DpTxSsInstance, 0);
		}
		else {
			xil_printf("ERR:AUX read transaction failed /timed "
				"out.\n\r");
		}
		xil_printf("+===> HPD Connection event ISR is executed.\n\r");
	}
	else {
		xil_printf("+===> HPD disconnection event detected.\n\r");
	}

	XDpTxSs_WriteReg(DpTxSsInstance->DpPtr->Config.BaseAddr,
		(XDPTXSS_INTERRUPT_MASK), IntrMask);
}

/*****************************************************************************/
/**
*
* This function is called when a Hot-Plug-Detect (HPD) pulse is received by
* the DisplayPort TX Subsystem core.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetCallback driver function to set this
*		function as the handler for HPD pulse.
*
******************************************************************************/
void DpTxSs_HpdPulseHandler(void *InstancePtr)
{
	XDpTxSs *DpTxSsInstance = (XDpTxSs *)InstancePtr;

	/* MST mode */
	if(DpTxSsInstance->UsrOpt.MstSupport == 1) {
		xil_printf("INFO:HPD Pulse event for MST.\n\r");
		DpTxSs_HpdEventCommon(DpTxSsInstance, 1);
	}
	else if (DpTxSsInstance->UsrOpt.MstSupport == 0) {
		xil_printf("INFO:HPD Pulse event for SST.\n\r");
		DpTxSs_HpdEventCommon(DpTxSsInstance, 0);
	}
}

/*****************************************************************************/
/**
*
* This function is called when the lane count change interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetCallback driver function to set this
*		function as the handler for lane count change.
*
******************************************************************************/
void DpTxSs_LaneCountChangeHandler(void *InstancePtr)
{
	xil_printf("Interrupt: lane count change.\n\r");
}

/*****************************************************************************/
/**
*
* This function is called when the link rate change interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetCallback driver function to set this
*		function as the handler for link rate change.
*
******************************************************************************/
void DpTxSs_LinkRateChangeHandler(void *InstancePtr)
{
	xil_printf("Interrupt: link rate change.\n\r");
}

/*****************************************************************************/
/**
*
* This function is called when the pre-emphasis and voltage swing adjustment
* interrupt occurs.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetCallback driver function to set this
*		function as the handler for pre-emphasis and voltage swing
*		adjust.
*
******************************************************************************/
void DpTxSs_PeVsAdjustHandler(void *InstancePtr)
{
	xil_printf("Interrupt: pre-emphasis and voltage swing adjust.\n\r");
}

/*****************************************************************************/
/**
*
* This callback is called when RX MSA values to be copied into TX MSA.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetCallback driver function to set this
*		function as the handler MSA copy.
*
******************************************************************************/
void DpTxSs_MsaHandler(void *InstancePtr)
{
	xil_printf("Interrupt: MSA handler.\n\r");
}

/*****************************************************************************/
/**
*
* This is a common function to set up DisplayPort TX Subsystem for
* Hot-Plug-Event (HPD).
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
* @param	Mode specifies stream transport mode from the following
*		- 1 = Multi Stream Transport
*		- 0 = Single Stream Transport
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DpTxSs_HpdEventCommon(XDpTxSs *InstancePtr, u8 Mode)
{
	u8 VSplitMode;
	u32 Status;

	if (Mode) {
		/* Enable MST mode */
		InstancePtr->UsrOpt.MstSupport =
				InstancePtr->Config.MstSupport;

		/* Restore maximum number of supported streams */
		InstancePtr->UsrOpt.NumOfStreams =
				InstancePtr->Config.NumMstStreams;
	}
	else {
		/* Enable SST mode */
		InstancePtr->UsrOpt.MstSupport = 0;

		/* set maximum number of streams to one */
		InstancePtr->UsrOpt.NumOfStreams = 1;
	}

	/* Set VmId to use EDID preferred timing */
	InstancePtr->UsrOpt.VmId = XVIDC_VM_USE_EDID_PREFERRED;

	/* Read capabilities of RX device */
	XDpTxSs_GetRxCapabilities(InstancePtr);

	/* Set link rate and lane count to maximum */
	XDpTxSs_SetLinkRate(InstancePtr, XDPTXSS_LINK_BW_SET_540GBPS);
	XDpTxSs_SetLaneCount(InstancePtr, XDPTXSS_LANE_COUNT_SET_4);

	do {
		Status = XDpTxSs_Start(InstancePtr);
	} while (Status != XST_SUCCESS);

	if ((InstancePtr->UsrOpt.VmId == (XVIDC_VM_UHD2_60_P)) &&
			(InstancePtr->UsrOpt.MstSupport)) {
		VSplitMode = 1;
	}
	else {
		VSplitMode = 0;
	}

	DpTxSs_StreamSrc(VSplitMode);
}
