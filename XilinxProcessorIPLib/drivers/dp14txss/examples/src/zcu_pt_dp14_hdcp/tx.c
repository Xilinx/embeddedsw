/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
#include "rx.h"

#define I2C_MUX_device_address 0x74
#define Si570_device_address 0x5D
#define audio_clk_Hz 24.576

extern u8 rx_trained;
#if ENABLE_HDCP_IN_DESIGN
extern u8 hdcp_capable_org ;
extern int mon_is_hdcp22_cap;
extern u8 hdcp_capable ;
extern u8 hdcp_repeater_org ;
extern u8 hdcp_repeater ;
extern u8 internal_rx_tx ;
extern u8 do_not_train_tx ;
#endif

static XVphy_User_Config PHY_User_Config_Table[] =
{
// Index,         TxPLL,               RxPLL,
//    TxChId,         RxChId,
// LineRate,              LineRateHz,
// QPLLRefClkSrc,          CPLLRefClkSrc,    QPLLRefClkFreqHz,CPLLRefClkFreqHz
	{   0,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK,    ONBOARD_REF_CLK,     270000000,270000000},
	{   1,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK,    ONBOARD_REF_CLK,     270000000,270000000},
	{   2,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  ONBOARD_REF_CLK,    ONBOARD_REF_CLK,     270000000,270000000},
	{   3,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
	{   4,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
	{   5,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
	{   6,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
	{   7,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
	{   8,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
	{   9,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
	{   10,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},

};

/************************** Function Prototypes ******************************/

u32 DpTxSs_Main(void);

static u8 CalculateChecksum(u8 *Data, u8 Size);
static XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
static void clk_wiz_locked(void);
//static void tx_main_loop(void);


//void hpd_pulse_con(XDpTxSs *InstancePtr, XDpTxSs_MainStreamAttributes Msa[4]);
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void ReportVideoCRC(void);

void bpc_help_menu(int DPTXSS_BPC_int);
void sub_help_menu(void);
void resolution_help_menu(void);
void select_link_lane(void);
void test_pattern_gen_help();
void format_help_menu(void);
void operationMenu(void);
char inbyte_local(void);
extern u8 tx_after_rx;

/************************** Variable Definitions *****************************/
#define DPCD_TEST_CRC_R_Cr   0x240
#define DPCD_TEST_SINK_MISC  0x246
#define DPCD_TEST_SINK_START 0x270
#define CRC_AVAIL_TIMEOUT    1000
/************************** Function Definitions *****************************/
extern XVidC_VideoMode resolution_table[];
extern lane_link_rate_struct lane_link_table[];

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
//	u32 Status = XST_SUCCESS;
//	XINTC *IntcInstPtr = &IntcInst;

	/* Set custom timer wait */
//	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpPt_CustomWaitUs, &TmrCtr);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_EVENT),
						&DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_PULSE),
						&DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
						&DpPt_LinkrateChgHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_PE_VS_ADJUST),
						&DpPt_pe_vs_adjustHandler, &DpTxSsInst);

	return (XST_SUCCESS);
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

	rate = get_LineRate();
	// If the requested rate is same, do not re-program.
	if (rate != prev_line_rate) {
		set_vphy(rate);
	}
	//update the previous link rate info at here
	prev_line_rate = rate;
}


void DpPt_pe_vs_adjustHandler(void *InstancePtr){
//      u8 Buffer[2];
//      u16 RegisterAddress;
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
				XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
						preemp);
				XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
						preemp);
				XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
						preemp);
				XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
						preemp);
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
		tx_is_reconnected++;
//		usleep(50000);
//		 This part has added to give HDCP a proper handle when hdp even happens
//		 HDCP block will disable Tx side encryption when hpd detected
		#if ENABLE_HDCP_IN_DESIGN
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
#if ENABLE_HDCP1x_IN_TX
				XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
#endif

#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				XHdcp1xExample_Poll();
#endif
				XDpTxSs_HdcpEnable(&DpTxSsInst);
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				XHdcp1xExample_Poll();
#endif
				if (DpRxSsInst.link_up_trigger == 1) {
					rx_trained = 1;
				}
		#endif
	}
	else
	{
        xil_printf ("TX Cable Disconnected !!\r\n");
		//DpTxSs_DisableAudio
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			     XDP_TX_AUDIO_CONTROL, 0x0);

		//on HPD d/c, it is important to bring down the HDCP
		tx_is_reconnected = 0;
		DpTxSsInst.no_video_trigger = 1;
	    //setting vswing
	    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
			XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
	    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
			XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
	    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
			XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
	    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
			XVPHY_GTHE4_DIFF_SWING_DP_V0P0);

	    //setting postcursor
	    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
			XVPHY_GTHE4_PREEMP_DP_L0);
	    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
			XVPHY_GTHE4_PREEMP_DP_L0);
	    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
			XVPHY_GTHE4_PREEMP_DP_L0);
	    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
			XVPHY_GTHE4_PREEMP_DP_L0);
#if ENABLE_HDCP_IN_DESIGN
		{
			xdbg_printf(XDBG_DEBUG_GENERAL, ".~\r\n");
			XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
			XDpTxSs_HdcpDisable(&DpTxSsInst);
#if ENABLE_HDCP1x_IN_TX
			XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
#endif
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
			XHdcp1xExample_Poll();
#endif
		}
#endif
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
extern u8 tx_done;
void hpd_pulse_con(XDpTxSs *InstancePtr, XDpTxSs_MainStreamAttributes Msa[4])
{
    u32 Status;

	//who populates this data ?
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
			){
		bw_set = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate; //InstancePtr->DpPtr->Config.MaxLinkRate;
		retrain_link = 1;
//        xil_printf ("bad bw\r\n");
	}
	if(lane_set != XDP_TX_LANE_COUNT_SET_1
			&& lane_set != XDP_TX_LANE_COUNT_SET_2
			&& lane_set != XDP_TX_LANE_COUNT_SET_4){
		lane_set = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount; //InstancePtr->DpPtr->Config.MaxLaneCount;
		retrain_link = 1;
//        xil_printf ("bad lane\r\n");
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
//			xil_printf ("bad lane sts 4\r\n");
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
//			xil_printf ("bad lane sts 2\r\n");
		}
	} else if (lane_set == XDP_TX_LANE_COUNT_SET_1) {
		lane0_sts = lane0_sts & 0x7;
		if ((lane0_sts !=
				(XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
				XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK)
				) || (laneAlignStatus != 1)) {
			retrain_link = 1;
//			xil_printf ("bad lane sts 1\r\n");
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
     if(dev_serv_intr_vec & 0x04) {
	 if(mon_is_hdcp22_cap)
	 {

		/* CP_IRQ is set, Call the HDCP22 Cp_Irq handler */
//    	    	xil_printf("HPD_Pulse: CP_IRQ, dev_serv_intr_vec = 0x%x\n\r",
//    	    			dev_serv_intr_vec);
#if ENABLE_HDCP22_IN_TX
			 DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);

		if (XHdcp22Tx_IsInProgress (DpTxSsInst.Hdcp22Ptr)) {
			/*Handle CP_IRQ*/
			XHdcp22_Handle_Cp_Irq(DpTxSsInst.Hdcp22Ptr);
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
				/* Disable Encryption */
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				XHdcp1xExample_Poll();
#endif

				/* Re-start authentication (the expectation is
				 * that HDCP is already in the authenticated state). */
				xdbg_printf(XDBG_DEBUG_GENERAL, "\033[1m\033[43m\033[34m (*<*)Tx-> \033[0m \n");
				XDpTxSs_Authenticate(&DpTxSsInst);
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				XHdcp1xExample_Poll();
#endif
				XDpTxSs_EnableEncryption(&DpTxSsInst, 0x1);
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				XHdcp1xExample_Poll();
#endif
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
				/* Disable Encryption */
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
//				XHdcp1xExample_Poll();

				/* Re-start authentication (the expectation is
				 * that HDCP is already in the authetnicated state. )*/
				// Post EVENT_DWNSTMREADY instead of authenticate
//				XDpTxSs_ReadDownstream(&DpTxSsInst);
//				XDpTxSs_Authenticate(&DpTxSsInst);
//				XHdcp1xExample_Poll();
			}

		}

	     /* Check if the Ro'_AVAILABLE bit is set. */
		if (BStatus & 0x02) {
#if ENABLE_HDCP_FLOW_GUIDE
			xdbg_printf(XDBG_DEBUG_GENERAL, "\033[1m\033[42m\033[37m (*<*)Ro'_AVAILABLE!"
							"\033[0m \n");
#endif
			if ((BStatus & 0x01) != 0x01) {
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				XHdcp1xExample_Poll();
#endif
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
//		XDpTxSs_Start(&DpTxSsInst);
		DpTxSubsystem_Start(&DpTxSsInst, Msa);
		tx_done = 1;
	}

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);
}


u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr,
			XDpTxSs_MainStreamAttributes Msa[4]) {
	u32 Status;
//	xil_printf ("MSA in TxSSStart is %d\r\n",Msa);
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
* This function sets up DPTxSubsystem
*
* @param	LineRate
* @param	LaneCount
* @param	edid 1st block
* @param	edid 2nd block
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DpTxSs_Setup(u8 *LineRate_init, u8 *LaneCount_init,
			u8 Edid_org[128], u8 Edid1_org[128]){
	u8 Status;

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_STATUS);

	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

	u8 connected;
	// this is intentional infinite while loop
	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
	xil_printf(
			"Please connect a DP Monitor to start the application !!!\r\n");
			connected = 1;
		}
	}

	//Waking up the monitor
	sink_power_cycle();


	//reading the first block of EDID
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
		//reading the second block of EDID
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
		xil_printf("Reading EDID contents of the DP Monitor..\r\n");

		Status  = XDp_TxAuxRead(DpTxSsInst.DpPtr,
								XDP_DPCD_MAX_LINK_RATE,  1, LineRate_init);
		Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
								XDP_DPCD_MAX_LANE_COUNT, 1, LaneCount_init);

		u8 rData = 0;
		// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
		XDp_TxAuxRead(DpTxSsInst.DpPtr,
			      XDP_DPCD_TRAIN_AUX_RD_INTERVAL,
			      1, &rData);
		/* if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled */
		if (rData & 0x80) {
			/* read maxLineRate */
			XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData);
			if (rData == XDP_DPCD_LINK_BW_SET_810GBPS) {
				*LineRate_init = 0x1E;
			}
		}


		if (Status != XST_SUCCESS) { // give another chance to monitor.
			//Waking up the monitor
			sink_power_cycle();

			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
			//reading the second block of EDID
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
			xil_printf("Reading EDID contents of the DP Monitor..\r\n");

			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
								XDP_DPCD_MAX_LINK_RATE, 1, LineRate_init);
			Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
								XDP_DPCD_MAX_LANE_COUNT, 1, LaneCount_init);
			if (Status != XST_SUCCESS)
				xil_printf ("Failed to read sink capabilities\r\n");

			// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
			XDp_TxAuxRead(DpTxSsInst.DpPtr,
				      XDP_DPCD_TRAIN_AUX_RD_INTERVAL,
				      1, &rData);
			/* if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled */
			if (rData & 0x80) {
				/* read maxLineRate */
				XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData);
				if (rData == XDP_DPCD_LINK_BW_SET_810GBPS) {
					*LineRate_init = 0x1E;
				}
			}

		}
	} else {
		xil_printf("Please connect a DP Monitor and try again !!!\r\n");
		return;
	}

	*LineRate_init &= 0xFF;
	*LaneCount_init &= 0xF;
//	xil_printf("System capabilities set to: LineRate %x, LaneCount %x\r\n",
//			*LineRate_init,*LaneCount_init);

#if ENABLE_AUDIO
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);
#endif
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
			(XVPHY_PLL_RESET_QPLL0_MASK |
			 XVPHY_PLL_RESET_QPLL1_MASK)); // 0x06
	XVphy_WriteReg(InstancePtr->Config.BaseAddr,
			XVPHY_PLL_RESET_REG, 0x0);

	XVphy_ResetGtPll(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,(FALSE));


	Status = XVphy_WaitForPmaResetDone(InstancePtr, QuadId,
			TxChId, XVPHY_DIR_TX);

	Status += XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);

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
			XDpTxSs_MainStreamAttributes Msa[4])
{
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;
	u8 format = user_config.user_format-1;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate


	u32 Status;
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
	}

	/*
	 * Setting Color Format
	 * User can change coefficients here - By default 601 is used for YCbCr
	 * */
	XDp_TxCfgSetColorEncode(DpTxSsInst.DpPtr, XDP_TX_STREAM_ID1, \
				format, XVIDC_BT_601, XDP_DR_CEA);
#if ENABLE_HDCP_IN_DESIGN
	XDpTxSs_HdcpDisable(&DpTxSsInst);
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
	XHdcp1xExample_Poll();
#endif
#endif
	// VTC requires linkup(video clk) before setting values.
	// This is why we need to linkup once to get proper CLK on VTC.
	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);

	xil_printf (".");
	//updates required timing values in Video Pattern Generator
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
	xil_printf (".");
	// setting video pattern
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
				 C_VideoUserStreamPattern[pat]);


	xil_printf (".");
	clk_wiz_locked();

	// Over writing the VTC, so that we no longer need to start tx twice
	if (DpTxSsInst.VtcPtr[0]) {
		Status = XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[0],
		&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0],
		DpTxSsInst.UsrOpt.VtcAdjustBs);
		if (Status != XST_SUCCESS) {
			/*xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
				"VTC%d setup failed!\n\r", Index);*/
		}
	}

// No longer needed to start twice
//	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
//	xil_printf (".");

//	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 500000);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 100000);

	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		xil_printf ("*");
		Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0x0);


	/*
	 * Initialize CRC
	 */
	/* Reset CRC*/
	XVidFrameCrc_Reset(&VidFrameCRC_tx);
	/* Set Pixel width in CRC engine*/
	XVidFrameCrc_WriteReg(VidFrameCRC_tx.Base_Addr,
				  VIDEO_FRAME_CRC_CONFIG,
				  XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					  XDP_TX_USER_PIXEL_WIDTH));

//	start_audio_passThrough(line_rate);
	xil_printf ("..done !\r\n");
#if ENABLE_HDCP_IN_DESIGN
	if(hdcp_capable_org == 1)	{
#if ENABLE_HDCP1x_IN_TX
		XDpTxSs_SetLane(&DpTxSsInst,
			DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount);
		XDpTxSs_SetPhysicalState(&DpTxSsInst, !(hdcp_capable_org));
#endif

#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
		XHdcp1xExample_Poll();
#endif
#if ENABLE_HDCP1x_IN_TX
		XDpTxSs_SetPhysicalState(&DpTxSsInst, (hdcp_capable_org));
#endif
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
		XHdcp1xExample_Poll();
#endif
		DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 10000);
	}
#endif
	tx_started = 1;
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

	volatile u32 res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
	u32 timer=0;

	while(res == 0 && timer < 1000) {
//		xil_printf ("~/~/");
		res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
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
    u32 Status;
    u8 retval=0;

	/* This is a PassThrough System to display the Video received on RX
	 * onto TX. There is nothing special done in the hpd_connect handler.
	 * On a HPD the application simply tries to retrain the monitor
	 */

	u8 auxValues;
#if  ENABLE_HDCP_IN_DESIGN
	hdcp_capable=0;
	retval =0;
#endif
	/*Wait to read the capabilities*/
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 2000000);

#if ENABLE_HDCP22_IN_TX
	XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x06921D, 1, &auxValues);
	retval = auxValues & 0x2;
#endif
	if(retval==2)
	{
#if ENABLE_HDCP22_IN_TX
		mon_is_hdcp22_cap=1;
		hdcp_capable=1;
	DpTxSsInst.HdcpProtocol=XDPTXSS_HDCP_22;
#endif
	}
	else
	{
#if ENABLE_HDCP1x_IN_TX
		XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x068028, 1, &auxValues);
		retval = auxValues & 0x1;
#endif
		if(retval==1)
		{
#if ENABLE_HDCP1x_IN_TX
			mon_is_hdcp22_cap=0;
			hdcp_capable=1;
		DpTxSsInst.HdcpProtocol=XDPTXSS_HDCP_1X;
#endif
		}
		else
		{
#if ENABLE_HDCP_IN_DESIGN
			hdcp_capable=0;
#endif
		}
	}

#if ENABLE_HDCP_IN_DESIGN
		if (hdcp_capable != hdcp_capable_org) {
			do_not_train_tx = 1;
			hdcp_capable_org = hdcp_capable;
		}
		else{
			do_not_train_tx = 0;
		}

#endif

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
	return DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
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
u32 set_vphy(int LineRate_init_tx){
	u32 Status=0;

#if TX_BUFFER_BYPASS
    switch(LineRate_init_tx){
            case XDP_TX_LINK_BW_SET_162GBPS:
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E,
						DIVIDER_162);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E,
						DIVIDER_162);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E,
						DIVIDER_162);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E,
						DIVIDER_162);

				break;

            case XDP_TX_LINK_BW_SET_270GBPS:
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E,
						DIVIDER_270);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E,
						DIVIDER_270);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E,
						DIVIDER_270);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E,
						DIVIDER_270);

				break;

            case XDP_TX_LINK_BW_SET_540GBPS:
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E,
						DIVIDER_540);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E,
						DIVIDER_540);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E,
						DIVIDER_540);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E,
						DIVIDER_540);

				break;

            case XDP_TX_LINK_BW_SET_810GBPS:
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E,
						DIVIDER_810); //57440);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E,
						DIVIDER_810); //57440);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E,
						DIVIDER_810); //57440);
				XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E,
						DIVIDER_810); //57440);

				break;
    }
#endif

	switch(LineRate_init_tx){
	case XDP_TX_LINK_BW_SET_162GBPS:
		Status = PHY_Configuration_Tx(&VPhyInst,
				PHY_User_Config_Table[(is_TX_CPLL) ? 0 : 3]);
		break;

	case XDP_TX_LINK_BW_SET_270GBPS:
		Status = PHY_Configuration_Tx(&VPhyInst,
				PHY_User_Config_Table[(is_TX_CPLL) ? 1 : 4]);
		break;

	case XDP_TX_LINK_BW_SET_540GBPS:
		Status = PHY_Configuration_Tx(&VPhyInst,
				PHY_User_Config_Table[(is_TX_CPLL) ? 2 : 5]);
		break;

	case XDP_TX_LINK_BW_SET_810GBPS:
		Status = PHY_Configuration_Tx(&VPhyInst,
				PHY_User_Config_Table[(is_TX_CPLL) ? 9 : 10]);
		break;
	}

	if (Status != XST_SUCCESS) {
		xil_printf (
			"+++++++ TX GT configuration encountered a failure +++++++\r\n");
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
