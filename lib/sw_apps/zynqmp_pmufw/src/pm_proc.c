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

/* Enable/disable macros for processor's wfi event in GPI2 register */
#define ENABLE_WFI(mask)    XPfw_RMW32(PMU_LOCAL_GPI2_ENABLE, (mask), (mask));

#define PM_PROC_RPU_LOVEC_ADDR  0x00000000U
#define PM_PROC_RPU_HIVEC_ADDR  0xFFFF0000U

/* Power consumptions for the APU for specific states */
#define DEFAULT_APU_POWER_ACTIVE	200
#define DEFAULT_APU_POWER_SUSPENDING	100
#define DEFAULT_APU_POWER_SLEEP		50
#define DEFAULT_APU_POWER_OFF		0

/* Power consumptions for the RPU for specific states */
#define DEFAULT_RPU_POWER_ACTIVE	200
#define DEFAULT_RPU_POWER_SUSPENDING	100
#define DEFAULT_RPU_POWER_SLEEP		50
#define DEFAULT_RPU_POWER_OFF		0

/**
 * RPUSaveResumeAddr() - Saved address from which RPU core should resume
 * @proc        Processor to which the address should be restored upon wake-up
 * @address     Resume address (64-bit)
 *
 * @return      XStatus of performing save operation
 *              - XST_SUCCESS is address is successfully saved
 *              - XST_INVALID_PARAM if address is invalid
 */
static int RPUSaveResumeAddr(PmProc* const proc, const u64 address)
{
	int status = XST_SUCCESS;
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
	proc->resumeAddress = address | 1ULL;
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
static int APUSaveResumeAddr(PmProc* const proc, const u64 address)
{
	/* Set bit0 to mark address as valid */
	proc->resumeAddress = address | 1ULL;
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
 * PmProcSleep() - Put a processor to sleep
 * @nodePtr Pointer to node-structure of processor to be put to sleep
 *
 * @return  Operation status
 *          - XST_PM_INTERNAL if processor cannot be identified
 *          - XST_SUCCESS if processor has no sleep handler (example Rpu0)
 *          - Returned status by pmu-rom if processor sleep handler is called
 */
static int PmProcSleep(PmNode* const nodePtr)
{
	PmProc* proc;
	u32 worstCaseLatency = 0;
	int status = XST_PM_INTERNAL;

	if ((NULL == nodePtr) || (false == NODE_IS_PROC(nodePtr->typeId))) {
		goto done;
	}

	proc = (PmProc*)nodePtr->derived;
	worstCaseLatency = proc->pwrDnLatency + proc->pwrUpLatency;
	/* Sanity check has already been performed in PmSelfSuspend */
	nodePtr->latencyMarg = proc->latencyReq - worstCaseLatency;

	/*
	 * Call proper PMU-ROM handler as needed, provided that the latency
	 * requirement is satisfied. Otherwise put processor into reset only!
	 */
	switch (nodePtr->nodeId) {
	case NODE_APU_0:
		XPfw_RMW32(CRF_APB_RST_FPD_APU,
			   CRF_APB_RST_FPD_APU_ACPU0_PWRON_RESET_MASK,
			   CRF_APB_RST_FPD_APU_ACPU0_PWRON_RESET_MASK);
		status = XpbrACPU0SleepHandler();
		break;
	case NODE_APU_1:
		XPfw_RMW32(CRF_APB_RST_FPD_APU,
			   CRF_APB_RST_FPD_APU_ACPU1_PWRON_RESET_MASK,
			   CRF_APB_RST_FPD_APU_ACPU1_PWRON_RESET_MASK);
		status = XpbrACPU1SleepHandler();
		break;
	case NODE_APU_2:
		XPfw_RMW32(CRF_APB_RST_FPD_APU,
			   CRF_APB_RST_FPD_APU_ACPU2_PWRON_RESET_MASK,
			   CRF_APB_RST_FPD_APU_ACPU2_PWRON_RESET_MASK);
		status = XpbrACPU2SleepHandler();
		break;
	case NODE_APU_3:
		XPfw_RMW32(CRF_APB_RST_FPD_APU,
			   CRF_APB_RST_FPD_APU_ACPU3_PWRON_RESET_MASK,
			   CRF_APB_RST_FPD_APU_ACPU3_PWRON_RESET_MASK);
		status = XpbrACPU3SleepHandler();
		break;
	case NODE_RPU_0:
		XPfw_RMW32(CRL_APB_RST_LPD_TOP,
			   CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK,
			   CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
		XPfw_RMW32(RPU_RPU_0_CFG,
			   RPU_RPU_0_CFG_NCPUHALT_MASK,
			  ~RPU_RPU_0_CFG_NCPUHALT_MASK);
		XPfw_RMW32(CRL_APB_RST_LPD_TOP,
			   CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK,
			   ~CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
		status = XST_SUCCESS;
		break;
	case NODE_RPU_1:
		XPfw_RMW32(CRL_APB_RST_LPD_TOP,
			   CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK,
			   CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
		XPfw_RMW32(RPU_RPU_1_CFG,
			   RPU_RPU_1_CFG_NCPUHALT_MASK,
			  ~RPU_RPU_1_CFG_NCPUHALT_MASK);
		XPfw_RMW32(CRL_APB_RST_LPD_TOP,
			   CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK,
			   ~CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
		status = XST_SUCCESS;
		break;
	default:
        /* Proc Node ID is not supported, so nothing to be done here */
		break;
	}

	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(nodePtr, PM_PROC_STATE_SLEEP);
	}

done:
	return status;
}

/**
 * PmProcWake() - Wake up a processor node
 * @nodePtr Pointer to node-structure of processor to be woken-up
 *
 * @return  Operation status
 *          - XST_PM_INTERNAL if processor cannot be identified
 *          - Returned status of power up function if something goes wrong
 *          - Returned status by pmu-rom if processor wake handler is called
 */
static int PmProcWake(PmNode* const nodePtr)
{
	int status = XST_SUCCESS;
	PmProc *proc;

	if (NULL == nodePtr) {
		status = XST_PM_INTERNAL;
		goto done;
	}

	if (PM_PWR_STATE_OFF == nodePtr->parent->node.currState) {
		/* Power parent is down, trigger its powering up */
		status = PmTriggerPowerUp(nodePtr->parent);
	}

	if (XST_SUCCESS != status) {
		goto done;
	}

	proc = (PmProc*)nodePtr->derived;
	proc->restoreResumeAddr(proc);

	/* Call proper PMU-ROM handler as needed */
	switch (nodePtr->nodeId) {
	case NODE_APU_0:
		status = XpbrACPU0WakeHandler();
		if (status != XST_SUCCESS) {
			break;
		}

		XPfw_RMW32(CRF_APB_RST_FPD_APU,
			   CRF_APB_RST_FPD_APU_ACPU0_PWRON_RESET_MASK,
			   0);
		break;
	case NODE_APU_1:
		status = XpbrACPU1WakeHandler();
		if (status != XST_SUCCESS) {
			break;
		}

		XPfw_RMW32(CRF_APB_RST_FPD_APU,
			   CRF_APB_RST_FPD_APU_ACPU1_PWRON_RESET_MASK,
			   0);
		break;
	case NODE_APU_2:
		status = XpbrACPU2WakeHandler();
		if (status != XST_SUCCESS) {
			break;
		}

		XPfw_RMW32(CRF_APB_RST_FPD_APU,
			   CRF_APB_RST_FPD_APU_ACPU2_PWRON_RESET_MASK,
			   0);
		break;
	case NODE_APU_3:
		status = XpbrACPU3WakeHandler();
		if (status != XST_SUCCESS) {
			break;
		}

		XPfw_RMW32(CRF_APB_RST_FPD_APU,
			   CRF_APB_RST_FPD_APU_ACPU3_PWRON_RESET_MASK,
			   0);
		break;
	case NODE_RPU_0:
		status = XpbrRstR50Handler();
		XPfw_RMW32(RPU_RPU_0_CFG,
			   RPU_RPU_0_CFG_NCPUHALT_MASK,
			   RPU_RPU_0_CFG_NCPUHALT_MASK);
		break;
	case NODE_RPU_1:
		status = XpbrRstR51Handler();
		XPfw_RMW32(RPU_RPU_1_CFG,
			   RPU_RPU_1_CFG_NCPUHALT_MASK,
			   RPU_RPU_1_CFG_NCPUHALT_MASK);
		break;
	default:
		status = XST_PM_INTERNAL;
		break;
	}

	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(nodePtr, PM_PROC_STATE_ACTIVE);
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
static int PmProcTrActiveToSuspend(PmProc* const proc)
{
	int status;

	PmDbg("ACTIVE->SUSPENDING %s\r\n", PmStrNode(proc->node.nodeId));

	ENABLE_WFI(proc->wfiEnableMask);
	PmNodeUpdateCurrState(&proc->node, PM_PROC_STATE_SUSPENDING);
	status = PmMasterNotify(proc->master, PM_PROC_EVENT_SELF_SUSPEND);

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
 */
static int PmProcTrToForcedOff(PmProc* const proc)
{
	int status;

	PmDbg("ACTIVE->FORCED_PWRDN %s\r\n", PmStrNode(proc->node.nodeId));

	status = PmProcSleep(&proc->node);
	/* Override the state set in PmProcSleep to indicate FORCED OFF */
	PmNodeUpdateCurrState(&proc->node, PM_PROC_STATE_FORCEDOFF);

	if (XST_SUCCESS == status) {
		status = PmMasterNotify(proc->master, PM_PROC_EVENT_FORCE_PWRDN);
	}

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
static int PmProcTrSuspendToActive(PmProc* const proc)
{
	int status;

	PmDbg("SUSPENDING->ACTIVE %s\r\n", PmStrNode(proc->node.nodeId));

	DISABLE_WFI(proc->wfiEnableMask);

	/* Notify master to cancel scheduled requests */
	status = PmMasterNotify(proc->master, PM_PROC_EVENT_ABORT_SUSPEND);
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
static int PmProcTrSuspendToSleep(PmProc* const proc)
{
	int status;

	PmDbg("SUSPENDING->SLEEP %s\r\n", PmStrNode(proc->node.nodeId));

	status = PmProcSleep(&proc->node);
	if (XST_SUCCESS == status) {
		/* Notify the master that the processor completed suspend */
		status = PmMasterNotify(proc->master, PM_PROC_EVENT_SLEEP);

		/* If suspended, remember which processor to wake-up first */
		if (true == PmMasterIsSuspended(proc->master)) {
			proc->master->wakeProc = proc;
		}
	}
	DISABLE_WFI(proc->wfiEnableMask);
	ENABLE_WAKE(proc->wakeEnableMask);

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
static int PmProcTrSleepToActive(PmProc* const proc)
{
	int status;

	/* Notify master (it will setup everything (if) needed for the wake) */
	status = PmMasterNotify(proc->master, PM_PROC_EVENT_WAKE);

	if (XST_SUCCESS == status) {
		PmDbg("SLEEP->ACTIVE %s\r\n", PmStrNode(proc->node.nodeId));
		status = PmProcWake(&proc->node);
		DISABLE_WAKE(proc->wakeEnableMask);
	}

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
static int PmProcTrForcePwrdnToActive(PmProc* const proc)
{
	int status;
	PmDbg("FORCED_PWRDN->ACTIVE %s\r\n", PmStrNode(proc->node.nodeId));

	/* Notify master, it will know if resources need to be updated */
	status = PmMasterNotify(proc->master, PM_PROC_EVENT_WAKE);

	if (XST_SUCCESS == status) {
		status = PmProcWake(&proc->node);
	}

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
int PmProcFsm(PmProc* const proc, const PmProcEvent event)
{
	int status = XST_PM_INTERNAL;
	PmStateId currState = proc->node.currState;

	switch (event) {
	case PM_PROC_EVENT_SELF_SUSPEND:
		if (PM_PROC_STATE_ACTIVE == currState) {
			status = PmProcTrActiveToSuspend(proc);
		}
		break;
	case PM_PROC_EVENT_FORCE_PWRDN:
		if (PM_PROC_STATE_SUSPENDING == currState) {
			DISABLE_WFI(proc->wfiEnableMask);
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
		}

		/* Reset latency requirement */
		proc->latencyReq = MAX_LATENCY;
		break;
	default:
		PmDbg("ERROR: unrecognized event %d\r\n", event);
		break;
	}
#ifdef DEBUG_PM
	if (status == XST_PM_INTERNAL) {
		PmDbg("ERROR: state #%d event #%d\r\n", currState, event);
	}
#endif

	return status;
}

/* NodeOps for all processors */
static const PmNodeOps pmProcOps = {
	.sleep = PmProcSleep,
	.wake = PmProcWake,
};

/* Power consumptions for the APU for specific states */
static u32 PmProcPowerAPU_X[] = {
	DEFAULT_APU_POWER_OFF,
	DEFAULT_APU_POWER_ACTIVE,
	DEFAULT_APU_POWER_SLEEP,
	DEFAULT_APU_POWER_SUSPENDING,
};

/* Power consumptions for the RPU for specific states */
static u32 PmProcPowerRPU_X[] = {
	DEFAULT_RPU_POWER_OFF,
	DEFAULT_RPU_POWER_ACTIVE,
	DEFAULT_RPU_POWER_SLEEP,
	DEFAULT_RPU_POWER_SUSPENDING,
};

/* Apu processors */
PmProc pmApuProcs_g[PM_PROC_APU_MAX] = {
	[PM_PROC_APU_0] = {
		.node = {
			.derived = &pmApuProcs_g[PM_PROC_APU_0],
			.nodeId = NODE_APU_0,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandApu_g,
			.clocks = NULL,
			.currState = PM_PROC_STATE_ACTIVE,
			.latencyMarg = MAX_LATENCY,
			.ops = &pmProcOps,
			.powerInfo = PmProcPowerAPU_X,
			.powerInfoCnt = ARRAY_SIZE(PmProcPowerAPU_X),
		},
		.master = &pmMasterApu_g,
		.wfiStatusMask = PMU_IOMODULE_GPI2_ACPU_0_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_ACPU_0_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_ACPU0_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_ACPU0_WAKE_MASK,
		.resumeCfg = APU_RVBARADDR0L,
		.resumeAddress = 0ULL,
		.saveResumeAddr = APUSaveResumeAddr,
		.restoreResumeAddr = APURestoreResumeAddr,
		.latencyReq = MAX_LATENCY,
		.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
		.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	},
	[PM_PROC_APU_1] = {
		.node = {
			.derived = &pmApuProcs_g[PM_PROC_APU_1],
			.nodeId = NODE_APU_1,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandApu_g,
			.clocks = NULL,
			.currState = PM_PROC_STATE_SLEEP,
			.latencyMarg = MAX_LATENCY,
			.ops = &pmProcOps,
			.powerInfo = PmProcPowerAPU_X,
			.powerInfoCnt = ARRAY_SIZE(PmProcPowerAPU_X),
		},
		.master = &pmMasterApu_g,
		.wfiStatusMask = PMU_IOMODULE_GPI2_ACPU_1_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_ACPU_1_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_ACPU1_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_ACPU1_WAKE_MASK,
		.resumeCfg = APU_RVBARADDR1L,
		.resumeAddress = 0ULL,
		.saveResumeAddr = APUSaveResumeAddr,
		.restoreResumeAddr = APURestoreResumeAddr,
		.latencyReq = MAX_LATENCY,
		.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
		.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	},
	[PM_PROC_APU_2] = {
		.node = {
			.derived = &pmApuProcs_g[PM_PROC_APU_2],
			.nodeId = NODE_APU_2,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandApu_g,
			.clocks = NULL,
			.currState = PM_PROC_STATE_SLEEP,
			.latencyMarg = MAX_LATENCY,
			.ops = &pmProcOps,
			.powerInfo = PmProcPowerAPU_X,
			.powerInfoCnt = ARRAY_SIZE(PmProcPowerAPU_X),
		},
		.master = &pmMasterApu_g,
		.wfiStatusMask = PMU_IOMODULE_GPI2_ACPU_2_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_ACPU_2_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_ACPU2_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_ACPU2_WAKE_MASK,
		.resumeCfg = APU_RVBARADDR2L,
		.resumeAddress = 0ULL,
		.saveResumeAddr = APUSaveResumeAddr,
		.restoreResumeAddr = APURestoreResumeAddr,
		.latencyReq = MAX_LATENCY,
		.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
		.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	},
	[PM_PROC_APU_3] = {
		.node = {
			.derived = &pmApuProcs_g[PM_PROC_APU_3],
			.nodeId = NODE_APU_3,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandApu_g,
			.clocks = NULL,
			.currState = PM_PROC_STATE_SLEEP,
			.latencyMarg = MAX_LATENCY,
			.ops = &pmProcOps,
			.powerInfo = PmProcPowerAPU_X,
			.powerInfoCnt = ARRAY_SIZE(PmProcPowerAPU_X),
		},
		.master = &pmMasterApu_g,
		.wfiStatusMask = PMU_IOMODULE_GPI2_ACPU_3_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_ACPU_3_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_ACPU3_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_ACPU3_WAKE_MASK,
		.resumeCfg = APU_RVBARADDR3L,
		.resumeAddress = 0ULL,
		.saveResumeAddr = APUSaveResumeAddr,
		.restoreResumeAddr = APURestoreResumeAddr,
		.latencyReq = MAX_LATENCY,
		.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
		.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	}
};

/* Rpu processors */
PmProc pmRpuProcs_g[PM_PROC_RPU_MAX] = {
	[PM_PROC_RPU_0] = {
		.node = {
			.derived = &pmRpuProcs_g[PM_PROC_RPU_0],
			.nodeId = NODE_RPU_0,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandRpu_g,
			.clocks = NULL,
			.currState = PM_PROC_STATE_ACTIVE,
			.latencyMarg = MAX_LATENCY,
			.ops = &pmProcOps,
			.powerInfo = PmProcPowerRPU_X,
			.powerInfoCnt = ARRAY_SIZE(PmProcPowerRPU_X),
		},
		.master = &pmMasterRpu0_g,
		.wfiStatusMask = PMU_IOMODULE_GPI2_R5_0_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_R5_0_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_R5_0_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_R5_0_WAKE_MASK,
		.resumeCfg = RPU_RPU_0_CFG,
		.resumeAddress = 0ULL,
		.saveResumeAddr = RPUSaveResumeAddr,
		.restoreResumeAddr = RPURestoreResumeAddr,
		.latencyReq = MAX_LATENCY,
		.pwrDnLatency = 0,
		.pwrUpLatency = 0,
	},
	[PM_PROC_RPU_1] = {
		.node = {
			.derived = &pmRpuProcs_g[PM_PROC_RPU_1],
			.nodeId = NODE_RPU_1,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandRpu_g,
			.clocks = NULL,
			.currState = PM_PROC_STATE_SLEEP,
			.latencyMarg = MAX_LATENCY,
			.ops = &pmProcOps,
			.powerInfo = PmProcPowerRPU_X,
			.powerInfoCnt = ARRAY_SIZE(PmProcPowerRPU_X),
		},
		.master = &pmMasterRpu1_g,
		.wfiStatusMask = PMU_IOMODULE_GPI2_R5_1_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_R5_1_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_R5_1_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_R5_1_WAKE_MASK,
		.resumeCfg = RPU_RPU_1_CFG,
		.resumeAddress = 0ULL,
		.saveResumeAddr = RPUSaveResumeAddr,
		.restoreResumeAddr = RPURestoreResumeAddr,
		.latencyReq = MAX_LATENCY,
		.pwrDnLatency = 0,
		.pwrUpLatency = 0,
	},
};
