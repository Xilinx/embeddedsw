/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file dppt.h
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI    07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#ifndef DPAPP_H_
#define DPAPP_H_

#include <stdio.h>
#include "xbasic_types.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xgpio.h"
#include "xuartlite.h"
#include "xuartlite_l.h"
#include "xtmrctr.h"
#include "xintc.h"
#include "xvphy.h"
#include "xvidc_edid.h"
#include "xvidc.h"
#include "xdptxss.h"
#include "xdprxss.h"
#include "xdp.h"
#include "stdlib.h"
#include "xvid_pat_gen.h"
#include "xspi.h"
#include "LMK04906.h"
#include "PLL_Conf.h"

#include "xdebug.h"

#include "xvid_pat_gen.h"
#include "xlib_string.h"
#include "xedid_print_example.h"

//#include "xhdcp1x_debug.h"
//#include "xhdcp1x_example.h"

#include "xdprxss_dp159.h"
#include "xaxivdma.h"
#include "dppt_vdma.h"
#include "dppt_vid_phy_config.h"
//#include "keymgmt.h"

#define DEBUG

typedef struct
{
	 XVidC_VideoMode VideoMode_local;
	unsigned char user_bpc;
	unsigned char *user_pattern;
	unsigned int user_numStreams;
	unsigned int user_stream_number;
	unsigned int mst_check_flag;
}user_config_struct;
extern user_config_struct user_config;

u8 support_640_480_60;
u8 support_800_600_60;


typedef unsigned int    UINT32;

#define UART_BASEADDR    				\
	XPAR_PROCESSOR_SUBSYSTEM_INTERCONNECT_AXI_UARTLITE_1_BASEADDR
#define TIMER_BASEADDR   				XPAR_TMRCTR_0_BASEADDR
#define VIDPHY_BASEADDR  				XPAR_VID_PHY_CONTROLLER_0_BASEADDR
#define CLK_2_GPIO_BASEADDR             XPAR_AXI_GPIO_0_BASEADDR

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XINTC_DPTXSS_DP_INTERRUPT_ID  	\
			XPAR_INTC_0_DPTXSS_0_DPTXSS_DP_IRQ_VEC_ID
#else
#define XINTC_DPTXSS_DP_INTERRUPT_ID  	\
			XPAR_INTC_0_DP12TXSS_0_VEC_ID
#endif

#define XINTC_DPRXSS_DP_INTERRUPT_ID  	\
			XPAR_INTC_0_DP12RXSS_0_DPRXSS_DP_IRQ_VEC_ID
#define XINTC_DPRXSS_IIC_INTERRUPT_ID  	\
			XPAR_INTC_0_DPRXSS_0_DPRXSS_IIC_IRQ_VEC_ID
#define XDPRXSS_DEVICE_ID		      	XPAR_DPRXSS_0_DEVICE_ID
#define XINTC_DEVICE_ID					XPAR_INTC_0_DEVICE_ID
#define XDPTXSS_DEVICE_ID		      	XPAR_DPTXSS_0_DEVICE_ID
#define IIC_DEVICE_ID                  	\
			XPAR_PROCESSOR_SUBSYSTEM_INTERCONNECT_AXI_IIC_1_DEVICE_ID

#define XINTC_TIMER_0 					XPAR_INTC_0_TMRCTR_0_VEC_ID
#define XINTC_HDCP_TIMER_ID				\
			XPAR_INTC_0_DPRXSS_0_DPRXSS_TIMER_IRQ_VEC_ID
#define XINTC_IIC_ID					XPAR_INTC_0_IIC_0_VEC_ID

#define TIMER_RESET_VALUE				1000
#define TIMER_HDCP_STABLIZATION_VALUE	100000000



//change to 1 to enable HDCP in design
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define ENABLE_HDCP_IN_DESIGN			1
#else
#define ENABLE_HDCP_IN_DESIGN			0
#endif

#define ENABLE_HDCP_FLOW_GUIDE			0

//setting to 1 enables the auto switchover from RX to TX when cable is unplugged
//setting to 0 enables the RX only mode TX is used to display the video
#define FOR_INTERNAL                	1

//Setting Just RX to 1 does not start the TX
#define JUST_RX                     	0

//This is reserved and should not be changed
#define COMPLIANCE                  	0

//The application by default uses the EDID of the downstream monitor
//Setting this to 0 will enable an internal EDID
#define USE_MONITOR_EDID 				1

//Set CAP_OVER_RIDE to 1 to limit the capabilities to a particular value
#define CAP_OVER_RIDE 					0
#define MAX_RATE 						0x14
#define MAX_LANE 						0x4

//Audio feature in untested. Do not change this value
#define ENABLE_AUDIO                	1

//Bypass the vid_common timings
//Not all resolutions are supported in vid_common library
//bypassing the vid_common lets us transmit video on TX that is not supported
//However it is still possible that all resolutions may not get
//displayed properly
#define BYPASS_VID_COMMON 				1

#define DEBUG_MAIN_FLOW					0

#define EEPROM_TEST_START_ADDRESS       0x80
#define IIC_SI570_ADDRESS  				0x5D
#define IIC_SWITCH_ADDRESS 				0x74
#define PAGE_SIZE       				16
#define NUM_MODES 						7
#define NUM_CLOCK_REGS 					6
#define PROG_48_KHZ_MODE   				0

#define SET_TX_TO_2BYTE	(XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_GT_DATAWIDTH/2)
#define SET_RX_TO_2BYTE	(XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_GT_DATAWIDTH/2)

#define BUFFER_BYPASS            XPAR_VID_PHY_CONTROLLER_0_TX_BUFFER_BYPASS

#endif /* DPAPP_H_ */
