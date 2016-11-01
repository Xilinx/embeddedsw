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

#include "csu.h"
#include "pm_api.h"
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
#include "xilfpga_pcap.h"
#include "pm_clock.h"

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
		PmDbg("ERROR PM operation failed - code %lu\r\n", status);
	}
#endif
	if (REQUEST_ACK_BLOCKING == ack) {
		/* Return status immediately */
		IPI_RESPONSE1(master->ipiMask, status);
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
	int status;
	u32 worstCaseLatency = 0;
	/* the node ID must refer to a processor belonging to this master */
	PmProc* proc = PmGetProcOfThisMaster(master, node);

	PmDbg("(%s, %lu, %lu)\r\n", PmStrNode(node), latency, state);

	if (NULL == proc) {
		PmDbg("ERROR node ID %s(=%lu) does not refer to a processor of "
		      "this master channel\r\n", PmStrNode(node), node);
		status = XST_INVALID_PARAM;
		goto done;
	}

	worstCaseLatency = proc->pwrDnLatency + proc->pwrUpLatency;
	if (latency < worstCaseLatency) {
		PmDbg("Specified latency is smaller than worst case latency! "
		      "Try latency > %lu\r\n", worstCaseLatency);
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
	IPI_RESPONSE1(master->ipiMask, status);
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
	int status = XST_SUCCESS;
	PmMaster* target = NULL;

	PmDbg("(%s, %s, %lu, %lu)\r\n", PmStrNode(node), PmStrAck(ack),
	      latency, state);

	if (REQUEST_ACK_BLOCKING == ack) {
		PmDbg("ERROR: invalid acknowledge REQUEST_ACK_BLOCKING\r\n");
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check whether the target is placeholder in PU */
	target = PmMasterGetPlaceholder(node);

	if (NULL == target) {
		PmDbg("ERROR: invalid node argument\r\n");
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (false == PmCanRequestSuspend(master, target)) {
		PmDbg("ERROR: not allowed to request suspend of %s\r\n",
		      PmStrNode(node));
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (true == PmIsRequestedToSuspend(target)) {
		status = XST_PM_DOUBLE_REQ;
		goto done;
	}

	/* Remember request info and init suspend */
	target->suspendRequest.initiator = master;
	target->suspendRequest.acknowledge = ack;
	PmInitSuspendCb(target, SUSPEND_REASON_PU_REQ, latency, state, 0U);

done:
	if (XST_SUCCESS != status) {
		/* Something went wrong, acknowledge immediatelly */
		PmProcessAckRequest(ack, master, node, status, 0);
	}
}

int PmForcePowerDownInt(u32 node, u32 *oppoint)
{
	int status;
	PmNode* nodePtr = PmGetNodeById(node);

	PmDbg("(%s)\r\n", PmStrNode(node));

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

	*oppoint = nodePtr->currState;

	/*
	 * Successfully powered down a node, now trigger opportunistic
	 * suspend to power down its parent(s) if possible
	 */
	if ((XST_SUCCESS == status) && (NULL != nodePtr->parent)) {
		PmOpportunisticSuspend(nodePtr->parent);
	}
done:
	return status;
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
	int status = PmForcePowerDownInt(node, &oppoint);

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

	PmDbg("(%s, %s)\r\n", PmStrNode(node), PmStrReason(reason));

	if (NULL == proc) {
		PmDbg("ERROR processor access for node %s not allowed\r\n",
		      PmStrNode(node));
		status = XST_PM_INVALID_NODE;
		goto done;
	}

	status = PmProcFsm(proc, PM_PROC_EVENT_ABORT_SUSPEND);

done:
	IPI_RESPONSE1(master->ipiMask, status);
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

	PmDbg("(%s, %s)\r\n", PmStrNode(node), PmStrAck(ack));

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
			  const u32 node)
{
	int status;
	u32 usage;
	PmRequirement* masterReq;
	PmNode* nodePtr;

	nodePtr = PmGetNodeById(node);
	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (true == NODE_IS_POWER(nodePtr->typeId)) {
		status = PmPowerRelease(master, (PmPower*)nodePtr->derived);
		goto done;
	}

	/* Get static requirements structure for this master/slave pair */
	masterReq = PmGetRequirementForSlave(master, node);

	if (NULL == masterReq) {
		status = XST_PM_NO_ACCESS;
		PmDbg("ERROR Can't find requirement for slave %s of master %s\r\n",
		      PmStrNode(node), PmStrNode(master->nid));
		goto done;
	}

	if (0U == (masterReq->info & PM_MASTER_USING_SLAVE_MASK)) {
		status = XST_FAILURE;
		PmDbg("WARNING %s attempt to release %s without previous "
		      "request\r\n", PmStrNode(master->nid), PmStrNode(node));
		goto done;
	}

	/* Release requirements */
	status = PmRequirementUpdate(masterReq, 0U);
	masterReq->info &= ~PM_MASTER_USING_SLAVE_MASK;

	usage = PmSlaveGetUsersMask(masterReq->slave);
	if (0U == usage) {
		PmNotifierEvent(&masterReq->slave->node, EVENT_ZERO_USERS);
	}

	if (XST_SUCCESS != status) {
		PmDbg("ERROR PmRequirementUpdate status = %d\r\n", status);
		goto done;
	}

done:
	PmDbg("(%s)\r\n", PmStrNode(node));
	IPI_RESPONSE1(master->ipiMask, status);
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
	PmRequirement* masterReq;
	PmNode* nodePtr;

	PmDbg("(%s, %lu, %lu, %s)\r\n", PmStrNode(node), capabilities,
	      qos, PmStrAck(ack));

	nodePtr = PmGetNodeById(node);
	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (true == NODE_IS_POWER(nodePtr->typeId)) {
		status = PmPowerRequest(master, (PmPower*)nodePtr->derived);
		goto done;
	}

	/*
	 * Each legal master/slave pair will have one static PmRequirement data
	 * structure. Retrieve the pointer to the structure in order to set the
	 * requested capabilities and mark slave as used by this master.
	 */
	masterReq = PmGetRequirementForSlave(master, node);

	if (NULL == masterReq) {
		/* Master is not allowed to use the slave with given node */
		PmDbg("ERROR Master can't use the slave\r\n");
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (0U != (PM_MASTER_USING_SLAVE_MASK & masterReq->info)) {
		status = XST_PM_DOUBLE_REQ;
		goto done;
	}

	status = PmSlaveVerifyRequest(masterReq->slave);
	if (XST_SUCCESS != status) {
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
	u32 caps = capabilities;
	PmRequirement* masterReq = PmGetRequirementForSlave(master, node);

	PmDbg("(%s, %lu, %lu, %s)\r\n", PmStrNode(node), capabilities,
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
	PmProcessAckRequest(ack, master, node, status, oppoint);
}

/**
 * PmGetApiVersion() - Provides API version number to the caller
 * @master  Master who initiated the request
 */
static void PmGetApiVersion(const PmMaster *const master)
{
	u32 version = (PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR;

	PmDbg("version %d.%d\r\n", PM_VERSION_MAJOR, PM_VERSION_MINOR);

	IPI_RESPONSE2(master->ipiMask, XST_SUCCESS, version);
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
	u32 data;
	int status = XST_SUCCESS;

	PmDbg("(%s) addr=0x%lx, mask=0x%lx, value=0x%lx\r\n",
	      PmStrNode(master->nid), address, mask, value);

	/* no bits to be updated */
	if (0U == mask) {
		goto done;
	}

	/* Check access permissions */
	if (false == PmGetMmioAccessWrite(master, address)) {
		PmDbg("(%s) ERROR: access denied for address 0x%lx\r\n",
		      PmStrNode(master->nid), address);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* replace whole word */
	if (0xffffffffU == mask) {
		goto do_write;
	}

	/* read-modify-write */
	data = XPfw_Read32(address);
	data &= ~mask;
	value &= mask;
	value |= data;

do_write:
	XPfw_Write32(address, value);
	PmClockSnoop(address, mask, value);

done:
	IPI_RESPONSE1(master->ipiMask, status);
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
	u32 value = 0;

	/* Check access permissions */
	if (false == PmGetMmioAccessRead(master, address)) {
		PmDbg("(%s) ERROR: access denied for address 0x%lx\r\n",
		      PmStrNode(master->nid), address);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	value = XPfw_Read32(address);
	status = XST_SUCCESS;
	PmDbg("(%s) addr=0x%lx, value=0x%lx\r\n", PmStrNode(master->nid),
	      address, value);

done:
	IPI_RESPONSE2(master->ipiMask, status, value);
}

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
			const u32 size, const u32 flags)
{
	u32 Status;

       Status = XFpga_PL_BitSream_Load(AddrHigh, AddrLow, size, flags);

       IPI_RESPONSE1(master->ipiMask, Status);
}

/**
 * PmFpgaGetStatus() - Get status of the PL-block
 * @master  Initiator of the request
 */
static void PmFpgaGetStatus(const PmMaster *const master)
{
	u32 value;

       value = XFpga_PcapStatus();

       IPI_RESPONSE2(master->ipiMask, XST_SUCCESS, value);
}

/**
 * PmGetChipid() - Get silicon version register
 */
static void PmGetChipid(const PmMaster *const master)
{
	u32 idcode = XPfw_Read32(CSU_IDCODE);
	u32 version = XPfw_Read32(CSU_VERSION);

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
	int status = XST_SUCCESS;
	PmRequirement* req = PmGetRequirementForSlave(master, sourceNode);

	/* Check if given target node is valid */
	if ((targetNode != master->nid) &&
	    (NULL == PmGetProcOfThisMaster(master, targetNode))) {
		status = XST_INVALID_PARAM;
		goto done;
	}

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
		req->info &= ~PM_MASTER_WAKEUP_REQ_MASK;
	} else {
		req->info |= PM_MASTER_WAKEUP_REQ_MASK;
	}

	/* If master has its own GIC, call its setWake function */
	if (NULL != req->master->gic) {
		req->master->gic->setWake(req->slave, enable);
	}

done:
	PmDbg("(%s, %s, %lu)\r\n", PmStrNode(master->nid), PmStrNode(sourceNode),
	      enable);

	IPI_RESPONSE1(master->ipiMask, status);
}

/**
 * PmSystemShutdown() - Request system shutdown or restart
 * @master  Master requesting system shutdown
 * @type    Shutdown type
 * @subtype Shutdown subtype
 */
static void PmSystemShutdown(const PmMaster *const master, const u32 type,
			     const u32 subtype)
{
	int status;

	PmDbg("(%lu, %lu)\r\n", type, subtype);

	switch (subtype) {
	case PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM:
		status = XST_NO_FEATURE;
		break;
	case PMF_SHUTDOWN_SUBTYPE_PS_ONLY:
	case PMF_SHUTDOWN_SUBTYPE_SYSTEM:
		/* Check whether the master is allowed to trigger system level action */
		if (true == PmSystemRequestNotAllowed(master)) {
			status = XST_PM_NO_ACCESS;
			goto done;
		}

		/* Check whether given arguments are ok */
		if ((PMF_SHUTDOWN_TYPE_SHUTDOWN != type) &&
		    (PMF_SHUTDOWN_TYPE_RESET != type)) {
			status = XST_INVALID_PARAM;
			goto done;
		}

		PmSystemProcessShutdown(master, type, subtype);
		break;
	default:
		PmDbg("invalid subtype (%lx)\n", subtype);
		status = XST_INVALID_PARAM;
		break;
	}

done:
	IPI_RESPONSE1(master->ipiMask, status);
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

	PmDbg("(%s, %lu)\r\n", PmStrNode(node), latency);

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
	IPI_RESPONSE1(master->ipiMask, status);
}

/**
 * PmSetConfiguration() - Load the configuration
 * @master  Master who initiated the loading of configuration
 * @address Address at which the configuration object is placed
 */
static void PmSetConfiguration(const PmMaster *const master, const u32 address)
{
	PmDbg("(0x%lx) %s: not implemented\r\n", address, PmStrNode(master->nid));
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
	int status = XST_SUCCESS;
	PmNode* nodePtr = PmGetNodeById(node);

	PmDbg("(%s)\r\n", PmStrNode(node));

	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	oppoint = nodePtr->currState;
	if (nodePtr->typeId >= PM_TYPE_SLAVE) {
		currReq = PmSlaveGetRequirements(node, master);
		usage = PmSlaveGetUsageStatus(node, master);
	}

done:
	IPI_RESPONSE4(master->ipiMask, status, oppoint, currReq, usage);
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
	int status = XST_SUCCESS;
	PmNode* nodePtr = PmGetNodeById(node);

	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	switch(type) {
	case PM_OPCHAR_TYPE_POWER:
		result = PmNodeGetPowerConsumption(nodePtr, nodePtr->currState);
		break;
	case PM_OPCHAR_TYPE_TEMP:
		PmDbg("(%s) WARNING: Temperature unsupported\r\n", PmStrNode(node));
		break;
	case PM_OPCHAR_TYPE_LATENCY:
		result = PmNodeGetWakeLatency(nodePtr);
		break;
	default:
		PmDbg("(%s) ERROR: Invalid type: %lu\r\n", PmStrNode(node), type);
		status = XST_INVALID_PARAM;
		goto done;
	}

done:
	PmDbg("(%s, %lu, %lu)\r\n", PmStrNode(node), type, result);
	IPI_RESPONSE2(master->ipiMask, status, result);
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
	int status;
	PmNode* nodePtr = PmGetNodeById(node);

	PmDbg("(%s, %lu, %lu, %lu)\r\n", PmStrNode(node), event, wake, enable);

	if (NULL == nodePtr) {
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
	IPI_RESPONSE1(master->ipiMask, status);
}

/**
 * PmInit() - Initialization of the power management data
 * @master  Initiator of the request
 */
void PmInit(const PmMaster* const master)
{
	PmDbg("\r\n");

	PmClockInitData();

	if (NULL != master) {
		IPI_RESPONSE1(master->ipiMask, XST_SUCCESS);
	}
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
		PmSystemShutdown(master, pload[1], pload[2]);
		break;
	case PM_REQUEST_NODE:
		PmRequestNode(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_RELEASE_NODE:
		PmReleaseNode(master, pload[1]);
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
	case PM_INIT:
		PmInit(master);
		break;
	case PM_FPGA_LOAD:
		PmFpgaLoad(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_FPGA_GET_STATUS:
		PmFpgaGetStatus(master);
		break;
	case PM_GET_CHIPID:
		PmGetChipid(master);
		break;
	default:
		PmDbg("ERROR unsupported PM API #%lu\r\n", pload[0]);
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
		PmDbg("ERROR invalid payload, status #%d\r\n", status);
		/* Acknowledge if possible */
		if (PM_PAYLOAD_ERR_API_ID != status) {
			u32 ack = PmRequestAcknowledge(pload);

			PmProcessAckRequest(ack, master, NODE_UNKNOWN,
					    XST_INVALID_PARAM, 0);
		}
	}
}
