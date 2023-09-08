/******************************************************************************
* Copyright (C) 2018 – 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_example.h
*
* This file contains set of definition for the main application
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
*              dd/mm/yy
* ----- ------ -------- --------------------------------------------------
* 1.00  YB     11/05/19 Initial release.
* </pre>
*
******************************************************************************/
#ifndef _XHDMI_EXAMPLE_H_
/**  prevent circular inclusions by using protection macros */
#define _XHDMI_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"

#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
#include "xiicps.h"
#else
#include "xiic.h"
#include "xiicps.h"
#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
#endif

#include "xil_io.h"

#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
#include "xuartlite_l.h"
#else
#include "xuartps.h"
#endif

#include "xil_types.h"
#include "xil_exception.h"
#include "string.h"

#if (defined XPS_BOARD_ZCU102)
#include "si570drv.h"
#elif (defined XPS_BOARD_ZCU106)
#include "si570drv.h"
#elif (defined XPS_BOARD_VCU118)
#else /* Place Holder for other board */
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#include "xhdmi_exdes_sm_tx.h"
#endif

#include "xhdmi_menu.h"
#include "video_fmc.h"
#include "xvidc.h"
#include "xv_hdmic.h"
#include "xv_hdmic_vsif.h"
#include "sleep.h"
#include "xhdmi_edid.h"

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR
/* This is only required for the audio over HDMI */
#define USE_HDMI_AUDGEN
#endif
#endif

#include "xhdmiphy1.h"
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
#include "xv_tpg.h"
#endif
#endif

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
#include "xv_axi4s_remap.h"
#endif

#ifdef XPAR_XGPIO_NUM_INSTANCES
#include "xgpio.h"
#endif

#ifdef XPAR_XGPIOPS_NUM_INSTANCES
#include "xgpiops.h"
#endif

#if defined (ARMR5) || (__aarch64__) || (__arm__)
#include "xscugic.h"
#else
#include "xintc.h"
#endif

#include "xtmrctr.h"

#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
#include "xv_frmbufwr_l2.h"
#include "xv_frmbufrd_l2.h"
#endif

#if defined (XPAR_XAXIS_SWITCH_NUM_INSTANCES)
#include "xaxis_switch.h"
#endif

/* AUXFIFOSIZE: Must be set to 3 or higher*/
#define AUXFIFOSIZE 10

#if defined (XPAR_XUARTPSV_NUM_INSTANCES )
#define UART_BASEADDR XPAR_XUARTPSV_0_BASEADDR
#elif defined (XPAR_XUARTLITE_NUM_INSTANCES)
#define UART_BASEADDR XPAR_MB_SS_0_AXI_UARTLITE_BASEADDR
#else
#define UART_BASEADDR XPAR_XUARTPS_0_BASEADDR
#endif

/******************************** OPTIONS ************************************/
/* These macro values need to changed whenever there is a change in version */
#define APP_MAJ_VERSION 1
#define APP_MIN_VERSION 0

#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
/* Define Maximum Supported Memory Color Format */
#define NUM_MEMORY_COLOR_FORMATS 17
#endif

/************************** Constant Definitions *****************************/
/* OnBoard_IicDev Definitions */
typedef enum {
	KCU105_SI570 = 1,
	KCU105_HPC,
	ZCU102_MGT_SI570,
	ZCU102_SI5328,
	ZCU106_MGT_SI570,
	VCU118_FMCP,
	VCK190_MGT_SI570,
	VEK280_ES_MGT_SI570
} XOnBoard_IicDev;

#if defined (XPS_BOARD_ZCU102)
/* TCA9528 (U34) Definitions */
#define ZCU102_U34_MUX_I2C_ADDR		0x74
#define ZCU102_U34_MUX_SEL_NONE		0x80
#define ZCU102_U34_MUX_MGTSI570_ADDR	0x5D
#define ZCU102_U34_MUX_SEL_SI570	0x08
/* TCA9528 (U34) Definitions */
#define ZCU102_U135_MUX_I2C_ADDR	0x75
#define ZCU102_U135_MUX_SEL_HPC1	0x02
#elif defined (XPS_BOARD_ZCU106)
/* TCA9528 (U34) Definitions */
#define ZCU106_U34_MUX_I2C_ADDR		0x74
#define ZCU106_U34_MUX_SEL_NONE		0x80
#define ZCU106_U34_MUX_MGTSI570_ADDR	0x5D
#define ZCU106_U34_MUX_SEL_SI570	0x08
/* TCA9528 (U34) Definitions */
#define ZCU106_U135_MUX_I2C_ADDR	0x75
#define ZCU106_U135_MUX_SEL_HPC0	0x01
#elif defined (XPS_BOARD_VCU118)
/* TCA9548 (U28) Definitions */
#define VCU118_U28_MUX_I2C_ADDR		0x74
#define VCU118_U28_MUX_SEL_NONE		0x02
#define VCU118_U28_MUX_SI570_ADDR	0x5D
#define VCU118_U28_MUX_SEL_SI570	0x20
/* TCA9548 (U80) Definitions */
#define VCU118_U80_MUX_I2C_ADDR		0x75
#define VCU118_U80_MUX_SEL_FMCP		0x02
#define VCU118_U80_MUX_SEL_NONE		0x40
#elif defined (XPS_BOARD_VCK190)
/* TCA9528 (U34) Definitions */
#define VCK190_U34_MUX_I2C_ADDR		0x74
#define VCK190_U34_MUX_SEL_NONE		0x80
#define VCK190_U34_MUX_MGTSI570_ADDR	0x5D
#define VCK190_U34_MUX_SEL_SI570	0x40
/* TCA9528 (U34) Definitions */
#define VCK190_U135_MUX_I2C_ADDR	0x75
#define VCK190_U135_MUX_SEL_HPC0	0x02
#elif defined (XPS_BOARD_VEK280)
/* TCA9528 (U34) Definitions */
#define VEK280_ES_U34_MUX_I2C_ADDR		0x74
#define VEK280_ES_U34_MUX_SEL_NONE		0x80
#define VEK280_ES_U34_MUX_MGTSI570_ADDR	0x5D
#define VEK280_ES_U34_MUX_SEL_SI570	0x40
/* TCA9528 (U34) Definitions */
#define VEK280_ES_U135_MUX_I2C_ADDR	0x75
#define VEK280_ES_U135_MUX_SEL_HPC0	0x02
#else
/* TCA9528 (U28) Definitions */
#define KCU105_U28_MUX_I2C_ADDR		0x74
#define KCU105_U28_MUX_SEL_NONE		0x07
#define KCU105_U28_MUX_SI570_ADDR	0x5D
#define KCU105_U28_MUX_SEL_SI570	0x01
/* PCA9544 (U80) Definitions */
#define KCU105_U80_MUX_I2C_ADDR		0x75
#define KCU105_U80_MUX_SEL_HPC		0x05
#define KCU105_U80_MUX_SEL_NONE		0x00
#endif

#define I2C_MUX_ADDR		0x74  /**< I2C Mux Address */
#define I2C_CLK_ADDR		0x6C  /**< I2C Clk Address IDT_8T49N241*/

#define VRR_MODE   1  /** 0 - NO VRR , 1 - MANUAL STRETCH , 2 - AUTO STRETCH */
                      /** Note : In Auto Stretch Mode, Enable/Disable VRR is not supported.
		                 VRR/FSYNC is always on in VTEM/AMD VSIF  Packet  */

#define FVA_FACTOR 1
#define CNMVRR     1
#define EDID_INIT  1 // 0 - Default, 1- 2.1 VRR EDID, 2 - AMD FreeSync EDID, 3 - HDMI 2.1 VRR TMDS

#define RC21008A_ADDR   0x09 /**<I2C RC21008A Address */

//#define  VTEM2FSYNC 1 // Enable When RX is VRR and TX is Fsync
/* Defining constants for colors in printing */
#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_WHITE    "\x1b[37m"
#define ANSI_COLOR_RESET    "\x1b[0m"

/**************************** Type Definitions *******************************/
/**
 * Define a function pointer for user enabled printing.
 */
typedef void (*Exdes_Debug_Printf)(const char *fmt, ...);

extern Exdes_Debug_Printf exdes_debug_print;
extern Exdes_Debug_Printf exdes_aux_debug_print;
extern Exdes_Debug_Printf exdes_hdcp_debug_print;

/* Macros for printing additional debugging messages. */
#define EXDES_DBG_PRINT \
	if (exdes_debug_print != NULL) exdes_debug_print
#define EXDES_AUXFIFO_DBG_PRINT \
	if (exdes_aux_debug_print != NULL) exdes_aux_debug_print
#define EXDES_HDCP_DBG_PRINT \
	if (exdes_hdcp_debug_print!= NULL) exdes_hdcp_debug_print

/**
 * This typedef defines the handler for the pass-thorugh
 * example design that handles the state of the pass-through design.
 */
typedef struct {
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	XV_Tx *hdmi_tx_ctlr; /**< Reference to the hdmi
	                       *  tx state machine controller. */
#endif
	XTmrCtr SysTmrInst;  /**< Generic timer instance for control
	                       *  and exception handlers. */
	u32 SysTmrPulseIntervalinMs; /**< User specified pulse interval
	                       *  for system timer. */

	u32 HdcpPulseCounter;
	u32 TmrPulseCnt1second;

	u8 IsTxPresent;	     /**< Flag to track the presence of TX. */
	u8 IsRxPresent;      /**< Flag to track the presence of RX. */

	u8 ForceIndependent;  /**< Flag to track independent or pass-through
			       *  mode of operation of the example design. */

	u8 SystemEvent;      /**< This flag tracks the happening of an
			       *  'event' on the system, which can the
			       *  detection of a new rx stream or a new
			       *  tx sink. */
	u8 TxStartTransmit;  /**< This flag allows the example design to
			       *  hold/allow the tranmission of a new
			       *  stream on the transmitter. */
	u8 TxBusy;           /**< This flag is set while the TX is initialized.
			       * This flag is used by the menu to check if TX operations
			       * are ongoing and block the menu prints from disrupting
			       * the time-sensitive operations of the TX. */
	u8 crop;       /* indicates if TX video needs to be scaled down to 4K
	               * this is a flag */
} XHdmi_Exdes;

/**
 * This enumeration defines the 'type' of sources that can provide
 * a video stream for the transmitter to output in the example design.
 */
typedef enum {
	EXDES_TX_INPUT_NONE_WAITFORNEWSTREAM,
	EXDES_TX_INPUT_NONE_NOCONNECTIONS,
	EXDES_TX_INPUT_NONE_RXONLY,
	EXDES_TX_INPUT_TPG,
	EXDES_TX_INPUT_RX,
} TxInputSourceType;

/************************** Variable Definitions *****************************/
/* VPhy structure */
extern XHdmiphy1     Hdmiphy1;

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES

/* HDMI TX SS structure */
extern XV_HdmiTxSs1 HdmiTxSs;

#ifdef USE_HDMI_AUDGEN
extern XhdmiAudioGen_t AudioGen;
#endif

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/* TPG structure */
extern XV_tpg Tpg;
extern XTpg_PatternId Pattern;
#endif

#endif /* XPAR_XV_HDMITXSS1_NUM_INSTANCES */

extern u8 AuxFifoStartFlag;
#define PS_IIC_CLK 100000
#define TX_RX_RATE 0

extern XIicPs Ps_Iic0, Iic1;
extern XIic Iic;

extern XHdmi_Exdes xhdmi_exdes_ctrlr;


/************************** Function Prototypes ******************************/
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
void SendVSInfoframe(XV_HdmiTxSs1 *HdmiTxSs1Ptr);

void Exdes_ConfigureTpgEnableInput(u32 EnableExtSrcInput);
void Exdes_ChangeColorbarOutput(XVidC_VideoMode VideoMode,
				XVidC_ColorFormat ColorFormat,
				XVidC_ColorDepth Bpc);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _XHDMI_EXAMPLE_H_ */
