/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx1_intr.c
*
* This file contains interrupt related functions for Xilinx HDMI RX core.
* Please see xv_hdmirx1.h for more details of the driver.
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

/************************** Constant Definitions *****************************/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void HdmiRx1_PioIntrHandler(XV_HdmiRx1 *InstancePtr);
static void HdmiRx1_TmrIntrHandler(XV_HdmiRx1 *InstancePtr);
static void HdmiRx1_VtdIntrHandler(XV_HdmiRx1 *InstancePtr);
static void HdmiRx1_DdcIntrHandler(XV_HdmiRx1 *InstancePtr);
static void HdmiRx1_AuxIntrHandler(XV_HdmiRx1 *InstancePtr);
static void HdmiRx1_AudIntrHandler(XV_HdmiRx1 *InstancePtr);
static void HdmiRx1_LinkStatusIntrHandler(XV_HdmiRx1 *InstancePtr);
static void HdmiRx1_FrlIntrHandler(XV_HdmiRx1 *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX driver.
*
* This handler reads the pending interrupt from PIO, DDC, TIMDET, AUX, AUD
* and LNKSTA peripherals, determines the source of the interrupts, clears the
* interrupts and calls callbacks accordingly.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XV_HdmiRx1_SetCallback() during initialization phase. An example delivered
* with this driver demonstrates how this could be done.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 instance that just
*       interrupted.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_IntrHandler(void *InstancePtr)
{
	u32 Data;
	XV_HdmiRx1 *HdmiRx1Ptr = (XV_HdmiRx1 *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(HdmiRx1Ptr != NULL);
	Xil_AssertVoid(HdmiRx1Ptr->IsReady == XIL_COMPONENT_IS_READY);

	/* PIO */
	Data = XV_HdmiRx1_ReadReg(HdmiRx1Ptr->Config.BaseAddress,
				  (XV_HDMIRX1_PIO_STA_OFFSET)) &
	       (XV_HDMIRX1_PIO_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to PIO handler */
		HdmiRx1_PioIntrHandler(HdmiRx1Ptr);
	}

	/* Timer */
	Data = XV_HdmiRx1_ReadReg(HdmiRx1Ptr->Config.BaseAddress,
				  (XV_HDMIRX1_TMR_STA_OFFSET)) &
	       (XV_HDMIRX1_TMR_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to PIO handler */
		HdmiRx1_TmrIntrHandler(HdmiRx1Ptr);
	}

	/* Video Timing detector */
	Data = XV_HdmiRx1_ReadReg(HdmiRx1Ptr->Config.BaseAddress,
				  (XV_HDMIRX1_VTD_STA_OFFSET)) &
	       (XV_HDMIRX1_VTD_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to video timing detector handler */
		HdmiRx1_VtdIntrHandler(HdmiRx1Ptr);
	}

	/* DDC */
	Data = XV_HdmiRx1_ReadReg(HdmiRx1Ptr->Config.BaseAddress,
				  (XV_HDMIRX1_DDC_STA_OFFSET)) &
	       (XV_HDMIRX1_DDC_STA_IRQ_MASK);

	/* Is the IRQ flag set */
	if (Data) {
		/* Jump to DDC handler */
		HdmiRx1_DdcIntrHandler(HdmiRx1Ptr);
	}

	/* AUX */
	Data = XV_HdmiRx1_ReadReg(HdmiRx1Ptr->Config.BaseAddress,
				  (XV_HDMIRX1_AUX_STA_OFFSET)) &
	       (XV_HDMIRX1_AUX_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to AUX handler */
		HdmiRx1_AuxIntrHandler(HdmiRx1Ptr);
	}

	/* Audio */
	Data = XV_HdmiRx1_ReadReg(HdmiRx1Ptr->Config.BaseAddress,
				  (XV_HDMIRX1_AUD_STA_OFFSET)) &
	       (XV_HDMIRX1_AUD_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to Audio handler */
		HdmiRx1_AudIntrHandler(HdmiRx1Ptr);
	}

	/* Link status */
	Data = XV_HdmiRx1_ReadReg(HdmiRx1Ptr->Config.BaseAddress,
				  (XV_HDMIRX1_LNKSTA_STA_OFFSET)) &
	       (XV_HDMIRX1_LNKSTA_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to Link Status handler */
		HdmiRx1_LinkStatusIntrHandler(HdmiRx1Ptr);
	}

	/* FRL */
	Data = XV_HdmiRx1_ReadReg(HdmiRx1Ptr->Config.BaseAddress,
				 (XV_HDMIRX1_FRL_STA_OFFSET)) &
				 (XV_HDMIRX1_FRL_STA_IRQ_MASK);

	/* Check for IRQ flag set */
	if (Data) {
		/* Jump to Link Status handler */
		HdmiRx1_FrlIntrHandler(HdmiRx1Ptr);
	}
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                 Callback Function Type
* -------------------------   -----------------------------------------------
* (XV_HDMIRX1_HANDLER_VTD)      VtdCallback
* (XV_HDMIRX1_HANDLER_AUX)      AuxCallback
* (XV_HDMIRX1_HANDLER_AUD)      AudCallback
* (XV_HDMIRX1_HANDLER_LNKSTA)   LnkStaCallback
* (XV_HDMIRX1_HANDLER_PIO)      PioCallback
* </pre>
*
* @param    InstancePtr is a pointer to the HDMI RX core instance.
* @param    HandlerType specifies the type of handler.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note     Invoking this function for a handler that already has been
*       installed replaces it with the new handler.
*
******************************************************************************/
int XV_HdmiRx1_SetCallback(XV_HdmiRx1 *InstancePtr,
		XV_HdmiRx1_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= (XV_HDMIRX1_HANDLER_CONNECT));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {

	case (XV_HDMIRX1_HANDLER_CONNECT):
		InstancePtr->ConnectCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->ConnectRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMIRX1_HANDLER_AUX):
		InstancePtr->AuxCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->AuxRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMIRX1_HANDLER_AUD):
		InstancePtr->AudCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->AudRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMIRX1_HANDLER_LNKSTA):
		InstancePtr->LnkStaCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->LnkStaRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Ddc*/
	case (XV_HDMIRX1_HANDLER_DDC):
		InstancePtr->DdcCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->DdcRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Stream down*/
	case (XV_HDMIRX1_HANDLER_STREAM_DOWN):
		InstancePtr->StreamDownCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->StreamDownRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Stream Init*/
	case (XV_HDMIRX1_HANDLER_STREAM_INIT):
		InstancePtr->StreamInitCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->StreamInitRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Stream up*/
	case (XV_HDMIRX1_HANDLER_STREAM_UP):
		InstancePtr->StreamUpCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->StreamUpRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* HDCP*/
	case (XV_HDMIRX1_HANDLER_HDCP):
		InstancePtr->HdcpCallback = (XV_HdmiRx1_HdcpCallback)CallbackFunc;
		InstancePtr->HdcpRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* HDCP 1.4 Event*/
	case (XV_HDMIRX1_HANDLER_DDC_HDCP_14_PROT):
		InstancePtr->Hdcp14ProtEvtCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->Hdcp14ProtEvtRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* HDCP 2.2 Event*/
	case (XV_HDMIRX1_HANDLER_DDC_HDCP_22_PROT):
		InstancePtr->Hdcp22ProtEvtCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->Hdcp22ProtEvtRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMIRX1_HANDLER_LINK_ERROR):
		InstancePtr->LinkErrorCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->LinkErrorRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Bridge FIFO Overflow*/
	case (XV_HDMIRX1_HANDLER_BRDG_OVERFLOW):
		InstancePtr->BrdgOverflowCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->BrdgOverflowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Sync Loss*/
	case (XV_HDMIRX1_HANDLER_SYNC_LOSS):
		InstancePtr->SyncLossCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->SyncLossRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Mode*/
	case (XV_HDMIRX1_HANDLER_MODE):
		InstancePtr->ModeCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->ModeRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* TMDS clock ratio*/
	case (XV_HDMIRX1_HANDLER_TMDS_CLK_RATIO):
		InstancePtr->TmdsClkRatioCallback =
                          (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->TmdsClkRatioRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Vic Error */
	case (XV_HDMIRX1_HANDLER_VIC_ERROR):
		InstancePtr->VicErrorCallback =
			  (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->VicErrorRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Phy Reset */
	case (XV_HDMIRX1_HANDLER_PHY_RESET):
		InstancePtr->PhyResetCallback =
			  (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->PhyResetRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Link Ready Error */
	case (XV_HDMIRX1_HANDLER_LNK_RDY_ERR):
		InstancePtr->LnkRdyErrorCallback =
			  (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->LnkRdyErrorRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Video Ready Error */
	case (XV_HDMIRX1_HANDLER_VID_RDY_ERR):
		InstancePtr->VidRdyErrorCallback =
			  (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->VidRdyErrorRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Skew Lock Error */
	case (XV_HDMIRX1_HANDLER_SKEW_LOCK_ERR):
		InstancePtr->SkewLockErrorCallback =
			  (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->SkewLockErrorRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL Config*/
	case (XV_HDMIRX1_HANDLER_FRL_CONFIG):
		InstancePtr->FrlConfigCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->FrlConfigRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL Start*/
	case (XV_HDMIRX1_HANDLER_FRL_START):
		InstancePtr->FrlStartCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->FrlStartRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* TMDS Config*/
	case (XV_HDMIRX1_HANDLER_TMDS_CONFIG):
		InstancePtr->TmdsConfigCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->TmdsConfigRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:L*/
	case (XV_HDMIRX1_HANDLER_FRL_LTSL):
		InstancePtr->FrlLtsLCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->FrlLtsLRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:1*/
	case (XV_HDMIRX1_HANDLER_FRL_LTS1):
		InstancePtr->FrlLts1Callback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->FrlLts1Ref = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:2*/
	case (XV_HDMIRX1_HANDLER_FRL_LTS2):
		InstancePtr->FrlLts2Callback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->FrlLts2Ref = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:3*/
	case (XV_HDMIRX1_HANDLER_FRL_LTS3):
		InstancePtr->FrlLts3Callback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->FrlLts3Ref = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:4*/
	case (XV_HDMIRX1_HANDLER_FRL_LTS4):
		InstancePtr->FrlLts4Callback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->FrlLts4Ref = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* FRL LTS:P*/
	case (XV_HDMIRX1_HANDLER_FRL_LTSP):
		InstancePtr->FrlLtsPCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->FrlLtsPRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMIRX1_HANDLER_VFP_CHANGE):
		InstancePtr->VfpChangeCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->VfpChangeRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMIRX1_HANDLER_VRR_RDY):
		InstancePtr->VrrRdyCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->VrrRdyRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	case (XV_HDMIRX1_HANDLER_DYN_HDR):
		InstancePtr->DynHdrCallback = (XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->DynHdrRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	/* DSC */
	case (XV_HDMIRX1_HANDLER_DSC_STRM_CH):
		InstancePtr->DSCStreamChangeEventCallback =
		(XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->DSCStrmChgEvtRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case (XV_HDMIRX1_HANDLER_DSC_PKT_ERR):
		InstancePtr->DSCPktErrCallback =
		(XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->DSCPktErrRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	case (XV_HDMIRX1_HANDLER_DSC_STS_UPDT):
		InstancePtr->DSCStsUpdtEvtCallback =
		(XV_HdmiRx1_Callback)CallbackFunc;
		InstancePtr->DSCStsUpdtEvtRef = CallbackRef;
		Status = XST_SUCCESS;
		break;

	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX Timing Detector
* (TIMDET) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx1_VtdIntrHandler(XV_HdmiRx1 *InstancePtr)
{
	u32 Status;
	u32 TotalPixFRLRatio = 0;
	u32 ActivePixFRLRatio = 0;
	u64 VidClk = 0;
	u8 Remainder = 0;
	XVidC_VideoMode DecodedVmId = 0;
	u8 VrrActive = FALSE;
	u8 FvaFactor = 1;
	const XVidC_VideoTimingMode *VidEntry;
	u8 Vic;
	XVidC_VideoTiming *BaseTiming;
	u8 Error = FALSE;

	if ((InstancePtr->VrrIF.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) &&
			(InstancePtr->VrrIF.VidTimingExtMeta.VRREnabled)) {
		VrrActive = TRUE;
	} else if ((InstancePtr->VrrIF.VrrIfType == XV_HDMIC_VRRINFO_TYPE_SPDIF) &&
			(InstancePtr->VrrIF.SrcProdDescIF.FreeSync.FreeSyncActive)) {
		VrrActive = TRUE;
	} else
		VrrActive = FALSE;

	if (InstancePtr->VrrIF.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM)
		FvaFactor = InstancePtr->VrrIF.VidTimingExtMeta.FVAFactorMinus1 + 1;

	/* Read Video timing detector Status register */
	Status = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_VTD_STA_OFFSET));

	/* Check for time base event */
	if ((Status) & (XV_HDMIRX1_VTD_STA_TIMEBASE_EVT_MASK)) {

		/* Clear event flag */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_VTD_STA_OFFSET),
				    (XV_HDMIRX1_VTD_STA_TIMEBASE_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Check if we are in lock state */
		if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_STREAM_LOCK) {
			u32 DscEnabledStream = XV_HdmiRx1_DSC_IsEnableStream(InstancePtr);
			InstancePtr->Stream.Video.IsDSCompressed = DscEnabledStream;

			/* Read video timing */
			Status = XV_HdmiRx1_GetVideoTiming(InstancePtr);

			if (Status == XST_SUCCESS) {

				if (DscEnabledStream) {
					u32 Data;
					XVidC_VideoTiming *UncompressedTiming;

					Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
								  XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT);

					UncompressedTiming = &(InstancePtr->Stream.Video.UncompressedTiming);
					UncompressedTiming->HFrontPorch =
						(Data >> XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT_ORIG_HFRONT_SHIFT) &
						XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT_ORIG_HFRONT_MASK;
					UncompressedTiming->HSyncWidth =
						(Data >> XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT_ORIG_HSYNC_SHIFT) &
						XV_HDMIRX1_DSC_CVTEM_HSYNC_HFRONT_ORIG_HSYNC_MASK;

					Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
								  XV_HDMIRX1_DSC_CVTEM_HBACK_HCACT);

					UncompressedTiming->HBackPorch =
						(Data >> XV_HDMIRX1_DSC_CVTEM_HBACK_HCACT_HBACK_SHIFT) &
						 XV_HDMIRX1_DSC_CVTEM_HBACK_HCACT_HBACK_MASK;

					Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
								  XV_HDMIRX1_DSC_CVTEM_HACT_VACT);

					UncompressedTiming->HActive =
						(Data >> XV_HDMIRX1_DSC_CVTEM_HACT_VACT_HACT_SHIFT) &
						XV_HDMIRX1_DSC_CVTEM_HACT_VACT_HACT_MASK;

					UncompressedTiming->HTotal =
						UncompressedTiming->HActive +
						UncompressedTiming->HFrontPorch +
						UncompressedTiming->HSyncWidth +
						UncompressedTiming->HBackPorch;

					UncompressedTiming->VActive = InstancePtr->Stream.Video.Timing.VActive;
					UncompressedTiming->F0PVTotal = InstancePtr->Stream.Video.Timing.F0PVTotal;
					UncompressedTiming->F0PVSyncWidth = InstancePtr->Stream.Video.Timing.F0PVSyncWidth;
					UncompressedTiming->F0PVFrontPorch = InstancePtr->Stream.Video.Timing.F0PVFrontPorch;
					UncompressedTiming->F0PVBackPorch = InstancePtr->Stream.Video.Timing.F0PVBackPorch;
					UncompressedTiming->HSyncPolarity = InstancePtr->Stream.Video.Timing.HSyncPolarity;
					UncompressedTiming->VSyncPolarity = InstancePtr->Stream.Video.Timing.VSyncPolarity;
				}

				if (InstancePtr->Stream.IsFrl == TRUE) {
					/*Get the ratio and print */
					ActivePixFRLRatio = XV_HdmiRx1_Divide(XV_HdmiRx1_GetFrlActivePixRatio(InstancePtr), 1000);
					TotalPixFRLRatio  = XV_HdmiRx1_Divide(XV_HdmiRx1_GetFrlTotalPixRatio(InstancePtr), 1000);
					VidClk = ActivePixFRLRatio * XV_HdmiRx1_Divide(InstancePtr->Config.FRLClkFreqkHz, 100);
					VidClk = XV_HdmiRx1_Divide(VidClk, TotalPixFRLRatio);
					InstancePtr->Stream.RefClk = (VidClk * 100000);

					if (XV_HdmiRx1_GetVideoProperties(InstancePtr) != XST_SUCCESS) {
#ifdef DEBUG_RX_FRL_VERBOSITY
						xil_printf(ANSI_COLOR_RED "XV_HdmiRx1_GetVideoProperties Failed\r\n" ANSI_COLOR_RESET);
#endif
					}
				}

				XV_HdmiRx1_SetPixelClk(InstancePtr);

				if (InstancePtr->Stream.IsFrl == TRUE) {
					VidClk = (InstancePtr->Stream.PixelClk / 100000) /
							InstancePtr->Stream.CorePixPerClk;
					VidClk = InstancePtr->Config.VideoClkFreqkHz / VidClk;
					Remainder = VidClk % 100;
					VidClk = VidClk / 100;
					if (Remainder >= 50) {
						VidClk++;
					}

					XV_HdmiRx1_SetFrlVClkVckeRatio(InstancePtr, VidClk);
				}

				/* If the colorspace is YUV420, then the
				 * frame rate must be doubled */
				if (InstancePtr->Stream.Video.ColorFormatId ==
						XVIDC_CSF_YCRCB_420) {
					/* Calculate and set the frame rate field */
					InstancePtr->Stream.Video.FrameRate =
					   (XVidC_FrameRate) (XV_HdmiRx1_Divide((InstancePtr->Stream.PixelClk << 1),
							(InstancePtr->Stream.Video.Timing.F0PVTotal *
									InstancePtr->Stream.Video.Timing.HTotal)));
				} else {
					/* Calculate and set the frame rate field */
					InstancePtr->Stream.Video.FrameRate =
					   (XVidC_FrameRate) (XV_HdmiRx1_Divide(InstancePtr->Stream.PixelClk,
							(InstancePtr->Stream.Video.Timing.F0PVTotal *
									InstancePtr->Stream.Video.Timing.HTotal)));
				}

				/* Lookup the video mode id */
				InstancePtr->Stream.Video.VmId =
				XVidC_GetVideoModeIdExtensive(&InstancePtr->Stream.Video.Timing,
						 InstancePtr->Stream.Video.FrameRate,
						 InstancePtr->Stream.Video.IsInterlaced,
						 (TRUE));

				if (InstancePtr->Stream.Vic != 0 && !VrrActive
						&& (FvaFactor < 2) && !DscEnabledStream) {
					DecodedVmId = XV_HdmiRx1_LookupVmId(InstancePtr->Stream.Vic);

					if (DecodedVmId != InstancePtr->Stream.Video.VmId) {
						/* Call VIC Error callback */
						if (InstancePtr->VicErrorCallback) {
							InstancePtr->VicErrorCallback(
									InstancePtr->VicErrorRef);
						}
					}
				}

				/*If video mode not found in the table tag it as custom */
				if (InstancePtr->Stream.Video.VmId ==
				    XVIDC_VM_NOT_SUPPORTED) {
					InstancePtr->Stream.Video.VmId = XVIDC_VM_CUSTOM;
				}

			        if (XVidC_IsStream3D(&InstancePtr->Stream.Video)){
			            XVidC_Set3DVideoStream(&InstancePtr->Stream.Video,
			                                   InstancePtr->Stream.Video.VmId,
			                                   InstancePtr->Stream.Video.ColorFormatId,
			                                   InstancePtr->Stream.Video.ColorDepth,
			                                   InstancePtr->Stream.Video.PixPerClk,
			                                   &InstancePtr->Stream.Video.Info_3D);
			        } else {
			            XVidC_SetVideoStream(&InstancePtr->Stream.Video,
			                                 InstancePtr->Stream.Video.VmId,
			                                 InstancePtr->Stream.Video.ColorFormatId,
			                                 InstancePtr->Stream.Video.ColorDepth,
			                                 InstancePtr->Stream.Video.PixPerClk);
			        }

				/* Video Interface can be only one of 0, 1 or 2 */
				Xil_AssertVoid(InstancePtr->SubsysVidIntfc == 0 ||
					       InstancePtr->SubsysVidIntfc == 1 ||
					       InstancePtr->SubsysVidIntfc == 2);

				if (InstancePtr->SubsysVidIntfc == 0 && !DscEnabledStream) {
					/* AXI4 Stream Interface */
					/* PPC can only be 4 or 8 */
					Xil_AssertVoid(InstancePtr->SubsysPpc == XVIDC_PPC_4 ||
						       InstancePtr->SubsysPpc == XVIDC_PPC_8);
					if (InstancePtr->SubsysPpc == XVIDC_PPC_4) {
						if (InstancePtr->Stream.Video.ColorFormatId != XVIDC_CSF_YCRCB_420) {
							/* for RGB/YUV444/YUV422 */
							if (InstancePtr->Stream.Video.Timing.HActive % 4)
								Error = TRUE;
						} else {
							/* for YUV420 */
							if (InstancePtr->Stream.Video.Timing.HActive % 8)
								Error = TRUE;
						}
					} else {
						/* 8PPC case for all color formats */
						if (InstancePtr->Stream.Video.Timing.HActive % 8)
							Error = TRUE;
					}
				} else if (InstancePtr->SubsysVidIntfc == 1) {
					/* Native Interface support only 4 ppc */
					Xil_AssertVoid(InstancePtr->SubsysPpc == XVIDC_PPC_4);

					if (InstancePtr->SubsysPpc == XVIDC_PPC_4) {
						if (InstancePtr->Stream.Video.ColorFormatId != XVIDC_CSF_YCRCB_420) {
							/* for RGB/YUV444/YUV422 */
							if ((InstancePtr->Stream.Video.Timing.HActive % 4) ||
							    (InstancePtr->Stream.Video.Timing.HSyncWidth % 4) ||
							    (InstancePtr->Stream.Video.Timing.HTotal % 4))
								Error = TRUE;
						} else {
							/* for YUV 420 */
							if ((InstancePtr->Stream.Video.Timing.HActive % 8) ||
							    (InstancePtr->Stream.Video.Timing.HTotal % 8))
								Error = TRUE;
						}
					}
				} else if (InstancePtr->SubsysVidIntfc == 2) {
					/* Native DE Interface support only 4 ppc */
					Xil_AssertVoid(InstancePtr->SubsysPpc == XVIDC_PPC_4);
				}

				if (Error) {
					if (!(InstancePtr->IsErrorPrintCount % 500)) {
						xil_printf(ANSI_COLOR_YELLOW "[ERROR] Resolution: %dx%d@%dHz,"\
							   "%s,%dbpc is not supported in %d Pixel per Clock Mode. \r\n"ANSI_COLOR_RESET,
							   InstancePtr->Stream.Video.Timing.HActive,
							   InstancePtr->Stream.Video.Timing.VActive,
							   InstancePtr->Stream.Video.FrameRate,
							   XVidC_GetColorFormatStr(InstancePtr->Stream.Video.ColorFormatId),
							   InstancePtr->Stream.Video.ColorDepth,
							   InstancePtr->Stream.Video.PixPerClk
							  );
						InstancePtr->IsErrorPrintCount = 0;
					}
					InstancePtr->IsErrorPrintCount++;
				} else {
					/* Enable AXI Stream output */
					XV_HdmiRx1_AxisEnable(InstancePtr, (TRUE));
					/* Set stream status to up */
					InstancePtr->Stream.State =
							XV_HDMIRX1_STATE_STREAM_UP;
					/* The stream is up */

					/* set VTEM is Received as TRUE */
					InstancePtr->IsFirstVtemReceived = TRUE ;

					/* Set stream sync status to est */
					InstancePtr->Stream.SyncStatus =
							XV_HDMIRX1_SYNCSTAT_SYNC_EST;

					/* Derive Base Timing */
					BaseTiming = &(InstancePtr->Stream.Video.BaseTiming);
					memcpy(BaseTiming,
							&(InstancePtr->Stream.Video.Timing),
							sizeof(XVidC_VideoTiming));
					InstancePtr->Stream.Video.BaseFrameRate =
						         InstancePtr->Stream.Video.FrameRate ;

					if (VrrActive | (FvaFactor >1)) {

						Vic = InstancePtr->Stream.Vic;
						if (Vic != 0) {
							DecodedVmId = XV_HdmiRx1_LookupVmId(Vic);
							VidEntry = XVidC_GetVideoModeData(DecodedVmId);
							BaseTiming->F0PVFrontPorch =
								VidEntry->Timing.F0PVFrontPorch;
							InstancePtr->Stream.Video.BaseFrameRate =
										VidEntry->FrameRate;

						} else if ((InstancePtr->VrrIF.VrrIfType == XV_HDMIC_VRRINFO_TYPE_VTEM) &&
									(InstancePtr->VrrIF.VidTimingExtMeta.VRREnabled) ){
							BaseTiming->F0PVFrontPorch =
								InstancePtr->VrrIF.VidTimingExtMeta.BaseVFront;
							InstancePtr->Stream.Video.BaseFrameRate =
								InstancePtr->VrrIF.VidTimingExtMeta.BaseRefreshRate;
						}
						BaseTiming->F0PVSyncWidth = BaseTiming->F0PVSyncWidth / FvaFactor ;
						BaseTiming->F0PVBackPorch = BaseTiming->F0PVBackPorch / FvaFactor ;

						BaseTiming->F0PVTotal = BaseTiming->VActive +
								BaseTiming->F0PVSyncWidth +
								BaseTiming->F0PVBackPorch +
								BaseTiming->F0PVFrontPorch;
					}


					/* Enable sync loss */
					/* XV_HdmiRx1_WriteReg(
					*	InstancePtr->Config.BaseAddress,
					*	(XV_HDMIRX1_VTD_CTRL_SET_OFFSET),
					*	(XV_HDMIRX1_VTD_CTRL_SYNC_LOSS_MASK));
					*/

					/* Enable the Dynamic HDR Datamover */
					XV_HdmiRx1_DynHDR_DM_Enable(InstancePtr);

					/* Call stream up callback */
					if (InstancePtr->StreamUpCallback) {
						InstancePtr->StreamUpCallback(
								InstancePtr->StreamUpRef);
					}
				}
			}
		}

		/* Check if we are in stream up state */
		else if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_STREAM_UP) {

			/* Read video timing */
			Status = XV_HdmiRx1_GetVideoTiming(InstancePtr);

			if (Status == XST_SUCCESS) {
				if (InstancePtr->Stream.SyncStatus ==
						XV_HDMIRX1_SYNCSTAT_SYNC_LOSS) {
					/* Sync Est/Recover Flag */
					InstancePtr->Stream.SyncStatus =
						XV_HDMIRX1_SYNCSTAT_SYNC_EST;

					/* Call sync lost callback */
					if (InstancePtr->SyncLossCallback) {
						InstancePtr->SyncLossCallback(
								InstancePtr->SyncLossRef);
					}
				}
			} else {
				/* Disable sync loss */
				/* XV_HdmiRx1_WriteReg(
				 *	InstancePtr->Config.BaseAddress,
				 *	(XV_HDMIRX1_VTD_CTRL_CLR_OFFSET),
				 *	(XV_HDMIRX1_VTD_CTRL_SYNC_LOSS_MASK));
				 */

#ifdef DEBUG_RX_FRL_VERBOSITY
				XVidC_ReportTiming(
					&InstancePtr->Stream.Video.Timing,
					InstancePtr->Stream.Video.IsInterlaced);
#endif

				if (InstancePtr->Stream.IsFrl != TRUE) {
					InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_LOCK;
				} else {
#ifdef DEBUG_RX_FRL_VERBOSITY
					xil_printf(ANSI_COLOR_YELLOW "===FRL "
						   "- VTD Stream Down===\r\n"
						   ANSI_COLOR_RESET);
#endif
					/* Toggle Internal Link Domain Reset */
					XV_HdmiRx1_INT_LRST(InstancePtr, TRUE);
					XV_HdmiRx1_INT_LRST(InstancePtr, FALSE);
					XV_HdmiRx1_AuxDisable(InstancePtr);
					/* Disable the Dynamic HDR Datamover */
					XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr);
					if (InstancePtr->StreamDownCallback) {
						InstancePtr->StreamDownCallback(
							InstancePtr->StreamDownRef);
					}

					/* Switch to bursty Vcke generation */
					XV_HdmiRx1_SetFrlVClkVckeRatio(InstancePtr, 0);

					InstancePtr->Stream.State =
						XV_HDMIRX1_STATE_STREAM_INIT;
				    XV_HdmiRx1_AuxEnable(InstancePtr);
					XV_HdmiRx1_Tmr1Start(InstancePtr,
							    TIME_200MS);
				}
			}
		}

	}

	/* Check for sync loss event */
	else if ((Status) & (XV_HDMIRX1_VTD_STA_SYNC_LOSS_EVT_MASK)) {

		/* Clear event flag */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_VTD_STA_OFFSET),
				    (XV_HDMIRX1_VTD_STA_SYNC_LOSS_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_STREAM_UP) {
			/* Enable the Stream Up + Sync Loss Flag */
			InstancePtr->Stream.SyncStatus =
					XV_HDMIRX1_SYNCSTAT_SYNC_LOSS;

			/* Call sync lost callback */
			if (InstancePtr->SyncLossCallback) {
				InstancePtr->SyncLossCallback(
						InstancePtr->SyncLossRef);
			}
		}
	}

	if ((Status) & (XV_HDMIRX1_VTD_STA_VFP_CH_EVT_MASK)) {
		/* Clear event flag */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_VTD_STA_OFFSET),
				    (XV_HDMIRX1_VTD_STA_VFP_CH_EVT_MASK));

		Status = XV_HdmiRx1_GetVideoTiming(InstancePtr);

		if (InstancePtr->VfpChangeCallback)
			InstancePtr->VfpChangeCallback(InstancePtr->VfpChangeRef);
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX DDC peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx1_DdcIntrHandler(XV_HdmiRx1 *InstancePtr)
{
	u32 Status;

	/* Read Status register */
	Status = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_STA_OFFSET));

	/* Check for HDCP write event */
	if ((Status) & (XV_HDMIRX1_DDC_STA_HDCP_WMSG_NEW_EVT_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_STA_OFFSET),
				    (XV_HDMIRX1_DDC_STA_HDCP_WMSG_NEW_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Callback */
		if (InstancePtr->HdcpCallback) {
			InstancePtr->HdcpCallback(InstancePtr->HdcpRef,
						  XV_HDMIRX1_DDC_STA_HDCP_WMSG_NEW_EVT_MASK);
		}
	}

	/* Check for HDCP read event */
	if ((Status) & (XV_HDMIRX1_DDC_STA_HDCP_RMSG_END_EVT_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_STA_OFFSET),
				    (XV_HDMIRX1_DDC_STA_HDCP_RMSG_END_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Callback */
		if (InstancePtr->HdcpCallback) {
			InstancePtr->HdcpCallback(InstancePtr->HdcpRef,
						  XV_HDMIRX1_DDC_STA_HDCP_RMSG_END_EVT_MASK);
		}
	}

	/* Check for HDCP read not complete event */
	if ((Status) & (XV_HDMIRX1_DDC_STA_HDCP_RMSG_NC_EVT_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_STA_OFFSET),
				    (XV_HDMIRX1_DDC_STA_HDCP_RMSG_NC_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Callback */
		if (InstancePtr->HdcpCallback) {
			InstancePtr->HdcpCallback(InstancePtr->HdcpRef,
						  XV_HDMIRX1_DDC_STA_HDCP_RMSG_NC_EVT_MASK);
		}
	}

	/* Check for HDCP 1.4 Aksv event */
	if ((Status) & (XV_HDMIRX1_DDC_STA_HDCP_AKSV_EVT_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_STA_OFFSET),
				    (XV_HDMIRX1_DDC_STA_HDCP_AKSV_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Callback */
		if (InstancePtr->HdcpCallback) {
			InstancePtr->HdcpCallback(InstancePtr->HdcpRef,
						  XV_HDMIRX1_DDC_STA_HDCP_AKSV_EVT_MASK);
		}
	}

	/* Check for HDCP 1.4 protocol event */
	if ((Status) & (XV_HDMIRX1_DDC_STA_HDCP_1_PROT_EVT_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_STA_OFFSET),
				    (XV_HDMIRX1_DDC_STA_HDCP_1_PROT_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Callback */
		if (InstancePtr->Hdcp14ProtEvtCallback) {
			InstancePtr->Hdcp14ProtEvtCallback(
					InstancePtr->Hdcp14ProtEvtRef);
		}
	}

	/* Check for HDCP 2.2 protocol event */
	if ((Status) & (XV_HDMIRX1_DDC_STA_HDCP_2_PROT_EVT_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_DDC_STA_OFFSET),
				    (XV_HDMIRX1_DDC_STA_HDCP_2_PROT_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Callback */
		if (InstancePtr->Hdcp22ProtEvtCallback) {
			InstancePtr->Hdcp22ProtEvtCallback(
					InstancePtr->Hdcp22ProtEvtRef);
		}
	}

	/*
	 * Check for interrupt generated when source sets
	 * SCDC 0x10 register bit 0 Status_Update
	 */
	if (Status & XV_HDMIRX1_DDC_STA_SCDC_DSC_STS_UPDT_EVT_MASK) {

		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    XV_HDMIRX1_DDC_STA_OFFSET,
				    XV_HDMIRX1_DDC_STA_SCDC_DSC_STS_UPDT_EVT_MASK);
		/*
		 * Clear the DSC Decode fail bit 7 in 0x40 SCDC register as per
		 * spec
		 */
		XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
					   XV_HDMIRX1_SCDCFIELD_DSC_DECODE_FAIL,
					   0);

		if (InstancePtr->DSCStsUpdtEvtCallback) {
			InstancePtr->DSCStsUpdtEvtCallback(
					InstancePtr->DSCStsUpdtEvtRef);
		}
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx1_PioIntrHandler(XV_HdmiRx1 *InstancePtr)
{
	u32 Event;
	u32 Data;

	/* Read PIO IN Event register.*/
	Event = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				   (XV_HDMIRX1_PIO_IN_EVT_OFFSET));

	/* Clear event flags */
	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMIRX1_PIO_IN_EVT_OFFSET), (Event));

	/* Read data */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  (XV_HDMIRX1_PIO_IN_OFFSET));

	/* Cable detect event has occurred*/
	if ((Event) & (XV_HDMIRX1_PIO_IN_DET_MASK)) {
		/* Cable is connected*/
		if ((Data) & (XV_HDMIRX1_PIO_IN_DET_MASK)) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("XV_HDMIRX1_PIO_IN_DET_MASK: "
				   "Cable Connect\r\n");
#endif
			/* Set connected flag*/
			InstancePtr->Stream.IsConnected = (TRUE);
			XV_HdmiRx1_FrlReset(InstancePtr, FALSE);

			/* Clears IsFrl and IsHdmi flags */
			InstancePtr->Stream.IsHdmi = FALSE;
			InstancePtr->Stream.IsFrl = FALSE;

			/* Check if user callback has been registered*/
			if (InstancePtr->ConnectCallback) {
				InstancePtr->ConnectCallback(
						InstancePtr->ConnectRef);
			}

			/* Check if user callback has been registered*/
			if (InstancePtr->TmdsConfigCallback) {
				InstancePtr->TmdsConfigCallback(
						InstancePtr->TmdsConfigRef);
			}
		}

		/* Cable is disconnected*/
		else {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("XV_HDMIRX1_PIO_IN_DET_MASK: "
				   "Cable Disconnect\r\n");
#endif
			/* Clear connected flag*/
			InstancePtr->Stream.IsConnected = (FALSE);

			/* Clear SCDC variables*/
			XV_HdmiRx1_DdcScdcClear(InstancePtr);
			XV_HdmiRx1_FrlReset(InstancePtr, TRUE);

			/* Disable the Dynamic HDR Datamover */
			XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr);

			/* Check if user callback has been registered*/
			if (InstancePtr->ConnectCallback) {
				InstancePtr->ConnectCallback(
						InstancePtr->ConnectRef);
			}
		}

	}

	/* Link ready event has occurred*/
	if ((Event) & (XV_HDMIRX1_PIO_IN_LNK_RDY_MASK)) {
		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			if ((Data) & (XV_HDMIRX1_PIO_IN_LNK_RDY_MASK)) {
				switch (InstancePtr->Stream.Frl.TrainingState) {
				case XV_HDMIRX1_FRLSTATE_LTS_3_RATE_CH:
					InstancePtr->Stream.Frl.TrainingState =
							XV_HDMIRX1_FRLSTATE_LTS_3_ARM_LNK_RDY;
					break;
				case XV_HDMIRX1_FRLSTATE_LTS_3_ARM_VID_RDY:
					InstancePtr->Stream.Frl.TrainingState =
							XV_HDMIRX1_FRLSTATE_LTS_3;
					XV_HdmiRx1_ExecFrlState(InstancePtr);
					break;
				default:
					InstancePtr->DBMessage =
							InstancePtr->Stream.Frl.TrainingState;

					/* Call Link Ready Error callback */
					if (InstancePtr->LnkRdyErrorCallback) {
						InstancePtr->LnkRdyErrorCallback(
								InstancePtr->LnkRdyErrorRef);
					}
#ifdef DEBUG_RX_FRL_VERBOSITY
					xil_printf(ANSI_COLOR_RED "LNK_RDY 1 Error! (%d)\r\n"
							ANSI_COLOR_RESET,
							InstancePtr->Stream.Frl.TrainingState);
#endif
					break;
				}
			} else {
#ifdef DEBUG_RX_FRL_VERBOSITY
				xil_printf("LNK_RDY:0\r\n");
#endif
				/* LNK_RDY goes down*/
			}
		} else if (InstancePtr->Stream.IsFrl == TRUE) {
			InstancePtr->DBMessage = 0x80;

			/* Call Link Ready Error callback */
			if (InstancePtr->LnkRdyErrorCallback) {
				InstancePtr->LnkRdyErrorCallback(
						InstancePtr->LnkRdyErrorRef);
			}
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf(ANSI_COLOR_RED "LNK_RDY during FRL Link!\r\n"
					ANSI_COLOR_RESET);
#endif
		} else {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf(ANSI_COLOR_RED "LNK_RDY TMDS!\r\n" ANSI_COLOR_RESET);
#endif

			/* Set stream status to idle*/
			InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_IDLE;

			/* Load timer*/
			XV_HdmiRx1_Tmr1Start(InstancePtr,
				XV_HdmiRx1_GetTime10Ms(InstancePtr)); /* 10 ms*/
		}
	}

	/* Video ready event has occurred*/
	if ((Event) & (XV_HDMIRX1_PIO_IN_VID_RDY_MASK)) {
		if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			if ((Data) & (XV_HDMIRX1_PIO_IN_VID_RDY_MASK)) {
				switch (InstancePtr->Stream.Frl.TrainingState) {
				case XV_HDMIRX1_FRLSTATE_LTS_3_RATE_CH:
					InstancePtr->Stream.Frl.TrainingState =
							XV_HDMIRX1_FRLSTATE_LTS_3_ARM_VID_RDY;
					break;
				case XV_HDMIRX1_FRLSTATE_LTS_3_ARM_LNK_RDY:
					InstancePtr->Stream.Frl.TrainingState =
							XV_HDMIRX1_FRLSTATE_LTS_3;
					XV_HdmiRx1_ExecFrlState(InstancePtr);
					break;
				default:
					InstancePtr->DBMessage =
							InstancePtr->Stream.Frl.TrainingState;

					/* Call Video Ready Error callback */
					if (InstancePtr->VidRdyErrorCallback) {
						InstancePtr->VidRdyErrorCallback(
								InstancePtr->VidRdyErrorRef);
					}
#ifdef DEBUG_RX_FRL_VERBOSITY
					xil_printf(ANSI_COLOR_RED "VID_RDY 1 Error! (%d)\r\n"
							ANSI_COLOR_RESET,
							InstancePtr->Stream.Frl.TrainingState);
#endif
					break;
				}
			} else {
#ifdef DEBUG_RX_FRL_VERBOSITY
				xil_printf("VID_RDY:0\r\n");
#endif
				/* VID_RDY goes down*/
			}

/*    		return;*/
		} else if (InstancePtr->Stream.IsFrl == TRUE) {
			InstancePtr->DBMessage = 0x80;

			/* Call Video Ready Error callback */
			if (InstancePtr->VidRdyErrorCallback) {
				InstancePtr->VidRdyErrorCallback(
						InstancePtr->VidRdyErrorRef);
			}
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf(ANSI_COLOR_RED "VID_RDY during FRL Link!\r\n"
					ANSI_COLOR_RESET);
#endif
/*    		return;*/
		} else {
			/* Ready*/
			if ((Data) & (XV_HDMIRX1_PIO_IN_VID_RDY_MASK)) {
				/* Check the previous state*/
				/* The link can only change to up when
				 * the previous state was init*/
				/* Else there was a glitch on the
				 * video ready input*/
				if (InstancePtr->Stream.State ==
				    XV_HDMIRX1_STATE_STREAM_INIT) {

					/* Toggle video reset for HDMI RX core */
					XV_HdmiRx1_INT_VRST(InstancePtr, TRUE);
					XV_HdmiRx1_INT_VRST(InstancePtr, FALSE);

					/* Toggle bridge reset */
					XV_HdmiRx1_EXT_VRST(InstancePtr, TRUE);
					XV_HdmiRx1_EXT_SYSRST(InstancePtr, TRUE);
					XV_HdmiRx1_EXT_VRST(InstancePtr, FALSE);
					XV_HdmiRx1_EXT_SYSRST(InstancePtr, FALSE);

				/* Set stream status to arm*/
					InstancePtr->Stream.State =
						XV_HDMIRX1_STATE_STREAM_ARM;

					/* Load timer - 200 ms (one UHD
					 * frame is 40 ms, 5 frames)*/
					XV_HdmiRx1_Tmr1Start(InstancePtr,
					XV_HdmiRx1_GetTime200Ms(InstancePtr));
				}
			}

			/* Stream down*/
			else {
				/* Assert HDMI RX core resets */
				XV_HdmiRx1_INT_VRST(InstancePtr, TRUE);
				XV_HdmiRx1_INT_LRST(InstancePtr, TRUE);

				/* Clear variables */
				XV_HdmiRx1_Clear(InstancePtr);

				/* Disable aux and audio peripheral*/
				/* At this state the link clock is not stable.*/
				/* Therefore these two peripheral are
				 * disabled to prevent any glitches.*/
				XV_HdmiRx1_AuxDisable(InstancePtr);
				XV_HdmiRx1_AudioDisable(InstancePtr);

				/* Disable VTD */
				XV_HdmiRx1_VtdDisable(InstancePtr);

				/* Disable link*/
				XV_HdmiRx1_LinkEnable(InstancePtr, (FALSE));

				/* Disable video*/
				/* MH AI: Don't reset bridge when clock is not stable*/
				XV_HdmiRx1_VideoEnable(InstancePtr, (TRUE));

				XV_HdmiRx1_AxisEnable(InstancePtr, (FALSE));

				/* Set stream status to down*/
				InstancePtr->Stream.State =
					XV_HDMIRX1_STATE_STREAM_DOWN;

				/* Disable sync loss*/
				XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
					(XV_HDMIRX1_VTD_CTRL_CLR_OFFSET),
					(XV_HDMIRX1_VTD_CTRL_SYNC_LOSS_MASK));

				/* Disable the Dynamic HDR Datamover */
				XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr);

				/* Call stream down callback*/
				if (InstancePtr->StreamDownCallback) {
					InstancePtr->StreamDownCallback(
							InstancePtr->StreamDownRef);
				}
			}
		}
	}

	/* SCDC Scrambler Enable*/
	if ((Event) & (XV_HDMIRX1_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK)) {

		/* Enable scrambler*/
		if ((Data) & (XV_HDMIRX1_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK)) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf(ANSI_COLOR_MAGENTA
				"XV_HDMIRX1_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK "
				"Enable\r\n" ANSI_COLOR_RESET);
#endif
			XV_HdmiRx1_SetScrambler(InstancePtr, (TRUE));
		}

		/* Disable scrambler*/
		else {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf(ANSI_COLOR_MAGENTA
				"XV_HDMIRX1_PIO_IN_SCDC_SCRAMBLER_ENABLE_MASK "
				"Disable\r\n" ANSI_COLOR_RESET);
#endif
			XV_HdmiRx1_SetScrambler(InstancePtr, (FALSE));
		}
	}

	/* Mode*/
	if (InstancePtr->Stream.State != XV_HDMIRX1_STATE_FRL_LINK_TRAINING &&
			(Event) & (XV_HDMIRX1_PIO_IN_MODE_MASK) &&
			InstancePtr->Stream.IsFrl != TRUE) {
		/* HDMI Mode*/
		if ((Data) & (XV_HDMIRX1_PIO_IN_MODE_MASK)) {
			InstancePtr->Stream.IsHdmi = TRUE;
		}

		/* DVI Mode*/
		else {
			InstancePtr->Stream.IsHdmi = FALSE;
		}

		if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_STREAM_UP ||
		    InstancePtr->Stream.State == XV_HDMIRX1_STATE_STREAM_LOCK) {
			/* Clear variables */
			XV_HdmiRx1_Clear(InstancePtr);

			/* Set stream status to idle*/
			InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_IDLE;

			/* Load timer*/
			XV_HdmiRx1_Tmr1Start(InstancePtr,
					XV_HdmiRx1_GetTime10Ms(InstancePtr)); /* 10 ms*/
		}

		/* Call mode callback*/
		if (InstancePtr->ModeCallback) {
			InstancePtr->ModeCallback(InstancePtr->ModeRef);
		}
	}

	/* TMDS clock ratio*/
	if ((Event) & (XV_HDMIRX1_PIO_IN_SCDC_TMDS_CLOCK_RATIO_MASK)) {
		/* Call TMDS Ratio callback*/
		if (InstancePtr->TmdsClkRatioCallback) {
			InstancePtr->TmdsClkRatioCallback(
					InstancePtr->TmdsClkRatioRef);
		}
	}

	/* Bridge Overflow event has occurred */
	if ((Event) & (XV_HDMIRX1_PIO_IN_BRDG_OVERFLOW_MASK)) {
		/* Check if user callback has been registered*/
		if (InstancePtr->BrdgOverflowCallback) {
			InstancePtr->BrdgOverflowCallback(
					InstancePtr->BrdgOverflowRef);
		}
	}

	/* DSC Stream Change Event */
	if (Event & XV_HDMIRX1_PIO_IN_DSC_EN_STRM_CHG_EVT_MASK) {

		u32 val = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					     XV_HDMIRX1_PIO_IN_OFFSET);

		if (!(XV_HDMIRX1_PIO_IN_DSC_EN_STRM_MASK & val)) {
			/*
			 * When the DSC CVT is not preset, clear bit 7 DSC
			 * Decode Fail in 0x40 SCDC register as per spec.
			 */
			XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
						    XV_HDMIRX1_SCDCFIELD_DSC_DECODE_FAIL,
						    0);
		}

		if (InstancePtr->DSCStreamChangeEventCallback) {
			InstancePtr->DSCStreamChangeEventCallback(
					InstancePtr->DSCStrmChgEvtRef);
		}
	}

	/* DSC PPS Packet Error event */
	if (Event & XV_HDMIRX1_PIO_IN_DSC_PPS_PKT_ERR_MASK) {
		if (InstancePtr->DSCPktErrCallback) {
			InstancePtr->DSCPktErrCallback(
					InstancePtr->DSCPktErrCallback);
		}
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX TMR peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx1_TmrIntrHandler(XV_HdmiRx1 *InstancePtr)
{
	u32 Status;

	/* Read Status register */
	Status = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_TMR_STA_OFFSET));

	/* Check for counter event */
	if ((Status) & (XV_HDMIRX1_TMR1_STA_CNT_EVT_MASK)) {

		/* Clear counter event*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_TMR_STA_OFFSET),
				    (XV_HDMIRX1_TMR1_STA_CNT_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			switch (InstancePtr->Stream.Frl.TrainingState) {
			case XV_HDMIRX1_FRLSTATE_LTS_L:
				XV_HdmiRx1_ExecFrlState(InstancePtr);
				break;
			case XV_HDMIRX1_FRLSTATE_LTS_P:
			case XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY:
			case XV_HDMIRX1_FRLSTATE_LTS_P_VID_RDY:
			case XV_HDMIRX1_FRLSTATE_LTS_3_RDY:
				break;
			default:
				InstancePtr->Stream.Frl.TrainingState =
						XV_HDMIRX1_FRLSTATE_LTS_3_TMR;
				XV_HdmiRx1_ExecFrlState(InstancePtr);
				break;
			}
			return;
		}

		/* Idle state*/
		if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_STREAM_IDLE) {
			if (InstancePtr->Stream.IsFrl != TRUE ||
			    (InstancePtr->Stream.IsFrl == TRUE &&
			     InstancePtr->Stream.Frl.TrainingState ==
					     XV_HDMIRX1_FRLSTATE_LTS_P_VID_RDY)) {
				/* The link is stable now*/
				/* Then the aux and audio peripherals can be enabled*/
				XV_HdmiRx1_AuxEnable(InstancePtr);
				XV_HdmiRx1_AudioEnable(InstancePtr);

				/* Release HDMI RX core resets */
				XV_HdmiRx1_INT_VRST(InstancePtr, FALSE);
				XV_HdmiRx1_INT_LRST(InstancePtr, FALSE);

				/* Enable link*/
				XV_HdmiRx1_LinkEnable(InstancePtr, (TRUE));

				/* Set stream status to init*/
				InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_INIT;

				/* Clear GetVideoPropertiesTries*/
				InstancePtr->Stream.GetVideoPropertiesTries = 0;
				/* xil_printf(ANSI_COLOR_RED "Skew Locked!\r\n" ANSI_COLOR_RESET); */
			} else {
				/*xil_printf(ANSI_COLOR_RED "Skew Unlocked!\r\n" ANSI_COLOR_RESET); */
			}

			/* Load timer - 200 ms (one UHD frame is 40 ms, 5 frames)*/
			XV_HdmiRx1_Tmr1Start(InstancePtr,
					XV_HdmiRx1_GetTime200Ms(InstancePtr));
		}

		/* Init state */
		else if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_STREAM_INIT) {

			/* Read video properties */
			if (XV_HdmiRx1_GetVideoProperties(InstancePtr) == XST_SUCCESS) {

				XV_HdmiRx1_SetPixelClk(InstancePtr);

				if (InstancePtr->Stream.IsFrl == TRUE) {
#ifdef DEBUG_RX_FRL_VERBOSITY
					xil_printf("Virtual Vid_Rdy:"
						"XV_HDMIRX1_STATE_STREAM_INIT\r\n");
#endif
					/* Toggle video reset for HDMI RX core */
					XV_HdmiRx1_INT_VRST(InstancePtr, TRUE);
					XV_HdmiRx1_INT_VRST(InstancePtr, FALSE);

					/* Toggle bridge reset */
					XV_HdmiRx1_EXT_VRST(InstancePtr, TRUE);
					XV_HdmiRx1_EXT_SYSRST(InstancePtr, TRUE);
					XV_HdmiRx1_EXT_VRST(InstancePtr, FALSE);
					XV_HdmiRx1_EXT_SYSRST(InstancePtr, FALSE);

					/* Set stream status to arm */
					InstancePtr->Stream.State =
						XV_HDMIRX1_STATE_STREAM_ARM;

					/* Load timer - 200 ms (one UHD frame
					 * is 40 ms, 5 frames) */
					XV_HdmiRx1_Tmr1Start(InstancePtr,
					XV_HdmiRx1_GetTime200Ms(InstancePtr));
				} else if (InstancePtr->StreamInitCallback) {
					/* Call stream init callback*/
					InstancePtr->StreamInitCallback(
							InstancePtr->StreamInitRef);
				}
			}

			else {
				/* Load timer - 200 ms (one UHD frame is 40 ms,
				 * 5 frames) */
				XV_HdmiRx1_Tmr1Start(InstancePtr,
					XV_HdmiRx1_GetTime200Ms(InstancePtr));
			}
		}

		/* Armed state*/
		else if (InstancePtr->Stream.State ==
				XV_HDMIRX1_STATE_STREAM_ARM) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("TMR: XV_HDMIRX1_STATE_STREAM_ARM\r\n");
#endif
			/* Enable VTD */
			XV_HdmiRx1_VtdEnable(InstancePtr);

			/* Enable interrupt */
			XV_HdmiRx1_VtdIntrEnable(InstancePtr);

			/* Set stream status to lock */
			InstancePtr->Stream.State =
					XV_HDMIRX1_STATE_STREAM_LOCK;
		}
	}

	if ((Status) & (XV_HDMIRX1_TMR2_STA_CNT_EVT_MASK)) {
		/* Clear counter event */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				(XV_HDMIRX1_TMR_STA_OFFSET),
				(XV_HDMIRX1_TMR2_STA_CNT_EVT_MASK));
	}

	if ((Status) & (XV_HDMIRX1_TMR3_STA_CNT_EVT_MASK)) {
		/* Clear counter event */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				(XV_HDMIRX1_TMR_STA_OFFSET),
				(XV_HDMIRX1_TMR3_STA_CNT_EVT_MASK));

		XV_HdmiRx1_PhyResetPoll(InstancePtr);
	}

	if ((Status) & (XV_HDMIRX1_TMR4_STA_CNT_EVT_MASK)) {
		/* Clear counter event*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				(XV_HDMIRX1_TMR_STA_OFFSET),
				(XV_HDMIRX1_TMR4_STA_CNT_EVT_MASK));

		/* Currently unused. */
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX AUX peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx1_AuxIntrHandler(XV_HdmiRx1 *InstancePtr)
{
	u32 Status;
	u8 Index;
	u8 CurFVAFactMinus1;

	/* Read Status register */
	Status = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUX_STA_OFFSET));

	/* Get Dynamic HDR packet and set next buffer address */
	if (Status & XV_HDMIRX1_AUX_STA_DYN_HDR_EVT_MASK) {
		/* Clear event flag */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUX_STA_OFFSET),
				    (XV_HDMIRX1_AUX_STA_DYN_HDR_EVT_MASK));

		if (InstancePtr->DynHdrCallback)
			InstancePtr->DynHdrCallback(InstancePtr->DynHdrRef);
	}

	/* Check for GCP colordepth event */
	if ((Status) & (XV_HDMIRX1_AUX_STA_GCP_CD_EVT_MASK)) {
		/* Clear event flag */
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUX_STA_OFFSET),
				    (XV_HDMIRX1_AUX_STA_GCP_CD_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		if ((Status) & (XV_HDMIRX1_AUX_STA_GCP_MASK)) {
			InstancePtr->Stream.Video.ColorDepth =
					XV_HdmiRx1_GetGcpColorDepth(InstancePtr);

			if (InstancePtr->Stream.IsFrl == TRUE) {
#ifdef DEBUG_RX_FRL_VERBOSITY
				xil_printf(ANSI_COLOR_YELLOW "===FRL - "
					"Mode Stream Down===\r\n" ANSI_COLOR_RESET);
#endif
			    XV_HdmiRx1_AuxDisable(InstancePtr);

			    /* Disable the Dynamic HDR Datamover */
			    XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr);

				if (InstancePtr->StreamDownCallback) {
					InstancePtr->StreamDownCallback(
							InstancePtr->StreamDownRef);
				}
			     XV_HdmiRx1_AuxEnable(InstancePtr);
				InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_INIT;
				XV_HdmiRx1_Tmr1Start(InstancePtr, TIME_200MS);
			}
		}
	}

	/* Check for new packet */
	if ((Status) & (XV_HDMIRX1_AUX_STA_NEW_MASK)) {
		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUX_STA_OFFSET),
				    (XV_HDMIRX1_AUX_STA_NEW_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Set HDMI flag in TMDS Mode */
		if (InstancePtr->Stream.IsFrl != TRUE) {
			InstancePtr->Stream.IsHdmi = TRUE;
		}

		/* Read header word and update AUX header field */
		InstancePtr->Aux.Header.Data =
			XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					   (XV_HDMIRX1_AUX_DAT_OFFSET));

		for (Index = 0x0; Index < 8; Index++) {
			/* Read data word and update AUX data field */
			InstancePtr->Aux.Data.Data[Index] =
				XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
						   (XV_HDMIRX1_AUX_DAT_OFFSET));
		}

		/* Callback */
		if (InstancePtr->AuxCallback) {
			InstancePtr->AuxCallback(InstancePtr->AuxRef);
		}
	}

	/* Link integrity check */
	if ((Status) & (XV_HDMIRX1_AUX_STA_ERR_MASK)) {
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUX_STA_OFFSET),
				    (XV_HDMIRX1_AUX_STA_ERR_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Callback */
		if (InstancePtr->LinkErrorCallback) {
			InstancePtr->LinkErrorCallback(InstancePtr->LinkErrorRef);
		}
	}

	if ((Status) & (XV_HDMIRX1_AUX_STA_FSYNC_CD_EVT_MASK)) {
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUX_STA_OFFSET),
				    (XV_HDMIRX1_AUX_STA_FSYNC_CD_EVT_MASK));

		InstancePtr->VrrIF.VrrIfType = XV_HDMIC_VRRINFO_TYPE_SPDIF;
		XV_HdmiRx1_ParseSrcProdDescInfoframe(InstancePtr);
		if (InstancePtr->VrrRdyCallback)
			InstancePtr->VrrRdyCallback(InstancePtr->VrrRdyRef);
	}

	if ((Status) & (XV_HDMIRX1_AUX_STA_VRR_CD_EVT_MASK)) {
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUX_STA_OFFSET),
				    (XV_HDMIRX1_AUX_STA_VRR_CD_EVT_MASK));

		InstancePtr->VrrIF.VrrIfType = XV_HDMIC_VRRINFO_TYPE_VTEM;
		CurFVAFactMinus1 =
			InstancePtr->VrrIF.VidTimingExtMeta.FVAFactorMinus1;
		XV_HdmiRx1_ParseVideoTimingExtMetaIF(InstancePtr);

		/*
		 * It's unexpected for FVA factor to change once set. So restart
		 * stream.
		 */
		if ((CurFVAFactMinus1 !=
			  InstancePtr->VrrIF.VidTimingExtMeta.FVAFactorMinus1) &&
				InstancePtr->IsFirstVtemReceived == TRUE
			) {
			InstancePtr->IsFirstVtemReceived = FALSE;
			if(InstancePtr->Stream.IsFrl == TRUE) {
				InstancePtr->Stream.State =
					XV_HDMIRX1_STATE_STREAM_INIT;
			     XV_HdmiRx1_AuxDisable(InstancePtr);

			     /* Disable the Dynamic HDR Datamover */
			     XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr);

			     if (InstancePtr->StreamDownCallback) {
				    InstancePtr->StreamDownCallback(
						InstancePtr->StreamDownRef);
			     }
			     XV_HdmiRx1_AuxEnable(InstancePtr);

				 XV_HdmiRx1_Tmr1Start(InstancePtr, TIME_200MS);
			} else {

				InstancePtr->Stream.State =
						XV_HDMIRX1_STATE_STREAM_LOCK;
			     XV_HdmiRx1_AuxDisable(InstancePtr);

			     XV_HdmiRx1_AuxEnable(InstancePtr);
			}

		} else {
		   InstancePtr->IsFirstVtemReceived = TRUE ;
		}

		if (InstancePtr->VrrRdyCallback)
			InstancePtr->VrrRdyCallback(InstancePtr->VrrRdyRef);
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX AUD peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx1_AudIntrHandler(XV_HdmiRx1 *InstancePtr)
{
	u32 Status;

	/* Read Status register*/
	Status = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUD_STA_OFFSET));

	/* Check for active stream event*/
	if ((Status) & (XV_HDMIRX1_AUD_STA_ACT_EVT_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUD_STA_OFFSET),
				    (XV_HDMIRX1_AUD_STA_ACT_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		InstancePtr->Stream.Audio.Active = (TRUE);
	}

	/* Check for audio channel event*/
	if ((Status) & (XV_HDMIRX1_AUD_STA_CH_EVT_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUD_STA_OFFSET),
				    (XV_HDMIRX1_AUD_STA_CH_EVT_MASK));

		if (InstancePtr->Stream.State ==
		    XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
			return;
		}

		/* Audio Format*/
		InstancePtr->AudFormat = (XV_HdmiRx1_AudioFormatType)
				((Status >> XV_HDMIRX1_AUD_STA_AUD_FMT_SHIFT) &
					XV_HDMIRX1_AUD_STA_AUD_FMT_MASK);

		/* Parsing only for 3D audio */
		if (InstancePtr->AudFormat == 0x3) {
			switch ((Status >>
				XV_HDMIRX1_AUD_STA_3DAUD_CH_SHIFT) &
					XV_HDMIRX1_AUD_STA_3DAUD_CH_MASK) {
			case 6:
				InstancePtr->Stream.Audio.Channels = 32;
				break;
			case 4:
				InstancePtr->Stream.Audio.Channels = 24;
				break;
			case 0:
				InstancePtr->Stream.Audio.Channels = 12;
				break;
			default:
				break;
			}
		} else {
			/* Active channels*/
			switch ((Status >>
					XV_HDMIRX1_AUD_STA_AUD_CH_SHIFT) &
				        XV_HDMIRX1_AUD_STA_AUD_CH_MASK) {

			/* 8 channels*/
			case 3 :
				InstancePtr->Stream.Audio.Channels = 8;
				break;

			/* 6 channels*/
			case 2 :
				InstancePtr->Stream.Audio.Channels = 6;
				break;

			/* 4 channels*/
			case 1 :
				InstancePtr->Stream.Audio.Channels = 4;
				break;

			/* 2 channels*/
			default :
				InstancePtr->Stream.Audio.Channels = 2;
				break;
			}
		}

		/* Callback */
		if (InstancePtr->AudCallback) {
			InstancePtr->AudCallback(InstancePtr->AudRef);
		}
	}

	/* Check for ACR Update Event*/
	if ((Status) & (XV_HDMIRX1_AUD_STA_ACR_UPD_MASK)) {

		/* Clear event flag*/
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_AUD_STA_OFFSET),
				    (XV_HDMIRX1_AUD_STA_ACR_UPD_MASK));

		/* Placeholder: Add ACR Callback handlings */
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDMI RX Link Status
* (LNKSTA) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx1_LinkStatusIntrHandler(XV_HdmiRx1 *InstancePtr)
{
	if (InstancePtr->Stream.State == XV_HDMIRX1_STATE_FRL_LINK_TRAINING) {
		return;
	}

	/* Callback */
	if (InstancePtr->LnkStaCallback) {
		InstancePtr->LnkStaCallback(InstancePtr->LnkStaRef);
	}
}

/*****************************************************************************/
/**
*
* This function is the HDMI RX FRL peripheral interrupt handler.
*
* This handler reads corresponding event interrupt from the FRL Status
* register.
*
* @param    InstancePtr is a pointer to the HDMI RX core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void HdmiRx1_FrlIntrHandler(XV_HdmiRx1 *InstancePtr)
{
	u32 Data;
	u8 StreamDownFlag = FALSE;

	/* Read FRL Status register */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
		                    (XV_HDMIRX1_FRL_STA_OFFSET));

	/* FRL_Rate change event has occurred */
	if ((Data) & (XV_HDMIRX1_FRL_STA_RATE_EVT_MASK)) {
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
					(XV_HDMIRX1_FRL_STA_OFFSET),
					XV_HDMIRX1_FRL_STA_RATE_EVT_MASK);

		/* Disable the Dynamic HDR Datamover */
		XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr);

		if (InstancePtr->StreamDownCallback) {
			InstancePtr->StreamDownCallback(InstancePtr->StreamDownRef);
		}

		InstancePtr->Stream.Frl.TrainingState =
					XV_HDMIRX1_FRLSTATE_LTS_3_RATE_CH;
		XV_HdmiRx1_ExecFrlState(InstancePtr);
/*        XV_HdmiRx1_SetFrl10MicroSecondsTimer(InstancePtr);*/
	}

	/* Source has cleared FLT_update flag */
	if ((Data) & (XV_HDMIRX1_FRL_STA_FLT_UPD_EVT_MASK)) {
		 XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				     (XV_HDMIRX1_FRL_STA_OFFSET),
				     XV_HDMIRX1_FRL_STA_FLT_UPD_EVT_MASK);
		 InstancePtr->Stream.Frl.FltUpdateAsserted = FALSE;
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_YELLOW "RX: INTR FLT_UP Cleared (%d)\r\n"
			ANSI_COLOR_RESET, XV_HdmiRx1_GetTmr1Value(InstancePtr));
#endif
		switch (InstancePtr->Stream.Frl.TrainingState) {
		case XV_HDMIRX1_FRLSTATE_LTS_P:
		case XV_HDMIRX1_FRLSTATE_LTS_3_RDY:
		case XV_HDMIRX1_FRLSTATE_LTS_3_ARM_VID_RDY:
		case XV_HDMIRX1_FRLSTATE_LTS_3_ARM_LNK_RDY:
		case XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY:
			break;
		default:
			InstancePtr->Stream.Frl.TrainingState =
					XV_HDMIRX1_FRLSTATE_LTS_3;
			break;
		}
		XV_HdmiRx1_ExecFrlState(InstancePtr);
	}

	/* Link training patterns has matched for all the active lanes */
	if ((Data) & (XV_HDMIRX1_FRL_STA_FLT_PM_EVT_MASK)) {
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
					(XV_HDMIRX1_FRL_STA_OFFSET),
					XV_HDMIRX1_FRL_STA_FLT_PM_EVT_MASK);
#ifdef DEBUG_RX_FRL_VERBOSITY
xil_printf(ANSI_COLOR_YELLOW "RX: INTR LTP_DET\r\n" ANSI_COLOR_RESET);
#endif
		if ((InstancePtr->Stream.Frl.TrainingState ==
		     XV_HDMIRX1_FRLSTATE_LTS_3) ||
		    (InstancePtr->Stream.Frl.TrainingState ==
		     XV_HDMIRX1_FRLSTATE_LTS_3_LTP_DET)) {

			InstancePtr->Stream.Frl.TrainingState =
					XV_HDMIRX1_FRLSTATE_LTS_3_LTP_DET;

			XV_HdmiRx1_ExecFrlState(InstancePtr);
/*			XV_HdmiRx1_SetFrl10MicroSecondsTimer(InstancePtr);*/
		}
	}

	/* FRL Start Conditions are met */
	if ((Data) & (XV_HDMIRX1_FRL_STA_LANE_LOCK_EVT_MASK)) {
		u8 Temp = XV_HdmiRx1_FrlDdcReadField(InstancePtr,
						     XV_HDMIRX1_SCDCFIELD_LNS_LOCK);
		 XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				     (XV_HDMIRX1_FRL_STA_OFFSET),
				     XV_HDMIRX1_FRL_STA_LANE_LOCK_EVT_MASK);

		 if (((InstancePtr->Stream.Frl.Lanes == 3 ? 0x7 : 0xF) ==
		      Temp) &&
		     (InstancePtr->Stream.Frl.TrainingState ==
		      XV_HDMIRX1_FRLSTATE_LTS_P)) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			 xil_printf("->LTS_P_FRL_RDY\r\n");
#endif

			 InstancePtr->Stream.Frl.TrainingState =
					XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY;
#ifdef DEBUG_RX_FRL_VERBOSITY
			 xil_printf(ANSI_COLOR_YELLOW "RX: INTR "
				    "FRL_START\r\n" ANSI_COLOR_RESET);
#endif
			 XV_HdmiRx1_ExecFrlState(InstancePtr);
/*			 XV_HdmiRx1_SetFrl10MicroSecondsTimer(InstancePtr);*/
		}
	}

	if ((Data) & (XV_HDMIRX1_FRL_STA_SKEW_LOCK_EVT_MASK)) {
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_FRL_STA_OFFSET),
				    XV_HDMIRX1_FRL_STA_SKEW_LOCK_EVT_MASK);
		if (XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				       (XV_HDMIRX1_FRL_STA_OFFSET)) &
		    XV_HDMIRX1_FRL_STA_SKEW_LOCK_MASK) {
			if (InstancePtr->Stream.Frl.TrainingState ==
					XV_HDMIRX1_FRLSTATE_LTS_P_VID_RDY) {
				/* Error: Skew locked again from an already
				 * locked state */
				StreamDownFlag = TRUE;
				InstancePtr->DBMessage =
						InstancePtr->Stream.Frl.TrainingState;

				/* Call Skew Lock Error callback */
				if (InstancePtr->SkewLockErrorCallback) {
					InstancePtr->SkewLockErrorCallback(
							InstancePtr->SkewLockErrorRef);
				}
			} else {
				/* Placeholder: skew locked, no actions
                                 * needed */
			}

			InstancePtr->Stream.Frl.TrainingState =
					XV_HDMIRX1_FRLSTATE_LTS_P_VID_RDY;
		} else {
			if (InstancePtr->Stream.Frl.TrainingState ==
					XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY) {
				/* Skew unlocked */
				StreamDownFlag = TRUE;
			} else if (InstancePtr->Stream.Frl.TrainingState !=
					XV_HDMIRX1_FRLSTATE_LTS_3_RATE_CH) {
				/* Unexpected skew unlock event is true only
				 * when it is not caused by rate change
				 * request. */
				InstancePtr->DBMessage =
						InstancePtr->Stream.Frl.TrainingState;

				/* Call Skew Lock Error callback */
				if (InstancePtr->SkewLockErrorCallback) {
					InstancePtr->SkewLockErrorCallback(
							InstancePtr->SkewLockErrorRef);
				}
			}

			if (InstancePtr->Stream.Frl.TrainingState ==
					XV_HDMIRX1_FRLSTATE_LTS_P_VID_RDY) {
				InstancePtr->Stream.Frl.TrainingState =
						XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY;
			}
		}

		if (StreamDownFlag == TRUE) {
			XV_HdmiRx1_INT_LRST(InstancePtr, TRUE);
			XV_HdmiRx1_INT_VRST(InstancePtr, TRUE);
			XV_HdmiRx1_EXT_VRST(InstancePtr, TRUE);
			XV_HdmiRx1_EXT_SYSRST(InstancePtr, TRUE);
			/* Disable VTD */
			XV_HdmiRx1_VtdDisable(InstancePtr);

			/* Disable the Dynamic HDR Datamover */
			XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr);

			if (InstancePtr->StreamDownCallback) {
				InstancePtr->StreamDownCallback(InstancePtr->StreamDownRef);
			}
		}

		switch (InstancePtr->Stream.Frl.TrainingState) {
		case XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY:
			InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_DOWN;
			break;
		case XV_HDMIRX1_FRLSTATE_LTS_P_VID_RDY:
			/* Set stream status to idle*/
			InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_IDLE;

			/* Load timer*/
			XV_HdmiRx1_Tmr1Start(InstancePtr,
					XV_HdmiRx1_GetTime10Ms(InstancePtr)); /* 10 ms*/
			break;
		default:
			break;
		}
	}
}
