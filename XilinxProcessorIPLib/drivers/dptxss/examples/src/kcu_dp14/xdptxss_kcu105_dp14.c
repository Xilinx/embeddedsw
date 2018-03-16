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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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
 * @file xdptxss_kcu105_dp14.c
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   KI   12/09/17 Initial release.
 *
*******************************************************************************/

#include "dppt.h"

int gIsKeyWrittenInEeeprom = FALSE;

typedef unsigned int    UINT32;
typedef unsigned int    UINT8;
typedef unsigned int    UINT16;

XIic IicInstance;

typedef u8 AddressType;
u8 si570_reg_value[NUM_MODES][NUM_CLOCK_REGS] = {
	/* As per Si570 programmable oscillator calculator. */
	// 7,     8,     9,    10,      11,      12,
	{0x4C, 0x42, 0xB0, 0x21, 0xDE, 0x77 }, // = {32kHz * 512)
	{0xA5, 0xC2, 0xAA, 0xCC, 0x9D, 0x51 }, // = (44.1kHz * 512)
	{0xE4, 0xC2, 0xF4, 0xB9, 0x4A, 0xA7 }, // = (48kHz * 512)
	{0xA2, 0XC2, 0XAA, 0XCC, 0X9D, 0X51 }, // = {88.2khZ * 512)
	{0x24, 0xC2, 0xB0, 0x21, 0xDE, 0x77 }, // = {96kHz * 512)
	{0xA1, 0x42, 0xAA, 0xCC, 0x9D, 0x51 }, // = (176.4kHz * 512)
	{0x22, 0x42, 0xB0, 0x21, 0xDE, 0x77 }  // = {192kHz * 512)
};

struct dma_chan_parms dma_struct[1];

XDpTxSs_MainStreamAttributes Msa[4];

#define COMPLIANCE_PAT1 0x3E0F83E0
#define COMPLIANCE_PAT2 0x0F83E0F8
#define COMPLIANCE_PAT3 0xF83E

#define DPCD_TEST_CRC_R_Cr   0x240
#define DPCD_TEST_SINK_MISC  0x246
#define DPCD_TEST_SINK_START 0x270
#define CRC_AVAIL_TIMEOUT    1000

/* Local Globals */

/************************** Variable Definitions *****************************/
XUartLite UartLite;		/* Instance of the UartLite device */
XGpio GpioLED;			/* Instance of the Gpio8bitsLED */
XTmrCtr TmrCtr;			/* Instance of the Timer/Counter */
XVphy VPhy_Instance;
XIntc IntcInst;			/* The interrupt controller instance.*/
XDpTxSs DpTxSsInst;		/* The DPTX Subsystem instance.*/
XDpTxSs_Config *DPTxSSConfig;
XDpRxSs DpRxSsInst;		/* The DPRX Subsystem instance.*/
XDpRxSs_Config *DPRxSSConfig;

volatile u32 mst_hpd_event;
user_config_struct user_config;

u8 StartTxAfterRx;
u16 RxTrainedFromMenu;
/**************************** Type Definitions *******************************/
u8 C_VideoUserStreamPattern[8] =
		{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17}; //Duplicate
u8 Bpc[] = {6, 8, 10, 12, 16};

struct dma_chan_parms dma_struct[1];

XDpTxSs_MainStreamAttributes Msa[4];

/************************** Function Prototypes ******************************/
static void Dprx_InterruptHandlerVmChange(void *InstancePtr);
static void Dprx_InterruptHandlerNoVideo(void *InstancePtr);
static void Dprx_InterruptHandlerVBlank(void *InstancePtr);
static void Dprx_InterruptHandlerTrainingLost(void *InstancePtr);
static void Dprx_InterruptHandlerVideo(void *InstancePtr);
static void Dprx_CheckSetupTx(void *InstancePtr);
static void Dprx_DetectResolution(void *InstancePtr);
static void Dprx_ResetVideoOutput(void *InstancePtr);


u8 prog_tx; /*This variable triggers detect_rx_video_and_startTx()*/
u8 rx_ran_once;
u32 training_done;
u32 vblank_count;
u8 start_tracking;
u8 change_detected;
u8 only_tx_active;
u8 rx_link_change_requested;
u8 switch_to_patgen;
u8 need_to_retrain_rx;
u8 Edid_org[128];
u8 Edid1_org[128];
u8 Edid2_org[128];
u8 max_cap_lanes;
u8 max_cap_org;
u8 tx_is_reconnected; /*This variable triggers hpd_con*/
u8 hpd_pulse_con_event; /*This variable triggers hpd_pulse_con*/

u8 enabled;
u8 gt_stable;
u8 LineRate_init_tx = 0x14;
u8 LaneCount_init_tx = 0x4;
int monitor_8K;
u8 use_monitor_edid;
u8 bypass_vid_common;
u8 rx_linkup_trig;
u8 edid_monitor[384];

XV_axi4s_remap          rx_remap;
XV_axi4s_remap          tx_remap;




//typedef struct
//{
//	u8 type;
//	u8 version;
//	u8 length;
//	u8 audio_coding_type;
//	u8 audio_channel_count;
//	u8 sampling_frequency;
//	u8 sample_size;
//	u8 level_shift;
//	u8 downmix_inhibit;
//	u8 channel_allocation;
//	u16 info_length;
//} XilAudioInfoFrame;

typedef struct
{
	u8 sec_id;//DP Specific
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
	u8 frame_count;
} XilAudioInfoFrame;

XilAudioInfoFrame *xilInfoFrame;

void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame);

/*The structure defines Generic Frame Packet fields*/
typedef struct
{
	u32 frame_count;
	u32 frame_count_q;
	u8 Header[4];
	u8 Payload[32];
} XilAudioExtFrame;

XilAudioInfoFrame AudioinfoFrame;
XilAudioExtFrame  SdpExtFrame;


volatile u32 SstHpdEvent = 0;
unsigned char bpc_table[] = {6,8,10,12,16};

typedef struct
{
	unsigned char lane_count;
	unsigned char link_rate;
} lane_link_rate_struct;


lane_link_rate_struct lane_link_table[]=
{
	{XDP_RX_OVER_LANE_COUNT_SET_1, XDP_RX_OVER_LINK_BW_SET_162GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_2, XDP_RX_OVER_LINK_BW_SET_162GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_4, XDP_RX_OVER_LINK_BW_SET_162GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_1, XDP_RX_OVER_LINK_BW_SET_270GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_2, XDP_RX_OVER_LINK_BW_SET_270GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_4, XDP_RX_OVER_LINK_BW_SET_270GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_1, XDP_RX_OVER_LINK_BW_SET_540GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_2, XDP_RX_OVER_LINK_BW_SET_540GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_4, XDP_RX_OVER_LINK_BW_SET_540GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_1, XDP_RX_OVER_LINK_BW_SET_810GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_2, XDP_RX_OVER_LINK_BW_SET_810GBPS},
	{XDP_RX_OVER_LANE_COUNT_SET_4, XDP_RX_OVER_LINK_BW_SET_810GBPS},

};

static XVphy_User_Config PHY_User_Config_Table[] =
{
/*	Index,         TxPLL,            RxPLL,
 *	TxChId,        RxChId,
 *	LineRate,      LineRateHz,       QPLLRefClkSrc,
 *	CPLLRefClkSrc, QPLLRefClkFreqHz, CPLLRefClkFreqHz
 * */
	{   0,  XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
	0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,      DP159_FORWARDED_CLK,
	DP159_FORWARDED_CLK,         270000000,           270000000     },
	{   1,  XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
	0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,      DP159_FORWARDED_CLK,
	DP159_FORWARDED_CLK,            270000000,           270000000  },
	{   2,  XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
	0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,      DP159_FORWARDED_CLK,
	DP159_FORWARDED_CLK,            270000000,           270000000  },
	{   3,  XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
	0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,      ONBOARD_REF_CLK,
	DP159_FORWARDED_CLK,         270000000,           270000000     },
	{   4,  XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
	0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,      ONBOARD_REF_CLK,
	DP159_FORWARDED_CLK,        270000000,           270000000      },
	{   5,  XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
	0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,      ONBOARD_REF_CLK,
	DP159_FORWARDED_CLK,        270000000,           270000000      },
	{   6,  XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
	0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,      ONBOARD_REF_CLK,
	ONBOARD_REF_CLK,         270000000,           270000000         },
	{   7,  XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
	0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,      ONBOARD_REF_CLK,
	ONBOARD_REF_CLK,                270000000,           270000000  },
	{   8,  XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
	0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,      ONBOARD_REF_CLK,
	ONBOARD_REF_CLK,                270000000,           270000000  },
	{   9,  XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
	0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,      ONBOARD_REF_CLK,
	ONBOARD_REF_CLK,                270000000,           270000000  },
	{   10,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
	XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
	0x1E,    XVPHY_DP_LINK_RATE_HZ_810GBPS,      DP159_FORWARDED_CLK,
	DP159_FORWARDED_CLK,        270000000,           270000000      }
};

/* extern XVidC_VideoMode resolution_table[]; */
/* Adding new resolution definition example
 * XVIDC_VM_3840x2160_30_P_SB, XVIDC_B_TIMING3_60_P_RB
 * and XVIDC_VM_3840x2160_60_P_RB has added.
 * */
typedef enum {
	XVIDC_VM_1920x1080_60_P_RB = (XVIDC_VM_CUSTOM + 1),
	XVIDC_B_TIMING3_60_P_RB ,
	XVIDC_VM_3840x2160_120_P_RB,
	XVIDC_VM_7680x4320_24_P,
	XVIDC_VM_7680x4320_25_P,
	XVIDC_VM_7680x4320_30_P,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_5120x2880_60_P_RB2,

	XVIDC_VM_7680x4320_30_MSTR,
	XVIDC_VM_5120x2880_60_MSTR,
	XVIDC_VM_3840x2160_120_MSTR,
	XVIDC_CM_NUM_SUPPORTED
} XVIDC_CUSTOM_MODES;

/* CUSTOM_TIMING: Here is the detailed 
 * timing for each custom resolutions.
 * */
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode[
		(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{
	{ XVIDC_VM_1920x1080_60_P_RB, "1920x1080@60Hz (RB)", XVIDC_FR_60HZ,
		{1920, 48, 32, 80, 2080, 1,
		1080, 3, 5, 23, 1111, 0, 0, 0, 0, 0}},
	{ XVIDC_B_TIMING3_60_P_RB, "2560x1440@60Hz (RB)", XVIDC_FR_60HZ,
		{2560, 48, 32, 80, 2720, 1,
		1440, 3, 5, 33, 1481, 0, 0, 0, 0, 0}},
	{ XVIDC_VM_3840x2160_120_P_RB, "3840x2160@120Hz (RB)", XVIDC_FR_120HZ,
		{3840, 8, 32, 40, 3920, 1,
		2160, 113, 8, 6, 2287, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_7680x4320_24_P, "7680x4320@24Hz", XVIDC_FR_24HZ,
		{7680, 352, 176, 592, 8800, 1,
		4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_25_P, "7680x4320@25Hz", XVIDC_FR_25HZ,
		{7680, 352, 176, 592, 8800, 1,
		4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_P, "7680x4320@30Hz", XVIDC_FR_30HZ,
		{7680, 8, 32, 40, 7760, 0,
		4320, 47, 8, 6, 4381, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_3840x2160_100_P_RB2, "3840x2160@100Hz (RB2)", XVIDC_FR_100HZ,
		{3840, 8, 32, 40, 3920, 0,
		2160, 91, 8, 6, 2265, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_DELL, "7680x4320_DELL@30Hz", XVIDC_FR_30HZ,
		{7680, 48, 32, 80, 7840, 0,
		4320, 3, 5, 53, 4381, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_5120x2880_60_P_RB2, "5120x2880@60Hz (RB2)", XVIDC_FR_60HZ,
		{5120, 8, 32, 40, 5200, 0,
		2880, 68, 8, 6, 2962, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_MSTR, "7680x4320_MSTR@30Hz", XVIDC_FR_30HZ,
		{7680, 25, 97, 239, 8041, 0,
		4320, 48, 9, 5, 4382, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_5120x2880_60_MSTR, "5120x2880@60Hz_MSTR", XVIDC_FR_60HZ,
		{5120, 25, 97, 239, 5481, 0,
		2880, 48, 9, 5, 2942, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_3840x2160_120_MSTR, "3840x2160@120Hz_MSTR", XVIDC_FR_120HZ,
		{3840, 48, 34, 79, 4001, 1,
		2160, 4, 6, 53, 2223, 0, 0, 0, 0, 1}},
};

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
	XVIDC_VM_USE_EDID_PREFERRED,

	XVIDC_VM_1920x1080_60_P_RB,
	XVIDC_VM_3840x2160_60_P_RB,
	XVIDC_VM_3840x2160_120_P_RB,
	XVIDC_VM_7680x4320_24_P,
	XVIDC_VM_7680x4320_30_P,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_5120x2880_60_P_RB2,
	XVIDC_VM_7680x4320_30_MSTR,
	XVIDC_VM_5120x2880_60_MSTR,
	XVIDC_VM_3840x2160_120_MSTR
};

double max_freq[] = {216.0, 172.8, 360.0, 288.0, 720.0, 576.0};


typedef struct
{
	unsigned char LaneCount;
	unsigned char LineRate;
	unsigned char pixel;
	unsigned char bpc;
} dp_conf_struct;

dp_conf_struct dp_conf;

int main(void)
{
	/* Initialize ICache */
	Xil_ICacheInvalidate ();
	Xil_ICacheEnable ();
	/* Initialize DCache */
	Xil_DCacheInvalidate ();
	Xil_DCacheEnable ();

	xil_printf("\n*****************************************************"
			"***********\n\r");
	char UserInput;

	u32 Status;
	u8 LineRate_init = XDP_DPCD_LINK_BW_SET_810GBPS;
	u8 LineRate_init_tx = XDP_DPCD_LINK_BW_SET_810GBPS;
	u8 LaneCount_init = XDP_DPCD_LANE_COUNT_SET_4;
	u8 LaneCount_init_tx = XDP_DPCD_LANE_COUNT_SET_4;
	u32 data, addr;
	u8 MainMenu =0;
//	u32 tmp_rd = 0;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	int i =0;
	int track_count = 0;
	int track_count1 = 0;
	int track_switch = 0;
	u8 exit = 0;
	u8 connected = 0;
	u8 Data[8];
	u32 count_track = 0;
	u8 pwr_dwn = 0;
	u8 in_pwr_save = 0;
	u32 aux_reg_address,num_of_aux_registers;
	u16 crc_wait_cnt;

	u8 UpdateBuffer[sizeof(AddressType) + PAGE_SIZE];
	u32 bpc_track =0;
	u32 recv_clk_freq_track=0;
	float recv_frame_clk_track=0.0;
	u8 done = 0;
	u8 pwr_dwn_x = 0;
	int edid_page = 1;
	u32 rxMsamisc0_track;
	u32 recv_frame_clk_int_track =0;
	u32 user_lane_count;
	u32 user_link_rate;
	u32 user_tx_LaneCount;
	u32 user_tx_LineRate;
	int m_aud = 0;
	int n_aud = 0;

	XV_axi4s_remap_Config   *rx_remap_Config;
	XV_axi4s_remap_Config   *tx_remap_Config;
	u32 clk_reg0;
	u32 clk_reg1;
	u32 clk_reg2;
	u8 pat_update = 1;
	u8 audio_on = 0;

	u8 mcdp6000_reset = 0;

	// Adding custom resolutions at here.
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
		   (XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
			(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));

	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom "
			   "timing table\r\r\n\n");
	}

	VideoFMC_Init();
	IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR,
				0, 270000000, TRUE);

	DP141_init(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR);

	/* Initializing user_config parameters */
	user_config.user_numStreams = 1;
	user_config.user_bpc = 8;
	user_config.user_format = 1;

	user_config.mst_check_flag=XPAR_DP_TX_HIER_V_DP_TXSS1_0_DP_MST_ENABLE;
	xil_printf("\n********************************"
		     "********************************\n\r");

	/* EDID mode start with pass-through mode. */
	use_monitor_edid = 1;
	/* Set vid_comm mode to be bypass */
	bypass_vid_common = 1;

	init_peripherals();
	/* Initializing Interrupts */

#if ENABLE_AUDIO
	for( i = 0; i < 6; i++ ) {
		UpdateBuffer[i] = si570_reg_value[2][i];
	}
	write_si570(UpdateBuffer);
	xil_printf("SI570 Config done\n\r");
#endif
	/* Reset VDMA to ensure clean recovery. */
	vdma_stop(&dma_struct);
	xil_printf("\nVDMA has been reset\n\r");
	xil_printf("\033[H\033[J"); //clears the screen
#if XPAR_XDPRXSS_NUM_INSTANCES
	DPRxInitialize();
	/* DPRxSs uses its own I2C driver */
	XDpRxSs_McDp6000_init(&DpRxSsInst, DpRxSsInst.IicPtr->BaseAddress);
#endif

	clk_reg0 = Xil_In32 (CLK_WIZ_BASE+0x200);
	clk_reg1 = Xil_In32 (CLK_WIZ_BASE+0x204);
	clk_reg2 = Xil_In32 (CLK_WIZ_BASE+0x208);

	xil_printf("\n********************************"
		     "********************************\n\r");
	xil_printf("            DisplayPort Pass Through Demonstration \n\r");
	xil_printf("                   (c) by Xilinx   ");
	xil_printf("%s %s\n\r\r\n", __DATE__  ,__TIME__ );
	xil_printf("                   System Configuration:\r\n");
	xil_printf("                      DP SS : %d byte\r\n",
				2 * SET_TX_TO_2BYTE);
	xil_printf("\n********************************"
		     "********************************\n\r");

	DPTxInitialize();
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);

	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
		xil_printf("Please connect a DP Monitor to start the "
				"application !!!\r\n");
		connected = 1;
		}
	}

	/* Waking up the monitor. */
	sink_power_cycle(400);

	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400000);
	/* Reading the first block of EDID */
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
		u8 Sum = 0;
		for (int i = 0; i < 128; i++) {
			Sum += Edid_org[i];
		}

		if(Sum != 0){
			xil_printf("Wrong EDID was read\r\n");
		}

		/* Reading the second block of EDID. */
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid2_org, 2);
		xil_printf("Reading EDID contents of the DP Monitor..\r\n");
		
		Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x1, 1, &max_cap_org);
		Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2,
					1, &max_cap_lanes);
		if(max_cap_org == XDP_DPCD_LINK_BW_SET_810GBPS)
			monitor_8K = 1;

		u8 rData = 0;
		/* Check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit. */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL,
					1, &rData);
		/* if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled. */
		if (rData & 0x80) {
			/* Read maximum rate. */
			XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData);
			if (rData == XDP_DPCD_LINK_BW_SET_810GBPS) {
				monitor_8K = 1;
				max_cap_org = 0x1E;
			}
		}

		if (Status != XST_SUCCESS) {
			xil_printf("Failed to read sink capabilities\r\n");
		}

#if CAP_OVER_RIDE
		LineRate_init = MAX_RATE;
		LineRate_init_tx = MAX_RATE;
		LaneCount_init = MAX_LANE;
		LaneCount_init_tx = MAX_LANE;
		initial_value = LineRate_init;
		DpTxSsInst.DpPtr->Config.MaxLinkRate = MAX_RATE;
		DpTxSsInst.DpPtr->Config.MaxLaneCount = MAX_LANE;
#else
		LineRate_init = max_cap_org;
		LineRate_init_tx = max_cap_org;
		LaneCount_init = max_cap_lanes&0x1F;
		LaneCount_init_tx = max_cap_lanes&0x1F;

		/* This status is carried from the previous AUX read. */
		if (Status != XST_SUCCESS) {
			max_cap_org = XDP_DPCD_LINK_BW_SET_540GBPS;
			max_cap_lanes = XDP_DPCD_LANE_COUNT_SET_4;
			LineRate_init = max_cap_org;
			LineRate_init_tx = max_cap_org;
			LaneCount_init = max_cap_lanes&0x1F;
			LaneCount_init_tx = max_cap_lanes&0x1F;
		}
#endif

	} else {
		xil_printf("Please connect a DP Monitor and try again !!!\r\n");
		return 0;
	}

	if (use_monitor_edid) {
		/* This is EDID pass-through mode. */
		xil_printf("Setting same EDID contents in DP RX..\r\n");

		update_edid();

	} else {
		/* This is None-EDID pass-through mode */
		/* setting default Xilinx EDID which supports 8K30, */
		edid_default();
	}

	xil_printf("System capabilities set to: LineRate %x, LaneCount %x\r\n",
				LineRate_init,LaneCount_init);
	DPPtIntrInitialize();

#if ENABLE_AUDIO
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 
			XDP_RX_AUDIO_CONTROL, 0x0);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_AUDIO_CONTROL, 0x0);

	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_AUDIO_CONTROL, 0x0);
#endif

	XDpRxSs_SetLinkRate(&DpRxSsInst, LineRate_init);
	XDpRxSs_SetLaneCount(&DpRxSsInst, LaneCount_init);
	XDpRxSs_Start(&DpRxSsInst);
//	/* Programming AUX defer to 6. */
//	tmp_rd = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x4);
//	tmp_rd |= tmp_rd | 0x06000000;
//	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x4, tmp_rd);

	/* Setting RX link to disabled state. This is to ensure
	 * that Source gets enough time to authenticate and do the 
	 * HDCP stuff (such as writing AKSVs)
	 * */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_LINK_ENABLE, 0x0);
	/* Programming the unplug time register of DP RX 
	 * for long value else the cable unplug events 
	 * come very frequently
	 * */

	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_BS_IDLE_TIME,
					0x047868C0);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK,
					0xFFF87FFD);

	/* setting up remapper at here */
	rx_remap_Config = XV_axi4s_remap_LookupConfig(REMAP_RX_DEVICE_ID);
	Status = XV_axi4s_remap_CfgInitialize(
			&rx_remap, rx_remap_Config, rx_remap_Config->BaseAddress);

	rx_remap.IsReady = XIL_COMPONENT_IS_READY;
	if(Status != XST_SUCCESS){
		xil_printf("ERROR:: AXI4S_REMAP Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	tx_remap_Config = XV_axi4s_remap_LookupConfig(REMAP_TX_DEVICE_ID);
	Status = XV_axi4s_remap_CfgInitialize(
			&tx_remap, tx_remap_Config, tx_remap_Config->BaseAddress);

	tx_remap.IsReady = XIL_COMPONENT_IS_READY;
	if(Status != XST_SUCCESS){
		xil_printf("ERROR:: AXI4S_REMAP Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	XV_axi4s_remap_Set_width(&rx_remap, 7680);
	XV_axi4s_remap_Set_height(&rx_remap, 4320);
	XV_axi4s_remap_Set_ColorFormat(&rx_remap, 0);
	XV_axi4s_remap_Set_inPixClk(&rx_remap, 4);
	XV_axi4s_remap_Set_outPixClk(&rx_remap, 4);

	XV_axi4s_remap_Set_width(&tx_remap, 7680);
	XV_axi4s_remap_Set_height(&tx_remap, 4320);
	XV_axi4s_remap_Set_ColorFormat(&tx_remap, 0);
	XV_axi4s_remap_Set_inPixClk(&tx_remap, 4);
	XV_axi4s_remap_Set_outPixClk(&tx_remap, 4);

	app_help();

	while (1) {
		UserInput = 0;
#if FOR_INTERNAL
		UserInput = GetInbyte();
#else
		UserInput = GetInbyte();
#endif

		switch (UserInput) {
		case 't':
			/* Ensuring HDCP is disabled. */
			MainMenu = 0;
			only_tx_active = 1;
			start_tracking = 0;

			rx_ran_once = 0;
			XIntc_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
			XIntc_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_LINK_ENABLE, 0x0);
			exit = 0;
			xil_printf("*************************************"
					"********************************\r\n");
			xil_printf("In this configuration the TX acts as "
					"Master. RX is disabled.       \r\n");
			xil_printf("This mode operates on the 270Mhz clock "
					"generated on the oscillator \r\n");
			xil_printf("on the FMC board. QPLL is used for TX \r\n");
			xil_printf("**************************************"
					"*******************************\r\n");
			DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
			DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;
			Msa[0].ComponentFormat = 0x0;
			dp_conf.LineRate  = LineRate_init_tx;
			dp_conf.LaneCount = LaneCount_init_tx;
			XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_TX,(FALSE));

			/* This configures the vid_phy for line 
			 * rate to start with. */
			switch (LineRate_init_tx) {
			case XDP_DPCD_LINK_BW_SET_162GBPS :
				prog_bb(XDP_DPCD_LINK_BW_SET_162GBPS, 1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
					PHY_User_Config_Table[3]);
				break;

			case XDP_DPCD_LINK_BW_SET_270GBPS :
				prog_bb(XDP_DPCD_LINK_BW_SET_270GBPS, 1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
					PHY_User_Config_Table[4]);
				break;

			case XDP_DPCD_LINK_BW_SET_540GBPS :
				prog_bb(XDP_DPCD_LINK_BW_SET_540GBPS, 1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
					PHY_User_Config_Table[5]);
				break;

			case XDP_DPCD_LINK_BW_SET_810GBPS :
				prog_bb(XDP_DPCD_LINK_BW_SET_810GBPS, 1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
					PHY_User_Config_Table[10]);
				break;

			}
			
			if (Status != XST_SUCCESS) {
				xil_printf("+++++++ TX GT configuration encou"
					    "ntered a failure +++++++\r\n");
			}
			/* The clk_wiz that generates half of lnk_clk has 
			 * to be programmed as soon  as lnk clk is valid.
			 * */
			LaneCount_init_tx = LaneCount_init_tx & 0x7;
			start_tx (LineRate_init_tx, LaneCount_init_tx,
					resolution_table[2], 8, 1, pat_update);

			dp_conf.LineRate =
					DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
			dp_conf.LaneCount =
					DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
			/* Enabling TX interrupts. */
			sub_help_menu ();
			CmdKey[0] = 0;
			CommandKey = 0;

			while (MainMenu == 0) {

				if (tx_is_reconnected == 1) {
					hpd_con();
					tx_is_reconnected = 0;
				}

				CmdKey[0] = 0;
				CommandKey = 0;

				CommandKey = xil_getc(0xff);
				Command = atoi(&CommandKey);
				if (Command != 0) {
					xil_printf("You have selected "
						   "command %d\r\n", Command);
				}

				switch (CommandKey) {
#if ENABLE_AUDIO
				case 'a' :
					if (audio_on == 0) {
						xilInfoFrame->audio_channel_count = 1;
						xilInfoFrame->audio_coding_type = 0;
						xilInfoFrame->channel_allocation = 0;
						xilInfoFrame->downmix_inhibit = 0;
						xilInfoFrame->info_length = 27;
						xilInfoFrame->level_shift = 0;
						xilInfoFrame->sample_size = 0;//16 bits
						xilInfoFrame->sampling_frequency = 0; //48 Hz
						xilInfoFrame->type = 0x84;
						xilInfoFrame->version = 0x12;
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_CONTROL, 0x0);
						sendAudioInfoFrame(xilInfoFrame);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_CHANNELS, 0x2);
						switch(dp_conf.LineRate){
						case  6:m_aud = 512; n_aud = 3375; break;
						case 10:m_aud = 512; n_aud = 5625; break;
						case 20:m_aud = 512; n_aud = 11250; break;
						case 30:m_aud = 512; n_aud = 16875; break;
						}
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_MAUD,  m_aud );
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_NAUD,  n_aud );
						Vpg_Audio_start();

						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_CONTROL, 0x1);
						xil_printf("Audio enabled\r\n");
						audio_on = 1;
					} else {
						Vpg_Audio_stop();
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_CONTROL, 0x0);
						xil_printf("Audio disabled\r\n");
						audio_on = 0;
					}
					break;
#endif
				case '1' :
					/* Resolution menu. */
					dp_conf.LineRate =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;

					dp_conf.LaneCount =
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;

					resolution_help_menu();
					exit = 0;
					while (exit == 0) {
					CmdKey[0] = 0;
					Command = 0;
					CmdKey[0] = inbyte_local();
					Command = (int)CmdKey[0];

					switch  (CmdKey[0]){
					case 'x' :
						exit = 1;
						sub_help_menu ();
						break;

					default :
					xil_printf("You have selected command '%c'\n\r",
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
					xil_printf("\r\nSetting resolution...\r\n");
					audio_on = 0;
					user_config.VideoMode_local = resolution_table[Command];


					start_tx (dp_conf.LineRate,dp_conf.LaneCount,
							resolution_table[Command],
							0,0, pat_update);

					dp_conf.LineRate =
						  DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
					dp_conf.LaneCount =
						  DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;

					exit = done;
					break;

					}
					}

					sub_help_menu ();
					break;

				case '2' :
					/* BPC menu. */
					dp_conf.LineRate =
						DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
					dp_conf.LaneCount =
						DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
					exit = 0;
					bpc_help_menu();
					while (exit == 0) {
						CommandKey = 0;
						Command = 0;
						CommandKey = inbyte_local();
						Command = (int)CommandKey;
						switch (CommandKey) {
						case 'x' :
							exit = 1;
							sub_help_menu ();
							break;

						default :
							Command = Command - 48;
							dp_conf.bpc = bpc_table[Command];
							xil_printf("You have selected %c\r\n",
											CommandKey);
							if ((Command>4)) {
								bpc_help_menu();
								done = 0;
								break;
							} else {
								xil_printf("Setting BPC of %d\r\n",
										dp_conf.bpc);
								done = 1;
							}
							start_tx (dp_conf.LineRate, dp_conf.LaneCount, 0,
									dp_conf.bpc, 0, pat_update);

							dp_conf.LineRate =
									DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;

							dp_conf.LaneCount =
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

						if(Command>47 && Command<58)
							Command = Command - 48;
						else if(Command>96 && Command<123)
							Command = Command - 87;

						switch (CmdKey[0]) {
						case 'x' :
							exit = 1;
							sub_help_menu ();
							break;

						default :
							xil_printf("You have selected command %c\n\r",
									CmdKey[0]);
							if ((Command >= 0) && (Command < 12)) {
								user_tx_LaneCount =
									lane_link_table[Command].lane_count;
								user_tx_LineRate =
									lane_link_table[Command].link_rate;

								if (lane_link_table[Command].lane_count >
								    DpTxSsInst.Config.MaxLaneCount) {
									xil_printf("This Lane Count is not "
										   "supported by Sink \n\r");
									xil_printf("Max Supported Lane Count"
										   " is 0x%x \n\r", 
										   DpTxSsInst.Config.MaxLaneCount);
									xil_printf("Training at Supported Lane "
										   "count  \r\n");
									dp_conf.LaneCount = 
										DpTxSsInst.Config.MaxLaneCount;
								}
								done = 1;
							} else {
								xil_printf("!!!Warning: You have selected "
									   "wrong option for lane count "
									   "and link rate\n\r");
								select_link_lane();
								done = 0;
								break;
							}

							/* Disabling TX interrupts. */
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								     XDP_TX_INTERRUPT_MASK, 0xFFF);
							LineRate_init_tx = user_tx_LineRate;
							LaneCount_init_tx = user_tx_LaneCount;

							switch (LineRate_init_tx) {
							case XDP_DPCD_LINK_BW_SET_162GBPS:
								prog_bb(XDP_DPCD_LINK_BW_SET_162GBPS, 1) ;
								Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[3]);
								break;

							case XDP_DPCD_LINK_BW_SET_270GBPS :
								prog_bb(XDP_DPCD_LINK_BW_SET_270GBPS, 1);
								Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[4]);
								break;

							case XDP_DPCD_LINK_BW_SET_540GBPS:
								prog_bb(XDP_DPCD_LINK_BW_SET_540GBPS, 1);
								Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[5]);
								break;

							case XDP_DPCD_LINK_BW_SET_810GBPS:
								prog_bb(XDP_DPCD_LINK_BW_SET_810GBPS, 1);
								Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[10]);
								break;
							}

							if (Status != XST_SUCCESS) {
								xil_printf("+++++++ TX GT configuration "
									   "encountered a failure +++++++\r\n");
							}

							need_to_retrain_rx = 0;
							XDpTxSs_Stop(&DpTxSsInst);
							audio_on = 0;
							xil_printf("TX Link & Lane Capability is set to "
								   "%x, %x\r\n", user_tx_LineRate,
								   user_tx_LaneCount);
							xil_printf("Setting TX to 8 BPC and 800x600 resolution\r\n");
							XDpTxSs_Reset(&DpTxSsInst);
							start_tx(user_tx_LineRate, user_tx_LaneCount,
								 resolution_table[2], 8, 1, pat_update);
							dp_conf.LineRate =
								DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
							dp_conf.LaneCount =
								DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
							exit = done;
							break;
						}
					}
					sub_help_menu ();
					break;

				/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

				case '4' :
					/* Pattern menu */
					test_pattern_gen_help();
					exit = 0;
					while (exit == 0) {
						CommandKey = 0;
						CommandKey = inbyte_local();
						Command = (int)CommandKey;
						Command = Command - 48;
						switch (CommandKey) {
						case 'x' :
							exit = 1;
							sub_help_menu ();
							break;

						default :
							if (Command > 0 && Command < 8) {
								xil_printf("You have selected video pattern %d "
									   "from the pattern list \r\n", Command);
								done = 1;
							} else {
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
					/* MSA */
					XDpTxSs_ReportMsaInfo(&DpTxSsInst);
					break;

				/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

				case '6' :
					/* EDID */
					XDptx_DbgPrintEdid(DpTxSsInst.DpPtr);
					break;

				/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

				case '7' :
					/* Link config and status */
					XDpTxSs_ReportLinkInfo(&DpTxSsInst);
					break;

				/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

				case '8' :
					/* Display DPCD reg */
					XDpTxSs_ReportSinkCapInfo(&DpTxSsInst);
					break;

				/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

				case '9' :
					/* "9 - Read Aux registers\n\r" */
					xil_printf("\r\n Give 4 bit Hex "
						   "value of base register 0x");
					aux_reg_address = xil_gethex(4);
					xil_printf("\r\n Give msb 2 bit Hex "
						   "value of base register 0x");
					aux_reg_address |= ((xil_gethex(2) << 16) & 0xFFFFFF);
					xil_printf("\r\n Give number of registers "
						   "that you want to read (1 to 9): ");
					num_of_aux_registers = xil_gethex(1);
					if ((num_of_aux_registers < 1) ||
					    (num_of_aux_registers > 9)) {
						xil_printf("\r\n!!!Warning: Invalid number "
						  "selected, hence reading only one register\r\n");
						num_of_aux_registers = 1;
					}
					xil_printf("\r\nGiven base address offset is 0x%x\r\n",
					aux_reg_address);
					for (i = 0 ; i < num_of_aux_registers ; i++) {
						Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
								       (aux_reg_address + i),
								       1, &Data);
		
						if (Status == XST_SUCCESS) {
							xil_printf("Value at address offset "
								"0x%x, is = 0x%x\r\n",
								(aux_reg_address + i),((Data[0]) & 0xFF));
						} else {
							xil_printf("Aux Read failure\r\n");
							break;
						}
					}
					break;

				/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
				case 'd' :
					if (in_pwr_save == 0) {
						pwr_dwn = 0x2;
						XDp_TxAuxWrite(DpTxSsInst.DpPtr,
							XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &pwr_dwn);
						in_pwr_save = 1;
						xil_printf("\r\n==========power down===========\r\n");
					} else {
						pwr_dwn = 0x1;
						XDp_TxAuxWrite(DpTxSsInst.DpPtr,
							XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &pwr_dwn);
						in_pwr_save = 0;
					xil_printf("\r\n==========out of power down===========\r\n");
						hpd_con();
					}
					break;

				/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
				case 'z' :
					sub_help_menu ();
					break;

				case 'n' :
					read_DP141();
					break;

				case 'c' :
					xil_printf("==========Frame CRC===========\r\n");
					xil_printf("CRC Cfg     =  0x%x\r\n",
						   XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0x0));
					xil_printf("CRC - R/Y   =  0x%x\r\n",
						   XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0x4) & 0xFFFF);
					xil_printf("CRC - G/Cr  =  0x%x\r\n",
						   XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0x4) >> 16);
					xil_printf("CRC - B/Cb  =  0x%x\r\n",
						   XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0x8) & 0xFFFF);
					xil_printf("Txd Hactive =  0x%x\r\n",
						   XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0xC) & 0xFFFF);
					xil_printf("Txd Vactive =  0x%x\r\n",
						   XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0xC) >> 16);

					Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
							DPCD_TEST_SINK_MISC, 1, &Data);

					/* Only if sink supports CRC function, proceed. */
					/* Read CRC value from sink */
					if ((Data[0] & 0x20) != 0) {

						/* Start TEST_CRC in sink*/
						Data[0] = 0x1;
						Status = XDp_TxAuxWrite(DpTxSsInst.DpPtr,
								DPCD_TEST_SINK_START, 1, &Data);

						/*Wait till CRC is available or timeout*/
						crc_wait_cnt=0;
						while(1) {
							/* Read CRC availability every few ms*/
							Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
								DPCD_TEST_SINK_MISC, 1, &Data);
							usleep(10000);
							if ((Data[0] & 0x0F) != 0) {
								xil_printf("Sink CRC - Available...\r\n");
								break;
							} else if (crc_wait_cnt == CRC_AVAIL_TIMEOUT) {
								xil_printf("Sink CRC - Timed Out...\r\n");
								break;
							} else {
								crc_wait_cnt++;
							}
						}

						/* Wait time so that Tx and Rx has enough 
						 * time to calculate CRC values*/
						usleep(100000);

						Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
								DPCD_TEST_CRC_R_Cr, 6, &Data);

						xil_printf("**** Sink CRC Values ****\r\n");
						xil_printf("CRC - R/Cr   =  0x%x\r\n",
								Data[0] | (Data[1]<<8));
						xil_printf("CRC - G/Y    =  0x%x\r\n",
								Data[2] | (Data[3]<<8));
						xil_printf("CRC - B/Cb   =  0x%x\r\n",
								Data[4] | (Data[5]<<8));
					}

					break;

				case 'e':
					xil_printf("EDID read is :\r\n");

					XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
					for (i = 0 ; i < 128 ; i++) {
						if (i % 16 == 0 && i != 0)
							xil_printf("\r\n");
						xil_printf("%02x ", Edid_org[i]);
					}

					int j = 1;

					/* Check multiple block EDID. */
					if (Edid_org[126] >= 1) {
						for(j = 1 ; j <= Edid_org[126] ; j++){

							xil_printf("\r\r\n\n");
							/* Reading the second block of EDID. */
							XDp_TxGetEdidBlock(DpTxSsInst.DpPtr,
									Edid1_org, j);

							for (i = 0 ; i < 128 ; i++) {
								if(i % 16 == 0 && i != 0)
									xil_printf("\r\n");
								xil_printf("%02x ", Edid1_org[i]);
							}
						}
					}
					xil_printf("\r\nEDID read over =======\r\n");

					break;

				case 'b' :
					exit = 0;
					format_help_menu();
					while (exit == 0) {
						CommandKey = 0;
						Command = 0;
						CommandKey = inbyte_local();
						if (CommandKey != 0) {
							Command = (int)CommandKey;
							switch (CommandKey) {
							case 'x' :
								exit = 1;
								sub_help_menu ();
								break;

							default :
								Command = Command - 48;
								user_config.user_format = Command;
								xil_printf("You have selected %c\r\n",
									   CommandKey);

								if ((Command <= 0) || (Command > 3)) {
									format_help_menu();
									done = 0;
									break;
								} else {
									xil_printf("Setting Format of %d\r\n",
										   user_config.user_format);
									done = 1;
								}

								pat_update = 3;
								Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
										C_VideoUserStreamPattern[pat_update]);

								start_tx(LineRate_init_tx, LaneCount_init_tx,
									 user_config.VideoMode_local, 8, 1, pat_update);

								exit = done;
								break;
							}
						}
					}
					sub_help_menu ();
					break;

				case 'x' :
					XDpTxSs_Stop(&DpTxSsInst);
					app_help ();
					MainMenu = 1;
					UserInput =0;
					XIntc_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
					XIntc_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
					XGpio_WriteReg(XPAR_AV_PAT_GEN_0_BASEADDR + 0x400,
							0x0, 0x0);
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_ENABLE, 0x0);
					XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_STATUS);
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_MASK,0xFFF);
					break;

				} //end of switch (CmdKey[0])
			} //end of while (abc == 0)

			break;

		case 's' :
#if !BUFFER_BYPASS
		case 'r' :
#endif
			reconfig_clkwiz(clk_reg0, clk_reg1, clk_reg2);
			reset_clkwiz ();
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_CHANNELS, 0x0);
			Vpg_Audio_stop();
			XDpTxSs_SetCallBack(&DpTxSsInst,
					    (XDPTXSS_HANDLER_DP_SET_MSA),
					    &DpPt_TxSetMsaValuesImmediate,
					    &DpTxSsInst);
			training_done = 0;
			start_tracking = 0;
				xil_printf("\r\n**************************%c*****************"
							"************************\r\n",UserInput);
				xil_printf("In this configuration the RX acts as Master while "
							"the TX is used to\r\n");
				xil_printf("display the video that is received on RX. This "
							"mode operates on the\r\n");
				xil_printf("clock forwarded by DP159. RX uses CPLL, TX uses "
							"QPLL and they operate\r\n");
				xil_printf("on independent reference clocks\r\n");
				xil_printf("*************************************************"
							"******************\r\n");

				XDpRxSs_SetLinkRate(&DpRxSsInst, LineRate_init);
				XDpRxSs_SetLaneCount(&DpRxSsInst, LaneCount_init);
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					     XDP_RX_LINK_ENABLE, 0x0);

			MainMenu = 0;

			only_tx_active = 0;
			XDpTxSs_Stop(&DpTxSsInst);
			xil_printf("RX Link & Lane Capability is set to %x, %x\r\n",
				(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr ,
						XDP_RX_DPCD_LINK_BW_SET)),
				(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr ,
						XDP_RX_DPCD_LANE_COUNT_SET)));
			/* Disabling TX interrupts. */
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_INTERRUPT_MASK, 0xFFF);

			switch (LineRate_init) {
			case 0x6:
				Status = PHY_Configuration_Rx(&VPhy_Instance,
						PHY_User_Config_Table[3]);
				break;

			case 0xA:
				Status = PHY_Configuration_Rx(&VPhy_Instance,
						PHY_User_Config_Table[4]);
				break;

			case 0x14:
				Status = PHY_Configuration_Rx(&VPhy_Instance,
						PHY_User_Config_Table[5]);

			case 0x1E:
				Status = PHY_Configuration_Rx(&VPhy_Instance,
						PHY_User_Config_Table[10]);

				break;
			}

			if (Status != XST_SUCCESS) {
				xil_printf("+++++++ RX GT configuration "
					   "encountered a failure +++++++\r\n");
			}
			
			XIntc_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
			XIntc_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

			rx_help_menu();
			xil_printf("Please plug in RX cable "
				   "to initiate training...\r\n");
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_LINK_ENABLE, 0x1);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_AUDIO_CONTROL, 0x1);

			/* Toggle HPD once */
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_HPD_INTERRUPT, 0x0BB80001);

			rx_ran_once = 1;

			/* ************************************************
			 * Main Rx-passthrough while loop start from here *
			 * ************************************************/
			while (1) {
				if (tx_is_reconnected == 1) {
					hpd_con();
					tx_is_reconnected = 0;
				}

				if (hpd_pulse_con_event == 1) {
					hpd_pulse_con_event = 0;
					hpd_pulse_con(&DpTxSsInst);
				}


				/* Check if Rx has trained correctly or not. */
				if (training_done == 1 && rx_linkup_trig == 1) {
					xil_printf("> Training done !!! (");
					xil_printf("BW: 0x%x, ", dp_conf.LineRate);
					xil_printf("Lanes: 0x%x, ", dp_conf.LaneCount);
					xil_printf("Status: 0x%x;",
						   (unsigned int)XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
									     XDP_RX_DPCD_LANE01_STATUS));
					xil_printf("0x%x).\n\r",
						   (unsigned int)XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
									     XDP_RX_DPCD_LANE23_STATUS));
					rx_linkup_trig = 0;
				}

#if FOR_INTERNAL
				/* This is the case Rx is lost, but Tx is still connected
				 * Switching to Video Pattern Gen to display color bar.
				 * */
				switch_to_Tx_only(&track_switch, &pwr_dwn_x);
#endif

				/* Detect the FrameRate change, BPC change at there.
				 * If there is change, then reset the Tx.
				 * */
				video_change_detect(&count_track, &rxMsamisc0_track, &bpc_track,
						&recv_clk_freq_track, &recv_frame_clk_track,
						&recv_frame_clk_int_track, &track_count);

				/* Start the TX only when timer counter is done 
				 * and when training_done is still
				 * valid. Many a times bogus interrupts put the 
				 * sw to go into TX mode.
				 * Bogus interrupts typically when training is 
				 * on or when cable is being unplugged.
				 *  Once Rx linked up correctly, check for vblank.
				 *  
				 * If there is enough vblank, then check MSA value,
				 * based on MSA value, linkup the Tx side for 
				 * pass-through.
				 * */
				detect_rx_video_and_startTx(&track_count1);
#if ENABLE_AUDIO
				/* Audio pass-through setting after 1000 iterations. */
				start_audio_passThrough();

				/* Info Frame Handling
				 * Info frame is sent once per frame and
				 * is static for that config
				 * Capture new Info Frame whenever config changes
				 * */
				if (AudioinfoFrame.frame_count != 0) {
					Print_InfoPkt();
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
							XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
					AudioinfoFrame.frame_count=0;
				}

				/* Ext Frame Handling */
				if (SdpExtFrame.frame_count != SdpExtFrame.frame_count_q) {
					SdpExtFrame.frame_count_q = SdpExtFrame.frame_count;
					Print_ExtPkt();
				}
#endif

				CommandKey = xil_getc(0xff);
				Command = atoi(&CommandKey);
				if (CommandKey != 0) {
					xil_printf("%c\r\n", CommandKey);
					switch (CommandKey) {

					case '1':
						select_rx_link_lane();
						CommandKey = GetInbyte();
						Command = (int)CommandKey;
						Command = Command -48;
						xil_printf("You have selected command=%d \n\r",
							   Command);
						if ((Command >= 0) && (Command < 9)) {
							user_lane_count =
								lane_link_table[Command].lane_count;
							user_link_rate =
								lane_link_table[Command].link_rate;

							if (lane_link_table[Command].lane_count >
							    DpRxSsInst.DpPtr->Config.MaxLaneCount) {
								xil_printf("This Lane Count is not "
									   "supported by Sink \n\r");
								xil_printf("Max Supported Lane Count "
									   "is 0x%x \n\r", 
									   DpRxSsInst.DpPtr->Config.MaxLaneCount);
								xil_printf("Training at Supported "
									   "Lane count \r\n");
								user_lane_count =
									DpRxSsInst.DpPtr->Config.MaxLaneCount;
							}

							if(lane_link_table[Command].link_rate >
							   DpRxSsInst.DpPtr->Config.MaxLinkRate){

								xil_printf("This link rate is not "
									   "supported by Sink \n\r");
								xil_printf("Max Supported Link "
									   "Rate is 0x%x \n\r", 
									   DpRxSsInst.DpPtr->Config.MaxLinkRate);
								xil_printf("Training at supported Link Rate\r\n");
								user_link_rate =
									DpRxSsInst.DpPtr->Config.MaxLinkRate;
							}

							xil_printf("RX Link & Lane Capability "
								   "is set to %x, %x\r\n",
								   user_link_rate, user_lane_count);
							xil_printf("\r\n **Important: Please ensure to "
								   "unplug & plug the cable after the "
								   "capabilities have been changed **\r\n");
							XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								    XDP_TX_INTERRUPT_STATUS);

							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								     XDP_TX_INTERRUPT_MASK, 0xFFF);
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[1]);

							vdma_stop(&dma_struct);
							reconfig_clkwiz(clk_reg0, clk_reg1, clk_reg2);

							start_tracking = 0;
							change_detected = 0;
							rx_link_change_requested = 1;
							XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
									       0x7FF8FFFF);
							/* Disabling TX interrupts. */
							XDpTxSs_Stop(&DpTxSsInst);
							XDpRxSs_SetLinkRate(&DpRxSsInst, user_link_rate);
							XDpRxSs_SetLaneCount(&DpRxSsInst, user_lane_count);

							switch (user_link_rate) {
							case 0x6:
								break;

							case 0xA:
								break;

							case 0x14:
								break;
							}
							if (Status != XST_SUCCESS) {
							
							}
						} else {
							xil_printf("!!!Warning: You have selected wrong "
								   "option for lane count and link "
								   "rate =%d \n\r",Command);
							break;
						}

						break;

					case '2':
						// debug_info();
						xil_printf("==========MCDP6000 Debug Data"
							   "===========\r\n");

						xil_printf("0x0700: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR,
											0x0700));
						xil_printf("0x0704: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR,
											0x0704));
						xil_printf("0x0754: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR,
											0x0754));
						xil_printf("0x0B20: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR,
											0x0B20));
						xil_printf("0x0B24: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR,
											0x0B24));
						xil_printf("0x0B28: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR,
											0x0B28));
						xil_printf("0x0B2C: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR,
											0x0B2C));

						xil_printf("==========RX Debug Data===========\r\n");
						XDpRxSs_ReportLinkInfo(&DpRxSsInst);
						XDpRxSs_ReportMsaInfo(&DpRxSsInst);
						xil_printf("==========TX Debug Data===========\r\n");
						XDpTxSs_ReportMsaInfo(&DpTxSsInst);
						XDpTxSs_ReportLinkInfo(&DpTxSsInst);

						break;

					case '3':
						start_tracking = 0;
						change_detected = 0;

						rx_link_change_requested = 1;
						XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
						XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x80000000);

						/* Disabling TX interrupts. */
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							     XDP_TX_INTERRUPT_MASK, 0xFFF);
						XDpTxSs_Stop(&DpTxSsInst);

						XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							     XDP_RX_HPD_INTERRUPT, 0x0BB80001);
//						Dprx_InterruptHandlerVmChange(DpRxSsInst.DpPtr);
						xil_printf("\r\n- HPD Toggled for 3ms! -\n\r");
						break;

					case '4':
						xil_printf("Restarting TX...\r\n");
						if (need_to_retrain_rx == 0) {
#if !JUST_RX
							prog_tx = 1;
#endif
						} else {
							xil_printf("Monitor change was detected. "
								   "please unplug-plug DP RX cable\r\n");
						}

						break;

					case '5':
						/* Disabling TX interrupts. */
						if (need_to_retrain_rx == 0) {

							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								     XDP_TX_INTERRUPT_MASK, 0xFFF);
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[1]);
							DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);
							Status = DpTxSubsystem_Start(&DpTxSsInst, 1);
							xil_printf("Switching TX to internal "
								   "pattern generator ....\n\r");
							XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								    XDP_TX_INTERRUPT_STATUS);
							/* Enabling TX interrupts. */
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								     XDP_TX_INTERRUPT_MASK, 0x0);
							xil_printf("Stopping VDMA ....");
							vdma_stop(&dma_struct);
							xil_printf("done !\n\r");
						} else {
							xil_printf("Monitor change was detected.. "
								   "please unplug-plug DP RX cable\r\n");
						}

						break;

					case '6':
						/* Disabling TX interrupts. */
						if (need_to_retrain_rx == 0) {
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								    XDP_TX_INTERRUPT_MASK, 0xFFF);
							xil_printf("Re-starting VDMA ....");
							vdma_start(&dma_struct,
									Msa[0].Vtm.Timing.HActive,
									Msa[0].Vtm.Timing.VActive,
									dp_conf.pixel, monitor_8K,
									dp_conf.LineRate,
									Msa[0].Vtm.FrameRate);

							Dprx_StartVDMA(&dma_struct[0].AxiVdma,
								XAXIVDMA_WRITE, Msa[0].Vtm.Timing.HActive,
								Msa[0].Vtm.Timing.VActive,
								dp_conf.pixel,dma_struct);
							Dprx_StartVDMA(&dma_struct[0].AxiVdma,
								XAXIVDMA_READ, Msa[0].Vtm.Timing.HActive,
								Msa[0].Vtm.Timing.VActive,
								dp_conf.pixel,dma_struct);

							xil_printf("done !\r\n");
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[0]);
							xil_printf("\r\nSwitching TX to RX video\n\r");
							DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 800000);
							Status = DpTxSubsystem_Start(&DpTxSsInst, 1);
							XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								    XDP_TX_INTERRUPT_STATUS);
							/* Enabling TX interrupts. */
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								     XDP_TX_INTERRUPT_MASK, 0x0);
						} else {
							xil_printf("Monitor change was detected.. "
								   "please unplug-plug DP RX cable\r\n");
						}

						break;

					case '7':
						if(mcdp6000_reset == 0){
							XDpRxSs_MCDP6000_SetRegister(XPAR_IIC_0_BASEADDR,
									I2C_MCDP6000_ADDR, 0x0504, 0x700E);

							mcdp6000_reset = 1;
							training_done = 0;
							start_tracking = 0;
						} else {
							XDpRxSs_MCDP6000_DpInit(XPAR_IIC_0_BASEADDR,
									I2C_MCDP6000_ADDR);
							XDpRxSs_MCDP6000_ResetDpPath(XPAR_IIC_0_BASEADDR,
									I2C_MCDP6000_ADDR);
							mcdp6000_reset = 0;
						}
						break;

					case 'd':
						select_rx_quad();
						CommandKey = GetInbyte();
						Command = (int)CommandKey;
						Command = Command -48;
						xil_printf("You have selected command=%d \n\r",
								Command);

						if ((Command >= 0) && (Command < 4)) {
							switch (Command) {
							case 0:
								Dprx_StartVDMA_trunc(
									&dma_struct[0].AxiVdma,
									XAXIVDMA_READ,
									Msa[0].Vtm.Timing.HActive,
									Msa[0].Vtm.Timing.VActive,
									dp_conf.pixel,dma_struct, 0);
								break;
							case 1:
								Dprx_StartVDMA_trunc(
									&dma_struct[0].AxiVdma,
									XAXIVDMA_READ,
									Msa[0].Vtm.Timing.HActive,
									Msa[0].Vtm.Timing.VActive,
									dp_conf.pixel,dma_struct,
								((Msa[0].Vtm.Timing.HActive - 3840) * BPC) /
								dp_conf.pixel);
								break;
							case 2:
								Dprx_StartVDMA_trunc(
									&dma_struct[0].AxiVdma,
									XAXIVDMA_READ,
									Msa[0].Vtm.Timing.HActive,
									Msa[0].Vtm.Timing.VActive,
									dp_conf.pixel,dma_struct,
									(Msa[0].Vtm.Timing.HActive * 
										(Msa[0].Vtm.Timing.VActive - 2160))* 
										BPC / dp_conf.pixel
								);
								break;
							case 3:
								Dprx_StartVDMA_trunc(
									&dma_struct[0].AxiVdma,
									XAXIVDMA_READ,
									Msa[0].Vtm.Timing.HActive,
									Msa[0].Vtm.Timing.VActive,
									dp_conf.pixel,dma_struct,
									((Msa[0].Vtm.Timing.HActive * 
										(Msa[0].Vtm.Timing.VActive - 2160)) +
										(Msa[0].Vtm.Timing.HActive - 3840))* 
										BPC / dp_conf.pixel
								);
								break;
							}
						} else {
							xil_printf("!!!Warning: You have selected "
								   "wrong option for Quad selection "
								   "= %d \n\r", Command);
							break;
						}

						break;

					case 'w':
						dbg_printf("\n\rEnter 4 hex characters: "
							   "Sink Write address offset 0x");
						addr = xil_gethex(4);
						dbg_printf("\n\rEnter 4 hex characters: "
							   "Sink Write Data 0x");
						data = xil_gethex(4);
						XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							     addr, data);
						break;

					case 'r':
						dbg_printf("\n\rEnter 4 hex characters: "
							   "Sink Read address offset 0x");
						addr = xil_gethex(4);
						data = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
											addr);

						dbg_printf("\n\rSink Read Addr %04x "
							   "Read Data: %04x\n\r",
							   (XPAR_DPRXSS_0_BASEADDR+addr), data);
						break;

					case 's':
						xil_printf("DP RX Bandwidth is "
							   "set to = %x \r\n",
							   XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_DPCD_LINK_BW_SET));
						xil_printf("DP RX Lane Count is set to = %x \r\n",
							   XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								       XDP_RX_DPCD_LANE_COUNT_SET));
						xil_printf("[LANE0_1 Status] = %x ",
							   XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								       XDP_RX_DPCD_LANE01_STATUS));
						if (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_DPCD_LANE_COUNT_SET) > 2) {
						        xil_printf(", [LANE2_3 Status] = %x",
						    		   XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						    			       XDP_RX_DPCD_LANE23_STATUS));
						}
						dbg_printf("\r\n DP Symbol Error Registers: "
							   "%x; %x \r\n",
							   XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								       XDP_RX_DPCD_SYM_ERR_CNT01),
							   (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								        XDP_RX_DPCD_SYM_ERR_CNT23)));
						break;
#if ENABLE_AUDIO
					case 'a':
						xil_printf("Disable & Enable Audio --->\r\n");
						XDpRxSs_AudioDisable(&DpRxSsInst);
						XDpRxSs_AudioEnable(&DpRxSsInst);

//						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//								XDP_TX_AUDIO_CHANNELS, 0x1);
//						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//								XDP_TX_AUDIO_CONTROL, 0x1);
						break;

					case 'b':
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_CONTROL, 0x0);
						break;
#endif
					case 'n':
						if (edid_page == 5) {
							edid_page = 2;
						} else {
							edid_page++;
						}
						edid_change(edid_page);

						break;

					case 'c':
						xil_printf("==========Frame CRC rx===========\r\n");
						xil_printf("CRC Cfg     =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_RX_BASEADDR, 0x0));
						xil_printf("CRC - R/Y   =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_RX_BASEADDR, 0x4) & 0xFFFF);
						xil_printf("CRC - G/Cr  =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_RX_BASEADDR, 0x4) >> 16);
						xil_printf("CRC - B/Cb  =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_RX_BASEADDR, 0x8) & 0xFFFF);
						xil_printf("Rxd Hactive =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_RX_BASEADDR, 0xC) & 0xFFFF);
						xil_printf("Rxd Vactive =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_RX_BASEADDR, 0xC) >> 16);
	
						xil_printf("=========Frame CRC tx==========\r\n");
						xil_printf("CRC Cfg     =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0x0));
						xil_printf("CRC - R/Y   =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0x4) & 0xFFFF);
						xil_printf("CRC - G/Cr  =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0x4) >> 16);
						xil_printf("CRC - B/Cb  =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0x8) & 0xFFFF);
						xil_printf("Rxd Hactive =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0xC) & 0xFFFF);
						xil_printf("Rxd Vactive =  0x%x\r\n",
							XDp_ReadReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0xC) >> 16);
						break;

					case 'm':
						xil_printf("========MCDP6000 Debug Data=========\r\n");
						xil_printf("0x0700: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x0700));
						xil_printf("0x0704: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x0704));
						xil_printf("0x0754: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x0754));
						xil_printf("0x0B20: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x0B20));
						xil_printf("0x0B24: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x0B24));
						xil_printf("0x0B28: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x0B28));
						xil_printf("0x0B2C: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x0B2C));

						xil_printf("0x1294: %08x  0x12BC: %08x  0x12E4: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x1294),
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x12BC),
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x12E4));
						xil_printf("0x1394: %08x  0x13BC: %08x  0x13E4: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x1394),
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x13BC),
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x13E4));
						xil_printf("0x1494: %08x  0x14BC: %08x  0x14E4: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x1494),
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x14BC),
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x14E4));
						xil_printf("0x1594: %08x  0x15BC: %08x  0x15E4: %08x\n\r",
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x1594),
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x15BC),
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR, 0x15E4));
						break;

					case 'u':
						xil_printf("\r\n Give 4 bit Hex "
							   "value of base register 0x");
						aux_reg_address = xil_gethex(4);
						xil_printf("\r\n");
						xil_printf("0x%x: %08x\n\r", aux_reg_address,
							   XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR,
											aux_reg_address));
						break;

					case 'o':
						xil_printf("\r\n Give 4 bit Hex "
							   "value of base register 0x");
						aux_reg_address = xil_gethex(4);
						xil_printf("\r\n");
						xil_printf("\r\n Give 8 bit Hex "
							   "value of write data 0x");
						data = xil_gethex(8);
						XDpRxSs_MCDP6000_SetRegister(
							XPAR_IIC_0_BASEADDR,
							I2C_MCDP6000_ADDR,
							aux_reg_address,
							data);
						xil_printf("\r\n");

						break;

					/* EDID pass-thorugh changer. */
					case 'q':
						if (use_monitor_edid == 1) {
							/* change the mode to none-pass-through mdoe. */
							use_monitor_edid = 0;
							xil_printf("Set as EDID non-pass-through mode\r\n");
						} else {
							/* This is EDID pass-through mode. */
							use_monitor_edid = 1;
							for(i = 0 ; i < (384 *4) ; i = i + (16 * 4)) {
								for(int j = i ; j < (i + (16 * 4)) ;j = j + 4) {
									XDp_WriteReg(XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
										     j, edid_monitor[(i/4)+1]);
								}
							}
							for(i = 0 ; i < (384 * 4) ; i = i + 4){
								XDp_WriteReg(XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
									     i, edid_monitor[i/4]);
							}

							xil_printf("Set as EDID pass-thorugh mode\r\n");
						}

						break;

					case 'v':
						if (bypass_vid_common == 1) {
							/* set as vid_comm mode. */
							bypass_vid_common = 0;
							xil_printf("Set vid_common mode\r\n");
						} else {
							/* set as vid_comm bypass mode */
							bypass_vid_common = 1;
							xil_printf("Set vid_common bypass mode\r\n");
						}

						break;

					case 'z':
						rx_help_menu();
						break;

					case 'x':
						XIntc_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
						XIntc_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
						XDpTxSs_Stop(&DpTxSsInst);
						XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							     XDP_RX_LINK_ENABLE, 0x0);
						Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
								C_VideoUserStreamPattern[1]);
						vdma_stop(&dma_struct);
						MainMenu = 1;
						rx_ran_once = 0;
						UserInput =0;
						vblank_count =0;
						enabled = 0;

						sink_power_cycle(400);
						Vpg_Audio_stop();
						XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							     XDP_RX_AUDIO_CONTROL, 0x0);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							     XDP_TX_AUDIO_CONTROL, 0x0);
						app_help ();
						XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							     XDP_RX_INTERRUPT_MASK, 0xFFF87FFF);
						/* All links down */
						XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_INTERRUPT_STATUS);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_INTERRUPT_MASK, 0xFFF);
						XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_LINK_ENABLE, 0x0);

						break;
					}//end of switch

				}//end of if(CommandKey!=0)

				if (MainMenu == 1) {
					/* break out of the rx menu */
					break;
				}
			}//end of while(1)
			break;

		default :
			app_help();
			break;
		}//end of userInput
	}//end of all menus

	Xil_DCacheInvalidate ();
	Xil_DCacheDisable ();
	/* Clean up ICache */
	Xil_ICacheInvalidate ();
	Xil_ICacheDisable ();

	return 0;
}

u8 XUartLite_RecvByte_local(u32 BaseAddress)
{
	do {
		if (mst_hpd_event == 1) {
			xil_printf("mst_hpd_event is 0x%x\r\n", mst_hpd_event);
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
	/* Initialize UART */
	Status = XUartLite_Initialize(&UartLite,
			XPAR_PROCESSOR_SUBSYSTEM_INTERCONNECT_AXI_UARTLITE_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:UART failed to initialize. \r\n");
		return XST_FAILURE;
	}

	/* Initialize timer. */
	Status = XTmrCtr_Initialize(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}

	/* Set up timer options. */
	XTmrCtr_SetResetValue(&TmrCtr,
			      XPAR_TMRCTR_0_DEVICE_ID,
			      TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XPAR_TMRCTR_0_DEVICE_ID);

	/* Initialize Video PHY Controller */
	XVphy_Config *CfgPtr = 
		XVphy_LookupConfig(XPAR_VID_PHY_CONTROLLER_0_DEVICE_ID);

	XVphy_DpInitialize(&VPhy_Instance, CfgPtr, 0,
			   PHY_User_Config_Table[5].CPLLRefClkSrc,
			   PHY_User_Config_Table[5].QPLLRefClkSrc,
			   PHY_User_Config_Table[5].TxPLL,
			   PHY_User_Config_Table[5].RxPLL,
			   PHY_User_Config_Table[5].LineRate);

	prog_bb(PHY_User_Config_Table[5].LineRate, 1);

	Two_byte_set(&VPhy_Instance, SET_TX_TO_2BYTE, SET_RX_TO_2BYTE);

	XVphy_ResetGtPll(&VPhy_Instance, 0,
			 PHY_User_Config_Table[5].TxChId,
			 XVPHY_DIR_TX, (TRUE));
	XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_TX,(TRUE));

	XVphy_ResetGtPll(&VPhy_Instance, 0,
			 PHY_User_Config_Table[5].TxChId,
			 XVPHY_DIR_TX, (FALSE));
	XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_TX,(FALSE));

	XVphy_ResetGtPll(&VPhy_Instance, 0,
			 PHY_User_Config_Table[5].RxChId,
			 XVPHY_DIR_RX, (TRUE));
	XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_RX,(TRUE));

	XVphy_ResetGtPll(&VPhy_Instance, 0,
			 PHY_User_Config_Table[5].RxChId,
			 XVPHY_DIR_RX, (FALSE));
	XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_RX,(FALSE));

	PHY_Configuration_Tx(&VPhy_Instance,
			     PHY_User_Config_Table[5]);

	/* Pointer to configuration data */
	XIic_Config *ConfigPtr_IIC; 

	/* Initialize the IIC driver so that it is ready to use. */
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
	if (!dma_struct[i].Config) {
		 xil_printf("No video DMA found "
			    "for ID %d\n\r", i);
		 return 1;
	} else {
		dma_struct[i].AXIVDMA_DEVICE_ID = i;
		/* Read Base Address */
		dma_struct[i].RD_ADDR_BASE = DDR_MEMORY + i * FRAME_LENGTH;
		/* Write Base Address */
		dma_struct[i].WR_ADDR_BASE = DDR_MEMORY+ i * FRAME_LENGTH;
		dma_struct[i].BlockStartOffset = 0; //SUBFRAME_START_OFFSET;
	}

	/* Initialize DMA engine */
	Status = XAxiVdma_CfgInitialize(&dma_struct[i].AxiVdma,
					dma_struct[i].Config,
					dma_struct[i].Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		 xil_printf("VDMA Initialization "
			    "failed %d\n\r", Status);
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
		xil_printf("ERR: DPTX SS initialize failed with "
			"Status = %d!\n\r",Status);
		return (XST_FAILURE);
	}

	/* Set custom timer wait */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst,
				    &DpPt_CustomWaitUs,
				    &TmrCtr);
	XDpTxSs_SetCallBack(&DpTxSsInst,
			    (XDPTXSS_HANDLER_DP_HPD_EVENT),
			    &DpPt_HpdEventHandler,
			    &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst,
			    (XDPTXSS_HANDLER_DP_HPD_PULSE),
			    &DpPt_HpdPulseHandler,
			    &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst,
			    (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
			    &DpPt_LinkrateChgHandler,
			    &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst,
			    (XDPTXSS_HANDLER_DP_PE_VS_ADJUST),
			    &DpPt_pe_vs_adjustHandler,
			    &DpTxSsInst);

	return (XST_SUCCESS);
}

int DPRxInitialize()
{
	u32 Status;

	/* Lookup and Initialize DP Rx Subsystem */

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


	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LINK_QUAL_EVENT,
			DpRxSs_AccessLinkQualHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_ERROR_COUNTER_EVENT,
			DpRxSs_AccessErrorCounterHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,
			DpRxSs_CRCTestEventHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
			&DpRxSs_InfoPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
			&DpRxSs_ExtPacketHandler, &DpRxSsInst);

	return (XST_SUCCESS);
}

/*
 * This process takes in all the MSA values and find out resolution, BPC,
 * refresh rate. Further this sets the pixel_width based on the pixel_clock and
 * lane set. This is to ensure that it matches the values in TX driver. Else
 * video cannot be passthrough. Approximation is implemented for refresh rates.
 * Sometimes a refresh rate of 60 is detected as 59
 * and vice-versa. Approximation is done for single digit.
 * */
static void Dprx_DetectResolution(void *InstancePtr)
{
	u32 DpHres = 0;
	u32 DpVres = 0;

	u32 DpHres_total, DpVres_total;
	u32 recv_clk_freq=0;
	float recv_frame_clk=0.0;
	u32 rxMsaMVid;
	u32 rxMsaNVid;
	u32 rxMsamisc0;

	DpHres = (XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
					XDP_RX_MSA_HRES));
	DpVres = (XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
					XDP_RX_MSA_VHEIGHT));

	Msa[0].Vtm.Timing.HActive = DpHres;
	Msa[0].Vtm.Timing.VActive = DpVres;

	DpHres_total = (XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
						XDP_RX_MSA_HTOTAL));
	DpVres_total = (XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
						XDP_RX_MSA_VTOTAL));
	Msa[0].Vtm.Timing.HTotal = DpHres_total;
	Msa[0].Vtm.Timing.F0PVTotal = DpVres_total;

	rxMsaMVid = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_MVID) &
		     0x00FFFFFF);
	rxMsaNVid = (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_NVID) &
		     0x00FFFFFF);
	Msa[0].MVid = rxMsaMVid;
	Msa[0].NVid = rxMsaNVid;

#ifdef fake_4K_monitor
	/* Those are for testing to fake 8K
	 * monitor to be 4K limiteed monitor
	 * if real 4K monitor is connected,
	 * no need to set them
	 * */
	max_cap_org = 0x14;
	monitor_8K = 0;
#endif

	/* This part re-calculate M/N vid value
	 * in case sink is not DP1.4 capable.
	 * If Rx(source) side is 8.1Gbps, but
	 * monitor(sink) is not 8.1Gbps capable,
	 * */
	if (dp_conf.LineRate == 0x1E && monitor_8K == 0) {
		/* Calculate pixel frequency */
		double freq_d = (810000.0 / (double)rxMsaNVid) * (double)rxMsaMVid;
		u32 pixel_freq = (u32)freq_d;

		/* ceil function at here round up */
		u32 mod = pixel_freq % 1000;
		if(mod != 0)
			pixel_freq += 1000;
		pixel_freq -= mod;

		/* update MSA values at here */
		Msa[0].MVid = pixel_freq;
		Msa[0].NVid = 27 * 1000 * 0x14;
		Msa[0].PixelClockHz = pixel_freq;
	}

	Msa[0].HStart = XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
					XDP_RX_MSA_HSTART);
	Msa[0].VStart = XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
					XDP_RX_MSA_VSTART);

	Msa[0].Vtm.Timing.HSyncWidth =
			XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
				    XDP_RX_MSA_HSWIDTH);
	Msa[0].Vtm.Timing.F0PVSyncWidth =
			XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
				    XDP_RX_MSA_VSWIDTH);

	Msa[0].Vtm.Timing.HSyncPolarity =
			XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
				    XDP_RX_MSA_HSPOL);
	Msa[0].Vtm.Timing.VSyncPolarity =
			XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
				    XDP_RX_MSA_VSPOL);

	recv_clk_freq = ((dp_conf.LineRate * 27.0) * rxMsaMVid) / rxMsaNVid;
	recv_frame_clk = ceil((recv_clk_freq*1000000.0) /
			      (DpHres_total * DpVres_total));
	//recv_frame_clk_int = recv_frame_clk;
	//Msa[0].Vtm.FrameRate = recv_frame_clk;
	u32 recv_frame_clk_int = recv_frame_clk;
	//Doing Approximation here
	if (recv_frame_clk_int == 59 || recv_frame_clk_int == 61) {
		recv_frame_clk_int = 60;
	} else if (recv_frame_clk_int == 29 || recv_frame_clk_int == 31) {
		recv_frame_clk_int = 30;
	} else if (recv_frame_clk_int == 76 || recv_frame_clk_int == 74) {
		recv_frame_clk_int = 75;
	} else if (recv_frame_clk_int == 121 || recv_frame_clk_int == 119) {
		recv_frame_clk_int = 120;
	}
	Msa[0].Vtm.FrameRate = recv_frame_clk_int; // update framerate

	rxMsamisc0 = ((XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_MSA_MISC0) >> 5) & 0x00000007);

	if ((recv_clk_freq * 1000000) > 540000000 && dp_conf.LaneCount == 4) {
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x04);
		dp_conf.pixel = 0x4;
		Msa[0].UserPixelWidth = 0x4;
	} else if ((recv_clk_freq * 1000000) > 270000000 && dp_conf.LaneCount != 1) {
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x02);
		dp_conf.pixel = 0x2;
		Msa[0].UserPixelWidth = 0x2;
	} else {
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x01);
		dp_conf.pixel = 0x1;
		Msa[0].UserPixelWidth = 0x1;
	}

	Msa[0].SynchronousClockMode = rxMsamisc0 & 1;
	dp_conf.bpc = Bpc[rxMsamisc0];
	Msa[0].BitsPerColor = dp_conf.bpc;
	Msa[0].Misc0 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				   XDP_RX_MSA_MISC0); //rxMsamisc0;
	Msa[0].Misc1 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				   XDP_RX_MSA_MISC1);

	XDp_RxSetLineReset(DpRxSsInst.DpPtr, 1);

	if (training_done == 1) {
		xil_printf("\r\n *** Detected resolution: %lu x "
			   "%lu @ %luHz, BPC = %lu***\n\r",
			   DpHres, DpVres, recv_frame_clk_int,
			   dp_conf.bpc);
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
	u32 Status = 0;

	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive =
			Msa[0].Vtm.Timing.HActive;
	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive =
			Msa[0].Vtm.Timing.VActive;

	/* Get the Video Mode Id depending on the frame rate */
	VmId = XVidC_GetVideoModeId(
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive,
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive,
			Msa[0].Vtm.FrameRate, 0);
//			recv_frame_clk_int,0); //XVIDC_FR_60HZ,0);

	if (bypass_vid_common) {
		xil_printf("Using the RX MSA values to "
			   "generate TX video timings\n\r");
		tx_with_msa = 1;
	} else {
		if ((XVIDC_VM_NOT_SUPPORTED == VmId)) {
			xil_printf("This resolution is not supported "
				   "in Video Library...using MSA values\n\r");
			tx_with_msa = 1;
		} else {
			xil_printf("This resolution is supported "
				   "in Video Library.\r\n");
			tx_with_msa = 0;
		}
	}

	/* Tx should be using QPLL only. */
//	if (is_TX_CPLL == 0) {
//		xil_printf("TX is running on QPLL.........\r\n");
		switch(dp_conf.LineRate)
		{
		case XDP_DPCD_MAX_LINK_RATE_162GBPS:
			prog_bb(XDP_DPCD_MAX_LINK_RATE_162GBPS,1);
			Status = PHY_Configuration_Tx(&VPhy_Instance,
					PHY_User_Config_Table[3]);
			break;

		case XDP_DPCD_MAX_LINK_RATE_270GBPS:
			prog_bb(XDP_DPCD_MAX_LINK_RATE_270GBPS,1);
			Status = PHY_Configuration_Tx(&VPhy_Instance,
					PHY_User_Config_Table[4]);
			break;

		case XDP_DPCD_MAX_LINK_RATE_540GBPS:
			prog_bb(XDP_DPCD_MAX_LINK_RATE_540GBPS,1);
			Status = PHY_Configuration_Tx(&VPhy_Instance,
					PHY_User_Config_Table[5]);
			break;

		case XDP_DPCD_MAX_LINK_RATE_810GBPS:
			/* if sink doesn't support 8.1Gbps, then just go as 5.4G */
			if (max_cap_org == XDP_DPCD_MAX_LINK_RATE_540GBPS) {
				xil_printf("Downshifting Tx LineRate to be 5.4Gbps\r\n");
				prog_bb(XDP_DPCD_MAX_LINK_RATE_540GBPS,1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
						PHY_User_Config_Table[5]);
				XDp_TxSetLinkRate(DpTxSsInst.DpPtr,
						XDP_DPCD_MAX_LINK_RATE_540GBPS);
				DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate =
						XDP_DPCD_MAX_LINK_RATE_540GBPS;
			} else { /* only use TX 8.1Gbps if the sink supports */
				prog_bb(XDP_DPCD_MAX_LINK_RATE_810GBPS,1);
				Status = PHY_Configuration_Tx(&VPhy_Instance,
						PHY_User_Config_Table[10]);
			}
			break;

		}
//	} else {
//		xil_printf("TX is running on CPLL.........\r\n");
//		XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_TX,(FALSE));
//		prog_bb(dp_conf.LineRate,0);
//		DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 800000);
//		Status = PHY_tx_reconfig(&VPhy_Instance,
//					 XVPHY_CHANNEL_ID_CHA,
//					 XVPHY_DIR_TX, 0);
//	}
	if (Status != XST_SUCCESS) {
		xil_printf("+++++++ TX GT configuration encountered a "
			   "failure (TX PT) +++++++\r\n");
	}

	xil_printf("max_cap_org:%x  monitor_8K:%x\r\n",
		   max_cap_org, monitor_8K);

	/* This block is to use with 4K60 monitor. */
	if (max_cap_org <= 0x14 || monitor_8K == 0) {
		/* 8K will be changed to 4K60 */
		if (DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive >= 7680 &&
		    DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive >= 4320) {
			xil_printf("\nforcing Tx to use 4K60\r\n");

			/* to keep 4Byte mode, it has to be 4K60 */
			VmId = XVIDC_VM_3840x2160_60_P;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive /= 2;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive /= 2;
			DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 =
					DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 & 0xFE;
			tx_with_msa = 0;
		} else if ((Msa[0].Vtm.FrameRate *
			    Msa[0].Vtm.Timing.HActive *
			    Msa[0].Vtm.Timing.VActive) >
			   (4096 * 2160 *60)) {
			/* 4K120 will be changed to 4K60 */
			xil_printf("\nforcing Tx to use 4K60\r\n");
			/* to keep 4Byte mode, it has to be 4K60 */
			VmId = XVIDC_VM_3840x2160_60_P;
			Msa[0].Vtm.FrameRate = 60;
			DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 =
					DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 & 0xFE;
			tx_with_msa = 0;
		}
	} else {
		tx_with_msa = 1;
	}

	Dprx_SetupTx(DpTxSsInst.DpPtr, tx_with_msa, VmId);
}

void Dprx_SetupTx(void *InstancePtr, u8 tx_with_msa, XVidC_VideoMode VmId)
{
	u32 Status;
	u32 rxMsamisc0;
	
	/* Disabling TX and TX interrupts */
	sink_power_cycle(400);
	/* Disabling TX and RX interrupts */
//	XDpTxSs_Reset(&DpTxSsInst);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_ENABLE, 0x0);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 100000);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_ENABLE, 0x1);

	xil_printf("Starting DP Tx .");
	/* 
	 * Following delay is a MUST as some monitors go into sleep or power
	 * down when line rate is switched Adding delay allows the monitor to
	 * recover from this stage
	 * */
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);

	/* Checking the sink line capability.
	 * if both soruce/sink support DP1.4, go as DP1.4 mode
	 * */
	if(max_cap_org < dp_conf.LineRate) {
		Status = XDpTxSs_SetLinkRate(&DpTxSsInst,
					XDP_DPCD_MAX_LINK_RATE_540GBPS);
	} else {
		Status = XDpTxSs_SetLinkRate(&DpTxSsInst, dp_conf.LineRate);
	}

	if (Status != XST_SUCCESS) {
		xil_printf("TX SetLink failure\r\n");
	}

	xil_printf(".");
	Status = XDpTxSs_SetLaneCount(&DpTxSsInst, dp_conf.LaneCount);
	if (Status != XST_SUCCESS) {
		xil_printf("TX SetLane failure\r\n");
	}
	xil_printf(".");

	rxMsamisc0 = ((XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		       XDP_RX_MSA_MISC0) >> 5)
		      & 0x00000007);
//	xil_printf("BPC : %d\r\n", Bpc[rxMsamisc0]);

	if (tx_with_msa == 0) {
		Status = XDpTxSs_SetBpc(&DpTxSsInst,Bpc[rxMsamisc0]);
		if (Status != XST_SUCCESS) {
			xil_printf("TX SetBPC failure\r\n");
		}
		xil_printf(".");
	}
	Status = XDpTxSs_GetRxCapabilities(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("TX Could not get sink Capabilities\r\n");
	}
	xil_printf(".");
	if (tx_with_msa == 0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, VmId);
		if (Status != XST_SUCCESS) {
				xil_printf("TX Set Vid Mode failure\r\n");
		}
		xil_printf(".");
	}
	XDpTxSs_SetHasRedriverInPath(&DpTxSsInst, 0);
	xil_printf(".");

	/* Setting Color Format
	 * User can change coefficients here - By default 601 is used for YCbCr
	 * */
	XDp_TxCfgSetColorEncode(DpTxSsInst.DpPtr, XDP_TX_STREAM_ID1, \
			(user_config.user_format-1), XVIDC_BT_601, XDP_DR_CEA);

	Status = DpTxSubsystem_Start(&DpTxSsInst, tx_with_msa);
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 0);
	clk_wiz_locked();
	xil_printf("\r\n");
	/* VDMA may not work for odd resolutions 
	 * which are not divisible by 4
	 * */
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[1]);

	/* Need to start again as VTC values are reset */
	xil_printf(".");
	clk_wiz_locked();

	Status = DpTxSubsystem_Start(&DpTxSsInst, tx_with_msa);
	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		sink_power_cycle(40000);

		DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 2000000);
		Status = XDpTxSs_SetLinkRate(&DpTxSsInst, dp_conf.LineRate);
		if (Status != XST_SUCCESS) {
			xil_printf("TX SetLink failure\r\n");
		}
		Status = XDpTxSs_SetLaneCount(&DpTxSsInst, dp_conf.LaneCount);
		if (Status != XST_SUCCESS) {
			xil_printf("TX SetLane failure\r\n");
		}

		if (tx_with_msa == 0) {
			Status = XDpTxSs_SetBpc(&DpTxSsInst, Bpc[rxMsamisc0]);
			if (Status != XST_SUCCESS) {
				xil_printf("TX SetBPC failure\r\n");
			}
		}
		Status = XDpTxSs_GetRxCapabilities(&DpTxSsInst);
		if (Status != XST_SUCCESS) {
			xil_printf("TX Could not get sink Capabilities\r\n");
		}

		if (tx_with_msa == 0) {
			Status = XDpTxSs_SetVidMode(&DpTxSsInst, VmId);
			if (Status != XST_SUCCESS) {
				xil_printf("TX Set Vid Mode failure\r\n");
			}
			xil_printf(".");
		}
		XDpTxSs_SetHasRedriverInPath(&DpTxSsInst, 0);
		xil_printf(".");
		Status = DpTxSubsystem_Start(&DpTxSsInst, tx_with_msa);
		Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
		if (Status != XST_SUCCESS) {
			xil_printf("TX training failed even "
				   "after 2 attempts\r\n");
		}
	}

	/* clear interrupt before enabling again */
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		    XDP_TX_INTERRUPT_STATUS);
	/* Enabling TX interrupts */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_INTERRUPT_MASK, 0x0);

	/* Update CRC block */
	XDp_WriteReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0,
		     XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				 XDP_TX_USER_PIXEL_WIDTH));
	XDp_WriteReg(XPAR_VIDEO_FRAME_CRC_RX_BASEADDR, 0,
		     XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				 XDP_TX_USER_PIXEL_WIDTH));


	if ((Status == XST_SUCCESS)) {
		/* as of VDMA interface, it is always 4 pixel per CLK
		 * remaper will convert to 4ppc always.
		 * */
		vdma_start(&dma_struct, Msa[0].Vtm.Timing.HActive,
			   Msa[0].Vtm.Timing.VActive, 4,
			   monitor_8K, dp_conf.LineRate,
			   Msa[0].Vtm.FrameRate);

		resetIp();
		remap_start(dma_struct);

		Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
				C_VideoUserStreamPattern[0]);
		xil_printf("done !\r\n");
	}
}

static void Dprx_InterruptHandlerVmChange(void *InstancePtr)
{
	if (vblank_count >= 50 && training_done == 1) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "*** Interrupt > Video "
						"Mode change ***\n\r");
		/* Disabling TX interrupts */
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			     XDP_TX_INTERRUPT_MASK, 0xFFF);
		XDpTxSs_Stop(&DpTxSsInst);
		Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
				C_VideoUserStreamPattern[1]);
		XDp_RxDtgDis(DpRxSsInst.DpPtr);

#if !JUST_RX
		prog_tx = 1;
#endif
		start_tracking = 0;
		change_detected = 0;
	}
}

static void Dprx_InterruptHandlerNoVideo(void *InstancePtr) {
//	xil_printf("*** No Video detected ***\n\r");
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_VIDEO_UNSUPPORTED, 1);

	AudioinfoFrame.frame_count=0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
}

static void Dprx_InterruptHandlerVBlank(void *InstancePtr) {
	if (DpRxSsInst.VBlankEnable == 1) {
		vblank_count++;
		if ((vblank_count % 20) == 0 && (vblank_count > 10)) {
			xil_printf("*");
		}
		if (vblank_count >= 50) {
			DpRxSsInst.VBlankEnable = 0;
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
#if !JUST_RX
			prog_tx = 1;
			rx_ran_once = 1;
#endif
		/* when Vblank is received, HDCP is put in enabled 
		 * state and the timer is started TX is not setup until 
		 * the timer is done. This ensures that certain sources
		 * like MacBook gets time to Authenticate.
		 * */
		} else if (vblank_count == 20) {
			XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x80000000);
		}
	}
}

static void Dprx_InterruptHandlerVideo(void *InstancePtr) {
//	xil_printf("*** Interrupt > Video detected ***\n\r");
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_VIDEO_UNSUPPORTED , 0);
}//End of Dprx_InterruptHandlerVideo()


//this is a handler in TP1
void Dprx_InterruptHandlerLinkBW(void *InstancePtr)
{
	u32 Status;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_INTERRUPT_MASK, 0xFFF);
	only_tx_active = 0;
	gt_stable = 0;

	PLLRefClkSel (&VPhy_Instance, DpRxSsInst.UsrOpt.LinkRate);
	switch (DpRxSsInst.UsrOpt.LinkRate) {
	case XDP_DPCD_MAX_LINK_RATE_162GBPS:
//		if(is_TX_CPLL) {
//			XVphy_PllInitialize(
//			&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
//			ONBOARD_REF_CLK, DP159_FORWARDED_CLK,
//			XVPHY_PLL_TYPE_CPLL, XVPHY_PLL_TYPE_CPLL);
//		} else {
			XVphy_PllInitialize(
			&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
			ONBOARD_REF_CLK, DP159_FORWARDED_CLK,
			XVPHY_PLL_TYPE_QPLL1, XVPHY_PLL_TYPE_CPLL);
//		}
		break;

	default:
//		if(is_TX_CPLL) {
			XVphy_PllInitialize(
			&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
			ONBOARD_REF_CLK, DP159_FORWARDED_CLK,
			XVPHY_PLL_TYPE_CPLL, XVPHY_PLL_TYPE_CPLL);
//		} else {
			XVphy_PllInitialize(
			&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
			ONBOARD_REF_CLK, DP159_FORWARDED_CLK,
			XVPHY_PLL_TYPE_QPLL1, XVPHY_PLL_TYPE_CPLL);
//		}
		break;
	}

	Status = XVphy_ClkInitialize(&VPhy_Instance, 0,
			 XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);

	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "+++++++ RX GT configuration "
						"encountered an error "
						"(TP1) +++++++\r\n");
	}
}

void Dprx_InterruptHandlerPllReset(void *InstancePtr)
{
	u32 Status1;
	u32 Status2;
//	if (is_TX_CPLL) {
//		XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_TX,(TRUE));
//		XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
//				 XVPHY_DIR_TX, (TRUE));
//		XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
//				 XVPHY_DIR_TX, (FALSE));
//		XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_TX,(FALSE));
//	}


	XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
			 XVPHY_DIR_RX, (TRUE));
	XVphy_ResetGtPll(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
			 XVPHY_DIR_RX, (FALSE));
	XVphy_BufgGtReset(&VPhy_Instance, XVPHY_DIR_RX,(FALSE));

	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x0007FFFF);

	Status2 = XVphy_WaitForResetDone(&VPhy_Instance, 0,
					XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);
	Status1 = XVphy_WaitForPllLock(&VPhy_Instance, 0,
					XVPHY_CHANNEL_ID_CHA);

	if (Status1 != XST_SUCCESS || Status2 != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "%lu %lu\r\n", Status1, Status2);
	}
}

void Dprx_InterruptHandlerTrainingDone(void *InstancePtr) {
	dp_conf.LaneCount =
		XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
			    XDP_RX_DPCD_LANE_COUNT_SET);

	dp_conf.LineRate = DpRxSsInst.UsrOpt.LinkRate;

	training_done = 1;
	rx_linkup_trig = 1;

	xdbg_printf(XDBG_DEBUG_GENERAL, "> Interrupt: Training done !!! "
		    "(BW: 0x%x, Lanes: 0x%x, Status: "
		    "0x%x;0x%x).\n\r", dp_conf.LineRate, dp_conf.LaneCount,
		    XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
				XDP_RX_DPCD_LANE01_STATUS),
		    XDp_ReadReg(DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
				XDP_RX_DPCD_LANE23_STATUS));

	training_done = 1;
	switch_to_patgen = 0;
	only_tx_active = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_AUDIO_CONTROL, 0x0);

	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_SET_MSA),
			&DpPt_TxSetMsaValuesImmediate, &DpTxSsInst);
}

static void Dprx_InterruptHandlerTrainingLost(void *InstancePtr) {
	training_done = 0;
	vblank_count =0;
	dp_conf.pixel = 0;
	prog_tx = 0;
	start_tracking = 0;
	change_detected = 0;
	rx_linkup_trig = 0;

	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_AUDIO_CONTROL, 0x0);

	/* Disabling TX interrupts */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_INTERRUPT_MASK, 0xFFF);
	XDpTxSs_Stop(&DpTxSsInst);
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
				 C_VideoUserStreamPattern[1]);
	vdma_stop(&dma_struct);
	xdbg_printf(XDBG_DEBUG_GENERAL,"> Interrupt: Training lost !\n\r");

	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x80000000);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
}

void Dprx_InterruptHandlerBwChange(void *InstancePtr)
{
//	xil_printf("> Interrupt: Bandwidth Change !\n\r");
}

void Dprx_InterruptHandlerUplug(void *InstancePtr)
{

	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF87FFF);
	rx_linkup_trig = 0;

	xdbg_printf(XDBG_DEBUG_GENERAL,"> Interrupt: Cable unplugged !\n\r");
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_INTERRUPT_MASK, 0xFFF);
	XDpTxSs_Stop(&DpTxSsInst);
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
				 C_VideoUserStreamPattern[1]);
	vdma_stop(&dma_struct);
	training_done = 0;
	vblank_count =0;
	prog_tx = 0;

	/* Disabling TX and TX interrupts */
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_AUDIO_CONTROL, 0x0);
	enabled = 0;
	start_tracking = 0;
	change_detected = 0;
	gt_stable = 1;
//	if (is_TX_CPLL == 1 && dp_conf.LineRate != 0x6) {
//		XVphy_PllInitialize(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CHA,
//				ONBOARD_REF_CLK, ONBOARD_REF_CLK,
//				XVPHY_PLL_TYPE_CPLL, XVPHY_PLL_TYPE_CPLL);
//	}

	/* When cable is unplugged, the system 
	 * should switch to TX only mode
	 * */
#if FOR_INTERNAL
	if (rx_link_change_requested == 0 && rx_ran_once == 1 &&
	    need_to_retrain_rx == 0) {
		xdbg_printf(XDBG_DEBUG_GENERAL, ">>> !!!!!!!!! RX cable "
						"unplugged. RX Video & "
						"REFCLK1 is lost !!!\n\r");
//		if (is_TX_CPLL == 0) {
			xdbg_printf(XDBG_DEBUG_GENERAL, ">>> !!!!!!!!! "
					"Displaying the default 800x600 "
					"color bar pattern !!!\n\r");
			switch_to_patgen = 1;
//		} else {
//			xdbg_printf(XDBG_DEBUG_GENERAL, ">>> !!!!!!!!! "
//					"Switching over the CPLL to "
//					"REFCLK0  !!!!!!!!!\n\r");
//			xdbg_printf(XDBG_DEBUG_GENERAL, ">>> !!!!!!!!! "
//					"Displaying the default 800x600 "
//					"color bar pattern  !!!!!!!\n\r");
//
//		switch_to_patgen = 1;
//		}
		only_tx_active = 0;
	} else {
		rx_link_change_requested = 0;
	}
	need_to_retrain_rx = 0;


	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	rx_ran_once = 0;
#endif
}

void Dprx_InterruptHandlerPwr(void *InstancePtr)
{
	XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DPCD_SET_POWER_STATE);
}

void Dprx_InterruptHandlerInfoPkt(void *InstancePtr)
{

}
void Dprx_InterruptHandlerExtPkt(void *InstancePtr){

}

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
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_HTOTAL + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_MSA_HTOTAL));

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_VTOTAL + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_MSA_VTOTAL));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_POLARITY + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_HSPOL) |
		     (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				  XDP_RX_MSA_VSPOL) <<
		      XDP_TX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_HSWIDTH + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_MSA_HSWIDTH));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_VSWIDTH + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_MSA_VSWIDTH));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_HRES + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_HRES));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_VRES + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_VHEIGHT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_HSTART + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_HSTART));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_VSTART + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_VSTART));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_MISC0 + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_MISC0));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_MAIN_STREAM_MISC1 + StreamOffsetAddr[0],
		     XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				 XDP_RX_MSA_MISC1));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_USER_PIXEL_WIDTH + StreamOffsetAddr[0],
			 dp_conf.pixel);
	/* Check for YUV422, BPP has to be set 
	 * using component value to 2 */
	if(((XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			 XDP_RX_MSA_MISC0))
	   & 0x6) == 0x2) {
		/* YUV422 */
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x1;
		Msa[0].ComponentFormat = 0x1;
	} else {
		/* RGB or YUV444 */
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;
		Msa[0].ComponentFormat = 0x0;
	}
}

/*****************************************************************************/
/**
*
* This function is sets up Interrupt system and start it
*
* @return	None.
*
******************************************************************************/
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
				(XInterruptHandler)XDpTxSs_DpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Hook up Rx interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
				(XInterruptHandler)XDpRxSs_DpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Hook up Rx interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_TIMER_0,
				(XInterruptHandler)XTmrCtr_InterruptHandler,
				&TmrCtr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Timer interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstPtr, XINTC_IIC_ID,
				(XInterruptHandler) XIic_InterruptHandler,
				   &IicInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: IIC interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	XIntc_Enable(IntcInstPtr, XINTC_IIC_ID);

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
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				IntcInstPtr);

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
	u8 Edid2[128];

	int i = 0;
	u8 max_cap_new;
	u8 max_cap_lanes_new;

//	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 20000);
	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x1, 1, &max_cap_new);
	max_cap_org = max_cap_new;
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2, 1, &max_cap_lanes_new);
	max_cap_lanes = max_cap_lanes_new;
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nCould not read sink capabilities\r\n");
	}

#if CAP_OVER_RIDE == 1
	max_cap_new = MAX_RATE;
	max_cap_lanes_new = MAX_LANE;
#endif

	/* reading the first block of EDID */
	Status |= XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid, 0);
	for (i = 0 ; i < 128 ; i++) {
		if (Edid_org[i] != Edid[i]) {
			need_to_retrain_rx = 1;
		}
		Edid_org[i] = Edid[i];
	}

	/* reading the subsequent blocks of EDID */
	if (Edid[126] > 0){
		Status |= XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1, 1);
		for (i = 0 ; i < 128 ; i++) {
			if (Edid1_org[i] != Edid1[i]) {
				need_to_retrain_rx = 1;
			}
			Edid1_org[i] = Edid1[i];
		}
	}
	if (Edid[126] >= 2){
		Status |= XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid2, 2);
		for (i = 0 ; i < 128 ; i++) {
			if (Edid2_org[i] != Edid2[i]) {
				need_to_retrain_rx = 1;
			}
			Edid2_org[i] = Edid2[i];
		}
	}


	update_edid();
	if(max_cap_new == XDP_DPCD_LINK_BW_SET_810GBPS)
		monitor_8K = 1;
	else
		monitor_8K = 0;

	if (Status != XST_SUCCESS) {
		xil_printf("\r\nCould not read sink EDID\r\n");
	}

	if (need_to_retrain_rx == 0) {
		if (only_tx_active == 0) {
			Dprx_CheckSetupTx(DpRxSsInst.DpPtr);
		} else {
			XDpTxSs_SetLinkRate(&DpTxSsInst,
			DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate);
			xil_printf(".");
			XDpTxSs_SetLaneCount(&DpTxSsInst,
			DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount);
			Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
			if (Status != XST_SUCCESS) {
				xil_printf("! Link re-training failed.\n\r");
			}
		}
	}


	if (need_to_retrain_rx == 1) {
		if (only_tx_active == 0 && need_to_retrain_rx == 1) {
			xil_printf("The Monitor has been changed....\r\n");
			if(use_monitor_edid)
				xil_printf("The EDID contents in RX have been updated..\r\n");


			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0);
			if ((max_cap_new != max_cap_org) || ((max_cap_lanes&0x1F)
			!= (max_cap_lanes_new&0x1F))) {
				max_cap_org = max_cap_new;
				max_cap_lanes = max_cap_lanes_new&0x1F;
				XDpRxSs_SetLinkRate(&DpRxSsInst, max_cap_org);
				XDpRxSs_SetLaneCount(&DpRxSsInst, max_cap_lanes);
				xil_printf("DP RX capability has been updated to: Linerate %x, "
				"LaneCount %x\r\n",max_cap_org,max_cap_lanes);
			}
			xil_printf("**** Issuing HPD *****\r\n");
			start_tracking = 0;
			Dprx_InterruptHandlerUplug(DpRxSsInst.DpPtr);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_LINK_ENABLE, 0x1);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_HPD_INTERRUPT,0x0BB80001);
		} else {
			if (need_to_retrain_rx == 1) {
			xil_printf("The Monitor has been changed. To avoid unpredictable "
				"behavior, please change the Link/Lane and retrain\r\n");
			}
		}
	}
}

void DpPt_HpdEventHandler(void *InstancePtr)
{

	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n+===> HPD Connected.\n\r");
		sink_power_cycle(1);

		tx_is_reconnected = 1;

// This part has added to give HDCP a proper handle when hdp even happens
// HDCP block will disable Tx side encryption when hpd detected

	} else {
		xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n+===> HPD Disconnected.\n\r");
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_AUDIO_CONTROL, 0x0);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_CONTROL, 0x0);
		//stop the VDMA only if in passthrough mode
		if (((only_tx_active == 0) && (training_done == 1))) {
			vdma_stop(&dma_struct);
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_CONTROL, 0x0);
		}
		tx_is_reconnected = 0;
		//on HPD d/c, it is important to bring down the HDCP
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
	u8 rData;

	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x202, 1, &lane0_sts);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x203, 1, &lane2_sts);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x204, 1, &rd_204);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x101, 1, &lane_set);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x100, 1, &bw_set);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			"Failed to read AUX registers on HPD pulse\r\n");
	}
	bw_set = bw_set & 0x1F;
	lane_set = lane_set & 0x1F;
	rd_204 = rd_204 & 0x1;
	lane0_sts = lane0_sts & 0x55;
	lane2_sts = lane2_sts & 0x55;

	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1, &rData);
	if (rData & 0x80) { // if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
		XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData); // read maxLineRate
		if(rData == 0x1E){
			monitor_8K = 1;
			max_cap_org = 0x1E;
		}
	}

	/* Check if CR, symbol and alignment is lost
	 * re-train if required
	 * */
	//xil_printf("is rx trained %x\r\n",IsRxTrained);
	if (need_to_retrain_rx == 0) {
		if (lane_set == 0x4) {
			if ((lane0_sts != 0x55) || (lane2_sts != 0x55) || (rd_204 != 1)) {
				XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
				XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
				if ((only_tx_active == 0) && (training_done == 1)) {
					Status = DpTxSubsystem_Start(&DpTxSsInst, 1);
					xdbg_printf (XDBG_DEBUG_GENERAL,"Retraining PT 4..\r\n");
				} else if (only_tx_active == 1) {
					Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
					xdbg_printf (XDBG_DEBUG_GENERAL,"Retraining 4..\r\n");
				}
			}
		} else if (lane_set == 0x2) {
			if ((lane0_sts != 0x55) || (rd_204 != 1)) {
				XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
				XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
				if ((only_tx_active == 0) && (training_done == 1)) {
					Status = DpTxSubsystem_Start(&DpTxSsInst, 1);
					xdbg_printf (XDBG_DEBUG_GENERAL,"Retraining PT 2..\r\n");
				} else if (only_tx_active == 1) {
					Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
					xdbg_printf (XDBG_DEBUG_GENERAL,"Retraining 2..\r\n");
				}
			}

		} else if (lane_set == 0x1) {
				if ((lane0_sts != 0x5) || (rd_204 != 1)) {
					XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
					XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
					if ((only_tx_active == 0) && (training_done == 1)) {
						Status = DpTxSubsystem_Start(&DpTxSsInst, 1);
						xdbg_printf (XDBG_DEBUG_GENERAL,"Retraining PT 1.\r\n");
					} else if (only_tx_active == 1) {
						Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
						xdbg_printf (XDBG_DEBUG_GENERAL,"Retraining 1..\r\n");
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
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);
	// enable the interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);
}

void DpPt_HpdPulseHandler(void *InstancePtr)
{
	xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nHPD Pulse event detected\n\r");
	/* Some monitors give HPD pulse repeatedly which causes
	 * HPD pulse function to be executed huge number of time.
	 * Hence hpd_pulse interrupt is disabled and then
	 * enabled when hpd_pulse function is executed
	 * */
	if ((only_tx_active == 1) ||
	    ((only_tx_active == 0) && (training_done == 1))) {
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_INTERRUPT_MASK,
				XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);
		hpd_pulse_con_event = 1;
	}
}


void DpPt_LinkrateChgHandler(void *InstancePtr)
{
}

/* If TX is unable to train at what it has been asked then
 * necessary down shift handling has to be done here
 * eg. reconfigure GT to new rate etc
 * This XAPP assumes that RX and TX would run at same rate
 * */
void DpPt_pe_vs_adjustHandler(void *InstancePtr)
{
	if (PE_VS_ADJUST == 1) {
		unsigned char preemp = 0;
		switch(DpTxSsInst.DpPtr->TxInstance.LinkConfig.PeLevel){
			case 0: preemp = XVPHY_GTHE3_PREEMP_DP_L0; break;
			case 1: preemp = XVPHY_GTHE3_PREEMP_DP_L1; break;
			case 2: preemp = XVPHY_GTHE3_PREEMP_DP_L2; break;
			case 3: preemp = XVPHY_GTHE3_PREEMP_DP_L3; break;
		}

		XVphy_SetTxPreEmphasis(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH1, preemp);
		XVphy_SetTxPreEmphasis(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH2, preemp);
		XVphy_SetTxPreEmphasis(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH3, preemp);
		XVphy_SetTxPreEmphasis(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH4, preemp);


		unsigned char diff_swing = 0;
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
		XVphy_SetTxVoltageSwing(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH1,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH2,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH3,
				diff_swing);
		XVphy_SetTxVoltageSwing(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH4,
				diff_swing);

	}

	if(DP141_ADJUST == 1){
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

/*****************************************************************************/
/**
 * sets the default drp values into the mmcm
 * this ensures that mmcm is back to original state
 *
 * @return None
 *
 *****************************************************************************/
void reconfig_clkwiz (u32 clk_reg0, u32 clk_reg1, u32 clk_reg2)
{
	Xil_Out32 (CLK_WIZ_BASE+0x200, clk_reg0);
	Xil_Out32 (CLK_WIZ_BASE+0x204, clk_reg1);
	Xil_Out32 (CLK_WIZ_BASE+0x208, clk_reg2);
	Xil_Out32 (CLK_WIZ_BASE+0x25C, 0x7);
	DpPt_CustomWaitUs(DpRxSsInst.DpPtr, 200000);
	Xil_Out32 (CLK_WIZ_BASE+0x25C, 0x2);
}

/*****************************************************************************/
/**
 * This function will reset Tx side MMCM
 *
 * @return None
 *
 *****************************************************************************/
void reset_clkwiz ()
{
	//reset the clk_wiz
	Xil_Out32 (GPIO_CLK_BASEADDR+0x8, 0x0);
	// deassert the reset
	Xil_Out32 (GPIO_CLK_BASEADDR+0x8, 0x1);
}

/*****************************************************************************/
/**
 * This function will check MMCM lock status
 * This MMCM output will be used for Tx side video CLK
 *
 * @return None
 *
 *****************************************************************************/
void clk_wiz_locked() {
	u8 timer = 0;
	while ((Xil_In32 (GPIO_CLK_BASEADDR+0x0)) == 0 && timer < 250) {
		xil_printf("~/~/");
		timer++; // timer is used to get out from infinite loop
	}

	if(timer >= 250)
		xil_printf("Failed to lock Tx_mmcm video clock generation\r\n");
	xil_printf("^^");
}

/*****************************************************************************/
/**
 * This is for Tx only mode
 * This function will start outputing internal video patern image
 * Setup GT, DP, patern and generator.
 *
 * @return None
 *
 *****************************************************************************/
void start_tx(u8 line_rate, u8 lane_count, XVidC_VideoMode res_table,
			u8 bpc, u8 pat, u8 pat_update)
{
	u32 Status;
	/* Disabling TX and TX interrupts */

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);
	sink_power_cycle(400);

	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 1000000);

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 100000);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x1);

	xil_printf("\r\nTraining TX with: Link rate %x, Lane count %d\r\n",
			line_rate,lane_count);

	XDpTxSs_SetLinkRate(&DpTxSsInst, line_rate);
	xil_printf(".");
	XDpTxSs_SetLaneCount(&DpTxSsInst, lane_count);
	xil_printf(".");
	if (res_table !=0) {
		Status = XDpTxSs_SetVidMode(&DpTxSsInst, res_table);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR: Setting resolution failed\n\r");
		}
		xil_printf(".");
	}
	if (bpc !=0 ) {
		Status = XDpTxSs_SetBpc(&DpTxSsInst, bpc);
		if (Status != XST_SUCCESS){
			xil_printf("ERR: Setting bpc to %d failed\r\n",bpc);
		}
		xil_printf(".");
	}

	/*
	 * Setting Color Format
	 * User can change coefficients here - By default 601 is used for YCbCr
	 * */
	XDp_TxCfgSetColorEncode(DpTxSsInst.DpPtr, XDP_TX_STREAM_ID1, \
			(user_config.user_format-1), XVIDC_BT_601, XDP_DR_CEA);



	Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
	xil_printf(".");
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
	xil_printf(".");
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
			C_VideoUserStreamPattern[pat_update]);

	xil_printf(".");
	clk_wiz_locked();
	XDp_TxDisableMainLink(DpTxSsInst.DpPtr);
	/* Update VTC */
	for (int i = 0; i < DpTxSsInst.UsrOpt.NumOfStreams; i++) {
		if (DpTxSsInst.VtcPtr[i]) {
			Status = XDpTxSs_VtcSetup(DpTxSsInst.VtcPtr[i],
					&DpTxSsInst.DpPtr->TxInstance.MsaConfig[i],
					DpTxSsInst.UsrOpt.VtcAdjustBs);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"VTC%d setup failed!\n\r", Index);
				return;
			}
		}
	}
	XDp_TxEnableMainLink(DpTxSsInst.DpPtr);

	xil_printf(".");
	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	/*In case link didn't come up correctly, try once again*/
	/*Second try won't require VTC update */
	if (Status != (XST_SUCCESS)) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\n\r");
			return;
		}
	}

	/* Initialize CRC */
	/* Reset CRC*/
	XVidFrameCrc_Reset();
	/* Set Pixel width in CRC engine*/
	XDp_WriteReg(XPAR_VIDEO_FRAME_CRC_TX_BASEADDR, 0,
		     XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_USER_PIXEL_WIDTH));

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		     XDP_TX_INTERRUPT_MASK, 0x0);
	xil_printf("..done !\r\n");
}

#if ENABLE_AUDIO
void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame)
{
	u8 db1, db2, db3, db4;
	u32 temp;
	u8 RSVD=0;

	/* Fixed paramaters */
	u8 dp_version = 0x11;

	/* Write #1 */
	db1 = 0x00; //sec packet ID fixed to 0 - SST Mode
	db2 = 0x80 + xilInfoFrame->type;
	db3 = xilInfoFrame->info_length&0xFF;
	db4 = (dp_version<<2)|(xilInfoFrame->info_length>>8);
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);
//	dbg_printf("\n[AUDIO_INFOFRAME] Word1=0x%x\r",temp);

	/* Write #2 */
	db1 = xilInfoFrame->audio_channel_count
		| (xilInfoFrame->audio_coding_type<<4) | (RSVD<<3);
	db2 = (RSVD<<5)| (xilInfoFrame->sampling_frequency<<2)
		| xilInfoFrame->sample_size;
	db3 = RSVD;
	db4 = xilInfoFrame->channel_allocation;
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		XDP_TX_AUDIO_INFO_DATA(1), temp);
//	dbg_printf("\n[AUDIO_INFOFRAME] Word2=0x%x\r",temp);

	/* Write #3 */
	db1 = (xilInfoFrame->level_shift<<3) | RSVD
			| (xilInfoFrame->downmix_inhibit <<7);
	db2 = RSVD;
	db3 = RSVD;
	db4 = RSVD;
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_INFO_DATA(1), temp);
//	dbg_printf("\n[AUDIO_INFOFRAME] Word3=0x%x\r",temp);

	/* Write #4 */
	db1 = RSVD;
	db2 = RSVD;
	db3 = RSVD;
	db4 = RSVD;
	temp = 0x00000000;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_AUDIO_INFO_DATA(1), temp);
//	dbg_printf("\n[AUDIO_INFOFRAME] Word4-Word8=0x%x\r",temp);
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

/* Buffer Bypass is to be used when TX operates on stable clock.
 * To use this, the TX should be configured 
 * with buffer bypass option.(hw change)
 * */
void prog_bb (u8 bw, u8 is_tx)
{
#if BUFFER_BYPASS
	/* For Buffer Bypass, the clock output from GT is refclk
	 * this needs to be called before GT is init
	 * For KCU105 the refclk is 270Mhz
	 * */
        // For 2B, 4B mode,
        // 0x14 -> 270Mhz, 135Mhz
        // 0xA -> 135Mhz, 67.5Mhz
        // 0x6 -> 81Mhz, 40.5Mhz

#if SET_TX_TO_2BYTE == 1
	VPhy_Instance.Quads[0].TxMmcm.ClkFbOutFrac = 0;
	VPhy_Instance.Quads[0].TxMmcm.ClkOut0Frac  = 0;
	if (is_tx == 1) { // TX only path using refclk0 of 270Mhz
		if (bw == 0x1E) { //405Mhz
		      VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 3;
		      VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
		      VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 2;
		} else if (bw == 0x14) { //270Mhz
		        VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 4;
		        VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
		        VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 4;
		} else if (bw == 0xA) { //135Mhz
		        VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 4;
		        VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
		        VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 8;
		} else { //81Mhz
		        VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 36;
		        VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 10;
		        VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 12;
		}
	} else { // TX is using CPLL but refclk1 of 270/135/270
          // For KCU105 the refclk is 270Mhz, 135 or 270Mhz
                // For 2B, 4B mode,
                // 0x14 -> 270Mhz, 135Mhz
                // 0xA -> 135Mhz, 67.5Mhz
                // 0x6 -> 81Mhz, 40.5Mhz
		if (bw == 0x1E) { //270Mhz
		          VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 3;
		          VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
		          VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 2;
		} else if (bw == 0x14) { //270Mhz
		          VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 4;
		          VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
		          VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 4;
		} else if (bw == 0xA) { //135Mhz
		          VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 8;
		          VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
		          VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 8;
		} else { //81Mhz
		          VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 36;
		          VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 10;
		          VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 12;
		}
	}

#else // for 4B the clocks to be genrated are half of 2B
	if (is_tx == 1) { // TX only path using refclk0 of 270Mhz
	          if (bw == 0x14) { //135Mhz
	            VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 4;
	            VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
	            VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 8;
	          } else if (bw == 0xA) { //67.5Mhz
	            VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 4;
	            VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
	            VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 16;
	          } else { //40.5Mhz
	            VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 36;
	            VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 10;
	            VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 24;
	          }
	} else { // TX is using CPLL but refclk1 of 270/135/270
	    // For KC705 the refclk is 270Mhz, 135 or 270Mhz
	          // For 2B, 4B mode,
	          // 0x14 -> 270Mhz, 135Mhz
	          // 0xA -> 135Mhz, 67.5Mhz
	          // 0x6 -> 81Mhz, 40.5Mhz
	          if (bw == 0x14) { //135Mhz
	            VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 4;
	            VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
	            VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 8;
	          } else if (bw == 0xA) { //67.5Mhz
	            VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 8;
	            VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 1;
	            VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 16;
	          } else { //40.5Mhz
	            VPhy_Instance.Quads[0].TxMmcm.ClkFbOutMult = 36;
	            VPhy_Instance.Quads[0].TxMmcm.DivClkDivide = 10;
	            VPhy_Instance.Quads[0].TxMmcm.ClkOut0Div   = 24;
	          }
	}
#endif

	XVphy_MmcmStart(&VPhy_Instance, 0, XVPHY_DIR_TX);
	xil_printf("*");
	while (!(XVphy_MmcmLocked(&VPhy_Instance, 0, XVPHY_DIR_TX))) {

	}

	xil_printf("*~~~");
	xil_printf("\r\n");
#endif
}

u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr, int with_msa)
{
	u32 Status;
	if (with_msa == 0) {
		Status = XDpTxSs_Start(&DpTxSsInst);
	} else {
		Status = XDpTxSs_StartCustomMsa(&DpTxSsInst, Msa);
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function will detect video resolution/frequency change
 * Many GPUs won't re-train when only video res/freq change.
 * Application needs to know if it changed and need to detect new parameter.
 * Based on the parameter, re-set and re-start VDAM and Tx
 *
 * @return None
 *
 *****************************************************************************/
int VideoFMC_Init(void)
{
	int Status;
	u8 Buffer[2];
	int ByteCount;

	xil_printf("VFMC: Setting IO Expanders...\n\r");


	XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
	/* Initialize the IIC driver so that it is ready to use. */
	ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr_IIC == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
		ConfigPtr_IIC->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set the I2C Mux to select the HPC FMC */
	Buffer[0] = 0x05;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_MUX_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
		return XST_FAILURE;
	}

	I2C_Scan(XPAR_IIC_0_BASEADDR);

	/* Configure VFMC IO Expander 0:
	 * Enable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x50;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_0_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}

	/* Configure VFMC IO Expander 1:
	 * Enable LMK03318 -> In a power-down state the I2C bus becomes unusable.
	 * Select IDT8T49N241 clock as source for FMC_GT_CLKp(0)
	 * Select IDT8T49N241 clock as source for FMC_GT_CLKp(1)
	 * Enable IDT8T49N241 */

	Buffer[0] = 0x1E; // Do not disable LMK!
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_1_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}

	Status = IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize IDT 8T49N241.\n\r");
		return XST_FAILURE;
	}

	Status = TI_LMK03318_PowerDown(XPAR_IIC_0_BASEADDR, I2C_LMK03318_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize TI LMK03318.\n\r");
		return XST_FAILURE;
	}

	xil_printf(" done!\n\r");
	return XST_SUCCESS;
}

void I2C_Scan(u32 BaseAddress)
{
	u8 Buffer[2];
	int BytesRecvd;
	int i;

	print("\n\r");
	print("---------------------\n\r");
	print("- I2C Scan: \n\r");
	print("---------------------\n\r");

	for (i = 0; i < 128; i++) {
		BytesRecvd = XIic_Recv(BaseAddress, i, (u8*)Buffer, 1, XIIC_STOP);
		if (BytesRecvd == 0) {
			continue;
		}
		xil_printf("Found device: 0x%02x\n\r",i);
	}
	print("\n\r");
}

/*****************************************************************************/
/**
 * This function will detect video resolution/frequency change
 * Many GPUs won't re-train when only video res/freq change.
 * Application needs to know if it changed and need to detect new parameter.
 * Based on the parameter, re-set and re-start VDAM and Tx
 *
 * @return None
 *
 *****************************************************************************/
void video_change_detect(u32 *count_track, u32 *rxMsamisc0_track,
		u32 *bpc_track, u32 *recv_clk_freq_track, float *recv_frame_clk_track,
		u32 *recv_frame_clk_int_track, int *track_count)
{
	/* This module tracks for refresh rate change. Many GPUs do not
	 * re-train when the refresh is change. This tracks the refresh
	 * rate ans restarts the TX if needed.
	 * */
	if (start_tracking == 1) {
		if (*count_track < 5000) {
			*count_track = *count_track + 1;
		} else {
			u32 rxMsaMVid_track = (
				XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_MVID) & 0x00FFFFFF);
			u32 rxMsaNVid_track = (
				XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_NVID) & 0x00FFFFFF);
			*rxMsamisc0_track = (
				(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_MISC0) >> 5) & 0x00000007);
			*bpc_track = Bpc[(int)(*rxMsamisc0_track)];
			*recv_clk_freq_track =
					((dp_conf.LineRate * 27.0)*rxMsaMVid_track)
						/ rxMsaNVid_track;


			u32 DpHres_total = (XDp_ReadReg(
				DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
				XDP_RX_MSA_HTOTAL));

			u32 DpVres_total = (XDp_ReadReg(
				DpRxSsInst.Config.DpSubCore.DpConfig.BaseAddr,
				XDP_RX_MSA_VTOTAL));

			*recv_frame_clk_track =
					ceil(((*recv_clk_freq_track) * 1000000.0) /
					     (DpHres_total*DpVres_total));

			*recv_frame_clk_int_track = *recv_frame_clk_track;
			if (*recv_frame_clk_int_track == 59 ||
				*recv_frame_clk_int_track == 61) {

				*recv_frame_clk_int_track = 60;

			} else if (*recv_frame_clk_int_track == 29 ||
				*recv_frame_clk_int_track == 31) {

				*recv_frame_clk_int_track = 30;

			} else if (*recv_frame_clk_int_track == 74 ||
				*recv_frame_clk_int_track == 76) {

				*recv_frame_clk_int_track = 75;

			}

			if ((*recv_frame_clk_int_track != Msa[0].Vtm.FrameRate)){
				xil_printf("Refresh rate changed from %d to %d\r\n",
					   Msa[0].Vtm.FrameRate,
					   *recv_frame_clk_int_track);

				/* in case 4K60 monitor with 4K120
				 * input, do not re-train. */
				if (*recv_frame_clk_int_track < 79 ||
				    monitor_8K == 1) {
					change_detected = 1;
					start_tracking = 0;
					count_track = 0;
				}
			} else if ((dp_conf.bpc != *bpc_track)) {
				xil_printf("BPC changed from %d to %d\r\n",
						dp_conf.bpc, *bpc_track);
				change_detected = 1;
				start_tracking = 0;
				*count_track = 0;
			}
		}
	}

	/* Check 5000 times. if true then it's a real change else
	 * a bogus one due to cable unplug
	 * */
	if (change_detected == 1 && training_done == 1) {
		*track_count = *track_count + 1;
		if (*track_count == 5000) {
			xil_printf("Restarting TX....\r\n");

			//Dprx_InterruptHandlerVmChange (&DpTxSsInst);
			XDpTxSs_Stop(&DpTxSsInst);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x0);
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_AUDIO_CONTROL, 0x0);
			Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
						C_VideoUserStreamPattern[1]);
			vdma_stop(&dma_struct);
			Dprx_DetectResolution(DpRxSsInst.DpPtr);
			Dprx_CheckSetupTx(DpRxSsInst.DpPtr);
			start_tracking = 1;
			change_detected = 0;
				*track_count = 0;
		}
	} else {
		change_detected = 0;
			*track_count = 0;
	}
}


/*****************************************************************************/
/**
 * This function detects incoming video and find out video parameters
 * Based on the parameters, set up Tx and VDMA, then start Tx output
 *
 * @return None
 *
 *****************************************************************************/
void detect_rx_video_and_startTx(int *track_count1)
{
	/* LinkTraining on Rx has done stable vblank is there
	 * ready to detect MSA value
	 * */
	if (prog_tx == 1 && training_done == 1 &&
	    need_to_retrain_rx == 0) {
		*track_count1 = *track_count1 + 1;

		if (*track_count1 == 20000) {
			Dprx_DetectResolution(DpRxSsInst.DpPtr);
			/* this is needed to ensure there are on
			 * hang issues when cable is unplugged
			 * */
			if (training_done == 1) {
				XAxiVdma_DmaStop(&dma_struct[0].AxiVdma,XAXIVDMA_WRITE);
				XAxiVdma_DmaStop(&dma_struct[0].AxiVdma,XAXIVDMA_READ);
				Dprx_ResetVideoOutput(DpRxSsInst.DpPtr);
#if !JUST_RX
				Dprx_CheckSetupTx(DpRxSsInst.DpPtr);
#endif
				prog_tx =0;
				start_tracking = 1;
				change_detected = 0;
				*track_count1 = 0;
			} else {
				prog_tx = 0;
				start_tracking = 0;
				change_detected = 0;
				*track_count1 = 0;
				XDpTxSs_Stop(&DpTxSsInst);
				XDpTxSs_Reset(&DpTxSsInst);
			}
		} else {
			if (*track_count1 == 10) {
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
		*track_count1 = 0;
	}
}

/*****************************************************************************/
/**
 * This function will be used when no incoming video at pass-through mode
 * If no video, switching to internal patern generator
 *
 * @return None
 *
 *****************************************************************************/
void switch_to_Tx_only(int *track_switch, u8 *pwr_dwn_x){
	u32 Status = 0;

	/* this kicks in when cable is unplugged. it is observed that 
	 * some monitors go into unrecoverable state when cable is unplugged.
	 * Keeping GT in stable state ensures good recovery of monitor
	 * */
	if (switch_to_patgen == 1 || gt_stable == 1) {
		if (*track_switch < 10000) {
			*track_switch = *track_switch + 1;
		} else if (*track_switch == 10000) {

			DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
			DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_INTERRUPT_MASK, 0xFFF);
			XDpTxSs_Stop(&DpTxSsInst);
			Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
						C_VideoUserStreamPattern[1]);
			vdma_stop(&dma_struct);
			Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
			XDpTxSs_Reset(&DpTxSsInst);
			if (need_to_retrain_rx == 0) {
//				if (is_TX_CPLL == 1 && dp_conf.LineRate != 0x6) {
//					XVphy_BufgGtReset(&VPhy_Instance,
//						XVPHY_DIR_RX, TRUE);
//					XVphy_ResetGtPll(&VPhy_Instance, 0,
//						XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(TRUE));
//					XVphy_ResetGtPll(&VPhy_Instance, 0,
//						XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(FALSE));
//					XVphy_BufgGtReset(&VPhy_Instance,
//						XVPHY_DIR_RX, FALSE);
//					switch(dp_conf.LineRate){
//						case XDP_DPCD_MAX_LINK_RATE_162GBPS:
//						prog_bb(XDP_DPCD_MAX_LINK_RATE_162GBPS,1);
//						Status =PHY_Configuration_Tx(&VPhy_Instance,
//								PHY_User_Config_Table[6]);
//						break;
//
//						case XDP_DPCD_MAX_LINK_RATE_270GBPS:
//						prog_bb(XDP_DPCD_MAX_LINK_RATE_270GBPS,1);
//						Status =PHY_Configuration_Tx(&VPhy_Instance,
//								PHY_User_Config_Table[7]);
//						break;
//
//						case XDP_DPCD_MAX_LINK_RATE_540GBPS:
//						prog_bb(XDP_DPCD_MAX_LINK_RATE_540GBPS,1);
//						Status =PHY_Configuration_Tx(&VPhy_Instance,
//								PHY_User_Config_Table[8]);
//						break;
//
//						case XDP_DPCD_MAX_LINK_RATE_810GBPS:
//						prog_bb(XDP_DPCD_MAX_LINK_RATE_810GBPS,1);
//						Status =PHY_Configuration_Tx(&VPhy_Instance,
//								PHY_User_Config_Table[9]);
//						break;
//					}
//					if (Status != XST_SUCCESS) {
			//			xil_printf("+++++++ TX GT configuration "
			//				   "encountered a failure +++++++\r\n");
//					}
//
//				} else {
					//safer to reconfigure GT as refclk1 is lost
					switch(dp_conf.LineRate){
						case XDP_DPCD_MAX_LINK_RATE_162GBPS:
							prog_bb(XDP_DPCD_MAX_LINK_RATE_162GBPS,1);
							Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[3]);
							break;

						case XDP_DPCD_MAX_LINK_RATE_270GBPS:
							prog_bb(XDP_DPCD_MAX_LINK_RATE_270GBPS,1);
							Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[4]);
							break;

						case XDP_DPCD_MAX_LINK_RATE_540GBPS:
							prog_bb(XDP_DPCD_MAX_LINK_RATE_540GBPS,1);
							Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[5]);
							break;

						case XDP_DPCD_MAX_LINK_RATE_810GBPS:
							prog_bb(XDP_DPCD_MAX_LINK_RATE_810GBPS,1);
							Status = PHY_Configuration_Tx(&VPhy_Instance,
										PHY_User_Config_Table[10]);
							break;
					}
//				}
				sink_power_cycle(200000);
				/* switchover to TX mode only when cable 
				 * is unplugged in normal RX opreration
				 * */
				if (switch_to_patgen == 1) {
					only_tx_active = 1;
					start_tx (dp_conf.LineRate,
							dp_conf.LaneCount,
							resolution_table[2],
							8, 1, 1);
					xil_printf(".");
					xil_printf("\r\nPlease plug-in the DP RX cable "
						   "to go back to passthrough mode\r\n");
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_AUDIO_CONTROL, 0x0);
					enabled = 0;
				}
			} else {
				xil_printf("Monitor change was detected.. "
					   "please unplug-plug DP RX cable\r\n");
			}
			switch_to_patgen = 0;
			gt_stable = 0;
			track_switch = 0;
		}
	}
	if(Status != 0)
		xil_printf("Switching to Tx failed\r\n");
}

/* Audio passThrough setting */
void start_audio_passThrough(){
	int m_aud = 0;
	int n_aud = 0;

	if (training_done == 1 && vblank_count < 50) { // video is not stable yet
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_AUDIO_CONTROL, 0x0);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_CONTROL, 0x0);
		enabled = 0;
	} else if (training_done == 0) { // Rx needs to be trained first
		enabled = 0;
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_AUDIO_CONTROL, 0x0);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_CONTROL, 0x0);
	}

	// stable video is there and ready to start Audio pass-through
	if (vblank_count == 50 && enabled == 0  && start_tracking == 1) {
		xilInfoFrame->audio_channel_count = 1;
		xilInfoFrame->audio_coding_type = 0;
		xilInfoFrame->channel_allocation = 0;
		xilInfoFrame->downmix_inhibit = 0;
		xilInfoFrame->info_length = 27;
		xilInfoFrame->level_shift = 0;
		xilInfoFrame->sample_size = 0;//16 bits
		xilInfoFrame->sampling_frequency = 0; //48 Hz
		xilInfoFrame->type = 0x84;
		xilInfoFrame->version = 0x12;
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_AUDIO_CONTROL, 0x0);
		sendAudioInfoFrame(xilInfoFrame);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_AUDIO_CHANNELS, 0x2);
		switch(dp_conf.LineRate){
			case  6:m_aud = 512; n_aud = 3375; break;
			case 10:m_aud = 512; n_aud = 5625; break;
			case 20:m_aud = 512; n_aud = 11250; break;
			case 30:m_aud = 512; n_aud = 16875; break;
		}
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_MAUD,  m_aud );
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_NAUD,  n_aud );
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_AUDIO_CONTROL, 0x0);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_AUDIO_CONTROL, 0x1);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_CONTROL, 0x0);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_CONTROL, 0x1);
		xil_printf("Starting audio in passthrough mode..\r\n");
		enabled = 1;
	}
}

/*****************************************************************************/
/**
 * This function will send power down and power up command over AUX
 *
 * @return None
 *
 *****************************************************************************/
void sink_power_cycle(u32 power_down_time){
	u8 pwr_dwn = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE,
			1, &pwr_dwn);
	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, power_down_time);
	pwr_dwn = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE,
			1, &pwr_dwn);
}

/*****************************************************************************/
/**
 * This function will update and overwrite EDID color depth
 * 10BPC will be changed to 8BPC due to design limitation
 *
 * @return None
 *
 *****************************************************************************/
void update_edid(){
	int i, j;

	switch (Edid_org[126]) {
	case 0:
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i] = Edid_org[i];
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i+128] = 0;
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i+256] = 0;
		break;
	case 1:
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i] = Edid_org[i];
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i+128] = Edid1_org[i];
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i+256] = 0;
		break;
	case 2:
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i] = Edid_org[i];
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i+128] = Edid1_org[i];
		for(i = 0 ; i < 128 ; i++)
			edid_monitor[i+256] = Edid2_org[i];
		break;
	}

	if (use_monitor_edid == 1) {
		for (i = 0 ; i < (384 * 4) ; i = i + (16 * 4)) {
			for (j = i ; j < (i + (16 * 4)) ; j = j + 4) {
				XDp_WriteReg(XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
					     j, edid_monitor[(i/4)+1]);
			}
		}
		for (i = 0 ; i < (384 *4) ; i = i + 4) {
			XDp_WriteReg(XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
				     i, edid_monitor[i/4]);
		}
	}

	/* !!!!!!This is only for initial release!!!!!!
	 !!!This is required because VDMA can't support 8K30 with 10bpc interface!!!
	 !!!Hence 8bpc is force, EDID coming from monitor will be tweaked at here !!
	*/
	u8 tmp=0;
	u8 checksum;
	/* check if 8bpc or more */
	if ((edid_monitor[0x14] & 0x70) !=  0x20) {
		edid_monitor[0x14] &= 0x8F;
		edid_monitor[0x14] |= 0x20; // over writing bpc info

		// calculate checksum at here
		for (i = 0 ; i < 127 ; i++)
			tmp += edid_monitor[i];

		checksum = 256 - tmp;
		edid_monitor[127] = checksum;

		if (use_monitor_edid == 1) {
			for (i = 0 ; i < (384 * 4) ; i = i + (16 * 4)) {
				for (j = i ; j < (i + (16 * 4)) ; j = j + 4) {
					XDp_WriteReg(XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
						     j, edid_monitor[(i/4)+1]);
				}
			}
			for(i = 0 ; i < (384 * 4) ; i = i + 4) {
				XDp_WriteReg(XPAR_DP_RX_HIER_VID_EDID_0_BASEADDR,
					     i, edid_monitor[i/4]);
			}
		}
	}

}

/*****************************************************************************/
/**
 * This function toggles HW reset line for all IP's
 *
 * @return None
 *
 *****************************************************************************/
void resetIp()
{
	power_down_HLSIPs();
	usleep(10000);          //hold reset line
	power_up_HLSIPs();
	usleep(10000);          //hold reset line
}

/*****************************************************************************/
/**
 * This function sets parameters for remap IP
 *
 * @return None
 *
 *****************************************************************************/
void remap_set(XV_axi4s_remap *remap, u8 in_ppc, u8 out_ppc, u16 width,
		u16 height, u8 color_format)
{
	XV_axi4s_remap_Set_width(remap, width);
	XV_axi4s_remap_Set_height(remap, height);
	XV_axi4s_remap_Set_ColorFormat(remap, color_format);
	XV_axi4s_remap_Set_inPixClk(remap, in_ppc);
	XV_axi4s_remap_Set_outPixClk(remap, out_ppc);
}

/*****************************************************************************/
/**
 * This function starts remap IP
 *
 * @return None
 *
 *****************************************************************************/
void remap_start(struct dma_chan_parms *dma_struct)
{
	u16 width;
	width = ((dma_struct->WriteCfg.HoriSizeInput * 4) / BPC);

	remap_set(&rx_remap, dp_conf.pixel,
		  4, // Rx side output is always 4ppc
		  width, dma_struct->WriteCfg.VertSizeInput , 0);

	width = ((dma_struct->ReadCfg.HoriSizeInput * 4) / BPC);

	remap_set(&tx_remap,
		  4, // Tx side input is always 4ppc
		  dp_conf.pixel, width,
		  dma_struct->ReadCfg.VertSizeInput, 0);

	XV_axi4s_remap_EnableAutoRestart(&rx_remap);
	XV_axi4s_remap_EnableAutoRestart(&tx_remap);

	XV_axi4s_remap_Start(&rx_remap);
	XV_axi4s_remap_Start(&tx_remap);
}


void power_down_HLSIPs(void){
	Xil_Out32(HLS_RESET, 0);
}

void power_up_HLSIPs(void){
	Xil_Out32(HLS_RESET, 1);
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

//	Print_InfoPkt();
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
			XDP_RX_AUDIO_INFO_DATA(1));
	SdpExtFrame.Header[0] =  ExtFrame[0]&0xFF;
	SdpExtFrame.Header[1] = (ExtFrame[0]&0xFF00)>>8;
	SdpExtFrame.Header[2] = (ExtFrame[0]&0xFF0000)>>16;
	SdpExtFrame.Header[3] = (ExtFrame[0]&0xFF000000)>>24;

	/*Payload Information*/
	for (i = 0 ; i < 8 ; i++)
	{
		ExtFrame[i+1] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_INFO_DATA(i+2));
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
	xil_printf(" Frame Count : %d \r\n",SdpExtFrame.frame_count);
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

	xil_printf("DpRxSs_AccessLinkQualHandler : 0x%x\r\n", ReadVal);

	/*Check for PRBS Mode*/
	if( (ReadVal&0x00000007) == XDP_RX_DPCD_LINK_QUAL_PRBS)
	{
		/*Enable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhy_Instance.Config.BaseAddr,
				XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x10101010;
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr,
				XVPHY_RX_CONTROL_REG, DrpVal);

		/*Reset PRBS7 Counters*/
		DrpVal = XVphy_ReadReg(VPhy_Instance.Config.BaseAddr,
				XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x08080808;
		XDp_WriteReg(VPhy_Instance.Config.BaseAddr,
				XVPHY_RX_CONTROL_REG, DrpVal);
		DrpVal = XVphy_ReadReg(VPhy_Instance.Config.BaseAddr,
				XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xF7F7F7F7;
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, XVPHY_RX_CONTROL_REG,
				DrpVal);

		/*Set PRBS mode in Retimer*/
		XDpRxSs_MCDP6000_EnablePrbs7_Rx(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
//    	MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	} else {
		/*Disable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhy_Instance.Config.BaseAddr,
				XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xEFEFEFEF;
		XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, XVPHY_RX_CONTROL_REG,
				DrpVal);

		/*Disable PRBS mode in Retimer*/
		XDpRxSs_MCDP6000_DisablePrbs7_Rx(XPAR_IIC_0_BASEADDR,
				I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
//		MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
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
#if ( AccessErrorCounterHandler_FLAG == 1)
	u16 DrpVal;
	u16 DrpVal_lower_lane0;
	u16 DrpVal_lower_lane1;
	u16 DrpVal_lower_lane2;
	u16 DrpVal_lower_lane3;

	/*Read PRBS Error Counter Value from Video PHY*/

	/*Lane 0 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH1,
			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal_lower_lane0);
	XVphy_DrpRd(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH1,
			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Lane 1 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH2,
			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal_lower_lane1);
	XVphy_DrpRd(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH2,
			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Lane 2 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH3,
			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal_lower_lane2);
	XVphy_DrpRd(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH3,
			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Lane 3 - Store only lower 16 bits*/
	XVphy_DrpRd(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH4,
			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal_lower_lane3);
	XVphy_DrpRd(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH4,
			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);

	/*Write into DP Core - Validity bit and lower 15 bit counter value*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DPC_L01_PRBS_CNTR,
			(0x8000|DrpVal_lower_lane0) |
			((0x8000|DrpVal_lower_lane1)<<16));
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DPC_L23_PRBS_CNTR,
			(0x8000|DrpVal_lower_lane2) |
			((0x8000|DrpVal_lower_lane3)<<16));

	/*Reset PRBS7 Counters*/
	DrpVal = XVphy_ReadReg(VPhy_Instance.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
	DrpVal = DrpVal | 0x08080808;
	XDp_WriteReg(VPhy_Instance.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);
	DrpVal = XVphy_ReadReg(VPhy_Instance.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
	DrpVal = DrpVal & 0xF7F7F7F7;
	XVphy_WriteReg(VPhy_Instance.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);
#endif
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

	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG);

	/*Record Training Algo Value - to be restored in non-phy test mode*/
	TrainingAlgoValue = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
			XDP_RX_MIN_VOLTAGE_SWING);

	/*Refer to DPCD 0x270 Register*/
	if ((ReadVal&0x8000) == 0x8000) {
		/*Enable PHY test mode - Set Min voltage swing to 0*/
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_MIN_VOLTAGE_SWING,
				(TrainingAlgoValue & 0xFFFFFFFC) | 0x80000000);

			/*Disable Training timeout*/
			ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
				XDP_RX_CDR_CONTROL_CONFIG);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_CDR_CONTROL_CONFIG, ReadVal | 0x40000000);

	} else {
		/*Disable PHY test mode & Set min voltage swing back to level 1*/
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_MIN_VOLTAGE_SWING,
				(TrainingAlgoValue & 0x7FFFFFFF) | 0x1);

		/*Enable Training timeout*/
		ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_CDR_CONTROL_CONFIG);
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_CDR_CONTROL_CONFIG, ReadVal & 0xBFFFFFFF);
	}
}
