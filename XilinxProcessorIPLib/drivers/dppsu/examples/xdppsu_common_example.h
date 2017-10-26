/*******************************************************************************
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
/******************************************************************************/
/**
 *
 * @file xdppsu_example_common.h
 *
 * Contains a design example using the XDpPsu driver. It performs a self test on
 * the DisplayPort TX core by training the main link at the maximum common
 * capabilities between the TX and RX and checking the lane status.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  09/17/17 Initial creation.
 * </pre>
 *
*******************************************************************************/

#ifndef XDPPSU_EXAMPLE_COMMON_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPPSU_EXAMPLE_COMMON_H_

/******************************* Include Files ********************************/

#include "xdppsu.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"

/**************************** Constant Definitions ****************************/

/* The unique device ID of the DisplayPort TX core instance to be used with the
 * examples. */
#define DPPSU_DEVICE_ID XPAR_PSU_DP_DEVICE_ID

/* The link rate setting to begin link training with. Valid values are:
 * XDPPSU_LINK_BW_SET_540GBPS, XDPPSU_LINK_BW_SET_270GBPS, and
 * XDPPSU_LINK_BW_SET_162GBPS. */
#define TRAIN_USE_LINK_RATE XDPPSU_LINK_BW_SET_540GBPS
/* The lane count setting to begin link training with. Valid values are:
 * XDPPSU_LANE_COUNT_SET_4, XDPPSU_LANE_COUNT_SET_2, and
 * XDPPSU_LANE_COUNT_SET_1. */
#define TRAIN_USE_LANE_COUNT XDPPSU_DPCD_LANE_COUNT_SET_2
/* If set to 1, TRAIN_USE_LINK_RATE and TRAIN_USE_LANE_COUNT will be ignored.
 * Instead, the maximum common link capabilities between the DisplayPort TX core
 * and the RX device will be used when establishing a link.
 * If set to 0, TRAIN_USE_LINK_RATE and TRAIN_USE_LANE_COUNT will determine the
 * link rate and lane count settings that the link training process will begin
 * with. */
#define TRAIN_USE_MAX_LINK 1

/**************************** Function Prototypes *****************************/

extern u32 DpPsu_PlatformInit(void);
extern u32 DpPsu_StreamSrcSync(XDpPsu *InstancePtr);
extern u32 DpPsu_StreamSrcSetup(XDpPsu *InstancePtr);
extern u32 DpPsu_StreamSrcConfigure(XDpPsu *InstancePtr);

u32 DpPsu_SetupExample(XDpPsu *InstancePtr, u16 DeviceId);
u32 DpPsu_StartLink(XDpPsu *InstancePtr);
u32 DpPsu_Run(XDpPsu *InstancePtr);

/*************************** Variable Declarations ****************************/

XDpPsu DpPsuInstance;

#endif /* XDPPSU_EXAMPLE_COMMON_H_ */
