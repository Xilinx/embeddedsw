/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file tx.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date       Changes
* ----- ----  ----------  ----------------------------------------------------
* 1.6   GM    02/18/2026  Initial release.
*
* </pre>
*
******************************************************************************/

#ifndef SRC_TX_H_
#define SRC_TX_H_
#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

#include "xdptxss.h"
#include "xvphy.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xiic.h"
#include "xiic_l.h"
#include "xtmrctr.h"
#include "xvidc_edid.h"
#include "sleep.h"
#include "stdlib.h"
#include "xvidframe_crc.h"
#include "xvphy_i.h"

/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPTXSS_HDCP_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_HDCP_IRQ_VEC_ID
#define XINTC_DPTXSS_TMR_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_TIMER_IRQ_VEC_ID
#endif

/*
 * The unique device ID of the DisplayPort Transmitter Subsystem HIP instance
 * to be used
 */
#define XDPTXSS_DEVICE_ID	XPAR_DPTXSS_0_DEVICE_ID

/*
 * If set to 1, example will run in MST mode. Otherwise, in SST mode.
 * In MST mode, this example reads the EDID of RX devices if connected in
 * daisy-chain.
 */
#define DPTXSS_MST		1
#define DPTXSS_LINK_RATE	XDPTXSS_LINK_BW_SET_540GBPS
#define DPTXSS_LANE_COUNT	XDPTXSS_LANE_COUNT_SET_4

/*
 * The video resolution from the display mode timings (DMT) table to use for
 * DisplayPort TX Subsystem. It can be set to use preferred video mode for
 * EDID of RX device.
 */
#define DPTXSS_VID_MODE		XVIDC_VM_USE_EDID_PREFERRED

/*
 * The color depth (bits per color component) to use DisplayPort TX
 * Subsystem.
 */
#define DPTXSS_BPC		XPAR_DPTXSS_0_BITS_PER_COLOR

#define is_TX_CPLL 		0
#define CLK135MHz_DIVIDER 	18
#define CLK270MHz_DIVIDER 	9
#define CLK162MHz_DIVIDER 	15

#define CLK_WIZ_BASE 		XPAR_CLK_WIZ_0_BASEADDR

#define PE_VS_ADJUST 		1

#define XVPHY_DRP_CPLL_FBDIV 		0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV 	0x2A
#define XVPHY_DRP_RXOUT_DIV 		0x63
#define XVPHY_DRP_RXCLK25 		0x6D
#define XVPHY_DRP_TXCLK25 		0x7A
#define XVPHY_DRP_TXOUT_DIV 		0x7C

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

typedef struct
{
	XVidC_VideoMode VideoMode_local;
	unsigned char user_bpc;
	unsigned char user_pattern;
	unsigned char user_format;
	unsigned int user_numStreams;
	unsigned int user_stream_number;
	unsigned int mst_check_flag;
} user_config_struct;

/************************** Function Prototypes ******************************/
/*
 * Interrupt helper functions
 */
void Dp21RxSs_Pt_TxHpdEventHandler(void *InstancePtr);
void Dp21RxSs_Pt_TxHpdPulseHandler(void *InstancePtr);
void Dp21RxSs_Pt_TxLinkrateChgHandler (void *InstancePtr);
void Dp21RxSs_Pt_TxPeVsAdjustHandler(void *InstancePtr);
void Dp21RxSs_Pt_FfeAdjustHandler(void *InstancePtr);
void Dp21RxSs_Pt_TxExtPacketHandler(void *InstancePtr);
void Dp21RxSs_Pt_TxVsyncHandler(void *InstancePtr);
void Dp21RxSs_Pt_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);

void Dp21RxSs_Pt_TxHpdCon(XDpTxSs *InstancePtr, u16 res_update);
void Dp21RxSs_Pt_TxHpdPulseCon(XDpTxSs *InstancePtr, XDpTxSs_MainStreamAttributes Msa[4]);
u32 Dp21RxSs_Pt_TxSsStart(XDpTxSs *InstancePtr,
			XDpTxSs_MainStreamAttributes Msa[4]);
char xil_getc(u32 timeout_ms);
void Vpg_Audio_start(void);
void Vpg_Audio_stop(void);
u32 Dp21RxSs_Pt_StartTx(u8 line_rate, u8 lane_count, user_config_struct user_config,
	     XDpTxSs_MainStreamAttributes Msa[4]);

void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void Dp21RxSs_Pt_SinkPowerDown(void);
void Dp21RxSs_Pt_SinkPowerUp(void);
u8 Dp21RxSs_Pt_GetLineRate(void);
u8 Dp21RxSs_Pt_GetLanecount(void);
void Dp21RxSs_Pt_SinkPowerCycle(void);
void Dp21RxSs_Pt_TxPeVsAdjustHandler(void *InstancePtr);
int IDT_8T49N24x_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			int FIn, int FOut, u8 FreeRun);
int IDT_8T49N24x_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);
void DpTxSs_Setup(u8 *LineRate_init, u8 *LaneCount_init,
			u8 Edid_org[128], u8 Edid1_org[128]);
u32 Dp21RxSs_Pt_TxSetupIntrSystem(void);
u32 Dp21RxSs_Pt_TxGetMaxCapsOfRx (XDpTxSs *InstancePtr);
#ifdef __cplusplus
}
#endif
#endif /* SRC_TX_H_ */
