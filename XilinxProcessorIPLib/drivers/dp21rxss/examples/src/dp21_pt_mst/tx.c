/******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.6   GM    03/06/26  Initial release.
*
* </pre>
*
******************************************************************************/
#include "main.h"
#include "tx.h"

#define I2C_MUX_device_address	0x74
#define Si570_device_address	0x5D
#define audio_clk_Hz		24.576

/************************** Function Prototypes ******************************/
u32 DpTxSs_Main(void);

static u8 CalculateChecksum(u8 *Data, u8 Size);
static XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
static void clk_wiz_locked(void);
void mask_intr(void);

void Dp21RxSs_PtMst_BpcHelpMenu(int DPTXSS_BPC_int);
void Dp21RxSs_PtMst_SubHelpMenu(void);
void Dp21RxSs_PtMst_ResolutionHelpMenu(void);
void Dp21RxSs_PtMst_SelectLinkLane(void);
void Dp21RxSs_PtMst_TestPatternGenHelp();
void format_help_menu(void);
char Dp21RxSs_PtMst_InByteLocal(void);

extern u8 tx_done;
extern volatile u8 hpd_pulse_con_event;		/**< This variable triggers hpd_pulse_con */
u8 linkrate_tx_run;
u8 lanecount_tx_run;
XDpTxSs DpTxSsInst;				/**< The DPTX Subsystem instance.*/
extern Video_CRC_Config VidFrameCRC_tx;
extern Video_CRC_Config VidFrameCRC_tx_1;
extern Video_CRC_Config VidFrameCRC_tx_2;
extern Video_CRC_Config VidFrameCRC_tx_3;
extern XTmrCtr TmrCtr;				/**< Timer instance.*/
int tx_started;
volatile u8 prev_line_rate;			/*
						 * This previous line rate to keep
						 * previous info to compare
						 * with new line rate request
						 */

extern XVphy VPhyInst;				/**< The DPRX Subsystem instance.*/
extern XAxis_Switch axis_switch;

u8 num_sinks = 0;
extern volatile u8 Tx_only;
/************************** Variable Definitions *****************************/
#define DPCD_TEST_CRC_R_Cr   0x240
#define DPCD_TEST_SINK_MISC  0x246
#define DPCD_TEST_SINK_START 0x270
#define CRC_AVAIL_TIMEOUT    1000
/************************** Function Definitions *****************************/
extern lane_link_rate_struct lane_link_table[];

extern XVphy_PllType VPHY_TX_PLL_TYPE;
extern XVphy_ChannelId VPHY_TX_CHANNEL_TYPE;
XVidC_VideoMode resolution_table[] =
{
	XVIDC_VM_640x480_60_P,
	XVIDC_VM_480_60_P,
	XVIDC_VM_800x600_60_P,
	XVIDC_VM_1024x768_60_P,
	XVIDC_VM_720_60_P,
	XVIDC_VM_1600x1200_60_P,
	XVIDC_VM_1366x768_60_P,
	XVIDC_VM_1080_60_P
};

u16 vsync_counter = 0;
u16 ektpkt_counter = 0;
u8 onetime = 0;
u8 prog_misc1 = 0;
u8 prog_fb_rd;
volatile int tx_is_reconnected = 0;	/**< This variable to keep track of
					 * the status of Tx link */
extern u8 SinkMaxLinkrate;
extern u8 SinkMaxLanecount;
u8 edid_monitor[384];

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
	/*
	 * Set custom timer wait
	 */
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

u32 get_max_capabilities (XDpTxSs *InstancePtr)
{
	u8 SinkCap[16];
	u8 SinkExtendedCap[16];
	u8 SinkMaxLanecount=0;
	u8 SinkMaxLinkrate=0;
	u16 max_link_lane = 0;
	u8 config_128_132b;
	u8 support_20g;
	u32 Status = XST_SUCCESS;

	Status = XDpTxSs_GetSinkCapabilities(InstancePtr, SinkCap, SinkExtendedCap,&config_128_132b);
	if(Status != XST_SUCCESS){
		xil_printf("Error in reading Rx Cap\r\n");
		return XST_FAILURE;
	}

	if(SinkCap[XDP_DPCD_TRAIN_AUX_RD_INTERVAL] &
	   XDP_DPCD_TRAIN_AUX_RD_EXT_RX_CAP_FIELD_PRESENT_MASK) {
		if(SinkExtendedCap[XDP_DPCD_ML_CH_CODING_CAP] &
		   XDP_TX_MAIN_LINK_CHANNEL_CODING_SET_128B_132B_MASK) {
			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
					       XDP_DPCD_128B_132B_SUPPORTED_LINK_RATE,
					       1, &SinkMaxLinkrate);
			support_20g = (SinkMaxLinkrate & 0x7);
			if ((support_20g == 0x7) || (support_20g == 0x3) || (support_20g == 0x6) || (support_20g == 0x2)) {
				SinkMaxLinkrate=XDP_TX_LINK_BW_SET_UHBR20;
			} else if ((support_20g == 0x4) || (support_20g == 0x5)) {
				SinkMaxLinkrate=XDP_TX_LINK_BW_SET_UHBR135;
			} else if (support_20g == 0x1) {
				SinkMaxLinkrate=XDP_TX_LINK_BW_SET_UHBR10;
			}
			SinkMaxLanecount= SinkExtendedCap[XDP_DPCD_MAX_LANE_COUNT] & XDP_DPCD_MAX_LANE_COUNT_MASK;
		} else {
			SinkMaxLinkrate = SinkExtendedCap[XDP_DPCD_MAX_LINK_RATE];
			SinkMaxLanecount= SinkExtendedCap[XDP_DPCD_MAX_LANE_COUNT] & XDP_DPCD_MAX_LANE_COUNT_MASK;
		}

	} else {
		SinkMaxLinkrate = SinkCap[XDP_DPCD_MAX_LINK_RATE];
		SinkMaxLanecount= SinkCap[XDP_DPCD_MAX_LANE_COUNT] & XDP_DPCD_MAX_LANE_COUNT_MASK;
	}

	max_link_lane = (SinkMaxLinkrate << 8) | (SinkMaxLanecount);

	return max_link_lane;
}

void DpPt_ffe_adjustHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u16 preemp = 0;
	u16 diff_swing = 0;

	switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PresetVal[0]) {
		case 0:
			preemp = 0x5; /**< XVPHY_GTHE4_PREEMP_DP_L0 */
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

void Read_EDID (void *InstancePtr, u8 enable_print)
{
	(void)InstancePtr;
	u8 Edid_org[128];
	u8 Edid1_org[128];
	u8 Edid2_org[128];
	int i;

	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);

		u8 Sum = 0;
		for (int i = 0; i < 128; i++) {
			Sum += Edid_org[i];
		}
		if(Sum != 0) {
			xil_printf("Wrong EDID was read\r\n");
		}

		/*
		 * Reading the second block of EDID
		 */
		if(Edid_org[126] > 0)
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
		if(Edid_org[126] > 1)
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid2_org, 2);

		xil_printf("Reading EDID contents of the DP Monitor..\r\n");

		switch (Edid_org[126]) {
			case 0:
				for(i=0; i<128; i++)
					edid_monitor[i] = Edid_org[i];
				for(i=0; i<128; i++)
					edid_monitor[i+128] = 0;
				for(i=0; i<128; i++)
					edid_monitor[i+256] = 0;
				break;
			case 1:
				for(i=0; i<128; i++)
					edid_monitor[i] = Edid_org[i];
				for(i=0; i<128; i++)
					edid_monitor[i+128] = Edid1_org[i];
				for(i=0; i<128; i++)
					edid_monitor[i+256] = 0;
				break;
			case 2:
				for(i=0; i<128; i++)
					edid_monitor[i] = Edid_org[i];
				for(i=0; i<128; i++)
					edid_monitor[i+128] = Edid1_org[i];
				for(i=0; i<128; i++)
					edid_monitor[i+256] = Edid2_org[i];
				break;
		}
	}

	if (enable_print) {
		for (i=0;i<128;i++) {
			if(i%16==0 && i != 0)
				xil_printf("\r\n");
			xil_printf ("%02x ", Edid_org[i]);
		}
		xil_printf("\r\n");

		if (Edid_org[126] > 0) {
			for (i=0;i<128;i++) {
				if(i%16==0 && i != 0)
					xil_printf("\r\n");
				xil_printf ("%02x ", Edid1_org[i]);
			}
		}
		xil_printf("\r\n");

		if(Edid_org[126] > 1) {
			for (i=0;i<128;i++) {
				if(i%16==0 && i != 0)
					xil_printf("\r\n");
				xil_printf ("%02x ", Edid2_org[i]);
			}
		}
	}
}

void DpTxSs_VsyncHandler(void *InstancePtr)
{
	(void)InstancePtr;
	vsync_counter++;
}

/*****************************************************************************/
/**
*
* This function is the callback function for Generic Packet Handling of
* 32-Bytes payload.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_ExtPacketHandler(void *InstancePtr)
{
	(void)InstancePtr;
	/*
	 * This handler can be used to modify the extended pkt
	 * The packet is written in the DP TX drivers
	 */
	ektpkt_counter++;
}

/*****************************************************************************/
/**
*
* This function sets link line rate
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpPt_LinkrateChgHandler(void *InstancePtr)
{
	(void)InstancePtr;
	/*
	 * If TX is unable to train at what it has been asked then
	 * necessary down shift handling has to be done here
	 * eg. reconfigure GT to new rate etc
	 */
	u32 Status = XST_SUCCESS;
	u8 rate;

	rate = get_LineRate();
	Status = Dp21RxSs_PtMst_PhyConfig (&VPhyInst, rate, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
	if (Status != XST_SUCCESS) {
		xil_printf ("PHY Config failed during the TX training\r\n");
	}

	/*
	 * Update the previous link rate info at here
	 */
	prev_line_rate = rate;
}

void DpPt_pe_vs_adjustHandler(void *InstancePtr)
{
	(void)InstancePtr;
	if (PE_VS_ADJUST == 1) {
		u16 preemp = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel) {
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
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel) {
			case 0:
				switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel) {
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

		/*
		 * Setting vswing
		 */
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
					diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
					diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
					diff_swing);

		/*
		 * Setting postcursor
		 */
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
* @param	InstancePtr is a pointer of void type.
* @param	MicroSeconds to wait
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
	(void)InstancePtr;
	u32 TimerVal = 0;
	u32 TimerVal_pre;
	u32 NumTicks = (MicroSeconds *
		        (AXI_SYSTEM_CLOCK_FREQ_HZ / 1000000));

	XTmrCtr_Reset(&TmrCtr, 0);
	XTmrCtr_Start(&TmrCtr, 0);

	/*
	 * Wait specified number of useconds.
	 */
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
* @param	InstancePtr is a pointer of void type.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/

void DpPt_HpdEventHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u16 max_link_lane_hpd;
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		sink_power_up();
		tx_is_reconnected = 1;
		xil_printf ("Cable Connected\r\n");
		max_link_lane_hpd = get_max_capabilities (&DpTxSsInst);
		SinkMaxLanecount = max_link_lane_hpd;
		SinkMaxLinkrate = (max_link_lane_hpd >> 8);

		onetime = 0;
	} else {
		tx_done = 0;
		xil_printf ("TX Cable Disconnected !!\r\n");
		Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);

		/*
		 * DpTxSs_DisableAudio
		 */
		Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
			      XDP_TX_AUDIO_CONTROL, 0x0);

		/*
		 * On HPD d/c, it is important to bring down the HDCP
		 */
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
* @param	InstancePtr is a pointer of void type.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpPt_HpdPulseHandler(void *InstancePtr)
{
	(void)InstancePtr;
	/*
	 * Some monitors give HPD pulse repeatedly which causes HPD pulse function to
	 * be executed huge number of times. Hence hpd_pulse interrupt is disabled
	 * and then enabled when hpd_pulse function is executed
	 */
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
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
* @param	InstancePtr is a pointer of void type.
* @param	Msa array of type XDpTxSs_MainStreamAttributes.
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
	u8 retrain_link = 0;

	if (!XVidC_EdidIsHeaderValid(InstancePtr->UsrHpdEventData.EdidOrg)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr,
				   DpTxSsInst.UsrHpdEventData.EdidOrg, 0);
	}

	u8 checksumMatch = 0;
	while (checksumMatch == 0) {
		if (CalculateChecksum(DpTxSsInst.UsrHpdEventData.EdidOrg, 128)) {
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr,
					   DpTxSsInst.UsrHpdEventData.EdidOrg, 0);
			checksumMatch = 0;
		} else {
			checksumMatch = 1;
		}
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
		/*
		 * Not all Monitors are capable of 1E
		 */
		bw_set = linkrate_tx_run;
		retrain_link = 1;
	}
	if(lane_set != XDP_TX_LANE_COUNT_SET_1
			&& lane_set != XDP_TX_LANE_COUNT_SET_2
			&& lane_set != XDP_TX_LANE_COUNT_SET_4){
		/*
		 * Not all links are trained at 4
		 */
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
		XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
		DpTxSubsystem_Start(&DpTxSsInst, Msa);
	}

	/*
	 * Mask unused Interrupts
	 */
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
		      XDP_TX_INTERRUPT_MASK, 0x0);
}

u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr,
			XDpTxSs_MainStreamAttributes Msa[4])
{
	(void)InstancePtr;
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
* This function starts tx process
*
* @param	line rate
* @param	lane counts
* @param	user_config of type user_config_struct
* @param	Msa array of type XDpTxSs_MainStreamAttributes
*
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
u32 Dp21RxSs_PtMst_StartTx(u8 line_rate, u8 lane_count, user_config_struct user_config,
			XDpTxSs_MainStreamAttributes Msa[4])
{
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;
	u8 format = user_config.user_format-1;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
					  0x15, 0x16, 0x17}; /**< Duplicate */
	u8 i = 0;
	(void)i;
	(void)pat;
	(void)C_VideoUserStreamPattern;
	int lr,lc = 0;
	u32 Status;

	lanecount_tx_run = lane_count;
	linkrate_tx_run = line_rate;

	tx_started = 0;

	Dp21RxSs_PtMst_Read(DpTxSsInst.DpPtr->Config.BaseAddr, 0x140);

	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
		      XDP_TX_INTERRUPT_MASK, 0xFFF);

	/*
	 * Waking up the monitor
	 */
	Dp21RxSs_PtMst_SinkPowerCycle();

	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);

	/*
	 * Give a bit of time for DP IP after monitor came up and starting Link training.
	 */
	usleep(100000);
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

	xil_printf("\r\nTraining TX with: Link rate %x, Lane %d, BPC %d\r\n",
		   line_rate,lane_count, bpc);

	Status = XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);
	if(Status != XST_SUCCESS){
		xil_printf("XDpTxSs_SetLinkRate failure\r\n");
	}
	xil_printf (".");
	XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);
	xil_printf (".");

	/*
	 * SetVidMode and SetBPC are not required in PassThrough
	 * In Passthrough these values are captured when
	 * DpTxSubsystem_Start is called with Msa values.
	 */
	if (res_table !=0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, res_table);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR: Setting resolution failed\r\n");
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
	 */
	if (format == 1) {
		format = XVIDC_CSF_YCRCB_422;
	} else if(format == 2) {
		format = XVIDC_CSF_YCRCB_444;
	}

	XDp_TxCfgSetColorEncode(DpTxSsInst.DpPtr, XDP_TX_STREAM_ID1, \
				format, XVIDC_BT_601, XDP_DR_VESA);

	/*
	 * VTC requires linkup(video clk) before setting values.
	 * This is why we need to linkup once to get proper CLK on VTC.
	 */
	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
			    "DP SS Start setup failed!\n\r");
	}

	xil_printf (".");

	Gen_vid_clk(DpTxSsInst.DpPtr,(XDP_TX_STREAM_ID1));
	xil_printf (".");

	xil_printf (".");
	clk_wiz_locked();

	/*
	 * Over writing the VTC, so that we no longer need to start tx twice.
	 */
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
		Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}

	/*
	 * Mask unsused interrupts
	 */
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0x0);

	vsync_counter = 0;

	/*
	 * Initialize CRC
	 *
	 * Reset CRC
	 */
	XVidFrameCrc_Reset(&VidFrameCRC_tx);

	/*
	 * Set Pixel width in CRC engine
	 */
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_tx, format);

	xil_printf ("..done !\r\n");
	tx_started = 1;

	/*
	 * Read Link rate over through channel
	 */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LINK_BW_SET, 1, &lr);

	/*
	 * Read Lane count through AUX channel
	 */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LANE_COUNT_SET, 1, &lc);

	xil_printf("(TX Link established at %x x %x)\r\n",(lr & 0xFF),(lc & XDP_DPCD_LANE_COUNT_SET_MASK));

	return XST_SUCCESS;
}

u32 start_tx_only(u8 line_rate, u8 lane_count,user_config_struct user_config)
{
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;
	u8 format = user_config.user_format-1;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
					  0x15, 0x16, 0x17};
	(void)pat;
	(void)C_VideoUserStreamPattern;

	int i = 0;
	int lr,lc = 0;

	num_sinks = 0;

	u32 Status;
	Vpg_Reset();

	/*
	 * Disabling TX interrupts.
	 */
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK, 0xFFF);

	/*
	 * Waking up the monitor
	 */
	Dp21RxSs_PtMst_SinkPowerCycle();

	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);

	/*
	 * Give a bit of time for DP IP after monitor came up and starting Link training.
	 */
	usleep(100000);
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

	xil_printf ("\r\nTraining TX with: Link rate %x, Lane count %d\r\n",
		    line_rate,lane_count);

	XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);
	xil_printf (".");
	XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);
	xil_printf (".");

	if (res_table !=0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, res_table);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR: Setting resolution failed\r\n");
		}
		xil_printf (".");
	}

	if (bpc !=0 ) {
		Status = XDpTxSs_SetBpc(&DpTxSsInst, bpc);
		if (Status != XST_SUCCESS){
			xil_printf("ERR: Setting bpc to %d failed\r\n",bpc);
		}
		xil_printf (".");
	}

	/*
	 * VTC requires linkup(video clk) before setting values.
	 */
	Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
	if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"SS Start failed!\n\r");
	}
	xil_printf (".");
	/*
	 * Updates required timing values in Video Pattern Generator.
	 */
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);

	/*
	 * All the streams use the same resolution.
	 */
	Gen_vid_clk(DpTxSsInst.DpPtr,(XDP_TX_STREAM_ID1));
	xil_printf (".");

	/*
	 * Setting video pattern.
	 */
	xil_printf (".");
	clk_wiz_locked();
	num_sinks = XDpTxSs_GetNumOfMstStreams(&DpTxSsInst);

	/*
	 * Keeping splitter in False mode
	 */
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	Status = XDpTxSs_DsSetup(DpTxSsInst.DsPtr, 0,
				 &DpTxSsInst.DpPtr->TxInstance.MsaConfig[0]);
#endif
	for (i = 0; i < num_sinks; i++) {
		if (DpTxSsInst.VtcPtr[i]) {
			Status |= XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[i],
			&DpTxSsInst.DpPtr->TxInstance.MsaConfig[i],
			DpTxSsInst.UsrOpt.VtcAdjustBs);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
						"VTC setup failed!\n\r");
			}
		}
	}

	xil_printf (".");
	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);
	xil_printf ("..done !\r\n");
	xil_printf ("MST trained with a total %d sinks\r\n", num_sinks);

	/*
	 * Read Link rate over through channel.
	 */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LINK_BW_SET, 1, &lr);

	/*
	 * Read Lane count through AUX channel.
	 */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LANE_COUNT_SET, 1, &lc);
	xil_printf("(TX Link established at %x x %x)\r\n",(lr & 0xFF),(lc & XDP_DPCD_LANE_COUNT_SET_MASK));

	/*
	 * Initialize CRC
	 *
	 * Reset CRC
	 */
	XVidFrameCrc_Reset(&VidFrameCRC_tx);

	/*
	 * Set Pixel width in CRC engine
	 */
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_tx, format);

	/*
	 * Reset CRC
	 */
	XVidFrameCrc_Reset(&VidFrameCRC_tx_1);

	/*
	 * Set Pixel width in CRC engine
	 */
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_tx_1, format);

	/*
	 * Reset CRC
	 */
	XVidFrameCrc_Reset(&VidFrameCRC_tx_2);

	/*
	 * Set Pixel width in CRC engine
	 */
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_tx_2, format);

	/*
	 * Reset CRC
	 */
	XVidFrameCrc_Reset(&VidFrameCRC_tx_3);

	/*
	 * Set Pixel width in CRC engine
	 */
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_tx_3, format);

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
static void clk_wiz_locked(void)
{
#ifndef SDT
	volatile u32 res = Dp21RxSs_PtMst_Read(XPAR_GPIO_0_BASEADDR,0x0);
#else
	volatile u32 res = Dp21RxSs_PtMst_Read(XPAR_XGPIO_0_BASEADDR,0x0);

	(void)res;

#endif
	u32 timer=0;

	while(res == 0 && timer < 1000) {
#ifndef SDT
		res = Dp21RxSs_PtMst_Read(XPAR_GPIO_0_BASEADDR,0x0);
#else
		res = Dp21RxSs_PtMst_Read(XPAR_XGPIO_0_BASEADDR,0x0);
#endif
		timer++; /**< Timer for timeout. No need to be specific time.
			      As long as long enough to wait lock */
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

void hpd_con(XDpTxSs *InstancePtr, u16 res_update)
{
	(void)InstancePtr;
	(void)res_update;
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
static __attribute__((unused)) XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane)
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

	PixelClockHz = (((Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_MSB] << 8) |
			 Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_LSB]) * 10) * 1000;

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

	/*
	 * Compute video mode timing values.
	 */
	Timing.HBackPorch = HBlank - (Timing.HFrontPorch + Timing.HSyncWidth);
	Timing.F0PVBackPorch = VBlank - (Timing.F0PVFrontPorch +
					 Timing.F0PVSyncWidth);
	Timing.HTotal = (Timing.HSyncWidth + Timing.HFrontPorch +
			 Timing.HActive + Timing.HBackPorch);
	Timing.F0PVTotal = (Timing.F0PVSyncWidth + Timing.F0PVFrontPorch +
			    Timing.VActive + Timing.F0PVBackPorch);
	FrameRate = PixelClockHz / (Timing.HTotal * Timing.F0PVTotal);

	/*
	 * Few monitors returns 59 HZ. Hence, setting to 60.
	 */
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
		/*
		 * Get video mode ID
		 */
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
void sink_power_down(void)
{
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
void sink_power_up(void)
{
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
u8 get_LineRate(void)
{
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
u8 get_Lanecounts(void)
{
	return DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
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
void Dp21RxSs_PtMst_SinkPowerCycle(void)
{
	usleep(4000);
	sink_power_up();

	/*
	 * Give enough time for monitor to wake up
	 */
	usleep(50000);
	sink_power_up(); /**< monitor to wake up once again due to CR-962717 */
	usleep(40000);
}

/*****************************************************************************/
/**
*
* This function masks the TX interrupts
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void mask_intr (void)
{
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);
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
void ReportVideoCRC_Tx()
{
	xil_printf("-----Stream 1-------\r\n");
	XVidFrameCrc_Report(&VidFrameCRC_tx);
	xil_printf("-----Stream 2-------\r\n");
	XVidFrameCrc_Report(&VidFrameCRC_tx_1);
	xil_printf("-----Stream 3-------\r\n");
	XVidFrameCrc_Report(&VidFrameCRC_tx_2);
	xil_printf("-----Stream 4-------\r\n");
	XVidFrameCrc_Report(&VidFrameCRC_tx_3);
}
#endif

#ifdef Tx
/*
 * This is for TX only mode
 */

extern XDpRxSs DpRxSsInst;    /**< The DPRX Subsystem instance.*/
#define DPRXSS_LINK_RATE        XDPRXSS_LINK_BW_SET_810GBPS
#define DPRXSS_LANE_COUNT       XDPRXSS_LANE_COUNT_SET_4

void main_loop()
{
	int i;
	u32 Status;
	u8 exit = 0;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	u8 LaneCount;
	u8 LineRate;
	u8 LineRate_init_tx = 0;
	u8 Edid_org[128];
	u8 Edid1_org[128];
	(void)Edid_org;
	(void)Edid1_org;
	u8 done = 0;
	u8 user_tx_LaneCount;
	u8 user_tx_LineRate;
	u32 aux_reg_address, num_of_aux_registers;
	u8 Data[8];
	u8 audio_on = 0;
	u32 data;
	int it = 0;
	(void)audio_on;
	(void)data;
	(void)it;

	unsigned char bpc_table[] = {6,8,10,12,16};

	u8 in_pwr_save = 0;
	u16 DrpVal =0;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
					  0x15, 0x16, 0x17}; /**< Duplicate */

	user_config_struct user_config;
	user_config.user_bpc = 8;
	user_config.VideoMode_local=XVIDC_VM_1920x1080_60_P;
	user_config.user_pattern = 1;
	user_config.user_format = XVIDC_CSF_RGB;

	Dp21RxSs_PtMst_Read(DpTxSsInst.DpPtr->Config.BaseAddr, 0x140);
#ifndef SDT
	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
#endif

#ifdef Rx
	/* Set Link rate and lane count to maximum.
	 *
	 * The RX is always set for Max capability of 5.4G and the
	 * extended capability bit is set.
	 * DP1.4 Sources are supposed to read the extended capability bit
	 * and decide whether the sink is 8.1G capable or not.
	 */
	XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

	/*
	 * Start DPRX Subsystem set
	 */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return;
	}
	DpRxSs_Setup();

	/*
	 * In Independent mode, all streams are consumed by CRC
	 * nothing is fwd to TX
	 */
	XAxisScr_MiPortDisable(&axis_switch, 0);
	XAxisScr_MiPortEnable (&axis_switch, 1, 0);
	XAxisScr_MiPortEnable (&axis_switch, 2, 1);
	XAxisScr_MiPortEnable (&axis_switch, 3, 2);
	XAxisScr_MiPortEnable (&axis_switch, 4, 3);
	XAxisScr_RegUpdateEnable (&axis_switch);
#endif
	Dp21RxSs_PtMst_SubHelpMenu ();

	exit = 0;
	while (exit == 0) { /**< for menu loop */
		if (tx_is_reconnected == 1) {
			start_tx_only (SinkMaxLinkrate, SinkMaxLanecount, user_config);
			tx_is_reconnected = 0;
		}

		if (hpd_pulse_con_event == 1) {
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst, DpTxSsInst.DpPtr->TxInstance.MsaConfig);
			Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);

			if (Status != XST_SUCCESS) {
				xil_printf ("Link is bad..re initiating training\r\n");
				Dp21RxSs_PtMst_SinkPowerCycle();
				tx_is_reconnected = 1;
			}
		}

		CmdKey[0] = 0;
		CommandKey = 0;

		CommandKey = Dp21RxSs_PtMst_Getc(0xff);
		Command = atoi(&CommandKey);
		if (Command != 0) {
			xil_printf("You have selected command %d\r\n", Command);
		}

		switch (CommandKey){
		case 'e':
			xil_printf ("EDID read is :\r\n");
			Read_EDID(&DpTxSsInst,1);

			xil_printf ("\r\nEDID read over =======\r\n");
			break;
		case 'c':
#ifdef Rx
			if (DpRxSsInst.link_up_trigger ==1) {
				xil_printf ("========== Rx CRC===========\r\n");
				ReportVideoCRC(0);
			}
#endif
#ifdef Tx
			xil_printf ("========== Tx CRC===========\r\n");
			ReportVideoCRC_Tx();
#endif
			break;

		case 'd' :
			if (in_pwr_save == 0) {
				sink_power_down();
				in_pwr_save = 1;
				xil_printf ("\r\n==========power down===========\r\n");
			} else {
				sink_power_up();
				in_pwr_save = 0;
				xil_printf ("\r\n==========power up===========\r\n");

				hpd_con(&DpTxSsInst, user_config.VideoMode_local);
			}
			break;

		case '1' :
			/*
			 * Resolution menu
			 */
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();
			Dp21RxSs_PtMst_ResolutionHelpMenu();
			exit = 0;

			while (exit == 0) {
				CmdKey[0] = 0;
				Command = 0;
				CmdKey[0] = Dp21RxSs_PtMst_InByteLocal();

				if(CmdKey[0]!=0){
					Command = (int)CmdKey[0];

					switch (CmdKey[0]) {
						case 'x' :
							exit = 1;
							Dp21RxSs_PtMst_SubHelpMenu ();
							break;
						default :
							xil_printf("You have selected command '%c'\r\n",
								    CmdKey[0]);
							if (CmdKey[0] >= 'a' && CmdKey[0] <= 'z') {
								Command = CmdKey[0] -'a' + 10;
								done = 1;
							} else if (Command > 47 && Command < 58) {
								Command = Command - 48;
								done = 1;
							} else if (Command >= 58 || Command <= 47) {
								Dp21RxSs_PtMst_ResolutionHelpMenu();
								done = 0;
								break;
							}
							xil_printf ("\r\nSetting resolution...\r\n");
							audio_on = 0;
							user_config.VideoMode_local = resolution_table[Command];

							LineRate = get_LineRate();
							LaneCount = get_Lanecounts();

							Dp21RxSs_PtMst_Write (XPAR_DP_TX_HIER_0_AV_PATGEN_0_BASEADDR + 0x400, 0x0, 0x0);
							usleep(10000);
							Dp21RxSs_PtMst_Write (XPAR_DP_TX_HIER_0_AV_PATGEN_0_BASEADDR + 0x400, 0x0, 0x0);
							usleep(10000);
							Dp21RxSs_PtMst_Write (XPAR_DP_TX_HIER_0_AV_PATGEN_0_BASEADDR + 0x400, 0x10, 0x0); /**< 0x0: Sine tone,
																	0x2: Ping tone */
							usleep(10000);
							Dp21RxSs_PtMst_Write (XPAR_DP_TX_HIER_0_AV_PATGEN_0_BASEADDR + 0x400,  0x20, 0x0); /**< 0x0: Sine tone,
																	 0x2: Ping tone */
							usleep(10000);
							Dp21RxSs_PtMst_Write (XPAR_DP_TX_HIER_0_AV_PATGEN_0_BASEADDR + 0x400,  0xA0, 0x0);
							usleep(10000);
							Dp21RxSs_PtMst_Write (XPAR_DP_TX_HIER_0_AV_PATGEN_0_BASEADDR + 0x400,  0xA4, 0x0000000);
							usleep(10000);
							Dp21RxSs_PtMst_Write (XPAR_DP_TX_HIER_0_AV_PATGEN_0_BASEADDR + 0x400,  0x4, 0x0);
							usleep(10000);
							start_tx_only (LineRate,LaneCount,user_config);
							exit = done;
							break;
					}
				}
			}
			exit = 0;
			Dp21RxSs_PtMst_SubHelpMenu ();
			break;

		case '3' :
			/*
			 * BPC menu
			 */
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();

			exit = 0;
			Dp21RxSs_PtMst_BpcHelpMenu(DPTXSS_BPC);
			while (exit == 0) {
				CommandKey = 0;
				Command = 0;
				CommandKey = Dp21RxSs_PtMst_InByteLocal();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					switch (CommandKey) {
						case 'x' :
							exit = 1;
							Dp21RxSs_PtMst_SubHelpMenu ();
							break;
						default :
							Command = Command - 48;
							user_config.user_bpc = bpc_table[Command];
							xil_printf("You have selected %c\r\n",
								    CommandKey);
							if (Command > 4) {
								Dp21RxSs_PtMst_BpcHelpMenu(DPTXSS_BPC);
								done = 0;
								break;
							} else {
								xil_printf("Setting BPC of %d\r\n",
												user_config.user_bpc);
								done = 1;
							}

							/* placeholder to keep references below */
							/* Keep references to avoid unused warnings (placeholder) */
							(void)VPhyInst;
							(void)VPHY_TX_PLL_TYPE;
							(void)VPHY_TX_CHANNEL_TYPE;
							(void)XVPHY_DIR_TX;

							start_tx_only (LineRate, LaneCount,user_config);
							LineRate = get_LineRate();
							LaneCount = get_Lanecounts();
							exit = done;
							break;
					}
				}
			}
			Dp21RxSs_PtMst_SubHelpMenu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '2' :
			xil_printf("Select the Link and Lane count\r\n");
			exit = 0;
			Dp21RxSs_PtMst_SelectLinkLane();
			while (exit == 0) {
				CmdKey[0] = 0;
				Command = 0;
				CmdKey[0] = Dp21RxSs_PtMst_InByteLocal();
				if(CmdKey[0]!=0){
					Command = (int)CmdKey[0];
					Command = Command - 48;
					switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
					   Dp21RxSs_PtMst_SubHelpMenu ();
					   break;

					   default :
						xil_printf("You have selected command %c\r\n", CmdKey[0]);
						if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
							Command = CmdKey[0] -'a' + 10;
						}

						if (Command < 22) {
							user_tx_LaneCount = lane_link_table[Command].lane_count;
							user_tx_LineRate = lane_link_table[Command].link_rate;
							if(lane_link_table[Command].lane_count
									> DpTxSsInst.Config.MaxLaneCount)
							{
								xil_printf("This Lane Count is not supported by Sink \r\n");
								xil_printf("Max Supported Lane Count is 0x%x \r\n",
									   DpTxSsInst.Config.MaxLaneCount);
								xil_printf("Training at Supported Lane count  \r\n");
								LaneCount = DpTxSsInst.Config.MaxLaneCount;
							}
							done = 1;
						} else {
							xil_printf("!!!Warning: You have selected wrong option"
								   " for lane count and link rate\r\n");
							Dp21RxSs_PtMst_SelectLinkLane();
							done = 0;
							break;
						}

						/*
						 * Disabling TX interrupts
						 */
						Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
							      XDP_TX_INTERRUPT_MASK, 0xFFF);
						LineRate_init_tx = user_tx_LineRate;

						Status = Dp21RxSs_PtMst_PhyConfig(&VPhyInst, LineRate_init_tx, VPHY_TX_PLL_TYPE,
								    VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
						XDpTxSs_Stop(&DpTxSsInst);
						audio_on = 0;
						xil_printf("TX Link & Lane Capability is set to %x, %x\r\n",
							   user_tx_LineRate, user_tx_LaneCount);
						xil_printf("Setting TX to 8 BPC and 800x600 resolution\r\n");
						XDpTxSs_Reset(&DpTxSsInst);
						user_config.user_bpc = 8;
						user_config.VideoMode_local = XVIDC_VM_800x600_60_P;
						user_config.user_pattern = 1;
						user_config.user_format = XVIDC_CSF_RGB;
						Dp21RxSs_PtMst_Write (XPAR_DP_TX_HIER_0_AV_PATGEN_0_BASEADDR + 0x400,  0x0, 0x0);

						start_tx_only(user_tx_LineRate, user_tx_LaneCount, user_config);
						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();
						exit = done;
						break;
					}
				}
			}
			exit = 0;
			Dp21RxSs_PtMst_SubHelpMenu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '4' :
			/*
			 * Pattern menu
			 */
			Dp21RxSs_PtMst_TestPatternGenHelp();
			exit = 0;
			while (exit == 0) {
				CommandKey = 0;
				CommandKey = Dp21RxSs_PtMst_InByteLocal();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					Command = Command - 48;
					switch  (CommandKey)
					{
						case 'x' :
							exit = 1;
							Dp21RxSs_PtMst_SubHelpMenu ();
							break;

						default :

							if(Command > 0 && Command < 8) {
								xil_printf("You have selected video pattern %d "
									   "from the pattern list \r\n", Command);
								done = 1;
							} else {
								xil_printf("!!!Warning : Invalid pattern selected \r\n");
								Dp21RxSs_PtMst_TestPatternGenHelp();
								done = 0;
								break;
							}
							user_config.user_pattern = Command;
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
										 C_VideoUserStreamPattern[user_config.user_pattern]);
							exit = done;
							break;
					}
				}
			}
			exit = 0;
			Dp21RxSs_PtMst_SubHelpMenu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '5' :
#ifdef Rx
			if (DpRxSsInst.link_up_trigger == 1) {
				xil_printf ("==========RX Debug Data===========\r\n");
				XDpRxSs_ReportMsaInfo(&DpRxSsInst);
			}
#endif
#ifdef Tx
			xil_printf ("==========TX Debug Data===========\r\n");
			XDpTxSs_ReportMsaInfo(&DpTxSsInst);
			XDpTxSs_ReportVtcInfo(&DpTxSsInst);
#endif

#ifdef Rx
			xil_printf ("DP RX addr is %x\r\n",DpRxSsInst.DpPtr->Config.BaseAddr);
#endif
#ifdef Tx
			xil_printf ("DP TX addr is %x\r\n",DpTxSsInst.DpPtr->Config.BaseAddr);
#endif
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '6' :
#ifdef Rx
			if (DpRxSsInst.link_up_trigger ==1) {
				xil_printf ("==========RX Debug Data===========\r\n");
				XDpRxSs_ReportLinkInfo(&DpRxSsInst);
			}
#endif
#ifdef Tx
			xil_printf("==========TX Debug Data===========\r\n");
			XDpTxSs_ReportLinkInfo(&DpTxSsInst);
#endif
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '7' :
			/*
			 * Display DPCD reg
			 */
			XDpTxSs_ReportSinkCapInfo(&DpTxSsInst);
			break;

		case '8' :
			/*
			 * 9 - Read Aux registers
			 */
			xil_printf("\r\n Give 4 bit Hex value of base register 0x");
			aux_reg_address = Dp21RxSs_PtMst_Gethex(4);

			xil_printf("\r\n Give msb 2 bit Hex value of base register 0x");

			aux_reg_address |= ((Dp21RxSs_PtMst_Gethex(2) << 16) & 0xFFFFFF);
			xil_printf("\r\n Give number of registers that you "
				   "want to read (1 to 9): ");
			num_of_aux_registers = Dp21RxSs_PtMst_Gethex(1);
			if((num_of_aux_registers < 1)||(num_of_aux_registers > 9)) {
					xil_printf("\r\n!!!Warning: Invalid number "
				   "selected, hence reading only one register\r\n");
					num_of_aux_registers = 1;
			}
			xil_printf("\r\nGiven base address offset is 0x%x\r\n",	aux_reg_address);

			for (i = 0; i < (int)num_of_aux_registers; i++) {
				Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, (aux_reg_address+i), 1, &Data);
				if(Status == XST_SUCCESS) {
					xil_printf("Value at address offset 0x%x, is = 0x%x\r\n",
						   (aux_reg_address+i),
						   ((Data[0]) & 0xFF));
				} else {
					xil_printf("Aux Read failure\r\n");
					break;
				}
			}
			break;

		/*
		 * Display VideoPHY status
		 */
		case 'b':
			xil_printf("Video PHY Config/Status --->\r\n");
			xil_printf(" RCS (0x10) = 0x%x\r\n", XVphy_ReadReg(VPhyInst.Config.BaseAddr,
							     XVPHY_REF_CLK_SEL_REG));
			xil_printf(" PR  (0x14) = 0x%x\r\n", XVphy_ReadReg(VPhyInst.Config.BaseAddr,
							     XVPHY_PLL_RESET_REG));
			xil_printf(" PLS (0x18) = 0x%x\r\n", XVphy_ReadReg(VPhyInst.Config.BaseAddr,
							     XVPHY_PLL_LOCK_STATUS_REG));
			xil_printf(" TXI (0x1C) = 0x%x\r\n", XVphy_ReadReg(VPhyInst.Config.BaseAddr,
							     XVPHY_TX_INIT_REG));
			xil_printf(" TXIS(0x20) = 0x%x\r\n", XVphy_ReadReg(VPhyInst.Config.BaseAddr,
							     XVPHY_TX_INIT_STATUS_REG));
			xil_printf(" RXI (0x24) = 0x%x\r\n", XVphy_ReadReg(VPhyInst.Config.BaseAddr,
							     XVPHY_RX_INIT_REG));
			xil_printf(" RXIS(0x28) = 0x%x\r\n", XVphy_ReadReg(VPhyInst.Config.BaseAddr,
							     XVPHY_RX_INIT_STATUS_REG));

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					     XVPHY_DRP_CPLL_FBDIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_FBDIV)"
				   " = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_CPLL_FBDIV, DrpVal);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					     XVPHY_DRP_CPLL_REFCLK_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_REFCLK_DIV)"
				   " = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_CPLL_REFCLK_DIV, DrpVal);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					     XVPHY_DRP_RXOUT_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_RXOUT_DIV)"
				   " = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_RXOUT_DIV, DrpVal);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					     XVPHY_DRP_TXOUT_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_TXOUT_DIV)"
				   " = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_TXOUT_DIV, DrpVal);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
		case 'z' :
			Dp21RxSs_PtMst_SubHelpMenu ();
			break;

		default:
			break;

		} /**< End of switch (CmdKey[0]) */

#ifdef Rx
		Dp21RxSs_PtMst_RxTracking();
#endif
	}
}
#endif
