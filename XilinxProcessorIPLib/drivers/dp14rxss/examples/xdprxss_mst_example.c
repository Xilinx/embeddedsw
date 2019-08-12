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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdprxss_mst_example.c
*
* This file contains a design example using the XDpRxSs driver in single stream
* (SST) transport or multi-stream transport (MST) mode.
*
* @note		For this example to display output, the user need to implement
*		initialization of the system (DpRxSs_PlatformInit), Video Phy
*		(DpRxSs_VideoPhyInit), start DisplayPort RX subsystem
*		(XDpRxSs_Start) and DisplayPort RX Subsystem setup
*		(DpRxSs_Setup).
*		The input to the Subsystem is from RX (GT).
*		This example requires an interrupt controller in the system.
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
#include "xtmrctr.h"
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
#define XINTC_DPRXSS_DP_INTERRUPT_ID	XPAR_INTC_0_DPRXSS_0_DPRXSS_DP_IRQ_VEC_ID
#define XINTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define XINTC				XIntc
#define XINTC_HANDLER			XIntc_InterruptHandler
#else /* Else part */
#define XINTC_DPRXSS_DP_INTERRUPT_ID 	XPAR_INTC_0_DPRXSS_0_DPRXSS_DP_IRQ_VEC_ID
#define XINTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC				XScuGic
#define XINTC_HANDLER			XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

/* The unique device ID of the Timer controller instance to be used */
#define XTMR_CTRL_DEVICE_ID		XPAR_TMRCTR_0_DEVICE_ID

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

u32 DpRxSs_MstExample(u16 DeviceId);
u32 DpRxSs_PlatformInit(void);
u32 DpRxSs_VideoPhyInit(void);
u32 DpRxSs_Setup(void);

/* Interrupt helper functions */
u32 DpRxSs_SetupIntrSystem(void);
void DpRxSs_VideoModeHandler(void *InstancePtr);
void DpRxSs_NoVideoHandler(void *InstancePtr);
void DpRxSs_VerticalBlankHandler(void *InstancePtr);
void DpRxSs_TrainingLostHandler(void *InstancePtr);
void DpRxSs_TrainingDoneHandler(void *InstancePtr);
void DpRxSs_DownReqestHandler(void *InstancePtr);
void DpRxSs_DownReplyHandler(void *InstancePtr);
void DpRxSs_UnplugHandler(void *InstancePtr);
void DpRxSs_LinkBandwidthHandler(void *InstancePtr);
void DpRxSs_PllResetHandler(void *InstancePtr);
u32 DpRxSs_SetupTimerHandler(void);
void DpRxSs_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
void DpRxSs_DetectResolution(XDpRxSs *InstancePtr);

/************************** Variable Definitions *****************************/

XDpRxSs DpRxSsInst;	/* The DPRX Subsystem instance.*/
XINTC IntcInst;		/* The interrupt controller instance. */
XTmrCtr TmrCtrInst;	/* The timer counter instance. */

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs SST/MST example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the MST/SST example passed.
*		- XST_FAILURE if the MST/SST example was unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main()
{
	u32 Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("DisplayPort RX Subsystem SST/MST example\n\r");
	xil_printf("(c) 2015 by Xilinx\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

	Status = DpRxSs_MstExample(XDPRXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort RX Subsystem SST/MST example failed."
			"\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DisplayPort RX Subsystem SST/MST example\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the MST/SST example using the
* XDpRxSs driver.
*
* @param	DeviceId is the unique device ID of the DisplayPort RX
*		Subsystem core.
*
* @return
*		- XST_SUCCESS if DisplayPort RX Subsystem configured in SST/MST
*		successfully.
*		- XST_FAILURE, if DisplayPort RX Subsystem failed to configure
*		in SST/MST.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_MstExample(u16 DeviceId)
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
		xil_printf("\n\rINFO:DPRXSS is  SST enabled. DPRXSS works "
			"only in SST mode.\n\r\n\r");
	}

	/* Set up a timer. */
	DpRxSs_SetupTimerHandler();

	Status = DpRxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\n\r");
		return XST_FAILURE;
	}

	/* Set Link rate and lane count to maximum */
	XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

	/* Start DPRXSS */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return XST_FAILURE;
	}

	/* Setup Video Phy, left to the user for implementation */
	DpRxSs_VideoPhyInit();

	/* Setup DPRX SS, left to the user for implementation */
	DpRxSs_Setup();

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
*		- XST_SUCCESS if DPRX Subsystem configured successfully.
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
* This function sets up a custom timer which the driver will use for MicroBlaze
* systems.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if a timer controller exists for use.
*		- XST_FAILURE otherwise.
*
* @note		A timer controller must be present in the system.
*
******************************************************************************/
u32 DpRxSs_SetupTimerHandler(void)
{
	u32 Status;

	Status = XTmrCtr_Initialize(&TmrCtrInst, XTMR_CTRL_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set a custom timer handler for improved delay accuracy on MicroBlaze
	 * systems since the driver does not assume/have a dependency on the
	 * system having a timer in the FPGA.
	 * Note: This only has an affect for MicroBlaze systems since the Zynq
	 * ARM SoC contains a timer, which is used when the driver calls the
	 * delay function. */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &DpRxSs_CustomWaitUs,
					&TmrCtrInst);

	XTmrCtr_SetResetValue(DpRxSsInst.DpPtr->UserTimerPtr, 0, 0);
	XTmrCtr_Reset(DpRxSsInst.DpPtr->UserTimerPtr, 0);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to override the driver's default sleep functionality.
* For MicroBlaze systems, the XDpRxSs_WaitUs driver function's default behavior
* is to use the MB_Sleep function from microblaze_sleep.h, which is implemented
* in software and only has millisecond accuracy. For this reason, using a
* hardware timer is preferable. For ARM/Zynq SoC systems, the SoC's timer is
* used - XDpRxSs_WaitUs will ignore this custom timer handler.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
* @param	MicroSeconds is the number of microseconds to delay/sleep for.
*
* @return	None.
*
* @note		Use the XDpRxSs_SetUserTimerHandler driver function to set this
*		function as the handler for when the XDpRxSs_WaitUs driver
*		function is called.
*
******************************************************************************/
void DpRxSs_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
	u32 TimerVal;

	XTmrCtr_Start(DpRxSsPtr->DpPtr->UserTimerPtr, 0);

	/* Wait specified number of useconds. */
	do {
		TimerVal = XTmrCtr_GetValue(DpRxSsPtr->DpPtr->UserTimerPtr, 0);
	} while (TimerVal < (MicroSeconds *
			(DpRxSsPtr->DpPtr->Config.SAxiClkHz / 1000000)));

	XTmrCtr_Stop(DpRxSsPtr->DpPtr->UserTimerPtr, 0);
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

	/* Set callbacks for the required interrupts */
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VM_CHG_EVENT,
				DpRxSs_VideoModeHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_EVENT,
				DpRxSs_NoVideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_EVENT,
				DpRxSs_VerticalBlankHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TLOST_EVENT,
				DpRxSs_TrainingLostHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
				DpRxSs_TrainingDoneHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,
				DpRxSs_DownReqestHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REP_EVENT,
				DpRxSs_DownReplyHandler, &DpRxSsInst);
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

	/* Enable the interrupt for the Pixel Splitter device */
	XScuGic_Enable(IntcInstance, XINTC_DPRXSS_DP_INTERRUPT_ID);
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
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	xil_printf("Interrupt: video mode change.\n\r");

	if (DpRxSsPtr->VBlankCount >= 20) {
		DpRxSs_DetectResolution(DpRxSsPtr);

		/* User is responsible for resetting the video pipe */
	}
}

/*****************************************************************************/
/**
*
* This function will present the resolution of the incoming video stream.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		The resolution will be rounded up to the nearest resolution
*		present in the XVidC_VideoTimingModes table.
*
******************************************************************************/
void DpRxSs_DetectResolution(XDpRxSs *InstancePtr)
{
	u32 DpHres, DpVres;
	u32 GetResCount = 0;

	do {
		DpHres = XDpRxSs_ReadReg(InstancePtr->DpPtr->Config.BaseAddr,
						XDPRXSS_MSA_HRES);
		DpVres = XDpRxSs_ReadReg(InstancePtr->DpPtr->Config.BaseAddr,
						XDPRXSS_MSA_VRES);
		GetResCount++;
		XDpRxSs_WaitUs(InstancePtr, 1000);
	} while (((DpHres == 0) || (DpVres == 0)) && (GetResCount < 2000));

	xil_printf("\n*** Detected resolution: %u x %u ***\n", DpHres, DpVres);
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
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	xil_printf("Interrupt: down request.\n\r");

	XDpRxSs_HandleDownReq(DpRxSsPtr);
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
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
	u32 Status;

	if (DpRxSsPtr->VBlankEnable) {
		DpRxSsPtr->VBlankCount++;
		xil_printf("Interrupt: vertical blanking (frame %d).\n",
						DpRxSsPtr->VBlankCount);

		/* Wait until 20 frames have been received before forwarding or
		 * outputting any video stream. */
		if (DpRxSsPtr->VBlankCount >= 20) {
			DpRxSsPtr->VBlankEnable = 0;
			DpRxSsPtr->VBlankCount = 0;
			XDp_RxInterruptDisable(DpRxSsPtr->DpPtr,
					XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

			Status = XDpRxSs_CheckLinkStatus(InstancePtr);
			if (Status == XST_SUCCESS) {
				/* User is responsible for resetting the video
				 * pipe.
				 */

				/* Detect resolution */
				DpRxSs_DetectResolution(InstancePtr);
			}
		}
	}
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

	/* User is responsible to take corrective actions */
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

	/* User is responsible to re-initialize Video Phy if link rate
	 * is mismatching.
	 */
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

	/* User is responsible to reset Video Phy */
}
