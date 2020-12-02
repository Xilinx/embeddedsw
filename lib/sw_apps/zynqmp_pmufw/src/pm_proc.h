/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Contains all functions, datas and definitions needed for
 * managing processor's states.
 *********************************************************************/

#ifndef PM_PROC_H_
#define PM_PROC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_common.h"
#include "pm_node.h"

typedef u8 PmProcEvent;

/*********************************************************************
 * Macros
 ********************************************************************/

#define DISABLE_WFI(mask)   XPfw_RMW32(PMU_LOCAL_GPI2_ENABLE, (mask), ~(mask));

/*
 * Processor is powered down as requested by a master which is privileged
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

#define RPU0_STATUS_MASK		BIT(1U)
#define RPU1_STATUS_MASK		BIT(2U)

#define PM_PROC_RPU_LOVEC_ADDR  0x00000000U
#define PM_PROC_RPU_HIVEC_ADDR  0xFFFF0000U

/*********************************************************************
 * Structure definitions
 ********************************************************************/

/**
 * PmProc - Processor node's structure
 * @node            Processor's node structure
 * @resumeAddress   Address from which processor should resume
 *                  resumeAddress BIT0=1 indicates valid address
 * @master          Master channel used by this processor
 * @saveResumeAddr  Pointer to function for saving the resume address
 * @restoreResumeAddr Pointer to function for restoring resume address
 * @init            Init handler specific to the processor
 * @sleep           Pointer to the processor's sleep handler
 * @wake            Pointer to the processor's wake handler
 * @resumeCfg       Address of register configuring processor's resume address
 * @pwrDnReqAddr    Address of the power down request register
 * @pwrDnReqMask    Mask in the power down request register
 * @latencyReq      Latenct requirement as passed in by self_suspend argument
 * @pwrDnLatency    Latency (in us) for transition to OFF state
 * @pwrUpLatency    Latency (in us) for transition to ON state
 * @mask            Unique mask of the processor in PM_IOMODULE_GPI2,
 *                  PM_IOMODULE_GPI1, PM_LOCAL_GPI2_ENABLE, and
 *                  PM_LOCAL_GPI1_ENABLE registers
 */
struct PmProc {
	PmNode node;
	u64 resumeAddress;
	PmMaster* master;
	s32 (*const saveResumeAddr)(PmProc* const proc, u64 address);
	void (*const restoreResumeAddr)(PmProc* const proc);
	void (*const init)(PmProc* const proc);
	s32 (*const sleep)(void);
	s32 (*const wake)(void);
	const u32 mask;
	const u32 resumeCfg;
	const u32 pwrDnReqAddr;
	const u32 pwrDnReqMask;
	u32 latencyReq;
	const u32 pwrDnLatency;
	const u32 pwrUpLatency;
};

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmProc pmProcApu0_g;
extern PmProc pmProcApu1_g;
extern PmProc pmProcApu2_g;
extern PmProc pmProcApu3_g;
extern PmProc pmProcRpu0_g;
extern PmProc pmProcRpu1_g;

extern PmNodeClass pmNodeClassProc_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
s32 PmProcFsm(PmProc* const proc, const PmProcEvent event);

bool PmProcHasResumeAddr(const PmProc* const proc);

PmProc* PmProcGetByWakeMask(const u32 wake);

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

s32 PmProcSleep(PmProc* const proc);

#ifdef ENABLE_UNUSED_RPU_PWR_DWN
void PmForceDownUnusableRpuCores(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PM_PROC_H_ */
