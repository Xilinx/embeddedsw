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
 * This file contains PM master related data structures and
 * functions for accessing them.
 *********************************************************************/

#include "pm_master.h"
#include "pm_proc.h"
#include "pm_defs.h"
#include "pm_callbacks.h"
#include "pm_notifier.h"
#include "pm_system.h"
#include "pm_sram.h"
#include "pm_ddr.h"
#include "pm_requirement.h"

#include "xpfw_ipi_manager.h"

#define PM_REQUESTED_SUSPEND        0x1U
#define TO_ACK_CB(ack, status) (REQUEST_ACK_NON_BLOCKING == (ack))

static const PmSlave* pmApuMemories[] = {
	&pmSlaveOcm0_g.slv,
	&pmSlaveOcm1_g.slv,
	&pmSlaveOcm2_g.slv,
	&pmSlaveOcm3_g.slv,
	&pmSlaveDdr_g,
	NULL,
};

/**
 * PmApuPrepareSuspendToRam() - Prepare the APU data structs for suspend to RAM
 */
static int PmApuPrepareSuspendToRam(void)
{
	int status;
	u32 i;
	PmRequirement* req = PmGetRequirementForSlave(&pmMasterApu_g, NODE_L2);

	if (NULL == req) {
		status = XST_FAILURE;
		goto done;
	}

	status = PmRequirementSchedule(req, 0U);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if (NULL == pmMasterApu_g.memories) {
		goto done;
	}

	i = 0U;
	while (NULL != pmMasterApu_g.memories[i]) {
		PmNodeId memNid = pmMasterApu_g.memories[i]->node.nodeId;

		req = PmGetRequirementForSlave(&pmMasterApu_g, memNid);
		if (NULL == req) {
			status = XST_FAILURE;
			goto done;
		}
		status = PmRequirementSchedule(req, PM_CAP_CONTEXT);
		i++;
	}

done:
	return status;
}

/**
 * PmApuEvaluateState() - Evaluate state specified by the APU master for itself
 * @state	State argument to be evaluated (specified as the argument of the
 *		self suspend call)
 *
 * @return	XST_SUCCESS if state is supported
 *		XST_NO_FEATURE if state is not supported
 *		Error code if requirements are not properly set for a slave
 */
static int PmApuEvaluateState(const u32 state)
{
	int status;

	switch (state) {
	case PM_APU_STATE_CPU_IDLE:
		status = XST_SUCCESS;
		break;
	case PM_APU_STATE_SUSPEND_TO_RAM:
		status = PmApuPrepareSuspendToRam();
		break;
	default:
		status = XST_NO_FEATURE;
		break;
	}

	return status;
}

PmMaster pmMasterApu_g = {
	.procs = pmApuProcs_g,
	.procsCnt = PM_PROC_APU_MAX,
	.wakeProc = NULL,
	.nid = NODE_APU,
	.ipiMask = IPI_PMU_0_IER_APU_MASK,
	.reqs = NULL,
	.permissions = IPI_PMU_0_IER_RPU_0_MASK | IPI_PMU_0_IER_RPU_1_MASK,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
	.state = PM_MASTER_STATE_ACTIVE,
	.gic = &pmGicProxy,
	.memories = pmApuMemories,
	.evalState = PmApuEvaluateState,
};

PmMaster pmMasterRpu0_g = {
	.procs = &pmRpuProcs_g[PM_PROC_RPU_0],
	.procsCnt = 1U,
	.wakeProc = NULL,
	.nid = NODE_RPU,
	.ipiMask = IPI_PMU_0_IER_RPU_0_MASK,
	.reqs = NULL,
	.permissions = IPI_PMU_0_IER_APU_MASK | IPI_PMU_0_IER_RPU_1_MASK,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
	.state = PM_MASTER_STATE_ACTIVE,
	.gic = NULL,
	.memories = NULL,
	.evalState = NULL,
};

PmMaster pmMasterRpu1_g = {
	.procs = &pmRpuProcs_g[PM_PROC_RPU_1],
	.procsCnt = 1U,
	.wakeProc = NULL,
	.nid = NODE_RPU_0, /* placeholder for request suspend, not used */
	.ipiMask = IPI_PMU_0_IER_RPU_1_MASK,
	.reqs = NULL,   /* lockstep mode is assumed for now */
	.permissions = IPI_PMU_0_IER_APU_MASK | IPI_PMU_0_IER_RPU_0_MASK,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
	.state = PM_MASTER_STATE_KILLED,
	.gic = NULL,
	.memories = NULL,
	.evalState = NULL,
};

PmMaster *const pmAllMasters[PM_MASTER_MAX] = {
	&pmMasterApu_g,
	&pmMasterRpu0_g,
	&pmMasterRpu1_g,
};

/**
 * PmGetMasterByIpiMask() - Use to get pointer to master structure by ipi mask
 * @mask    IPI Mask of a master (requestor) in IPI registers
 *
 * @return  Pointer to a PmMaster structure or NULL if master is not found
 */
const PmMaster* PmGetMasterByIpiMask(const u32 mask)
{
	u32 i;
	const PmMaster *mst = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		if (0U != (mask & pmAllMasters[i]->ipiMask)) {
			mst = pmAllMasters[i];
			break;
		}
	}

	return mst;
}

/**
 * PmGetProcOfThisMaster() - Get processor pointer with given node id, if
 *          such processor exist within the master
 * @master  Master within which the search is performed
 * @nodeId  Node of the processor to be found
 *
 * @return  Pointer to processor with the given node id (which is within the
 *          master), or NULL if such processor is not found.
 */
PmProc* PmGetProcOfThisMaster(const PmMaster* const master,
			      const PmNodeId nodeId)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < master->procsCnt; i++) {
		if (nodeId == master->procs[i].node.nodeId) {
			proc = &master->procs[i];
		}
	}

	return proc;
}

/**
 * PmGetProcOfOtherMaster() - Get pointer to the processor with given node id,
 *          by excluding given master from the search
 * @master  Master to be excluded from search
 * @nodeId  Node id of the processor to be found
 *
 * @return  Pointer to processor that is not within the master and which has
 *          given node id, or NULL if such processor is not found
 */
PmProc* PmGetProcOfOtherMaster(const PmMaster* const master,
			       const PmNodeId nodeId)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		u32 p;

		if (master == pmAllMasters[i]) {
			continue;
		}

		for (p = 0U; p < pmAllMasters[i]->procsCnt; p++) {
			if (nodeId == pmAllMasters[i]->procs[p].node.nodeId) {
				proc = &pmAllMasters[i]->procs[p];
				goto done;
			}
		}
	}

done:
	return proc;
}

/**
 * PmGetProcByNodeId() - Get a pointer to processor structure by the node id
 * @nodeId  Node of the processor to be found
 *
 * @return  Pointer to a processor structure whose node is provided, or
 *          NULL if processor is not found
 */
PmProc* PmGetProcByNodeId(const PmNodeId nodeId)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		u32 p;

		for (p = 0U; p < pmAllMasters[i]->procsCnt; p++) {
			if (nodeId == pmAllMasters[i]->procs[p].node.nodeId) {
				proc = &pmAllMasters[i]->procs[p];
				goto done;
			}
		}
	}

done:
	return proc;
}

/**
 * PmGetProcByWfiStatus() - Get processor struct by wfi interrupt status
 * @mask    WFI interrupt mask read from GPI2 register
 *
 * @return  Pointer to a processor structure whose wfi mask is provided, or
 *          NULL if processor is not found
 */
PmProc* PmGetProcByWfiStatus(const u32 mask)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		u32 p;

		for (p = 0U; p < pmAllMasters[i]->procsCnt; p++) {
			if (0U != (mask & pmAllMasters[i]->procs[p].wfiStatusMask)) {
				proc = &pmAllMasters[i]->procs[p];
				goto done;
			}
		}
	}

done:
	return proc;
}

/**
 * PmGetProcByWakeStatus() - Get proc struct by wake interrupt status
 * @mask    GIC wake mask read from GPI1 register
 *
 * @return  Pointer to a processor structure whose wake mask is provided
 */
PmProc* PmGetProcByWakeStatus(const u32 mask)
{
	u32 i;
	PmProc *proc = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		u32 p;

		for (p = 0U; p < pmAllMasters[i]->procsCnt; p++) {
			if (0U != (mask & pmAllMasters[i]->procs[p].wakeStatusMask)) {
				proc = &pmAllMasters[i]->procs[p];
				goto done;
			}
		}
	}

done:
	return proc;
}

/**
 * PmWakeUpCancelScheduled() - Cancel scheduled wake-up sources of the master
 * @master  Pointer to a master whose scheduled wake-up sources should be
 *          cancelled
 */
static void PmWakeUpCancelScheduled(PmMaster* const master)
{
	PmRequirement* req = master->reqs;

	PmDbg("%s\r\n", PmStrNode(master->nid));

	while (NULL != req) {
		req->info &= ~PM_MASTER_WAKEUP_REQ_MASK;
		req = req->nextSlave;
	}

	/* Clear all wake-up sources */
	if (NULL != master->gic) {
		master->gic->clear();
	}
}

/**
 * PmCanRequestSuspend() - Check whether master is privileged to request another
 *                         master to suspend
 * @reqMaster   Master which requests another master to suspend
 * @respMaster  Master whose suspend is requested and which is extected to
 *              response to the request by initiating its own self suspend
 *
 * @return      Check result
 *              - True if master has privilege to request suspend
 *              - False if master has no privilege
 */
bool PmCanRequestSuspend(const PmMaster* const reqMaster,
			 const PmMaster* const respMaster)
{
	return 0U != (reqMaster->permissions & respMaster->ipiMask);
}

/**
 * PmIsRequestedToSuspend() - Check whether the master is requested from some
 *                            other master to suspend
 * @master      Master to check for
 *
 * @return      Check result
 *              - True if master is requested to suspend
 *              - False if no other master has requested this master to suspend
 */
bool PmIsRequestedToSuspend(const PmMaster* const master)
{
	return NULL != master->suspendRequest.initiator;
}

/**
 * PmMasterSuspendAck() - Acknowledge to the suspend request of another master
 * @mst		Master which is responding to the suspend request
 * @response	Status which is acknowledged as a response (whether the suspend
 *		operation is performed successfully)
 * @return	Status of the operation of sending acknowledge:
 *		- XST_SUCCESS if before calling this function the caller checked
 *		  that PmIsRequestedToSuspend returns true (the acknowledge
 *		  may need to be sent)
 *		- XST_FAILURE otherwise - this function didn't suppose to be
 *		  called
 */
int PmMasterSuspendAck(PmMaster* const mst, const int response)
{
	int status = XST_SUCCESS;

	if (NULL == mst->suspendRequest.initiator) {
		status = XST_FAILURE;
		goto done;
	}

	if (REQUEST_ACK_NON_BLOCKING == mst->suspendRequest.acknowledge) {
		PmAcknowledgeCb(mst->suspendRequest.initiator,
				mst->procs->node.nodeId, response,
				mst->procs->node.currState);
	} else if (REQUEST_ACK_BLOCKING == mst->suspendRequest.acknowledge) {
		IPI_RESPONSE1(mst->ipiMask, response);
	} else {
		/* No acknowledge */
	}
	mst->suspendRequest.initiator = NULL;

done:
	return status;
}

/**
 * PmMasterLastProcSuspending() - Check is the last awake processor suspending
 * @master	Master holding array of processors to be checked
 * @return	TRUE if there is exactly one processor in suspending state and
 *		all others are in sleep or forced power down.
 *		FALSE otherwise
 */
static bool PmMasterLastProcSuspending(const PmMaster* const master)
{
	bool status = false;
	u32 sleeping = 0U;
	u32 i;

	for (i = 0U; i < master->procsCnt; i++) {
		if (NODE_IS_OFF(&master->procs[i].node)) {
			/* Count how many processors is down */
			sleeping++;
		} else {
			/* Assume the one processor is suspending */
			status = PmProcIsSuspending(&master->procs[i]);
		}
	}

	/* If the number of asleep/down processors mismatch return false */
	if (master->procsCnt != (1U + sleeping)) {
		status = false;
	}

	/* Return whether the last standing processor is in suspending state */
	return status;
}

/**
 * PmMasterAllProcsDown() - Check if all processors are in sleep or forced down
 * @master	Master holding array of processors to be checked
 * @return	TRUE if all processors are in either sleep or forced down state
 *		FALSE otherwise
 */
static bool PmMasterAllProcsDown(const PmMaster* const master)
{
	bool status = true;
	u32 i;

	for (i = 0U; i < master->procsCnt; i++) {
		if (false == NODE_IS_OFF(&master->procs[i].node)) {
			status = false;
		}
	}

	return status;
}

/**
 * PmMasterNotify() - Notify master of a state change of its processor
 * @master      Pointer to master object which needs to be notified
 * @event       Processor Event to notify the master about
 *
 * @return      Status of potential changing of slave states or success.
 */
int PmMasterNotify(PmMaster* const master, const PmProcEvent event)
{
	int status = XST_SUCCESS;

	switch (event) {
	case PM_PROC_EVENT_SELF_SUSPEND:
		if ((true == PmMasterIsActive(master)) &&
		    (true == PmMasterLastProcSuspending(master))) {
			master->state = PM_MASTER_STATE_SUSPENDING;
		}
		break;
	case PM_PROC_EVENT_SLEEP:
		if (true == PmMasterIsSuspending(master)) {
			status = PmRequirementUpdateScheduled(master, true);
			master->state = PM_MASTER_STATE_SUSPENDED;
			if (true == PmIsRequestedToSuspend(master)) {
				status = PmMasterSuspendAck(master, XST_SUCCESS);
			}
		}
		break;
	case PM_PROC_EVENT_ABORT_SUSPEND:
		if (true == PmMasterIsSuspending(master)) {
			PmRequirementCancelScheduled(master);
			PmWakeUpCancelScheduled(master);
			master->state = PM_MASTER_STATE_ACTIVE;
		}
		break;
	case PM_PROC_EVENT_WAKE:
		if (true == PmMasterIsSuspended(master)) {
			status = PmRequirementUpdateScheduled(master, false);
		} else if (true == PmMasterIsKilled(master)) {
			PmRequirementRequestDefault(master);
			status = PmRequirementUpdateScheduled(master, false);
		} else {
			/* Must have else branch due to MISRA */
		}
		master->state = PM_MASTER_STATE_ACTIVE;
		break;
	case PM_PROC_EVENT_FORCE_PWRDN:
		if (true == PmMasterAllProcsDown(master)) {
			status = PmRequirementReleaseAll(master);
			PmWakeUpCancelScheduled(master);
			PmNotifierUnregisterAll(master);
			master->state = PM_MASTER_STATE_KILLED;
		}
		break;
	default:
		status = XST_PM_INTERNAL;
		PmDbg("ERROR: undefined event #%d\r\n", event);
		break;
	}

	return status;
}

/**
 * PmMasterWake() - Wake up the subsystem (master knows which processor to wake)
 * @mst		Master whose processor shall be woken up
 * @return	Status of the wake-up operation
 */
int PmMasterWake(const PmMaster* const mst)
{
	int status;
	PmProc* proc = mst->wakeProc;

	/* If master has a gic the wake-up sources need to be cleared */
	if (NULL != mst->gic) {
		mst->gic->clear();
	}

	if (NULL == proc) {
		proc = &mst->procs[0];
	}
	status = PmProcFsm(proc, PM_PROC_EVENT_WAKE);

	return status;
}

/**
 * PmMasterGetPlaceholder() - Check whether there is a master which holds nodeId
 * @nodeId      Id of the node whose placeholder should be found
 *
 * @return      Pointer to the master if such exist, otherwise NULL
 */
PmMaster* PmMasterGetPlaceholder(const PmNodeId nodeId)
{
	PmMaster* holder = NULL;
	u32 i;

	/* Find the master with the node placeholder */
	for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
		if (nodeId == pmAllMasters[i]->nid) {
			holder = pmAllMasters[i];
			break;
		}
	}

	return holder;
}
