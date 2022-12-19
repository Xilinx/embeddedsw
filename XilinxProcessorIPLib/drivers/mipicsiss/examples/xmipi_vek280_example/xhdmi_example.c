/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_example.c
*
* This file demonstrates how to use Xilinx HDMI TX Subsystem, HDMI RX Subsystem
* and Video PHY Controller drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
*              dd/mm/yy
* ----- ------ -------- --------------------------------------------------
* 1.00  YB     11/05/19 Initial release.
* 1.01  KU     07/07/20 Support for FrameBuffer
*                       Can use FRL on RX and TMDS on TX
*                       Video will be clipped if RX resolution in more than
*                       4K
*                       Support Revision 2 (Pass 4) of OnSemi retimer
*                       Menu option updated for EDID and Training based on
*                       MaxRate configuration
*                       Added support for 16 BPC
* 1.02  KU     30/11/20 AVI InfoFrame Version set to 3 for resolutions
* 			VIC > 127
* 			Onsemi redriver tweaked for TMDS mode in TX
* 1.03  ssh    03/17/21 Added EdidHdmi20_t, PsIic0 and PsIic1 declarations
* 1.04  ssh    04/20/21 Added support for Dynamic HDR and Versal
* 1.05	ssh    07/14/21 Added support for Native Video
* 1.06  KU     30/08/21 RX FRL settings updated for VCU118
* 1.07  ssh    01/28/22 Updated GT Swing settings for VCK190
* 1.08  ssh    02/01/22 Updated Enable CTS Conversion Function
* 1.09	ssh    02/04/22 Added param to support Tx to train at the same rate as Rx
* 1.10  ssh    03/02/22 Added LCPLL and RPLL config for VCK190 Exdes
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_example.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define AUX_FIFO_CLEAR 1

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int I2cMux(void);
int I2cClk(u32 InFreq, u32 OutFreq);

#if defined (__arm__) && (!defined(ARMR5))
int OnBoardSi5324Init(void);
#endif

void Info(void);
void DetailedInfo(void);

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
void UpdateFrameRate(XHdmiphy1 *Hdmiphy1Ptr,
		     XV_HdmiTxSs1 *HdmiTxSs1Ptr,
		     XVidC_FrameRate FrameRate);
void UpdateColorFormat(XHdmiphy1 *Hdmiphy1Ptr,
		       XV_HdmiTxSs1 *HdmiTxSs1Ptr,
		       XVidC_ColorFormat ColorFormat);
void UpdateColorDepth(XHdmiphy1 *Hdmiphy1Ptr,
		      XV_HdmiTxSs1 *HdmiTxSs1Ptr,
		      XVidC_ColorDepth ColorDepth);
void CloneTxEdid(void);
void ResetAuxFifo(void);
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
void Exdes_ConfigureTpgEnableInput(u32 EnableExtSrcInput);
void ResetTpg(void);
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
void ResetInRemap(void);
void ResetOutRemap(void);
#endif
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
void TxInfoFrameReset(void);

/**
 * These functions are used as callbacks to handle the triggers
 * from the hdmi 2.1 tx interrupt controlling layer (state machine)
 * to the example design.
 */
void XV_Tx_HdmiTrigCb_CableConnectionChange(void *InstancePtr);
void XV_Tx_HdmiTrigCb_SetupTxFrlRefClk(void *InstancePtr);
void XV_Tx_HdmiTrigCb_SetupTxTmdsRefClk(void *InstancePtr);
void XV_Tx_HdmiTrigCb_GetFRLClk(void *InstancePtr);
void XV_Tx_HdmiTrigCb_StreamOff(void *InstancePtr);
void XV_Tx_HdmiTrigCb_SetupAudioVideo(void *InstancePtr);
void XV_Tx_HdmiTrigCb_StreamOn(void *InstancePtr);
void XV_Tx_HdmiTrigCb_VidSyncRecv(void *InstancePtr);
void XV_Tx_HdmiTrigCb_EnableCableDriver(void *InstancePtr);
void XV_Tx_HdmiTrigCb_FrlFfeConfigDevice(void *InstancePtr);
void XV_Tx_HdmiTrigCb_FrlConfigDeviceSetup(void *InstancePtr);
void XV_Tx_HdmiTrigCb_ReadyToStartTransmit(void *InstancePtr);

void XV_Tx_Hdcp_EnforceBlanking(void *InstancePtr);
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

void Hdmiphy1ErrorCallback(void *CallbackRef);
void Hdmiphy1ProcessError(void);

#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES)
u32 Exdes_FBInitialize(XV_FrmbufWr_l2 *WrInstancePtr,
		       XV_FrmbufRd_l2 *RdInstancePtr,
		       XGpioPs *rstInstancePtr);
void VidFrameBufRdDone(void *CallbackRef);
void VidFrameBufWrDone(void *CallbackRef);
void ResetFrameBuf(u8 fb_reset);

void XV_ConfigVidFrameBuf_wr(XV_FrmbufWr_l2 *FrmBufWrPtr);
void XV_ConfigVidFrameBuf_rd(XV_FrmbufRd_l2 *FrmBufRdPtr);

#endif

u32 Exdes_LoadHdcpKeys(void *IicPtr);

u32 Exdes_SysTmrInitialize(XHdmi_Exdes *InstancePtr, u32 TmrId,
		u32 TmrIntrId);
u32 Exdes_UsToTicks(u32 TimeInUs, u32 TmrCtrClkFreq);
void Exdes_StartSysTmr(XHdmi_Exdes *InstancePtr, u32 IntervalInMs);
void Exdes_SysTmrCallback(void *CallbackRef, u8 TmrCtrNumber);
void Exdes_SysTimerIntrHandler(void *CallbackRef);
u8 Exdes_LookupVic(XVidC_VideoMode VideoMode);
XIic Iic;
/************************* Variable Definitions *****************************/
/* VPHY structure */
XHdmiphy1       Hdmiphy1;
u8              Hdmiphy1ErrorFlag;
u8              Hdmiphy1PllLayoutErrorFlag;
XVfmc           Vfmc[1];
XV_HdmiC_VrrInfoFrame VrrInforFrame ;
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/* HDMI TX SS structure */
XV_HdmiTxSs1    HdmiTxSs;
u8 HdmiTxErrorFlag = 0;
EdidHdmi EdidHdmi_t;

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/* Test Pattern Generator Structure */
XV_tpg          Tpg;
/* Video Pattern */
XTpg_PatternId  Pattern;
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
XV_axi4s_remap InRemap;
XV_axi4s_remap OutRemap;
#endif

u32 VsyncCounter = 0;
u32 AuxHwFullCounter = 0;

XHdmiC_Aux      AuxFifo[AUXFIFOSIZE];
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/* Variable for pass-through operation */
u8              AuxFifoStartIndex;
u8              AuxFifoEndIndex;
u8              AuxFifoCount;
u8              AuxFifoOvrFlowCnt;

u8 AuxFifoStartFlag = (FALSE);
u32 FrWrDoneCounter = 0;
u32 FrRdDoneCounter = 0;

extern u8 Interrupt = 0;

#endif

#ifdef XPAR_XGPIO_NUM_INSTANCES
/* GPIO structure */
XGpio            Gpio_Tpg_resetn;
#endif

/* Interrupt Controller Data Structure */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
XScuGic          Intc;
#else
XIntc            Intc;
#endif



/* HDMI Application Menu: Data Structure */
extern XHdmi_Menu       HdmiMenu;

/* General HDMI Application Variable Declaration (Scratch Pad) */
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
extern XV_Tx xhdmi_example_tx_controller;
#endif
XHdmi_Exdes xhdmi_exdes_ctrlr;

/* Scratch pad for user enabled printing. */
Exdes_Debug_Printf exdes_debug_print = NULL;
Exdes_Debug_Printf exdes_aux_debug_print = NULL;
Exdes_Debug_Printf exdes_hdcp_debug_print = NULL;

#if defined (VTEM2FSYNC) && (! defined XPAR_XVTC_NUM_INSTANCES)
u8 BaseFrameRate_VRR = 0;
#endif


u8 SinkReady = FALSE;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is used to enable and additional debugging prints
* in the applicaiton.
*
* @param  Printfunc is the function to enable printing.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SetDebugPrintf(Exdes_Debug_Printf PrintFunc)
{
	exdes_debug_print = PrintFunc;
}

/*****************************************************************************/
/**
*
* This function is used to enable and additional debugging prints
* for the Aux Fifo operations in the applicaiton.
*
* @param  Printfunc is the function to enable printing.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SetAuxFifoDebugPrintf(Exdes_Debug_Printf PrintFunc)
{
	exdes_aux_debug_print = PrintFunc;
}

/*****************************************************************************/
/**
*
* This function is used to enable and additional debugging prints
* for the HDCP operations in the applicaiton.
*
* @param  Printfunc is the function to enable printing.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SetHdcpDebugPrintf(Exdes_Debug_Printf PrintFunc)
{
	exdes_hdcp_debug_print = PrintFunc;
}
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function is used to initialize the TX controller data strucuture
* for the HDMI 2.1 TX interrupt controller layer (see xhdmi_example_tx_sm.c/.h)
* Here the interrupt ids of the HDMI 2.1 TX SS and related cores are set and
* the HDMI 2.1 TX interrupt controller layer is initialized.
*
* @param  InstancePtr is the instance of the HDMI 2.1 TX interrupt controller
*         layer data structure.
* @param  HdmiTxSsDevId is the device id of the HDMI 2.1 TX SS.
* @param  VPhyDevId is the device if the associated Hdmi Phy.
*
* @return None.
*
* @note
*
******************************************************************************/
u32 XV_Tx_InitController(XV_Tx *InstancePtr, u32 HdmiTxSsDevId,
		u32 VPhyDevId)
{
	u32 Status = XST_SUCCESS;

	/* Set the state machine controller references. */
	InstancePtr->HdmiTxSs = &HdmiTxSs;
	InstancePtr->VidPhy = &Hdmiphy1;
	InstancePtr->Intc = &Intc;

	/* Setup the System Interrupt vector Id references. */
	XV_Tx_IntrVecId IntrVecIds;
#if defined(__arm__) || (__aarch64__)
#if defined(USE_HDCP_HDMI_TX)
	IntrVecIds.IntrVecId_HdmiTxSs = XPAR_FABRIC_V_HDMITXSS1_0_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_HdmiTxSs = XPAR_FABRIC_V_HDMITXSS1_0_VEC_ID;
#endif /* USE_HDCP_HDMI_TX */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	IntrVecIds.IntrVecId_Hdcp14 =
			XPAR_FABRIC_V_HDMITXSS1_0_HDCP14_IRQ_VEC_ID;
	IntrVecIds.IntrVecId_Hdcp14Timer =
			XPAR_FABRIC_V_HDMITXSS1_0_HDCP14_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp14 = (UINTPTR)(NULL);
	IntrVecIds.IntrVecId_Hdcp14Timer = (UINTPTR)NULL;
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	IntrVecIds.IntrVecId_Hdcp22Timer =
			XPAR_FABRIC_V_HDMITXSS1_0_HDCP22_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp22Timer = (UINTPTR)NULL;
#endif
	IntrVecIds.IntrVecId_VPhy = XPAR_FABRIC_V_HDMIPHY1_0_VEC_ID;
#else /* microblaze */
#if defined(USE_HDCP_HDMI_TX)
	IntrVecIds.IntrVecId_HdmiTxSs = XPAR_INTC_0_V_HDMITXSS1_0_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_HdmiTxSs = XPAR_INTC_0_V_HDMITXSS1_0_VEC_ID;
#endif /* USE_HDCP_HDMI_TX */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	IntrVecIds.IntrVecId_Hdcp14 =
			XPAR_INTC_0_V_HDMITXSS1_0_HDCP14_IRQ_VEC_ID;
	IntrVecIds.IntrVecId_Hdcp14Timer =
			XPAR_INTC_0_V_HDMITXSS1_0_HDCP14_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp14 = (UINTPTR)NULL;
	IntrVecIds.IntrVecId_Hdcp14Timer = (UINTPTR)NULL;
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	IntrVecIds.IntrVecId_Hdcp22Timer =
			XPAR_INTC_0_V_HDMITXSS1_0_HDCP22_TIMER_IRQ_VEC_ID;
#else
	IntrVecIds.IntrVecId_Hdcp22Timer = (UINTPTR)NULL;
#endif
	IntrVecIds.IntrVecId_VPhy = XPAR_INTC_0_V_HDMIPHY1_0_VEC_ID;
#endif /* defined(__arm__) || (__aarch64__) */

	/* Initialize the Video Transmitter for HDMI. */
	Status = XV_Tx_Hdmi_Initialize(InstancePtr, HdmiTxSsDevId,
					VPhyDevId, IntrVecIds);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization of Video "
		           "Transmitter for HDMI failed !!\r\n");
		return XST_FAILURE;
	}

	/* Set the Application version in TXSs driver structure */
	XV_HdmiTxSS1_SetAppVersion(&HdmiTxSs, APP_MAJ_VERSION, APP_MIN_VERSION);

	/* To set the debugging prints in the HDMI 2.1 TX interrupt controlling
	 * layer this function can be initialized by xil_printf or any other
	 * valid printing function. */
	XV_Tx_SetDebugPrints(NULL);
	XV_Tx_SetDebugStateMachinePrints(NULL);
	XV_Tx_SetDebugTxNewStreamSetupPrints(NULL);

	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_CONNECTION_CHANGE,
				(void *)XV_Tx_HdmiTrigCb_CableConnectionChange,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_SETUP_TXTMDSREFCLK,
				(void *)XV_Tx_HdmiTrigCb_SetupTxTmdsRefClk,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
			    XV_TX_TRIG_HANDLER_SETUP_TXFRLREFCLK,
				(void *)XV_Tx_HdmiTrigCb_SetupTxFrlRefClk,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_GET_FRL_CLOCK,
				(void *)XV_Tx_HdmiTrigCb_GetFRLClk,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_SETUP_AUDVID,
				(void *)XV_Tx_HdmiTrigCb_SetupAudioVideo,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_STREAM_ON,
				(void *)XV_Tx_HdmiTrigCb_StreamOn,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_ENABLE_CABLE_DRIVERS,
				(void *)XV_Tx_HdmiTrigCb_EnableCableDriver,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_VSYNC_RECV,
				(void *)XV_Tx_HdmiTrigCb_VidSyncRecv,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_STREAM_OFF,
				(void *)XV_Tx_HdmiTrigCb_StreamOff,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_READYTOSTARTTX,
				(void *)XV_Tx_HdmiTrigCb_ReadyToStartTransmit,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_FRL_FFE_CONFIG_DEVICE,
				(void *)XV_Tx_HdmiTrigCb_FrlFfeConfigDevice,
				(void *)InstancePtr);
	Status |= XV_Tx_SetTriggerCallbacks(InstancePtr,
				XV_TX_TRIG_HANDLER_FRL_CONFIG_DEVICE_SETUP,
				(void *)XV_Tx_HdmiTrigCb_FrlConfigDeviceSetup,
				(void *)InstancePtr);
	return Status;
}
#endif

/*****************************************************************************/
/**
*
* This function is used to initialize the example desing controller
* data strucuture that is used to track the presence and instances
* of the hdmi 2.1 receiver and transmitter.
*
* @param  InstancePtr is the instance of the example design handle
*         data structure.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_InitController(XHdmi_Exdes *InstancePtr)
{
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	InstancePtr->hdmi_tx_ctlr = &xhdmi_example_tx_controller;
#endif

	xhdmi_exdes_ctrlr.IsRxPresent = FALSE;
	xhdmi_exdes_ctrlr.IsTxPresent = FALSE;

	xhdmi_exdes_ctrlr.SystemEvent = FALSE;

	xhdmi_exdes_ctrlr.TxStartTransmit = FALSE;

	Exdes_SetDebugPrintf(NULL);
	Exdes_SetAuxFifoDebugPrintf(NULL);
	Exdes_SetHdcpDebugPrintf(NULL);
}

/*****************************************************************************/
/**
*
* This function is used to initialize the system timer that is used to
* count and generate given periodic interrupts.
*
* @param    InstancePtr is a pointer to the example design handler
* @param    TmrId is the device id of the system timer
* @param    TmrIntrId is the interrupt vector id of the system timer
*
* @return   XST_SUCCESS if successful.
*
* @note	    None.
*
******************************************************************************/
u32 Exdes_SysTmrInitialize(XHdmi_Exdes *InstancePtr, u32 TmrId,
		u32 TmrIntrId)
{
	u32 Status = XST_SUCCESS;

	/* Initialize the system timer */
	Status = XTmrCtr_Initialize(&InstancePtr->SysTmrInst, TmrId);
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Setup the timer interrupt */
#if defined(__arm__) || (__aarch64__)
	Status |= XScuGic_Connect(InstancePtr->hdmi_tx_ctlr->Intc,
			TmrIntrId,
			(XInterruptHandler)Exdes_SysTimerIntrHandler,
			(void *)InstancePtr);

	XScuGic_Enable(InstancePtr->hdmi_tx_ctlr->Intc, TmrIntrId);
#else
	Status |= XIntc_Connect(InstancePtr->hdmi_tx_ctlr->Intc,
			TmrIntrId,
			(XInterruptHandler)Exdes_SysTimerIntrHandler,
			(void *)&xhdmi_exdes_ctrlr);

	XIntc_Enable(InstancePtr->hdmi_tx_ctlr->Intc, TmrIntrId);
#endif
#endif
	return Status;
}

/*****************************************************************************/
/**
*
* This function time specified in Us to ticks for a specified timer frequency.
*
* @param    TimeInUs is the time in microseconds.
* @param    TmrCtrClkFreq is the frequency of the clock to be used for
*           counting the ticks.
*
* @return   Number of ticks.
*
* @note	    None.
*
******************************************************************************/
u32 Exdes_UsToTicks(u32 TimeInUs, u32 TmrCtrClkFreq)
{
	u32 TimeoutFreq = 0;
	u32 NumTicks = 0;

	/* Check for greater than one second */
	if (TimeInUs > 1000000ul) {
		u32 NumSeconds = 0;

		/* Determine theNumSeconds */
		NumSeconds = (TimeInUs/1000000ul);

		/* Update theNumTicks */
		NumTicks = (NumSeconds*TmrCtrClkFreq);

		/* Adjust theTimeoutInUs */
		TimeInUs -= (NumSeconds*1000000ul);
	}

	/* Convert TimeoutFreq to a frequency */
	TimeoutFreq  = 1000;
	TimeoutFreq *= 1000;
	TimeoutFreq /= TimeInUs;

	/* Update NumTicks */
	NumTicks += ((TmrCtrClkFreq / TimeoutFreq) + 1);

	return (NumTicks);
}

/*****************************************************************************/
/**
*
* This function is used to start a the timer to generate periodic pulses of
* tmrctr interrupt.
*
* @param    InstancePtr is a pointer to the example design handler.
* @param    IntervalInMs is the user specified period for the timer pulses.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void Exdes_StartSysTmr(XHdmi_Exdes *InstancePtr, u32 IntervalInMs)
{
	u32 NumTicks = 0;
	u32 TimerOptions = 0;
	u8 TmrCtrNumber = 0;

	NumTicks = Exdes_UsToTicks((IntervalInMs * 1000),
			InstancePtr->SysTmrInst.Config.SysClockFreqHz);

	/* Calculate the number of maximum pulses that can counted upto
	 * at which point we need to reset the counters.
	 */
	InstancePtr->SysTmrPulseIntervalinMs = IntervalInMs;

	/* Set the number of pulses in 1 second time. */
	/* The user can add more counts here for 500ms, 2 seconds etc.
	 * and use them to control any time based operations in
	 * Exdes_SysTmrCallback tmrctr callback handler.
	 */
	InstancePtr->TmrPulseCnt1second =
				1000 / InstancePtr->SysTmrPulseIntervalinMs;

	/* Stop the timer */
	XTmrCtr_Stop(&InstancePtr->SysTmrInst, TmrCtrNumber);

	/* Configure the callback */
	XTmrCtr_SetHandler(&InstancePtr->SysTmrInst,
			   &Exdes_SysTmrCallback,
			   (void*)InstancePtr);

	/* Configure the timer options to generate a pulse train of
	 * interrupts of IntervalInMs periods. */
	TimerOptions  = XTmrCtr_GetOptions(&InstancePtr->SysTmrInst,
					   TmrCtrNumber);
	TimerOptions |= (XTC_DOWN_COUNT_OPTION |
					XTC_INT_MODE_OPTION |
					XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetOptions(&InstancePtr->SysTmrInst,
			   TmrCtrNumber, TimerOptions);

	xil_printf("System timer configured to generate (and count)"
			"pulses at %dms intervals.\r\n",
			InstancePtr->SysTmrPulseIntervalinMs);

	/* Set the timeout and start */
	XTmrCtr_SetResetValue(&InstancePtr->SysTmrInst,
			      TmrCtrNumber, NumTicks);
	XTmrCtr_Start(&InstancePtr->SysTmrInst, TmrCtrNumber);
}

/*****************************************************************************/
/**
*
* This function is set as the callback handler from the timer counter
* interrupt handler. The counters that are used in the application for
* timer based control and exclusion are be handled in the this function.
*
* The user needs to modify this callback to set the counters and
* exclusion flags based on the pulse counts in the pulse train generated
* by the system timer.
*
* @param    CallbackRef is the function callback reference.
* @param    TmrCtrNumber is the timer counter number.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void Exdes_SysTmrCallback(void *CallbackRef, u8 TmrCtrNumber)
{
	XHdmi_Exdes *InstancePtr = (XHdmi_Exdes *)CallbackRef;

	/* Increment the hdcp pulse counters. */
	InstancePtr->HdcpPulseCounter++;

	/* Check if the Hdcp pulse count has exceeded the reset value
	 * based on the required time out for exception in HDCP polling to
	 * check downstream authentication. */
	if (InstancePtr->HdcpPulseCounter > InstancePtr->TmrPulseCnt1second) {
		/* Reset the hdcp pulse counter */
		InstancePtr->HdcpPulseCounter = 0;
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
		/* Free the Hdcp to attempt a authentication on
		 * the transmitter if needed. */
		XV_Tx_SetHdcpAuthReqExclusion(InstancePtr->hdmi_tx_ctlr, FALSE);
#endif
	}

	/* Stop the timer */
	/* XTmrCtr_Stop(&InstancePtr->SysTmrInst, TmrCtrNumber); */
}

/*****************************************************************************/
/**
*
* This function is set as the callback handler from the interrupt controller
* for the system timer interrupts.
*
* @param    InstancePtr is the callback reference.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void Exdes_SysTimerIntrHandler(void *CallbackRef)
{
	XHdmi_Exdes *InstancePtr = (XHdmi_Exdes *)CallbackRef;
	XTmrCtr_InterruptHandler(&InstancePtr->SysTmrInst);
}


u8 Exdes_LookupVic(XVidC_VideoMode VideoMode)
{
	XHdmiC_VicTable const *Entry;
	u8 Index;

	for (Index = 0; Index < sizeof(VicTable)/sizeof(XHdmiC_VicTable);
		Index++) {
	  Entry = &VicTable[Index];
	  if (Entry->VmId == VideoMode)
		return (Entry->Vic);
	}
	return 0;
}

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function sets the colorbar output of the TX with new stream values.
*
* @param  VideoMode is the video mode value for the new stream.
* @param  ColorFormat is the color format value for the new stream.
* @param  Bpc is the bit per color value for the new stream.
*
* @return None.
*
* @note   This API can alternatively be implemented as a generic API in the
*         HDMI 2.1 TX interrupt controlling layer.
*         An example of such an API maybe,
*         XV_Tx_UpdateVidStream(XV_Tx *InstancePtr, VideoMode, Colorformat, Bpc)
*
******************************************************************************/
void Exdes_ChangeColorbarOutput(XVidC_VideoMode   VideoMode,
				XVidC_ColorFormat ColorFormat,
				XVidC_ColorDepth  Bpc)
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	const XVidC_VideoTimingMode *VidStreamPtr;
	u8 Vic;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
	VidStreamPtr = XVidC_GetVideoModeData(VideoMode);

	/* Set the new parameters in the video stream. */
	HdmiTxSsVidStreamPtr->Timing = VidStreamPtr->Timing;
	HdmiTxSsVidStreamPtr->FrameRate = VidStreamPtr->FrameRate;
	HdmiTxSsVidStreamPtr->VmId = VideoMode;
	HdmiTxSsVidStreamPtr->ColorFormatId = ColorFormat;
	HdmiTxSsVidStreamPtr->ColorDepth = Bpc;

	Vic = Exdes_LookupVic(VideoMode);
	XV_Tx_SetVic(&xhdmi_example_tx_controller, Vic);

	/* Check if Tx and Rx are set to run independently.
	 * If so then disable writing to the aux fifo in Rx Aux,
	 * reset the AUX fifo, reset the AVIInfoframe and
	 * reset the VSInfoframe.
	 */
	if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
		XHdmiC_AVI_InfoFrame *AviInfoFramePtr =
				XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
		XHdmiC_VSIF *VSIFPtr = XV_HdmiTxSs1_GetVSIF(&HdmiTxSs);

		/* Reset Avi InfoFrame */
		(void)memset((void *)AviInfoFramePtr, 0,
			     sizeof(XHdmiC_AVI_InfoFrame));
		/* Reset Vendor Specific InfoFrame */
		(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));

		/* Update AVI InfoFrame */
		AviInfoFramePtr->ColorSpace =
				XV_HdmiC_XVidC_To_IfColorformat(ColorFormat);
		AviInfoFramePtr->VIC = HdmiTxSs.HdmiTx1Ptr->Stream.Vic;

		if ((AviInfoFramePtr->VIC > 127) || (AviInfoFramePtr->ColorSpace > 3)) {
			AviInfoFramePtr->Version = 3;
		} else {
			AviInfoFramePtr->Version = 2;
		}

	}
}
#endif

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function updates the Gaming Feature Settings for the TX stream.
*
* @param  XV_HdmiTxSsVidStreamPtr is the video stream of the
*         HDMI 2.1 TX controller.
* @param VrrEnable is VRR Mode Enabled or not.
* @param FvaFactor is Fva Factor Value.
* @param CnmVrrEnable is CNMVRR is Enabled or not.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_UpdateGamingFeatures(XV_HdmiTxSs1 *HdmiTxSs1Ptr,
				u8 VrrEnable,u8 FvaFactor,u8 CnmVrrEnable)
{
#if (VRR_MODE == 0) // NO VRR
	XV_HdmiTxSS1_SetVrrMode(HdmiTxSs1Ptr,
				FALSE,FALSE,
				FvaFactor,CnmVrrEnable);
#elif (VRR_MODE == 1) // MANUAL STRETCH
	XV_HdmiTxSS1_SetVrrMode(HdmiTxSs1Ptr,
				FALSE,VrrEnable,
				FvaFactor,CnmVrrEnable);
#else
	XV_HdmiTxSS1_SetVrrMode(HdmiTxSs1Ptr,
				TRUE,VrrEnable,
				FvaFactor,CnmVrrEnable);
#endif
}

/*****************************************************************************/
/**
*
* This function updates the AVI Info frame for the TX stream.
*
* @param  XV_HdmiTxSsVidStreamPtr is the video stream of the
*         HDMI 2.1 TX controller.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_UpdateAviInfoFrame(XVidC_VideoStream *HdmiTxSsVidStreamPtr)
{
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XVidC_ColorFormat Colorformat;

	/* Update AVI InfoFrame */
	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
	Colorformat = HdmiTxSs.HdmiTx1Ptr->Stream.Video.ColorFormatId;

	AviInfoFramePtr->ColorSpace = XV_HdmiC_XVidC_To_IfColorformat(Colorformat);
	AviInfoFramePtr->VIC = HdmiTxSs.HdmiTx1Ptr->Stream.Vic;

	if ((AviInfoFramePtr->VIC > 127) || (AviInfoFramePtr->ColorSpace > 3)) {
		AviInfoFramePtr->Version = 3;
	} else {
		AviInfoFramePtr->Version = 2;
	}

	if ((HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x480_60_I) ||
	    (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x576_50_I)) {
		AviInfoFramePtr->PixelRepetition =
				XHDMIC_PIXEL_REPETITION_FACTOR_2;
	} else {
		AviInfoFramePtr->PixelRepetition =
				XHDMIC_PIXEL_REPETITION_FACTOR_1;
	}
}
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function check the capabilities of the sink connected downstream to
* the HDMI 2.1 TX.
*
* @return XST_SUCCESS if the capabilities are successfully read.
*         XST_FAILURE otherwise.
*
* @note
*
******************************************************************************/
u32 Exdes_CheckDwnstrmSinkCaps()
{
	u32 Status = XST_SUCCESS;

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)&HdmiTxSs;
#ifdef XPAR_XV_TPG_NUM_INSTANCES
	/* Reset the TPG */
	ResetTpg();
#endif
#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	ResetInRemap();
	ResetOutRemap();
#endif

	/* Initialize EDID App during cable connect */
	EDIDConnectInit(&EdidHdmi_t);

	/* Read the EDID and the SCDC */
	EdidScdcCheck(HdmiTxSs1Ptr, &EdidHdmi_t);

	/* Obtain the stream information:
	 * Notes: XV_HdmiTxSs1_GetVideoStream are with updated
	 * value, either colorbar or pass-through.
	 */
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Check whether the sink is DVI/HDMI Supported */
	if (EdidHdmi_t.EdidCtrlParam.IsHdmi == XVIDC_ISDVI) {
		if (HdmiTxSsVidStreamPtr->ColorDepth != XVIDC_BPC_8 ||
		    HdmiTxSsVidStreamPtr->ColorFormatId != XVIDC_CSF_RGB) {
			xil_printf(ANSI_COLOR_YELLOW"Un-able to set TX stream,"
					" sink is DVI\r\n"
					ANSI_COLOR_RESET"\r\n");
			/* Don't set TX, if the Sink is DVI, but the source
			 * properties are:
			 *      - Color Depth more than 8 BPC
			 *      - Color Space not RGB
			 */
			return Status;
		} else {
			xil_printf(ANSI_COLOR_YELLOW"Set TX stream to DVI,"
					" sink is DVI\r\n"ANSI_COLOR_RESET"\r\n");
			XV_Tx_SetDviMode(&xhdmi_example_tx_controller);
		}
	} else {
		if (EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp !=
		    XVIDC_MAXFRLRATE_NOT_SUPPORTED) {
			xil_printf(ANSI_COLOR_YELLOW"Set TX stream to HDMI FRL,"
					" sink is HDMI\r\n"
					ANSI_COLOR_RESET"\r\n");
			XV_Tx_SetHdmiFrlMode(&xhdmi_example_tx_controller);
		} else {
			xil_printf(ANSI_COLOR_YELLOW"Set TX stream to HDMI TMDS,"
					" sink is HDMI\r\n"
					ANSI_COLOR_RESET"\r\n");
			XV_Tx_SetHdmiTmdsMode(&xhdmi_example_tx_controller);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function configures and enables the TPG input.
*
* @param  EnableExtSrcInput is a truth value that is used to decide if the
*         TPG should be configured in colorbar or pass-through.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_ConfigureTpgEnableInput(u32 EnableExtSrcInput)
{
	xil_printf("Inside Exdes_ConfigureTpgEnableInput ... \r\n");

#ifdef XPAR_XV_TPG_NUM_INSTANCES
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);

	XV_tpg *pTpg = &Tpg;
	u32 width, height;
	XVidC_VideoMode VideoMode;
	VideoMode = HdmiTxSsVidStreamPtr->VmId;
	ResetTpg();
#endif
#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	ResetInRemap();
	ResetOutRemap();
#endif
#ifdef XPAR_XV_TPG_NUM_INSTANCES
		/* If Color bar, the 480i/576i HActive need to be divided by 2 */
		/* 1440x480i/1440x576i --> 720x480i/720x576i */
		/* NTSC/PAL Support */
		if ((VideoMode == XVIDC_VM_1440x480_60_I) ||
		    (VideoMode == XVIDC_VM_1440x576_50_I)) {
			width  = HdmiTxSsVidStreamPtr->Timing.HActive/2;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		} else {
			/* If not NTSC/PAL, the HActive,
			 * and VActive remain as it is
			 */
			width  = HdmiTxSsVidStreamPtr->Timing.HActive;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		}
	/* Work around:
	 * Can't set TPG to pass-through mode if the width or height = 0
	 */
	xil_printf("TPG Input enabled %d,%d: \r\n",width,height );
	if (!(width == 0 || height == 0)) {
		/* Stop TPG */
		XV_tpg_DisableAutoRestart(pTpg);

		XV_tpg_Set_height(pTpg, height);
		XV_tpg_Set_width(pTpg,  width);
		XV_tpg_Set_colorFormat(pTpg,
				       HdmiTxSsVidStreamPtr->ColorFormatId);
		XV_tpg_Set_bckgndId(pTpg, 9);
		XV_tpg_Set_ovrlayId(pTpg, 0);
		XV_tpg_Set_enableInput(pTpg, TRUE);
		XV_tpg_Set_passthruStartX(pTpg,0);
		XV_tpg_Set_passthruStartY(pTpg,0);
		XV_tpg_Set_passthruEndX(pTpg,width);
		XV_tpg_Set_passthruEndY(pTpg,height);
		xil_printf("TPG Input enabled ... \r\n");

		/* Start TPG */
		EXDES_DBG_PRINT("%s, Starting Tpg ... width : %d, "
				"height = %d. \r\n", __func__, width, height);
		XV_tpg_EnableAutoRestart(pTpg);
		XV_tpg_Start(pTpg);
	} else {
		/* If the width = 0 and height = 0 don't proceed configuring
		 * other HLS core.
		 */
		return;
	}
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#if !XPAR_XV_HDMITXSS1_0_INCLUDE_YUV420_SUP
	/* Configuring the InRemap which performs
	 *    PPC Conversion from PPC4 --> PPC8 for InRemap
	 *.   PPC Conversion from PPC8 --> PPC4 for OutRemap
	 *    YUV AXI4-Stream Packet on InRemap --> TPG --> OutRemap (8 PPC)
	 * Below configuring following Video HLS IP (TPG Flow)
	 */
	XV_axi4s_remap_Set_height(&InRemap,height);
	XV_axi4s_remap_Set_width(&InRemap,width);

	XV_axi4s_remap_Set_inPixClk(&InRemap,
			XPAR_XV_HDMIRXSS1_0_INPUT_PIXELS_PER_CLOCK);
	XV_axi4s_remap_Set_outPixClk(&InRemap,
			XPAR_XV_AXI4S_REMAP_0_OUT_SAMPLES_PER_CLOCK);

	XV_axi4s_remap_Set_height(&OutRemap,height);
	XV_axi4s_remap_Set_width(&OutRemap,width);

	XV_axi4s_remap_Set_inPixClk(&OutRemap,
			XPAR_XV_AXI4S_REMAP_1_IN_SAMPLES_PER_CLOCK);
	XV_axi4s_remap_Set_outPixClk(&OutRemap,
			XPAR_XV_HDMITXSS1_0_INPUT_PIXELS_PER_CLOCK);

	if (EnableExtSrcInput) {
		XV_axi4s_remap_Set_ColorFormat(&InRemap,
				HdmiTxSsVidStreamPtr->ColorFormatId);
		XV_axi4s_remap_Set_ColorFormat(&OutRemap,
				HdmiTxSsVidStreamPtr->ColorFormatId);

		if (HdmiTxSsVidStreamPtr->ColorFormatId==XVIDC_CSF_YCRCB_420) {
			XV_axi4s_remap_Set_inHDMI420(&OutRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&OutRemap, 1);

			XV_axi4s_remap_Set_inHDMI420(&InRemap, 1);
			XV_axi4s_remap_Set_outHDMI420(&InRemap, 0);
		} else {
			XV_axi4s_remap_Set_inHDMI420(&OutRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&OutRemap, 0);

			XV_axi4s_remap_Set_inHDMI420(&InRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&InRemap, 0);
		}

	} else {
		if (HdmiTxSsVidStreamPtr->ColorFormatId==XVIDC_CSF_YCRCB_420) {
			XV_axi4s_remap_Set_ColorFormat(&InRemap,
                                    XVIDC_CSF_YCRCB_420);
			XV_axi4s_remap_Set_ColorFormat(&OutRemap,
                                    XVIDC_CSF_YCRCB_420);

			XV_axi4s_remap_Set_inHDMI420(&OutRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&OutRemap, 1);
		} else {
			XV_axi4s_remap_Set_ColorFormat(&InRemap,
					HdmiTxSsVidStreamPtr->ColorFormatId);
			XV_axi4s_remap_Set_ColorFormat(&OutRemap,
					HdmiTxSsVidStreamPtr->ColorFormatId);

			XV_axi4s_remap_Set_inHDMI420(&OutRemap, 0);
			XV_axi4s_remap_Set_outHDMI420(&OutRemap, 0);
		}
	}
	XV_axi4s_remap_EnableAutoRestart(&InRemap);
	XV_axi4s_remap_EnableAutoRestart(&OutRemap);

	XV_axi4s_remap_Start(&InRemap);
	XV_axi4s_remap_Start(&OutRemap);
#endif
#endif
}
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function configures the audio for the example design when it is in
* colorbar mode.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_AudioConfig_Colorbar()
{
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;

	AudioInfoFramePtr = XV_HdmiTxSs1_GetAudioInfoframe(&HdmiTxSs);

	/* Reset Audio InfoFrame */
	(void)memset((void *)AudioInfoFramePtr, 0, sizeof(XHdmiC_AudioInfoFrame));

	/* Set audio format and number of audio channels */
	XV_Tx_SetAudioFormatAndChannels(&xhdmi_example_tx_controller,
					XV_HDMITX1_AUDFMT_LPCM, 2);

	/* Set to use the Internal ACR module of the HDMI TX */
	XV_Tx_SetUseInternalACR(&xhdmi_example_tx_controller);
	/* Set the Audio Sampling Frequency to 48kHz */
	XV_Tx_SetAudSamplingFreq(&xhdmi_example_tx_controller,
			XHDMIC_SAMPLING_FREQ_48K);

	/* Refer to CEA-861-D for Audio InfoFrame Channel Allocation
	 * - - - - - - FR FL
	 */
	AudioInfoFramePtr->ChannelAllocation = 0x0;
	/* Refer to Stream Header */
	AudioInfoFramePtr->SampleFrequency = 0x0;
}
#endif


#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function is used to send the info frame when the example design is in
* colorbar mode.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
void Exdes_SendInfoFrame_Colorbar()
{
	u32 Status = XST_SUCCESS;
	u32 RegValue = 0;

	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;

	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)&HdmiTxSs;

	AVIInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(HdmiTxSs1Ptr);
	AudioInfoFramePtr = XV_HdmiTxSs1_GetAudioInfoframe(HdmiTxSs1Ptr);

	/* Generate Aux from the current TX InfoFrame */
	AuxFifo[0] = XV_HdmiC_AVIIF_GeneratePacket(AVIInfoFramePtr);
	EXDES_AUXFIFO_DBG_PRINT("%s,%d. Independent TX ? = %d ; "
			"(AVIIF_generatePacket)AuxFifo[0].Header.Byte[0] = "
			"0x%x\r\n", __func__, __LINE__,
			xhdmi_exdes_ctrlr.ForceIndependent,
			AuxFifo[0].Header.Byte[0]);

	RegValue = XV_HdmiTx1_ReadReg(HdmiTxSs.Config.BaseAddress,
			(XV_HDMITX1_AUX_STA_OFFSET));
	Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr,
						      &(AuxFifo[0]));
	/* If TX Core's hardware Aux FIFO is full, from the while loop,
	 * retry during the next main while iteration.
	 */
	if (Status != (XST_SUCCESS)) {
		EXDES_AUXFIFO_DBG_PRINT(ANSI_COLOR_RED"%s,%d. HW Aux Full "
				"(AuxStatus = 0x%x)"ANSI_COLOR_RESET"\r\n",
				__func__, __LINE__, RegValue);
		AuxHwFullCounter++;
	}

	/* GCP does not need to be sent out because GCP packets on
	 * the TX side is handled by the HDMI TX core fully.
	 */

	AuxFifo[0] = XV_HdmiC_AudioIF_GeneratePacket(AudioInfoFramePtr);
	Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr,
						      &(AuxFifo[0]));
	/* If TX Core's hardware Aux FIFO is full, from the while loop,
	 * retry during the next main while iteration.
	 */
	if (Status != (XST_SUCCESS)) {
		EXDES_AUXFIFO_DBG_PRINT(ANSI_COLOR_RED"%s,%d. HW Aux (AudioIF) "
				"Full (AuxStatus = 0x%x)"ANSI_COLOR_RESET"\r\n",
				__func__, __LINE__, RegValue);
		AuxHwFullCounter++;
	}

	/* SendVSInfoframe(HdmiTxSs1Ptr); */
}
#endif
/*****************************************************************************/
/**
*
* This function checks if there has been a change in resolution on the incoming
* rx stream.
*
* @param  None.
*
* @return None.
*
* @note
*
******************************************************************************/
u32 Exdes_CheckforResChange()
{
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
#ifdef XPAR_XV_TPG_NUM_INSTANCES
		/* Disable the input to Tpg, until the stream
		 * starts with the new resolution video
		 * params again.
		 */
		XV_tpg_Set_enableInput(&Tpg, FALSE);
		/* Alternatively,
		 * Exdes_ConfigureTpgEnableInput(FALSE);
		 */
#endif
#endif
		return TRUE;
}
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
/*****************************************************************************/
/**
*
* This function updates the parameters of the transmitter stream based on
* the input source for the tx stream (i.e. if the input for tx stream is the
* rx stream or previously set or default colorbar).
* rx stream.
*
* @param  InstancePtr is the instance of the example design handle
*         data structure.
* @param  TxInputSrc is the input source for the tx.
*
* @return None.
*
* @note
*
******************************************************************************/
u32 Exdes_UpdateTxParams(XHdmi_Exdes *ExdesInstance,
			 TxInputSourceType TxInputSrc)
{
	u32 Status = XST_SUCCESS;
#if (!defined XPAR_XVTC_NUM_INSTANCES) || (defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES))

   u8 FVaFactor = FVA_FACTOR;
#endif
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
	XVidC_VideoMode	CurrentVmId = HdmiTxSsVidStreamPtr->VmId;

	/* There is no need to crop when TX is FRL */
	if (xhdmi_exdes_ctrlr.IsTxPresent && HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl) {
		xhdmi_exdes_ctrlr.crop = FALSE;
	}

	EXDES_AUXFIFO_DBG_PRINT("%s,%d: Aux Fifo Reset \r\n",
				__func__, __LINE__);
	ResetAuxFifo();
	switch (TxInputSrc) {
	case EXDES_TX_INPUT_NONE_WAITFORNEWSTREAM:
	case EXDES_TX_INPUT_NONE_NOCONNECTIONS:
	case EXDES_TX_INPUT_NONE_RXONLY:
		Status = XST_FAILURE;
		break;

	case EXDES_TX_INPUT_TPG:
		/* If the VmId is not set for the stream or is invalid, then
		 * set  a 1080p stream.
		 * Check if this is a bringup condition where the stream timing
		 * VActive and HActive are 0.
		 * Otherwise keep, as is, the existing / last set stream
		 * information in the transmitter.
		 */
		if ((CurrentVmId > XVIDC_VM_NUM_SUPPORTED) ||
		    (CurrentVmId == XVIDC_VM_NO_INPUT) ||
		    (CurrentVmId == XVIDC_VM_NOT_SUPPORTED) ||
		    (CurrentVmId == XVIDC_VM_CUSTOM &&
		     CurrentVmId < (XVidC_VideoMode)XVIDC_CM_NUM_SUPPORTED) ||
		    (HdmiTxSsVidStreamPtr->ColorDepth < XVIDC_BPC_6) ||
		    (HdmiTxSsVidStreamPtr->Timing.HActive == 0 ||
		     HdmiTxSsVidStreamPtr->Timing.VActive == 0)) {
		xil_printf("Not Supported \n\r");


			/* Check if we have selected the custom
			 * resolution from the colorbar menu. */
			if (CurrentVmId > (XVidC_VideoMode)XVIDC_VM_CUSTOM &&
			    CurrentVmId < (XVidC_VideoMode)XVIDC_CM_NUM_SUPPORTED) {
				/* Do nothing, and continue with the
				 * custom resolution stream information.
				 */
			} else {
				xil_printf(ANSI_COLOR_YELLOW "Video format is"
						" not supported. Reverting to "
						"default video format.\r\n"
						ANSI_COLOR_RESET);
				Exdes_ChangeColorbarOutput(XVIDC_VM_1920x1080_60_P,
						XVIDC_CSF_RGB, XVIDC_BPC_8);
			}
		}

		/* Set the FRL Cke source to internal. */
		XV_Tx_SetFRLCkeSrcToExternal(xhdmi_exdes_ctrlr.hdmi_tx_ctlr, FALSE);
		break;
	default:
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to determine the source for tx stream based on
* the configuration of the system and the presence/absence of rx and tx steams.
*
* @param  None.
*
* @return Returns the Tx Input source.
*
* @note
*
******************************************************************************/
TxInputSourceType Exdes_DetermineTxSrc()
{
	TxInputSourceType TxInputSrc;
			/* Since we are in independent mode, the RX can
			 * be disconnected or be waiting on a new stream.
			 * In either case we should start TX regardless.
			 */
			TxInputSrc = EXDES_TX_INPUT_RX;
			EXDES_DBG_PRINT("TxInputSrc = %d\r\n", TxInputSrc);
	EXDES_DBG_PRINT("TxInputSrc = %d\r\n", TxInputSrc);
	return TxInputSrc;
}
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function clones the EDID of the connected sink device to the HDMI RX
* @return None.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void CloneTxEdid(void)
{

	xil_printf("\r\nEdid Cloning no possible with HDMI RX SS.\r\n");

}

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function resets the TPG.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetFrameBuf(u8 fb_reset)
{
	u32 RegValue;
	/*
	 * fb_reset 0 or 2 resets wr fb
	 * fb_reset 1 or 2 resets rd fb
	 */

	if (fb_reset == 0 || fb_reset == 2) {
		RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 2);

		XGpio_SetDataDirection(&Gpio_Tpg_resetn, 2, 0);
		XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 2, RegValue&0xE);
		usleep(1000);

		XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 2, RegValue|0x1);
		usleep(1000);
	}

	if (fb_reset == 1 || fb_reset == 2) {
		RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 2);

		XGpio_SetDataDirection(&Gpio_Tpg_resetn, 2, 0);
		XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 2, RegValue&0xD);
		usleep(1000);

		XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 2, RegValue|0x2);
		usleep(1000);
	}

}




/*****************************************************************************/
/**
*
* This function resets the TPG.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetTpg(void)
{
	u32 RegValue;

	RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 1);

	XGpio_SetDataDirection(&Gpio_Tpg_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue&0xE);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue|0x1);
	usleep(1000);
}
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function resets the InRemap.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetInRemap(void){
	u32 RegValue;

	RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 1);

	XGpio_SetDataDirection(&Gpio_Tpg_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue&0xD);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue|0x2);
	usleep(1000);
}

/*****************************************************************************/
/**
*
* This function resets the OutRemap.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetOutRemap(void){
	u32 RegValue;

	RegValue = XGpio_DiscreteRead(&Gpio_Tpg_resetn, 1);

	XGpio_SetDataDirection(&Gpio_Tpg_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue&0xB);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, RegValue|0x4);
	usleep(1000);
}
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function resets the AuxFifo.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetAuxFifo(void)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	AuxFifoStartFlag   = (FALSE);
	AuxFifoStartIndex  = 0;
	AuxFifoEndIndex    = 0;
	AuxFifoCount	   = 0;
	AuxFifoOvrFlowCnt  = 0;
#endif
}
#endif

/*****************************************************************************/
/**
*
* This function sends vendor specific infoframe.
*
* @param  HdmiTxSs1Ptr is a pointer to the HDMI 2.1 TX SS.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void SendVSInfoframe(XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{
	u32 Status = XST_SUCCESS;
	XHdmiC_VSIF *VSIFPtr;
	VSIFPtr = XV_HdmiTxSs1_GetVSIF(HdmiTxSs1Ptr);

	XHdmiC_Aux Aux;

	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));
	(void)memset((void *)&Aux, 0, sizeof(XHdmiC_Aux));

	VSIFPtr->Version = 0x1;
	VSIFPtr->IEEE_ID = 0xC03;

	if (XVidC_IsStream3D(&(HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video))) {
		VSIFPtr->Format = XHDMIC_VSIF_VF_3D;
		VSIFPtr->Info_3D.Stream =
				HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Info_3D;
		VSIFPtr->Info_3D.MetaData.IsPresent = FALSE;
	} else if (HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_24_P ||
		   HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_25_P ||
		   HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_30_P ||
		   HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId ==
					   XVIDC_VM_4096x2160_24_P) {
		VSIFPtr->Format = XHDMIC_VSIF_VF_EXTRES;

		/* Set HDMI VIC */
		switch(HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.VmId) {
			case XVIDC_VM_4096x2160_24_P :
				VSIFPtr->HDMI_VIC = 4;
				break;
			case XVIDC_VM_3840x2160_24_P :
				VSIFPtr->HDMI_VIC = 3;
				break;
			case XVIDC_VM_3840x2160_25_P :
				VSIFPtr->HDMI_VIC = 2;
				break;
			case XVIDC_VM_3840x2160_30_P :
				VSIFPtr->HDMI_VIC = 1;
				break;
			default :
				break;
		}
	} else {
		VSIFPtr->Format = XHDMIC_VSIF_VF_NOINFO;
	}

	Aux = XV_HdmiC_VSIF_GeneratePacket(VSIFPtr);

	Status = XV_HdmiTxSs1_SendGenericAuxInfoframe(HdmiTxSs1Ptr, &Aux);
	/* If TX Core's hardware Aux FIFO is full, from the while loop,
	 * retry during the next main while iteration.
	 */
	if (Status != (XST_SUCCESS)) {
		/* Enable this print to profile the overflow of HW AUX Fifo.
		 * However, in case of overlow this print will flood
		 * the UART output port.
		 * xil_printf(ANSI_COLOR_RED "%s,%d. HW Aux Full"
		 *            ANSI_COLOR_RESET "\r\n", __func__, __LINE__);
		 */
	}

}

/*****************************************************************************/
/**
*
* This function resets the Tx Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxInfoFrameReset(void)
{
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;

	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
	AudioInfoFramePtr = XV_HdmiTxSs1_GetAudioInfoframe(&HdmiTxSs);

	/* Reset Avi InfoFrame */
	(void)memset((void *)AviInfoFramePtr,
	             0,
		     sizeof(XHdmiC_AVI_InfoFrame));

	/* Reset Audio InfoFrame */
	(void)memset((void *)AudioInfoFramePtr,
	             0,
		     sizeof(XHdmiC_AudioInfoFrame));

	AviInfoFramePtr->Version = 2;
	AviInfoFramePtr->ColorSpace = XHDMIC_COLORSPACE_RGB;
	AviInfoFramePtr->VIC = 16;
	AviInfoFramePtr->PicAspectRatio = XHDMIC_PIC_ASPECT_RATIO_16_9;
	/* To set the channel count, do,
	 * AudioInfoFramePtr->ChannelCount = XHDMIC_AUDIO_CHANNEL_COUNT_3;
	 */
}
#endif

/*****************************************************************************/
/**
*
* This function setup sets up the On Board IIC MUX to select device
*
* @param  None.
*
* @return The number of bytes sent.
*
* @note   None.
*
******************************************************************************/
int I2cMuxSel(void *IicPtr, XOnBoard_IicDev Dev)
{
	u8 Iic_Mux_Addr;
	u8 Buffer;
	int Status;

#if ! (defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280_ES))
	XIicPs *Iic_Ptr = IicPtr;

	/* Set operation to 7-bit mode */
	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
	/* Clear Repeated Start option */
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);

#if defined (XPS_BOARD_ZCU102)
	if (Dev == ZCU102_MGT_SI570) {
		Iic_Mux_Addr = ZCU102_U34_MUX_I2C_ADDR;
		Buffer = ZCU102_U34_MUX_SEL_SI570;
	}
#elif defined (XPS_BOARD_ZCU106)
	if (Dev == ZCU106_MGT_SI570) {
		Iic_Mux_Addr = ZCU106_U34_MUX_I2C_ADDR;
		Buffer = ZCU106_U34_MUX_SEL_SI570;
	}
#elif defined (XPS_BOARD_VCK190)
	if (Dev == VCK190_MGT_SI570) {
		Iic_Mux_Addr = VCK190_U34_MUX_I2C_ADDR;
		Buffer = VCK190_U34_MUX_SEL_SI570;
	}
#elif defined (XPS_BOARD_VEK280_ES)
	if (Dev == VEK280_ES_MGT_SI570) {
		Iic_Mux_Addr = VEK280_ES_U34_MUX_I2C_ADDR;
		Buffer = VEK280_ES_U34_MUX_SEL_SI570;
	}
#endif

	Status = XIicPs_MasterSendPolled(Iic_Ptr, (u8 *)&Buffer, 1, Iic_Mux_Addr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(Iic_Ptr->IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(Iic_Ptr));
	}

	if (Status == XST_SUCCESS) {
		return 1;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;

#if defined (XPS_BOARD_VCU118)
	if (Dev == VCU118_FMCP) {
		Iic_Mux_Addr = VCU118_U80_MUX_I2C_ADDR;
		Buffer = VCU118_U80_MUX_SEL_FMCP;
	}

#elif defined (XPS_BOARD_VEK280_ES)
	if (Dev ==VEK280_ES_MGT_SI570) {
				Iic_Mux_Addr = VEK280_ES_U34_MUX_I2C_ADDR;
				Buffer = VEK280_ES_U135_MUX_I2C_ADDR;
			}


#else

	if (Dev == VCK190_MGT_SI570) {
			Iic_Mux_Addr = VCK190_U34_MUX_I2C_ADDR;
			Buffer = VCK190_U135_MUX_I2C_ADDR;
		}

#endif

	Status =  XIic_Send(Iic_Ptr->BaseAddress, Iic_Mux_Addr,(u8 *)&Buffer,
							1, XIIC_STOP);
#endif

	return Status;
}

/*****************************************************************************/
/**
*
* This function setup SI5324 clock generator either in free or locked mode.
*
* @param  Index specifies an index for selecting mode frequency.
* @param  Mode specifies either free or locked mode.
*
* @return
*   - Zero if error in programming external clock.
*   - One if programmed external clock.
*
* @note   None.
*
******************************************************************************/
int I2cClk(u32 InFreq, u32 OutFreq)
{
	int Status;

//	Vfmc_I2cMuxSelect(&Vfmc[0]);

	if (OutFreq != 0) {
	/* VFMC TX Clock Source */
		Status = IDT_8T49N24x_I2cClk(&Iic, I2C_CLK_ADDR,
						InFreq, OutFreq);
	}

	/* To profile the in frequency and out frequency, each time a new
	 * clock is set, do,
	 * xil_printf("===\r\n===IN: %d, OUT: %d, Status: %d===\r\n===\r\n",
	 *            InFreq, OutFreq, Status);
	 */
	return Status;
}

/*****************************************************************************/
/**
*
* This function outputs the video timing , Audio, Link Status, HDMI RX state of
* HDMI RX core. In addition, it also prints information about HDMI TX, and
* HDMI GT cores.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Info(void)
{
	xil_printf("\r\n-----\r\n");
	xil_printf("Info\r\n");
	xil_printf("-----\r\n\r\n");

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    xil_printf("------------\r\n");
    xil_printf("HDMI TX SubSystem\r\n");
    xil_printf("------------\r\n");
    xil_printf("HDMI TX timing\r\n");
    xil_printf("------------\r\n");
    XV_HdmiTxSs1_ReportTiming(&HdmiTxSs);
    xil_printf("Audio\r\n");
    xil_printf("---------\r\n");
    XV_HdmiTxSs1_ReportAudio(&HdmiTxSs);
    xil_printf("Static HDR DRM Infoframe\r\n");
    xil_printf("---------\r\n");
    XV_HdmiTxSs1_ReportDRMInfo(&HdmiTxSs);
#endif

	/* GT */
	xil_printf("\r\n------------\r\n");
	xil_printf("HDMI PHY\r\n");
	xil_printf("------------\r\n");
	xil_printf("GT status\r\n");
	xil_printf("---------\r\n");
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	xil_printf("TX reference clock frequency: %0d Hz\r\n",
		   XHdmiphy1_ClkDetGetRefClkFreqHz(&Hdmiphy1,
						   XHDMIPHY1_DIR_TX));
#endif
	XHdmiphy1_HdmiDebugInfo(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CH1);

}

/*****************************************************************************/
/**
*
* This function prints additional details of the system for debug purpose
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void DetailedInfo(void)
{
	u32 Data = 0;
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	u8* EdCounters;
#endif
	xil_printf("\r\n------------\r\n");
	xil_printf("Additional Info\r\n");
	xil_printf("------------\r\n");
	xil_printf("System Status\r\n");
	xil_printf("------------\r\n");

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	FrWrDoneCounter = 0;
	FrRdDoneCounter = 0;
	xil_printf("AuxFifo Overflow Count: %d\r\n", AuxFifoOvrFlowCnt);
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xil_printf("VsyncCounter: %d\r\n",
			VsyncCounter);
	xil_printf ("TX HW AUX FULL Counter: %d\r\n", AuxHwFullCounter);
	VsyncCounter = 0;
	AuxHwFullCounter = 0;
#endif
	xil_printf("------------\r\n");
	xil_printf("Bridge Status\r\n");
	xil_printf("------------\r\n");
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xil_printf("TX BRIDGE OVERFLOW ,%d\r\n",
		   xhdmi_exdes_ctrlr.hdmi_tx_ctlr->ErrorStats.TxBrdgOverflowCnt);
	xil_printf("TX BRIDGE UNDERFLOW ,%d\r\n",
		   xhdmi_exdes_ctrlr.hdmi_tx_ctlr->ErrorStats.TxBrdgUnderflowCnt);
#endif

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	{
	u8 status[2];
	XV_HdmiTx1_DdcReadReg(HdmiTxSs.HdmiTx1Ptr,
			      XV_HDMITX1_DDC_ADDRESS,
			      1,
			      0x40,
			      (u8*)&(status));
	xil_printf("SCDC status : %x\r\n",status[0]);
	}
	/* Note: Reading the SCDC Character Error Detection and Reed-Solomon
	 * Corrections Counter will clear their values at the sink */
	xil_printf("------------\r\n");
	xil_printf("Error Detection Status From Sink\r\n");
	xil_printf("------------\r\n");
	EdCounters = XV_HdmiTxSs1_GetScdcEdRegisters(&HdmiTxSs);
	xil_printf("Channel 0 CED: ");

	/* Valid only when the valid bit (0x80) is set */
	if (EdCounters[1] & 0x80) {
		Data = EdCounters[1];
		Data = ((EdCounters[1] & 0x7F) << 8) | EdCounters[0];
		xil_printf("%d\r\n", Data);
	} else {
		xil_printf("Invalid\r\n", Data);
	}

	xil_printf("Channel 1 CED: ");

	/* Valid only when the valid bit (0x80) is set */
	if (EdCounters[3] & 0x80) {
		Data = ((EdCounters[3] & 0x7F) << 8) | EdCounters[2];
		xil_printf("%d\r\n", Data);
	} else {
		xil_printf("Invalid\r\n", Data);
	}

	xil_printf("Channel 2 CED: ");

	/* Valid only when the valid bit (0x80) is set */
	if (EdCounters[5] & 0x80) {
		Data = ((EdCounters[1] & 0x7F) << 8) | EdCounters[4];
		xil_printf("%d\r\n", Data);
	} else {
		xil_printf("Invalid\r\n", Data);
	}

	if (HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl == TRUE) {
		xil_printf("Channel 3 CED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (EdCounters[8] & 0x80) {
			Data = ((EdCounters[8] & 0x7F) << 8) | EdCounters[7];
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}

		xil_printf("RSED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (EdCounters[10] & 0x80) {
			Data = ((EdCounters[10] & 0x7F) << 8) | EdCounters[9];
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}
	}
#endif
	xil_printf("------------\r\n");
	xil_printf("System: IP Version\r\n");
	xil_printf("------------\r\n");
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xil_printf("HDMI TX SS\r\n");
	xil_printf("------------");
	XV_HdmiTxSs1_ReportCoreInfo(&HdmiTxSs);
	XV_HdmiTxSs1_ReportSubcoreVersion(&HdmiTxSs);
#endif

#if defined(XPAR_XHDMIPHY1_NUM_INSTANCES)
	xil_printf("------------\r\n");
	xil_printf("HDMI PHY\r\n");
	xil_printf("------------\r\n");
	Data = XHdmiphy1_GetVersion(&Hdmiphy1);
	xil_printf("  HDMI Phy version : %02d.%02d (%04x)\r\n",
		   ((Data >> 24) & 0xFF),
		   ((Data >> 16) & 0xFF),
		   (Data & 0xFFFF));
#endif

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xil_printf("\r\n------------\r\n");
	xil_printf("TX Core Status\r\n");
	xil_printf("------------\r\n");
	XV_HdmiTxSs1_DebugInfo(&HdmiTxSs);
#endif

}

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_CableConnectionChange(void *InstancePtr)
{
	XV_Tx *txInst = (XV_Tx *)InstancePtr;
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)txInst->HdmiTxSs;

	xhdmi_exdes_ctrlr.SystemEvent = FALSE;

	if (HdmiTxSs1Ptr->IsStreamConnected == (FALSE)) {
		xil_printf("TX Is Stream COnnected FALSE\n\r");
#if AUX_FIFO_CLEAR
		/* If the RX is present we assume that before the tx
		 * was disconnected the system was in pass-through.
		 */
		if (xhdmi_exdes_ctrlr.IsRxPresent == TRUE &&
		    xhdmi_exdes_ctrlr.ForceIndependent != TRUE) {
			/* Reset the AUX fifo. */
			ResetAuxFifo();
		}
#endif
		xhdmi_exdes_ctrlr.IsTxPresent = FALSE;
	} else {

		if (Exdes_CheckDwnstrmSinkCaps() == XST_SUCCESS) {
			XV_Tx_SetEdidParsingDone(xhdmi_exdes_ctrlr.hdmi_tx_ctlr,
						 TRUE);
			xil_printf("EDID Parsing Pass\r\n");

			if ((EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp > 0) &&
			    (EdidHdmi_t.EdidCtrlParam.IsSCDCPresent ==
			     XVIDC_SUPPORTED)) {
				/* System event has already been set to
				 * true by default.
				 * When the downstream supports FRL, we must
				 * complete FRL training first and then resume
				 * the system operations.
				 */
				XV_Tx_SetFrlEdidInfo(
					xhdmi_exdes_ctrlr.hdmi_tx_ctlr,
					EdidHdmi_t.EdidCtrlParam.IsSCDCPresent,
					EdidHdmi_t.EdidCtrlParam.MaxFrlRateSupp);
				xhdmi_exdes_ctrlr.SystemEvent = FALSE;
			}
		} else {
			XV_Tx_SetEdidParsingDone(xhdmi_exdes_ctrlr.hdmi_tx_ctlr,
						 FALSE);
			xil_printf("EDID Parsing Fails\r\n");
		}

		xhdmi_exdes_ctrlr.IsTxPresent = TRUE;
	}

	EXDES_DBG_PRINT("sysEventDebug:%s:%d:::%d\r\n", __func__,
			__LINE__, xhdmi_exdes_ctrlr.SystemEvent);
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_GetFRLClk(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;

	u32 TotalPixFRLRatio;
	u32 ActivePixFRLRatio;
	u32 HdmiTxRefClkHz;

	if ((xhdmi_exdes_ctrlr.ForceIndependent == FALSE) && IsRx && IsTx) {
		ActivePixFRLRatio =
			XV_HdmiRx1_GetFrlActivePixRatio(HdmiRxSs.HdmiRx1Ptr) / 1000;
		TotalPixFRLRatio =
			XV_HdmiRx1_GetFrlTotalPixRatio(HdmiRxSs.HdmiRx1Ptr) / 1000;
		EXDES_DBG_PRINT("sysEventDebug:%s :: ActivePixFRLRatio = %d | ",
				__func__, ActivePixFRLRatio);
		EXDES_DBG_PRINT("TotalPixFRLRatio = %d | ",TotalPixFRLRatio);

		HdmiTxRefClkHz =
			(((u64)ActivePixFRLRatio * 450000) / TotalPixFRLRatio);
		EXDES_DBG_PRINT("HdmiTxRefClkHz = %d | ",HdmiTxRefClkHz);
		HdmiTxRefClkHz = HdmiTxRefClkHz*1000;
		EXDES_DBG_PRINT("HdmiTxRefClkHz*1000 = %d\r\n",HdmiTxRefClkHz);

		if (XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.TMDSClockRatio) {
			XV_TxInst->VidPhy->HdmiTxRefClkHz = HdmiTxRefClkHz >> 2;
		} else {
			XV_TxInst->VidPhy->HdmiTxRefClkHz = HdmiTxRefClkHz;
		}

		XV_TxInst->VidPhy->HdmiTxRefClkHz = HdmiTxRefClkHz;
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_SetupTxFrlRefClk(void *InstancePtr)
{
	int Status;

	Status = XST_FAILURE;
	xil_printf("XV_Tx_HdmiTrigCb_SetupTxFrlRefClk\r\n");
	Status = Vfmc_Mezz_HdmiTxRefClock_Sel(&Vfmc[0],
			VFMC_MEZZ_TxRefclk_From_Si5344);
	XHdmiphy1_ClkDetFreqReset(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX);
	if (Status == XST_FAILURE) {
		EXDES_DBG_PRINT("I2cClk " ANSI_COLOR_RED
				"Program Failure!\r\n" ANSI_COLOR_RESET);
	}
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_SetupTxTmdsRefClk(void *InstancePtr)
{
	int Status;

	Status = XST_FAILURE;
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;

	Status = Vfmc_Mezz_HdmiTxRefClock_Sel(&Vfmc[0],
			VFMC_MEZZ_TxRefclk_From_IDT);

#if 0
	/* Reset the AUX fifo so it is cleared of any previous TX Vsync upates,
	 * written either from Rx AUX in pass-through or set in Tx Only.
	 *
	 * This will ensure that from Tx clock starting to Tx Vsync the
	 * Aux Fifo remains empty.
	 */
	ResetAuxFifo();
#endif
	XV_Tx_SetFrlIntVidCkeGen(XV_TxInst);

	if ((xhdmi_exdes_ctrlr.ForceIndependent == FALSE) && IsRx && IsTx) {
		EXDES_DBG_PRINT("%s : triggering tx reference clock in "
				"pass-through. \r\n Tx Oversampling "
				"rate = %d.\r\n",__func__,
				Hdmiphy1.HdmiTxSampleRate);
	} else if ((xhdmi_exdes_ctrlr.ForceIndependent == FALSE) && !IsRx && IsTx) {
		EXDES_DBG_PRINT("%s : triggering tx reference "
				"clock in colorbar \r\n", __func__);
		/* Program external clock generator in free running mode */
		Status = I2cClk(0, XV_TxInst->VidPhy->HdmiTxRefClkHz);
	} else if (xhdmi_exdes_ctrlr.ForceIndependent == TRUE) {
		EXDES_DBG_PRINT(ANSI_COLOR_YELLOW "%s : triggering tx reference "
				"clock independently from rx, %d \r\n"
				ANSI_COLOR_RESET, __func__,
				XV_TxInst->VidPhy->HdmiTxRefClkHz);
		EXDES_DBG_PRINT (ANSI_COLOR_YELLOW "%s : triggering tx reference "
				"clock independently from rx, %d \r\n"
				ANSI_COLOR_RESET, __func__,
				XV_TxInst->VidPhy->HdmiTxRefClkHz);
		/* Program external clock generator in free running mode */
		Status = I2cClk(0, XV_TxInst->VidPhy->HdmiTxRefClkHz);
	} else {
		EXDES_DBG_PRINT("%s,%d : No Transmitter present, cannot "
				"program clock !!", __func__, __LINE__);
	}

	if (Status == XST_FAILURE) {
		EXDES_DBG_PRINT("I2cClk " ANSI_COLOR_RED
				"Program Failure!\r\n" ANSI_COLOR_RESET);
	} else {
		usleep(1000000);
	}
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_StreamOff(void *InstancePtr)
{
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
                 defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	u8 IsTx = xhdmi_exdes_ctrlr.IsTxPresent;
	u8 IsRx = xhdmi_exdes_ctrlr.IsRxPresent;

	if (IsRx && IsTx) {
#if AUX_FIFO_CLEAR
		/* Reset the AUX fifo. */
		ResetAuxFifo();
#endif
		XV_HdmiRxSs1_VRST(&HdmiRxSs, TRUE);
	}
#endif
/**
 *	Additionally, the AVI Info frame and Vendor specific info frame
 *	cane also be reset here,
 *	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
 *	XHdmiC_VSIF *VSIFPtr;
 *	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
 *	VSIFPtr = XV_HdmiTxSs1_GetVSIF(&HdmiTxSs);
 *	/\* Reset Avi InfoFrame *\/
 *	(void)memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));
 *	/\* Reset Vendor Specific InfoFrame *\/
 *	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));
 */

}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_SetupAudioVideo(void *InstancePtr)
{
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_VSIF *VSIFPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
#if AUX_FIFO_CLEAR
	/* Get the AVI Info frame and Vendor specific
	 * info-frame and reset them.
	 */
	AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(&HdmiTxSs);
	VSIFPtr = XV_HdmiTxSs1_GetVSIF(&HdmiTxSs);
	/* Reset Avi InfoFrame */
	(void)memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));
	/* Reset Vendor Specific InfoFrame */
	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));
#endif

#ifdef XPAR_XV_TPG_NUM_INSTANCES
	/* Set colorbar pattern */
	Pattern = XTPG_BKGND_COLOR_BARS;
#endif
	xhdmi_exdes_ctrlr.TxBusy = TRUE;

		Exdes_UpdateAviInfoFrame(HdmiTxSsVidStreamPtr);
		Exdes_ConfigureTpgEnableInput(TRUE);
		Exdes_AudioConfig_Colorbar();
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_StreamOn(void *InstancePtr)
{
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
			xil_printf("Tx stream is up in colorbar \r\n");
			xil_printf("--------\r\nColorbar :\r\n");
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
	XVidC_ReportStreamInfo(HdmiTxSsVidStreamPtr);
	if (HdmiTxSs.HdmiTx1Ptr->Stream.IsFrl == TRUE) {
		xil_printf("\tTX FRL Rate:              %d lanes @ %d Gbps\r\n",
				HdmiTxSs.HdmiTx1Ptr->Stream.Frl.Lanes,
				HdmiTxSs.HdmiTx1Ptr->Stream.Frl.LineRate);
	} else {
		xil_printf("\tTX     Mode:              TMDS\r\n");
	}
	xil_printf("--------\r\n");

	xhdmi_exdes_ctrlr.TxBusy = FALSE;
	VsyncCounter = 0;
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_VidSyncRecv(void *InstancePtr)
{
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;

	/* Check whether the sink is DVI/HDMI Supported
	 * If the sink is DVI, don't send Info-frame
	 */
	if (EdidHdmi_t.EdidCtrlParam.IsHdmi == XVIDC_ISHDMI &&
	    (XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.IsHdmi == TRUE)) {
			Exdes_SendInfoFrame_Colorbar();
	} else {
		/* To ensure that the downstream is DVI, or never transitions
		 * to HDMI use the following print to check.
		 * Additonally, re-check the videomode of the stream here.
		 * EXDES_DBG_PRINT("%s,%d. Downstream is not HDMI !!\r\n",
		 *		__func__, __LINE__);
		 */
	}

	VsyncCounter++;
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_EnableCableDriver(void *InstancePtr)
{
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;
	u64 TxLineRate;
	u8 TxDiffSwingVal;

	TxLineRate = XV_Tx_GetLineRate(XV_TxInst);

	u8 Lanes = XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.Lanes;

	if (SinkReadyCheck(XV_TxInst->HdmiTxSs, &EdidHdmi_t)) {
		EXDES_DBG_PRINT("Setting Cable Driver, TxLineRate = %d%d\r\n",
				(u32)(TxLineRate >> 32), (u32)TxLineRate);

		/* if hdmi mode is TMDS then we need to configure the
		 * tx fmc driver for each resolution, othersie for FRL
		 * tx fmc drivers will be configured in FRL config.
		 */
		if (XV_HdmiTxSs1_GetTransportMode(XV_TxInst->HdmiTxSs) ==
				FALSE) {
			Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(&Vfmc[0],
							     FALSE,
							     TxLineRate, Lanes);

#if defined (XPS_BOARD_ZCU106) || \
	defined (XPS_BOARD_VCU118) || \
	defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280_ES)
			/* Adjust GT TX Diff Swing based on Line rate */
			if (Vfmc[0].TxMezzType >= VFMC_MEZZ_HDMI_ONSEMI_R0 &&
				Vfmc[0].TxMezzType <  VFMC_MEZZ_INVALID) {
				/*Convert Line Rate to Mbps */
				TxLineRate = (u32)((u64) TxLineRate / 1000000);

				/* HDMI 2.0 */
				if ((TxLineRate >= 3400) &&
				    (TxLineRate < 6000)) {
					if (Vfmc[0].TxMezzType ==
						VFMC_MEZZ_HDMI_ONSEMI_R0) {
						/* Set Tx Diff Swing to
						 * 963 mV */
						TxDiffSwingVal = 0xE;
					}
					else if (Vfmc[0].TxMezzType >=
						VFMC_MEZZ_HDMI_ONSEMI_R1) {
						/* Set Tx Diff Swing to
						 * 1000 mV */
						TxDiffSwingVal = 0xF;
					}
				}
				/* HDMI 1.4 1.65-3.4 Gbps */
				else if ((TxLineRate >= 1650) &&
				         (TxLineRate < 3400)) {
					/* Set Tx Diff Swing to 1000 mV */
					TxDiffSwingVal = 0xF;
				}
				/* HDMI 1.4 0.25-1.65 Gbps */
				else {
					/* Set Tx Diff Swing to 822 mV */
					TxDiffSwingVal = 0xB;
				}

				for (int ChId=1; ChId <= 4; ChId++) {
					XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1,
								0,
								ChId,
								TxDiffSwingVal);
				}
			}
#endif
		}
	}
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_ReadyToStartTransmit(void *InstancePtr)
{
	xhdmi_exdes_ctrlr.SystemEvent = TRUE;
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_FrlFfeConfigDevice(void *InstancePtr)
{
	/* Nothing to be done here.
	 * This function is available as a place holder
	 * for the users to configure the on board or
	 * external mezzanine cards for HDMI 2.1
	 * during the FRL ffe training.
	 */
}

/*****************************************************************************/
/**
*
* This function is called from the TX State machine layer.
*
* @param  InstancePtr is the callback.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_Tx_HdmiTrigCb_FrlConfigDeviceSetup(void *InstancePtr)
{
	u8 Data = 0;

	EXDES_DBG_PRINT("sysEventDebug:%s,%d: Setting device configurations "
			"at Frl Config.\r\n", __func__, __LINE__);
	XV_Tx *XV_TxInst = (XV_Tx *)InstancePtr;

	u64 LineRate =
		((u64)(XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.LineRate)) *
			((u64)(1e9));

	u8 Lanes = XV_TxInst->HdmiTxSs->HdmiTx1Ptr->Stream.Frl.Lanes;

	Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(&Vfmc[0],
					     TRUE,
					     LineRate, Lanes);

	/* Adjust GT TX Diff Swing based on Mode */
	for (int ChId=1; ChId <= 4; ChId++) {
		if (Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_ONSEMI_R0) {
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU106) || \
	defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280_ES)
			Data = 0xD;
#elif defined (XPS_BOARD_VCU118)
			Data = ChId==4 ? 0x1C : 0x1A;
#endif
		} else if (Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_ONSEMI_R1) {
#if defined (XPS_BOARD_ZCU106)
			/* Set TxDiffSwing to 1000 mV for all channels */
			Data = 0xF;
#elif defined (XPS_BOARD_VCU118)
			Data = ChId==4 ? 0x1C : 0x1A;
#elif defined (XPS_BOARD_ZCU102)
			Data = 0xD;
#elif defined (XPS_BOARD_VCK190)
			Data = 0xD;
#elif defined (XPS_BOARD_VEK280_ES)
			Data = 0xD;
#endif
		} else if (Vfmc[0].TxMezzType >= VFMC_MEZZ_HDMI_ONSEMI_R2) {
#if defined (XPS_BOARD_ZCU106)
				Data = 0xD;
#elif defined (XPS_BOARD_VCU118)
			Data = 0xD;
#elif defined (XPS_BOARD_ZCU102)
			Data = 0xD;
#elif defined (XPS_BOARD_VCK190)
			Data = 0xD;
#elif defined (XPS_BOARD_VEK280_ES)
			Data = 0xD;
#endif
		}
#if defined (XPS_BOARD_ZCU106) || \
	defined (XPS_BOARD_VCU118) || \
	defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280_ES)
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId, Data);
#endif
	}
}
#endif

/*****************************************************************************/
/**
*
* This function is called whenever an error condition in VPHY occurs.
* This will fill the FIFO of VPHY error events which will be processed outside
* the ISR.
*
* @param  CallbackRef is the VPHY instance pointer
* @param  ErrIrqType is the VPHY error type
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Hdmiphy1ErrorCallback(void *CallbackRef)
{
	Hdmiphy1ErrorFlag = TRUE;
}

/*****************************************************************************/
/**
*
* This function is called in the application to process the pending
* VPHY errors
*
* @param  None.
*
* @return None.
*
* @note   This function can be expanded to perform necessary actions depending
*         on the error type. For example, XHDMIPHY1_ERR_PLL_LAYOUT can be
*         used to automatically switch in and out of bonded mode for
*         GTXE2 devices
*
******************************************************************************/
void Hdmiphy1ProcessError(void)
{
	if (Hdmiphy1ErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED"VPHY Error: See log for details"
				ANSI_COLOR_RESET "\r\n");
	}
	/* Clear Flag */
	Hdmiphy1ErrorFlag = FALSE;

}

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called whenever an error condition in HDMI TX occurs.
* This will fill the FIFO of HDMI TX error events which will be processed
* outside the ISR.
*
* @param  CallbackRef is the HDMI TXSS instance pointer
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void HdmiTxErrorCallback(void *CallbackRef)
{
	HdmiTxErrorFlag = TRUE;
}

/*****************************************************************************/
/**
*
* This function is called in the application to process the pending
* HDMI TX errors
*
* @param  None.
*
* @return None.
*
* @note   This function can be expanded to perform necessary actions depending
*         on the error type.
*
******************************************************************************/
void HdmiTxProcessError(void)
{
	if (HdmiTxErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED"HDMI TX Error: See log for details"
				ANSI_COLOR_RESET "\r\n");
	}

	/* Clear Flag */
	HdmiTxErrorFlag = FALSE;
}

/*****************************************************************************/
/**
*
* This function updates the ColorFormat for the current video stream
*
* @param Hdmiphy1Ptr is a pointer to the VPHY core instance.
* @param HdmiTxSs1Ptr is a pointer to the XV_HdmiTxSs1 instance.
* @param Requested ColorFormat
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateColorFormat(XHdmiphy1         *Hdmiphy1Ptr,
		       XV_HdmiTxSs1      *HdmiTxSs1Ptr,
		       XVidC_ColorFormat ColorFormat)
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Check passthrough */
	if ((xhdmi_exdes_ctrlr.IsRxPresent && xhdmi_exdes_ctrlr.IsTxPresent) &&
	    xhdmi_exdes_ctrlr.ForceIndependent == FALSE) {
		xil_printf("Error: Color space conversion in "
			   "pass-through mode is not supported!\r\n");
		return;
	}

	/* Inform user that pixel repetition is not supported */
	if (((HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x480_60_I) ||
	     (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x576_50_I)) &&
	    (ColorFormat == XVIDC_CSF_YCRCB_422)) {

		xil_printf("The video bridge is unable to support "
			   "pixel repetition in YUV 422 Color space\r\n");

	}

	Exdes_ChangeColorbarOutput(HdmiTxSsVidStreamPtr->VmId,
				   ColorFormat,
				   HdmiTxSsVidStreamPtr->ColorDepth);

	/* Trigger transmitter (re)start */
	xhdmi_exdes_ctrlr.SystemEvent = TRUE;
}

/*****************************************************************************/
/**
*
* This function updates the ColorDepth for the current video stream
*
* @param Hdmiphy1Ptr is a pointer to the VPHY core instance.
* @param HdmiTxSs1Ptr is a pointer to the XV_HdmiTxSs1 instance.
* @param Requested ColorFormat
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateColorDepth(XHdmiphy1         *Hdmiphy1Ptr,
		      XV_HdmiTxSs1      *HdmiTxSs1Ptr,
		      XVidC_ColorDepth  ColorDepth)
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Check if the exdes is in Passthrough mode.
	 * If not, then check if Color Space is YUV422.
	 * If still not, then check if Rate is more than 5.94 Gbps */
	if ((xhdmi_exdes_ctrlr.IsRxPresent && xhdmi_exdes_ctrlr.IsTxPresent) &&
	    (xhdmi_exdes_ctrlr.ForceIndependent == FALSE)) {
		xil_printf("Color depth conversion in pass-through "
			   "mode not supported!\r\n");
		return;
	} else if ((HdmiTxSsVidStreamPtr->ColorFormatId ==
	            XVIDC_CSF_YCRCB_422) &&
	           (ColorDepth != XVIDC_BPC_12)) {
		xil_printf("YUV422 only supports 36-bits color depth!\r\n");
		return;
	} else if ((HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.IsFrl != TRUE) &&
	           (((HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_3840x2160_60_P) ||
	             (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_3840x2160_50_P)) &&
	            ((HdmiTxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_RGB) ||
	             (HdmiTxSsVidStreamPtr->ColorFormatId ==
	              XVIDC_CSF_YCRCB_444)) &&
	            (ColorDepth != XVIDC_BPC_8))) {
		xil_printf("2160p60 & 2160p50 on RGB & YUV444 only"
			   " supports 24-bits colordepth!\r\n");
		return;
	}

	Exdes_ChangeColorbarOutput(HdmiTxSsVidStreamPtr->VmId,
				   HdmiTxSsVidStreamPtr->ColorFormatId,
				   ColorDepth);

	/* Trigger transmitter (re)start */
	xhdmi_exdes_ctrlr.SystemEvent = TRUE;

}

/*****************************************************************************/
/**
*
* This function updates the FrameRate for the current video stream
*
* @param Hdmiphy1Ptr is a pointer to the VPHY core instance.
* @param HdmiTxSs1Ptr is a pointer to the XV_HdmiTxSs1 instance.
* @param Requested FrameRate
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateFrameRate(XHdmiphy1       *Hdmiphy1Ptr,
		     XV_HdmiTxSs1    *HdmiTxSs1Ptr,
		     XVidC_FrameRate FrameRate)
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs1_GetVideoStream(HdmiTxSs1Ptr);

	/* Check pass through */
	if ((xhdmi_exdes_ctrlr.IsRxPresent && xhdmi_exdes_ctrlr.IsTxPresent) &&
	    xhdmi_exdes_ctrlr.ForceIndependent == FALSE) {
		xil_printf("Frame rate conversion in pass-through"
			   " mode not supported!\r\n");
		return;
	}

	/* Check if requested video mode is available */
	XVidC_VideoMode VmId =
		XVidC_GetVideoModeIdExtensive(&HdmiTxSsVidStreamPtr->Timing,
					FrameRate,
					HdmiTxSsVidStreamPtr->IsInterlaced,
					(FALSE));

	HdmiTxSsVidStreamPtr->VmId = VmId;

	Exdes_ChangeColorbarOutput(HdmiTxSsVidStreamPtr->VmId,
				HdmiTxSsVidStreamPtr->ColorFormatId,
				HdmiTxSsVidStreamPtr->ColorDepth);

	/* Trigger transmitter (re)start */
	xhdmi_exdes_ctrlr.SystemEvent = TRUE;
}
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

/*****************************************************************************/
/**
*
* This is the callback for assertion error.
*
* @param File is string name of the file where the assertion failure occured.
* @param LIne is the line wheer the assertion failure occured.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
/*void Xil_AssertCallbackRoutine(u8 *File, s32 Line)
{
	xil_printf("Assertion in File %s, on line %0d\r\n", File, Line);
}
*/

/*****************************************************************************/
/**
*
* This function updates the FrameRate for the current video stream
*
* @param  ps_iic0_deviceid is the device id of the ps iic device.
* @param  ps_iic1_deviceid is the device id of the ps iic device.
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_SetupClkSrc(u32 ps_iic0_deviceid, u32 ps_iic1_deviceid)
{
	u32 Status = XST_SUCCESS;
#if !((defined XPS_BOARD_ZCU102) || (defined XPS_BOARD_ZCU106) || \
	    defined (XPS_BOARD_VCK190) || defined (XPS_BOARD_VEK280_ES))

	XIicPs_Config *XIic0Ps_ConfigPtr;
	XIicPs_Config *XIic1Ps_ConfigPtr;

	/* Initialize IIC */
	/* Initialize PS IIC0 */
	XIic0Ps_ConfigPtr = XIicPs_LookupConfig(ps_iic0_deviceid);
	if (NULL == XIic0Ps_ConfigPtr) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Ps_Iic0, XIic0Ps_ConfigPtr,
				XIic0Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_Reset(&Ps_Iic0);
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Ps_Iic0, PS_IIC_CLK);

	/* Initialize PS IIC1 */
	XIic1Ps_ConfigPtr = XIicPs_LookupConfig(ps_iic1_deviceid);
	if (NULL == XIic1Ps_ConfigPtr) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic1, XIic1Ps_ConfigPtr,
				XIic1Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_Reset(&Iic1);
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic1, PS_IIC_CLK);

#if (defined XPS_BOARD_ZCU102)
	I2cMuxSel(&Iic, ZCU102_MGT_SI570);
#elif (defined XPS_BOARD_ZCU106)
	I2cMuxSel(&Iic, ZCU106_MGT_SI570);
#elif (defined XPS_BOARD_VCK190)
	I2cMuxSel(&Ps_Iic0, VCK190_MGT_SI570);
#elif (defined XPS_BOARD_VEK280_ES)
	I2cMuxSel(&Ps_Iic0, VEK280_ES_MGT_SI570);
#endif

#else /* VCU118 */
	Status = XIic_Initialize(&Iic, XPAR_IIC_0_DEVICE_ID);
	Status |= XIic_Start(&Iic);

	I2cMuxSel(&Iic, VCK190_MGT_SI570);
#endif



#if (defined XPS_BOARD_VCK190)
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	/* Set DRU MGT REFCLK Frequency */
//	Si570_SetFreq(&Ps_Iic0, 0x5F, 400.00);
//	/* Delay 50ms to allow SI chip to lock */
//	usleep (50000);
#endif
#elif (!defined XPS_BOARD_VCU118)
#endif

	return Status;
}

/*****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* HDMI cores. The function is application-specific since the actual system
* may or may not have an interrupt controller. The HDMI cores could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if interrupt setup was successful.
*   - A specific error code defined in "xstatus.h" if an error
*   occurs.
*
* @note   This function assumes a Microblaze system and no operating
*   system is used.
*
******************************************************************************/
int SetupInterruptSystem(void)
{
	int Status;
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic *IntcInstPtr = &Intc;
#else
	XIntc *IntcInstPtr = &Intc;
#endif

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if (IntcCfgPtr == NULL) {
		xil_printf("ERR:: Interrupt Controller not found");
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XScuGic_CfgInitialize(IntcInstPtr,
				IntcCfgPtr,
				IntcCfgPtr->CpuBaseAddress);
#else
	Status = XIntc_Initialize(IntcInstPtr, XPAR_INTC_0_DEVICE_ID);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\r\n");
		return XST_FAILURE;
	}


	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
#if defined (__MICROBLAZE__)
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
        /* Status = XIntc_Start(IntcInstPtr, XIN_SIMULATION_MODE); */
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				(XScuGic *)IntcInstPtr);
#else
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				(XIntc *)IntcInstPtr);
#endif

	return (XST_SUCCESS);
}

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function initialized the In Remapper and Out Remapper.
*
* @param  inremap_deviceid is the In Remapper Device ID
* @param  outremap_deviceid is the Out Remapper Device ID
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_RemapInitialize(u32 inremap_deviceid, u32 outremap_deviceid)
{
	u32 Status = XST_SUCCESS;
	XV_axi4s_remap_Config *InRemapPtr, *OutRemapPtr;
	InRemapPtr = XV_axi4s_remap_LookupConfig(inremap_deviceid);
	if (InRemapPtr == NULL) {
		InRemap.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XV_axi4s_remap_CfgInitialize(&InRemap,
			InRemapPtr, InRemapPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: InRemap Initialization failed %d\r\n",
			Status);
		return(XST_FAILURE);
	}

	OutRemapPtr = XV_axi4s_remap_LookupConfig(outremap_deviceid);
	if (OutRemapPtr == NULL) {
		OutRemap.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XV_axi4s_remap_CfgInitialize(&OutRemap,
			OutRemapPtr, OutRemapPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: OutRemap Initialization failed %d\r\n",
			Status);
		return(XST_FAILURE);
	}
	return Status;
}
#endif

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function initialized the TPG.
*
* @param  Gpio_tpg_resetn_deviceid is the GPIO devive id used to reset the TPG.
* @param  tpg_deviceid is the TPG devive id.
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_TpgInitialize(u32 Gpio_tpg_resetn_deviceid, u32 tpg_deviceid)
{
	u32 Status = XST_SUCCESS;
	XGpio_Config *Gpio_Tpg_resetn_ConfigPtr;
	XV_tpg_Config *Tpg_ConfigPtr;

	/* Initialize GPIO for Tpg Reset */
	Gpio_Tpg_resetn_ConfigPtr = XGpio_LookupConfig(Gpio_tpg_resetn_deviceid);
	if (Gpio_Tpg_resetn_ConfigPtr == NULL) {
		Gpio_Tpg_resetn.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XGpio_CfgInitialize(&Gpio_Tpg_resetn,
				Gpio_Tpg_resetn_ConfigPtr,
				Gpio_Tpg_resetn_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for TPG Reset \r\n "
				"Initialization failed %d\r\n", Status);
		return (XST_FAILURE);
	}

	Tpg_ConfigPtr = XV_tpg_LookupConfig(tpg_deviceid);
	if (Tpg_ConfigPtr == NULL) {
		Tpg.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XV_tpg_CfgInitialize(&Tpg,
				Tpg_ConfigPtr, Tpg_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: TPG Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}
	return Status;
}
#endif
#endif

#ifdef XPAR_XAXIS_SWITCH_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function initialized the AXI Stream Switch.
*
* @param  Gpio_tpg_resetn_deviceid is the GPIO devive id used to reset the TPG.
* @param  tpg_deviceid is the TPG devive id.
*
* @return XST_SUCCESS if the clock source is successfuly set.
*         XST_FAILURE otherwise.
*
* @note   None.
*
******************************************************************************/
u32 Exdes_AxisSwitchInitialize(XAxis_Switch *AxisSwitchPtr,u32 deviceid)
{
	u32 Status = XST_SUCCESS;
	XAxis_Switch_Config *AxisSwitchConfigPtr;

	/* Initialize GPIO for Tpg Reset */
	AxisSwitchConfigPtr = XAxisScr_LookupConfig(deviceid);
	if (AxisSwitchConfigPtr == NULL) {
		AxisSwitchPtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XAxisScr_CfgInitialize(AxisSwitchPtr, AxisSwitchConfigPtr,
					AxisSwitchConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: AXIS Stream Switch \r\n "
				"Initialization failed for Device ID:%d,Status:\r\n",deviceid,Status);
		return (XST_FAILURE);
	}
	return Status;
}
#endif


/*****************************************************************************/
/**
*
* Main function to call example with HDMI TX, HDMI RX and HDMI GT drivers.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if HDMI example was successfully.
*   - XST_FAILURE if HDMI example failed.
*
* @note   None.
*
******************************************************************************/
int config_hdmi()
{
	u32 Status = XST_FAILURE;
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	TxInputSourceType TxInputSrc;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	/**
	 * Reset the Fifo buffer which will be used for holding
	 * incoming auxillary packets.
	 */
	ResetAuxFifo();
#endif
#if (!defined XPAR_XVTC_NUM_INSTANCES) || (defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFRD_NUM_INSTANCES))
#if defined (VTEM2FSYNC)
	XV_HdmiC_VrrInfoFrame  *HdmiRxVrrInfoFrameVRRPtr;
	XVidC_VideoStream      *HdmiRxSsVidStreamVRRPtr;
#endif
#endif

	xil_printf("\r\n\r\n");
	xil_printf("------------------------------------------\r\n");
#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
	xil_printf("--- HDMI 2.1 SS + HdmiPhy VRR Example v%d.%d ---\r\n",
			APP_MAJ_VERSION, APP_MIN_VERSION);
#else
	xil_printf("--- HDMI 2.1 SS + HdmiPhy Example v%d.%d---\r\n",
			APP_MAJ_VERSION, APP_MIN_VERSION);
#endif
	xil_printf("---    (c) 2019 by Xilinx, Inc.        ---\r\n");
	xil_printf("------------------------------------------\r\n");
	xil_printf("Build %s - %s\r\n", __DATE__, __TIME__);
	xil_printf("------------------------------------------\r\n");
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	xhdmi_exdes_ctrlr.TxBusy            = (TRUE);
	HdmiTxErrorFlag = FALSE;
#endif
	Hdmiphy1ErrorFlag = FALSE;
	Hdmiphy1PllLayoutErrorFlag = FALSE;



	/* Initialize platform */
	//xil_printf("Initializing platform. \r\n");
	//init_platform();

	/**
	 * Setup IIC devices and clock sources.
	 */
	xil_printf("Initializing IIC and clock sources. \r\n");
#if !(defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_ZCU104)
	/* Initialize the PS_I2C and Initialize the clocks */
	Status = Exdes_SetupClkSrc(XPAR_XIICPS_0_DEVICE_ID,
			XPAR_XIICPS_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("PS IIC initialization failed. \r\n");
	}
#elif !(__microblaze__)
	/* Initialize the IIC and Initialize the clocks */
	Status = Exdes_SetupClkSrc((u32)NULL, (u32)NULL);
	if (Status != XST_SUCCESS) {
		xil_printf("PS IIC initialization failed. \r\n");
	}
#endif

	/**
	 * Initialize the HDMI Video Transmitter "state machine"
	 * functionality and the downstream HDCP device.
	 */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	xil_printf("Initializing HDMI Video Transmitter. \r\n");

	/* Initialize the controller for the Video TX for HDMI */
	Status = XV_Tx_InitController(&xhdmi_example_tx_controller,
			XPAR_XV_HDMITXSS1_0_DEVICE_ID,
			XPAR_HDMIPHY1_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("HDMI Video Transmitter system setup failed !!\r\n");
	} else {
		xil_printf("HDMI Video Transmitter system setup successful.\r\n");
	}
#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

	/**
	 * Set the Video Phy Error callbacks.
	 */
	XHdmiphy1_SetErrorCallback(&Hdmiphy1,
				(void *)Hdmiphy1ErrorCallback,
				(void *)&Hdmiphy1);

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	Status |= XV_HdmiTxSs1_SetCallback(&HdmiTxSs,
			XV_HDMITXSS1_HANDLER_ERROR,
			(void *)HdmiTxErrorCallback,
			(void *)&HdmiTxSs);
#endif

	/* Initialize Video FMC and GT TX output*/
	Status = Vfmc_HdmiInit(&Vfmc[0], XPAR_HDMI_HIER_0_AXI_GPIO_0_DEVICE_ID,
						&Iic, VFMC_HPC0);
	if (Status == XST_FAILURE) {
		xil_printf("VFMC Initialization Error! Exiting Program...\r\n");
		return 0;
	} else {
		for (int ChId=1; ChId <= 4; ChId++) {
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#if defined (XPS_BOARD_ZCU102)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xC : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, B */
#elif defined (XPS_BOARD_ZCU106)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xC : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, B */
#elif defined (XPS_BOARD_VCU118)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xC : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x3);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x3);/*1, B */
#elif defined (XPS_BOARD_VEK280_ES)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xC : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, B */
#elif defined (XPS_BOARD_VCK190)
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0xC : 0xD);/*0xc 0xb */
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, A */
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x5);/*1, B */
#else
/* Place holder for future board support, Below Value just a random value */
			XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0, ChId, 0xB);
			XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x9);
			XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0, ChId,
					(Vfmc[0].TxMezzType == VFMC_MEZZ_HDMI_PASSIVE) ? 0x1 : 0x9);
#endif
#endif
		}
	}

	/* Initialize IDT to make sure LoL is LOW */
	I2cClk(0, 400000000);
	/* Delay 50ms to allow clockgen to lock */
	usleep (50000);

	/**
	 *  Initialize the controller for the example design mode.
	 */
	xil_printf("Initializing Example design controller. \r\n");
	Exdes_InitController(&xhdmi_exdes_ctrlr);

	/**
	 * Initialize the TPG and associated GPIO to reset the TPG.
	 */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
	xil_printf("Initializing TPG. \r\n");

	Status = Exdes_TpgInitialize(XPAR_HDMI_HIER_0_V_TPG_SS_0_AXI_GPIO_DEVICE_ID,
			XPAR_HDMI_HIER_0_V_TPG_SS_0_V_TPG_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Error in initializing TPG and "
				"GPIO to reset the TPG !! \r\n");
	} else {
		xil_printf("TPG and connected GPIO "
				"successfully initialized. \r\n");
	}
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	xil_printf("Initializing In-Remapper and Out-Remapper. \r\n");

	Status = Exdes_RemapInitialize(XPAR_XV_AXI4S_REMAP_0_DEVICE_ID,
			XPAR_XV_AXI4S_REMAP_1_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Error in initializing InRemap and "
				"OutRemap for 8PPC & 8kp60@YUV420!! \r\n");
	} else {
		xil_printf("InRemap and OutRemap "
				"successfully initialized. \r\n");
	}
#endif
#endif


	xil_printf("---------------------------------\r\n");

	/* Enable exceptions. */
//	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
//	Xil_ExceptionEnable();

	/* Initialize the system timer */
	Exdes_SysTmrInitialize(&xhdmi_exdes_ctrlr,
					XPAR_TMRCTR_1_DEVICE_ID,
#if defined(__arm__) || (__aarch64__)
					XPAR_FABRIC_TMRCTR_1_VEC_ID);
#else
					XPAR_INTC_0_TMRCTR_1_VEC_ID);
#endif


#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Reset the TX Info Frame. */
	TxInfoFrameReset();
#endif


#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Enable Scrambling Override
	 * Note: Setting the override to TRUE will allow scrambling to be
	 *       disabled for video where TMDS Clock > 340 MHz which breaks the
	 *       HDMI Specification
	 * E.g.:
	 *   XV_HdmiTxSs1_SetVideoStreamScramblingOverrideFlag(&HdmiTxSs, TRUE);
	 */

//	u8 SinkReady = FALSE;
#endif
	xhdmi_exdes_ctrlr.ForceIndependent = TRUE;
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	xhdmi_exdes_ctrlr.IsRxPresent = 0;
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	/* Set the data and clock selection on channel 4
	 * on the TX FMC Mezzanine card. */
	Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_TX_CH4_As_DataAndClock);
	XHdmiphy1_Hdmi20Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX);
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	Exdes_ChangeColorbarOutput(XVIDC_VM_1920x1080_60_P,
			XVIDC_CSF_RGB, XVIDC_BPC_8);


	Exdes_ConfigureTpgEnableInput(TRUE);


//	xil_printf("Max FRL Rate is %d\n\r", HdmiTxSs.Config.MaxFrlRate);
	/* Declare the maximum FRL_Rate supported by TX */
	XV_HdmiTxSs1_SetFrlMaxFrlRate(&HdmiTxSs, HdmiTxSs.Config.MaxFrlRate);

	/* Declare the FFE_Levels supported by TX */
	XV_HdmiTxSs1_SetFfeLevels(&HdmiTxSs, 0);
	XV_HdmiTxSs1_Start(&HdmiTxSs);
#endif
	/* Start the system timer to generate a repetitive pulse to
	 * handle exceptions on counters for HDMI TX.
	 */
	/* Setting the periodic interval to 100ms. */
	Exdes_StartSysTmr(&xhdmi_exdes_ctrlr, 100);

	xhdmi_exdes_ctrlr.ForceIndependent = (TRUE);
}

void start_hdmi()
{

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	u32 Status = XST_FAILURE;
	TxInputSourceType TxInputSrc;

#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
		SinkReady = SinkReadyCheck(&HdmiTxSs, &EdidHdmi_t);
#endif
		/* Check if the example design has been triggered from
		 * the Hdmi Rx/Tx state machine layer.l
		 */
		if (xhdmi_exdes_ctrlr.SystemEvent == TRUE) {
			/* Clear the 'System Event' trigger. */
			xhdmi_exdes_ctrlr.SystemEvent = FALSE;
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
			/* Determine the source for the transmitter. */
			TxInputSrc = Exdes_DetermineTxSrc();

			/* Update the transmitter stream parameters. */
			if (Exdes_UpdateTxParams(&xhdmi_exdes_ctrlr,
			                         TxInputSrc) ==
			    XST_SUCCESS) {
				xhdmi_exdes_ctrlr.TxStartTransmit = TRUE;
			}
#endif
		}
		xil_printf ("Tx Transmit is %d \n\r", xhdmi_exdes_ctrlr.TxStartTransmit);



#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
		/* Poll the Example design controller for the need
		 * to [re]start TX. */
		if (xhdmi_exdes_ctrlr.TxStartTransmit == TRUE && SinkReady) {
			/* xil_printf("xhdmi_exdes_ctrlr.TxStartTransmit\r\n"); */

			/* Ideally, here, we should get the video stream
			 * from Rx, but we have already copied the Rx stream
			 * to Tx, so getting the stream from TX should be an
			 * additional check for the stability
			 * of the software flow. */

			HdmiTxSsVidStreamPtr =
				XV_HdmiTxSs1_GetVideoStream(
					xhdmi_exdes_ctrlr.hdmi_tx_ctlr->HdmiTxSs);


			/* /\* Setup the AVI Info Frame. *\/ */
			/* Exdes_UpdateAviInfoFrame(HdmiTxSsVidStreamPtr); */

			/* Setup the Tx Stream and trigger it */
			Status = XV_Tx_VideoSetupAndStart(
						xhdmi_exdes_ctrlr.hdmi_tx_ctlr,
						HdmiTxSsVidStreamPtr);

			if (Status != XST_SUCCESS) {
				xil_printf("XV_Tx_VideoSetupAndStart Failure\r\n");
			} else {
				xil_printf("XV_Tx_VideoSetupAndStart Success\r\n");
			}




			/* Disable the TxStartTransmit flag */
			xhdmi_exdes_ctrlr.TxStartTransmit = FALSE;
		}
#endif

		/* HDMI Menu */

		/* VPHY error */
		Hdmiphy1ProcessError();

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
		/* HDMI TX error */
		HdmiTxProcessError();
#endif

}
