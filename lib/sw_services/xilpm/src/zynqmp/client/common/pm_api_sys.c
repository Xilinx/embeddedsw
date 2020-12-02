/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file pm_api_sys.c
 *
 * PM Definitions implementation
 * @addtogroup xpm_apis XilPM APIs
 * @{
 *****************************************************************************/
#include "pm_client.h"
#include "pm_common.h"
#include "pm_api_sys.h"
#include "pm_callbacks.h"
#include "pm_clock.h"

/** @name Payload Packets
 *
 * Assigning of argument values into array elements.
 * pause and pm_dbg are used for debugging and should be removed in
 * final version.
 * @{
 */
#define PACK_PAYLOAD(pl, arg0, arg1, arg2, arg3, arg4, arg5, rsvd)		\
	pl[0] = (u32)(arg0);						\
	pl[1] = (u32)(arg1);						\
	pl[2] = (u32)(arg2);						\
	pl[3] = (u32)(arg3);						\
	pl[4] = (u32)(arg4);						\
	pl[5] = (u32)(arg5);						\
	pl[6] = (u32)(rsvd);						\
	pm_dbg("%s(%x, %x, %x, %x, %x, %x)\n", (__func__), (arg1), (arg2), (arg3), (arg4), (arg5), (rsvd));

#define PACK_PAYLOAD0(pl, api_id) \
	PACK_PAYLOAD(pl, (api_id), 0U, 0U, 0U, 0U, 0U, 0U)
#define PACK_PAYLOAD1(pl, api_id, arg1) \
	PACK_PAYLOAD(pl, (api_id), (arg1), 0U, 0U, 0U, 0U, 0U)
#define PACK_PAYLOAD2(pl, api_id, arg1, arg2) \
	PACK_PAYLOAD(pl, (api_id), (arg1), (arg2), 0U, 0U, 0U, 0U)
#define PACK_PAYLOAD3(pl, api_id, arg1, arg2, arg3) \
	PACK_PAYLOAD(pl, (api_id), (arg1), (arg2), (arg3), 0U, 0U, 0U)
#define PACK_PAYLOAD4(pl, api_id, arg1, arg2, arg3, arg4) \
	PACK_PAYLOAD(pl, (api_id), (arg1), (arg2), (arg3), (arg4), 0U, 0U)
#define PACK_PAYLOAD5(pl, api_id, arg1, arg2, arg3, arg4, arg5) \
	PACK_PAYLOAD(pl, (api_id), (arg1), (arg2), (arg3), (arg4), (arg5), 0U)
/**@}*/

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
	XStatus status = (XStatus)XST_FAILURE;

	if (NULL == IpiInst) {
		pm_dbg("ERROR passing NULL pointer to %s\n", __func__);
		status = (XStatus)XST_INVALID_PARAM;
		goto done;
	}

	XPm_ClientSetPrimaryMaster();

	if (NULL != primary_master) {
		primary_master->ipi = IpiInst;
		status = (XStatus)XST_SUCCESS;
	}
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
	enum XPmBootStatus ret;

	XPm_ClientSetPrimaryMaster();
	/* Error out if primary master is not defined */
	if (NULL == primary_master) {
		ret = PM_BOOT_ERROR;
		goto done;
	}

	pwrdn_req = pm_read(primary_master->pwrctl);
	if (0U != (pwrdn_req & primary_master->pwrdn_mask)) {
		pwrdn_req &= ~primary_master->pwrdn_mask;
		pm_write(primary_master->pwrctl, pwrdn_req);
		ret = PM_RESUME;
		goto done;
	} else {
		ret = PM_INITIAL_BOOT;
		goto done;
	}

done:
	return ret;
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
	XStatus status = (XStatus)XST_FAILURE;

	/*
	 * Wait until previous IPI request is handled by the PMU.
	 * If PMU is busy, keep trying until PMU becomes responsive
	 */
	do {
		status = XIpiPsu_PollForAck(primary_master->ipi,
					    IPI_PMU_PM_INT_MASK,
					    PM_IPI_TIMEOUT);
		if (status != XST_SUCCESS) {
			pm_dbg("%s: ERROR timed out while waiting for PMU to"
			       " finish processing previous PM-API call\n", __func__);
		}
	} while (XST_SUCCESS != status);

	XPm_ClientSuspendFinalize();
}

#ifdef ENABLE_IPI_CRC
/*****************************************************************************/
/**
*
* This function calculates the CRC for the data
*
* @param	BufAddr - buffer on which CRC is calculated
* @param	BufSize - size of the buffer
*
* @return	Checksum - 16 bit CRC value
*
* @note		None.
*
******************************************************************************/
static u32 XPm_CalculateCRC(u32 BufAddr, u32 BufSize)
{
	const u32 CrcInit = 0x4F4EU;
	const u32 Order = 16U;
	const u32 Polynom = 0x8005U;
	u32 i;
	u32 j;
	u32 c;
	u32 Bit;
	u32 Crc = CrcInit;
	u32 DataIn;
	u32 CrcMask, CrcHighBit;

	CrcMask = ((u32)(((u32)1 << (Order - (u32)1)) -(u32)1) << (u32)1) | (u32)1;
	CrcHighBit = (u32)((u32)1 << (Order - (u32)1));
	for(i = 0U; i < BufSize; i++) {
		DataIn = Xil_In8(BufAddr + i);
		c = (u32)DataIn;
		j = 0x80U;
		while(0U != j) {
			Bit = Crc & CrcHighBit;
			Crc <<= 1U;
			if(0U != (c & j)) {
				Bit ^= CrcHighBit;
			}
			if(0U != Bit) {
				Crc ^= Polynom;
			}
			j >>= 1U;
		}
		Crc &= CrcMask;
	}
	return Crc;
}
#endif

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
	XStatus status = (XStatus)XST_FAILURE;

	status = XIpiPsu_PollForAck(master->ipi, IPI_PMU_PM_INT_MASK,
				    PM_IPI_TIMEOUT);
	if (status != XST_SUCCESS) {
		pm_dbg("%s: ERROR: Timeout expired\n", __func__);
		goto done;
	}

#ifdef ENABLE_IPI_CRC
	/*
	 * Note : The last word payload[7] in IPI Msg is reserved for CRC.
	 * This is only for safety applications.
	 */
	payload[7] = XPm_CalculateCRC((UINTPTR)payload, IPI_W0_TO_W6_SIZE);

#endif

	status = XIpiPsu_WriteMessage(master->ipi, IPI_PMU_PM_INT_MASK,
				      payload, PAYLOAD_ARG_CNT,
				      XIPIPSU_BUF_TYPE_MSG);
	if (status != XST_SUCCESS) {
		pm_dbg("%s: xilpm: ERROR writing to IPI request buffer\n", __func__);
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
	u32 response[RESPONSE_ARG_CNT] = {0U};
	XStatus status = (XStatus)XST_FAILURE;

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
		pm_dbg("%s xilpm: ERROR reading from PMU's IPI response buffer\n", __func__);
		goto done;
	}

#ifdef ENABLE_IPI_CRC
	/*
	 * Note : The last word response[7] in IPI Msg is reserved for CRC.
	 * Compute the CRC and compare.
	 * This is only for safety applications.
	 */
	if (response[7] != XPm_CalculateCRC((UINTPTR)response, IPI_W0_TO_W6_SIZE)) {
		pm_dbg("%s: xilpm: ERROR IPI buffer CRC mismatch\n", __func__);
		status = (XStatus)XST_FAILURE;
		goto done;
	}
#endif

	/*
	 * Read response from IPI buffer
	 * buf-0: success or error+reason
	 * buf-1: value1
	 * buf-2: value2
	 * buf-3: value3
	 */
	if (NULL != value1) {
		*value1 = response[1];
	}
	if (NULL != value2) {
		*value2 = response[2];
	}
	if (NULL != value3) {
		*value3 = response[3];
	}

	status = (XStatus)response[0];
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
	XStatus ret = (XStatus)XST_FAILURE;
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
			ret = (XStatus)XST_INVALID_PARAM;
			goto done;
		}
	}
	/*
	 * Do client specific suspend operations
	 * (e.g. disable interrupts and set powerdown request bit)
	 */
	XPm_ClientSuspend(master);

	/* Send request to the PMU */
	PACK_PAYLOAD5(payload, PM_SELF_SUSPEND, nid, latency, state, (u32)address,
		     (u32)(address >> 32U));
	ret = pm_ipi_send(master, payload);
	if (XST_SUCCESS != ret) {
		goto done;
	}
	/* Wait for PMU to finish handling request */
	ret = pm_ipi_buff_read32(master, NULL, NULL, NULL);

done:
	return ret;
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_SET_CONFIGURATION, address);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Wait for PMU to finish handling request */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD0(payload, PM_INIT_FINALIZE);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Wait for PMU to finish handling request */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
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
 * @param  ack     Requested acknowledge type
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD4(payload, PM_REQUEST_SUSPEND, target, ack, latency, state);
		ret = pm_ipi_send(primary_master, payload);

		if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack)) {
			ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
		}
	}

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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];
	u64 encodedAddress;
	struct XPm_Master *master = pm_get_master_by_node(target);

	XPm_ClientWakeup(master);

	/* encode set Address into 1st bit of address */
	encodedAddress = address | !!setAddress;

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD4(payload, PM_REQUEST_WAKEUP, target, (u32)encodedAddress,
				 (u32)(encodedAddress >> 32), ack);
		ret = pm_ipi_send(primary_master, payload);

		if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack)) {
			ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
		}
	}
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_FORCE_POWERDOWN, target, ack);
		ret = pm_ipi_send(primary_master, payload);

		if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack)) {
			ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
		}
	}
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL == primary_master) {
		status = (XStatus)XST_FAILURE;
		goto done;
	}

	/* Send request to the PMU */
	PACK_PAYLOAD2(payload, PM_ABORT_SUSPEND, reason, primary_master->node_id);
	status = pm_ipi_send(primary_master, payload);
	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Check the response from PMU */
	status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	if (XST_SUCCESS != status) {
		goto done;
	}

	/*
	 * Do client specific abort suspend operations
	 * (e.g. enable interrupts and clear powerdown request bit)
	 */
	XPm_ClientAbortSuspend();

done:
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD3(payload, PM_SET_WAKEUP_SOURCE, target, wkup_node, enable);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Read the result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_SYSTEM_SHUTDOWN, type, subtype);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Read the result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		PACK_PAYLOAD4(payload, PM_REQUEST_NODE, node, capabilities, qos, ack);
		ret = pm_ipi_send(primary_master, payload);

		if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack)) {
			ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
		}
	}
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		PACK_PAYLOAD4(payload, PM_SET_REQUIREMENT, nid, capabilities, qos, ack);
		ret = pm_ipi_send(primary_master, payload);

		if ((XST_SUCCESS == ret) && (REQUEST_ACK_BLOCKING == ack)) {
			ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
		}
	}
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_RELEASE_NODE, node);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Read the result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_SET_MAX_LATENCY, node, latency);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Read the result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
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
		pm_dbg("%s: WARNING: dropping unhandled init suspend request!\n", __func__);
		pm_dbg("Dropped %s (%d, %d, %d, %d)\n", __func__, pm_susp.reason,
			pm_susp.latency, pm_susp.state, pm_susp.timeout);
	}
	pm_dbg("%s (%d, %d, %d, %d)\n", __func__, reason, latency, state, timeout);

	pm_susp.reason = reason;
	pm_susp.latency = latency;
	pm_susp.state = state;
	pm_susp.timeout = timeout;
	pm_susp.received = (u8)true;
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
		pm_dbg("%s: WARNING: dropping unhandled acknowledge!\n", __func__);
		pm_dbg("Dropped %s (%d, %d, %d)\n", __func__, pm_ack.node,
			pm_ack.status, pm_ack.opp);
	}
	pm_dbg("%s (%d, %d, %d)\n", __func__, node, status, oppoint);

	pm_ack.node = node;
	pm_ack.status = status;
	pm_ack.opp = oppoint;
	pm_ack.received = (u8)true;
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
		const enum XPmNotifyEvent event,
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD0(payload, PM_GET_API_VERSION);
		ret = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != ret) {
			goto done;
		}

		/* Return result from IPI return buffer */
		ret = pm_ipi_buff_read32(primary_master, version, NULL, NULL);
	}
done:
	return ret;
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
 *   - 0 : if CPU is off (powered down),
 *   - 1 : if CPU is active (powered up),
 *   - 2 : if CPU is in sleep (powered down),
 *   - 3 : if CPU is suspending (powered up)
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_GET_NODE_STATUS, node);
		ret = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != ret) {
			goto done;
		}

		/* Return result from IPI return buffer */
		ret = pm_ipi_buff_read32(primary_master, &nodestatus->status,
					  &nodestatus->requirements,
					  &nodestatus->usage);
	}
done:
	return ret;
}

/****************************************************************************/
/**
 * @brief  Call this function to request the power management controller to
 * return information about an operating characteristic of a component.
 *
 * @param  node   ID of the component or sub-system in question.
 * @param  type   Type of operating characteristic requested:
 * - power (current power consumption),
 * - latency (current latency in micro seconds to return to active state),
 * - temperature (current temperature),
 * @param  result Used to return the requested operating characteristic.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Power value is not actual power consumption of device. It is default
 * 	   dummy power value which is fixed in PMUFW.
 * 	   Temperature type is not supported for ZynqMP.
 *
 ****************************************************************************/
XStatus XPm_GetOpCharacteristic(const enum XPmNodeId node,
				const enum XPmOpCharType type,
				u32* const result)
{
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_GET_OP_CHARACTERISTIC, node, type);
		ret = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != ret) {
			goto done;
		}

	/* Return result from IPI return buffer */
		ret = pm_ipi_buff_read32(primary_master, result, NULL, NULL);
	}

done:
	return ret;
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
			const enum XPmResetAction resetaction)
{
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_RESET_ASSERT, reset, resetaction);
		ret = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != ret) {
			goto done;
		}

	/* Return result from IPI return buffer */
		ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return ret;
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_RESET_GET_STATUS, reset);
		ret = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != ret) {
			goto done;
		}

	/* Return result from IPI return buffer */
		ret = pm_ipi_buff_read32(primary_master, status, NULL, NULL);
	}

done:
	return ret;
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
 * ( - EVENT_STATE_CHANGE, EVENT_ZERO_USERS),
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL == notifier) {
		pm_dbg("%s ERROR: NULL notifier pointer\n", __func__);
		ret = (XStatus)XST_INVALID_PARAM;
		goto done;
	}

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD4(payload, PM_REGISTER_NOTIFIER, notifier->node,
				  notifier->event, notifier->flags, 1);
		ret = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != ret) {
			goto done;
		}

		ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);

		if (XST_SUCCESS != ret) {
			goto done;
		}

		/* Add notifier in the list only if PMU has it registered */
		ret = XPm_NotifierAdd(notifier);
	}

done:
	return ret;
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
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL == notifier) {
		pm_dbg("%s ERROR: NULL notifier pointer\n", __func__);
		ret = (XStatus)XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Remove first the notifier from the list. If it's not in the list
	 * report an error, and don't trigger PMU since it don't have it
	 * registered either.
	 */
	ret = XPm_NotifierRemove(notifier);
	if (XST_SUCCESS != ret) {
		goto done;
	}

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD4(payload, PM_REGISTER_NOTIFIER, notifier->node,
				  notifier->event, 0, 0);
		ret = pm_ipi_send(primary_master, payload);
		if (XST_SUCCESS != ret) {
			goto done;
		}

		ret = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}
	else {
		ret = (XStatus)XST_FAILURE;
	}

done:
	return ret;
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD3(payload, PM_MMIO_WRITE, address, mask, value);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

	/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
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
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_MMIO_READ, address);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

	/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, value, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to enable (activate) a clock
 *
 * @param  clk Identifier of the target clock to be enabled
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_ClockEnable(const enum XPmClock clk)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_CLOCK_ENABLE, clk);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to disable (gate) a clock
 *
 * @param  clk Identifier of the target clock to be disabled
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_ClockDisable(const enum XPmClock clk)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_CLOCK_DISABLE, clk);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to get status of a clock gate state
 *
 * @param  clk  Identifier of the target clock
 * @param  status Location to store clock gate state (1=enabled, 0=disabled)
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_ClockGetStatus(const enum XPmClock clk, u32 *const status)
{
	XStatus ret = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_CLOCK_GETSTATE, clk);
		ret = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != ret) {
			goto done;
		}

		/* Return result from IPI return buffer */
		ret = pm_ipi_buff_read32(primary_master, status, NULL, NULL);
	}

done:
	return ret;
}

/****************************************************************************/
/**
 * @brief  Call this function to set divider for a clock
 *
 * @param  clk   Identifier of the target clock
 * @param  divider Divider value to be set
 * @param  divId ID of the divider to be set
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
static XStatus XPm_ClockSetOneDivider(const enum XPmClock clk,
				      const u32 divider,
				      const u32 divId)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD3(payload, PM_CLOCK_SETDIVIDER, clk, divId, divider);
		status = pm_ipi_send(primary_master, payload);
		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to set divider for a clock
 *
 * @param  clk   Identifier of the target clock
 * @param  divider Divider value to be set
 *
 * @return XST_INVALID_PARAM or status of performing the operation as returned
 * by the PMU-FW
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_ClockSetDivider(const enum XPmClock clk, const u32 divider)
{
	XStatus status = (XStatus)XST_INVALID_PARAM;
	u8 mapping = XPm_GetClockDivType(clk);
	u32 div0 = 0U;
	u32 div1 = 0U;

	mapping = XPm_MapDivider(clk, divider, &div0, &div1);
	if (0U == mapping) {
		goto done;
	}

	if (0U != (mapping & (1U << PM_CLOCK_DIV0_ID))) {
		status = XPm_ClockSetOneDivider(clk, div0, PM_CLOCK_DIV0_ID);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

	if (0U != (mapping & (1U << PM_CLOCK_DIV1_ID))) {
		status = XPm_ClockSetOneDivider(clk, div1, PM_CLOCK_DIV1_ID);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Local function to get one divider (DIV0 or DIV1) of a clock
 *
 * @param  clk   Identifier of the target clock
 * @param  divider Location to store the divider value
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
static XStatus XPm_ClockGetOneDivider(const enum XPmClock clk,
				      u32 *const divider,
				      const u32 divId)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_CLOCK_GETDIVIDER, clk, divId);
		status = pm_ipi_send(primary_master, payload);
		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, divider, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to get divider of a clock
 *
 * @param  clk   Identifier of the target clock
 * @param  divider Location to store the divider value
 *
 * @return XST_INVALID_PARAM or status of performing the operation as returned
 * by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_ClockGetDivider(const enum XPmClock clk, u32 *const divider)
{
	XStatus status = (XStatus)XST_INVALID_PARAM;
	u8 type = XPm_GetClockDivType(clk);
	u32 div_val;

	if ((NULL == divider) || (0U == type)) {
		goto done;
	}

	*divider = 1U;
	if (0U != (type & (1U << PM_CLOCK_DIV0_ID))) {
		status = XPm_ClockGetOneDivider(clk, &div_val, PM_CLOCK_DIV0_ID);
		if (XST_SUCCESS != status) {
			goto done;
		}
		*divider *= div_val;
	}

	if (0U != (type & (1U << PM_CLOCK_DIV1_ID))) {
		status = XPm_ClockGetOneDivider(clk, &div_val, PM_CLOCK_DIV1_ID);
		if (XST_SUCCESS != status) {
			goto done;
		}
		*divider *= div_val;
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to set parent for a clock
 *
 * @param  clk  Identifier of the target clock
 * @param  parent Identifier of the target parent clock
 *
 * @return XST_INVALID_PARAM or status of performing the operation as returned
 * by the PMU-FW.
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_ClockSetParent(const enum XPmClock clk,
			   const enum XPmClock parent)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];
	u32 select = 0U;

	status = XPm_GetSelectByClockParent(clk, parent, &select);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_CLOCK_SETPARENT, clk, select);
		status = pm_ipi_send(primary_master, payload);
		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to get parent of a clock
 *
 * @param  clk  Identifier of the target clock
 * @param  parent Location to store clock parent ID
 *
 * @return XST_INVALID_PARAM or status of performing the operation as returned
 * by the PMU-FW.

 ****************************************************************************/
XStatus XPm_ClockGetParent(const enum XPmClock clk,
			   enum XPmClock* const parent)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];
	u32 select = 0U;

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_CLOCK_GETPARENT, clk);
		status = pm_ipi_send(primary_master, payload);
		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, &select, NULL, NULL);
		if (XST_SUCCESS != status) {
			goto done;
		}

		status = XPm_GetClockParentBySelect(clk, select, parent);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to set rate of a clock
 *
 * @param  clk  Identifier of the target clock
 * @param  rate   Clock frequency (rate) to be set
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 * @note   If the action isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_ClockSetRate(const enum XPmClock clk, const u32 rate)
{
	pm_dbg("%s(%u, %u) not supported\n", __func__, clk, rate);

	return (XStatus)XST_NO_FEATURE;
}

/****************************************************************************/
/**
 * @brief  Call this function to get rate of a clock
 *
 * @param  clk  Identifier of the target clock
 * @param  rate   Location where the rate should be stored
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_ClockGetRate(const enum XPmClock clk, u32 *const rate)
{
	pm_dbg("%s(%u, %u) not supported\n", __func__, clk, rate);

	return (XStatus)XST_NO_FEATURE;
}

/****************************************************************************/
/**
 * @brief  Call this function to set a PLL parameter
 *
 * @param  node      PLL node identifier
 * @param  parameter PLL parameter identifier
 * @param  value     Value of the PLL parameter
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_PllSetParameter(const enum XPmNodeId node,
			    const enum XPmPllParam parameter,
			    const u32 value)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD3(payload, PM_PLL_SET_PARAMETER, node, parameter, value);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to get a PLL parameter
 *
 * @param  node      PLL node identifier
 * @param  parameter PLL parameter identifier
 * @param  value     Location to store value of the PLL parameter
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_PllGetParameter(const enum XPmNodeId node,
			    const enum XPmPllParam parameter,
			    u32 *const value)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_PLL_GET_PARAMETER, node, parameter);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, value, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to set a PLL mode
 *
 * @param  node  PLL node identifier
 * @param  mode  PLL mode to be set
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_PllSetMode(const enum XPmNodeId node, const enum XPmPllMode mode)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_PLL_SET_MODE, node, mode);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to get a PLL mode
 *
 * @param  node  PLL node identifier
 * @param  mode  Location to store the PLL mode
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_PllGetMode(const enum XPmNodeId node, enum XPmPllMode* const mode)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];
	u32 mode_val = 0U;

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_PLL_GET_MODE, node);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, &mode_val, NULL, NULL);
		*mode = (enum XPmPllMode)mode_val;
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Locally used function to request or release a pin control
 *
 * @param  pin  PIN identifier (index from range 0-77)
 * @api    API identifier (request or release pin control)
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
static XStatus XPm_PinCtrlAction(const u32 pin, const enum XPmApiId api)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, api, pin);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to request a pin control
 *
 * @param  pin  PIN identifier (index from range 0-77)
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_PinCtrlRequest(const u32 pin)
{
	return XPm_PinCtrlAction(pin, PM_PINCTRL_REQUEST);
}

/****************************************************************************/
/**
 * @brief  Call this function to release a pin control
 *
 * @param  pin  PIN identifier (index from range 0-77)
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_PinCtrlRelease(const u32 pin)
{
	return XPm_PinCtrlAction(pin, PM_PINCTRL_RELEASE);
}

/****************************************************************************/
/**
 * @brief  Call this function to set a pin function
 *
 * @param  pin  Pin identifier
 * @param  fn   Pin function to be set
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_PinCtrlSetFunction(const u32 pin, const enum XPmPinFn fn)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_PINCTRL_SET_FUNCTION, pin, fn);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to get currently configured pin function
 *
 * @param  pin  PLL node identifier
 * @param  fn   Location to store the pin function
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_PinCtrlGetFunction(const u32 pin, enum XPmPinFn* const fn)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];
	u32 fn_val = 0U;

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD1(payload, PM_PINCTRL_GET_FUNCTION, pin);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, &fn_val, NULL, NULL);
		*fn = (enum XPmPinFn)fn_val;
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to set a pin parameter
 *
 * @param  pin   Pin identifier
 * @param  param Pin parameter identifier
 * @param  value Value of the pin parameter to set
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 * @note   If the access isn't permitted this function returns an error code.
 *
 ****************************************************************************/
XStatus XPm_PinCtrlSetParameter(const u32 pin,
				const enum XPmPinParam param,
				const u32 value)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD3(payload, PM_PINCTRL_CONFIG_PARAM_SET, pin, param, value);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, NULL, NULL, NULL);
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call this function to get currently configured value of pin parameter
 *
 * @param  pin   Pin identifier
 * @param  param Pin parameter identifier
 * @param  value Location to store value of the pin parameter
 *
 * @return Status of performing the operation as returned by the PMU-FW
 *
 ****************************************************************************/
XStatus XPm_PinCtrlGetParameter(const u32 pin,
				const enum XPmPinParam param,
				u32* const value)
{
	XStatus status = (XStatus)XST_FAILURE;
	u32 payload[PAYLOAD_ARG_CNT];

	if (NULL != primary_master) {
		/* Send request to the PMU */
		PACK_PAYLOAD2(payload, PM_PINCTRL_CONFIG_PARAM_GET, pin, param);
		status = pm_ipi_send(primary_master, payload);

		if (XST_SUCCESS != status) {
			goto done;
		}

		/* Return result from IPI return buffer */
		status = pm_ipi_buff_read32(primary_master, value, NULL, NULL);
	}

done:
	return status;
}
 /** @} */
