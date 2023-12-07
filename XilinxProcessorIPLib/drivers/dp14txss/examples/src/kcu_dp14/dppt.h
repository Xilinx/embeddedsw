/*******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file dppt.h
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI    07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#ifndef DPAPP_H_
#define DPAPP_H_

#include <stdio.h>
#include <math.h>
#include "xiic.h"
#include "xiic_l.h"
#include "xbasic_types.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xgpio.h"
#include "xuartlite.h"
#include "xuartlite_l.h"
#include "xtmrctr.h"
#include "xintc.h"
#include "xvphy.h"
#include "xvphy_i.h"
#include "xvidc_edid.h"
#include "xvidc.h"
#include "xdptxss.h"
#include "xdprxss.h"
#include "xdp.h"
#include "stdlib.h"
#ifndef SDT
#include "microblaze_sleep.h"
#endif
#include "xvid_pat_gen.h"

#include "idt_8t49n24x.h"
#include "ti_lmk03318.h"
#include "videofmc_defs.h"

#include "xdebug.h"

#include "xvid_pat_gen.h"
#include "xlib_string.h"
#include "xedid_print_example.h"

#include "xaxivdma.h"
#include "dppt_vdma.h"
#include "dppt_vid_phy_config.h"
#include "xvidframe_crc.h"
#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#include "xv_axi4s_remap.h"
#endif
typedef struct
{
	 XVidC_VideoMode VideoMode_local;
	unsigned char user_bpc;
	unsigned char *user_pattern;
	unsigned char user_format;
	unsigned int user_numStreams;
	unsigned int user_stream_number;
	unsigned int mst_check_flag;
}user_config_struct;
extern user_config_struct user_config;

typedef unsigned int    UINT32;

#define UART_BASEADDR 	\
	XPAR_PROCESSOR_SUBSYSTEM_INTERCONNECT_AXI_UARTLITE_1_BASEADDR
#define TIMER_BASEADDR		XPAR_TMRCTR_0_BASEADDR
#define VIDPHY_BASEADDR		XPAR_VID_PHY_CONTROLLER_0_BASEADDR
#define CLK_2_GPIO_BASEADDR	XPAR_AXI_GPIO_0_BASEADDR

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPTXSS_DP_INTERRUPT_ID  	\
	XPAR_INTC_0_DPTXSS_0_DPTXSS_DP_IRQ_VEC_ID
#else
#define XINTC_DPTXSS_DP_INTERRUPT_ID  	\
	XPAR_INTC_0_DP14TXSS_0_VEC_ID
#endif

#define XINTC_DPRXSS_DP_INTERRUPT_ID  	\
	XPAR_INTC_0_DP14RXSS_0_DPRXSS_DP_IRQ_VEC_ID
#define XINTC_DPRXSS_IIC_INTERRUPT_ID  	\
	XPAR_INTC_0_DPRXSS_0_DPRXSS_IIC_IRQ_VEC_ID
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID
#define XINTC_DEVICE_ID 	XPAR_INTC_0_DEVICE_ID
#define XDPTXSS_DEVICE_ID 	XPAR_DPTXSS_0_DEVICE_ID
#define IIC_DEVICE_ID 		\
	XPAR_PROCESSOR_SUBSYSTEM_INTERCONNECT_AXI_IIC_1_DEVICE_ID

#define XINTC_TIMER_0 		XPAR_INTC_0_TMRCTR_0_VEC_ID
#define XINTC_HDCP_TIMER_ID	\
	XPAR_INTC_0_DPRXSS_0_DPRXSS_TIMER_IRQ_VEC_ID
#define XINTC_IIC_ID 		XPAR_INTC_0_IIC_0_VEC_ID

#define TIMER_RESET_VALUE 		1000
#define TIMER_HDCP_STABLIZATION_VALUE	100000000

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#define REMAP_RX_BASEADDR  XPAR_XV_AXI4S_REMAP_0_S_AXI_CTRL_BASEADDR
#define REMAP_TX_BASEADDR  XPAR_XV_AXI4S_REMAP_1_S_AXI_CTRL_BASEADDR
#define REMAP_RX_DEVICE_ID  XPAR_XV_AXI4S_REMAP_0_DEVICE_ID
#define REMAP_TX_DEVICE_ID  XPAR_XV_AXI4S_REMAP_1_DEVICE_ID
#endif
#define CLK_WIZ_BASE  XPAR_DP_TX_HIER_VID_CLK_RST_HIER_CLK_WIZ_0_BASEADDR

#define XPAR_VIDEO_FRAME_CRC_TX_BASEADDR \
				XPAR_DP_TX_HIER_VIDEO_FRAME_CRC_TX_BASEADDR
#define XPAR_VIDEO_FRAME_CRC_RX_BASEADDR \
				XPAR_DP_RX_HIER_VIDEO_FRAME_CRC_RX_BASEADDR
#define XPAR_AV_PAT_GEN_0_BASEADDR  XPAR_DP_TX_HIER_AV_PAT_GEN_0_BASEADDR
#define GPIO_CLK_BASEADDR XPAR_DP_TX_HIER_VID_CLK_RST_HIER_AXI_GPIO_0_BASEADDR

#ifndef SDT
#define HLS_RESET XPAR_GPIO_1_BASEADDR
#else
#define HLS_RESET XPAR_XGPIO_1_BASEADDR
#endif

/* Change to 1 to enable HDCP in design. */
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define ENABLE_HDCP_IN_DESIGN		1
#else
#define ENABLE_HDCP_IN_DESIGN		0
#endif

#define ENABLE_HDCP_FLOW_GUIDE		0

/* Setting to 1 enables the auto switchover 
 * from RX to TX when cable is unplugged
 * Setting to 0 enables the RX only mode 
 * TX is used to display the video
 * */
#define FOR_INTERNAL                	1

/* Setting Just RX to 1 does not start the TX */
#define JUST_RX                     	0

/* This is reserved and should not be changed */
#define COMPLIANCE                  	0

/* The application by default uses the EDID 
 * of the downstream monitor
 * Setting this to 0 will enable an internal EDID
 * */
//#define USE_MONITOR_EDID 				1

/* Set CAP_OVER_RIDE to 1 to limit the 
 * capabilities to a particular value
 * */
#define CAP_OVER_RIDE 			0
#define MAX_RATE 			0x1E
#define MAX_LANE 			0x4

#define PE_VS_ADJUST	1
#define DP141_ADJUST    0

/* Audio feature in untested. Do not change this value */
#define ENABLE_AUDIO                	1

/* Bypass the vid_common timings
 * Not all resolutions are supported in vid_common library 
 * bypassing the vid_common lets us transmit video on TX 
 * that is not supported.
 * However it is still possible that all resolutions 
 * may not get displayed properly.
 * */
//#define BYPASS_VID_COMMON 				1
#define DEBUG_MAIN_FLOW					0

#define EEPROM_TEST_START_ADDRESS       0x80
#define IIC_SI570_ADDRESS  		0x5D
#define IIC_SWITCH_ADDRESS 		0x74
#define PAGE_SIZE       		16
#define NUM_MODES 			7
#define NUM_CLOCK_REGS 			6
#define PROG_48_KHZ_MODE   		0

#ifndef SDT
#define SET_TX_TO_2BYTE	(XPAR_DP_RX_HIER_V_DP_RXSS1_0_DP_GT_DATAWIDTH/2)
#define SET_RX_TO_2BYTE	(XPAR_DP_TX_HIER_V_DP_TXSS1_0_DP_GT_DATAWIDTH/2)
#else
#define SET_TX_TO_2BYTE	(XPAR_DP_TX_HIER_V_DP_TXSS1_0_DP_GT_DATA_WIDTH/2)
#define SET_RX_TO_2BYTE	(XPAR_DP_RX_HIER_V_DP_RXSS1_0_DP_GT_DATA_WIDTH/2)
#endif
#define BUFFER_BYPASS            XPAR_VID_PHY_CONTROLLER_0_TX_BUFFER_BYPASS


//#define is_TX_CPLL 0

#define XDP_RX_CRC_CONFIG       0x074
#define XDP_RX_DPC_LINK_QUAL_CONFIG 0x454
#define XDP_RX_DPC_L01_PRBS_CNTR    0x45C
#define XDP_RX_DPC_L23_PRBS_CNTR    0x460
#define XDP_RX_DPCD_LINK_QUAL_PRBS  0x3
#define XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER 0x15E
#define XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER 0x15F

/* Enable this only for PHY compliance test */
#define AccessErrorCounterHandler_FLAG 0
/* ************************************************************************** */

int DPPtIntrInitialize();
int DpPt_SetupIntrSystem();
void DpPt_HpdEventHandler(void *InstancePtr);
void DpPt_HpdPulseHandler(void *InstancePtr);
void DpPt_LinkrateChgHandler (void *InstancePtr);
void DpPt_pe_vs_adjustHandler(void *InstancePtr);
void DpPt_TxSetMsaValuesImmediate(void *InstancePtr);
void Dprx_InterruptHandlerPllReset(void *InstancePtr);
void Dprx_InterruptHandlerLinkBW(void *InstancePtr);
void Dprx_InterruptHandlerTrainingDone(void *InstancePtr);
void Dprx_InterruptHandlerBwChange(void *InstancePtr);
void Dprx_InterruptHandlerInfoPkt(void *InstancePtr);
void Dprx_InterruptHandlerExtPkt(void *InstancePtr);
void Dprx_InterruptHandlerUplug(void *InstancePtr);
void Dprx_InterruptHandlerPwr(void *InstancePtr);
u8 get_LineRate(void);

void DpRxSs_AccessLaneSetHandler(void *InstancePtr);
void DpRxSs_AccessLinkQualHandler(void *InstancePtr);
void DpRxSs_AccessErrorCounterHandler(void *InstancePtr);
void DpRxSs_CRCTestEventHandler(void *InstancePtr);
void DpRxSs_InfoPacketHandler(void *InstancePtr);
void DpRxSs_ExtPacketHandler(void *InstancePtr);

void Dprx_HdcpAuthCallback(void *InstancePtr);
void Dprx_HdcpUnAuthCallback(void *InstancePtr);
void DpPt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
int DPTxInitialize();
int DPRxInitialize();
int init_peripherals();
void hpd_con();
void hpd_pulse_con();
void start_tx(u8 line_rate, u8 lane_count, XVidC_VideoMode res_table,
						u8 bpc, u8 pat, u8 pat_update);
void prog_bb(u8 bw, u8 is_tx);
void vdma_stop();
void vdma_start();

//XVidC_VideoMode GetPreferredVm(u8 *EdidPtr, u8 cap, u8 lane);
void phy_cmpl (char CommandKey);

/* Introduced to address reduced blanking 
 * linereset issue from 16.4 release
 * */
u32 DpTxSubsystem_Start(XDpTxSs *, int );

char GetInbyte(void);
char inbyte_local(void);

void Dprx_SetupTxWithCustomMsa(void *InstancePtr, u8 tx_with_msa);
void Dprx_SetupTx(void *InstancePtr, u8 tx_with_msa, XVidC_VideoMode VmId);
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
void format_help_menu(void);
int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		    u16 RegisterAddress, u8 Value);
u8 i2c_read_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		  u16 RegisterAddress);
void DP141_init(u32 I2CBaseAddress, u8 I2CSlaveAddress);

u32 DpTxSubsystem_Start(XDpTxSs *InstancePtr, int with_msa);
void edid_change(int page);
void edid_default();
int VideoFMC_Init(void);
int write_si570();
void I2C_Scan(u32 BaseAddress);
void select_rx_quad();
void read_DP141();

void video_change_detect(u32 *count_track, u32 *rxMsamisc0_track,
		u32 *bpc_track, u32 *recv_clk_freq_track,
		float *recv_frame_clk_track,
		u32 *recv_frame_clk_int_track, int *track_count);
void detect_rx_video_and_startTx(int *track_count1);
void switch_to_Tx_only(int *track_switch, u8 *pwr_dwn_x);
void start_audio_passThrough();
void sink_power_cycle(u32 power_down_time);
void Vpg_Audio_start(void);
void Vpg_Audio_stop();

void power_down_HLSIPs(void);
void power_up_HLSIPs(void);
void remap_start(struct dma_chan_parms *dma_struct);
void resetIp();
void update_edid();
void Print_InfoPkt();
void Print_ExtPkt();

#endif /* DPAPP_H_ */
