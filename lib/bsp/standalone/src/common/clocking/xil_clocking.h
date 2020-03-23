/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xil_clocking.h
*
* The xil_clocking.h file contains clocking related functions and macros.
* certain conditions.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 7.2 sd  12/11/19 First release
* 7.2 sd  03/20/20 Added checking for isolation case
* </pre>
*
******************************************************************************/

#ifndef XIL_CLOCKING_H	/* prevent circular inclusions */
#define XIL_CLOCKING_H	/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xdebug.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xparameters.h"
#include "xstatus.h"
#if defined  (XPAR_XCRPSU_0_DEVICE_ID)
#include "xclockps.h"
#else
typedef u32 XClock_OutputClks;
typedef u64 XClockRate;
#endif

/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/


XStatus Xil_ClockDisable(XClock_OutputClks ClockId);
XStatus Xil_ClockEnable(XClock_OutputClks ClockId);
XStatus Xil_ClockInit(void);
XStatus Xil_ClockGet(void);
XStatus Xil_ClockSetRate(XClock_OutputClks ClockId, XClockRate Rate, XClockRate *SetRate);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
