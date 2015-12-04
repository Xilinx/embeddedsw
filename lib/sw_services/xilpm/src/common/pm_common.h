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

/*
 * CONTENT
 * Definitions of commonly used macros and data types needed for
 * PU Power Management. This file should be common for all PU's.
 */

#ifndef _PM_COMMON_H_
#define _PM_COMMON_H_

#include "pm_ipi.h"
#include "pm_ipi_buffer.h"
#include "pm_defs.h"
#include "xparameters.h"

#define DEBUG_MODE

#define PM_ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

#define PAYLOAD_ARG_CNT		6U	/* 1 for API ID + 5 for API arguments */
#define PAYLOAD_ARG_SIZE	4U	/* size in bytes */

#define IPI_PMU_PM_INT_MASK	XPAR_XIPIPS_TARGET_PSU_MICROBLAZE_0_CH0_MASK

struct XPm_Ipi {
	const u32 mask;
	const u32 base;
	const u32 buffer_base;
};

struct XPm_Master {
	const enum XPmNodeId node_id;
	const u32 pwrdn_mask;
	const struct XPm_Ipi *const ipi;
};

const enum XPmNodeId pm_get_subsystem_node(void);
const struct XPm_Master *pm_get_master(const u32 cpuid);
const struct XPm_Master *pm_get_master_by_node(const enum XPmNodeId nid);

#define APU_0_PWRCTL_CPUPWRDWNREQ_MASK	0x00000001U
#define APU_1_PWRCTL_CPUPWRDWNREQ_MASK	0x00000002U
#define APU_2_PWRCTL_CPUPWRDWNREQ_MASK	0x00000004U
#define APU_3_PWRCTL_CPUPWRDWNREQ_MASK	0x00000008U
#define IPI_RPU_MASK			0x00000100U

#define UNDEFINED_CPUID		(~0)

#define pm_read(addr)		Xil_In32(addr)
#define pm_write(addr, value)	Xil_Out32(addr, value)
#define pm_enable_int()		Xil_ExceptionEnable()
#define pm_disable_int()	Xil_ExceptionDisable()

/* Conditional debugging prints */
#ifdef DEBUG_MODE
	#define pm_dbg(MSG, ...) \
		do { \
			pm_print(MSG,##__VA_ARGS__); \
		} while (0)
#else
	#define pm_dbg(MSG, ...)	{}
#endif

#ifndef bool
	#define bool	u8
	#define true	1U
	#define false	0U
#endif

void XPm_ClientSuspend(const struct XPm_Master *const master);
void XPm_ClientAbortSuspend(void);
void XPm_ClientWakeup(const struct XPm_Master *const master);

/* Do not modify below this line */
extern const enum XPmNodeId subsystem_node;
extern const struct XPm_Master *primary_master;

#endif /* _PM_COMMON_H_ */
