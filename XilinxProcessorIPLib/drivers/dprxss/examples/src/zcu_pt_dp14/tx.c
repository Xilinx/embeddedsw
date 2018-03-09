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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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

#include "tx.h"

#include "main.h"

#define I2C_MUX_device_address 0x74
#define Si570_device_address 0x5D
#define audio_clk_Hz 24.576


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
		  0x14,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,         270000000,270000000},
	{   10,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,
		  ONBOARD_REF_CLK,        ONBOARD_REF_CLK,     270000000,270000000},
};


/************************** Function Prototypes ******************************/

u32 DpTxSs_Main(void);
u32 DpTxSs_PlatformInit(void);

u32 DpTxSs_SetupIntrSystem(void);
u32 DpTxSs_VideoPhyInit(u16 DeviceId);
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void clk_wiz_locked(void);
void hpd_pulse_con(XDpTxSs *InstancePtr);
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
static u8 CalculateChecksum(u8 *Data, u8 Size);
XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
void ReportVideoCRC(void);
extern void tx_main_loop(void);

void bpc_help_menu(int DPTXSS_BPC_int);
void sub_help_menu(void);
void resolution_help_menu(void);
void select_link_lane(void);
void test_pattern_gen_help();
void format_help_menu(void);
void operationMenu(void);
char inbyte_local(void);
u32 xil_gethex(u8 num_chars);

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
* This function is the main entry point for the interrupt example using the
* XDpTxSs driver. This function will set up the system with interrupts and
* set up Hot-Plug-Event (HPD) handlers.
*
* @param	DeviceId is the unique device ID of the DisplayPort TX
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		  was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
*		order to illustrate interrupt handling taking place for HPD
*		events.
*
******************************************************************************/
u32 DpTxSs_Main()
{
	u8 LineRate_init = XDP_TX_LINK_BW_SET_540GBPS;
	u8 LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;
	u8 LaneCount_init = XDP_TX_LANE_COUNT_SET_4;
	u8 LaneCount_init_tx = XDP_TX_LANE_COUNT_SET_4;
	u8 Edid_org[128], Edid1_org[128];
	u8 connected = 0;

	user_config_struct user_config;
	user_config.user_bpc = 8;
	user_config.VideoMode_local = XVIDC_VM_800x600_60_P;
	user_config.user_pattern = 1; /*Color Ramp (Default)*/
	user_config.user_format = XVIDC_CSF_RGB;


#if ENABLE_AUDIO
	// I2C MUX device address : 0x74
	// Si570 device address : 0x5D
	//setting Si570 on zcu102 to be 24.576MHz for audio
	clk_set(I2C_MUX_device_address, Si570_device_address, audio_clk_Hz);
#endif
	DpTxSs_Setup(&LineRate_init, &LaneCount_init, Edid_org, Edid1_org);

	/* Check if monitor is connected or not
	 * This is intentional infinite while loop
	 * */
	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
			xil_printf(
				"Please connect a DP Monitor to start the application !!!\r\n");
			connected = 1;
		}
	}

	/* Waking up the monitor */
	sink_power_cycle();

	/* Do not return in order to allow interrupt handling to run. HPD events
	 * (connect, disconnect, and pulse) will be detected and handled.
	 */
	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;

	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
	/* This configures the vid_phy for line rate to start with
	 * Even though CPLL can be used in limited case,
	 * using QPLL is recommended for more coverage.
		*/
	set_vphy(LineRate_init_tx);

	LaneCount_init_tx = LaneCount_init_tx & 0x7;
	/* 800x600 8bpc as default */
	start_tx (LineRate_init_tx, LaneCount_init_tx,user_config, 0);
	/* Enabling TX interrupts */

	tx_main_loop();

	return XST_SUCCESS;
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
	/* If TX is unable to train at what it has been asked then
	 * necessary down shift handling has to be done here
	 * eg. reconfigure GT to new rate etc
	 * */
	u8 rate;

	rate = get_LineRate();
	/* If the requested rate is same, do not re-program. */
	if (rate != prev_line_rate) {
		set_vphy(rate);
	}
	/* update the previous link rate info at here */
	prev_line_rate = rate;
}

void DpPt_pe_vs_adjustHandler(void *InstancePtr){
	if(PE_VS_ADJUST == 1){
		unsigned char preemp;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
			case 0: preemp = XVPHY_GTHE3_PREEMP_DP_L0; break;
			case 1: preemp = XVPHY_GTHE3_PREEMP_DP_L1; break;
			case 2: preemp = XVPHY_GTHE3_PREEMP_DP_L2; break;
			case 3: preemp = XVPHY_GTHE3_PREEMP_DP_L3; break;
		}

		XVphy_SetTxPreEmphasis(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, preemp);
		XVphy_SetTxPreEmphasis(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, preemp);
		XVphy_SetTxPreEmphasis(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, preemp);
		XVphy_SetTxPreEmphasis(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, preemp);


		unsigned char diff_swing;
		switch (DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel) {
		case 0:
			switch (DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel) {
			case 0:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L0;
				break;
			case 1:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L1;
				break;
			case 2:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L2;
				break;
			case 3:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L3;
				break;
			}
			break;
		case 1:
			switch (DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel) {
			case 0:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L1;
				break;
			case 1:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L2;
				break;
			case 2:
			case 3:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L3;
				break;
			}
			break;
		case 2:
			switch (DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel) {
			case 0:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L2;
				break;
			case 1:
			case 2:
			case 3:
				diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L3;
				break;
			}
			break;
		case 3:
			diff_swing = XVPHY_GTHE3_DIFF_SWING_DP_L3; break;
		}
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
				diff_swing);

	}

	if (DP141_ADJUST == 1) {
		u8 data =0;
//		data = i2c_read_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x02);
		unsigned char diff_swing;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.VsLevel){
			case 0: diff_swing = 0x8; break;
			case 1: diff_swing = 0x8; break;
			case 2: diff_swing = 0x8; break;
			case 3: diff_swing = 0xC; break;
		}

		data = data & 0xF0;
		data |= diff_swing;

//		unsigned char preemp;
//		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
//			case 0: preemp = 0; break;
//			case 1: preemp = 1; break;
//			case 2: preemp = 3; break;
//			case 3: preemp = 7; break;
//		}
//		data |= (preemp << 4);

		i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x02, data);
		i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x05, data);
		i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x08, data);
		i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x0B, data);
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

	u32 TimerVal, TimerVal_pre;
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
		sink_power_down();
		sink_power_up();
		tx_is_reconnected = 1;
	} else {
		//DpTxSs_DisableAudio
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			     XDP_TX_AUDIO_CONTROL, 0x0);

		/* On HPD d/c, it is important to bring down the HDCP. */
		tx_is_reconnected = 0;
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
void hpd_pulse_con(XDpTxSs *InstancePtr)
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
		} else {
			checksumMatch = 1;
		}
	}

	lane_set = lane_set & 0x1F;
	bw_set = bw_set & 0x1F;
	laneAlignStatus = laneAlignStatus & 0x1;
	
	if (bw_set != XDP_TX_LINK_BW_SET_162GBPS &&
	    bw_set != XDP_TX_LINK_BW_SET_270GBPS &&
	    bw_set != XDP_TX_LINK_BW_SET_540GBPS) {
		bw_set = InstancePtr->DpPtr->Config.MaxLinkRate;
		retrain_link = 1;
	}

	if (lane_set != XDP_TX_LANE_COUNT_SET_1 &&
	    lane_set != XDP_TX_LANE_COUNT_SET_2 &&
	    lane_set != XDP_TX_LANE_COUNT_SET_4) {
		lane_set = InstancePtr->DpPtr->Config.MaxLaneCount;
		retrain_link = 1;
	}

	lane0_sts = lane0_sts & 0x77;
	lane2_sts = lane2_sts & 0x77;
	if (lane_set == XDP_TX_LANE_COUNT_SET_4) {
		if ((lane0_sts != (XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK)) ||
		    (lane2_sts != (XDP_DPCD_STATUS_LANE_2_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_2_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_2_SL_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_3_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_3_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_3_SL_DONE_MASK)) ||
		    (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	} else if (lane_set == XDP_TX_LANE_COUNT_SET_2) {
		if ((lane0_sts != (XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_1_SL_DONE_MASK)) ||
		    (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	} else if (lane_set == XDP_TX_LANE_COUNT_SET_1) {
		lane0_sts = lane0_sts & 0x7;
		if ((lane0_sts != (XDP_DPCD_STATUS_LANE_0_CR_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_CE_DONE_MASK |
				   XDP_DPCD_STATUS_LANE_0_SL_DONE_MASK)) ||
		   (laneAlignStatus != 1)) {
			retrain_link = 1;
		}
	}

	if (retrain_link == 1) {
		sink_power_cycle();
		XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
		XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
		XDpTxSs_Start(&DpTxSsInst);
	}

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);
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
	/* This is intentional infinite while loop. */
	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
			xil_printf("Please connect a DP Monitor to start "
				   "the application !!!\r\n");
			connected = 1;
		}
	}

	/* Waking up the monitor */
	sink_power_cycle();

	/* Reading the first block of EDID */
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
		/* Reading the second block of EDID */
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
		xil_printf("Reading EDID contents of the DP Monitor..\r\n");

		Status  = XDp_TxAuxRead(DpTxSsInst.DpPtr,
					XDP_DPCD_MAX_LINK_RATE, 1,
					LineRate_init);
		Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
					XDP_DPCD_MAX_LANE_COUNT, 1,
					LaneCount_init);

		u8 rData = 0;
		/* check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit */
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

		if (Status != XST_SUCCESS) {
			/* give another chance to monitor */
			/* Waking up the monitor */
			sink_power_cycle();

			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
			/* Reading the second block of EDID */
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
			xil_printf("Reading EDID contents of "
				   "the DP Monitor..\r\n");

			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
					       XDP_DPCD_MAX_LINK_RATE,
					       1, LineRate_init);
			Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
						XDP_DPCD_MAX_LANE_COUNT,
						1, LaneCount_init);
			if (Status != XST_SUCCESS)
				xil_printf("Failed to read sink "
					   "capabilities\r\n");
		}
	} else {
		xil_printf("Please connect a DP Monitor "
			   "and try again !!!\r\n");
		return;
	}

	*LineRate_init &= 0xFF;
	*LaneCount_init &= 0xF;
	xil_printf("System capabilities set to: LineRate %x, LaneCount %x\r\n",
		   *LineRate_init,*LaneCount_init);

#if ENABLE_AUDIO
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_CONTROL, 0x0);
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

	/* Required for VPHY setting */
	RxPllSelect = PHY_User_Config_Table.RxPLL;
	TxChId      = PHY_User_Config_Table.TxChId;

	/* Set the Ref Clock Frequency */
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, QpllRefClkSel,
				PHY_User_Config_Table.QPLLRefClkFreqHz);
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, CpllRefClkSel,
				PHY_User_Config_Table.CPLLRefClkFreqHz);
	XVphy_CfgLineRate(InstancePtr, QuadId, TxChId,
			  PHY_User_Config_Table.LineRateHz);

	XVphy_PllInitialize(InstancePtr, QuadId, TxChId,
			    QpllRefClkSel, CpllRefClkSel,
			    TxPllSelect, RxPllSelect);

	/* Initialize GT with ref clock and PLL selects
	 * GT DRPs may not get completed if GT is busy
	 * doing something else, hence, this is run in 
	 * loop and retried 100 times
	 */
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

	Status = XVphy_ResetGtPll(InstancePtr, QuadId,
				  XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DIR_TX, (FALSE));

	Status += XVphy_WaitForPmaResetDone(InstancePtr, QuadId,
					    XVPHY_CHANNEL_ID_CHA,
					    XVPHY_DIR_TX);
	Status += XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);
	Status += XVphy_WaitForResetDone(InstancePtr, QuadId,
					 XVPHY_CHANNEL_ID_CHA,
					 XVPHY_DIR_TX);
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
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13,
					  0x14, 0x15, 0x16, 0x17}; //Duplicate

	u32 Status;
	/* Disabling TX interrupts */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, 
		     XDP_TX_INTERRUPT_MASK, 0xFFF);
	/* Waking up the monitor */
	sink_power_cycle();

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	/* Give a bit of time for DP IP after monitor
	 * came up and starting Link training */
	usleep(100000);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

	xil_printf("\r\nTraining TX with: Link rate %x, Lane count %d\r\n",
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
	 * Setting Color Format
	 * User can change coefficients here - By default 601 is used for YCbCr
	 * */
	XDp_TxCfgSetColorEncode(DpTxSsInst.DpPtr, XDP_TX_STREAM_ID1, 
				format, XVIDC_BT_601, XDP_DR_CEA);

	/* VTC requires linkup(video clk) before setting values.
	 * This is why we need to linkup once to get proper CLK on VTC.
	 * */
	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);

	xil_printf (".");
	/* updates required timing values in Video Pattern Generator */
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
	xil_printf (".");
	/* setting video pattern */
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
				 C_VideoUserStreamPattern[pat]);

	// Over writing the VTC, so that we no longer need to start tx twice
	if (DpTxSsInst.VtcPtr[0]) {
		Status = XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[0],
		&DpTxSsInst.DpPtr->TxInstance.MsaConfig[0],
		DpTxSsInst.UsrOpt.VtcAdjustBs);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
				"VTC%d setup failed!\n\r", Index);
		}
	}



	xil_printf (".");
	clk_wiz_locked();

// No longer needed to start twice
//	Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
	xil_printf (".");
	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, Msa);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0x0);

	/* Initialize CRC */
	/* Reset CRC*/
	XVidFrameCrc_Reset(&VidFrameCRC_tx);
	/* Set Pixel width in CRC engine*/
	XVidFrameCrc_WriteReg(VidFrameCRC_tx.Base_Addr,
				  VIDEO_FRAME_CRC_CONFIG,
				  XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					  XDP_TX_USER_PIXEL_WIDTH));
	xil_printf ("..done !\r\n");

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
void clk_wiz_locked(void) {

	volatile u32 res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
	u32 timer=0;

	while(res == 0 && timer < 1000) {
		xil_printf ("~/~/");
		res = XDp_ReadReg(XPAR_GPIO_0_BASEADDR,0x0);
		timer++;
		/* timer for timeout. No need to be specific time */
		/* As long as long enough to wait lock */
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
// as soon as HPD is connected the application reads the EDID to find out
// if the monitor was changed.

void hpd_con(XDpTxSs *InstancePtr, u8 Edid_org[128],
		u8 Edid1_org[128], u16 res_update)
{
	u32 Status=XST_SUCCESS;
	u8 max_cap_new = InstancePtr->UsrHpdEventData.MaxCapNew;
	u8 max_cap_lanes_new = InstancePtr->UsrHpdEventData.MaxCapLanesNew;
	u32 htotal_test_hpd;
	u32 vtotal_test_hpd;
	u32 freq_test_hpd;
	u8 good_edid_hpd = 1;
	XVidC_VideoMode VmId_test_hpd;
	XVidC_VideoMode VmId_ptm_hpd;
	u8 bpc_hpd;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13,
					  0x14,	0x15, 0x16, 0x17}; //Duplicate

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_STATUS);
	/* Enabling TX interrupts */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_INTERRUPT_MASK, 0xFFF);

	memcpy(Edid_org, InstancePtr->UsrHpdEventData.EdidOrg, 128);

	tx_is_reconnected--;

	if (XVidC_EdidIsHeaderValid(Edid_org)) {
		good_edid_hpd = 1;
	} else {
		good_edid_hpd = 0;
	}

	if (!CalculateChecksum(Edid_org, 128)) {
		good_edid_hpd = 1;
	} else {
		good_edid_hpd = 0;
	}

	/* Till here is the requirement per DP spec */

	/* *******************************************
	 * From here is optional per application     *
	 * ******************************************/

	/* check if lane is 1/2/4 or something else */
	if (max_cap_lanes_new != 1 &&
	    max_cap_lanes_new != 2 &&
	    max_cap_lanes_new != 4) {
		/* soemthing wrong. Read again */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT,
			      1, &max_cap_lanes_new);
	}

	/* check if line speed is either 0x6, 0xA, 0x14 */
	if (max_cap_new != XDP_TX_LINK_BW_SET_540GBPS &&
	    max_cap_new != XDP_TX_LINK_BW_SET_270GBPS &&
	    max_cap_new != XDP_TX_LINK_BW_SET_162GBPS) {
		/* soemthing wrong. Read again */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE,
			      1, &max_cap_new);
	}

	if (good_edid_hpd == 1) {
		htotal_test_hpd = XVidC_EdidGetStdTimingsH(Edid_org, 1);
		vtotal_test_hpd = XVidC_EdidGetStdTimingsV(Edid_org, 1);
		freq_test_hpd   = XVidC_EdidGetStdTimingsFrr(Edid_org, 1);
		//XVidC_UnregisterCustomTimingModes();
		VmId_test_hpd = XVidC_GetVideoModeId(htotal_test_hpd,
						     vtotal_test_hpd,
						     freq_test_hpd, 0);
		VmId_ptm_hpd = GetPreferredVm(Edid_org, max_cap_new ,
					      max_cap_lanes_new&0x1F);
		bpc_hpd = XVidC_EdidGetColorDepth(Edid_org);

		if (VmId_ptm_hpd == XVIDC_VM_NOT_SUPPORTED) {
			VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
			bpc_hpd = 6;
		}
	} else {
		VmId_test_hpd = XVIDC_VM_NOT_SUPPORTED;
		VmId_ptm_hpd = XVIDC_VM_640x480_60_P;
		res_update = XVIDC_VM_640x480_60_P;
		bpc_hpd = 6;
		good_edid_hpd = 0;
	}

	if (max_cap_new == XDP_TX_LINK_BW_SET_540GBPS ||
	    max_cap_new == XDP_TX_LINK_BW_SET_270GBPS ||
	    max_cap_new == XDP_TX_LINK_BW_SET_162GBPS) {
		Status = set_vphy(max_cap_new);
	
		XDpTxSs_SetLinkRate(&DpTxSsInst, max_cap_new);
		XDpTxSs_SetLaneCount(&DpTxSsInst,
				     max_cap_lanes_new & 0x1F);
	
	    if (good_edid_hpd == 1) {
			if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED) &&
			    (VmId_test_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				XDpTxSs_SetVidMode(&DpTxSsInst, res_update);
			} else if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				XDpTxSs_SetVidMode(&DpTxSsInst, VmId_ptm_hpd);
			} else {
				XDpTxSs_SetVidMode(&DpTxSsInst, VmId_test_hpd);
			}
		} else {
			XDpTxSs_SetVidMode(&DpTxSsInst, res_update);
		}
	
	        /* over subscription check */
		/*RGB or YCbCr444 - Hence 3 components*/
		u32 StreamBandwidth = (((
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HTotal *
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.F0PVTotal *
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.FrameRate)/1000000) *
			(bpc_hpd * 3))/8/DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
	
		u32 LinkBandwidth =
				(DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate * 27);
	
		if (StreamBandwidth>LinkBandwidth) {
			xil_printf("StreamBandwidth=%d, LinkBandwidth=%d\r\n",
				   StreamBandwidth,LinkBandwidth);
			switch (bpc_hpd) {
			case 16:
				bpc_hpd = 12;
				break;
			case 12:
				bpc_hpd = 10;
				break;
			case 10:
				bpc_hpd = 8;
				break;
			case 8:
				bpc_hpd = 6;
				break;
			default:
				bpc_hpd = 6;
				break;
			}
			StreamBandwidth = (((
				DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HTotal *
				DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.F0PVTotal *
				DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.FrameRate) / 1000000) *
				(bpc_hpd * 3))/8/DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
			if(StreamBandwidth>LinkBandwidth){
				/* setting low resolution to
				 * fit in the bandwidth */
				xil_printf("Over subscription and "
					   "can't display\r\n");
			} else {
				xil_printf("Setting BPC to be "
					   "%d\r\n", bpc_hpd);
			}
		}
	
		XDpTxSs_SetBpc(&DpTxSsInst, bpc_hpd);
		XDpTxSs_Start(&DpTxSsInst);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			 XDP_TX_INTERRUPT_MASK, 0x0);
		Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
		Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
				 C_VideoUserStreamPattern[1]);
		Status = XDpTxSs_Start(&DpTxSsInst);


		Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
		if (Status != XST_SUCCESS) {
		Status = XDpTxSs_Start(&DpTxSsInst);
		}
	}

	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
}

/*****************************************************************************/
/**
*
* This function to send Audio Information Frame
*
* @param	XilAudioInfoFrame
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame)
{
	u8 db1, db2, db3, db4;
	u32 temp;
	u8 RSVD=0;
	
	/* Fixed paramaters */
	u8  dp_version   = 0x11;
	
	/* Write #1 */
	db1 = 0x00; //sec packet ID fixed to 0 - SST Mode
	db2 = 0x80 + xilInfoFrame->type;
	db3 = xilInfoFrame->info_length&0xFF;
	db4 = (dp_version<<2)|(xilInfoFrame->info_length>>8);
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_INFO_DATA(1), temp);

	/* Write #2 */
	db1 = xilInfoFrame->audio_channel_count |
	      (xilInfoFrame->audio_coding_type<<4) | (RSVD<<3);
	db2 = (RSVD<<5)| (xilInfoFrame->sampling_frequency<<2) |
	      xilInfoFrame->sample_size;
	db3 = RSVD;
	db4 = xilInfoFrame->channel_allocation;
	temp = db4 << 24 | db3 << 16 | db2 << 8 | db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_INFO_DATA(1), temp);

	/* Write #3 */
	db1 = (xilInfoFrame->level_shift << 3) | RSVD |
	      (xilInfoFrame->downmix_inhibit << 7);
	db2 = RSVD;
	db3 = RSVD;
	db4 = RSVD;
	temp = db4 << 24 | db3 << 16 | db2 << 8 | db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_INFO_DATA(1), temp);

	/* Write #4 */
	db1 = RSVD;
	db2 = RSVD;
	db3 = RSVD;
	db4 = RSVD;
	temp = 0x00000000;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_INFO_DATA(1), temp);
	/* Write #5 */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_INFO_DATA(1), temp);

	/* Write #6 */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_INFO_DATA(1), temp);
	/* Write #7 */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_INFO_DATA(1), temp);
	/* Write #8 */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_AUDIO_INFO_DATA(1), temp);
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
XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane)
{
	u8 *Ptm;
	u16 HBlank;
	u16 VBlank;
	u32 PixelClockHz;
	XVidC_FrameRate FrameRate;
	XVidC_VideoTiming Timing;
	XVidC_VideoMode VmId;
	u8 bpp;
	double pixel_freq, pixel_freq1;
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
u8 get_Lanecounts(void)
{
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
		xil_printf("+++++++ TX GT configuration "
			   "encountered a failure +++++++\r\n");
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
void sink_power_cycle(void)
{
	/* Waking up the monitor */
	sink_power_down();
	/* give enough time for monitor to power down */
	usleep(400);
	sink_power_up();
	/* give enough time for monitor to wake up CR-962717 */
	usleep(30000);
	/* Monitor to wake up once again due to CR-962717 */
	sink_power_up();
	usleep(4000);
}


/*****************************************************************************/
/**
*
* This function is the main loop function of Tx
*
* @param	none.
*
* @return	none.
*
* @note		None.
*
******************************************************************************/
void tx_main_loop()
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
	u8 Edid_org[128], Edid1_org[128];
	u8 done=0;
	u32 user_tx_LaneCount , user_tx_LineRate;
	u32 aux_reg_address, num_of_aux_registers;
	u8 Data[8];
#if ENABLE_AUDIO
	u8 audio_on=0;
	XilAudioInfoFrame *xilInfoFrame;
#endif
	u8 in_pwr_save = 0;
	u16 DrpVal =0;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13,
					  0x14, 0x15, 0x16, 0x17}; //Duplicate

	unsigned char bpc_table[] = {6,8,10,12,16};


	user_config_struct user_config;
	user_config.user_bpc=8;
	user_config.VideoMode_local=XVIDC_VM_800x600_60_P;
	user_config.user_pattern=1;
	user_config.user_format = XVIDC_CSF_RGB;

#if ENABLE_AUDIO
	xilInfoFrame = 0; // initialize
#endif

	sub_help_menu ();

	while (1) { // for menu loop
		if (tx_is_reconnected == 1) {
			hpd_con(&DpTxSsInst, Edid_org, Edid1_org,
					user_config.VideoMode_local);
			tx_is_reconnected = 0;
		}

		if(hpd_pulse_con_event == 1){
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst);
			if(XDpTxSs_CheckLinkStatus(&DpTxSsInst)){
				sink_power_cycle();
			}
		}


		CmdKey[0] = 0;
		CommandKey = 0;


		CommandKey = xil_getc(0xff);
		Command = atoi(&CommandKey);
		if (Command != 0) {
			xil_printf("You have selected command %d\r\n", Command);
		}

		switch (CommandKey){
		case 'e':
			xil_printf ("EDID read is :\r\n");

			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
			for (i=0;i<128;i++) {
				if(i%16==0 && i != 0)
					xil_printf("\r\n");
				xil_printf ("%02x ", Edid_org[i]);
			}

			xil_printf ("\r\r\n\n");
			//reading the second block of EDID
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);

			for (i=0;i<128;i++) {
				if(i%16==0 && i != 0)
					xil_printf("\r\n");
				xil_printf ("%02x ", Edid1_org[i]);
			}
			xil_printf ("\r\nEDID read over =======\r\n");

			break;

		case 'd' :

			if (in_pwr_save == 0) {
				sink_power_down();
				in_pwr_save = 1;
				xil_printf (
					"\r\n==========power down===========\r\n");
			} else {
				sink_power_up();
				in_pwr_save = 0;
				xil_printf (
					"\r\n==========power up===========\r\n");

				hpd_con(&DpTxSsInst, Edid1_org, Edid1_org,
				user_config.VideoMode_local);
			}
			break;


#if ENABLE_AUDIO
		case 'a' :
			audio_on = XDp_ReadReg(
					DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_CONTROL);
			if (audio_on == 0) {
				xilInfoFrame->audio_channel_count = 0;
				xilInfoFrame->audio_coding_type = 0;
				xilInfoFrame->channel_allocation = 0;
				xilInfoFrame->downmix_inhibit = 0;
				xilInfoFrame->info_length = 27;
				xilInfoFrame->level_shift = 0;
				xilInfoFrame->sample_size = 1;//16 bits
				xilInfoFrame->sampling_frequency = 3; //48 Hz
				xilInfoFrame->type = 4;
				xilInfoFrame->version = 1;
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x0);
				sendAudioInfoFrame(xilInfoFrame);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CHANNELS, 0x1);
				switch(LineRate)
				{
					case  6:m_aud = 24576; n_aud = 162000; break;
					case 10:m_aud = 24576; n_aud = 270000; break;
					case 20:m_aud = 24576; n_aud = 540000; break;
				}
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_MAUD,  m_aud );
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_NAUD,  n_aud );

				Vpg_Audio_start();
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x1);
				xil_printf ("Audio enabled\r\n");
				audio_on = 1;
			} else {
				Vpg_Audio_stop();
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x0);
				xil_printf ("Audio disabled\r\n");
				audio_on = 0;
			}
			break;
#endif
		case '1' :
			//resolution menu
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();
			resolution_help_menu();
			exit = 0;
			while (exit == 0) {
				CmdKey[0] = 0;
				Command = 0;
				CmdKey[0] = inbyte_local();
				if(CmdKey[0]!=0){
					Command = (int)CmdKey[0];

					switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
					xil_printf("You have selected command '%c'\r\n",
															CmdKey[0]);
						if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
							Command = CmdKey[0] -'a' + 10;
							done = 1;
						}

						else if (Command > 47 && Command < 58) {
							Command = Command - 48;
							done = 1;
						}
						else if (Command >= 58 || Command <= 47) {
							resolution_help_menu();
							done = 0;
							break;
						}
						xil_printf ("\r\nSetting resolution...\r\n");
#if ENABLE_AUDIO
						audio_on = 0;
#endif
						user_config.VideoMode_local =
											resolution_table[Command];


						start_tx (LineRate,LaneCount,user_config, 0);
						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();

						exit = done;
						break;
					}
				}
			}

			sub_help_menu ();
			break;

		case '2' :
			// BPC menu
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();

			exit = 0;
			bpc_help_menu(DPTXSS_BPC);
			while (exit == 0) {
				CommandKey = 0;
				Command = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					switch  (CommandKey)
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
							Command = Command - 48;
							user_config.user_bpc = bpc_table[Command];
							xil_printf("You have selected %c\r\n",
															CommandKey);
							if((Command>4) || (Command < 0))
							{
								bpc_help_menu(DPTXSS_BPC);
								done = 0;
								break;
							}
							else
							{
								xil_printf("Setting BPC of %d\r\n",
												user_config.user_bpc);
								done = 1;
							}
							start_tx (LineRate, LaneCount,user_config, 0);
							LineRate = get_LineRate();
							LaneCount = get_Lanecounts();
							exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '3' :
			xil_printf("Select the Link and Lane count\r\n");
			exit = 0;
			select_link_lane();
			while (exit == 0) {
				CmdKey[0] = 0;
				Command = 0;
				CmdKey[0] = inbyte_local();
				if(CmdKey[0]!=0){
					Command = (int)CmdKey[0];
					Command = Command - 48;
					switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
						xil_printf("You have selected command %c\r\n",
															CmdKey[0]);
						if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
							Command = CmdKey[0] -'a' + 10;
						}

						if((Command>=0)&&(Command<12))
						{
							user_tx_LaneCount =
									lane_link_table[Command].lane_count;
							user_tx_LineRate =
									lane_link_table[Command].link_rate;
							if(lane_link_table[Command].lane_count
									> DpTxSsInst.Config.MaxLaneCount)
							{
								xil_printf(
					"This Lane Count is not supported by Sink \r\n");
								xil_printf(
					"Max Supported Lane Count is 0x%x \r\n",
										DpTxSsInst.Config.MaxLaneCount);
								xil_printf(
					"Training at Supported Lane count  \r\n");
							LaneCount = DpTxSsInst.Config.MaxLaneCount;
							}
							done = 1;
						}
						else
						{
							xil_printf(
							"!!!Warning: You have selected wrong option"
							" for lane count and link rate\r\n");
							select_link_lane();
							done = 0;
							break;
						}
						// Disabling TX interrupts
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_INTERRUPT_MASK, 0xFFF);
						LineRate_init_tx = user_tx_LineRate;
						Status = set_vphy(LineRate_init_tx);

						XDpTxSs_Stop(&DpTxSsInst);
#if ENABLE_AUDIO
						audio_on = 0;
#endif
						xil_printf(
					"TX Link & Lane Capability is set to %x, %x\r\n",
					user_tx_LineRate, user_tx_LaneCount);
						xil_printf(
					"Setting TX to 8 BPC and 800x600 resolution\r\n");
						XDpTxSs_Reset(&DpTxSsInst);
						user_config.user_bpc=8;
						user_config.VideoMode_local
										=XVIDC_VM_800x600_60_P;
						user_config.user_pattern=1;
						user_config.user_format = XVIDC_CSF_RGB;
						start_tx (user_tx_LineRate, user_tx_LaneCount,
												user_config, 0);
						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();
						exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '4' :
			//pattern menu;
			test_pattern_gen_help();
			exit = 0;
			while (exit == 0) {
				CommandKey = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					Command = Command - 48;
					switch  (CommandKey)
					{
						case 'x' :
							exit = 1;
							sub_help_menu ();
							break;

						default :

							if(Command>0 && Command<8)
							{
								xil_printf(
								"You have selected video pattern %d "
								"from the pattern list \r\n", Command);
								done = 1;
							}
							else
							{
								xil_printf(
						"!!!Warning : Invalid pattern selected \r\n");
								test_pattern_gen_help();
								done = 0;
								break;
							}
							user_config.user_pattern = Command;
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[
											user_config.user_pattern]);
							exit = done;
							break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '5' :
			//MSA;
			XDpTxSs_ReportMsaInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '6' :
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();
			exit = 0;
			format_help_menu();
			while (exit == 0) {
				CommandKey = 0;
				Command = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					switch  (CommandKey)
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
							Command = Command - 48;
							user_config.user_format = Command;
							xil_printf("You have selected %c\r\n",
															CommandKey);
							if((Command<=0)||(Command>3))
							{
								format_help_menu();
								done = 0;
								break;
							}
							else
							{
								xil_printf("Setting Format of %d\r\n",
											user_config.user_format);
								done = 1;
							}
							if(user_config.user_format!=1)
							{
							//Only Color Square is supported for YCbCr
								user_config.user_pattern = 3;
							}
							else
							{
								//Set Color Ramp for RGB (default)
								user_config.user_pattern = 1;
							}
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[
											user_config.user_pattern]);
							start_tx (LineRate, LaneCount,user_config, 0);
							LineRate = get_LineRate();
							LaneCount = get_Lanecounts();
							exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '7' :
			//Link config and status
			XDpTxSs_ReportLinkInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '8' :
			//Display DPCD reg
			XDpTxSs_ReportSinkCapInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '9' :
			//"9 - Read Aux registers\r\n"
			xil_printf(
		"\r\n Give 4 bit Hex value of base register 0x");
			aux_reg_address = xil_gethex(4);
			xil_printf(
			  "\r\n Give msb 2 bit Hex value of base register 0x");
			aux_reg_address |= ((xil_gethex(2)<<16) & 0xFFFFFF);
			xil_printf("\r\n Give number of registers that you "
								"want to read (1 to 9): ");
			num_of_aux_registers = xil_gethex(1);
			if((num_of_aux_registers<1)||(num_of_aux_registers>9))
			{
					xil_printf("\r\n!!!Warning: Invalid number "
				   "selected, hence reading only one register\r\n");
					num_of_aux_registers = 1;
			}
			xil_printf("\r\nGiven base address offset is 0x%x\r\n",
										aux_reg_address);
			for(i=0;i<num_of_aux_registers;i++)
			{
					Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
						(aux_reg_address+i), 1, &Data);
					if(Status == XST_SUCCESS)
					{
							xil_printf("Value at address offset "
						"0x%x, is = 0x%x\r\n",
										(aux_reg_address+i),
										((Data[0]) & 0xFF));
					} else {
							xil_printf("Aux Read failure\r\n");
							break;
					}
			}
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		// Display VideoPHY status
		case 'b':
			xil_printf("Video PHY Config/Status --->\r\n");
			xil_printf(" RCS (0x10) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_REF_CLK_SEL_REG));
			xil_printf(" PR  (0x14) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_PLL_RESET_REG));
			xil_printf(" PLS (0x18) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_PLL_LOCK_STATUS_REG));
			xil_printf(" TXI (0x1C) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_TX_INIT_REG));
			xil_printf(" TXIS(0x20) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_TX_INIT_STATUS_REG));
			xil_printf(" RXI (0x24) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_RX_INIT_REG));
			xil_printf(" RXIS(0x28) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_RX_INIT_STATUS_REG));

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_CPLL_FBDIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_FBDIV) "
				"= 0x%x, Val = 0x%x\r\n",XVPHY_DRP_CPLL_FBDIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_CPLL_REFCLK_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_REFCLK_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_CPLL_REFCLK_DIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_RXOUT_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_RXOUT_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_RXOUT_DIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_TXOUT_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_TXOUT_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_TXOUT_DIV,
				DrpVal
			);

			break;


			// CRC read
		case 'm' :
			//XVidFrameCrc_Report(&VidFrameCRC_tx);

			xil_printf ("========== Source CRC===========\r\n");
			xil_printf ("Txd Hactive =  %d\r\n",
					((XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0xC)&0xFFFF) + 1) *
								(XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x0)));
			xil_printf ("Txd Vactive =  %d\r\n",
					XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0xC)>>16);
			xil_printf ("CRC Cfg     =  0x%x\r\n",
					XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x0));
			xil_printf ("CRC - R/Y   =  0x%x\r\n",
					XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x4)&0xFFFF);
			xil_printf ("CRC - G/Cr  =  0x%x\r\n",
					XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x4)>>16);
			xil_printf ("CRC - B/Cb  =  0x%x\r\n",
					XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x8)&0xFFFF);



			/* Start TEST_CRC in sink*/

			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
				DPCD_TEST_SINK_MISC, 1, &Data);

			// Check if sink supports CRC function
			if((Data[0]&0x20)!=0){
				Data[0] = 0x1;
				Status = XDp_TxAuxWrite(DpTxSsInst.DpPtr,
						DPCD_TEST_SINK_START, 1, &Data);

				/*Wait till CRC is available or timeout*/
				u16 crc_wait_cnt=0;
				while(1){
					/* Read CRC availability every few ms*/
					Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
						DPCD_TEST_SINK_MISC, 1, &Data);
					//DpPt_CustomWaitUs(&DpTxSsInst,10000);
					usleep(10000);
					if((Data[0]&0x0F)!=0){
//						xil_printf("Sink CRC - Available...\r\n");
						break;
					}else if(crc_wait_cnt==CRC_AVAIL_TIMEOUT){
//						xil_printf("Sink CRC - Timed Out...\r\n");
						break;
					}else{
						crc_wait_cnt++;
					}
				}

				/*Wait time so that Tx and Rx has enough time to
				  calculate CRC values*/
				if((Data[0]&0x0F)!=0){ // Sink supports CRC
					usleep(100000);

					Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
							DPCD_TEST_CRC_R_Cr, 6, &Data);

					xil_printf ("========== Sink CRC ===========\r\n");
					xil_printf ("CRC - R/Cr   =  0x%x\r\n",
							Data[0] | (Data[1]<<8));
					xil_printf ("CRC - G/Y    =  0x%x\r\n",
							Data[2] | (Data[3]<<8));
					xil_printf ("CRC - B/Cb   =  0x%x\r\n",
							Data[4] | (Data[5]<<8));
				}
			}

			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
		case 'z' :
			sub_help_menu ();
			break;
		case 'x' :
			xil_printf("exit Tx Only mode\r\n");

			XDpTxSs_Stop(&DpTxSsInst);
			XScuGic_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
			XScuGic_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

			Vpg_Audio_stop();
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_ENABLE, 0x0);
			XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);

			operationMenu();
			return;

		} //end of switch (CmdKey[0])
	}
}
