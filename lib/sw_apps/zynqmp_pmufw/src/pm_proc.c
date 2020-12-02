/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Definitions of processors and finite state machine
 * used for managing processor's power states.
 * Every processor must have:
 * 1. Entry in GPI1 register for wfi interrupt (wfi enable and status
 *    masks)
 * 2. Entry in GPI2 register for GIC wake interrupt (wake enable and
 *    status masks)
 * 3. Operations structure in node definition with:
 *      - wake function pointer, that is always releasing reset and
 *        might include some more actions regarding the processor's
 *        state setting.
 *      - sleep function might exist but doesn't have to. For example,
 *        APU processors have sleep that powers down processor island,
 *        but RPU processors have no private objects whose state can
 *        be changed at this point. In future, every processor should
 *        have sleep function in which its clock will be gated.
 *********************************************************************/

#include "pm_defs.h"
#include "pm_proc.h"
#include "pm_master.h"
#include "crl_apb.h"
#include "crf_apb.h"
#include "xpfw_rom_interface.h"
#include "apu.h"
#include "rpu.h"
#include "pm_system.h"
#include "pm_clock.h"
#include "xpfw_aib.h"

/* Enable/disable macros for processor's wfi event in GPI2 register */
#define ENABLE_WFI(mask)    XPfw_RMW32(PMU_LOCAL_GPI2_ENABLE, (mask), (mask));

/* Power consumptions for the APU for specific states */
#define DEFAULT_APU_POWER_ACTIVE	200U
#define DEFAULT_APU_POWER_SUSPENDING	100U
#define DEFAULT_APU_POWER_SLEEP		0U
#define DEFAULT_APU_POWER_OFF		0U

/* Power consumptions for the RPU for specific states */
#define DEFAULT_RPU_POWER_ACTIVE	200U
#define DEFAULT_RPU_POWER_SUSPENDING	100U
#define DEFAULT_RPU_POWER_SLEEP		0U
#define DEFAULT_RPU_POWER_OFF		0U

/**
 * PmProcHasResumeAddr() - Check whether the processor has the resume address
 * @proc       Processor to check
 */
bool PmProcHasResumeAddr(const PmProc* const proc)
{
	return (0ULL != (proc->resumeAddress & 1ULL));
}

/**
 * RPUSaveResumeAddr() - Saved address from which RPU core should resume
 * @proc        Processor to which the address should be restored upon wake-up
 * @address     Resume address (64-bit)
 *
 * @return      XStatus of performing save operation
 *              - XST_SUCCESS is address is successfully saved
 *              - XST_INVALID_PARAM if address is invalid
 */
static s32 RPUSaveResumeAddr(PmProc* const proc, const u64 address)
{
	s32 status = XST_SUCCESS;
	u32 addrLow = (u32) (address & 0xffffffffULL);

	/*
	 * For RPU processors lower 32-bits matter - only 2 values are
	 * possible to configure, report an error is addrLow is none of
	 * these.
	 */
	if ((PM_PROC_RPU_LOVEC_ADDR != addrLow) &&
	    (PM_PROC_RPU_HIVEC_ADDR != addrLow)) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	/* Set bit0 to mark address as valid */
	proc->resumeAddress = address | (u64)1ULL;
done:
	return status;
}

/**
 * APUSaveResumeAddr() - Saved address from which APU core should resume
 * @proc        Processor to which the address should be restored upon wake-up
 * @address     Resume address (64-bit)
 *
 * @return      XST_SUCCESS
 */
static s32 APUSaveResumeAddr(PmProc* const proc, const u64 address)
{
	/* Set bit0 to mark address as valid */
	proc->resumeAddress = address | (u64)1ULL;
	return XST_SUCCESS;
}

/**
 * RPURestoreResumeAddr() - Restore resume address for RPU core
 * @proc        Processor whose address should be restored
 *
 * Note: RPU processors get restored resume address by configuring VINITHI bit
 * in configuration register (RPUs can resume only from 2 addresses).
 */
static void RPURestoreResumeAddr(PmProc* const proc)
{
	/* mask out resumeAddress BIT0, which indicates address validity */
	u32 addrLow = (u32) (proc->resumeAddress & 0xfffffffeULL);

	if (0ULL == (proc->resumeAddress & 1ULL)) {
		goto done;
	}

	/* CFG_VINITHI_MASK mask is common for both processors */
	if (PM_PROC_RPU_LOVEC_ADDR == addrLow) {
		XPfw_RMW32(proc->resumeCfg, RPU_RPU_0_CFG_VINITHI_MASK,
			   ~RPU_RPU_0_CFG_VINITHI_MASK);
	} else {
		XPfw_RMW32(proc->resumeCfg, RPU_RPU_0_CFG_VINITHI_MASK,
			   RPU_RPU_0_CFG_VINITHI_MASK);
	}

	/* Mark resume address as invalid by setting it to 0 */
	proc->resumeAddress = 0ULL;

done:
	return;
}

/**
 * APURestoreResumeAddr() - Restore resume address for APU core
 * @proc        Processor whose address should be restored
 */
static void APURestoreResumeAddr(PmProc* const proc)
{
	/* mask out resumeAddress BIT0, which indicates address validity */
	u32 addrLow = (u32) (proc->resumeAddress & 0xfffffffeULL);
	u32 addrHigh = (u32) (proc->resumeAddress >> 32ULL);

	if (0ULL == (proc->resumeAddress & 1ULL)) {
		goto done;
	}

	XPfw_Write32(proc->resumeCfg, addrLow);
	XPfw_Write32(proc->resumeCfg + 4U, addrHigh);

	/* Mark resume address as invalid by setting it to 0 */
	proc->resumeAddress = 0ULL;
done:
	return;
}

/**
 * PmProcApu0Sleep() - Put APU_0 into sleep
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcApu0Sleep(void)
{
	return (s32)XpbrACPU0SleepHandler();
}

/**
 * PmProcApu1Sleep() - Put APU_1 into sleep
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcApu1Sleep(void)
{
	return (s32)XpbrACPU1SleepHandler();
}

/**
 * PmProcApu2Sleep() - Put APU_2 into sleep
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcApu2Sleep(void)
{
	return (s32)XpbrACPU2SleepHandler();
}

/**
 * PmProcApu3Sleep() - Put APU_3 into sleep
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcApu3Sleep(void)
{
	return (s32)XpbrACPU3SleepHandler();
}

/**
 * PmProcRpu0Sleep() - Put RPU_0 into sleep (reset only)
 * @return      Always success, reset cannot fail
 */
static s32 PmProcRpu0Sleep(void)
{
	XPfw_AibEnable(XPFW_AIB_RPU0_TO_LPD);
	XPfw_AibEnable(XPFW_AIB_LPD_TO_RPU0);

	XPfw_RMW32(CRL_APB_RST_LPD_TOP,
		   CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK,
		   CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
	XPfw_RMW32(RPU_RPU_0_CFG,
		   RPU_RPU_0_CFG_NCPUHALT_MASK,
		  ~RPU_RPU_0_CFG_NCPUHALT_MASK);
	XPfw_RMW32(CRL_APB_RST_LPD_TOP,
		   CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK,
		  ~CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);

	XPfw_AibDisable(XPFW_AIB_RPU0_TO_LPD);
	XPfw_AibDisable(XPFW_AIB_LPD_TO_RPU0);

	return XST_SUCCESS;
}

/**
 * PmProcRpu1Sleep() - Put RPU_1 into sleep (reset only)
 * @return      Always success, reset cannot fail
 */
static s32 PmProcRpu1Sleep(void)
{
	XPfw_AibEnable(XPFW_AIB_RPU1_TO_LPD);
	XPfw_AibEnable(XPFW_AIB_LPD_TO_RPU1);

	XPfw_RMW32(CRL_APB_RST_LPD_TOP,
		   CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK,
		   CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
	XPfw_RMW32(RPU_RPU_1_CFG,
		   RPU_RPU_1_CFG_NCPUHALT_MASK,
		  ~RPU_RPU_1_CFG_NCPUHALT_MASK);
	XPfw_RMW32(CRL_APB_RST_LPD_TOP,
		   CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK,
		  ~CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);

	XPfw_AibDisable(XPFW_AIB_RPU1_TO_LPD);
	XPfw_AibDisable(XPFW_AIB_LPD_TO_RPU1);

	return XST_SUCCESS;
}

/**
 * PmProcApu0Wake() - Wake up APU_0
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcApu0Wake(void)
{
	return (s32)XpbrACPU0WakeHandler();
}

/**
 * PmProcApu1Wake() - Wake up APU_1
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcApu1Wake(void)
{
	return (s32)XpbrACPU1WakeHandler();
}

/**
 * PmProcApu2Wake() - Wake up APU_2
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcApu2Wake(void)
{
	return (s32)XpbrACPU2WakeHandler();
}

/**
 * PmProcApu3Wake() - Wake up APU_3
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcApu3Wake(void)
{
	return (s32)XpbrACPU3WakeHandler();
}

/**
 * PmProcRpu0Wake() - Wake up RPU_0
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcRpu0Wake(void)
{
	s32 status;

	status = (s32)XpbrRstR50Handler();
	if (XST_SUCCESS != status) {
		goto done;
	}
	XPfw_RMW32(RPU_RPU_0_CFG,
		   RPU_RPU_0_CFG_NCPUHALT_MASK,
		   RPU_RPU_0_CFG_NCPUHALT_MASK);
done:
	return status;
}

/**
 * PmProcRpu1Wake() - Wake up RPU_1
 * @return      The status returned by PMU-ROM
 */
static s32 PmProcRpu1Wake(void)
{
	s32 status;

	status = (s32)XpbrRstR51Handler();
	if (XST_SUCCESS != status) {
		goto done;
	}
	XPfw_RMW32(RPU_RPU_1_CFG,
		   RPU_RPU_1_CFG_NCPUHALT_MASK,
		   RPU_RPU_1_CFG_NCPUHALT_MASK);
done:
	return status;
}

/**
 * PmProcRpu1Init() - Initialize RPU_1
 * @proc	RPU_1 processor
 */
static void PmProcRpu1Init(PmProc* const proc)
{
	u32 mode = XPfw_Read32(RPU_RPU_GLBL_CNTL);

	/* For RPU lockstep mode RPU_1 is assumed to be always down */
	if (0U == (mode & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK)) {
		proc->node.currState = PM_PROC_STATE_FORCEDOFF;
	}
}

/**
 * PmProcDisableEvents() - Disable wake and sleep events for the processor
 * @proc	Processor node
 */
static void PmProcDisableEvents(const PmProc* const proc)
{
	/* Disable wake event in GPI1 */
	DISABLE_WAKE(proc->mask);

	/* Disable wfi event in GPI2 */
	DISABLE_WFI(proc->mask);
}

/**
 * PmProcWake() - Wake up a processor node
 * @proc        Processor to be woken-up
 *
 * @return      Return status of processor specific wake handler
 */
static s32 PmProcWake(PmProc* const proc)
{
	s32 status;

	if (NULL != proc->node.parent) {
		status = PmPowerRequestParent(&proc->node);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}
	if (NULL != proc->node.clocks) {
		status = PmClockRequest(&proc->node);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

	proc->restoreResumeAddr(proc);
	status = proc->wake();

	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(&proc->node, PM_PROC_STATE_ACTIVE);
	}

done:
	return status;
}

/**
 * PmProcSleep() - Put processor node to sleep
 * @proc        Processor to sleep
 *
 * @return      Return status of processor specific sleep handler
 */
s32 PmProcSleep(PmProc* const proc)
{
	s32 status;

	status = proc->sleep();

	if (XST_SUCCESS != status) {
		goto done;
	}
	if (NULL != proc->node.parent) {
		PmPowerReleaseParent(&proc->node);
	}
	if (NULL != proc->node.clocks) {
		PmClockRelease(&proc->node);
	}

done:
	return status;
}

/**
 * PmProcTrActiveToSuspend() - FSM transition from active to suspend state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *          - XST_SUCCESS is always returned as this transition cannot fail
 *
 * @note    Executes when processor's request for self suspend gets processed.
 */
static s32 PmProcTrActiveToSuspend(PmProc* const proc)
{
	s32 status;

	PmInfo("%s active->susp\r\n", proc->node.name);

	ENABLE_WFI(proc->mask);
	PmNodeUpdateCurrState(&proc->node, PM_PROC_STATE_SUSPENDING);
	status = PmMasterFsm(proc->master, PM_MASTER_EVENT_SELF_SUSPEND);

	return status;
}

/**
 * PmProcTrToForcedOff() - FSM transition from active to force powerdown
 *                         state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *
 * @note    Executes when some other processor authorized to do so requests
 *          through PM API the PMU to powered down this processor. This
 *          processor is not informed about the following power down and
 *          therefore PMU does not wait for it to execute wfi. If processor has
 *          no implemented sleep function it will continue executing
 *          instructions.
 *          If the power down request bit is set when the processor is forced
 *          off, the bit must be cleared to ensure that
 *          1. Processor correctly concludes on the future boot that it is not
 *             resuming
 *          2. No wfi propagates to the PMU on the future boot (before processor
 *             clears the bit on its own)
 */
static s32 PmProcTrToForcedOff(PmProc* const proc)
{
	s32 status;
	bool killed;
	u32 pwrReq;

	PmInfo("%s active->forced off\r\n", proc->node.name);

	proc->node.latencyMarg = MAX_LATENCY;
	proc->resumeAddress = 0ULL;
	status = PmProcSleep(proc);
	PmNodeUpdateCurrState(&proc->node, PM_PROC_STATE_FORCEDOFF);
	PmProcDisableEvents(proc);

	pwrReq = XPfw_Read32(proc->pwrDnReqAddr);
	if (0U != (proc->pwrDnReqMask & pwrReq)) {
		pwrReq &= ~proc->pwrDnReqMask;
		XPfw_Write32(proc->pwrDnReqAddr, pwrReq);
	}

	if ((XST_SUCCESS != status) || (NULL == proc->master)) {
		goto done;
	}

	/* If master is also forced down we do not need to notify it */
	killed = PmMasterIsKilled(proc->master);
	if (false == killed) {
		status = PmMasterFsm(proc->master, PM_MASTER_EVENT_FORCED_PROC);
	}

done:
	return status;
}

/**
 * PmProcTrSuspendToActive() - FSM transition from suspend to active state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status (should be always success)
 *
 * @note    Executes when processor requests abort suspend through PM API.
 */
static s32 PmProcTrSuspendToActive(PmProc* const proc)
{
	s32 status;

	PmInfo("%s susp->active\r\n", proc->node.name);

	DISABLE_WFI(proc->mask);

	/* Notify master to cancel scheduled requests */
	status = PmMasterFsm(proc->master, PM_MASTER_EVENT_ABORT_SUSPEND);
	PmNodeUpdateCurrState(&proc->node, PM_PROC_STATE_ACTIVE);

	return status;
}

/**
 * PmProcTrSuspendToSleep() - FSM transition from suspend to sleep state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *
 * @note    Processor had previously called self suspend and now PMU has
 *          received processor's wfi interrupt.
 */
static s32 PmProcTrSuspendToSleep(PmProc* const proc)
{
	s32 status;
	u32 worstCaseLatency = proc->pwrDnLatency + proc->pwrUpLatency;

	PmInfo("%s susp->sleep\r\n", proc->node.name);
	proc->node.latencyMarg = proc->latencyReq - worstCaseLatency;

	status = PmProcSleep(proc);
	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(&proc->node, PM_PROC_STATE_SLEEP);

		/* Notify the master that the processor completed suspend */
		status = PmMasterFsm(proc->master, PM_MASTER_EVENT_SLEEP);

		/* If suspended, remember which processor to wake-up first */
		if (true == PmMasterIsSuspended(proc->master)) {
			proc->master->wakeProc = proc;
		}
	}
	DISABLE_WFI(proc->mask);
	ENABLE_WAKE(proc->mask);

	return status;
}

/**
 * PmProcTrSleepToActive() - FSM transition from sleep to active state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *
 * @note    Processor had previously called self suspend and before it had
 *          executed wfi PMU has received PM API request to force power down
 *          of this processor. Therefore, PMU does not wait for wfi interrupt
 *          from this processor to come, but puts it to sleep.
 */
static s32 PmProcTrSleepToActive(PmProc* const proc)
{
	s32 status;

	PmInfo("%s sleep->active\r\n", proc->node.name);
	status = PmProcWake(proc);
	DISABLE_WAKE(proc->mask);

	return status;
}

/**
 * PmProcTrForcePwrdnToActive() - FSM transition from forced powerdown to active
 *                                state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *
 * @note    Processor had previously called self suspend and before it had
 *          executed wfi PMU has received PM API request to force power down
 *          of this processor. Therefore, PMU does not wait for wfi interrupt
 *          from this processor to come, but puts it to sleep.
 */
static s32 PmProcTrForcePwrdnToActive(PmProc* const proc)
{
	s32 status;

	PmInfo("%s forced off->active\r\n", proc->node.name);
	status = PmProcWake(proc);

	return status;
}

/**
 * PmProcFsm() - Implements finite state machine (FSM) for a processor
 * @proc    Pointer to the processor the event is for
 * @event   Processor-specific event to act upon
 *
 * @return  Status of the processor state change operation
 *
 * @note    This FSM coordinates the state transitions for an individual
 *          processor.
 */
s32 PmProcFsm(PmProc* const proc, const PmProcEvent event)
{
	s32 status = XST_PM_INTERNAL;
	PmStateId currState = proc->node.currState;

	switch (event) {
	case PM_PROC_EVENT_SELF_SUSPEND:
		if (PM_PROC_STATE_ACTIVE == currState) {
			status = PmProcTrActiveToSuspend(proc);
		}
		break;
	case PM_PROC_EVENT_FORCE_PWRDN:
		if (PM_PROC_STATE_SUSPENDING == currState) {
			DISABLE_WFI(proc->mask);
		}
		status = PmProcTrToForcedOff(proc);

		/* Reset latency requirement */
		proc->latencyReq = MAX_LATENCY;
		break;
	case PM_PROC_EVENT_ABORT_SUSPEND:
		if (PM_PROC_STATE_SUSPENDING == currState) {
			status = PmProcTrSuspendToActive(proc);
		} else {
			status = XST_SUCCESS;
		}
		if (true == PmIsRequestedToSuspend(proc->master)) {
			status = PmMasterSuspendAck(proc->master,
						    XST_PM_ABORT_SUSPEND);
		}

		/* Reset latency requirement */
		proc->latencyReq = MAX_LATENCY;
		break;
	case PM_PROC_EVENT_SLEEP:
		if (PM_PROC_STATE_SUSPENDING == currState) {
			status = PmProcTrSuspendToSleep(proc);
		}
		break;
	case PM_PROC_EVENT_WAKE:
		if (PM_PROC_STATE_SLEEP == currState) {
			status = PmProcTrSleepToActive(proc);
		} else if (PM_PROC_STATE_FORCEDOFF == currState) {
			status = PmProcTrForcePwrdnToActive(proc);
		} else if (PM_PROC_STATE_ACTIVE == currState) {
			status = XST_SUCCESS;
		} else if (PM_PROC_STATE_SUSPENDING == currState) {
			status = XST_PM_CONFLICT;
		} else {
			/* For MISRA compliance */
		}

		/* Reset latency requirement */
		proc->latencyReq = MAX_LATENCY;
		break;
	default:
		PmErr("Unknown event %d\r\n", event);
		break;
	}
	if (status == XST_PM_INTERNAL) {
		PmErr("state #%d event #%d\r\n", currState, event);
	}

	return status;
}

/**
 * PmProcGetByWakeMask() - Get processor struct by wake interrupt status mask
 * @wake	GIC wake mask read from GPI1 register
 *
 * @return	Processor whose wake mask is provided as the argument
 */
PmProc* PmProcGetByWakeMask(const u32 wake)
{
	PmProc* found = NULL;
	u32 i;

	for (i = 0U; i < pmNodeClassProc_g.bucketSize; i++) {
		PmProc* proc = (PmProc*)pmNodeClassProc_g.bucket[i]->derived;

		if (0U != (proc->mask & wake)) {
			found = proc;
			break;
		}
	}

	return found;
}

/**
 * PmProcClearConfig() - Clear configuration of the processor node
 * @procNode	Processor node
 */
static void PmProcClearConfig(PmNode* const procNode)
{
	PmProc* const proc = (PmProc*)procNode->derived;

	proc->latencyReq = MAX_LATENCY;
	proc->resumeAddress = 0ULL;
	proc->master = NULL;

	PmProcDisableEvents(proc);
}

/**
 * PmProcConstruct() - Constructor for the processor node
 * @node	Processor node
 */
static void PmProcConstruct(PmNode* const node)
{
	PmProc* const proc = (PmProc*)node->derived;

	PmProcDisableEvents(proc);
}

/**
 * PmProcGetWakeUpLatency() - Get wake-up latency of the processor node
 * @node	Processor node whose wake-up latency should be get
 * @lat		Pointer to the location where the latency value should be stored
 *
 * @return	XST_SUCCESS if latency value is stored in *lat, XST_NO_FEATURE
 *		if the latency depends on power parent which has no method
 *		(getWakeUpLatency) to provide latency information
 */
static s32 PmProcGetWakeUpLatency(const PmNode* const node, u32* const lat)
{
	PmProc* const proc = (PmProc*)node->derived;
	PmNode* const powerNode = &node->parent->node;
	s32 status = XST_SUCCESS;
	u32 latency = 0U;

	*lat = 0U;
	if (PM_PROC_STATE_ACTIVE == node->currState) {
		goto done;
	}

	*lat = proc->pwrUpLatency;
	if (PM_PROC_STATE_SUSPENDING == proc->node.currState) {
		*lat += proc->pwrDnLatency;
		goto done;
	}

	if (NULL == powerNode->class->getWakeUpLatency) {
		status = XST_NO_FEATURE;
		goto done;
	}

	status = powerNode->class->getWakeUpLatency(powerNode, &latency);
	if (XST_SUCCESS == status) {
		*lat += latency;
	}

done:
	return status;

}

/**
 * PmProcForceDown() - Force down the processor node
 * @node	Processor node to force down
 *
 * @return	Status of performing the force down operation
 */
static s32 PmProcForceDown(PmNode* const node)
{
	PmProc* const proc = (PmProc*)node->derived;
	s32 status = XST_SUCCESS;

	if (PM_PROC_STATE_FORCEDOFF != node->currState) {
		status = PmProcFsm(proc, PM_PROC_EVENT_FORCE_PWRDN);
	}

	return status;
}

#ifdef ENABLE_UNUSED_RPU_PWR_DWN
void PmForceDownUnusableRpuCores(void)
{
	u32 value = Xil_In32(PMU_GLOBAL_GLOBAL_GEN_STORAGE4);
	u32 mode;

	/*
	 * If RPU core is not used then bit of PMU_GLOBAL_GLOBAL_GEN_STORAGE4
	 * for that core is cleared. So check that bit and force down that core.
	 */
	if (0U == (value & RPU0_STATUS_MASK)) {
		(void)PmProcForceDown(&pmProcRpu0_g.node);
	}
	if (0U == (value & RPU1_STATUS_MASK)) {
		mode = XPfw_Read32(RPU_RPU_GLBL_CNTL);

		/* For RPU lockstep mode RPU_1 is assumed to be always down */
		if (0U == (mode & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK)) {
			PmProc *proc = &pmProcRpu1_g;

			if (NULL != proc->node.parent) {
				PmPowerReleaseParent(&proc->node);
			}
			if (NULL != proc->node.clocks) {
				PmClockRelease(&proc->node);
			}
			if (NULL != proc->master) {
				(void)PmMasterFsm(proc->master,
					    PM_MASTER_EVENT_FORCED_PROC);
			}
		} else {
			(void)PmProcFsm(&pmProcRpu1_g, PM_PROC_EVENT_FORCE_PWRDN);
		}
	}

	/* Mark RPU0 and RPU1 status as power down in RPU usage status bits */
	XPfw_RMW32(PMU_GLOBAL_GLOBAL_GEN_STORAGE4,
		   RPU0_STATUS_MASK | RPU1_STATUS_MASK,
		   RPU0_STATUS_MASK | RPU1_STATUS_MASK);
}
#endif

/**
 * PmProcInit() - Startup initialization of the processor node
 * @node	Node to initialize
 *
 * @return	Status of initializing the node
 */
static s32 PmProcInit(PmNode* const node)
{
	PmProc* const proc = (PmProc*)node->derived;
	s32 status = XST_SUCCESS;

	PmProcDisableEvents(proc);
	if (NULL != proc->init) {
		proc->init(proc);
	}

	if (PM_PROC_STATE_ACTIVE != node->currState) {
		goto done;
	}
	if (NULL != node->parent) {
		status = PmPowerRequestParent(node);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}
	if (NULL != node->clocks) {
		status = PmClockRequest(node);
	}

done:
	return status;
}

/**
 * PmProcIsUsable() - Check if processor is usable by the currently set config
 * @node	Processor node
 *
 * @return	True if processor is usable, false otherwise
 */
static bool PmProcIsUsable(PmNode* const node)
{
	bool usable = false;
	PmProc* const proc = (PmProc*)node->derived;

	/* Processor is usable if it has an associated master */
	if (NULL != proc->master) {
		usable = true;
	}

	return usable;
}

/**
 * PmProcGetPerms() - Get permissions of masters to control processor clocks
 * @node	Target processor node
 *
 * @return	IPI masks of the processor's master
 */
static u32 PmProcGetPerms(const PmNode* const node)
{
	const PmProc* proc = (PmProc*)node->derived;
	u32 perms = 0U;

	if (NULL != proc->master) {
		perms = proc->master->ipiMask;
	}

	return perms;
}

/* Power consumptions for the APU for specific states */
static u8 PmProcPowerAPU_X[] = {
	DEFAULT_APU_POWER_OFF,
	DEFAULT_APU_POWER_ACTIVE,
	DEFAULT_APU_POWER_SLEEP,
	DEFAULT_APU_POWER_SUSPENDING,
};

/* Power consumptions for the RPU for specific states */
static u8 PmProcPowerRPU_X[] = {
	DEFAULT_RPU_POWER_OFF,
	DEFAULT_RPU_POWER_ACTIVE,
	DEFAULT_RPU_POWER_SLEEP,
	DEFAULT_RPU_POWER_SUSPENDING,
};

/* Apu processors */
PmProc pmProcApu0_g = {
	.node = {
		.derived = &pmProcApu0_g,
		.nodeId = NODE_APU_0,
		.class = &pmNodeClassProc_g,
		.parent = &pmPowerIslandApu_g,
		.clocks = NULL,
		.currState = PM_PROC_STATE_ACTIVE,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmProcPowerAPU_X),
		DEFINE_NODE_NAME("apu0"),
	},
	.resumeAddress = 0ULL,
	.master = NULL,
	.saveResumeAddr = APUSaveResumeAddr,
	.restoreResumeAddr = APURestoreResumeAddr,
	.init = NULL,
	.sleep = PmProcApu0Sleep,
	.wake = PmProcApu0Wake,
	.resumeCfg = APU_RVBARADDR0L,
	.pwrDnReqAddr = APU_PWRCTL,
	.pwrDnReqMask = 0x1U,
	.latencyReq = MAX_LATENCY,
	.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
	.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	.mask = PMU_IOMODULE_GPI2_ACPU_0_SLEEP_MASK,
};

PmProc pmProcApu1_g = {
	.node = {
		.derived = &pmProcApu1_g,
		.nodeId = NODE_APU_1,
		.class = &pmNodeClassProc_g,
		.parent = &pmPowerIslandApu_g,
		.clocks = NULL,
		.currState = PM_PROC_STATE_FORCEDOFF,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmProcPowerAPU_X),
		DEFINE_NODE_NAME("apu1"),
	},
	.resumeAddress = 0ULL,
	.master = NULL,
	.saveResumeAddr = APUSaveResumeAddr,
	.restoreResumeAddr = APURestoreResumeAddr,
	.init = NULL,
	.sleep = PmProcApu1Sleep,
	.wake = PmProcApu1Wake,
	.resumeCfg = APU_RVBARADDR1L,
	.pwrDnReqAddr = APU_PWRCTL,
	.pwrDnReqMask = 0x2U,
	.latencyReq = MAX_LATENCY,
	.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
	.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	.mask = PMU_IOMODULE_GPI2_ACPU_1_SLEEP_MASK,
};

PmProc pmProcApu2_g = {
	.node = {
		.derived = &pmProcApu2_g,
		.nodeId = NODE_APU_2,
		.class = &pmNodeClassProc_g,
		.parent = &pmPowerIslandApu_g,
		.clocks = NULL,
		.currState = PM_PROC_STATE_FORCEDOFF,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmProcPowerAPU_X),
		DEFINE_NODE_NAME("apu2"),
	},
	.resumeAddress = 0ULL,
	.master = NULL,
	.saveResumeAddr = APUSaveResumeAddr,
	.restoreResumeAddr = APURestoreResumeAddr,
	.init = NULL,
	.sleep = PmProcApu2Sleep,
	.wake = PmProcApu2Wake,
	.resumeCfg = APU_RVBARADDR2L,
	.pwrDnReqAddr = APU_PWRCTL,
	.pwrDnReqMask = 0x4U,
	.latencyReq = MAX_LATENCY,
	.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
	.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	.mask = PMU_IOMODULE_GPI2_ACPU_2_SLEEP_MASK,
};

PmProc pmProcApu3_g = {
	.node = {
		.derived = &pmProcApu3_g,
		.nodeId = NODE_APU_3,
		.class = &pmNodeClassProc_g,
		.parent = &pmPowerIslandApu_g,
		.clocks = NULL,
		.currState = PM_PROC_STATE_FORCEDOFF,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmProcPowerAPU_X),
		DEFINE_NODE_NAME("apu3"),
	},
	.resumeAddress = 0ULL,
	.master = NULL,
	.saveResumeAddr = APUSaveResumeAddr,
	.restoreResumeAddr = APURestoreResumeAddr,
	.init = NULL,
	.sleep = PmProcApu3Sleep,
	.wake = PmProcApu3Wake,
	.resumeCfg = APU_RVBARADDR3L,
	.pwrDnReqAddr = APU_PWRCTL,
	.pwrDnReqMask = 0x8U,
	.latencyReq = MAX_LATENCY,
	.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
	.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	.mask = PMU_IOMODULE_GPI2_ACPU_3_SLEEP_MASK,
};

/* Rpu processors */
PmProc pmProcRpu0_g = {
	.node = {
		.derived = &pmProcRpu0_g,
		.nodeId = NODE_RPU_0,
		.class = &pmNodeClassProc_g,
		.parent = &pmPowerIslandRpu_g.power,
		.clocks = NULL,
		.currState = PM_PROC_STATE_ACTIVE,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmProcPowerRPU_X),
		DEFINE_NODE_NAME("rpu0"),
	},
	.resumeAddress = 0ULL,
	.master = NULL,
	.saveResumeAddr = RPUSaveResumeAddr,
	.restoreResumeAddr = RPURestoreResumeAddr,
	.init = NULL,
	.sleep = PmProcRpu0Sleep,
	.wake = PmProcRpu0Wake,
	.resumeCfg = RPU_RPU_0_CFG,
	.pwrDnReqAddr = RPU_RPU_0_PWRDWN,
	.pwrDnReqMask = RPU_RPU_0_PWRDWN_EN_MASK,
	.latencyReq = MAX_LATENCY,
	.pwrDnLatency = 0U,
	.pwrUpLatency = 0U,
	.mask = PMU_IOMODULE_GPI2_R5_0_SLEEP_MASK,
};

PmProc pmProcRpu1_g = {
	.node = {
		.derived = &pmProcRpu1_g,
		.nodeId = NODE_RPU_1,
		.class = &pmNodeClassProc_g,
		.parent = &pmPowerIslandRpu_g.power,
		.clocks = NULL,
		.currState = PM_PROC_STATE_ACTIVE,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmProcPowerRPU_X),
		DEFINE_NODE_NAME("rpu1"),
	},
	.resumeAddress = 0ULL,
	.master = NULL,
	.saveResumeAddr = RPUSaveResumeAddr,
	.restoreResumeAddr = RPURestoreResumeAddr,
	.init = PmProcRpu1Init,
	.sleep = PmProcRpu1Sleep,
	.wake = PmProcRpu1Wake,
	.resumeCfg = RPU_RPU_1_CFG,
	.pwrDnReqAddr = RPU_RPU_1_PWRDWN,
	.pwrDnReqMask = RPU_RPU_1_PWRDWN_EN_MASK,
	.latencyReq = MAX_LATENCY,
	.pwrDnLatency = 0U,
	.pwrUpLatency = 0U,
	.mask = PMU_IOMODULE_GPI2_R5_1_SLEEP_MASK,
};

/* Collection of processor nodes */
static PmNode* pmNodeProcBucket[] = {
	&pmProcApu0_g.node,
	&pmProcApu1_g.node,
	&pmProcApu2_g.node,
	&pmProcApu3_g.node,
	&pmProcRpu0_g.node,
	&pmProcRpu1_g.node,
};

PmNodeClass pmNodeClassProc_g = {
	.clearConfig = PmProcClearConfig,
	.construct = PmProcConstruct,
	.getWakeUpLatency = PmProcGetWakeUpLatency,
	.getPowerData = PmNodeGetPowerInfo,
	.forceDown = PmProcForceDown,
	.init = PmProcInit,
	.isUsable = PmProcIsUsable,
	.getPerms = PmProcGetPerms,
	DEFINE_NODE_BUCKET(pmNodeProcBucket),
	.id = NODE_CLASS_PROC,
};

#endif
