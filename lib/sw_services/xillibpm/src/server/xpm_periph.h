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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_PERIPH_H_
#define XPM_PERIPH_H_

#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Periph XPm_Periph;

/* Core Operations */
struct XPm_PeriphOps {
	XStatus (*SetWakeupSource)(XPm_Periph *Periph, u8 Enable);
};

/**
 * The processor core class.  This is the base class for all processor cores.
 */
struct XPm_Periph {
	XPm_Device Device; /**< Device: Base class */
	struct XPm_PeriphOps *PeriphOps; /**< Core operations */
	u32 GicProxyMask; /**< GIC Proxy Mask */
	u32 GicProxyGroup; /**< GIC Proxy Group */
};

/************************** Function Prototypes ******************************/
XStatus XPmPeriph_Init(XPm_Periph *Periph, u32 Id, u32 BaseAddress,
		       XPm_Power *Power, XPm_ClockNode *Clock,
		       XPm_ResetNode *Reset, u32 GicProxyMask,
		       u32 GicProxyGroup);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PERIPH_H_ */
