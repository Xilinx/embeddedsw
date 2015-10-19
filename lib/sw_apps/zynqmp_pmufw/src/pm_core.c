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
 * This file contains implementation of the PM API functions, which
 * should be used directly only by power management itself.
 *********************************************************************/

#include "pm_core.h"
#include "pm_node.h"
#include "pm_proc.h"
#include "pm_defs.h"
#include "pm_common.h"
#include "pm_callbacks.h"
#include "ipi_buffer.h"
#include "pm_mmio_access.h"

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
#ifdef DEBUG_PM
	if (status != XST_SUCCESS) {
		PmDbg("ERROR PM operation failed - code %d\n", status);
	}
#endif
	if (REQUEST_ACK_BLOCKING == ack) {
		/* Return status immediately */
		XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, status);
	} else if (REQUEST_ACK_CB_STANDARD == ack) {
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
 * @state   Not supported
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
	int status;
	u32 worstCaseLatency = 0;
	/* the node ID must refer to a processor belonging to this master */
	PmProc* proc = PmGetProcOfThisMaster(master, node);

	PmDbg("(%s, %d, %d)\n", PmStrNode(node), latency, state);

	if (NULL == proc) {
		PmDbg("ERROR node ID %s(=%d) does not refer to a processor of "
		      "this master channel\n", PmStrNode(node), node);
		status = XST_INVALID_PARAM;
		goto done;
	}

	worstCaseLatency = proc->pwrDnLatency + proc->pwrUpLatency;
	if (latency < worstCaseLatency) {
		PmDbg("Specified latency is smaller than worst case latency! "
		      "Try latency > %lu\n", worstCaseLatency);
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

done:
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, status);
}

/**
 * PmRequestSuspend() - Requested suspend by a PU for another PU
 * @master  PU from which the request is initiated
 * @node    PU node to be suspended
 * @ack     Acknowledge request
 * @latency Desired wakeup latency - Not supported
 * @state   Desired power state   - Not supported
 *
 * If suspend has been successfully requested, the requested PU needs to
 * initiate its own self suspend. Remember to acknowledge to the requestor
 * after:
 * 1. PU/processor gets powered down (after it has initiates self suspend),
 * 2. PU/processor aborts suspend,
 * 3. PU/processor does not respond to the request (timeout) - not supported
 */
static void PmRequestSuspend(const PmMaster *const master,
			     const u32 node,
			     const u32 ack,
			     const u32 latency,
			     const u32 state)
{
	int status;
	PmMaster* target = NULL;

	PmDbg("(%s, %s, %d, %d)\n", PmStrNode(node), PmStrAck(ack),
	      latency, state);

	if (REQUEST_ACK_BLOCKING == ack) {
		PmDbg("ERROR: invalid acknowledge REQUEST_ACK_BLOCKING\n");
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check whether the target is placeholder in PU */
	target = PmMasterGetPlaceholder(node);

	if (NULL == target) {
		PmDbg("ERROR: invalid node argument\n");
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (false == PmCanRequestSuspend(master, target)) {
		PmDbg("ERROR: not allowed to request suspend of %s\n",
		      PmStrNode(node));
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/*
	 * Remember to acknowledge to the requestor when:
	 * 1. PU gets powered down, 2. PU aborts suspend,
	 * 3. PU does not respond to the request (timeout).
	 */
	if (REQUEST_ACK_NO != ack) {
		status = PmRememberSuspendRequest(master, target, ack);
	} else {
		status = XST_SUCCESS;
	}

	if (XST_SUCCESS == status) {
		/* Request is ok and saved (remembered to acknowledge) */
		PmInitSuspendCb(target, SUSPEND_REASON_PU_REQ,
				latency, state, MAX_LATENCY);
	}

done:
	if (XST_SUCCESS != status) {
		/* Something went wrong, acknowledge immediatelly */
		PmProcessAckRequest(ack, master, node, status, 0);
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
	int status;
	u32 oppoint = 0U;
	PmNode* nodePtr = PmGetNodeById(node);

	PmDbg("(%s, %s)\n", PmStrNode(node), PmStrAck(ack));

	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	switch (nodePtr->typeId) {
	case PM_TYPE_PROC:
		status = PmProcFsm((PmProc*)nodePtr->derived,
				   PM_PROC_EVENT_FORCE_PWRDN);
		break;
	case PM_TYPE_PWR_ISLAND:
	case PM_TYPE_PWR_DOMAIN:
		status = PmForceDownTree((PmPower*)nodePtr->derived);
		break;
	default:
		status = XST_INVALID_PARAM;
		break;
	}

	oppoint = nodePtr->currState;

	/*
	 * Successfully powered down a node, now trigger opportunistic
	 * suspend to power down its parent(s) if possible
	 */
	if ((XST_SUCCESS == status) && (NULL != nodePtr->parent)) {
		PmOpportunisticSuspend(nodePtr->parent);
	}

done:
	PmProcessAckRequest(ack, master, node, status, oppoint);
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
	int status;
	PmProc* proc = PmGetProcOfThisMaster(master, node);

	PmDbg("(%s, %s)\n", PmStrNode(node), PmStrReason(reason));

	if (NULL == proc) {
		PmDbg("ERROR processor access for node %s not allowed\n",
		      PmStrNode(node));
		status = XST_PM_INVALID_NODE;
		goto done;
	}

	status = PmProcFsm(proc, PM_PROC_EVENT_ABORT_SUSPEND);

done:
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, status);
}

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
	int status;
	u32 oppoint = 0U;
	PmProc* proc = PmGetProcByNodeId(node);

	PmDbg("(%s, %s)\n", PmStrNode(node), PmStrAck(ack));

	if (NULL == proc) {
		status = XST_PM_INVALID_NODE;
		goto done;
	}

	if (1U == setAddress) {
		proc->saveResumeAddr(proc, address);
	}

	status = PmProcFsm(proc, PM_PROC_EVENT_WAKE);
	oppoint = proc->node.currState;

done:
	PmProcessAckRequest(ack, master, node, status, oppoint);
}

/**
 * PmReleaseNode() - Release a slave node
 * @master  Master who initiated the request
 * @node    Node to be released
 *
 * @note    Node to be released must have been requested before
 */
static void PmReleaseNode(const PmMaster *master,
			  const u32 node,
			  const u32 latency)
{
	int status;
	/* Get static requirements structure for this master/slave pair */
	PmRequirement* masterReq = PmGetRequirementForSlave(master, node);

	if (NULL == masterReq) {
		status = XST_PM_NO_ACCESS;
		PmDbg("ERROR Can't find requirement for slave %s of master %s\n",
		      PmStrNode(node), PmStrNode(master->procs[0].node.nodeId));
		goto done;
	}

	if (0U == (masterReq->info & PM_MASTER_USING_SLAVE_MASK)) {
		status = XST_FAILURE;
		PmDbg("WARNING %s attempt to release %s without previous "
		      "request\n", PmStrNode(master->procs[0].node.nodeId),
		      PmStrNode(node));
		goto done;
	}

	/* Release requirements */
	status = PmRequirementUpdate(masterReq, 0U);
	masterReq->info &= ~PM_MASTER_USING_SLAVE_MASK;

	if (XST_SUCCESS != status) {
		PmDbg("ERROR PmRequirementUpdate status = %d\n", status);
		goto done;
	}

done:
	PmDbg("(%s, %d)\n", PmStrNode(node), latency);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, status);
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
	int status;
	u32 oppoint = 0U;
	/*
	 * Each legal master/slave pair will have one static PmRequirement data
	 * structure. Retrieve the pointer to the structure in order to set the
	 * requested capabilities and mark slave as used by this master.
	 */
	PmRequirement* masterReq = PmGetRequirementForSlave(master, node);

	PmDbg("(%s, %d, %d, %s)\n", PmStrNode(node), capabilities,
	      qos, PmStrAck(ack));

	if (NULL == masterReq) {
		/* Master is not allowed to use the slave with given node */
		PmDbg("ERROR Master can't use the slave\n");
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (PM_MASTER_USING_SLAVE_MASK & masterReq->info) {
		status = XST_PM_DOUBLE_REQ;
		goto done;
	}

	/* Set requested capabilities if they are valid */
	masterReq->info |= PM_MASTER_USING_SLAVE_MASK;
	status = PmRequirementUpdate(masterReq, capabilities);

done:
	PmProcessAckRequest(ack, master, node, status, oppoint);
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
	int status;
	u32 oppoint = 0U;
	PmRequirement* masterReq = PmGetRequirementForSlave(master, node);

	PmDbg("(%s, %d, %d, %s)\n", PmStrNode(node), capabilities,
	      qos, PmStrAck(ack));

	/* Is there a provision for the master to use the given slave node */
	if (NULL == masterReq) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Check if master has previously requested the node */
	if (0U == (PM_MASTER_USING_SLAVE_MASK & masterReq->info)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Master is using slave (previously has requested node) */
	switch (master->procs->node.currState) {
	case PM_PROC_STATE_SUSPENDING:
		/* Schedule setting the requirement */
		status = PmRequirementSchedule(masterReq, capabilities);
		break;
	case PM_PROC_STATE_ACTIVE:
		/* Set capabilities now - if they are valid */
		status = PmRequirementUpdate(masterReq, capabilities);
		break;
	default:
		/*
		 * Should never happen as processor cannot call this API while
		 * powered down.
		 */
		status = XST_FAILURE;
		break;
	}
	oppoint = masterReq->slave->node.currState;

done:
	PmProcessAckRequest(ack, master, node, status, oppoint);
}

/**
 * PmGetApiVersion() - Provides API version number to the caller
 * @master  Master who initiated the request
 */
static void PmGetApiVersion(const PmMaster *const master)
{
	u32 version = (PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR;

	PmDbg("version %d.%d\n", PM_VERSION_MAJOR, PM_VERSION_MINOR);

	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, XST_SUCCESS);
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + PAYLOAD_ELEM_SIZE,
		     version);
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
			const u32 mask, const u32 value)
{
	int status;

	PmDbg("(%s) addr=0x%lx, mask=0x%lx, value=0x%lx\n",
	      PmStrNode(master->nid), address, mask, value);

	/* Check access permissions */
	if (false == PmGetMmioAccess(master, address)) {
		PmDbg("(%s) ERROR: access denied for address 0x%lx\n",
		      PmStrNode(master->nid), address);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	XPfw_Write32(address, mask & value);
	status = XST_SUCCESS;
done:
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, status);
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
	int status;
	u32 value;

	/* Check access permissions */
	if (false == PmGetMmioAccess(master, address)) {
		PmDbg("(%s) ERROR: access denied for address 0x%lx\n",
		      PmStrNode(master->nid), address);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	value = XPfw_Read32(address);
	status = XST_SUCCESS;
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET + PAYLOAD_ELEM_SIZE,
		     value);
	PmDbg("(%s) addr=0x%lx, value=0x%lx\n", PmStrNode(master->nid),
	      address, value);

done:
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, status);
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
	int status = XST_SUCCESS;
	PmRequirement* req = PmGetRequirementForSlave(master, sourceNode);

	/* Is master allowed to use resource (slave)? */
	if (NULL == req) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Does slave have wake-up capability */
	if (NULL == req->slave->wake) {
		status = XST_NO_FEATURE;
		goto done;
	}

	/* Set/clear request info according to the enable flag */
	if (0U == enable) {
		req->info &= ~PM_MASTER_WAKEUP_REQ_MASK;
	} else {
		req->info |= PM_MASTER_WAKEUP_REQ_MASK;
	}

done:
	PmDbg("(%s, %s, %d)\n", PmStrNode(master->procs->node.nodeId),
	      PmStrNode(sourceNode), enable);

	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, status);
}

/**
 * PmSystemShutdown() - Request system shutdown or restart
 * @master  Master requesting system shutdown
 * @restart Flag, 0 is for shutdown, 1 for restart (not-supported)
 */
static void PmSystemShutdown(const PmMaster *const master, const u32 restart)
{
	u32 i;

	PmDbg("(%d)\n", restart);

	if (PM_SHUTDOWN == restart) {
		for (i = 0U; i < ARRAY_SIZE(pmAllMasters); i++) {
			/* Master requesting shutdown will suspend on its own */
			if ((master != pmAllMasters[i]) &&
			    (PM_PROC_STATE_ACTIVE ==
			     pmAllMasters[i]->procs->node.currState)) {
				PmInitSuspendCb(pmAllMasters[i],
						SUSPEND_REASON_SYS_SHUTDOWN,
						MAX_LATENCY, 0, MAX_LATENCY);
			}
		}
	}
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
	int status = XST_SUCCESS;
	PmRequirement* masterReq = PmGetRequirementForSlave(master, node);

	PmDbg("(%s, %lu)\n", PmStrNode(node), latency);

	/* Check if the master can use given slave node */
	if (NULL == masterReq) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Check if master has previously requested the node */
	if (0U == (PM_MASTER_USING_SLAVE_MASK & masterReq->info)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	masterReq->latencyReq = latency;
	status = PmUpdateSlave(masterReq->slave);

done:
	XPfw_Write32(master->buffer + IPI_BUFFER_RESP_OFFSET, status);
}

/**
 * PmSetConfiguration() - Load the configuration
 * @master  Master who initiated the loading of configuration
 * @address Address at which the configuration object is placed
 */
static void PmSetConfiguration(const PmMaster *const master, const u32 address)
{
	PmDbg("(0x%x) not implemented\n", address);
}

/**
 * PmGetNodeStatus() - Get the status of the node
 * @master  Initiator of the request
 * @node    Node whose status should be returned
 */
static void PmGetNodeStatus(const PmMaster *const master, const u32 node)
{
	PmDbg("(%s) not implemented\n", PmStrNode(node));
}

/**
 * PmGetOpCharacteristics() - Get operating characteristics of a node
 * @master  Initiator of the request
 * @node    Node in question
 * @type    Type of the operating characteristics (power, temperature)
 */
static void PmGetOpCharacteristics(const PmMaster *const master, const u32 node,
				   const u32 type)
{
	PmDbg("(%s, %d) not implemented\n", PmStrNode(node), type);
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
	PmDbg("(%s, %d, %d, %d) not implemented\n", PmStrNode(node),
	      event, wake, enable);
}

/**
 * PmResetAssert() - Request setting of reset (1 - assert, 0 - release)
 * @master      Initiator of the request
 * @reset       Reset to be configured
 * @assertFlag  Flag stating should reset be asserted (1) or released (0)
 */
static void PmResetAssert(const PmMaster *const master, const u32 reset,
			  const u32 assertFlag)
{
	PmDbg("%s(%d, %d) not implemented\n", __func__, reset, assertFlag);
}

/**
 * PmResetGetStatus() - Get status of the reset
 * @master  Initiator of the request
 * @reset   Reset whose status should be returned
 */
static void PmResetGetStatus(const PmMaster *const master, const u32 reset)
{
	PmDbg("(%d) not implemented\n", reset);
}

/**
 * PmProcessApiCall() - Called to process PM API call
 * @master  Pointer to a requesting master structure
 * @pload   Pointer to array of integers with the information about the pm call
 *          (api id + arguments of the api)
 *
 * @note    Payload arguments are checked and validated before calling this.
 */
static void PmProcessApiCall(const PmMaster *const master, const u32 *pload)
{
	u32 setAddress;
	u64 address;

	switch (pload[0]) {
	case PM_SELF_SUSPEND:
		address = ((u64) pload[5]) << 32ULL;
		address += pload[4];
		PmSelfSuspend(master, pload[1], pload[2], pload[3], address);
		break;
	case PM_REQUEST_SUSPEND:
		PmRequestSuspend(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_FORCE_POWERDOWN:
		PmForcePowerdown(master, pload[1], pload[2]);
		break;
	case PM_ABORT_SUSPEND:
		PmAbortSuspend(master, pload[1], pload[2]);
		break;
	case PM_REQUEST_WAKEUP:
		/* setAddress is encoded in the 1st bit of the low-word address */
		setAddress = pload[2] & 0x1U;
		/* addresses are word-aligned, ignore bit 0 */
		address = ((u64) pload[3]) << 32ULL;
		address += pload[2] & ~0x1U;
		PmRequestWakeup(master, pload[1], setAddress, address, pload[4]);
		break;
	case PM_SET_WAKEUP_SOURCE:
		PmSetWakeupSource(master, pload[1], pload[2], pload[3]);
		break;
	case PM_SYSTEM_SHUTDOWN:
		PmSystemShutdown(master, pload[1]);
		break;
	case PM_REQUEST_NODE:
		PmRequestNode(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_RELEASE_NODE:
		PmReleaseNode(master, pload[1], pload[2]);
		break;
	case PM_SET_REQUIREMENT:
		PmSetRequirement(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_SET_MAX_LATENCY:
		PmSetMaxLatency(master, pload[1], pload[2]);
		break;
	case PM_GET_API_VERSION:
		PmGetApiVersion(master);
		break;
	case PM_SET_CONFIGURATION:
		PmSetConfiguration(master, pload[1]);
		break;
	case PM_GET_NODE_STATUS:
		PmGetNodeStatus(master, pload[1]);
		break;
	case PM_GET_OP_CHARACTERISTIC:
		PmGetOpCharacteristics(master, pload[1], pload[2]);
		break;
	case PM_REGISTER_NOTIFIER:
		PmRegisterNotifier(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_RESET_ASSERT:
		PmResetAssert(master, pload[1], pload[2]);
		break;
	case PM_RESET_GET_STATUS:
		PmResetGetStatus(master, pload[1]);
		break;
	case PM_MMIO_WRITE:
		PmMmioWrite(master, pload[1], pload[2], pload[3]);
		break;
	case PM_MMIO_READ:
		PmMmioRead(master, pload[1]);
		break;
	default:
		PmDbg("ERROR unsupported PM API #%d\n", pload[0]);
		PmProcessAckRequest(PmRequestAcknowledge(pload), master,
				    NODE_UNKNOWN, XST_INVALID_VERSION, 0);
		break;
	}
}

/**
 * PmProcessRequest() - Process PM API call
 * @master  Pointer to a requesting master structure
 * @pload   Pointer to array of integers with the information about the pm call
 *          (api id + arguments of the api)
 *
 * @note    Called to process PM API call. If specific PM API receives less
 *          than 4 arguments, extra arguments are ignored.
 */
void PmProcessRequest(const PmMaster *const master, const u32 *pload)
{
	PmPayloadStatus status = PmCheckPayload(pload);

	if (PM_PAYLOAD_OK == status) {
		PmProcessApiCall(master, pload);
	} else {
		PmDbg("ERROR invalid payload, status #%d\n", status);
		/* Acknowledge if possible */
		if (PM_PAYLOAD_ERR_API_ID != status) {
			u32 ack = PmRequestAcknowledge(pload);
			/* If api does receive acknowledge */
			if (REQUEST_ACK_NO != ack) {
				/* Acknowledge that parameters are invalid */
				PmAcknowledgeCb(master, NODE_UNKNOWN,
						XST_INVALID_PARAM, 0);
			}
		}
	}
}
