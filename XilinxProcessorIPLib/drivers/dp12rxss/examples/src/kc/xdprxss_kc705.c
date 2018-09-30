/*******************************************************************************
 *
 * Copyright (C) Xilinx, Inc.  All rights reserved.
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
/******************************************************************************/
/**
 *
 * @file dpp.c
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   KU   04/20/16 Initial release.
 * 1.1   KI   02/15/17 Vivado 2016.4 compartible code
 *            02/23/17 Added compliance related for Rx
 *            03/03/17 Added tx_preset for stability
 *
*******************************************************************************/

#include "dppt.h"

#if ENABLE_HDCP_IN_DESIGN
unsigned int gKeyMGMTBaseAddress[2] = {
		XPAR_DP_RX_HIER_HDCP_KEYMNGMT_BLK_0_BASEADDR,
		XPAR_DP_TX_HIER_HDCP_KEYMNGMT_BLK_1_BASEADDR};
#else
unsigned int gKeyMGMTBaseAddress[2] = {0, 0};
#endif

int gIsKeyWrittenInEeeprom = FALSE;

typedef unsigned int    	UINT32;
typedef unsigned int    	UINT8;
typedef unsigned int    	UINT16;

extern XIic IicInstance;

typedef u8 AddressType;
u8 si570_reg_value[NUM_MODES][NUM_CLOCK_REGS] = {
	// 7,     8,     9,    10,      11,      12,
	//- As per Si570 programmable oscillator calculator
	{0x4C, 0x42, 0xB0, 0x21, 0xDE, 0x77 }, // = {32kHz * 512)
	{0xA5, 0xC2, 0xAA, 0xCC, 0x9D, 0x51 }, // = (44.1kHz * 512)
	{0xE4, 0xC2, 0xF4, 0xB9, 0x4A, 0xA7 }, // = (48kHz * 512)
//	{0xE4, 0x42, 0xA9, 0x40, 0x40, 0x15 }, // = (48kHz * 512)
	{0xA2, 0XC2, 0XAA, 0XCC, 0X9D, 0X51 }, // = {88.2khZ * 512)
	{0x24, 0xC2, 0xB0, 0x21, 0xDE, 0x77 }, // = {96kHz * 512)
	{0xA1, 0x42, 0xAA, 0xCC, 0x9D, 0x51 }, // = (176.4kHz * 512)
	{0x22, 0x42, 0xB0, 0x21, 0xDE, 0x77 }  // = {192kHz * 512)
};

u8 UpdateBuffer[sizeof(AddressType) + PAGE_SIZE];
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];
u8 ReadBuffer[PAGE_SIZE];

struct dma_chan_parms dma_struct[1];

XDpTxSs_MainStreamAttributes Msa[4];

static XVphy_User_Config PHY_User_Config_Table[] =
{
  // Index,         TxPLL,               RxPLL,
//		TxChId,                    RxChId,         LineRate,
//		LineRateHz,                QPLLRefClkSrc,
//		CPLLRefClkSrc,       QPLLRefClkFreqHz,    CPLLRefClkFreqHz
  {   0,     XVPHY_PLL_TYPE_QPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN,     XVPHY_CHANNEL_ID_CHA,     0x06,
		  XVPHY_DP_LINK_RATE_HZ_162GBPS,      ONBOARD_REF_CLK,
		  DP159_FORWARDED_CLK,        162000000,           81000000        },
  {   1,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,     0x06,
		  XVPHY_DP_LINK_RATE_HZ_162GBPS,      DP159_FORWARDED_CLK,
		  DP159_FORWARDED_CLK,        81000000,            81000000        },
  {   2,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,     0x0A,
		  XVPHY_DP_LINK_RATE_HZ_270GBPS,      DP159_FORWARDED_CLK,
		  DP159_FORWARDED_CLK,        135000000,           135000000        },
  {   3,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,     0x14,
		  XVPHY_DP_LINK_RATE_HZ_540GBPS,      DP159_FORWARDED_CLK,
		  DP159_FORWARDED_CLK,        270000000,           270000000        },
  {   4,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,     0x06,
		  XVPHY_DP_LINK_RATE_HZ_162GBPS,      ONBOARD_REF_CLK,
		  ONBOARD_REF_CLK,            135000000,           135000000        },
  {   5,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,     0x0A,
		  XVPHY_DP_LINK_RATE_HZ_270GBPS,      ONBOARD_REF_CLK,
		  ONBOARD_REF_CLK,            135000000,           135000000        },
  {   6,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,     0x14,
		  XVPHY_DP_LINK_RATE_HZ_540GBPS,      ONBOARD_REF_CLK,
		  ONBOARD_REF_CLK,            135000000,           135000000        },

};
/* Local Globals */

/************************** Variable Definitions *****************************/
XUartLite UartLite; /* Instance of the UartLite device */
XGpio GpioLED; /* Instance of the Gpio8bitsLED */
XTmrCtr TmrCtr; /* Instance of the Timer/Counter */
XVphy VPhy_Instance;
XIntc IntcInst;		/* The interrupt controller instance.*/
XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
XDpTxSs_Config *DPTxSSConfig;
XDpRxSs DpRxSsInst;	/* The DPRX Subsystem instance.*/
XDpRxSs_Config *DPRxSSConfig;

volatile u32 mst_hpd_event=0;
user_config_struct user_config;

u8 StartTxAfterRx=0;
u16 RxTrainedFromMenu=0;
/**************************** Type Definitions *******************************/
u8 C_VideoUserStreamPattern[8] =
			{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17}; //Duplicate
u8 Bpc[] = {6, 8, 10, 12, 16};


/************************** Function Prototypes ******************************/

int DPPtIntrInitialize();
int DpPt_SetupIntrSystem();
void DpPt_HpdEventHandler(void *InstancePtr);
void DpPt_HpdPulseHandler(void *InstancePtr);
void DpPt_LinkrateChgHandler (void *InstancePtr);
void DpPt_TxSetMsaValuesImmediate(void *InstancePtr);
void Dprx_InterruptHandlerPllReset(void *InstancePtr);
void Dprx_InterruptHandlerLinkBW(void *InstancePtr);
void Dprx_InterruptHandlerTrainingDone(void *InstancePtr);
void Dprx_InterruptHandlerBwChange(void *InstancePtr);
void Dprx_InterruptHandlerInfoPkt(void *InstancePtr);
void Dprx_InterruptHandlerExtPkt(void *InstancePtr);
void Dprx_InterruptHandlerUplug(void *InstancePtr);
void Dprx_InterruptHandlerPwr(void *InstancePtr);
void Dprx_HdcpAuthCallback(void *InstancePtr);
void Dprx_HdcpUnAuthCallback(void *InstancePtr);
#if ENABLE_HDCP_IN_DESIGN
static void Dppt_TimeOutCallback(void *InstancePtr, u8 TmrCtrNumber);
#endif
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
int DPTxInitialize();
int DPRxInitialize();
int init_peripherals();
void hpd_con();
void hpd_pulse_con();
void start_tx(u8 line_rate, u8 lane_count,
				XVidC_VideoMode res_table, u8 bpc, u8 pat);
void prog_bb(u8 bw, u8 is_tx);
void vdma_stop();
void vdma_start();
void tx_preset();

static void Dprx_InterruptHandlerVmChange(void *InstancePtr);
static void Dprx_InterruptHandlerNoVideo(void *InstancePtr);
static void Dprx_InterruptHandlerVBlank(void *InstancePtr);
static void Dprx_InterruptHandlerTrainingLost(void *InstancePtr);
static void Dprx_InterruptHandlerVideo(void *InstancePtr);
static void Dprx_CheckSetupTx(void *InstancePtr);
static void Dprx_DetectResolution(void *InstancePtr);
static void Dprx_ResetVideoOutput(void *InstancePtr);

char GetInbyte(void);
char inbyte_local(void);

void Dprx_SetupTxWithCustomMsa(void *InstancePtr, u8 tx_with_msa);
void Dprx_SetupTx(void *InstancePtr, u8 tx_with_msa, 	XVidC_VideoMode VmId);
void app_help();
void bpc_help_menu();
void reset_clkwiz();
void clk_wiz_locked();
void reconfig_clkwiz ();
void resolution_help_menu();
void rx_help_menu();
void select_link_lane();
void select_rx_link_lane();
void sub_help_menu();
void test_pattern_gen_help();
int write_si570();
int ceil_func(double x);

u8 is_TX_CPLL=1;
u8 hdcp_capable = 0;
u8 hdcp_capable_org = 0;
u8 hdcp_repeater_org = 0;
u8 hdcp_repeater = 0;
u32 training_done_lane01;
u32 training_done_lane23;
u32 training_done_lanecnt;
u32 dp_msa_hres;
u32 dp_msa_vres;
u32 rxMsaMVid;
u32 rxMsaNVid;
u32 rxMsaMVid_track;
u32 rxMsaNVid_track;
u32 rxMsamisc0_track;
u32 bpc_track =0;
u8 LaneCount;
u8 LineRate;
u32 rxMsamisc0;
u32 DpHres_total, DpVres_total;
u32 recv_clk_freq=0;
float recv_frame_clk=0.0;
u8 pixel = 0;
u32 bpc = 0;
u8 comp = 0;
u8 comp_track = 0;
u8 prog_tx =0;
u32 usr_data_cnt_lane = 0;
u32 words_per_line = 0;
u32 recv_clk_see = 0;
u32 recv_clk_freq_track=0;
float recv_frame_clk_track=0.0;
u32 recv_frame_clk_int_track =0;
u32 recv_frame_clk_int =0;
u8 vdma_start_read = 0;
u8 tx_pat_source = 0;
volatile u32 vblank_done =0;
u8 rx_ran_once = 0;
u32 tx_bw;
u8 bw_change_flag = 0;
u8 internal_rx_tx = 0;
u8 manual_sel = 0;
u32 IsRxTrained = 0;
u32 training_done = 0;
u8 ooo = 0;
u32 IsResChange = 0;
u32 vblank_count =0;
u8 vdma_start_write = 0;
u32 training_done_lane01;
u32 training_done_lane23;
u8 start_tracking = 0;
u8 change_detected = 0;
u32 IsTxEncrypted = 0;
u32 IsTxAuthenticated = 0;
u32 bw_tp1;
u32 lane_tp1;
u32 initial_value;
u8 only_tx_active = 0;
u8 coming_from_rx = 0;
u8 coming_from_tx = 0;
u8 switch_to_rx = 0;
u8 rx_link_change_requested = 0;
u8 tp1_received = 0;
u32 linkrate_set = 0x14;
u32 linkrate_set2 = 0;
u8 switch_to_tx = 0;
u8 switch_to_patgen = 0;
u8 hpd_issued = 0;
u8 need_to_retrain_rx = 0;
u8 count = 0;
u8 Edid_org[128];
u8 Edid1_org[128];
u8 max_cap_lanes;
u8 max_cap_org;
u8 tx_disconnected = 0;
u8 tx_is_reconnected = 0;
u32 clk_reg0;
u32 clk_reg1;
u32 clk_reg2;
u32 wait_count = 0;
u8 enabled = 0;
u8 audio_on = 0;
u8 prog_clk = 0;
u8 hpd_pulse = 0;
u8 done = 0;
u8 pwr_dwn_x = 0;
u8 pat_update = 1;
u8 hdcp_on = 0;
u8 gt_stable = 0;
typedef struct
{
        u8 type;
        u8 version;
        u8 length;
        u8 audio_coding_type;
        u8 audio_channel_count;
        u8 sampling_frequency;
        u8 sample_size;
        u8 level_shift;
        u8 downmix_inhibit;
        u8 channel_allocation;
        u16 info_length;
} XilAudioInfoFrame;

XilAudioInfoFrame *xilInfoFrame;

//xilInfoFrame->audio_channel_count = 0;
//                	        xilInfoFrame->audio_coding_type = 0;
//                	        xilInfoFrame->channel_allocation = 0;
//                	        xilInfoFrame->downmix_inhibit = 0;
//                	        xilInfoFrame->info_length = 27;
//                	        xilInfoFrame->level_shift = 0;
//                	        xilInfoFrame->sample_size = 1;//16 bits
//                	        xilInfoFrame->sampling_frequency = 3; //48 Hz
//                	        xilInfoFrame->type = 4;
//                	        xilInfoFrame->version = 1;

void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame);

int ii, m_aud, n_aud,misc0;



volatile u32 SstHpdEvent = 0;
unsigned char bpc_table[] = {6,8,10,12,16};
typedef struct
{
        unsigned char lane_count;
        unsigned char link_rate;
}lane_link_rate_struct;


lane_link_rate_struct lane_link_table[]=
{
	{XDP_RX_OVER_LANE_COUNT_SET_1,XDP_RX_OVER_LINK_BW_SET_162GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_2,XDP_RX_OVER_LINK_BW_SET_162GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_4,XDP_RX_OVER_LINK_BW_SET_162GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_1,XDP_RX_OVER_LINK_BW_SET_270GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_2,XDP_RX_OVER_LINK_BW_SET_270GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_4,XDP_RX_OVER_LINK_BW_SET_270GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_1,XDP_RX_OVER_LINK_BW_SET_540GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_2,XDP_RX_OVER_LINK_BW_SET_540GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_4,XDP_RX_OVER_LINK_BW_SET_540GBPS},

};
u32 user_lane_count;
u32 user_link_rate;
u32 user_tx_LaneCount;
u32 user_tx_LineRate;


XVidC_VideoMode resolution_table[] =
{
                XVIDC_VM_640x480_60_P,
                XVIDC_VM_480_60_P,
                XVIDC_VM_800x600_60_P,
                XVIDC_VM_1024x768_60_P,
                XVIDC_VM_720_60_P,
                XVIDC_VM_1600x1200_60_P,
                XVIDC_VM_1366x768_60_P,
                XVIDC_VM_1080_60_P,
                XVIDC_VM_UHD_30_P,
                XVIDC_VM_UHD_60_P,
                XVIDC_VM_2560x1600_60_P,
                XVIDC_VM_1280x1024_60_P,
                XVIDC_VM_1792x1344_60_P,
                XVIDC_VM_848x480_60_P,
                XVIDC_VM_1280x960_60_P,
                XVIDC_VM_1920x1440_60_P,
                XVIDC_VM_USE_EDID_PREFERRED
};


///Start of Main

int main(void)
{

	/* Initialize ICache */
	Xil_ICacheInvalidate ();
	Xil_ICacheEnable ();
	/* Initialize DCache */
	Xil_DCacheInvalidate ();
	Xil_DCacheEnable ();

    xil_printf("\n*******************************************************\n\r");
	char UserInput;

	u32 Status;
	u8 LineRate_init = 0x14;
	u8 LineRate_init_tx = 0x14;
	u8 LaneCount_init = 0x4;
	u8 LaneCount_init_tx = 0x4;
    u32 tmp_rd = 0;
	u32 data, addr;
	u8 MainMenu =0;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	int i =0;
	int j =0;
	int track_count = 0;
	int track_count1 = 0;
	int track_switch = 0;
	u8 exit = 0;
	u8 connected = 0;
	u8 Data[8];
	u32 count_track = 0;
	u8 pwr_dwn = 0;

	u32 aux_reg_address,num_of_aux_registers;
	lmk();
    Dppt_Tx_SetRefClocks (0xA, 1);
	/* Initializing user_config parameters */
	user_config.user_numStreams = 1;
	user_config.user_bpc = 8;
	user_config.user_pattern = &StreamPattern[0];

	user_config.mst_check_flag=XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_MST_ENABLE;
	xil_printf("\n*******************************************************\n\r");

	init_peripherals();
    /* Initializing Interrupts */
#if ENABLE_AUDIO

    for( i = 0; i < 6; i++ )
    {
            UpdateBuffer[i] = si570_reg_value[2][i];
    }
    write_si570();
    xil_printf("SI570 Config done\n\r");
#endif
    // reset VDMA to ensure clean recovery
    vdma_stop();
	xil_printf("\nVDMA has been reset\n\r");
    xil_printf("\033[H\033[J"); //clears the screen
#if XPAR_XDPRXSS_NUM_INSTANCES
	DPRxInitialize();
#endif

    clk_reg0 = Xil_In32 (CLK_WIZ_BASE+0x200);
    clk_reg1 = Xil_In32 (CLK_WIZ_BASE+0x204);
    clk_reg2 = Xil_In32 (CLK_WIZ_BASE+0x208);

	xil_printf("\n*******************************************************\n\r");
	xil_printf("            DisplayPort Pass Through Demonstration       \n\r");
	xil_printf("                   (c) 2015 by Xilinx\n\r\r\n");
	xil_printf("                   System Configuration:\r\n");
	xil_printf("                      DP SS : %d byte\r\n",2*SET_TX_TO_2BYTE);
	xil_printf("                      HDCP  : %d \r\n",ENABLE_HDCP_IN_DESIGN);
	xil_printf("\n*******************************************************\n\r");
#if COMPLIANCE
	xil_printf("\n***********APPLICATION IS IN COMPLIANCE MODE***********\n\r");
	xil_printf("\n***********APPLICATION IS IN COMPLIANCE MODE***********\n\r");
	xil_printf("\n***********APPLICATION IS IN COMPLIANCE MODE***********\n\r");
	xil_printf ("\r\n");
#endif

	DPTxInitialize();
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);

	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

    while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
	if (connected == 0) {
	xil_printf("Please connect a DP Monitor to start the application!\r\n");
	connected = 1;
	}
    }

	//Waking up the monitor
	pwr_dwn = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400);
	pwr_dwn = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);



    DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400000);
	//reading the first block of EDID
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
	XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
	//reading the second block of EDID
	XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
	xil_printf("Reading EDID contents of the DP Monitor..\r\n");

	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x1, 1, &max_cap_org);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2, 1, &max_cap_lanes);
	if (Status != XST_SUCCESS) {
		xil_printf ("Failed to read sink capabilities\r\n");
	}

#if CAP_OVER_RIDE
	LineRate_init = MAX_RATE;
	LineRate_init_tx = MAX_RATE;
	LaneCount_init = MAX_LANE;
	LaneCount_init_tx = MAX_LANE;
	initial_value = LineRate_init;
#else
	LineRate_init = max_cap_org;
	LineRate_init_tx = max_cap_org;
	LaneCount_init = max_cap_lanes&0x1F;
	LaneCount_init_tx = max_cap_lanes&0x1F;
	initial_value = LineRate_init;
#endif

	} else {
		xil_printf("Please connect a DP Monitor and try again !!!\r\n");
		return 0;
	}

#if USE_MONITOR_EDID
	xil_printf("Setting same EDID contents in DP RX..\r\n");
//	xil_printf("EDID blk 1\r\n");
//
//    for(i=0;i<128;i++){
//    	 xil_printf ("%x ",Edid_org[i]);
//     }
//	xil_printf("\r\nEDID blk 2\r\n");
//
//    for(i=0;i<128;i++){
//    	 xil_printf ("%x ",Edid1_org[i]);
//     }

	UINT8 edid_monitor[384];
	switch (Edid_org[126]){
	case 0:
		for(i=0; i<128; i++)
			edid_monitor[i] = Edid_org[i];
		break;
	case 1:
		for(i=0; i<128; i++)
			edid_monitor[i] = Edid_org[i];
		for(i=0; i<128; i++)
			edid_monitor[i+128] = Edid1_org[i];
		break;
	}

    for(i=0;i<(256*4);i=i+(16*4)){
        for(j=i;j<(i+(16*4));j=j+4){
            XDp_WriteReg (XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
			j, edid_monitor[(i/4)+1]);
        }
    }
    for(i=0;i<(256*4);i=i+4){
        XDp_WriteReg (XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
		i, edid_monitor[i/4]);
    }

#endif

#if !USE_MONITOR_EDID

	u8 edid[256] = {
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
		0x61, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x0E, 0x19, 0x01, 0x04, 0xB5, 0x3C, 0x22, 0x78,
		0x3A, 0x4D, 0xD5, 0xA7, 0x55, 0x4A, 0x9D, 0x24,
		0x0E, 0x50, 0x54, 0xBF, 0xEF, 0x00, 0xD1, 0xC0,
		0x81, 0x40, 0x81, 0x80, 0x95, 0x00, 0xB3, 0x00,
		0x71, 0x4F, 0x81, 0xC0, 0x01, 0x01, 0x4D, 0xD0,
		0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80, 0x30, 0x20,
		0x35, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x1A,
		0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80,
		0xB0, 0x58, 0x8A, 0x00, 0x54, 0x4F, 0x21, 0x00,
		0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x1D,
		0x50, 0x18, 0xA0, 0x3C, 0x04, 0x11, 0x00, 0xF0,
		0xF8, 0x38, 0xF0, 0x3C, 0x00, 0x00, 0x00, 0xFC,
		0x00, 0x58, 0x49, 0x4C, 0x49, 0x4E, 0x58, 0x20,
		0x44, 0x50, 0x0A, 0x20, 0x20, 0x20, 0x01, 0x19,

		0x02, 0x03, 0x27, 0x71, 0x4F, 0x01, 0x02, 0x03,
		0x11, 0x12, 0x13, 0x04, 0x14, 0x05, 0x1F, 0x90,
		0x0E, 0x0F, 0x1D, 0x1E, 0x23, 0x09, 0x17, 0x07,
		0x83, 0x01, 0x00, 0x00, 0x6A, 0x03, 0x0C, 0x00,
		0x00, 0x00, 0x00, 0x78, 0x20, 0x00, 0x00, 0x56,
		0x5E, 0x00, 0xA0, 0xA0, 0xA0, 0x29, 0x50, 0x30,
		0x20, 0x35, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00,
		0x1E, 0xE2, 0x68, 0x00, 0xA0, 0xA0, 0x40, 0x2E,
		0x60, 0x30, 0x20, 0x36, 0x00, 0x54, 0x4F, 0x21,
		0x00, 0x00, 0x1A, 0x01, 0x1D, 0x00, 0xBC, 0x52,
		0xD0, 0x1E, 0x20, 0xB8, 0x28, 0x55, 0x40, 0x54,
		0x4F, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A, 0xD0,
		0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55,
		0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x18, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0
	};

    for(i=0;i<(256*4);i=i+(16*4)){
        for(j=i;j<(i+(16*4));j=j+4){
            XDp_WriteReg (XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
                j, edid[(i/4)+1]);
        }
    }
    for(i=0;i<(256*4);i=i+4){
        XDp_WriteReg (XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR, i, edid[i/4]);
    }

#endif

     xil_printf("System capabilities set to: LineRate %x, LaneCount %x\r\n",
			 LineRate_init,LaneCount_init);
     DPPtIntrInitialize();

#if ENABLE_HDCP_IN_DESIGN
    u32 TxAuthAttempts = 0;
    u8 auxValues_org[9];
    XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x068028, 1, auxValues_org);
    hdcp_capable_org = auxValues_org[0] & 0x1;
    hdcp_repeater_org = auxValues_org[0] & 0x2;
    if ((hdcp_capable_org == 0)) { // || (hdcp_repeater_org == 0x2)) {
	hdcp_capable_org = 0;
        xil_printf ("HDCP feature is being disabled in the system\r\n");
    } else {
        xil_printf ("System is capable of displaying HDCP content...\r\n");
    }
    KEYMGMT_Init();
    XHdcp1xExample_Init();
    DpTxSsInst.Hdcp1xPtr->IsRepeater = 0;
    DpRxSsInst.Hdcp1xPtr->IsRepeater = 0;
    XHdcp1xExample_Enable();
#else
        xil_printf ("--->HDCP feature is not enabled in application<---\r\n");
#endif


#if ENABLE_AUDIO
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
    XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);

    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
#endif

	XDpRxSs_SetLinkRate(&DpRxSsInst, LineRate_init);
	XDpRxSs_SetLaneCount(&DpRxSsInst, LaneCount_init);
	XDpRxSs_Start(&DpRxSsInst);
	// programming AUX defer to 6
    tmp_rd = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x4);
    tmp_rd |= tmp_rd | 0x06000000;
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x4, tmp_rd);

// Setting RX link to disabled state. This is to ensure that Source gets enough
// time to authenticate and do the HDCP stuff (such as writing AKSVs)
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x0);
//	// programming the unplug time register of DP RX for long value
//	// else the cable unplug events come very frequently
#if COMPLIANCE
//Need short timeout for compliance test 5.3.2.1 - this is not ideal in interop
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_BS_IDLE_TIME, 0x007868C0);
#else
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_BS_IDLE_TIME, 0x047868C0);
#endif


	// Enabling only HDCP and/or TP1,2,3 and interrupts.
#if ENABLE_HDCP_IN_DESIGN
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_INTERRUPT_MASK, 0xFE00FFFD);
	XHdcp1xExample_Poll();
#else
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_INTERRUPT_MASK, 0xFFF87FFD);
#endif

	app_help();

	while(1)
	{
#if ENABLE_HDCP_IN_DESIGN
	XHdcp1xExample_Poll();
#endif
		UserInput = 0;
#if FOR_INTERNAL
		if (switch_to_tx == 1) {
			UserInput = 't';
			switch_to_tx = 0;
			switch_to_rx = 0;
			coming_from_rx = 1;
		} else {
		UserInput = GetInbyte();
		manual_sel = 1;
		}
#else
		UserInput = GetInbyte();
		manual_sel = 1;
#endif

		switch(UserInput)
		{

			case 't':
				//Ensuring HDCP is disabled
#if ENABLE_HDCP_IN_DESIGN
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
				XDpTxSs_HdcpDisable(&DpTxSsInst);
				XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
				XHdcp1xExample_Poll();
#endif
				MainMenu = 0;
				only_tx_active = 1;
				switch_to_tx =0;
				is_TX_CPLL = 1;
				start_tracking = 0;
				tp1_received = 0;
				rx_ran_once = 0;
				vblank_done = 0;
				if (manual_sel == 1) {
					XIntc_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
					XIntc_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_LINK_ENABLE, 0x0);
				}
				exit = 0;
				xil_printf ("************************************************");
				xil_printf ("*******************\r\n");
				xil_printf ("In this configuration the TX acts as Master. RX ");
				xil_printf ("is disabled.       \r\n");
				xil_printf ("This mode operates on the 135Mhz clock generated");
				xil_printf ("on the oscillator \r\n");
				xil_printf ("on the FMC board. CPLL is used for TX \r\n");
				xil_printf ("************************************************");
				xil_printf ("*******************\r\n");
				DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
				DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
				DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;
                                Msa[0].ComponentFormat = 0x0;
				LineRate = LineRate_init_tx;
				LaneCount = LaneCount_init_tx;
// This configures the vid_phy for line rate to start with
                switch(LineRate_init_tx)
                {
			case 0x6:
				prog_bb (0x6, 1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
											PHY_User_Config_Table[4]);
				break;

			case 0xA:
				prog_bb (0xA, 1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
											PHY_User_Config_Table[5]);
				break;

			case 0x14:
				prog_bb (0x14, 1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
											PHY_User_Config_Table[6]);
				break;
                }
                if (Status != XST_SUCCESS) {
			xil_printf ("+++++++ TX GT configuration encountered a ");
			xil_printf ("failure +++++++\r\n");
                }
// The clk_wiz that generates half of lnk_clk has to be programmed as soon
//		as lnk clk is valid
#if (ENABLE_HDCP_IN_DESIGN && SET_TX_TO_2BYTE==1)
                Xil_Out32 (CLK_2_GPIO_BASEADDR+0x8, 0x1);
                ComputeMandD_txlnk ((LineRate_init_tx*270*1000/40),
								LineRate_init_tx);
#endif
		    LaneCount_init_tx = LaneCount_init_tx & 0x7;
			start_tx (LineRate_init_tx, LaneCount_init_tx,
							resolution_table[2], 8, 1);
				IsRxTrained = 1;
				LineRate = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
				LaneCount = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
				// Enabling TX interrupts
				sub_help_menu ();
//			u32 hdcpTxCmdInProgress = 0;
				CmdKey[0] = 0;
				CommandKey = 0;

			while (MainMenu == 0) {

				if (tx_is_reconnected == 1) {
					hpd_con();
					tx_is_reconnected = 0;
				}

				CmdKey[0] = 0;
				CommandKey = 0;

#if ENABLE_HDCP_IN_DESIGN
			XHdcp1xExample_Poll();
#endif	/*ENABLE_HDCP_IN_DESIGN*/
				CommandKey = xil_getc(0xff);
				Command = atoi(&CommandKey);
				if (Command != 0) {
				xil_printf("You have selected command %d\r\n", Command);
				}
				switch (CommandKey)
				{
#if ENABLE_AUDIO
			case 'a' :
						if (audio_on == 0) {
//                           xil_printf ("\n\r Infoframe %x\r\n",xilInfoFrame);
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
								0x300, 0x0);
			        sendAudioInfoFrame(xilInfoFrame);
			        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								0x304, 0x1);
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
							XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
												0x0, 0x1);
							XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
												0x0, 0x2);
							XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
												0x10, 0x2);
							XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
												0x20, 0x2);

							//0x04120002 channel status    16.4 release
							XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
												0xA0, 0x10000244);

							//channel statu    16.4 release
							XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
												0xA4, 0x40000000);
							XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
												0x4, 0x202);
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x300, 0x1);
							xil_printf ("Audio enabled\r\n");
							audio_on = 1;
						} else {
							XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
												0x0, 0x0);
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x300, 0x0);
							xil_printf ("Audio disabled\r\n");
							audio_on = 0;
						}
					break;
#endif
					case '1' :
						//resolution menu
						LineRate =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
						LaneCount =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
						resolution_help_menu();
						exit = 0;
						while (exit == 0) {
						CmdKey[0] = 0;
						Command = 0;
						CmdKey[0] = inbyte_local();
						Command = (int)CmdKey[0];

					    switch  (CmdKey[0])
					    {
					       case 'x' :
						   exit = 1;
						   sub_help_menu ();
						   break;

					       default :
						xil_printf("You have selected command '%c'\n\r"
									,CmdKey[0]);
                            if (CmdKey[0] == 'a') {
                                    user_config.VideoMode_local
										= XVIDC_VM_2560x1600_60_P;
                                    Command = 10;
                                    done = 1;
                            } else if (CmdKey[0] == 'b') {
                                    user_config.VideoMode_local
										= XVIDC_VM_1280x1024_60_P;
                                    Command = 11;
                                    done = 1;
                            } else if (CmdKey[0] == 'c') {
                                    user_config.VideoMode_local
										= XVIDC_VM_1792x1344_60_P;
                                    Command = 12;
                                    done = 1;
                            } else if (CmdKey[0] == 'd') {
                                    user_config.VideoMode_local
										= XVIDC_VM_848x480_60_P;
                                    Command = 13;
                                    done = 1;
                            } else if (CmdKey[0] == 'e') {
                                    user_config.VideoMode_local
										= XVIDC_VM_1280x960_60_P;
                                    Command = 14;
                                    done = 1;
                            } else if (CmdKey[0] == 'f') {
                                    user_config.VideoMode_local
										= XVIDC_VM_1920x1440_60_P;
                                    Command = 15;
                                    done = 1;
                            }

							else if (Command > 47 && Command < 58) {
								Command = Command - 48;
								user_config.VideoMode_local
									= resolution_table[Command];
								done = 1;
							}
							else if (Command >= 58 || Command <= 47) {
								resolution_help_menu();
								done = 0;
								break;
							}
							xil_printf ("\r\nSetting resolution...\r\n");
							audio_on = 0;
							start_tx (LineRate, LaneCount,
										resolution_table[Command], 0,0);
							LineRate =
							  DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
							LaneCount =
							  DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
							exit = done;
							break;
						}
						}

						sub_help_menu ();
						break;

					case '2' :
						// BPC menu
						LineRate =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
						LaneCount =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
						exit = 0;
						bpc_help_menu();
						while (exit == 0) {
						CommandKey = 0;
						Command = 0;
						CommandKey = inbyte_local();
						Command = (int)CommandKey;
					    switch  (CommandKey)
					    {
					       case 'x' :
						   exit = 1;
						   sub_help_menu ();
						   break;

					       default :
								Command = Command - 48;
								bpc = bpc_table[Command];
								xil_printf("You have selected %c\r\n",
											CommandKey);
								if((Command>4) || (Command == 0))
								{
									bpc_help_menu();
									done = 0;
									break;
								}
								else
								{
									xil_printf("Setting BPC of %d\r\n", bpc);
									done = 1;
								}
								start_tx (LineRate, LaneCount, 0, bpc, 0);
								LineRate =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
								LaneCount =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
								exit = done;
							break;
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
							CmdKey[0] = GetInbyte();
							Command = (int)CmdKey[0];
							Command = Command - 48;
					    switch  (CmdKey[0])
					    {
					       case 'x' :
						   exit = 1;
						   sub_help_menu ();
						   break;

					       default :
							xil_printf("You have selected command %c\n\r",
										CmdKey[0]);
							if((Command>=0)&&(Command<9))
							{
								user_tx_LaneCount =
										lane_link_table[Command].lane_count;
								user_tx_LineRate =
										lane_link_table[Command].link_rate;
								if(lane_link_table[Command].lane_count >
										DpTxSsInst.Config.MaxLaneCount)
								{
				xil_printf("This Lane Count is not supported by Sink \n\r"
							"Max Supported Lane Count is 0x%x \n\r",
							DpTxSsInst.Config.MaxLaneCount);
				xil_printf("Training at Supported Lane count  \r\n");
									LaneCount = DpTxSsInst.Config.MaxLaneCount;
								}
								done = 1;
							}
							else
							{
				xil_printf("!!!Warning: You have selected ");
				xil_printf("wrong option for lane count and link rate\n\r");
								select_link_lane();
								done = 0;
								break;
							}
							// Disabling TX interrupts
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											0x144, 0xFFF);
							LineRate_init_tx = user_tx_LineRate;
							LaneCount_init_tx = user_tx_LaneCount;

						   switch(LineRate_init_tx)
						   {
							   case 0x6:
								   prog_bb (0x6,1);
								Status = PHY_Configuration_Tx(&VPhy_Instance,
											PHY_User_Config_Table[4]);
								 break;

							   case 0xA:
								   prog_bb (0xA,1);
								Status = PHY_Configuration_Tx(&VPhy_Instance,
											PHY_User_Config_Table[5]);
								 break;

							   case 0x14:
								   prog_bb (0x14,1);
								Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[6]);
								 break;
						   }
							if (Status != XST_SUCCESS) {
							xil_printf ("+++++++ TX GT configuration "
										"encountered a failure +++++++\r\n");
							}
#if (ENABLE_HDCP_IN_DESIGN && SET_TX_TO_2BYTE==1)
							Xil_Out32 (CLK_2_GPIO_BASEADDR+0x8, 0x1);
							ComputeMandD_txlnk ((LineRate_init_tx*270*1000/40),
									LineRate_init_tx);
#endif
							need_to_retrain_rx = 0;
							XDpTxSs_Stop(&DpTxSsInst);
							audio_on = 0;
							xil_printf("TX Link & Lane Capability is set to "
										"%x, %x\r\n",
									user_tx_LineRate, user_tx_LaneCount);
							xil_printf("Setting TX to 8 BPC and 800x600 "
										"resolution\r\n");
							XDpTxSs_Reset(&DpTxSsInst);
							start_tx (user_tx_LineRate, user_tx_LaneCount,
											resolution_table[2], 8, 1);
							LineRate =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
							LaneCount =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
							exit = done;
							break;
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
									xil_printf("You have selected video "
											"pattern %d from the "
											"pattern list \r\n", Command);
									done = 1;
								}
								else
								{
									xil_printf("!!!Warning : Invalid pattern "
											"selected \r\n");
									test_pattern_gen_help();
									done = 0;
									break;
								}
								pat_update = Command;
								Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
										C_VideoUserStreamPattern[pat_update]);
								exit = done;
								break;
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
						//EDID;
						XDptx_DbgPrintEdid(DpTxSsInst.DpPtr);
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
						//"9 - Read Aux registers\n\r"
		                xil_printf("\r\n Give 4 bit Hex value of base "
							"register 0x");
		                aux_reg_address = xil_gethex(4);
		                xil_printf("\r\n Give msb 2 bit Hex value of "
							"base register 0x");
		                aux_reg_address |= ((xil_gethex(2)<<16) & 0xFFFFFF);
		                xil_printf("\r\n Give number of registers that "
							"you want to read (1 to 9): ");
		                num_of_aux_registers = xil_gethex(1);
		                if((num_of_aux_registers<1)||(num_of_aux_registers>9))
		                {
		                        xil_printf("\r\n!!!Warning: Invalid number "
								"selected, hence reading only "
								"one register\r\n");
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
#if ENABLE_HDCP_IN_DESIGN
						case 'i' :
							if (hdcp_on == 0) {
									xil_printf ("\r\n==========TX HDCP enabled"
												"===========\r\n");
									XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
									XHdcp1xExample_Poll();
									XDpTxSs_HdcpEnable(&DpTxSsInst);
									XHdcp1xExample_Poll();
									XDpTxSs_Authenticate(&DpTxSsInst);
									XHdcp1xExample_Poll();
									XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
									XHdcp1xExample_Poll();
									hdcp_on = 1;
							} else {
									xil_printf ("\r\n==========TX HDCP disabled"
												"===========\r\n");
									XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
									XHdcp1xExample_Poll();
									XDpTxSs_HdcpDisable(&DpTxSsInst);
									XHdcp1xExample_Poll();
									XDpTxSs_SetPhysicalState(&DpTxSsInst,
																FALSE);
									XHdcp1xExample_Poll();
									hdcp_on = 0;
							}
							break;

					case 'p' :
						xil_printf ("\r\n==========TX HDCP Debug Data"
									"===========\r\n");
						XDpTxSs_ReportHdcpInfo(&DpTxSsInst);
						break;

#endif
						/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

					case 'x' :
						XDpTxSs_Stop(&DpTxSsInst);
						app_help ();
						MainMenu = 1;
						UserInput =0;
#if ENABLE_HDCP_IN_DESIGN
						XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
						XDpTxSs_HdcpDisable(&DpTxSsInst);
						XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
						XHdcp1xExample_Poll();
#endif
						XIntc_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
						XIntc_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
						XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
											0x0, 0x0);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_ENABLE, 0x0);
						XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144,
										0xFFF);
						break;

						/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
					case 'z' :
						sub_help_menu ();
						break;

					} //end of switch (CmdKey[0])
			} //end of while (abc == 0)

			break;

			case 's' :
			case 'r' :
				reconfig_clkwiz();
				reset_clkwiz ();
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
				XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR + 0x400, 0x0, 0x0);
				XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_SET_MSA),
								&DpPt_TxSetMsaValuesImmediate, &DpTxSsInst);
				tx_pat_source = 0;
				training_done = 0;
				start_tracking = 0;
				IsRxTrained = 0;
				if (UserInput == 'r') {
				xil_printf ("\r\n**************************%c****************"
							"*************************\r\n",UserInput);
				xil_printf ("In this configuration the RX acts as Master while"
							"the TX is used to\r\n");
				xil_printf ("display the video that is received on RX. This "
							"mode operates on the\r\n");
				xil_printf ("clock forwarded by DP159. CPLL is used for RX "
							"and TX\r\n");
				xil_printf ("************************************************"
							"*******************\r\n");

#if CAP_OVER_RIDE
				LineRate_init = MAX_RATE;
				LineRate_init_tx = MAX_RATE;
				LaneCount_init = MAX_LANE;
				LaneCount_init_tx = MAX_LANE;
				initial_value = LineRate_init;
#else
				LineRate_init = max_cap_org;
				LineRate_init_tx = max_cap_org;
				LaneCount_init = max_cap_lanes&0x1F;
				LaneCount_init_tx = max_cap_lanes&0x1F;
				initial_value = LineRate_init;
#endif
				is_TX_CPLL = 1;
				} else {
				xil_printf ("\r\n**************************%c***************"
							"**************************\r\n",UserInput);
				xil_printf ("In this configuration the RX acts as Master while"
							" the TX is used to\r\n");
				xil_printf ("display the video that is received on RX. This "
							"mode operates on the\r\n");
				xil_printf ("clock forwarded by DP159. RX uses CPLL, TX uses "
							"QPLL and they operate\r\n");
				xil_printf ("on independent reference clocks\r\n");
				xil_printf ("System capability is being set to: LineRate 0x6, "
							"LaneCount 4 \r\n");
				xil_printf ("************************************************"
							"*******************\r\n");
				LineRate_init = 0x6;
				LineRate_init_tx = 0x6;
				LaneCount_init = max_cap_lanes&0x1F;
				LaneCount_init_tx = max_cap_lanes&0x1F;
				initial_value = LineRate_init;
				is_TX_CPLL = 0;
				}
				if (manual_sel == 1) {
					XDpRxSs_SetLinkRate(&DpRxSsInst, LineRate_init);
					XDpRxSs_SetLaneCount(&DpRxSsInst, LaneCount_init);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_LINK_ENABLE, 0x0);
				}
				MainMenu = 0;
				switch_to_rx =0;
				only_tx_active = 0;
				XDpTxSs_Stop(&DpTxSsInst);
				xil_printf("RX Link & Lane Capability is set to %x, %x\r\n",
						(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr ,
								XDP_RX_DPCD_LINK_BW_SET)),
								(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_DPCD_LANE_COUNT_SET)));
				// Disabling TX interrupts
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
			if (manual_sel == 1) {
				switch(LineRate_init)
	                {
						case 0x6:
							if(is_TX_CPLL)
								Status = PHY_Configuration_Rx(&VPhy_Instance,
											PHY_User_Config_Table[1]);
							else {
								Dppt_Tx_SetRefClocks (0x6,0);
								Status = PHY_Configuration_Rx(&VPhy_Instance,
											PHY_User_Config_Table[0]);
							}
							break;

						case 0xA:
							Status = PHY_Configuration_Rx(&VPhy_Instance,
										PHY_User_Config_Table[2]);
							break;

						case 0x14:
							Status = PHY_Configuration_Rx(&VPhy_Instance,
										PHY_User_Config_Table[3]);
							break;
	                }
			}
                if (Status != XST_SUCCESS) {
			xil_printf ("+++++++ RX GT configuration encountered a "
						"failure +++++++\r\n");
                }
				if (manual_sel == 1) {
					XIntc_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
					XIntc_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
				}

				rx_help_menu();
				xil_printf ("Please plug in RX cable to initiate "
							"training...\r\n");
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_LINK_ENABLE, 0x1);
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								0x300, 0x1);
#if !COMPLIANCE
			while (vblank_done == 0) {
//                 // waiting until RX is completely trained
			}

#endif
				rx_ran_once = 1;
//				u32 hdcpCmdInProgress = 0;
			while(1)
			{
				if (tx_is_reconnected == 1) {
					hpd_con();
					tx_is_reconnected = 0;
				}
#if FOR_INTERNAL
                    // this kicks in when cable is unplugged. it is observed
				// that some monitors go into unrecoverable state when
				// cable is unplugged. Keeping GT in stable state
                    // ensures good recovery of monitor
                    if (switch_to_patgen == 1 || gt_stable == 1) {
						if (track_switch < 10000) {
							track_switch = track_switch + 1;
						} else {
							DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback
								= NULL;
							DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef
								= NULL;
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											0x144, 0xFFF);
							XDpTxSs_Stop(&DpTxSsInst);
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
											C_VideoUserStreamPattern[1]);
							vdma_stop();
							Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
							XDpTxSs_Reset(&DpTxSsInst);
#if ENABLE_HDCP_IN_DESIGN
							XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
							XDpTxSs_HdcpDisable(&DpTxSsInst);
							XDpTxSs_SetPhysicalState(&DpTxSsInst,
											hdcp_capable_org);
							XHdcp1xExample_Poll();
#endif
							if (need_to_retrain_rx == 0) {
								if (is_TX_CPLL == 1) {
//                            	Dppt_Tx_SetRefClocks (0x6,1);
									switch(LineRate)
									{
										case 0x6:
											prog_bb (0x6,1);
											Status = PHY_Configuration_Tx(
													&VPhy_Instance,
													PHY_User_Config_Table[4]);
										break;

										case 0xA:
											prog_bb (0xA,1);
											Status = PHY_Configuration_Tx(
													&VPhy_Instance,
													PHY_User_Config_Table[5]);
										break;

										case 0x14:
											prog_bb (0x14,1);
											Status = PHY_Configuration_Tx(
													&VPhy_Instance,
													PHY_User_Config_Table[6]);
										break;
									}
									if (Status != XST_SUCCESS) {
xil_printf ("+++++++ TX GT configuration encountered a failure +++++++\r\n");
									}
								} else {
								prog_bb (0x6, 1);
								Status = PHY_Configuration_Tx(
										&VPhy_Instance,
											PHY_User_Config_Table[0]);

								}

							pwr_dwn_x = 0x2;
							XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1,
									&pwr_dwn_x);
							DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 200000);
							pwr_dwn_x = 0x1;
							XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1,
									&pwr_dwn_x);
// switchover to TX mode only when cable is unplugged in normal RX opreration
							if (switch_to_patgen == 1) {
							only_tx_active = 1;
						start_tx (LineRate, LaneCount, resolution_table[2],
									8, 1);
						xil_printf (".");
							xil_printf ("\r\nPlease plug-in the DP RX cable to"
										"go back to passthrough mode\r\n");
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											0x300, 0x0);
							wait_count = 0;
							enabled = 0;
							}
						} else {
							xil_printf ("Monitor change was detected.. "
										"please unplug-plug DP RX cable\r\n");
						}
						switch_to_patgen = 0;
						gt_stable = 0;
						track_switch = 0;
                    }
                    }
#endif
                    //This module tracks for refresh rate change. Many GPUs
                    // do not re-train when the refresh is change This
                    // tracks the refresh rate ans restarts the TX if needed.
				if (start_tracking == 1) {
					if (count_track < 5000) {
						count_track = count_track + 1;
					} else {
							rxMsaMVid_track = (XDp_ReadReg(
									DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_MSA_MVID) & 0x00FFFFFF);
							rxMsaNVid_track = (XDp_ReadReg(
									DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_MSA_NVID) & 0x00FFFFFF);
							rxMsamisc0_track = ((XDp_ReadReg(
									DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_MSA_MISC0) >> 5) & 0x00000007);
							bpc_track = Bpc[rxMsamisc0_track];
							comp_track = ((XDp_ReadReg(
									DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_MSA_MISC0) >> 1) & 0x00000003);
							recv_clk_freq_track =
									((LineRate * 27.0) *
											rxMsaMVid_track)/rxMsaNVid_track;
							recv_frame_clk_track =
									ceil_func((recv_clk_freq_track*1000000.0) /
											(DpHres_total*DpVres_total));
							recv_frame_clk_int_track = recv_frame_clk_track;
                            if (recv_frame_clk_int_track == 59 ||
				recv_frame_clk_int_track == 61) {
							recv_frame_clk_int_track = 60;
						} else if (	recv_frame_clk_int_track == 29 ||
									recv_frame_clk_int_track == 31) {
							recv_frame_clk_int_track = 30;
						} else if (	recv_frame_clk_int_track == 74 ||
									recv_frame_clk_int_track == 76) {
							recv_frame_clk_int_track = 75;
						}
                            if ((recv_frame_clk_int_track
					!= recv_frame_clk_int)) {
				xil_printf ("Refresh rate changed from"
						"%d to %d\r\n",
										recv_frame_clk_int,
										recv_frame_clk_int_track);
                                change_detected = 1;
					        start_tracking = 0;
					        count_track = 0;
                            } else if ((bpc != bpc_track)){
				xil_printf ("BPC changed from %d to %d\r\n",
								bpc, bpc_track);
                                change_detected = 1;
					        start_tracking = 0;
					        count_track = 0;
                            } else if ((comp != comp_track)) {
				xil_printf ("Color pattern changed "
							"from %d to %d\r\n",
											comp, comp_track);
				change_detected = 1;
				start_tracking = 0;
				count_track = 0;
                            }
					}
				}
				//check 5000 times. if true then it's a real change else
				//  a bogus one due to cable unplug
				if (change_detected == 1 && training_done == 1) {
					track_count = track_count + 1;
					if (track_count == 5000) {
						xil_printf ("Restarting TX....\r\n");
						IsRxTrained = 0;
						//Dprx_InterruptHandlerVmChange (&DpTxSsInst);
						XDpTxSs_Stop(&DpTxSsInst);
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											0x300, 0x0);
							XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
											0x300, 0x0);
						Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
										C_VideoUserStreamPattern[1]);
						vdma_stop();
						Dprx_ResetVideoOutput(DpRxSsInst.DpPtr);
						Dprx_DetectResolution(DpRxSsInst.DpPtr);
						Dprx_CheckSetupTx(DpRxSsInst.DpPtr);
						start_tracking = 1;
						IsRxTrained = 1;
						wait_count = 0;
						change_detected = 0;
	                        track_count = 0;
					}
				} else {
					change_detected = 0;
                        track_count = 0;
				}
#if ENABLE_AUDIO
				if (IsRxTrained == 1 && wait_count < 10000) {
					wait_count = wait_count + 1;
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
									0x300, 0x0);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									0x300, 0x0);
					enabled = 0;
//	        			xil_printf ("%x",wait_count);
				} else if (IsRxTrained == 0) {
					wait_count = 0;
					enabled = 0;
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
									0x300, 0x0);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									0x300, 0x0);
				}

				if (wait_count == 10000 && enabled == 0) {
				     xilInfoFrame->audio_channel_count = 0;
						xilInfoFrame->audio_coding_type = 0;
						xilInfoFrame->channel_allocation = 0;
						xilInfoFrame->downmix_inhibit = 0;
						xilInfoFrame->info_length = 27;
						xilInfoFrame->level_shift = 0;
						xilInfoFrame->sample_size = 1;//16 bits
						xilInfoFrame->sampling_frequency = 4; //48 Hz
						xilInfoFrame->type = 4;
						xilInfoFrame->version = 1;
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										0x300, 0x0);
						sendAudioInfoFrame(xilInfoFrame);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										0x304, 0x1);
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
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										0x300, 0x0);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										0x300, 0x1);
						XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										0x300, 0x0);
						XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										0x300, 0x1);
						xil_printf ("Starting audio in passthrough mode..\r\n");
						enabled = 1;
				}
#endif
                   //Start the TX only when timer counter is done and when
			   //	training_done is still valid
			   //Many a times bogus interrupts put the sw to go into TX mode
			   //Bogus interrupts typically when training is on or
			   //  when cable is being unplugged
#if ENABLE_HDCP_IN_DESIGN
				if (prog_tx == 1 && DpRxSsInst.TmrCtrResetDone == 1 &&
						training_done == 1 && need_to_retrain_rx == 0)
#else
				if (prog_tx == 1 && training_done == 1
						&& need_to_retrain_rx == 0)
#endif
				{
					track_count1 = track_count1 + 1;
					if (track_count1 == 20000)
						{
							Dprx_DetectResolution(DpRxSsInst.DpPtr);
							// this is needed to ensure there are on hang
							//   issues when cable is unplugged
							if (training_done == 1)
							{
								XAxiVdma_DmaStop(&dma_struct[0].AxiVdma,
													XAXIVDMA_WRITE);
								XAxiVdma_DmaStop(&dma_struct[0].AxiVdma,
													XAXIVDMA_READ);
								Dprx_ResetVideoOutput(DpRxSsInst.DpPtr);
#if !JUST_RX
								Dprx_CheckSetupTx(DpRxSsInst.DpPtr);
#endif
								prog_tx =0;
								start_tracking = 1;
								change_detected = 0;
								IsRxTrained = 1;
                                wait_count = 0;
								track_count1 = 0;
#if ENABLE_HDCP_IN_DESIGN
							if(hdcp_capable_org == 1)
							{
								xil_printf("$");
								DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 2000000);
								xil_printf (".");
								XDpTxSs_SetLane(&DpTxSsInst,
	DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount); //LaneCount_init_tx);
								XDpTxSs_SetPhysicalState(&DpTxSsInst,
										!hdcp_capable_org);
								XHdcp1xExample_Poll();
								XDpTxSs_SetPhysicalState(&DpTxSsInst,
										hdcp_capable_org);
								XHdcp1xExample_Poll();
							}//hdcp_capable_org check
#endif
								} else {
									prog_tx = 0;
									start_tracking = 0;
									change_detected = 0;
									track_count1 = 0;
									XDpTxSs_Stop(&DpTxSsInst);
									XDpTxSs_Reset(&DpTxSsInst);
								}
						} else {//if (track_count1 == 2000)	hdcp_capable_org
							if (track_count1 == 10) {
								XDpTxSs_Stop(&DpTxSsInst);
								Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
										C_VideoUserStreamPattern[1]);
								XAxiVdma_DmaStop(&dma_struct[0].AxiVdma,
													XAXIVDMA_WRITE);
								XAxiVdma_DmaStop(&dma_struct[0].AxiVdma,
													XAXIVDMA_READ);
								Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
								XDpTxSs_Reset(&DpTxSsInst);

							}

						}
					} else {
#if ENABLE_HDCP_IN_DESIGN
						if(DpRxSsInst.TmrCtrResetDone == 1){
							prog_tx =0;
						}
#else
						//prog_tx =0;
#endif
					track_count1 = 0;
				}

#if ENABLE_HDCP_IN_DESIGN
					XHdcp1xExample_Poll();
					if(XHdcp1x_IsEncrypted(DpRxSsInst.Hdcp1xPtr)
							&& IsRxTrained && !need_to_retrain_rx
							&& XDpTxSs_IsConnected(&DpTxSsInst))
					{
//						xil_printf("&");
						if(XHdcp1x_IsEncrypted(DpTxSsInst.Hdcp1xPtr)==0)
						{
							xil_printf("*");
							if(TxAuthAttempts == 0)
							{
								DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1500000);
							}
							/* Waiting for authenticate to complete */
							DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 150000);
							TxAuthAttempts++;
							XHdcp1xExample_Poll();
							if (XDpTxSs_IsAuthenticated(&DpTxSsInst)==0)
							{
								/* DP TX State 10 : Un-authenticated */
								if(DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 10){
										XDpTxSs_Authenticate(&DpTxSsInst);
								}
								else if (
								  DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 0
								  ||
								  DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 11) {
									/* DP TX State 0 : Disabled
									 * DP TX State 11 : Phy-layer-down */
									XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
									XHdcp1xExample_Poll();
									XDpTxSs_HdcpEnable(&DpTxSsInst);
									XHdcp1xExample_Poll();
									XDpTxSs_Authenticate(&DpTxSsInst);
									XHdcp1xExample_Poll();
									XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
									XHdcp1xExample_Poll();
								}
							}
							else
							{
									DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 75000);
									TxAuthAttempts = 0;
									XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
							}

							if(TxAuthAttempts == 100)
							{
//										xil_printf("&");
								xil_printf(">>>> HDCPTX Authentication "
										"failed , going to colorbar mode \r\n");
									switch_to_tx = 1;
									TxAuthAttempts = 0;
									break;
							}
						}
					}
					else
					{
							/*
							 * Bring down TX encryption/authentication
							 */
							if (XDpTxSs_IsAuthenticated(&DpTxSsInst)==1)
							{
								xil_printf(".~\r\n");
								XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
								XDpTxSs_HdcpDisable(&DpTxSsInst);
								XDpTxSs_SetPhysicalState(&DpTxSsInst,
										hdcp_capable_org);
								XHdcp1xExample_Poll();
//								XDpTxSs_ReportHdcpInfo(&DpTxSsInst);
							}
					}
#endif
					CommandKey = xil_getc(0xff);
					Command = atoi(&CommandKey);
					if(CommandKey!=0)

						{
							xil_printf("%c\r\n",CommandKey);
							switch(CommandKey){

							case '1':
								select_rx_link_lane();
								CommandKey = GetInbyte();
								Command = (int)CommandKey;
								Command = Command -48;
								xil_printf("You have selected command=%d \n\r",
												Command);
								if((Command>=0)&&(Command<9))
								{
									user_lane_count =
											lane_link_table[Command].lane_count;
									user_link_rate =
											lane_link_table[Command].link_rate;
									if(lane_link_table[Command].lane_count >
										DpRxSsInst.DpPtr->Config.MaxLaneCount)
									{
										xil_printf("This Lane Count is not "
												"supported by Sink \n\r");
										xil_printf("Max Supported Lane Count "
												"is 0x%x \n\r",
										DpRxSsInst.DpPtr->Config.MaxLaneCount);
										xil_printf("Training at Supported "
												"Lane count  \r\n");
										user_lane_count =
										  DpRxSsInst.DpPtr->Config.MaxLaneCount;
									}
									if(lane_link_table[Command].link_rate >
									  DpRxSsInst.DpPtr->Config.MaxLinkRate)
									{
										xil_printf("This link rate is not "
												"supported by Sink \n\r");
										xil_printf("Max Supported Link Rate is "
										"0x%x \n\r",
										DpRxSsInst.DpPtr->Config.MaxLinkRate);

										xil_printf("Training at supported Link "
											"Rate\r\n");
										user_link_rate =
										   DpRxSsInst.DpPtr->Config.MaxLinkRate;
									}
									xil_printf("RX Link & Lane Capability is "
										"set to %x, %x\r\n",
										user_link_rate, user_lane_count);
									xil_printf ("\r\n **Important: Please "
										"ensure to unplug & plug the cable "
										"after the capabilities have been "
										"changed **\r\n");
									XDp_ReadReg(
									  DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
									XDp_WriteReg(
											DpTxSsInst.DpPtr->Config.BaseAddr,
											0x144, 0xFFF);
									Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
											C_VideoUserStreamPattern[1]);
									vdma_stop();
									reconfig_clkwiz();
									start_tracking = 0;
									change_detected = 0;
									IsRxTrained = 0;
									rx_link_change_requested = 1;
									XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
											0x7FF8FFFF);
									// Disabling TX interrupts
									XDpTxSs_Stop(&DpTxSsInst);
									XDpRxSs_SetLinkRate(&DpRxSsInst,
											user_link_rate);
									XDpRxSs_SetLaneCount(&DpRxSsInst,
											user_lane_count);
									initial_value = user_link_rate;
									switch(user_link_rate)
									{
										case 0x6:
											if(is_TX_CPLL) {
												Dppt_Tx_SetRefClocks (0x6,1);
												Status = PHY_Configuration_Rx(
													&VPhy_Instance,
													PHY_User_Config_Table[1]);
											} else {
												Dppt_Tx_SetRefClocks (0x6,0);
												Status = PHY_Configuration_Rx(
													&VPhy_Instance,
													PHY_User_Config_Table[0]);
											}
											break;
										case 0xA:
											if (is_TX_CPLL == 0) {
											xil_printf (
			"Switching to CPLL as this line rate is not supported by QPLL\r\n");
											is_TX_CPLL = 1;
											Dppt_Tx_SetRefClocks (0xA,1);
											}
											Status = PHY_Configuration_Rx(
												&VPhy_Instance,
												PHY_User_Config_Table[2]);
											break;
										case 0x14:
											if (is_TX_CPLL == 0) {
											xil_printf (
			"Switching to CPLL as this line rate is not supported by QPLL\r\n");
											is_TX_CPLL = 1;
											Dppt_Tx_SetRefClocks (0x14,1);
											}
											Status = PHY_Configuration_Rx(
												&VPhy_Instance,
												PHY_User_Config_Table[3]);
											break;
									 }

									if (Status != XST_SUCCESS) {
										xil_printf (
			"+++++++ RX GT configuration encountered a failure +++++++\r\n");
									}
								}
								else
								{
									xil_printf("!!!Warning: You have selected "
				"wrong option for lane count and link rate =%d \n\r",Command);
									break;
								}

								break;

							case '2':
							//	debug_info();
								xil_printf (
									"==========RX Debug Data===========\r\n");
							XDpRxSs_ReportLinkInfo(&DpRxSsInst);
							XDpRxSs_ReportMsaInfo(&DpRxSsInst);
							xil_printf (
								"==========TX Debug Data===========\r\n");
								XDpTxSs_ReportMsaInfo(&DpTxSsInst);
								XDpTxSs_ReportLinkInfo(&DpTxSsInst);

								break;

							case '3':
								start_tracking = 0;
								change_detected = 0;
								hpd_issued = 1;
								IsRxTrained = 0;
								rx_link_change_requested = 1;
								XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
														0xFFF8FFFF);
								XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
														0x80000000);
								// Disabling TX interrupts
								XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
														0x144, 0xFFF);
								XDpTxSs_Stop(&DpTxSsInst);
								XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
											XDP_RX_HPD_INTERRUPT,0x0BB80001);
//								Dprx_InterruptHandlerVmChange(DpRxSsInst.DpPtr);
								xil_printf("\r\n- HPD Toggled for 3ms! -\n\r");
								break;

							case '4':
								xil_printf ("Restarting TX...\r\n");
								if (need_to_retrain_rx == 0) {
#if !JUST_RX

#if ENABLE_HDCP_IN_DESIGN
								IsRxTrained = 0;
								XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
								XDpTxSs_HdcpDisable(&DpTxSsInst);
								XDpTxSs_SetPhysicalState(&DpTxSsInst,
										hdcp_capable_org);
								XHdcp1xExample_Poll();
#endif
								prog_tx = 1;
#endif
								} else {
									xil_printf (
			"Monitor change was detected.. please unplug-plug DP RX cable\r\n");
								}
                                break;


							case '5':
								// Disabling TX interrupts
								if (need_to_retrain_rx == 0) {

								XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x144, 0xFFF);
								Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
												C_VideoUserStreamPattern[1]);
								DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);
						        Status = XDpTxSs_StartCustomMsa(&DpTxSsInst,
																Msa);
//						        if (Status != XST_SUCCESS) {
//						        	xil_printf ("TX Start failure\r\n");
//						        }
								xil_printf(
					"Switching TX to internal pattern generator ....\n\r");
						        XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											0x140);
						        // Enabling TX interrupts
						        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										0x144, 0x0);
								tx_pat_source = 1;
								xil_printf("Stopping VDMA ....");
								vdma_stop();
								xil_printf("done !\n\r");
								} else {
									xil_printf (
			"Monitor change was detected.. please unplug-plug DP RX cable\r\n");
								}
                                break;

							case '6':
								// Disabling TX interrupts
								if (need_to_retrain_rx == 0) {
								XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x144, 0xFFF);
								xil_printf("Re-starting VDMA ....");
								vdma_start();
						        xil_printf ("done !\r\n");
								Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
												C_VideoUserStreamPattern[0]);
								xil_printf("\r\nSwitching TX to RX video\n\r");
								tx_pat_source = 0;
						        DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 800000);
						        Status = XDpTxSs_StartCustomMsa(&DpTxSsInst,
																Msa);
//						        if (Status != XST_SUCCESS) {
//						        	xil_printf ("TX Start failure\r\n");
//						        }
						        XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											0x140);
						        // Enabling TX interrupts
						        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											0x144, 0x0);
								} else {
									xil_printf (
			"Monitor change was detected.. please unplug-plug DP RX cable\r\n");
								}
                                break;

					case 'w':
								dbg_printf(
					"\n\rEnter 4 hex characters: Sink Write address offset 0x");
								addr = xil_gethex(4);
								dbg_printf(
							"\n\rEnter 4 hex characters: Sink Write Data 0x");
								data = xil_gethex(4);
								XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
												addr, data);
								break;

							case 'r':
								dbg_printf(
					"\n\rEnter 4 hex characters: Sink Read address offset 0x");
								addr = xil_gethex(4);
								data = XDp_ReadReg(
									DpRxSsInst.DpPtr->Config.BaseAddr, addr);
								dbg_printf("\n\rSink Read Addr %04x Read Data:"
							"%04x\n\r", (XPAR_DPRXSS_0_BASEADDR+addr), data);
								break;

							case 'p':
								xil_printf (
							"\r\n==========RX HDCP Debug Data===========\r\n");
								XDpRxSs_ReportHdcpInfo(&DpRxSsInst);
								xil_printf (
							"\r\n==========TX HDCP Debug Data===========\r\n");
								XDpTxSs_ReportHdcpInfo(&DpTxSsInst);
                                break;

							case 's':
								xil_printf("DP RX Bandwidth is set to = %x\r\n"
								,XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_DPCD_LINK_BW_SET));
							    xil_printf("DP RX Lane Count is set to = %x\r\n"
							    ,XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_DPCD_LANE_COUNT_SET));
							    xil_printf("[LANE0_1 Status] = %x "
							    ,XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_DPCD_LANE01_STATUS));
							    if(XDp_ReadReg(
									DpRxSsInst.DpPtr->Config.BaseAddr,
											XDP_RX_DPCD_LANE_COUNT_SET)>2)
							    {
								    xil_printf(", [LANE2_3 Status] = %x",
									XDp_ReadReg(
										DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_DPCD_LANE23_STATUS));
							    }
							    dbg_printf("\r\n DP Symbol Error Registers: "
								"%x; %x \r\n", XDp_ReadReg(
									DpRxSsInst.DpPtr->Config.BaseAddr,
											XDP_RX_DPCD_SYM_ERR_CNT01),
									   (XDp_ReadReg(
											DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_DPCD_SYM_ERR_CNT23)));
								break;

							case 'z' :
								rx_help_menu();
								break;
							case 'x' :
								XIntc_Disable(&IntcInst,
										XINTC_DPTXSS_DP_INTERRUPT_ID);
								XIntc_Disable(&IntcInst,
										XINTC_DPRXSS_DP_INTERRUPT_ID);
								XDpTxSs_Stop(&DpTxSsInst);
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_LINK_ENABLE, 0x0);
								Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
										C_VideoUserStreamPattern[1]);
								vdma_stop();
								MainMenu = 1;
								rx_ran_once = 0;
								UserInput =0;
								vblank_done = 0;
								vblank_count =0;
								IsRxTrained = 0;
								wait_count = 0;
								enabled = 0;
								pwr_dwn_x = 0x2;
								XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1,
										&pwr_dwn_x);
								DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400);
								pwr_dwn_x = 0x1;
								XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1,
										&pwr_dwn_x);

								// default to 135Mhz
								Dppt_Tx_SetRefClocks (0x6,1);
								XGpio_WriteReg (XPAR_AV_PAT_GEN_0_BASEADDR
										+ 0x400, 0x0, 0x0);
		                        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							0x300, 0x0);
		                        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							0x300, 0x0);
								app_help ();
#if ENABLE_HDCP_IN_DESIGN
								XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_INTERRUPT_MASK, 0xFE00FFFF);
#else
								XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_INTERRUPT_MASK, 0xFFF87FFF);
#endif

#if ENABLE_HDCP_IN_DESIGN
								XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
								XDpTxSs_HdcpDisable(&DpTxSsInst);
								XDpTxSs_SetPhysicalState(&DpTxSsInst,
															hdcp_capable_org);
								XHdcp1xExample_Poll();
								XDpRxSs_SetPhysicalState(&DpRxSsInst, FALSE);
								XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
								XHdcp1xExample_Poll();
								XDpTxSs_SetPhysicalState(&DpTxSsInst, FALSE);
								XHdcp1xExample_Poll();
								XDpRxSs_StopTimer(&DpRxSsInst);
								IsTxEncrypted = 0;
								IsTxAuthenticated = 0;
#endif

// All links down
								XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x140);
								XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x144, 0xFFF);
								XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_LINK_ENABLE, 0x0);

								break;
#if COMPLIANCE
							case 'm':
//to manually start tx in compliance mode
							    training_done_lane01 = XDp_ReadReg(
							      DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
									  XDP_RX_DPCD_LANE01_STATUS);
							    training_done_lane23 = XDp_ReadReg(
							      DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
									  XDP_RX_DPCD_LANE23_STATUS);
							    xil_printf("> Interrupt: Training done !!! (BW:"
								" 0x%x, Lanes: 0x%x, Status: 0x%x;0x%x)"
								".\n\r",LineRate, LaneCount,
									training_done_lane01,training_done_lane23);

								Dprx_DetectResolution(DpRxSsInst.DpPtr);
								XAxiVdma_DmaStop(&dma_struct[0].AxiVdma,
											XAXIVDMA_WRITE);
								XAxiVdma_DmaStop(&dma_struct[0].AxiVdma,
											XAXIVDMA_READ);
								Dprx_ResetVideoOutput(DpRxSsInst.DpPtr);
								Dprx_CheckSetupTx(DpRxSsInst.DpPtr);
								break;
#endif
#if ENABLE_AUDIO
							case 'a':
								XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x304, 0x1);
								XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x300, 0x1);
								break;

							case 'b':
								XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												0x300, 0x0);
								break;
#endif

							}//end of switch

						}//end of if(CommandKey!=0)


//	        		}//end of 	if (!pauseRxMenu)

				if(MainMenu == 1){
						//break out of the rx menu
						break;
					}
			}//end of while(1)
			break;

	        default :
	           app_help();
	        break;
		}
	}

	Xil_DCacheInvalidate ();
	Xil_DCacheDisable ();
	/* Clean up ICache */
	Xil_ICacheInvalidate ();
	Xil_ICacheDisable ();

   return 0;
}

u8 XUartLite_RecvByte_local(u32 BaseAddress)
{
	do
	{
		if(mst_hpd_event==1)
		{
			xil_printf("mst_hpd_event is 0x%x\r\n",mst_hpd_event);
			mst_hpd_event = 2;
			return (u8)0;
		}
	} while(XUartLite_IsReceiveEmpty(BaseAddress));
	return (u8)XUartLite_ReadReg(BaseAddress, XUL_RX_FIFO_OFFSET);
}

char inbyte_local(void)
{
	 return XUartLite_RecvByte_local(STDIN_BASEADDRESS);
}

int init_peripherals()
{
	u32 Status;
	// Initialize UART
	Status = XUartLite_Initialize(&UartLite,
			XPAR_PROCESSOR_SUBSYSTEM_INTERCONNECT_AXI_UARTLITE_1_DEVICE_ID);
	if (Status!=XST_SUCCESS){
	xil_printf("ERR:UART failed to initialize. \r\n");
	return XST_FAILURE;
	}

	// Initialize timer.
	Status = XTmrCtr_Initialize(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);
	if (Status != XST_SUCCESS){
	xil_printf("ERR:Timer failed to initialize. \r\n");
	return XST_FAILURE;
	}
	// Set up timer options.
	XTmrCtr_SetResetValue(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);

	// Initialize Video PHY Controller

	XVphy_Config *CfgPtr = XVphy_LookupConfig(
			XPAR_VID_PHY_CONTROLLER_0_DEVICE_ID);

	XVphy_DpInitialize(&VPhy_Instance, CfgPtr, 0,
            PHY_User_Config_Table[6].CPLLRefClkSrc,
            PHY_User_Config_Table[6].QPLLRefClkSrc,
            PHY_User_Config_Table[6].TxPLL,
            PHY_User_Config_Table[6].RxPLL,
            PHY_User_Config_Table[6].LineRate);

	Two_byte_set (&VPhy_Instance, SET_TX_TO_2BYTE, SET_RX_TO_2BYTE);
    prog_bb (PHY_User_Config_Table[6].LineRate, 0x1);
	XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
						XVPHY_DIR_RX,(FALSE));
	XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
						XVPHY_DIR_TX,(FALSE));
	PHY_Configuration_Tx(&VPhy_Instance, PHY_User_Config_Table[6]);

    XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */

    /*
     * Initialize the IIC driver so that it is ready to use.
     */
    ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
    if (ConfigPtr_IIC == NULL) {
            return XST_FAILURE;
    }

    Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
								ConfigPtr_IIC->BaseAddress);
    if (Status != XST_SUCCESS) {
            return XST_FAILURE;
    }

    int i = 0;
	dma_struct[i].Config = XAxiVdma_LookupConfig(i);
	if (!dma_struct[i].Config)
	{
		 xil_printf("No video DMA found for ID %d\n\r", i);
				 return 1;
	} else {
		dma_struct[i].AXIVDMA_DEVICE_ID = i;
		/* Read Base Address */
		dma_struct[i].RD_ADDR_BASE = DDR_MEMORY + i*FRAME_LENGTH;
		/* Write Base Address */
		dma_struct[i].WR_ADDR_BASE = DDR_MEMORY+ i*FRAME_LENGTH ;
		dma_struct[i].BlockStartOffset = 0; //SUBFRAME_START_OFFSET;
	}
	/* Initialize DMA engine */
	Status = XAxiVdma_CfgInitialize(&dma_struct[i].AxiVdma,
					dma_struct[i].Config, dma_struct[i].Config->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		 xil_printf("VDMA Initialization failed %d\n\r", Status);
				 return 1;
	}

    return XST_SUCCESS;
}



int DPTxInitialize()
{
	u32 Status;
	DPTxSSConfig = XDpTxSs_LookupConfig(XDPTXSS_DEVICE_ID);
	if (DPTxSSConfig == NULL) {
		xil_printf("ERR: DPTX SS core not found!\n\r");
		return (XST_FAILURE);
	}

	Status = XDpTxSs_CfgInitialize(&DpTxSsInst, DPTxSSConfig,
										DPTxSSConfig->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DPTX SS initialize failed with Status = %d!\n\r",
				Status);
		return (XST_FAILURE);
	}

	/* Set custom timer wait */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpPt_CustomWaitUs, &TmrCtr);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_EVENT),
								&DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_PULSE),
								&DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
								&DpPt_LinkrateChgHandler, &DpTxSsInst);

	return (XST_SUCCESS);
}

int DPRxInitialize()
{
	u32 Status;

	// Lookup and Initialize DP Rx Subsystem

	DPRxSSConfig = XDpRxSs_LookupConfig(XDPRXSS_DEVICE_ID);
	if (DPRxSSConfig == NULL) {
		xil_printf("ERR: DPRX SS core not found!\n\r");
		return (XST_FAILURE);
	}
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, DPRxSSConfig,
										DPRxSSConfig->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DPRX SS initialize failed!\n\r");
		return (XST_FAILURE);
	}

	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &DpPt_CustomWaitUs, &TmrCtr);

	/* Setup callbacks */
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_PLL_RESET_EVENT,
							&Dprx_InterruptHandlerPllReset, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_LINKBW_EVENT,
							&Dprx_InterruptHandlerLinkBW, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
							&Dprx_InterruptHandlerTrainingDone, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_BW_CHG_EVENT,
							&Dprx_InterruptHandlerBwChange, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
							&Dprx_InterruptHandlerInfoPkt, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
							&Dprx_InterruptHandlerExtPkt, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_UNPLUG_EVENT,
							&Dprx_InterruptHandlerUplug, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,
							&Dprx_InterruptHandlerPwr, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VM_CHG_EVENT,
							&Dprx_InterruptHandlerVmChange, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_EVENT,
							&Dprx_InterruptHandlerNoVideo, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_EVENT,
							&Dprx_InterruptHandlerVBlank, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TLOST_EVENT,
							&Dprx_InterruptHandlerTrainingLost, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_EVENT,
							&Dprx_InterruptHandlerVideo, &DpRxSsInst);

#if (XPAR_XHDCP_NUM_INSTANCES > 0 && ENABLE_HDCP_IN_DESIGN == 1 )
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_HDCP_AUTHENTICATED,
							&Dprx_HdcpAuthCallback, &DpRxSsInst);
#endif

	return (XST_SUCCESS);
}

// This process takes in all the MSA values and find out resolution, BPC,
// refresh rate. Further this sets the pixel_width based on the pixel_clock
// and lane set this is to ensure that it matches the values in TX driver.
// Else video cannot be passthrough.
// Approximation is implemented for refresh rates. Sometimes a refresh rate of
// 60 is detected as 59 and vice-versa. Approximation is done for single digit.

static void Dprx_DetectResolution(void *InstancePtr)
{
	u32 DpHres = 0;
	u32 DpVres = 0;
	u32 GetResCount = 0;
	int i =0;
	do {
		DpHres = (XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
					XDP_RX_MSA_HRES));
		DpVres = (XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
					XDP_RX_MSA_VHEIGHT));
		GetResCount++;
	} while ( (((DpHres == 0) || (DpVres == 0)) || (GetResCount < 10000))
														&& training_done == 1);
	dp_msa_hres = DpHres;
	dp_msa_vres = DpVres;

	Msa[0].Vtm.Timing.HActive = dp_msa_hres;
	Msa[0].Vtm.Timing.VActive = dp_msa_vres;

	GetResCount = 0;
	DpHres_total = (XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
						XDP_RX_MSA_HTOTAL));
	DpVres_total = (XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
						XDP_RX_MSA_VTOTAL));
	while ( ((DpHres_total ==0 || DpVres_total == 0) || GetResCount < 10000)
														&& training_done == 1) {
		DpHres_total = (XDp_ReadReg(
			DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr, XDP_RX_MSA_HTOTAL));
		DpVres_total = (
			XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
						XDP_RX_MSA_VTOTAL));
		GetResCount++;
	}
	XDp_RxSetLineReset(DpRxSsInst.DpPtr, 1);
	Msa[0].Vtm.Timing.HTotal = DpHres_total;
	Msa[0].Vtm.Timing.F0PVTotal = DpVres_total;
	GetResCount = 0;
	rxMsaMVid = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_MVID)
								& 0x00FFFFFF);
	rxMsaNVid = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_NVID)
								& 0x00FFFFFF);
	while ( ((rxMsaMVid ==0 || rxMsaNVid == 0) || GetResCount < 10000)
														&& training_done == 1) {
		rxMsaMVid = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_MSA_MVID) & 0x00FFFFFF);
		rxMsaNVid = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_MSA_NVID) & 0x00FFFFFF);
		GetResCount++;
	}
	Msa[0].MVid = rxMsaMVid;
	Msa[0].NVid = rxMsaNVid;

	Msa[0].HStart = XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
									XDP_RX_MSA_HSTART);
	Msa[0].VStart = XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
									XDP_RX_MSA_VSTART);

	Msa[0].Vtm.Timing.HSyncWidth = XDp_ReadReg(
			DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr, XDP_RX_MSA_HSWIDTH);
	Msa[0].Vtm.Timing.F0PVSyncWidth = XDp_ReadReg(
			DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr, XDP_RX_MSA_VSWIDTH);

	Msa[0].Vtm.Timing.HSyncPolarity = XDp_ReadReg(
			DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr, XDP_RX_MSA_HSPOL);
	Msa[0].Vtm.Timing.VSyncPolarity = XDp_ReadReg(
			DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr, XDP_RX_MSA_VSPOL);

	recv_clk_freq = ((LineRate * 27.0)*rxMsaMVid)/rxMsaNVid;
	recv_frame_clk =
			ceil_func((recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total));
	recv_frame_clk_int = recv_frame_clk;
	//Doing Approximation here
	if (recv_frame_clk_int == 59 || recv_frame_clk_int == 61) {
		recv_frame_clk_int = 60;
	} else if (recv_frame_clk_int == 29 || recv_frame_clk_int == 31) {
		recv_frame_clk_int = 30;
	} else if (recv_frame_clk_int == 76 || recv_frame_clk_int == 74) {
		recv_frame_clk_int = 75;
	}

	recv_frame_clk_int_track = recv_frame_clk_int;
	recv_clk_see = ((LineRate * 27.0)*rxMsaMVid*1000)/rxMsaNVid;

	if((recv_clk_freq*1000000)>300000000 && LaneCount==4){
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x04);
		pixel = 0x4;
		Msa[0].UserPixelWidth = 0x4;
	}
	else if((recv_clk_freq*1000000)>75000000 && LaneCount!=1){
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x02);
		pixel = 0x2;
		Msa[0].UserPixelWidth = 0x2;
	}
	else{
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x01);
		pixel = 0x1;
		Msa[0].UserPixelWidth = 0x1;
	}

	rxMsamisc0 = ((XDp_ReadReg(
		DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_MISC0) >> 5) & 0x00000007);

	while (i < 100000 && training_done == 1) {
		rxMsamisc0 = ((XDp_ReadReg(
				DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_MSA_MISC0) >> 5) & 0x00000007);
		i++;
	}
	comp = ((XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
										XDP_RX_MSA_MISC0) >> 1) & 0x00000003);
	Msa[0].SynchronousClockMode = rxMsamisc0 & 1;
	bpc = Bpc[rxMsamisc0];
	Msa[0].BitsPerColor = bpc;
	Msa[0].Misc0 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_MSA_MISC0); //rxMsamisc0;
	Msa[0].Misc1 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_MSA_MISC1);
	if (training_done == 1) {
		xdbg_printf("*** Detected resolution: %d x %d @ %dHz, BPC = %d, Color "
					"= %d***\n\r", DpHres, DpVres,recv_frame_clk_int,bpc,comp);
	}
}
static void Dprx_ResetVideoOutput(void *InstancePtr)
{
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
//	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 800000);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);

}

static void Dprx_CheckSetupTx(void *InstancePtr)
{
	XVidC_VideoMode VmId;
	u8 tx_with_msa = 0;
    u32 Status;

	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive = dp_msa_hres;
	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive = dp_msa_vres;

	// Get the Video Mode Id depending on the frame rate
	VmId = XVidC_GetVideoModeId(
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive,
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive,
			recv_frame_clk_int,0); //XVIDC_FR_60HZ,0);

#if BYPASS_VID_COMMON
	xil_printf("Using the RX MSA values to generate TX video timings\n\r");
	tx_with_msa = 1;

#else
	if ((XVIDC_VM_NOT_SUPPORTED == VmId)) {
		xdbg_printf("This resolution is not supported in Video Library..."
					"using MSA values\n\r");
		tx_with_msa = 1;
	} else {
		xdbg_printf("This resolution is supported in Video Library.\r\n");
		tx_with_msa = 0;
	}
#endif

	//Configure GT channel when running on independent clock
	if (is_TX_CPLL == 0) {
		xdbg_printf("TX is running on QPLL........\n\r");
	switch(LineRate)
	{
		case 0x6:
	//lmk is programmed when 'r' is selected
			prog_bb (0x6, 1);
			Status = PHY_Configuration_Tx(&VPhy_Instance,
							PHY_User_Config_Table[0]);//, 0, is_TX_CPLL);
			break;
		default:
			xdbg_printf ("******** Invalid combination ********\r\n");
			break;
// these are illegal for KC705
//    		case 0xA:
//    			PHY_Configuration_Tx(&VPhy_Instance, PHY_User_Config_Table[5],
//    			0, is_TX_CPLL);
//    			break;
//
//    		case 0x14:
//    			PHY_Configuration_Tx(&VPhy_Instance, PHY_User_Config_Table[6],
//    			0, is_TX_CPLL);
//    			break;
	}
	}else { // TX running with CPLL
		xdbg_printf("TX is running on CPLL........\n\r");
	prog_bb (LineRate, 0);
	Status = XVphy_ClkInitialize(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
									XVPHY_DIR_TX);
		XVphy_ResetGtTxRx(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
										XVPHY_DIR_TX, FALSE);
		Status |= XVphy_WaitForPllLock(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA);
		Status |= XVphy_WaitForResetDone(&VPhy_Instance, 0,
									XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX);
	}
	if (Status != XST_SUCCESS) {
		xdbg_printf ("+++++++ TX GT configuration encountered a failure "
					"(TX PT) +++++++\r\n");
	}
#if (ENABLE_HDCP_IN_DESIGN && SET_TX_TO_2BYTE==1)
    Xil_Out32 (CLK_2_GPIO_BASEADDR+0x8, 0x1);
    ComputeMandD_txlnk ((LineRate*270*1000/40), LineRate);
#endif
	Dprx_SetupTx(DpTxSsInst.DpPtr, tx_with_msa, VmId);
}

void Dprx_SetupTx(void *InstancePtr, u8 tx_with_msa, XVidC_VideoMode VmId)
{
	u32 Status;
    u8 pwr_dwn;

	// This tx_preset() function will increase stability in some special
    //  cases with some monitors. It will setup Tx once with 1.62G x1
    //  setting, to make some monitor happy to bring up.
	tx_preset();

            //Disabling TX and TX interrupts
	pwr_dwn = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400);
	pwr_dwn = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
	//Disabling TX and TX interrupts
//	XDpTxSs_Reset(&DpTxSsInst);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 100000);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

	xil_printf("Starting DP Tx .");
	//Following delay is a MUST as some monitors go into sleep or power down
	//  when line rate is switched
	//  Adding delay allows the monitor to recover from this stage
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);
	Status = XDpTxSs_SetLinkRate(&DpTxSsInst, LineRate);
	if (Status != XST_SUCCESS) {
		xil_printf ("TX SetLink failure\r\n");
	}
	xil_printf(".");
	Status = XDpTxSs_SetLaneCount(&DpTxSsInst, LaneCount);
	if (Status != XST_SUCCESS) {
				xil_printf ("TX SetLane failure\r\n");
	}
	xil_printf(".");
	if (tx_with_msa == 0) {
		Status = XDpTxSs_SetBpc(&DpTxSsInst,Bpc[rxMsamisc0]);
		if (Status != XST_SUCCESS) {
				xil_printf ("TX SetBPC failure\r\n");
		}
		xil_printf(".");
	}
	Status = XDpTxSs_GetRxCapabilities(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
				xil_printf ("TX Could not get Rx Capabilities\r\n");
	}
	xil_printf(".");
	if (tx_with_msa == 0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, VmId);
		if (Status != XST_SUCCESS) {
				xil_printf ("TX Set Vid Mode failure\r\n");
		}
		xil_printf(".");
	}
	XDpTxSs_SetHasRedriverInPath(&DpTxSsInst, 0);
	xil_printf(".");
	if (tx_with_msa == 0) {
		Status = XDpTxSs_Start(&DpTxSsInst);
	} else {
		Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
	}
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 0);
	clk_wiz_locked();
	xil_printf ("\r\n");
// VDMA may not work for odd resolutions which are not divisible by 4
//	vdma_start();
//	xil_printf ("done !\r\n");
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[1]);
	// Need to start again as VTC values are reset
//	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);
	if (tx_with_msa == 0) {
		Status = XDpTxSs_Start(&DpTxSsInst);
	} else {
		Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
	}
	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
//		vdma_stop();
//		xil_printf ("An attempt to train TX failed... re-training\r\n");
//		xil_getc(0);
		pwr_dwn = 0x2;
		XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
		DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 40000);
		pwr_dwn = 0x1;
		XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);

		DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 2000000);
		Status = XDpTxSs_SetLinkRate(&DpTxSsInst, LineRate);
		if (Status != XST_SUCCESS) {
			xil_printf ("TX SetLink failure\r\n");
		}
//		xil_printf(".");
		Status = XDpTxSs_SetLaneCount(&DpTxSsInst, LaneCount);
		if (Status != XST_SUCCESS) {
					xil_printf ("TX SetLane failure\r\n");
		}
//		xil_printf(".");
		if (tx_with_msa == 0) {
			Status = XDpTxSs_SetBpc(&DpTxSsInst,Bpc[rxMsamisc0]);
			if (Status != XST_SUCCESS) {
					xil_printf ("TX SetBPC failure\r\n");
			}
//			xil_printf(".");
		}
		Status = XDpTxSs_GetRxCapabilities(&DpTxSsInst);
		if (Status != XST_SUCCESS) {
					xil_printf ("TX Could not get Rx Capabilities\r\n");
		}
//		xil_printf(".");
		if (tx_with_msa == 0) {
			Status = XDpTxSs_SetVidMode(&DpTxSsInst, VmId);
			if (Status != XST_SUCCESS) {
					xil_printf ("TX Set Vid Mode failure\r\n");
			}
			xil_printf(".");
		}
		XDpTxSs_SetHasRedriverInPath(&DpTxSsInst, 0);
		xil_printf(".");
		if (tx_with_msa == 0) {
			Status = XDpTxSs_Start(&DpTxSsInst);
		} else {
			Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
		}
		Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
		if (Status != XST_SUCCESS) {
			xil_printf ("TX training failed even after 2 attempts\r\n");
		}
//					xil_printf("> >>>>>> STAGE 3.1 >>>>>>\n\r");
	}

	//clear interrupt before enabling again
	//XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
	//Enabling TX interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0x0);
	rxMsaMVid_track = rxMsaMVid;
	rxMsaNVid_track = rxMsaNVid;
	if ((Status == XST_SUCCESS)) {
		vdma_start();
		Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[tx_pat_source]);
		xil_printf ("done !\r\n");
	}

}


static void Dprx_InterruptHandlerVmChange(void *InstancePtr)
{
//	u32 Status;

if (vblank_count >= 200 && training_done == 1) {
	xdbg_printf("*** Interrupt > Video Mode change ***\n\r");
	//Disabling TX interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
	XDpTxSs_Stop(&DpTxSsInst);
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[1]);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	if (internal_rx_tx == 0) {
#if !JUST_RX
	prog_tx = 1;
#endif
	start_tracking = 0;
	change_detected = 0;
	} else {
		Dprx_DetectResolution(DpRxSsInst.DpPtr);
		Dprx_ResetVideoOutput(DpRxSsInst.DpPtr);
		vdma_start();
	}
}

}

static void Dprx_InterruptHandlerNoVideo(void *InstancePtr) {
//	xil_printf("*** No Video detected ***\n\r");
	IsResChange = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_VIDEO_UNSUPPORTED , 1);
}

static void Dprx_InterruptHandlerVBlank(void *InstancePtr) {
//    u32 Status;
	if (DpRxSsInst.VBlankEnable == 1) {
		vblank_count++;
		if (vblank_count >= 200) {
//			xil_printf ("200 vblanks\r\n");
			DpRxSsInst.VBlankEnable = 0;
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
									XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
			vblank_done = 1;
			if (internal_rx_tx == 0) {
#if !JUST_RX
			prog_tx = 1;
			rx_ran_once = 1;
#endif
			} else {
				Dprx_DetectResolution(DpRxSsInst.DpPtr);
				Dprx_ResetVideoOutput(DpRxSsInst.DpPtr);
				vdma_start();
			}
			//when Vblank is received, HDCP is put in enabled state and the
			// timer is started. TX is not setup until the timer is done.
			// This ensures that certain sources like MacBook gets
			//  time to Authenticate.
			#if ENABLE_HDCP_IN_DESIGN
				if (internal_rx_tx == 0)
				{
					XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x01F80000);
					XDpRxSs_StartTimer(&DpRxSsInst);
				}
			#endif

		} //end of (vblank_count >= 100)
		else if (vblank_count == 80)
		{

#if ENABLE_HDCP_IN_DESIGN
			XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x01F80000);
			XDpRxSs_SetLane(&DpRxSsInst, LaneCount);
		    XDpRxSs_SetPhysicalState(&DpRxSsInst, hdcp_capable_org); //TRUE);
		    XHdcp1xExample_Poll();
		    XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
		    XHdcp1xExample_Poll();
			XHdcp1xExample_Enable();
			XHdcp1xExample_Poll();
#endif
		} else if (vblank_count == 20) {
			XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x80000000);
		}

	}

}
static void Dprx_InterruptHandlerVideo(void *InstancePtr) {
//	xil_printf("*** Interrupt > Video detected ***\n\r");
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_VIDEO_UNSUPPORTED
					, 0);


}//End of Dprx_InterruptHandlerVideo()


//this is a handler in TP1
void Dprx_InterruptHandlerLinkBW(void *InstancePtr)
{
	u32 Status;
	PLLRefClkSel (&VPhy_Instance, DpRxSsInst.UsrOpt.LinkRate);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
	only_tx_active = 0;
	gt_stable = 0;
	XDpTxSs_Stop(&DpTxSsInst);
// For KC705 only possible option to run Tx with QPLL and RX with CPLL is when
	//linkrate = 0x6 and is_tx_cpll == 0
// For all other linerates it is CPLL for RX and TX
	if ((DpRxSsInst.UsrOpt.LinkRate == 0x6) && (is_TX_CPLL == 0)) {
		XVphy_PllInitialize(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
					ONBOARD_REF_CLK, DP159_FORWARDED_CLK,
					XVPHY_PLL_TYPE_QPLL, XVPHY_PLL_TYPE_CPLL);
	} else {
		XVphy_PllInitialize(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
					DP159_FORWARDED_CLK, DP159_FORWARDED_CLK,
					XVPHY_PLL_TYPE_CPLL, XVPHY_PLL_TYPE_CPLL);
	}
	Status = XVphy_ClkInitialize(&VPhy_Instance, 0,
					XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);

	if (Status != XST_SUCCESS) {
		xdbg_printf ("+++++++ RX GT configuration encountered an error (TP1)"
				 "+++++++\r\n");
     }
//     Xil_DCacheFlush();
     initial_value = DpRxSsInst.UsrOpt.LinkRate;

}

void Dprx_InterruptHandlerPllReset(void *InstancePtr)
{
	u32 Status1;
	u32 Status2;

	//CPLLPD disabled, enable CPLLs
	XVphy_PowerDownGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA, (FALSE));

#if !COMPLIANCE
if (internal_rx_tx == 0) {
	XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,
						(FALSE));
	if (is_TX_CPLL == 1) {
	XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,
						(FALSE));
//	XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CMN, XVPHY_DIR_TX,
//						(FALSE));
	}
}
#endif

#if COMPLIANCE
    XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,
					(FALSE));
    XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CMN, XVPHY_DIR_TX,
					(FALSE));
#endif

#if ENABLE_HDCP_IN_DESIGN
        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x01FFFFFF);
#else
        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x0007FFFF);
#endif

        Status2 = XVphy_WaitForResetDone(&VPhy_Instance, 0,
								XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);
        Status1 = XVphy_WaitForPllLock(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA);

        Status1 += Status2; // only to surpress warnings
        xdbg_printf ("%d %d\r\n", Status1, Status2);

#if (ENABLE_HDCP_IN_DESIGN && SET_RX_TO_2BYTE==1)
//        Xil_Out32 (CLK_2_GPIO_BASEADDR, 0x0);
        Xil_Out32 (CLK_2_GPIO_BASEADDR, 0x1);
        ComputeMandD_rxlnk ((DpRxSsInst.UsrOpt.LinkRate*270*1000/40),
							DpRxSsInst.UsrOpt.LinkRate);
#endif
}

void Dprx_InterruptHandlerTrainingDone(void *InstancePtr)
{
    LaneCount = XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
							XDP_RX_DPCD_LANE_COUNT_SET);
    LineRate = DpRxSsInst.UsrOpt.LinkRate;

    training_done = 1;

#if !COMPLIANCE
    training_done_lane01 = XDp_ReadReg(
		DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
			XDP_RX_DPCD_LANE01_STATUS);
    training_done_lane23 = XDp_ReadReg(
		DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
			XDP_RX_DPCD_LANE23_STATUS);
    xdbg_printf("> Interrupt: Training done !!!(BW: 0x%x, Lanes: 0x%x, Status: "
		"0x%x;0x%x).\n\r", LineRate, LaneCount,
			training_done_lane01, training_done_lane23);
#if ENABLE_HDCP_IN_DESIGN
    XDpRxSs_SetLane(&DpRxSsInst, LaneCount);
    XDpRxSs_SetPhysicalState(&DpRxSsInst, hdcp_capable_org); //TRUE);
    XHdcp1xExample_Poll();
#endif
    training_done = 1;
    switch_to_tx = 0;
    switch_to_patgen = 0;
    only_tx_active = 0;
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
#endif

	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_SET_MSA),
							&DpPt_TxSetMsaValuesImmediate, &DpTxSsInst);
}

static void Dprx_InterruptHandlerTrainingLost(void *InstancePtr) {


#if !COMPLIANCE
	training_done = 0;
	vblank_done = 0;
	vblank_count =0;
	training_done_lane01 = 0;
	training_done_lane23 = 0;
	pixel = 0;
	prog_tx = 0;
	start_tracking = 0;
	switch_to_tx = 0;
	change_detected = 0;
	IsRxTrained = 0;

	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
	//Disabling TX interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
	XDpTxSs_Stop(&DpTxSsInst);
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[1]);
	vdma_stop();
	xdbg_printf("> Interrupt: Training lost !\n\r");
#if ENABLE_HDCP_IN_DESIGN
	XDpRxSs_SetPhysicalState(&DpRxSsInst, FALSE);
	XDpRxSs_StopTimer(&DpRxSsInst);
	IsTxEncrypted = 0;
	IsTxAuthenticated = 0;

	// This function will over write timer function pointer to be the right one.
	Dprx_HdcpUnAuthCallback((void *)&DpRxSsInst); 	// Added from 16.4 release

#endif

	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x80000000);

#endif

	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
}


void Dprx_InterruptHandlerBwChange(void *InstancePtr)
{
//	xil_printf("> Interrupt: Bandwidth Change !\n\r");
}



void Dprx_InterruptHandlerUplug(void *InstancePtr)
{

    //power down GT PLL immediately.  Hold PLLs in power down
    XVphy_PowerDownGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
					(TRUE));

    //Reset GTs.  Hold reset
    XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,
					(TRUE));


#if ENABLE_HDCP_IN_DESIGN
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFE00FFFF);
#else
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF87FFF);
#endif


#if !COMPLIANCE
	xdbg_printf("> Interrupt: Cable unplugged !\n\r");
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
	XDpTxSs_Stop(&DpTxSsInst);
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[1]);
	vdma_stop();
	training_done = 0;
	vblank_done = 0;
	vblank_count =0;
	prog_tx = 0;
	training_done_lane01 = 0;
	training_done_lane23 = 0;
	//Disabling TX and TX interrupts
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
	enabled = 0;
	wait_count = 0;
	IsRxTrained = 0;
	start_tracking = 0;
	change_detected = 0;
	gt_stable = 1;
	if (is_TX_CPLL == 1) {
	XVphy_PllInitialize(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
			ONBOARD_REF_CLK, ONBOARD_REF_CLK,
			XVPHY_PLL_TYPE_CPLL, XVPHY_PLL_TYPE_CPLL);
	}

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
	IsTxEncrypted = 0;
	IsTxAuthenticated = 0;
#endif


    // when cable is unplugged, the system should switch to TX only mode
#if FOR_INTERNAL
	if (rx_link_change_requested == 0 && rx_ran_once == 1
			&& need_to_retrain_rx == 0) { // && hpd_issued == 0) {
		xdbg_printf(">>> !!!!!!!!! RX cable unplugged. RX Video & REFCLK1 is "
				"lost !!!!!!!!!\n\r");
	if (is_TX_CPLL == 0) {
		xdbg_printf(">>> !!!!!!!!! Displaying the default 800x600 color bar "
				"pattern  !!!!!!!!!\n\r");
		switch_to_patgen = 1;
	} else {
		xdbg_printf(">>> !!!!!!!!! Switching over the CPLL to REFCLK0  "
				"!!!!!!!!!\n\r");
		xdbg_printf(">>> !!!!!!!!! Displaying the default 800x600 color bar "
				"pattern  !!!!!!!!!\n\r");
	switch_to_patgen = 1;
//	switch_to_tx = 1;
//	rx_ran_once = 0;
//	switch_to_patgen = 0;
	}

//power up GT PLL
    XVphy_PowerDownGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
    (FALSE));

// un-Reset GTs.  release-reset
    XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,
    (FALSE));

	} else {
	switch_to_tx = 0;
	rx_link_change_requested = 0;
	}
	need_to_retrain_rx = 0;
	tx_pat_source = 0;

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	rx_ran_once = 0;
#endif

#endif

}

void Dprx_InterruptHandlerPwr(void *InstancePtr)
{
#if !COMPLIANCE
	u32 rdata;
	rdata = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_DPCD_SET_POWER_STATE);
	if(rdata == 0x2)
	{
//			XVidC_Dp159Initialize(DpRxSsInst.IicPtr);
		XDpRxSs_Dp159Config(DpRxSsInst.IicPtr, XDPRXSS_DP159_CT_UNPLUG,
				DpRxSsInst.UsrOpt.LinkRate, DpRxSsInst.UsrOpt.LaneCount);
	}
#endif
}

void Dprx_InterruptHandlerInfoPkt(void *InstancePtr)
{

}
void Dprx_InterruptHandlerExtPkt(void *InstancePtr){

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

	xdbg_printf("\033[33m * \033[0m \r\n");
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

int DPPtIntrInitialize()
{
	u32 Status;

	/* Setup interrupt handling */
	Status = DpPt_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:Interrupt set up failed!\n\r");
		return XST_FAILURE;
	}
	return Status;
}

void xildpWaitmS(u32 ms)
{
    XTmrCtr_Start(&TmrCtr, 0);
    while( XTmrCtr_GetValue(&TmrCtr, 0) <
			( ms * ( XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000000 ) ) )
	;
    XTmrCtr_Stop(&TmrCtr, 0);
}

u32 StreamOffsetAddr[4] = {0, XILINX_DISPLAYPORT_VID2_BASE_ADDRESS_OFFSET,
			  XILINX_DISPLAYPORT_VID3_BASE_ADDRESS_OFFSET,
			  XILINX_DISPLAYPORT_VID4_BASE_ADDRESS_OFFSET};
/*
 * This function is a call back to write the MSA values to Tx as they are
 * read from the Rx, instead of reading them from the Video common library
 */
void DpPt_TxSetMsaValuesImmediate(void *InstancePtr)
{

	/* Set the main stream attributes to the associated DisplayPort TX core
	 * registers. */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HTOTAL +
			StreamOffsetAddr[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HTOTAL));

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VTOTAL +
			StreamOffsetAddr[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VTOTAL));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_POLARITY
			+ StreamOffsetAddr[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSPOL)|
			(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSPOL)
				<<	XDP_TX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HSWIDTH+
			StreamOffsetAddr[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HSWIDTH));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VSWIDTH+
			StreamOffsetAddr[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VSWIDTH));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HRES +
			StreamOffsetAddr[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HRES));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VRES +
			StreamOffsetAddr[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VHEIGHT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HSTART +
			StreamOffsetAddr[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HSTART));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VSTART +
			StreamOffsetAddr[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VSTART));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC0 +
			StreamOffsetAddr[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC0));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC1 +
			StreamOffsetAddr[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC1));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_USER_PIXEL_WIDTH +
			StreamOffsetAddr[0], pixel);
     /* Check for YUV422, BPP has to be set using component value to 2 */
     if( ( (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_MISC0))
						 & 0x6 ) == 0x2  ) {
//           xil_printf("%s:: YUV422 \r\n", __func__);
           DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x1;
           Msa[0].ComponentFormat = 0x1;
     }
     else {
//           xil_printf("%s:: RGB or YUV444 \r\n", __func__);
           DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;
           Msa[0].ComponentFormat = 0x0;
     }


}

int DpPt_SetupIntrSystem()
{
	int Status;
	XIntc *IntcInstPtr = &IntcInst;

	/* Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstPtr, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\n\r");
		return XST_FAILURE;
	}

	/* Hook up interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
			(XInterruptHandler)XDpTxSs_DpIntrHandler, &DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Hook up Rx interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
			(XInterruptHandler)XDpRxSs_DpIntrHandler, &DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Hook up Rx interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_TIMER_0,
			(XInterruptHandler)XTmrCtr_InterruptHandler, &TmrCtr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Timer interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstPtr, XINTC_IIC_ID,
			(XInterruptHandler) XIic_InterruptHandler,  &IicInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: IIC interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	XIntc_Enable(IntcInstPtr, XINTC_IIC_ID);

#if ENABLE_HDCP_IN_DESIGN
	/* Hook up Rx interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_HDCP_TIMER_ID,
			(XInterruptHandler)XDpRxSs_TmrCtrIntrHandler,&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Timer interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	XIntc_Enable(&IntcInst, XINTC_HDCP_TIMER_ID);
#endif

	/* Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */

	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception
	 * table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XIntc_InterruptHandler, IntcInstPtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();
	return (XST_SUCCESS);
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
void hpd_con()
{
    u32 Status;
	u8 Edid[128];
	u8 Edid1[128];

	int i = 0;
	int j = 0;
	u8 max_cap_new;
	u8 max_cap_lanes_new;

//    DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 20000);
	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x1, 1, &max_cap_new);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2, 1, &max_cap_lanes_new);
	if (Status != XST_SUCCESS) {
		xil_printf ("\r\nCould not read sink capabilities\r\n");
	}

#if CAP_OVER_RIDE == 1
	max_cap_new = MAX_RATE;
	max_cap_lanes_new = MAX_LANE;
#endif

	//reading the first block of EDID
	Status |= XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid, 0);
	//reading the subsequent blocks of EDID
	Status |= XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1, 1);
	if (Status != XST_SUCCESS) {
		xil_printf ("\r\nCould not read sink EDID\r\n");
	}

	for (i=0; i<128; i++) {
		if (Edid_org[i] != Edid[i]) {
			need_to_retrain_rx = 1;
		}
		Edid_org[i] = Edid[i];
	}
	for (i=0; i<128; i++) {
		if (Edid1_org[i] != Edid1[i]) {
			need_to_retrain_rx = 1;
		}
		Edid1_org[i] = Edid1[i];
	}

#if ENABLE_HDCP_IN_DESIGN
	u8 auxValues[9];

    XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x068028, 1, auxValues);

    hdcp_capable = auxValues[0] & 0x1;
    hdcp_repeater = auxValues[0] & 0x2;
	if (hdcp_capable != hdcp_capable_org) {
		need_to_retrain_rx = 1;
		hdcp_capable_org = hdcp_capable;
	}
#endif

    if (need_to_retrain_rx == 0) {

#if ENABLE_HDCP_IN_DESIGN
		if (XDpTxSs_IsAuthenticated(&DpTxSsInst)==1)
		{
			XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
			XDpTxSs_HdcpDisable(&DpTxSsInst);
			XHdcp1xExample_Poll();
		}
#endif

	 if (only_tx_active == 0) {
			Dprx_CheckSetupTx(DpRxSsInst.DpPtr);
			IsRxTrained = 1;
	 } else {
			 XDpTxSs_SetLinkRate(&DpTxSsInst,
					 DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate);
			 xil_printf (".");
			 XDpTxSs_SetLaneCount(&DpTxSsInst,
					 DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount);
			 Status = XDpTxSs_Start(&DpTxSsInst);
			 if (Status != XST_SUCCESS) {
				 xil_printf("! Link re-training failed.\n\r");
			 }
	 }

#if ENABLE_HDCP_IN_DESIGN
	if(XDpTxSs_CheckLinkStatus(&DpTxSsInst) == XST_SUCCESS) {
		/* Check here if Tx is not authenticated already, and that is
		 * in the unauthenticated state, i.e it is ready
		 * to receive a authenticate command. (CurrentState
		 * 10 is unauthenticated. */
		if (XDpTxSs_IsAuthenticated(&DpTxSsInst) == 0
				&& DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 10)
		{
			/* TX is reconnected, we disabled encryption and the HDCP SM
			 * when TX was disconnected. Now we have already enabled and
			 * brought up TX, we should begin authenticating on the
			 * downstream TX. */
			XDpTxSs_Authenticate(&DpTxSsInst);
			XHdcp1xExample_Poll();
			XDpTxSs_EnableEncryption(&DpTxSsInst, 0x1);
			XHdcp1xExample_Poll();
		}
	}
#endif

    }


    if (need_to_retrain_rx == 1) {
#if USE_MONITOR_EDID
		UINT8 edid_monitor[384];
		switch (Edid[126]){
		case 0:
			for(i=0; i<128; i++)
				edid_monitor[i] = Edid[i];
			for(i=128; i<256; i++)
				edid_monitor[i] = 0;
			break;
		case 1:
			for(i=0; i<128; i++)
				edid_monitor[i] = Edid[i];
			for(i=0; i<128; i++)
				edid_monitor[i+128] = Edid1[i];
			break;
		}

		for(i=0;i<(256*4);i=i+(16*4)){
			for(j=i;j<(i+(16*4));j=j+4){
				XDp_WriteReg (XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
				j, edid_monitor[(i/4)+1]);
			}
		}
		for(i=0;i<(256*4);i=i+4){
			XDp_WriteReg (XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
			i, edid_monitor[i/4]);
		}

#endif



     if (only_tx_active == 0 && need_to_retrain_rx == 1) {
     xil_printf("The Monitor has been changed....\r\n");
#if USE_MONITOR_EDID
     xil_printf("The EDID contents in RX have been updated..\r\n");
#endif

#if ENABLE_HDCP_IN_DESIGN

#if 0 //ENABLE_HDCP_FLOW_GUIDE
			if (XDpTxSs_IsAuthenticated(&DpTxSsInst)==1)
			{
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
				XDpTxSs_HdcpDisable(&DpTxSsInst);
				XHdcp1xExample_Poll();
			}
#endif
	XDpRxSs_SetPhysicalState(&DpRxSsInst, FALSE);
	XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
	XHdcp1xExample_Poll();
	XDpTxSs_SetPhysicalState(&DpTxSsInst, FALSE);
	XHdcp1xExample_Poll();
	XDpRxSs_StopTimer(&DpRxSsInst);
	IsTxEncrypted = 0;
	IsTxAuthenticated = 0;
#endif

	 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0);
	 if ((max_cap_new != max_cap_org) || ((max_cap_lanes&0x1F)
									 != (max_cap_lanes_new&0x1F))) {
		initial_value = max_cap_new;
		max_cap_org = max_cap_new;
		max_cap_lanes = max_cap_lanes_new&0x1F;
		XDpRxSs_SetLinkRate(&DpRxSsInst, max_cap_org);
		XDpRxSs_SetLaneCount(&DpRxSsInst, max_cap_lanes);
		xil_printf("DP RX capability has been updated to: Linerate %x, "
							"LaneCount %x\r\n",max_cap_org,max_cap_lanes);
	 }
	    xil_printf("**** Issuing HPD *****\r\n");
	    start_tracking = 0;
	    //give hpd ??
	    IsRxTrained = 0;
	    Dprx_InterruptHandlerUplug(DpRxSsInst.DpPtr);
	    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_LINK_ENABLE, 0x1);
	    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_HPD_INTERRUPT,0x0BB80001);
		}else {
	 if (need_to_retrain_rx == 1) {
	     xil_printf("The Monitor has been changed. To avoid unpredictable "
				 "behavior, please change the Link/Lane and retrain\r\n");


#if ENABLE_HDCP_IN_DESIGN
				 Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
				 if (Status == XST_SUCCESS) {
//					 xil_printf(" DP TX :: Link still up. \r\n");
					if (XDpTxSs_IsAuthenticated(&DpTxSsInst) == 0){
						XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
						XHdcp1xExample_Poll();
						XDpTxSs_HdcpEnable(&DpTxSsInst);
						XHdcp1xExample_Poll();

						if (DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 10) {
							XDpTxSs_Authenticate(&DpTxSsInst);
							XHdcp1xExample_Poll();
							XDpTxSs_EnableEncryption(&DpTxSsInst, 0x1);
							XHdcp1xExample_Poll();
						}
					}
				 } else {
//					 	xil_printf(" DP TX :: LINK DOWN !!! \r\n");
						XDpTxSs_SetLinkRate(&DpTxSsInst,
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate);
						xil_printf (".");
						XDpTxSs_SetLaneCount(&DpTxSsInst,
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount);
						Status = XDpTxSs_Start(&DpTxSsInst);
						if (Status != XST_SUCCESS) {
							xil_printf("! Link re-training failed.\n\r");
						} else {
							if (DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 10) {
								XDpTxSs_Authenticate(&DpTxSsInst);
								XHdcp1xExample_Poll();
								XDpTxSs_EnableEncryption(&DpTxSsInst, 0x1);
								XHdcp1xExample_Poll();
							}
						}
				 }
#endif
	 }
		}
    }
}

void DpPt_HpdEventHandler(void *InstancePtr)
{
	u8 pwr_dwn;

	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		xdbg_printf("\r\n+===> HPD Connected.\n\r");
		pwr_dwn = 0x2;
		XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
		pwr_dwn = 0x1;
		XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
		tx_is_reconnected = 1;

// This part has added to give HDCP a proper handle when hdp even happens
// HDCP block will disable Tx side encryption when hpd detected
#if ENABLE_HDCP_IN_DESIGN
		XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
		XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
		XHdcp1xExample_Poll();
		XDpTxSs_HdcpEnable(&DpTxSsInst);
		XHdcp1xExample_Poll();
		if (training_done == 1) {
			IsRxTrained = 1;
		}
#endif

	}
	else
	{
		xdbg_printf("\r\n+===> HPD Disconnected.\n\r");
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
		audio_on = 0;
		//stop the VDMA only if in passthrough mode
		if (((only_tx_active == 0) && (training_done == 1))) {
			vdma_stop();
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x300, 0x0);
		}
		IsRxTrained = 0;
		tx_is_reconnected = 0;
		//on HPD d/c, it is important to bring down the HDCP
#if ENABLE_HDCP_IN_DESIGN
		if (XDpTxSs_IsAuthenticated(&DpTxSsInst)==1)
		{
			xdbg_printf(".~\r\n");
			XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
			XDpTxSs_HdcpDisable(&DpTxSsInst);
			XHdcp1xExample_Poll();
			XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
			XHdcp1xExample_Poll();
		}
#endif
	}
}

/*****************************************************************************/
/**
*
* This function is called when a Hot-Plug-Detect (HPD) pulse is received by
* the DisplayPort TX Subsystem core.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetCallback driver function to set this
*		function as the handler for HPD pulse.
*
******************************************************************************/

void hpd_pulse_con()
{
    u32 Status;
    u8 lane0_sts;
    u8 lane2_sts;
    u8 lane_set;
    u8 rd_204;
    u8 bw_set;

    Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x202, 1, &lane0_sts);
    Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x203, 1, &lane2_sts);
    Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x204, 1, &rd_204);
    Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x101, 1, &lane_set);
    Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x100, 1, &bw_set);
    if (Status != XST_SUCCESS) {
        xdbg_printf ("Failed to read AUX registers on HPD pulse\r\n");
    }
    bw_set = bw_set & 0x1F;
    lane_set = lane_set & 0x1F;
    rd_204 = rd_204 & 0x1;
    lane0_sts = lane0_sts & 0x55;
    lane2_sts = lane2_sts & 0x55;
#if 0
    xdbg_printf ("lanes set : %x\r\n",lane_set);
    xdbg_printf ("BW set : %x\r\n",bw_set);
     xdbg_printf ("lane0 : %x\r\n",lane0_sts);
     xdbg_printf ("lane2 : %x\r\n",lane2_sts);
#endif

#if ENABLE_HDCP_IN_DESIGN
     u8 dev_serv_intr_vec;
     u8 BStatus;
     /* Check for the CP_IRQ interrupt. Check the CP_IRQ
      * bit in the DEVICE_SERVICE_IRQ_VECTOR (0x0201) */
     Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x201, 1, &dev_serv_intr_vec);
     if(dev_serv_intr_vec & 0x04) {
	/* CP_IRQ is set, read the BStatus register */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x068029, 1, &BStatus);
//    	xil_printf(" HPD_Pulse: CP_IRQ, (BStatus : %x) \n", BStatus);

	/* Check if the Link Integrity Failure Bit is set */
	if (BStatus & 0x04) {
#if ENABLE_HDCP_FLOW_GUIDE
		xdbg_printf("\033[1m\033[41m\033[37m (*<*)TxLink! \033[0m \n");
#endif
			/* State 5 : Authenticated,
			 * State 6 : Link Integrity Check */
			if(DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 6 ||
					DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 5) {
				/* Disable Encryption */
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
				XHdcp1xExample_Poll();

				/* Re-start authentication (the expectation is
				 * that HDCP is already in the authenticated state). */
				xdbg_printf("\033[1m\033[43m\033[34m (*<*)Tx-> \033[0m \n");
				XDpTxSs_Authenticate(&DpTxSsInst);
				XHdcp1xExample_Poll();
				XDpTxSs_EnableEncryption(&DpTxSsInst, 0x1);
				XHdcp1xExample_Poll();
			}
	}

	/* Check if the READY bit is set. */
		if (BStatus & 0x01) {
#if ENABLE_HDCP_FLOW_GUIDE
			xdbg_printf("\033[1m\033[42m\033[37m (*<*)Ready! \033[0m \n");
#endif
			/* DP TX State 8 : Wait-for-Ready */
			if(DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 8) {
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
			xdbg_printf("\033[1m\033[42m\033[37m (*<*)Ro'_AVAILABLE!"
							"\033[0m \n");
#endif
			if ((BStatus & 0x01) != 0x01) {
				XHdcp1xExample_Poll();
			}
		}

		/* Check if CP_IRQ is spurious */
		if (BStatus == 0x00) {
#if ENABLE_HDCP_FLOW_GUIDE
			xdbg_printf("\033[1m\033[41m\033[37m (*<*)Spurious CP_IRQ!"
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
#endif

    //Check if CR, symbol and alignment is lost
    //re-train if required
//     xil_printf ("is rx trained %x\r\n",IsRxTrained);

	if (need_to_retrain_rx == 0) {// && IsRxTrained == 1) {
		if (lane_set == 0x4) {
	        if ((lane0_sts != 0x55) || (lane2_sts != 0x55) || (rd_204 != 1)) {
				XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
				XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
				if ((only_tx_active == 0) && (training_done == 1)) {
					Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
					xdbg_printf ("Retraining PT 4..\r\n");
				 } else if (only_tx_active == 1) {
					Status = XDpTxSs_Start(&DpTxSsInst);
					xdbg_printf ("Retraining 4..\r\n");
				 }

	        }
	     } else if (lane_set == 0x2) {
	        if ((lane0_sts != 0x55) || (rd_204 != 1)) {
				 XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
				 XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
				 if ((only_tx_active == 0) && (training_done == 1)) {
					Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
					xdbg_printf ("Retraining PT 2..\r\n");
				 } else if (only_tx_active == 1) {
					Status = XDpTxSs_Start(&DpTxSsInst);
					xdbg_printf ("Retraining 2..\r\n");
				 }
	        }

	     } else if (lane_set == 0x1) {
	        if ((lane0_sts != 0x5) || (rd_204 != 1)) {
				 XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
				 XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
				 if ((only_tx_active == 0) && (training_done == 1)) {
					Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
					xdbg_printf ("Retraining PT 1..\r\n");
				 } else if (only_tx_active == 1) {
					Status = XDpTxSs_Start(&DpTxSsInst);
					xdbg_printf ("Retraining 1..\r\n");
				 }
	        }
		} else {
			//something went wrong completely.. restart training
			if ((only_tx_active == 0) && (training_done == 1)) {
				Dprx_CheckSetupTx(&DpTxSsInst);
			}
                }
	}
	// clearing the interrupt if it is set
	// XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
	// enable the interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0x0);
}

void DpPt_HpdPulseHandler(void *InstancePtr)
{

	xdbg_printf("\r\nHPD Pulse event detected\n\r");
		// Some monitors give HPD pulse repeatedly which causes HPD pulse
		// function to be executed huge number of time.
		// hence hpd_pulse interrupt is disabled and then enabled when
		// hpd_pulse function is executed

		if ((only_tx_active == 1) || ((only_tx_active == 0)
										&& (training_done == 1))) {
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0x10);
			hpd_pulse_con();
        }
}


void DpPt_LinkrateChgHandler(void *InstancePtr)
{

// If TX is unable to train at what it has been asked then
// necessary down shift handling has to be done here
// eg. reconfigure GT to new rate etc
// This XAPP assumes that RX and TX would run at same rate

}

/*****************************************************************************/
/**
*
* This function is called when DisplayPort TX Subsystem core requires delay
* or sleep. It provides timer with predefined amount of loop iterations.
*
* @param	InstancePtr is a pointer to the XDp instance.
*
* @return	None.
*
* @note		Use the XDpTxSs_SetUserTimerHandler driver function to set
*		this function as the handler custom delay/sleep.
*
******************************************************************************/
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{

	u32 TimerVal;
	XDp *DpInstance = (XDp *)InstancePtr;
	u32 NumTicks = (MicroSeconds * (DpInstance->Config.SAxiClkHz /
			1000000));

	XTmrCtr_Reset(DpInstance->UserTimerPtr, 0);
	XTmrCtr_Start(DpInstance->UserTimerPtr, 0);

	/* Wait specified number of useconds. */
	do {
		TimerVal = XTmrCtr_GetValue(DpInstance->UserTimerPtr, 0);
	} while (TimerVal < NumTicks);
}


char GetInbyte(void)
{
	return XUartLite_RecvByte(UART_BASEADDR);

}


//sets the default drp values into the mmcm
//this ensures that mmcm is back to original state
void reconfig_clkwiz () {
	Xil_Out32 (CLK_WIZ_BASE+0x200, clk_reg0);
	Xil_Out32 (CLK_WIZ_BASE+0x204, clk_reg1);
	Xil_Out32 (CLK_WIZ_BASE+0x208, clk_reg2);
	Xil_Out32 (CLK_WIZ_BASE+0x25C, 0x7);
	DpPt_CustomWaitUs(DpRxSsInst.DpPtr, 200000);
	Xil_Out32 (CLK_WIZ_BASE+0x25C, 0x2);
}

void reset_clkwiz () {
	//reset the clk_wiz
	Xil_Out32 (XPAR_VID_CLK_RST_HIER_AXI_GPIO_0_BASEADDR+0x8, 0x0);
	// deassert the reset
	Xil_Out32 (XPAR_VID_CLK_RST_HIER_AXI_GPIO_0_BASEADDR+0x8, 0x1);
}

void clk_wiz_locked() {

	while ((Xil_In32 (XPAR_VID_CLK_RST_HIER_AXI_GPIO_0_BASEADDR+0x0)) == 0 ) {
		xil_printf ("~/~/");
	}
	xil_printf ("^^");
}


void start_tx(u8 line_rate, u8 lane_count,
				XVidC_VideoMode res_table,u8 bpc, u8 pat) {

	u32 Status;
    u8 pwr_dwn;
	//Disabling TX and TX interrupts

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
	pwr_dwn = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400);
	pwr_dwn = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);

    DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);

    XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
    DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 100000);
    XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

    xil_printf ("\r\nTraining TX with: Link rate %x, Lane count %d\r\n",
				line_rate,lane_count);

	XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);
    xil_printf (".");
    XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);
    xil_printf (".");
    if (res_table !=0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, res_table);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR: Setting resolution failed\n\r");
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
#if ENABLE_HDCP_IN_DESIGN
	XDpTxSs_HdcpDisable(&DpTxSsInst);
	XHdcp1xExample_Poll();
#endif
	Status = XDpTxSs_Start(&DpTxSsInst);
	xil_printf (".");
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
	xil_printf (".");
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
								C_VideoUserStreamPattern[pat_update]);
	xil_printf (".");
	clk_wiz_locked();
	Status = XDpTxSs_Start(&DpTxSsInst);
	xil_printf (".");
        Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = XDpTxSs_Start(&DpTxSsInst);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\n\r");
			return;
		}
	}

	// this packing_CLK change only be required for 12/16 bpc case.
	// This block should be moved to driver in next release.
	// Patched on 16.4 release
	if( 	XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_GT_DATAWIDTH == 4 &&
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].PixelClockHz != 0 &&
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].UserPixelWidth != 0 &&
			DpTxSsInst.UsrOpt.Bpc > 10){

		u32 packing_clk = (
				DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].PixelClockHz /
				DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].UserPixelWidth)/
				DpTxSsInst.UsrOpt.Bpc * 8;
		u8 need_link_clk = 0;
		u32 linkclk = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate){
			case 0x14:
				linkclk = XVPHY_DP_LINK_RATE_HZ_540GBPS /
						XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_GT_DATAWIDTH / 10;
				need_link_clk = (packing_clk < linkclk) ? 1 : 0;
				break;
			case 0x0A:
				linkclk = XVPHY_DP_LINK_RATE_HZ_270GBPS /
						XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_GT_DATAWIDTH / 10;
				need_link_clk = (packing_clk < linkclk) ? 1 : 0;
				break;
			default:
				linkclk = XVPHY_DP_LINK_RATE_HZ_162GBPS /
						XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_GT_DATAWIDTH / 10;
				need_link_clk = (packing_clk < linkclk) ? 1 : 0;
				break;
		}
		if(need_link_clk == 0) // writing VIDEO_PACKING_CLOCK_CONTROL bit
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x90, 0x0);
		else
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x90, 0x1);
	}

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0x0);
	xil_printf ("..done !\r\n");

#if ENABLE_HDCP_IN_DESIGN
	if(hdcp_capable_org == 1)	{
		XDpTxSs_SetLane(&DpTxSsInst,
			DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount);
		XDpTxSs_SetPhysicalState(&DpTxSsInst, !(hdcp_capable_org));
		XHdcp1xExample_Poll();
		XDpTxSs_SetPhysicalState(&DpTxSsInst, (hdcp_capable_org));
		XHdcp1xExample_Poll();
		DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 10000);
	}
#endif
}

#if ENABLE_AUDIO
volatile u8 TransmitComplete;   /* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;    /* Flag to check completion of Reception */


static void ReceiveHandler(XIic * InstancePtr)
{
        ReceiveComplete = 0;
}

static void SendHandler(XIic * InstancePtr)
{
        TransmitComplete = 0;
}

static void StatusHandler(XIic * InstancePtr, int Event)
{

}

int iic_write(u16 ByteCount)
{
        int Status;

        /*
         * Set the defaults.
         */
        TransmitComplete = 1;
        IicInstance.Stats.TxErrors = 0;
        XIic_InterruptHandler(&IicInstance);

        /*
         * Start the IIC device.
         */
        //xil_printf("sat0\r\n");
        Status = XIic_Start(&IicInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);
        /*
         * Send the Data.
         */
        Status = XIic_MasterSend(&IicInstance, WriteBuffer, ByteCount);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        /*
         * Wait till the transmission is completed.
         */
        while ((TransmitComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
			/*
			 * This condition is required to be checked in the case where we
			 * are writing two consecutive buffers of data to the EEPROM.
			 * The EEPROM takes about 2 milliseconds time to update the data
			 * internally after a STOP has been sent on the bus.
			 * A NACK will be generated in the case of a second write before
			 * the EEPROM updates the data internally resulting in a
			 * Transmission Error.
			 */
                XIic_InterruptHandler(&IicInstance);

                if (IicInstance.Stats.TxErrors != 0) {
                        XIic_InterruptHandler(&IicInstance);

                        /*
                         * Enable the IIC device.
                         */
                        Status = XIic_Start(&IicInstance);
                        if (Status != XST_SUCCESS) {
                                return XST_FAILURE;
                        }
                        //xil_printf("sat3\r\n");
                        XIic_InterruptHandler(&IicInstance);

                        if (!XIic_IsIicBusy(&IicInstance)) {
                                /*
                                 * Send the Data.
                                 */
                                XIic_InterruptHandler(&IicInstance);

                                Status = XIic_MasterSend(&IicInstance,
                                                         WriteBuffer,
                                                         ByteCount);
                                if (Status == XST_SUCCESS) {
                                        IicInstance.Stats.TxErrors = 0;
                                }
                                else {
                                }
                        }
                }
        }

        /*
         * Stop the IIC device.
         */
        //xil_printf("sat4\r\n");
        Status = XIic_Stop(&IicInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        //xil_printf("sat5\r\n");

        return XST_SUCCESS;
}

int iic_read(AddressType addr, u8 *BufferPtr, u16 ByteCount)
{
        int Status;
//      AddressType Address = EEPROM_TEST_START_ADDRESS;
        AddressType Address;
        Address = addr;

        /*
         * Set the Defaults.
         */
        ReceiveComplete = 1;
        XIic_InterruptHandler(&IicInstance);

        /*
         * Position the Pointer in EEPROM.
         */
        //xil_printf("st11\r\n");
        if (sizeof(Address) == 1) {
                WriteBuffer[0] = (u8) (Address);
        }
        else {
                WriteBuffer[0] = (u8) (Address >> 8);
                WriteBuffer[1] = (u8) (Address);
        }
        XIic_InterruptHandler(&IicInstance);

        Status = iic_write(sizeof(Address));
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        /*
         * Start the IIC device.
         */
        Status = XIic_Start(&IicInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        //xil_printf("start read2\r\n");
        XIic_InterruptHandler(&IicInstance);
        /*
         * Receive the Data.
         */
        Status = XIic_MasterRecv(&IicInstance, BufferPtr, ByteCount);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        /*
         * Wait till all the data is received.
         */
        while ((ReceiveComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
                XIic_InterruptHandler(&IicInstance);

        }
        /*
         * Stop the IIC device.
         */
        Status = XIic_Stop(&IicInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        return XST_SUCCESS;
}



int write_si570()
{
  u32 Index;
  int Status;
  AddressType Address = EEPROM_TEST_START_ADDRESS;
  AddressType addr;

  XIic_SetSendHandler(&IicInstance, &IicInstance,
                    (XIic_Handler) SendHandler);
  XIic_SetRecvHandler(&IicInstance, &IicInstance,
                    (XIic_Handler) ReceiveHandler);
  XIic_SetStatusHandler(&IicInstance, &IicInstance,
                      (XIic_StatusHandler) StatusHandler);
  /*
   * Initialize the data to write and the read buffer.
   */
  if (sizeof(Address) == 1) {
    WriteBuffer[0] = (u8) (Address);
  }
  else {
    WriteBuffer[0] = (u8) (Address >> 8);
    WriteBuffer[1] = (u8) (Address);
    ReadBuffer[Index] = 0;
  }

  /*
   * Set the Slave address to the PCA9543A.
   */
  Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE,
                         IIC_SWITCH_ADDRESS);
  //xil_printf("set addr end\r\n");
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }
  /*
   * Write to the IIC Switch.
   */
  WriteBuffer[0] = 0x01; //Select Bus0 - U1
  Status = iic_write(1);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }
  //xil_printf("eeprom write end\r\n");
  /*
   * Set the Slave address to the SI570
   */
  Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE,
                         IIC_SI570_ADDRESS);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }
  //xil_printf("set addr end\r\n");
  /*
   * Write to the SI570
   */
  // Set frequency back to default power-up value
  // In this case 156.250000 MHz
  //Freeze DCO bit in Reg 137
  WriteBuffer[0] = 137;
  WriteBuffer[1] = 0x10;
  Status = iic_write(sizeof(Address) + 1);
  if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("1 end\r\n");
//Recall the 156.2500000 value from NVM
//by setting RECALL (bit 0) = 1 in Reg 135
WriteBuffer[0] = 135;
WriteBuffer[1] = 0x01;
Status = iic_write(sizeof(Address) + 1);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("2 end\r\n");
//Un-Freeze DCO bit in Reg 137
WriteBuffer[0] = 137;
WriteBuffer[1] = 0x00;
Status = iic_write(sizeof(Address) + 1);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("3 end\r\n");
//Assert New Frequency bit in Reg 135
WriteBuffer[0] = 135;
WriteBuffer[1] = 0x40;
Status = iic_write(sizeof(Address) + 1);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("4 end\r\n");
// Wait 10 ms
int zz,kk;
for(kk= 0; kk<100000; kk++)
        zz = kk+1;
kk += zz; // Only to surpress warning
//udelay(1000);
//xil_printf("5 end\r\n");
// Update to user requested frequency
//Freeze DCO bit in Reg 137
WriteBuffer[0] = 137;
WriteBuffer[1] = 0x10;
Status = iic_write(sizeof(Address) + 1);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("6 end\r\n");
Status = iic_write(sizeof(Address) + 1);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("7 end\r\n");
//Set New Frequency to 400 MHz when starting from 156.25 MHz
WriteBuffer[0] = 7;
WriteBuffer[1] = UpdateBuffer[0];
WriteBuffer[2] = UpdateBuffer[1];
WriteBuffer[3] = UpdateBuffer[2];
WriteBuffer[4] = UpdateBuffer[3];
WriteBuffer[5] = UpdateBuffer[4];
WriteBuffer[6] = UpdateBuffer[5];

Status = iic_write(sizeof(Address) + 6);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("8 end\r\n");
//Un-Freeze DCO bit in Reg 137
WriteBuffer[0] = 137;
WriteBuffer[1] = 0x00;

Status = iic_write(sizeof(Address) + 1);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("9 end\r\n");
//Assert New Frequency bit in Reg 135
WriteBuffer[0] = 135;
WriteBuffer[1] = 0x40;

Status = iic_write(sizeof(Address) + 1);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("a end\r\n");
/*
 * Read from the SI570
 */
//xil_printf("Reading data from SI570\r\n");
addr = 7;
Status = iic_read(addr, ReadBuffer, 6);
if (Status != XST_SUCCESS) {
      return XST_FAILURE;
}
//xil_printf("b end\r\n");
/* Display Read Buffer
 *
 */
for (Index = 0; Index < 6; Index++) {
//    xil_printf("ReadBuffer[%02d] = %02X\r\n", Index, ReadBuffer[Index]);
}


return XST_SUCCESS;
}


void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame)
{
    u8 db1, db2, db3, db4;
    u32 temp;
    u8 RSVD=0;

    //Fixed paramaters
    u8  dp_version   = 0x11;

        //Write #1
    db1 = 0x00; //sec packet ID fixed to 0 - SST Mode
    db2 = 0x80 + xilInfoFrame->type;
    db3 = xilInfoFrame->info_length&0xFF;
    db4 = (dp_version<<2)|(xilInfoFrame->info_length>>8);
        temp = db4<<24|db3<<16|db2<<8|db1;
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);
//        dbg_printf("\n[AUDIO_INFOFRAME] Word1=0x%x\r",temp);

        //Write #2
        db1 = xilInfoFrame->audio_channel_count |
				(xilInfoFrame->audio_coding_type<<4) | (RSVD<<3);
        db2 = (RSVD<<5)| (xilInfoFrame->sampling_frequency<<2)
					| xilInfoFrame->sample_size;
        db3 = RSVD;
        db4 = xilInfoFrame->channel_allocation;
        temp = db4<<24|db3<<16|db2<<8|db1;
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_INFO_DATA(1), temp);
//        dbg_printf("\n[AUDIO_INFOFRAME] Word2=0x%x\r",temp);

        //Write #3
        db1 = (xilInfoFrame->level_shift<<3) | RSVD
					| (xilInfoFrame->downmix_inhibit <<7);
        db2 = RSVD;
        db3 = RSVD;
        db4 = RSVD;
        temp = db4<<24|db3<<16|db2<<8|db1;
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);
//        dbg_printf("\n[AUDIO_INFOFRAME] Word3=0x%x\r",temp);

        //Write #4
        db1 = RSVD;
        db2 = RSVD;
        db3 = RSVD;
        db4 = RSVD;
        temp = 0x00000000;
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);
//        dbg_printf("\n[AUDIO_INFOFRAME] Word4-Word8=0x%x\r",temp);
        //Write #5
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);

        //Write #6
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);
        //Write #7
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);
        //Write #8
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);
}


#endif


//Buffer Bypass is to be used when TX operates on stable clock.
//To use this, the TX should be configured with buffer bypass option.(hw change)

void prog_bb (u8 bw, u8 is_tx) {
#if BUFFER_BYPASS

// For Buffer Bypass, the clock output from GT is refclk
// this needs to be called before GT is init
// For KC705 the refclk is 135Mhz
	// For 2B, 4B mode,
	// 0x14 -> 270Mhz, 135Mhz
	// 0xA -> 135Mhz, 67.5Mhz
	// 0x6 -> 81Mhz, 40.5Mhz

	XVphy_MmcmReset(&VPhy_Instance, 0, XVPHY_DIR_TX, FALSE);
	xil_printf ("^^^");
	while (!((XVphy_ReadReg(VPhy_Instance.Config.BaseAddr, 0x120)) & 0x200)) {
	}

#if SET_TX_TO_2BYTE == 1
  if (is_tx == 1) { // TX only path using refclk0 of 135Mhz
	if (bw == 0x14) { //270Mhz
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
				(0x0 << 16 | 0x8 << 8 | 0x1));
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
				(0x0 << 16 | 0x4));
	} else if (bw == 0xA) { //135Mhz
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
				(0x0 << 16 | 0x8 << 8 | 0x1));
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
				(0x0 << 16 | 0x8));
	} else { //81Mhz
		if (is_TX_CPLL == 1) { // operates on 135Mhz refclk0
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
					(0x0 << 16 | 0x24 << 8 | 0x5));
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
					(0x0 << 16 | 0xC));
		} else { // operating on QPLL with 162Mhz refclk0
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
					(0x0 << 16 | 0xC << 8 | 0x2));
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
					(0x0 << 16 | 0xC));
		}
	}
  } else { // TX is using CPLL but refclk1 of 270/135/81
	  // For KC705 the refclk is 270Mhz, 135 or 81Mhz
		// For 2B, 4B mode,
		// 0x14 -> 270Mhz, 135Mhz
		// 0xA -> 135Mhz, 67.5Mhz
		// 0x6 -> 81Mhz, 40.5Mhz
	if (bw == 0x14) { //270Mhz
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
				(0x0 << 16 | 0x4 << 8 | 0x1));
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
				(0x0 << 16 | 0x4));
	} else if (bw == 0xA) { //135Mhz
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
				(0x0 << 16 | 0x8 << 8 | 0x1));
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
				(0x0 << 16 | 0x8));
	} else { //81Mhz
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
				(0x0 << 16 | 0xB << 8 | 0x1));
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
				(0x0 << 16 | 0xB));
	}
  }

#else // for 4B the clocks to be genrated are half of 2B
      if (is_tx == 1) { // TX only path using refclk0 of 135Mhz
		if (bw == 0x14) { //135Mhz
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
					(0x0 << 16 | 0x8 << 8 | 0x1));
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
					(0x0 << 16 | 0x8));
		} else if (bw == 0xA) { //67.5Mhz
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
					(0x0 << 16 | 0x8 << 8 | 0x1));
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
					(0x0 << 16 | 0x10));
		} else { //40.5Mhz
			if (is_TX_CPLL == 1) { // operates on 135Mhz refclk0
				XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
						(0x0 << 16 | 0x24 << 8 | 0x5));
				XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
						(0x0 << 16 | 0x18));
			} else { // operating on QPLL with 162Mhz refclk0
				XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
						(0x0 << 16 | 0xC << 8 | 0x2));
				XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
						(0x0 << 16 | 0x18));
			}
		}
      } else { // TX is using CPLL but refclk1 of 270/135/81
	  // For KC705 the refclk is 270Mhz, 135 or 81Mhz
		// For 2B, 4B mode,
		// 0x14 -> 270Mhz, 135Mhz
		// 0xA -> 135Mhz, 67.5Mhz
		// 0x6 -> 81Mhz, 40.5Mhz
		if (bw == 0x14) { //135Mhz
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
					(0x0 << 16 | 0x4 << 8 | 0x1));
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
					(0x0 << 16 | 0x8));
		} else if (bw == 0xA) { //67.5Mhz
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
					(0x0 << 16 | 0x8 << 8 | 0x1));
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
					(0x0 << 16 | 0x10));
		} else { //40.5Mhz
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x124,
					(0x0 << 16 | 0xB << 8 | 0x1));
			XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x128,
					(0x0 << 16 | 0x16));
		}
      }
#endif

	XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, 0x120, 0x1);
		while (!((XVphy_ReadReg(VPhy_Instance.Config.BaseAddr, 0x120))
																	& 0x100)) {
		}
		xil_printf ("*");
		XVphy_MmcmReset(&VPhy_Instance, 0, XVPHY_DIR_TX, FALSE);
		while (!((XVphy_ReadReg(VPhy_Instance.Config.BaseAddr, 0x120))
																	& 0x200)) {
        }
		xil_printf ("^^^\r\n");
#endif
}


void vdma_stop() {
//	xil_printf ("\r\nResetting VDMA...");
	XAxiVdma_DmaStop(&dma_struct[0].AxiVdma, XAXIVDMA_WRITE);
//	xil_printf (".");
	XAxiVdma_DmaStop(&dma_struct[0].AxiVdma, XAXIVDMA_READ);
//	xil_printf (".");
	XAxiVdma_Reset (&dma_struct[0].AxiVdma, XAXIVDMA_WRITE);
//	xil_printf (".");
	XAxiVdma_Reset (&dma_struct[0].AxiVdma, XAXIVDMA_READ);
//	xil_printf (".");
	while ((XAxiVdma_ResetNotDone(&dma_struct[0].AxiVdma, XAXIVDMA_WRITE)) ||
			(XAxiVdma_ResetNotDone(&dma_struct[0].AxiVdma, XAXIVDMA_READ))) {
	}
//	xil_printf ("!\r\n");
}

void vdma_start() {
    Dprx_StartVDMA(&dma_struct[0].AxiVdma, XAXIVDMA_WRITE, dp_msa_hres,
					dp_msa_vres, pixel,dma_struct);
    Dprx_StartVDMA(&dma_struct[0].AxiVdma, XAXIVDMA_READ, dp_msa_hres,
					dp_msa_vres, pixel,dma_struct);
}


// This function is to increase stability in some special cases.
void tx_preset() {

	XDpTxSs_SetLinkRate(&DpTxSsInst, 6);
	XDpTxSs_SetLaneCount(&DpTxSsInst, 1);
	XDpTxSs_SetVidMode(&DpTxSsInst, 1);
	XDpTxSs_SetBpc(&DpTxSsInst, 8);
	XDpTxSs_Start(&DpTxSsInst);

}
int ceil_func(double x){
	return (int)( x < 0.0 ? x : x+0.9 );
}
