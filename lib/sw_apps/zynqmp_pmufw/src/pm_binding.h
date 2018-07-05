/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 */
/*********************************************************************
 * Function declarations to be used for integrating
 * power management within PMU firmware.
 *********************************************************************/

#ifndef PM_BINDING_H_
#define PM_BINDING_H_

#include "xil_types.h"
#include "xstatus.h"

/*********************************************************************
 * Enum definitions
 ********************************************************************/
/* Return status for checking whether IPI interrupt is a PM API call */
typedef enum {
	XPFW_PM_IPI_NOT_PM_CALL,
	XPFW_PM_IPI_IS_PM_CALL,
	XPFW_PM_IPI_SRC_UNKNOWN,
} XPfw_PmIpiStatus;

/*********************************************************************
 * Function declarations
 ********************************************************************/
/* Initialize power management firmware */
void XPfw_PmInit(void);

/* Check whether the ipi is power management related */
XPfw_PmIpiStatus XPfw_PmCheckIpiRequest(const u32 isrVal, const u32* apiId);

/* Call from IPI interrupt routine to handle PM API request */
int XPfw_PmIpiHandler(const u32 IsrMask, const u32* Payload, u8 Len);

/* Call from GPI2 interrupt routine to handle processor sleep request */
int XPfw_PmWfiHandler(const u32 srcMask);

/* Call from GPI1 interrupt routine to handle wake request */
int XPfw_PmWakeHandler(const u32 srcMask);

/* Call from DAP event handlers to inform PM about the FPD power state change */
void XPfw_DapFpdWakeEvent(void);

/* Call from DAP event handlers to inform PM about the RPU power state change */
void XPfw_DapRpuWakeEvent(void);

#endif
