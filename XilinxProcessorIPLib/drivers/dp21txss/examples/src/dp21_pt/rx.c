/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

#ifdef Rx
extern XDpRxSs DpRxSsInst;    /* The DPRX Subsystem instance.*/
extern XDpTxSs DpTxSsInst; 		/* The DPTX Subsystem instance.*/
volatile u8 rx_unplugged = 0;
volatile u16 DrpVal=0;
volatile u16 DrpVal_lower_lane0=0;
volatile u16 DrpVal_lower_lane1=0;
volatile u16 DrpVal_lower_lane2=0;
volatile u16 DrpVal_lower_lane3=0;
extern u32 vblank_init;
extern u8 vblank_captured;
extern volatile u8 rx_trained;
extern u8 tx_after_rx;
extern u8 Video_valid;
extern u8 tx_done;
extern void mask_intr (void);
extern Video_CRC_Config VidFrameCRC_rx; /* Video Frame CRC instance */
extern XTmrCtr TmrCtr; 		/* Timer instance.*/
DP_Rx_Training_Algo_Config RxTrainConfig;
extern XVphy VPhyInst; 	/* The DPRX Subsystem instance.*/

u32 hdrframe[9];
u16 fifoOverflow=0;

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

	//changes for mst
	ReadVal= XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     0xD0);
	ReadVal = ReadVal & (0xFFFFFFFE);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			     0xD0, ReadVal);

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
			XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK|
			  XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK |
			 XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK |
			 XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK |
			 XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK);

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
	/* Set callbacks for all the interrupts */
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_DRV_HANDLER_DP_VID_EVENT,
			    &DpRxSs_VideoValidHandler, &DpRxSsInst);
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
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,
            &Dprx_InterruptHandlerDownReq, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_DWN_REP_EVENT,
            &Dprx_InterruptHandlerDownReply, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,
            &Dprx_InterruptHandlerPayloadAlloc, &DpRxSsInst);
    XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_ACT_RX_EVENT,
            &Dprx_InterruptHandlerActRx, &DpRxSsInst);
	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

	return (XST_SUCCESS);
}

void Dprx_InterruptHandlerPayloadAlloc(void *InstancePtr)
{
	/* Virtual Channel Payload allocation,
	 * de-allocation and partial deletion handler
	 */
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;
	u32 RegVal = 0;
	XDp *DpPtr = DpRxSsPtr->DpPtr;
	RegVal = XDp_ReadReg(DpPtr->Config.BaseAddr, XDP_RX_MST_ALLOC);
	XDp_RxAllocatePayloadStream(DpPtr);
}

void Dprx_InterruptHandlerActRx(void *InstancePtr)
{
        /* ACT Receive Interrupt Handler */
    XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
                    XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

    XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
                    0x3FFFF);
}


void Dprx_InterruptHandlerDownReq(void *InstancePtr)
{
	/* Down Request Buffer Ready handler
	 * (Indicates the availability of the Down request)
	 */
	XDp_RxHandleDownReq(DpRxSsInst.DpPtr);
}

void Dprx_InterruptHandlerDownReply(void *InstancePtr)
{
	/* Down Reply Buffer Read handler (indicates a
	 * read event from down reply buffer by upstream source)
	 */

	/* Increment the DownRequest Counter (if any) */
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
void DpRxSs_VideoValidHandler(void *InstancePtr)
{
	Video_valid = 1;
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
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_EXT_PKT_MASK);

	Video_valid = 0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_VIDEO_MASK);
	tx_done=0;
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
	Video_valid = 0;

	/* Reset CRC Test Counter in DP DPCD Space */
	XVidFrameCrc_Reset(&VidFrameCRC_rx);
	VidFrameCRC_rx.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			 XDP_RX_CRC_CONFIG,
			 (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
					 VidFrameCRC_rx.TEST_CRC_CNT));

	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
	XDpRxSs_AudioDisable(&DpRxSsInst);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
#ifdef Tx
	// mask the TX interrupts to avoid spurious HPDs
	mask_intr();
#endif
	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.VBlankCount = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
	tx_done = 0;
	tx_after_rx = 0;
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
{;
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

	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_VIDEO_MASK);
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
	Video_valid=0;
	xil_printf("Rx cable unplugged\r\n");
	/* Disable & Enable Audio */
	rx_unplugged = 1;

#ifdef Tx
	// mask the TX interrupts to avoid spurious HPDs
	mask_intr();
#endif

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
	/*Program Video PHY to requested line rate*/
	PLLRefClkSel (&VPhyInst, DpRxSsInst.UsrOpt.LinkRate);

    if ((DpRxSsInst.UsrOpt.LinkRate == 0x1E) ||
              (DpRxSsInst.UsrOpt.LinkRate == 0x14) ||
              (DpRxSsInst.UsrOpt.LinkRate == 0x0A) ||
              (DpRxSsInst.UsrOpt.LinkRate == 0x06) ) {

            XVphy_SetupDP21Phy (&VPhyInst, 0, XVPHY_CHANNEL_ID_CMN0,
                    XVPHY_DIR_RX, DpRxSsInst.UsrOpt.LinkRate, ONBOARD_REF_CLK,
                    XVPHY_PLL_TYPE_QPLL0);
    } else {
            XVphy_SetupDP21Phy (&VPhyInst, 0, XVPHY_CHANNEL_ID_CMN0,
                    XVPHY_DIR_RX, DpRxSsInst.UsrOpt.LinkRate, ONBOARD_400_CLK,
					XVPHY_PLL_TYPE_QPLL0);

    }

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
	u32 ReadVal,Status=0;
	/*Enable AUX Defers for training purpose*/
    ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                   XDP_RX_AUX_CLK_DIVIDER);
    ReadVal = ReadVal & 0xF0FF00FF;
    ReadVal = ReadVal | (AUX_DEFER_COUNT<<24);
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                   XDP_RX_AUX_CLK_DIVIDER, ReadVal);

    Status = XVphy_DP21PhyReset (&VPhyInst, 0, XVPHY_CHANNEL_ID_CMN0,
                XVPHY_DIR_RX);
    if (Status == XST_FAILURE) {
        xil_printf ("Rx Issue encountered in PHY config and reset\r\n");
    }

	/*Enable all interrupts */
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
void DpRxSs_BWChangeHandler(void *InstancePtr)
{

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
	}

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

	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DPC_LINK_QUAL_CONFIG);

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
    }
#endif
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

}
#endif
