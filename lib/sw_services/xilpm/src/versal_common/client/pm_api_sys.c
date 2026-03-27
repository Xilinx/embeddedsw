/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file pm_api_sys.c
 *
 * PM Definitions implementation
 * @addtogroup xpm_versal_apis XilPM Versal APIs
 * @{
 *****************************************************************************/
#include "pm_api_sys.h"
#include "pm_callbacks.h"
#include "pm_client.h"
#if defined  (XPM_SUPPORT) && (__aarch64__) && (EL1_NONSECURE == 1)
#include "xil_smc.h"
#endif

/** @cond INTERNAL */

/* Payload Packets */
#define PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5)	\
	Payload[0] = (u32)Arg0;						\
	Payload[1] = (u32)Arg1;						\
	Payload[2] = (u32)Arg2;						\
	Payload[3] = (u32)Arg3;						\
	Payload[4] = (u32)Arg4;						\
	Payload[5] = (u32)Arg5;						\
	XPm_Dbg("%s(%x, %x, %x, %x, %x)\r\n", __func__, Arg1, Arg2, Arg3, Arg4, Arg5);

#define XILPM_MODULE_ID			(0x02UL)

#define HEADER(len, ApiId)		((len << 16U) | (XILPM_MODULE_ID << 8U) | ((u32)ApiId))

/**
 * @brief SMC Function ID for pass-through firmware commands
 *
 * The full SMC FID (0xC2000FFF) used for extended format PLM commands.
 * Combines the standard SMC calling convention (0xC2000000) with the
 * pass-through command ID (0xFFF).
 */
#define PASS_THROUGH_FW_CMD_SMC_FID	(0xC2000FFFU)

/**
 * Old-format SMC FIDs for TF-A-specific PM APIs. These are handled
 * by TF-A's TF_A_specific_handler rather than going through the PLM
 * pass-through path (0xC2000FFF).
 */
#define PM_GET_CALLBACK_DATA_SMC_FID		(0xC2000a01ULL)
#define TF_A_PM_REGISTER_SGI_SMC_FID		(0xC2000a04ULL)
#define PM_NOTIFY_CB_TYPE			(32U)

#define PACK_PAYLOAD0(Payload, ApiId) \
	PACK_PAYLOAD(Payload, HEADER(0UL, ApiId), 0, 0, 0, 0, 0)
#define PACK_PAYLOAD1(Payload, ApiId, Arg1) \
	PACK_PAYLOAD(Payload, HEADER(1UL, ApiId), Arg1, 0, 0, 0, 0)
#define PACK_PAYLOAD2(Payload, ApiId, Arg1, Arg2) \
	PACK_PAYLOAD(Payload, HEADER(2UL, ApiId), Arg1, Arg2, 0, 0, 0)
#define PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2, Arg3) \
	PACK_PAYLOAD(Payload, HEADER(3UL, ApiId), Arg1, Arg2, Arg3, 0, 0)
#define PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
	PACK_PAYLOAD(Payload, HEADER(4UL, ApiId), Arg1, Arg2, Arg3, Arg4, 0)
#define PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
	PACK_PAYLOAD(Payload, HEADER(5UL, ApiId), Arg1, Arg2, Arg3, Arg4, Arg5)

/**
 * @brief Extract upper 32 bits from a 64-bit value
 *
 * @param n 64-bit value to extract from
 *
 * @return Upper 32 bits of the input value as uint32_t
 */
#define upper_32_bits(n)	((uint32_t)(((n) >> 32U)))

/**
 * @brief Extract lower 32 bits from a 64-bit value
 *
 * @param n 64-bit value to extract from
 *
 * @return Lower 32 bits of the input value as uint32_t
 */
#define lower_32_bits(n)	((uint32_t)((n) & 0xFFFFFFFFU))

/**
 * @brief Maximum number of payload arguments supported by XPm_GenericRequest
 *
 * This defines the maximum number of variable arguments (0-6) that can be
 * passed to the XPm_GenericRequest function for PM API calls. The actual
 * number of arguments used depends on the specific PM API being called.
 *
 * @note The SMC path supports up to 6 payload arguments. The IPI path
 *       supports up to 5 payload arguments (Args[5] is not transmitted).
 */
#define MAX_PAYLOAD_ARG		(6U)

/**
 * @brief Maximum number of response arguments supported by XPm_GenericRequest
 *
 * This defines the maximum number of response values (0-7) that can be
 * returned by the XPm_GenericRequest function for PM API calls. The actual
 * number of response values depends on the specific PM API being called.
 *
 * @note The value is set to 7 to accommodate PM APIs that return more
 *       than the standard 3 response values, providing flexibility for
 *       extended response data.
 */
#define MAX_RESPONSE_ARG	(7U)

#if !(defined(XPM_SUPPORT) && defined(__aarch64__) && defined(EL1_NONSECURE) && (EL1_NONSECURE == 1))
/****************************************************************************/
/**
 * @brief  Sends IPI request to the target module
 *
 * @param  Proc  Pointer to the processor who is initiating request
 * @param  Payload API id and call arguments to be written in IPI buffer
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
static XStatus XPm_IpiSend(struct XPm_Proc *const Proc, u32 *Payload)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XIpiPsu_PollForAck(Proc->Ipi, TARGET_IPI_INT_MASK,
				    PM_IPI_TIMEOUT);
	if (Status != XST_SUCCESS) {
		XPm_Err("IPI Timeout expired in %s\n", __func__);
		goto done;
	}

	Status = XIpiPsu_WriteMessage(Proc->Ipi, TARGET_IPI_INT_MASK, Payload,
				      PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_MSG);
	if (Status != XST_SUCCESS) {
		XPm_Err("Writing to IPI request buffer failed\n");
		goto done;
	}

	Status = XIpiPsu_TriggerIpi(Proc->Ipi, TARGET_IPI_INT_MASK);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Reads IPI Response after target module has handled interrupt
 *
 * @param  Proc Pointer to the processor who is waiting and reading Response
 * @param  Val1 Used to return value from 2nd IPI buffer element (optional)
 * @param  Val2 Used to return value from 3rd IPI buffer element (optional)
 * @param  Val3 Used to return value from 4th IPI buffer element (optional)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
static XStatus Xpm_IpiReadBuff32(struct XPm_Proc *const Proc, u32 *Val1,
				 u32 *Val2, u32 *Val3)
{
	u32 Response[RESPONSE_ARG_CNT] = {0};
	XStatus Status = (s32)XST_FAILURE;

	/* Wait until current IPI interrupt is handled by target module */
	Status = XIpiPsu_PollForAck(Proc->Ipi, TARGET_IPI_INT_MASK,
				    PM_IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		XPm_Err("IPI Timeout expired in %s\n", __func__);
		goto done;
	}

	Status = XIpiPsu_ReadMessage(Proc->Ipi, TARGET_IPI_INT_MASK, Response,
				     RESPONSE_ARG_CNT, XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		XPm_Err("Reading from IPI response buffer failed\n");
		goto done;
	}

	/*
	 * Read Response from IPI buffer
	 * buf-0: success or error+reason
	 * buf-1: Val1
	 * buf-2: Val2
	 * buf-3: Val3
	 */
	if (NULL != Val1) {
		*Val1 = Response[1];
	}
	if (NULL != Val2) {
		*Val2 = Response[2];
	}
	if (NULL != Val3) {
		*Val3 = Response[3];
	}

	Status = (s32)Response[0];

done:
	return Status;
}
#endif

/****************************************************************************/
/**
 * @brief  Returns version of register notifier API from fetched from server
 *
 * @return PmApiRegisterNotifierVersionServer register notifier API version
 * fetched from server
 *
 ****************************************************************************/
u32 XPm_GetRegisterNotifierVersionServer(void)
{
	XStatus Status = (s32)XST_FAILURE;
	static u32 PmApiRegisterNotifierVersionServer = 0U;

	if (0U == PmApiRegisterNotifierVersionServer) {
		/*Fetching version of PM_REGISTER_NOTIFIER API from server*/
		Status = XPm_FeatureCheck((u32)PM_REGISTER_NOTIFIER,
					  &PmApiRegisterNotifierVersionServer);
		if ((s32)XST_SUCCESS != Status) {
			XPm_Err("Couldn't fetch server API version for"
				"pm_register_notifier\n");
			PmApiRegisterNotifierVersionServer = 0U;
		}
	}

	return PmApiRegisterNotifierVersionServer;
}

/****************************************************************************/
/**
 * @brief  Generic wrapper for PM API calls with variable arguments and responses
 *
 * @param  ApiId     API identifier
 * @param  NumArgs   Number of arguments being passed (0-6)
 * @param  Response  Pointer to response array (pass NULL if no response expected)
 *                   Response[0] = first response value
 *                   Response[1] = second response value
 *                   Response[2] = third response value
 * @param  ...       Variable arguments (actual argument values)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 *
 * @note   This is a generic wrapper that handles the common IPI/SMC send/receive
 *         pattern used by most PM API functions. It uses variable arguments to
 *         support flexible argument counts (0-6 arguments).
 *         Pass NULL for Response if no response is expected.
 *         The function supports both IPI and SMC interfaces based on compile-time
 *         configuration.
 *
 ****************************************************************************/
static XStatus XPm_GenericRequest(u32 ApiId, u32 NumArgs, u32 *Response, ...)
{
	XStatus Status = (s32)XST_FAILURE;
	va_list arg_list;
	u32 Args[MAX_PAYLOAD_ARG] = {0};
	u32 i;

	/* Validate argument count */
	if (NumArgs > MAX_PAYLOAD_ARG) {
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	/* Extract arguments from va_list */
	va_start(arg_list, Response);
	for (i = 0; i < NumArgs; i++) {
		Args[i] = va_arg(arg_list, u32);
	}
	va_end(arg_list);

#if defined  (XPM_SUPPORT) && (__aarch64__) && (EL1_NONSECURE == 1)
	XSmc_OutVar Out;
	u64 SmcFid;
	u64 SmcArgs1;
	u64 SmcArgs2;
	u64 SmcArgs3;
	u64 SmcArgs4;
	u32 PlmHeader;

	/*
	 * PM_SELF_SUSPEND and PM_ABORT_SUSPEND are not supported via the
	 * SMC interface. In the PSCI path, TF-A internally handles the
	 * suspend notification to PLM when XPm_ClientSuspendFinalize()
	 * issues PSCI CPU_SUSPEND, so a duplicate PM_SELF_SUSPEND from
	 * the client library is not needed. Since PM_SELF_SUSPEND is not
	 * sent to PLM, PM_ABORT_SUSPEND has no pending suspend to cancel.
	 */
	if (ApiId == PM_SELF_SUSPEND) {
		XPm_Dbg("PM_SELF_SUSPEND is not sent via SMC; use "
			 "XPm_ClientSuspendFinalize() for PSCI CPU_SUSPEND\r\n");
		Status = (s32)XST_SUCCESS;
		goto done;
	}

	if (ApiId == PM_ABORT_SUSPEND) {
		XPm_Dbg("PM_ABORT_SUSPEND is not supported via SMC; "
			 "PM_SELF_SUSPEND is not sent in this path\r\n");
		Status = (s32)XST_SUCCESS;
		goto done;
	}

	/*
	 * Extended SMC Format:
	 * - SMC ID: Fixed 0xC2000FFF
	 * - x1: Args[0] << 32 | PLM_HEADER
	 * - x2: Args[2] << 32 | Args[1]
	 * - x3: Args[4] << 32 | Args[3]
	 * - x4: Args[5]
	 * Supports 6 payload arguments and 6 response values
	 */

	/* Construct PLM header with module ID, API ID, and length */
	PlmHeader = HEADER(NumArgs, ApiId);

	/* Fixed SMC ID for extended format */
	SmcFid = PASS_THROUGH_FW_CMD_SMC_FID;

	/* Pack arguments: PLM header in lower 32 bits of x1 */
	SmcArgs1 = ((u64)Args[0] << 32) | ((u64)PlmHeader);
	SmcArgs2 = ((u64)Args[2] << 32) | ((u64)Args[1]);
	SmcArgs3 = ((u64)Args[4] << 32) | ((u64)Args[3]);
	SmcArgs4 = (u64)Args[5];

	/* Make SMC call with extended format */
	Out = Xil_Smc(SmcFid, SmcArgs1, SmcArgs2, SmcArgs3, SmcArgs4, 0, 0, 0);

	/* Extract response values from extended format (6 values) */
	if (NULL != Response) {
		Response[0] = upper_32_bits(Out.Arg0); /* ret_payload0 */
		Response[1] = lower_32_bits(Out.Arg1); /* ret_payload1 */
		Response[2] = upper_32_bits(Out.Arg1); /* ret_payload2 */
		Response[3] = lower_32_bits(Out.Arg2); /* ret_payload3 */
		Response[4] = upper_32_bits(Out.Arg2); /* ret_payload4 */
		Response[5] = lower_32_bits(Out.Arg3); /* ret_payload5 */
	}

	/* Status is in lower 32 bits of Arg0 */
	Status = lower_32_bits(Out.Arg0);

#else
	u32 Payload[PAYLOAD_ARG_CNT];
	struct XPm_Proc *Proc;

	/*
	 * Special handling for PM_SELF_SUSPEND in IPI path:
	 *
	 * PM_SELF_SUSPEND requires using the processor's own IPI channel (not PrimaryProc)
	 * because:
	 * 1. The DeviceId (Args[0]) identifies which processor is suspending itself
	 * 2. Each processor has its own IPI channel for communication with PLM
	 * 3. The suspend request must be sent from the suspending processor's IPI channel
	 *    so PLM can correctly identify and manage that specific processor's state
	 * 4. XPm_ClientSuspend() performs processor-specific pre-suspend operations like
	 *    saving context and preparing for WFI execution
	 *
	 * For all other PM APIs, we use PrimaryProc since they are general requests
	 * that don't require processor-specific IPI channels.
	 */
	if (ApiId == PM_SELF_SUSPEND) {
		Proc = XPm_GetProcByDeviceId(Args[0]);
		if (NULL == Proc) {
			XPm_Err("Invalid Device ID\r\n");
			Status = (s32)XST_INVALID_PARAM;
			goto done;
		}

		XPm_ClientSuspend(Proc);
	} else {
		Proc = PrimaryProc;
	}


	/*
	 * Pack payload with correct NumArgs in header. IPI buffer holds
	 * 1 header + 5 arguments; Args[5] cannot be transmitted via IPI
	 * and is silently dropped (only the SMC path carries all 6 args).
	 */
	PACK_PAYLOAD(Payload, HEADER(NumArgs, ApiId), Args[0], Args[1], Args[2], Args[3], Args[4]);

	/* Send request to the target module */
	Status = XPm_IpiSend(Proc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	if (NULL != Response) {
		Status = Xpm_IpiReadBuff32(Proc, &Response[0], &Response[1], &Response[2]);
	} else {
		Status = Xpm_IpiReadBuff32(Proc, NULL, NULL, NULL);
	}

#endif

done:
	return Status;
}
/** @endcond */

/****************************************************************************/
/**
 * @brief  Initialize xilpm library
 *
 * @param  IpiInst Pointer to IPI driver instance
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPm_InitXilpm(XIpiPsu *IpiInst)
{
	XStatus Status = (s32)XST_FAILURE;
	struct XPm_Proc *Proc;

	if (NULL == IpiInst) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_SetPrimaryProc();
	if ((s32)XST_SUCCESS != Status) {
		goto done;
	}

	PrimaryProc->Ipi = IpiInst;

	Proc = XPm_GetProcByDeviceId(PrimaryProc->DevId);

	XPm_ClientWakeUp(Proc);

done:
	return Status;
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
 *
 *
 ****************************************************************************/
enum XPmBootStatus XPm_GetBootStatus(void)
{
	u32 PwrDwnReq;
	enum XPmBootStatus Ret;
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_SetPrimaryProc();
	if ((s32)XST_SUCCESS != Status) {
		XPm_Err("%s: Error in setting primary proc: %x\r\n", __func__, Status);
	}

	/* Error out if primary proc is not defined */
	if (NULL == PrimaryProc) {
		Ret = PM_BOOT_ERROR;
		goto done;
	}

#if defined(XPM_SUPPORT) && defined(__aarch64__) && defined(EL1_NONSECURE) && (EL1_NONSECURE == 1)
	{
		/*
		 * In the PSCI path, TF-A clears the CPUPWRDWNREQ bit on resume
		 * (via pm_client_wakeup in pwr_domain_suspend_finish), so we
		 * cannot use it for resume detection. Instead, check the
		 * RAM-based flag set by XPm_ClientSuspendFinalize() before
		 * the PSCI CPU_SUSPEND call. The flag is placed in .data via
		 * section attribute so it survives the BSP _start .bss clear.
		 */
		if (XPm_PsciSuspendFlag != 0U) {
			XPm_PsciSuspendFlag = 0U;
			Ret = PM_RESUME;
			goto done;
		}
	}
#endif

	PwrDwnReq = XPm_Read(PrimaryProc->PwrCtrl);
	if (0U != (PwrDwnReq & PrimaryProc->PwrDwnMask)) {
		PwrDwnReq &= ~PrimaryProc->PwrDwnMask;
		XPm_Write(PrimaryProc->PwrCtrl, PwrDwnReq);
		Ret = PM_RESUME;
		goto done;
	} else {
		Ret = PM_INITIAL_BOOT;
		goto done;
	}

done:
	return Ret;
}

/****************************************************************************/
/**
 * @brief  This function is used to request the version and ID code of a chip
 *
 * @param  IDCode  Returns the chip ID code.
 * @param  Version Returns the chip version.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_GetChipID(u32* IDCode, u32 *Version)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if ((NULL == IDCode) || (NULL == Version)) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_GET_CHIPID, 0U, RetPayload);
	if (XST_SUCCESS == Status) {
		*IDCode = RetPayload[0];
		*Version = RetPayload[1];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to request the version number of the API
 * running on the platform management controller.
 *
 * @param  Version Returns the API 32-bit version number.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_GetApiVersion(u32 *Version)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == Version) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_GET_API_VERSION, 0U, RetPayload);
	if (XST_SUCCESS == Status) {
		*Version = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to request the device
 *
 * @param  DeviceId		Device which needs to be requested
 * @param  Capabilities		Device Capabilities, can be combined
 *				- PM_CAP_ACCESS  : full access / functionality
 *				- PM_CAP_CONTEXT : preserve context
 *				- PM_CAP_WAKEUP  : emit wake interrupts
 * @param  QoS			Quality of Service (0-100) required
 * @param  Ack			Requested acknowledge type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_RequestNode(const u32 DeviceId, const u32 Capabilities,
			const u32 QoS, const u32 Ack)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_REQUEST_NODE, 4U, NULL, DeviceId,
				    Capabilities, QoS, Ack);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to release the requested device
 *
 * @param  DeviceId		Device which needs to be released
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ReleaseNode(const u32 DeviceId)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_RELEASE_NODE, 1U, NULL, DeviceId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the requirement for specified device
 *
 * @param  DeviceId		Device for which requirement needs to be set
 * @param  Capabilities		Device Capabilities, can be combined
 *				- PM_CAP_ACCESS  : full access / functionality
 *				- PM_CAP_CONTEXT : preserve context
 *				- PM_CAP_WAKEUP  : emit wake interrupts
 * @param  QoS			Quality of Service (0-100) required
 * @param  Ack			Requested acknowledge type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_SetRequirement(const u32 DeviceId, const u32 Capabilities,
				 const u32 QoS, const u32 Ack)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_SET_REQUIREMENT, 4U, NULL, DeviceId,
				    Capabilities, QoS, Ack);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the device status
 *
 * @param  DeviceId		Device for which status is requested
 * @param  NodeStatus		Structure pointer to store device status
 *				- Status - The current power state of the device
 *				 - For CPU nodes:
 *				  - 0 : if CPU is powered down,
 *				  - 1 : if CPU is active (powered up),
 *				  - 8 : if CPU is suspending (powered up)
 *				 - For power islands and power domains:
 *				  - 0 : if island is powered down,
 *				  - 2 : if island is powered up
 *				 - For slaves:
 *				  - 0 : if slave is powered down,
 *				  - 1 : if slave is powered up,
 *				  - 9 : if slave is in retention
 *
 *				- Requirement - Requirements placed on the device by the caller
 *
 *				- Usage
 *				 - 0 : node is not used by any PU,
 *				 - 1 : node is used by caller exclusively,
 *				 - 2 : node is used by other PU(s) only,
 *				 - 3 : node is used by caller and by other PU(s)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_GetNodeStatus(const u32 DeviceId, XPm_NodeStatus *const NodeStatus)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 Response[MAX_RESPONSE_ARG];

	if (NULL == NodeStatus) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_GET_NODE_STATUS, 1U, Response, DeviceId);
	if (XST_SUCCESS == Status) {
		NodeStatus->status = Response[0];
		NodeStatus->requirements = Response[1];
		NodeStatus->usage = Response[2];
	}
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to assert or release reset for a particular
 * reset line. Alternatively a reset pulse can be requested as well.
 *
 * @param  ResetId		Reset ID
 * @param  Action		Reset action to be taken
 *				- PM_RESET_ACTION_RELEASE for Release Reset
 *				- PM_RESET_ACTION_ASSERT for Assert Reset
 *				- PM_RESET_ACTION_PULSE for Pulse Reset
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ResetAssert(const u32 ResetId, const u32 Action)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_RESET_ASSERT, 2U, NULL, ResetId, Action);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the status of reset
 *
 * @param  ResetId		Reset ID
 * @param  State		Pointer to store the status of specified reset
 *				- 0 for reset released
 *				- 1 for reset asserted
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ResetGetStatus(const u32 ResetId, u32 *const State)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == State) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_RESET_GET_STATUS, 1U, RetPayload, ResetId);
	if (XST_SUCCESS == Status) {
		*State = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to request the pin
 *
 * @param  PinId		Pin ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PinCtrlRequest(const u32 PinId)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_PINCTRL_REQUEST, 1U, NULL, PinId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to release the pin
 *
 * @param  PinId		Pin ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PinCtrlRelease(const u32 PinId)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_PINCTRL_RELEASE, 1U, NULL, PinId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the function on specified pin
 *
 * @param  PinId		Pin ID
 * @param  FunctionId		Function ID which needs to be set
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PinCtrlSetFunction(const u32 PinId, const u32 FunctionId)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_PINCTRL_SET_FUNCTION, 2U, NULL, PinId, FunctionId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the function on specified pin
 *
 * @param  PinId		Pin ID
 * @param  FunctionId		Pointer to Function ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PinCtrlGetFunction(const u32 PinId, u32 *const FunctionId)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == FunctionId) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_PINCTRL_GET_FUNCTION, 1U, RetPayload, PinId);
	if (XST_SUCCESS == Status) {
		*FunctionId = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the pin parameter of specified pin
 *
 * @param  PinId		Pin ID
 * @param  ParamId		Parameter ID
 * @param  ParamVal		Value of the parameter
 *
 * @details The following table lists the parameter ID and their respective values:
 *
 * ----------------------------------------------------------------------------
 *  ParamId				| ParamVal
 * ---------------------|------------------------------------------------------
 *  PINCTRL_CONFIG_SLEW_RATE | PINCTRL_SLEW_RATE_SLOW, PINCTRL_SLEW_RATE_FAST
 *  PINCTRL_CONFIG_BIAS_STATUS | PINCTRL_BIAS_DISABLE, PINCTRL_BIAS_ENABLE
 *  PINCTRL_CONFIG_PULL_CTRL | PINCTRL_BIAS_PULL_DOWN, PINCTRL_BIAS_PULL_UP
 *  PINCTRL_CONFIG_SCHMITT_CMOS | PINCTRL_INPUT_TYPE_CMOS, PINCTRL_INPUT_TYPE_SCHMITT
 *  PINCTRL_CONFIG_DRIVE_STRENGTH | PINCTRL_DRIVE_STRENGTH_TRISTATE, PINCTRL_DRIVE_STRENGTH_4MA, PINCTRL_DRIVE_STRENGTH_8MA, PINCTRL_DRIVE_STRENGTH_12MA
 *  PINCTRL_CONFIG_TRI_STATE | PINCTRL_TRI_STATE_DISABLE, PINCTRL_TRI_STATE_ENABLE
 * ----------------------------------------------------------------------------
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PinCtrlSetParameter(const u32 PinId, const u32 ParamId, const u32 ParamVal)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_PINCTRL_CONFIG_PARAM_SET, 3U, NULL,
				    PinId, ParamId, ParamVal);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the pin parameter of specified pin
 *
 * @param  PinId		Pin ID
 * @param  ParamId		Parameter ID
 * @param  ParamVal		Pointer to the value of the parameter
 *
 * @details The following table lists the parameter ID and their respective values:
 *
 * ----------------------------------------------------------------------------
 *  ParamId				| ParamVal
 * ---------------------|------------------------------------------------------
 *  PINCTRL_CONFIG_SLEW_RATE | PINCTRL_SLEW_RATE_SLOW, PINCTRL_SLEW_RATE_FAST
 *  PINCTRL_CONFIG_BIAS_STATUS | PINCTRL_BIAS_DISABLE, PINCTRL_BIAS_ENABLE
 *  PINCTRL_CONFIG_PULL_CTRL | PINCTRL_BIAS_PULL_DOWN, PINCTRL_BIAS_PULL_UP
 *  PINCTRL_CONFIG_SCHMITT_CMOS | PINCTRL_INPUT_TYPE_CMOS, PINCTRL_INPUT_TYPE_SCHMITT
 *  PINCTRL_CONFIG_DRIVE_STRENGTH | PINCTRL_DRIVE_STRENGTH_TRISTATE, PINCTRL_DRIVE_STRENGTH_4MA, PINCTRL_DRIVE_STRENGTH_8MA, PINCTRL_DRIVE_STRENGTH_12MA
 *  PINCTRL_CONFIG_VOLTAGE_STATUS | 1 for 1.8v mode, 0 for 3.3v mode
 *  PINCTRL_CONFIG_TRI_STATE | PINCTRL_TRI_STATE_DISABLE, PINCTRL_TRI_STATE_ENABLE
 * ----------------------------------------------------------------------------
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PinCtrlGetParameter(const u32 PinId, const u32 ParamId, u32 *const ParamVal)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == ParamVal) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_PINCTRL_CONFIG_PARAM_GET, 2U, RetPayload,
				    PinId, ParamId);
	if (XST_SUCCESS == Status) {
		*ParamVal = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function performs driver-like IOCTL functions on shared system
 * devices.
 *
 * @param DeviceId		ID of the device
 * @param IoctlId		IOCTL function ID
 * @param Arg1			Argument 1
 * @param Arg2			Argument 2
 * @param Response		Ioctl response
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_DevIoctl(const u32 DeviceId, const pm_ioctl_id IoctlId, const u32 Arg1,
		     const u32 Arg2, u32 *const Response)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == Response) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_IOCTL, 4U, RetPayload, DeviceId, IoctlId, Arg1, Arg2);
	if (XST_SUCCESS == Status) {
		*Response = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function performs driver-like IOCTL functions on shared system
 * devices.
 *
 * @param DeviceId		ID of the device
 * @param IoctlId		IOCTL function ID
 * @param Payload		Payload buffer
 * @param Response		Response buffer for Ioctl Response
 * @param PayloadSize		Payload buffer size
 * @param ResponseSize		Response buffer size
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note This API is a new version of existing API XPm_DevIoctl API.
 * It supports all the IOCTL functions supported by the old API and it adds
 * support for a couple new ones.
 * Users should migrate to using the new API signature in their applications.
 * The older API will be deprecated and support for it may be removed in
 * future releases.
 *
 * There are multiple reasons for introducing a new API signature:
 *  - The old API had fixed number of payload arguments, the new one supports
 *  variable length of payload buffer for forward compatibility
 *  - To maintain backwards compatibility for old API
 *  - For scalability reasons when support for "large payload" is introduced
 *
 ****************************************************************************/
XStatus XPm_DevIoctl2(u32 DeviceId, pm_ioctl_id IoctlId,
		      const u32 *Payload, u32 PayloadSize,
		      u32 *const Response, u32 ResponseSize)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG] = {0};
	u32 i;

	if ((NULL == Payload) || (NULL == Response)) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Payload: 3 for API/Node/Ioctl Ids + 2 for Reserved/CRC
	 * Response: 4 for Reserved/CRC
	 */
	if (((PAYLOAD_ARG_CNT - 5U) < PayloadSize) ||
	    ((RESPONSE_ARG_CNT - 4U) < ResponseSize)) {
		XPm_Err("Invalid size of Payload/Response buffer %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_IOCTL, 5U, RetPayload, DeviceId, IoctlId,
				    Payload[0], Payload[1], Payload[2]);
	if (XST_SUCCESS == Status) {
		for (i = 0U; i < ResponseSize; i++) {
			Response[i] = RetPayload[i];
		}
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to enable the specified clock
 *
 * @param  ClockId		Clock ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ClockEnable(const u32 ClockId)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_CLOCK_ENABLE, 1U, NULL, ClockId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to disable the specified clock
 *
 * @param  ClockId		Clock ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ClockDisable(const u32 ClockId)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_CLOCK_DISABLE, 1U, NULL, ClockId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the state of specified clock
 *
 * @param  ClockId		Clock ID
 * @param  State		Pointer to store the clock state
 *				- 1 for enable and 0 for disable
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ClockGetStatus(const u32 ClockId, u32 *const State)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == State) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_CLOCK_GETSTATE, 1U, RetPayload, ClockId);
	if (XST_SUCCESS == Status) {
		*State = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the divider value for specified clock
 *
 * @param  ClockId		Clock ID
 * @param  Divider		Value of the divider
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ClockSetDivider(const u32 ClockId, const u32 Divider)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_CLOCK_SETDIVIDER, 2U, NULL, ClockId, Divider);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get divider value for specified clock
 *
 * @param  ClockId		Clock ID
 * @param  Divider		Pointer to store divider value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ClockGetDivider(const u32 ClockId, u32 *const Divider)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == Divider) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_CLOCK_GETDIVIDER, 1U, RetPayload, ClockId);
	if (XST_SUCCESS == Status) {
		*Divider = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the parent for specified clock
 *
 * @param  ClockId		Clock ID
 * @param  ParentIdx		Parent clock index
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ClockSetParent(const u32 ClockId, const u32 ParentIdx)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_CLOCK_SETPARENT, 2U, NULL, ClockId, ParentIdx);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the parent of specified clock
 *
 * @param  ClockId		Clock ID
 * @param  ParentIdx		Pointer to store the parent clock index
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_ClockGetParent(const u32 ClockId, u32 *const ParentIdx)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == ParentIdx) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_CLOCK_GETPARENT, 1U, RetPayload, ClockId);
	if (XST_SUCCESS == Status) {
		*ParentIdx = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the parameters for specified PLL clock
 *
 * @param  ClockId		Clock ID
 * @param  ParamId		Parameter ID
 *				- PM_PLL_PARAM_ID_DIV2
 *				- PM_PLL_PARAM_ID_FBDIV
 *				- PM_PLL_PARAM_ID_DATA
 *				- PM_PLL_PARAM_ID_PRE_SRC
 *				- PM_PLL_PARAM_ID_POST_SRC
 *				- PM_PLL_PARAM_ID_LOCK_DLY
 *				- PM_PLL_PARAM_ID_LOCK_CNT
 *				- PM_PLL_PARAM_ID_LFHF
 *				- PM_PLL_PARAM_ID_CP
 *				- PM_PLL_PARAM_ID_RES
 * @param  Value		Value of parameter
 *				(See register description for possible values)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PllSetParameter(const u32 ClockId,
			    const enum XPm_PllConfigParams ParamId,
			    const u32 Value)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_PLL_SET_PARAMETER, 3U, NULL, ClockId, ParamId, Value);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the parameter of specified PLL clock
 *
 * @param  ClockId		Clock ID
 * @param  ParamId		Parameter ID
 *				- PM_PLL_PARAM_ID_DIV2
 *				- PM_PLL_PARAM_ID_FBDIV
 *				- PM_PLL_PARAM_ID_DATA
 *				- PM_PLL_PARAM_ID_PRE_SRC
 *				- PM_PLL_PARAM_ID_POST_SRC
 *				- PM_PLL_PARAM_ID_LOCK_DLY
 *				- PM_PLL_PARAM_ID_LOCK_CNT
 *				- PM_PLL_PARAM_ID_LFHF
 *				- PM_PLL_PARAM_ID_CP
 *				- PM_PLL_PARAM_ID_RES
 * @param  Value		Pointer to store parameter value
 *				(See register description for possible values)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PllGetParameter(const u32 ClockId,
			    const enum XPm_PllConfigParams ParamId,
			    u32 *const Value)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == Value) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_PLL_GET_PARAMETER, 2U, RetPayload, ClockId, ParamId);
	if (XST_SUCCESS == Status) {
		*Value = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the mode of specified PLL clock
 *
 * @param  ClockId		Clock ID
 * @param  Value		Mode which need to be set
 *				- 0 for Reset mode
 *				- 1 for Integer mode
 *				- 2 for Fractional mode
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PllSetMode(const u32 ClockId, const u32 Value)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_PLL_SET_MODE, 2U, NULL, ClockId, Value);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the mode of specified PLL clock
 *
 * @param  ClockId		Clock ID
 * @param  Value		Pointer to store the value of mode
 *				- 0 for Reset mode
 *				- 1 for Integer mode
 *				- 2 for Fractional mode
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_PllGetMode(const u32 ClockId, u32 *const Value)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == Value) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_PLL_GET_MODE, 1U, RetPayload, ClockId);
	if (XST_SUCCESS == Status) {
		*Value = RetPayload[0];
	}

done:
	return Status;
}

#ifndef __microblaze__
/****************************************************************************/
/**
 * @brief  This function is used by a CPU to declare that it is about to
 * suspend itself.
 *
 * @param DeviceId	Device ID of the CPU
 * @param Latency	Maximum wake-up latency requirement in us(microsecs)
 * @param State		Instead of specifying a maximum latency, a CPU can also
 *			explicitly request a certain power state.
 * @param Address	Address from which to resume when woken up.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_SelfSuspend(const u32 DeviceId, const u32 Latency,
			const u8 State, const u64 Address)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_SELF_SUSPEND, 5U, NULL, DeviceId, Latency,
				    State, (u32)Address, (u32)(Address >> 32));

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used to request power up of a CPU node
 * within the same PU, or to power up another PU.
 *
 * @param  TargetDevId	   Device ID of the CPU or PU to be powered/woken up.
 * @param  SetAddress Specifies whether the start address argument is being passed.
 * - 0 : do not set start address
 * - 1 : set start address
 * @param  Address    Address from which to resume when woken up.
 * Will only be used if set_address is 1.
 * @param  Ack		Requested acknowledge type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_RequestWakeUp(const u32 TargetDevId, const u8 SetAddress,
			  const u64 Address, const u32 Ack)
{
	XStatus Status = (s32)XST_FAILURE;
	u64 EncodedAddr;

	/* encode set Address into 1st bit of address */
	EncodedAddr = Address | ((1U == SetAddress) ? 1U : 0U);

	Status = XPm_GenericRequest(PM_REQUEST_WAKEUP, 4U, NULL, TargetDevId,
				    (u32)EncodedAddr, (u32)(EncodedAddr >> 32), Ack);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This Function waits for firmware to finish all previous API requests
 * sent by the PU and performs client specific actions to finish suspend
 * procedure (e.g. execution of wfi instruction on A53 and R5 processors).
 *
 * @note   This function should not return if the suspend procedure is
 * successful.
 *
 ****************************************************************************/
void XPm_SuspendFinalize(void)
{
	XStatus Status = (s32)XST_FAILURE;

	/*
	 * Wait until previous IPI request is handled by the PLM.
	 * If PLM is busy, keep trying until PLM becomes responsive
	 */
	do {
		Status = XIpiPsu_PollForAck(PrimaryProc->Ipi,
					    TARGET_IPI_INT_MASK,
					    PM_IPI_TIMEOUT);
		if (Status != XST_SUCCESS) {
			XPm_Err("Timed out while waiting for PLM to"
				" finish processing previous PM-API call\n");
		}
	} while (XST_SUCCESS != Status);

	XPm_ClientSuspendFinalize();
}

#else
XStatus XPm_SelfSuspend(const u32 DeviceId, const u32 Latency,
			const u8 State, const u64 Address)
{
	(void)DeviceId;
	(void)Latency;
	(void)State;
	(void)Address;

	XPm_Err("%s is not supported for Microblaze\n", __func__);

	return (s32)XST_NO_FEATURE;
}

XStatus XPm_RequestWakeUp(const u32 TargetDevId, const u8 SetAddress,
			  const u64 Address, const u32 Ack)
{
	(void)TargetDevId;
	(void)SetAddress;
	(void)Address;
	(void)Ack;

	XPm_Err("%s is not supported for Microblaze\n", __func__);

	return (s32)XST_NO_FEATURE;
}

void XPm_SuspendFinalize(void)
{
	XPm_Err("%s is not supported for Microblaze\n", __func__);

	return;
}

#endif

/****************************************************************************/
/**
 * @brief  This function is called by a CPU after a SelfSuspend call to
 * notify the platform management controller that CPU has aborted suspend
 * or in response to an init suspend request when the PU refuses to suspend.
 *
 * @param  Reason Reason code why the suspend can not be performed or completed
 * - ABORT_REASON_WKUP_EVENT : local wakeup-event received
 * - ABORT_REASON_PU_BUSY : PU is busy
 * - ABORT_REASON_NO_PWRDN : no external powerdown supported
 * - ABORT_REASON_UNKNOWN : unknown error during suspend procedure
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_AbortSuspend(const enum XPmAbortReason Reason)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_ABORT_SUSPEND, 2U, NULL, Reason, PrimaryProc->DevId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * Do client specific abort suspend operations
	 * (e.g. enable interrupts and clear powerdown request bit)
	 */
	XPm_ClientAbortSuspend();

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief This function is used to force power down the subsystem if the
 * subsystem is unresponsive and by calling this API all the resources of
 * that subsystem will be automatically released.
 *
 * @param  TargetDevId	Subsystem ID to be forced powered down.
 * @param  Ack		Requested acknowledge type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Force power down may not be requested by a PU for itself.
 *
 ****************************************************************************/
XStatus XPm_ForcePowerDown(const u32 TargetDevId, const u32 Ack)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_FORCE_POWERDOWN, 2U, NULL, TargetDevId, Ack);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a privileged PU to shut down
 * or restart the complete device.
 *
 * @param  Type		Shutdown type (shutdown/restart)
 * @param  SubType	Shutdown subtype (subsystem-only/PU-only/system)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_SystemShutdown(const u32 Type, const u32 SubType)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_SYSTEM_SHUTDOWN, 2U, NULL, Type, SubType);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used by a CPU to set wakeup source
 *
 * @param  TargetDeviceId	Device ID of the target
 * @param  DeviceId		Device ID used as wakeup source
 * @param  Enable		1 - Enable, 0 - Disable
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_SetWakeUpSource(const u32 TargetDeviceId, const u32 DeviceId,
			    const u32 Enable)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_SET_WAKEUP_SOURCE, 3U, NULL,
				    TargetDeviceId, DeviceId, Enable);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function queries information about the platform resources.
 *
 * @param QueryId	The type of data to query
 * @param Arg1		Query argument 1
 * @param Arg2		Query argument 2
 * @param Arg3		Query argument 3
 * @param Data		Pointer to the output data
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPm_Query(const u32 QueryId, const u32 Arg1, const u32 Arg2,
		  const u32 Arg3, u32 *const Data)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG] = {0};

	if (NULL == Data) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	Status = XPm_GenericRequest(PM_QUERY_DATA, 4U, RetPayload, QueryId, Arg1, Arg2, Arg3);

	switch (QueryId) {
	case (u32)XPM_QID_CLOCK_GET_NAME:
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_NAME:
		/*
		 * XPM_QID_CLOCK_GET_NAME and XPM_QID_PINCTRL_GET_FUNCTION_NAME store
		 * part of their clock names in Status variable which is stored
		 * in response. So this value should not be treated as error code.
		 * Consider error only if clock name is not found.
		 */
		if (XST_SUCCESS != Status) {
			Data[0] = (u32)('\0');
			Status = (s32)XST_FAILURE;
		} else {
#if !(defined(XPM_SUPPORT) && defined(__aarch64__) && defined(EL1_NONSECURE) && (EL1_NONSECURE == 1))
			Data[0] = (u32)Status;
#endif
			Data[1] = RetPayload[0];
			Data[2] = RetPayload[1];
			Data[3] = RetPayload[2];
		}
		break;

	case (u32)XPM_QID_CLOCK_GET_TOPOLOGY:
	case (u32)XPM_QID_CLOCK_GET_MUXSOURCES:
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_GROUPS:
	case (u32)XPM_QID_PINCTRL_GET_PIN_GROUPS:
	case (u32)XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS:
	case (u32)XPM_QID_CLOCK_GET_ATTRIBUTES:
	case (u32)XPM_QID_PINCTRL_GET_NUM_PINS:
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTIONS:
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS:
	case (u32)XPM_QID_CLOCK_GET_NUM_CLOCKS:
	case (u32)XPM_QID_CLOCK_GET_MAX_DIVISOR:
	case (u32)XPM_QID_PLD_GET_PARENT:
	case (u32)XPM_QID_PINCTRL_GET_ATTRIBUTES:
		if (XST_SUCCESS == Status) {
			Data[0] = RetPayload[0];
			Data[1] = RetPayload[1];
			Data[2] = RetPayload[2];
		}
		break;

	default:
		Status = (s32)XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used by a CPU to announce a change in the
 *	   maximum wake-up latency requirements for a specific device
 *	   currently used by that CPU.
 *
 * @param  DeviceId	Device ID.
 * @param  Latency	Maximum wake-up latency required.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Setting maximum wake-up latency can constrain the set of
 *	   possible power states a resource can be put into.
 *
 ****************************************************************************/
XStatus XPm_SetMaxLatency(const u32 DeviceId, const u32 Latency)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_SET_MAX_LATENCY, 2U, NULL, DeviceId, Latency);

	return Status;
}

/****************************************************************************/
/**
 * @brief  Call this function to request the power management controller to
 * return information about an operating characteristic of a component.
 *
 * @param  DeviceId   Device ID.
 * @param  Type       Type of operating characteristic requested:
 *		      - power (current power consumption),
 *		      - latency (current latency in micro seconds to return
 *				 to active state),
 *		      - temperature (current temperature in Celsius
 *				     (Q8.7 format)),
 * @param  Result     Used to return the requested operating characteristic.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Currently power type is not supported for Versal.
 *
 ****************************************************************************/
XStatus XPm_GetOpCharacteristic(const u32 DeviceId,
				const enum XPmOpCharType Type,
				u32 *const Result)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == Result) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_GET_OP_CHARACTERISTIC, 2U, RetPayload, DeviceId, Type);
	if (XST_SUCCESS == Status) {
		*Result = RetPayload[0];
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is called to notify the power management controller
 * about the completed power management initialization.
 *
 * @return XST_SUCCESS if successful, otherwise an error code
 *
 ****************************************************************************/
XStatus XPm_InitFinalize(void)
{
	XStatus Status = (s32)XST_FAILURE;

	Status = XPm_GenericRequest(PM_INIT_FINALIZE, 0U, NULL);

	return Status;
}

/****************************************************************************/
/**
 * @brief  A PU can call this function to request that the power management
 * controller call its notify callback whenever a qualifying event occurs.
 * One can request to be notified for a specific or any event related to
 * a specific node.
 *
 * @param  Notifier Pointer to the notifier object to be associated with
 * the requested notification.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   The caller shall initialize the notifier object before invoking
 * the XPm_RegisteredNotifier function. While notifier is registered,
 * the notifier object shall not be modified by the caller.
 *
 ****************************************************************************/
XStatus XPm_RegisterNotifier(XPm_Notifier* const Notifier)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 Version;

	if (NULL == Notifier) {
		XPm_Err("NULL notifier pointer\n");
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Version = XPm_GetRegisterNotifierVersionServer();
	if (0U == Version) {
		goto done;
	}

	/* Send request to the target module */
	Status = XPm_GenericRequest(PM_REGISTER_NOTIFIER, 4U, NULL,
				    Notifier->node, Notifier->event, Notifier->flags, 1);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Add notifier in the list only if target module has it registered */
	Status = XPm_NotifierAdd(Notifier);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  A PU calls this function to unregister for the previously
 * requested notifications.
 *
 * @param  Notifier Pointer to the notifier object associated with the
 * previously requested notification
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_UnregisterNotifier(XPm_Notifier* const Notifier)
{
	XStatus Status = (s32)XST_FAILURE;

	if (NULL == Notifier) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Remove first the notifier from the list. If it's not in the list
	 * report an error, and don't trigger target module since it don't have
	 * it registered either.
	 */
	Status = XPm_NotifierRemove(Notifier);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Send request to the target module (enable=0 to unregister) */
	Status = XPm_GenericRequest(PM_REGISTER_NOTIFIER, 4U, NULL,
				    Notifier->node, Notifier->event,
				    0, 0);

done:
	return Status;
}

/* Callback API functions */
struct pm_init_suspend pm_susp = {
	.received = 0U,
/* initialization of other fields is irrelevant while 'received' is false */
};

struct pm_acknowledge pm_ack = {
	.received = 0U,
/* initialization of other fields is irrelevant while 'received' is false */
};

/****************************************************************************/
/**
 * @brief  Callback function to be implemented in each PU, allowing the power
 * management controller to request that the PU suspend itself.
 *
 * @param  Reason  Suspend reason:
 * - SUSPEND_REASON_PU_REQ : Request by another PU
 * - SUSPEND_REASON_ALERT : Unrecoverable SysMon alert
 * - SUSPEND_REASON_SHUTDOWN : System shutdown
 * - SUSPEND_REASON_RESTART : System restart
 * @param  Latency Maximum wake-up latency in us(micro secs). This information
 * can be used by the PU to decide what level of context saving may be
 * required.
 * @param  State   Targeted sleep/suspend state.
 * @param  Timeout Timeout in ms, specifying how much time a PU has to initiate
 * its suspend procedure before it's being considered unresponsive.
 *
 * @return None
 *
 * @note   If the PU fails to act on this request the power management
 * controller or the requesting PU may choose to employ the forceful
 * power down option.
 *
 ****************************************************************************/
void XPm_InitSuspendCb(const enum XPmSuspendReason Reason,
		       const u32 Latency, const u32 State, const u32 Timeout)
{
	if (1U == pm_susp.received) {
		XPm_Dbg("%s: WARNING: dropping unhandled init suspend request!\n", __func__);
		XPm_Dbg("Dropped %s (%d, %d, %d, %d)\n", __func__, pm_susp.reason,
			pm_susp.latency, pm_susp.state, pm_susp.timeout);
	}
	XPm_Dbg("%s (%d, %d, %d, %d)\n", __func__, Reason, Latency, State, Timeout);

	pm_susp.reason = Reason;
	pm_susp.latency = Latency;
	pm_susp.state = State;
	pm_susp.timeout = Timeout;
	pm_susp.received = 1U;
}

/****************************************************************************/
/**
 * @brief  This function is called by the power management controller in
 * response to any request where an acknowledge callback was requested,
 * i.e. where the 'ack' argument passed by the PU was REQUEST_ACK_NON_BLOCKING.
 *
 * @param  Node    ID of the component or sub-system in question.
 * @param  Status  Status of the operation:
 * - OK: the operation completed successfully
 * - ERR: the requested operation failed
 * @param  Oppoint Operating point of the node in question
 *
 * @return None
 *
 ****************************************************************************/
void XPm_AcknowledgeCb(const u32 Node, const XStatus Status, const u32 Oppoint)
{
	if (1U == pm_ack.received) {
		XPm_Dbg("%s: WARNING: dropping unhandled acknowledge!\n", __func__);
		XPm_Dbg("Dropped %s (%d, %d, %d)\n", __func__, pm_ack.node,
			pm_ack.status, pm_ack.opp);
	}
	XPm_Dbg("%s (%d, %d, %d)\n", __func__, Node, Status, Oppoint);

	pm_ack.node = Node;
	pm_ack.status = Status;
	pm_ack.opp = Oppoint;
	pm_ack.received = 1U;
}

/****************************************************************************/
/**
 * @brief  This function is called by the power management controller if an
 * event the PU was registered for has occurred. It will populate the notifier
 * data structure passed when calling XPm_RegisterNotifier.
 *
 * @param  Node     ID of the device the event notification is related to.
 * @param  Event    ID of the event
 * @param  Oppoint  Current operating state of the device.
 *
 * @return None
 *
 ****************************************************************************/
void XPm_NotifyCb(const u32 Node, const u32 Event, const u32 Oppoint)
{
	XPm_Dbg("%s (%d, %d, %d)\n", __func__, Node, Event, Oppoint);
	XPm_NotifierProcessEvent(Node, Event, Oppoint);
}

#if defined  (XPM_SUPPORT) && (__aarch64__) && (EL1_NONSECURE == 1)
/**
 * XPm_HandlePmNotification() - Handle a pending PM notification (SMC mode).
 *
 * Calls PM_GET_CALLBACK_DATA (0xa01) TF-A SMC to read the IPI payload.
 * Can be invoked from an SGI handler (interrupt-driven) or called directly
 * for polling.
 *
 * Return format from PM_GET_CALLBACK_DATA:
 *   x0 = callback_type | (arg1 << 32)
 *   x1 = arg2 | (arg3 << 32)
 *
 * For PM_NOTIFY_CB (type=32): arg1=node, arg2=event, arg3=oppoint.
 */
void XPm_HandlePmNotification(void)
{
	XSmc_OutVar Out = Xil_Smc(PM_GET_CALLBACK_DATA_SMC_FID,
				  0U, 0U, 0U, 0U, 0U, 0U, 0U);
	u32 cb_type = lower_32_bits(Out.Arg0);

	if (cb_type == PM_NOTIFY_CB_TYPE) {
		u32 node = upper_32_bits(Out.Arg0);
		u32 event = lower_32_bits(Out.Arg1);
		u32 oppoint = upper_32_bits(Out.Arg1);
		XPm_NotifyCb(node, event, oppoint);
	}
}

/**
 * XPm_RegisterSgi() - Register an SGI with TF-A for PM notifications.
 *
 * Tells TF-A to signal the non-secure world via the specified SGI number
 * whenever an IPI callback arrives from PLM. The caller must first set up
 * a GIC handler for this SGI that calls XPm_HandlePmNotification().
 *
 * @param  SgiNum  SGI interrupt number to register (0-15).
 *
 * @return XST_SUCCESS on success, error code otherwise.
 *
 * @note   Uses old-format SMC (TF_A_PM_REGISTER_SGI_SMC_FID) because
 *         this is a TF-A-specific API handled by TF_A_specific_handler,
 *         not a PLM pass-through command.
 */
XStatus XPm_RegisterSgi(u32 SgiNum)
{
	XSmc_OutVar Out;

	Out = Xil_Smc(TF_A_PM_REGISTER_SGI_SMC_FID, (u64)SgiNum,
		      0U, 0U, 0U, 0U, 0U, 0U);

	return (XStatus)lower_32_bits(Out.Arg0);
}

/**
 * XPm_UnregisterSgi() - Unregister SGI from TF-A for PM notifications.
 *
 * Tells TF-A to stop sending SGI notifications to the non-secure world.
 * The caller should disable and disconnect the GIC handler afterwards.
 *
 * @return XST_SUCCESS on success, error code otherwise.
 *
 * @note   Uses old-format SMC (TF_A_PM_REGISTER_SGI_SMC_FID) because
 *         this is a TF-A-specific API handled by TF_A_specific_handler,
 *         not a PLM pass-through command.
 */
XStatus XPm_UnregisterSgi(void)
{
	XSmc_OutVar Out;

	Out = Xil_Smc(TF_A_PM_REGISTER_SGI_SMC_FID, ((u64)1U << 32),
		      0U, 0U, 0U, 0U, 0U, 0U);

	return (XStatus)lower_32_bits(Out.Arg0);
}
#endif

/** @cond INTERNAL */
XStatus XPm_SetConfiguration(const u32 Address)
{
	/* Suppress compilation warning */
	(void)Address;

	XPm_Err("%s() API is not supported\r\n", __func__);
	return (s32)XST_SUCCESS;
}

XStatus XPm_MmioWrite(const u32 Address, const u32 Mask, const u32 Value)
{
	/* Suppress compilation warning */
	(void)Address;
	(void)Mask;
	(void)Value;

	XPm_Err("%s() API is not supported\r\n", __func__);
	return (s32)XST_FAILURE;
}

XStatus XPm_MmioRead(const u32 Address, u32 *const Value)
{
	/* Suppress compilation warning */
	(void)Address;
	(void)Value;

	XPm_Err("%s() API is not supported\r\n", __func__);
	return (s32)XST_FAILURE;
}
/** @endcond */

/****************************************************************************/
/**
 * @brief  This function queries information about the feature version.
 *
 * @param FeatureId	The feature ID (API-ID)
 * @param Version	Pointer to array of words (32-bit).
 *     - version[0] - If fails 0, else EEMI API ID version number (positive integer).
 *		      This is supported in version 1 and 2 of PM_FEATURE_CHECK.
 *     - version[1] - lower 32-bit (0 - 31) bitmask to check if IOCTL or QUERY ID
		      is supported. If Bit 1 is set for PM_IOCTL, then IOCTL 1 is
 *		      supported. Only supported if API PM_FEATURE_CHECK version is 2.
 *     - version[2] - upper 32-bit bitmask to check support of IOCTL ID or QUERY ID.
 *		      if bit 0 is set for PM_IOCTL, then IOCTL number 32 is supported.
 *		      Only supported if API PM_FEATURE_CHECK version is 2.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPm_FeatureCheck(const u32 FeatureId, u32 *Version)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 RetPayload[MAX_RESPONSE_ARG];

	if (NULL == Version) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_GenericRequest(PM_FEATURE_CHECK, 1U, RetPayload, FeatureId);
	if (XST_SUCCESS == Status) {
		*Version = RetPayload[0];
	}

done:
	return Status;
}
