/*******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
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
#include "tx.h"

u8 rx_unplugged = 0;

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
u32 DpRxSs_Setup(void)
{
	u32 ReadVal;

	/*Disable Rx*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_LINK_ENABLE, 0x0);


	/*Disable All Interrupts*/
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

#if ENABLE_HDCP_IN_DESIGN
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_INTERRUPT_MASK, 0xFE00FFFD);
	XHdcp1xExample_Poll();
#else
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_INTERRUPT_MASK, 0xFFF87FFD);
#endif

	/*Enable Training related interrupts*/
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			  XDP_RX_INTERRUPT_MASK_TP1_MASK |
			  XDP_RX_INTERRUPT_MASK_TP2_MASK |
			XDP_RX_INTERRUPT_MASK_TP3_MASK|
			XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
			XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK|
			XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);

#if ENABLE_HDCP_IN_DESIGN
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_HDCP_DEBUG_WRITE_MASK |
			XDP_RX_INTERRUPT_MASK_HDCP_AKSV_WRITE_MASK |
			XDP_RX_INTERRUPT_MASK_HDCP_AN_WRITE_MASK |
			XDP_RX_INTERRUPT_MASK_HDCP_AINFO_WRITE_MASK |
			XDP_RX_INTERRUPT_MASK_HDCP_RO_READ_MASK |
			XDP_RX_INTERRUPT_MASK_HDCP_BINFO_READ_MASK);
#endif

	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_TP4_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

#if ENABLE_HDCP_IN_DESIGN
	XHdcp1xExample_Poll();
#endif

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

#if (XPAR_XHDCP_NUM_INSTANCES > 0 && ENABLE_HDCP_IN_DESIGN == 1 )
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_HDCP_AUTHENTICATED,
							&Dprx_HdcpAuthCallback, &DpRxSsInst);
#endif

	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);


	return (XST_SUCCESS);
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

	AudioinfoFrame.frame_count=0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
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
//	xil_printf("inside DpRxSs_VerticalBlankHandler\n\r");

	DpRxSsInst.VBlankCount++;
	if (DpRxSsInst.VBlankCount > 200) {
		//when Vblank is received, HDCP is put in enabled state and the
		// timer is started. TX is not setup until the timer is done.
		// This ensures that certain sources like MacBook gets
		//  time to Authenticate.
		#if ENABLE_HDCP_IN_DESIGN
				XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x01F80000);
				XDpRxSs_StartTimer(&DpRxSsInst);
		#endif

	} //end of (vblank_count >= 100)
	else if (DpRxSsInst.VBlankCount == 80)
	{

#if ENABLE_HDCP_IN_DESIGN
		XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x01F80000);
//		XDpRxSs_SetLane(&DpRxSsInst, DpRxSsInst.UsrOpt.LaneCount);
	    XDpRxSs_SetPhysicalState(&DpRxSsInst, hdcp_capable_org); //TRUE);
	    XHdcp1xExample_Poll();
	    XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
	    XHdcp1xExample_Poll();
		XHdcp1xExample_Enable();
		XHdcp1xExample_Poll();
#endif

	}
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
extern u32 appx_fs_dup;
u32 maud_dup = 0;
u32 naud_dup = 0;
extern u8 rx_trained;
void DpRxSs_TrainingLostHandler(void *InstancePtr)
{

#if ENABLE_HDCP_IN_DESIGN
	XDpRxSs_SetPhysicalState(&DpRxSsInst, FALSE);
	XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
	XHdcp1xExample_Poll();
	XDpTxSs_SetPhysicalState(&DpTxSsInst, FALSE);
	XHdcp1xExample_Poll();
	XDpRxSs_StopTimer(&DpRxSsInst);

	// This function will over write timer function pointer to be the right one.
	Dprx_HdcpUnAuthCallback((void *)&DpRxSsInst); 	// Added from 16.4 release

#endif
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
//	XDpRxSs_AudioDisable(&DpRxSsInst);
	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.VBlankCount = 0;
	appx_fs_dup = 0;
	rx_trained = 0;
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

	DpRxSsInst.link_up_trigger = 1;
	DpRxSsInst.VBlankCount = 0;
	rx_unplugged = 0;
#if ENABLE_HDCP_IN_DESIGN
//    XDpRxSs_SetLane(&DpRxSsInst, DpRxSsInst.UsrOpt.LaneCount);
    XDpRxSs_SetPhysicalState(&DpRxSsInst, hdcp_capable_org); //TRUE);
    XHdcp1xExample_Poll();
#endif
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
	appx_fs_dup = 0;
//	XDpRxSs_AudioDisable(&DpRxSsInst);
	AudioinfoFrame.frame_count = 0;
	SdpExtFrame.Header[1] = 0;
	SdpExtFrame_q.Header[1] = 0;
	SdpExtFrame.frame_count = 0;
	SdpExtFrame.frame_count = 0;

	/*Enable Training related interrupts*/
#if ENABLE_HDCP_IN_DESIGN
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFE00FFFF);
#else
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xF9FFFFFF /*0xFFF87FFF*/);
#endif
//	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
//			       XDP_RX_INTERRUPT_MASK_ALL_MASK);

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
	xil_printf("Cable unplugged2 %d %d %d\r\n", DpRxSsInst.link_up_trigger,
			                              DpRxSsInst.VBlankCount, rx_unplugged);
#if ENABLE_HDCP_IN_DESIGN
#if ENABLE_HDCP_FLOW_GUIDE
	XDpRxSs_HdcpDisable(&DpRxSsInst);
	XDpTxSs_HdcpDisable(&DpTxSsInst);
	XHdcp1xExample_Poll();
#endif
	XDpRxSs_SetPhysicalState(&DpRxSsInst, FALSE);
	XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
	XHdcp1xExample_Poll();
	XDpTxSs_SetPhysicalState(&DpTxSsInst, FALSE);
	XHdcp1xExample_Poll();
	XDpRxSs_StopTimer(&DpRxSsInst);
//	IsTxEncrypted = 0;
//	IsTxAuthenticated = 0;
#endif
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

#if PHY_COMP
	/*Enable all interrupts, except unplug */
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			0x79FFFFFF);

#else
	/*Enable all interrupts */
#if ENABLE_HDCP_IN_DESIGN
        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x81FFFFFF/*0x81FFFFFF*/);
#else
        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0xF9FFFFFF/*0x0007FFFF*/);
#endif


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


	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DPC_LINK_QUAL_CONFIG);

	//xil_printf("DpRxSs_AccessLinkQualHandler : 0x%x\r\n", ReadVal);

	/*Check for PRBS Mode*/
	if( (ReadVal&0x00000007) == XDP_RX_DPCD_LINK_QUAL_PRBS){
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
		XDpRxSs_MCDP6000_EnablePrbs7_Rx(XPAR_IIC_0_BASEADDR,
					I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(XPAR_IIC_0_BASEADDR,
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
		XDpRxSs_MCDP6000_DisablePrbs7_Rx(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(XPAR_IIC_0_BASEADDR,
				      I2C_MCDP6000_ADDR);
	//    	MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	}
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
	u16 DrpVal;
	u16 DrpVal_lower_lane0;
	u16 DrpVal_lower_lane1;
	u16 DrpVal_lower_lane2;
	u16 DrpVal_lower_lane3;
	u32 DrpVal1;

//	xil_printf("DpRxSs_AccessErrorCounterHandler\r\n");


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

	/*Write into DP Core - Validity bit and lower 15 bit counter value*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_DPC_L01_PRBS_CNTR,
		     (0x8000 | DrpVal_lower_lane0) |
		     ((0x8000 | DrpVal_lower_lane1) << 16));
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_DPC_L23_PRBS_CNTR,
		     (0x8000 | DrpVal_lower_lane2) |
		     ((0x8000 | DrpVal_lower_lane3) << 16));

	XDpRxSs_MCDP6000_Read_ErrorCounters(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);

	xil_printf("0x061C: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
			XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x061C));
	xil_printf("0x0504: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
			XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0504));
	xil_printf("0x0604: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
			XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0604));
	xil_printf("0x12BC: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
			XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x12BC));
	xil_printf("0x12E4: %08x\n\r", XDpRxSs_MCDP6000_GetRegister(
			XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x12E4));

	/* Reset PRBS7 Counters */
	DrpVal1 = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG);
	DrpVal1 = DrpVal1 | 0x08080808;
	XDp_WriteReg(VPhyInst.Config.BaseAddr,
		     XVPHY_RX_CONTROL_REG, DrpVal1);
	DrpVal1 = XVphy_ReadReg(VPhyInst.Config.BaseAddr,
			       XVPHY_RX_CONTROL_REG);
	DrpVal1 = DrpVal1 & 0xF7F7F7F7;
	XVphy_WriteReg(VPhyInst.Config.BaseAddr,
		       XVPHY_RX_CONTROL_REG, DrpVal1);
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
	int i=1;
	for(i = 1 ; i < 9 ; i++) {
		InfoFrame[i] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_INFO_DATA(i));
	}

	//storing the info frame here
			AudioinfoFrame.frame_count++;

			AudioinfoFrame.version = InfoFrame[1]>>26;
			AudioinfoFrame.type = (InfoFrame[1]>>8)&0xFF;
			AudioinfoFrame.sec_id = InfoFrame[1]&0xFF;
			AudioinfoFrame.info_length = (InfoFrame[1]>>16)&0x3FF;

			AudioinfoFrame.audio_channel_count = InfoFrame[2]&0x7;
			AudioinfoFrame.audio_coding_type = (InfoFrame[2]>>4)&0xF;
			AudioinfoFrame.sample_size = (InfoFrame[2]>>8)&0x3;
			AudioinfoFrame.sampling_frequency = (InfoFrame[2]>>10)&0x7;
			AudioinfoFrame.channel_allocation = (InfoFrame[2]>>24)&0xFF;

			AudioinfoFrame.level_shift = (InfoFrame[3]>>3)&0xF;
			AudioinfoFrame.downmix_inhibit = (InfoFrame[3]>>7)&0x1;
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
	u32 ExtFrame[9];
	int i=1;

	SdpExtFrame.frame_count++;

	/*Header Information*/
	ExtFrame[0] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_AUDIO_EXT_DATA(1));
	SdpExtFrame.Header[0] =  ExtFrame[0]&0xFF;
	SdpExtFrame.Header[1] = (ExtFrame[0]&0xFF00)>>8;
	SdpExtFrame.Header[2] = (ExtFrame[0]&0xFF0000)>>16;
	SdpExtFrame.Header[3] = (ExtFrame[0]&0xFF000000)>>24;

	/*Payload Information*/
	for (i = 0 ; i < 8 ; i++)
	{
		ExtFrame[i+1] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_EXT_DATA(i+2));
		SdpExtFrame.Payload[(i*4)]   =  ExtFrame[i+1]&0xFF;
		SdpExtFrame.Payload[(i*4)+1] = (ExtFrame[i+1]&0xFF00)>>8;
		SdpExtFrame.Payload[(i*4)+2] = (ExtFrame[i+1]&0xFF0000)>>16;
		SdpExtFrame.Payload[(i*4)+3] = (ExtFrame[i+1]&0xFF000000)>>24;
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
#if ENABLE_HDCP_IN_DESIGN
/* *********************************************************
 *
 * This function is assigned to callback on completion
 * of HDCP RX authentication.
 *
 * @param InstancePtr - DP RX SS HDCP core instance
 *
 * @return None.
 *
 */
void Dprx_HdcpAuthCallback(void *InstancePtr) {
	XDpRxSs *XDpRxSsInst = (XDpRxSs *)InstancePtr;

	/* Set Timer Counter reset done */
	XDpRxSsInst->TmrCtrResetDone = 1;
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
		XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
		XHdcp1xExample_Poll();
		XDpTxSs_HdcpEnable(&DpTxSsInst);
		XHdcp1xExample_Poll();
	}
}

static void Dppt_TimeOutCallback(void *InstancePtr, u8 TmrCtrNumber)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;
	/* Verify arguments.*/
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(TmrCtrNumber < XTC_DEVICE_TIMER_COUNT);

	/* Set Timer Counter reset done */
	XDpRxSsPtr->TmrCtrResetDone = 1;
}

/* *********************************************************
 *
 * This function is assigned to callback on
 * HDCP RX de- authentication.
 *
 * @param InstancePtr - DP RX SS HDCP core instance
 *
 * @return None.
 *
 */
void Dprx_HdcpUnAuthCallback(void *InstancePtr) {
	XDpRxSs *XDpRxSsInst = (XDpRxSs *)InstancePtr;

	/* Configure the callback */
	XTmrCtr_SetHandler(XDpRxSsInst->TmrCtrPtr,
			(XTmrCtr_Handler)Dppt_TimeOutCallback,
				InstancePtr);

}


#endif /* ENABLE_HDCP_IN_DESIGN */
