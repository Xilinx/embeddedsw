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

#include "xvidframe_crc.h"

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

#include "xv_frmbufrd_l2.h"
#include "xv_frmbufwr_l2.h"
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
#define XINTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC                    XScuGic
#define XINTC_HANDLER            XScuGic_InterruptHandler

/* The unique device ID of the instances used in example
 */
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID
#define XVPHY_DEVICE_ID 	XPAR_VPHY_0_DEVICE_ID
#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID

#define VIDEO_CRC_BASEADDR 	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR 	XPAR_PSU_UART_0_BASEADDR
#define VIDPHY_BASEADDR 	XPAR_VPHY_0_BASEADDR
#define VID_EDID_BASEADDR 	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR
#define IIC_DEVICE_ID 		XPAR_IIC_0_DEVICE_ID

#define FRMBUF_RD_DEVICE_ID  XPAR_XV_FRMBUFRD_0_DEVICE_ID
#define FRMBUF_WR_DEVICE_ID  XPAR_XV_FRMBUFWR_0_DEVICE_ID
#define VIDEO_FRAME_CRC_TX_BASEADDR \
			XPAR_DP_TX_HIER_0_VIDEO_FRAME_CRC_TX_BASEADDR
#define VIDEO_FRAME_CRC_RX_BASEADDR \
			XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define REMAP_RX_BASEADDR  XPAR_DP_RX_HIER_0_REMAP_RX_S_AXI_CTRL_BASEADDR
#define REMAP_TX_BASEADDR  XPAR_DP_TX_HIER_0_REMAP_TX_S_AXI_CTRL_BASEADDR
#define REMAP_RX_DEVICE_ID  XPAR_DP_RX_HIER_0_REMAP_RX_DEVICE_ID
#define REMAP_TX_DEVICE_ID  XPAR_DP_TX_HIER_0_REMAP_TX_DEVICE_ID

#define AXI_SYSTEM_CLOCK_FREQ_HZ \
			XPAR_PROCESSOR_HIER_0_AXI_TIMER_0_CLOCK_FREQ_HZ
/* DP Specific Defines
 */
#define DPRXSS_LINK_RATE        XDPRXSS_LINK_BW_SET_810GBPS
#define DPRXSS_LANE_COUNT        XDPRXSS_LANE_COUNT_SET_4
#define SET_TX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATAWIDTH/2)
#define SET_RX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATAWIDTH/2)
#define XDP_RX_CRC_CONFIG       0x074
#define XDP_RX_CRC_COMP0        0x078
#define XDP_RX_CRC_COMP1        0x07C
#define XDP_RX_CRC_COMP2        0x080
#define XDP_RX_DPC_LINK_QUAL_CONFIG 0x454
#define XDP_RX_DPC_L01_PRBS_CNTR    0x45C
#define XDP_RX_DPC_L23_PRBS_CNTR    0x460
#define XDP_RX_DPCD_LINK_QUAL_PRBS  0x3

/*
 * User can tune these variables as per their system
 */

/*Max timeout tuned as per tester - AXI Clock=100 MHz
 *Some GPUs may need larger value, So user may tune if needed
 */
#define DP_BS_IDLE_TIMEOUT      0x047868C0//0x0091FFFF
#define VBLANK_WAIT_COUNT       200

/*For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
  For Interop, set this to 6.
*/
#define AUX_DEFER_COUNT         6
/* DEFAULT VALUE=0. Enabled programming of
 *Rx Training Algo Register for Debugging Purpose
 */
#define LINK_TRAINING_DEBUG     0

/*EDID Selection*/
#define DP12_EDID_ENABLED 0

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


/************************** Function Prototypes ******************************/

u32 DpSs_Main();
u32 DpSs_PlatformInit(void);
u32 DpRxSs_VideoPhyInit(u16 DeviceId);

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
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u8 RegisterAddress, u8 Value);
void Dplb_Main(void);
/************************** Variable Definitions *****************************/

//XDpRxSs DpRxSsInst; 	/* The DPRX Subsystem instance.*/
XINTC IntcInst; 	/* The interrupt controller instance. */
XVphy VPhyInst; 	/* The DPRX Subsystem instance.*/
XTmrCtr TmrCtr; 	/* Timer instance.*/
XIic IicInstance; 	/* I2C bus for MC6000 and IDT */

/************************** Function Definitions *****************************/

XV_FrmbufRd_l2     frmbufrd;
XV_FrmbufWr_l2     frmbufwr;
u32 XVFRMBUFRD_BUFFER_BASEADDR;
u32 XVFRMBUFWR_BUFFER_BASEADDR;
XV_axi4s_remap_Config   *rx_remap_Config;
XV_axi4s_remap          rx_remap;
XV_axi4s_remap_Config   *tx_remap_Config;
XV_axi4s_remap          tx_remap;
