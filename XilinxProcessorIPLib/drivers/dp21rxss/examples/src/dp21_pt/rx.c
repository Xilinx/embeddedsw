/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file rx.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
*  1.6  GM  02/18/26 Initial release.
*
* </pre>
*
******************************************************************************/

#include "main.h"
#include "rx.h"

//#define DEBUG

XilAudioInfoFrame_rx AudioinfoFrame;
#ifdef Rxo
XilAudioExtFrame  SdpExtFrame;
XilAudioExtFrame  SdpExtFrame_q;
extern u32 infofifo[64]; /**< RX and TX can store up to 4 infoframes each. fifo of 8 */
extern u8 endindex;
extern u8 fifocount;
u32 hdrframe[9];
u16 fifoOverflow=0;
void Dp21RxSs_Pt_RxSetEDID (u8 edid_monitor[384]);
extern u8 tx_pass;
extern u8 startindex;

extern XDpRxSs DpRxSsInst;    		/**< The DPRX Subsystem instance. */
extern XDpTxSs DpTxSsInst; 		/**< The DPTX Subsystem instance. */
volatile u8 rx_unplugged = 0;
extern u32 vblank_init;
extern u8 vblank_captured;
extern volatile u8 rx_trained;
extern u8 tx_after_rx;
extern u8 Video_valid;
extern u8 tx_done;
extern void Dp21RxSs_Pt_TxIntrMask (void);
extern Video_CRC_Config VidFrameCRC_rx;	/**< Video Frame CRC instance */
extern XTmrCtr TmrCtr; 			/**< Timer instance. */
DP_Rx_Training_Algo_Config RxTrainConfig;
extern XVphy VPhyInst;			/**< The DPRX Subsystem instance. */

u32 hdrframe[9];
extern XVphy_PllType VPHY_RX_PLL_TYPE;
extern XVphy_ChannelId VPHY_RX_CHANNEL_TYPE;

#if ENABLE_HDCP_IN_DESIGN
extern u8 hdcp_capable_org ;
extern u8 hdcp_capable ;
extern u8 hdcp_repeater_org ;
extern u8 hdcp_repeater ;
extern u8 internal_rx_tx ;
#endif

/*****************************************************************************/
/**
*
* This function configures DisplayPort RX Subsystem.
*
* @param    None.
*
* @return
*        - XST_SUCCESS if DP RX Subsystem configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note        None.
*
******************************************************************************/
u32 Dp21RxSs_Pt_RxSetup(void)
{
	u32 ReadVal;

	ReadVal= XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0xD0);
	ReadVal = ReadVal & (0xFFFFFFFE);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0xD0, ReadVal);

	/*
	 * Disable RX.
	 */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x0);

	/*
	 * Disable All Interrupts
	 */
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);
#if ENABLE_HDCP_IN_DESIGN
	Dp21RxSs_Pt_Hdcp1xExamplePoll();
#else
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_INTERRUPT_MASK, 0xFFF87FFD);
#endif

	/*
	 * Enable Training related interrupts
	 */
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_TP1_MASK |
			      XDP_RX_INTERRUPT_MASK_TP2_MASK |
			      XDP_RX_INTERRUPT_MASK_TP3_MASK |
			      XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK |
			      XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK |
			      XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK |
			      XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK |
			      XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK |
			      XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK |
			      XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK);
#if (ENABLE_HDCP_RX)
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_HDCP_DEBUG_WRITE_MASK |
			      XDP_RX_INTERRUPT_MASK_HDCP_AKSV_WRITE_MASK |
			      XDP_RX_INTERRUPT_MASK_HDCP_AN_WRITE_MASK |
			      XDP_RX_INTERRUPT_MASK_HDCP_AINFO_WRITE_MASK |
			      XDP_RX_INTERRUPT_MASK_HDCP_RO_READ_MASK |
			      XDP_RX_INTERRUPT_MASK_HDCP_BINFO_READ_MASK);
#endif
	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			       XDP_RX_INTERRUPT_MASK_TP4_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);
	Dp21RxSs_Pt_Hdcp1xExamplePoll();

	/*
	 * Setting AUX Defer Count of Link Status Reads to 8 during Link
	 * Training 8 Defer counts is chosen to handle worst case time
	 * interrupt service load (PL system working at 100 MHz) when
	 * working with R5.
	 */
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_AUX_CLK_DIVIDER);
	ReadVal = ReadVal & 0xF0FF00FF;
	ReadVal = ReadVal | (AUX_DEFER_COUNT<<24);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_AUX_CLK_DIVIDER,
		     ReadVal);

	/*
	 * Setting BS Idle timeout value to long value
	 */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_BS_IDLE_TIME, DP_BS_IDLE_TIMEOUT);

	if (LINK_TRAINING_DEBUG==1) {
		/*
		 * Updating Vswing Iteration Count
		 */
		RxTrainConfig.ChEqOption = 0;
		RxTrainConfig.ClockRecoveryOption = 1;
		RxTrainConfig.Itr1Premp = 0;
		RxTrainConfig.Itr2Premp = 0;
		RxTrainConfig.Itr3Premp = 0;
		RxTrainConfig.Itr4Premp = 0;
		RxTrainConfig.Itr5Premp = 0;
		RxTrainConfig.MinVoltageSwing = 1;
		RxTrainConfig.SetPreemp = 1;
		RxTrainConfig.SetVswing = 0;
		RxTrainConfig.VswingLoopCount = 3;

		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MIN_VOLTAGE_SWING,
			(RxTrainConfig.MinVoltageSwing) |
			(RxTrainConfig.ClockRecoveryOption << 2) |
			(RxTrainConfig.VswingLoopCount << 4) |
			(RxTrainConfig.SetVswing << 8) |
			(RxTrainConfig.ChEqOption << 10) |
			(RxTrainConfig.SetPreemp << 12) |
			(RxTrainConfig.Itr1Premp << 14) |
			(RxTrainConfig.Itr2Premp << 16) |
			(RxTrainConfig.Itr3Premp << 18) |
			(RxTrainConfig.Itr4Premp << 20) |
			(RxTrainConfig.Itr5Premp << 22)
		);
	}

	/*
	 * Enable CRC Support
	 */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
		     VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5);

       /*
	* Disabling timeout
	*/
        ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_CDR_CONTROL_CONFIG);

        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                     XDP_RX_CDR_CONTROL_CONFIG,
                     ReadVal |
                     XDP_RX_CDR_CONTROL_CONFIG_DISABLE_TIMEOUT);

        /*
	 * Setting 8B10 Mode for backward compatibility
	 */
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x1);
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1600, 0x1);
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_OVER_CTRL_DPCD, 0x0);

	/*
	 * Enable Rx
	 */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_LINK_ENABLE, 0x1);

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
* @param    None
*
* @return
*        - XST_SUCCESS if interrupt setup was successful.
*        - A specific error code defined in "xstatus.h" if an error
*        occurs.
*
* @note        None.
*
******************************************************************************/
u32 Dp21RxSs_Pt_RxSetupIntrSystem(void)
{
	/* Set callbacks for all the interrupts */
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_DRV_HANDLER_DP_VID_EVENT,
			    &Dp21RxSs_Pt_RxVideoValidHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,
			    &Dp21RxSs_Pt_RxPwrChgHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_EVENT,
			    &Dp21RxSs_Pt_RxNoVideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VM_CHG_EVENT,
			    &Dp21RxSs_Pt_RxVmChgHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_EVENT,
			    &Dp21RxSs_Pt_RxVerticalBlankHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TLOST_EVENT,
			    &Dp21RxSs_Pt_RxTrainingLostHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_EVENT,
			    &Dp21RxSs_Pt_RxVideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
			    &Dp21RxSs_Pt_RxTrainingDoneHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_UNPLUG_EVENT,
			    &Dp21RxSs_Pt_RxUnplugHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_LINKBW_EVENT,
			    &Dp21RxSs_Pt_RxLinkBandwidthHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_PLL_RESET_EVENT,
			    &Dp21RxSs_Pt_RxPllResetHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_BW_CHG_EVENT,
			    &Dp21RxSs_Pt_RxBWChgHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LINK_QUAL_EVENT,
			    &Dp21RxSs_Pt_RxAccessLinkQualHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst,
			    XDPRXSS_HANDLER_ACCESS_ERROR_COUNTER_EVENT,
			    &Dp21RxSs_Pt_RxAccessErrorCounterHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,
			    &Dp21RxSs_Pt_RxCRCTestEventHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
			    &Dp21RxSs_Pt_RxInfoPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
			    &Dp21RxSs_Pt_RxExtPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,
			    &Dp21RxSs_Pt_RxIntrHandlerDownReq, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REP_EVENT,
			    &Dp21RxSs_Pt_RxIntrHandlerDownReply, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,
			    &Dp21RxSs_Pt_RxIntrHandlerPayloadAlloc, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ACT_RX_EVENT,
			    &Dp21RxSs_Pt_RxIntrHandlerActRx, &DpRxSsInst);

#if ((XPAR_DPRXSS_0_HDCP_ENABLE > 0) && ENABLE_HDCP_IN_DESIGN)
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_HDCP_AUTHENTICATED,
			    &Dp21RxSs_Pt_RxHdcpAuthCallback, &DpRxSsInst);
#endif
#if ((XPAR_DPRXSS_0_HDCP22_ENABLE > 0) && ENABLE_HDCP_IN_DESIGN)
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_HDCP22_AUTHENTICATED,
			    &Dp21RxSs_Pt_RxHdcpAuthCallback, &DpRxSsInst);
#endif
	/*
	 * Set custom timer wait
	 */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &Dp21RxSs_Pt_CustomWaitUs, &TmrCtr);

	return (XST_SUCCESS);
}

void Dp21RxSs_Pt_RxIntrHandlerPayloadAlloc(void *InstancePtr)
{
	/*
	 * Virtual Channel Payload allocation,
	 * de-allocation and partial deletion handler
	 */
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
	XDp *DpPtr = DpRxSsPtr->DpPtr;
	(void)XDp_ReadReg(DpPtr->Config.BaseAddr, XDP_RX_MST_ALLOC);
	XDp_RxAllocatePayloadStream(DpPtr);
}

void Dp21RxSs_Pt_RxIntrHandlerActRx(void *InstancePtr)
{
	(void)InstancePtr;
        /*
	 * ACT Receive Interrupt Handler
	 */
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			       0x3FFFF);
}

void Dp21RxSs_Pt_RxIntrHandlerDownReq(void *InstancePtr)
{
	(void)InstancePtr;
	/*
	 * Down Request Buffer Ready handler
	 * (Indicates the availability of the Down request)
	 */
	XDp_RxHandleDownReq(DpRxSsInst.DpPtr);
}

void Dp21RxSs_Pt_RxIntrHandlerDownReply(void *InstancePtr)
{
	(void)InstancePtr;
	/*
	 * Down Reply Buffer Read handler (indicates a
	 * read event from down reply buffer by upstream source)
	 *
	 * Increment the DownRequest Counter (if any)
	 */
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the power state interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxPwrChgHandler(void *InstancePtr)
{
	(void)InstancePtr;

}

/*****************************************************************************/
/**
*
* This function is the callback function for when the power state interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxVideoValidHandler(void *InstancePtr)
{
	(void)InstancePtr;
#ifdef DEBUG
	xil_printf("Dp21RxSs_Pt_RxVideoValidHandler\r\n");
	Video_valid = 1;
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_VIDEO_MASK);
#endif
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a video mode change
* interrupt occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxVmChgHandler(void *InstancePtr)
{
	(void)InstancePtr;

}

/*****************************************************************************/
/**
*
* This function is the callback function for when a no video interrupt occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxNoVideoHandler(void *InstancePtr)
{
	(void)InstancePtr;
	#ifdef DEBUG
	xil_printf("Dp21RxSs_Pt_RxNoVideoHandler\r\n");
	#endif
	Video_valid = 0;
	DpRxSsInst.VBlankCount = 0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);

	/* Reset CRC Test Counter in DP DPCD Space */
	XVidFrameCrc_Reset(&VidFrameCRC_rx);
	VidFrameCRC_rx.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			 XDP_RX_CRC_CONFIG,
			 (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
					 VidFrameCRC_rx.TEST_CRC_CNT));

	DpRxSsInst.no_video_trigger = 1;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_VIDEO_MASK);
	tx_done=0;
	AudioinfoFrame.frame_count=0;
	AudioinfoFrame.all_count=0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_EXT_PKT_MASK);

}

/*****************************************************************************/
/**
*
* This function is the callback function for when a vertical blank interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxVerticalBlankHandler(void *InstancePtr)
{
	(void)InstancePtr;
	DpRxSsInst.VBlankCount++;

#if ENABLE_HDCP_IN_DESIGN
	if (DpRxSsInst.VBlankCount > 200) {
		/*
		 * When Vblank is received, HDCP is put in enabled state and the
		 * timer is started. TX is not setup until the timer is done.
		 * This ensures that certain sources like MacBook gets
		 * time to Authenticate.
		 */
#if (ENABLE_HDCP_IN_DESIGN && (ENABLE_HDCP_RX))
		XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x01F80000);
		XDpRxSs_StartTimer(&DpRxSsInst);

#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
		 Dp21RxSs_Pt_Hdcp1xExamplePoll();
#endif
#endif
	} /**< End of (vblank_count > 200) */
	else if (DpRxSsInst.VBlankCount == 80)
	{

#if ENABLE_HDCP_IN_DESIGN
		XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x01F80000);
#if (ENABLE_HDCP1x_IN_RX && (ENABLE_HDCP_TX))
	    XDpRxSs_SetPhysicalState(&DpRxSsInst, hdcp_capable_org);
#else
#if ENABLE_HDCP1x_IN_RX
		XDpRxSs_SetPhysicalState(&DpRxSsInst, 1);
#endif
#endif
		Dp21RxSs_Pt_Hdcp1xExamplePoll();

#if ENABLE_HDCP1x_IN_TX
		XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
#endif

		Dp21RxSs_Pt_Hdcp1xExamplePoll();
		XDpRxSs_HdcpEnable(&DpRxSsInst);
		Dp21RxSs_Pt_Hdcp1xExamplePoll();
#endif
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a training lost interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/

void Dp21RxSs_Pt_RxTrainingLostHandler(void *InstancePtr)
{
	(void)InstancePtr;
#if ENABLE_HDCP1x_IN_RX
	XDpRxSs_SetPhysicalState(&DpRxSsInst, FALSE);
#endif
#if ENABLE_HDCP1x_IN_TX
	XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
#endif
	Dp21RxSs_Pt_Hdcp1xExamplePoll();

#if ENABLE_HDCP1x_IN_TX
	XDpTxSs_SetPhysicalState(&DpTxSsInst, FALSE);
#endif
	Dp21RxSs_Pt_Hdcp1xExamplePoll();
#if (ENABLE_HDCP_RX)
	XDpRxSs_StopTimer(&DpRxSsInst);

	/*
	 * This function will over write timer function pointer to be the right one.
	 */
	Dp21RxSs_Pt_RxHdcpUnAuthCallback((void *)&DpRxSsInst); 	/**< Added from 16.4 release */
#endif

	Video_valid = 0;

	/*
	 * Reset CRC Test Counter in DP DPCD Space
	 */
	XVidFrameCrc_Reset(&VidFrameCRC_rx);
	VidFrameCRC_rx.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_CRC_CONFIG,
		     (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
		      VidFrameCRC_rx.TEST_CRC_CNT));

	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
	XDpRxSs_AudioDisable(&DpRxSsInst);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
#ifdef Txo
	/*
	 * Mask the TX interrupts to avoid spurious HPDs
	 */
	Dp21RxSs_Pt_TxIntrMask();
#endif
	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.VBlankCount = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
	tx_done = 0;
	tx_after_rx = 0;
	AudioinfoFrame.frame_count = 0;
	AudioinfoFrame.all_count = 0;

	if (rx_trained == 1) {
		xil_printf ("Training Lost !!\r\n");
	}
	rx_trained = 0;
	vblank_captured = 0;
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
void Dp21RxSs_Pt_RxVideoHandler(void *InstancePtr)
{
	(void)InstancePtr;
#ifdef DEBUG
	xil_printf("Dp21RxSs_Pt_RxVideoHandler\r\n");
#endif
	Video_valid = 1;
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
			       XDP_RX_INTERRUPT_MASK_VIDEO_MASK);
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
void Dp21RxSs_Pt_RxTrainingDoneHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u32 ReadVal;

#if (ENABLE_HDCP_RX)
	XDpRxSs_SetLane(&DpRxSsInst, DpRxSsInst.UsrOpt.LaneCount);
#endif

#if ENABLE_HDCP1x_IN_RX
	XDpRxSs_SetPhysicalState(&DpRxSsInst, 0);
	Dp21RxSs_Pt_Hdcp1xExamplePoll();
	XDpRxSs_SetPhysicalState(&DpRxSsInst, 1);
	Dp21RxSs_Pt_Hdcp1xExamplePoll();
	XDpRxSs_HdcpSetProtocol(&DpRxSsInst, XDPRXSS_HDCP_14);
	XDpRxSs_HdcpEnable(&DpRxSsInst);
#endif
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_AUX_CLK_DIVIDER);
	ReadVal = ReadVal & 0xF0FF00FF;
	ReadVal = ReadVal | (0<<24);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_AUX_CLK_DIVIDER, ReadVal);

	DpRxSsInst.link_up_trigger = 1;
	DpRxSsInst.VBlankCount = 0;
	rx_unplugged = 0;

	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_VIDEO_MASK);

	/*
	 * Keep the Audio FIFO in reset state
	 */
	ReadVal = XDp_ReadReg(TX_CLK_RST_ADDR, 0x8);
	ReadVal = (ReadVal & 0x3);
	ReadVal = (ReadVal & 0x1); /**< Putting FIFO in reset */
	XDp_WriteReg(TX_CLK_RST_ADDR, 0x8, ReadVal);
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
void Dp21RxSs_Pt_RxUnplugHandler(void *InstancePtr)
{
	(void)InstancePtr;
#if ENABLE_HDCP_IN_DESIGN
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFE00FFFF);
#else
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xF9FFFFFF /*0xFFF87FFF*/);
#endif
	Video_valid = 0;
	xil_printf("Rx cable unplugged\r\n");

	/*
	 * Disable & Enable Audio
	 */
	AudioinfoFrame.frame_count = 0;
	AudioinfoFrame.all_count = 0;
	SdpExtFrame.Header[1] = 0;
	SdpExtFrame_q.Header[1] = 0;
	SdpExtFrame.frame_count = 0;
	SdpExtFrame.frame_count = 0;

	rx_unplugged = 1;

#ifdef Txo
	/*
	 * Mask the TX interrupts to avoid spurious HPDs
	 */
	Dp21RxSs_Pt_TxIntrMask();
#endif
	/*
	 * Enable Training related interrupts
	 */
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
			       XDP_RX_INTERRUPT_MASK_ALL_MASK);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

	/*
	 * Enable Training related interrupts
	 */
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_TP1_MASK |
			      XDP_RX_INTERRUPT_MASK_TP2_MASK |
			      XDP_RX_INTERRUPT_MASK_TP3_MASK|
			      XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK |
			      XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK |
			      XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK |
			      XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK |
			      XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK |
			      XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK |
			      XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK);

	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			       XDP_RX_INTERRUPT_MASK_TP4_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK |
			       XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.VBlankCount = 0;
	DpRxSsInst.no_video_trigger = 1;

#if ENABLE_HDCP22_IN_RX
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_SOFT_RESET,
		     XDP_RX_SOFT_RESET_HDCP22_MASK);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_SOFT_RESET, 0);
#endif

#if ENABLE_HDCP_IN_DESIGN
#if ENABLE_HDCP_FLOW_GUIDE
	XDpRxSs_HdcpDisable(&DpRxSsInst);
	XDpTxSs_HdcpDisable(&DpTxSsInst);
	Dp21RxSs_Pt_Hdcp1xExamplePoll();
#endif

#if ENABLE_HDCP1x_IN_RX
	XDpRxSs_SetPhysicalState(&DpRxSsInst, FALSE);
#endif

#if ENABLE_HDCP1x_IN_TX
	XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
#endif
	Dp21RxSs_Pt_Hdcp1xExamplePoll();
#if ENABLE_HDCP1x_IN_TX
	XDpTxSs_SetPhysicalState(&DpTxSsInst, FALSE);
#endif
	Dp21RxSs_Pt_Hdcp1xExamplePoll();
#if (ENABLE_HDCP_RX)
	XDpRxSs_StopTimer(&DpRxSsInst);
#endif
#endif
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
void Dp21RxSs_Pt_RxLinkBandwidthHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u32 Status = XST_SUCCESS;
	Status = Dp21RxSs_Pt_PhyConfig(&VPhyInst, DpRxSsInst.UsrOpt.LinkRate, VPHY_RX_PLL_TYPE, VPHY_RX_CHANNEL_TYPE, XVPHY_DIR_RX);
	if (Status != XST_SUCCESS) {
		xil_printf ("PHY Config on RX failed\r\n");
	}
	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.VBlankCount = 0;
	DpRxSsInst.no_video_trigger = 1;
#if ENABLE_HDCP_IN_DESIGN
	DpRxSsInst.TmrCtrResetDone = 0;
#endif
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
void Dp21RxSs_Pt_RxPllResetHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u32 ReadVal, Status = 0;

	/*
	 * Enable AUX Defers for training purpose
	 */
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_AUX_CLK_DIVIDER);
	ReadVal = ReadVal & 0xF0FF00FF;
	ReadVal = ReadVal | (AUX_DEFER_COUNT<<24);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_AUX_CLK_DIVIDER, ReadVal);

	Status = XVphy_DP21PhyReset(&VPhyInst, 0, VPHY_RX_CHANNEL_TYPE,
				    XVPHY_DIR_RX);
	if (Status == XST_FAILURE) {
		xil_printf ("Rx Issue encountered in PHY config and reset\r\n");
	}

	/*
	 * Enable all interrupts
	 */
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_ALL_MASK);

	DpRxSsInst.no_video_trigger = 1;
}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxBWChgHandler(void *InstancePtr)
{
	(void)InstancePtr;

}

/*****************************************************************************/
/**
*
* This function is the callback function for Access lane set request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxAccessLaneSetHandler(void *InstancePtr)
{
	(void)InstancePtr;
}

/*****************************************************************************/
/**
*
* This function is the callback function for Test CRC Event request.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxCRCTestEventHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u16 ReadVal;
	u32 TrainingAlgoValue;

	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_CRC_CONFIG);

	/*
	 * Record Training Algo Value - to be restored in non-phy test mode
	 */
	TrainingAlgoValue = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_MIN_VOLTAGE_SWING);

	/*
	 * Refer to DPCD 0x270 Register
	 */
	if((ReadVal&0x8000) == 0x8000) {
		/*
		 * Enable PHY test mode - Set Min voltage swing to 0
		 */
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_MIN_VOLTAGE_SWING,
			     (TrainingAlgoValue & 0xFFFFFFFC) | 0x80000000);

		/*
		 * Disable Training timeout
		 */
		ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
				      XDP_RX_CDR_CONTROL_CONFIG);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_CDR_CONTROL_CONFIG, ReadVal | 0x40000000);
	} else {
		/*
		 * Disable PHY test mode & Set min
		 * voltage swing back to level 1
		 */
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_MIN_VOLTAGE_SWING,
			     (TrainingAlgoValue & 0x7FFFFFFF) | 0x1);

		/*
		 * Enable Training timeout
		 */
		ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
				      XDP_RX_CDR_CONTROL_CONFIG);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_CDR_CONTROL_CONFIG, ReadVal & 0xBFFFFFFF);
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access link qual request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxAccessLinkQualHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u32 ReadVal;
	u32 DrpVal;
	u32 LinkQualConfig;

	LinkQualConfig = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				     XDP_RX_DPC_LINK_QUAL_CONFIG);

	/*
	 * AUX Defer is needed mainly for PRBS error counter transfer
	 *
	 * Paranoid defer for error counter reads to ensure value is properly reported
	 */
	if((LinkQualConfig&0x00000007) != 0x0) {
		ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				      XDP_RX_AUX_CLK_DIVIDER);
		ReadVal = ReadVal & 0xF0FF00FF;
		ReadVal = ReadVal | (AUX_DEFER_COUNT_PHY << 24);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     XDP_RX_AUX_CLK_DIVIDER, ReadVal);
	}

	/*
	 * Check for PRBS Mode
	 */
	if((LinkQualConfig&0x00000007) == XDP_RX_DPCD_LINK_QUAL_PRBS) {
		/*
		 * Enable PRBS Mode in Video PHY
		 */
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x10101010;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);

		/*
		 * Reset PRBS7 Counters
		 */
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x08080808;
		XDp_WriteReg(VPhyInst.Config.BaseAddr,
			     XVPHY_RX_CONTROL_REG, DrpVal);
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xF7F7F7F7;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);
	} else {
		/*
		 * Disable PRBS Mode in Video PHY
		 */
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xEFEFEFEF;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);
	}
#ifdef DEBUG
	xil_printf("Dp21RxSs_Pt_RxAccessLinkQualHandler : 0x%x\r\n", ReadVal);
#endif
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access prbs error count.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxAccessErrorCounterHandler(void *InstancePtr)
{
	u16 DrpVal = 0;
	u16 DrpVal_lower_lane0 = 0;
	u16 DrpVal_lower_lane1 = 0;
	u16 DrpVal_lower_lane2 = 0;
	u16 DrpVal_lower_lane3 = 0;
	u32 DrpValConfig;
	u32 SymErrVal_lane01;
	u32 SymErrVal_lane23;
	u32 ReadVal;
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	/*
	 * Read twice from VPHY
	 */
	SymErrVal_lane01 = XVphy_ReadReg(
					 VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
	SymErrVal_lane23 = XVphy_ReadReg(
					 VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);

	SymErrVal_lane01 = XVphy_ReadReg(
					 VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
	SymErrVal_lane23 = XVphy_ReadReg(
					 VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);

	/*
	 * Read PRBS Error Counter Value from Video PHY
	 *
	 * Lane 0 - Store only lower 16 bits
	 */
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane0);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER,
		    &DrpVal);

	/*
	 * Lane 1 - Store only lower 16 bits
	 */
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane1);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*
	 * Lane 2 - Store only lower 16 bits
	 */
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane2);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*
	 * Lane 3 - Store only lower 16 bits
	 */
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane3);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	ReadVal = XDp_ReadReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
			XDP_RX_DPC_LINK_QUAL_CONFIG);

	/*
	 * Write into DP Core - Validity bit and lower 15 bit counter value
	 */
	if((ReadVal&0x7) == XDP_RX_DPCD_LINK_QUAL_PRBS) {
		/*
		 * Write PRBS Values from GT to DPCD Error Reg Space
		 */
		XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
			     XDP_RX_DPC_L01_PRBS_CNTR,
			     (0x8000 | DrpVal_lower_lane0) |
			     ((0x8000 | DrpVal_lower_lane1) << 16));
		XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
			     XDP_RX_DPC_L23_PRBS_CNTR,
			     (0x8000 | DrpVal_lower_lane2) |
			     ((0x8000 | DrpVal_lower_lane3) << 16));
	} else {
		/*
		 * Write symbol error counter values from VPHY to DPCD Error Reg Space
		 */
		XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
			     XDP_RX_DPC_L01_PRBS_CNTR,
			     SymErrVal_lane01|0x80008000);
		XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
			     XDP_RX_DPC_L23_PRBS_CNTR,
			     SymErrVal_lane23|0x80008000);
	}
#if PRBS_ERRCNTR_CLEAR_ON_READ
	if(((ReadVal&0x7) == XDP_RX_DPCD_LINK_QUAL_PRBS) &&
			(DrpVal_lower_lane0!=0x0 || DrpVal_lower_lane1!=0x0 ||
			 DrpVal_lower_lane2!=0x0 || DrpVal_lower_lane3!=0x0 )) {
		/*
		 * Reset PRBS7 Counters
		 */
		DrpValConfig = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					     XVPHY_RX_CONTROL_REG);
		DrpValConfig = DrpValConfig | 0x08080808;

		XDp_WriteReg(VPhyInst.Config.BaseAddr,
			     XVPHY_RX_CONTROL_REG, DrpValConfig);
		DrpValConfig = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					     XVPHY_RX_CONTROL_REG);
		DrpValConfig = DrpValConfig & 0xF7F7F7F7;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpValConfig);
    }
#endif
}

/*****************************************************************************/
/**
*
* This function reports CRC values of Video components
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxReportVideoCRC()
{
	XVidFrameCrc_Report(&VidFrameCRC_rx, PPC_RX);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Info Packet Handling.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/

void Dp21RxSs_Pt_RxInfoPacketHandler(void *InstancePtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
#if ENABLE_AUDIO
        int i;
        for(i = 0; i < 8; i++) {
		if (tx_pass) {
			/*
			 * Start putting into FIFO. These will be programmed into TX
			 */
			infofifo[(endindex*8)+i] = XDp_ReadReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
							       XDP_RX_AUDIO_INFO_DATA(1));
                } else {
			/*
			 * Read or Ignore until TX is up and running.
			 */
                        XDp_ReadReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
				    XDP_RX_AUDIO_INFO_DATA(1));
                }
        }

	if (tx_pass) {
		if(endindex < (AUXFIFOSIZE - 1)) {
			endindex++;
		} else {
			endindex = 0;
		}

                if (fifocount >= AUXFIFOSIZE) {
                        fifoOverflow++;
                }

                fifocount++;
        }
#endif
}

/*****************************************************************************/
/**
*
* This function is the callback function for Generic Packet Handling of
* 32-Bytes payload.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Dp21RxSs_Pt_RxExtPacketHandler(void *InstancePtr)
{
	int i = 1;
	u32 ExtFrame[9];
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	SdpExtFrame.frame_count++;
        /*
	 * Header Information
	 */
        ExtFrame[0] = XDp_ReadReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
				  XDP_RX_AUDIO_EXT_DATA(1));

        SdpExtFrame.Header[0] =  ExtFrame[0]&0xFF;
        SdpExtFrame.Header[1] = (ExtFrame[0]&0xFF00)>>8;
        SdpExtFrame.Header[2] = (ExtFrame[0]&0xFF0000)>>16;
        SdpExtFrame.Header[3] = (ExtFrame[0]&0xFF000000)>>24;
        if (SdpExtFrame.Header[2] == 0x05) {
		/*
		 * This example supports only colorimetry extended
		 * packets
		 */
        } else {
                /*
		 * For future use
                 */
        }

        /*
	 * Payload Information
	 */
        for (i = 0 ; i < 8 ; i++)
        {
                ExtFrame[i+1] = XDp_ReadReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
                                XDP_RX_AUDIO_EXT_DATA(i+2));
                SdpExtFrame.Payload[(i*4)]   =  ExtFrame[i+1]&0xFF;
                SdpExtFrame.Payload[(i*4)+1] = (ExtFrame[i+1]&0xFF00)>>8;
                SdpExtFrame.Payload[(i*4)+2] = (ExtFrame[i+1]&0xFF0000)>>16;
                SdpExtFrame.Payload[(i*4)+3] = (ExtFrame[i+1]&0xFF000000)>>24;
        }
}

void Dp21RxSs_Pt_RxPrint_InfoPkt()
{
        xil_printf("Received Audio Info Packet::\r\n");
        xil_printf(" -frame_count                        : 0x%x \r\n",
                        AudioinfoFrame.frame_count);
        xil_printf(" -version                            : 0x%x \r\n",
                        AudioinfoFrame.version);
        xil_printf(" -type                                       : 0x%x \r\n",
                        AudioinfoFrame.type);
        xil_printf(" -sec_id                             : 0x%x \r\n",
                        AudioinfoFrame.sec_id);
        xil_printf(" -info_length                        : 0x%x \r\n",
                        AudioinfoFrame.info_length);
        xil_printf(" -audio_channel_count        : 0x%x \r\n",
                        AudioinfoFrame.audio_channel_count);
        xil_printf(" -audio_coding_type          : 0x%x \r\n",
                        AudioinfoFrame.audio_coding_type);
        xil_printf(" -sample_size                        : 0x%x \r\n",
                        AudioinfoFrame.sample_size);
        xil_printf(" -sampling_frequency         : 0x%x \r\n",
                        AudioinfoFrame.sampling_frequency);
        xil_printf(" -channel_allocation         : 0x%x \r\n",
                        AudioinfoFrame.channel_allocation);
        xil_printf(" -level_shift                        : 0x%x \r\n",
                        AudioinfoFrame.level_shift);
        xil_printf(" -downmix_inhibit            : 0x%x \r\n",
                        AudioinfoFrame.downmix_inhibit);
}

void Dp21RxSs_Pt_RxSetEDID(u8 edid_monitor[384])
{
	int i = 0;
	int j = 0;

	for (i = 0; i < (384*4); i = i+(16*4)) {
		for(j = i; j < (i+(16*4)); j = j+4) {
			XDp_WriteReg (VID_EDID_BASEADDR, j, edid_monitor[(i/4)+1]);
		}
	}

	for (i = 0; i < (384*4); i = i+4) {
		XDp_WriteReg (VID_EDID_BASEADDR, i, edid_monitor[i/4]);
	}

	xil_printf ("EDID of the Sink cloned into DpRxSs\r\n");
}

#if ENABLE_HDCP_IN_DESIGN
/*****************************************************************************/
/**
 * This function is assigned to callback on completion
 * of HDCP RX authentication.
 *
 * @param	InstancePtr is a pointer to the XDpRxSs HDCP core instance.
 *
 * @return	None.
 * @note		None.
 *
******************************************************************************/
void Dp21RxSs_Pt_RxHdcpAuthCallback(void *InstancePtr)
{
	XDpRxSs *XDpRxSsInst = (XDpRxSs *)InstancePtr;

#if (ENABLE_HDCP1x_IN_RX)
	/* Set Timer Counter reset done */
	XDpRxSsInst->TmrCtrResetDone = 1;
#endif
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
#if (ENABLE_HDCP_TX)
		XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
#endif

#if ENABLE_HDCP1x_IN_TX
		XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
#endif

		Dp21RxSs_Pt_Hdcp1xExamplePoll();

#if (ENABLE_HDCP_TX)
		XDpTxSs_HdcpEnable(&DpTxSsInst);
#endif
		Dp21RxSs_Pt_Hdcp1xExamplePoll();
	}
}

#if (ENABLE_HDCP_RX)
static void Dp21RxSs_Pt_RxHdcpTimeOutCallback(void *InstancePtr, u8 TmrCtrNumber)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	/*
	 * Verify arguments.
	 */
	Xil_AssertVoid(DpRxSsPtr != NULL);
	Xil_AssertVoid(TmrCtrNumber < XTC_DEVICE_TIMER_COUNT);

	/*
	 * Set Timer Counter reset done
	 */
	DpRxSsPtr->TmrCtrResetDone = 1;
}

/* *********************************************************
 *
 * This function is assigned to callback on
 * HDCP RX de-authentication.
 *
 * @param InstancePtr - DP RX SS HDCP core instance
 *
 * @return None.
 *
 */
void Dp21RxSs_Pt_RxHdcpUnAuthCallback(void *InstancePtr)
{
	XDpRxSs *XDpRxSsInst = (XDpRxSs *)InstancePtr;

	/*
	 * Configure the callback
	 */
	XTmrCtr_SetHandler(XDpRxSsInst->TmrCtrPtr,
			   (XTmrCtr_Handler)Dp21RxSs_Pt_RxHdcpTimeOutCallback,
			   InstancePtr);
}
#endif
#endif /**< ENABLE_HDCP_IN_DESIGN */
#endif
