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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "pm_client.h"
#include "pm_common.h"
#include "pm_api_sys.h"
#include "pm_ipi_buffer.h"

/**
 * Assigning of argument values into array elements.
 * pause and pm_dbg are used for debugging and should be removed in
 * final version.
 */
#define PACK_PAYLOAD(pl, arg0, arg1, arg2, arg3, arg4)			\
	pl[0] = (u32)arg0;						\
	pl[1] = (u32)arg1;						\
	pl[2] = (u32)arg2;						\
	pl[3] = (u32)arg3;						\
	pl[4] = (u32)arg4;						\
	pm_dbg("%s(%d, %d, %d, %d)\n", __func__, arg1, arg2, arg3, arg4);

/**
 * XPm_GetBootStatus() - checks for reason of boot
 *
 * Function returns information about the boot reason.
 * If the boot is not a system startup but a resume,
 * power down request bitfield for this processor will be cleared.
 *
 * @return	Returns processor boot status
 */
enum XPmBootStatus XPm_GetBootStatus()
{
	u32 pwrdn_req = pm_read(MASTER_PWRCTL);
	if (0 != (pwrdn_req & primary_master->pwrdn_mask)) {
		pm_write(MASTER_PWRCTL, pwrdn_req & (~primary_master->pwrdn_mask));
		return PM_RESUME;
	} else {
		return PM_INITIAL_BOOT;
	}
}

/**
 * pm_ipi_wait() - wait for pmu to handle request
 * @master	master which is waiting for PMU to handle request
 */
static enum XPmStatus pm_ipi_wait(const struct XPm_Master *const master)
{
	volatile u32 status = 1;

	/* Wait until previous interrupt is handled by PMU */
	while (status) {
		status = pm_read(master->ipi->base + IPI_OBS_OFFSET)
			& IPI_PMU_PM_INT_MASK;
		/* TODO: 1) Use timer to add delay between read attempts */
		/* TODO: 2) Return PM_RET_ERR_TIMEOUT if this times out */
	}
	return PM_RET_SUCCESS;
}

/**
 * pm_ipi_send() - Sends IPI request to the PMU
 * @master	Pointer to the master who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 *
 * @return	Returns status, either success or error+reason
 */
static enum XPmStatus pm_ipi_send(const struct XPm_Master *const master,
				      u32 payload[PAYLOAD_ARG_CNT])
{
	u32 i;
	u32 offset = 0;
	u32 buffer_base = master->ipi->buffer_base
		+ IPI_BUFFER_TARGET_PMU_OFFSET
		+ IPI_BUFFER_REQUEST_OFFSET;

	/* Wait until previous interrupt is handled by PMU */
	pm_ipi_wait(master);
	/* Write payload into IPI buffer */
	for (i = 0; i < PAYLOAD_ARG_CNT; i++) {
		pm_write(buffer_base + offset, payload[i]);
		offset += PAYLOAD_ARG_SIZE;
	}
	/* Generate IPI to PMU */
	pm_write(master->ipi->base + IPI_TRIG_OFFSET, IPI_PMU_PM_INT_MASK);
	return PM_RET_SUCCESS;
}

/**
 * pm_ipi_buff_read32() - Reads IPI response after PMU has handled interrupt
 * @master	Pointer to the master who is waiting and reading response
 * @value 	Used to return value from 2nd IPI buffer element (optional)
 *
 * @return	Returns status, either success or error+reason
 */
static enum XPmStatus pm_ipi_buff_read32(const struct XPm_Master *const master,
					     u32 *value)
{
	u32 buffer_base = master->ipi->buffer_base
		+ IPI_BUFFER_TARGET_PMU_OFFSET
		+ IPI_BUFFER_RESP_OFFSET;
	volatile u32 status = 1;

	/* Wait until current IPI interrupt is handled by PMU */
	while (status) {
		status = pm_read(master->ipi->base + IPI_OBS_OFFSET) & IPI_PMU_PM_INT_MASK;
		/* TODO: 1) Use timer to add delay between read attempts */
		/* TODO: 2) Return PM_RET_ERR_TIMEOUT if this times out */
	}

	/*
	 * Read response from IPI buffer
	 * buf-0: success or error+reason
	 * buf-1: value
	 * buf-2: unused
	 * buf-3: unused
	 */
	if (NULL != value)
		*value = pm_read(buffer_base + PAYLOAD_ARG_SIZE);

	return pm_read(buffer_base);
}

/**
 * XPm_SelfSuspend() - PM call for master to suspend itself
 * @node	Node id of the master or subsystem
 * @latency	Requested maximum wakeup latency (not supported)
 * @state	Requested state (not supported)
 *
 * This is a blocking call, it will return only once PMU has responded
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_SelfSuspend(const enum XPmNodeId nid,
				   const u32 latency,
				   const u8 state)
{
	enum XPmStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	const struct XPm_Master *master = pm_get_master_by_node(nid);
	if (NULL == master) {
		/*
		 * If a subsystem node ID (APU or RPU) was passed then
		 * the master to be used is the primary master.
		 * E.g. for the APU the primary master is APU0
		 */
		if (subsystem_node == nid) {
			master = primary_master;
		} else {
			return PM_RET_ERROR_ARGS;
		}
	}
	/*
	 * Do client specific suspend operations
	 * (e.g. disable interrupts and set powerdown request bit)
	 */
	XPm_ClientSuspend(master);
	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_SELF_SUSPEND, nid, latency, state, 0);
	ret = pm_ipi_send(master, payload);
	if (PM_RET_SUCCESS != ret)
		return ret;
	/* Wait for PMU to finish handling request */
	return pm_ipi_wait(master);
}

/**
 * XPm_RequestSuspend() - PM call to request for another PU or subsystem to
 *		      be suspended gracefully.
 * @target	Node id of the targeted PU or subsystem
 * @ack		Flag to specify whether acknowledge is requested
 * @latency	Requested wakeup latency (not supported)
 * @state	Requested state (not supported)
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_RequestSuspend(const enum XPmNodeId target,
				  const enum XPmRequestAck ack,
				  const u32 latency, const u8 state)
{
	enum XPmStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_REQUEST_SUSPEND, target, ack, latency, state);
	ret = pm_ipi_send(primary_master, payload);

	if ((PM_RET_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL);
	else
		return ret;
}

/**
 * XPm_RequestWakeUp() - PM call for master to wake up selected master or subsystem
 * @node	Node id of the master or subsystem
 * @ack		Flag to specify whether acknowledge requested
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_RequestWakeUp(const enum XPmNodeId target,
				 const enum XPmRequestAck ack)
{
	enum XPmStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];
	const struct XPm_Master *master = pm_get_master_by_node(target);

	XPm_ClientWakeup(master);
	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_REQUEST_WAKEUP, target, ack, 0, 0);
	ret = pm_ipi_send(primary_master, payload);

	if ((PM_RET_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL);
	else
		return ret;
}

/**
 * XPm_ForcePowerDown() - PM call to request for another PU or subsystem to
 *			  be powered down forcefully
 * @target	Node id of the targeted PU or subsystem
 * @ack		Flag to specify whether acknowledge is requested
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_ForcePowerDown(const enum XPmNodeId target,
				      const enum XPmRequestAck ack)
{
	enum XPmStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_FORCE_POWERDOWN, target, ack, 0, 0);
	ret = pm_ipi_send(primary_master, payload);

	if ((PM_RET_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL);
	else
		return ret;
}

/**
 * XPm_AbortSuspend() - PM call to announce that a prior suspend request
 *			is to be aborted.
 * @reason	Reason for the abort
 *
 * Calling PU expects the PMU to abort the initiated suspend procedure.
 * This is a non-blocking call without any acknowledge.
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_AbortSuspend(const enum XPmAbortReason reason)
{
	enum XPmStatus status;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_ABORT_SUSPEND, reason, primary_master->node_id, 0, 0);
	status = pm_ipi_send(primary_master, payload);
	if (PM_RET_SUCCESS == status)
		/* Wait for PMU to finish handling request */
		status = pm_ipi_wait(primary_master);
	/*
	 * Do client specific abort suspend operations
	 * (e.g. enable interrupts and clear powerdown request bit)
	 */
	XPm_ClientAbortSuspend();

	return status;
}

/**
 * XPm_SetWakeUpSource() - PM call to specify the wakeup source while suspended
 * @target	Node id of the targeted PU or subsystem
 * @wkup_node	Node id of the wakeup peripheral
 * @enable	Enable or disable the specified peripheral as wake source
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_SetWakeUpSource(const enum XPmNodeId target,
					const enum XPmNodeId wkup_node,
					const u8 enable)
{
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD(payload, PM_SET_WAKEUP_SOURCE, target, wkup_node, enable, 0);
	return pm_ipi_send(primary_master, payload);
}

/**
 * XPm_SystemShutdown() - PM call to request a system shutdown or restart
 * @restart	Shutdown or restart? 0 for shutdown, 1 for restart
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_SystemShutdown(const u8 restart)
{
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD(payload, PM_SYSTEM_SHUTDOWN, restart, 0, 0, 0);
	return pm_ipi_send(primary_master, payload);
}

/* APIs for managing PM slaves: */

/**
 * XPm_RequestNode() - PM call to request a node with specifc capabilities
 * @node	Node id of the slave
 * @capabilities Requested capabilities of the slave
 * @qos		Quality of service (not supported)
 * @ack		Flag to specify whether acknowledge is requested
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_RequestNode(const enum XPmNodeId node,
			       const u32 capabilities,
			       const u32 qos,
			       const enum XPmRequestAck ack)
{
	enum XPmStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD(payload, PM_REQUEST_NODE, node, capabilities, qos, ack);
	ret = pm_ipi_send(primary_master, payload);

	if ((PM_RET_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL);
	else
		return ret;
}

/**
 * XPm_SetRequirement() - PM call to set requirement for PM slaves
 * @node	Node id of the slave
 * @capabilities Requested capabilities of the slave
 * @qos		Quality of service (not supported)
 * @ack		Flag to specify whether acknowledge is requested
 *
 * This API function is to be used for slaves a PU already has requested
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_SetRequirement(const enum XPmNodeId nid,
				      const u32 capabilities,
				      const u32 qos,
				      const enum XPmRequestAck ack)
{
	enum XPmStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD(payload, PM_SET_REQUIREMENT, nid, capabilities, qos, ack);
	ret = pm_ipi_send(primary_master, payload);

	if ((PM_RET_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL);
	else
		return ret;
}

/**
 * XPm_ReleaseNode() - PM call to release a node
 * @node	Node id of the slave
 * @latency	Requested maximum wakeup latency
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_ReleaseNode(const enum XPmNodeId node,
				   const u32 latency)
{
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD(payload, PM_RELEASE_NODE, node, latency, 0, 0);
	return pm_ipi_send(primary_master, payload);
}

/**
 * XPm_SetMaxLatency() - PM call to set wakeup latency requirements
 * @node	Node id of the slave
 * @latency	Requested maximum wakeup latency
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_SetMaxLatency(const enum XPmNodeId node,
				      const u32 latency)
{
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_SET_MAX_LATENCY, node, latency, 0, 0);
	return pm_ipi_send(primary_master, payload);
}

/* Callback API functions */
struct pm_init_suspend pm_susp = {
	.received = false,
/* initialization of other fields is irrelevant while 'received' is false */
};

struct pm_acknowledge pm_ack = {
	.received = false,
/* initialization of other fields is irrelevant while 'received' is false */
};

/**
 * XPm_InitSuspendCb() - callback to handle request made by PMU for initiating
 *			  suspend of this PU. If PU does not suspend itself
 *			  within given timeout, PMU might force it to power down.
 * @reason	Suspend reason
 * @latency	Maximum allowed latency for waking up (determines lowest state)
 * @state	State to which the PU should suspend
 * @timeout	How much time this PU have to suspend itself
 *
 * This function executes in interrupt context. The function itself should never
 * invoke PM API calls that could block (because of performance reasons) or
 * execution of wfi (impossible). Therefore, this function should schedule
 * suspend procedure to be done out of the interrupt context. In this case,
 * it is simply done by filling the structure with arguments of the call and
 * marking that the init suspend request is received.
 */
void XPm_InitSuspendCb(const enum XPmSuspendReason reason,
		       const u32 latency,
		       const u32 state,
		       const u32 timeout)
{
	if (true == pm_susp.received) {
		pm_dbg("WARNING: dropping unhandled init suspend request!\n");
		pm_dbg("Dropped %s (%d, %d, %d, %d)\n", __func__, pm_susp.reason,
			pm_susp.latency, pm_susp.state, pm_susp.timeout);
	}
	pm_dbg("%s (%d, %d, %d, %d)\n", __func__, reason, latency, state, timeout);

	pm_susp.reason = reason;
	pm_susp.latency = latency;
	pm_susp.state = state;
	pm_susp.timeout = timeout;
	pm_susp.received = true;
}

/**
 * XPm_AcknowledgeCb() - callback to handle acknowledge from PMU
 * @node	Node about which the acknowledge is about
 * @status	Acknowledged status
 * @oppoint	Operating point of the node in question
 */
void XPm_AcknowledgeCb(const enum XPmNodeId node,
		       const enum XPmStatus status,
		       const u32 oppoint)
{
	if (true == pm_ack.received) {
		pm_dbg("WARNING: dropping unhandled acknowledge!\n");
		pm_dbg("Dropped %s (%d, %d, %d)\n", __func__, pm_ack.node,
			pm_ack.status, pm_ack.opp);
	}
	pm_dbg("%s (%d, %d, %d)\n", __func__, node, status, oppoint);

	pm_ack.node = node;
	pm_ack.status = status;
	pm_ack.opp = oppoint;
	pm_ack.received = true;
}

void XPm_NotifyCb(const enum XPmNodeId node,
		  const u32 event,
		  const u32 oppoint)
{
	pm_dbg("WARNING: %s (%d, %d, %d) - NO CUSTOM HANDLING\n", __func__,
		node, event, oppoint);
}

/* Miscellaneous API functions */

/**
 * XPm_GetApiVersion() - Get version number of PMU PM firmware
 * @version	Returns 32-bit version number of PMU Power Management Firmware
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_GetApiVersion(u32 *version)
{
	enum XPmStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_GET_API_VERSION, 0, 0, 0, 0);
	ret = pm_ipi_send(primary_master, payload);

	if (PM_RET_SUCCESS != ret)
		return ret;

	/* Return result from IPI return buffer */
	return pm_ipi_buff_read32(primary_master, version);
}


/**
 * XPm_GetNodeStatus() - PM call to request a node's current power state
 * @node	Node id of the slave
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_GetNodeStatus(const enum XPmNodeId node)
{
	/* TODO: Add power state argument!! */
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD(payload, PM_GET_NODE_STATUS, node, 0, 0, 0);
	return pm_ipi_send(primary_master, payload);
}

/**
 * XPm_MmioWrite() - Perform write to protected mmio
 * @address	Address to write to
 * @mask	Mask to apply
 * @value	Value to write
 *
 * This function provides access to PM-related control registers
 * that may not be directly accessible by a particular PU.
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_MmioWrite(const u32 address, const u32 mask,
			     const u32 value)
{
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_MMIO_WRITE, address, mask, value, 0);
	return pm_ipi_send(primary_master, payload);
}

/**
 * XPm_MmioRead() - Read value from protected mmio
 * @address	Address to write to
 * @value	Value to write
 *
 * This function provides access to PM-related control registers
 * that may not be directly accessible by a particular PU.
 *
 * @return	Returns status, either success or error+reason
 */
enum XPmStatus XPm_MmioRead(const u32 address, u32 *const value)
{
	enum XPmStatus status;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD(payload, PM_MMIO_READ, address, 0, 0, 0);
	status = pm_ipi_send(primary_master, payload);

	if (PM_RET_SUCCESS != status)
		return status;

	/* Return result from IPI return buffer */
	return pm_ipi_buff_read32(primary_master, value);
}
