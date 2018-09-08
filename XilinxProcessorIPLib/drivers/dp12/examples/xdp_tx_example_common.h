/*******************************************************************************
 *
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
 * @file xdp_tx_example_common.h
 *
 * Contains a design example using the XDp driver (operating in TX mode). It
 * performs a self test on the DisplayPort TX core by training the main link at
 * the maximum common capabilities between the TX and RX and checking the lane
 * status.
 *
 * @note	The DisplayPort TX core does not work alone - video/audio
 *		sources need to be set up in the system correctly, as well as
 *		setting up the output path (for example, configuring the
 *		hardware system with the DisplayPort TX core output to an FMC
 *		card with DisplayPort output capabilities. Some platform
 *		initialization will need to happen prior to calling XDp driver
 *		functions. See XAPP1178 as a reference.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial creation.
 * </pre>
 *
*******************************************************************************/

#ifndef XDP_TX_EXAMPLE_COMMON_H_
/* Prevent circular inclusions by using protection macros. */
#define XDP_TX_EXAMPLE_COMMON_H_

/******************************* Include Files ********************************/

#include "xdp.h"
#include "xil_printf.h"
#include "xparameters.h"

/**************************** Constant Definitions ****************************/

/* The unique device ID of the DisplayPort TX core instance to be used with the
 * examples. */
#define DPTX_DEVICE_ID XPAR_DISPLAYPORT_0_DEVICE_ID

/* If set to 1, the link training process will continue training despite failing
 * by attempting training at a reduced link rate. It will also continue
 * attempting to train the link at a reduced lane count if needed. With this
 * option enabled, link training will return failure only when all link rate and
 * lane count combinations have been exhausted - that is, training fails using
 * 1-lane and a 1.62Gbps link rate.
 * If set to 0, link training will return failure if the training failed using
 * the current lane count and link rate settings.
 * TRAIN_ADAPTIVE is used by the examples as input to the
 * XDp_TxEnableTrainAdaptive driver function. */
#define TRAIN_ADAPTIVE 1

/* A value of 1 is used to indicate that the DisplayPort output path has a
 * redriver on the board that adjusts the voltage swing and pre-emphasis levels
 * that are outputted from the FPGA. If this is the case, the voltage swing and
 * pre-emphasis values supplied to the DisplayPort TX core will be evenly
 * distributed among the available levels as specified in the IP documentation.
 * Otherwise, a value of 0 is used to indicate that no redriver is present. In
 * order to meet the necessary voltage swing and pre-emphasis levels required by
 * a DisplayPort RX device, the level values specified to the DisplayPort TX
 * core will require some compensation.
 * TRAIN_HAS_REDRIVER is used by the examples as input to the
 * XDp_TxSetHasRedriverInPath driver function.
 * Note: There are 16 possible voltage swing levels and 32 possible pre-emphasis
 *       levels in the DisplayPort TX core that will be mapped to 4 possible
 *       voltage swing and 4 possible pre-emphasis levels in the RX device. */
#define TRAIN_HAS_REDRIVER 1

/* The link rate setting to begin link training with. Valid values are:
 * XDP_TX_LINK_BW_SET_540GBPS, XDP_TX_LINK_BW_SET_270GBPS, and
 * XDP_TX_LINK_BW_SET_162GBPS. */
#define TRAIN_USE_LINK_RATE XDP_TX_LINK_BW_SET_540GBPS
/* The lane count setting to begin link training with. Valid values are:
 * XDP_TX_LANE_COUNT_SET_4, XDP_TX_LANE_COUNT_SET_2, and
 * XDP_TX_LANE_COUNT_SET_1. */
#define TRAIN_USE_LANE_COUNT XDP_TX_LANE_COUNT_SET_4
/* If set to 1, TRAIN_USE_LINK_RATE and TRAIN_USE_LANE_COUNT will be ignored.
 * Instead, the maximum common link capabilities between the DisplayPort TX core
 * and the RX device will be used when establishing a link.
 * If set to 0, TRAIN_USE_LINK_RATE and TRAIN_USE_LANE_COUNT will determine the
 * link rate and lane count settings that the link training process will begin
 * with. */
#define TRAIN_USE_MAX_LINK 1

/**************************** Function Prototypes *****************************/

extern u32 Dptx_PlatformInit(void);
extern u32 Dptx_StreamSrcSync(XDp *InstancePtr);
extern u32 Dptx_StreamSrcSetup(XDp *InstancePtr);
extern u32 Dptx_StreamSrcConfigure(XDp *InstancePtr);

u32 Dptx_SetupExample(XDp *InstancePtr, u16 DeviceId);
u32 Dptx_StartLink(XDp *InstancePtr);
u32 Dptx_Run(XDp *InstancePtr);

/*************************** Variable Declarations ****************************/

XDp DpInstance;

#endif /* XDP_TX_EXAMPLE_COMMON_H_ */
