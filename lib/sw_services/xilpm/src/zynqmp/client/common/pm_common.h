/******************************************************************************
* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @{
 * @cond xilpm_internal
 */
#define PM_ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

/* 1 for API ID + 5 for API arguments + 1 for Reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT		XIPIPSU_MAX_MSG_LEN

/* 1 for status + 3 for values + 3 for Reserved + 1 for CRC */
#define RESPONSE_ARG_CNT	XIPIPSU_MAX_MSG_LEN

#define PM_IPI_TIMEOUT		(~0U)

#define IPI_PMU_PM_INT_MASK	XPAR_XIPIPS_TARGET_PSU_PMU_0_CH0_MASK

#define BIT32(n)				((u32)1U << (n))

/**
 * XPm_Master - Master structure
 */
struct XPm_Master {
	const enum XPmNodeId node_id; /**< Node ID */
	const u32 pwrctl;             /**< Power Control Register Address */
	const u32 pwrdn_mask;         /**< Power Down Mask */
	XIpiPsu *ipi;                 /**< IPI Instance */
};

struct XPm_Master *pm_get_master(const u32 cpuid);
struct XPm_Master *pm_get_master_by_node(const enum XPmNodeId nid);

#define APU_0_PWRCTL_CPUPWRDWNREQ_MASK	0x00000001U
#define APU_1_PWRCTL_CPUPWRDWNREQ_MASK	0x00000002U
#define APU_2_PWRCTL_CPUPWRDWNREQ_MASK	0x00000004U
#define APU_3_PWRCTL_CPUPWRDWNREQ_MASK	0x00000008U

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

void XPm_ClientSuspend(const struct XPm_Master *const master);
void XPm_ClientAbortSuspend(void);
void XPm_ClientWakeup(const struct XPm_Master *const master);
void XPm_ClientSuspendFinalize(void);
void XPm_ClientSetPrimaryMaster(void);

/* Do not modify below this line */
extern const enum XPmNodeId subsystem_node;
extern struct XPm_Master *primary_master;

/**
 * @}
 * @endcond
 */
#ifdef __cplusplus
}
#endif

#endif /* PM_COMMON_H */
