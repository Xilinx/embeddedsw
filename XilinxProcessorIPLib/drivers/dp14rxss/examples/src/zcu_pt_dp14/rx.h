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
* @file rx.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* </pre>
*
******************************************************************************/

#include <xdprxss.h>
#include <xdprxss_mcdp6000.h>
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

/*
 * The following constants map to the names of the hardware instances.
 * They are only defined here such that a user can easily change all the
 * needed device IDs in one place.
 * There is only one interrupt controlled to be selected from SCUGIC and GPIO
 * INTC. INTC selection is based on INTC parameters defined xparameters.h file.
 */
#define XINTC_DPRXSS_DP_INTERRUPT_ID \
    XPAR_FABRIC_DPRXSS_0_VEC_ID
#define XINTC_DEVICE_ID 	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define 			XINTC XScuGic
#define XINTC_HANDLER 		XScuGic_InterruptHandler

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
//#define DP_BS_IDLE_TIMEOUT      0x047868C0//0x0091FFFF
//#define VBLANK_WAIT_COUNT       20

/*For compliance, please set AUX_DEFER_COUNT to be 8
 * (Only for ZCU102-ARM R5 based Rx system).
  For Interop, set this to 6.
*/
//#define AUX_DEFER_COUNT         8
/* DEFAULT VALUE=0. Enabled programming of
 *Rx Training Algo Register for Debugging Purpose
 */
//#define LINK_TRAINING_DEBUG     0

/*EDID Selection*/
//#define DP12_EDID_ENABLED 0

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

/*The structure defines sub-fields of Register 0x214*/
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

void DpRxSs_Main(void);
u32 DpRxSs_VideoPhyInit(u16 DeviceId);
u32 DpRxSs_Setup(void);

/* Interrupt helper functions */
u32 DpRxSs_SetupIntrSystem(void);

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
void Print_InfoPkt();
void Print_ExtPkt();

/************************** Variable Definitions *****************************/

XDpRxSs DpRxSsInst;    /* The DPRX Subsystem instance.*/
Video_CRC_Config VidFrameCRC_rx; /* Video Frame CRC instance */
DP_Rx_Training_Algo_Config RxTrainConfig;
XIic IicInstance;	/* I2C bus for MC6000 and IDT */

#define XACR_WriteReg(BaseAddress, RegOffset, Data)   \
    Xil_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))

#define XACR_ReadReg(BaseAddress, RegOffset)   \
    Xil_In32((BaseAddress) + ((u32)RegOffset))


#define RXACR_MODE   0x20
#define RXACR_MAUD   0x50
#define RXACR_NAUD   0x54
#define RXACR_DIV    0x70
#define RXACR_ENABLE 0x8
