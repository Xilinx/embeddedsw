/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx1.c
*
* This is the main file for Xilinx HDMI RX core. Please see xv_hdmirx1.h for
* more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     02/05/19 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmirx1.h"
#include "string.h"

#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"
/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

static void StubCallback(void *CallbackRef);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the HDMI RX core. This function must be called
* prior to using the HDMI RX core. Initialization of the HDMI RX includes
* setting up the instance data, and ensuring the hardware is in a quiescent
* state.
*
* @param    InstancePtr is a pointer to the XHdmiRx1 core instance.
* @param    CfgPtr points to the configuration structure associated with
*       the HDMI RX core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XV_HdmiRx1_CfgInitialize was successful.
*       - XST_FAILURE if HDMI RX PIO ID mismatched.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_CfgInitialize(XV_HdmiRx1 *InstancePtr, XV_HdmiRx1_Config *CfgPtr,
		UINTPTR EffectiveAddr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XV_HdmiRx1));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
		     sizeof(XV_HdmiRx1_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Check PIO ID */
	RegValue = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				      (XV_HDMIRX1_PIO_ID_OFFSET));
	RegValue = ((RegValue) >> (XV_HDMIRX1_SHIFT_16)) & (XV_HDMIRX1_MASK_16);
	if (RegValue != (XV_HDMIRX1_PIO_ID)) {
		return (XST_FAILURE);
	}

	/*
	Callbacks
	These are placeholders pointing to the StubCallback
	The actual callback pointers will be assigned by the SetCallback function
	*/

	InstancePtr->ConnectCallback = NULL;
	InstancePtr->AuxCallback = NULL;
	InstancePtr->AudCallback = NULL;
	InstancePtr->LnkStaCallback = NULL;
	InstancePtr->DdcCallback = NULL;
	/* InstancePtr->StreamDownCallback = (XV_HdmiRx1_Callback)((void *)StubCallback);*/
	InstancePtr->StreamDownCallback = NULL;
	InstancePtr->StreamInitCallback = NULL;
	InstancePtr->StreamUpCallback = NULL;
	InstancePtr->FrlConfigCallback = NULL;
	InstancePtr->FrlStartCallback = NULL;
	InstancePtr->TmdsConfigCallback = NULL;

	/* Set the HdcpCallback and HdcpRef to stub and XV_HdmiRx1 instance
	 * unless it is overwritten by the subcore initialization.
	 * This add tolerance to the system to not fail in assertion
	 * if the upstream attempts a hdcp operation when the hdcp cores
	 * are not initialized because of bad/incorrect/missing keys.
	 */
	InstancePtr->HdcpCallback = (XV_HdmiRx1_HdcpCallback)((void *)StubCallback);
	InstancePtr->HdcpRef = (void *)InstancePtr;

	InstancePtr->LinkErrorCallback = NULL;
	InstancePtr->SyncLossCallback = NULL;
	InstancePtr->ModeCallback = NULL;
	InstancePtr->TmdsClkRatioCallback = NULL;
	InstancePtr->Stream.Frl.LtpMatchedCounts = 0;
	InstancePtr->Stream.Frl.CurFrlRate = 0;

	/* Clear HDMI variables */
	XV_HdmiRx1_Clear(InstancePtr);

	/* core always runs at 4 ppc */
	InstancePtr->Stream.CorePixPerClk = XVIDC_PPC_4;

	/* Clear connected flag*/
	InstancePtr->Stream.IsConnected = (FALSE);

	InstancePtr->Stream.Frl.FltNoRetrain = (FALSE);
	InstancePtr->Stream.Frl.FltNoTimeout = (FALSE);

	XV_HdmiRx1_FrlIntrDisable(InstancePtr);
	XV_HdmiRx1_FrlReset(InstancePtr, TRUE);

	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_SINK_VER,
				    1);

	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_FRL_RATE,
				    0);

	/* Reset all peripherals*/
	XV_HdmiRx1_PioDisable(InstancePtr);
	XV_HdmiRx1_Tmr1Disable(InstancePtr);
	XV_HdmiRx1_Tmr2Disable(InstancePtr);
	XV_HdmiRx1_Tmr3Disable(InstancePtr);
	XV_HdmiRx1_Tmr4Disable(InstancePtr);
	XV_HdmiRx1_VtdDisable(InstancePtr);
	XV_HdmiRx1_DdcDisable(InstancePtr);
	XV_HdmiRx1_AuxDisable(InstancePtr);
	XV_HdmiRx1_AudioDisable(InstancePtr);
	XV_HdmiRx1_LnkstaDisable(InstancePtr);
	XV_HdmiRx1_PioIntrDisable(InstancePtr);
	XV_HdmiRx1_Tmr1IntrDisable(InstancePtr);
	XV_HdmiRx1_Tmr2IntrDisable(InstancePtr);
	XV_HdmiRx1_Tmr3IntrDisable(InstancePtr);
	XV_HdmiRx1_Tmr4IntrDisable(InstancePtr);
	XV_HdmiRx1_VtdIntrDisable(InstancePtr);
	XV_HdmiRx1_DdcScdcClear(InstancePtr);
	XV_HdmiRx1_SetHpd(InstancePtr,FALSE);

	/*
	PIO peripheral
	*/

	/* PIO: Set event rising edge masks */
	RegValue = (XV_HDMIRX1_PIO_IN_BRDG_OVERFLOW_MASK) |
		(XV_HDMIRX1_PIO_IN_DET_MASK) |
		(XV_HDMIRX1_PIO_IN_LNK_RDY_MASK) |
		(XV_HDMIRX1_PIO_IN_VID_RDY_MASK) |
		(XV_HDMIRX1_PIO_IN_MODE_MASK) |
		(XV_HDMIRX1_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK) |
		(XV_HDMIRX1_PIO_IN_SCDC_TMDS_CLOCK_RATIO_MASK);

	if (InstancePtr->Config.DSC)
		RegValue |= XV_HDMIRX1_PIO_IN_DSC_PPS_PKT_ERR_MASK |
			XV_HDMIRX1_PIO_IN_DSC_EN_STRM_CHG_EVT_MASK;

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_PIO_IN_EVT_RE_OFFSET, RegValue);

	/* PIO: Set event falling edge masks */
	RegValue = (XV_HDMIRX1_PIO_IN_DET_MASK) |
		(XV_HDMIRX1_PIO_IN_VID_RDY_MASK) |
		(XV_HDMIRX1_PIO_IN_MODE_MASK) |
		(XV_HDMIRX1_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK) |
		(XV_HDMIRX1_PIO_IN_SCDC_TMDS_CLOCK_RATIO_MASK);

	if (InstancePtr->Config.DSC)
		RegValue |= XV_HDMIRX1_PIO_IN_DSC_EN_STRM_CHG_EVT_MASK;

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_PIO_IN_EVT_FE_OFFSET, RegValue);

	/*
	Timer
	*/

	/* Set run flag */
	XV_HdmiRx1_Tmr1Enable(InstancePtr);

	/* Enable interrupt */
	XV_HdmiRx1_Tmr1IntrEnable(InstancePtr);

	/* Set run flag */
	XV_HdmiRx1_Tmr2Enable(InstancePtr);

	/* Enable interrupt */
	XV_HdmiRx1_Tmr2IntrEnable(InstancePtr);

	/* Set run flag */
	XV_HdmiRx1_Tmr3Enable(InstancePtr);

	/* Enable interrupt */
	XV_HdmiRx1_Tmr3IntrEnable(InstancePtr);

	/* Enable Skew Lock Event */
	XV_HdmiRx1_SkewLockEvtEnable(InstancePtr);

	/*
	Video Timing detector peripheral
	*/

	/* Set timebase - 16 ms*/
	XV_HdmiRx1_VtdSetTimebase(InstancePtr,
				  XV_HdmiRx1_GetTime16Ms(InstancePtr));

	/* The VTD run flag is set in the armed state*/

	/*
	DDC peripheral
	*/

	/* Enable DDC */
	XV_HdmiRx1_DdcEnable(InstancePtr);

	/* Enable DDC peripheral interrupt */
	/*XV_HdmiRx1_DdcIntrEnable(InstancePtr);*/

	/* Enable SCDC*/
	XV_HdmiRx1_DdcScdcEnable(InstancePtr);

	/*
	AUX peripheral
	*/

	/* The aux peripheral will be enabled in the RX init done callback*/
	/*XV_HdmiRx1_AuxEnable(InstancePtr);*/

	/* Enable AUX peripheral interrupt */
	XV_HdmiRx1_AuxIntrEnable(InstancePtr);
	XV_HdmiRx1_AuxFSyncVrrChEvtEnable(InstancePtr);

	/*
	Audio peripheral
	*/

	/* The audio peripheral willl be enabled in the RX init done callback */
	/* XV_HdmiRx1_AudioEnable(InstancePtr); */

	/* Enable AUD peripheral interrupt */
	XV_HdmiRx1_AudioIntrEnable(InstancePtr);

	/* Enable Audio ACR Update Event */
	/* XV_HdmiRx1_SetAudioAcrUpdateEventEn(InstancePtr); */

	/* Enable Link Status */
	XV_HdmiRx1_LnkstaEnable(InstancePtr);

	/* Enable FRL peripheral */
	XV_HdmiRx1_FrlReset(InstancePtr, FALSE);

	/* Enable FRL Interrupt */
	XV_HdmiRx1_FrlIntrEnable(InstancePtr);
	xil_printf("RX: FRL Base: 0x%X\r\n",
		   (InstancePtr)->Config.BaseAddress + XV_HDMIRX1_FRL_BASE);

	InstancePtr->Stream.Frl.DefaultLtp.Byte[0] = XV_HDMIRX1_LTP_LFSR0;
	InstancePtr->Stream.Frl.DefaultLtp.Byte[1] = XV_HDMIRX1_LTP_LFSR1;
	InstancePtr->Stream.Frl.DefaultLtp.Byte[2] = XV_HDMIRX1_LTP_LFSR2;
	InstancePtr->Stream.Frl.DefaultLtp.Byte[3] = XV_HDMIRX1_LTP_LFSR3;

	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_FLT_READY,
				    1);

	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_FRL_RATE,
				    0);

	XV_HdmiRx1_SetFrlRateWrEvent_En(InstancePtr);

	/* Enable Link Status peripheral interrupt */
	/* XV_HdmiRx1_LinkIntrEnable(InstancePtr); */

	/* Reset the hardware and set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the AXI4-Lite Clock Frequency
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    ClkFreq specifies the value that needs to be set.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiRx1_SetAxiClkFreq(XV_HdmiRx1 *InstancePtr, u32 ClkFreq)
{
	InstancePtr->Config.AxiLiteClkFreq = ClkFreq;
}

/*****************************************************************************/
/**
*
* This function clears the HDMI RX variables and sets them to the defaults.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiRx1_Clear(XV_HdmiRx1 *InstancePtr)
{
	u32 Index;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* The stream is down*/
	InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_DOWN;
	InstancePtr->Stream.IsHdmi = FALSE;
	InstancePtr->Stream.IsFrl = FALSE;
	/* Default RGB*/
	InstancePtr->Stream.Video.ColorFormatId = (XVIDC_CSF_RGB);
	InstancePtr->Stream.Video.IsInterlaced = 0;
	/* Default 8 bits*/
	InstancePtr->Stream.Video.ColorDepth = (XVIDC_BPC_8);
	InstancePtr->Stream.Video.PixPerClk = (XVIDC_PPC_4);
	InstancePtr->Stream.Video.VmId = (XVIDC_VM_NO_INPUT);
	InstancePtr->Stream.Video.Is3D = FALSE;
	InstancePtr->Stream.Video.Info_3D.Format = XVIDC_3D_UNKNOWN;
	InstancePtr->Stream.Video.Timing.HActive = 0;
	InstancePtr->Stream.Video.Timing.HFrontPorch = 0;
	InstancePtr->Stream.Video.Timing.HSyncWidth = 0;
	InstancePtr->Stream.Video.Timing.HBackPorch = 0;
	InstancePtr->Stream.Video.Timing.HTotal = 0;
	InstancePtr->Stream.Video.Timing.HSyncPolarity = 0;
	InstancePtr->Stream.Video.Timing.VActive = 0;
	InstancePtr->Stream.Video.Timing.F0PVFrontPorch = 0;
	InstancePtr->Stream.Video.Timing.F0PVSyncWidth = 0;
	InstancePtr->Stream.Video.Timing.F0PVBackPorch = 0;
	InstancePtr->Stream.Video.Timing.F0PVTotal = 0;
	InstancePtr->Stream.Video.Timing.F1VFrontPorch = 0;
	InstancePtr->Stream.Video.Timing.F1VSyncWidth = 0;
	InstancePtr->Stream.Video.Timing.F1VBackPorch = 0;
	InstancePtr->Stream.Video.Timing.F1VTotal = 0;
	InstancePtr->Stream.Video.Timing.VSyncPolarity = 0;
	InstancePtr->Stream.Vic = 0;
	/* Idle stream*/
	InstancePtr->Stream.Audio.Active = (FALSE);
	/* 2 channels*/
	InstancePtr->Stream.Audio.Channels = 2;
	InstancePtr->Stream.GetVideoPropertiesTries = 0;
	/* Set FRL State*/
	InstancePtr->Stream.Frl.TrainingState = XV_HDMIRX1_FRLSTATE_LTS_L;

	/* AUX */
	InstancePtr->Aux.Header.Data = 0;
	for (Index = 0; Index < 8; Index++) {
		InstancePtr->Aux.Data.Data[Index] = 0;
	}

	/* Audio */
	InstancePtr->AudCts = 0;
	InstancePtr->AudN = 0;
	InstancePtr->AudFormat = 0;

	InstancePtr->IsErrorPrintCount = 0;
	InstancePtr->IsFirstVtemReceived = FALSE;

	/* Call stream down callback*/
	if (InstancePtr->StreamDownCallback) {
		InstancePtr->StreamDownCallback(InstancePtr->StreamDownRef);
	}
}

/*****************************************************************************/
/**
*
* This function starts the HDMI RX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     This is required after a reset or initialization.
*
******************************************************************************/
void XV_HdmiRx1_Start(XV_HdmiRx1 *InstancePtr) {
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Set run flag */
	XV_HdmiRx1_PioEnable(InstancePtr);

	/* Enable interrupt */
	XV_HdmiRx1_PioIntrEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function stops the HDMI RX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
******************************************************************************/
void XV_HdmiRx1_Stop(XV_HdmiRx1 *InstancePtr) {
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Clear run flag */
	XV_HdmiRx1_PioDisable(InstancePtr);

	/* Disable interrupt */
	XV_HdmiRx1_PioIntrDisable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets the HDMI RX stream parameters.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    Ppc specifies the pixel per clock.
*       - 4 = XVIDC_PPC_4
* @param    Clock specifies reference pixel clock frequency.
*
* @return
*       - XST_SUCCESS is always returned.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_SetStream(XV_HdmiRx1 *InstancePtr, XVidC_PixelsPerClock Ppc, u32 Clock)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Ppc == (XVIDC_PPC_4)) ||
			  (Ppc == (XVIDC_PPC_8)));
	Xil_AssertNonvoid(Clock > 0x0);

	/*
	 * Pixels per clock. Not used.
	 * Set only for legacy reasons to display in the stream info.
	 */
	InstancePtr->Stream.Video.PixPerClk = Ppc;

	/* Reference clock */
	InstancePtr->Stream.RefClk = Clock;

	/* Set RX pixel rate */
	XV_HdmiRx1_SetPixelRate(InstancePtr);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI RX Internal VRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI RX Internal VRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI RX. Therefore, clearing the PIO reset output will assert
*       the HDMI Internal video reset.
*       C-style signature:
*       void XV_HdmiRx1_INT_VRST(XV_HdmiRx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiRx1_INT_VRST(XV_HdmiRx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (Reset) {
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_INT_VRST_MASK));
	} else {
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_INT_VRST_MASK));
	}
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI RX Internal LRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI RX Internal LRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI RX. Therefore, clearing the PIO reset output will assert
*       the HDMI Internal link reset.
*       C-style signature:
*       void XV_HdmiRx1_INT_VRST(XV_HdmiRx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiRx1_INT_LRST(XV_HdmiRx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (Reset) {
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_INT_LRST_MASK));
	} else {
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_INT_LRST_MASK));
	}
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI RX External VRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI RX External VRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI RX. Therefore, clearing the PIO reset output will assert
*       the HDMI external video reset.
*       C-style signature:
*       void XV_HdmiRx1_EXT_VRST(XV_HdmiRx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiRx1_EXT_VRST(XV_HdmiRx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (Reset) {
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_EXT_VRST_MASK));
	} else {
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_EXT_VRST_MASK));
	}
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI RX External SYSRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI RX External SYSRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI RX. Therefore, clearing the PIO reset output will assert
*       the HDMI External system reset.
*       C-style signature:
*       void XV_HdmiRx1_EXT_SYSRST(XV_HdmiRx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiRx1_EXT_SYSRST(XV_HdmiRx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (Reset) {
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_EXT_SYSRST_MASK));
	} else {
		XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_EXT_SYSRST_MASK));
	}
}

/*****************************************************************************/
/**
*
* This function sets the pixel rate.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*       - XST_SUCCESS is always returned.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_SetPixelRate(XV_HdmiRx1 *InstancePtr)
{
	u32 RegValue;
	u8 PixelRate;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Mask pixel rate */
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMIRX1_PIO_OUT_MSK_OFFSET),
			    (XV_HDMIRX1_PIO_OUT_PIXEL_RATE_MASK));

	/* Check pixel per clock */
	switch (InstancePtr->Stream.CorePixPerClk) {
	case (XVIDC_PPC_4):
		PixelRate = 2;
		break;

	default:
		PixelRate = 0;
		break;
	}

	/* Set pixel rate for video path */
	RegValue = PixelRate << (XV_HDMIRX1_PIO_OUT_PIXEL_RATE_SHIFT);
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMIRX1_PIO_OUT_OFFSET), RegValue);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the color format
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_SetColorFormat(XV_HdmiRx1 *InstancePtr)
{
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Mask PIO Out Mask register */
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMIRX1_PIO_OUT_MSK_OFFSET),
			    (XV_HDMIRX1_PIO_OUT_COLOR_SPACE_MASK));

	/* Check for color format */
	switch (InstancePtr->Stream.Video.ColorFormatId) {
	case (XVIDC_CSF_YCRCB_444):
		RegValue = 1;
		break;

	case (XVIDC_CSF_YCRCB_422):
		RegValue = 2;
		break;

	case (XVIDC_CSF_YCRCB_420):
		RegValue = 3;
		break;

	default:
		RegValue = 0;
		break;
	}

	/* Write color space into PIO Out register */
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMIRX1_PIO_OUT_OFFSET),
			    (RegValue << (XV_HDMIRX1_PIO_OUT_COLOR_SPACE_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function enables/clear Hot-Plug-Detect.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    SetClr specifies TRUE/FALSE value to either enable or
*       clear HPD respectively.
*
* @return
*       - XST_SUCCESS is always returned.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_SetHpd(XV_HdmiRx1 *InstancePtr, u8 SetClr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((SetClr == (TRUE)) || (SetClr == (FALSE)));

	if (SetClr) {
		XV_HdmiRx1_FrlReset(InstancePtr, FALSE);

		/* Set HPD */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_HPD_MASK));
	} else {
		/* Reset and clear FRL_Rate of SCDC register */
		XV_HdmiRx1_FrlReset(InstancePtr, TRUE);

		/* Clear HPD */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMIRX1_PIO_OUT_HPD_MASK));
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function provides status of the HDMI RX core Link Status peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    Type specifies one of the type for which status to be provided:
*       - 0 = Link error counter for channel 0.
*       - 1 = Link error counter for channel 1.
*       - 2 = Link error counter for channel 2.
*
* @return   Link status of the HDMI RX core link.
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_GetLinkStatus(XV_HdmiRx1 *InstancePtr, u8 Type)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Type < 0x6);

	RegValue = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				      ((XV_HDMIRX1_LNKSTA_LNK_ERR0_OFFSET) +
				       (4 * Type)));

	return RegValue;
}

/*****************************************************************************/
/**
*
* This function provides status of one of the link error counters reached the
* maximum value.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*       - TRUE = Maximum error counter reached.
*       - FALSE = Maximum error counter not reached.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_IsLinkStatusErrMax(XV_HdmiRx1 *InstancePtr)
{
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read Link Status peripheral Status register */
	Status = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				    ((XV_HDMIRX1_LNKSTA_STA_OFFSET)) &
				     (XV_HDMIRX1_LNKSTA_STA_ERR_MAX_MASK));

	if (Status) {
		Status = (TRUE);
	} else {
		Status = (FALSE);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function clears the link error counters.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_ClearLinkStatus(XV_HdmiRx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Set Error Clear bit */
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMIRX1_LNKSTA_CTRL_SET_OFFSET),
			    (XV_HDMIRX1_LNKSTA_CTRL_ERR_CLR_MASK));

	/* Clear Error Clear bit */
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMIRX1_LNKSTA_CTRL_CLR_OFFSET),
			    (XV_HDMIRX1_LNKSTA_CTRL_ERR_CLR_MASK));
}

/*****************************************************************************/
/**
*
* This function provides audio clock regenerating CTS (Cycle-Time Stamp) value
* at the HDMI sink device.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   Audio clock CTS value.
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_GetAcrCts(XV_HdmiRx1 *InstancePtr)
{
	u32 CtsValue;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read cycle time stamp value */
	CtsValue = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				      (XV_HDMIRX1_AUD_CTS_OFFSET));

	return CtsValue;
}

/*****************************************************************************/
/**
*
* This function provides audio clock regenerating factor N value.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   ACR N value.
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_GetAcrN(XV_HdmiRx1 *InstancePtr)
{
	u32 AcrNValue;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read ACR factor N value */
	AcrNValue = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				       (XV_HDMIRX1_AUD_N_OFFSET));

	return AcrNValue;
}

/*****************************************************************************/
/**
*
* This function gets the size of the EDID buffer of the DDC slave.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*       - EDID buffer size
*
* @note     None.
*
******************************************************************************/
u16 XV_HdmiRx1_DdcGetEdidWords(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read status register*/
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_DDC_EDID_STA_OFFSET));
	Data >>= XV_HDMIRX1_DDC_STA_EDID_WORDS_SHIFT;
	Data &= XV_HDMIRX1_DDC_STA_EDID_WORDS_MASK;

	return (Data);
}

/*****************************************************************************/
/**
*
* This function loads the EDID data into the DDC slave.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
* @param    EdidData is a pointer to the EDID data array.
* @param    Length is the length, in bytes, of the EDID array.
*
* @return
*       - XST_SUCCESS if the EDID data was loaded successfully
*       - XST_FAILURE if the EDID data load failed
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_DdcLoadEdid(XV_HdmiRx1 *InstancePtr, u8 *EdidData, u16 Length)
{
	u8 Data;
	u16 Index;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if the EDID data fits in the DDC slave EDID buffer*/
	if (XV_HdmiRx1_DdcGetEdidWords(InstancePtr) >= Length) {
		/* Clear EDID write pointer*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_EDID_WP_OFFSET), 0);

		/* Copy EDID data*/
		for (Index = 0; Index < Length; Index++) {
			Data = *(EdidData + Index);
			XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
					    (XV_HDMIRX1_DDC_EDID_DATA_OFFSET),
					    (Data));
		}

		/* Enable EDID*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_CTRL_SET_OFFSET),
				    (XV_HDMIRX1_DDC_CTRL_EDID_EN_MASK));

		return (XST_SUCCESS);
	}
	/* The EDID data is larger than the DDC slave EDID buffer size*/
	else {
		xdbg_printf(XDBG_DEBUG_GENERAL,"The EDID data structure "
				"is too large to be stored in the DDC "
				"peripheral (%0d).\r\n", Length);
		return (XST_FAILURE);
	}
}

/*****************************************************************************/
/**
*
* This function sets the HDCP address in the DDC peripheral.
* This is implemented as a function and not a macro, so the HDCP driver can
* bind the function call with a handler.
*
* @param    InstancePtr is a pointer to the XHdmi_Rx core instance.
* @param    Address is the HDCP address.
*
* @return   None.
*
* @note     C-style signature:
*       void XHdmiRx1_DdcHdcpSetAddress(XHdmi_Rx *InstancePtr, u8 Address)
*
******************************************************************************/
void XV_HdmiRx1_DdcHdcpSetAddress(XV_HdmiRx1 *InstancePtr, u32 Address)
{
	/* Verify argument.*/
	Xil_AssertVoid(InstancePtr != NULL);

	/* Write Address*/
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMIRX1_DDC_HDCP_ADDRESS_OFFSET),
			    (Address));
}

/*****************************************************************************/
/**
*
* This function writes HDCP data in the DDC peripheral.
* This is implemented as a function and not a macro, so the HDCP driver can
* bind the function call with a handler.
*
* @param    InstancePtr is a pointer to the XHdmi_Rx core instance.
* @param    Data is the HDCP data to be written.
*
* @return   None.
*
* @note     C-style signature:
*       void XHdmiRx1_DdcHdcpWriteData(XHdmi_Rx *InstancePtr, u8 Data)
*
******************************************************************************/
void XV_HdmiRx1_DdcHdcpWriteData(XV_HdmiRx1 *InstancePtr, u32 Data)
{
	/* Verify argument.*/
	Xil_AssertVoid(InstancePtr != NULL);

	/* Write data*/
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMIRX1_DDC_HDCP_DATA_OFFSET),
			    (Data));
}

/*****************************************************************************/
/**
*
* This function reads HDCP data from the DDC peripheral.
* This is implemented as a function and not a macro, so the HDCP driver can
* bind the function call with a handler.
*
* @param    InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @return   Returns the HDCP data read from the DDC peripheral.
*
* @note     C-style signature:
*       u32 XHdmiRx1_DdcHdcpReadData(XHdmi_Rx *InstancePtr)
*
******************************************************************************/
u32 XV_HdmiRx1_DdcHdcpReadData(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XV_HdmiRx1_ReadReg((InstancePtr)->Config.BaseAddress,
				  (XV_HDMIRX1_DDC_HDCP_DATA_OFFSET));
	return (Data);
}

/*****************************************************************************/
/**
*
* This function gets the number of bytes of the HDCP 2.2 write
* buffer in the DDC slave.
*
* @param    InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @return
*       - HDCP 2.2 write buffer words
*
* @note     None.
*
******************************************************************************/
u16 XV_HdmiRx1_DdcGetHdcpWriteMessageBufferWords(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read status register*/
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_DDC_HDCP_STA_OFFSET));
	Data >>= XV_HDMIRX1_DDC_STA_HDCP_WMSG_WORDS_SHIFT;
	Data &= XV_HDMIRX1_DDC_STA_HDCP_WMSG_WORDS_MASK;

	return (Data);
}

/*****************************************************************************/
/**
*
* This function returns the status of the HDCP 2.2 write buffer in the DDC slave.
*
* @param    InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @return
*       - TRUE = HDCP 2.2 message buffer is empty.
*       - FALSE = HDCP 2.2 message buffer contains data.
*
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_DdcIsHdcpWriteMessageBufferEmpty(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read status register*/
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_DDC_HDCP_STA_OFFSET));
	if (Data & XV_HDMIRX1_DDC_STA_HDCP_WMSG_EP_MASK) {
		return (TRUE);
	} else {
		return (FALSE);
	}
}

/*****************************************************************************/
/**
*
* This function gets the number of bytes of the HDCP 2.2 read
* buffer in the DDC slave.
*
* @param    InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @return
*       - HDCP 2.2 read buffer words
*
* @note     None.
*
******************************************************************************/
u16 XV_HdmiRx1_DdcGetHdcpReadMessageBufferWords(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read status register*/
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_DDC_HDCP_STA_OFFSET));
	Data >>= XV_HDMIRX1_DDC_STA_HDCP_RMSG_WORDS_SHIFT;
	Data &= XV_HDMIRX1_DDC_STA_HDCP_RMSG_WORDS_MASK;

	return (Data);
}

/*****************************************************************************/
/**
*
* This function returns the status of the HDCP 2.2 read message
* buffer in the DDC slave.
*
* @param    InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @return
*       - TRUE = HDCP 2.2 message buffer is empty.
*       - FALSE = HDCP 2.2 message buffer contains data.
*
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_DdcIsHdcpReadMessageBufferEmpty(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read status register*/
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_DDC_HDCP_STA_OFFSET));
	if (Data & XV_HDMIRX1_DDC_STA_HDCP_RMSG_EP_MASK) {
		return (TRUE);
	} else {
		return (FALSE);
	}
}

/******************************************************************************/
/**
*
* This function prints stream and timing information on STDIO/UART console.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_Info(XV_HdmiRx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Print stream information */
	XVidC_ReportStreamInfo(&InstancePtr->Stream.Video);
	if(InstancePtr->Stream.Video.IsDSCompressed) {
		xil_printf("\r\n\tCompressed: \r\n");
		XVidC_ReportTiming(&InstancePtr->Stream.Video.Timing,
				   InstancePtr->Stream.Video.IsInterlaced);
		xil_printf("\r\n\tUncompressed: \r\n");
		XVidC_ReportTiming(&InstancePtr->Stream.Video.UncompressedTiming,
				   InstancePtr->Stream.Video.IsInterlaced);
	} else {
		/* Print timing information */
		XVidC_ReportTiming(&InstancePtr->Stream.Video.Timing,
				   InstancePtr->Stream.Video.IsInterlaced);
	}
}

/******************************************************************************/
/**
*
* This function prints debug information on STDIO/UART console.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_DebugInfo(XV_HdmiRx1 *InstancePtr)
{
	u32 FrlStatus;
	u32 Data;
	u32 Data1;
	u32 PioIn;
	u32 DbgSta;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Version */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_VER_ID_OFFSET));

	xil_printf("CORE_VER_PUB, INT: 0x%X, 0x%X\r\n",
			XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_HDMIRX1_VER_VERSION_OFFSET)),
			Data & 0xFFFF);
	xil_printf("ADD_CORE_DBG: 0x%X\r\n", (Data >> 16) & 0xFFFF0000);


	FrlStatus = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					  (XV_HDMIRX1_FRL_STA_OFFSET));
	xil_printf("FRL_MODE, LANES, STR, RATE: %d, %d, %d, %d\r\n",
			(FrlStatus & XV_HDMIRX1_FRL_STA_FRL_MODE_MASK) &&
			XV_HDMIRX1_FRL_STA_FRL_MODE_MASK,
			(FrlStatus & XV_HDMIRX1_FRL_STA_FRL_LANES_MASK) &&
			XV_HDMIRX1_FRL_STA_FRL_LANES_MASK,
			(FrlStatus & XV_HDMIRX1_FRL_STA_STR_MASK) &&
			XV_HDMIRX1_FRL_STA_STR_MASK,
			(FrlStatus >> XV_HDMIRX1_FRL_STA_FRL_RATE_SHIFT) &
			XV_HDMIRX1_FRL_STA_FRL_RATE_MASK);

	/* HDMI/DVI Mode */
	PioIn = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_PIO_IN_OFFSET));
	xil_printf("HDMI/DVI Mode: %d\r\n",
			(PioIn & XV_HDMIRX1_PIO_IN_MODE_MASK) &&
			XV_HDMIRX1_PIO_IN_MODE_MASK);

	/* FRL word aligner tap select changed Status */
	DbgSta = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			  (XV_HDMIRX1_DBG_STA_OFFSET));
	xil_printf("WA_TAP Changed[3:0]: 0x%X\r\n",
			DbgSta & XV_HDMIRX1_DBG_STA_WA_TAP_CHGALL_MASK);

	xil_printf("FRL_WA_Lock[3:0]/Toggle[3:0]: 0x%X/0x%X\r\n",
			(FrlStatus >> XV_HDMIRX1_FRL_STA_WA_LOCK_ALLL_SHIFT) &
			XV_HDMIRX1_FRL_STA_WA_LOCK_ALLL_MASK,
			(FrlStatus >> XV_HDMIRX1_DBG_STA_WA_LOCK_CHGALL_SHIFT) &
			XV_HDMIRX1_DBG_STA_WA_LOCK_CHGALL_MASK);

	xil_printf("TMDS_ALN_LOCK: %d\r\n",
			(PioIn & XV_HDMIRX1_PIO_IN_ALIGNER_LOCK_MASK) &&
			XV_HDMIRX1_PIO_IN_ALIGNER_LOCK_MASK);

	xil_printf("TMDS_SCRM_LOCK[2:0]: 0x%X\r\n",
			(PioIn >> XV_HDMIRX1_PIO_IN_SCRAMBLER_LOCKALLL_SHIFT) &
			XV_HDMIRX1_PIO_IN_SCRAMBLER_LOCKALLL_MASK);


	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_SR_SSB_ERR_CNT0_OFFSET));
	xil_printf("FRL SR/SSB Period Errors (add_core_dbg=51)\r\n  Lane0 "
			"(Non-Training/Training): %d/%d\r\n",
			Data & XV_HDMIRX1_SR_SSB_ERR1_MASK,
			(Data >> XV_HDMIRX1_SR_SSB_ERR2_SHIFT) &
			XV_HDMIRX1_SR_SSB_ERR2_MASK);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_SR_SSB_ERR_CNT1_OFFSET));
	xil_printf("  Lane1 (Non-Training/Training): %d/%d\r\n",
			Data & XV_HDMIRX1_SR_SSB_ERR1_MASK,
			(Data >> XV_HDMIRX1_SR_SSB_ERR2_SHIFT) &
			XV_HDMIRX1_SR_SSB_ERR2_MASK);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_SR_SSB_ERR_CNT2_OFFSET));
	xil_printf("  Lane2 (Non-Training/Training): %d/%d\r\n",
			Data & XV_HDMIRX1_SR_SSB_ERR1_MASK,
			(Data >> XV_HDMIRX1_SR_SSB_ERR2_SHIFT) &
			XV_HDMIRX1_SR_SSB_ERR2_MASK);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_SR_SSB_ERR_CNT3_OFFSET));
	xil_printf("  Lane3 (Non-Training/Training): %d/%d\r\n",
			Data & XV_HDMIRX1_SR_SSB_ERR1_MASK,
			(Data >> XV_HDMIRX1_SR_SSB_ERR2_SHIFT) &
			XV_HDMIRX1_SR_SSB_ERR2_MASK);

	/* FRL Scrambler Lock Status */
	xil_printf("FRL_SCRM_Lock[3:0]/Toggle[3:0]: 0x%X/0x%X\r\n",
			(FrlStatus >> XV_HDMIRX1_FRL_STA_SCRM_LOCK_ALLL_SHIFT) &
			XV_HDMIRX1_FRL_STA_SCRM_LOCK_ALLL_MASK,
			(DbgSta >> XV_HDMIRX1_DBG_STA_SCRM_LOCK_CHGALL_SHIFT) &
			XV_HDMIRX1_DBG_STA_SCRM_LOCK_CHGALL_MASK);

	/* FRL Aligner Lock Status */
	xil_printf("FRL_LANE_Lock[3:0]/Toggle[3:0]: 0x%X/0x%X\r\n",
			(FrlStatus >> XV_HDMIRX1_FRL_STA_LANE_LOCK_ALLL_SHIFT) &
			XV_HDMIRX1_FRL_STA_LANE_LOCK_ALLL_MASK,
			(DbgSta >> XV_HDMIRX1_DBG_STA_LANE_LOCK_CHGALL_SHIFT) &
			XV_HDMIRX1_DBG_STA_LANE_LOCK_CHGALL_MASK);

	/* Skew Lock Status */
	xil_printf("FRL_SKEW_Lock/Toggle: %d/%d\r\n",
			(FrlStatus & XV_HDMIRX1_FRL_STA_SKEW_LOCK_MASK) &&
			XV_HDMIRX1_FRL_STA_SKEW_LOCK_MASK,
			(DbgSta & XV_HDMIRX1_DBG_STA_SKEW_LOCK_CHG_MASK) &&
			XV_HDMIRX1_DBG_STA_SKEW_LOCK_CHG_MASK);

	/* FEC_DPACK_ERR, RSCC, RSFC_CNT Status */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_FRL_ERR_CNT1_OFFSET));
	Data1 = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_FRL_RSFC_CNT_OFFSET));
	xil_printf("FEC_DPACK_ERR, RSCC, RSFC_CNT: %d, %d, %d\r\n",
			(Data >> XV_HDMIRX1_FRL_ERR_CNT1_DPACK_ERR_CNT_SHIFT) &&
			XV_HDMIRX1_FRL_ERR_CNT1_DPACK_ERR_CNT_MASK,
			(Data & XV_HDMIRX1_FRL_ERR_CNT1_RSCC_ERR_CNT_SHIFT) &&
			XV_HDMIRX1_FRL_ERR_CNT1_RSCC_ERR_CNT_MASK,
			Data1);

	/* FRL VID Lock Status and counter */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_FRL_VID_LOCK_CNT_OFFSET));
	xil_printf("FRL_Anlz[Dpack In] Vid Timing\r\n"
			"  FRL_VID_Lock/Toggle_Cnt: %d/%d\r\n",
			(FrlStatus & XV_HDMIRX1_FRL_STA_VID_LOCK_MASK) &&
			XV_HDMIRX1_FRL_STA_VID_LOCK_MASK, Data);

	/* FRL Tribyte Analyzer timing changed counter Status */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_TRIB_ANLZ_TIM_OFFSET));
	xil_printf("Trib_Anlz[Dpack Out] Vid Timing\r\n"
			"  Tim_Toggle_Cnt: %d\r\n",
			Data & XV_HDMIRX1_TRIB_ANLZ_TIM_CHGD_CNT_MASK);
	xil_printf("  Trib_HS_Pol/VS_Pol: %d/%d\r\n",
			(Data & XV_HDMIRX1_TRIB_ANLZ_TIM_HS_POL_MASK) &&
			XV_HDMIRX1_TRIB_ANLZ_TIM_HS_POL_MASK,
			(Data & XV_HDMIRX1_TRIB_ANLZ_TIM_VS_POL_MASK) &&
			XV_HDMIRX1_TRIB_ANLZ_TIM_VS_POL_MASK);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_TRIB_HBP_HS_OFFSET));
	xil_printf("  Trib_HS_SZ: %d\r\n",
			Data & XV_HDMIRX1_TRIB_HBP_HS_HS_SZ_MASK);
	xil_printf("  Trib_HBP_SZ: %d\r\n",
			(Data >> XV_HDMIRX1_TRIB_HBP_HS_HBP_SZ_SHIFT) &
			XV_HDMIRX1_TRIB_HBP_HS_HBP_SZ_MASK);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_TRIB_ANLZ_LN_ACT_OFFSET));
	xil_printf("  Trib_ACT_SZ: %d\r\n",
			Data & XV_HDMIRX1_TRIB_ANLZ_LN_ACT_ACT_SZ_MASK);
	xil_printf("  Trib_LN_SZ: %d\r\n",
			(Data >> XV_HDMIRX1_TRIB_ANLZ_LN_ACT_LN_SZ_SHIFT) &
			XV_HDMIRX1_TRIB_ANLZ_LN_ACT_LN_SZ_MASK);

	xil_printf("FRL_TOT/ACT_RATIO: %d/%d\r\n",
			XV_HdmiRx1_GetFrlTotalPixRatio(InstancePtr),
			XV_HdmiRx1_GetFrlActivePixRatio(InstancePtr));

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_FRL_RATIO_TOT_OFFSET));
	Data1 = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_FRL_RATIO_ACT_OFFSET));
	xil_printf("FRL_VCKE_FREQ: %d\r\n",
			(Data1*(XPAR_XV_HDMIRX1_0_VID_REF_CLK/1000))/(Data*4));

	/* FRL Packets ECC Error Status */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_PKT_ECC_ERR_OFFSET));
	xil_printf("PKT_ECC_ERR: %d\r\n", Data);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_LNKSTA_STA_OFFSET));
	xil_printf("DCS_DEEP_LOCK/8CD_LOCK: %d/%d\r\n",
			(Data & XV_HDMIRX1_LNKSTA_STA_DCS_DEEP_LOCK_MASK) &&
			XV_HDMIRX1_LNKSTA_STA_DCS_DEEP_LOCK_MASK,
			(Data & XV_HDMIRX1_LNKSTA_STA_DCS_8CD_LOCK_MASK) &&
			XV_HDMIRX1_LNKSTA_STA_DCS_8CD_LOCK_MASK);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMIRX1_VCKE_SYS_CNT_OFFSET));
	xil_printf("FRL_VCKE_FREQ(Measured): %d\r\n",
			(5000*(XPAR_XV_HDMIRX1_0_VID_REF_CLK/1000))/Data);
}

/******************************************************************************/
/**
*
* This function prints out RX's SCDC registers and values on STDIO/UART
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_DdcRegDump(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	Data = 0xFFFFFFFF;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("Addr  Data\r\n");
	xil_printf("----  ----\r\n");

	for (u32 Offset = 0; Offset <= 0x5A; Offset++) {
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  XV_HDMIRX1_FRL_SCDC_OFFSET);

	for (u16 i = 0; i < 100; i++) {
		Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					  XV_HDMIRX1_FRL_SCDC_OFFSET);

		if (Data & XV_HDMIRX1_FRL_SCDC_RDY_MASK) {
			break;
		}
	}

	if ((Data & XV_HDMIRX1_FRL_SCDC_RDY_MASK) == 0) {
		xil_printf("XV_HdmiRx1_FrlDdcReadField F1\r\n");
	}


		Data = (XV_HDMIRX1_FRL_SCDC_ADDR_MASK & Offset) |
				XV_HDMIRX1_FRL_SCDC_RD_MASK;


		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
					XV_HDMIRX1_FRL_SCDC_OFFSET, Data);

		for (u16 i = 0; i < 100; i++) {
			Data =
			XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						  XV_HDMIRX1_FRL_SCDC_OFFSET);

			if (Data & XV_HDMIRX1_FRL_SCDC_RDY_MASK) {
				Data = (Data >> XV_HDMIRX1_FRL_SCDC_DAT_SHIFT) &
						0xFF;
				xil_printf("0x%02X: 0x%02X\r\n", Offset, Data);
				break;
			}
		}
	}
}

/*****************************************************************************/
/**
* This function prints out HDMI RX register
*
* @param	InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiRx1_RegisterDebug(XV_HdmiRx1 *InstancePtr)
{
	u32 RegOffset;

	xil_printf("-------------------------------------\r\n");
	xil_printf("       HDMI RX Register Dump \r\n");
	xil_printf("-------------------------------------\r\n");
	for (RegOffset = 0;
			RegOffset <= XV_HDMIRX1_FRL_VID_LOCK_CNT_OFFSET; ) {
			xil_printf("0x%04x      0x%08x\r\n",RegOffset,
			XV_HdmiRx1_ReadReg(
			InstancePtr->Config.BaseAddress, RegOffset));
		RegOffset += 4;
		/* Ignore the DDC Space Register */
		if (RegOffset == 0x114) {
			RegOffset = 0x140;
		}
	}
}

/*****************************************************************************/
/**
*
* This function provides status of the stream
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*       - TRUE = Stream is up.
*       - FALSE = Stream is down.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_IsStreamUp(XV_HdmiRx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_STREAM_UP) {
		return (TRUE);
	} else {
		return (FALSE);
	}
}

/*****************************************************************************/
/**
*
* This function provides the stream scrambler status
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*       - TRUE = Stream is scrambled.
*       - FALSE = Stream is not scrambled.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_IsStreamScrambled(XV_HdmiRx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Stream.IsScrambled);
}

/*****************************************************************************/
/**
*
* This function provides the stream connected status
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*       - TRUE = Stream is connected.
*       - FALSE = Stream is connected.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_IsStreamConnected(XV_HdmiRx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Stream.IsConnected);
}

/*****************************************************************************/
/**
*
* This function gets the SCDC TMDS clock ratio bit
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*       - TRUE = TMDS clock ratio bit is set.
*       - FALSE = TMDS clock ratio bit is cleared.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_GetTmdsClockRatio(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_PIO_IN_OFFSET));

	if ((Data) & (XV_HDMIRX1_PIO_IN_SCDC_TMDS_CLOCK_RATIO_MASK)) {
		return (TRUE);
	} else {
		return (FALSE);
	}
}

/*****************************************************************************/
/**
*
* This function returns the AVI VIC (captured by the AUX peripheral)
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   The AVI VIC code.
*
* @note     None.
*
******************************************************************************/
u8 XV_HdmiRx1_GetAviVic(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read status register*/
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_AUX_STA_OFFSET));
	Data >>= XV_HDMIRX1_AUX_STA_AVI_VIC_SHIFT;
	Data &= XV_HDMIRX1_AUX_STA_AVI_VIC_MASK;

	return (u8)(Data);
}

/*****************************************************************************/
/**
*
* This function returns the AVI colorspace (captured by the AUX peripheral)
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   The AVI colorspace value.
*
* @note     None.
*
******************************************************************************/
XVidC_ColorFormat XV_HdmiRx1_GetAviColorSpace(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;
	XVidC_ColorFormat ColorSpace;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read status register*/
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_AUX_STA_OFFSET));
	Data >>= XV_HDMIRX1_AUX_STA_AVI_CS_SHIFT;
	Data &= XV_HDMIRX1_AUX_STA_AVI_CS_MASK;

	switch (Data) {
	case 1:
		ColorSpace = (XVIDC_CSF_YCRCB_422);
		break;

	case 2:
		ColorSpace = (XVIDC_CSF_YCRCB_444);
		break;

	case 3:
		ColorSpace = (XVIDC_CSF_YCRCB_420);
		break;

	default:
		ColorSpace = (XVIDC_CSF_RGB);
		break;
	}
	return (ColorSpace);
}

/*****************************************************************************/
/**
*
* This function returns the GCP color depth (captured by the AUX peripheral)
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   The GCP color depth.
*
* @note     None.
*
******************************************************************************/
XVidC_ColorDepth XV_HdmiRx1_GetGcpColorDepth(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;
	XVidC_ColorDepth ColorDepth;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read status register*/
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_AUX_STA_OFFSET));
	Data >>= XV_HDMIRX1_AUX_STA_GCP_CD_SHIFT;
	Data &= XV_HDMIRX1_AUX_STA_GCP_CD_MASK;

	switch (Data) {
	case 1:
		ColorDepth = (XVIDC_BPC_10);
		break;

	case 2:
		ColorDepth = (XVIDC_BPC_12);
		break;

	case 3:
		ColorDepth = (XVIDC_BPC_16);
		break;

	default:
		ColorDepth = (XVIDC_BPC_8);
		break;
	}
	return (ColorDepth);
}

/*****************************************************************************/
/**
*
* This function calculates the divider for the frame calculation
*
* @param    Dividend is the dividend value to use in the calculation.
* @param    Divisor is the divisor value to use in the calculation.
*
* @return   The result of the calculation.
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_Divide(u32 Dividend, u32 Divisor)
{
	u32 Result;
	u32 Remainder;
	Result = Dividend / Divisor;
	Remainder = Dividend % Divisor;
	if (Remainder) {
		if (Remainder > (Divisor/2)) {
			Result += 1;
		}
	}
	return (Result);
}

/*****************************************************************************/
/**
*
* This function searches for the video mode based on the vic.
*
* @param    Vic
*
* @return   Vic defined in the VIC table.
*
* @note     None.
*
******************************************************************************/
XVidC_VideoMode XV_HdmiRx1_LookupVmId(u8 Vic)
{
	XHdmiC_VicTable const *Entry;
	u8 Index;

	for (Index = 0;
	     Index < sizeof(VicTable) / sizeof(XHdmiC_VicTable);
	     Index++) {
		Entry = &VicTable[Index];
		if (Entry->Vic == Vic) {
			return (Entry->VmId);
		}
	}

	return XVIDC_VM_NOT_SUPPORTED;
}

/*****************************************************************************/
/**
*
* This function reads the video properties from the aux peripheral
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_GetVideoProperties(XV_HdmiRx1 *InstancePtr)
{
	u32 Status;
	u32 Vic;

	/* Read AUX peripheral status register*/
	Status =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				     (XV_HDMIRX1_AUX_STA_OFFSET));

	/* Check if AVI ready flag has been set*/
	if ((Status) & (XV_HDMIRX1_AUX_STA_AVI_MASK)) {
		/* Get AVI colorspace*/
		InstancePtr->Stream.Video.ColorFormatId =
				XV_HdmiRx1_GetAviColorSpace(InstancePtr);

		/* Get AVI Vic*/
		Vic = XV_HdmiRx1_GetAviVic(InstancePtr);
		if (Vic)
			InstancePtr->Stream.Vic = Vic;

		/* Get GCP colordepth*/
		/* In HDMI the colordepth in YUV422 is always 12 bits
		 * (although on the link itself it is being
		 * transmitted as 8-bits.*/
		/* Therefore if the colorspace is YUV422,
		 * then force the colordepth to 12 bits.*/
		if (InstancePtr->Stream.Video.ColorFormatId ==
		    XVIDC_CSF_YCRCB_422) {
			InstancePtr->Stream.Video.ColorDepth = XVIDC_BPC_12;
		}
		/* Else read the colordepth from the general control packet*/
		else {
			InstancePtr->Stream.Video.ColorDepth =
					XV_HdmiRx1_GetGcpColorDepth(InstancePtr);
		}

		return (XST_SUCCESS);
	} else {
		/* If we tried more than 8 times and still
		 * haven't received any AVI infoframes,*/
		/* then the source is DVI.*/
		/* In this case the video properties
		 * are forced to RGB and 8 bpc.*/
		if (InstancePtr->Stream.GetVideoPropertiesTries > 7) {
			/* Force AVI colorspace to RGB*/
			InstancePtr->Stream.Video.ColorFormatId = XVIDC_CSF_RGB;

			/* Set AVI vic to zero*/
			InstancePtr->Stream.Vic = 0;

			/* Force color depth to 8 bpc*/
			InstancePtr->Stream.Video.ColorDepth = XVIDC_BPC_8;

			return (XST_SUCCESS);
		} else {
			/* Increment tries*/
			InstancePtr->Stream.GetVideoPropertiesTries++;
			return (XST_FAILURE);
		}
	}
}

/*****************************************************************************/
/**
*
* This function reads the video timing from the VTD peripheral
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_GetVideoTiming(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;

	/* Local timing parameters*/
	u16 HActive = 0;
	u16 HFrontPorch = 0;
	u16 HSyncWidth = 0;
	u16 HBackPorch = 0;
	u16 HTotal = 0;
	/* u16 HSyncPolarity; /\*squash unused variable compiler warning *\/ */
	u16 VActive = 0;
	u16 F0PVFrontPorch = 0;
	u16 F0PVSyncWidth = 0;
	u16 F0PVBackPorch = 0;
	u16 F0PVTotal = 0;
	u16 F1VFrontPorch = 0;
	u16 F1VSyncWidth = 0;
	u16 F1VBackPorch = 0;
	u16 F1VTotal = 0;
	u8 Match;
	u8 YUV420_Correction;
	u8 IsInterlaced;
	u8 VrrActive;
	u32 DscEnabledStream = XV_HdmiRx1_DSC_IsEnableStream(InstancePtr);

	if ((InstancePtr->VrrIF.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) &&
			(InstancePtr->VrrIF.VidTimingExtMeta.VRREnabled || InstancePtr->VrrIF.VidTimingExtMeta.QMSEnabled)) {
		VrrActive = TRUE;
	} else if ((InstancePtr->VrrIF.VrrIfType == XV_HDMIC_VRRINFO_TYPE_SPDIF) &&
			(InstancePtr->VrrIF.SrcProdDescIF.FreeSync.FreeSyncActive)) {
		VrrActive = TRUE;
	} else
		VrrActive = FALSE;

	/* If the colorspace is YUV420, then the
	 * horizontal parameters must be doubled*/
	if (InstancePtr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
		YUV420_Correction = 2;
	} else {
		YUV420_Correction = 1;
	}

	/* First we read the video parameters from the VTD
	 * and store them in a local variable*/
	/* Read Total Pixels */
	HTotal =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				     (XV_HDMIRX1_VTD_TOT_PIX_OFFSET)) *
		  YUV420_Correction;

	/* Read Active Pixels */
	HActive =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				      (XV_HDMIRX1_VTD_ACT_PIX_OFFSET)) *
		   YUV420_Correction;

	if (!DscEnabledStream) {
		/* Read Hsync Width */
		HSyncWidth =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						 (XV_HDMIRX1_VTD_HSW_OFFSET)) *
			YUV420_Correction;

		/* Read HFront Porch */
		HFrontPorch =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						  (XV_HDMIRX1_VTD_HFP_OFFSET)) *
			YUV420_Correction;

		/* Read HBack Porch */
		HBackPorch =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						 (XV_HDMIRX1_VTD_HBP_OFFSET)) *
			YUV420_Correction;
	}

	/* Active lines field 1 */
	VActive =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				      (XV_HDMIRX1_VTD_ACT_LIN_OFFSET)) & 0xFFFF;
	/* Total lines field 1 */
	F0PVTotal =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_HDMIRX1_VTD_TOT_LIN_OFFSET)) & 0xFFFF;
	/* Read VSync Width field 1*/
	F0PVSyncWidth =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					    (XV_HDMIRX1_VTD_VSW_OFFSET)) & 0xFFFF;
	/* Read VFront Porch field 1*/
	F0PVFrontPorch =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					     (XV_HDMIRX1_VTD_VFP_OFFSET)) & 0xFFFF;
	/* Read VBack Porch field 1 */
	F0PVBackPorch =  XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					    (XV_HDMIRX1_VTD_VBP_OFFSET)) & 0xFFFF;

	if (!DscEnabledStream) {
		/* Total lines field 2 */
		F1VTotal =  ((XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						 (XV_HDMIRX1_VTD_TOT_LIN_OFFSET))) >> 16);
		/* Read VSync Width field 2*/
		F1VSyncWidth =  ((XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						     (XV_HDMIRX1_VTD_VSW_OFFSET))) >> 16);
		/* Read VFront Porch field 2*/
		F1VFrontPorch =  ((XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						      (XV_HDMIRX1_VTD_VFP_OFFSET))) >> 16);
		/* Read VBack Porch field 2 */
		F1VBackPorch =  ((XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						     (XV_HDMIRX1_VTD_VBP_OFFSET))) >> 16);
	}

	/* Read Status register */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_VTD_STA_OFFSET));

	/* Check video format */
	if ((Data) & (XV_HDMIRX1_VTD_STA_FMT_MASK)) {
		/* Interlaced */
		IsInterlaced = 1;
	} else {
		/* Progressive */
		IsInterlaced = 0;
	}

	/* Next, we compare these values with the previous stored values*/
	/* By default the match is true*/
	Match = TRUE;

	if (!DscEnabledStream) {

		if (!HActive | !HFrontPorch | !HSyncWidth | !HBackPorch | !HTotal |
		    !VActive | !F0PVFrontPorch | !F0PVSyncWidth |
		    !F0PVBackPorch | !F0PVTotal) {
			Match = FALSE;
		}

		if ((IsInterlaced == 1) &
		    (!F1VFrontPorch | !F1VSyncWidth | !F1VBackPorch | !F1VTotal)) {
			Match = FALSE;
		}

		/* Htotal*/
		if (HTotal != InstancePtr->Stream.Video.Timing.HTotal) {
			Match = FALSE;
		}

		/* HActive*/
		if (HActive != InstancePtr->Stream.Video.Timing.HActive) {
			Match = FALSE;
		}

		/* HSyncWidth*/
		if (HSyncWidth != InstancePtr->Stream.Video.Timing.HSyncWidth) {
			Match = FALSE;
		}

		/* HFrontPorch*/
		if (HFrontPorch != InstancePtr->Stream.Video.Timing.HFrontPorch) {
			Match = FALSE;
		}

		/* HBackPorch*/
		if (HBackPorch != InstancePtr->Stream.Video.Timing.HBackPorch) {
			Match = FALSE;
		}

		/* F0PVTotal*/
		if (F0PVTotal != InstancePtr->Stream.Video.Timing.F0PVTotal) {
			if (!VrrActive)
				Match = FALSE;
		}

		/* F1VTotal*/
		if (F1VTotal != InstancePtr->Stream.Video.Timing.F1VTotal) {
			Match = FALSE;
		}

		/* VActive*/
		if (VActive != InstancePtr->Stream.Video.Timing.VActive) {
			Match = FALSE;
		}

		/* F0PVSyncWidth*/
		if (F0PVSyncWidth != InstancePtr->Stream.Video.Timing.F0PVSyncWidth) {
			Match = FALSE;
		}

		/* F1VSyncWidth*/
		if (F1VSyncWidth != InstancePtr->Stream.Video.Timing.F1VSyncWidth) {
			Match = FALSE;
		}

		/* F0PVFrontPorch*/
		if (F0PVFrontPorch != InstancePtr->Stream.Video.Timing.F0PVFrontPorch) {
			if (!VrrActive)
				Match = FALSE;
		}

		/* F1VFrontPorch*/
		if (F1VFrontPorch != InstancePtr->Stream.Video.Timing.F1VFrontPorch) {
			Match = FALSE;
		}

		/* F0PVBackPorch*/
		if (F0PVBackPorch != InstancePtr->Stream.Video.Timing.F0PVBackPorch) {
			Match = FALSE;
		}

		/* F1VBackPorch*/
		if (F1VBackPorch != InstancePtr->Stream.Video.Timing.F1VBackPorch) {
			Match = FALSE;
		}

		if (HTotal != (HActive + HFrontPorch + HSyncWidth + HBackPorch)) {
			Match = FALSE;
		}

		if (F0PVTotal !=
		    (VActive + F0PVFrontPorch + F0PVSyncWidth + F0PVBackPorch)) {
			if (!VrrActive)
				Match = FALSE;
		}

		if (IsInterlaced == 1) {
			if (F1VTotal != (VActive + F1VFrontPorch +
					 F1VSyncWidth + F1VBackPorch)) {
				Match = FALSE;
			}
		} else {
			if (F1VFrontPorch | F1VSyncWidth | F1VBackPorch) {
				Match = FALSE;
			}
		}
	} else {
		/* When DSC is enabled */
		if (!HActive | !VActive | !HTotal | !F0PVTotal |
		    !F0PVSyncWidth | !F0PVFrontPorch | !F0PVBackPorch)
			Match = FALSE;

		if (HTotal != InstancePtr->Stream.Video.Timing.HTotal)
			Match = FALSE;
		if (HActive != InstancePtr->Stream.Video.Timing.HActive)
			Match = FALSE;
		if (VActive != InstancePtr->Stream.Video.Timing.VActive)
			Match = FALSE;
		if (F0PVSyncWidth != InstancePtr->Stream.Video.Timing.F0PVSyncWidth)
			Match = FALSE;
		if (F0PVBackPorch != InstancePtr->Stream.Video.Timing.F0PVBackPorch)
			Match = FALSE;
		if (!VrrActive) {
			if (F0PVTotal != InstancePtr->Stream.Video.Timing.F0PVTotal)
				Match = FALSE;
			if (F0PVFrontPorch != InstancePtr->Stream.Video.Timing.F0PVFrontPorch)
				Match = FALSE;
			if (F0PVTotal != (VActive + F0PVFrontPorch + F0PVSyncWidth + F0PVBackPorch))
				Match = FALSE;
		}
	}

	/* Then we store the timing parameters regardless if there was a match*/
	/* Read Total Pixels */
	InstancePtr->Stream.Video.Timing.HTotal =  HTotal;
	/* Read Active Pixels */
	InstancePtr->Stream.Video.Timing.HActive = HActive;
	/* Active lines field 1 */
	InstancePtr->Stream.Video.Timing.VActive = VActive;
	/* Total lines field 1 */
	InstancePtr->Stream.Video.Timing.F0PVTotal = F0PVTotal;
	/* Read VSync Width field 1*/
	InstancePtr->Stream.Video.Timing.F0PVSyncWidth = F0PVSyncWidth;
	/* Read VFront Porch field 1*/
	InstancePtr->Stream.Video.Timing.F0PVFrontPorch = F0PVFrontPorch;
	/* Read VBack Porch field 1 */
	InstancePtr->Stream.Video.Timing.F0PVBackPorch =  F0PVBackPorch;

	if (!DscEnabledStream) {
		/* Read Hsync Width */
		InstancePtr->Stream.Video.Timing.HSyncWidth = HSyncWidth;
		/* Read HFront Porch */
		InstancePtr->Stream.Video.Timing.HFrontPorch = HFrontPorch;
		/* Read HBack Porch */
		InstancePtr->Stream.Video.Timing.HBackPorch = HBackPorch;
		/* Total lines field 2 */
		InstancePtr->Stream.Video.Timing.F1VTotal = F1VTotal;
		/* Read VSync Width field 2*/
		InstancePtr->Stream.Video.Timing.F1VSyncWidth = F1VSyncWidth;
		/* Read VFront Porch field 2*/
		InstancePtr->Stream.Video.Timing.F1VFrontPorch = F1VFrontPorch;
		/* Read VBack Porch field 2 */
		InstancePtr->Stream.Video.Timing.F1VBackPorch =  F1VBackPorch;
	}

	/* Do we have a match?*/
	/* Yes, then continue processing*/
	if (Match) {
		/* Read Status register */
		Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					  (XV_HDMIRX1_VTD_STA_OFFSET));

		/* Check video format */
		if ((Data) & (XV_HDMIRX1_VTD_STA_FMT_MASK)) {
			/* Interlaced */
			InstancePtr->Stream.Video.IsInterlaced = 1;
		} else {
			/* Progressive */
			InstancePtr->Stream.Video.IsInterlaced = 0;
		}

		/* Check Vsync polarity */
		if ((Data) & (XV_HDMIRX1_VTD_STA_VS_POL_MASK)) {
			/* Positive */
			InstancePtr->Stream.Video.Timing.VSyncPolarity = 1;
		} else {
			/* Negative */
			InstancePtr->Stream.Video.Timing.VSyncPolarity = 0;
		}

		/* Check Hsync polarity */
		if ((Data) & (XV_HDMIRX1_VTD_STA_HS_POL_MASK)) {
			/* Positive */
			InstancePtr->Stream.Video.Timing.HSyncPolarity = 1;
		} else {
			/* Negative */
			InstancePtr->Stream.Video.Timing.HSyncPolarity = 0;
		}

		/* Return success*/
		return (XST_SUCCESS);
	} else {
		/* Setting VmId to not supported for verifying HDMI VTD*/
		InstancePtr->Stream.Video.VmId = XVIDC_VM_NOT_SUPPORTED;

		/* No match */
		return (XST_FAILURE);
	}
}

/*****************************************************************************/
/**
*
* This function sets the PixelClk based on the current ColorDepth, RefClk and
* ColorFormatId.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
******************************************************************************/
void XV_HdmiRx1_SetPixelClk(XV_HdmiRx1 *InstancePtr)
{
	/* Derive the PixelClk from the reference clock and color depth*/
	/* In case of YUV 422 the reference clock is the pixel clock*/
	if (InstancePtr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_422) {
		InstancePtr->Stream.PixelClk = InstancePtr->Stream.RefClk;
	} else {
		/* For the other color spaces the pixel clock
		 * needs to be adjusted*/
		switch (InstancePtr->Stream.Video.ColorDepth) {
		case XVIDC_BPC_10:
			InstancePtr->Stream.PixelClk =
					(InstancePtr->Stream.RefClk << 2)/5;
			break;

		case XVIDC_BPC_12:
			InstancePtr->Stream.PixelClk =
					(InstancePtr->Stream.RefClk << 1)/3;
			break;

		case XVIDC_BPC_16:
			InstancePtr->Stream.PixelClk =
					InstancePtr->Stream.RefClk >> 1;
			break;

		default:
			InstancePtr->Stream.PixelClk =
					InstancePtr->Stream.RefClk;
			break;
		}
	}
}

/*****************************************************************************/
/**
*
* This function checks if RX's CED or RSED counters are incrementing at the
* rate of 4 or higher per second or if they first hit the maximum value
* (0x7FFF) then set the CED_Update or RSED_Update SCDC flags if true.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     This function needs to be called every 1 second to comply with
*           the spec on CED_Update and RSED_Update flags updating.
*
******************************************************************************/
void XV_HdmiRx1_UpdateEdFlags(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u16 Data = 0;
	u8 CedUpdateFlag = FALSE;

	Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr,
			(XV_HDMIRX1_SCDCFIELD_CH0_ERRCNT_MSB));

	/* Proceed only after confirming that the valid bit is set */
	if (Data & 0x80) {
		Data &= 0x7F;
		Data = (Data << 8) | XV_HdmiRx1_FrlDdcReadField(InstancePtr,
				(XV_HDMIRX1_SCDCFIELD_CH0_ERRCNT_LSB));

		if ((Data - InstancePtr->Stream.CedCounter[0]) >= 4 ||
				(Data != InstancePtr->Stream.CedCounter[0] &&
						Data == 0x7FFF)) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("CED CH0: %d\r\n", Data);
#endif
			CedUpdateFlag = TRUE;
		}

		InstancePtr->Stream.CedCounter[0] = Data;
	}

	Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr,
			(XV_HDMIRX1_SCDCFIELD_CH1_ERRCNT_MSB));

	/* Proceed only after confirming that the valid bit is set */
	if (Data & 0x80) {
		Data &= 0x7F;
		Data = (Data << 8) | XV_HdmiRx1_FrlDdcReadField(InstancePtr,
				(XV_HDMIRX1_SCDCFIELD_CH1_ERRCNT_LSB));

		if ((Data - InstancePtr->Stream.CedCounter[1]) >= 4 ||
				(Data != InstancePtr->Stream.CedCounter[1] &&
						Data == 0x7FFF)) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("CED CH1: %d\r\n", Data);
#endif
			CedUpdateFlag = TRUE;
		}

		InstancePtr->Stream.CedCounter[1] = Data;
	}

	Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr,
			(XV_HDMIRX1_SCDCFIELD_CH2_ERRCNT_MSB));

	/* Proceed only after confirming that the valid bit is set */
	if (Data & 0x80) {
		Data &= 0x7F;
		Data = (Data << 8) | XV_HdmiRx1_FrlDdcReadField(InstancePtr,
				(XV_HDMIRX1_SCDCFIELD_CH2_ERRCNT_LSB));

		if ((Data - InstancePtr->Stream.CedCounter[2]) >= 4 ||
				(Data != InstancePtr->Stream.CedCounter[2] &&
						Data == 0x7FFF)) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("CED CH2: %d\r\n", Data);
#endif
			CedUpdateFlag = TRUE;
		}

		InstancePtr->Stream.CedCounter[2] = Data;
	}

	Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr,
			(XV_HDMIRX1_SCDCFIELD_CH3_ERRCNT_MSB));

	/* Proceed only after confirming that the valid bit is set */
	if (Data & 0x80) {
		Data &= 0x7F;
		Data = (Data << 8) | XV_HdmiRx1_FrlDdcReadField(InstancePtr,
				(XV_HDMIRX1_SCDCFIELD_CH3_ERRCNT_LSB));

		if ((Data - InstancePtr->Stream.CedCounter[3]) >= 4 ||
				(Data != InstancePtr->Stream.CedCounter[3] &&
						Data == 0x7FFF)) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("CED CH3: %d\r\n", Data);
#endif
			CedUpdateFlag = TRUE;
		}

		InstancePtr->Stream.CedCounter[3] = Data;
	}

	Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr,
				(XV_HDMIRX1_SCDCFIELD_RSCCNT_MSB));

	/* Proceed only after confirming that the valid bit is set */
	if (Data & 0x80) {
		Data &= 0x7F;
		Data = (Data << 8) | XV_HdmiRx1_FrlDdcReadField(InstancePtr,
				(XV_HDMIRX1_SCDCFIELD_RSCCNT_LSB));

		if ((Data - InstancePtr->Stream.RsCounter) >= 4 ||
				(Data != InstancePtr->Stream.RsCounter &&
						Data == 0x7FFF)) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("RSED: %d\r\n", Data);
#endif

			XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
					XV_HDMIRX1_SCDCFIELD_RSED_UPDATE,
					1);
		}

		InstancePtr->Stream.RsCounter = Data;
	}

	if (CedUpdateFlag == TRUE) {
		XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				XV_HDMIRX1_SCDCFIELD_CED_UPDATE, 1);
	}
}

/*****************************************************************************/
/**
*
* This function sets the timer of RX Core.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @param	Milliseconds specifies the timer's frequency (in milliseconds)
*
* @param	TimerSelect selects which of the timer unit to be used
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiRx1_TmrStartMs(XV_HdmiRx1 *InstancePtr, u32 Milliseconds,
		u8 TimerSelect)
{
	u32 ClockCycles;

	if (Milliseconds > 0) {
		ClockCycles = InstancePtr->Config.AxiLiteClkFreq /
				(1000 / Milliseconds);
	} else {
		ClockCycles = 0;
	}

	if (TimerSelect == 1) {
		XV_HdmiRx1_Tmr1Start(InstancePtr, ClockCycles);
	} else if (TimerSelect == 2) {
		XV_HdmiRx1_Tmr2Start(InstancePtr, ClockCycles);
	} else if (TimerSelect == 3) {
		XV_HdmiRx1_Tmr3Start(InstancePtr, ClockCycles);
	} else if (TimerSelect == 4) {
		XV_HdmiRx1_Tmr4Start(InstancePtr, ClockCycles);
	}
}

/*****************************************************************************/
/**
*
* This function is a stub for the asynchronous callback. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, all
* handlers are set to this callback. It is considered an error for this
* handler to be invoked.
*
* @param    CallbackRef is a callback reference passed in by the upper
*       layer when setting the callback functions, and passed back to
*       the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void StubCallback(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef != NULL);
	/* Xil_AssertVoidAlways(); */
}

XV_HdmiC_VideoTimingExtMeta *XV_HdmiRx1_GetVidTimingExtMeta(
		XV_HdmiRx1 *InstancePtr)
{
	return &(InstancePtr->VrrIF.VidTimingExtMeta);
}

void XV_HdmiRx1_ParseVideoTimingExtMetaIF(XV_HdmiRx1 *InstancePtr)
{
	XV_HdmiC_VideoTimingExtMeta *ExtMeta;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);

	ExtMeta = XV_HdmiRx1_GetVidTimingExtMeta(InstancePtr);
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					XV_HDMIRX1_AUX_VTEM_OFFSET);

	ExtMeta->VRREnabled = Data & XV_HDMIRX1_AUX_VTEM_VRR_EN_MASK;
	ExtMeta->MConstEnabled = (Data & XV_HDMIRX1_AUX_VTEM_M_CONST_MASK) >>
					XV_HDMIRX1_AUX_VTEM_M_CONST_SHIFT;
	ExtMeta->FVAFactorMinus1 = (Data & XV_HDMIRX1_AUX_VTEM_FVA_FACT_M1_MASK) >>
					XV_HDMIRX1_AUX_VTEM_FVA_FACT_M1_SHIFT;
	ExtMeta->BaseVFront = (Data & XV_HDMIRX1_AUX_VTEM_BASE_VFRONT_MASK) >>
					XV_HDMIRX1_AUX_VTEM_BASE_VFRONT_SHIFT;
	ExtMeta->BaseRefreshRate = (Data &
			XV_HDMIRX1_AUX_VTEM_BASE_REFRESH_RATE_MASK) >>
			XV_HDMIRX1_AUX_VTEM_BASE_REFRESH_RATE_SHIFT;
	ExtMeta->RBEnabled = (Data & XV_HDMIRX1_AUX_VTEM_RB_MASK) >>
				XV_HDMIRX1_AUX_VTEM_RB_SHIFT;
	ExtMeta->QMSEnabled = (Data & XV_HDMIRX1_AUX_VTEM_QMS_EN_MASK) >>
				XV_HDMIRX1_AUX_VTEM_QMS_EN_SHIFT;
	ExtMeta->NextTransferRate = (Data & XV_HDMIRX1_AUX_VTEM_NEXT_TFR_MASK) >>
				XV_HDMIRX1_AUX_VTEM_NEXT_TFR_SHIFT;
}

XV_HdmiC_SrcProdDescIF *XV_HdmiRx1_GetSrcProdDescIF(
		XV_HdmiRx1 *InstancePtr)
{
	return &(InstancePtr->VrrIF.SrcProdDescIF);
}

void XV_HdmiRx1_ParseSrcProdDescInfoframe(XV_HdmiRx1 *InstancePtr)
{
	XV_HdmiC_SrcProdDescIF *SpdIfPtr;
	XV_HdmiC_FreeSync *FreeSyncPtr;
	XV_HdmiC_FreeSyncPro *FreeSyncProPtr;
	u32 FsyncData, FsyncProData;

	Xil_AssertVoid(InstancePtr != NULL);

	SpdIfPtr = XV_HdmiRx1_GetSrcProdDescIF(InstancePtr);
	FreeSyncPtr = &SpdIfPtr->FreeSync;
	FreeSyncProPtr = &SpdIfPtr->FreeSyncPro;

	FsyncData = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				XV_HDMIRX1_AUX_FSYNC_OFFSET);

	FreeSyncPtr->Version = FsyncData & XV_HDMIRX1_AUX_FSYNC_VERSION_MASK;
	FreeSyncPtr->FreeSyncSupported = (FsyncData &
				XV_HDMIRX1_AUX_FSYNC_SUPPORT_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_SUPPORT_SHIFT;
	FreeSyncPtr->FreeSyncEnabled = (FsyncData &
				XV_HDMIRX1_AUX_FSYNC_ENABLED_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_ENABLED_SHIFT;
	FreeSyncPtr->FreeSyncActive = (FsyncData &
				XV_HDMIRX1_AUX_FSYNC_ACTIVE_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_ACTIVE_SHIFT;
	FreeSyncPtr->FreeSyncMinRefreshRate = (FsyncData &
				XV_HDMIRX1_AUX_FSYNC_MIN_REF_RATE_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_MIN_REF_RATE_SHIFT;
	FreeSyncPtr->FreeSyncMaxRefreshRate = (FsyncData &
				XV_HDMIRX1_AUX_FSYNC_MAX_REF_RATE_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_MAX_REF_RATE_SHIFT;

	if (FreeSyncPtr->Version == 2) {
		FsyncProData = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					XV_HDMIRX1_AUX_FSYNC_PRO_OF);
		FreeSyncProPtr->NativeColorSpaceActive = (FsyncData &
				XV_HDMIRX1_AUX_FSYNC_PRO_NTV_CS_ACT_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_PRO_NTV_CS_ACT_SHIFT;
		FreeSyncProPtr->BrightnessControlActive = (FsyncData &
				XV_HDMIRX1_AUX_FSYNC_PRO_BRIGHT_CTRL_ACT_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_PRO_BRIGHT_CTRL_ACT_SHIFT;
		FreeSyncProPtr->LocalDimControlActive = (FsyncData &
				XV_HDMIRX1_AUX_FSYNC_PRO_LDIMM_CTRL_ACT_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_PRO_LDIMM_CTRL_ACT_SHIFT;
		FreeSyncProPtr->sRGBEOTFActive = FsyncProData &
				XV_HDMIRX1_AUX_FSYNC_PRO_SRGB_EOTF_MASK;
		FreeSyncProPtr->BT709EOTFActive = (FsyncProData &
				XV_HDMIRX1_AUX_FSYNC_PRO_BT709_EOTF_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_PRO_BT709_EOTF_SHIFT;
		FreeSyncProPtr->Gamma22EOTFActive = (FsyncProData &
				XV_HDMIRX1_AUX_FSYNC_PRO_GAMMA_2_2_EOTF_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_PRO_GAMMA_2_2_EOTF_SHIFT;
		FreeSyncProPtr->Gamma26EOTFActive = (FsyncProData &
				XV_HDMIRX1_AUX_FSYNC_PRO_GAMMA_2_6_EOTF_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_PRO_GAMMA_2_6_EOTF_SHIFT;
		FreeSyncProPtr->PQEOTFActive = (FsyncProData &
				XV_HDMIRX1_AUX_FSYNC_PRO_PQ_EOTF_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_PRO_PQ_EOTF_SHIFT;
		FreeSyncProPtr->BrightnessControl = (FsyncProData &
				XV_HDMIRX1_AUX_FSYNC_PRO_BRIGHT_CTRL_MASK) >>
				XV_HDMIRX1_AUX_FSYNC_PRO_BRIGHT_CTRL_SHIFT;
	}
}

/*****************************************************************************/
/**
*
* This function returns VRR infoframe type
*
* @param    InstancePtr is a pointer to the XHdmiRx1 core instance.
*
* @return XV_HdmiRx1_VrrInfoframeType
*
* @note   None.
*
******************************************************************************/
XV_HdmiC_VrrInfoframeType XV_HdmiRx1_GetVrrIfType(XV_HdmiRx1 *InstancePtr)
{
	return InstancePtr->VrrIF.VrrIfType;
}

/*****************************************************************************/
/**
*
* This function Sets VRR infoframe type
*
* @param    InstancePtr is a pointer to the XHdmiRx1 core instance.
* @param    Type of type XV_HdmiRx1_VrrInfoframeType
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRx1_SetVrrIfType(XV_HdmiRx1 *InstancePtr,
		XV_HdmiC_VrrInfoframeType Type)
{
	InstancePtr->VrrIF.VrrIfType = Type;
}

/*****************************************************************************/
/**
*
* This function sets the Dynamic HDR buffer address
*
* @param    InstancePtr is a pointer to the XHdmiRx1 core instance.
* @param    Addr is an address in 64bit format.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRx1_DynHDR_SetAddr(XV_HdmiRx1 *InstancePtr, u64 Addr)
{
	u32 tmpaddr;

	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(Addr);
	Xil_AssertVoid(!(Addr & 0x3F));

	if (!InstancePtr->Config.DynamicHDR) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "\r\nWarning: HdmiRx1 Dynamic HDR disabled\r\n");
		return;
	}

	tmpaddr = (u32)Addr;
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    XV_HDMIRX1_AUX_DYN_HDR_MEMADDR_LSB_OFFSET,
			    tmpaddr);

	tmpaddr = (u32)((Addr & 0xFFFFFFFF00000000) >> 32);
	XV_HdmiRx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    XV_HDMIRX1_AUX_DYN_HDR_MEMADDR_MSB_OFFSET,
			    tmpaddr);
}

/*****************************************************************************/
/**
*
* This function gets the Dynamic HDR packet type, length, whether graphics
* overlay and errors if any.
*
* @param    InstancePtr is a pointer to the XHdmiRx1 core instance.
* @param    RxDynHdrInfoPtr is a pointer to XHdmiRx1 Dynamic HDR info instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRx1_DynHDR_GetInfo(XV_HdmiRx1 *InstancePtr,
			       XV_HdmiRx1_DynHDR_Info *RxDynHdrInfoPtr)
{
	u32 tmp;

	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(RxDynHdrInfoPtr);

	if (!InstancePtr->Config.DynamicHDR) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "\r\nWarning: HdmiRx1 Dynamic HDR disabled\r\n");
		return;
	}

	tmp = XV_HdmiRx1_ReadReg((InstancePtr)->Config.BaseAddress,
				 XV_HDMIRX1_AUX_DYN_HDR_INFO_OFFSET);
	RxDynHdrInfoPtr->pkt_type = tmp &
		XV_HDMIRX1_AUX_DYN_HDR_INFO_PKT_TYPE_MASK;
	RxDynHdrInfoPtr->pkt_length = (tmp &
		XV_HDMIRX1_AUX_DYN_HDR_INFO_PKT_LEN_MASK) >>
		XV_HDMIRX1_AUX_DYN_HDR_INFO_PKT_LEN_SHIFT;

	tmp = XV_HdmiRx1_ReadReg((InstancePtr)->Config.BaseAddress,
				 XV_HDMIRX1_AUX_DYN_HDR_STS_OFFSET);
	RxDynHdrInfoPtr->gof = tmp & XV_HDMIRX1_AUX_DYN_HDR_STS_GOF_MASK;
	RxDynHdrInfoPtr->err = (XV_HdmiRx1_DynHdrErrType)((tmp &
		XV_HDMIRX1_AUX_DYN_HDR_STS_ERR_MASK) >>
		XV_HDMIRX1_AUX_DYN_HDR_STS_ERR_MASK);
}

u32 XV_HdmiRx1_DSC_IsEnableStream(XV_HdmiRx1 *InstancePtr)
{
	u32 tmp;

	Xil_AssertNonvoid(InstancePtr);

	if (!InstancePtr->Config.DSC)
		return 0;

	tmp = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				 XV_HDMIRX1_PIO_IN_OFFSET);

	return ((tmp & XV_HDMIRX1_PIO_IN_DSC_EN_STRM_MASK) ? 1 : 0);
}

int XV_HdmiRx1_DSC_SetDecodeFail(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr);

	if (!InstancePtr->Config.DSC)
		return XST_NO_FEATURE;

	return XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
					   XV_HDMIRX1_SCDCFIELD_DSC_DECODE_FAIL,
					   1);
}

int XV_HdmiRx1_DSC_SetDscFrlMax(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr);

	if (!InstancePtr->Config.DSC)
		return XST_NO_FEATURE;

	return XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
					   XV_HDMIRX1_SCDCFIELD_DSC_FRL_MAX,
					   1);
}
