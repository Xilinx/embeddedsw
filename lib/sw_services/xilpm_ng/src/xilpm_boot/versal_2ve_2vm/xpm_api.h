/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_API_H_
#define XPM_API_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"
#include "xplmi_ipi.h"
#include "xpm_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Typedef for the XPlmi command handler function pointer
 *
 * This typedef defines the function signature for the XPlmi command handler.
 * The command handler function takes a pointer to an XPlmi_Cmd structure as
 * input and returns an integer value.
 *
 * @param Cmd Pointer to the XPlmi_Cmd structure containing the command data
 * @return Integer value representing the status of the command handling
 */
typedef int (*XPlmi_CmdHandler)(XPlmi_Cmd *Cmd);

#ifdef XPLMI_IPI_DEVICE_ID
/* Macros for IPI responses (return values and callbacks) */
#define IPI_RESPONSE1(Mask, Arg0)						\
{										\
	u32 Response[XPLMI_CMD_RESP_SIZE] = {Arg0};				\
	if (XST_SUCCESS != XPlmi_IpiWrite(Mask, Response, XPLMI_CMD_RESP_SIZE,	\
					  XIPIPSU_BUF_TYPE_RESP)) {		\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
}

#define IPI_MESSAGE4(Mask, Arg0, Arg1, Arg2, Arg3)				\
{										\
	u32 Response[XPLMI_CMD_RESP_SIZE] = {Arg0, Arg1, Arg2, Arg3};		\
	if (XST_SUCCESS != XPlmi_IpiWrite(Mask, Response, XPLMI_CMD_RESP_SIZE,	\
					  XIPIPSU_BUF_TYPE_MSG)) {		\
		PmWarn("Error in IPI write response\r\n");			\
	}									\
}
#endif /* XPLMI_IPI_DEVICE_ID */

/* Force power down timeout in us */
#define XPM_PWR_DWN_TIMEOUT	(60000U)
/* Max number of processor base addresses */
#define MAX_BASEADDR_LEN	(3U)

XStatus XPm_Init(void (*const RequestCb)(u32 SubsystemId, XPmApiCbId_t EventId, u32 *Payload),
		 int (*const RestartCb)(u32 ImageId, u32 *FuncId));
XStatus XPm_GetDeviceBaseAddr(u32 DeviceId, u32 *BaseAddr);
XStatus XPm_PmcRequestDevice(u32 DeviceId);
XStatus XPm_PmcActivateSubsystem(u32 SubsystemId);
XStatus XPm_PmcWakeAllCores(void);
XStatus XPm_PmcWakeUpCore(u32 CoreId , u32 SetAddress, u64 Address);
XStatus XPm_PmcGetDeviceState(u32 DeviceId, u32 *const DeviceState);
XStatus XPm_SystemShutdown(u32 SubsystemId, u32 Type, u32 SubType, u32 CmdType);
XStatus XPm_PmcReleaseDevice(u32 DeviceId);
XStatus XPm_AddSubsystem(XPlmi_Cmd* Cmd);
XStatus XPm_AddRequirement(XPlmi_Cmd* Cmd);
XStatus XPm_InitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs);
XStatus XPm_HookAfterPlmCdo(void);
XStatus XPm_HnicxNpiDataXfer(u32 Address, u32 Value);
u32 XPm_GetSubsystemId(u32 ImageId);
XPlmi_ModuleCmd* XPm_GetPmCmds(void);
XStatus XPm_AddNode(XPlmi_Cmd *Cmd);
XStatus XPm_HookAfterBootPdi(void);
XStatus XPm_RuntimeInit(void);
u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask);
XStatus XPm_AddDDRMemRegnForDefaultSubsystem(const XPm_MemCtrlrDevice *MCDev);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_API_H_ */
