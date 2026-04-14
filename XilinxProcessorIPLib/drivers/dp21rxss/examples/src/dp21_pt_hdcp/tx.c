/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include <xdptxss.h>

#define I2C_MUX_device_address 0x74
#define Si570_device_address 0x5D
#define audio_clk_Hz 24.576

/************************** Function Prototypes ******************************/

u32 DpTxSs_Main(void);

static u8 CalculateChecksum(u8 *Data, u8 Size);
static void clk_wiz_locked(void);

int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
// void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
// void ReportVideoCRC(void);
void mask_intr(void);
u8 edid_monitor[384];

void bpc_help_menu(int DPTXSS_BPC_int);
void sub_help_menu(void);
void resolution_help_menu(void);
void select_link_lane(void);
void test_pattern_gen_help();
void format_help_menu(void);
char inbyte_local(void);
u8 tx_pass = 0;
XDp_TxAudioInfoFrame *xilInfoFrame;
extern u32 infofifo[32];
extern u8 endindex;
extern u8 fifocount;

u8 startindex = 0;
u16 vsync_counter = 0;
u16 ektpkt_counter = 0;
extern u8 rx_trained;
extern u8 tx_done;
extern volatile u8 hpd_pulse_con_event; 	/* This variable triggers hpd_pulse_con */
u8 linkrate_tx_run;
u8 lanecount_tx_run;
extern XDpTxSs DpTxSsInst; 		/* The DPTX Subsystem instance.*/
extern Video_CRC_Config VidFrameCRC_tx;
extern XTmrCtr TmrCtr; 		/* Timer instance.*/
int tx_started;
extern XDpRxSs DpRxSsInst;
volatile u8 prev_line_rate; 		/* This previous line rate to keep
				 * previous info to compare
				 * with new line rate request*/

extern XVphy VPhyInst; 	/* The DPRX Subsystem instance.*/
// extern XVphy_User_Config PHY_User_Config_Table[];
/************************** Variable Definitions *****************************/
#define DPCD_TEST_CRC_R_Cr   0x240
#define DPCD_TEST_SINK_MISC  0x246
#define DPCD_TEST_SINK_START 0x270
#define CRC_AVAIL_TIMEOUT    1000
/************************** Function Definitions *****************************/
extern lane_link_rate_struct lane_link_table[];

extern XVphy_PllType VPHY_TX_PLL_TYPE;
extern XVphy_ChannelId VPHY_TX_CHANNEL_TYPE;

#if ENABLE_HDCP_IN_DESIGN
extern u8 hdcp_capable_org ;
extern int mon_is_hdcp22_cap;
extern u8 hdcp_capable ;
extern u8 hdcp_repeater_org ;
extern u8 hdcp_repeater ;
extern u8 internal_rx_tx ;
extern u8 do_not_train_tx ;
#endif

#ifdef Txo
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

void DpTxSs_Audio (XDpTxSs *InstancePtr, u8 Enable) {

if (!Enable) {
	XDp_WriteReg(InstancePtr->DpPtr->Config.BaseAddr,
								     XDP_TX_AUDIO_CONTROL, 0x0);
} else {
	XDp_WriteReg(InstancePtr->DpPtr->Config.BaseAddr,
								     XDP_TX_AUDIO_CONTROL, 0x1);
}

}

void DpTxSs_Start (XDpTxSs *InstancePtr, u8 Enable) {
if (!Enable) {
	XDp_WriteReg(InstancePtr->DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
} else {
	XDp_WriteReg(InstancePtr->DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);
}
}

void Read_EDID (void *InstancePtr, u8 enable_print) {
	(void)InstancePtr;
	(void)enable_print;

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
		if(Sum != 0){
			xil_printf("Wrong EDID was read\r\n");
		}

		//reading the second block of EDID
		if(Edid_org[126] > 0)
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
		if(Edid_org[126] > 1)
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid2_org, 2);

		xil_printf("Reading EDID contents of the DP Monitor..\r\n");

		switch (Edid_org[126]){
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

u32 get_max_capabilities (XDpTxSs *InstancePtr) {

    u8 SinkCap[16], SinkExtendedCap[16];
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
		    XDP_DPCD_TRAIN_AUX_RD_EXT_RX_CAP_FIELD_PRESENT_MASK){
		if(SinkExtendedCap[XDP_DPCD_ML_CH_CODING_CAP] &
			    XDP_TX_MAIN_LINK_CHANNEL_CODING_SET_128B_132B_MASK){
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
		}else{
			SinkMaxLinkrate = SinkExtendedCap[XDP_DPCD_MAX_LINK_RATE];
			SinkMaxLanecount= SinkExtendedCap[XDP_DPCD_MAX_LANE_COUNT] & XDP_DPCD_MAX_LANE_COUNT_MASK;
		}

	}else{
		SinkMaxLinkrate = SinkCap[XDP_DPCD_MAX_LINK_RATE];
		SinkMaxLanecount= SinkCap[XDP_DPCD_MAX_LANE_COUNT] & XDP_DPCD_MAX_LANE_COUNT_MASK;
	}

    max_link_lane = (SinkMaxLinkrate << 8) | (SinkMaxLanecount);
    return max_link_lane;
}

// u16 vsync_counter = 0;
// u16 ektpkt_counter = 0;
u8 onetime = 0;
u8 prog_misc1 = 0;
u8 prog_fb_rd;
volatile int tx_is_reconnected=0;		/* This variable to keep track
				 * of the status of Tx link*/


void DpPt_ffe_adjustHandler(void *InstancePtr){
	(void)InstancePtr;

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
	(void)InstancePtr;
#if ENABLE_AUDIO
	    tx_pass = 1;
        u8 i = 0;
        u32 fifosts = 0;

        if(fifocount > AUXFIFOSIZE-1) {
                startindex = endindex;
        }

        while (startindex != endindex) {
                fifosts = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
                                                 0x6A0);
                fifosts &= 0x00000003;
                i = 0;
                if (fifosts == 0) {
                        for (i = 0; i < 8; i++) {
                                XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
                                                XDP_TX_AUDIO_INFO_DATA(1), infofifo[(startindex*8+i)]);
                        }
                } else {
//                      xil_printf(ANSI_COLOR_RED"TX InfoFrame Fifo is full!!"ANSI_COLOR_RESET"\r\n");
                }
                if(startindex < (AUXFIFOSIZE - 1)) {
                        startindex++;
                } else {
                        startindex = 0;
                }

        }
#endif
        fifocount = 0;
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
	(void)InstancePtr;
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
	(void)InstancePtr;
	// If TX is unable to train at what it has been asked then
	// necessary down shift handling has to be done here
	// eg. reconfigure GT to new rate etc
    u32 Status = XST_SUCCESS;
    u8 rate;
	// u8 lanes;
	rate = get_LineRate();
    // lanes = XPAR_DP_TX_HIER_0_V_DP_TXSS2_1_LANE_COUNT;//get_Lanecounts();
	Status = config_phy (&VPhyInst, rate, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
	if (Status != XST_SUCCESS) {
		xil_printf ("PHY Config failed during the TX training\r\n");
	}

	//update the previous link rate info at here
	prev_line_rate = rate;
}


void DpPt_pe_vs_adjustHandler(void *InstancePtr){
	(void)InstancePtr;
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
	(void)InstancePtr;

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
extern u8 SinkMaxLinkrate;
extern u8 SinkMaxLanecount;
extern u8 Tx_only_done;

void DpPt_HpdEventHandler(void *InstancePtr)
{
	(void)InstancePtr;
	u16 max_link_lane_hpd;
	u32 ReadVal;
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
//		sink_power_down();
		sink_power_up();
		xil_printf ("Cable Connected\r\n");
		Read_EDID (&DpTxSsInst, 0);
		tx_is_reconnected = 1;
		max_link_lane_hpd = get_max_capabilities (&DpTxSsInst);
        SinkMaxLanecount = max_link_lane_hpd;
	    SinkMaxLinkrate = (max_link_lane_hpd >> 8);
		// xil_printf ("max rate is %x\r\n",SinkMaxLinkrate);
		// xil_printf ("max lane is %x\r\n",SinkMaxLanecount);
		onetime = 0;
		Tx_only_done=0;
		#if ENABLE_HDCP_IN_DESIGN
#if (ENABLE_HDCP1x_IN_TX | ENABLE_HDCP22_IN_TX)
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
#endif
#if ENABLE_HDCP1x_IN_TX
				XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
#endif

#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				Hdcp1xExample_Poll();
#endif
#if (ENABLE_HDCP1x_IN_TX | ENABLE_HDCP22_IN_TX)
				XDpTxSs_HdcpEnable(&DpTxSsInst);
#endif
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				Hdcp1xExample_Poll();
#endif
				if (DpRxSsInst.link_up_trigger == 1) {
					rx_trained = 1;
				}
		#endif

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

		#if ENABLE_HDCP_IN_DESIGN
		{

#if (ENABLE_HDCP1x_IN_TX | ENABLE_HDCP22_IN_TX)
			XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
			XDpTxSs_HdcpDisable(&DpTxSsInst);
#endif
#if ENABLE_HDCP1x_IN_TX
			XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
#endif
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
			Hdcp1xExample_Poll();
#endif
		}
#endif

	}

	//Keep the Audio FIFO in reset state
	ReadVal = XDp_ReadReg(TX_CLK_RST_ADDR, 0x8);
	ReadVal = (ReadVal & 0x7);
	ReadVal = (ReadVal & 0x1); //Putting FIFO in reset, bit[1]
	XDp_WriteReg(TX_CLK_RST_ADDR, 0x8, ReadVal);

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
	(void)InstancePtr;

// Some monitors give HPD pulse repeatedly which causes HPD pulse function to
//		be executed a huge number of times. Hence hpd_pulse interrupt is disabled
//		and then enabled when hpd_pulse function is executed
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_INTERRUPT_MASK, 0xFFFFF010);
		    //  XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);
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
#if ENABLE_HDCP_IN_DESIGN
	u32 Status;
#endif
	u8 lane0_sts = InstancePtr->UsrHpdPulseData.Lane0Sts;
	u8 lane2_sts = InstancePtr->UsrHpdPulseData.Lane2Sts;
	u8 laneAlignStatus = InstancePtr->UsrHpdPulseData.LaneAlignStatus;
	u8 bw_set = InstancePtr->UsrHpdPulseData.BwSet;
	u8 lane_set = InstancePtr->UsrHpdPulseData.LaneSet;
	u8 down_strm = InstancePtr->UsrHpdPulseData.AuxValues[4];

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
		//Not all monitors are capable of 1E
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
#if ENABLE_HDCP_IN_DESIGN
	if(mon_is_hdcp22_cap)
	{
	if ((down_strm & 0x40) == 0x40) {
//		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		retrain_link = 1;
//		xil_printf ("dwn strm\r\n");
	}
	}
#endif

	if (retrain_link) {
		tx_done = 0;
	}

	if (!retrain_link) {

#if (ENABLE_HDCP_IN_DESIGN)
#define DEVICE_SERVICE_IRQ_VECTOR 0x0201
#define DEVICE_SERVICE_IRQ_VECTOR_CP_IRQ_MASK 0x04

     u8 dev_serv_intr_vec;
     u8 BStatus;
     /* Check for the CP_IRQ interrupt. Check the CP_IRQ
      * bit in the DEVICE_SERVICE_IRQ_VECTOR (0x0201) */
     Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x201, 1, &dev_serv_intr_vec);
     (void)Status;
     if(dev_serv_intr_vec & 0x04) {
	 if(mon_is_hdcp22_cap)
	 {

		/* CP_IRQ is set, Call the HDCP22 Cp_Irq handler */
//    	    	xil_printf("HPD_Pulse: CP_IRQ, dev_serv_intr_vec = 0x%x\n\r",
//    	    			dev_serv_intr_vec);
#if ENABLE_HDCP22_IN_TX
			//  DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);

		if (XHdcp22Tx_Dp_IsInProgress (DpTxSsInst.Hdcp22Ptr)) {
			/*Handle CP_IRQ*/
			XHdcp22Tx_Dp_Handle_Cp_Irq(DpTxSsInst.Hdcp22Ptr);
		}
#endif
	 }
	 else
	 {
	/* CP_IRQ is set, read the BStatus register */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x068029, 1, &BStatus);
//    	xil_printf(" HPD_Pulse: CP_IRQ, (BStatus : %x) \n", BStatus);

	/* Check if the Link Integrity Failure Bit is set */
	if (BStatus & 0x04) {
#if ENABLE_HDCP_FLOW_GUIDE
		xdbg_printf(XDBG_DEBUG_GENERAL, "\033[1m\033[41m\033[37m (*<*)TxLink! \033[0m \n");
#endif
			/* State 5 : Authenticated,
			 * State 6 : Link Integrity Check */
			if(
#if ENABLE_HDCP1x_IN_TX
					DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 6 ||
					DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 5
#else
					0
#endif
					) {
#if (ENABLE_HDCP_TX)
				/* Disable Encryption */
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
#endif

				Hdcp1xExample_Poll();

				/* Re-start authentication (the expectation is
				 * that HDCP is already in the authenticated state). */
				xdbg_printf(XDBG_DEBUG_GENERAL, "\033[1m\033[43m\033[34m (*<*)Tx-> \033[0m \n");
#if (ENABLE_HDCP_TX)
				XDpTxSs_Authenticate(&DpTxSsInst);
#endif

				Hdcp1xExample_Poll();

#if (ENABLE_HDCP_TX)
				XDpTxSs_EnableEncryption(&DpTxSsInst, 0x1);
#endif

				Hdcp1xExample_Poll();
			}
	}

	/* Check if the READY bit is set. */
		if (BStatus & 0x01) {
#if ENABLE_HDCP_FLOW_GUIDE
			xdbg_printf(XDBG_DEBUG_GENERAL, "\033[1m\033[42m\033[37m (*<*)Ready! \033[0m \n");
#endif
			/* DP TX State 8 : Wait-for-Ready */
			if(
#if ENABLE_HDCP1x_IN_TX
					DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 8
#else
					0
#endif
			) {
#if (ENABLE_HDCP_TX)
				/* Disable Encryption */
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
#endif
//				Hdcp1xExample_Poll();

				/* Re-start authentication (the expectation is
				 * that HDCP is already in the authetnicated state. )*/
				// Post EVENT_DWNSTMREADY instead of authenticate
//				XDpTxSs_ReadDownstream(&DpTxSsInst);
//				XDpTxSs_Authenticate(&DpTxSsInst);
//				Hdcp1xExample_Poll();
			}

		}

	     /* Check if the Ro'_AVAILABLE bit is set. */
		if (BStatus & 0x02) {
#if ENABLE_HDCP_FLOW_GUIDE
			xdbg_printf(XDBG_DEBUG_GENERAL, "\033[1m\033[42m\033[37m (*<*)Ro'_AVAILABLE!"
							"\033[0m \n");
#endif
			if ((BStatus & 0x01) != 0x01) {

				Hdcp1xExample_Poll();

			}
		}

		/* Check if CP_IRQ is spurious */
		if (BStatus == 0x00) {
#if ENABLE_HDCP_FLOW_GUIDE
			xdbg_printf(XDBG_DEBUG_GENERAL, "\033[1m\033[41m\033[37m (*<*)Spurious CP_IRQ!"
						"\033[0m \n");
#endif
			/* Disable Hpd for a while (100ms) */
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0x013);

			/* Wait */
			DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 2000000);

			/* Enable the all DP TX interrupts again */
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0x0);
		}

	 }
     }
#endif
}


	if (retrain_link == 1) {
//		sink_power_cycle();
		XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
		DpTxSubsystem_Start(&DpTxSsInst, Msa);
	}

	/* Mask unused interrupts
	 *
	 */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFFFF000);
}


u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr,
			XDpTxSs_MainStreamAttributes Msa[4]) {
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
			XDpTxSs_MainStreamAttributes Msa_Pt[4])
{
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 format = user_config.user_format-1;
	int lr,lc=0;
	u32 Status;
	lanecount_tx_run = lane_count;
	linkrate_tx_run = line_rate;
	//Disabling TX interrupts
    tx_started = 0;

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			0x140);

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFFFFFFF);
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
    if (res_table !=0) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
	} else {
		Status = DpTxSubsystem_Start(&DpTxSsInst, Msa_Pt);
	}
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
		Status = DpTxSubsystem_Start(&DpTxSsInst, Msa_Pt);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}
	/* Mask unused interrupts
	 *
	 */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFFFF000);

	vsync_counter = 0;

	/*
	 * Initialize CRC
	 */
	/* Reset CRC*/
	XVidFrameCrc_Reset(&VidFrameCRC_tx);
	/* Set Pixel width in CRC engine*/
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_tx, format);

	xil_printf ("..done !\r\n");
	tx_pass = 0;
	tx_started = 1;

	/* Read Link rate over through channel */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LINK_BW_SET, 1, &lr);

	/* Read Lane count through AUX channel */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LANE_COUNT_SET, 1,
			&lc);

	xil_printf("(TX Link established at %x x %x)\r\n",(lr & 0xFF),(lc & XDP_DPCD_LANE_COUNT_SET_MASK));
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
