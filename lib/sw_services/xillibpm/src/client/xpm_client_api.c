/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#include "xpm_client_api.h"
#include "xpm_client_ipi.h"

/* Payload Packets */
#define PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5)	\
	Payload[0] = (u32)Arg0;						\
	Payload[1] = (u32)Arg1;						\
	Payload[2] = (u32)Arg2;						\
	Payload[3] = (u32)Arg3;						\
	Payload[4] = (u32)Arg4;						\
	Payload[5] = (u32)Arg5;						\
	XPm_Dbg("%s(%x, %x, %x, %x, %x)\r\n", __func__, Arg1, Arg2, Arg3, Arg4, Arg5);

#define LIBPM_MODULE_ID			(0x02)

#define HEADER(len, ApiId)		((len << 16) | (LIBPM_MODULE_ID << 8) | (ApiId))

#define PACK_PAYLOAD0(Payload, ApiId) \
	PACK_PAYLOAD(Payload, HEADER(0, ApiId), 0, 0, 0, 0, 0)
#define PACK_PAYLOAD1(Payload, ApiId, Arg1) \
	PACK_PAYLOAD(Payload, HEADER(1, ApiId), Arg1, 0, 0, 0, 0)
#define PACK_PAYLOAD2(Payload, ApiId, Arg1, Arg2) \
	PACK_PAYLOAD(Payload, HEADER(2, ApiId), Arg1, Arg2, 0, 0, 0)
#define PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2, Arg3) \
	PACK_PAYLOAD(Payload, HEADER(3, ApiId), Arg1, Arg2, Arg3, 0, 0)
#define PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
	PACK_PAYLOAD(Payload, HEADER(4, ApiId), Arg1, Arg2, Arg3, Arg4, 0)
#define PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
	PACK_PAYLOAD(Payload, HEADER(5, ApiId), Arg1, Arg2, Arg3, Arg4, Arg5)

/****************************************************************************/
/**
 * @brief  Initialize xillibpm library
 *
 * @param  IpiInst Pointer to IPI driver instance
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPmClient_InitXillibpm(XIpiPsu *IpiInst)
{
	XStatus Status = XST_SUCCESS;

	if (NULL == IpiInst) {
		XPm_Dbg("ERROR passing NULL pointer to %s\r\n", __func__);
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XPmClient_SetPrimaryProc();

	PrimaryProc->Ipi = IpiInst;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to request the version number of the API
 * running on the platform management controller.
 *
 * @param  version Returns the API 32-bit version number.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetApiVersion(u32 *Version)
{
	XStatus Status;
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
 * @param  TargetSubsystemId	Targeted Id of subsystem
 * @param  DeviceId		Device which needs to be requested
 * @param  Capabilities		Device Capabilities, can be combined
 *				- PM_CAP_ACCESS  : full access / functionality
 *				- PM_CAP_CONTEXT : preserve context
 *				- PM_CAP_WAKEUP  : emit wake interrupts
 * @param  Latency		Maximum wake-up latency in us
 * @param  QoS			Quality of Service (0-100) required
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_RequestDevice(const u32 TargetSubsystemId, const u32 DeviceId,
				const u32 Capabilities, const u32 Latency,
				const u32 QoS)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD5(Payload, PM_REQUEST_DEVICE, TargetSubsystemId, DeviceId,
		      Capabilities, Latency, QoS);

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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_ReleaseDevice(const u32 DeviceId)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD1(Payload, PM_RELEASE_DEVICE, DeviceId);

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
 * @param  Latency		Maximum wake-up latency in us
 * @param  QoS			Quality of Service (0-100) required
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SetRequirement(const u32 DeviceId, const u32 Capabilities,
				 const u32 Latency, const u32 QoS)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD4(Payload, PM_SET_REQUIREMENT, DeviceId, Capabilities, Latency, QoS);

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
 * @param  DeviceStatus		Structure pointer to store device status
 * 				- Status - The current power state of the device
 * 				 - For CPU nodes:
 * 				  - 0 : if CPU is powered down,
 * 				  - 1 : if CPU is active (powered up),
 * 				  - 2 : if CPU is suspending (powered up)
 * 				 - For power islands and power domains:
 * 				  - 0 : if island is powered down,
 * 				  - 1 : if island is powered up
 * 				 - For slaves:
 * 				  - 0 : if slave is powered down,
 * 				  - 1 : if slave is powered up,
 * 				  - 2 : if slave is in retention
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetDeviceStatus(const u32 DeviceId,
				  XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == DeviceStatus) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_GET_DEVICE_STATUS, DeviceId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, &DeviceStatus->Status,
				   &DeviceStatus->Requirement,
				   &DeviceStatus->Usage);

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
 *				- 1 for Assert Reset
 *				- 2 for Release Reset
 *				- 3 for Pulse Reset
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_ResetAssert(const u32 ResetId, const u32 Action)
{
	XStatus Status;
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
 *				- 1 for reset asserted
 *				- 2 for reset released
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_ResetGetStatus(const u32 ResetId, u32 *const State)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == State) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_PinCtrlRequest(const u32 PinId)
{
	XStatus Status;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_PinCtrlRelease(const u32 PinId)
{
	XStatus Status;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SetPinFunction(const u32 PinId, const u32 FunctionId)
{
	XStatus Status;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetPinFunction(const u32 PinId, u32 *const FunctionId)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == FunctionId) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
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
 * ----------------------------------------------------------------------------
 *  ParamId				| ParamVal
 * ----------------------------------------------------------------------------
 *  PINCTRL_CONFIG_SLEW_RATE		| PINCTRL_SLEW_RATE_SLOW
 *					| PINCTRL_SLEW_RATE_FAST
 *					|
 *  PINCTRL_CONFIG_BIAS_STATUS		| PINCTRL_BIAS_DISABLE
 *					| PINCTRL_BIAS_ENABLE
 *					|
 *  PINCTRL_CONFIG_PULL_CTRL		| PINCTRL_BIAS_PULL_DOWN
 *					| PINCTRL_BIAS_PULL_UP
 *					|
 *  PINCTRL_CONFIG_SCHMITT_CMOS		| PINCTRL_INPUT_TYPE_CMOS
 *					| PINCTRL_INPUT_TYPE_SCHMITT
 *					|
 *  PINCTRL_CONFIG_DRIVE_STRENGTH	| PINCTRL_DRIVE_STRENGTH_TRISTATE
 *					| PINCTRL_DRIVE_STRENGTH_4MA
 *					| PINCTRL_DRIVE_STRENGTH_8MA
 *					| PINCTRL_DRIVE_STRENGTH_12MA
 *					|
 *  PINCTRL_CONFIG_TRI_STATE		| PINCTRL_TRI_STATE_DISABLE
 *					| PINCTRL_TRI_STATE_ENABLE
 * ----------------------------------------------------------------------------
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SetPinParameter(const u32 PinId, const u32 ParamId, const u32 ParamVal)
{
	XStatus Status;
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
 * ----------------------------------------------------------------------------
 *  ParamId				| ParamVal
 * ----------------------------------------------------------------------------
 *  PINCTRL_CONFIG_SLEW_RATE		| PINCTRL_SLEW_RATE_SLOW
 *					| PINCTRL_SLEW_RATE_FAST
 *					|
 *  PINCTRL_CONFIG_BIAS_STATUS		| PINCTRL_BIAS_DISABLE
 *					| PINCTRL_BIAS_ENABLE
 *					|
 *  PINCTRL_CONFIG_PULL_CTRL		| PINCTRL_BIAS_PULL_DOWN
 *					| PINCTRL_BIAS_PULL_UP
 *					|
 *  PINCTRL_CONFIG_SCHMITT_CMOS		| PINCTRL_INPUT_TYPE_CMOS
 *					| PINCTRL_INPUT_TYPE_SCHMITT
 *					|
 *  PINCTRL_CONFIG_DRIVE_STRENGTH	| PINCTRL_DRIVE_STRENGTH_TRISTATE
 *					| PINCTRL_DRIVE_STRENGTH_4MA
 *					| PINCTRL_DRIVE_STRENGTH_8MA
 *					| PINCTRL_DRIVE_STRENGTH_12MA
 *					|
 *  PINCTRL_CONFIG_VOLTAGE_STATUS	| 1 for 1.8v mode
 *					| 0 for 3.3v mode
 *					|
 *  PINCTRL_CONFIG_TRI_STATE		| PINCTRL_TRI_STATE_DISABLE
 *					| PINCTRL_TRI_STATE_ENABLE
 * ----------------------------------------------------------------------------
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetPinParameter(const u32 PinId, const u32 ParamId, u32 *const ParamVal)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == ParamVal) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
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
