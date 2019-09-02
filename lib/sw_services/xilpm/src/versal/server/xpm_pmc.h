/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#ifndef XPM_PMC_H_
#define XPM_PMC_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Pmc XPm_Pmc;

/**
 * The PMC processor class.
 */
struct XPm_Pmc {
	XPm_Core Core; /**< Processor core device */
	u32 PmcIouSlcrBaseAddr; /**< PMC IOU SLCR register base address */
	u32 PmcGlobalBaseAddr; /**< PMC GLOBAL register base address */
	u32 PmcAnalogBaseAddr; /**< PMC Analog register base address */
};

/************************** Function Prototypes ******************************/
XStatus XPmPmc_Init(XPm_Pmc *Pmc, u32 DevcieId, u32 Ipi, u32 *BaseAddress,
		    XPm_Power *Power,  XPm_ClockNode *Clock,
		    XPm_ResetNode *Reset);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PMC_H_ */
