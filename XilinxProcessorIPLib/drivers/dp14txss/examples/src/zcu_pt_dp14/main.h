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
#include "sleep.h"
#include <xil_cache.h>

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
#define RX_ACR_ADDR XPAR_DP_RX_HIER_0_RX_ACR_BASEADDR

#define AXI_SYSTEM_CLOCK_FREQ_HZ \
			XPAR_PROCESSOR_HIER_0_AXI_TIMER_0_CLOCK_FREQ_HZ
/* DP Specific Defines
 */
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
// Set PHY_COMP to 1 when doing the PHY and LL compliance
// For normal operation, this needs to be set to 0
#define PHY_COMP 0
#define EDID_1_ENABLED PHY_COMP

/*Max timeout tuned as per tester - AXI Clock=100 MHz
 *Some GPUs may need larger value, So user may tune if needed
 */
#define DP_BS_IDLE_TIMEOUT      (0x047868C0*PHY_COMP)+(0x0091FFFF*!PHY_COMP)
#define VBLANK_WAIT_COUNT       (20+(180*PHY_COMP))
//Wait for Following number of infoframes before asserting info
//frame captured
#define AUD_INFO_COUNT          20
//Set this to start the audio only after receiving infoframes
#define WAIT_ON_AUD_INFO          1

/*For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
  For Interop, set this to 6.
*/
#define AUX_DEFER_COUNT         (6+(2*PHY_COMP))
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
#define AUD_START_DELAY 200000

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
/************************** Function Prototypes ******************************/

u32 DpSs_Main();
u32 DpSs_PlatformInit(void);
u32 DpRxSs_VideoPhyInit(u16 DeviceId);
void frameBuffer_stop(void);
void frameBuffer_stop_wr(void);
void frameBuffer_stop_rd(void);

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
//void frameBuffer_start_wr(XVidC_VideoMode VmId,
//		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

void frameBuffer_start_wr(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

//void frameBuffer_start(XVidC_VideoMode VmId,
//		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

//void frameBuffer_start_rd(XVidC_VideoMode VmId,
//		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

void frameBuffer_start_rd(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);


u32 xil_gethex(u8 num_chars);
void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame);
/************************** Variable Definitions *****************************/

//XDpRxSs DpRxSsInst; 	/* The DPRX Subsystem instance.*/
XINTC IntcInst; 	/* The interrupt controller instance. */
XVphy VPhyInst; 	/* The DPRX Subsystem instance.*/
XTmrCtr TmrCtr; 	/* Timer instance.*/
XIic IicInstance; 	/* I2C bus for MC6000 and IDT */

/************************** Function Definitions *****************************/

XV_FrmbufRd_l2     frmbufrd;
XV_FrmbufWr_l2     frmbufwr;
u64 XVFRMBUFRD_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR;

u64 XVFRMBUFRD_BUFFER_BASEADDR_Y;
u64 XVFRMBUFWR_BUFFER_BASEADDR_Y;

//u64 BUF1 =  0x10000000;
//u64 BUF2 =  0x18000000;
//u64 BUF3 =  0x20000000;
//u64 BUF4 =  0x28000000;


XV_axi4s_remap_Config   *rx_remap_Config;
XV_axi4s_remap          rx_remap;
XV_axi4s_remap_Config   *tx_remap_Config;
XV_axi4s_remap          tx_remap;


XilAudioInfoFrame *xilInfoFrame;
XIicPs_Config *XIic0Ps_ConfigPtr;
XIicPs_Config *XIic1Ps_ConfigPtr;

#if ENABLE_AUDIO
XI2s_Tx I2s_tx;
XI2s_Rx I2s_rx;
XGpio   aud_gpio;

XI2stx_Config *Config;
XI2srx_Config *Config_rx;
XGpio_Config  *aud_gpio_ConfigPtr;
XAxis_Switch axis_switch_rx;
XAxis_Switch axis_switch_tx;

#endif
