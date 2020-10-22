/*******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
* </pre>
*
******************************************************************************/

#include "main.h"
#include "rx.h"

volatile u8 rx_unplugged = 0;
volatile u16 DrpVal=0;
volatile u16 DrpVal_lower_lane0=0;
volatile u16 DrpVal_lower_lane1=0;
volatile u16 DrpVal_lower_lane2=0;
volatile u16 DrpVal_lower_lane3=0;
extern XDpTxSs DpTxSsInst;
extern u32 vblank_init;
extern u8 vblank_captured;
extern u32 appx_fs_dup;
extern u8 rx_trained;
extern u8 start_i2s_clk;
extern u8 i2s_tx_started;
extern u8 rx_aud;
extern u8 tx_after_rx;
extern u8 tx_done;
extern void mask_intr (void);

#ifdef versal
#if (VERSAL_FABRIC_8B10B == 1)
extern XClk_Wiz_Config *CfgPtr_Dynamic_rx;
extern XClk_Wiz ClkWiz_Dynamic_rx;
#endif
#endif


//extern u8 audio_info_avail;
u32 infofifo[64]; //RX and TX can store upto 4 infoframes each. fifo of 8
u8 endindex = 0;
u8 fifocount = 0;
u32 hdrframe[9];
u16 fifoOverflow=0;
extern u8 tx_pass;
extern u8 startindex;


void DpRxSs_SetVsc (u8 vsc) {
	u32 VSC_MASK = 0x00000004;
	u32 RegVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DTG_ENABLE);
	if (vsc) {
		RegVal |= VSC_MASK;
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_DTG_ENABLE, RegVal);
		xil_printf (ANSI_COLOR_GREEN"DP RX enabled for VSC Colorimetry support"ANSI_COLOR_RESET" \r\n");
	} else {
		RegVal &= ~VSC_MASK;
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_DTG_ENABLE, RegVal);
	}
}

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
u32 DpRxSs_Setup(u8 freesync, u8 vsc)
{
	u32 ReadVal;

	/*Disable Rx*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_LINK_ENABLE, 0x0);

	/*Disable All Interrupts*/
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

	/*Enable Training related interrupts*/
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			  XDP_RX_INTERRUPT_MASK_TP1_MASK |
			  XDP_RX_INTERRUPT_MASK_TP2_MASK |
			XDP_RX_INTERRUPT_MASK_TP3_MASK|
			XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
			XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK|
			XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);

	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_TP4_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

	/* Setting AUX Defer Count of Link Status Reads to 8 during Link
	 * Training 8 Defer counts is chosen to handle worst case time
	 * interrupt service load (PL system working at 100 MHz) when
	 * working with R5.
	 * */
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		XDP_RX_AUX_CLK_DIVIDER);
	ReadVal = ReadVal & 0xF0FF00FF;
	ReadVal = ReadVal | (AUX_DEFER_COUNT<<24);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		XDP_RX_AUX_CLK_DIVIDER, ReadVal);

	/*Setting BS Idle timeout value to long value*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_BS_IDLE_TIME, DP_BS_IDLE_TIMEOUT);

	if(LINK_TRAINING_DEBUG==1){
		/*Updating Vswing Iteration Count*/
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
				RxTrainConfig.MinVoltageSwing |
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

	/*Enable CRC Support*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
			VidFrameCRC_rx.TEST_CRC_SUPPORTED<<5);
#if PHY_COMP
	/*Set to 0 when Audio is NOT supported*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_LOCAL_EDID_VIDEO, 0x0);
#endif

#if ADAPTIVE
	if (freesync) {
		//Setting RX DPCD to be Adaptive-Sync capable
		XDpRxSs_SetAdaptiveSyncCaps(&DpRxSsInst, 1);

#if ADAPTIVE_TYPE
		//Disabling Adaptive interrupt
		XDpRxSs_MaskAdaptiveIntr(&DpRxSsInst, 0xC0000000);
#else
		//enabling Adaptive interrupt
		XDpRxSs_UnMaskAdaptiveIntr(&DpRxSsInst, 0xC0000000);
#endif
		xil_printf (ANSI_COLOR_GREEN"DP RX enabled for Adaptive Sync"ANSI_COLOR_RESET" \r\n");
	} else {
		//Disabling Adaptive interrupt
		XDpRxSs_MaskAdaptiveIntr(&DpRxSsInst, 0xC0000000);
		XDpRxSs_SetAdaptiveSyncCaps(&DpRxSsInst, 0);
		xil_printf (ANSI_COLOR_GREEN"DP RX not enabled for Adaptive Sync"ANSI_COLOR_RESET" \r\n");
	}
#else
	XDpRxSs_MaskAdaptiveIntr(&DpRxSsInst, 0xC0000000);
	XDpRxSs_SetAdaptiveSyncCaps(&DpRxSsInst, 0);
#endif

	//Setting VSC Colorimetry capability in RX
	DpRxSs_SetVsc(vsc);

	/*Enable Rx*/
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
u32 DpRxSs_SetupIntrSystem(void)
{
//	u32 Status;
//	XINTC *IntcInstPtr = &IntcInst;

	/* Set callbacks for all the interrupts */
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,
			    &DpRxSs_PowerChangeHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_EVENT,
			    &DpRxSs_NoVideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VM_CHG_EVENT,
			    &DpRxSs_VmChangeHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_EVENT,
			    &DpRxSs_VerticalBlankHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TLOST_EVENT,
			    &DpRxSs_TrainingLostHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_EVENT,
			    &DpRxSs_VideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
			    &DpRxSs_TrainingDoneHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_UNPLUG_EVENT,
			    &DpRxSs_UnplugHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_LINKBW_EVENT,
			    &DpRxSs_LinkBandwidthHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_PLL_RESET_EVENT,
			    &DpRxSs_PllResetHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_BW_CHG_EVENT,
			    &DpRxSs_BWChangeHandler, &DpRxSsInst);
//	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LANE_SET_EVENT,
//			    DpRxSs_AccessLaneSetHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LINK_QUAL_EVENT,
			    &DpRxSs_AccessLinkQualHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst,
			    XDPRXSS_HANDLER_ACCESS_ERROR_COUNTER_EVENT,
			    &DpRxSs_AccessErrorCounterHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,
			    &DpRxSs_CRCTestEventHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
			&DpRxSs_InfoPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
			&DpRxSs_ExtPacketHandler, &DpRxSsInst);
#if ADAPTIVE
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_VBLANK_EVENT,
			&DpRxSs_AdaptiveVblankHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ADAPTIVESYNC_SDP_EVENT,
			&DpRxSs_AdaptiveSDPHandler, &DpRxSsInst);

#endif

	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the Adaptive SDP interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AdaptiveSDPHandler(void *InstancePtr)
{

}

/*****************************************************************************/
/**
*
* This function is the callback function for when the Adaptive Vblank interrupt
* occurs. Here the new value of Vblank is read. The difference in vblank is
* then programmed into VTC
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AdaptiveVblankHandler(void *InstancePtr)
{
	u32 vblank;
	u32 delta;
	vblank = XDpRxSs_GetVblank(&DpRxSsInst);

	/* Calculate delta of Vtotal */
	if (vblank_captured) {
		delta = vblank - vblank_init;
		/* Update the Stretch value in VTC */
#if !ADAPTIVE_TYPE && ADAPTIVE
		XDpTxSs_VtcAdaptiveSyncSetup(DpTxSsInst.VtcPtr[0], ADAPTIVE_TYPE, delta);
		//xil_printf ("delta = %d\r\n",delta);
#endif
	}


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
void DpRxSs_PowerChangeHandler(void *InstancePtr)
{

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
void DpRxSs_VmChangeHandler(void *InstancePtr)
{
//	tx_after_rx = 1;
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
void DpRxSs_NoVideoHandler(void *InstancePtr)
{
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
	type_vsc = 0;
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
void DpRxSs_VerticalBlankHandler(void *InstancePtr)
{
	DpRxSsInst.VBlankCount++;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a training lost interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/

void DpRxSs_TrainingLostHandler(void *InstancePtr)
{
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
	XDpRxSs_AudioDisable(&DpRxSsInst);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	// mask the TX interrupts to avoid spurious HPDs
	mask_intr();
	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.VBlankCount = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
	appx_fs_dup = 0;
	tx_done = 0;
	start_i2s_clk = 0;
	i2s_tx_started = 0;
	rx_aud = 0;
	tx_after_rx = 0;
//	audio_info_avail = 0;
    AudioinfoFrame.frame_count = 0;
    AudioinfoFrame.all_count = 0;
	if (rx_trained == 1) {
		xil_printf ("Training Lost !!\r\n");
	}
	rx_trained = 0;
	vblank_captured = 0;
	endindex = 0;
	fifocount = 0;
	startindex = 0;
	type_vsc = 0;

	XDpRxSs_MaskAdaptiveIntr(&DpRxSsInst, 0xC0000000);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a valid video interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_VideoHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the training done interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_TrainingDoneHandler(void *InstancePtr)
{
	u32 ReadVal;
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		XDP_RX_AUX_CLK_DIVIDER);
	ReadVal = ReadVal & 0xF0FF00FF;
	ReadVal = ReadVal | (0<<24);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		XDP_RX_AUX_CLK_DIVIDER, ReadVal);

	DpRxSsInst.link_up_trigger = 1;
	DpRxSsInst.VBlankCount = 0;
	rx_unplugged = 0;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the unplug event occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/


void DpRxSs_UnplugHandler(void *InstancePtr)
{
	/* Disable & Enable Audio */
	rx_unplugged = 1;
//	appx_fs_dup = 0;
//	XDpRxSs_AudioDisable(&DpRxSsInst);
//    audio_info_avail = 0;
    AudioinfoFrame.frame_count = 0;
    AudioinfoFrame.all_count = 0;
	SdpExtFrame.Header[1] = 0;
	SdpExtFrame_q.Header[1] = 0;
	SdpExtFrame.frame_count = 0;
	SdpExtFrame.frame_count = 0;
	type_vsc = 0;

	// mask the TX interrupts to avoid spurious HPDs
	mask_intr();

	/*Enable Training related interrupts*/
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
			       XDP_RX_INTERRUPT_MASK_ALL_MASK);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_TP1_MASK |
			      XDP_RX_INTERRUPT_MASK_TP2_MASK |
			XDP_RX_INTERRUPT_MASK_TP3_MASK|
			XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
			XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK|
			XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);

	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_TP4_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.VBlankCount = 0;
	DpRxSsInst.no_video_trigger = 1;

}

/*****************************************************************************/
/**
*
* This function is the callback function for when the link bandwidth change
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_LinkBandwidthHandler(void *InstancePtr)
{
	u32 Status;
#ifndef versal
	/*Program Video PHY to requested line rate*/
	PLLRefClkSel (&VPhyInst, DpRxSsInst.UsrOpt.LinkRate);

	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			 XVPHY_DIR_RX,(TRUE));

	XVphy_PllInitialize(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
				ONBOARD_REF_CLK, ONBOARD_REF_CLK,
				XVPHY_PLL_TYPE_QPLL1, XVPHY_PLL_TYPE_CPLL);
	Status = XVphy_ClkInitialize(&VPhyInst, 0,
									XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);
	if(Status != XST_SUCCESS)
		xil_printf("XVphy_ClkInitialize failed\r\n");
#else

	u8 bridge;
	u32 retry = 0;
	u32 dprx_sts=0;
	u32 rate=0;
	u32 HighTime;
	u32 DivEdge;
	u32 Reg;
	u32 P5Enable;
	u32 P5fEdge;
	u16 Oval;
	u16 Dval;
	u16 Mval;

	/* For each line rate, the M, D, Div values for MMCM are chosen such that
	 * MMCM gives a /20 clock output for /16 clk input.
	 *
	 * GT ch0outclk (/16) --> MMCM --> /20 clock
	 *
	 * Thus:
	 * 8.1G  : Input MMCM clock is 506.25, output is 405
	 * 5.4G  : Input MMCM clock is 337.5, output is 270
	 * 2.7G  : Input MMCM clock is 168.75, output is 135
	 * 1.62G : Input MMCM clock is 101.25, output is 81
	 */

    if (DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_810GBPS) {
		bridge = VERSAL_810G;
		Mval = 28;
		Dval = 5;
		Oval = 7;
    } else if (DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_540GBPS) {
		bridge = VERSAL_540G;
		Mval = 44;
		Dval = 5;
		Oval = 11;
    } else if (DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_270GBPS) {
		bridge = VERSAL_270G;
		Mval = 88;
		Dval = 5;
		Oval = 22;
    } else {
		bridge = VERSAL_162G;
		Mval = 148;
		Dval = 5;
		Oval = 37;
    }

#if (VERSAL_FABRIC_8B10B == 1)
    /* MMCM is dynamically programmed for the respective rate
     * using the M, D, Div values
     */
	HighTime = (Oval / 4);
	Reg =  XCLK_WIZ_REG3_PREDIV2 | XCLK_WIZ_REG3_USED | XCLK_WIZ_REG3_MX;
	if (Oval % 4 <= 1) {
		DivEdge = 0;
	} else {
		DivEdge = 1;
	}
	Reg |= (DivEdge << 8);
	P5fEdge = Oval % 2;
	P5Enable = Oval % 2;
	Reg = Reg | P5Enable << XCLK_WIZ_CLKOUT0_P5EN_SHIFT | P5fEdge << XCLK_WIZ_CLKOUT0_P5FEDGE_SHIFT;
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG3_OFFSET, Reg);
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG4_OFFSET, Reg);

	/* Implement D */
	HighTime = (Dval / 2);
	Reg  = 0;
	Reg = Reg & ~(1 << XCLK_WIZ_REG12_EDGE_SHIFT);
	DivEdge = Dval % 2;
	Reg = Reg | DivEdge << XCLK_WIZ_REG12_EDGE_SHIFT;
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG12_OFFSET, Reg);
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG13_OFFSET, Reg);

	/* Implement M*/
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);

	DivEdge = Mval % 2;
	HighTime = Mval / 2;
	Reg = HighTime | HighTime << 8;
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG2_OFFSET, Reg);
	Reg = XCLK_WIZ_REG1_PREDIV2 | XCLK_WIZ_REG1_EN | XCLK_WIZ_REG1_MX;

	if (DivEdge) {
		Reg = Reg | (1 << XCLK_WIZ_REG1_EDGE_SHIFT);
	} else {
		Reg = Reg & ~(1 << XCLK_WIZ_REG1_EDGE_SHIFT);
	}
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG1_OFFSET, Reg);
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG11_OFFSET, 0x2e);
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG14_OFFSET, 0xe80);
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG15_OFFSET, 0x4271);
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG16_OFFSET, 0x43e9);
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG17_OFFSET, 0x001C);
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_REG26_OFFSET, 0x0001);
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_RECONFIG_OFFSET, 0x3);
#endif

    rate = bridge << 1;
    GtCtrl (GT_RATE_MASK, rate, 0);
    dprx_sts = 0;
	while (dprx_sts != ALL_LANE && retry < 10000) {
		 dprx_sts = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x208);
		 dprx_sts &= ALL_LANE;
		 retry++;
	}
	if (retry == 10000) {
		xil_printf ("+++ RX GT Configuration failure ++++\r\n");
	}
#if (VERSAL_FABRIC_8B10B == 1)
	retry = 0;
	/* MMCM issued a reset */
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, 0x0, 0xA);
	while(!(XClk_Wiz_ReadReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_STATUS_OFFSET) & 1)) {
		if(retry == 10000) {
				break;
		}
//					usleep(100);
		retry++;
	}
	if (retry == 10000) {
		xil_printf ("Rx Clk_wizard failed to lock\r\n");
	}
#endif
#endif

	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.VBlankCount = 0;
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
void DpRxSs_PllResetHandler(void *InstancePtr)
{

	u32 ReadVal;
	/*Enable AUX Defers for training purpose*/
    ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                   XDP_RX_AUX_CLK_DIVIDER);
    ReadVal = ReadVal & 0xF0FF00FF;
    ReadVal = ReadVal | (AUX_DEFER_COUNT<<24);
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                   XDP_RX_AUX_CLK_DIVIDER, ReadVal);

#ifndef versal
	/* Issue resets to Video PHY - This API
	 * called after line rate is programmed */
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			 XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			 XVPHY_DIR_RX, (FALSE));
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(FALSE));
	XVphy_WaitForResetDone(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			       XVPHY_DIR_RX);
	XVphy_WaitForPllLock(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA);
#endif

#if PHY_COMP
	/*Enable all interrupts, except unplug */
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			0x79FFFFFF);

#else
	/*Enable all interrupts */
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			      XDP_RX_INTERRUPT_MASK_ALL_MASK);

#endif

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
void DpRxSs_BWChangeHandler(void *InstancePtr)
{
//    MCDP6000_ResetDpPath(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access lane set request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AccessLaneSetHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for Test CRC Event request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_CRCTestEventHandler(void *InstancePtr)
{
	u16 ReadVal;
	u32 TrainingAlgoValue;

	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_CRC_CONFIG);

	/*Record Training Algo Value - to be restored in non-phy test mode*/
	TrainingAlgoValue = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
			XDP_RX_MIN_VOLTAGE_SWING);

	/*Refer to DPCD 0x270 Register*/
	if( (ReadVal&0x8000) == 0x8000){
			/*Enable PHY test mode - Set Min voltage swing to 0*/
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_MIN_VOLTAGE_SWING,
				(TrainingAlgoValue&0xFFFFFFFC)|0x80000000);

			/*Disable Training timeout*/
			ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_CDR_CONTROL_CONFIG);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_CDR_CONTROL_CONFIG, ReadVal|0x40000000);

	}else{
		/* Disable PHY test mode & Set min
		 * voltage swing back to level 1 */
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MIN_VOLTAGE_SWING,
						(TrainingAlgoValue&0x7FFFFFFF)|0x1);

			/*Enable Training timeout*/
			ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_CDR_CONTROL_CONFIG);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_CDR_CONTROL_CONFIG, ReadVal&0xBFFFFFFF);
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
void DpRxSs_AccessLinkQualHandler(void *InstancePtr)
{
	u32 ReadVal;
	u32 DrpVal;
	u32 LinkQualConfig;


	LinkQualConfig = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DPC_LINK_QUAL_CONFIG);

	/*AUX Defer is needed mainly for PRBS error counter transfer*/
	/*Paranoid defer for error counter reads to
	 * ensure value is properly reported*/
	if( (LinkQualConfig&0x00000007) != 0x0){
		ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_AUX_CLK_DIVIDER);
		ReadVal = ReadVal & 0xF0FF00FF;
		ReadVal = ReadVal | (AUX_DEFER_COUNT_PHY<<24);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_AUX_CLK_DIVIDER, ReadVal);
	}

#ifndef versal
	/*Check for PRBS Mode*/
	if( (LinkQualConfig&0x00000007) == XDP_RX_DPCD_LINK_QUAL_PRBS){
		/*Enable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x10101010;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);

		/*Reset PRBS7 Counters*/
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

		/*Set PRBS mode in Retimer*/
		XDpRxSs_MCDP6000_EnablePrbs7_Rx(&DpRxSsInst,
					I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(&DpRxSsInst,
				      I2C_MCDP6000_ADDR);
	//    	MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	} else {
		/*Disable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
				       XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xEFEFEFEF;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG, DrpVal);

		/*Disable PRBS mode in Retimer*/
		XDpRxSs_MCDP6000_DisablePrbs7_Rx(&DpRxSsInst,
											I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(&DpRxSsInst,
				      I2C_MCDP6000_ADDR);
	//    	MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	}
#endif
	xil_printf("DpRxSs_AccessLinkQualHandler : 0x%x\r\n", ReadVal);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access prbs error count.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AccessErrorCounterHandler(void *InstancePtr)
{
	u32 DrpValConfig;

	u32 SymErrVal_lane01;
	u32 SymErrVal_lane23;

	u32 ReadVal;

#ifndef versal
	/*Read twice from VPHY*/
	SymErrVal_lane01 = XVphy_ReadReg(
			VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
	SymErrVal_lane23 = XVphy_ReadReg(
			VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);

	SymErrVal_lane01 = XVphy_ReadReg(
			VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
	SymErrVal_lane23 = XVphy_ReadReg(
			VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);

	/*Read PRBS Error Counter Value from Video PHY*/

	/*Lane 0 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane0);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER,
		    &DrpVal);

	/*Lane 1 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane1);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Lane 2 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane2);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Lane 3 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER,
		    &DrpVal_lower_lane3);
	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		    XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
#endif
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DPC_LINK_QUAL_CONFIG);

//	xil_printf("XDP_RX_DPC_LINK_QUAL_CONFIG: 0x%x\r\n",ReadVal);

	/*Write into DP Core - Validity bit and lower 15 bit counter value*/
    if((ReadVal&0x7) == XDP_RX_DPCD_LINK_QUAL_PRBS)
    {
	/*Write PRBS Values from GT to DPCD Error Reg Space*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_DPC_L01_PRBS_CNTR,
		     (0x8000 | DrpVal_lower_lane0) |
		     ((0x8000 | DrpVal_lower_lane1) << 16));
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_DPC_L23_PRBS_CNTR,
		     (0x8000 | DrpVal_lower_lane2) |
		     ((0x8000 | DrpVal_lower_lane3) << 16));

    }
    else
    {
	/*Write symbol error counter values from VPHY to DPCD Error Reg Space*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_DPC_L01_PRBS_CNTR,
			 SymErrVal_lane01|0x80008000);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_DPC_L23_PRBS_CNTR,
			 SymErrVal_lane23|0x80008000);

    }

#if PRBS_ERRCNTR_CLEAR_ON_READ
    if( ((ReadVal&0x7) == XDP_RX_DPCD_LINK_QUAL_PRBS) &&
	( DrpVal_lower_lane0!=0x0 || DrpVal_lower_lane1!=0x0 ||
	  DrpVal_lower_lane2!=0x0 || DrpVal_lower_lane3!=0x0 )
      )
    {
#ifndef versal
		/* Reset PRBS7 Counters */
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
#endif
    }
#endif

    xil_printf("DpRxSs_AccessErrorCounterHandler\r\n");
}


/*****************************************************************************/
/**
*
* This function reports CRC values of Video components
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void ReportVideoCRC()
{
	XVidFrameCrc_Report(&VidFrameCRC_rx);
}


/*****************************************************************************/
/**
*
* This function load EDID content into EDID Memory. User can change as per
*     their requirement.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void LoadEDID(void)
 {
	int i=0;
	int j=0;

#if(EDID_1_ENABLED)
	unsigned char edid[256] = {
			// This is good for compliance test
            0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
			0x61, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x17, 0x01, 0x04, 0xb5, 0x3c, 0x22, 0x78,
			0x00, 0x1d, 0xf5, 0xae, 0x4f, 0x35, 0xb3, 0x25,
            0x0d, 0x50, 0x54, 0x21, 0x08, 0x00, 0x81, 0x00,
			0xb3, 0x00, 0xd1, 0x00, 0xd1, 0xc0, 0xa9, 0x40,
            0x81, 0x80, 0x01, 0x01, 0x01, 0x01, 0xbe, 0x6e,
			0x00, 0x68, 0xf1, 0x70, 0x5a, 0x80, 0x64, 0x58,
            0x8a, 0x00, 0xba, 0x89, 0x21, 0x00, 0x00, 0x1a,
			0x00, 0x00, 0x00, 0xff, 0x00, 0x0a, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x58,
            0x49, 0x4c, 0x20, 0x44, 0x50, 0x0a, 0x20, 0x20,
			0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd,
            0x00, 0x31, 0x56, 0x1d, 0x71, 0x1e, 0x00, 0x0a,
			0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xda,
            0x02, 0x03, 0x0e, 0xc1, 0x41, 0x90, 0x23, 0x09,
			0x1f, 0x07, 0x83, 0x01, 0x00, 0x00, 0x02, 0x3a,
            0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
			0x45, 0x00, 0x55, 0x50, 0x21, 0x00, 0x00, 0x1e,
            0x01, 0x1d, 0x80, 0x18, 0x71, 0x1c, 0x16, 0x20,
			0x58, 0x2c, 0x25, 0x00, 0x55, 0x50, 0x21, 0x00,
            0x00, 0x9e, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0,
			0x1e, 0x20, 0x6e, 0x28, 0x55, 0x00, 0x55, 0x50,
            0x21, 0x00, 0x00, 0x1e, 0x8c, 0x0a, 0xd0, 0x8a,
			0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00,
            0x55, 0x50, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbb
	};
#else
	// 8K30, 8K24, 5K, 4K120, 4K100 + Audio
	unsigned char edid[256] = {
			0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
			0x61, 0x98, 0x23, 0x01, 0x00, 0x00, 0x00, 0x00,
			0x28, 0x1C, 0x01, 0x04, 0xB5, 0x3C, 0x22, 0x78,
			0x26, 0x61, 0x50, 0xA6, 0x56, 0x50, 0xA0, 0x00,
			0x0D, 0x50, 0x54, 0xA5, 0x6B, 0x80, 0xD1, 0xC0,
			0x81, 0xC0, 0x81, 0x00, 0x81, 0x80, 0xA9, 0x00,
			0xB3, 0x00, 0xD1, 0xFC, 0x01, 0x01, 0x04, 0x74,
			0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58,
			0x8A, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x1A,
			0x4D, 0xD0, 0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80,
			0x30, 0x20, 0x35, 0x00, 0x56, 0x50, 0x21, 0x00,
			0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x1E,
			0x3C, 0x32, 0xB4, 0x66, 0x01, 0x0A, 0x20, 0x20,
			0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC,
			0x00, 0x58, 0x69, 0x6C, 0x69, 0x6E, 0x78, 0x20,
			0x73, 0x69, 0x6E, 0x6B, 0x0A, 0x20, 0x01, 0x17,
			0x70, 0x12, 0x6E, 0x00, 0x00, 0x81, 0x00, 0x04,
			0x23, 0x09, 0x03, 0x07, 0x03, 0x00, 0x64, 0xEB,
			0xA0, 0x01, 0x04, 0xFF, 0x0E, 0xA0, 0x00, 0x2F,
			0x80, 0x21, 0x00, 0x6F, 0x08, 0x3E, 0x00, 0x03,
			0x00, 0x05, 0x00, 0xFD, 0x68, 0x01, 0x04, 0xFF,
			0x13, 0x4F, 0x00, 0x27, 0x80, 0x1F, 0x00, 0x3F,
			0x0B, 0x51, 0x00, 0x43, 0x00, 0x07, 0x00, 0x65,
			0x8E, 0x01, 0x04, 0xFF, 0x1D, 0x4F, 0x00, 0x07,
			0x80, 0x1F, 0x00, 0xDF, 0x10, 0x3C, 0x00, 0x2E,
			0x00, 0x07, 0x00, 0x86, 0x3D, 0x01, 0x04, 0xFF,
			0x1D, 0x4F, 0x00, 0x07, 0x80, 0x1F, 0x00, 0xDF,
			0x10, 0x30, 0x00, 0x22, 0x00, 0x07, 0x00, 0x5C,
			0x7F, 0x01, 0x00, 0xFF, 0x0E, 0x4F, 0x00, 0x07,
			0x80, 0x1F, 0x00, 0x6F, 0x08, 0x73, 0x00, 0x65,
			0x00, 0x07, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90
	};
#endif

	for (i = 0 ; i < ( 256 * 4 ) ; i = i + (16 * 4)) {
		for ( j = i ; j < ( i + (16 * 4)) ; j = j + 4 ) {
			XDp_WriteReg (VID_EDID_BASEADDR, j, edid[( i / 4 ) + 1 ] );
		}
	}
	for ( i = 0 ; i < ( 256 * 4 ); i = i + 4 ) {
		XDp_WriteReg (VID_EDID_BASEADDR, i, edid[i / 4]);
	}

}


/*****************************************************************************/
/**
*
* This function is the callback function for Info Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/

void DpRxSs_InfoPacketHandler(void *InstancePtr)
{
	u32 InfoFrame[9];
	int i=0;
	for(i = 0 ; i < 8 ; i++) {
		if (tx_pass) {
			//Start putting into FIFO. These will be programmed into TX
			infofifo[(endindex*8)+i] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_AUDIO_INFO_DATA(1));
		} else {
			// Read of Ignore until TX is up and running
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
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
	//		xil_printf ("Aux fifo overflow\r\n");
			fifoOverflow++;
		}

		fifocount++;
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for Generic Packet Handling of
* 32-Bytes payload.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/


void DpRxSs_ExtPacketHandler(void *InstancePtr)
{

	int i=1;
	u32 ExtFrame[9];

	SdpExtFrame.frame_count++;

	/*Header Information*/
	ExtFrame[0] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_AUDIO_EXT_DATA(1));
	SdpExtFrame.Header[0] =  ExtFrame[0]&0xFF;
	SdpExtFrame.Header[1] = (ExtFrame[0]&0xFF00)>>8;
	SdpExtFrame.Header[2] = (ExtFrame[0]&0xFF0000)>>16;
	SdpExtFrame.Header[3] = (ExtFrame[0]&0xFF000000)>>24;

	if (SdpExtFrame.Header[2] == 0x05) {
		/* This example supports only colorimetry extended
		 * packets
		 */
		ExtFrame_tx_vsc.Header = ExtFrame[0];
		type_vsc = 0x13;
	} else {
		/* For future use
		 *
		 */
	}

	/*Payload Information*/
	for (i = 0 ; i < 8 ; i++)
	{
		ExtFrame[i+1] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_EXT_DATA(i+2));
		SdpExtFrame.Payload[(i*4)]   =  ExtFrame[i+1]&0xFF;
		SdpExtFrame.Payload[(i*4)+1] = (ExtFrame[i+1]&0xFF00)>>8;
		SdpExtFrame.Payload[(i*4)+2] = (ExtFrame[i+1]&0xFF0000)>>16;
		SdpExtFrame.Payload[(i*4)+3] = (ExtFrame[i+1]&0xFF000000)>>24;
		if (type_vsc == 0x13) {
			ExtFrame_tx_vsc.Payload[i] = ExtFrame[i+1];
		} else {
			/* For future use
			 *
			 */
		}
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for Info Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Print_InfoPkt()
{
	xil_printf("Received Audio Info Packet::\r\n");
	xil_printf(" -frame_count		 	 : 0x%x \r\n",
			AudioinfoFrame.frame_count);
	xil_printf(" -version			 	 : 0x%x \r\n",
			AudioinfoFrame.version);
	xil_printf(" -type				 	 : 0x%x \r\n",
			AudioinfoFrame.type);
	xil_printf(" -sec_id				 : 0x%x \r\n",
			AudioinfoFrame.sec_id);
	xil_printf(" -info_length			 : 0x%x \r\n",
			AudioinfoFrame.info_length);
	xil_printf(" -audio_channel_count	 : 0x%x \r\n",
			AudioinfoFrame.audio_channel_count);
	xil_printf(" -audio_coding_type		 : 0x%x \r\n",
			AudioinfoFrame.audio_coding_type);
	xil_printf(" -sample_size			 : 0x%x \r\n",
			AudioinfoFrame.sample_size);
	xil_printf(" -sampling_frequency	 : 0x%x \r\n",
			AudioinfoFrame.sampling_frequency);
	xil_printf(" -channel_allocation	 : 0x%x \r\n",
			AudioinfoFrame.channel_allocation);
	xil_printf(" -level_shift			 : 0x%x \r\n",
			AudioinfoFrame.level_shift);
	xil_printf(" -downmix_inhibit		 : 0x%x \r\n",
			AudioinfoFrame.downmix_inhibit);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Ext Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Print_ExtPkt()
{
	int i=0;

	xil_printf("Received SDP Packet Type::\r\n");
	switch(SdpExtFrame.Header[1])
	{
		case 0x04: xil_printf(" -> Extension\r\n"); break;
		case 0x05: xil_printf(" -> Audio_CopyManagement\r\n"); break;
		case 0x06: xil_printf(" -> ISRC\r\n"); break;
		case 0x07: xil_printf(" -> Video Stream Configuration (VSC)\r\n");break;
		case 0x20: xil_printf(" -> Video Stream Configuration Extension"
				" for VESA (VSC_EXT_VESA) - Used for HDR Metadata\r\n"); break;
		case 0x21: xil_printf(" -> VSC_EXT_CEA for future CEA INFOFRAME with "
				"payload of more than 28 bytes\r\n"); break;
		default: xil_printf(" -> Reserved/Not Defined\r\n"); break;
	}
	xil_printf(" Header Bytes : 0x%x, 0x%x, 0x%x, 0x%x \r\n",
			SdpExtFrame.Header[0],
			SdpExtFrame.Header[1],
			SdpExtFrame.Header[2],
			SdpExtFrame.Header[3]);
	for(i=0;i<8;i++)
	{
		xil_printf(" Payload Bytes : 0x%x, 0x%x, 0x%x, 0x%x \r\n",
				SdpExtFrame.Payload[(i*4)],
				SdpExtFrame.Payload[(i*4)+1],
				SdpExtFrame.Payload[(i*4)+2],
				SdpExtFrame.Payload[(i*4)+3]);
	}
	xil_printf(" Frame Count : %d \r\n",SdpExtFrame.frame_count);
}
