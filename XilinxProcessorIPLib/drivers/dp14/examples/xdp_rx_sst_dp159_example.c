/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_rx_sst_dp159_example.c
 *
 * Contains a design example using the XDp driver for an DP RX core generated
 * with SST-only functionality.
 * This example runs the programming sequence for the DP159 retimer. An I2C
 * controller will need to exist in the design have a connection to the DP159.
 * A user-defined hook is used for delay. The reasoning behind this is that
 * MicroBlaze sleep is not very accurate without a hardware timer. For systems
 * that have a hardware timer, the user may override the default MicroBlaze
 * sleep with a function that will use the hardware timer.
 * Also, this example sets up the interrupt controller and defines handlers for
 * various interrupt types. In this way, the RX interrupt handler will arbitrate
 * the different interrupts to their respective interrupt handlers defined in
 * this example.
 * This example will print out the detected resolution of the incoming
 * DisplayPort video stream.
 * This example is meant to take in the incoming DisplayPort video stream and
 * pass it through using the Dprd_Vidpipe* functions which are left for the user
 * to implement.
 *
 * @note	This example requires an AXI timer in the system.
 * @note	For this example to work, the user will need to implement
 *		initialization of the system (Dprx_PlatformInit) as this is
 *		system-specific.
 * @note	For this example to display output, the user will need to
 *		implement the Dprx_Vidpipe* functions which configure and
 *		reset the video pipeline as required as this is system-specific.
 * @note	In the presence of a DP159 retimer, an I2C controller needs to
 *		exist in the design to program it. Depending on how the
 *		DP159 connection exists, the user will need to implement
 *		Dprx_Dp159Reset function.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial creation.
 * 2.0   als  07/07/15 Added DP159 programming.
 * 4.0   als  02/07/16 Added end of line reset for reduced blanking.
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
#ifdef XPAR_IIC_0_DEVICE_ID
/* Default behavior: If an I2C controller exists use the first instance to
 * control a DP159 retimer. This is system-specific. */
#define USE_DP159
#include "xiic.h"
#include "xdprxss_dp159.h"
#endif /* XPAR_IIC_0_DEVICE_ID */
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
#ifdef USE_DP159
#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
#define IIC_INTERRUPT_ID	XPAR_INTC_0_IIC_1_VEC_ID
#endif /* USE_DP159 */
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

u32 Dprx_SstExample(XDp *InstancePtr, u16 DeviceId, INTC *IntcPtr, u16 IntrId,
			u16 DpIntrId, XTmrCtr *TimerCounterPtr, u16 TimerId);
static u32 Dprx_SetupExample(XDp *InstancePtr, u16 DeviceId);
static u32 Dprx_SetupTimerHandler(XDp *InstancePtr, XTmrCtr *TimerCounterPtr,
								u16 TimerId);
static u32 Dprx_SetupInterruptHandler(XDp *InstancePtr, INTC *IntcPtr,
						u16 IntrId, u16 DpIntrId);
#ifdef USE_DP159
static u32 Dprx_Dp159Setup(XIic *InstancePtr, u16 DeviceId);
static void Dprx_Dp159Config(XDp *InstancePtr, XIic *IicInstancePtr,
								u8 ConfigType);
#endif /* USE_DP159 */
static void Dprx_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
static void Dprx_ResetVideoOutput(void *InstancePtr);
static void Dprx_DetectResolution(void *InstancePtr);
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

extern void Dprx_VidpipeConfig(XDp *InstancePtr);
extern void Dprx_VidpipeReset(void);
extern void Dprx_Dp159Reset(void);

/*************************** Variable Declarations ****************************/

XDp DpInstance; /* The Dp instance. */
INTC IntcInstance; /* The interrupt controller instance. */
XTmrCtr TimerCounterInst; /* The timer counter instance. */
#ifdef USE_DP159
XIic IicInst; /* The I2C controller instance for DP159 programming. */
#endif /* USE_DP159 */

/* Used by interrupt handlers. */
u8 VBlankEnable;
u8 VBlankCount;

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDp (operating in RX mode)
 * interrupt with timer example. If the Dprx_SstExample function, which sets up
 * the system succeeds, this function will wait for interrupts.
 *
 * @param	None.
 *
 * @return
 *		- XST_FAILURE if the interrupt example was unsuccessful - system
 *		  setup failed.
 *
 * @note	Unless setup failed, main will never return since
 *		Dprx_SstExample is blocking.
 *
*******************************************************************************/
int main(void)
{
	u32 Status;

	/* Run the XDp (in RX mode) interrupt with timer example. */
	Status = Dprx_SstExample(&DpInstance, DPRX_DEVICE_ID,
				&IntcInstance, INTC_DEVICE_ID, DP_INTERRUPT_ID,
				&TimerCounterInst, TMRC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("dp_rx_sst_dp159 Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran dp_rx_sst_dp159 Example\r\n");
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
u32 Dprx_SstExample(XDp *InstancePtr, u16 DeviceId, INTC *IntcPtr, u16 IntrId,
			u16 DpIntrId, XTmrCtr *TimerCounterPtr, u16 TimerId)
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

#ifdef USE_DP159
	/* Initialize and configure the DP159 retimer. */
	Status = Dprx_Dp159Setup(&IicInst, IIC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* USE_DP159 */

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

#ifdef USE_DP159
/******************************************************************************/
/**
 * This function sets up the I2C controller and uses it to initialize the DP159.
 *
 * @param	InstancePtr is a pointer to the XIic instance representing the
 *		I2C controller connected to the same I2C bus which the DP159 is
 *		is addressable from.
 * @param	DeviceId is the device ID of the I2C controller.
 *
 * @return
 *		- XST_SUCCESS if the I2C controller and the DP159 retimer have
 *		  been successfully set up and initialized.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 Dprx_Dp159Setup(XIic *InstancePtr, u16 DeviceId)
{
	u32 Status;
	XIic_Config *ConfigPtr;

	ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(InstancePtr, ConfigPtr,
							ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XIic_DynInit(InstancePtr->BaseAddress);

	/* This is hardware system specific - it is up to the user to implement
	 * this function if needed. In some systems, a GPIO controller can reset
	 * the DP159. In other systems, the reset functionality may be
	 * accomplished using:
	 *	void XDpRxSs_Dp159Reset(XIic *InstancePtr, u8 Reset); */
	Dprx_Dp159Reset();
	/*******************/

	Status = XDpRxSs_Dp159Initialize(InstancePtr);

	return Status;
}

/******************************************************************************/
/**
 * This function reconfigures the DP159 retimer based on the current lane count
 * and link rate configuration.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	IicInstancePtr is a pointer to the XIic instance representing
 *		the I2C controller connected to the same I2C bus which the DP159
 *		is addressable from.
 * @param	ConfigType determines which DP159 programming sequence to use.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Dprx_Dp159Config(XDp *InstancePtr, XIic *IicInstancePtr,
								u8 ConfigType)
{
	u8 LaneCount;
	u8 LinkRate;

	LaneCount = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_OVER_LANE_COUNT_SET);
	LaneCount &= XDP_RX_OVER_LANE_COUNT_SET_MASK;

	LinkRate = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_OVER_LINK_BW_SET);

	XDpRxSs_Dp159Config(IicInstancePtr, ConfigType, LinkRate, LaneCount);
}
#endif /* USE_DP159 */

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
static void Dprx_ResetVideoOutput(void *InstancePtr)
{
	xil_printf("\tDisabling the display timing generator.\n");
	XDp_RxDtgDis(InstancePtr);

	xil_printf("\tResetting the video output pipeline.\n");
	/* This is hardware system specific - it is up to the user to implement
	 * this function if needed. */
	Dprx_VidpipeReset();
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
 *		present in the XDpRxSs_VideoTimingModes table.
 *
*******************************************************************************/
static void Dprx_DetectResolution(void *InstancePtr)
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
	XDp_RxInterruptEnable(InstancePtr, XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
	VBlankCount = 0;

	xil_printf("\tDisabling the display timing generator.\n");
	XDp_RxDtgDis(InstancePtr);
	xil_printf("\tResetting the video output pipeline.\n");
	/* This is hardware system specific - it is up to the user to implement
	 * this function if needed. */
	Dprx_VidpipeReset();
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

#ifdef USE_DP159
	Dprx_Dp159Config(InstancePtr, &IicInst, XDPRXSS_DP159_CT_TP1);
#endif /* XPAR_INTC_0_DEVICE_ID */
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

#ifdef USE_DP159
	Dprx_Dp159Config(InstancePtr, &IicInst, XDPRXSS_DP159_CT_TP2);
#endif /* XPAR_INTC_0_DEVICE_ID */
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

#ifdef USE_DP159
	Dprx_Dp159Config(InstancePtr, &IicInst, XDPRXSS_DP159_CT_TP3);
#endif /* XPAR_INTC_0_DEVICE_ID */
}
