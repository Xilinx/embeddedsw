/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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

/****************************************************************************/
/**
 * @brief  This function is used to enable the specified clock
 *
 * @param  ClockId		Clock ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_ClockEnable(const u32 ClockId)
{
	XStatus Status;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_ClockDisable(const u32 ClockId)
{
	XStatus Status;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetClockState(const u32 ClockId, u32 *const State)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == State) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SetClockDivider(const u32 ClockId, const u32 Divider)
{
	XStatus Status;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetClockDivider(const u32 ClockId, u32 *const Divider)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Divider) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
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
 * @param  ParentId		Parent ID which needs to be set
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SetClockParent(const u32 ClockId, const u32 ParentId)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD2(Payload, PM_CLOCK_SETPARENT, ClockId, ParentId);

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
 * @param  ParentId		Pointer to store the parent ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetClockParent(const u32 ClockId, u32 *const ParentId)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == ParentId) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
		goto done;
	}

	PACK_PAYLOAD1(Payload, PM_CLOCK_GETPARENT, ClockId);

	/* Send request to the target module */
	Status = XPm_IpiSend(PrimaryProc, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Return result from IPI return buffer */
	Status = Xpm_IpiReadBuff32(PrimaryProc, ParentId, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to set the parameters for specified PLL clock
 *
 * @param  ClockId		Clock ID
 * @param  ParamId		Parameter ID
 *				- PLL_PARAM_ID_CLKOUTDIV
 *				- PLL_PARAM_ID_FBDIV
 *				- PLL_PARAM_ID_FRAC_DATA
 *				- PLL_PARAM_ID_PRE_SRC
 *				- PLL_PARAM_ID_POST_SRC
 *				- PLL_PARAM_ID_LOCK_DLY
 *				- PLL_PARAM_ID_LOCK_CNT
 *				- PLL_PARAM_ID_LFHF
 *				- PLL_PARAM_ID_CP
 *				- PLL_PARAM_ID_RES
 * @param  Value		Value of parameter
 *				(See register description for possible values)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SetPllParameter(const u32 ClockId,
				  const enum XPm_PllConfigParams ParamId,
				  const u32 Value)
{
	XStatus Status;
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
 *				- PLL_PARAM_ID_CLKOUTDIV
 *				- PLL_PARAM_ID_FBDIV
 *				- PLL_PARAM_ID_FRAC_DATA
 *				- PLL_PARAM_ID_PRE_SRC
 *				- PLL_PARAM_ID_POST_SRC
 *				- PLL_PARAM_ID_LOCK_DLY
 *				- PLL_PARAM_ID_LOCK_CNT
 *				- PLL_PARAM_ID_LFHF
 *				- PLL_PARAM_ID_CP
 *				- PLL_PARAM_ID_RES
 * @param  Value		Pointer to store parameter value
 *				(See register description for possible values)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetPllParameter(const u32 ClockId,
				  const enum XPm_PllConfigParams ParamId,
				  u32 *const Value)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Value) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SetPllMode(const u32 ClockId, const u32 Value)
{
	XStatus Status;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_GetPllMode(const u32 ClockId, u32 *const Value)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	if (NULL == Value) {
		XPm_Dbg("ERROR: Passing NULL pointer to %s\r\n", __func__);
		Status = XST_FAILURE;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SelfSuspend(const u32 DeviceId, const u32 Latency,
			      const u8 State, const u64 Address)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];
	struct XPm_Proc *Proc;

	Proc = XpmClient_GetProcByDeviceId(DeviceId);
	if (NULL == Proc) {
		XPm_Dbg("ERROR: Invalid Device ID\r\n");
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XPmClient_Suspend(Proc);

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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_RequestWakeUp(const u32 TargetDevId, const bool SetAddress,
				const u64 Address, const u32 Ack)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 EncodedAddr;
	struct XPm_Proc *Proc;

	Proc = XpmClient_GetProcByDeviceId(TargetDevId);

	XPmClient_WakeUp(Proc);

	/* encode set Address into 1st bit of address */
	EncodedAddr = Address | !!SetAddress;

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
void XPmClient_SuspendFinalize(void)
{
	XStatus Status;

	/*
	 * Wait until previous IPI request is handled by the PMU.
	 * If PMU is busy, keep trying until PMU becomes responsive
	 */
	do {
		Status = XIpiPsu_PollForAck(PrimaryProc->Ipi,
					    TARGET_IPI_INT_MASK,
					    PM_IPI_TIMEOUT);
		if (Status != XST_SUCCESS) {
			XPm_Dbg("ERROR timed out while waiting for PMU to"
				" finish processing previous PM-API call\n");
		}
	} while (XST_SUCCESS != Status);

	XPmClient_ClientSuspendFinalize();
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_RequestSuspend(const u32 TargetSubsystemId, const u32 Ack,
				 const u32 Latency, const u32 State)
{
	XStatus Status;
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
 * @param  reason Reason code why the suspend can not be performed or completed
 * - ABORT_REASON_WKUP_EVENT : local wakeup-event received
 * - ABORT_REASON_PU_BUSY : PU is busy
 * - ABORT_REASON_NO_PWRDN : no external powerdown supported
 * - ABORT_REASON_UNKNOWN : unknown error during suspend procedure
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_AbortSuspend(const enum XPmAbortReason Reason)
{
	XStatus Status;
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
	XPmClient_ClientAbortSuspend();

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
XStatus XPmClient_ForcePowerDown(const u32 TargetDevId, const u32 Ack)
{
	XStatus Status;
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
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SystemShutdown(const u32 Type, const u32 SubType)
{
	XStatus Status;
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
 * @param  TargetSubsystemId	Subsystem ID of the target
 * @param  DeviceID		Device ID used as wakeup source
 * @param  Enable		1 - Enable, 0 - Disable
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClient_SetWakeupSource(const u32 TargetSubsystemId, const u32 DeviceID,
				  const u32 Enable)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD3(Payload, PM_SET_WAKEUP_SOURCE, TargetSubsystemId, DeviceID, Enable);

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
