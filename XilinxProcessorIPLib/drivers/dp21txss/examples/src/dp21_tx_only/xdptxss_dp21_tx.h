/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xdptxss_zcu102_tx.h
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
* Ver  Who      Date      Changes
* ---- ---      --------  --------------------------------------------------.
* 1.00  ND      18/10/22  Common DP 2.1 tx only application for zcu102 and
* 						  vcu118
* 1.01	ND		26/02/24  Added support for 13.5 and 20G
* 1.02  ND      24/03/25  Added support for PARRETO fmc
 *</pre>
 *
*****************************************************************************/

#ifndef SRC_XDPTXSS_ZCU102_TX_H_
#define SRC_XDPTXSS_ZCU102_TX_H_
#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

#include "xdptxss.h"

#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#ifndef PLATFORM_MB
#include "xscugic.h"
#else
#include "xintc.h"
#endif

#include "xvphy.h"
#include "xvphy_i.h"
#ifndef PLATFORM_MB
#include "xuartps_hw.h"
#include "xiicps.h"
#else
#include "xuartlite.h"
#include "xuartlite_l.h"
#endif
#include "xiic.h"
#include "xiic_l.h"
#include "xtmrctr.h"
#include "xvidc_edid.h"
#include "sleep.h"
#include "stdlib.h"
#include "xvidframe_crc.h"
#include "clk_set.h"
#include "videofmc_defs.h"
#include "xvidframe_crc.h"

/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#ifndef PLATFORM_MB
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DP21TXSS_0_VEC_ID

#define XINTC				XScuGic
#define XINTC_HANDLER			XScuGic_InterruptHandler
#else
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_INTC_0_DP21TXSS_0_VEC_ID

#define XINTC_HANDLER			XIntc_InterruptHandler

#endif

/* The unique device ID of the DisplayPort Transmitter Subsystem HIP instance
 * to be used
 */
#define XDPTXSS_DEVICE_ID		XPAR_DPTXSS_0_DEVICE_ID
#define VIDEO_CRC_BASEADDR      XPAR_VIDEO_FRAME_CRC_RX_BASEADDR

/* If set to 1, example will run in MST mode. Otherwise, in SST mode.
 * In MST mode, this example reads the EDID of RX devices if connected in
 * daisy-chain.
 */
#define DPTXSS_MST			1
#define DPTXSS_LINK_RATE		XDPTXSS_LINK_BW_SET_540GBPS
#define DPTXSS_LANE_COUNT		XDPTXSS_LANE_COUNT_SET_4

#define PARRETO_FMC //enable this if parreto fmc is used else disable for diode fmc

/* DEFAULT VALUE=0. Enabled programming of
 *  *Rx Training Algo Register for Debugging Purpose
 *   */
#define LINK_TRAINING_DEBUG     0

/*EDID Selection*/
#define DP12_EDID_ENABLED 0

/* The video resolution from the display mode timings (DMT) table to use for
 * DisplayPort TX Subsystem. It can be set to use preferred video mode for
 * EDID of RX device.
 */
#define DPTXSS_VID_MODE			XVIDC_VM_USE_EDID_PREFERRED

/* The color depth (bits per color component) to use DisplayPort TX
 * Subsystem.
 */
#define DPTXSS_BPC			XPAR_DPTXSS_0_BITS_PER_COLOR

#ifndef SDT
#define SET_TX_TO_2BYTE		\
		(XPAR_XDP_0_GT_DATAWIDTH/2)
#else
#define SET_TX_TO_2BYTE		\
		(XPAR_XDP_0_GT_DATA_WIDTH / 2)
#endif

#define TIMER_RESET_VALUE				1000
#define is_TX_CPLL 0
#define CLK135MHz_DIVIDER 18
#define CLK270MHz_DIVIDER 9
#define CLK162MHz_DIVIDER 15
#define ENABLE_AUDIO 1
#if (ENABLE_AUDIO == 1)
	#define AV_PAT_GEN_BASE  XPAR_TX_SUBSYSTEM_AV_PAT_GEN_0_BASEADDR
#endif

#define audio_clk_Hz 24.576
#define EEPROM_TEST_START_ADDRESS       0x80
#define PAGE_SIZE                       16
#define NUM_MODES                       7
#define NUM_CLOCK_REGS                  6

#ifndef PLATFORM_MB
#define XINTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#else
#define XINTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#endif
#define XVPHY_DEVICE_ID		XPAR_VPHY_0_DEVICE_ID
#define CLK_WIZ_BASE      	XPAR_CLK_WIZ_0_BASEADDR
#ifndef SDT
#define IIC_BASE_ADDR 		XPAR_IIC_0_BASEADDR
#else
#define IIC_BASE_ADDR		XPAR_XIIC_0_BASEADDR
#endif
#define IIC_DEVICE_ID       XPAR_IIC_0_DEVICE_ID
#define PE_VS_ADJUST 1
#define XVPHY_DRP_CPLL_FBDIV		0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV	0x2A
#define XVPHY_DRP_RXOUT_DIV			0x63
#define XVPHY_DRP_RXCLK25			0x6D
#define XVPHY_DRP_TXCLK25			0x7A
#define XVPHY_DRP_TXOUT_DIV			0x7C
#define XVPHY_DRP_PROGDIV           0x3E

// The following are the PROGDIVCLK divider values when BufferBypass is
// enabled
#define DIVIDER_162                 57423
#define DIVIDER_270                 57415
#define DIVIDER_540                 57442
#define DIVIDER_810                 57440

#define	XVPHY_GTHE4_PREEMP_DP_L0    0x3
#define	XVPHY_GTHE4_PREEMP_DP_L1    0xD
#define	XVPHY_GTHE4_PREEMP_DP_L2    0x16
#define	XVPHY_GTHE4_PREEMP_DP_L3    0x1D

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

#define COLOR_FORMAT_SHIFT 4
#define BPC_SHIFT 8
#define DYNAMIC_RANGE_SHIFT 15

#if !defined (XPS_BOARD_ZCU102)
#define CRC_CFG 0x5
#else
#define CRC_CFG 0x4
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/*The structure defines Generic Frame Packet fields*/
typedef struct
{
        u32 frame_count;
        u32 frame_count_q;
        u8 Header[4];
        u8 Payload[32];
} XilAudioExtFrame;

//XilAudioExtFrame  SdpExtFrame;
//XilAudioExtFrame  SdpExtFrame_q;


typedef struct
{
	 XVidC_VideoMode VideoMode_local;
	unsigned char user_bpc;
	unsigned char user_pattern;
	unsigned char user_format;
	unsigned int user_numStreams;
	unsigned int user_stream_number;
	unsigned int mst_check_flag;
}user_config_struct;


typedef struct
{
        unsigned char lane_count;
        unsigned char link_rate;
}lane_link_rate_struct;


/************************** Function Prototypes ******************************/
void hpd_con(XDpTxSs *InstancePtr, u8 Edid_org[128], u8 Edid1_org[128],
		u16 res_update);
void hpd_pulse_con(XDpTxSs *InstancePtr);
char xil_getc(u32 timeout_ms);
void Vpg_Audio_start(void);
void Vpg_Audio_stop(void);
u32 start_tx(u8 line_rate, u8 lane_count, user_config_struct user_config);
u32 config_phy(int LineRate_init_tx);
#ifdef PARRETO_FMC
int i2c_write_freq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress,
                u32 Value);
u8 i2c_read_freq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress);
u8 i2c_read_tdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress);
int i2c_write_tdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress, u8 Value);
#endif
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void sink_power_down(void);
void sink_power_up(void);
u8 get_LineRate(void);
u8 get_Lanecounts(void);
void sink_power_cycle(void);
void DpPt_pe_vs_adjustHandler(void *InstancePtr);
int VideoFMC_Init(void);
int IDT_8T49N24x_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress, int FIn,
		int FOut, u8 FreeRun);
int IDT_8T49N24x_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);

/************************** Variable Definitions *****************************/
typedef enum {
        ONBOARD_REF_CLK = 1,
#ifdef PLATFORM_MB
		ONBOARD_400_CLK = 2,
#else
		ONBOARD_400_CLK = 3,
#endif
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
u32 PHY_Configuration_Tx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table);
#ifdef __cplusplus
}
#endif
#endif /* SRC_XDPTXSS_ZCU102_TX_H_ */
