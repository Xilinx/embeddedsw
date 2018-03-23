/******************************************************************************
*
* Copyright (C) 2015-2016 Xilinx, Inc.  All rights reserved.
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
#include "pm_callbacks.h"

/** @name Payload Packets
 *
 * Assigning of argument values into array elements.
 * pause and pm_dbg are used for debugging and should be removed in
 * final version.
 * @{
 */
#define PACK_PAYLOAD(pl, arg0, arg1, arg2, arg3, arg4, arg5)		\
	pl[0] = (u32)arg0;						\
	pl[1] = (u32)arg1;						\
	pl[2] = (u32)arg2;						\
	pl[3] = (u32)arg3;						\
	pl[4] = (u32)arg4;						\
	pl[5] = (u32)arg5;						\
	pm_dbg("%s(%x, %x, %x, %x, %x)\n", __func__, arg1, arg2, arg3, arg4, arg5);

#define PACK_PAYLOAD0(pl, api_id) \
	PACK_PAYLOAD(pl, api_id, 0, 0, 0, 0, 0)
#define PACK_PAYLOAD1(pl, api_id, arg1) \
	PACK_PAYLOAD(pl, api_id, arg1, 0, 0, 0, 0)
#define PACK_PAYLOAD2(pl, api_id, arg1, arg2) \
	PACK_PAYLOAD(pl, api_id, arg1, arg2, 0, 0, 0)
#define PACK_PAYLOAD3(pl, api_id, arg1, arg2, arg3) \
	PACK_PAYLOAD(pl, api_id, arg1, arg2, arg3, 0, 0)
#define PACK_PAYLOAD4(pl, api_id, arg1, arg2, arg3, arg4) \
	PACK_PAYLOAD(pl, api_id, arg1, arg2, arg3, arg4, 0)
#define PACK_PAYLOAD5(pl, api_id, arg1, arg2, arg3, arg4, arg5) \
	PACK_PAYLOAD(pl, api_id, arg1, arg2, arg3, arg4, arg5)
/*@}*/

/****************************************************************************/
/**
 * @brief  Initialize xilpm library
 *
 * @param  IpiInst Pointer to IPI driver instance
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_InitXilpm(XIpiPsu *IpiInst)
{
	XStatus status = XST_SUCCESS;

	if (NULL == IpiInst) {
		pm_dbg("ERROR passing NULL pointer to %s\n", __func__);
		status = XST_INVALID_PARAM;
		goto done;
	}

	XPm_ClientSetPrimaryMaster();

	primary_master->ipi = IpiInst;
done:
	return status;
}

/****************************************************************************/
/**
 * @brief This Function returns information about the boot reason.
 * If the boot is not a system startup but a resume,
 * power down request bitfield for this processor will be cleared.
 *
 * @return Returns processor boot status
 * - PM_RESUME : If the boot reason is because of system resume.
 * - PM_INITIAL_BOOT : If this boot is the initial system startup.
 *
 * @note   None
 *
 ****************************************************************************/
enum XPmBootStatus XPm_GetBootStatus(void)
{
	u32 pwrdn_req;

	XPm_ClientSetPrimaryMaster();

	pwrdn_req = pm_read(primary_master->pwrctl);
	if (0 != (pwrdn_req & primary_master->pwrdn_mask)) {
		pwrdn_req &= ~primary_master->pwrdn_mask;
		pm_write(primary_master->pwrctl, pwrdn_req);
		return PM_RESUME;
	} else {
		return PM_INITIAL_BOOT;
	}
}

/****************************************************************************/
/**
 * @brief  This Function waits for PMU to finish all previous API requests
 * sent by the PU and performs client specific actions to finish suspend
 * procedure (e.g. execution of wfi instruction on A53 and R5 processors).
 *
 * @note   This function should not return if the suspend procedure is
 * successful.
 *
 ****************************************************************************/
void XPm_SuspendFinalize(void)
{
	XStatus status;

	/*
	 * Wait until previous IPI request is handled by the PMU.
	 * If PMU is busy, keep trying until PMU becomes responsive
	 */
	do {
		status = XIpiPsu_PollForAck(primary_master->ipi,
					    IPI_PMU_PM_INT_MASK,
					    PM_IPI_TIMEOUT);
		if (status != XST_SUCCESS) {
			pm_dbg("ERROR timed out while waiting for PMU to"
			       " finish processing previous PM-API call\n");
		}
	} while (XST_SUCCESS != status);

	XPm_ClientSuspendFinalize();
}

/****************************************************************************/
/**
 * @brief  Sends IPI request to the PMU
 *
 * @param  master  Pointer to the master who is initiating request
 * @param  payload API id and call arguments to be written in IPI buffer
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus pm_ipi_send(struct XPm_Master *const master,
			   u32 payload[PAYLOAD_ARG_CNT])
{
	XStatus status;

	status = XIpiPsu_PollForAck(master->ipi, IPI_PMU_PM_INT_MASK,
				    PM_IPI_TIMEOUT);
	if (status != XST_SUCCESS) {
		pm_dbg("%s: ERROR: Timeout expired\n", __func__);
		goto done;
	}

	status = XIpiPsu_WriteMessage(master->ipi, IPI_PMU_PM_INT_MASK,
				      payload, PAYLOAD_ARG_CNT,
				      XIPIPSU_BUF_TYPE_MSG);
	if (status != XST_SUCCESS) {
		pm_dbg("xilpm: ERROR writing to IPI request buffer\n");
		goto done;
	}

	status = XIpiPsu_TriggerIpi(master->ipi, IPI_PMU_PM_INT_MASK);
done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Reads IPI response after PMU has handled interrupt
 *
 * @param  master Pointer to the master who is waiting and reading response
 * @param  value1 Used to return value from 2nd IPI buffer element (optional)
 * @param  value2 Used to return value from 3rd IPI buffer element (optional)
 * @param  value3 Used to return value from 4th IPI buffer element (optional)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus pm_ipi_buff_read32(struct XPm_Master *const master,
				  u32 *value1, u32 *value2, u32 *value3)
{
	u32 response[RESPONSE_ARG_CNT];
	XStatus status;

	/* Wait until current IPI interrupt is handled by PMU */
	status = XIpiPsu_PollForAck(master->ipi, IPI_PMU_PM_INT_MASK,
				  PM_IPI_TIMEOUT);

	if (status != XST_SUCCESS) {
		pm_dbg("%s: ERROR: Timeout expired\n", __func__);
		goto done;
	}

	status = XIpiPsu_ReadMessage(master->ipi, IPI_PMU_PM_INT_MASK,
				     response, RESPONSE_ARG_CNT,
				     XIPIPSU_BUF_TYPE_RESP);

	if (status != XST_SUCCESS) {
		pm_dbg("xilpm: ERROR reading from PMU's IPI response buffer\n");
		goto done;
	}

	/*
	 * Read response from IPI buffer
	 * buf-0: success or error+reason
	 * buf-1: value1
	 * buf-2: value2
	 * buf-3: value3
	 */
	if (NULL != value1)
		*value1 = response[1];
	if (NULL != value2)
		*value2 = response[2];
	if (NULL != value3)
		*value3 = response[3];

	status = response[0];
done:
	return status;
}

/****************************************************************************/
/**
 * @brief  This function is used by a CPU to declare that it is about to
 * suspend itself. After the PMU processes this call it will wait for the
 * requesting CPU to complete the suspend procedure and become ready to be
 * put into a sleep state.
 *
 * @param  nid     Node ID of the CPU node to be suspended.
 * @param  latency Maximum wake-up latency requirement in us(microsecs)
 * @param  state   Instead of specifying a maximum latency, a CPU can also
 * explicitly request a certain power state.
 * @param  address Address from which to resume when woken up.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This is a blocking call, it will return only once PMU has responded
 *
 ****************************************************************************/
XStatus XPm_SelfSuspend(const enum XPmNodeId nid,
			const u32 latency,
			const u8 state,
			const u64 address)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	struct XPm_Master *master = pm_get_master_by_node(nid);
	if (NULL == master) {
		/*
		 * If a subsystem node ID (APU or RPU) was passed then
		 * the master to be used is the primary master.
		 * E.g. for the APU the primary master is APU0
		 */
		if (subsystem_node == nid) {
			master = primary_master;
		} else {
			return XST_INVALID_PARAM;
		}
	}
	/*
	 * Do client specific suspend operations
	 * (e.g. disable interrupts and set powerdown request bit)
	 */
	XPm_ClientSuspend(master);

	/* Send request to the PMU */
	PACK_PAYLOAD5(payload, PM_SELF_SUSPEND, nid, latency, state, (u32)address,
		     (u32)(address >> 32));
	ret = pm_ipi_send(master, payload);
	if (XST_SUCCESS != ret)
		return ret;
	/* Wait for PMU to finish handling request */
	return pm_ipi_buff_read32(master, NULL, NULL, NULL);
}

/****************************************************************************/
/**
 * @brief  This function is called to configure the power management
 * framework. The call triggers power management controller to load the
 * configuration object and configure itself according to the content of the
 * object.
 *
 * @param  address Start address of the configuration object
 *
 * @return XST_SUCCESS if successful, otherwise an error code
 *
 * @note   The provided address must be in 32-bit address space which is
 * accessible by the PMU.
 *
 ****************************************************************************/
XStatus XPm_SetConfiguration(const u32 address)
{
	XStatus status;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD1(payload, PM_SET_CONFIGURATION, address);
	status = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != status)
		return status;

	return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
}

/****************************************************************************/
/**
 * @brief  This function is called to notify the power management controller
 * about the completed power management initialization.
 *
 * @return XST_SUCCESS if successful, otherwise an error code
 *
 * @note   It is assumed that all used nodes are requested when this call is
 * made. The power management controller may power down the nodes which are
 * not requested after this call is processed.
 *
 ****************************************************************************/
XStatus XPm_InitFinalize(void)
{
	XStatus status;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD0(payload, PM_INIT_FINALIZE);
	status = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != status)
		return status;

	return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
}

/****************************************************************************/
/**
 * @brief  This function is used by a PU to request suspend of another PU.
 * This call triggers the power management controller to notify the PU
 * identified by 'nodeID' that a suspend has been requested. This will
 * allow said PU to gracefully suspend itself by calling XPm_SelfSuspend
 * for each of its CPU nodes, or else call XPm_AbortSuspend with its PU
 * node as argument and specify the reason.
 *
 * @param  target  Node ID of the PU node to be suspended
 * @param  ack     Requested acknowledge type. REQUEST_ACK_BLOCKING is not
 * supported.
 * @param  latency Maximum wake-up latency requirement in us(micro sec)
 * @param  state   Instead of specifying a maximum latency, a PU can
 * also explicitly request a certain power state.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If 'ack' is set to PM_ACK_NON_BLOCKING, the requesting PU will
 * be notified upon completion of suspend or if an error occurred,
 * such as an abort. REQUEST_ACK_BLOCKING is not supported for this command.
 *
 ****************************************************************************/
XStatus XPm_RequestSuspend(const enum XPmNodeId target,
			   const enum XPmRequestAck ack,
			   const u32 latency, const u8 state)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD4(payload, PM_REQUEST_SUSPEND, target, ack, latency, state);
	ret = pm_ipi_send(primary_master, payload);

	if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	else
		return ret;
}

/****************************************************************************/
/**
 * @brief  This function can be used to request power up of a CPU node
 * within the same PU, or to power up another PU.
 *
 * @param  target     Node ID of the CPU or PU to be powered/woken up.
 * @param  setAddress Specifies whether the start address argument is being passed.
 * - 0 : do not set start address
 * - 1 : set start address
 * @param  address    Address from which to resume when woken up.
 * Will only be used if set_address is 1.
 * @param  ack        Requested acknowledge type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If acknowledge is requested, the calling PU will be notified
 * by the power management controller once the wake-up is completed.
 *
 ****************************************************************************/
XStatus XPm_RequestWakeUp(const enum XPmNodeId target,
			  const bool setAddress,
			  const u64 address,
			  const enum XPmRequestAck ack)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];
	u64 encodedAddress;
	struct XPm_Master *master = pm_get_master_by_node(target);

	XPm_ClientWakeup(master);

	/* encode set Address into 1st bit of address */
	encodedAddress = address | !!setAddress;

	/* Send request to the PMU */
	PACK_PAYLOAD4(payload, PM_REQUEST_WAKEUP, target, (u32)encodedAddress,
		     (u32)(encodedAddress >> 32), ack);
	ret = pm_ipi_send(primary_master, payload);

	if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	else
		return ret;
}

/****************************************************************************/
/**
 * @brief  One PU can request a forced poweroff of another PU or its power
 * island or power domain. This can be used for killing an unresponsive PU,
 * in which case all resources of that PU will be automatically released.
 *
 * @param  target Node ID of the PU node or power island/domain to be
 * powered down.
 * @param  ack    Requested acknowledge type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Force power down may not be requested by a PU for itself.
 *
 ****************************************************************************/
XStatus XPm_ForcePowerDown(const enum XPmNodeId target,
			   const enum XPmRequestAck ack)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD2(payload, PM_FORCE_POWERDOWN, target, ack);
	ret = pm_ipi_send(primary_master, payload);

	if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	else
		return ret;
}

/****************************************************************************/
/**
 * @brief  This function is called by a CPU after a XPm_SelfSuspend call to
 * notify the power management controller that CPU has aborted suspend
 * or in response to an init suspend request when the PU refuses to suspend.
 *
 * @param  reason Reason code why the suspend can not be performed or completed
 * - ABORT_REASON_WKUP_EVENT : local wakeup-event received
 * - ABORT_REASON_PU_BUSY : PU is busy
 * - ABORT_REASON_NO_PWRDN : no external powerdown supported
 * - ABORT_REASON_UNKNOWN : unknown error during suspend procedure
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Calling PU expects the PMU to abort the initiated suspend procedure.
 * This is a non-blocking call without any acknowledge.
 *
 ****************************************************************************/
XStatus XPm_AbortSuspend(const enum XPmAbortReason reason)
{
	XStatus status;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD2(payload, PM_ABORT_SUSPEND, reason, primary_master->node_id);
	status = pm_ipi_send(primary_master, payload);
	if (XST_SUCCESS == status) {
		/* Wait for PMU to finish handling request */
		status = XIpiPsu_PollForAck(primary_master->ipi,
					IPI_PMU_PM_INT_MASK, PM_IPI_TIMEOUT);
		if (status != XST_SUCCESS) {
			pm_dbg("%s: ERROR: Timeout expired\n", __func__);
		}
	}

	/*
	 * Do client specific abort suspend operations
	 * (e.g. enable interrupts and clear powerdown request bit)
	 */
	XPm_ClientAbortSuspend();

	return status;
}

/****************************************************************************/
/**
 * @brief  This function is called by a PU to add or remove a wake-up source
 * prior to going to suspend. The list of wake sources for a PU is
 * automatically cleared whenever the PU is woken up or when one of its
 * CPUs aborts the suspend procedure.
 *
 * @param  target    Node ID of the target to be woken up.
 * @param  wkup_node Node ID of the wakeup device.
 * @param  enable    Enable flag:
 * - 1 : the wakeup source is added to the list
 * - 0 : the wakeup source is removed from the list
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Declaring a node as a wakeup source will ensure that the node
 * will not be powered off. It also will cause the PMU to configure the
 * GIC Proxy accordingly if the FPD is powered off.
 *
 ****************************************************************************/
XStatus XPm_SetWakeUpSource(const enum XPmNodeId target,
			    const enum XPmNodeId wkup_node,
			    const u8 enable)
{
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD3(payload, PM_SET_WAKEUP_SOURCE, target, wkup_node, enable);
	return pm_ipi_send(primary_master, payload);
}

/****************************************************************************/
/**
 * @brief  This function can be used by a privileged PU to shut down
 * or restart the complete device.
 *
 * @param  restart Should the system be restarted automatically?
 * - PM_SHUTDOWN : no restart requested, system will be powered off permanently
 * - PM_RESTART : restart is requested, system will go through a full reset
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   In either case the PMU will call XPm_InitSuspendCb for each of
 * the other PUs, allowing them to gracefully shut down. If a PU is asleep
 * it will be woken up by the PMU. The PU making the XPm_SystemShutdown
 * should perform its own suspend procedure after calling this API. It will
 * not receive an init suspend callback.
 *
 ****************************************************************************/
XStatus XPm_SystemShutdown(u32 type, u32 subtype)
{
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD2(payload, PM_SYSTEM_SHUTDOWN, type, subtype);
	return pm_ipi_send(primary_master, payload);
}

/* APIs for managing PM slaves */

/****************************************************************************/
/**
 * @brief  Used to request the usage of a PM-slave. Using this API call a PU
 * requests access to a slave device and asserts its requirements on that
 * device. Provided the PU is sufficiently privileged, the PMU will enable
 * access to the memory mapped region containing the control registers of
 * that device. For devices that can only be serving a single PU, any other
 * privileged PU will now be blocked from accessing this device until the
 * node is released.
 *
 * @param  node         Node ID of the PM slave requested
 * @param  capabilities Slave-specific capabilities required, can be combined
 * - PM_CAP_ACCESS : full access / functionality
 * - PM_CAP_CONTEXT : preserve context
 * - PM_CAP_WAKEUP : emit wake interrupts
 * @param  qos          Quality of Service (0-100) required
 * @param  ack          Requested acknowledge type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_RequestNode(const enum XPmNodeId node,
			const u32 capabilities,
			const u32 qos,
			const enum XPmRequestAck ack)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD4(payload, PM_REQUEST_NODE, node, capabilities, qos, ack);
	ret = pm_ipi_send(primary_master, payload);

	if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	else
		return ret;
}

/****************************************************************************/
/**
 * @brief  This function is used by a PU to announce a change in requirements
 * for a specific slave node which is currently in use.
 *
 * @param  nid          Node ID of the PM slave.
 * @param  capabilities Slave-specific capabilities required.
 * @param  qos          Quality of Service (0-100) required.
 * @param  ack          Requested acknowledge type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If this function is called after the last awake CPU within the PU
 * calls SelfSuspend, the requirement change shall be performed after the CPU
 * signals the end of suspend to the power management controller,
 * (e.g. WFI interrupt).
 *
 ****************************************************************************/
XStatus XPm_SetRequirement(const enum XPmNodeId nid,
			   const u32 capabilities,
			   const u32 qos,
			   const enum XPmRequestAck ack)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD4(payload, PM_SET_REQUIREMENT, nid, capabilities, qos, ack);
	ret = pm_ipi_send(primary_master, payload);

	if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack))
		return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	else
		return ret;
}

/****************************************************************************/
/**
 * @brief  This function is used by a PU to release the usage of a PM slave.
 * This will tell the power management controller that the node is no longer
 * needed by that PU, potentially allowing the node to be placed into an
 * inactive state.
 *
 * @param  node Node ID of the PM slave.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_ReleaseNode(const enum XPmNodeId node)
{
	u32 payload[PAYLOAD_ARG_CNT];
	PACK_PAYLOAD1(payload, PM_RELEASE_NODE, node);
	return pm_ipi_send(primary_master, payload);
}

/****************************************************************************/
/**
 * @brief  This function is used by a PU to announce a change in the maximum
 * wake-up latency requirements for a specific slave node currently used by
 * that PU.
 *
 * @param  node    Node ID of the PM slave.
 * @param  latency Maximum wake-up latency required.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Setting maximum wake-up latency can constrain the set of possible
 * power states a resource can be put into.
 *
 ****************************************************************************/
XStatus XPm_SetMaxLatency(const enum XPmNodeId node,
			  const u32 latency)
{
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD2(payload, PM_SET_MAX_LATENCY, node, latency);
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

/****************************************************************************/
/**
 * @brief  Callback function to be implemented in each PU, allowing the power
 * management controller to request that the PU suspend itself.
 *
 * @param  reason  Suspend reason:
 * - SUSPEND_REASON_PU_REQ : Request by another PU
 * - SUSPEND_REASON_ALERT : Unrecoverable SysMon alert
 * - SUSPEND_REASON_SHUTDOWN : System shutdown
 * - SUSPEND_REASON_RESTART : System restart
 * @param  latency Maximum wake-up latency in us(micro secs). This information
 * can be used by the PU to decide what level of context saving may be
 * required.
 * @param  state   Targeted sleep/suspend state.
 * @param  timeout Timeout in ms, specifying how much time a PU has to initiate
 * its suspend procedure before it's being considered unresponsive.
 *
 * @return None
 *
 * @note   If the PU fails to act on this request the power management
 * controller or the requesting PU may choose to employ the forceful
 * power down option.
 *
 ****************************************************************************/
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

/****************************************************************************/
/**
 * @brief  This function is called by the power management controller in
 * response to any request where an acknowledge callback was requested,
 * i.e. where the 'ack' argument passed by the PU was REQUEST_ACK_NON_BLOCKING.
 *
 * @param  node    ID of the component or sub-system in question.
 * @param  status  Status of the operation:
 * - OK: the operation completed successfully
 * - ERR: the requested operation failed
 * @param  oppoint Operating point of the node in question
 *
 * @return None
 *
 * @note   None
 *
 ****************************************************************************/
void XPm_AcknowledgeCb(const enum XPmNodeId node,
		       const XStatus status,
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

/****************************************************************************/
/**
 * @brief  This function is called by the power management controller if an
 * event the PU was registered for has occurred. It will populate the notifier
 * data structure passed when calling XPm_RegisterNotifier.
 *
 * @param  node    ID of the node the event notification is related to.
 * @param  event   ID of the event
 * @param  oppoint Current operating state of the node.
 *
 * @return None
 *
 * @note   None
 *
 ****************************************************************************/
void XPm_NotifyCb(const enum XPmNodeId node,
		  const u32 event,
		  const u32 oppoint)
{
	pm_dbg("%s (%d, %d, %d)\n", __func__, node, event, oppoint);
	XPm_NotifierProcessEvent(node, event, oppoint);
}

/* Miscellaneous API functions */

/****************************************************************************/
/**
 * @brief  This function is used to request the version number of the API
 * running on the power management controller.
 *
 * @param  version Returns the API 32-bit version number.
 * Returns 0 if no PM firmware present.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetApiVersion(u32 *version)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD0(payload, PM_GET_API_VERSION);
	ret = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != ret)
		return ret;

	/* Return result from IPI return buffer */
	return pm_ipi_buff_read32(primary_master, version, NULL, NULL);
}

/****************************************************************************/
/**
 * @brief  This function is used to obtain information about the current state
 * of a component. The caller must pass a pointer to an XPm_NodeStatus
 * structure, which must be pre-allocated by the caller.
 *
 * @param  node       ID of the component or sub-system in question.
 * @param  nodestatus Used to return the complete status of the node.
 *
 * - status - The current power state of the requested node.
 *  - For CPU nodes:
 *   - 0 : if CPU is powered down,
 *   - 1 : if CPU is active (powered up),
 *   - 2 : if CPU is suspending (powered up)
 *  - For power islands and power domains:
 *   - 0 : if island is powered down,
 *   - 1 : if island is powered up
 *  - For PM slaves:
 *   - 0 : if slave is powered down,
 *   - 1 : if slave is powered up,
 *   - 2 : if slave is in retention
 *
 * - requirement - Slave nodes only: Returns current requirements the
 * requesting PU has requested of the node.
 *
 * - usage - Slave nodes only: Returns current usage status of the node:
 *  - 0 : node is not used by any PU,
 *  - 1 : node is used by caller exclusively,
 *  - 2 : node is used by other PU(s) only,
 *  - 3 : node is used by caller and by other PU(s)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetNodeStatus(const enum XPmNodeId node,
			  XPm_NodeStatus *const nodestatus)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD1(payload, PM_GET_NODE_STATUS, node);
	ret = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != ret)
		return ret;

	/* Return result from IPI return buffer */
	return pm_ipi_buff_read32(primary_master, &nodestatus->status,
				  &nodestatus->requirements,
				  &nodestatus->usage);
}

/****************************************************************************/
/**
 * @brief  Call this function to request the power management controller to
 * return information about an operating characteristic of a component.
 *
 * @param  node   ID of the component or sub-system in question.
 * @param  type   Type of operating characteristic requested:
 * - power (current power consumption),
 * - latency (current latency in us to return to active state),
 * - temperature (current temperature),
 * @param  result Used to return the requested operating characteristic.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetOpCharacteristic(const enum XPmNodeId node,
				const enum XPmOpCharType type,
				u32* const result)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD2(payload, PM_GET_OP_CHARACTERISTIC, node, type);
	ret = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != ret)
		return ret;

	/* Return result from IPI return buffer */
	return pm_ipi_buff_read32(primary_master, result, NULL, NULL);
}

/****************************************************************************/
/**
 * @brief  This function is used to assert or release reset for a particular
 * reset line. Alternatively a reset pulse can be requested as well.
 *
 * @param  reset  ID of the reset line
 * @param  assert Identifies action:
 * - PM_RESET_ACTION_RELEASE : release reset,
 * - PM_RESET_ACTION_ASSERT : assert reset,
 * - PM_RESET_ACTION_PULSE : pulse reset,
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_ResetAssert(const enum XPmReset reset,
			const enum XPmResetAction assert)
{
       XStatus ret;
       u32 payload[PAYLOAD_ARG_CNT];

       /* Send request to the PMU */
       PACK_PAYLOAD2(payload, PM_RESET_ASSERT, reset, assert);
       ret = pm_ipi_send(primary_master, payload);

       if (XST_SUCCESS != ret)
               return ret;

       /* Return result from IPI return buffer */
       return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
}

/****************************************************************************/
/**
 * @brief  Call this function to get the current status of the selected
 * reset line.
 *
 * @param  reset  Reset line
 * @param  status Status of specified reset (true - asserted, false - released)
 *
 * @return Returns 1/XST_FAILURE for 'asserted' or
 * 0/XST_SUCCESS for 'released'.
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_ResetGetStatus(const enum XPmReset reset, u32 *status)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD1(payload, PM_RESET_GET_STATUS, reset);
	ret = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != ret)
		return ret;

	/* Return result from IPI return buffer */
	return pm_ipi_buff_read32(primary_master, status, NULL, NULL);
}

/****************************************************************************/
/**
 * @brief  A PU can call this function to request that the power management
 * controller call its notify callback whenever a qualifying event occurs.
 * One can request to be notified for a specific or any event related to
 * a specific node.
 *
 * @param  notifier Pointer to the notifier object to be associated with
 * the requested notification. The notifier object contains the following
 * data related to the notification:
 *
 * - nodeID : ID of the node to be notified about,
 *
 * - eventID : ID of the event in question, '-1' denotes all events
 * ( - EVENT_STATE_CHANGE, EVENT_ZERO_USERS, EVENT_ERROR_CONDITION),
 *
 * - wake : true: wake up on event, false: do not wake up
 * (only notify if awake), no buffering/queueing
 *
 * - callback : Pointer to the custom callback function to be called when the
 * notification is available. The callback executes from interrupt context,
 * so the user must take special care when implementing the callback.
 * Callback is optional, may be set to NULL.
 *
 * - received : Variable indicating how many times the notification has been
 * received since the notifier is registered.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   The caller shall initialize the notifier object before invoking
 * the XPm_RegisteredNotifier function. While notifier is registered,
 * the notifier object shall not be modified by the caller.
 *
 ****************************************************************************/
XStatus XPm_RegisterNotifier(XPm_Notifier* const notifier)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	if (!notifier) {
		pm_dbg("%s ERROR: NULL notifier pointer\n", __func__);
		return XST_INVALID_PARAM;
	}

	/* Send request to the PMU */
	PACK_PAYLOAD4(payload, PM_REGISTER_NOTIFIER, notifier->node,
		      notifier->event, notifier->flags, 1);
	ret = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != ret)
		return ret;

	ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);

	if (XST_SUCCESS != ret)
		return ret;

	/* Add notifier in the list only if PMU has it registered */
	return XPm_NotifierAdd(notifier);
}

/****************************************************************************/
/**
 * @brief  A PU calls this function to unregister for the previously
 * requested notifications.
 *
 * @param  notifier Pointer to the notifier object associated with the
 * previously requested notification
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_UnregisterNotifier(XPm_Notifier* const notifier)
{
	XStatus ret;
	u32 payload[PAYLOAD_ARG_CNT];

	if (!notifier) {
		pm_dbg("%s ERROR: NULL notifier pointer\n", __func__);
		return XST_INVALID_PARAM;
	}

	/*
	 * Remove first the notifier from the list. If it's not in the list
	 * report an error, and don't trigger PMU since it don't have it
	 * registered either.
	 */
	ret = XPm_NotifierRemove(notifier);
	if (XST_SUCCESS != ret)
		return ret;

	/* Send request to the PMU */
	PACK_PAYLOAD4(payload, PM_REGISTER_NOTIFIER, notifier->node,
		      notifier->event, 0, 0);
	ret = pm_ipi_send(primary_master, payload);

	return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
}

/* Direct Control API Functions */
/****************************************************************************/
/**
 * @brief  Call this function to write a value directly into a register that
 * isn't accessible directly, such as registers in the clock control unit.
 * This call is bypassing the power management logic. The permitted addresses
 * are subject to restrictions as defined in the PCW configuration.
 *
 * @param  address Physical 32-bit address of memory mapped register to write
 * to.
 * @param  mask    32-bit value used to limit write to specific bits in the
 * register.
 * @param  value   Value to write to the register bits specified by the mask.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_MmioWrite(const u32 address, const u32 mask, const u32 value)
{
	XStatus status;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD3(payload, PM_MMIO_WRITE, address, mask, value);
	status = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != status)
		return status;

	/* Return result from IPI return buffer */
	return pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
}

/****************************************************************************/
/**
 * @brief  Call this function to read a value from a register that isn't
 * accessible directly. The permitted addresses are subject to restrictions
 * as defined in the PCW configuration.
 *
 * @param  address Physical 32-bit address of memory mapped register to
 * read from.
 * @param  value   Returns the 32-bit value read from the register
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_MmioRead(const u32 address, u32 *const value)
{
	XStatus status;
	u32 payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PACK_PAYLOAD1(payload, PM_MMIO_READ, address);
	status = pm_ipi_send(primary_master, payload);

	if (XST_SUCCESS != status)
		return status;

	/* Return result from IPI return buffer */
	return pm_ipi_buff_read32(primary_master, value, NULL, NULL);
}
