/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xdprxss_zcu102_rx.h
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00  Kei  08/09/17 First Release
 *</pre>
 *
*****************************************************************************/

#ifndef SRC_XDPRXSS_ZCU102_RX_H_
#define SRC_XDPRXSS_ZCU102_RX_H_

/***************************** Include Files *********************************/

#include "xdptxss.h"
#include "xdprxss.h"
#include "xvphy.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xscugic.h"
#include "xuartps_hw.h"
#include "xiic.h"
#include "xiic_l.h"
#include "xtmrctr.h"
//#include "xspi.h"
#include "xvidc_edid.h"
#include "sleep.h"
#include "stdlib.h"
#include "xvidframe_crc.h"
#include "clk_set.h"
#include "xiicps.h"
#include "videofmc_defs.h"
#include "xv_frmbufrd_l2.h"
#include "xv_frmbufwr_l2.h"
#include "xaxis_switch.h"
#include "xv_axi4s_remap.h"

/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DPTXSS_0_VEC_ID

#define XINTC_DPRXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DPRXSS_0_VEC_ID


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
#define XDPRXSS_DEVICE_ID		XPAR_DPRXSS_0_DEVICE_ID

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
#define IIC_DEVICE_ID       XPAR_IIC_0_DEVICE_ID
#define XVPHY_DEVICE_ID		XPAR_VPHY_0_DEVICE_ID
#define FRMBUF_RD_DEVICE_ID  XPAR_XV_FRMBUFRD_0_DEVICE_ID
#define FRMBUF_WR_DEVICE_ID  XPAR_XV_FRMBUFWR_0_DEVICE_ID

#define SET_TX_TO_2BYTE		\
		(XPAR_XDP_0_GT_DATAWIDTH/2)

#define SET_RX_TO_2BYTE		\
		(XPAR_XDP_0_GT_DATAWIDTH/2)


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

#define PE_VS_ADJUST 1
#define DP141_ADJUST 0

#define XVPHY_DRP_CPLL_FBDIV		0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV	0x2A
#define XVPHY_DRP_RXOUT_DIV			0x63
#define XVPHY_DRP_RXCLK25			0x6D
#define XVPHY_DRP_TXCLK25			0x7A
#define XVPHY_DRP_TXOUT_DIV			0x7C

u32 XVFRMBUFRD_BUFFER_BASEADDR;
u32 XVFRMBUFWR_BUFFER_BASEADDR;

/***************** Macros (Inline Functions) Definitions *********************/


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



typedef struct
{
	 XVidC_VideoMode VideoMode_local;
	unsigned char user_bpc;
	unsigned char user_pattern;
	unsigned char user_format;
	unsigned int user_numStreams;
	unsigned int user_stream_number;
	unsigned int mst_check_flag;
}user_config_struct;


typedef struct
{
        unsigned char lane_count;
        unsigned char link_rate;
}lane_link_rate_struct;


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
} XilAudioInfoFrame_rx;


/*The structure defines Generic Frame Packet fields*/
typedef struct
{
        u32 frame_count;
        u32 frame_count_q;
        u8 Header[4];
        u8 Payload[32];
} XilAudioExtFrame;

XilAudioInfoFrame_rx AudioinfoFrame;
XilAudioExtFrame  SdpExtFrame;
XilAudioExtFrame  SdpExtFrame_q;


/************************** Function Prototypes ******************************/
void hpd_con(XDpTxSs *InstancePtr, u8 Edid_org[128], u8 Edid1_org[128], u16 res_update);
void hpd_pulse_con(XDpTxSs *InstancePtr);
char xil_getc(u32 timeout_ms);
static u32 xil_gethex(u8 num_chars);
void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame);
void Vpg_Audio_start(void);
void Vpg_Audio_stop(void);
u32 start_tx(u8 line_rate, u8 lane_count, user_config_struct user_config,
			XDpTxSs_MainStreamAttributes Msa[4]);
u32 start_tx_only(u8 line_rate, u8 lane_count, user_config_struct user_config);
u32 PHY_Configuration_Tx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table);
u32 set_vphy(int LineRate_init_tx);

void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void sink_power_down(void);
void sink_power_up(void);
u8 get_LineRate(void);
u8 get_Lanecounts(void);
void sink_power_cycle(void);
int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress, u8 Value);
void DpPt_pe_vs_adjustHandler(void *InstancePtr);
int VideoFMC_Init(void);
int IDT_8T49N24x_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress, int FIn, int FOut, u8 FreeRun);
int IDT_8T49N24x_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);

/************************** Variable Definitions *****************************/
XV_FrmbufRd_l2     frmbufrd;
XV_FrmbufWr_l2     frmbufwr;
XAxis_Switch axis_switch;
XAxis_Switch axis_switch_tx;
XDpRxSs DpRxSsInst;	/* The DPTX Subsystem instance.*/
XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
XIic IicInstance;	/* I2C bus for Si570 */
XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
XScuGic IntcInst;
XVphy VPhyInst;	/* The DPRX Subsystem instance.*/
XTmrCtr TmrCtr; /* Timer instance.*/
Video_CRC_Config VidFrameCRC;

int tx_is_reconnected; /*This variable to keep track of the status of Tx link*/
u8 prev_line_rate; /*This previous line rate to keep previous info to compare
						with new line rate request*/
u8 hpd_pulse_con_event; /*This variable triggers hpd_pulse_con*/

u8 num_sinks;

#define XDP_RX_CRC_CONFIG       0x074
#define XDP_RX_CRC_COMP0        0x078
#define XDP_RX_CRC_COMP1        0x07C
#define XDP_RX_CRC_COMP2        0x080
#define XDP_RX_DPC_LINK_QUAL_CONFIG 0x454
#define XDP_RX_DPC_L01_PRBS_CNTR    0x45C
#define XDP_RX_DPC_L23_PRBS_CNTR    0x460
#define XDP_RX_DPCD_LINK_QUAL_PRBS  0x3

#define XVPHY_DRP_RX_DATA_WIDTH     0x03
#define XVPHY_DRP_RX_INT_DATA_WIDTH 0x66


#define DP_BS_IDLE_TIMEOUT      0x77359400
#define AUX_DEFER_COUNT         6
/* DEFAULT VALUE=0. Enabled programming of
 *Rx Training Algo Register for Debugging Purpose
 */
#define LINK_TRAINING_DEBUG     0

typedef struct {
        u8  MinVoltageSwing;
        u8  ClockRecoveryOption;
        u16 VswingLoopCount;
        u16 SetVswing;
        u16 ChEqOption;
        u8  SetPreemp;
        u8  Itr1Premp;
        u8  Itr2Premp;
        u8  Itr3Premp;
        u8  Itr4Premp;
        u8  Itr5Premp;
} DP_Rx_Training_Algo_Config;


DP_Rx_Training_Algo_Config RxTrainConfig;

#define REMAP_RX_BASEADDR  XPAR_DP_RX_HIER_0_REMAP_RX_S_AXI_CTRL_BASEADDR
#define REMAP_TX_BASEADDR  XPAR_DP_TX_HIER_0_REMAP_TX_S_AXI_CTRL_BASEADDR
#define REMAP_RX_DEVICE_ID  XPAR_DP_RX_HIER_0_REMAP_RX_DEVICE_ID
#define REMAP_TX_DEVICE_ID  XPAR_DP_TX_HIER_0_REMAP_TX_DEVICE_ID

XV_axi4s_remap_Config   *rx_remap_Config;
XV_axi4s_remap          rx_remap;
XV_axi4s_remap_Config   *tx_remap_Config;
XV_axi4s_remap          tx_remap;

XDpTxSs_Config *ConfigPtr;
XDpRxSs_Config *ConfigPtr_rx;


#define VBLANK_WAIT_COUNT       100

//#define DPRXSS_LINK_RATE        XDPRXSS_LINK_BW_SET_810GBPS
#define DPRXSS_LINK_RATE        XDPRXSS_LINK_BW_SET_540GBPS
#define DPRXSS_LANE_COUNT        XDPRXSS_LANE_COUNT_SET_4

/* Interrupt helper functions */
void DpPt_HpdEventHandler(void *InstancePtr);
void DpPt_HpdPulseHandler(void *InstancePtr);
void DpPt_LinkrateChgHandler (void *InstancePtr);
void DpPt_pe_vs_adjustHandler(void *InstancePtr);
void DpPt_TxSetMsaValuesImmediate(void *InstancePtr);

void DpRxSs_PowerChangeHandler(void *InstancePtr);
void DpRxSs_NoVideoHandler(void *InstancePtr);
void DpRxSs_VerticalBlankHandler(void *InstancePtr);
void DpRxSs_TrainingLostHandler(void *InstancePtr);
void DpRxSs_VideoHandler(void *InstancePtr);
void DpRxSs_InfoPacketHandler(void *InstancePtr);
void DpRxSs_ExtPacketHandler(void *InstancePtr);
void DpRxSs_TrainingDoneHandler(void *InstancePtr);
void DpRxSs_UnplugHandler(void *InstancePtr);
void DpRxSs_LinkBandwidthHandler(void *InstancePtr);
void DpRxSs_PllResetHandler(void *InstancePtr);
void DpRxSs_BWChangeHandler(void *InstancePtr);
void DpRxSs_AccessLaneSetHandler(void *InstancePtr);
void DpRxSs_AccessLinkQualHandler(void *InstancePtr);
void DpRxSs_AccessErrorCounterHandler(void *InstancePtr);
void DpRxSs_CRCTestEventHandler(void *InstancePtr);
void XDp_RxInterruptDisable1(XDp *InstancePtr, u32 Mask);
void XDp_RxInterruptEnable1(XDp *InstancePtr, u32 Mask);
void DpRxSs_InfoPacketHandler(void *InstancePtr);
void DpRxSs_ExtPacketHandler(void *InstancePtr);
void Dprx_InterruptHandlerDownReq(void *InstancePrt);
void Dprx_InterruptHandlerDownReply(void *InstancePrt);
void Dprx_InterruptHandlerPayloadAlloc(void *InstancePrt);
void Dprx_InterruptHandlerActRx(void *InstancePtr);
void Print_InfoPkt();
void Print_ExtPkt();
u32 DpRxSs_Setup(void);
void resetIp();
void power_down_HLSIPs(void);
void power_up_HLSIPs(void);
void bufferWr_callback(void *InstancePtr);
void Dprx_DetectResolution(void *InstancePtr, u16 offset);
void Dppt_DetectResolution(void *InstancePtr, u16 offset,
							XDpTxSs_MainStreamAttributes Msa[4]);
void remap_start(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);
void Dprx_ResetVideoOutput(void *InstancePtr);
u32 Dp_SetupIntrSystem(void);
u32 DpTxSs_VideoPhyInit(u16 DeviceId);
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr,
			XDpTxSs_MainStreamAttributes Msa[4]);
void DpTxSs_Setup(u8 *LineRate_init, u8 *LaneCount_init,
										u8 Edid_org[128], u8 Edid1_org[128]);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Tx_to_two_byte, u8 Rx_to_two_byte);
void clk_wiz_locked(void);
void hpd_pulse_con(XDpTxSs *InstancePtr);
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
//void LMK04906_init(XSpi *SPI_LMK04906);
static u8 CalculateChecksum(u8 *Data, u8 Size);
XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
void ReportVideoCRC(void);
extern void main_loop(void);
extern void pt_loop(void);
void XVphy_SetTxPreEmphasis(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		u8 Pe);
void XVphy_SetTxVoltageSwing(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u8 Vs);

#endif /* SRC_XDPRXSS_ZCU102_RX_H_ */
