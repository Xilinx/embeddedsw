/******************************************************************************
*
* Copyright (C) 2015-2019 Xilinx, Inc.  All rights reserved.
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

/*****************************************************************************/
/**
 * @file pm_common.h
 *
 * Definitions of commonly used macros and data types needed for
 * PU Power Management. This file should be common for all PU's.
 *****************************************************************************/

#ifndef PM_COMMON_H
#define PM_COMMON_H

#include "xparameters.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xipipsu.h"
#include "pm_defs.h"
#ifdef DEBUG_MODE
#include "xil_printf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PM_ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

/* 1 for API ID + 5 for API arguments + 1 for Reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT		8U

/* 1 for status + 3 for values + 3 for Reserved + 1 for CRC */
#define RESPONSE_ARG_CNT	8U

#define PM_IPI_TIMEOUT		(~0U)

#define IPI_PMU_PM_INT_MASK	XPAR_XIPIPS_TARGET_PSU_PMU_0_CH0_MASK

/**
 * XPm_Master - Master structure
 */
struct XPm_Master {
	const enum XPmNodeId node_id; /**< Node ID */
	const u32 pwrctl;             /** < Power Control Register Address */
	const u32 pwrdn_mask;         /**< Power Down Mask */
	XIpiPsu *ipi;                 /**< IPI Instance */
};

enum XPmNodeId pm_get_subsystem_node(void);
struct XPm_Master *pm_get_master(const u32 cpuid);
struct XPm_Master *pm_get_master_by_node(const enum XPmNodeId nid);

#define APU_0_PWRCTL_CPUPWRDWNREQ_MASK	0x00000001U
#define APU_1_PWRCTL_CPUPWRDWNREQ_MASK	0x00000002U
#define APU_2_PWRCTL_CPUPWRDWNREQ_MASK	0x00000004U
#define APU_3_PWRCTL_CPUPWRDWNREQ_MASK	0x00000008U

#define IPI_W0_TO_W6_SIZE		28U
#define IPI_RPU_MASK			0x00000100U

#define UNDEFINED_CPUID		(~0U)

#define pm_read(addr)		Xil_In32(addr)
#define pm_write(addr, value)	Xil_Out32(addr, value)
#define pm_enable_int()		Xil_ExceptionEnable()
#define pm_disable_int()	Xil_ExceptionDisable()

/* Conditional debugging prints */
#ifdef DEBUG_MODE
#define pm_dbg xil_printf
#else
	#define pm_dbg(...)	{}
#endif

#ifndef bool
	#define bool	u8
	#define true	1U
	#define false	0U
#endif

void XPm_ClientSuspend(const struct XPm_Master *const master);
void XPm_ClientAbortSuspend(void);
void XPm_ClientWakeup(const struct XPm_Master *const master);
void XPm_ClientSuspendFinalize(void);
void XPm_ClientSetPrimaryMaster(void);

/* Do not modify below this line */
extern const enum XPmNodeId subsystem_node;
extern struct XPm_Master *primary_master;

#ifdef __cplusplus
}
#endif

#endif /* PM_COMMON_H */
