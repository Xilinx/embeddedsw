/*******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file main.h
*
* This file contains a design example using the XDpRxSs driver in single stream
* (SST) transport mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 vk 10/04/17 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <xdp.h>
#include <xdp_hw.h>
#include <xdprxss.h>
#include <xdptxss.h>
#include <xdprxss_mcdp6000.h>
#ifdef versal
#include "xuartpsv_hw.h"
#include "xiicps.h"
#include "xclk_wiz.h"
#endif
//#ifndef versal
#include <xiic.h>
#include <xiic_l.h>
//#endif
#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xparameters.h>
#include <xstatus.h>
#include <xtmrctr.h>
#ifndef versal
#include <xuartps_hw.h>
#include <xvphy.h>
#include <xvphy_dp.h>
#include <xvphy_hw.h>
#endif
#include "sleep.h"
#include <xil_cache.h>

#include "xvidframe_crc.h"

#ifdef SDT
#define XPAR_XV_FRMBUFRD_NUM_INSTANCES XPAR_XV_FRMBUF_RD_NUM_INSTANCES
#define XPAR_XV_FRMBUFWR_NUM_INSTANCES XPAR_XV_FRMBUF_WR_NUM_INSTANCES
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xintc.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#endif /* XPAR_INTC_0_DEVICE_ID */

#include "xiicps.h"
#include "videofmc_defs.h"
#include "idt_8t49n24x.h"

#ifndef SDT
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
#include "xv_frmbufrd_l2.h"
#define FRMBUF_RD_DEVICE_ID  XPAR_XV_FRMBUFRD_0_DEVICE_ID
#endif
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
#include "xv_frmbufwr_l2.h"
#define FRMBUF_WR_DEVICE_ID  XPAR_XV_FRMBUFWR_0_DEVICE_ID
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#include "xv_axi4s_remap.h"
#endif
#else
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
#include "xv_frmbufrd_l2.h"
#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
#include "xv_frmbufwr_l2.h"
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#include "xv_axi4s_remap.h"
#endif
#endif

#include "xi2stx.h"
#include "xi2srx.h"
#include "xgpio.h"
#include "xaxis_switch.h"

#define Tx
#define Rx
#define PT
#define LB
/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#ifndef SDT
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DP14TXSS_0_VEC_ID

#define XINTC_DPRXSS_DP_INTERRUPT_ID \
    XPAR_FABRIC_DP14RXSS_0_VEC_ID

#define XINTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC                    XScuGic
#define XINTC_HANDLER            XScuGic_InterruptHandler

/* The unique device ID of the instances used in example
 */
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID

#ifndef versal
#define XVPHY_DEVICE_ID 	XPAR_VPHY_0_DEVICE_ID
#endif

#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID
#endif
#define VIDEO_CRC_BASEADDR 	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR 	XPAR_PSU_UART_0_BASEADDR
#define VIDPHY_BASEADDR 	XPAR_VPHY_0_BASEADDR
#define VID_EDID_BASEADDR 	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR
#define IIC_DEVICE_ID 		XPAR_IIC_0_DEVICE_ID

#ifdef versal
#define IIC_BASE_ADDR 					0
#else
#define IIC_BASE_ADDR 					XPAR_IIC_0_BASEADDR
#endif

#define VIDEO_FRAME_CRC_TX_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR \
			XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#ifndef SDT
#define REMAP_RX_BASEADDR  XPAR_DP_RX_HIER_0_REMAP_RX_S_AXI_CTRL_BASEADDR
#define REMAP_TX_BASEADDR  XPAR_DP_TX_HIER_0_REMAP_TX_S_AXI_CTRL_BASEADDR
#define REMAP_RX_DEVICE_ID  XPAR_DP_RX_HIER_0_REMAP_RX_DEVICE_ID
#define REMAP_TX_DEVICE_ID  XPAR_DP_TX_HIER_0_REMAP_TX_DEVICE_ID
#endif
#define RX_ACR_ADDR XPAR_DP_RX_HIER_0_RX_ACR_BASEADDR
#ifndef SDT
#define AXI_SYSTEM_CLOCK_FREQ_HZ \
			XPAR_PROCESSOR_HIER_0_AXI_TIMER_0_CLOCK_FREQ_HZ
#else
#define AXI_SYSTEM_CLOCK_FREQ_HZ \
XPAR_PROCESSOR_HIER_0_AXI_TIMER_0_CLOCK_FREQUENCY
#endif
/* DP Specific Defines
 */
#ifndef SDT
#define SET_TX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATAWIDTH/2)
#define SET_RX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATAWIDTH/2)
#else
#define SET_TX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATA_WIDTH/2)
#define SET_RX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATA_WIDTH/2)
#endif
#define XDP_RX_CRC_CONFIG       0x074
#define XDP_RX_CRC_COMP0        0x078
#define XDP_RX_CRC_COMP1        0x07C
#define XDP_RX_CRC_COMP2        0x080
#define XDP_RX_DPC_LINK_QUAL_CONFIG 0x454
#define XDP_RX_DPC_L01_PRBS_CNTR    0x45C
#define XDP_RX_DPC_L23_PRBS_CNTR    0x460
#define XDP_RX_DPCD_LINK_QUAL_PRBS  0x3

#define TX_BUFFER_BYPASS XPAR_VID_PHY_CONTROLLER_0_TX_BUFFER_BYPASS
#define XVPHY_DRP_PROGDIV           0x3E
#define DIVIDER_162                 57423
#define DIVIDER_270                 57415
#define DIVIDER_540                 57442
#define DIVIDER_810                 57440

#define XVPHY_GTHE4_PREEMP_DP_L0    0x3
#define XVPHY_GTHE4_PREEMP_DP_L1    0xD
#define XVPHY_GTHE4_PREEMP_DP_L2    0x16
#define XVPHY_GTHE4_PREEMP_DP_L3    0x1D

#define XVPHY_GTHE4_DIFF_SWING_DP_V0P0 0x1
#define XVPHY_GTHE4_DIFF_SWING_DP_V0P1 0x2
#define XVPHY_GTHE4_DIFF_SWING_DP_V0P2 0x5
#define XVPHY_GTHE4_DIFF_SWING_DP_V0P3 0xB

#define XVPHY_GTHE4_DIFF_SWING_DP_V1P0 0x2
#define XVPHY_GTHE4_DIFF_SWING_DP_V1P1 0x5
#define XVPHY_GTHE4_DIFF_SWING_DP_V1P2 0x7

#define XVPHY_GTHE4_DIFF_SWING_DP_V2P0 0x4
#define XVPHY_GTHE4_DIFF_SWING_DP_V2P1 0x7

#define XVPHY_GTHE4_DIFF_SWING_DP_V3P0 0x8

#define GT_RST_HOLD_MASK 0x80000000
#define GT_RST_MASK 0x00000001
#define GT_VSWING_MASK 0x00001F00
#define GT_POSTCUR_MASK 0x007C0000
#define GT_LANE_MASK 0x00000070
#define GT_RATE_MASK 0x00000006
#define GT_PLL_MASK 0x00000008

#define SINGLE_LANE 					0x00000011
#define ALL_LANE						0x0000003F
#define VERSAL_810G                     3
#define VERSAL_540G                     2
#define VERSAL_270G                     1
#define VERSAL_162G                     0

#ifdef versal
#define XVPHY_DEVICE_ID					0
#define GT_QUAD_BASE                    XPAR_GT_QUAD_GTWIZ_VERSAL_0_BASEADDR
#define CH1CLKDIV_REG					0x3694
#define DIV 							0x00000260
#define	DIV3 						    0x00000278
#define DIV_MASK 						0x000003FF

/* In Versal, GT, DP is implemented in RAW16 mode and requires
 * two clocks. One is /16 and other is /20
 * In case of RX, the /20 clock is derived using MMCM
 * In case of TX, the /20 clock is derived from txch1outclk by
 * modifying GT parameter.
 * If the hardware is of 1 lane, then /20 clock for TX also has
 * to be derived using MMCM similar to the RX.
 */

#define VERSAL_FABRIC_8B10B             1
#endif

/* The following FLAG enables the Adaptive Sync feature of the
 * DP RX and TX. Set ADAPTIVE to 0 if ADAPTIVE sync is not
 * needed.
 */
#define ADAPTIVE 1

/* The following determines the type of ADAPTIVE sync mechanism
 * in the system.
 * 1 : TX will adapt to the incoming rate. FrameBuffer Read is triggered
 * 		every time FrameBuffer Write is completed. Here, TX will always
 * 		be in Adaptive Sync mode, even if the received video is of
 * 		constant rate.
 * 0 : TX will be in guided mode. it will stretch by the amount
 *      specified by the user. This amount is determined by the information
 *      provided by the DP RX on every frame. FrameBuffer Read is in
 *      auto-restart mode. This mode is interrupt intensive.
 *      In this mode, interrupt is enabled on RX. This interrupt is
 *      asserted when RX detects any change in rate
 */
#define ADAPTIVE_TYPE (0 * ADAPTIVE)

/* This value determines the amount by which the Vertical Front Porch is
 * stretched. When Adaptive Mode is 0x1, this specifies the Max limit of
 * stretching.
 * In case of Mode 0, user should manually program the amount of stretch
 * in the VTC
 */
#define DPTXSS_VFP_STRETCH 0xFFF

/*
 * User can tune these variables as per their system
 */
// Set PHY_COMP to 1 when doing the PHY and LL compliance
// For normal operation, this needs to be set to 0
#define PHY_COMP 0
#define EDID_1_ENABLED !PHY_COMP

/*Max timeout tuned as per tester - AXI Clock=100 MHz
 *Some GPUs may need larger value, So user may tune if needed
 */
#define DP_BS_IDLE_TIMEOUT      (0x047868C0*!PHY_COMP)+(0x091FFFFF*PHY_COMP)
#define VBLANK_WAIT_COUNT       50
//Wait for Following number of infoframes before asserting info
//frame captured
#define AUD_INFO_COUNT          20
//Set this to start the audio only after receiving infoframes
#define WAIT_ON_AUD_INFO          1

/*For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
  For Interop, set this to 6.
*/
#define AUX_DEFER_COUNT         (6)//+(2*PHY_COMP))
#define AUX_DEFER_COUNT_PHY     1
#define PRBS_ERRCNTR_CLEAR_ON_READ 1
/* DEFAULT VALUE=0. Enabled programming of
 *Rx Training Algo Register for Debugging Purpose
 */
#define LINK_TRAINING_DEBUG     0

/*EDID Selection*/
#define DP12_EDID_ENABLED 0

#define I2S_CLK_MULT 768

/* Set this to 1 to enabled I2S Playback, Capture
 * with Audio clock recovery
 * Set to 0 to bypass I2S and route the Audio internally
  */
#define I2S_AUDIO 0

/* Some Monitors require audio to be started after some delay
 * This param adds a delay to start the audio
 */
#define AUD_START_DELAY 5000 //00

/* VPHY Specific Defines
 */
#define XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG    0x084
#define XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG    0x088

#define XVPHY_DRP_CPLL_FBDIV        0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV   0x2A
#define XVPHY_DRP_RXOUT_DIV         0x63
#define XVPHY_DRP_RXCLK25           0x6D
#define XVPHY_DRP_TXCLK25           0x7A
#define XVPHY_DRP_TXOUT_DIV         0x7C
#define XVPHY_DRP_RX_DATA_WIDTH     0x03
#define XVPHY_DRP_RX_INT_DATA_WIDTH 0x66
#define XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER 0x25E
#define XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER 0x25F

#define TX_DATA_WIDTH_REG 0x7A
#define TX_INT_DATAWIDTH_REG 0x85
/* Timer Specific Defines
 */
#define TIMER_RESET_VALUE        1000

#define ENABLE_AUDIO XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_AUDIO_ENABLE

#define AUXFIFOSIZE 4
#define XDP_DPCD_EXT_DPCD_FEATURE 0x2210

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/
#ifndef versal
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
#endif

typedef struct
{
        unsigned char lane_count;
        unsigned char link_rate;
} lane_link_rate_struct;

/************************** Function Prototypes ******************************/

u32 DpSs_Main();
u32 DpSs_PlatformInit(void);
u32 DpRxSs_VideoPhyInit(u16 DeviceId);
void frameBuffer_stop(void);
void frameBuffer_stop_wr(void);
void frameBuffer_stop_rd(void);

#ifndef versal
void PHY_Two_byte_set(XVphy *InstancePtr, u8 Rx_to_two_byte,
			u8 Tx_to_two_byte);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
#endif

void AppHelp();
void ReportVideoCRC();
void CalculateCRC(void);
void GtCtrl(u32 mask, u32 data, u8 is_tx);

void LoadEDID(void);
char XUartPs_RecvByte_NonBlocking();
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds);

int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u16 RegisterAddress, u8 Value);
int VideoFMC_Init(void);
u32 DpSs_SetupIntrSystem(void);
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
void bufferWr_callback(void *InstancePtr);
#endif
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
void bufferRd_callback(void *InstancePtr);
#endif
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u8 RegisterAddress, u8 Value);
void Dplb_Main(void);
char xil_getc(u32 timeout_ms);
void power_down_HLSIPs(void);
void power_up_HLSIPs(void);

void remap_start_wr(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);
void remap_start_rd(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);
int IDT_8T49N24x_Configure(u32 I2CBaseAddress, u8 I2CSlaveAddress);

void operationMenu(void);

void frameBuffer_start_wr(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

void frameBuffer_start_rd(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);


u32 xil_gethex(u8 num_chars);

/************************** Function Definitions *****************************/
/* Defining constants for colors in printing */
#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_WHITE    "\x1b[37m"
#define ANSI_COLOR_RESET    "\x1b[0m"

#ifdef __cplusplus
}
#endif
