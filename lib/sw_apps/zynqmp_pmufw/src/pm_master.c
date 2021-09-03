/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

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
#include "pmu_global.h"
#include "pm_node_reset.h"
#include "pm_clock.h"
#include "xpfw_restart.h"

#include "xpfw_ipi_manager.h"

#define SUBSYSTEM_RESTART_MASK	BIT(16U)
#define PM_REQUESTED_SUSPEND        0x1U
#define TO_ACK_CB(ack, status) (REQUEST_ACK_NON_BLOCKING == (ack))

#define TCM_HIVEC_BKP_LOCATION	0xFFFFFFC0U
#define HIVEC_TO_RESORE_CODE_BRANCH_OPCODE	0xEA003FBEU

#define DEFINE_PM_PROCS(c)	.procs = ((c)), \
				.procsCnt = ARRAY_SIZE((c))

#if defined(PM_LOG_LEVEL) && (PM_LOG_LEVEL > 0)
#define DEFINE_MASTER_NAME(n)	.name = n
#else
#define DEFINE_MASTER_NAME(n)	.name = ""
#endif

static PmMaster* pmMasterHead = NULL;

/* Restore code structure*/
typedef struct RPURestoreCode {
	unsigned int AddrLocation;
	unsigned int RestoreData;
}RPURestoreCode;

static const RPURestoreCode RestoreCode[6] = {
	// ldr r0,=0xFFFFFFC0
	{ .AddrLocation = 0XFFFFFF00U, .RestoreData = 0XE3E0003FU },

	// ldr r1, [r0]
	{ .AddrLocation = 0XFFFFFF04U, .RestoreData = 0XE5901000U },

	// ldr r0, [pc, #4]; ~0x14
	{ .AddrLocation = 0XFFFFFF08U, .RestoreData = 0XE59F0004U },

	// str r1, [r0]
	{ .AddrLocation = 0XFFFFFF0CU, .RestoreData = 0XE5801000U },

	// b 0xFFFC0000
	{ .AddrLocation = 0XFFFFFF10U, .RestoreData = 0XEAFF003AU },

	// =0xFFFF0000
	{ .AddrLocation = 0XFFFFFF14U, .RestoreData = PM_PROC_RPU_HIVEC_ADDR },
};

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
static s32 PmApuPrepareSuspendToRam(void)
{
	s32 status;
	u32 i;
	PmRequirement* req = PmRequirementGet(&pmMasterApu_g, &pmSlaveL2_g.slv);

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
		req = PmRequirementGet(&pmMasterApu_g,
					pmMasterApu_g.memories[i]);
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
static s32 PmApuEvaluateState(const u32 state)
{
	s32 status;

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

/**
 * PmMasterRpuRemapAddr() - Remap address from RPU's (lockstep) to PMU's view
 * @address     Address to remap
 *
 * @return      Remapped address or the provided address if no remapping done
 */
static u32 PmMasterRpuRemapAddr(const u32 address)
{
	u32 remapAddr = address;

	if (address < (4U * pmSlaveTcm0A_g.size)) {
		remapAddr += pmSlaveTcm0A_g.base;
	}

	return remapAddr;
}

/**
 * PmMasterRpu0RemapAddr() - Remap address from RPU_0's to PMU's view
 * @address     Address to remap
 *
 * @return      Remapped address or the provided address if no remapping done
 */
static u32 PmMasterRpu0RemapAddr(const u32 address)
{
	u32 remapAddr = address;

	if (address < pmSlaveTcm0A_g.size) {
		remapAddr += pmSlaveTcm0A_g.base;
	} else {
		if ((address >= (2U * pmSlaveTcm0A_g.size)) &&
		    (address < ((2U * pmSlaveTcm0A_g.size) + pmSlaveTcm0B_g.size))) {
			remapAddr += pmSlaveTcm0B_g.base;
		}
	}

	return remapAddr;
}

/**
 * PmMasterRpu1RemapAddr() - Remap address from RPU_1's to PMU's view
 * @address     Address to remap
 *
 * @return      Remapped address or the provided address if no remapping done
 */
static u32 PmMasterRpu1RemapAddr(const u32 address)
{
	u32 remapAddr = address;

	if (address < pmSlaveTcm1A_g.size) {
		remapAddr += pmSlaveTcm1A_g.base;
	} else {
		if ((address >= (2U * pmSlaveTcm1A_g.size)) &&
		    (address < ((2U * pmSlaveTcm1A_g.size) + pmSlaveTcm1B_g.size))) {
			remapAddr += pmSlaveTcm1B_g.base;
		}
	}

	return remapAddr;
}

static PmProc* pmMasterApuProcs[] = {
	&pmProcApu0_g,
	&pmProcApu1_g,
	&pmProcApu2_g,
	&pmProcApu3_g,
};

PmMaster pmMasterApu_g = {
	DEFINE_PM_PROCS(pmMasterApuProcs),
	.wakeProc = NULL,
	.nid = NODE_APU,
	.ipiMask = IPI_PMU_0_IER_APU_MASK,
	.reqs = NULL,
	.nextMaster = NULL,
	.wakePerms = 0U,
	.suspendPerms = 0U,
	.suspendTimeout = 0U,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
	.state = PM_MASTER_STATE_UNINITIALIZED,
	.gic = &pmGicProxy,
	.memories = pmApuMemories,
	.evalState = PmApuEvaluateState,
	.remapAddr = NULL,
	DEFINE_MASTER_NAME("APU")
};

static PmProc* pmMasterRpuProcs[] = {
	&pmProcRpu0_g,
};

/* RPU in lockstep mode */
PmMaster pmMasterRpu_g = {
	DEFINE_PM_PROCS(pmMasterRpuProcs),
	.wakeProc = NULL,
	.nextMaster = NULL,
	.nid = NODE_RPU,
	.ipiMask = IPI_PMU_0_IER_RPU_0_MASK,
	.reqs = NULL,
	.wakePerms = 0U,
	.suspendPerms = 0U,
	.suspendTimeout = 0U,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
	.state = PM_MASTER_STATE_UNINITIALIZED,
	.gic = NULL,
	.memories = NULL,
	.evalState = NULL,
	.remapAddr = PmMasterRpuRemapAddr,
	DEFINE_MASTER_NAME("RPU")
};

static PmProc* pmMasterRpu0Procs[] = {
	&pmProcRpu0_g,
};

/* RPU in split mode can have 2 masters: RPU_0 and RPU_1 */
PmMaster pmMasterRpu0_g = {
	DEFINE_PM_PROCS(pmMasterRpu0Procs),
	.wakeProc = NULL,
	.nextMaster = NULL,
	.nid = NODE_RPU_0,
	.ipiMask = 0U,
	.reqs = NULL,
	.wakePerms = 0U,
	.suspendPerms = 0U,
	.suspendTimeout = 0U,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
	.state = PM_MASTER_STATE_UNINITIALIZED,
	.gic = NULL,
	.memories = NULL,
	.evalState = NULL,
	.remapAddr = PmMasterRpu0RemapAddr,
	DEFINE_MASTER_NAME("RPU0"),
};

static PmProc* pmMasterRpu1Procs[] = {
	&pmProcRpu1_g,
};

PmMaster pmMasterRpu1_g = {
	DEFINE_PM_PROCS(pmMasterRpu1Procs),
	.wakeProc = NULL,
	.nid = NODE_RPU_1,
	.ipiMask = 0U,
	.reqs = NULL,
	.nextMaster = NULL,
	.wakePerms = 0U,
	.suspendPerms = 0U,
	.suspendTimeout = 0U,
	.suspendRequest = {
		.initiator = NULL,
		.acknowledge = 0U,
	},
	.state = PM_MASTER_STATE_UNINITIALIZED,
	.gic = NULL,
	.memories = NULL,
	.evalState = NULL,
	.remapAddr = PmMasterRpu1RemapAddr,
	DEFINE_MASTER_NAME("RPU1"),
};

/* Array of all possible masters supported by the PFW */
static PmMaster *const pmMastersAll[] = {
	&pmMasterApu_g,
	&pmMasterRpu_g,		/* RPU lockstep */
	&pmMasterRpu0_g,	/* RPU split mode, core 0 */
	&pmMasterRpu1_g,	/* RPU split mode, core 1 */
};

/**
 * PmMasterAdd() - Add new master in the list
 * @newMaster   Master to be added in the list
 */
static void PmMasterAdd(PmMaster* const newMaster)
{
	newMaster->nextMaster = pmMasterHead;
	pmMasterHead = newMaster;
}

/**
 * PmMasterDefaultConfig() - Add default masters (call only upon PM init)
 */
void PmMasterDefaultConfig(void)
{
	PmMasterAdd(&pmMasterApu_g);
	PmMasterAdd(&pmMasterRpu_g);
}

/**
 * PmMasterSetConfig() - Set configuration for a master
 * @mst         Master for which the configuration is set
 * @cfg         Configuration data to set
 *
 * @note        Master is automatically added in the list of available masters
 */
void PmMasterSetConfig(PmMaster* const mst, const PmMasterConfig* const cfg)
{
	u32 i;

	mst->ipiMask = cfg->ipiMask;
	mst->suspendTimeout = cfg->suspendTimeout;
	mst->suspendPerms = cfg->suspendPerms;
	mst->wakePerms = cfg->wakePerms;
	PmMasterAdd(mst);

	/* Update pointers of processors to the master */
	for (i = 0U; i < mst->procsCnt; i++) {
		mst->procs[i]->master = mst;
	}
}

void PmMasterClearConfig(void)
{
	PmMaster* mst = pmMasterHead;

	while (NULL != mst) {
		PmMaster* next;

		/* Clear the configuration of the master */
		mst->wakeProc = NULL;
		mst->ipiMask = 0U;
		mst->wakePerms = 0U;
		mst->suspendPerms = 0U;
		mst->suspendTimeout = 0U;
		mst->suspendRequest.initiator = NULL;
		mst->suspendRequest.acknowledge = 0U;

		/* Clear requirements of the master */
		mst->reqs = NULL;

		/* Clear the pointer to the next master */
		next = mst->nextMaster;
		mst->nextMaster = NULL;

		/* Process next master */
		mst = next;
	}

	/* Delete the list of available masters */
	pmMasterHead = NULL;

	/* Free allocated requirements from the heap */
	PmRequirementFreeAll();
}

/**
 * PmGetMasterByIpiMask() - Use to get pointer to master structure by ipi mask
 * @mask    IPI Mask of a master (requestor) in IPI registers
 *
 * @return  Pointer to a PmMaster structure or NULL if master is not found
 */
PmMaster* PmGetMasterByIpiMask(const u32 mask)
{
	PmMaster* mst = pmMasterHead;

	while (NULL != mst) {
		if (0U != (mask & mst->ipiMask)) {
			break;
		}
		mst = mst->nextMaster;
	}

	return mst;
}

/**
 * PmMasterGetNextFromIpiMask() - Get next master from ORed masters' IPI mask
 * @mask	Mask from which we need to extract a master
 *
 * @return	Pointer to a master of NULL if mask does not encode a master
 *
 * @note	The argument represents ORed IPI masks of multiple (or none)
 *		masters, where each master is encoded by a set bit. If a pointer
 *		to the master is found, the associated bitfield in the mask is
 *		cleared.
 */
PmMaster* PmMasterGetNextFromIpiMask(u32* const mask)
{
	PmMaster* master = NULL;
	u32 masterCnt = (u32)__builtin_popcount(*mask);
	u32 ipiMask;

	if (0U == masterCnt) {
		goto done;
	}

	ipiMask = (u32)1 << (u32)__builtin_ctz(*mask);
	master = PmGetMasterByIpiMask(ipiMask);
	*mask &= ~ipiMask;

done:
	return master;
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
		if (nodeId == master->procs[i]->node.nodeId) {
			proc = master->procs[i];
		}
	}

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
	PmProc *proc = NULL;
	PmMaster* mst = pmMasterHead;

	while (NULL != mst) {
		u32 p;

		for (p = 0U; p < mst->procsCnt; p++) {
			if (0U != (mask & mst->procs[p]->mask)) {
				proc = mst->procs[p];
				goto done;
			}
		}
		mst = mst->nextMaster;
	}

done:
	return proc;
}

/**
 * PmMasterConfigWakeEvents() - Configure wake events for the master
 * @master	Master for which wake events should be configured
 * @enable	Flag (true to enable event propagation, false otherwise)
 *
 * @note	Config method of the wake event class must ensure that only set
 *		wake gets enabled/disabled.
 */
static void PmMasterConfigWakeEvents(PmMaster* const master, const bool enable)
{
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		PmWakeEvent* we = req->slave->wake;

		if ((NULL != we) && (NULL != we->class)) {
			if (NULL != we->class->config) {
				we->class->config(we, master->ipiMask, enable);
			}
		}
		req = req->nextSlave;
	}
}

/**
 * PmWakeUpCancelScheduled() - Cancel scheduled wake-up sources of the master
 * @master  Pointer to a master whose scheduled wake-up sources should be
 *          cancelled
 */
static void PmWakeUpCancelScheduled(PmMaster* const master)
{
	PmRequirement* req = master->reqs;

	while (NULL != req) {
		req->info &= ~(u8)PM_MASTER_WAKEUP_REQ_MASK;
		req = req->nextSlave;
	}

	/* Clear all wake-up sources */
	if (NULL != master->gic) {
		master->gic->clear();
	}
	PmMasterConfigWakeEvents(master, false);
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
	return 0U != (reqMaster->ipiMask & respMaster->suspendPerms);
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
s32 PmMasterSuspendAck(PmMaster* const mst, const s32 response)
{
	s32 status = XST_SUCCESS;

	if (NULL == mst->suspendRequest.initiator) {
		status = XST_FAILURE;
		goto done;
	}

	if (REQUEST_ACK_NON_BLOCKING == mst->suspendRequest.acknowledge) {
		PmAcknowledgeCb(mst->suspendRequest.initiator,
				mst->procs[0]->node.nodeId, (u32)response,
				mst->procs[0]->node.currState);
	} else if (REQUEST_ACK_BLOCKING == mst->suspendRequest.acknowledge) {
		IPI_RESPONSE1(mst->ipiMask, (u32)response);
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
		if (NODE_IS_OFF(&master->procs[i]->node)) {
			/* Count how many processors is down */
			sleeping++;
		} else {
			/* Assume the one processor is suspending */
			status = PmProcIsSuspending(master->procs[i]);
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
		if (false == NODE_IS_OFF(&master->procs[i]->node)) {
			status = false;
		}
	}

	return status;
}

/**
 * PmWakeMasterBySlave() - Wake the master for which slave is set as
 *			   wakeup source
 * @slave	Slave which set as wakeup source
 *
 * @return	Status of performing wake
 */
s32 PmWakeMasterBySlave(const PmSlave * const slave)
{
	PmMaster *mst = pmMasterHead;
	s32 finalStatus = XST_SUCCESS;
	s32 status;

	while (NULL != mst) {
		PmRequirement *masterReq = PmRequirementGet(mst, slave);

		if ((masterReq->info & PM_MASTER_WAKEUP_REQ_MASK) != 0U) {
			status = PmMasterWake(mst);
			if (status != XST_SUCCESS) {
				finalStatus = XST_FAILURE;
			}
		}
		mst = mst->nextMaster;
	}
	return finalStatus;
}

/**
 * PmMasterWakeProc() - Master prepares for wake and wakes the processor
 * @proc	Processor to wake up
 *
 * @return	Status of performing wake
 */
s32 PmMasterWakeProc(PmProc* const proc)
{
	s32 status;
	bool hasResumeAddr = PmProcHasResumeAddr(proc);

	if (false == hasResumeAddr) {
		status = XST_FAILURE;
		goto done;
	}

	status = PmMasterFsm(proc->master, PM_MASTER_EVENT_WAKE);
	if (XST_SUCCESS != status) {
		goto done;
	}
	status = PmProcFsm(proc, PM_PROC_EVENT_WAKE);

done:
	return status;
}

/**
 * PmMasterForceDownProcs() - Force down processors of this master
 * @master	Master whose processors need to be forced down
 *
 * @return	Status of forcing down
 */
static s32 PmMasterForceDownProcs(const PmMaster* const master)
{
	u32 i;
	s32 status = XST_SUCCESS;

	for (i = 0U; i < master->procsCnt; i++) {
		s32 ret = PmNodeForceDown(&master->procs[i]->node);

		if (XST_SUCCESS != ret) {
			status = ret;
		}
	}

	return status;
}

/**
 * PmMasterForceDownCleanup() - Cleanup due to the force down
 * @master	Master being forced down
 *
 * @return	Status of performing cleanup (releasing resources)
 */
static s32 PmMasterForceDownCleanup(PmMaster* const master)
{
	s32 status;

	status = PmRequirementRelease(master->reqs, RELEASE_ALL);
	PmWakeUpCancelScheduled(master);
	PmNotifierUnregisterAll(master);
	master->wakeProc = NULL;
	master->suspendRequest.initiator = NULL;

	return status;
}

/**
 * PmMasterIdleSlaves() - Idle and reset active slaves of a master
 * @master	Master whose slaves need to be idled and reset
 *
 * @note	Idle and reset slaves which have an active clock and
 * 		- Are not requested by any other master and this master is
		uninitialized, or
		- Are exclusively used by this master
 */
static void PmMasterIdleSlaves(PmMaster* const master)
{
#ifdef ENABLE_NODE_IDLING
	PmRequirement* req = master->reqs;
	PmNode* Node;

	PmInfo("%s idle slaves\r\n", master->name);

	while (NULL != req) {
		u32 usage = PmSlaveGetUsageStatus(req->slave, master);
		Node = &req->slave->node;

		if (0U == (Node->flags & NODE_IDLE_DONE)) {
			if (((PM_MASTER_STATE_UNINITIALIZED == master->state) &&
			     (0U == (usage & PM_USAGE_OTHER_MASTER))) ||
			    (usage == PM_USAGE_CURRENT_MASTER)) {
				if (XST_SUCCESS == PmClockIsActive(Node)) {
					PmNodeReset(master, Node->nodeId,
						NODE_IDLE_REQ);
					Node->flags |= NODE_IDLE_DONE;
				}
			}
		}
		req = req->nextSlave;
	}
#endif
}

/**
 * PmMasterFsm() - Implements finite state machine (FSM) for a master
 * @master	Master whose state machine is triggered
 * @event	Event to which the master's state machine need to react
 *
 * @return	Status of changing state
 */
s32 PmMasterFsm(PmMaster* const master, const PmMasterEvent event)
{
	s32 status = XST_SUCCESS;
	bool condition;
	u8 prevState = master->state;

	switch (event) {
	case PM_MASTER_EVENT_SELF_SUSPEND:
		condition = PmMasterLastProcSuspending(master);
		if ((PM_MASTER_STATE_ACTIVE == master->state) &&
		    (true == condition)) {
			master->state = PM_MASTER_STATE_SUSPENDING;
		}
		break;
	case PM_MASTER_EVENT_SLEEP:
		if (PM_MASTER_STATE_SUSPENDING == master->state) {
#ifdef ENABLE_POS
			bool isPoS = PmSystemDetectPowerOffSuspend(master);
			if (true == isPoS) {
				status = PmSystemPreparePowerOffSuspend();
			}
#endif
			status = PmRequirementUpdateScheduled(master, true);
			master->state = PM_MASTER_STATE_SUSPENDED;
			condition = PmIsRequestedToSuspend(master);
			if (true == condition) {
				status = PmMasterSuspendAck(master, XST_SUCCESS);
			}
			PmMasterConfigWakeEvents(master, true);
#ifdef ENABLE_POS
			if (true == isPoS) {
				status = PmSystemFinalizePowerOffSuspend();
			}
#endif
		}
		break;
	case PM_MASTER_EVENT_ABORT_SUSPEND:
		if (PM_MASTER_STATE_SUSPENDING == master->state) {
			PmRequirementCancelScheduled(master);
			PmWakeUpCancelScheduled(master);
			master->state = PM_MASTER_STATE_ACTIVE;
		}
		break;
	case PM_MASTER_EVENT_WAKE:
		if (PM_MASTER_STATE_SUSPENDED == master->state) {
			status = PmRequirementUpdateScheduled(master, false);
			if (XST_SUCCESS == status) {
				PmWakeUpCancelScheduled(master);
			}
		} else if (PM_MASTER_STATE_KILLED == master->state) {
			PmRequirementPreRequest(master);
			status = PmRequirementUpdateScheduled(master, false);
			if (XST_SUCCESS == status) {
				PmRequirementClockRestore(master);
			}
		} else {
			/* Must have else branch due to MISRA */
		}
		if (PM_MASTER_STATE_UNINITIALIZED != master->state) {
			master->state = PM_MASTER_STATE_ACTIVE;
		}
		break;
	case PM_MASTER_EVENT_FORCED_PROC:
		condition = PmMasterAllProcsDown(master);
		if (true == condition) {
			PmMasterIdleSlaves(master);
			status = PmMasterForceDownCleanup(master);
			master->state = PM_MASTER_STATE_KILLED;
		}
		break;
	case PM_MASTER_EVENT_FORCE_DOWN:
		master->state = PM_MASTER_STATE_KILLED;
		status = PmMasterForceDownProcs(master);
		if (XST_SUCCESS == status) {
			if (PM_MASTER_STATE_UNINITIALIZED == prevState) {
				master->state = PM_MASTER_STATE_UNINITIALIZED;
			}
			PmMasterIdleSlaves(master);
			if (PM_MASTER_STATE_UNINITIALIZED != prevState) {
				status = PmMasterForceDownCleanup(master);
			}
		}
		break;
	default:
		status = XST_PM_INTERNAL;
		PmErr("Unknown event #%d\r\n", event);
		break;
	}

	return status;
}

/**
 * PmMasterWake() - Wake up the subsystem (master knows which processor to wake)
 * @mst		Master whose processor shall be woken up
 * @return	Status of the wake-up operation
 */
s32 PmMasterWake(const PmMaster* const mst)
{
	s32 status;
	PmProc* proc = mst->wakeProc;

	if (NULL == proc) {
		proc = mst->procs[0];
	}
	status = PmMasterWakeProc(proc);

	return status;
}

/**
 * PmGetStartAddress() - Get master hand off address for restart
 * @master	Master to restart
 * @address	Hand off address
 *
 * @return	Void
 */
static void PmGetStartAddress (PmMaster* const master, u64 *address)
{
	if ((master == &pmMasterRpu_g) || (master == &pmMasterRpu0_g)) {
		u32 i;
		*address = PM_PROC_RPU_HIVEC_ADDR;	/* Highvec */

		/* Backup of HiVec memory */
		XPfw_Write32(TCM_HIVEC_BKP_LOCATION,
				XPfw_Read32(PM_PROC_RPU_HIVEC_ADDR));

		/* Branch to restore location */
		XPfw_Write32(PM_PROC_RPU_HIVEC_ADDR,
				HIVEC_TO_RESORE_CODE_BRANCH_OPCODE);

		/*
		 * Code to restore HiVec and branch to FSBL
		 */
		for (i = 0; i < ARRAYSIZE(RestoreCode); i++) {
			XPfw_Write32(RestoreCode[i].AddrLocation,
					RestoreCode[i].RestoreData);
		}
	} else {
		*address = FSBL_LOAD_ADDR;
	}
}

/**
 * PmMasterRestart() - Restart the master
 * @master	Master to restart
 *
 * @return	Status of performing the operation
 */
s32 PmMasterRestart(PmMaster* const master)
{
	s32 status;
	u64 address = 0xFFFC0000ULL;

	u32 FsblProcInfo = XPfw_Read32(PMU_GLOBAL_GLOBAL_GEN_STORAGE5);

	if ((FSBL_RUNNING_ON_A53 == (FsblProcInfo & FSBL_STATE_PROC_INFO_MASK)) &&
			(master == &pmMasterApu_g)) {
		if (FSBL_Store_Restore_Info.OcmAndFsblInfo == XPFW_OCM_IS_USED) {
			PmWarn("OCM is used by XilFPGA. APU-restart will not work\r\n");
			status = XST_FAILURE;
			goto done;
		}

		if (FSBL_Store_Restore_Info.OcmAndFsblInfo != 0x0U) {
#if defined(USE_DDR_FOR_APU_RESTART) && defined(ENABLE_SECURE)
			status = XPfw_RestoreFsblToOCM();
			if (XST_SUCCESS != status) {
				goto done;
			}
#endif
		}
	} else if (((FSBL_RUNNING_ON_R5_0 == (FsblProcInfo & FSBL_STATE_PROC_INFO_MASK)) &&
					(master == &pmMasterRpu0_g)) ||
			((FSBL_RUNNING_ON_R5_L == (FsblProcInfo & FSBL_STATE_PROC_INFO_MASK)) &&
					(master == &pmMasterRpu_g))) {
		/* Do nothing */
	} else {
		status = XST_NO_FEATURE;
		goto done;
	}

	XPfw_RecoveryAck(master);
	PmSystemPrepareForRestart(master);
	status = PmMasterFsm(master, PM_MASTER_EVENT_FORCE_DOWN);
	if (XST_SUCCESS != status) {
		goto done;
	}
	status = PmMasterFsm(master, PM_MASTER_EVENT_WAKE);
	if (XST_SUCCESS != status) {
		goto done;
	}
	XPfw_RMW32(PMU_GLOBAL_GLOBAL_GEN_STORAGE4, SUBSYSTEM_RESTART_MASK,
                        SUBSYSTEM_RESTART_MASK);

	PmGetStartAddress (master, &address);

#ifdef ENABLE_POS
	/* Signal to FSBL */
	XPfw_Write32(PMU_GLOBAL_GLOBAL_GEN_STORAGE1, 1U);
#endif
	status = master->procs[0]->saveResumeAddr(master->procs[0], address);
	if (XST_SUCCESS != status) {
		goto done;
	}
	status = PmProcFsm(master->procs[0], PM_PROC_EVENT_WAKE);
	PmSystemRestartDone(master);

done:
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
	for (i = 0U; i < ARRAY_SIZE(pmMastersAll); i++) {
		if (nodeId == pmMastersAll[i]->nid) {
			holder = pmMastersAll[i];
			break;
		}
	}

	return holder;
}

/**
 * PmMasterCanForceDown() - Check if master has permissions to force power down
 *                          the power node
 * @master      Master which wants to force power down the node
 * @power       Target power node
 *
 * @return      True if master has permission to force power down the node,
 *              false otherwise
 */
inline bool PmMasterCanForceDown(const PmMaster* const master,
				 const PmPower* const power)
{
	return 0U != (power->forcePerms & master->ipiMask);
}

#ifdef IDLE_PERIPHERALS
void PmMasterIdleSystem(void)
{
	PmMaster* mst = pmMasterHead;

	while (NULL != mst) {
		PmMasterIdleSlaves(mst);
		mst = mst->nextMaster;
	}
}
#endif

/**
 * PmMasterInitFinalize() - Master has completed initialization, finalize init
 * @master	Master which has finalized initialization
 */
s32 PmMasterInitFinalize(PmMaster* const master)
{
	s32 status;

	master->state = PM_MASTER_STATE_ACTIVE;

	status = PmRequirementRelease(master->reqs, RELEASE_UNREQUESTED);

	return status;
}

#ifdef ENABLE_POS
/**
 * PmMasterIsLastSuspending() - Check if master is in suspending state and all
 *                              other masters in system are suspended or killed
 * @master     Master to be checked
 *
 * @return     True if master is in suspending state and all other masters are
 *             killed or suspended, false otherwise
 */
bool PmMasterIsLastSuspending(const PmMaster* const master)
{
	PmMaster* mst = pmMasterHead;
	bool isLastWake = true;

	while (NULL != mst) {
		if (master == mst) {
			if (mst->state == PM_MASTER_STATE_SUSPENDING) {
				mst = mst->nextMaster;
				continue;
			} else {
				isLastWake = false;
				break;
			}
		}

		if ((mst->state != PM_MASTER_STATE_SUSPENDED) &&
			(mst->state != PM_MASTER_STATE_KILLED)) {
			isLastWake = false;
			break;
		}

		mst = mst->nextMaster;
	}

	return isLastWake;
}

/**
 * @PmMasterIsUniqueWakeup() - Check if any device except slave is used as
 * 			       wakeup source in system
 *
 * @slave      Slave node to be checked
 *
 * @return     True if slave is the unique wakeup source used in system, false
 *             otherwise
 */
bool PmMasterIsUniqueWakeup(const PmSlave* const slave)
{
	PmMaster* mst = pmMasterHead;
	bool isUnique = false;

	while (NULL != mst) {
		PmRequirement* req = mst->reqs;

		while (NULL != req) {
			if ((PM_MASTER_WAKEUP_REQ_MASK & req->info) != 0U) {
				if (req->slave != slave) {
					isUnique = false;
					goto done;
				} else {
					isUnique = true;
				}
			}
			req = req->nextSlave;
		}

		mst = mst->nextMaster;
	}

done:
	return isUnique;
}

/**
 * @PmMasterReleaseAll() - Release all requirements for every master in system
 *
 * @return     Status of releasing all requirements for every master in system
 */
s32 PmMasterReleaseAll(void)
{
	s32 status = 0;
	PmMaster* mst = pmMasterHead;

	while (NULL != mst) {
		status = PmRequirementRelease(mst->reqs, RELEASE_ALL);
		if (XST_SUCCESS != status) {
			goto done;
		}

		mst = mst->nextMaster;
	}

done:
	return status;
}
#endif

#endif
