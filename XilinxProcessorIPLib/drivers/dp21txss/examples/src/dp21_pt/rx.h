/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
#ifdef __cplusplus
extern "C" {
#endif

#include <xdprxss.h>
#include <xdprxss_mcdp6000.h>
#include <xparameters.h>
#include <xstatus.h>
#include <xtmrctr.h>
#include <xvphy.h>
#include <xvphy_dp.h>
#include <xvphy_hw.h>
#include "xvidframe_crc.h"

#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#ifndef SDT
#include "xintc.h"
#else
#include "xinterrupt_wrap.h"
#endif
#else
/* For ARM/Zynq SoC systems. */
#ifndef SDT
#include "xscugic.h"
#else
#include "xinterrupt_wrap.h"
#endif
#endif /* XPAR_INTC_0_DEVICE_ID */

//#include "xiicps.h"

/* The unique device ID of the instances used in example
 */
#define XDPRXSS_DEVICE_ID 	XPAR_DPRXSS_0_DEVICE_ID
#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID

#define VIDEO_CRC_BASEADDR 	XPAR_DP_RX_HIER_0_VIDEO_FRAME_CRC_0_BASEADDR
#define UARTLITE_BASEADDR 	XPAR_PSU_UART_0_BASEADDR
#ifndef SDT
#define VIDPHY_BASEADDR 	XPAR_VPHY_0_BASEADDR
#else
#define VIDPHY_BASEADDR 	XPAR_XVPHY_0_BASEADDR
#endif

#define VID_EDID_BASEADDR 	XPAR_DP_RX_HIER_0_VID_EDID_0_BASEADDR
#define IIC_DEVICE_ID 		XPAR_IIC_0_DEVICE_ID

/* DP Specific Defines
 */
#define DPRXSS_LINK_RATE        XDPRXSS_LINK_BW_SET_810GBPS
#define DPRXSS_LANE_COUNT       XDPRXSS_LANE_COUNT_SET_4
#ifndef SDT
#define SET_TX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATAWIDTH/2)
#define SET_RX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATAWIDTH/2)
#else
#define SET_TX_TO_2BYTE            \
    (XPAR_XDP_0_GT_DATA_WIDTH/2)
#define SET_RX_TO_2BYTE            \
    (XPAR_XDP_1_GT_DATA_WIDTH/2)
#endif
#define XDP_RX_CRC_CONFIG       0x074
#define XDP_RX_CRC_COMP0        0x078
#define XDP_RX_CRC_COMP1        0x07C
#define XDP_RX_CRC_COMP2        0x080
#define XDP_RX_DPC_LINK_QUAL_CONFIG 0x454
#define XDP_RX_DPC_L01_PRBS_CNTR    0x45C
#define XDP_RX_DPC_L23_PRBS_CNTR    0x460
#define XDP_RX_DPCD_LINK_QUAL_PRBS  0x3

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

/*The structure defines Generic Frame Packet fields*/
typedef struct
{
	u32 frame_count;
	u32 frame_count_q;
	u8 Header[4];
	u8 Payload[32];
} XilAudioExtFrame;

/************************** Function Prototypes ******************************/

void DpRxSs_Main(void);
u32 DpRxSs_Setup(void);

/* Interrupt helper functions */
u32 DpRxSs_SetupIntrSystem(void);

void DpRxSs_VideoValidHandler(void *InstancePtr);
void DpRxSs_PowerChangeHandler(void *InstancePtr);
void DpRxSs_NoVideoHandler(void *InstancePtr);
void DpRxSs_VmChangeHandler(void *InstancePtr);
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
void Dprx_InterruptHandlerDownReq(void *InstancePtr);
void Dprx_InterruptHandlerDownReply(void *InstancePtr);
void Dprx_InterruptHandlerPayloadAlloc(void *InstancePtr);
void Dprx_InterruptHandlerActRx(void *InstancePtr);

/************************** Variable Definitions *****************************/
#define XACR_WriteReg(BaseAddress, RegOffset, Data)   \
    Xil_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))

#define XACR_ReadReg(BaseAddress, RegOffset)   \
    Xil_In32((BaseAddress) + ((u32)RegOffset))


#define RXACR_MODE   0x20
#define RXACR_MAUD   0x50
#define RXACR_NAUD   0x54
#define RXACR_DIV    0x70
#define RXACR_ENABLE 0x8
#ifdef __cplusplus
}
#endif
