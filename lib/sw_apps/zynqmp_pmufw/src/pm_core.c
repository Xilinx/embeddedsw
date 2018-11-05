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
#include "xpfw_config.h"
#ifdef ENABLE_PM

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
#ifdef ENABLE_FPGA_LOAD
#include "xilfpga_pcap.h"
#endif
#include "pm_clock.h"
#include "pm_requirement.h"
#include "pm_config.h"
#include "xpfw_platform.h"
#include "xpfw_resets.h"
#include "rpu.h"
#ifdef ENABLE_SECURE
#include "xsecure.h"
#endif
#include "pmu_iomodule.h"

#define AMS_REF_CTRL_REG_OFFSET	0x108

/**
 * PmKillBoardPower() - Power-off board by sending KILL signal to power chip
 */
#if defined(BOARD_SHUTDOWN_PIN) && defined(BOARD_SHUTDOWN_PIN_STATE)
static void PmKillBoardPower(void)
{
	u32 reg = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	u32 mask = PMU_IOMODULE_GPO1_MIO_0_MASK << BOARD_SHUTDOWN_PIN;
	u32 value = BOARD_SHUTDOWN_PIN_STATE << BOARD_SHUTDOWN_PIN;

	reg = (reg & (~mask)) | (mask & value);
	XPfw_Write32(PMU_IOMODULE_GPO1, reg);
}
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

	PmDbg(DEBUG_DETAILED,"(%s, %lu, %lu, 0x%llx)\r\n", PmStrNode(node),
			latency, state, address);

	if (NULL == proc) {
		PmDbg(DEBUG_DETAILED,"ERROR node ID %s(=%lu) does not refer to a "
				"processor of this master channel\r\n", PmStrNode(node), node);
		status = XST_INVALID_PARAM;
		goto done;
	}

	worstCaseLatency = proc->pwrDnLatency + proc->pwrUpLatency;
	if (latency < worstCaseLatency) {
		PmDbg(DEBUG_DETAILED,"Specified latency is smaller than worst case "
				"latency! Try latency > %lu\r\n", worstCaseLatency);
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

	PmDbg(DEBUG_DETAILED,"(%s, %s, %lu, %lu)\r\n", PmStrNode(node),
			PmStrAck(ack), latency, state);

	if (REQUEST_ACK_BLOCKING == ack) {
		PmDbg(DEBUG_DETAILED,"ERROR: invalid acknowledge "
				"REQUEST_ACK_BLOCKING\r\n");
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check whether the target is placeholder in PU */
	target = PmMasterGetPlaceholder(node);

	if (NULL == target) {
		PmDbg(DEBUG_DETAILED,"ERROR: invalid node argument\r\n");
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (false == PmCanRequestSuspend(master, target)) {
		PmDbg(DEBUG_DETAILED,"ERROR: not allowed to request suspend of %s\r\n",
		      PmStrNode(node));
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
	u32 oppoint = 0U;
	int status;
	PmNode* nodePtr = PmGetNodeById(node);

	if (NULL == nodePtr) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (NODE_IS_SLAVE(nodePtr)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (NODE_IS_POWER(nodePtr)) {
		PmPower* power = (PmPower*)nodePtr->derived;
		if (false == PmMasterCanForceDown(master, power)) {
			status = XST_PM_NO_ACCESS;
			goto done;
		}
	}

	status = PmNodeForceDown(nodePtr);
	oppoint = nodePtr->currState;

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

	PmDbg(DEBUG_DETAILED,"(%s, %s)\r\n", PmStrNode(node), PmStrReason(reason));

	if (NULL == proc) {
		PmDbg(DEBUG_DETAILED,"ERROR processor access for node %s "
				"not allowed\r\n", PmStrNode(node));
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
	PmProc* proc = PmNodeGetProc(node);

	PmDbg(DEBUG_DETAILED,"(%s, %s, %lu, %llu, %lu)\r\n", PmStrNode(node),
			PmStrAck(ack), setAddress, address, ack);

	if ((NULL == proc) || (NULL == proc->master)) {
		PmDbg(DEBUG_DETAILED,"ERROR: Invalid node argument %s\r\n",
				PmStrNode(node));
		status = XST_PM_INVALID_NODE;
		goto done;
	}

	if ((false == PmMasterCanRequestWake(master, proc->master)) &&
	    (master != proc->master)) {
		PmDbg(DEBUG_DETAILED,"ERROR: No permission to wake-up %s\r\n",
				PmStrNode(node));
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (1U == setAddress) {
		proc->saveResumeAddr(proc, address);
	} else {
		if (false == PmProcHasResumeAddr(proc)) {
			PmDbg(DEBUG_DETAILED,"ERROR: missing address argument\r\n");
			status = XST_INVALID_PARAM;
			goto done;
		}
	}

	status = PmMasterWakeProc(proc);
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
	PmSlave* slave;

	/* Check if node is slave. If it is, handle request via requirements */
	slave = PmNodeGetSlave(node);
	if (NULL == slave) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Get static requirements structure for this master/slave pair */
	masterReq = PmRequirementGet(master, slave);

	if (NULL == masterReq) {
		status = XST_PM_NO_ACCESS;
		PmDbg(DEBUG_DETAILED,"ERROR Can't find requirement for slave %s of "
				"master %s\r\n", PmStrNode(node), PmStrNode(master->nid));
		goto done;
	}

	if (!MASTER_REQUESTED_SLAVE(masterReq)) {
		status = XST_FAILURE;
		PmDbg(DEBUG_DETAILED,"WARNING %s attempt to release %s without "
			"previous request\r\n", PmStrNode(master->nid), PmStrNode(node));
		goto done;
	}

	/* Release requirements */
	status = PmRequirementRelease(masterReq, RELEASE_ONE);

	usage = PmSlaveGetUsersMask(masterReq->slave);
	if (0U == usage) {
		PmNotifierEvent(&masterReq->slave->node, EVENT_ZERO_USERS);
	}

	if (XST_SUCCESS != status) {
		PmDbg(DEBUG_DETAILED,"ERROR PmRequirementUpdate status = %d\r\n", status);
		goto done;
	}

done:
	PmDbg(DEBUG_DETAILED,"(%s)\r\n", PmStrNode(node));
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
	PmSlave* slave;

	PmDbg(DEBUG_DETAILED,"(%s, %lu, %lu, %s)\r\n", PmStrNode(node),
			capabilities, qos, PmStrAck(ack));

	/* Check if node is slave. If it is, handle request via requirements */
	slave = PmNodeGetSlave(node);
	if (NULL == slave) {
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
		PmDbg(DEBUG_DETAILED,"ERROR Master can't use the slave\r\n");
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	if (MASTER_REQUESTED_SLAVE(masterReq)) {
		status = XST_PM_DOUBLE_REQ;
		PmDbg(DEBUG_DETAILED,"Warning %d: slave already requested\r\n",
				status);
		goto done;
	}

	status = PmSlaveVerifyRequest(masterReq->slave);
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Set requested capabilities if they are valid */
	status = PmRequirementRequest(masterReq, capabilities);

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
	PmRequirement* masterReq;
	PmSlave* slave = PmNodeGetSlave(node);

	PmDbg(DEBUG_DETAILED,"(%s, %lu, %lu, %s)\r\n", PmStrNode(node),
			capabilities, qos, PmStrAck(ack));

	/* Set requirement call applies only to slaves */
	if (NULL == slave) {
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
	PmProcessAckRequest(ack, master, node, status, oppoint);
}

/**
 * PmGetApiVersion() - Provides API version number to the caller
 * @master  Master who initiated the request
 */
static void PmGetApiVersion(const PmMaster *const master)
{
	u32 version = (PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR;

	PmDbg(DEBUG_DETAILED,"version %d.%d\r\n", PM_VERSION_MAJOR,
			PM_VERSION_MINOR);

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

#ifdef DEBUG_MMIO
	PmDbg(DEBUG_DETAILED,"(%s) addr=0x%lx, mask=0x%lx, value=0x%lx\r\n",
	      PmStrNode(master->nid), address, mask, value);
#endif
	/* no bits to be updated */
	if (0U == mask) {
		goto done;
	}

	/* Do not update AMS_REF_CLOCK register */
	if ((CRL_APB_BASEADDR + AMS_REF_CTRL_REG_OFFSET) == address) {
		goto done;
	}

	/* Check access permissions */
	if (false == PmGetMmioAccessWrite(master, address)) {
		PmDbg(DEBUG_DETAILED,"(%s) ERROR: access denied for address 0x%lx\r\n",
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
		PmDbg(DEBUG_DETAILED,"(%s) ERROR: access denied for address 0x%lx\r\n",
		      PmStrNode(master->nid), address);
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	value = XPfw_Read32(address);
	status = XST_SUCCESS;
#ifdef DEBUG_MMIO
	PmDbg(DEBUG_DETAILED,"(%s) addr=0x%lx, value=0x%lx\r\n",
			PmStrNode(master->nid), address, value);
#endif

done:
	IPI_RESPONSE2(master->ipiMask, status, value);
}

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
			const u32 KeyAddr, const u32 flags)
{
	u32 Status;
	UINTPTR WrAddr = ((u64)AddrHigh << 32)|AddrLow;

       Status = XFpga_PL_BitSream_Load(WrAddr, KeyAddr, flags);

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
#endif

#ifdef ENABLE_SECURE
/**
 * PmSecureRsaAes() - Load secure image.
 * This function loads the secure images back to memory, it supports
 * loading of authenticated, encrypted or both encrypted and
 * authenticated images back to memory.
 *
 * AddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *         will read the data
 *
 * AddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *         will read the data
 *
 * WrSize: Number of 32bit words that the DMA should write
 *
 * @return  error status based on implemented functionality(SUCCESS by default)
 */
static void PmSecureRsaAes(const PmMaster *const master,
			const u32 AddrHigh, const u32 AddrLow,
			const u32 size, const u32 flags)
{
	u32 Status;

	Status = XSecure_RsaAes(AddrHigh, AddrLow, size, flags);

	IPI_RESPONSE1(master->ipiMask, Status);
}

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

	Status = XSecure_Sha3Hash(SrcAddrHigh, SrcAddrLow,
			SrcSize, Flags);

	IPI_RESPONSE1(master->ipiMask, Status);
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

	Status = XSecure_RsaCore(SrcAddrHigh, SrcAddrLow,
			SrcSize, Flags);

	IPI_RESPONSE1(master->ipiMask, Status);
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
	XSecure_DataAddr Addr = {0};

	Status = XSecure_SecureImage(SrcAddrHigh, SrcAddrLow, KupAddrHigh, KupAddrLow, &Addr);

	IPI_RESPONSE3(master->ipiMask, Status, Addr.AddrHigh, Addr.AddrLow);
}
#endif

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
	int status = XST_SUCCESS;
	PmRequirement* req;
	PmSlave* slave = PmNodeGetSlave(sourceNode);

	/* Check if given target node is valid */
	if ((targetNode != master->nid) &&
	    (NULL == PmGetProcOfThisMaster(master, targetNode))) {
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
		req->info &= ~PM_MASTER_WAKEUP_REQ_MASK;
	} else {
		req->info |= PM_MASTER_WAKEUP_REQ_MASK;
	}

	if (NULL != slave->wake) {
		slave->wake->class->set(slave->wake, master->ipiMask, enable);
	}

done:
	PmDbg(DEBUG_DETAILED,"(%s, %s, %lu)\r\n", PmStrNode(master->nid),
			PmStrNode(sourceNode), enable);

	IPI_RESPONSE1(master->ipiMask, status);
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
	int status = XST_SUCCESS;

	PmDbg(DEBUG_DETAILED,"(%lu, %lu)\r\n", type, subtype);

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
#ifdef IDLE_PERIPHERALS
		PmMasterIdleSystem();
#endif
		/*
		 * Reset states of Slave devices before shutdown. This ensures
		 * that devices are in on state after reboot.
		 */
		PmResetSlaveStates();

		XPfw_ResetPsOnly();
		break;
	case PMF_SHUTDOWN_SUBTYPE_SYSTEM:
		/* Bypass RPLL before SRST : Workaround for a bug in 1.0 Silicon */
		if (XPfw_PlatformGetPsVersion() == XPFW_PLATFORM_PS_V1) {
			XPfw_UtilRMW(CRL_APB_RPLL_CTRL, CRL_APB_RPLL_CTRL_BYPASS_MASK,
					 CRL_APB_RPLL_CTRL_BYPASS_MASK);
		}
#ifdef IDLE_PERIPHERALS
		PmMasterIdleSystem();
#endif
		/*
		 * Reset states of Slave devices before shutdown. This ensures
		 * that devices are in on state after reboot.
		 */
		PmResetSlaveStates();

		XPfw_RMW32(CRL_APB_RESET_CTRL,
			   CRL_APB_RESET_CTRL_SOFT_RESET_MASK,
			   CRL_APB_RESET_CTRL_SOFT_RESET_MASK);
		break;
	default:
		PmDbg(DEBUG_DETAILED,"invalid subtype (%lx)\n", subtype);
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
	PmRequirement* masterReq;
	PmSlave* slave = PmNodeGetSlave(node);

	PmDbg(DEBUG_DETAILED,"(%s, %lu)\r\n", PmStrNode(node), latency);

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
	IPI_RESPONSE1(master->ipiMask, status);
}

/**
 * PmSetConfiguration() - Load the configuration
 * @master  Master who initiated the loading of configuration
 * @address Address at which the configuration object is placed
 */
static void PmSetConfiguration(const PmMaster *const master, const u32 address)
{
	int status;
	u32 configAddr = address;
	u32 callerIpiMask = master->ipiMask;

	PmDbg(DEBUG_DETAILED,"(0x%lx) %s\r\n", address, PmStrNode(master->nid));

	if (NULL != master->remapAddr) {
		configAddr = master->remapAddr(address);
	}

	status = PmConfigLoadObject(configAddr, callerIpiMask);
	/*
	 * Respond using the saved IPI mask of the caller (master's IPI mask
	 * may change after setting the configuration)
	 */
	IPI_RESPONSE1(callerIpiMask, status);
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
			PmDbg(DEBUG_DETAILED,"ERROR: expected split mode, found "
					"lockstep\r\n");
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
				PmDbg(DEBUG_DETAILED,"ERROR: expected lockstep, found "
						"split mode\r\n");
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
	int status = XST_SUCCESS;
	PmNode* nodePtr = PmGetNodeById(node);

	PmDbg(DEBUG_DETAILED,"(%s)\r\n", PmStrNode(node));

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
		if (NULL == nodePtr->class->getPowerData) {
			status = XST_NO_FEATURE;
			goto done;
		}
		status = nodePtr->class->getPowerData(nodePtr, &result);
		break;
	case PM_OPCHAR_TYPE_TEMP:
		PmDbg(DEBUG_DETAILED,"(%s) WARNING: Temperature unsupported\r\n",
				PmStrNode(node));
		break;
	case PM_OPCHAR_TYPE_LATENCY:
		if (NULL == nodePtr->class->getWakeUpLatency) {
			status = XST_NO_FEATURE;
			goto done;
		}
		status = nodePtr->class->getWakeUpLatency(nodePtr, &result);
		break;
	default:
		PmDbg(DEBUG_DETAILED,"(%s) ERROR: Invalid type: %lu\r\n",
				PmStrNode(node), type);
		status = XST_INVALID_PARAM;
		goto done;
	}

done:
	PmDbg(DEBUG_DETAILED,"(%s, %lu, %lu)\r\n", PmStrNode(node), type, result);
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

	PmDbg(DEBUG_DETAILED,"(%s, %lu, %lu, %lu)\r\n", PmStrNode(node), event,
			wake, enable);

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
 * PmInitFinalize() - Notification from a master that it has initialized PM
 * @master  Initiator of the request
 */
void PmInitFinalize(PmMaster* const master)
{
	int status;

	PmDbg(DEBUG_DETAILED, "(%s)\r\n", PmStrNode(master->nid));
	status = PmMasterInitFinalize(master);
	IPI_RESPONSE1(master->ipiMask, status);
}

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
	case PM_GET_API_VERSION:
	case PM_GET_CHIPID:
	case PM_SET_CONFIGURATION:
		approved = true;
		break;
	default:
		approved = false;
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
 * @note    Payload arguments are checked and validated before calling this.
 */
static void PmProcessApiCall(PmMaster *const master, const u32 *pload)
{
	u32 setAddress;
	u64 address;
	bool approved = PmApiApprovalCheck(pload[0]);

	if (false == approved) {
		IPI_RESPONSE1(master->ipiMask, XST_PM_NO_ACCESS);
		goto done;
	}
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
	case PM_INIT_FINALIZE:
		PmInitFinalize(master);
		break;
#ifdef ENABLE_FPGA_LOAD
	case PM_FPGA_LOAD:
		PmFpgaLoad(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_FPGA_GET_STATUS:
		PmFpgaGetStatus(master);
		break;
#endif
	case PM_GET_CHIPID:
		PmGetChipid(master);
		break;
#ifdef ENABLE_SECURE
	case PM_SECURE_RSA_AES:
		PmSecureRsaAes(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_SECURE_SHA:
		PmSecureSha(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_SECURE_RSA:
		PmSecureRsa(master, pload[1], pload[2], pload[3], pload[4]);
		break;
	case PM_SECURE_IMAGE:
		PmSecureImage(master, pload[1], pload[2], pload[3], pload[4]);
		break;
#endif
	default:
		PmDbg(DEBUG_DETAILED,"ERROR unsupported PM API #%lu\r\n", pload[0]);
		PmProcessAckRequest(PmRequestAcknowledge(pload), master,
				    NODE_UNKNOWN, XST_INVALID_VERSION, 0);
		break;
	}
done:
	return;
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
void PmProcessRequest(PmMaster *const master, const u32 *pload)
{
	PmPayloadStatus status = PmCheckPayload(pload);

	if (PM_PAYLOAD_OK == status) {
		PmProcessApiCall(master, pload);
	} else {
		PmDbg(DEBUG_DETAILED,"ERROR invalid payload, status #%d\r\n", status);
		/* Acknowledge if possible */
		if (PM_PAYLOAD_ERR_API_ID != status) {
			u32 ack = PmRequestAcknowledge(pload);

			PmProcessAckRequest(ack, master, NODE_UNKNOWN,
					    XST_INVALID_PARAM, 0);
		}
	}
}

/**
 * PmShutdownInterruptHandler() - Send suspend request to all active masters
 */
void PmShutdownInterruptHandler(void)
{
#if defined(PMU_MIO_INPUT_PIN) && (PMU_MIO_INPUT_PIN >= 0) \
				&& (PMU_MIO_INPUT_PIN <= 5)
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
	    (PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK << PMU_MIO_INPUT_PIN))
		return;
#endif
	u32 rpu_mode = XPfw_Read32(RPU_RPU_GLBL_CNTL);

	if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterApu_g)) {
		PmInitSuspendCb(&pmMasterApu_g,
				SUSPEND_REASON_SYS_SHUTDOWN, 1, 0, 0);
	}
	if (0U == (rpu_mode & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK)) {
		if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu0_g)) {
			PmInitSuspendCb(&pmMasterRpu0_g,
					SUSPEND_REASON_SYS_SHUTDOWN, 1, 0, 0);
		}
		if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu1_g)) {
			PmInitSuspendCb(&pmMasterRpu1_g,
					SUSPEND_REASON_SYS_SHUTDOWN, 1, 0, 0);
		}
	} else {
		if (PM_MASTER_STATE_ACTIVE == PmMasterIsActive(&pmMasterRpu_g)) {
			PmInitSuspendCb(&pmMasterRpu_g,
					SUSPEND_REASON_SYS_SHUTDOWN, 1, 0, 0);
		}
	}
}

#endif
