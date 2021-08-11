/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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

	if (NULL == IpiInst) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	XPm_SetPrimaryProc();

	PrimaryProc->Ipi = IpiInst;

	Status = (s32)XST_SUCCESS;

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

	XPm_SetPrimaryProc();

	/* Error out if primary proc is not defined */
	if (NULL == PrimaryProc) {
		Ret = PM_BOOT_ERROR;
		goto done;
	}

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD0(Payload, PM_GET_CHIPID);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, IDCode, Version, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD0(Payload, PM_GET_API_VERSION);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, Version, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD4(Payload, PM_REQUEST_NODE, DeviceId, Capabilities,
		      QoS, Ack);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD1(Payload, PM_RELEASE_NODE, DeviceId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD4(Payload, PM_SET_REQUIREMENT, DeviceId, Capabilities, QoS, Ack);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get the device status
 *
 * @param  DeviceId		Device for which status is requested
 * @param  NodeStatus		Structure pointer to store device status
 * 				- Status - The current power state of the device
 * 				 - For CPU nodes:
 * 				  - 0 : if CPU is powered down,
 * 				  - 1 : if CPU is active (powered up),
 * 				  - 8 : if CPU is suspending (powered up)
 * 				 - For power islands and power domains:
 * 				  - 0 : if island is powered down,
 * 				  - 2 : if island is powered up
 * 				 - For slaves:
 * 				  - 0 : if slave is powered down,
 * 				  - 1 : if slave is powered up,
 * 				  - 9 : if slave is in retention
 *
 * 				- Requirement - Requirements placed on the device by the caller
 *
 * 				- Usage
 * 				 - 0 : node is not used by any PU,
 * 				 - 1 : node is used by caller exclusively,
 * 				 - 2 : node is used by other PU(s) only,
 * 				 - 3 : node is used by caller and by other PU(s)
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == NodeStatus) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_GET_NODE_STATUS, DeviceId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, &NodeStatus->status,
				   &NodeStatus->requirements,
				   &NodeStatus->usage);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_RESET_ASSERT, ResetId, Action);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == State) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_RESET_GET_STATUS, ResetId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, State, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD1(Payload, PM_PINCTRL_REQUEST, PinId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD1(Payload, PM_PINCTRL_RELEASE, PinId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_PINCTRL_SET_FUNCTION, PinId, FunctionId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == FunctionId) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_PINCTRL_GET_FUNCTION, PinId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, FunctionId, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD3(Payload, PM_PINCTRL_CONFIG_PARAM_SET, PinId, ParamId, ParamVal);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == ParamVal) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD2(Payload, PM_PINCTRL_CONFIG_PARAM_GET, PinId, ParamId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, ParamVal, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function performs driver-like IOCTL functions on shared system
 * devices.
 *
 * @param DeviceId              ID of the device
 * @param IoctlId               IOCTL function ID
 * @param Arg1                  Argument 1
 * @param Arg2                  Argument 2
 * @param Response              Ioctl response
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Response) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD4(Payload, PM_IOCTL, DeviceId, IoctlId, Arg1, Arg2);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, Response, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD1(Payload, PM_CLOCK_ENABLE, ClockId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD1(Payload, PM_CLOCK_DISABLE, ClockId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == State) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_CLOCK_GETSTATE, ClockId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, State, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_CLOCK_SETDIVIDER, ClockId, Divider);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Divider) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_CLOCK_GETDIVIDER, ClockId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, Divider, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_CLOCK_SETPARENT, ClockId, ParentIdx);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == ParentIdx) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_CLOCK_GETPARENT, ClockId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, ParentIdx, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to get rate of specified clock
 *
 * @param  ClockId	Clock ID
 * @param  Rate		Pointer to store the rate clock
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPm_ClockGetRate(const u32 ClockId, u32 *const Rate)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD1(Payload, PM_CLOCK_GETRATE, ClockId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, Rate, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the rate of specified clock
 *
 * @param  ClockId	Clock ID
 * @param  Rate		Clock rate
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPm_ClockSetRate(const u32 ClockId, const u32 Rate)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_CLOCK_SETRATE, ClockId, Rate);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD3(Payload, PM_PLL_SET_PARAMETER, ClockId, ParamId, Value);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Value) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD2(Payload, PM_PLL_GET_PARAMETER, ClockId, ParamId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, Value, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_PLL_SET_MODE, ClockId, Value);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Value) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_PLL_GET_MODE, ClockId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, Value, NULL, NULL);

done:
	return Status;
}

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
	u32 Payload[PAYLOAD_ARG_CNT];
	struct XPm_Proc *Proc;

	Proc = XPm_GetProcByDeviceId(DeviceId);
	if (NULL == Proc) {
		XPm_Err("Invalid Device ID\r\n");
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	XPm_ClientSuspend(Proc);

	PACK_PAYLOAD5(Payload, PM_SELF_SUSPEND, DeviceId, Latency, State,
		      (u32)Address, (u32)(Address >> 32));

	/* Send request to the target module */
	Status = XPm_IpiSend(Proc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(Proc, NULL, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used to request power up of a CPU node
 * within the same PU, or to power up another PU.
 *
 * @param  TargetDevId     Device ID of the CPU or PU to be powered/woken up.
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
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 EncodedAddr;
	struct XPm_Proc *Proc;

	Proc = XPm_GetProcByDeviceId(TargetDevId);

	XPm_ClientWakeUp(Proc);

	/* encode set Address into 1st bit of address */
	EncodedAddr = Address | ((1U == SetAddress) ? 1U : 0U);

	PACK_PAYLOAD4(Payload, PM_REQUEST_WAKEUP, TargetDevId, (u32)EncodedAddr,
			(u32)(EncodedAddr >> 32), Ack);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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

/****************************************************************************/
/**
 * @brief  This function is used by a CPU to request suspend to another CPU.
 *
 * @param  TargetSubsystemId	Subsystem ID of the target
 * @param  Ack			Requested acknowledge type
 * @param  Latency		Maximum wake-up latency requirement in us(microsecs)
 * @param  State		Power State
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 *
 *
 ****************************************************************************/
XStatus XPm_RequestSuspend(const u32 TargetSubsystemId, const u32 Ack,
			   const u32 Latency, const u32 State)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD4(Payload, PM_REQUEST_SUSPEND, TargetSubsystemId, Ack, Latency, State);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_ABORT_SUSPEND, Reason, PrimaryProc->DevId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);
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
 * @brief  This function is used by PU to request a forced poweroff of another
 * PU or its power island or power domain. This can be used for killing an
 * unresponsive PU, in which case all resources of that PU will be
 * automatically released.
 *
 * @param  TargetDevId	Device ID of the PU node to be forced powered down.
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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_FORCE_POWERDOWN, TargetDevId, Ack);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_SYSTEM_SHUTDOWN, Type, SubType);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD3(Payload, PM_SET_WAKEUP_SOURCE, TargetDeviceId, DeviceId, Enable);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Data) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD4(Payload, PM_QUERY_DATA, QueryId, Arg1, Arg2, Arg3);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	switch (QueryId) {
	case (u32)XPM_QID_CLOCK_GET_NAME:
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_NAME:
		/*
		 * XPM_QID_CLOCK_GET_NAME and XPM_QID_PINCTRL_GET_FUNCTION_NAME store
		 * part of their clock names in Status variable which is stored
		 * in response. So this value should not be treated as error code.
		 * Consider error only if clock name is not found.
		 */
		Status = Xpm_IpiReadBuff32(PrimaryProc, &Data[1], &Data[2], &Data[3]);
		if (XST_SUCCESS != Status) {
			Data[0] = (u32)('\0');
			Status = (s32)XST_FAILURE;
		} else {
			Data[0] = (u32)Status;
		}
		break;

	case (u32)XPM_QID_CLOCK_GET_TOPOLOGY:
	case (u32)XPM_QID_CLOCK_GET_MUXSOURCES:
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_GROUPS:
	case (u32)XPM_QID_PINCTRL_GET_PIN_GROUPS:
		Status = Xpm_IpiReadBuff32(PrimaryProc, &Data[0], &Data[1], &Data[2]);
		break;

	case (u32)XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS:
		Status = Xpm_IpiReadBuff32(PrimaryProc, &Data[0], &Data[1], NULL);
		break;

	case (u32)XPM_QID_CLOCK_GET_ATTRIBUTES:
	case (u32)XPM_QID_PINCTRL_GET_NUM_PINS:
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTIONS:
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS:
	case (u32)XPM_QID_CLOCK_GET_NUM_CLOCKS:
	case (u32)XPM_QID_CLOCK_GET_MAX_DIVISOR:
	case (u32)XPM_QID_PLD_GET_PARENT:
		Status = Xpm_IpiReadBuff32(PrimaryProc, &Data[0], NULL, NULL);
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
 *         maximum wake-up latency requirements for a specific device
 *         currently used by that CPU.
 *
 * @param  DeviceId	Device ID.
 * @param  Latency	Maximum wake-up latency required.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   Setting maximum wake-up latency can constrain the set of
 *         possible power states a resource can be put into.
 *
 ****************************************************************************/
XStatus XPm_SetMaxLatency(const u32 DeviceId, const u32 Latency)
{
	XStatus Status = (s32)XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_SET_MAX_LATENCY, DeviceId, Latency);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Read the result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Call this function to request the power management controller to
 * return information about an operating characteristic of a component.
 *
 * @param  DeviceId   Device ID.
 * @param  Type       Type of operating characteristic requested:
 *                    - power (current power consumption),
 *                    - latency (current latency in micro seconds to return
 *                               to active state),
 *                    - temperature (current temperature in Celsius
 *                                   (Q8.7 format)),
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
	u32 Payload[PAYLOAD_ARG_CNT];

	/* Send request to the target module */
	PACK_PAYLOAD2(Payload, PM_GET_OP_CHARACTERISTIC, DeviceId, Type);
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, Result, NULL, NULL);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD0(Payload, PM_INIT_FINALIZE);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);

done:
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
	u32 Payload[PAYLOAD_ARG_CNT];
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
	PACK_PAYLOAD4(Payload, PM_REGISTER_NOTIFIER, Notifier->node,
		      Notifier->event, Notifier->flags, 1);
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);
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
	u32 Payload[PAYLOAD_ARG_CNT];

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

	/* Send request to the target module */
	PACK_PAYLOAD4(Payload, PM_REGISTER_NOTIFIER, Notifier->node,
		      Notifier->event, 0, 0);
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, NULL, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

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
 * @param Version	Pointer to the output data where  version of
 *			feature store.
 *			For the supported feature get non zero value in version,
 *			But if version is 0U that means feature not supported.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPm_FeatureCheck(const u32 FeatureId, u32 *Version)
{

	XStatus Status = (s32)XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Version) {
		XPm_Err("Passing NULL pointer to %s\r\n", __func__);
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_FEATURE_CHECK, FeatureId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, Version, NULL, NULL);

done:
	return Status;
}
