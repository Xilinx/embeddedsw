/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

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
#include <xiic.h>
#include <xiic_l.h>
#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xparameters.h>
#include <xstatus.h>
#include <xtmrctr.h>
#include <xuartps_hw.h>
#include <xvphy.h>
#include <xvphy_dp.h>
#include <xvphy_hw.h>
#include "sleep.h"

#include "xvidframe_crc.h"

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xintc.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif

#include "xiicps.h"
#include "videofmc_defs.h"
#include "idt_8t49n24x.h"

#include "xv_frmbufrd_l2.h"
#include "xv_frmbufwr_l2.h"

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#include "xv_axi4s_remap.h"
#endif

//#include "xi2stx.h"
//#include "xi2srx.h"
#include "xgpio.h"
//#include "xaxis_switch.h"

#define ENABLE_HDCP_IN_DESIGN			1

#if (ENABLE_HDCP_IN_DESIGN && XPAR_DPRXSS_0_HDCP22_ENABLE)
#define ENABLE_HDCP22_IN_RX				1
#else
#define ENABLE_HDCP22_IN_RX				0
#endif

#if (ENABLE_HDCP_IN_DESIGN && XPAR_DPTXSS_0_HDCP22_ENABLE)
#define ENABLE_HDCP22_IN_TX				1
#else
#define ENABLE_HDCP22_IN_TX				0
#endif

#if (ENABLE_HDCP_IN_DESIGN && XPAR_DPRXSS_0_HDCP_ENABLE)
#define ENABLE_HDCP1x_IN_RX				1
#else
#define ENABLE_HDCP1x_IN_RX				0
#endif

#if (ENABLE_HDCP_IN_DESIGN && XPAR_DPTXSS_0_HDCP_ENABLE)
#define ENABLE_HDCP1x_IN_TX				1
#else
#define ENABLE_HDCP1x_IN_TX				0
#endif

#include "xhdcp1x_debug.h"
#include "xhdcp1x_example.h"

#include "keymgmt.h"
#include "xhdcp22_example.h"
extern XHdcp22_Repeater     Hdcp22Repeater;


#define TxOnly
#define RxOnly
#define PT
#define LB

#define USE_EEPROM_HDCP_KEYS
/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#ifndef SDT
#if (ENABLE_HDCP1x_IN_TX || ENABLE_HDCP22_IN_TX)
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DP14TXSS_0_DPTXSS_DP_IRQ_VEC_ID
#else
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DP14TXSS_0_VEC_ID
#endif

#if (ENABLE_HDCP1x_IN_RX || ENABLE_HDCP22_IN_RX)
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
    XPAR_FABRIC_DP14RXSS_0_DPRXSS_DP_IRQ_VEC_ID
#else
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
		XPAR_FABRIC_DP14RXSS_0_VEC_ID
#endif

#define XINTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC                    XScuGic
#define XINTC_HANDLER            XScuGic_InterruptHandler

/* The unique device ID of the instances used in example
 */
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID
#define XVPHY_DEVICE_ID 	XPAR_VPHY_0_DEVICE_ID
#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID
#endif

#ifndef SDT
#define VIDEO_CRC_BASEADDR 	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR 	XPAR_PSU_UART_0_BASEADDR
#define VIDPHY_BASEADDR 	XPAR_VPHY_0_BASEADDR
#define VID_EDID_BASEADDR 	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR
#define IIC_DEVICE_ID 		XPAR_IIC_0_DEVICE_ID

#define FRMBUF_RD_DEVICE_ID  XPAR_XV_FRMBUFRD_0_DEVICE_ID
#define FRMBUF_WR_DEVICE_ID  XPAR_XV_FRMBUFWR_0_DEVICE_ID
#ifdef XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR
#define VIDEO_FRAME_CRC_TX_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR \
			XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#endif
#else
#define VIDEO_CRC_BASEADDR	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR	XPAR_UART0_BASEADDR
#define VIDPHY_BASEADDR		XPAR_XVPHY_0_BASEADDR
#define VID_EDID_BASEADDR	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR
#define IIC_DEVICE_ID		XPAR_XIIC_0_BASEADDR

#ifdef XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR
#define VIDEO_FRAME_CRC_TX_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR \
			XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#endif
#endif
#ifndef SDT
#define REMAP_RX_BASEADDR  XPAR_DP_RX_HIER_0_REMAP_RX_S_AXI_CTRL_BASEADDR
#define REMAP_TX_BASEADDR  XPAR_DP_TX_HIER_0_REMAP_TX_S_AXI_CTRL_BASEADDR
#define REMAP_RX_DEVICE_ID  XPAR_DP_RX_HIER_0_REMAP_RX_DEVICE_ID
#define REMAP_TX_DEVICE_ID  XPAR_DP_TX_HIER_0_REMAP_TX_DEVICE_ID
#endif
#ifndef SDT
#define AXI_SYSTEM_CLOCK_FREQ_HZ \
			XPAR_PROCESSOR_HIER_0_AXI_TIMER_0_CLOCK_FREQ_HZ
#else
#define AXI_SYSTEM_CLOCK_FREQ_HZ \
XPAR_PROCESSOR_HIER_0_AXI_TIMER_0_CLOCK_FREQUENCY
#endif
/* DP Specific Defines
 */
#define DPRXSS_LINK_RATE        XDPRXSS_LINK_BW_SET_810GBPS
#define DPRXSS_LANE_COUNT        XDPRXSS_LANE_COUNT_SET_4

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


/*
 * User can tune these variables as per their system
 */

#define PHY_COMP 0
#define EDID_1_ENABLED 0

/*Max timeout tuned as per tester - AXI Clock=100 MHz
 *Some GPUs may need larger value, So user may tune if needed
 */
#define DP_BS_IDLE_TIMEOUT      (0x047868C0*PHY_COMP)+(0x0091FFFF*!PHY_COMP)
//0xFFFFFFFF //0x0091FFFF
#define VBLANK_WAIT_COUNT       (200+(180*PHY_COMP))//changed

/*For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
  For Interop, set this to 6.
*/
#define AUX_DEFER_COUNT         (6+(2*PHY_COMP))
/* DEFAULT VALUE=0. Enabled programming of
 *Rx Training Algo Register for Debugging Purpose
 */
#define LINK_TRAINING_DEBUG     0

/*EDID Selection*/
#define DP12_EDID_ENABLED 0

#define I2S_CLK_MULT 768

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

/*Offset address for APB register to enable RX VSC Capability in DPCD*/
#define VSC_CAP_APB_REG_OFFSET 		0X000C

/*APB register bit[2] enable*/
#define RX_VSC_CAP_ENABLE			0x4

/*Register for Rx colorimetery info from VSC SDP*/
#define RX_COLORIMETRY_INFO_SDP_REG 	0x644

/*Enable VSC Extended packet on every VSYC*/
#define VSC_EXT_PKT_VSYNC_ENABLE		0x1000

#define AUXFIFOSIZE 4
#define XDP_DPCD_EXT_DPCD_FEATURE 0x2210
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
        unsigned char lane_count;
        unsigned char link_rate;
} lane_link_rate_struct;

/************************** Function Prototypes ******************************/
void Dprx_HdcpAuthCallback(void *InstancePtr);
void Dprx_HdcpUnAuthCallback(void *InstancePtr);
#if ENABLE_HDCP_IN_DESIGN
static void Dppt_TimeOutCallback(void *InstancePtr, u8 TmrCtrNumber);
#endif
u32 DpSs_Main();
u32 DpSs_PlatformInit(void);
u32 DpRxSs_VideoPhyInit(u16 DeviceId);
int I2cClk_Ps(u32 InFreq, u32 OutFreq);
void PHY_Two_byte_set(XVphy *InstancePtr, u8 Rx_to_two_byte,
			u8 Tx_to_two_byte);
void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
void AppHelp();
void ReportVideoCRC();
void CalculateCRC(void);
void LoadEDID(void);
char XUartPs_RecvByte_NonBlocking();
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds);

int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u16 RegisterAddress, u8 Value);
int VideoFMC_Init(void);
u32 DpSs_SetupIntrSystem(void);
void bufferWr_callback(void *InstancePtr);
void bufferRd_callback(void *InstancePtr);
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u8 RegisterAddress, u8 Value);
void Dplb_Main(void);
char xil_getc(u32 timeout_ms);
void power_down_HLSIPs(void);
void power_up_HLSIPs(void);
//void remap_start(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);
void remap_start_wr(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);
void remap_start_rd(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);
int IDT_8T49N24x_Configure(u32 I2CBaseAddress, u8 I2CSlaveAddress);

void operationMenu(void);
void frameBuffer_start_wr(XVidC_VideoMode VmId,
		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

//void frameBuffer_start(XVidC_VideoMode VmId,
//		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

void frameBuffer_start_rd(XVidC_VideoMode VmId,
		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);


u32 xil_gethex(u8 num_chars);

/*********************************************************************/
#ifdef __cplusplus
}
#endif
