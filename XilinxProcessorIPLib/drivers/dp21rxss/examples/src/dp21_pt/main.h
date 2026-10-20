/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.6  GM  02/18/26 Initial release.
 *
 * </pre>
 *
******************************************************************************/

/***************************** Include Files *********************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <xparameters.h>
#include <xdp.h>
#include <xdp_hw.h>
#include <xdprxss.h>
#include <xdptxss.h>
#if !defined(XPS_BOARD_VCK190) && !defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
#include <xiic.h>
#include <xiic_l.h>
#endif /* Added for VCK190/VEK385 support */
#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xstatus.h>
#include <xtmrctr.h>

#if defined (PLATFORM_MB) || defined (__riscv)
#include <xuartlite_l.h>
#include "xintc.h"
#else
#if !defined(XPS_BOARD_VCK190) && !defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
#include <xuartps.h>
#include <xuartps_hw.h>
#else /* Added for VCK190/VEK385 support */
#include <xuartpsv_hw.h>
#endif /* Added for VCK190/VEK385 support */
#endif

#if !defined(XPS_BOARD_VCK190) && !defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
#include <xvphy.h>
#include <xvphy_dp.h>
#include <xvphy_hw.h>
#else /* Added for VCK190/VEK385 support */
/* Versal (VCK190) has no XVphy GT-PHY IP/driver. These are minimal
 * stand-in types so existing XVphy-typed globals/prototypes below still
 * compile; none of their real fields are read on this board - the VCK190
 * path drives both DP cores (RX and TX) with raw register writes instead. */
typedef struct { UINTPTR BaseAddr; } XVphy;
typedef u8 XVphy_ChannelId;
typedef u8 XVphy_PllType;
typedef u8 XVphy_PllRefClkSelType;
typedef u8 XVphy_DirectionType;
#define XVPHY_CHANNEL_ID_CH1   0
#define XVPHY_CHANNEL_ID_CH2   1
#define XVPHY_CHANNEL_ID_CH3   2
#define XVPHY_CHANNEL_ID_CH4   3
#define XVPHY_CHANNEL_ID_CMN0  4	/* unused on VCK190; no VCU118/ZCU102 QPLL sharing concept exists */
#define XVPHY_CHANNEL_ID_CMN1  5	/* unused on VCK190; no VCU118/ZCU102 QPLL sharing concept exists */
#define XVPHY_DIR_TX           0
#define XVPHY_DIR_RX           1
#include "xclk_wiz.h"
#endif /* Added for VCK190/VEK385 support */

#include "sleep.h"
#include <xil_cache.h>

#include "xvidframe_crc.h"
#include "si570drv.h"

#define XPAR_XV_FRMBUFRD_NUM_INSTANCES XPAR_XV_FRMBUF_RD_NUM_INSTANCES
#define XPAR_XV_FRMBUFWR_NUM_INSTANCES XPAR_XV_FRMBUF_WR_NUM_INSTANCES

#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xinterrupt_wrap.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xinterrupt_wrap.h"
#endif /* XPAR_INTC_0_DEVICE_ID */

#include "videofmc_defs.h"

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
#include "xv_frmbufrd_l2.h"
#define FRMBUF_RD_DEVICE_ID  XPAR_XV_FRMBUFRD_0_DEVICE_ID
#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
#include "xv_frmbufwr_l2.h"
#define FRMBUF_WR_DEVICE_ID  XPAR_XV_FRMBUFWR_0_DEVICE_ID
#endif

#include "xgpio.h"
#include "xv_tpg.h"

#define XPAR_DPTXSS_0_HDCP_ENABLE	0
#define XPAR_DPTXSS_0_HDCP22_ENABLE	0
#ifndef XPAR_DPRXSS_0_HDCP_ENABLE
#define XPAR_DPRXSS_0_HDCP_ENABLE	0
#endif
#ifndef XPAR_DPRXSS_0_HDCP22_ENABLE
#define XPAR_DPRXSS_0_HDCP22_ENABLE	0
#endif

#define USE_EEPROM_HDCP_KEYS
#define ENABLE_HDCP_IN_DESIGN		(XPAR_DPTXSS_0_HDCP_ENABLE | XPAR_DPRXSS_0_HDCP_ENABLE | \
                                         XPAR_DPTXSS_0_HDCP22_ENABLE | XPAR_DPRXSS_0_HDCP22_ENABLE)

#define ENABLE_HDCP22_IN_RX		(XPAR_DPRXSS_0_HDCP22_PRESENT)
#define ENABLE_HDCP1x_IN_RX		(XPAR_DPRXSS_0_HDCP14_PRESENT)
#define ENABLE_HDCP_RX			(ENABLE_HDCP22_IN_RX | ENABLE_HDCP1x_IN_RX)

#if ENABLE_HDCP1x_IN_RX
#define KEYMGMT_BASEADDR_RX XPAR_DP_RX_HIER_0_HDCP_KEYMNGMT_BLK_0_BASEADDR
#else
#define KEYMGMT_BASEADDR_RX 0
#endif

#define ENABLE_HDCP22_IN_TX		(XPAR_DPTXSS_0_HDCP22_PRESENT)
#define ENABLE_HDCP1x_IN_TX		(XPAR_DPTXSS_0_HDCP14_PRESENT)
#define ENABLE_HDCP_TX                  (ENABLE_HDCP22_IN_TX | ENABLE_HDCP1x_IN_TX)

#if ENABLE_HDCP1x_IN_TX
#define KEYMGMT_BASEADDR_TX XPAR_DP_TX_HIER_0_HDCP_KEYMNGMT_BLK_0_BASEADDR
#else
#define KEYMGMT_BASEADDR_TX 0
#endif

#if ENABLE_HDCP_IN_DESIGN
#include "xhdcp22_example.h"
extern XHdcp22_Repeater     Hdcp22Repeater;
#endif
#define ENABLE_AUDIO  (XPAR_DP_RX_HIER_0_V_DP_RXSS2_0_DP_SECONDARY_SUPPORT & \
                         XPAR_DP_TX_HIER_0_V_DP_TXSS2_0_DP_SECONDARY_SUPPORT)
#define PT

#ifdef PT	/**< Passthrough */

#ifndef Rxo	/**< Enable Rx */
#define Rxo
#endif

#ifndef Txo	/**< Enable Tx */
#define Txo
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
#define XINTC_DEVICE_ID 	XPAR_INTC_0_DEVICE_ID
#define XINTC_HANDLER 		XIntc_InterruptHandler

/* The unique device ID of the instances used in example
 */
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID

#if defined(XPS_BOARD_VCK190) || defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
#define XVPHY_DEVICE_ID		0	/* unused on VCK190; no XVphy instance exists */
#else
#define XVPHY_DEVICE_ID 	XPAR_VID_PHY_CONTROLLER_1_DEVICE_ID
#endif /* Added for VCK190/VEK385 support */


#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID

#define TX_CLK_RST_ADDR XPAR_DP_TX_HIER_0_TX_CLK_RST_BASEADDR

#define VIDEO_CRC_BASEADDR 	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR 	XPAR_PSU_UART_0_BASEADDR

#define VID_EDID_BASEADDR 	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR
#define IIC_DEVICE_ID 		XPAR_IIC_0_DEVICE_ID
#if defined(XPS_BOARD_VCK190) || defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
#define IIC_BASE_ADDR		0	/* unused on VCK190; no PL AXI IIC core exists */
#else /* Added for VCK190/VEK385 support */
#define IIC_BASE_ADDR 		XPAR_XIIC_0_BASEADDR
#endif /* Added for VCK190/VEK385 support */


#define VIDEO_FRAME_CRC_TX_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR \
			XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR

#if defined(XPS_BOARD_VCK190) || defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
#define PROC_HIER_HLS_RST_BASEADDR XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR
#else /* Added for VCK190/VEK385 support */
#define PROC_HIER_HLS_RST_BASEADDR XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR
#endif /* Added for VCK190/VEK385 support */

#if defined(XPS_BOARD_VCK190) || defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
#define AXI_SYSTEM_CLOCK_FREQ_HZ \
			XPAR_PROCESSOR_HIER_0_AXI_TIMER_0_CLOCK_FREQUENCY
#else /* Added for VCK190/VEK385 support */
#define AXI_SYSTEM_CLOCK_FREQ_HZ \
			XPAR_PROC_HIER_0_AXI_TIMER_0_CLOCK_FREQUENCY
#endif /* Added for VCK190/VEK385 support */

/*
 * Timer Specific Defines
 */
#define TIMER_RESET_VALUE        1000

/*
 * DP Specific Defines
 */
#define XDP_RX_CRC_CONFIG       	0x074
#define XDP_RX_CRC_COMP0        	0x078
#define XDP_RX_CRC_COMP1        	0x07C
#define XDP_RX_CRC_COMP2        	0x080
#define XDP_RX_DPC_LINK_QUAL_CONFIG 0x454
#define XDP_RX_DPC_L01_PRBS_CNTR    0x45C
#define XDP_RX_DPC_L23_PRBS_CNTR    0x460
#define XDP_RX_DPCD_LINK_QUAL_PRBS  0x3

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
#define XVPHY_GTHE4_DIFF_SWING_DP_DP20 0xF

#if defined(XPS_BOARD_VCK190) || defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
/*
 * GT_QUAD_BASE and the TX pixel-clock wizard (ClkWiz_Dynamic) instance
 * names differ between the VCK190 PT design (confirmed from the .hwh
 * inside dpss_vck190_pt_wrapper.xsa: "gt_quad_hier_0/gtwiz_versal_0" and
 * "dp_tx_hier_0/clk_wizard_2") and the VEK385 PT design (confirmed from
 * its generated xparameters.h: "gt_quad/gtwiz_versal_0" and
 * "dp_tx_hier_0/clkx5_wiz_0") - each resolved to the real xparameters.h
 * macro for its own board rather than a generic guessed name.
 */
#if defined(XPS_BOARD_VCK190)
#define GT_QUAD_BASE                XPAR_GT_QUAD_HIER_0_GTWIZ_VERSAL_0_BASEADDR
#define TX_CLK_WIZ_DEVICE_ID        XPAR_DP_TX_HIER_0_CLK_WIZARD_2_DEVICE_ID
#define TX_CLK_WIZ_BASEADDR         XPAR_DP_TX_HIER_0_CLK_WIZARD_2_BASEADDR
#else /* XPS_BOARD_VEK385_1 */
#define GT_QUAD_BASE                XPAR_GT_QUAD_GTWIZ_VERSAL_0_BASEADDR
#define TX_CLK_WIZ_DEVICE_ID        XPAR_DP_TX_HIER_0_CLKX5_WIZ_0_DEVICE_ID
#define TX_CLK_WIZ_BASEADDR         XPAR_DP_TX_HIER_0_CLKX5_WIZ_0_BASEADDR
#endif
#define GT_RST_MASK                 0x00000001
#define GT_VSWING_MASK              0x00001F00
#define GT_POSTCUR_MASK             0x007C0000
#define GT_RATE_MASK                0x0000000E
#define SINGLE_LANE                 0x00000011
#define ALL_LANE                    0x0000003F
#define VERSAL_200G                 6
#define VERSAL_135G                 5
#define VERSAL_100G                 4
#define VERSAL_810G                 3
#define VERSAL_540G                 2
#define VERSAL_270G                 1
#define VERSAL_162G                 0
#define DP_LINK_RATE_HZ_162GBPS     1620000000LL
#define DP_LINK_RATE_HZ_270GBPS     2700000000LL
#define DP_LINK_RATE_HZ_540GBPS     5400000000LL
#define DP_LINK_RATE_HZ_810GBPS     8100000000LL
#define DP_LINK_RATE_HZ_10GBPS      10000000000LL
#define DP_LINK_RATE_HZ_135GBPS     13500000000LL
#define DP_LINK_RATE_HZ_20GBPS      20000000000LL

/*
 * GT power-good GPIO: gt_quad/gtpowergood is wired to processor_hier_0/
 * axi_gpio_0's gpio_io_i (confirmed from the VCK190 PT design's .hwh,
 * inside dpss_vck190_pt_wrapper.xsa - MODULE FULLNAME="/processor_hier_0/
 * axi_gpio_0", C_BASEADDR 0xA40D0000), shared by both dp_tx_hier_0 and
 * dp_rx_hier_0. Use the auto-generated xparameters.h macros for this
 * instance rather than a hardcoded address, matching this app's existing
 * convention for other hierarchical instances (e.g. XPAR_DP_TX_HIER_0_
 * TX_CLK_RST_BASEADDR).
 */
#define GT_POWERGOOD_GPIO_DEVICE_ID XPAR_PROCESSOR_HIER_0_AXI_GPIO_0_DEVICE_ID
#define GT_POWERGOOD_GPIO_BASEADDR  XPAR_PROCESSOR_HIER_0_AXI_GPIO_0_BASEADDR
#endif /* Added for VCK190/VEK385 support */

/*
 * Max timeout tuned as per tester - AXI Clock=100 MHz
 * Some GPUs may need larger value, So user may tune if needed
 */
#define DP_BS_IDLE_TIMEOUT       0xFFFFFF
#define VBLANK_WAIT_COUNT       200

/*
 * For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
 * For Interop, set this to 6.
 */
#define AUX_DEFER_COUNT         (6)
#define AUX_DEFER_COUNT_PHY     1
#define PRBS_ERRCNTR_CLEAR_ON_READ 1

/*
 * DEFAULT VALUE=0. Enabled programming of
 * Rx Training Algo Register for Debugging Purpose
 */
#define LINK_TRAINING_DEBUG     0

#define audio_clk_Hz 24.576
#define EEPROM_TEST_START_ADDRESS       0x80
#define PAGE_SIZE                       16
#define NUM_MODES                       7
#define NUM_CLOCK_REGS                  6

#define AUXFIFOSIZE 4

#if defined (PLATFORM_MB) || defined (__riscv)
#define AXIMMWIDTH 512
#else
#define AXIMMWIDTH 256
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/
typedef enum {
	XVIDC_VM_7680x4320_30_DELL = (XVIDC_VM_CUSTOM + 1),
	XVIDC_CM_NUM_SUPPORTED
} XVIDC_CUSTOM_MODES;

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
u32 Dp21RxSs_Pt_Main();
u32 Dp21RxSs_Pt_PlatformInit(void);
void Dp21RxSs_Pt_FbStop(void);
void Dp21RxSs_Pt_FbStopWr(void);
void Dp21RxSs_Pt_FbStopRd(void);
void Dp21RxSs_Pt_PLLRefClkSel(XVphy *InstancePtr, u8 link_rate, XVphy_ChannelId Channel);
void Dp21RxSs_Pt_RxReportVideoCRC();
void Dp21RxSs_Pt_CalculateCRC(void);
char XUartPs_RecvByte_NonBlocking();
void Dp21RxSs_Pt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);
int Dp21RxSs_Pt_VideoFMC_Init(void);
u32 Dp21RxSs_Pt_SetupIntrSystem(void);
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
void Dp21RxSs_Pt_WrBufCallback(void);
#endif
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
void Dp21RxSs_Pt_RbBufCallback(void);
#endif
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u8 RegisterAddress, u8 Value);
char xil_getc(u32 timeout_ms);
void Dp21RxSs_Pt_Menu(void);
void Dp21RxSs_Pt_FbStartWr(XDpTxSs_MainStreamAttributes Msa[4]);
void Dp21RxSs_Pt_FbStartRd(XDpTxSs_MainStreamAttributes Msa[4]);
void Dp21RxSs_Pt_FfeAdjustHandler(void *InstancePtr);
u32 xil_gethex(u8 num_chars);
int Dp21RxSs_Pt_I2cWriteFreq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress,
                u32 Value);
u8 Dp21RxSs_Pt_I2cReadFreq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress);
u8 Dp21RxSs_Pt_I2cReadTdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress);
int Dp21RxSs_Pt_I2cWriteTdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress, u8 Value);
u32 Dp21RxSs_Pt_PhyConfig(XVphy *InstancePtr, int LineRate_init_tx, XVphy_PllType Pll, XVphy_ChannelId Channel, XVphy_DirectionType Dir);
#if defined(XPS_BOARD_VCK190) || defined(XPS_BOARD_VEK385_1) /* Added for VCK190/VEK385 support */
void GtCtrl(u32 mask, u32 data, u8 is_tx);
#endif /* Added for VCK190/VEK385 support */

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
