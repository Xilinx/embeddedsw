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
#include "xpfw_rom_interface.h"

/**
 * PmProcSleep() - Put a processor to sleep
 * @nodePtr Pointer to node-structure of processor to be put to sleep
 *
 * @return  Operation status
 *          - PM_RET_SUCCESS if transition succeeded
 *          - PM_RET_ERROR_FAILURE if pmu rom failed to set the state
 */
u32 PmProcSleep(PmNode* const nodePtr)
{
	u32 ret;

	if (NULL == nodePtr) {
		ret = PM_RET_ERROR_FAILURE;
		goto done;
	}

	/* Call proper PMU-ROM handler as needed */
	switch (nodePtr->nodeId) {
	case NODE_APU_0:
		ret = XpbrACPU0SleepHandler();
		break;
	case NODE_APU_1:
		ret = XpbrACPU1SleepHandler();
		break;
	case NODE_APU_2:
		ret = XpbrACPU2SleepHandler();
		break;
	case NODE_APU_3:
		ret = XpbrACPU3SleepHandler();
		break;
	default:
		ret = XST_SUCCESS;
		break;
	}

	if (XST_SUCCESS != ret) {
		ret = PM_RET_ERROR_FAILURE;
		goto done;
	}

	nodePtr->currState = PM_PROC_STATE_SLEEP;
	ret = PM_RET_SUCCESS;

done:
	return ret;
}

/**
 * PmProcWake() - Wake up a processor node
 * @nodePtr Pointer to node-structure of processor to be woken-up
 *
 * @return  Operation status
 *          - PM_RET_SUCCESS if transition succeeded
 *          - PM_RET_ERROR_FAILURE if pmu rom failed to set the state
 */
u32 PmProcWake(PmNode* const nodePtr)
{
	u32 ret = PM_RET_SUCCESS;

	if (NULL == nodePtr) {
		ret = PM_RET_ERROR_INTERNAL;
		goto done;
	}

	if (PM_PWR_STATE_OFF == nodePtr->parent->node.currState) {
		/* Power parent is down, trigger its powering up */
		ret = PmTriggerPowerUp(nodePtr->parent);
	}

	if (PM_RET_SUCCESS != ret) {
		goto done;
	}

	/* Call proper PMU-ROM handler as needed */
	switch (nodePtr->nodeId) {
	case NODE_APU_0:
		ret = XpbrACPU0WakeHandler();
		break;
	case NODE_APU_1:
		ret = XpbrACPU1WakeHandler();
		break;
	case NODE_APU_2:
		ret = XpbrACPU2WakeHandler();
		break;
	case NODE_APU_3:
		ret = XpbrACPU3WakeHandler();
		break;
	case NODE_RPU_0:
		ret = XpbrRstR50Handler();
		break;
	case NODE_RPU_1:
		ret = XpbrRstR51Handler();
		break;
	default:
		ret = XST_SUCCESS;
		break;
	}

	if (XST_SUCCESS != ret) {
		ret = PM_RET_ERROR_FAILURE;
		goto done;
	}

	nodePtr->currState = PM_PROC_STATE_ACTIVE;
	ret = PM_RET_SUCCESS;

done:
	return ret;
}

/**
 * PmProcTrActiveToSuspend() - FSM transition from active to suspend state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *          - PM_RET_SUCCESS is always returned as this transition cannot fail
 *
 * @note    Executes when processor's request for self suspend gets processed.
 */
static u32 PmProcTrActiveToSuspend(PmProc* const proc)
{
	PmDbg("ACTIVE->SUSPENDING %s\n", PmStrNode(proc->node.nodeId));

	ENABLE_WFI(proc->wfiEnableMask);
	proc->node.currState = PM_PROC_STATE_SUSPENDING;

	return PM_RET_SUCCESS;
}

/**
 * PmProcTrToForcedOff() - FSM transition from active to force powerdown
 *                         state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *          - PM_RET_SUCCESS if transition succeeded
 *          - PM_RET_ERROR_FAILURE if pmu rom failed to set the state
 *          - PM_RET_ERROR_INTERNAL if processor structures are incorrectly
 *                                  initialized
 *
 * @note    Executes when some other processor authorized to do so requests
 *          through PM API the PMU to powered down this processor. This
 *          processor is not informed about the following power down and
 *          therefore PMU does not wait for it to execute wfi. If processor has
 *          no implemented sleep function it will continue executing
 *          instructions.
 */
static u32 PmProcTrToForcedOff(PmProc* const proc)
{
	u32 status;

	PmDbg("ACTIVE->FORCED_PWRDN %s\n", PmStrNode(proc->node.nodeId));

	status = PmProcSleep(&proc->node);
	/* Override the state set in PmProcSleep to indicate FORCED OFF */
	proc->node.currState = PM_PROC_STATE_FORCEDOFF;
	if (true == proc->isPrimary) {
		/* Notify master to release all requirements of this processor */
		PmMasterNotify(proc->master, PM_PROC_EVENT_FORCE_PWRDN);
	}

	return status;
}

/**
 * PmProcTrSuspendToActive() - FSM transition from suspend to active state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *          - PM_RET_SUCCESS is always returned as this transition cannot fail
 *
 * @note    Executes when processor requests abort suspend through PM API.
 */
static u32 PmProcTrSuspendToActive(PmProc* const proc)
{
	PmDbg("SUSPENDING->ACTIVE %s\n", PmStrNode(proc->node.nodeId));

	DISABLE_WFI(proc->wfiEnableMask);
	DISABLE_WAKE(proc->wakeEnableMask);

	/* Notify master to cancel scheduled requests */
	PmMasterNotify(proc->master, PM_PROC_EVENT_ABORT_SUSPEND);
	proc->node.currState = PM_PROC_STATE_ACTIVE;

	return PM_RET_SUCCESS;
}

/**
 * PmProcTrSuspendToSleep() - FSM transition from suspend to sleep state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *          - PM_RET_SUCCESS if transition succeeded
 *          - PM_RET_ERROR_FAILURE if pmu rom failed to set the state
 *          - PM_RET_ERROR_INTERNAL if processor structures are incorrectly
 *                                  initialized
 *
 * @note    Processor had previously called self suspend and now PMU has
 *          received processor's wfi interrupt.
 */
static u32 PmProcTrSuspendToSleep(PmProc* const proc)
{
	u32 status;

	PmDbg("SUSPENDING->SLEEP %s\n", PmStrNode(proc->node.nodeId));

	status = PmProcSleep(&proc->node);
	if (true == proc->isPrimary) {
		/*
		 * Notify master to update slave capabilities according to the
		 * scheduled requests for after primary processor goes to sleep.
		 */
		PmMasterNotify(proc->master, PM_PROC_EVENT_SLEEP);
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
 *          - PM_RET_SUCCESS if transition succeeded
 *          - PM_RET_ERROR_FAILURE if pmu rom failed to set the state
 *          - PM_RET_ERROR_INTERNAL if processor structures are incorrectly
 *                                  initialized
 *
 * @note    Processor had previously called self suspend and before it had
 *          executed wfi PMU has received PM API request to force power down
 *          of this processor. Therefore, PMU does not wait for wfi interrupt
 *          from this processor to come, but puts it to sleep.
 */
static u32 PmProcTrSleepToActive(PmProc* const proc)
{
	u32 status;

	if (true == proc->isPrimary) {
		/*
		 * Notify master to update slave capabilities according to the
		 * scheduled requests for before the primary processor
		 * gets woken-up.
		 */
		PmMasterNotify(proc->master, PM_PROC_EVENT_WAKE);
	}

	PmDbg("SLEEP->ACTIVE %s\n", PmStrNode(proc->node.nodeId));

	status = PmProcWake(&proc->node);

	/* Keep wfi interrupt disabled while processor is active */
	DISABLE_WFI(proc->wfiEnableMask);
	DISABLE_WAKE(proc->wakeEnableMask);

	return status;
}

/**
 * PmProcTrForcePwrdnToActive() - FSM transition from forced powerdown to active
 *                                state
 * @proc    Pointer to processor whose FSM is changing state
 *
 * @return  Operation status
 *          - PM_RET_SUCCESS if transition succeeded
 *          - PM_RET_ERROR_FAILURE if pmu rom failed to set the state
 *          - PM_RET_ERROR_INTERNAL if processor does not have wake function
 *
 * @note    Processor had previously called self suspend and before it had
 *          executed wfi PMU has received PM API request to force power down
 *          of this processor. Therefore, PMU does not wait for wfi interrupt
 *          from this processor to come, but puts it to sleep.
 */
static u32 PmProcTrForcePwrdnToActive(PmProc* const proc)
{
	u32 status;

	PmDbg("FORCED_PWRDN->ACTIVE %s\n", PmStrNode(proc->node.nodeId));

	status = PmProcWake(&proc->node);

	return status;
}

/**
 * PmProcFsm() - Implements finite state machine (FSM) for a processor
 * @proc    Pointer to the processor the event is for
 * @event   Processor-specific event to act upon
 *
 * @note    This FSM coordinates the state transitions for an individual
 *          processor.
 */
u32 PmProcFsm(PmProc* const proc, const PmProcEvent event)
{
	u32 status = PM_RET_SUCCESS;
	u32 currState = proc->node.currState;

	switch (event) {
	case PM_PROC_EVENT_SELF_SUSPEND:
		if (PM_PROC_STATE_ACTIVE == currState) {
			status = PmProcTrActiveToSuspend(proc);
		} else {
			PmDbg("%s illegal state %d for SUSPND event\n",
			      __func__, currState);
			status = PM_RET_ERROR_INTERNAL;
		}
		break;
	case PM_PROC_EVENT_FORCE_PWRDN:
		if (PM_PROC_STATE_SUSPENDING == currState) {
			DISABLE_WFI(proc->wfiEnableMask);
		}
		status = PmProcTrToForcedOff(proc);
		break;
	case PM_PROC_EVENT_ABORT_SUSPEND:
		if (PM_PROC_STATE_SUSPENDING == currState) {
			status = PmProcTrSuspendToActive(proc);
		} else {
			PmDbg("%s illegal state %d for ABORT event\n",
			      __func__, currState);
			status = PM_RET_ERROR_INTERNAL;
		}
		break;
	case PM_PROC_EVENT_SLEEP:
		if (PM_PROC_STATE_SUSPENDING == currState) {
			status = PmProcTrSuspendToSleep(proc);
		} else {
			PmDbg("%s illegal state %d for SLEEP event\n",
			      __func__, currState);
			status = PM_RET_ERROR_INTERNAL;
		}
		break;
	case PM_PROC_EVENT_WAKE:
		if (PM_PROC_STATE_SLEEP == currState) {
			status = PmProcTrSleepToActive(proc);
		} else if (PM_PROC_STATE_FORCEDOFF == currState) {
			status = PmProcTrForcePwrdnToActive(proc);
		} else {
			PmDbg("%s illegal state %d for WAKE event\n",
			      __func__, currState);
			status = PM_RET_ERROR_INTERNAL;
		}
		break;
	default:
		PmDbg("%s unrecognized event\n", __func__, event);
		status = PM_RET_ERROR_INTERNAL;
		break;
	}

	return status;
}

/* NodeOps for all processors */
static const PmNodeOps pmProcOps = {
	.sleep = PmProcSleep,
	.wake = PmProcWake,
};

/* Apu processors */
PmProc pmApuProcs_g[PM_PROC_APU_MAX] = {
	[PM_PROC_APU_0] = {
		.node = {
			.derived = &pmApuProcs_g[PM_PROC_APU_0],
			.nodeId = NODE_APU_0,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandApu_g,
			.currState = PM_PROC_STATE_ACTIVE,
			.ops = &pmProcOps,
		},
		.master = &pmMasterApu_g,
		.isPrimary = true,
		.wfiStatusMask = PMU_IOMODULE_GPI2_ACPU_0_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_ACPU_0_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_ACPU0_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_ACPU0_WAKE_MASK,
	},
	[PM_PROC_APU_1] = {
		.node = {
			.derived = &pmApuProcs_g[PM_PROC_APU_1],
			.nodeId = NODE_APU_1,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandApu_g,
			.currState = PM_PROC_STATE_SLEEP,
			.ops = &pmProcOps,
		},
		.master = &pmMasterApu_g,
		.isPrimary = false,
		.wfiStatusMask = PMU_IOMODULE_GPI2_ACPU_1_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_ACPU_1_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_ACPU1_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_ACPU1_WAKE_MASK,
	},
	[PM_PROC_APU_2] = {
		.node = {
			.derived = &pmApuProcs_g[PM_PROC_APU_2],
			.nodeId = NODE_APU_2,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandApu_g,
			.currState = PM_PROC_STATE_SLEEP,
			.ops = &pmProcOps,
		},
		.master = &pmMasterApu_g,
		.isPrimary = false,
		.wfiStatusMask = PMU_IOMODULE_GPI2_ACPU_2_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_ACPU_2_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_ACPU2_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_ACPU2_WAKE_MASK,
	},
	[PM_PROC_APU_3] = {
		.node = {
			.derived = &pmApuProcs_g[PM_PROC_APU_3],
			.nodeId = NODE_APU_3,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandApu_g,
			.currState = PM_PROC_STATE_SLEEP,
			.ops = &pmProcOps,
		},
		.master = &pmMasterApu_g,
		.isPrimary = false,
		.wfiStatusMask = PMU_IOMODULE_GPI2_ACPU_3_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_ACPU_3_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_ACPU3_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_ACPU3_WAKE_MASK,
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
			.currState = PM_PROC_STATE_ACTIVE,
			.ops = &pmProcOps,
		},
		.master = &pmMasterRpu0_g,
		.isPrimary = true,
		.wfiStatusMask = PMU_IOMODULE_GPI2_R5_0_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_R5_0_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_R5_0_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_R5_0_WAKE_MASK,
	},
	[PM_PROC_RPU_1] = {
		.node = {
			.derived = &pmRpuProcs_g[PM_PROC_RPU_1],
			.nodeId = NODE_RPU_1,
			.typeId = PM_TYPE_PROC,
			.parent = &pmPowerIslandRpu_g,
			.currState = PM_PROC_STATE_SLEEP,
			.ops = &pmProcOps,
		},
		.master = &pmMasterRpu1_g,
		.isPrimary = false,
		.wfiStatusMask = PMU_IOMODULE_GPI2_R5_1_SLEEP_MASK,
		.wakeStatusMask = PMU_IOMODULE_GPI1_R5_1_WAKE_MASK,
		.wfiEnableMask = PMU_LOCAL_GPI2_ENABLE_R5_1_PWRDWN_REQ_MASK,
		.wakeEnableMask = PMU_LOCAL_GPI1_ENABLE_R5_1_WAKE_MASK,
	},
};
