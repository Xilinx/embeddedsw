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
 */

/*********************************************************************
 * Contains all functions, datas and definitions needed for
 * managing processor's states.
 *********************************************************************/

#ifndef PM_PROC_H_
#define PM_PROC_H_

#include "pm_common.h"
#include "pm_node.h"

typedef u8 PmProcEvent;

/*********************************************************************
 * Macros
 ********************************************************************/
/* Used for designated initialization */
#define PM_PROC_APU_0   0U
#define PM_PROC_APU_1   1U
#define PM_PROC_APU_2   2U
#define PM_PROC_APU_3   3U
#define PM_PROC_APU_MAX 4U

#define PM_PROC_RPU_0   0U
#define PM_PROC_RPU_1   1U
#define PM_PROC_RPU_MAX 2U

#define DISABLE_WFI(mask)   XPfw_RMW32(PMU_LOCAL_GPI2_ENABLE, (mask), ~(mask));

/*
 * Processor is powered down as requested by a master which is priviledged
 * to request so. Processor has not saved its context.
 */
#define PM_PROC_STATE_FORCEDOFF     0U

/*
 * Processor sleep state (specific for the processor implementation,
 * if processor has its own power domain it is powered down)
 */
#define PM_PROC_STATE_SLEEP         2U

/*
 * Processor active state. If it executes WFI without previously requesting
 * suspend through PM API it is considered active.
 */
#define PM_PROC_STATE_ACTIVE        1U

/*
 * Processor suspending state. It has called pm_self_suspend but WFI
 * interrupt from this processor is not yet received.
 */
#define PM_PROC_STATE_SUSPENDING    3U

/* Triggered when pm_self_suspend call is received for a processor */
#define PM_PROC_EVENT_SELF_SUSPEND  1U

/*
 * Triggered by pm_abort_suspend call made by a processor to cancel its
 * own suspend.
 */
#define PM_PROC_EVENT_ABORT_SUSPEND 2U

/* Triggered when processor has executed wfi instruction */
#define PM_PROC_EVENT_SLEEP         3U

/* Triggered when a master requested force powerdown for this processor */
#define PM_PROC_EVENT_FORCE_PWRDN   4U

/* Triggered when PMU receives wake interrupt targeted to the processor */
#define PM_PROC_EVENT_WAKE          5U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
typedef struct PmMaster PmMaster;
typedef struct PmProc PmProc;

/**
 * PmProc - Processor node's structure
 * @node            Processor's node structure
 * @resumeAddress   Address from which processor should resume
 *                  resumeAddress BIT0=1 indicates valid address
 * @master          Master channel used by this processor
 * @saveResumeAddr  Pointer to function for saving the resume address
 * @restoreResumeAddr Pointer to function for restoring resume address
 * @wfiStatusMask   Mask in PM_IOMODULE_GPI2 register (WFI interrupt)
 * @wakeStatusMask  Mask in PM_IOMODULE_GPI1 register (GIC wake interrupt)
 * @wfiEnableMask   Mask in PM_LOCAL_GPI2_ENABLE register (WFI interrupt)
 * @wakeEnableMask  mask in PM_LOCAL_GPI1_ENABLE register (GIC wake interrupt)
 * @resumeCfg       Address of register configuring processor's resume address
 * @latencyReq      Latenct requirement as passed in by self_suspend argument
 * @pwrDnLatency    Latency (in us) for transition to OFF state
 * @pwrUpLatency    Latency (in us) for transition to ON state
 */
typedef struct PmProc {
	PmNode node;
	u64 resumeAddress;
	PmMaster* const master;
	int (*const saveResumeAddr)(PmProc* const, u64);
	void (*const restoreResumeAddr)(PmProc* const);
	const u32 wfiStatusMask;
	const u32 wakeStatusMask;
	const u32 wfiEnableMask;
	const u32 wakeEnableMask;
	const u32 resumeCfg;
	u32 latencyReq;
	const u32 pwrDnLatency;
	const u32 pwrUpLatency;
} PmProc;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmProc pmApuProcs_g[PM_PROC_APU_MAX];
extern PmProc pmRpuProcs_g[PM_PROC_RPU_MAX];

/*********************************************************************
 * Function declarations
 ********************************************************************/
int PmProcFsm(PmProc* const proc, const PmProcEvent event);

/**
 * PmProcIsForcedOff() - Check whether given processor is in forced off state
 */
static inline bool PmProcIsForcedOff(const PmProc* const procPtr)
{
	return PM_PROC_STATE_FORCEDOFF == procPtr->node.currState;
}

/**
 * PmProcIsAsleep() - Check whether given processor is in sleep state
 */
static inline bool PmProcIsAsleep(const PmProc* const procPtr)
{
	return PM_PROC_STATE_SLEEP == procPtr->node.currState;
}

/**
 * PmProcIsSuspending() - Check whether given processor is in suspending state
 */
static inline bool PmProcIsSuspending(const PmProc* const procPtr)
{
	return PM_PROC_STATE_SUSPENDING == procPtr->node.currState;
}

#endif
