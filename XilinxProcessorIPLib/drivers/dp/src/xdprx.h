/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
 * @file xdprx.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 * </pre>
 *
*******************************************************************************/

#ifndef XDPRX_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPRX_H_

/******************************* Include Files ********************************/

#include "xdp.h"
#include "xdprx_hw.h"

/****************************** Type Definitions ******************************/

/**
 * This typedef contains configuration information about the main link settings.
 */
typedef struct {
	u8 LaneCount;			/**< The current lane count of the main
						link. */
	u8 LinkRate;			/**< The current link rate of the main
						link. */
} XDprx_LinkConfig;

/**
 * The XDprx driver instance data. The user is required to allocate a variable
 * of this type for every XDprx device in the system. A pointer to a variable of
 * this type is then passed to the driver API functions.
 */
typedef struct {
	XDp_Config Config;			/**< Configuration structure for
							the DisplayPort RX
							core. It is important to
							keep this member first
							in the XDprx order. */
	u32 IsReady;				/**< Device is initialized and
							ready. */
	XDprx_LinkConfig LinkConfig;		/**< Configuration structure for
							the main link. */
	XDp_TimerHandler UserTimerWaitUs;	/**< Custom user function for
							delay/sleep. */
	void *UserTimerPtr;			/**< Pointer to a timer instance
							used by the custom user
							delay/sleep function. */
} XDprx;

/**************************** Function Prototypes *****************************/

/* xdprx.c: Setup and initialization functions. */
void XDprx_CfgInitialize(XDprx *InstancePtr, XDp_Config *ConfigPtr,
							u32 EffectiveAddr);
u32 XDprx_InitializeRx(XDprx *InstancePtr);

/* xdprx.c: General usage functions. */
void XDprx_DtgEn(XDprx *InstancePtr);
void XDprx_DtgDis(XDprx *InstancePtr);
void XDprx_SetLinkRate(XDprx *InstancePtr, u8 LinkRate);
void XDprx_SetLaneCount(XDprx *InstancePtr, u8 LaneCount);
void XDprx_SetUserTimerHandler(XDprx *InstancePtr,
			XDp_TimerHandler CallbackFunc, void *CallbackRef);
void XDprx_WaitUs(XDprx *InstancePtr, u32 MicroSeconds);

#endif /* XDPRX_H_ */
