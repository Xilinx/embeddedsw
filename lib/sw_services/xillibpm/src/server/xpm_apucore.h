/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef XPM_APUCORE_H_
#define XPM_APUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_ACPU_0_CPUPWRDWNREQ_MASK  BIT(0)
#define XPM_ACPU_1_CPUPWRDWNREQ_MASK  BIT(1)
#define XPM_ACPU_0_PWR_CTRL_MASK	BIT(0)
#define XPM_ACPU_1_PWR_CTRL_MASK	BIT(1)

typedef struct XPm_ApuCore XPm_ApuCore;

/**
 * The APU core class.
 */
struct XPm_ApuCore {
	XPm_Core Core; /**< Processor core devices */
};

/************************** Function Prototypes ******************************/
XStatus XPmApuCore_Init(XPm_ApuCore *ApuCore,
	u32 Id,
	u32 Ipi,
	u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_APUCORE_H_ */
