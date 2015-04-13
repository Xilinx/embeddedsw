/******************************************************************************
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
******************************************************************************/

/**********************************************************************
 * CONTENT
 * Definitions of commonly used macros and data types needed for
 * PU Power Management. This file should be common for all PU's.
 *********************************************************************/

#ifndef _PM_COMMON_H_
#define _PM_COMMON_H_

#include "pm_ipi.h"
#include "pm_ipi_buffer.h"
#include "pm_defs.h"

#define DEBUG_MODE

#define PM_ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

#define PAYLOAD_ARG_CNT		5U
#define PAYLOAD_ARG_SIZE	4U	/* size in bytes */

/* Power Management IPI interrupt number */
#define PM_INT_NUM		0
#define IPI_PMU_PM_INT_BASE	(IPI_PMU_0_TRIG + (PM_INT_NUM * 0x1000))
#define IPI_PMU_PM_INT_MASK	(IPI_APU_ISR_PMU_0_MASK << PM_INT_NUM)
#if (PM_INT_NUM < 0 || PM_INT_NUM > 3)
	#error PM_INT_NUM value out of range
#endif

struct XPm_Ipi {
	const uint32_t mask;
	const uint32_t base;
	const uint32_t buffer_base;
};

struct XPm_Master {
	const enum XPmNodeId node_id;
	const uint32_t pwrdn_mask;
	const struct XPm_Ipi *const ipi;
};

const enum XPmNodeId pm_get_subsystem_node(void);
const struct XPm_Master *pm_get_master(const uint32_t cpuid);
const struct XPm_Master *pm_get_master_by_node(const enum XPmNodeId nid);


#endif /* _PM_COMMON_H_ */
