/******************************************************************************
* Copyright (C) 2016 - 2019 Xilinx, Inc.  All rights reserved.
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

#ifndef XPFW_AIB_H_
#define XPFW_AIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xstatus.h"
#include "xil_types.h"


enum XPfwAib {
	XPFW_AIB_RPU0_TO_LPD,
	XPFW_AIB_RPU1_TO_LPD,
	XPFW_AIB_LPD_TO_DDR,
	XPFW_AIB_LPD_TO_FPD,
	XPFW_AIB_LPD_TO_RPU0,
	XPFW_AIB_LPD_TO_RPU1,
	XPFW_AIB_LPD_TO_USB0,
	XPFW_AIB_LPD_TO_USB1,
	XPFW_AIB_LPD_TO_OCM,
	XPFW_AIB_LPD_TO_AFI_FS2,
	XPFW_AIB_FPD_TO_LPD_NON_OCM,
	XPFW_AIB_FPD_TO_LPD_OCM,
	XPFW_AIB_FPD_TO_AFI_FS0,
	XPFW_AIB_FPD_TO_AFI_FS1,
	XPFW_AIB_FPD_TO_GPU,
	XPFW_AIB_ID_MAX
};

/**
 * Wait times for HW related actions
 */
#define AIB_ACK_TIMEOUT	(0xFU)

void XPfw_AibEnable(enum XPfwAib AibId);
void XPfw_AibDisable(enum XPfwAib AibId);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_AIB_H_ */
