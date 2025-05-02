/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 KI  07/13/17 Initial release.
*
* </pre>
*
******************************************************************************/

#include "main.h"
#include "tx.h"

#define I2C_MUX_device_address 0x74
#define Si570_device_address 0x5D
#define audio_clk_Hz 24.576

/************************** Function Prototypes ******************************/

u32 DpTxSs_Main(void);

static u8 CalculateChecksum(u8 *Data, u8 Size);
static XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
static void clk_wiz_locked(void);

int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void ReportVideoCRC(void);
void mask_intr(void);

void bpc_help_menu(int DPTXSS_BPC_int);
void sub_help_menu(void);
void resolution_help_menu(void);
void select_link_lane(void);
void test_pattern_gen_help();
void format_help_menu(void);
void operationMenu(void);
char inbyte_local(void);

extern u8 tx_done;
extern volatile u8 hpd_pulse_con_event; 	/* This variable triggers hpd_pulse_con */
u8 linkrate_tx_run;
u8 lanecount_tx_run;
XDpTxSs DpTxSsInst; 		/* The DPTX Subsystem instance.*/
extern Video_CRC_Config VidFrameCRC_tx;
extern XTmrCtr TmrCtr; 		/* Timer instance.*/
int tx_started;
volatile u8 prev_line_rate; 		/* This previous line rate to keep
				 * previous info to compare
				 * with new line rate request*/

extern XVphy VPhyInst; 	/* The DPRX Subsystem instance.*/
extern XVphy_User_Config PHY_User_Config_Table[];
/************************** Variable Definitions *****************************/
#define DPCD_TEST_CRC_R_Cr   0x240
#define DPCD_TEST_SINK_MISC  0x246
#define DPCD_TEST_SINK_START 0x270
#define CRC_AVAIL_TIMEOUT    1000
/************************** Function Definitions *****************************/
extern lane_link_rate_struct lane_link_table[];

#ifdef Tx
/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* DisplayPort TX Subsystem core. The function is application-specific since
* the actual system may or may not have an interrupt controller. The DPTX
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
u32 DpTxSs_SetupIntrSystem(void)
{
	/* Set custom timer wait */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpPt_CustomWaitUs, &TmrCtr);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_EVENT),
						&DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_PULSE),
						&DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
						&DpPt_LinkrateChgHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_PE_VS_ADJUST),
						&DpPt_pe_vs_adjustHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_EXT_PKT_EVENT),
						&DpTxSs_ExtPacketHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_VSYNC),
						&DpTxSs_VsyncHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_FFE_PRESET_ADJUST),
			(void *)DpPt_ffe_adjustHandler, &DpTxSsInst);

	return (XST_SUCCESS);
}

u16 vsync_counter = 0;
u16 ektpkt_counter = 0;
u8 onetime = 0;
u8 prog_misc1 = 0;
u8 prog_fb_rd;
volatile int tx_is_reconnected=0;		/* This variable to keep track
				 * of the status of Tx link*/


void DpPt_ffe_adjustHandler(void *InstancePtr){

	u16 preemp = 0;
	u16 diff_swing = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PresetVal[0]){
		case 0:
			preemp = 0x5; //XVPHY_GTHE4_PREEMP_DP_L0;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
		case 1:
			preemp = XVPHY_GTHE4_PREEMP_DP_L1;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
		case 2:
			preemp = XVPHY_GTHE4_PREEMP_DP_L2;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
		case 3:
			preemp = XVPHY_GTHE4_PREEMP_DP_L3;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;
			break;
		default:
			preemp = XVPHY_GTHE4_PREEMP_DP_L3;
			diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_DP20;

			break;

		}

		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
				diff_swing);

		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, preemp);
}


void DpTxSs_VsyncHandler(void *InstancePtr)
{
	vsync_counter++;
}

/*****************************************************************************/
/**
*
* This function is the callback function for Generic Packet Handling of
* 32-Bytes payload.
*
* @param    InstancePtr is a pointer to the XDpTxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpTxSs_ExtPacketHandler(void *InstancePtr)
{
	/* This handler can be used to modify the extended pkt
	 * The packet is written in the DP TX drivers
	 */
	ektpkt_counter++;
}

/*****************************************************************************/
/**
*
* This function sets link line rate
*
* @param
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpPt_LinkrateChgHandler(void *InstancePtr)
{
	// If TX is unable to train at what it has been asked then
	// necessary down shift handling has to be done here
	// eg. reconfigure GT to new rate etc

    u8 rate;
	u8 lanes;
	rate = get_LineRate();
    lanes = XPAR_DP_TX_HIER_0_V_DP_TXSS2_1_LANE_COUNT;//get_Lanecounts();
	config_phy(rate);

	//update the previous link rate info at here
	prev_line_rate = rate;
}


void DpPt_pe_vs_adjustHandler(void *InstancePtr){
	if(PE_VS_ADJUST == 1){
		u16 preemp = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
			case 0:
				preemp = XVPHY_GTHE4_PREEMP_DP_L0;
				break;
			case 1:
				preemp = XVPHY_GTHE4_PREEMP_DP_L1;
				break;
			case 2:
				preemp = XVPHY_GTHE4_PREEMP_DP_L2;
				break;
			case 3:
				preemp = XVPHY_GTHE4_PREEMP_DP_L3;
				break;
		}

		u16 diff_swing = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel){
			case 0:
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
				case 0:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V0P0;
					break;
				case 1:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V0P1;
					break;
				case 2:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V0P2;
					break;
				case 3:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V0P3;
					break;
				}
				break;
			case 1:
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
				case 0:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V1P0;
					break;
				case 1:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V1P1;
					break;
				case 2:
				case 3:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V1P2;
					break;
				}
				break;
			case 2:
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
				case 0:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V2P0;
					break;
				case 1:
				case 2:
				case 3:
					diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V2P1;
					break;
				}
				break;
			case 3:
				diff_swing = XVPHY_GTHE4_DIFF_SWING_DP_V3P0;
				break;
		}

		//setting vswing
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
				diff_swing);

		//setting postcursor
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, preemp);
		XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, preemp);
	}
}



/*****************************************************************************/
/**
*
* This function use h/w timer to count specific Microseconds
*
* @param	pointer to timer
* @param	MicroSeconds to wait
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{

	u32 TimerVal = 0, TimerVal_pre;
	u32 NumTicks = (MicroSeconds *
		        (AXI_SYSTEM_CLOCK_FREQ_HZ / 1000000));

	XTmrCtr_Reset(&TmrCtr, 0);
	XTmrCtr_Start(&TmrCtr, 0);

	/* Wait specified number of useconds. */
	do {
		TimerVal_pre = TimerVal;
		TimerVal = XTmrCtr_GetValue(&TmrCtr, 0);
		if(TimerVal_pre == TimerVal){
			break;
		}
	} while (TimerVal < NumTicks);
}

/*****************************************************************************/
/**
*
* This function takes care HPD event
*
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpPt_HpdEventHandler(void *InstancePtr)
{
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
//		sink_power_down();
		sink_power_up();
		tx_is_reconnected = 1;
		xil_printf ("Cable Connected\r\n");
		onetime = 0;
	}
	else
	{
		tx_done = 0;
        xil_printf ("TX Cable Disconnected !!\r\n");
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);

		//DpTxSs_DisableAudio
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			     XDP_TX_AUDIO_CONTROL, 0x0);

		//on HPD d/c, it is important to bring down the HDCP
		tx_is_reconnected = 0;
		DpTxSsInst.no_video_trigger = 1;
	    DpPt_pe_vs_adjustHandler (&VPhyInst);
	}
}

/*****************************************************************************/
/**
*
* This function takes care HPD pulse interrupt
*
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpPt_HpdPulseHandler(void *InstancePtr)
{

// Some monitors give HPD pulse repeatedly which causes HPD pulse function to
//		be executed huge number of time. Hence hpd_pulse interrupt is disabled
//		and then enabled when hpd_pulse function is executed
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_INTERRUPT_MASK,
		     XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);
	hpd_pulse_con_event = 1;
	xil_printf ("HPD Pulse..\r\n");

}

/*****************************************************************************/
/**
*
* This function is the main hpd pulse process.
*
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void hpd_pulse_con(XDpTxSs *InstancePtr, XDpTxSs_MainStreamAttributes Msa[4])
{
	u8 lane0_sts = InstancePtr->UsrHpdPulseData.Lane0Sts;
	u8 lane2_sts = InstancePtr->UsrHpdPulseData.Lane2Sts;
	u8 laneAlignStatus = InstancePtr->UsrHpdPulseData.LaneAlignStatus;
	u8 bw_set = InstancePtr->UsrHpdPulseData.BwSet;
	u8 lane_set = InstancePtr->UsrHpdPulseData.LaneSet;

	u8 retrain_link=0;

	if (!XVidC_EdidIsHeaderValid(InstancePtr->UsrHpdEventData.EdidOrg)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr,
				DpTxSsInst.UsrHpdEventData.EdidOrg, 0);
	}

	u8 checksumMatch = 0;
	while (checksumMatch == 0) {
		if (CalculateChecksum(DpTxSsInst.UsrHpdEventData.EdidOrg, 128)) {
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr,
					DpTxSsInst.UsrHpdEventData.EdidOrg,0);
			checksumMatch = 0;
		}else
			checksumMatch = 1;
		}

	lane_set = lane_set & 0x1F;
	bw_set = bw_set & 0x1F;
	laneAlignStatus = laneAlignStatus & 0x1;

	if(bw_set != XDP_TX_LINK_BW_SET_162GBPS
			&& bw_set != XDP_TX_LINK_BW_SET_270GBPS
			&& bw_set != XDP_TX_LINK_BW_SET_540GBPS
			&& bw_set != XDP_TX_LINK_BW_SET_810GBPS
			&& bw_set != XDP_TX_LINK_BW_SET_UHBR10
			&& bw_set != XDP_TX_LINK_BW_SET_UHBR135
			&& bw_set != XDP_TX_LINK_BW_SET_UHBR20){
		//Not all MOnitors are capable of 1E
		bw_set = linkrate_tx_run;
		retrain_link = 1;
	}
	if(lane_set != XDP_TX_LANE_COUNT_SET_1
			&& lane_set != XDP_TX_LANE_COUNT_SET_2
			&& lane_set != XDP_TX_LANE_COUNT_SET_4){
		//Not all links are trained at 4
		lane_set = lanecount_tx_run;
		retrain_link = 1;
	}

	lane0_sts = lane0_sts & 0x77;
	lane2_sts = lane2_sts & 0x77;
	if (lane_set == XDP_TX_LANE_COUNT_SET_4) {
		if ((lane0_sts !=
				(XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK)
				) || (lane2_sts !=
				(XDP_DPCD_STATUS_LANE_2_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_2_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_2_SL_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_3_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_3_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_3_SL_DONE_MASK)
				) || (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	} else if (lane_set == XDP_TX_LANE_COUNT_SET_2) {
		if ((lane0_sts !=
				(XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK)
				) || (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	} else if (lane_set == XDP_TX_LANE_COUNT_SET_1) {
		lane0_sts = lane0_sts & 0x7;
		if ((lane0_sts !=
				(XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK)
				) || (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	}

	if (retrain_link == 1) {
//		sink_power_cycle();
		XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
		DpTxSubsystem_Start(&DpTxSsInst, Msa);
	}

	/* Mask Unsued Interrupts
	 *
	 */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0x0);
}


u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr,
			XDpTxSs_MainStreamAttributes Msa[4]) {
	u32 Status;
	if (Msa == 0) {
		Status = XDpTxSs_Start(&DpTxSsInst);
	} else {
		Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
	}
	return Status;
}

/*****************************************************************************/
/**
*
* This function sets up PHY
*
* @param	pointer to VideoPHY
* @param	User Config table
*
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
u32 PHY_Configuration_Tx(XVphy *InstancePtr,
		XVphy_User_Config PHY_User_Config_Table)
{
	XVphy_PllRefClkSelType QpllRefClkSel;
	XVphy_PllRefClkSelType CpllRefClkSel;
	XVphy_PllType TxPllSelect;
	XVphy_PllType RxPllSelect; // Required for VPHY setting
	XVphy_ChannelId TxChId;
	//XVphy_ChannelId RxChId;
	u8 QuadId = 0;
	u32 Status = XST_FAILURE;
	u32 retries = 0;

	QpllRefClkSel = PHY_User_Config_Table.QPLLRefClkSrc;
	CpllRefClkSel = PHY_User_Config_Table.CPLLRefClkSrc;
	TxPllSelect   = PHY_User_Config_Table.TxPLL;
	// Required for VPHY setting
	RxPllSelect = PHY_User_Config_Table.RxPLL;
	TxChId      = PHY_User_Config_Table.TxChId;

			//Set the Ref Clock Frequency
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, QpllRefClkSel,
				PHY_User_Config_Table.QPLLRefClkFreqHz);
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, CpllRefClkSel,
				PHY_User_Config_Table.CPLLRefClkFreqHz);
	XVphy_CfgLineRate(InstancePtr, QuadId, TxChId,
			  PHY_User_Config_Table.LineRateHz);

	XVphy_PllInitialize(InstancePtr, QuadId, TxChId,
					QpllRefClkSel, CpllRefClkSel, TxPllSelect, RxPllSelect);

	// Initialize GT with ref clock and PLL selects
	// GT DRPs may not get completed if GT is busy doing something else
	// hence this is run in loop and retried 100 times
	while (Status != XST_SUCCESS) {
		Status = XVphy_ClkInitialize(InstancePtr, QuadId,
					     TxChId, XVPHY_DIR_TX);
		if (retries > 100) {
			retries = 0;
			xil_printf ("exhausted\r\n");
			break;
		}
		retries++;
	}

	XVphy_WriteReg(InstancePtr->Config.BaseAddr,
			XVPHY_PLL_RESET_REG,
			XVPHY_PLL_RESET_QPLL1_MASK); // 0x06

	XVphy_WriteReg(InstancePtr->Config.BaseAddr,
			XVPHY_PLL_RESET_REG, 0x0);

	XVphy_ResetGtPll(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,(FALSE));


	Status = XVphy_WaitForPmaResetDone(InstancePtr, QuadId,
			TxChId, XVPHY_DIR_TX);
	if (Status  != XST_SUCCESS) {
		xil_printf ("++++rst failed error++++\r\n");
	}

	Status += XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);
	if (Status  != XST_SUCCESS) {
		xil_printf ("++++lock failed++++\r\n");
	}

	Status += XVphy_WaitForResetDone(InstancePtr, QuadId,
			TxChId, XVPHY_DIR_TX);

	if (Status  != XST_SUCCESS) {
		xil_printf ("++++TX GT config encountered error++++\r\n");
	}

	return (Status);

}

/*****************************************************************************/
/**
*
* This function starts tx process
*
* @param	line rate
* @param	lane counts
* @param	pointer to resolution table
* @param	bit per components
* @param	video pattern to output
*
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
u32 start_tx(u8 line_rate, u8 lane_count, user_config_struct user_config,
			XDpTxSs_MainStreamAttributes Msa_test[4])
{
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;
	u8 format = user_config.user_format-1;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate
	u8 i = 0;
	u32 Status;
	lanecount_tx_run = lane_count;
	linkrate_tx_run = line_rate;
	//Disabling TX interrupts
    tx_started = 0;

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			0x140);

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);
	//Waking up the monitor
	sink_power_cycle();

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	// Give a bit of time for DP IP after monitor came up and starting Link training
	usleep(100000);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

	xil_printf("\r\nTraining TX with: Link rate %x, Lane %d, BPC %d\r\n",
		   line_rate,lane_count, bpc);

	Status = XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);
	if(Status != XST_SUCCESS){
		xil_printf("XDpTxSs_SetLinkRate failure\r\n");
	}
	xil_printf (".");
	XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);
	xil_printf (".");

	// SetVidMode and SetBPC are not required in PassThrough
	// In Passthrough these values are captured when
	// DpTxSubsystem_Start is called with Msa values
	if (res_table !=0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, res_table);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR: Setting resolution failed\r\n");
		}else{
//			xil_printf("XDpTxSs_SetVidMode successful\r\n");
		}
		xil_printf (".");
	}
	if (bpc !=0 ) {
		Status = XDpTxSs_SetBpc(&DpTxSsInst, bpc);
		if (Status != XST_SUCCESS){
			xil_printf("ERR: Setting bpc to %d failed\r\n",bpc);
		}
	}

	/*
	 * Setting Color Format
	 * User can change coefficients here - By default 601 is used for YCbCr
	 * */
	if (format == 1){
		format = XVIDC_CSF_YCRCB_422;
	}else if(format == 2){
		format = XVIDC_CSF_YCRCB_444;
	}
	XDp_TxCfgSetColorEncode(DpTxSsInst.DpPtr, XDP_TX_STREAM_ID1, \
				format, XVIDC_BT_601, XDP_DR_VESA);

	// VTC requires linkup(video clk) before setting values.
	// This is why we need to linkup once to get proper CLK on VTC.
//	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa_test);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
			"DP SS Start setup failed!\n\r");
	}

	xil_printf (".");

	Gen_vid_clk(DpTxSsInst.DpPtr,(XDP_TX_STREAM_ID1));
	xil_printf (".");

	xil_printf (".");
	clk_wiz_locked();

	// Over writing the VTC, so that we no longer need to start tx twice
	if (DpTxSsInst.VtcPtr[0]) {
		Status = XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[0],
		&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0],
		DpTxSsInst.UsrOpt.VtcAdjustBs);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
				"VTC setup failed!\n\r");
		}
	}

	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		xil_printf ("*");
		XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);
		Status = DpTxSubsystem_Start(&DpTxSsInst, Msa_test);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}
	/* Mask unsused interrupts
	 *
	 */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0x0);

	vsync_counter = 0;

	/*
	 * Initialize CRC
	 */
	/* Reset CRC*/
	XVidFrameCrc_Reset(&VidFrameCRC_tx);
	/* Set Pixel width in CRC engine*/
	if (format != 2) {
		XVidFrameCrc_WriteReg(VidFrameCRC_tx.Base_Addr,
				  VIDEO_FRAME_CRC_CONFIG,
				  5);
	} else { // 422
		XVidFrameCrc_WriteReg(VidFrameCRC_tx.Base_Addr,
				  VIDEO_FRAME_CRC_CONFIG,
					5 | 0x80000000);

	}

	xil_printf ("..done !\r\n");
	tx_started = 1;

	int lr,lc=0;

	/* Read Link rate over through channel */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LINK_BW_SET, 1, &lr);


	/* Read Lane count through AUX channel */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LANE_COUNT_SET, 1,
			&lc);

	xil_printf("Tx trained at %x x %x\r\n",(lr & 0xFF),(lc & XDP_DPCD_LANE_COUNT_SET_MASK));
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function to check MMCM lock status
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void clk_wiz_locked(void) {
#ifndef SDT
	volatile u32 res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
#else
volatile u32 res = XDp_ReadReg(XPAR_XGPIO_0_BASEADDR,0x0);

#endif
	u32 timer=0;

	while(res == 0 && timer < 1000) {
//		xil_printf ("~/~/");
#ifndef SDT
		res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
#else
		res = XDp_ReadReg(XPAR_XGPIO_0_BASEADDR,0x0);
#endif
		timer++; // timer for timeout. No need to be specific time.
					// As long as long enough to wait lock
	}
	xil_printf ("^^");
}


/*****************************************************************************/
/**
*
* This function is called when a Hot-Plug-Detect (HPD) event is received by
* the DisplayPort TX Subsystem core.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetCallback driver function to set this
*		function as the handler for HPD event.
*
******************************************************************************/

void hpd_con(XDpTxSs *InstancePtr, u8 Edid_org[128],
		u8 Edid1_org[128], u16 res_update)
{
//	u8 rData;
//	u32 Status;

	/* This is a PassThrough System to display the Video received on RX
	 * onto TX. There is nothing special done in the hpd_connect handler.
	 * On a HPD the application simply tries to retrain the monitor
	 */
}

/*****************************************************************************/
/**
*
* This function to calculate EDID checksum
*
* @param	edid data
* @param	size of the edid data
*
* @return	checksum number
*
* @note		None.
*
******************************************************************************/
static u8 CalculateChecksum(u8 *Data, u8 Size)
{
	u8 Index;
	u8 Sum = 0;

	for (Index = 0; Index < Size; Index++) {
		Sum += Data[Index];
	}

	return Sum;
}


/*****************************************************************************/
/**
*
* This function to find out preferred Video Mode ID
*
* @param	pointer to edid data
* @param	maximum capability of line speed
* @param	number of lane
*
* @return	Video Mode ID
*
* @note		None.
*
******************************************************************************/
static XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane)
{
	u8 *Ptm;
	u16 HBlank;
	u16 VBlank;
	u32 PixelClockHz;
	XVidC_FrameRate FrameRate;
	XVidC_VideoTiming Timing;
	XVidC_VideoMode VmId;
	u8 bpp;
	double pixel_freq, pixel_freq1 = 0;
	double max_freq[] = {216.0, 172.8, 360.0, 288.0,
				720.0, 576.0, 1440, 1152};

	(void)memset((void *)&Timing, 0, sizeof(XVidC_VideoTiming));

	Ptm = &EdidPtr[XDP_EDID_PTM];

	bpp = XVidC_EdidGetColorDepth(EdidPtr) * 3;

	HBlank = ((Ptm[XDP_EDID_DTD_HRES_HBLANK_U4] &
		   XDP_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
		 Ptm[XDP_EDID_DTD_HBLANK_LSB];

	VBlank = ((Ptm[XDP_EDID_DTD_VRES_VBLANK_U4] &
		   XDP_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
		 Ptm[XDP_EDID_DTD_VBLANK_LSB];

	Timing.HActive = (((Ptm[XDP_EDID_DTD_HRES_HBLANK_U4] &
			    XDP_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
			  XDP_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			 Ptm[XDP_EDID_DTD_HRES_LSB];

	Timing.VActive = (((Ptm[XDP_EDID_DTD_VRES_VBLANK_U4] &
			    XDP_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
			  XDP_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			 Ptm[XDP_EDID_DTD_VRES_LSB];

	PixelClockHz = (((Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_MSB] <<
					8) | Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_LSB]) * 10) * 1000;

	Timing.HFrontPorch = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
				XDP_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK) >>
			       XDP_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT) << 8) |
			     Ptm[XDP_EDID_DTD_HFPORCH_LSB];

	Timing.HSyncWidth = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
			       XDP_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK) >>
			      XDP_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT) << 8) |
			    Ptm[XDP_EDID_DTD_HSPW_LSB];

	Timing.F0PVFrontPorch = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
				   XDP_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK) >>
				  XDP_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT) << 8) |
				((Ptm[XDP_EDID_DTD_VFPORCH_VSPW_L4] &
				  XDP_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK) >>
				 XDP_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT);

	Timing.F0PVSyncWidth = ((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
				 XDP_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK) << 8) |
			       (Ptm[XDP_EDID_DTD_VFPORCH_VSPW_L4] &
			        XDP_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK);

	/* Compute video mode timing values. */
	Timing.HBackPorch = HBlank - (Timing.HFrontPorch + Timing.HSyncWidth);
	Timing.F0PVBackPorch = VBlank - (Timing.F0PVFrontPorch +
					 Timing.F0PVSyncWidth);
	Timing.HTotal = (Timing.HSyncWidth + Timing.HFrontPorch +
			 Timing.HActive + Timing.HBackPorch);
	Timing.F0PVTotal = (Timing.F0PVSyncWidth + Timing.F0PVFrontPorch +
			    Timing.VActive + Timing.F0PVBackPorch);
	FrameRate = PixelClockHz / (Timing.HTotal * Timing.F0PVTotal);


	/* Few monitors returns 59 HZ. Hence, setting to 60. */
	if (FrameRate == 59) {
		FrameRate = 60;
	}

	pixel_freq = (FrameRate * Timing.HTotal * Timing.F0PVTotal) / 1000000.0;

	switch (cap) {
	case XDP_TX_LINK_BW_SET_162GBPS:
		if (bpp == 24) {
			pixel_freq1 = max_freq[0];
		} else {
			pixel_freq1 = max_freq[1];
		}
		break;
	case XDP_TX_LINK_BW_SET_270GBPS:
		if (bpp == 24) {
			pixel_freq1 = max_freq[2];
		} else {
			pixel_freq1 = max_freq[3];
		}
		break;
	case XDP_TX_LINK_BW_SET_540GBPS:
		if (bpp == 24) {
			pixel_freq1 = max_freq[4];
		} else {
			pixel_freq1 = max_freq[5];
		}
		break;
	case XDP_TX_LINK_BW_SET_810GBPS:
		if (bpp == 24) {
			pixel_freq1 = max_freq[4];
		} else {
			pixel_freq1 = max_freq[5];
		}
		break;
	}

	switch (lane) {
	case 0x1:
		pixel_freq1 = pixel_freq1/4.0;
		break;

	case 0x2:
		pixel_freq1 = pixel_freq1/2.0;
		break;

	case 0x4:
		pixel_freq1 = pixel_freq1;
		break;

	default:
		break;
	}
	if (pixel_freq1 < pixel_freq) {
		VmId = XVIDC_VM_NOT_SUPPORTED;
	} else {
		/* Get video mode ID */
		VmId = XVidC_GetVideoModeId(Timing.HActive, Timing.VActive,
					    FrameRate,
					    XVidC_EdidIsDtdPtmInterlaced(EdidPtr));

	}

	return VmId;
}


/*****************************************************************************/
/**
*
* This function powers down sink
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void sink_power_down(void){
	u8 Data[8];
	Data[0] = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr,
		       XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

/*****************************************************************************/
/**
*
* This function powers down sink
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void sink_power_up(void){
	u8 Data[8];
	Data[0] = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr,
		       XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

/*****************************************************************************/
/**
*
* This function returns current line rate
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u8 get_LineRate(void){
//	return DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
	return XDpTxSs_GetLinkRate(&DpTxSsInst);
}

/*****************************************************************************/
/**
*
* This function returns current lane counts
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u8 get_Lanecounts(void){
	return DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
}

/*****************************************************************************/
/**
*
* This function sets VPHY based on the linerate
*
* @param	user_config_struct.
*
* @return	Status.
*
* @note		None.
*
******************************************************************************/
u32 config_phy(int LineRate_init_tx){
	u32 Status=XST_SUCCESS;
	u8 linerate;

	//Tx on QPLL1 doesnt support 13.5g
	 if(LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR135){
		xil_printf("Tx on QPLL1 doesnt support 13.5G hence downshifting the linkrate\r\n");
		return XST_FAILURE;
	 }

	if (LineRate_init_tx == XDP_TX_LINK_BW_SET_810GBPS) {
		PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?9:10].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_540GBPS) {
		PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?2:5].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_270GBPS) {
		PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?1:4].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_162GBPS) {
		PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?0:3].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR10) {
	PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?11:12].LineRate);
	} else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR135) {
	PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?13:14].LineRate);
	}
	else if (LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR20) {
	PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[(is_TX_CPLL)?15:16].LineRate);
	}

    if ((LineRate_init_tx == 0x1E) ||
             (LineRate_init_tx == 0x14) ||
             (LineRate_init_tx == 0x0A) ||
             (LineRate_init_tx == 0x06) ) {

             XVphy_SetupDP21Phy (&VPhyInst, 0, XVPHY_CHANNEL_ID_CMN1,
                              XVPHY_DIR_TX, LineRate_init_tx, ONBOARD_REF_CLK,
                              XVPHY_PLL_TYPE_QPLL1);
    } else {
             XVphy_SetupDP21Phy (&VPhyInst, 0,XVPHY_CHANNEL_ID_CMN1,
                              XVPHY_DIR_TX, LineRate_init_tx, ONBOARD_400_CLK,
							  XVPHY_PLL_TYPE_QPLL1);

    }

	Status = XVphy_DP21PhyReset (&VPhyInst, 0, XVPHY_CHANNEL_ID_CMN1,
				XVPHY_DIR_TX);
	if (Status == XST_FAILURE) {
			xil_printf ("Issue encountered in PHY config and reset\r\n");
	}

	if (Status != XST_SUCCESS) {
		xil_printf (
   "+++++++ TX GT configuration encountered a failure config+++++++\r\n");
	}
	return Status;
}


/*****************************************************************************/
/**
*
* This function power cycle the sink
*
* @param	user_config_struct.
*
* @return	Status.
*
* @note		None.
*
******************************************************************************/
void sink_power_cycle(void){
	//Waking up the monitor
//	sink_power_down();
	// give enough time for monitor to power down
	usleep(4000);
	sink_power_up();
//	// give enough time for monitor to wake up    CR-962717
	usleep(50000);
//	usleep(50000);
	sink_power_up();//monitor to wake up once again due to CR-962717
	usleep(40000);
//	usleep(40000);
}

/*****************************************************************************/
/**
*
* This function masks the TX interrupts
*
* @param	user_config_struct.
*
* @return	Status.
*
* @note		None.
*
******************************************************************************/


void mask_intr (void) {

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);

}
#endif
