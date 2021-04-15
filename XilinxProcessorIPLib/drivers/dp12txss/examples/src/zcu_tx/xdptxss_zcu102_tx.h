/*******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/*
 * xdptxss_zcu102_tx.h
 *
 *  Created on: Aug 9, 2017
 *      Author: keiito
 */

#ifndef SRC_XDPTXSS_ZCU102_TX_H_
#define SRC_XDPTXSS_ZCU102_TX_H_





/***************************** Include Files *********************************/

#include "xdptxss.h"
#include "xvphy.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xscugic.h"
#include "xuartps_hw.h"
#include "xiic.h"
#include "xiic_l.h"
#include "xtmrctr.h"
#include "xspi.h"
#include "xvidc_edid.h"
#include "sleep.h"
#include "stdlib.h"
#include "xvidframe_crc.h"
/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
* There is only one interrupt controlled to be selected from SCUGIC and GPIO
* INTC. INTC selection is based on INTC parameters defined xparameters.h file.
*/
#define XINTC_DPTXSS_DP_INTERRUPT_ID \
	XPAR_FABRIC_DP12TXSS_0_VEC_ID
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPTXSS_HDCP_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_HDCP_IRQ_VEC_ID
#define XINTC_DPTXSS_TMR_INTERRUPT_ID \
	XPAR_INTC_0_DPTXSS_0_DPTXSS_TIMER_IRQ_VEC_ID
#endif
#define XINTC_DEVICE_ID			XPAR_SCUGIC_SINGLE_DEVICE_ID
#define XINTC				XScuGic
#define XINTC_HANDLER			XScuGic_InterruptHandler

/* The unique device ID of the DisplayPort Transmitter Subsystem HIP instance
 * to be used
 */
#define XDPTXSS_DEVICE_ID		XPAR_DPTXSS_0_DEVICE_ID

/* If set to 1, example will run in MST mode. Otherwise, in SST mode.
 * In MST mode, this example reads the EDID of RX devices if connected in
 * daisy-chain.
 */
#define DPTXSS_MST			1
#define DPTXSS_LINK_RATE		XDPTXSS_LINK_BW_SET_540GBPS
#define DPTXSS_LANE_COUNT		XDPTXSS_LANE_COUNT_SET_4

/* The video resolution from the display mode timings (DMT) table to use for
 * DisplayPort TX Subsystem. It can be set to use preferred video mode for
 * EDID of RX device.
 */
#define DPTXSS_VID_MODE			XVIDC_VM_USE_EDID_PREFERRED

/* The color depth (bits per color component) to use DisplayPort TX
 * Subsystem.
 */
#define DPTXSS_BPC			XPAR_DPTXSS_0_BITS_PER_COLOR
#define IIC_DEVICE_ID       XPAR_IIC_0_DEVICE_ID
#define XVPHY_DEVICE_ID		XPAR_VPHY_0_DEVICE_ID

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

#define CLK_WIZ_BASE      				XPAR_CLK_WIZ_0_BASEADDR




#define XVPHY_DRP_CPLL_FBDIV		0x28
#define XVPHY_DRP_CPLL_REFCLK_DIV	0x2A
#define XVPHY_DRP_RXOUT_DIV			0x63
#define XVPHY_DRP_RXCLK25			0x6D
#define XVPHY_DRP_TXCLK25			0x7A
#define XVPHY_DRP_TXOUT_DIV			0x7C

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
void hpd_con(XDpTxSs *InstancePtr, u8 Edid_org[128], u8 Edid1_org[128], u16 res_update);
void hpd_pulse_con(XDpTxSs *InstancePtr);
char xil_getc(u32 timeout_ms);
void sendAudioInfoFrame(XilAudioInfoFrame *xilInfoFrame);
void Vpg_Audio_start(void);
void Vpg_Audio_stop(void);
u32 start_tx(u8 line_rate, u8 lane_count, user_config_struct user_config);
u32 PHY_Configuration_Tx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table);
u32 set_vphy(int LineRate_init_tx);

void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);
void sink_power_down(void);
void sink_power_up(void);
u8 get_LineRate(void);
u8 get_Lanecounts(void);
void sink_power_cycle(void);

#endif /* SRC_XDPTXSS_ZCU102_TX_H_ */
