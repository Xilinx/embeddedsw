/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * This file contains implementation of the PM API functions, which
 * should be used directly only by power management itself.
 *********************************************************************/

#include "csu.h"
#include "pm_core.h"
#include "pm_node.h"
#include "pm_proc.h"
#include "pm_defs.h"
#include "pm_common.h"
#include "pm_callbacks.h"
#include "pm_reset.h"
#include "pm_notifier.h"
#include "pm_mmio_access.h"
#include "pm_system.h"
#include "pm_pinctrl.h"
#ifdef ENABLE_FPGA_LOAD
#include "xilfpga.h"
#endif
#include "pm_clock.h"
#include "pm_requirement.h"
#include "pm_config.h"
#ifdef ENABLE_IOCTL
#include "pm_ioctl.h"
#endif
#include "xpfw_platform.h"
#include "xpfw_resets.h"
#include "rpu.h"
#ifdef ENABLE_SECURE
#include "xsecure.h"
#ifndef XSK_ACCESS_PUF_USER_EFUSE
#include "xilskey_eps_zynqmp_puf.h"
#endif
#endif
#ifdef EFUSE_ACCESS
#include "xilskey_eps_zynqmp.h"
#endif
#include "pmu_iomodule.h"
#include "xpfw_ipi_manager.h"
#include "xpfw_restart.h"

#ifdef ENABLE_WDT
#include "xpfw_mod_wdt.h"
#endif

#define AES_PUF_KEY_SEL_MASK	0x2U

#define INVALID_ACK_ARG(a)	(((a) < REQUEST_ACK_MIN) || ((a) > REQUEST_ACK_MAX))

/*
 * PM error numbers, mostly used to identify erroneous usage of EEMI. Note:
 * these errors are errors from the perspective of using EEMI API. PMU-FW
 * is robust and deals properly with those errors (do not affect its internal
 * operations). Standard XStatus is not used for this purpose because these
 * error codes are more detailed and used for logging/optimizing prints
 */
#define PM_ERRNO_INVALID_NODE		1U
#define PM_ERRNO_INVALID_ACK		2U
#define PM_ERRNO_NO_PERMISSION		3U
#define PM_ERRNO_NO_WR_PERMISSION	4U
#define PM_ERRNO_NO_RD_PERMISSION	5U
#define PM_ERRNO_NO_RESET_PERMISSION	6U
#define PM_ERRNO_NO_ADDRESS		7U
#define PM_ERRNO_NO_REQUEST		8U
#define PM_ERRNO_DOUBLE_REQUEST		9U
#define PM_ERRNO_INVALID_LATENCY	10U
#define PM_ERRNO_INVALID_TYPE		11U
#define PM_ERRNO_INVALID_SUBTYPE	12U
#define PM_ERRNO_INVALID_RESET		13U
#define PM_ERRNO_NO_TEMP_SUPPORT	14U

#if defined(PM_LOG_LEVEL) && (PM_LOG_LEVEL >= PM_WARNING)
#define PmLog(errno, value, mst)	\
	PmLogInt(__LINE__, errno, value, mst)

/**
 * PmLogInt() - Log an error related to EEMI usage
 * @line	Line in this file where the error is logged
 * @errno	Error code
 * @value	Additional information about the error (an integer)
 * @mst		Name of the master who issued the EEMI API
 */
static void PmLogInt(const u32 line, const u32 errno, const u32 value,
		     const char* const mst)
{
	pm_printf("pm_core.c@%lu %s> ", line, mst);

	switch (errno) {
	case PM_ERRNO_INVALID_NODE:
		PmWarn("Invalid node %lu\r\n", value);
		break;
	case PM_ERRNO_INVALID_ACK:
		PmWarn("Invalid ack %lu\r\n", value);
		break;
	case PM_ERRNO_NO_PERMISSION:
		PmWarn("No permission\r\n");
		break;
	case PM_ERRNO_NO_WR_PERMISSION:
		PmWarn("No write permission to 0x%lx\r\n", value);
		break;
	case PM_ERRNO_NO_RD_PERMISSION:
		PmWarn("No read permission to 0x%lx\r\n", value);
		break;
	case PM_ERRNO_NO_RESET_PERMISSION:
		PmWarn("No reset %lu permission\r\n", value);
		break;
	case PM_ERRNO_NO_ADDRESS:
		PmWarn("Address not provided\r\n");
		break;
	case PM_ERRNO_NO_REQUEST:
		PmWarn("Node %d not requested\r\n", value);
		break;
	case PM_ERRNO_DOUBLE_REQUEST:
		PmWarn("Node %d already requested\r\n", value);
		break;
	case PM_ERRNO_INVALID_LATENCY:
		PmWarn("Invalid latency! Try > %lu\r\n", value);
		break;
	case PM_ERRNO_INVALID_SUBTYPE:
		PmWarn("Invalid subtype %lu\r\n", value);
		break;
	case PM_ERRNO_INVALID_TYPE:
		PmWarn("Invalid type %lu\r\n", value);
		break;
	case PM_ERRNO_INVALID_RESET:
		PmWarn("Invalid reset %lu\r\n", value);
		break;
	case PM_ERRNO_NO_TEMP_SUPPORT:
		PmWarn("Temperature not supported\r\n");
		break;
	default:
		PmWarn("Invalid errno %lu\r\n", errno);
		break;
	}
}
#else
#define PmLog(errno, value, mst)	{}
#endif


/**
 * PmKillBoardPower() - Power-off board by sending KILL signal to power chip
 */
#if defined(BOARD_SHUTDOWN_PIN) && defined(BOARD_SHUTDOWN_PIN_STATE)
void PmKillBoardPower(void)
{
	u32 reg = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	u32 mask = PMU_IOMODULE_GPO1_MIO_0_MASK << BOARD_SHUTDOWN_PIN;
	u32 value = BOARD_SHUTDOWN_PIN_STATE << BOARD_SHUTDOWN_PIN;
	u32 mioPinOffset;

	mioPinOffset = IOU_SLCR_MIO_PIN_34_OFFSET + (BOARD_SHUTDOWN_PIN - 2U)*4U;

	reg = (reg & (~mask)) | (mask & value);
	XPfw_Write32(PMU_IOMODULE_GPO1, reg);
	/* Configure board shutdown pin to be controlled by the PMU */
	XPfw_RMW32((IOU_SLCR_BASE + mioPinOffset),
			0x000000FEU, 0x00000008U);
}
#else
void PmKillBoardPower(void) { }
#endif

/**
 * PmProcessAckRequest() -Returns appropriate acknowledge if required
 * @ack     Ack argument as requested by the master
 * @master  IPI channel to use
 * @nodeId  Node ID of requesting PU
 * @status  Status of PM's operation
 * @oppoint Operating point of node in question
 */
static void PmProcessAckRequest(const u32 ack,
				const PmMaster* const master,
				const PmNodeId nodeId,
				const u32 status,
				const u32 oppoint)
{
	if (REQUEST_ACK_BLOCKING == ack) {
		/* Return status immediately */
		IPI_RESPONSE1(master->ipiMask, (u32)status);
	} else if (REQUEST_ACK_NON_BLOCKING == ack) {
		/* Return acknowledge through callback */
		PmAcknowledgeCb(master, nodeId, status, oppoint);
	} else {
		/* No returning of the acknowledge */
	}
}

/**
 * PmSelfSuspend() - Requested self suspend for a processor
 * @master  Master who initiated the request
 * @node    Processor or subsystem node to be suspended
 * @latency Maximum latency for processor to go back to active state
 * @state   Encoded state that is specific for each master
 *
 * @note    Used to announce starting of self suspend procedure. Node will be
 *          put to sleep when server handles corresponding processor's WFI
 *          interrupt.
 */
static void PmSelfSuspend(const PmMaster *const master,
			  const u32 node,
			  const u32 latency,
			  const u32 state,
			  const u64 address)
{
	s32 status;
	u32 worstCaseLatency = 0U;
	/* the node ID must refer to a processor belonging to this master */
	PmProc* proc = PmGetProcOfThisMaster(master, (PmNodeId)node);

	PmInfo("%s> SelfSuspend(%lu, %lu, %lu, 0x%llx)\r\n", master->name, node,
	       latency, state, address);

	if (NULL == proc) {
		PmLog(PM_ERRNO_INVALID_NODE, node, master->name);
		status = XST_INVALID_PARAM;
		goto done;
	}

	worstCaseLatency = proc->pwrDnLatency + proc->pwrUpLatency;
	if (latency < worstCaseLatency) {
		PmLog(PM_ERRNO_INVALID_LATENCY, worstCaseLatency, master->name);
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Remember latency requirement */
	proc->latencyReq = latency;

	status = proc->saveResumeAddr(proc, address);
	if (XST_SUCCESS != status) {
		goto done;
	}
	status = PmProcFsm(proc, PM_PROC_EVENT_SELF_SUSPEND);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if (NULL != master->evalState) {
		status = master->evalState(state);
	}

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmRequestSuspend() - Requested suspend by a PU for another PU
 * @master  PU from which the request is initiated
 * @node    PU node to be suspended
 * @ack     Acknowledge request
 * @latency Desired wakeup latency
 * @state   Desired power state
 *
 * If suspend has been successfully requested, the requested PU needs to
 * initiate its own self suspend. Remember to acknowledge to the requestor
 * after:
 * 1. PU's last awake processor goes to sleep (self suspend completed),
 * 2. PU/processor aborts suspend,
 * 3. PU/processor does not respond to the request (timeout) - not supported
 */
static void PmRequestSuspend(const PmMaster *const master,
			     const u32 node,
			     const u32 ack,
			     const u32 latency,
			     const u32 state)
{
	s32 status = XST_SUCCESS;
	PmMaster* target = NULL;

	PmInfo("%s> RequestSuspend(%lu, %lu, %lu, %lu)\r\n", master->name, node,
	       ack, latency, state);

	/* Only these two acknowledges are allowed for request suspend */
	if ((REQUEST_ACK_NO != ack) && (REQUEST_ACK_NON_BLOCKING != ack)) {
		PmLog(PM_ERRNO_INVALID_ACK, ack, master->name);
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check whether the target is placeholder in PU */
	target = PmMasterGetPlaceholder((PmNodeId)node);

	if (NULL == target) {
		PmLog(PM_ERRNO_INVALID_NODE, node, master->name);
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (false == PmCanRequestSuspend(master, target)) {
		PmLog(PM_ERRNO_NO_PERMISSION, 0U, master->name);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (true == PmIsRequestedToSuspend(target)) {
		status = XST_PM_DOUBLE_REQ;
		goto done;
	}

	/* If target master cannot receive callback return failure */
	if (false == PmMasterCanReceiveCb(target)) {
		status = XST_FAILURE;
		goto done;
	}

	/* Remember request info and init suspend */
	target->suspendRequest.initiator = master;
	target->suspendRequest.acknowledge = ack;
	PmInitSuspendCb(target, SUSPEND_REASON_PU_REQ, latency, state, 0U);

done:
	if (XST_SUCCESS != status) {
		/* Something went wrong, acknowledge immediately */
		PmProcessAckRequest(ack, master, (PmNodeId)node, (u32)status, 0U);
	}
}

/**
 * PmForcePowerdown() - Powerdown a PU or domain forcefully
 * @master  Master who initiated the request
 * @node    Processor, subsystem or domain node to be powered down
 * @ack     Acknowledge request
 *
 * @note    The affected PUs are not notified about the upcoming powerdown,
 *          and PMU does not wait for their WFI interrupt.
 *          Admissible nodes are :
 *          1. Processor nodes (RPU0..1, APU0..3, and in future: PL Procs)
 *          2. Parent nodes (APU, RPU, FPD, and in future PL)
 */
static void PmForcePowerdown(const PmMaster *const master,
			     const u32 node,
			     const u32 ack)
{
	u32 oppoint = 0U;
	s32 status;
	PmNode* nodePtr = PmGetNodeById(node);
	PmPower *power;

	if ((NULL == nodePtr) || INVALID_ACK_ARG(ack)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (NODE_IS_POWER(nodePtr)) {
		power = (PmPower*)nodePtr->derived;
	} else if (NODE_IS_PROC(nodePtr)) {
		PmProc* proc = (PmProc*)nodePtr->derived;
		power = (PmPower*)proc->node.parent;
		/* Master can't force off its proc. */
		if (proc->master->nid == master->nid) {
			status = XST_PM_NO_ACCESS;
			goto done;
		}
	} else {
		/* Slaves and PLLs can not be force power down */
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (false == PmMasterCanForceDown(master, power)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	status = PmNodeForceDown(nodePtr);
	oppoint = nodePtr->currState;

done:
	PmProcessAckRequest(ack, master, (PmNodeId)node, (u32)status, oppoint);
}

/**
 * PmAbortSuspend() - Abort previously requested suspend
 * @master  Master who initiated the request
 * @reason  Reason of aborting suspend
 * @node    Node ID of processor node to abort suspend for
 *
 * @note    Only processor within the master can initiate its own abortion of
 *          suspend.
 */
static void PmAbortSuspend(const PmMaster *const master,
			   const u32 reason,
			   const u32 node)
{
	s32 status;
	PmProc* proc = PmGetProcOfThisMaster(master, (PmNodeId)node);

	PmInfo("%s> AbortSuspend(%lu, %lu)\r\n", master->name, node, reason);

	if (NULL == proc) {
		PmLog(PM_ERRNO_INVALID_NODE, node, master->name);
		status = XST_PM_INVALID_NODE;
		goto done;
	}

	if ((reason < ABORT_REASON_MIN) || (reason > ABORT_REASON_MAX)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	status = PmProcFsm(proc, PM_PROC_EVENT_ABORT_SUSPEND);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

static void PmProbeRpuState(void);

/**
 * PmRequestWakeup() - Power-up processor or subsystem
 * @master  Master who initiated the request
 * @node    Processor or subsystem node to be powered up
 * @ack     Acknowledge request
 */
static void PmRequestWakeup(const PmMaster *const master, const u32 node,
			    const u32 setAddress, const u64 address,
			    const u32 ack)
{
	s32 status;
	u32 oppoint = 0U;
	PmProc* proc = (PmProc*)PmNodeGetProc(node);

	PmInfo("%s> RequestWakeup(%lu, %lu, %llu, %lu)\r\n", master->name, node,
	       setAddress, address, ack);

	if ((NULL == proc) || (NULL == proc->master)) {
		PmLog(PM_ERRNO_INVALID_NODE, node, master->name);
		status = XST_PM_INVALID_NODE;
		goto done;
	}

	if (INVALID_ACK_ARG(ack)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if ((false == PmMasterCanRequestWake(master, proc->master)) &&
	    (master != proc->master)) {
		PmLog(PM_ERRNO_NO_PERMISSION, 0U, master->name);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (1U == setAddress) {
		(void)proc->saveResumeAddr(proc, address);
	} else {
		if (false == PmProcHasResumeAddr(proc)) {
			PmLog(PM_ERRNO_NO_ADDRESS, 0U, master->name);
			status = XST_INVALID_PARAM;
			goto done;
		}
	}

	if ((NODE_RPU == node) || (NODE_RPU_0 == node) || (NODE_RPU_1 == node)) {
		PmProbeRpuState();
	}

	status = PmMasterWakeProc(proc);
	oppoint = proc->node.currState;

done:
	PmProcessAckRequest(ack, master, (PmNodeId)node, (u32)status, oppoint);
}

/**
 * PmReleaseNode() - Release a slave node
 * @master  Master who initiated the request
 * @node    Node to be released
 *
 * @note    Node to be released must have been requested before
 */
static void PmReleaseNode(const PmMaster *master,
			  const u32 node)
{
	s32 status;
	u32 usage;
	PmRequirement* masterReq;
	PmSlave* slave;

	/* Check if node is slave. If it is, handle request via requirements */
	slave = (PmSlave*)PmNodeGetSlave(node);
	if (NULL == slave) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Get static requirements structure for this master/slave pair */
	masterReq = PmRequirementGet(master, slave);

	if (NULL == masterReq) {
		PmLog(PM_ERRNO_NO_PERMISSION, 0U, master->name);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (!MASTER_REQUESTED_SLAVE(masterReq)) {
		PmLog(PM_ERRNO_NO_REQUEST, node, master->name);
		status = XST_FAILURE;
		goto done;
	}

	/* Release requirements */
	status = PmRequirementRelease(masterReq, RELEASE_ONE);

	usage = PmSlaveGetUsersMask(masterReq->slave);
	if (0U == usage) {
		PmNotifierEvent(&masterReq->slave->node, EVENT_ZERO_USERS);
	}

done:
	PmInfo("%s> ReleaseNode(%lu)\r\n", master->name, node);
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmRequestNode() - Request to use a slave node
 * @master          Master who initiated the request
 * @node            Node requested
 * @capabilities    Requested capabilities
 * @qos             Requested quality of service - Not supported
 * @ack             Acknowledge request
 */
static void PmRequestNode(const PmMaster *master,
			  const u32 node,
			  const u32 capabilities,
			  const u32 qos,
			  const u32 ack)
{
	s32 status;
	u32 oppoint = 0U;
	PmRequirement* masterReq;
	PmSlave* slave;

	PmInfo("%s> RequestNode(%lu, %lu, %lu, %lu)\r\n", master->name, node,
	       capabilities, qos, ack);

	/* Check if node is slave. If it is, handle request via requirements */
	slave = (PmSlave*)PmNodeGetSlave(node);
	if ((NULL == slave) || INVALID_ACK_ARG(ack)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Each legal master/slave pair will have one static PmRequirement data
	 * structure. Retrieve the pointer to the structure in order to set the
	 * requested capabilities and mark slave as used by this master.
	 */
	masterReq = PmRequirementGet(master, slave);

	if (NULL == masterReq) {
		/* Master is not allowed to use the slave with given node */
		PmLog(PM_ERRNO_NO_PERMISSION, 0U, master->name);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (MASTER_REQUESTED_SLAVE(masterReq)) {
		PmLog(PM_ERRNO_DOUBLE_REQUEST, node, master->name);
		status = XST_PM_DOUBLE_REQ;
		goto done;
	}

	status = PmSlaveVerifyRequest(masterReq->slave);
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Set requested capabilities if they are valid */
	status = PmRequirementRequest(masterReq, capabilities);

done:
	PmProcessAckRequest(ack, master, (PmNodeId)node, (u32)status, oppoint);
}

/**
 * PmSetRequirement() - Setting requement for a slave
 * @master          Master who initiated the request
 * @node            Node whose requirements setting is requested
 * @capabilities    Requested capabilities
 * @qos             Requested quality of service - Not supported
 * @ack             Acknowledge request
 *
 * @note            If processor which initiated request is in suspending state,
 *                  requirement will be set once PMU handles processor's WFI
 *                  interrupt. If processor is active, setting is done
 *                  immediately (if possible).
 */
static void PmSetRequirement(const PmMaster *master,
			     const u32 node,
			     const u32 capabilities,
			     const u32 qos,
			     const u32 ack)
{
	s32 status;
	u32 oppoint = 0U;
	u32 caps = capabilities;
	PmRequirement* masterReq;
	PmSlave* slave = (PmSlave*)PmNodeGetSlave(node);

	PmInfo("%s> SetRequirement(%lu, %lu, %lu, %lu)\r\n", master->name, node,
	       capabilities, qos, ack);

	/* Set requirement call applies only to slaves */
	if ((NULL == slave) || INVALID_ACK_ARG(ack)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Is there a provision for the master to use the given slave node */
	masterReq = PmRequirementGet(master, slave);
	if (NULL == masterReq) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Check if master has previously requested the node */
	if (!MASTER_REQUESTED_SLAVE(masterReq)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* If slave is set as wake source add the PM_CAP_WAKEUP flag */
	if (0U != (PM_MASTER_WAKEUP_REQ_MASK & masterReq->info)) {
		caps |= PM_CAP_WAKEUP;
	}

	/* Master is using slave (previously has requested node) */
	if (true == PmMasterIsSuspending(master)) {
		/* Schedule setting the requirement */
		status = PmRequirementSchedule(masterReq, caps);
	} else {
		/* Set capabilities now - if they are valid */
		status = PmRequirementUpdate(masterReq, caps);
	}
	oppoint = masterReq->slave->node.currState;

done:
	PmProcessAckRequest(ack, master, (PmNodeId)node, (u32)status, oppoint);
}

/**
 * PmGetApiVersion() - Provides API version number to the caller
 * @master  Master who initiated the request
 */
static void PmGetApiVersion(const PmMaster *const master)
{
	u32 version = ((u32)PM_VERSION_MAJOR << 16U) | (u32)PM_VERSION_MINOR;

	PmInfo("%s> GetApiVersion %d.%d\r\n", master->name, PM_VERSION_MAJOR,
	       PM_VERSION_MINOR);

	IPI_RESPONSE2(master->ipiMask, XST_SUCCESS, version);
}

/**
 * PmResetAssert() - Configure reset line
 * @master      Initiator of the request
 * @reset       ID of reset to be configured
 * @action      Specifies the action (assert, release, pulse)
 */
void PmResetAssert(const PmMaster *const master, const u32 reset,
		   const u32 action)
{
	s32 status;
	const PmReset *resetPtr = PmGetResetById(reset);

	PmInfo("%s> ResetAssert(%lu, %lu)\r\n", master->name, reset, action);

	if (NULL == resetPtr) {
		PmLog(PM_ERRNO_INVALID_RESET, reset, master->name);
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check whether the master has access to this reset line */
	if (false == PmResetMasterHasAccess(master, resetPtr)) {
		PmLog(PM_ERRNO_NO_RESET_PERMISSION, reset, master->name);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	status = PmResetDoAssert(resetPtr, action);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmResetGetStatus() - Get status of the reset
 * @master  Initiator of the request
 * @reset   Reset whose status should be returned
 */
static void PmResetGetStatus(const PmMaster *const master, const u32 reset)
{
	u32 resetStatus = 0U;
	s32 status = XST_SUCCESS;
	const PmReset *resetPtr = PmGetResetById(reset);

	PmInfo("%s> ResetGetStatus(%lu)\r\n", master->name, reset);

	if (NULL == resetPtr) {
		PmLog(PM_ERRNO_INVALID_RESET, reset, master->name);
		status = XST_INVALID_PARAM;
		goto done;
	}

	status = (s32)PmResetGetStatusInt(resetPtr, &resetStatus);

done:
	IPI_RESPONSE2(master->ipiMask, (u32)status, resetStatus);
}

/**
 * PmMmioWrite() - Perform write to protected mmio
 * @master  Master who initiated the request
 * @address Address to write to
 * @mask    Mask to apply
 * @value   Value to write
 *
 * @note    This function provides access to PM-related control registers
 *          that may not be directly accessible by a particular PU.
 */
static void PmMmioWrite(const PmMaster *const master, const u32 address,
			const u32 mask, u32 value)
{
	s32 status = XST_SUCCESS;

	/* no bits to be updated */
	if (0U == mask) {
		goto done;
	}

	/* Check access permissions */
	if (false == PmGetMmioAccessWrite(master, address)) {
		PmLog(PM_ERRNO_NO_WR_PERMISSION, address, master->name);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	XPfw_RMW32(address, mask, value);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmMmioRead() - Read value from protected mmio
 * @master  Master who initiated the request
 * @address Address to write to
 *
 * @note    This function provides access to PM-related control registers
 *          that may not be directly accessible by a particular PU.
 */
static void PmMmioRead(const PmMaster *const master, const u32 address)
{
	s32 status;
	u32 value = 0;

	/* Check access permissions */
	if (false == PmGetMmioAccessRead(master, address)) {
		PmLog(PM_ERRNO_NO_RD_PERMISSION, address, master->name);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	value = XPfw_Read32(address);
	status = XST_SUCCESS;

done:
	IPI_RESPONSE2(master->ipiMask, (u32)status, value);
}

#if defined (ENABLE_WDT) &&	\
	((XPFW_CFG_PMU_FPGA_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT) ||	\
	 (XPFW_CFG_PMU_SHA3_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT) || 	\
	 (XPFW_CFG_PMU_RSA_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT) ||	\
	 (XPFW_CFG_PMU_AES_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT) ||	\
	 (XPFW_CFG_PMU_SECURE_IMG_LOAD_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT))

/* Notification IDs used to inform R5 about STL status
 *
 * STL will not run between notifications PM_NOTIFY_STL_NO_OP_ENTER &
 * PM_NOTIFY_STL_NO_OP_EXIT
 *
 * PM_NOTIFY_STL_NO_OP_EXIT  : STL is restored
 * PM_NOTIFY_STL_NO_OP_ENTER : STL is stopped
 */
typedef enum {
	PM_NOTIFY_STL_NO_OP_EXIT,
	PM_NOTIFY_STL_NO_OP_ENTER
} PmR5StlNoOpNotification;

/**
 * PmNotifyR5AndModifyWdtTimeout() - Notify R5 and modify watchdog timeout.
 *
 * Timeout: Watchdog Timeout value in ms
 * Notification : Notification parameter
 *
 * @return  None
 */
static void PmNotifyR5AndModifyWdtTimeout(u32 Timeout,
		PmR5StlNoOpNotification Notification)
{
	IPI_REQUEST2(IPI_PMU_0_IER_RPU_0_MASK, PM_NOTIFY_STL_NO_OP, Notification);
	if (XST_SUCCESS != XPfw_IpiTrigger(IPI_PMU_0_IER_RPU_0_MASK)) {
		PmWarn("Error in IPI trigger\r\n");
	}

	XPfw_WdtSetVal(Timeout);
}

#endif

#ifdef ENABLE_FPGA_LOAD
/**
 * Pmfpgaload() - Load the bitstream into the PL.
 * This function does the calls the necessary PCAP interfaces based on flags.
 *
 * AddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *         will read the data to be written to PCAP interface
 *
 * AddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *         will read the data to be written to PCAP interface
 *
 * WrSize: Number of 32bit words that the DMA should write to
 *        the PCAP interface
 *
 * @return  error status based on implemented functionality (SUCCESS by default)
 */
static void PmFpgaLoad(const PmMaster *const master,
			const u32 AddrHigh, const u32 AddrLow,
			const u32 KeyAddr, const u32 Flags)
{
	u32 Status;
	XFpga XFpgaInstance = {0U};
	UINTPTR BitStreamAddr = ((u64)AddrHigh << 32)|AddrLow;

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_FPGA_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_FPGA_WDT_TIMEOUT,
			PM_NOTIFY_STL_NO_OP_ENTER);
#endif

	Status = XFpga_Initialize(&XFpgaInstance);
	if (XST_SUCCESS != (s32)Status) {
		goto done;
	}

	if (Flags & XFPGA_ENCRYPTION_USERKEY_EN) {
		Status = XFpga_BitStream_Load(&XFpgaInstance, BitStreamAddr,
						 KeyAddr, 0U, Flags);
	} else {
		Status = XFpga_BitStream_Load(&XFpgaInstance, BitStreamAddr,
						 0U, KeyAddr, Flags);
	}

    if ((XFpgaInstance.PLInfo.SecureOcmState == (u32)TRUE) &&
			((Flags & XFPGA_AUTHENTICATION_OCM_EN) ==
					XFPGA_AUTHENTICATION_OCM_EN)) {
	FSBL_Store_Restore_Info.OcmAndFsblInfo |= XPFW_OCM_IS_USED;
    }
#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_FPGA_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_EXIT);
#endif
  done:
       IPI_RESPONSE1(master->ipiMask, (u32)Status);
}

/**
 * PmFpgaGetStatus() - Get status of the PL-block
 * @master  Initiator of the request
 */
static void PmFpgaGetStatus(const PmMaster *const master)
{
	u32 Status;
	u32 Value = 0;
	XFpga XFpgaInstance = {0U};

	Status = XFpga_Initialize(&XFpgaInstance);
	if (XST_SUCCESS != (s32)Status) {
		goto done;
	}

	Value = XFpga_InterfaceStatus(&XFpgaInstance);
	if (Value == XFPGA_INVALID_INTERFACE_STATUS) {
		Status = XST_FAILURE;
	}

 done:
	IPI_RESPONSE2(master->ipiMask, (u32)Status, Value);
}

#if defined(ENABLE_FPGA_READ_CONFIG_DATA) || defined(ENABLE_FPGA_READ_CONFIG_REG)
/**
 * PmFpgaRead() - Perform the FPGA configuration Read back
 *
 * Reg_Numframes: Configuration register offset (or) Number of frames to read
 * AddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *         will read/write the data to the PCAP interface
 * AddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *         will read/write the data to the PCAP interface
 * Readback_Type: Type of FPGA Read back operation
 *		0 - Configuration Register Read back
 *		1 - Configuration Data Read back
 *
 * @return error status based on implemented functionality(SUCCESS by default)
 */
static void PmFpgaRead(const PmMaster *const master,
		       const u32 Reg_Numframes, const u32 AddrLow,
		       const u32 AddrHigh, u32 Readback_Type)
{
	u32 Status;
	u32 Value = 0U;
	XFpga XFpgaInstance = {0U};
	UINTPTR Address = ((u64)AddrHigh << 32U)|AddrLow;

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_FPGA_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_FPGA_WDT_TIMEOUT,
			PM_NOTIFY_STL_NO_OP_ENTER);
#endif

	Status = XFpga_Initialize(&XFpgaInstance);
	if (XST_SUCCESS != (s32)Status) {
		goto done;
	}

	if (Readback_Type != 0U) {
#if defined(ENABLE_FPGA_READ_CONFIG_DATA)
		Status = XFpga_GetPlConfigData(&XFpgaInstance, Address, Reg_Numframes);
#else
		PmWarn("Unsupported EEMI API\r\n");
		Status = XST_NO_ACCESS;
#endif

	} else {
#if defined(ENABLE_FPGA_READ_CONFIG_REG)
		Status = XFpga_GetPlConfigReg(&XFpgaInstance, Address, Reg_Numframes);
		Value = *(UINTPTR *)Address;
#else
		PmWarn("Unsupported EEMI API\r\n");
		Status = XST_NO_ACCESS;
#endif
	}

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_FPGA_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_EXIT);
#endif

 done:
	IPI_RESPONSE2(master->ipiMask, (u32)Status, Value);
}
#endif
#endif

#ifdef ENABLE_SECURE
/**
 * PmSecureSha() - To calculate the SHA3 hash on the provided data.
 *
 * @SrcAddrHigh: Higher 32-bit Linear memory space from where data
 *         will be read.
 *
 * @SrcAddrLow: Lower 32-bit Linear memory space from where data
 *         will be read.
 *
 * @SrcSize: Number of bytes of data on which hash should be calculated
 *
 * @Flags: provides inputs for operation to be performed
 *
 * @return  error status based on implemented functionality(SUCCESS by default)
 */
static void PmSecureSha(const PmMaster *const master,
			const u32 SrcAddrHigh, const u32 SrcAddrLow,
			const u32 SrcSize, const u32 Flags)
{
	u32 Status;

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_SHA3_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_SHA3_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_ENTER);
#endif

	Status = XSecure_Sha3Hash(SrcAddrHigh, SrcAddrLow,
			SrcSize, Flags);
#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_SHA3_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_EXIT);
#endif
	IPI_RESPONSE1(master->ipiMask, (u32)Status);
}

/**
 * PmSecureRsa() - To encrypt or decrypt the data with provided public or
 * private kley components by using RSA core.
 *
 * @SrcAddrHigh: Higher 32-bit Linear memory space from where data
 *         will be read.
 *
 * @SrcAddrLow: Lower 32-bit Linear memory space from where data
 *         will be read.
 *
 * @SrcSize: Number of bytes of data.
 *
 * @Flags: provides inputs for operation to be performed
 *
 * @return  error status based on implemented functionality(SUCCESS by default)
 */
static void PmSecureRsa(const PmMaster *const master,
			const u32 SrcAddrHigh, const u32 SrcAddrLow,
			const u32 SrcSize, const u32 Flags)
{
	u32 Status;

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_RSA_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_RSA_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_ENTER);
#endif

	Status = XSecure_RsaCore(SrcAddrHigh, SrcAddrLow,
			SrcSize, Flags);

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_RSA_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_EXIT);
#endif

	IPI_RESPONSE1(master->ipiMask, (u32)Status);
}

/**
 * PmSecureAes() - To encrypt or decrypt the data with the provided
 *  key (Device/KUP/PUF keys).
 *
 * @SrcAddrHigh: Higher 32-bit address of the XSecure_AesParams structure
 *         	from where data addresses will be read.
 *
 * @SrcAddrLow: Lower 32-bit address of the XSecure_AesParams structure
 *         	from where data addresses will be read.
 *
 * @return  error status based on implemented functionality(SUCCESS by default)
 */
static void PmSecureAes(const PmMaster *const master,
			const u32 SrcAddrHigh, const u32 SrcAddrLow)
{
	u32 Status = XST_SUCCESS;
#ifndef XSK_ACCESS_PUF_USER_EFUSE
	XilSKey_Puf InstancePtr;
#endif
	u64 WrAddr = ((u64)SrcAddrHigh << 32U) | SrcAddrLow;
	XSecure_AesParams *Aes = (XSecure_AesParams *)(UINTPTR)WrAddr;

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_AES_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_AES_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_ENTER);
#endif

	if (Aes->KeySrc == AES_PUF_KEY_SEL_MASK) {
		#ifndef XSK_ACCESS_PUF_USER_EFUSE
			Status = XilSKey_Puf_Regeneration(&InstancePtr);
			if (Status != 0U) {
				goto END;
			}
		#else
			Status = XST_NOT_ENABLED;
			goto END;
		#endif
	}

	Status = XSecure_AesOperation(SrcAddrHigh, SrcAddrLow);

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_AES_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_EXIT);
#endif

END:
	IPI_RESPONSE2(master->ipiMask, XST_SUCCESS, (u32)Status);
}

/**
 * PmSecureImage() - To process secure image
 *
 * @SrcAddrHigh: Higher 32-bit Linear memory space from where data
 *         will be read.
 *
 * @SrcAddrLow: Lower 32-bit Linear memory space from where data
 *         will be read.
 *
 * @KupAddrHigh: Higher 32-bit Linear memory space from where data
 *         will be read.
 *
 * @KupAddrLow: Lower 32-bit Linear memory space from where data
 *         will be read.
 *
 *
 * @return  error status based on implemented functionality(SUCCESS by default)
 */
static void PmSecureImage(const PmMaster *const master,
			const u32 SrcAddrHigh, const u32 SrcAddrLow, const u32 KupAddrHigh, const u32 KupAddrLow)
{
	u32 Status;
	XSecure_DataAddr Addr = {0U};

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_SECURE_IMG_LOAD_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_SECURE_IMG_LOAD_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_ENTER);
#endif

	Status = XSecure_SecureImage(SrcAddrHigh, SrcAddrLow, KupAddrHigh, KupAddrLow, &Addr);

#if defined (ENABLE_WDT) &&	\
	(XPFW_CFG_PMU_SECURE_IMG_LOAD_WDT_TIMEOUT > XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT)
	PmNotifyR5AndModifyWdtTimeout(XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT,
				PM_NOTIFY_STL_NO_OP_EXIT);
#endif

	if (Status != 0x0U) {
		 PmErr("Failed image loading  with error : %x\r\n", Status);
	}

	IPI_RESPONSE3(master->ipiMask, (u32)Status, Addr.AddrHigh, Addr.AddrLow);
}
#endif

/**
 * This function provides access to efuse memory
 *
 * @AddrHigh: Higher 32-bit address of the XilSKey_Efuse structure.
 * @AddrLow: Lower 32-bit address of the XilSKey_Efuse structure.
 *
 * @return  error status based on implemented functionality(SUCCESS by default)
 */
static void PmEfuseAccess(const PmMaster *const master,
			const u32 AddrHigh, const u32 AddrLow)
{
	u32 Status;

#ifdef EFUSE_ACCESS
	Status = XilSkey_ZynqMpEfuseAccess(AddrHigh, AddrLow);
#else
	Status = XST_NOT_ENABLED;
#endif

	IPI_RESPONSE2(master->ipiMask, XST_SUCCESS, (u32)Status);
}

/**
 * PmGetChipid() - Get silicon version register
 */
static void PmGetChipid(const PmMaster *const master)
{
	u32 idcode = XPfw_Read32(CSU_IDCODE);
	u32 version = XPfw_Read32(CSU_VERSION);
	u32 pl_init = XPfw_Read32(CSU_PCAP_STATUS_REG);
	u32 efuse_ipdisable = XPfw_Read32(EFUSE_IPDISABLE);

	efuse_ipdisable &= EFUSE_IPDISABLE_VERSION;
	pl_init &= CSU_PCAP_STATUS_PL_INIT_MASK_VAL;

	version |= efuse_ipdisable << CSU_VERSION_EMPTY_SHIFT;
	version |= pl_init <<
			(CSU_VERSION_PL_STATE_SHIFT - CSU_PCAP_STATUS_PL_INIT_SHIFT_VAL);

	IPI_RESPONSE3(master->ipiMask, XST_SUCCESS, idcode, version);
}

/**
 * PmSetWakeupSource() - Master requests to be woken-up by the slaves interrupt
 * @master      Initiator of the request
 * @targetNode  Master node to be woken-up (currently must be same as initiator)
 * @sourceNode  Source of the wake-up (slave that generates interrupt)
 * @enable      Flag stating should event be enabled or disabled
 *
 * @note        GIC wake interrupt is automatically enabled when a processor
 *              goes to sleep.
 */
static void PmSetWakeupSource(const PmMaster *const master,
			      const u32 targetNode,
			      const u32 sourceNode,
			      const u32 enable)
{
	s32 status = XST_SUCCESS;
	PmRequirement* req;
	PmSlave* slave = (PmSlave*)PmNodeGetSlave(sourceNode);

	/* Check if given target node is valid */
	if ((targetNode != master->nid) &&
	    (NULL == PmGetProcOfThisMaster(master, (PmNodeId)targetNode))) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* The call applies only to slave nodes */
	if (NULL == slave) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	req = PmRequirementGet(master, slave);
	/* Is master allowed to use resource (slave)? */
	if (NULL == req) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Check whether the slave has wake-up capability */
	status = PmSlaveHasWakeUpCap(req->slave);
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Set/clear request info according to the enable flag */
	if (0U == enable) {
		req->info &= ~(u8)PM_MASTER_WAKEUP_REQ_MASK;
	} else if (1U == enable) {
		req->info |= PM_MASTER_WAKEUP_REQ_MASK;
	} else {
		status = XST_INVALID_PARAM;
		goto done;
	}
	if (NULL != slave->wake) {
		slave->wake->class->set(slave->wake, master->ipiMask, enable);
	}

done:
	PmInfo("%s> SetWakeupSource(%lu, %lu, %lu)\r\n", master->name,
	       targetNode, sourceNode, enable);
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmSystemShutdown() - Request system shutdown or restart
 * @master  Master requesting system shutdown
 * @type    Shutdown type
 * @subtype Shutdown subtype
 */
static void PmSystemShutdown(PmMaster* const master, const u32 type,
			     const u32 subtype)
{
	s32 status = XST_SUCCESS;

	PmInfo("%s> SystemShutdown(%lu, %lu)\r\n", master->name, type, subtype);

	/* Check whether master has permission for system shutdown/restart or not,
	 * If master has permission for system shutdown/restart then allow below
	 * list of operations, else return XST_PM_NO_ACCESS as error.
	 * 	1> System Restart/Shutdown
	 *	2> PS-only Restart
	 */
	if ((PMF_SHUTDOWN_TYPE_SHUTDOWN == type) ||
	    ((PMF_SHUTDOWN_TYPE_RESET == type) &&
	    ((PMF_SHUTDOWN_SUBTYPE_PS_ONLY == subtype) ||
	     (PMF_SHUTDOWN_SUBTYPE_SYSTEM == subtype)))) {
		if (false == PmMasterCanSysShutdown(master)) {
			status = XST_PM_NO_ACCESS;
			goto done;
		}
	}

	/* For shutdown type the subtype is irrelevant: shut the caller down */
	if (PMF_SHUTDOWN_TYPE_SHUTDOWN == type) {
		status = PmMasterFsm(master, PM_MASTER_EVENT_FORCE_DOWN);

#if defined(BOARD_SHUTDOWN_PIN) && defined(BOARD_SHUTDOWN_PIN_STATE)
		if (PMF_SHUTDOWN_SUBTYPE_SYSTEM == subtype) {
			PmKillBoardPower();
		}
#endif
		goto done;
	}

	if (PMF_SHUTDOWN_TYPE_RESET != type) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Now distinguish the restart scope depending on the subtype */
	switch (subtype) {
	case PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM:
		status = PmMasterRestart(master);
		break;
	case PMF_SHUTDOWN_SUBTYPE_PS_ONLY:
		XPfw_ResetPsOnly();
		break;
	case PMF_SHUTDOWN_SUBTYPE_SYSTEM:
		XPfw_ResetSystem();
		break;
	default:
		PmLog(PM_ERRNO_INVALID_SUBTYPE, subtype, master->name);
		status = XST_INVALID_PARAM;
		break;
	}

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmSetMaxLatency() - set maximum allowed latency for the node
 * @master  Initiator of the request who must previously requested the node
 * @node    Node whose latency, and consequently deepest possible state, is
 *          specified
 * @latency Maximum allowed latency
 */
static void PmSetMaxLatency(const PmMaster *const master, const u32 node,
			    const u32 latency)
{
	s32 status = XST_SUCCESS;
	PmRequirement* masterReq;
	PmSlave* slave = (PmSlave*)PmNodeGetSlave(node);

	PmInfo("%s> SetMaxLatency(%lu, %lu)\r\n", master->name, node, latency);

	if (NULL == slave) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	masterReq = PmRequirementGet(master, slave);
	/* Check if the master can use given slave node */
	if (NULL == masterReq) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Check if master has previously requested the node */
	if (!MASTER_REQUESTED_SLAVE(masterReq)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	masterReq->latencyReq = latency;
	masterReq->info |= PM_MASTER_SET_LATENCY_REQ;
	status = PmUpdateSlave(masterReq->slave);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmSetConfiguration() - Load the configuration
 * @master  Master who initiated the loading of configuration
 * @address Address at which the configuration object is placed
 */
static void PmSetConfiguration(const PmMaster *const master, const u32 address)
{
	s32 status;
	u32 configAddr = address;
	u32 callerIpiMask = master->ipiMask;

	PmInfo("%s> SetConfig(0x%lx)\r\n", master->name, address);

	if (NULL != master->remapAddr) {
		configAddr = master->remapAddr(address);
	}

	status = PmConfigLoadObject(configAddr, callerIpiMask);
	/*
	 * Respond using the saved IPI mask of the caller (master's IPI mask
	 * may change after setting the configuration)
	 */
	IPI_RESPONSE1(callerIpiMask, (u32)status);
}

/**
 * PmProcRpuForceDownFix() - Fix the state of RPU processor to be forced down
 * @proc	RPU processor
 */
static void PmProcRpuForceDownFix(PmProc* const proc)
{
	if (NULL != proc->node.parent) {
		if (0U != (NODE_LOCKED_POWER_FLAG & proc->node.flags)) {
			PmPowerReleaseParent(&proc->node);
		}
	}
	if (NULL != proc->node.clocks) {
		if (0U != (NODE_LOCKED_CLOCK_FLAG & proc->node.flags)) {
			PmClockRelease(&proc->node);
		}
	}
	proc->node.currState = PM_PROC_STATE_FORCEDOFF;
	proc->master->state = PM_MASTER_STATE_KILLED;
}

/**
 * PmProbeRpuState() - Probe and update the state of RPU
 *
 * @note	The probe is performed only once
 */
static void PmProbeRpuState(void)
{
	static bool probed = false;
	u32 halt0 = XPfw_Read32(RPU_RPU_0_CFG);
	u32 reset = XPfw_Read32(CRL_APB_RST_LPD_TOP);
	u32 mode = XPfw_Read32(RPU_RPU_GLBL_CNTL);

	if (true == probed) {
		goto done;
	}

	/* If reset is asserted or (deasserted and RPU_0 is halted) */
	if ((0U != (reset & CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK)) ||
	   ((0U == (reset & CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK)) &&
	    (0U == (halt0 & RPU_RPU_0_CFG_NCPUHALT_MASK)))) {
		PmProcRpuForceDownFix(&pmProcRpu0_g);
	}

	/* If RPU lockstep mode is configured in hardware */
	if (0U == (mode & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK)) {
		if (NULL != pmProcRpu1_g.master) {
			PmErr("expected split mode, found lockstep\r\n");
			goto done;
		}
		pmProcRpu1_g.node.currState = PM_PROC_STATE_FORCEDOFF;
	} else {
		u32 halt1 = XPfw_Read32(RPU_RPU_1_CFG);

		/* If reset is asserted or (deasserted and RPU_1 is halted) */
		if ((0U != (reset & CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK)) ||
		   ((0U == (reset & CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK)) &&
		    (0U == (halt1 & RPU_RPU_1_CFG_NCPUHALT_MASK)))) {
			if (NULL == pmProcRpu1_g.master) {
				PmErr("expected lockstep, found split mode\r\n");
				goto done;
			}
			PmProcRpuForceDownFix(&pmProcRpu1_g);
		}
	}
	probed = true;

done:
	return;
}

/**
 * PmGetNodeStatus() - Get the status of the node
 * @master  Initiator of the request
 * @node    Node whose status should be returned
 */
static void PmGetNodeStatus(const PmMaster *const master, const u32 node)
{
	u32 oppoint = 0U;
	u32 currReq = 0U;
	u32 usage = 0U;
	s32 status = XST_SUCCESS;
	PmNode* nodePtr = PmGetNodeById(node);

	PmInfo("%s> GetNodeStatus(%lu)\r\n", master->name, node);

	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if ((NODE_RPU == node) || (NODE_RPU_0 == node) || (NODE_RPU_1 == node)) {
		PmProbeRpuState();
	}

	oppoint = nodePtr->currState;
	if (NODE_IS_SLAVE(nodePtr)) {
		PmSlave* const slave = (PmSlave*)nodePtr->derived;

		currReq = PmSlaveGetRequirements(slave, master);
		usage = PmSlaveGetUsageStatus(slave, master);
	}

done:
	IPI_RESPONSE4(master->ipiMask, (u32)status, oppoint, currReq, usage);
}

/**
 * PmGetOpCharacteristics() - Get operating characteristics of a node
 * @master  Initiator of the request
 * @node    Node in question
 * @type    Type of the operating characteristics
 *          power, temperature and latency
 */
static void PmGetOpCharacteristics(const PmMaster *const master, const u32 node,
				   const u32 type)
{
	u32 result = 0U;
	s32 status = XST_SUCCESS;
	PmNode* nodePtr = PmGetNodeById(node);

	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	switch(type) {
	case PM_OPCHAR_TYPE_POWER:
		if (NULL == nodePtr->class->getPowerData) {
			status = XST_NO_FEATURE;
			goto done;
		}
		status = nodePtr->class->getPowerData(nodePtr, &result);
		break;
	case PM_OPCHAR_TYPE_TEMP:
		PmLog(PM_ERRNO_NO_TEMP_SUPPORT, node, master->name);
		status = XST_NO_FEATURE;
		break;
	case PM_OPCHAR_TYPE_LATENCY:
		if (NULL == nodePtr->class->getWakeUpLatency) {
			status = XST_NO_FEATURE;
			goto done;
		}
		status = nodePtr->class->getWakeUpLatency(nodePtr, &result);
		break;
	default:
		PmLog(PM_ERRNO_INVALID_TYPE, type, master->name);
		status = XST_INVALID_PARAM;
		break;
	}

done:
	PmInfo("%s> PmGetOpChar(%lu, %lu, %lu)\r\n", master->name, node, type,
	       result);
	IPI_RESPONSE2(master->ipiMask, (u32)status, result);
}

/**
 * PmRegisterNotifier() - Register a master to be notified about the event
 * @master  Master to be notified
 * @node    Node to which the event is related
 * @event   Event in question
 * @wake    Wake master upon capturing the event if value 1, do not wake if 0
 * @enable  Enable the registration for value 1, disable for value 0
 */
static void PmRegisterNotifier(const PmMaster *const master, const u32 node,
			       const u32 event, const u32 wake,
			       const u32 enable)
{
	s32 status;
	PmNode* nodePtr = PmGetNodeById(node);

	PmInfo("%s> RegisterNotifier(%lu, %lu, %lu, %lu)\r\n", master->name,
	       node, event, wake, enable);

	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	if (((0U != wake) && (1U != wake)) || ((0U != enable) && (1U != enable)) ||
	    ((EVENT_STATE_CHANGE != event) && (EVENT_ZERO_USERS != event))) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	if (0U == enable) {
		PmNotifierUnregister(master, nodePtr, event);
		status = XST_SUCCESS;
	} else {
		status = PmNotifierRegister(master, nodePtr, event, wake);
	}

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmInitFinalize() - Notification from a master that it has initialized PM
 * @master  Initiator of the request
 */
static void PmInitFinalize(PmMaster* const master)
{
	s32 status;

	PmInfo("%s> InitFinalize\r\n", master->name);
	status = PmMasterInitFinalize(master);
#ifdef ENABLE_UNUSED_RPU_PWR_DWN
	PmForceDownUnusableRpuCores();
#endif
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmClockSetParent() - Set the clock parent (configure clock's mux)
 * @master		The caller
 * @clockId		ID of the target clock
 * @select		Mux select value
 */
static void PmClockSetParent(PmMaster* const master, const u32 clockId,
		      const u32 select)
{
	PmClock* clockPtr;
	s32 status = XST_SUCCESS;

	PmInfo("%s> ClockSetParent(%lu, %lu)\r\n", master->name, clockId,
	       select);
	clockPtr = PmClockGetById(clockId);
	if (NULL == clockPtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}
#ifndef DISABLE_CLK_PERMS
	status = PmClockCheckPermission(clockPtr, master->ipiMask);
	if (XST_SUCCESS != status) {
		goto done;
	}
#endif
	status = PmClockMuxSetParent(clockPtr, select);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmClockGetParent() - Get the mux select value of the current clock parent
 * @master		The caller
 * @clockId		ID of the target clock
 */
static void PmClockGetParent(PmMaster* const master, const u32 clockId)
{
	PmClock* clockPtr;
	u32 select = 0U;
	s32 status = XST_SUCCESS;

	PmInfo("%s> ClockGetParent(%lu)\r\n", master->name, clockId);
	clockPtr = PmClockGetById(clockId);
	if (NULL == clockPtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	status = PmClockMuxGetParent(clockPtr, &select);

done:
	IPI_RESPONSE2(master->ipiMask, (u32)status, select);
}

/**
 * PmClockGateConfig() - Configure clock gate if master has privileges to do so
 * @master	The caller
 * @clkId	ID of the target clock
 * @enable	1=enable the clock, 0=disable the clock
 */
static void PmClockGateConfig(PmMaster* const master, const u32 clkId, const u8 enable)
{
	PmClock* clockPtr;
	s32 status = XST_SUCCESS;

	PmInfo("%s> ClockGate(%lu, %lu)\r\n", master->name, clkId, enable);
	clockPtr = PmClockGetById(clkId);
	if (NULL == clockPtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}
#ifndef DISABLE_CLK_PERMS
	status = PmClockCheckPermission(clockPtr, master->ipiMask);
	if (XST_SUCCESS != status) {
		goto done;
	}
#endif
	status = PmClockGateSetState(clockPtr, enable);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmClockGetStatus() - Get clock gate status
 * @master	Master that initiated the call
 * @clockId	ID of the clock in question
 */
static void PmClockGetStatus(PmMaster* const master, const u32 clockId)
{
	PmClock* clockPtr;
	s32 status = XST_SUCCESS;
	u8 enable = 0x0U;

	PmInfo("%s> ClockGetStatus(%lu)\r\n", master->name, clockId);
	clockPtr = PmClockGetById(clockId);
	if (NULL == clockPtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	status = PmClockGateGetState(clockPtr, &enable);

done:
	IPI_RESPONSE2(master->ipiMask, (u32)status, enable);
}

/**
 * PmClockSetDivider() - Set divider of the clock
 * @master	Master that initiated the call
 * @clockId	ID of the clock in question
 * @divId	Identifier of the divider value to be set
 * @val		Divider value to be set
 */
static void PmClockSetDivider(PmMaster* const master, const u32 clockId,
		       const u32 divId, const u32 val)
{
	PmClock* clockPtr;
	s32 status = XST_SUCCESS;

	PmInfo("%s> ClockSetDivider(%lu, %lu, %lu)\r\n", master->name, clockId,
	       divId, val);
	clockPtr = PmClockGetById(clockId);
	if ((NULL == clockPtr) || (0U == val) ||
	    INVALID_DIV_ID(divId)) {
		status = XST_INVALID_PARAM;
		goto done;
	}
#ifndef DISABLE_CLK_PERMS
	status = PmClockCheckPermission(clockPtr, master->ipiMask);
	if (XST_SUCCESS != status) {
		goto done;
	}
#endif
	status = PmClockDividerSetVal(clockPtr, divId, val);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmClockGetDivider() - Get currently configured divider value of the clock
 * @master	Master that initiated the call
 * @clockId	ID of the clock in question
 * @divId	ID of the divider
 */
static void PmClockGetDivider(PmMaster* const master, const u32 clockId,
		       const u32 divId)
{
	PmClock* clockPtr;
	s32 status = XST_SUCCESS;
	u32 divVal = 0U;

	PmInfo("%s> ClockGetDivider(%lu)\r\n", master->name, clockId);
	clockPtr = PmClockGetById(clockId);
	if (NULL == clockPtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	status = PmClockDividerGetVal(clockPtr, divId, &divVal);

done:
	IPI_RESPONSE2(master->ipiMask, (u32)status, divVal);
}

/**
 * PmPllSetParam() - Set PLL parameter
 * @master	Master that initiated the call
 * @pllId	PLL node ID
 * @paramId	PLL parameter ID
 * @value	PLL parameter value to set
 */
static void PmPllSetParam(PmMaster* const master, const u32 pllId, const u32 paramId,
		   const u32 value)
{
	s32 status = XST_SUCCESS;
	PmPll* pll = PmNodeGetPll(pllId);

	PmInfo("%s> PllSetParam(%lu, %lu, %lu)\r\n", master->name, pllId,
	       paramId, value);

	if (NULL == pll) {
		status = XST_INVALID_PARAM;
		goto done;
	}
#ifndef DISABLE_CLK_PERMS
	if (0U == (master->ipiMask & PmPllGetPermissions(pll))) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}
#endif
	status = PmPllSetParameterInt(pll, paramId, value);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmPllGetParam() - Get PLL parameter
 * @master	Master that initiated the call
 * @pllId	PLL node ID
 * @paramId	PLL parameter ID
 */
static void PmPllGetParam(PmMaster* const master, const u32 pllId, const u32 paramId)
{
	s32 status = XST_SUCCESS;
	PmPll* pll = PmNodeGetPll(pllId);
	u32 value = 0U;

	PmInfo("%s> PllGetParam(%lu, %lu)\r\n", master->name, pllId, paramId);
	if (NULL == pll) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	status = PmPllGetParameterInt(pll, paramId, &value);

done:
	IPI_RESPONSE2(master->ipiMask, (u32)status, value);
}

/**
 * PmPllSetMode() - Set PLL mode
 * @master	Master that initiated the call
 * @pllId	PLL node ID
 * @mode	PLL mode to set
 */
static void PmPllSetMode(PmMaster* const master, const u32 pllId, const u32 mode)
{
	s32 status = XST_SUCCESS;
	PmPll* pll = PmNodeGetPll(pllId);

	PmInfo("%s> PllSetMode(%lu, %lu, %lu)\r\n", master->name, pllId, mode);

	if (NULL == pll) {
		status = XST_INVALID_PARAM;
		goto done;
	}
#ifndef DISABLE_CLK_PERMS
	if (0U == (master->ipiMask & PmPllGetPermissions(pll))) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}
#endif
	status = PmPllSetModeInt(pll, mode);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmPllGetMode() - Get PLL mode
 * @master	Master that initiated the call
 * @pllId	PLL node ID
 * @mode	PLL mode
 */
static void PmPllGetMode(PmMaster* const master, const u32 pllId)
{
	s32 status = XST_SUCCESS;
	PmPll* pll = PmNodeGetPll(pllId);
	u32 mode = 0U;

	PmInfo("%s> PllGetMode(%lu)\r\n", master->name, pllId);
	if (NULL == pll) {
		status = XST_INVALID_PARAM;
		goto done;
	}
	mode = PmPllGetModeInt(pll);

done:
	IPI_RESPONSE2(master->ipiMask, (u32)status, mode);
}

/**
 * PmPinCtrlRequest() - Request PIN control
 * @master	Master that initiated the call
 * @pinId	ID of the pin
 */
static void PmPinCtrlRequest(PmMaster* const master, const u32 pinId)
{
	s32 status = XST_SUCCESS;

	PmInfo("%s> PmPinCtrlRequest(%lu)\r\n", master->name, pinId);

	status = PmPinCtrlRequestInt(master->ipiMask, pinId);

	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmPinCtrlRelease() - Release PIN control
 * @master	Master that initiated the call
 * @pinId	ID of the pin
 */
static void PmPinCtrlRelease(PmMaster* const master, const u32 pinId)
{
	s32 status = XST_SUCCESS;

	PmInfo("%s> PmPinCtrlRelease(%lu)\r\n", master->name, pinId);

	status = PmPinCtrlReleaseInt(master->ipiMask, pinId);

	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmPinCtrlGetFunction() - Get configured PIN function
 * @master	Master that initiated the call
 * @pinId	ID of the pin
 */
static void PmPinCtrlGetFunction(PmMaster* const master, const u32 pinId)
{
	s32 status;
	u32 fnId = 0U;

	PmInfo("%s> PmPinCtrlGetFunc(%lu)\r\n", master->name, pinId);

	status = PmPinCtrlGetFunctionInt(pinId, &fnId);

	IPI_RESPONSE2(master->ipiMask, (u32)status, fnId);
}

/**
 * PmPinCtrlSetFunction() - Set the PIN function
 * @master	Master that initiated the call
 * @pinId	ID of the pin
 * @fnId	Pin function ID
 */
static void PmPinCtrlSetFunction(PmMaster* const master, const u32 pinId,
			  const u32 fnId)
{
	s32 status = XST_SUCCESS;

	PmInfo("%s> PmPinCtrlSetFunc(%lu, %lu)\r\n", master->name, pinId, fnId);
	status = PmPinCtrlCheckPerms(master->ipiMask, pinId);
	if (XST_SUCCESS != status) {
		goto done;
	}
	status = PmPinCtrlSetFunctionInt(master, pinId, fnId);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

/**
 * PmPinCtrlConfigParamGet() - Get the PIN configuration parameter value
 * @master	Master that initiated the call
 * @pinId	ID of the pin
 * @paramId	Parameter ID
 */
static void PmPinCtrlConfigParamGet(PmMaster* const master, const u32 pinId,
			     const u32 paramId)
{
	s32 status;
	u32 value = 0U;

	PmInfo("%s> PmPinCtrlParamGet(%lu, %lu)\r\n", master->name, pinId,
		paramId);
	status = PmPinCtrlGetParam(pinId, paramId, &value);

	IPI_RESPONSE2(master->ipiMask, (u32)status, value);
}

/**
 * PmPinCtrlConfigParamSet() - Set the PIN configuration parameter value
 * @master	Master that initiated the call
 * @pinId	ID of the pin
 * @paramId	Parameter ID
 * @val		Parameter value to be set
 */
static void PmPinCtrlConfigParamSet(PmMaster* const master, const u32 pinId,
			     const u32 paramId, const u32 val)
{
	s32 status;

	PmInfo("%s> PmPinCtrlParamSet(%lu, %lu, %lu)\r\n", master->name, pinId,
		paramId, val);
	status = PmPinCtrlCheckPerms(master->ipiMask, pinId);
	if (XST_SUCCESS != status) {
		goto done;
	}
	status = PmPinCtrlSetParam(pinId, paramId, val);

done:
	IPI_RESPONSE1(master->ipiMask, (u32)status);
}

#ifdef ENABLE_IOCTL
/**
 * PmDevIoctl() - This function performs driver-like IOCTL functions on
 * shared system devices.
 * @master	Master that initiated the call
 * @deviceId	Node id of the device
 * @ioctlId	The ioctl id to be requested for
 * @arg1	Argument to be passed to the ioctl call
 * @arg2	Argument to be passed to the ioctl call
 */
static void PmDevIoctl(const PmMaster* const master, const u32 deviceId,
		       XPm_IoctlId ioctlId, u32 arg1, u32 arg2)
{
	s32 status = XST_FAILURE;
	u32 value = 0;

	PmInfo("%s> PmDevIoctl(%lu, %lu, %lu, %lu)\r\n", master->name,
		deviceId, ioctlId, arg1, arg2);

	switch (ioctlId) {
	case PM_IOCTL_SET_FEATURE_CONFIG:
		status = PmSetFeatureConfig((XPm_FeatureConfigId)arg1, arg2);
		break;
	case PM_IOCTL_GET_FEATURE_CONFIG:
		status = PmGetFeatureConfig((XPm_FeatureConfigId)arg1, &value);
		break;
	default:
		status = XST_INVALID_PARAM;
		break;
	}

	if (PM_IOCTL_GET_FEATURE_CONFIG == ioctlId) {
		IPI_RESPONSE2(master->ipiMask, (u32)status, value);
	} else {
		IPI_RESPONSE1(master->ipiMask, (u32)status);
	}
}
#endif

/**
 * PmApiApprovalCheck() - Check if the API ID can be processed at the moment
 * @apiId	PM API ID
 *
 * @return	True if API can be processed, false otherwise
 */
static bool PmApiApprovalCheck(const u32 apiId)
{
	bool approved = PmConfigObjectIsLoaded();

	if (true == approved) {
		goto done;
	}

	/* If the object is not loaded only APIs below can be processed */
	switch (apiId) {
	case PM_API(PM_GET_API_VERSION):
	case PM_API(PM_GET_CHIPID):
	case PM_API(PM_SET_CONFIGURATION):
		approved = true;
		break;
	default:
		approved = false;
		break;
	}

done:
	return approved;
}

/**
 * PmProcessApiCall() - Called to process PM API call
 * @master  Pointer to a requesting master structure
 * @pload   Pointer to array of integers with the information about the pm call
 *          (api id + arguments of the api)
 *
 * @note    Called to process PM API call. If specific PM API receives less
 *          than 4 arguments, extra arguments are ignored.
 */
void PmProcessRequest(PmMaster *const master, const u32 *pload)
{
	u32 setAddress;
	u64 address;
	bool approved = PmApiApprovalCheck(pload[0]);

	if (false == approved) {
		IPI_RESPONSE1(master->ipiMask, XST_PM_NO_ACCESS);
		goto done;
	}
	switch (pload[0]) {
	case PM_API(PM_SELF_SUSPEND):
		address = ((u64) pload[5]) << 32ULL;
		address += pload[4];
		PmSelfSuspend(master, pload[1], pload[2], pload[3], address);
		break;
	case PM_API(PM_REQUEST_SUSPEND):
		PmRequestSuspend(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_API(PM_FORCE_POWERDOWN):
		PmForcePowerdown(master, pload[1], pload[2]);
		break;
	case PM_API(PM_ABORT_SUSPEND):
		PmAbortSuspend(master, pload[1], pload[2]);
		break;
	case PM_API(PM_REQUEST_WAKEUP):
		/* setAddress is encoded in the 1st bit of the low-word address */
		setAddress = pload[2] & 0x1U;
		/* addresses are word-aligned, ignore bit 0 */
		address = ((u64) pload[3]) << 32ULL;
		address += ((u64)pload[2] & ~((u64)0x1));
		PmRequestWakeup(master, pload[1], setAddress, address, pload[4]);
		break;
	case PM_API(PM_SET_WAKEUP_SOURCE):
		PmSetWakeupSource(master, pload[1], pload[2], pload[3]);
		break;
	case PM_API(PM_SYSTEM_SHUTDOWN):
		PmSystemShutdown(master, pload[1], pload[2]);
		break;
	case PM_API(PM_REQUEST_NODE):
		PmRequestNode(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_API(PM_RELEASE_NODE):
		PmReleaseNode(master, pload[1]);
		break;
	case PM_API(PM_SET_REQUIREMENT):
		PmSetRequirement(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_API(PM_SET_MAX_LATENCY):
		PmSetMaxLatency(master, pload[1], pload[2]);
		break;
	case PM_API(PM_GET_API_VERSION):
		PmGetApiVersion(master);
		break;
	case PM_API(PM_SET_CONFIGURATION):
		PmSetConfiguration(master, pload[1]);
		break;
	case PM_API(PM_GET_NODE_STATUS):
		PmGetNodeStatus(master, pload[1]);
		break;
	case PM_API(PM_GET_OP_CHARACTERISTIC):
		PmGetOpCharacteristics(master, pload[1], pload[2]);
		break;
	case PM_API(PM_REGISTER_NOTIFIER):
		PmRegisterNotifier(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_API(PM_RESET_ASSERT):
		PmResetAssert(master, pload[1], pload[2]);
		break;
	case PM_API(PM_RESET_GET_STATUS):
		PmResetGetStatus(master, pload[1]);
		break;
	case PM_API(PM_MMIO_WRITE):
		PmMmioWrite(master, pload[1], pload[2], pload[3]);
		break;
	case PM_API(PM_MMIO_READ):
		PmMmioRead(master, pload[1]);
		break;
	case PM_API(PM_INIT_FINALIZE):
		PmInitFinalize(master);
		break;
#ifdef ENABLE_FPGA_LOAD
	case PM_API(PM_FPGA_LOAD):
		PmFpgaLoad(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_API(PM_FPGA_GET_STATUS):
		PmFpgaGetStatus(master);
		break;
#if defined(ENABLE_FPGA_READ_CONFIG_DATA) || defined(ENABLE_FPGA_READ_CONFIG_REG)
	case PM_API(PM_FPGA_READ):
		PmFpgaRead(master, pload[1], pload[2], pload[3], pload[4]);
		break;
#endif
#endif
	case PM_API(PM_GET_CHIPID):
		PmGetChipid(master);
		break;
#ifdef ENABLE_SECURE
	case PM_API(PM_SECURE_SHA):
		PmSecureSha(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_API(PM_SECURE_RSA):
		PmSecureRsa(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_API(PM_SECURE_IMAGE):
		PmSecureImage(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_API(PM_SECURE_AES):
		PmSecureAes(master, pload[1], pload[2]);
		break;
#endif
	case PM_API(PM_CLOCK_SETPARENT):
		PmClockSetParent(master, pload[1], pload[2]);
		break;
	case PM_API(PM_CLOCK_GETPARENT):
		PmClockGetParent(master, pload[1]);
		break;
	case PM_API(PM_CLOCK_ENABLE):
		PmClockGateConfig(master, pload[1], 1U);
		break;
	case PM_API(PM_CLOCK_DISABLE):
		PmClockGateConfig(master, pload[1], 0U);
		break;
	case PM_API(PM_CLOCK_GETSTATE):
		PmClockGetStatus(master, pload[1]);
		break;
	case PM_API(PM_CLOCK_SETDIVIDER):
		PmClockSetDivider(master, pload[1], pload[2], pload[3]);
		break;
	case PM_API(PM_CLOCK_GETDIVIDER):
		PmClockGetDivider(master, pload[1], pload[2]);
		break;
	case PM_API(PM_PLL_SET_PARAM):
		PmPllSetParam(master, pload[1], pload[2], pload[3]);
		break;
	case PM_API(PM_PLL_GET_PARAM):
		PmPllGetParam(master, pload[1], pload[2]);
		break;
	case PM_API(PM_PLL_SET_MODE):
		PmPllSetMode(master, pload[1], pload[2]);
		break;
	case PM_API(PM_PLL_GET_MODE):
		PmPllGetMode(master, pload[1]);
		break;
	case PM_API(PM_EFUSE_ACCESS):
		PmEfuseAccess(master, pload[1], pload[2]);
		break;
	case PM_API(PM_PINCTRL_REQUEST):
		PmPinCtrlRequest(master, pload[1]);
		break;
	case PM_API(PM_PINCTRL_RELEASE):
		PmPinCtrlRelease(master, pload[1]);
		break;
	case PM_API(PM_PINCTRL_GET_FUNCTION):
		PmPinCtrlGetFunction(master, pload[1]);
		break;
	case PM_API(PM_PINCTRL_SET_FUNCTION):
		PmPinCtrlSetFunction(master, pload[1], pload[2]);
		break;
	case PM_API(PM_PINCTRL_CONFIG_PARAM_GET):
		PmPinCtrlConfigParamGet(master, pload[1], pload[2]);
		break;
	case PM_API(PM_PINCTRL_CONFIG_PARAM_SET):
		PmPinCtrlConfigParamSet(master, pload[1], pload[2], pload[3]);
		break;
#ifdef ENABLE_IOCTL
	case PM_API(PM_IOCTL):
		PmDevIoctl(master, pload[1], (XPm_IoctlId)pload[2], pload[3], pload[4]);
		break;
#endif
	default:
		PmWarn("Unsupported EEMI API #%lu\r\n", pload[0]);
		IPI_RESPONSE1(master->ipiMask, XST_INVALID_VERSION);
		break;
	}
done:
	return;
}

/**
 * PmShutdownInterruptHandler() - Send suspend request to all active masters
 */
void PmShutdownInterruptHandler(void)
{
#if defined(PMU_MIO_INPUT_PIN) && (PMU_MIO_INPUT_PIN >= 0U) \
				&& (PMU_MIO_INPUT_PIN <= 5U)
	/*
	 * Default status of MIO26 pin is 1. So MIO wake event bit in GPI1
	 * register is always 1, which is used to identify shutdown event.
	 *
	 * GPI event occurs only when any bit of GPI register changes from
	 * 0 to 1. When any GPI1 event occurs Gpi1InterruptHandler() checks
	 * GPI1 register and process interrupts for the bits which are 1.
	 * Because of MIO wake bit is 1 in GPI1 register, shutdown handler
	 * will be called every time when any of GPI1 event occurs.
	 *
	 * There is no way to identify which bit cause GPI1 interrupt.
	 * So every time Gpi1InterruptHandler() is checking bit which are 1
	 * And calls respective handlers.
	 *
	 * To handle such case avoid power off when any other (other than MIO
	 * wake)bit in GPI1 register is 1. If no other bit is 1 in GPI1 register
	 * and still PMU gets GPI1 interrupt means that MIO26 pin state is
	 * changed from (1 to 0 and 0 to 1). In this case it is confirmed that
	 * it is event for shutdown only and not because of other events.
	 * There are chances that some shutdown events are missed (1 out of 50)
	 * but it should not harm.
	 */
	if (XPfw_Read32(PMU_IOMODULE_GPI1) !=
	    (PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK << PMU_MIO_INPUT_PIN)) {
		return;
	}
#endif
	u32 rpu_mode = XPfw_Read32(RPU_RPU_GLBL_CNTL);

	if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterApu_g)) {
		PmInitSuspendCb(&pmMasterApu_g,
				SUSPEND_REASON_SYS_SHUTDOWN, 1U, 0U, 0U);
	}
	if (0U == (rpu_mode & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK)) {
		if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu0_g)) {
			PmInitSuspendCb(&pmMasterRpu0_g,
					SUSPEND_REASON_SYS_SHUTDOWN, 1U, 0U, 0U);
		}
		if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu1_g)) {
			PmInitSuspendCb(&pmMasterRpu1_g,
					SUSPEND_REASON_SYS_SHUTDOWN, 1U, 0U, 0U);
		}
	} else {
		if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu_g)) {
			PmInitSuspendCb(&pmMasterRpu_g,
					SUSPEND_REASON_SYS_SHUTDOWN, 1U, 0U, 0U);
		}
	}
}

#endif
