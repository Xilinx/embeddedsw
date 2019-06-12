/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_rx_mst_example.c
 *
 * Contains a design example using the XDp driver with multi-stream transport
 * (MST) functionality.
 *
 * @note	This example requires an AXI timer in the system.
 * @note	This example requires an interrupt controller in the system.
 * @note	For this example to work, the user will need to implement
 *		initialization of the system (Dprx_PlatformInit) as this is
 *		system-specific.
 * @note	For this example to display output, the user will need to
 *		implement the Dprx_Vidpipe* functions which configure and
 *		reset the video pipeline as required as this is system-specific.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 2.0   als  06/07/15 Initial creation.
 * 4.0   als  02/07/16 Added end of line reset for reduced blanking.
 *                     Allocate payload ISR to call XDp_RxAllocatePayloadStream.
 * 5.1   ms   01/23/17 Added xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"
#include "xparameters.h"
#include "xil_printf.h"
#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xintc.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#endif /* XPAR_INTC_0_DEVICE_ID */
#include "xtmrctr.h"

/**************************** Constant Definitions ****************************/

/* The following constants map to the XPAR parameters created in the
 * xparameters.h file. */
#define DPRX_DEVICE_ID		XPAR_DISPLAYPORT_0_DEVICE_ID
#ifdef XPAR_INTC_0_DEVICE_ID
#define DP_INTERRUPT_ID \
XPAR_PROCESSOR_SUBSYSTEM_INTERCONNECT_AXI_INTC_1_DISPLAYPORT_0_AXI_INT_INTR
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
#define DP_INTERRUPT_ID		XPAR_FABRIC_DISPLAYPORT_0_AXI_INT_INTR
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */
#define TMRC_DEVICE_ID		XPAR_TMRCTR_0_DEVICE_ID

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

u32 Dprx_MstExample(XDp *InstancePtr, u16 DeviceId, INTC *IntcPtr,
	u16 IntrId, u16 DpIntrId, XTmrCtr *TimerCounterPtr, u16 TimerId);
static u32 Dprx_SetupExample(XDp *InstancePtr, u16 DeviceId);
static u32 Dprx_SetupTimerHandler(XDp *InstancePtr, XTmrCtr *TimerCounterPtr,
								u16 TimerId);
static u32 Dprx_SetupInterruptHandler(XDp *InstancePtr, INTC *IntcPtr,
						u16 IntrId, u16 DpIntrId);
static void Dprx_SetupDownTopology(XDp *InstancePtr);
static void Dprx_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
static void Dprx_ResetVideoOutput(XDp *InstancePtr);
static void Dprx_DetectResolution(XDp *InstancePtr);
static void Dprx_InterruptHandlerVmChange(void *InstancePtr);
static void Dprx_InterruptHandlerPowerState(void *InstancePtr);
static void Dprx_InterruptHandlerNoVideo(void *InstancePtr);
static void Dprx_InterruptHandlerVBlank(void *InstancePtr);
static void Dprx_InterruptHandlerTrainingLost(void *InstancePtr);
static void Dprx_InterruptHandlerVideo(void *InstancePtr);
static void Dprx_InterruptHandlerTrainingDone(void *InstancePtr);
static void Dprx_InterruptHandlerBwChange(void *InstancePtr);
static void Dprx_InterruptHandlerTp1(void *InstancePtr);
static void Dprx_InterruptHandlerTp2(void *InstancePtr);
static void Dprx_InterruptHandlerTp3(void *InstancePtr);
static void Dprx_InterruptHandlerDownReq(void *InstancePtr);
static void Dprx_InterruptHandlerDownReply(void *InstancePtr);
static void Dprx_InterruptHandlerAudioOver(void *InstancePtr);
static void Dprx_InterruptHandlerPayloadAlloc(void *InstancePtr);
static void Dprx_InterruptHandlerActRx(void *InstancePtr);
static void Dprx_InterruptHandlerCrcTest(void *InstancePtr);

extern void Dprx_PlatformInit(XDp *InstancePtr);
extern void Dprx_VidpipeConfig(XDp *InstancePtr);
extern void Dprx_VidpipeReset(XDp *InstancePtr);

/*************************** Variable Declarations ****************************/

XDp DpInstance; /* The Dp instance. */
INTC IntcInstance; /* The interrupt controller instance. */
XTmrCtr TimerCounterInst; /* The timer counter instance. */

/* Used by interrupt handlers. */
u8 VBlankEnable;
u8 VBlankCount;

/* A generic EDID structure. */
u8 GenEdid[128] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
	0x61, 0x2c, 0x01, 0x00, 0x78, 0x56, 0x34, 0x12,
	0x01, 0x18, 0x01, 0x04, 0xa0, 0x2f, 0x1e, 0x78,
	0x00, 0xee, 0x95, 0xa3, 0x54, 0x4c, 0x99, 0x26,
	0x0f, 0x50, 0x54, 0x21, 0x08, 0x00, 0x71, 0x4f,
	0x81, 0x80, 0xb3, 0x00, 0xd1, 0xc0, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a,
	0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
	0x45, 0x00, 0x40, 0x84, 0x63, 0x00, 0x00, 0x1e,
	0x00, 0x00, 0x00, 0xff, 0x00, 0x58, 0x49, 0x4c,
	0x44, 0x50, 0x53, 0x49, 0x4e, 0x4b, 0x0a, 0x20,
	0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x58,
	0x49, 0x4c, 0x20, 0x44, 0x50, 0x0a, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd,
	0x00, 0x38, 0x3c, 0x1e, 0x53, 0x10, 0x00, 0x0a,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x39
};

/* A generic DPCD structure. */
u8 GenDpcd[] = {
	0x12, 0x0a, 0x84, 0x01, 0x01, 0x00, 0x00, 0x00, 0x02
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDp (operating in RX mode)
 * interrupt with timer example. If the Dprx_MstExample function, which sets up
 * the system succeeds, this function will wait for interrupts.
 *
 * @param	None.
 *
 * @return
 *		- XST_FAILURE if the interrupt example was unsuccessful - system
 *		  setup failed.
 *
 * @note	Unless setup failed, main will never return since
 *		Dprx_MstExample is blocking.
 *
*******************************************************************************/
int main(void)
{
	u32 Status;

	/* Run the XDp (in RX mode) interrupt with timer example. */
	Status = Dprx_MstExample(&DpInstance, DPRX_DEVICE_ID,
				&IntcInstance, INTC_DEVICE_ID, DP_INTERRUPT_ID,
				&TimerCounterInst, TMRC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("dp_rx_mst Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran dp_rx_mst Example\r\n");
	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * The main entry point for the interrupt with timer example using the XDp
 * driver. This function will set up the system, interrupt controller and
 * interrupt handlers, and the custom sleep handler.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DeviceId is the unique device ID of the DisplayPort RX core
 *		instance.
 * @param	IntcPtr is a pointer to the interrupt instance.
 * @param	IntrId is the unique device ID of the interrupt controller.
 * @param	DpIntrId is the interrupt ID of the DisplayPort RX connection to
 *		the interrupt controller.
 * @param	TimerCounterPtr is a pointer to the timer instance.
 * @param	TimerId is the ID of the timer controller to use for delays.
 *
 * @return
 *		- XST_SUCCESS if the system was set up correctly and link
 *		  training was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dprx_MstExample(XDp *InstancePtr, u16 DeviceId, INTC *IntcPtr,
		u16 IntrId, u16 DpIntrId, XTmrCtr *TimerCounterPtr, u16 TimerId)
{
	u32 Status;

	/* Do platform initialization here. This is hardware system specific -
	 * it is up to the user to implement this function. */
	Dprx_PlatformInit(InstancePtr);
	/*******************/

	Status = Dprx_SetupExample(InstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set up a timer. */
	Dprx_SetupTimerHandler(InstancePtr, TimerCounterPtr, TimerId);

	/* Set up interrupt handling in the system. */
	Status = Dprx_SetupInterruptHandler(InstancePtr, IntcPtr, IntrId,
								DpIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Do not return in order to allow interrupt handling to run. */
	while (1);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will setup and initialize the DisplayPort RX core. The core's
 * configuration parameters will be retrieved based on the configuration
 * to the DisplayPort RX core instance with the specified device ID.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DeviceId is the unique device ID of the DisplayPort RX core
 *		instance.
 *
 * @return
 *		- XST_SUCCESS if the device configuration was found and obtained
 *		  and if the main link was successfully established.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 Dprx_SetupExample(XDp *InstancePtr, u16 DeviceId)
{
	XDp_Config *ConfigPtr;
	u32 Status;

	/* Obtain the device configuration for the DisplayPort RX core. */
	ConfigPtr = XDp_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the InstancePtr's Config
	 * structure. */
	XDp_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddr);

	XDp_RxSetLaneCount(InstancePtr, InstancePtr->Config.MaxLaneCount);
	XDp_RxSetLinkRate(InstancePtr, InstancePtr->Config.MaxLinkRate);

	/* Set up the downstream topology which the RX will respond to sideband
	 * messages with. */
	Dprx_SetupDownTopology(InstancePtr);

	/* Initialize the DisplayPort RX core. */
	Status = XDp_Initialize(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets up a custom timer which the driver will use for MicroBlaze
 * systems.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	TimerCounterPtr is a pointer to the timer instance.
 * @param	TimerId is the ID of the timer controller to use for delays.
 *
 * @return
 *		- XST_SUCCESS if a timer controller exists for use.
 *		- XST_FAILURE otherwise.
 *
 * @note	A timer controller must be present in the system.
 *
*******************************************************************************/
static u32 Dprx_SetupTimerHandler(XDp *InstancePtr, XTmrCtr *TimerCounterPtr,
								u16 TimerId)
{
	u32 Status;

	Status = XTmrCtr_Initialize(TimerCounterPtr, TimerId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set a custom timer handler for improved delay accuracy on MicroBlaze
	 * systems since the driver does not assume/have a dependency on the
	 * system having a timer in the FPGA.
	 * Note: This only has an affect for MicroBlaze systems since the Zynq
	 * ARM SoC contains a timer, which is used when the driver calls the
	 * delay function. */
	XDp_SetUserTimerHandler(InstancePtr, &Dprx_CustomWaitUs,
							TimerCounterPtr);

	XTmrCtr_SetResetValue(InstancePtr->UserTimerPtr, 0, 0);
	XTmrCtr_Reset(InstancePtr->UserTimerPtr, 0);

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
 * @param	DpIntrId is the interrupt ID of the DisplayPort RX connection to
 *		the interrupt controller.
 *
 * @return
 *		- XST_SUCCESS if the interrupt system was successfully set up.
 *		- XST_FAILURE otherwise.
 *
 * @note	An interrupt controller must be present in the system, connected
 *		to the processor and the DisplayPort RX core.
 *
*******************************************************************************/
static u32 Dprx_SetupInterruptHandler(XDp *InstancePtr, INTC *IntcPtr,
						u16 IntrId, u16 DpIntrId)
{
	u32 Status;

	/* Set the HPD interrupt handlers. */
	XDp_RxSetIntrVmChangeHandler(InstancePtr,
			Dprx_InterruptHandlerVmChange, InstancePtr);
	XDp_RxSetIntrPowerStateHandler(InstancePtr,
			Dprx_InterruptHandlerPowerState, InstancePtr);
	XDp_RxSetIntrNoVideoHandler(InstancePtr,
			Dprx_InterruptHandlerNoVideo, InstancePtr);
	XDp_RxSetIntrVBlankHandler(InstancePtr,
			Dprx_InterruptHandlerVBlank, InstancePtr);
	XDp_RxSetIntrTrainingLostHandler(InstancePtr,
			Dprx_InterruptHandlerTrainingLost, InstancePtr);
	XDp_RxSetIntrVideoHandler(InstancePtr,
			Dprx_InterruptHandlerVideo, InstancePtr);
	XDp_RxSetIntrTrainingDoneHandler(InstancePtr,
			Dprx_InterruptHandlerTrainingDone, InstancePtr);
	XDp_RxSetIntrBwChangeHandler(InstancePtr,
			Dprx_InterruptHandlerBwChange, InstancePtr);
	XDp_RxSetIntrTp1Handler(InstancePtr,
			Dprx_InterruptHandlerTp1, InstancePtr);
	XDp_RxSetIntrTp2Handler(InstancePtr,
			Dprx_InterruptHandlerTp2, InstancePtr);
	XDp_RxSetIntrTp3Handler(InstancePtr,
			Dprx_InterruptHandlerTp3, InstancePtr);
	XDp_RxSetIntrDownReqHandler(InstancePtr,
		Dprx_InterruptHandlerDownReq, InstancePtr);
	XDp_RxSetIntrDownReplyHandler(InstancePtr,
		Dprx_InterruptHandlerDownReply, InstancePtr);
	XDp_RxSetIntrAudioOverHandler(InstancePtr,
		Dprx_InterruptHandlerAudioOver, InstancePtr);
	XDp_RxSetIntrPayloadAllocHandler(InstancePtr,
		Dprx_InterruptHandlerPayloadAlloc, InstancePtr);
	XDp_RxSetIntrActRxHandler(InstancePtr,
		Dprx_InterruptHandlerActRx, InstancePtr);
	XDp_RxSetIntrCrcTestHandler(InstancePtr,
		Dprx_InterruptHandlerCrcTest, InstancePtr);

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

	/* Register the interrupt controller handler with the exception
	 * table. */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)INTC_HANDLER, IntcPtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets up the downstream topology that will be used by the RX to
 * respond to down requests coming from the TX.
 * - 4 identical sinks (each with its own global unique identifier) are exposed
 *   to LINK_ADDRESS sideband messages.
 * - The extended display identification data (EDID) will be set to a generic
 *   EDID (GenEdid) for each of the 4 sinks. REMOTE_I2C_READ sideband messages
 *   on address 0x50 will be replied to with the contents set by
 *   XDp_RxSetIicMapEntry for 0x50.
 * - REMOTE_DPCD_READ sideband messages will be responded to with the contents
 *   set for the sink using XDp_RxSetDpcdMap. All sinks are given the a generic
 *   DPCD (GenDpcd).
 * - ENUM_PATH_RESOURCES sideband messages will be responded to with the value
 *   set by XDp_RxMstSetPbn (or 0 if the sink already has a stream allocated to
 *   it).
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_SetupDownTopology(XDp *InstancePtr)
{
	XDp_SbMsgLinkAddressReplyPortDetail Port;
	u8 PortIndex;
	u8 GuidIndex;

	/* Ensure that all ports are not exposed in the link address. */
	for (PortIndex = 0; PortIndex < XDP_MAX_NPORTS; PortIndex++) {
		XDp_RxMstExposePort(InstancePtr, PortIndex, 0);
	}

	/* Configure the commonality between the downstream sink devices. */
	Port.InputPort = 0;
	Port.PeerDeviceType = 0x3;
	Port.MsgCapStatus = 0;
	Port.DpDevPlugStatus = 1;
	Port.LegacyDevPlugStatus = 0;
	Port.DpcdRev = 0x11;
	Port.NumSdpStreams = 0;
	Port.NumSdpStreamSinks = 0;

	/* Configure the unique port number and GUID for all possible downstream
	 * sinks. */
	for (PortIndex = 1; PortIndex < XDP_MAX_NPORTS; PortIndex++) {
		/* Set the GUID to a repeating pattern of the port number. */
		for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
			Port.Guid[GuidIndex] = PortIndex;
		}
		XDp_RxMstSetPort(InstancePtr, PortIndex, &Port);
		/* Port.PortNum is set to the index. */
	}

	/* Set I2C maps. */
	XDp_RxSetIicMapEntry(InstancePtr, 1, 0x50, 128, GenEdid);
	XDp_RxSetIicMapEntry(InstancePtr, 2, 0x50, 128, GenEdid);
	XDp_RxSetIicMapEntry(InstancePtr, 3, 0x50, 128, GenEdid);
	XDp_RxSetIicMapEntry(InstancePtr, 4, 0x50, 128, GenEdid);

	/* Set DPCD maps. */
	XDp_RxSetDpcdMap(InstancePtr, 1, 0, sizeof(GenDpcd), GenDpcd);
	XDp_RxSetDpcdMap(InstancePtr, 2, 0, sizeof(GenDpcd), GenDpcd);
	XDp_RxSetDpcdMap(InstancePtr, 3, 0, sizeof(GenDpcd), GenDpcd);
	XDp_RxSetDpcdMap(InstancePtr, 4, 0, sizeof(GenDpcd), GenDpcd);

	/* Set available PBN. */
	XDp_RxMstSetPbn(InstancePtr, 1, 2560);
	XDp_RxMstSetPbn(InstancePtr, 2, 2560);
	XDp_RxMstSetPbn(InstancePtr, 3, 2560);
	XDp_RxMstSetPbn(InstancePtr, 4, 2560);

	/* Set up the input port and expose it. */
	XDp_RxMstSetInputPort(InstancePtr, 0, NULL);

	/* Expose the ports configured above. Used when replying to a
	 * LINK_ADDRESS request. */
	XDp_RxMstExposePort(InstancePtr, 1, 1);
	XDp_RxMstExposePort(InstancePtr, 2, 1);
	XDp_RxMstExposePort(InstancePtr, 3, 1);
	XDp_RxMstExposePort(InstancePtr, 4, 1);
	/* Make sure that the number of downstream ports matches the number
	 * exposed, otherwise the LINK_ADDRESS reply will be incorrect. */
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
static void Dprx_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
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

/******************************************************************************/
/**
 * This function is used to reset video output for this example.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_ResetVideoOutput(XDp *InstancePtr)
{
	xil_printf("\tDisabling the display timing generator.\n");
	XDp_RxDtgDis(InstancePtr);

	xil_printf("\tResetting the video output pipeline.\n");
	/* This is hardware system specific - it is up to the user to implement
	 * this function if needed. */
	Dprx_VidpipeReset(InstancePtr);
	/*******************/

	xil_printf("\tConfiguring the video output pipeline.\n");
	/* This is hardware system specific - it is up to the user to implement
	 * this function if needed. */
	Dprx_VidpipeConfig(InstancePtr);
	/*******************/

	xil_printf("\tRe-enabling the display timing generator.\n");
	XDp_RxDtgEn(InstancePtr);
}

/******************************************************************************/
/**
 * This function will present the resolution of the incoming video stream.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	The resolution will be rounded up to the nearest resolution
 *		present in the XVidC_VideoTimingModes table.
 *
*******************************************************************************/
static void Dprx_DetectResolution(XDp *InstancePtr)
{
	u32 DpHres, DpVres;
	u32 GetResCount = 0;

	do {
		DpHres = (XDp_ReadReg(((XDp *)InstancePtr)->Config.BaseAddr,
							XDP_RX_MSA_HRES));
		DpVres = (XDp_ReadReg(((XDp *)InstancePtr)->Config.BaseAddr,
							XDP_RX_MSA_VHEIGHT));
		GetResCount++;
		XDp_WaitUs(InstancePtr, 1000);
	} while (((DpHres == 0) || (DpVres == 0)) && (GetResCount < 2000));

	xil_printf("\n*** Detected resolution: %u x %u ***\n", DpHres, DpVres);

	XDp_RxSetLineReset(InstancePtr, 1);
}

/******************************************************************************/
/**
 * This function is the callback function for when a video mode chang interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerVmChange(void *InstancePtr)
{
	u32 Status;

	xil_printf("> Interrupt: video mode change.\n");

	Status = XDp_RxCheckLinkStatus(InstancePtr);
	if ((Status == XST_SUCCESS) && (VBlankCount >= 20)) {
		Dprx_ResetVideoOutput(InstancePtr);
		Dprx_DetectResolution(InstancePtr);
	}
}

/******************************************************************************/
/**
 * This function is the callback function for when the power state interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerPowerState(void *InstancePtr)
{
	xil_printf("> Interrupt: power state change request.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a no video interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerNoVideo(void *InstancePtr)
{
	xil_printf("> Interrupt: no-video flags in the VBID field after active "
						"video has been received.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a vertical blanking interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerVBlank(void *InstancePtr)
{
	u32 Status;

	if (VBlankEnable) {
		VBlankCount++;

		xil_printf("> Interrupt: vertical blanking (frame %d).\n",
								VBlankCount);

		/* Wait until 20 frames have been received before forwarding or
		 * outputting any video stream. */
		if (VBlankCount >= 20) {
			VBlankEnable = 0;
			VBlankCount = 0;
			XDp_RxInterruptDisable(InstancePtr,
					XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

			Status = XDp_RxCheckLinkStatus(InstancePtr);
			if (Status == XST_SUCCESS) {
				Dprx_ResetVideoOutput(InstancePtr);
				Dprx_DetectResolution(InstancePtr);
			}
		}
	}
}

/******************************************************************************/
/**
 * This function is the callback function for when a training lost interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerTrainingLost(void *InstancePtr)
{
	xil_printf("> Interrupt: training lost.\n");

	/* Re-enable vertical blanking interrupt and counter. */
	VBlankEnable = 1;
	VBlankCount = 0;
	XDp_RxInterruptEnable(InstancePtr, XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

	xil_printf("\tDisabling the display timing generator.\n");
	XDp_RxDtgDis(InstancePtr);
	xil_printf("\tResetting the video output pipeline.\n");
	/* This is hardware system specific - it is up to the user to implement
	 * this function if needed. */
	Dprx_VidpipeReset(InstancePtr);
	/*******************/
}

/******************************************************************************/
/**
 * This function is the callback function for when a valid video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerVideo(void *InstancePtr)
{
	xil_printf("> Interrupt: a valid video frame is detected on main "
								"link.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a training done interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerTrainingDone(void *InstancePtr)
{
	xil_printf("> Interrupt: training done.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a bandwidth change interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerBwChange(void *InstancePtr)
{
	xil_printf("> Interrupt: bandwidth change.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a training pattern 1
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerTp1(void *InstancePtr)
{
	xil_printf("> Interrupt: training pattern 1.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a training pattern 2
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerTp2(void *InstancePtr)
{
	xil_printf("> Interrupt: training pattern 2.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a training pattern 3
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerTp3(void *InstancePtr)
{
	xil_printf("> Interrupt: training pattern 3.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a down request interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerDownReq(void *InstancePtr)
{
	xil_printf("> Interrupt: down request.\n");

	XDp_RxHandleDownReq(InstancePtr);
}

/******************************************************************************/
/**
 * This function is the callback function for when a down reply interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerDownReply(void *InstancePtr)
{
	xil_printf("> Interrupt: down reply.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when an audio packet overflow
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerAudioOver(void *InstancePtr)
{
	xil_printf("> Interrupt: audio packet overflow.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a payload allocation
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerPayloadAlloc(void *InstancePtr)
{
	xil_printf("> Interrupt: payload allocation.\n");

	XDp_RxAllocatePayloadStream(InstancePtr);
}

/******************************************************************************/
/**
 * This function is the callback function for when an ACT sequence interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerActRx(void *InstancePtr)
{
	xil_printf("> Interrupt: ACT sequence received.\n");
}

/******************************************************************************/
/**
 * This function is the callback function for when a CRC test start interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_InterruptHandlerCrcTest(void *InstancePtr)
{
	xil_printf("> Interrupt: CRC test started.\n");
}
