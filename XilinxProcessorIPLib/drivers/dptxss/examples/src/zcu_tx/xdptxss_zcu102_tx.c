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



/***************************** Include Files *********************************/

#include "xdptxss.h"
#include "xvphy.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
	/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#include "xiic.h"
#include "xiic_l.h"
#include "xtmrctr.h"
#include "xuartlite.h"
#include "xuartlite_l.h"
#include "xspi.h"
#include "xvidc_edid.h"
#include "sleep.h"
#include "stdlib.h"
//#include "LMK04906.h"
/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_TX_SUBSYSTEM_DP_TX_SUBSYSTEM_0_DPTXSS_DP_IRQ_INTR
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPTXSS_HDCP_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_HDCP_IRQ_VEC_ID
#define XINTC_DPTXSS_TMR_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_TIMER_IRQ_VEC_ID
#endif
#define XINTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC				XScuGic
#define XINTC_HANDLER			XScuGic_InterruptHandler

/* The unique device ID of the DisplayPort Transmitter Subsystem HIP instance
 * to be used
 */
#define XDPTXSS_DEVICE_ID		XPAR_DPTXSS_0_DEVICE_ID

/* If set to 1, example will run in MST mode. Otherwise, in SST mode.
 * In MST mode, this example reads the EDID of RX devices if connected in
 * daisy-chain.
 */
#define DPTXSS_MST			1
#define DPTXSS_LINK_RATE		XDPTXSS_LINK_BW_SET_540GBPS
#define DPTXSS_LANE_COUNT		XDPTXSS_LANE_COUNT_SET_4

/* The video resolution from the display mode timings (DMT) table to use for
 * DisplayPort TX Subsystem. It can be set to use preferred video mode for
 * EDID of RX device.
 */
#define DPTXSS_VID_MODE			XVIDC_VM_USE_EDID_PREFERRED

/* The color depth (bits per color component) to use DisplayPort TX
 * Subsystem.
 */
#define DPTXSS_BPC			XPAR_DPTXSS_0_BITS_PER_COLOR
#define IIC_DEVICE_ID       XPAR_PROCESSOR_SYSTEM_AXI_IIC_1_DEVICE_ID
#define XVPHY_DEVICE_ID		XPAR_VID_PHY_CONTROLLER_0_DEVICE_ID
#define UART_BASEADDR		XPAR_PROCESSOR_SYSTEM_AXI_UARTLITE_1_BASEADDR

#define SET_TX_TO_2BYTE		\
		(XPAR_TX_SUBSYSTEM_DP_TX_SUBSYSTEM_0_DP_GT_DATAWIDTH/2)

#define TIMER_RESET_VALUE				1000
#define is_TX_CPLL 0
#define CLK135MHz_DIVIDER 18
#define CLK270MHz_DIVIDER 9
#define CLK162MHz_DIVIDER 15
#define ENABLE_AUDIO 1
#if (ENABLE_AUDIO == 1)
	#define AV_PAT_GEN_BASE  XPAR_TX_SUBSYSTEM_AV_PAT_GEN_0_BASEADDR
#endif

#define CLK_WIZ_BASE      				XPAR_CLK_WIZ_0_BASEADDR




#define XVPHY_DRP_CPLL_FBDIV		0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV	0x2A
#define XVPHY_DRP_RXOUT_DIV			0x63
#define XVPHY_DRP_RXCLK25			0x6D
#define XVPHY_DRP_TXCLK25			0x7A
#define XVPHY_DRP_TXOUT_DIV			0x7C

/***************** Macros (Inline Functions) Definitions *********************/

//#define DPDEBUG

#ifdef DPDEBUG
#define debug_DP_printf printf
#else
#define debug_DP_printf 1 ? (void) 0 : printf
#endif

int printf(const char *format, ...);


/**************************** Type Definitions *******************************/
/**************************** Type Definitions *******************************/
typedef enum {
        ONBOARD_REF_CLK = 1,
        DP159_FORWARDED_CLK = 3,
} XVphy_User_GT_RefClk_Src;

typedef struct {
        u8 Index;
        XVphy_PllType  TxPLL;
        XVphy_PllType  RxPLL;
        XVphy_ChannelId TxChId;
        XVphy_ChannelId RxChId;
        u32 LineRate;
        u64 LineRateHz;
        XVphy_User_GT_RefClk_Src QPLLRefClkSrc;
        XVphy_User_GT_RefClk_Src CPLLRefClkSrc;
        u64 QPLLRefClkFreqHz;
        u64 CPLLRefClkFreqHz;
} XVphy_User_Config;
static XVphy_User_Config PHY_User_Config_Table[] =
{
  // Index,         TxPLL,               RxPLL,
 //	TxChId,         RxChId,
// LineRate,              LineRateHz,
// QPLLRefClkSrc,          CPLLRefClkSrc,    QPLLRefClkFreqHz,CPLLRefClkFreqHz
  {   0,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  DP159_FORWARDED_CLK,    DP159_FORWARDED_CLK,     270000000,81000000},
  {   1,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  DP159_FORWARDED_CLK,    DP159_FORWARDED_CLK,     270000000,135000000},
  {   2,     XVPHY_PLL_TYPE_CPLL,   XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CHA,     XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  DP159_FORWARDED_CLK,    DP159_FORWARDED_CLK,     270000000,270000000},
  {   3,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x06,    XVPHY_DP_LINK_RATE_HZ_162GBPS,
		  ONBOARD_REF_CLK,        DP159_FORWARDED_CLK,     270000000,81000000},
  {   4,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x0A,    XVPHY_DP_LINK_RATE_HZ_270GBPS,
		  ONBOARD_REF_CLK,        DP159_FORWARDED_CLK,     270000000,135000000},
  {   5,     XVPHY_PLL_TYPE_QPLL1,  XVPHY_PLL_TYPE_CPLL,
		  XVPHY_CHANNEL_ID_CMN1,    XVPHY_CHANNEL_ID_CHA,
		  0x14,    XVPHY_DP_LINK_RATE_HZ_540GBPS,
		  ONBOARD_REF_CLK,        DP159_FORWARDED_CLK,     270000000,270000000},
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

};

typedef struct {
        u8  TEST_CRC_CNT;
        u8  TEST_CRC_SUPPORTED;
        u8  TEST_CRC_START_STOP;
        u16 Pixel_r;
        u16 Pixel_g;
        u16 Pixel_b;
        u8  Mode_422;
} Video_CRC_Config;

typedef struct
{
	 XVidC_VideoMode VideoMode_local;
	unsigned char user_bpc;
	unsigned char user_pattern;
	unsigned int user_numStreams;
	unsigned int user_stream_number;
	unsigned int mst_check_flag;
}user_config_struct;

u8 StreamPattern[5] = {0x11, 0x13, 0x15, 0x16, 0x10};
u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
											0x15, 0x16, 0x17}; //Duplicate
unsigned char bpc_table[] = {6,8,10,12,16};
double max_freq[] = {216.0, 172.8, 360.0, 288.0, 720.0, 576.0};

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

// adding new resolution definition example
// XVIDC_VM_3840x2160_30_P_SB, XVIDC_B_TIMING3_60_P_RB
// and XVIDC_VM_3840x2160_60_P_RB has added
typedef enum {
    XVIDC_VM_1920x1080_60_P_RB = (XVIDC_VM_CUSTOM + 1),
	XVIDC_B_TIMING3_60_P_RB = (XVIDC_VM_1920x1080_60_P_RB + 1),
	XVIDC_VM_3840x2160_60_P_RB = (XVIDC_B_TIMING3_60_P_RB + 1),
    XVIDC_CM_NUM_SUPPORTED
} XVIDC_CUSTOM_MODES;

// Here is the detailed timing for each custom resolutions.
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode[
					(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{
    { XVIDC_VM_1920x1080_60_P_RB, "1920x1080@60Hz (RB)", XVIDC_FR_60HZ,
        {1920, 48, 32, 80, 2080, 1,
         1080, 3, 5, 23, 1111, 0, 0, 0, 0, 0}
    },

    { XVIDC_B_TIMING3_60_P_RB, "2560x1440@60Hz (RB)", XVIDC_FR_60HZ,
	{2560, 48, 32, 80, 2720, 1,
	 1440, 3, 5, 33, 1481, 0, 0, 0, 0, 0}
    },

    { XVIDC_VM_3840x2160_60_P_RB, "3840x2160@60Hz (RB)", XVIDC_FR_60HZ,
	{3840, 48, 32, 80, 4000, 1,
	 2160, 3, 5, 54, 2222, 0, 0, 0, 0, 0}
    },
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
				XVIDC_VM_3840x2160_60_P_RB
};

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

#define XDP_TX_CRC_CONFIG    	0x074
#define XDP_TX_CRC_COMP0    	0x078
#define XDP_TX_CRC_COMP1    	0x07C
#define XDP_TX_CRC_COMP2    	0x080

/* Video Frame CRC Specific Defines
 */
#define VIDEO_FRAME_CRC_CONFIG			0x00
#define VIDEO_FRAME_CRC_VALUE_G_R		0x04
#define VIDEO_FRAME_CRC_VALUE_B			0x08
#define VIDEO_FRAME_CRC_ACTIVE_COUNTS	0x0C

/************************** Function Prototypes ******************************/

u32 DpTxSs_Main(u16 DeviceId);
u32 DpTxSs_PlatformInit(void);


/* Interrupt helper functions */
void DpPt_HpdEventHandler(void *InstancePtr);
void DpPt_HpdPulseHandler(void *InstancePtr);
void DpPt_LinkrateChgHandler (void *InstancePtr);


u32 DpTxSs_SetupIntrSystem(void);


u32 DpTxSs_VideoPhyInit(u16 DeviceId);
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr, int with_msa);
//void DpTxSs_Setup(u8 * LineRate_init, u8 * LaneCount_init);
void DpTxSs_Setup(u8 *LineRate_init, u8 *LaneCount_init,
										u8 Edid_org[128], u8 Edid1_org[128]);
u32 PHY_Configuration_Tx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table);
char GetInbyte(void);
u32 start_tx(u8 line_rate, u8 lane_count, user_config_struct user_config);
void hpd_con(u8 Edid_org[128], u8 Edid1_org[128], u16 res_update);
u8 XUartLite_RecvByte_local(u32 BaseAddress);
char inbyte_local(void);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
extern int PLL_init_Seting(XSpi *SPI_LMK04906, u32 div_value);
extern u32 clk_set(u8 i2c_mux_addr, u8 i2c_dev_addr, double set_freq);
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Rx_to_two_byte);
char xil_getc(u32 timeout_ms);
u32 xil_gethex(u8 num_chars);
void sub_help_menu(void);
void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame);
void clk_wiz_locked();
void DpTxSs_DisableAudio();
void hpd_pulse_con();
void app_help ();
void bpc_help_menu(int);
void select_link_lane(void);
void resolution_help_menu(void);
void test_pattern_gen_help();
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void LMK04906_init(XSpi *SPI_LMK04906);

static u8 CalculateChecksum(u8 *Data, u8 Size);
XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
void ReportVideoCRC();

/************************** Variable Definitions *****************************/

XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
XIic IicInstance;	/* I2C bus for Si570 */
XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
static XScuGic IntcInst;
XVphy VPhyInst;	/* The DPRX Subsystem instance.*/
XTmrCtr TmrCtr; /* Timer instance.*/
XUartLite UartLite; /* Instance of the UartLite device */
volatile Video_CRC_Config VidFrameCRC;

int tx_is_reconnected; /*This variable to keep track of the status of Tx link*/
u8 prev_line_rate; /*This previous line rate to keep previous info to compare
						with new line rate request*/

//volatile Video_CRC_Config VidFrameCRC;
/************************** Function Definitions *****************************/



/*****************************************************************************/
/**
*
* This is the main function for XDpTxSs interrupt example. If the
* DpTxSs_IntrExample function which sets up the system succeeds, this function
* will wait for the interrupts. Once a connection event or pulse is detected,
* DpTxSs will RX device capabilities and re-start the subsystem.
*
* @param	None.
*
* @return
*		- XST_FAILURE if the interrupt example was unsuccessful.
*
* @note		Unless setup failed, main will never return since
*		DpTxSs_IntrExample is blocking (it is waiting on interrupts
*		for Hot-Plug-Detect (HPD) events.
*
******************************************************************************/
int main()
{
	u32 Status;

	xil_printf("------------------------------------------\r\n");
	xil_printf("DisplayPort TX Subsystem Example Design\r\n");
	xil_printf("(c) 2017 by Xilinx\r\n");
	xil_printf("-------------------------------------------\r\n\r\n");

	Status = DpTxSs_Main(XDPTXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort TX Subsystem interrupt example failed.");
		return XST_FAILURE;
	}

	xil_printf(
			"Successfully ran DisplayPort TX Subsystem interrupt example\r\n");

	return XST_SUCCESS;
}

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
u32 DpTxSs_Main(u16 DeviceId)
{
	int i;
	u32 Status;
	XDpTxSs_Config *ConfigPtr;
	u8 exit = 0;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	u8 LaneCount;
	u8 LineRate;
	u8 LineRate_init = XDP_TX_LINK_BW_SET_540GBPS;
	u8 LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;
	u8 LaneCount_init = 0x4;
	u8 LaneCount_init_tx = 0x4;
	u8 Edid_org[128], Edid1_org[128];
	u8 done=0;
	u32 user_tx_LaneCount , user_tx_LineRate;
	u32 aux_reg_address, num_of_aux_registers;
	u8 Data[8];
	u8 audio_on=0;
	XilAudioInfoFrame *xilInfoFrame;
	int m_aud, n_aud;
	u8 in_pwr_save = 0;
	u8 connected = 0;
	u8 pwr_dwn;
	u16 DrpVal =0;

	user_config_struct user_config;
	user_config.user_bpc = 8;
	user_config.VideoMode_local = XVIDC_VM_800x600_60_P;
	user_config.user_pattern = 1;


	// Adding custom resolutions at here.
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
	}



	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\r\n");
	Status = DpTxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\r\n");
	}
	xil_printf("Platform initialization done.\r\n");


	XSpi SPI_LMK04906;  /* SPI Device Point*/

	LMK04906_init(&SPI_LMK04906);
	PLL_init_Seting(&SPI_LMK04906, CLK270MHz_DIVIDER);
	xil_printf("Programming 270 MHz Clock for GTREFCLK0...\r\n");

	xilInfoFrame = 0; // initialize


#if ENABLE_AUDIO
	// I2C MUX device address : 0x74
	// Si570 device address : 0x5D
	//setting Si570 on zcu102 to be 24.576MHz for audio
	clk_set(0x74, 0x5D, 24.576);
#endif

	/* Obtain the device configuration for the DisplayPort TX Subsystem */
	ConfigPtr = XDpTxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the DpTxSsInst's Config
	 * structure. */
	Status = XDpTxSs_CfgInitialize(&DpTxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPTXSS config initialization failed.\r\n");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpTxSsInst.UsrOpt.MstSupport) {
		xil_printf("\r\nINFO:DPTXSS is MST enabled. DPTXSS can be "
			"switched to SST/MST\r\n\r\n");
	}
	else {
		xil_printf("\r\nINFO:DPTXSS is  SST enabled. DPTXSS works "
			"only in SST mode.\r\n\r\n");
	}

	Status = DpTxSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system setup failed.\r\n");
		return XST_FAILURE;
	}


	/* Setup Video Phy, left to the user for implementation */
	DpTxSs_VideoPhyInit(XVPHY_DEVICE_ID);

	DpTxSs_Setup(&LineRate_init, &LaneCount_init, Edid_org, Edid1_org);



	// check if monitor is connected or not
    while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
	if (connected == 0) {
	xil_printf(
			"Please connect a DP Monitor to start the application !!!\r\n");
	connected = 1;
	}
    }

	//Waking up the monitor
	pwr_dwn = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
	// give enough time for monitor to power down
	usleep(400);
	pwr_dwn = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, 0x00600, 1, &pwr_dwn);
	// give enough time for monitor to wake up
	usleep(400000);

	/* Do not return in order to allow interrupt handling to run. HPD events
	 * (connect, disconnect, and pulse) will be detected and handled.
	 */
	while (1){
		XScuGic_Enable(&IntcInst,XINTC_DPTXSS_DP_INTERRUPT_ID);

		exit = 0;
		DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
		DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;

		LineRate = LineRate_init_tx;
		LaneCount = LaneCount_init_tx;
		XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
// This configures the vid_phy for line rate to start with
		//Even though CPLL can be used in limited case,
		//using QPLL is recommended for more coverage.
		switch(LineRate_init_tx){
			case XDP_TX_LINK_BW_SET_162GBPS:
				Status = PHY_Configuration_Tx(&VPhyInst,
							PHY_User_Config_Table[(is_TX_CPLL)?0:3]);
				break;

			case XDP_TX_LINK_BW_SET_270GBPS:
				Status = PHY_Configuration_Tx(&VPhyInst,
							PHY_User_Config_Table[(is_TX_CPLL)?1:4]);
				break;

			case XDP_TX_LINK_BW_SET_540GBPS:
				Status = PHY_Configuration_Tx(&VPhyInst,
							PHY_User_Config_Table[(is_TX_CPLL)?2:5]);
				break;
		}

		if (Status != XST_SUCCESS) {
			xil_printf (
	   "+++++++ TX GT configuration encountered a failure +++++++\r\n");
		}
// The clk_wiz that generates half of lnk_clk has to be programmed
//  as soon as lnk clk is valid

		LaneCount_init_tx = LaneCount_init_tx & 0x7;
		//800x600 8bpc as default
		start_tx (LineRate_init_tx, LaneCount_init_tx,user_config);
		LineRate = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
		LaneCount = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount;
		// Enabling TX interrupts
		sub_help_menu ();
		CmdKey[0] = 0;
		CommandKey = 0;

		while (1) { // for menu loop

			if (tx_is_reconnected == 1) {
				hpd_con(Edid_org, Edid1_org, user_config.VideoMode_local);
				tx_is_reconnected = 0;
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
						Data[0] = 0x2;
						XDp_TxAuxWrite(DpTxSsInst.DpPtr,
						XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
						in_pwr_save = 1;
						xil_printf (
							"\r\n==========power down===========\r\n");
					} else {
						Data[0] = 0x1;
						XDp_TxAuxWrite(DpTxSsInst.DpPtr,
						XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
						in_pwr_save = 0;
						xil_printf (
							"\r\n==========out of power down===========\r\n");
						hpd_con(Edid1_org, Edid1_org,
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
						Xil_Out32 (AV_PAT_GEN_BASE + 0x400 + 0x0, 0x1);
						Xil_Out32 (AV_PAT_GEN_BASE + 0x400 + 0x0, 0x2);
						Xil_Out32 (AV_PAT_GEN_BASE + 0x400 + 0x10, 0x2);
						Xil_Out32 (AV_PAT_GEN_BASE + 0x400 + 0x20, 0x2);
						Xil_Out32 (AV_PAT_GEN_BASE + 0x400 + 0xA0,
								0x10000244);//channel status    16.4 release
						Xil_Out32 (AV_PAT_GEN_BASE + 0x400 + 0xA4,
								0x40000000);//channel statu    16.4 release

						Xil_Out32 (AV_PAT_GEN_BASE + 0x400 + 0x4, 0x202);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_CONTROL, 0x1);
						xil_printf ("Audio enabled\r\n");
						audio_on = 1;
					} else {
						Xil_Out32 (AV_PAT_GEN_BASE + 0x400 + 0x0, 0x0);
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_AUDIO_CONTROL, 0x0);
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
							audio_on = 0;
							user_config.VideoMode_local =
												resolution_table[Command];


							start_tx (LineRate,LaneCount,user_config);
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
					bpc_help_menu(DPTXSS_BPC);
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
								start_tx (LineRate, LaneCount,user_config);
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
						CmdKey[0] = XUartLite_RecvByte(UART_BASEADDR);
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
							if((Command>=0)&&(Command<9))
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
							LaneCount_init_tx = user_tx_LaneCount;

						   switch(LineRate_init_tx)
						   {
							   case XDP_TX_LINK_BW_SET_162GBPS:
								Status = PHY_Configuration_Tx(&VPhyInst,
									PHY_User_Config_Table[(is_TX_CPLL)?0:3]);
								break;

							   case XDP_TX_LINK_BW_SET_270GBPS:
								Status = PHY_Configuration_Tx(&VPhyInst,
									PHY_User_Config_Table[(is_TX_CPLL)?1:4]);
								break;

							   case XDP_TX_LINK_BW_SET_540GBPS:
								Status = PHY_Configuration_Tx(&VPhyInst,
									PHY_User_Config_Table[(is_TX_CPLL)?2:5]);
								break;
						   }
							if (Status != XST_SUCCESS) {
								xil_printf (
			"+++++++ TX GT configuration encountered a failure +++++++\r\n");
							}

							XDpTxSs_Stop(&DpTxSsInst);
							audio_on = 0;
							xil_printf(
							  "TX Link & Lane Capability is set to %x, %x\r\n",
										  user_tx_LineRate, user_tx_LaneCount);
							xil_printf(
							  "Setting TX to 8 BPC and 800x600 resolution\r\n");
							XDpTxSs_Reset(&DpTxSsInst);
							user_config.user_bpc=8;
							user_config.VideoMode_local=XVIDC_VM_800x600_60_P;
							user_config.user_pattern=1;
							start_tx (user_tx_LineRate, user_tx_LaneCount,
													user_config);
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
					sub_help_menu ();
					break;

					/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

				case '5' :
					//MSA;
					XDpTxSs_ReportMsaInfo(&DpTxSsInst);
					break;

					/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

//					case '6' :
//						//EDID;
//						XDptx_DbgPrintEdid(DpTxSsInst.DpPtr);
//						break;

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
					ReportVideoCRC();
					break;

//				case 'x' :
//					XDpTxSs_Stop(&DpTxSsInst);
//					app_help ();
//
//					XScuGic_Disable(&IntcInst,
//									XINTC_DPTXSS_DP_INTERRUPT_ID);
//
//					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//											XDP_TX_ENABLE, 0x0);
//					XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//											XDP_TX_INTERRUPT_STATUS);
//					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//											XDP_TX_INTERRUPT_MASK, 0xFFF);
//					break;

					/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
				case 'z' :
					sub_help_menu ();
					break;

				} //end of switch (CmdKey[0])
		} //end of while
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required platform-specifc peripherals.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if required peripherals are initialized and
*		configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_PlatformInit(void)
{

	u32 Status;
	// Initialize UART
	Status = XUartLite_Initialize(&UartLite,
							XPAR_PROCESSOR_SYSTEM_AXI_UARTLITE_1_DEVICE_ID);
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
	return XST_SUCCESS;
}



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
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	/* Set custom timer wait */
	XDpTxSs_SetUserTimerHandler(&DpTxSsInst, &DpPt_CustomWaitUs, &TmrCtr);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_EVENT),
						&DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_PULSE),
						&DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
						&DpPt_LinkrateChgHandler, &DpTxSsInst);


	/* The configuration parameters of the interrupt controller */
	XScuGic_Config *IntcConfig;

	/* Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(XINTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcConfig,
				IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
			(Xil_InterruptHandler)XDpTxSs_DpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	/* Enable the interrupt for the Pixel Splitter device */
	XScuGic_Enable(IntcInstPtr,
				XPAR_FABRIC_TX_SUBSYSTEM_DP_TX_SUBSYSTEM_0_DPTXSS_DP_IRQ_INTR);


	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception
	 * table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XINTC_HANDLER, IntcInstPtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();

	return (XST_SUCCESS);
}



/*****************************************************************************/
/**
*
* This function configures Video Phy.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if Video Phy configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_VideoPhyInit(u16 DeviceId)
{
	XVphy_Config *ConfigPtr;

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
	ConfigPtr = XVphy_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

    PLLRefClkSel (&VPhyInst, PHY_User_Config_Table[5].LineRate);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
            PHY_User_Config_Table[5].CPLLRefClkSrc,
            PHY_User_Config_Table[5].QPLLRefClkSrc,
            PHY_User_Config_Table[5].TxPLL,
            PHY_User_Config_Table[5].RxPLL,
            PHY_User_Config_Table[5].LineRate);

	// initial line Rate setting
	prev_line_rate = PHY_User_Config_Table[5].LineRate;

	PHY_Two_byte_set (&VPhyInst, SET_TX_TO_2BYTE);

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function sets proper ref clk frequency and line rate
*
* @param	InstancePtr is a pointer to the Video PHY instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate) {
	switch (link_rate) {
	case XDP_TX_LINK_BW_SET_162GBPS:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											DP159_FORWARDED_CLK, 81000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN1, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			break;
	case XDP_TX_LINK_BW_SET_540GBPS:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											DP159_FORWARDED_CLK, 270000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN1, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;
	default:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										ONBOARD_REF_CLK, 270000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
											DP159_FORWARDED_CLK, 135000000);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN1, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			break;
	}
}
/*****************************************************************************/
/**
*
* This function sets GT in 16-bits (2-Byte) or 32-bits (4-Byte) mode.
*
* @param	InstancePtr is a pointer to the Video PHY instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Tx_to_two_byte)
{

	u16 DrpVal;
	u16 WriteVal;
	u32 Status;
	u16 TX_DATA_WIDTH_REG = 0x7A;
	u16 TX_INT_DATAWIDTH_REG = 0x85;

    if (Tx_to_two_byte == 1) {

		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
											TX_DATA_WIDTH_REG, &DrpVal);

		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		}
		DrpVal &= ~0xF;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x3;
		Status  =XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
												TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
												TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
												TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
												TX_DATA_WIDTH_REG, WriteVal);
		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		}

		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
											TX_INT_DATAWIDTH_REG, &DrpVal);
		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		}

		DrpVal &= ~0xC00;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x0;
		Status  =XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
										TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
										TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
										TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
										TX_INT_DATAWIDTH_REG, WriteVal);
		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		}
		xil_printf ("TX Channel configured for 2byte mode\r\n");
    }


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
// This XAPP assumes that RX and TX would run at same rate

	u8 rate;
	u32 Status=XST_SUCCESS;

	rate = DpTxSsInst.DpPtr->TxInstance.LinkConfig.LinkRate;
	if (rate != prev_line_rate) {
		switch(rate){
			case 0x6:
			Status = PHY_Configuration_Tx(&VPhyInst,
					PHY_User_Config_Table[(is_TX_CPLL)?0:3]);
			break;

			case 0xA:
			Status = PHY_Configuration_Tx(&VPhyInst,
					PHY_User_Config_Table[(is_TX_CPLL)?1:4]);
			break;

			case 0x14:
			Status = PHY_Configuration_Tx(&VPhyInst,
					PHY_User_Config_Table[(is_TX_CPLL)?2:5]);
			break;
		}
		//update the previous link rate info at here
		prev_line_rate = rate;
	}
	if (Status != XST_SUCCESS) {
		debug_DP_printf (
		"+++++++ TX GT configuration encountered a failure +++++++\r\n");
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
	u32 NumTicks = (MicroSeconds * (
				XPAR_PROCESSOR_SYSTEM_AXI_TIMER_0_CLOCK_FREQ_HZ / 1000000));

	XTmrCtr_Reset(&TmrCtr, 0);
	XTmrCtr_Start(&TmrCtr, 0);

	/* Wait specified number of useconds. */
	do {
		TimerVal_pre = TimerVal;
		TimerVal = XTmrCtr_GetValue(&TmrCtr, 0);
		if(TimerVal_pre == TimerVal){
			debug_DP_printf("Something went wrong with DpPt_CustomWaitUs\r\n");
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
	u8 pwr_dwn;

	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		debug_DP_printf("\r\n+===> HPD Connected.\r\n");
		pwr_dwn = 0x2;
		XDp_TxAuxWrite(DpTxSsInst.DpPtr, XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1,
																	&pwr_dwn);
		pwr_dwn = 0x1;
		XDp_TxAuxWrite(DpTxSsInst.DpPtr, XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1,
																	&pwr_dwn);
		tx_is_reconnected = 1;
	}
	else
	{
		debug_DP_printf("\r\n+===> HPD Disconnected.\r\n");

		//DpTxSs_DisableAudio
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												XDP_TX_AUDIO_CONTROL, 0x0);

		//on HPD d/c, it is important to bring down the HDCP
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

	debug_DP_printf("\r\nHPD Pulse event detected\r\n");
// Some monitors give HPD pulse repeatedly which causes HPD pulse function to
//		be executed huge number of time. Hence hpd_pulse interrupt is disabled
//		and then enabled when hpd_pulse function is executed
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_INTERRUPT_MASK,
					XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);
			hpd_pulse_con();
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
void hpd_pulse_con()
{

	u32 Status=0;
	u8 Edid[128];
	u8 auxValues[9];
	u8 lane0_sts;
	u8 lane2_sts;
	u8 laneAlignStatus;
	u8 bw_set;
	u8 lane_set;

     //reading the first block of EDID
     Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
			 XDP_DPCD_SINK_COUNT, 6, auxValues);
     Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
			 XDP_DPCD_STATUS_LANE_0_1, 1, &lane0_sts);
     Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
			 XDP_DPCD_STATUS_LANE_2_3, 1, &lane2_sts);
     Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
			 XDP_DPCD_LANE_ALIGN_STATUS_UPDATED, 1, &laneAlignStatus);
     Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
			 XDP_DPCD_LANE_COUNT_SET, 1, &lane_set);
     Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
			 XDP_DPCD_LINK_BW_SET, 1, &bw_set);
     // wait for response
     DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400);

     XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LINK_BW_SET, 1, &bw_set);
     // wait for response
     DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400);
     XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_LANE_COUNT_SET, 1, &lane_set);
     // wait for response
     DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400);
     XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid, 0);
     // wait for response
     DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400);

     // EDID corruption handing
	if (!XVidC_EdidIsHeaderValid(Edid)) {
		debug_DP_printf("\r\n   Error : EDID header is corrupt...\r\n");
		// if EDID data is corrupt, set 640x480 6bpc
		//start_tx (0x6, 1,resolution_table[0], 6, 1);
		//return;
	}


     lane_set = lane_set & 0x1F;
     bw_set = bw_set & 0x1F;
     laneAlignStatus = laneAlignStatus & 0x1;

     if(bw_set != 0x6 && bw_set != 0xA && bw_set != 0x14){
	 debug_DP_printf ("Something is wrong bw_set:%x\r\n",bw_set);
	 return;
     }
     if(lane_set != 1 && lane_set != 2 && lane_set != 4){
	 debug_DP_printf ("Something is wrong lane_set:%x\r\n",lane_set);
	 return;
     }

     //end hdcp
     if (bw_set != 0) {
		 if (lane_set == 0x4) {
			lane0_sts = lane0_sts & 0x55;
			lane2_sts = lane2_sts & 0x55;
			if ((lane0_sts != 0x55) || (lane2_sts != 0x55)
												|| (laneAlignStatus != 1)) {
				XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
				debug_DP_printf (".");
				XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);
				 Status = XDpTxSs_Start(&DpTxSsInst);
				 debug_DP_printf ("training 4\r\n");
			}
		 } else if (lane_set == 0x2) {
			lane0_sts = lane0_sts & 0x55;
			if ((lane0_sts != 0x55) || (laneAlignStatus != 1)) {
				XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
				debug_DP_printf (".");
				XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);

				 Status = XDpTxSs_Start(&DpTxSsInst);
				 debug_DP_printf ("training 2\r\n");
			}
		 } else if (lane_set == 0x1) {
			lane0_sts = lane0_sts & 0x5;
			if ((lane0_sts != 0x5) || (laneAlignStatus != 1)) {
				XDpTxSs_SetLinkRate(&DpTxSsInst, bw_set);
				debug_DP_printf (".");
				XDpTxSs_SetLaneCount(&DpTxSsInst, lane_set);

				 Status = XDpTxSs_Start(&DpTxSsInst);
				 debug_DP_printf ("training 1\r\n");
			}
		 }
     }
     XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);
     debug_DP_printf ("HPD pulse over\r\n");

}



// introduced to address reduced blanking linereset issue from 16.4 release
u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr, int with_msa){
	u32 Status;
		Status = XDpTxSs_Start(&DpTxSsInst);

	// Disable linereset all the time.
	// Patched on 2016.4 release
	// This code should move into Displayport driver side on later release
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											XDP_TX_LINE_RESET_DISABLE, 0);


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
	u8 pwr_dwn = 0;
	u8 Status;

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);

	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

	u8 connected;
    while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
	if (connected == 0) {
	xil_printf(
			"Please connect a DP Monitor to start the application !!!\r\n");
	connected = 1;
	}
    }

	//Waking up the monitor
	pwr_dwn = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1,
																	&pwr_dwn);
	// Just giving a bit of time for monitor to operate power command
	usleep(400);
	pwr_dwn = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1,
																	&pwr_dwn);
	// Just giving a bit of time for monitor to operate power command
	usleep(400);

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
		if (Status != XST_SUCCESS) { // give another chance to monitor.
			//Waking up the monitor
			pwr_dwn = 0x2;
			XDp_TxAuxWrite(DpTxSsInst.DpPtr,
								XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &pwr_dwn);
			// Just giving a bit of time for monitor to operate power command
			usleep(4000);
			pwr_dwn = 0x1;
			XDp_TxAuxWrite(DpTxSsInst.DpPtr,
								XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &pwr_dwn);
			// Just giving a bit of time for monitor to operate power command
			usleep(4000);

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
		}
	} else {
		xil_printf("Please connect a DP Monitor and try again !!!\r\n");
		return;
	}

	*LineRate_init &= 0xFF;
	*LaneCount_init &= 0xF;
     xil_printf("System capabilities set to: LineRate %x, LaneCount %x\r\n",
											 *LineRate_init,*LaneCount_init);


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
									XVphy_User_Config PHY_User_Config_Table){

	XVphy_PllRefClkSelType QpllRefClkSel;
	XVphy_PllRefClkSelType CpllRefClkSel;
	XVphy_PllType TxPllSelect;
	XVphy_PllType RxPllSelect; // Required for VPHY setting
	XVphy_ChannelId TxChId;
	//XVphy_ChannelId RxChId;
	u8 QuadId = 0;
	u32 Status = XST_FAILURE;
	u32 retries = 0;

	QpllRefClkSel   		= PHY_User_Config_Table.QPLLRefClkSrc;
	CpllRefClkSel   		= PHY_User_Config_Table.CPLLRefClkSrc;
	TxPllSelect             = PHY_User_Config_Table.TxPLL;
	// Required for VPHY setting
	RxPllSelect             = PHY_User_Config_Table.RxPLL;
	TxChId                  = PHY_User_Config_Table.TxChId;



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
		Status = XVphy_ClkInitialize(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX);
		if (retries > 100) {
			retries = 0;
			xil_printf ("exhausted\r\n");
			break;
		}
		retries++;
	}

	XVphy_WriteReg(InstancePtr->Config.BaseAddr,
			XVPHY_PLL_RESET_REG,
			(XVPHY_PLL_RESET_QPLL0_MASK | XVPHY_PLL_RESET_QPLL1_MASK)); // 0x06
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_PLL_RESET_REG, 0x0);
	XVphy_ResetGtPll(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,
																	(FALSE));
//	XVphy_ResetGtPll(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CMN1, XVPHY_DIR_TX,
//																	(FALSE));

	Status = XVphy_WaitForPmaResetDone(InstancePtr, QuadId,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX);
	Status += XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);
	Status += XVphy_WaitForResetDone(InstancePtr, QuadId,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX);
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
u32 start_tx(u8 line_rate, u8 lane_count,user_config_struct user_config){
	XVidC_VideoMode res_table = user_config.VideoMode_local;
	u8 bpc = user_config.user_bpc;
	u8 pat = user_config.user_pattern;

	u32 Status;
    u8 pwr_dwn;
	//Disabling TX and TX interrupts

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
												XDP_TX_INTERRUPT_MASK, 0xFFF);
	pwr_dwn = 0x2;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1,
																	&pwr_dwn);
	// Just giving a bit of time for monitor to operate power command
	usleep(400);
	pwr_dwn = 0x1;
	XDp_TxAuxWrite(DpTxSsInst.DpPtr, XDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1,
																	&pwr_dwn);
	// Just giving a bit of time for monitor to operate power command
	usleep(1000000);


    XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
// Give a bit of time for DP IP after monitor came up and starting Link training
	usleep(100000);
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

	// VTC requires linkup(video clk) before setting values.
	// This is why we need to linkup once to get proper CLK on VTC.
	Status = DpTxSubsystem_Start(&DpTxSsInst, 0);


	// VTC reset here?  << check this

	xil_printf (".");
	//updates required timing values in Video Pattern Generator
	Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
	xil_printf (".");
	// setting video pattern
	Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[pat]);
	xil_printf (".");
	clk_wiz_locked();
	Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
	xil_printf (".");
        Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status != (XST_SUCCESS)) {
		Status = DpTxSubsystem_Start(&DpTxSsInst, 0);
		if (Status != XST_SUCCESS) {
			xil_printf("ERR:DPTX SS start failed\r\n");
			return (XST_FAILURE);
		}
	}
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK, 0x0);

	/*
	 * Initialize CRC
	 */
	/* Reset CRC*/
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
			0x10);
	/* Set Pixel width in CRC engine*/
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR, VIDEO_FRAME_CRC_CONFIG,
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
void clk_wiz_locked() {

	volatile u32 res = Xil_In32 (
			XPAR_TX_SUBSYSTEM_VID_CLK_RST_HIER_AXI_GPIO_0_BASEADDR+0x0);
	while ( res == 0 ) {
		xil_printf ("~/~/");
		res = Xil_In32 (
				XPAR_TX_SUBSYSTEM_VID_CLK_RST_HIER_AXI_GPIO_0_BASEADDR+0x0);
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

void hpd_con(u8 Edid_org[128], u8 Edid1_org[128], u16 res_update)
{

	u32 Status;
	u8 auxValues[9];
	u8 max_cap_new;
	u8 max_cap_lanes_new;
	u8 lane0_sts;
	u8 lane2_sts;
	u8 rd_200;
	u8 dpcd[88];
	u32 htotal_test_hpd;
	u32 vtotal_test_hpd;
	u32 freq_test_hpd;
	u8 good_edid_hpd = 1;
	XVidC_VideoMode VmId_test_hpd;
	XVidC_VideoMode VmId_ptm_hpd;
	u8 bpc_hpd;
	u8 hdcp_capable;
	u8 tmp[12];


	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_STATUS);
	//Enabling TX interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_INTERRUPT_MASK,0xFFF);


	// From here is the requirement per DP spec
	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1,
															&max_cap_new);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1,
															&max_cap_lanes_new);

	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_REV, 12, tmp);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_SINK_COUNT, 6, tmp);


	tx_is_reconnected--;
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_REV, 11, &dpcd);
	XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_SINK_COUNT, 1, &rd_200);
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_STATUS_LANE_0_1, 1, &lane0_sts);
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_STATUS_LANE_2_3, 1, &lane2_sts);
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1, &max_cap_new);
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1,
															&max_cap_lanes_new);
	// check if lane is 1/2/4 or something else
	if(max_cap_lanes_new == 1||max_cap_lanes_new == 2||max_cap_lanes_new == 4)
		;
	else{
		// soemthing wrong. Read again
		XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1,
														&max_cap_lanes_new);
	}


	if (XVidC_EdidIsHeaderValid(Edid_org)) {
		good_edid_hpd = 1;
	}
	else {
		good_edid_hpd = 0;
		debug_DP_printf("\r\nError : EDID header is corrupt...\r\n");
	}

	if (!CalculateChecksum(Edid_org, 128)) {
		good_edid_hpd = 1;
	} else {
		good_edid_hpd = 0;
		debug_DP_printf("\r\nError : EDID checksum is not 0\r\n");
	}
	// Till here is the requirement per DP spec

	// From here is optional per application
	if (good_edid_hpd == 1) {
		htotal_test_hpd = XVidC_EdidGetStdTimingsH(Edid_org, 1);
		vtotal_test_hpd = XVidC_EdidGetStdTimingsV(Edid_org, 1);
		freq_test_hpd   = XVidC_EdidGetStdTimingsFrr(Edid_org, 1);
		XVidC_UnregisterCustomTimingModes();
		VmId_test_hpd = XVidC_GetVideoModeId(htotal_test_hpd, vtotal_test_hpd,
															freq_test_hpd,0);
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


	if (max_cap_new == XDP_TX_LINK_BW_SET_540GBPS
			|| max_cap_new == XDP_TX_LINK_BW_SET_270GBPS
			|| max_cap_new == XDP_TX_LINK_BW_SET_162GBPS) {
		switch(max_cap_new){
			case XDP_TX_LINK_BW_SET_162GBPS:
				Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?0:3]);
				break;

			case XDP_TX_LINK_BW_SET_270GBPS:
				Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?1:4]);
				break;

			default :
				Status = PHY_Configuration_Tx(&VPhyInst,
						PHY_User_Config_Table[(is_TX_CPLL)?2:5]);
				break;
		}

		if (Status != XST_SUCCESS) {
			debug_DP_printf (
			"+++++++ TX GT configuration encountered a failure +++++++\r\n");
		}


		debug_DP_printf ("Training with %x %x\r\n",max_cap_new,
													max_cap_lanes_new&0x1F);
        XDpTxSs_SetLinkRate(&DpTxSsInst, max_cap_new);
		XDpTxSs_SetLaneCount(&DpTxSsInst, max_cap_lanes_new&0x1F);

        if (good_edid_hpd == 1) {
			if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)
							&& (VmId_test_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				XDpTxSs_SetVidMode(&DpTxSsInst, res_update);
			} else if ((VmId_ptm_hpd != XVIDC_VM_NOT_SUPPORTED)) {
				XDpTxSs_SetVidMode(&DpTxSsInst, VmId_ptm_hpd);
			} else {
				XDpTxSs_SetVidMode(&DpTxSsInst, VmId_test_hpd);
			}
        } else {
		XDpTxSs_SetVidMode(&DpTxSsInst, res_update);
        }

        XDpTxSs_SetBpc(&DpTxSsInst, bpc_hpd);
        debug_DP_printf (".");
        XDpTxSs_Start(&DpTxSsInst);
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											XDP_TX_INTERRUPT_MASK, 0x0);
        debug_DP_printf (".");
        Vpg_StreamSrcConfigure(DpTxSsInst.DpPtr, 0, 1);
        debug_DP_printf (".");
        Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr, C_VideoUserStreamPattern[1]);
        Status = XDpTxSs_Start(&DpTxSsInst);


        Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
        if (Status != XST_SUCCESS) {
			Status = XDpTxSs_Start(&DpTxSsInst);
        }
	}



	Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
	if (Status == XST_SUCCESS) {
		// HDCP1.3 related access
		XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x068028, 1, auxValues);
		hdcp_capable = auxValues[0] & 0x1;
		if (hdcp_capable == 1) {

        } else {
		debug_DP_printf (
			"Sink is either repeater enabled or not capable of HDCP\r\n");
        }
	}

	debug_DP_printf ("HPD over\r\n");
}



u8 XUartLite_RecvByte_local(u32 BaseAddress){
	while(XUartLite_IsReceiveEmpty(BaseAddress))
		;
	return (u8)XUartLite_ReadReg(BaseAddress, XUL_RX_FIFO_OFFSET);
}

char inbyte_local(void){
	 return XUartLite_RecvByte_local(STDIN_BASEADDRESS);
}


/*****************************************************************************/
/**
*
* This function to get uart input from user
*
* @param	timeout_ms
*
* @return
*		- received charactor
*
* @note		None.
*
******************************************************************************/
char xil_getc(u32 timeout_ms){
	char c;
	u32 timeout = 0;

	extern XTmrCtr TmrCtr;

	//dbg_printf ("timeout_ms = %x\r\n",timeout_ms);
	// Reset and start timer
	if ( timeout_ms > 0 && timeout_ms != 0xff ){
	  XTmrCtr_Start(&TmrCtr, 0);
	}

	while(XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS) && (timeout == 0)){
		if ( timeout_ms == 0 ){ // no timeout - wait for ever
		   timeout = 0;
		} else if ( timeout_ms == 0xff ) { // no wait - special case
		   timeout = 1;
		} else if(timeout_ms > 0){
			if(XTmrCtr_GetValue(&TmrCtr, 0)
										> ( timeout_ms * (100000000 / 1000) )){
				timeout = 1;
			}
		}
	}
	if(timeout == 1){
		c = 0;
	} else {
		c = XUartLite_RecvByte(STDIN_BASEADDRESS);
	}

	return c;
}


/*****************************************************************************/
/**
*
* This function to convert integer to hex
*
* @param	timeout_ms
*
* @return
*		- received charactor
*
* @note		None.
*
******************************************************************************/
u32 xil_gethex(u8 num_chars){
	u32 data;
	u32 i;
	u8 term_key;
	data = 0;

	for(i=0;i<num_chars;i++){
		term_key = xil_getc(0);
		xil_printf("%c",term_key);
		if(term_key >= 'a') {
			term_key = term_key - 'a' + 10;
		} else if(term_key >= 'A') {
				term_key = term_key - 'A' + 10;
		} else {
			term_key = term_key - '0';
		}
		data = (data << 4) + term_key;
	}
	return data;
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

	//Write #2
	db1 = xilInfoFrame->audio_channel_count
					| (xilInfoFrame->audio_coding_type<<4) | (RSVD<<3);
	db2 = (RSVD<<5)| (xilInfoFrame->sampling_frequency<<2)
					| xilInfoFrame->sample_size;
	db3 = RSVD;
	db4 = xilInfoFrame->channel_allocation;
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);

	//Write #3
	db1 = (xilInfoFrame->level_shift<<3) | RSVD
								| (xilInfoFrame->downmix_inhibit <<7);
	db2 = RSVD;
	db3 = RSVD;
	db4 = RSVD;
	temp = db4<<24|db3<<16|db2<<8|db1;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);

	//Write #4
	db1 = RSVD;
	db2 = RSVD;
	db3 = RSVD;
	db4 = RSVD;
	temp = 0x00000000;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_AUDIO_INFO_DATA(1), temp);
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


	(void)memset((void *)&Timing, 0, sizeof(XVidC_VideoTiming));

	Ptm = &EdidPtr[XDP_EDID_PTM];

	bpp = XVidC_EdidGetColorDepth(EdidPtr) *3;

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

//	debug_DP_printf(XDBG_DEBUG_GENERAL,"SS INFO:"
//								"HAct:%d, VAct:%d, FR:%d\r\n", Timing.HActive,
//												Timing.VActive, FrameRate);

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
		xil_printf ("Unsupported resolution\r\n");

	} else {
		/* Get video mode ID */
		VmId = XVidC_GetVideoModeId(Timing.HActive, Timing.VActive,
							FrameRate, XVidC_EdidIsDtdPtmInterlaced(EdidPtr));

	}

	return VmId;
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
void ReportVideoCRC()
 {
	xil_printf ("==========Frame CRC===========\r\n");
	xil_printf ("CRC Cfg     =  0x%x\r\n",XDp_ReadReg(
			XPAR_TX_SUBSYSTEM_CRC_BASEADDR,VIDEO_FRAME_CRC_CONFIG));
	xil_printf ("CRC - R/Y   =  0x%x\r\n",XDp_ReadReg(
			XPAR_TX_SUBSYSTEM_CRC_BASEADDR,VIDEO_FRAME_CRC_VALUE_G_R)&0xFFFF);
	xil_printf ("CRC - G/Cr  =  0x%x\r\n",XDp_ReadReg(
			XPAR_TX_SUBSYSTEM_CRC_BASEADDR,VIDEO_FRAME_CRC_VALUE_G_R)>>16);
	xil_printf ("CRC - B/Cb  =  0x%x\r\n",XDp_ReadReg(
			XPAR_TX_SUBSYSTEM_CRC_BASEADDR,VIDEO_FRAME_CRC_VALUE_B)&0xFFFF);
 }


/*****************************************************************************/
/**
*
* This function Calculates CRC values of Video components
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void CalculateCRC(u8 line_rate, u8 lane_count)
{

	/*Set pixel mode as per lane count - it is default behavior
	  User has to adjust this accordingly if there is change in
	  pixel width programming
	 */
	XDp_WriteReg(XPAR_TX_SUBSYSTEM_CRC_BASEADDR,
							VIDEO_FRAME_CRC_CONFIG, lane_count);

}
