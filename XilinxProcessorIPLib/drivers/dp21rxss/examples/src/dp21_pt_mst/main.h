/*******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file main.h
*
* This file contains a design example using both the XDpTxSs and XDpRxSs
* drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.6  GM  03/06/26 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "xparameters.h"
#include <xdp.h>
#include <xdp_hw.h>
#include <xdprxss.h>
#include <xdptxss.h>
#include <xiic.h>
#include <xiic_l.h>
#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xstatus.h>
#include <xtmrctr.h>
#include <xuartlite_l.h>
#include <xvphy.h>
#include <xvphy_dp.h>
#include <xvphy_hw.h>

#include <xil_cache.h>

#include "xvidframe_crc.h"
#include "xaxis_switch.h"

#ifdef SDT
#define XPAR_XV_FRMBUFRD_NUM_INSTANCES XPAR_XV_FRMBUF_RD_NUM_INSTANCES
#define XPAR_XV_FRMBUFWR_NUM_INSTANCES XPAR_XV_FRMBUF_WR_NUM_INSTANCES
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
/*
 * For MicroBlaze systems.
 */
#ifndef SDT
#include "xintc.h"
#else
#include "xinterrupt_wrap.h"
#endif
#else
/*
 * For ARM/Zynq SoC systems.
 */
#ifndef SDT
#include "xscugic.h"
#else
#include "xinterrupt_wrap.h"
#endif
#endif /**< XPAR_INTC_0_DEVICE_ID */

#include "videofmc_defs.h"

#ifndef SDT
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
#include "xv_frmbufrd_l2.h"
#define FRMBUF_RD_DEVICE_ID  XPAR_XV_FRMBUFRD_0_DEVICE_ID
#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
#include "xv_frmbufwr_l2.h"
#define FRMBUF_WR_DEVICE_ID  XPAR_XV_FRMBUFWR_0_DEVICE_ID
#endif
#else
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
#include "xv_frmbufrd_l2.h"
#define FRMBUF_RD_DEVICE_ID  XPAR_XV_FRMBUFRD_0_DEVICE_ID
#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
#include "xv_frmbufwr_l2.h"
#define FRMBUF_WR_DEVICE_ID  XPAR_XV_FRMBUFWR_0_DEVICE_ID
#endif
#endif

#include "xgpio.h"

#define PT

#ifdef PT	/**< Passthrough */

#ifndef Rx	/**< Enable Rx */
#define Rx
#endif

#ifndef Tx	/**< Enable Tx */
#define Tx
#endif

#endif

/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/

#define AXIMMWIDTH 512

#define XINTC_DEVICE_ID 	XPAR_INTC_0_DEVICE_ID
#define XINTC_HANDLER 		XIntc_InterruptHandler

#ifndef SDT
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_INTC_0_DP21TXSS_0_VEC_ID

#define XINTC_DPRXSS_DP_INTERRUPT_ID \
    XPAR_INTC_0_DP21RXSS_0_VEC_ID
#endif
/*
 * The unique device ID of the instances used in example
 */
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID

#define XVPHY_DEVICE_ID 	XPAR_VID_PHY_CONTROLLER_1_DEVICE_ID

#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID

#define VIDEO_CRC_BASEADDR 	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR 	XPAR_PSU_UART_0_BASEADDR
#define VID_EDID_BASEADDR 	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR
#define IIC_DEVICE_ID 		XPAR_IIC_0_DEVICE_ID
#ifndef SDT
#define IIC_BASE_ADDR 		XPAR_IIC_0_BASEADDR
#else
#define IIC_BASE_ADDR 		XPAR_XIIC_0_BASEADDR
#endif

#define VIDEO_FRAME_CRC_TX_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX_BASEADDR
#define VIDEO_FRAME_CRC_TX1_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX1_BASEADDR
#define VIDEO_FRAME_CRC_TX2_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX2_BASEADDR
#define VIDEO_FRAME_CRC_TX3_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX3_BASEADDR

#define VIDEO_FRAME_CRC_RX_BASEADDR \
			XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR_1 \
		XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_1_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR_2 \
		XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_2_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR_3 \
		XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_3_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR_4 \
		XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_4_BASEADDR

#ifndef SDT
#define AXI_SYSTEM_CLOCK_FREQ_HZ \
			XPAR_PROC_HIER_0_AXI_TIMER_0_CLOCK_FREQUENCY
#else
#define AXI_SYSTEM_CLOCK_FREQ_HZ \
			XPAR_PROC_HIER_0_AXI_TIMER_0_CLOCK_FREQUENCY
#endif

/*
 * DP Specific Defines
 */
#define XDP_RX_CRC_CONFIG       	0x074
#define XDP_RX_CRC_COMP0        	0x078
#define XDP_RX_CRC_COMP1        	0x07C
#define XDP_RX_CRC_COMP2        	0x080
#define XDP_RX_DPC_LINK_QUAL_CONFIG	0x454
#define XDP_RX_DPC_L01_PRBS_CNTR	0x45C
#define XDP_RX_DPC_L23_PRBS_CNTR	0x460
#define XDP_RX_DPCD_LINK_QUAL_PRBS	0x3

#define XVPHY_GTHE4_PREEMP_DP_L0	0x3
#define XVPHY_GTHE4_PREEMP_DP_L1	0xD
#define XVPHY_GTHE4_PREEMP_DP_L2	0x16
#define XVPHY_GTHE4_PREEMP_DP_L3	0x1D

#define XVPHY_GTHE4_DIFF_SWING_DP_V0P0	0x1
#define XVPHY_GTHE4_DIFF_SWING_DP_V0P1	0x2
#define XVPHY_GTHE4_DIFF_SWING_DP_V0P2	0x5
#define XVPHY_GTHE4_DIFF_SWING_DP_V0P3	0xB
#define XVPHY_GTHE4_DIFF_SWING_DP_V1P0	0x2
#define XVPHY_GTHE4_DIFF_SWING_DP_V1P1	0x5
#define XVPHY_GTHE4_DIFF_SWING_DP_V1P2	0x7
#define XVPHY_GTHE4_DIFF_SWING_DP_V2P0	0x4
#define XVPHY_GTHE4_DIFF_SWING_DP_V2P1	0x7
#define XVPHY_GTHE4_DIFF_SWING_DP_V3P0	0x8
#define XVPHY_GTHE4_DIFF_SWING_DP_DP20	0xF

/*
 * Max timeout tuned as per tester - AXI Clock = 100 MHz
 * Some GPUs may need larger value, So user may tune if needed
 */
#define DP_BS_IDLE_TIMEOUT		0xFFFFFF
#define VBLANK_WAIT_COUNT		200

/*
 * For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
 * For Interop, set this to 6.
 */
#define AUX_DEFER_COUNT         (6)
#define AUX_DEFER_COUNT_PHY     1
#define PRBS_ERRCNTR_CLEAR_ON_READ 1

/*
 * DEFAULT VALUE = 0. Enabled programming of
 * Rx Training Algo Register for Debugging Purpose
 */
#define LINK_TRAINING_DEBUG     0

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

typedef enum {
	XVIDC_VM_7680x4320_30_DELL = (XVIDC_VM_CUSTOM + 1),
	XVIDC_CM_NUM_SUPPORTED
} XVIDC_CUSTOM_MODES;

#define RATE_162	0x6
#define RATE_270	0xA
#define RATE_540	0x14
#define RATE_810	0x1E
#define RATE_1000	0x1
#define RATE_1350	0x4
#define RATE_2000	0x2

#define XVPHY_DP_LINK_RATE_HZ_1000GBPS  10000000000LL
#define XVPHY_DP_LINK_RATE_HZ_1350GBPS  13500000000LL
#define XVPHY_DP_LINK_RATE_HZ_2000GBPS  20000000000LL

#define XVPHY_DP_REF_CLK_FREQ_HZ_400	 400000000LL

typedef struct
{
	unsigned char lane_count;
	unsigned char link_rate;
} lane_link_rate_struct;

/************************** Function Prototypes ******************************/
u32 Dp21RxSs_PtMst_Main();
u32 Dp21RxSs_PtMst_PlatformInit(void);
void Dp21RxSs_PtMst_FbStop(void);
void Dp21RxSs_PtMst_WrFbStop(void);
void Dp21RxSs_PtMst_RdFbStop(void);
void Dp21RxSs_PtMst_PLLRefClkSel (XVphy *InstancePtr, u8 link_rate, XVphy_ChannelId Channel);
void ReportVideoCRC(u8 select);
void Dp21RxSs_PtMst_CalculateCRC(u8 Stream);
char XUartPs_RecvByte_NonBlocking();
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
int Dp21RxSs_PtMst_VideoFMC_Init(void);
u32 Dp21RxSs_PtMst_SetupIntrSystem(void);
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
void Dp21RxSs_PtMst_WrBufferCallback(void *InstancePtr);
#endif

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
void Dp21RxSs_PtMst_RdBufferCallback(void *InstancePtr);
#endif

int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u8 RegisterAddress, u8 Value);
char Dp21RxSs_PtMst_Getc(u32 timeout_ms);
void Dp21RxSs_PtMst_OperationMenu(void);
void Dp21RxSs_PtMst_WrFbStart(XDpTxSs_MainStreamAttributes Msa[4], u8 Stream);
void Dp21RxSs_PtMst_RdFbStart(XDpTxSs_MainStreamAttributes Msa[4], u8 Stream);
void DpPt_ffe_adjustHandler(void *InstancePtr);
u32 Dp21RxSs_PtMst_Gethex(u8 num_chars);

int Dp21RxSs_PtMst_I2cWriteFreq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress,
		   u32 Value);
u8 Dp21RxSs_PtMst_I2cReadFreq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress);
u8 Dp21RxSs_PtMst_I2cReadTdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress);
int Dp21RxSs_PtMst_I2cWriteTdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		      u8 RegisterAddress, u8 Value);

u32 Dp21RxSs_PtMst_PhyConfig(XVphy *InstancePtr, int LineRate_init_tx, XVphy_PllType Pll,
	       XVphy_ChannelId Channel, XVphy_DirectionType Dir);
u32 Dp21RxSs_PtMst_Read(u32 BaseAddress, u32 RegOffset);
void Dp21RxSs_PtMst_Write(u32 BaseAddress, u32 RegOffset, u32 Data);
void Read_EDID (void *InstancePtr, u8 enable_print);
void Set_EDID (u8 edid_monitor[384]);

/* Forward declarations to avoid implicit declaration warnings */
void main_loop(void);
void Gen_vid_clk(XDp *InstancePtr, u8 Stream);
void Vpg_Reset(void);
u32 DpRxSs_Setup(void);
void Dp21RxSs_PtMst_RxTracking(void);

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
