/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx_example_common.h
 *
 * Contains a design example using the XDptx driver. It performs a self test on
 * the DisplayPort TX core by training the main link at the maximum common
 * capabilities between the TX and RX and checking the lane status.
 *
 * @note        The DisplayPort TX core does not work alone. Some platform
 *              initialization will need to happen prior to calling XDptx driver
 *              functions. See XAPP1178 as a reference.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a als  06/17/14 Initial creation.
 * </pre>
 *
*******************************************************************************/

#ifndef XDPTX_EXAMPLE_COMMON_H_
#define XDPTX_EXAMPLE_COMMON_H_

/******************************* Include Files ********************************/

#include "xdptx.h"
#include "xil_types.h"

/**************************** Constant Definitions ****************************/

#define TRAIN_ADAPTIVE 1
#define TRAIN_HAS_REDRIVER 1
#define USE_MAX_LINK 1
#define USE_LINK_RATE XDPTX_LINK_BW_SET_540GBPS
#define USE_LANE_COUNT 4

/**************************** Function Prototypes *****************************/

extern u32 Dptx_PlatformInit(void);
extern u32 Dptx_ConfigureVidgen(XDptx *InstancePtr);

u32 Dptx_SetupExample(XDptx *InstancePtr, u16 DeviceId);
u32 Dptx_Run(XDptx *InstancePtr, u8 LaneCount, u8 LinkRate);

/*************************** Variable Declarations ****************************/

XDptx DptxInstance;

#endif /* XDPTX_EXAMPLE_COMMON_H_ */
