/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
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
 * Ver   Who     Date     Changes
 * ----- ----    -------- -----------------------------------------------
 * 1.00  Nishant 19/12/20 Added suppport for vck190
 *</pre>
 *
*****************************************************************************/

#ifndef SRC_XDPTXSS_ZCU102_TX_H_
#define SRC_XDPTXSS_ZCU102_TX_H_

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

#ifdef versal
#include "xuartpsv_hw.h"
#include <xdp.h>
#include <xdp_hw.h>
#include "xiicps.h"
#include "xclk_wiz.h"
#include "xgpio.h"
#else
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
#endif

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
	XPAR_FABRIC_DP14TXSS_0_VEC_ID

#define XINTC				XScuGic
#define XINTC_HANDLER			XScuGic_InterruptHandler
#else
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_INTC_0_DP14TXSS_0_VEC_ID

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

#define SET_TX_TO_2BYTE		\
		(XPAR_XDP_0_GT_DATAWIDTH/2)

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

#ifdef versal
#define GT_QUAD_BASE  				XPAR_GT_QUAD_HIER_0_GT_QUAD_BASE_BASEADDR
#define TXCLKDIV_REG				0x3694
#define	DIV3 						0x00000278
#define	DIV 						0x00000260
#define DIV_MASK 					0x000003FF

#define XINTC_DEVICE_ID			        XPAR_SCUGIC_0_DEVICE_ID
#define CLK_WIZ_BASE      				XPAR_CLK_WIZARD_1_BASEADDR
#define XVPHY_DEVICE_ID					0
#define IIC_BASE_ADDR 					0
#define IIC_DEVICE_ID                   0
#define SINGLE_LANE 					0x00000011
#define ALL_LANE						0x0000003F
#define VERSAL_810G                     3
#define VERSAL_540G                     2
#define VERSAL_270G                     1
#define VERSAL_162G                     0
/* For Versal the DP and GT are interfaced in RAW16 mode
 * 8b10b is implemented in Fabric. This requires two clocks to be
 * generated from GT, /16 clock and /20 clock,
 * ch0txoutclk generates /16 clk
 * ch1tcoutclk is used to generate /20 clk
 * In case if the hardware is built for 1 lane, then a MMCM is to be
 * used to generate the /20 clock
 * Refer to the PassThrough design to see generation of /20 clk on RX side
 * Similarly, the /20 clk for TX be generated for 1 lane design.
 * if the hardware is generated for 2 or 4 lanes, then there is no need of
 * MMCM.
 */
#define VERSAL_FABRIC_8B10B             1
#else
#ifndef PLATFORM_MB
#define XINTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#else
#define XINTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#endif
#define XVPHY_DEVICE_ID		XPAR_VPHY_0_DEVICE_ID
#define CLK_WIZ_BASE      	XPAR_CLK_WIZ_0_BASEADDR
#define IIC_BASE_ADDR 		XPAR_IIC_0_BASEADDR
#define IIC_DEVICE_ID       XPAR_IIC_0_DEVICE_ID
#endif

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

// Following values of VSwing and Pre-emphasis have been identified
// for GTHE4. These have been tweaked for ZCU102 and VFMC card & have been
// found to be passing the PHY compliance requirements
// It is not necessary that these values would work across any design
// Users should update these values to get the PHY compliance working
// on custom setups.

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

#define TX_BUFFER_BYPASS            XPAR_VID_PHY_CONTROLLER_0_TX_BUFFER_BYPASS


/* This switch is used to enable PHY compliance mode. */
// Compliance mode only supported for ZCU102
#define PHY_COMP 0

#define COLOR_FORMAT_SHIFT 4
#define BPC_SHIFT 8
#define DYNAMIC_RANGE_SHIFT 15
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

void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void sink_power_down(void);
void sink_power_up(void);
u8 get_LineRate(void);
u8 get_Lanecounts(void);
void sink_power_cycle(void);
int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress,
		u8 Value);
void DpPt_pe_vs_adjustHandler(void *InstancePtr);
int VideoFMC_Init(void);
int IDT_8T49N24x_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress, int FIn,
		int FOut, u8 FreeRun);
int IDT_8T49N24x_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress);

/************************** Variable Definitions *****************************/
#ifndef versal
//XVphy VPhyInst;	/* The DPRX Subsystem instance.*/
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
u32 PHY_Configuration_Tx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table);

#else
void ReadModifyWrite(u32 MaskValue, u32 data);
#endif

//XTmrCtr TmrCtr; /* Timer instance.*/


//int tx_is_reconnected; /*This variable to keep track of the status of Tx link*/

#endif /* SRC_XDPTXSS_ZCU102_TX_H_ */
