/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_types.h"
#include "xpm_nodeid.h"
#include "xstatus.h"
#include "xplmi.h"
#include "xplmi_cmd.h"
#include "xplmi_dma.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_requirement.h"
#include "xpm_device.h"
#include "xpm_feature_check.h"
#include "xpm_periph.h"
#include "xpm_gic_proxy.h"
#include "xpm_core.h"
#include "xplmi_scheduler.h"
#include "xpm_regs.h"
#include "xpm_pll.h"
#include "xplmi_sysmon.h"
#include "xpm_pldevice.h"
#include "xpm_ioctl.h"
#include "xpm_notifier.h"
#include "xpm_runtime_device.h"
#include "xpm_runtime_reset.h"
#include "xpm_runtime_clock.h"
#include "xpm_runtime_api.h"
#include "xpm_rail.h"
#include "xpm_runtime_pin.h"
#include "xpm_runtime_pll.h"
#include "xpm_runtime_core.h"
#include "xpm_runtime_power.h"
#include "xpm_runtime_periph.h"
#include "xpm_access.h"
#include "xpm_power_handlers.h"
#include "xpm_debug.h"
#include "xpm_memtbl.h"


#define XPm_RegisterWakeUpHandler(GicId, SrcId, NodeId)	\
	{ \
		Status = XPlmi_GicRegisterHandler(GicId, SrcId, \
				&XPm_DispatchWakeHandler, (void *)(NodeId)); \
		if (Status != XST_SUCCESS) {\
			goto END;\
		}\
	}

/* XilPM Runtime Banner String */
#ifndef XILPM_RUNTIME_BANNER
	#define XILPM_RUNTIME_BANNER_STRING ""
#endif

#if (XILPM_RUNTIME_BANNER == 1)
	#define XILPM_RUNTIME_BANNER_STRING "XilPm_Runtime: EEMI"
#elif (XILPM_RUNTIME_BANNER == 2)
	#define XILPM_RUNTIME_BANNER_STRING "XilPm_Runtime: SUBSYS"
#else
	#define XILPM_RUNTIME_BANNER_STRING "XilPm_Runtime: UNKNOWN"
#endif

static XStatus XPm_DoAddSubsystem(XPlmi_Cmd* Cmd);
static XStatus XPm_DoAddRequirement(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetRequirement(XPlmi_Cmd* Cmd);
static XStatus XPm_DoRequestDevice(XPlmi_Cmd* Cmd);
static XStatus XPm_DoReleaseDevice(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetApiVersion(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetNodeStatus(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetWakeUpSource(XPlmi_Cmd* Cmd);
static XStatus XPm_GetDeviceStatus(const u32 SubsystemId,
	const u32 DeviceId,
	XPm_DeviceStatus *const DeviceStatus);
static XStatus XPm_DoFeatureCheck(XPlmi_Cmd* Cmd);
static XStatus XPm_AbortSuspend(const u32 SubsystemId, const u32 Reason, const u32 DeviceId);
static XStatus XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId, const u32 Latency);
static XStatus XPm_GetChipID(u32* IDCode, u32 *Version);
static XStatus XPm_Query(const u32 Qid, const u32 Arg1, const u32 Arg2, const u32 Arg3, u32 *const Output);

static XStatus XPm_DoRegisterNotifier(XPlmi_Cmd* Cmd);
/****************************************************************************/
/**
 * @brief  Command handler that unpacks the self-suspend payload and calls
 *         XPm_SelfSuspend.
 *
 * @param Cmd		Pointer to the received command containing the payload
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_DoSelfSuspend(XPlmi_Cmd* Cmd);
static XStatus XPm_DoForcePowerdown(XPlmi_Cmd* Cmd);

static XStatus XPm_RegisterNotifier(const u32 SubsystemId, const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 Enable,
			 const u32 IpiMask);
static XStatus XPm_SetWakeUpSource(const u32 SubsystemId, const u32 TargetNodeId,
			    const u32 SourceNodeId, const u32 Enable);
static void XPm_CoreIdle(XPm_Core *Core);
static XStatus XPm_DoAbortSuspend(XPlmi_Cmd* Cmd);

static XStatus XPm_DoWakeUp(XPlmi_Cmd* Cmd);
static XStatus XPmSubsystem_ShutdownSystemCleanup(u32 SubsystemId);
static XStatus XPm_SubsystemPwrUp(u32 SubsystemId);
static XStatus SetSubsystemState_ByCore(const XPm_Core* Core, const u32 State);
static XStatus XPm_DoSystemShutdown(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetMaxLatency(XPlmi_Cmd* Cmd);
static XStatus XPm_DoResetAssert(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetResetState(XPlmi_Cmd* Cmd);
static XStatus XPm_DoInitFinalize(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetChipID(XPlmi_Cmd* Cmd);
static XStatus XPm_DoActivateSubsystem(XPlmi_Cmd* Cmd);
static XStatus XPm_DoPinCtrlRequest(XPlmi_Cmd* Cmd);
static XStatus XPm_DoPinCtrlRelease(XPlmi_Cmd* Cmd);


static XStatus XPm_ActivateSubsystem(u32 SubsystemId, u32 TargetSubsystemId);

static XStatus XPm_DoGetPinFunction(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetPinFunction(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetPinParameter(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetPinParameter(XPlmi_Cmd* Cmd);
static XStatus XPm_DoDevIoctl(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetPllMode(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetPllMode(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetPllParameter(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetPllParameter(XPlmi_Cmd* Cmd);
static XStatus XPm_DoQueryData(XPlmi_Cmd* Cmd);
static XStatus XPm_DoClockEnable(XPlmi_Cmd* Cmd);
static XStatus XPm_DoClockDisable(XPlmi_Cmd *Cmd);
static XStatus XPm_DoGetClockState(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetClockDivider(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetClockDivider(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetClockParent(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetClockParent(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSetNodeAccess(XPlmi_Cmd * Cmd);
#ifdef XPLMI_EM_LINK_NODES_SUPPORT
static XStatus XPm_DoGetEventStatusRegs(XPlmi_Cmd * Cmd);
#endif /* XPLMI_EM_LINK_NODES_SUPPORT */
static XStatus XPm_DoGetClockRate(XPlmi_Cmd * Cmd);
#ifdef XILPM_NG_ENABLE_CLK_SCMI
static XStatus XPm_DoProgramClockRate(XPlmi_Cmd* Cmd);
static XStatus XPm_DoDescribeClockRate(XPlmi_Cmd* Cmd);
#endif /* XILPM_NG_ENABLE_CLK_SCMI */

/* Defined in xilpm-boot */

/****************************************************************************/
/**
 * @brief Resolve an indexed clock, reset, or device node ID.
 *
 * Non-indexed and unsupported node classes are returned unchanged. Indexed
 * nodes are resolved through their class-specific topology table.
 *
 * @param NodeId     Node ID to resolve.
 * @param ResolvedId Pointer to receive the resolved or unchanged node ID.
 *
 * @return XST_SUCCESS when the ID is resolved or does not require resolution;
 *         XPM_PM_INVALID_NODE for a NULL output or an indexed topology hole;
 *         XST_INVALID_PARAM when an indexed node exceeds its class limit.
 *
 ****************************************************************************/
static XStatus XPm_ResolveIndexedNodeId(const u32 NodeId, u32 *const ResolvedId)
{
	XStatus Status = XPM_PM_INVALID_NODE;
	u32 NodeIdx;
	const XPm_ClockNode *Clk = NULL;
	const XPm_ResetNode *Rst = NULL;
	const XPm_Device *Device = NULL;

	/* Reject requests that cannot receive the resolved identifier. */
	if (NULL == ResolvedId) {
		goto done;
	}

	/* Preserve non-indexed and unsupported identifiers by default. */
	*ResolvedId = NodeId;
	NodeIdx = NODEINDEX(NodeId);

	/* Select the topology table associated with the node class. */
	switch (NODECLASS(NodeId)) {
	case (u32)XPM_NODECLASS_CLOCK:
		/* Resolve only indexed clocks after validating the table index. */
		if ((u32)XPM_NODESUBCL_CLOCK_INDEXED != NODESUBCLASS(NodeId)) {
			break;
		}
		if (NodeIdx >= (u32)XPM_NODEIDX_CLK_MAX) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		Clk = XPmClock_GetByIdx(NodeIdx);
		if (Clk == NULL) {
			goto done;
		}
		*ResolvedId = Clk->Node.Id;
		break;

	case (u32)XPM_NODECLASS_RESET:
		/* Resolve only indexed resets after validating the table index. */
		if ((u32)XPM_NODESUBCL_RESET_INDEXED != NODESUBCLASS(NodeId)) {
			break;
		}
		if (NodeIdx >= (u32)XPM_NODEIDX_RST_MAX) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		Rst = XPmReset_GetByIdx(NodeIdx);
		if (Rst == NULL) {
			goto done;
		}
		*ResolvedId = Rst->Node.Id;
		break;

	case (u32)XPM_NODECLASS_DEVICE:
		/* Resolve only indexed devices after validating the table index. */
		if ((u32)XPM_NODESUBCL_DEV_INDEXED != NODESUBCLASS(NodeId)) {
			break;
		}
		if (NodeIdx >= (u32)XPM_NODEIDX_DEV_MAX) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		Device = XPmDevice_GetByIndex(NodeIdx);
		if (Device == NULL) {
			goto done;
		}
		*ResolvedId = Device->Node.Id;
		break;

	default:
		/* No action required for unsupported node classes */
		break;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Format "prefix_<index>" string into buffer without libc stdio.
 *
 * @param Buf     Destination buffer
 * @param BufSize Size of destination buffer
 * @param Prefix  Null-terminated prefix string (e.g., "power_", "reset_")
 * @param Index   Numeric index to append
 */
static void XPm_FormatNodeName(char *Buf, u32 BufSize, const char *Prefix, u32 Index)
{
	u32 i = 0U;
	u32 Val = Index;
	u32 Tmp = Val;
	u32 NumDigits = 1U;
	u32 Pos;

	/* Copy prefix */
	while ((Prefix[i] != '\0') && (i < (BufSize - 1U))) {
		Buf[i] = Prefix[i];
		i++;
	}

	/* Count decimal digits needed */
	while (Tmp >= 10U) {
		Tmp /= 10U;
		NumDigits++;
	}

	/* Clamp to remaining buffer space */
	if ((i + NumDigits) >= BufSize) {
		NumDigits = BufSize - 1U - i;
	}

	/* Write digits right-to-left directly into buffer */
	Pos = i + NumDigits;
	Buf[Pos] = '\0';
	while (Pos > i) {
		u32 Ascii = (u32)'0' + (Val % 10U);
		Pos--;
		Buf[Pos] = (char)Ascii;
		Val /= 10U;
	}
}

/****************************************************************************/
/**
 * @brief Return the name of a clock, device, or reset node.
 *
 * Clock names come from topology after indexed-ID resolution. Device and reset
 * names are synthesized as "power_<index>" and "reset_<index>" respectively.
 *
 * @param NodeId Node ID whose name is requested.
 * @param Resp   Response buffer of at least MAX_NAME_BYTES bytes.
 *
 * @return XST_SUCCESS for a supported node class, XST_INVALID_PARAM for an
 *         unsupported class, or an error from a response-buffer operation.
 *
 * @note A failed clock ID resolution or lookup returns XST_SUCCESS with an
 *       empty name.
 *
 ****************************************************************************/
static XStatus XPm_GetNodeName(u32 NodeId, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 NodeClass;
	u32 ClockId;
	const XPm_ClockNode *Clk = NULL;

	/* Clear the full response so failed lookups return an empty name. */
	Status = Xil_SMemSet(Resp, MAX_NAME_BYTES, 0U, MAX_NAME_BYTES);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	NodeClass = NODECLASS(NodeId);

	u8 BadClass = 0U;
	/* Obtain or construct the name according to the node class. */
	switch (NodeClass) {
	case (u32)XPM_NODECLASS_CLOCK:
		/* Resolve indexed clocks before consulting the clock topology. */
		Status = XPm_ResolveIndexedNodeId(NodeId, &ClockId);
		if (XST_SUCCESS != Status) {
			break;
		}

		Clk = XPmClock_GetById(ClockId);
		if (NULL == Clk) {
			goto done;
		}

		/* Copy the topology-provided clock name into the response. */
		Status = Xil_SMemCpy((char *)Resp, MAX_NAME_BYTES, &Clk->Name[0], MAX_NAME_BYTES, MAX_NAME_BYTES);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;

	case (u32)XPM_NODECLASS_DEVICE:
		/* Device names use their node index with the power prefix. */
		XPm_FormatNodeName((char *)Resp, MAX_NAME_BYTES, "power_", NODEINDEX(NodeId));
		break;

	case (u32)XPM_NODECLASS_RESET:
		/* Reset names use their node index with the reset prefix. */
		XPm_FormatNodeName((char *)Resp, MAX_NAME_BYTES, "reset_", NODEINDEX(NodeId));
		break;

	default:
		Status = XST_INVALID_PARAM;
		BadClass = 1U;
		break;
	}

	if (1U == BadClass) {
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPm_RegisterWakeUpHandlers(void);
static void XPm_ProcessAckReq(const u32 Ack, const u32 IpiMask, const XStatus Status,
			      const u32 NodeId, const u32 NodeState);
static XStatus XPm_RestartCbWrapper(const u32 SubsystemId);

static XStatus XPm_SetClockState(const u32 SubsystemId, const u32 ClockId, const u32 Enable);
static XStatus XPm_SetClockDivider(const u32 SubsystemId, const u32 ClockId, const u32 Divider);
static XStatus XPm_GetClockDivider(const u32 ClockId, u32 *const Divider);
static XStatus XPm_SetClockParent(const u32 SubsystemId, const u32 ClockId, const u32 ParentIdx);
static XStatus XPm_GetClockParent(const u32 ClockId, u32 *const ParentIdx);
static XStatus XPm_PinCtrlRequest(const u32 SubsystemId, const u32 PinId);
static XStatus XPm_PinCtrlRelease(const u32 SubsystemId, const u32 PinId);
static XStatus XPm_SetPinFunction(const u32 SubsystemId, const u32 PinId, const u32 FunctionId);
static XStatus XPm_GetPinFunction(const u32 PinId, u32 *const FunctionId);
static XStatus XPm_SetPinParameter(const u32 SubsystemId, const u32 PinId, const u32 ParamId, const u32 ParamVal);
static XStatus XPm_GetPinParameter(const u32 PinId, const u32 ParamId, u32 *const ParamVal);

static XStatus XPm_ForcePowerdown(u32 SubsystemId, const u32 NodeId, const u32 Ack,
				    const u32 CmdType, const u32 IpiMask);
static XStatus XPm_SetResetState(const u32 SubsystemId, const u32 ResetId,
				    const u32 Action, const u32 CmdType);

/****************************************************************************/
/**
 * @brief  This function returns node count based on class
 *
 * @param Class		Node class (Clock, Reset, or Device)
 * @param Output	Pointer to store the count
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 *
 ****************************************************************************/
static XStatus XPm_GetNodeCount(const u32 Class, u32 *Output)
{
	XStatus Status = XST_FAILURE;

	/* Select the count source associated with the requested node class. */
	switch (Class) {
	case (u32)XPM_NODECLASS_CLOCK:
		Status = XPmClock_GetNumClocks(Output);
		break;
	case (u32)XPM_NODECLASS_RESET:
		/* Indexed reset and device classes use their fixed topology limits. */
		*Output = (u32)XPM_NODEIDX_RST_MAX - 1U;
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_DEVICE:
		*Output = (u32)XPM_NODEIDX_DEV_MAX - 1U;
		Status = XST_SUCCESS;
		break;
	default:
		/* Reject classes for which this interface has no count source. */
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief Initialize the XilPM runtime command interface.
 *
 * Stores the multiboot value, installs runtime handlers in the PLM command
 * table, initializes the subsystem module, and registers wake-up interrupts.
 * It should be called before any other XPM runtime functions are used.
 *
 * @return XST_SUCCESS when subsystem and wake-up handler initialization
 *         succeeds, otherwise the error returned by either initialization step.
 *
 ****************************************************************************/
XStatus XPm_RuntimeInit(void)
{
	XStatus Status = XST_FAILURE;
	const char *Banner = XILPM_RUNTIME_BANNER_STRING;
	PmInfo("Initializing XilPm Runtime Library [%s]\r\n", Banner);

	/* Store multiboot register value*/
	XPlmi_StoreMultiboot();

	/* Initializing XPLmi_PmCmds array*/
	XPlmi_ModuleCmd* XPlmi_PmCmds =  XPm_GetPmCmds();

	/* Install handlers for interface discovery and status queries. */
	XPlmi_PmCmds[PM_GET_API_VERSION].Handler = (XPlmi_CmdHandler)&XPm_DoGetApiVersion;
	XPlmi_PmCmds[PM_GET_NODE_STATUS].Handler = (XPlmi_CmdHandler)&XPm_DoGetNodeStatus;

	/* Install handlers for notification and power-state transitions. */
	XPlmi_PmCmds[PM_REGISTER_NOTIFIER].Handler = (XPlmi_CmdHandler)&XPm_DoRegisterNotifier;
	XPlmi_PmCmds[PM_SELF_SUSPEND].Handler = (XPlmi_CmdHandler)&XPm_DoSelfSuspend;
	XPlmi_PmCmds[PM_FORCE_POWERDOWN].Handler = (XPlmi_CmdHandler)&XPm_DoForcePowerdown;
	XPlmi_PmCmds[PM_ABORT_SUSPEND].Handler = (XPlmi_CmdHandler)&XPm_DoAbortSuspend;
	XPlmi_PmCmds[PM_REQUEST_WAKEUP].Handler = (XPlmi_CmdHandler)&XPm_DoWakeUp;
	XPlmi_PmCmds[PM_SET_WAKEUP_SOURCE].Handler = (XPlmi_CmdHandler)&XPm_DoSetWakeUpSource;
	XPlmi_PmCmds[PM_SYSTEM_SHUTDOWN].Handler = (XPlmi_CmdHandler)&XPm_DoSystemShutdown;

	/* Install handlers for device ownership and requirements. */
	XPlmi_PmCmds[PM_REQUEST_NODE].Handler = (XPlmi_CmdHandler)&XPm_DoRequestDevice;
	XPlmi_PmCmds[PM_RELEASE_NODE].Handler = (XPlmi_CmdHandler)&XPm_DoReleaseDevice;
	XPlmi_PmCmds[PM_SET_REQUIREMENT].Handler = (XPlmi_CmdHandler)&XPm_DoSetRequirement;
	XPlmi_PmCmds[PM_SET_MAX_LATENCY].Handler = (XPlmi_CmdHandler)&XPm_DoSetMaxLatency;
	XPlmi_PmCmds[PM_RESET_ASSERT].Handler = (XPlmi_CmdHandler)&XPm_DoResetAssert;
	XPlmi_PmCmds[PM_RESET_GET_STATUS].Handler = (XPlmi_CmdHandler)&XPm_DoGetResetState;
	XPlmi_PmCmds[PM_INIT_FINALIZE].Handler = (XPlmi_CmdHandler)&XPm_DoInitFinalize;
	XPlmi_PmCmds[PM_GET_CHIPID].Handler = (XPlmi_CmdHandler)&XPm_DoGetChipID;

	/* Install handlers for pin-control operations. */
	XPlmi_PmCmds[PM_PINCTRL_REQUEST].Handler = (XPlmi_CmdHandler)&XPm_DoPinCtrlRequest;
	XPlmi_PmCmds[PM_PINCTRL_RELEASE].Handler = (XPlmi_CmdHandler)&XPm_DoPinCtrlRelease;
	XPlmi_PmCmds[PM_PINCTRL_GET_FUNCTION].Handler = (XPlmi_CmdHandler)&XPm_DoGetPinFunction;
	XPlmi_PmCmds[PM_PINCTRL_SET_FUNCTION].Handler = (XPlmi_CmdHandler)&XPm_DoSetPinFunction;
	XPlmi_PmCmds[PM_PINCTRL_CONFIG_PARAM_GET].Handler = (XPlmi_CmdHandler)&XPm_DoGetPinParameter;
	XPlmi_PmCmds[PM_PINCTRL_CONFIG_PARAM_SET].Handler = (XPlmi_CmdHandler)&XPm_DoSetPinParameter;

	/* Install handlers for PLL configuration. */
	XPlmi_PmCmds[PM_PLL_SET_MODE].Handler = (XPlmi_CmdHandler)&XPm_DoSetPllMode;
	XPlmi_PmCmds[PM_PLL_GET_MODE].Handler = (XPlmi_CmdHandler)&XPm_DoGetPllMode;
	XPlmi_PmCmds[PM_PLL_SET_PARAMETER].Handler = (XPlmi_CmdHandler)&XPm_DoSetPllParameter;
	XPlmi_PmCmds[PM_PLL_GET_PARAMETER].Handler = (XPlmi_CmdHandler)&XPm_DoGetPllParameter;
	XPlmi_PmCmds[PM_IOCTL].Handler = (XPlmi_CmdHandler)&XPm_DoDevIoctl;
	XPlmi_PmCmds[PM_QUERY_DATA].Handler = (XPlmi_CmdHandler)&XPm_DoQueryData;

	/* Install handlers for clock control and inspection. */
	XPlmi_PmCmds[PM_CLOCK_ENABLE].Handler = (XPlmi_CmdHandler)&XPm_DoClockEnable;
	XPlmi_PmCmds[PM_CLOCK_DISABLE].Handler = (XPlmi_CmdHandler)&XPm_DoClockDisable;
	XPlmi_PmCmds[PM_CLOCK_SETDIVIDER].Handler = (XPlmi_CmdHandler)&XPm_DoSetClockDivider;
	XPlmi_PmCmds[PM_CLOCK_GETDIVIDER].Handler = (XPlmi_CmdHandler)&XPm_DoGetClockDivider;
	XPlmi_PmCmds[PM_CLOCK_SETPARENT].Handler = (XPlmi_CmdHandler)&XPm_DoSetClockParent;
	XPlmi_PmCmds[PM_CLOCK_GETPARENT].Handler = (XPlmi_CmdHandler)&XPm_DoGetClockParent;
	XPlmi_PmCmds[PM_CLOCK_GETSTATE].Handler = (XPlmi_CmdHandler)&XPm_DoGetClockState;

	/* Install handlers for dynamic subsystem configuration. */
	XPlmi_PmCmds[PM_ADD_SUBSYSTEM].Handler = (XPlmi_CmdHandler)&XPm_DoAddSubsystem;
	XPlmi_PmCmds[PM_ADD_REQUIREMENT].Handler = (XPlmi_CmdHandler)&XPm_DoAddRequirement;
	XPlmi_PmCmds[PM_ACTIVATE_SUBSYSTEM].Handler = (XPlmi_CmdHandler)&XPm_DoActivateSubsystem;
	XPlmi_PmCmds[PM_FEATURE_CHECK].Handler = (XPlmi_CmdHandler)&XPm_DoFeatureCheck;
	XPlmi_PmCmds[PM_SET_NODE_ACCESS].Handler = (XPlmi_CmdHandler)&XPm_DoSetNodeAccess;
#ifdef XPLMI_EM_LINK_NODES_SUPPORT
	XPlmi_PmCmds[PM_GET_EVENT_STATUS_REGS].Handler =
		(XPlmi_CmdHandler)&XPm_DoGetEventStatusRegs;
#endif /* XPLMI_EM_LINK_NODES_SUPPORT */
	XPlmi_PmCmds[PM_CLOCK_GETRATE].Handler = (XPlmi_CmdHandler)&XPm_DoGetClockRate;
#ifdef XILPM_NG_ENABLE_CLK_SCMI
	XPlmi_PmCmds[PM_CLOCK_PROGRAM_RATE].Handler = (XPlmi_CmdHandler)&XPm_DoProgramClockRate;
	XPlmi_PmCmds[PM_CLOCK_DESCRIBERATE].Handler = (XPlmi_CmdHandler)&XPm_DoDescribeClockRate;
#endif /* XILPM_NG_ENABLE_CLK_SCMI */

	/* Initialize subsystem state before accepting wake-up events. */
	Status = XPmSubsystem_ModuleInit();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Connect platform wake sources after subsystem initialization. */
	Status = XPm_RegisterWakeUpHandlers();

done:
	return Status;
}

/*****************************************************************************/
/**
 * @brief Wake the APU core linked to a GIC proxy wake interrupt
 *
 * Uses XPm_RequestWakeUp() so FPD is restored first if it was powered off
 * during suspend.
 *
 * @param  PeriphIdx	Index of the peripheral that raised the wake interrupt
 *
 * @return XST_SUCCESS if the core is running or was woken,
 *	   XST_FAILURE or error code on failure
 *
 *****************************************************************************/
static XStatus XPm_GicProxyWakeUp(const u32 PeriphIdx)
{
	XStatus Status = XST_FAILURE;
	u32 WakeProcId = 0U;
	u32 SubsystemId;

	XPm_Periph *Periph = (XPm_Periph *)XPmDevice_GetByIndex(PeriphIdx);
	if (NULL == Periph) {
		goto done;
	}

	Status = XPmRuntime_Periph_GetWakeProcId(Periph, &WakeProcId);
	if ((XST_SUCCESS != Status) || (0U == WakeProcId)) {
		Status = XST_FAILURE;
		goto done;
	}

	XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(WakeProcId);
	if (NULL == Core) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Do not process anything if core is already running */
	if ((u8)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Extract subsystem id from wake-up core */
	SubsystemId = XPmDevice_GetSubsystemIdOfCore((XPm_Device *)Core);
	if (INVALID_SUBSYSID == SubsystemId) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Request wakeup for the core */
	Status = XPm_RequestWakeUp(SubsystemId, WakeProcId, 0, 0, 0, XPLMI_CMD_SECURE);

done:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This is the handler for wake up interrupts
 *
 * @param  DeviceIdx	Index of peripheral device
 *
 * @return Status	XST_SUCCESS if processor wake successfully
 *			XST_FAILURE or error code in case of failure
 *
 *****************************************************************************/
static int XPm_DispatchWakeHandler(void *DeviceIdx)
{
	XStatus Status;

	Status = XPm_GicProxyWakeUp((u32)DeviceIdx);
	return Status;
}

/****************************************************************************/
/**
 * @brief Register wakeup handlers with XilPlmi
 * @return XST_SUCCESS on success and error code on failure
 ****************************************************************************/
static XStatus XPm_RegisterWakeUpHandlers(void)
{
	XStatus Status = XST_FAILURE;

	/**
	 * Register the events for PM
	 */
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC29, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC30, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC31, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC0, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC1, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC2, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC3, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC4, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC5, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC6, XPM_NODEIDX_DEV_USB_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP5, (u32)XPLMI_GICP1_SRC4, XPM_NODEIDX_DEV_MMI_GEM);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP6, (u32)XPLMI_GICP6_SRC8, XPM_NODEIDX_DEV_RTC);

END:
	return Status;
}

XStatus XPmPower_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	XStatus Status = XST_SUCCESS;
	const XPm_Power *Power = XPmPower_GetById(DeviceId);
	const XPm_Power *Parent;

	*Latency = 0;
	if (NULL == Power) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u8)XPM_POWER_STATE_ON == Power->Node.State) {
		goto done;
	}

	*Latency += Power->PwrUpLatency;

	Parent = Power->Parent;

	/* Account latencies of parents if a parent is down */
	while (NULL != Parent) {
		if ((u8)XPM_POWER_STATE_ON == Parent->Node.State) {
			break;
		}

		*Latency += Parent->PwrUpLatency;
		Parent = Parent->Parent;
	}

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a notifier registration command.
 */
/****************************************************************************/
static XStatus XPm_DoRegisterNotifier(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 NodeId = Cmd->Payload[0];
	u32 Event = Cmd->Payload[1];
	u32 Wake = Cmd->Payload[2];
	u32 Enable = Cmd->Payload[3];
	u32 IpiMask = Cmd->IpiMask;
	Status = XPm_RegisterNotifier(SubsystemId, NodeId, Event, Wake, Enable, IpiMask);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a feature-check command and return version data in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoFeatureCheck(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 ApiId = Cmd->Payload[0];
	u32 *Version = &Cmd->Response[1];
	Status = XPm_FeatureCheck(ApiId, Version);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief Return PM_VERSION in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoGetApiVersion(XPlmi_Cmd* Cmd)
{
	/* Return the interface version in the command response. */
	PmInfo("XPm_DoGetApiVersion\n\r");
	Cmd->Response[1] = PM_VERSION;
	Cmd->Response[0] = XST_SUCCESS;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief Handle a node-status query.
 *
 * Resolves an indexed node ID and returns its device status. A missing indexed
 * node is reported in the first response word while the handler itself returns
 * success; access denial is handled in the same way.
 *
 * @param Cmd Pointer to the command containing the node ID and requesting
 *            subsystem, and the response buffer.
 *
 * @return XST_SUCCESS when the query completes or its result is reported in
 *         the response; otherwise an indexed-ID resolution or query error.
 *
 ****************************************************************************/
static XStatus XPm_DoGetNodeStatus(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 IndexedNodeId = Cmd->Payload[0];
	u32 NodeId;

	Status = XPm_ResolveIndexedNodeId(IndexedNodeId, &NodeId);
	if (XST_SUCCESS != Status) {
		Cmd->Response[0] = (u32)Status;
		/*
		 * For getter APIs, a hole in the SCMI (INDEXED) ID space is
		 * not a failure — the API returns SUCCESS with the actual
		 * status in the response payload so the caller (e.g. TF-A
		 * SCMI server) can act accordingly.
		 */
		if (XPM_PM_INVALID_NODE == Status) {
			Status = XST_SUCCESS;
		}
		goto done;
	}

	/* TODO: FIXME fix casting below */
	Status = XPm_GetDeviceStatus(SubsystemId, NodeId, (XPm_DeviceStatus*)(&Cmd->Response[1]));
	if (XPM_PM_NO_ACCESS == Status) {
		Cmd->Response[0] = XPM_PM_NO_ACCESS;
		Status = XST_SUCCESS;
	} else {
		Cmd->Response[0] = (u32)Status;
	}

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a wake-up source configuration command.
 */
/****************************************************************************/
static XStatus XPm_DoSetWakeUpSource(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 TargetNodeId = Cmd->Payload[0];
	u32 SourceNodeId = Cmd->Payload[1];
	u32 Enable = Cmd->Payload[2];
	Status = XPm_SetWakeUpSource(SubsystemId, TargetNodeId, SourceNodeId, Enable);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

static XStatus XPm_SetWakeUpSource(const u32 SubsystemId, const u32 TargetNodeId,
			    const u32 SourceNodeId, const u32 Enable)
{
	// //XPM_EXPORT_CMD(PM_SET_WAKEUP_SOURCE, XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);
	XStatus Status = XST_FAILURE;
	XPm_Periph *Periph = NULL;
	const XPm_Subsystem *Subsystem;

	/* Check if given target node is valid and present in device list */
	if ((NODECLASS(TargetNodeId) != (u32)XPM_NODECLASS_DEVICE) ||
	    (NODESUBCLASS(TargetNodeId) != (u32)XPM_NODESUBCL_DEV_CORE) ||
	    (NULL == XPmDevice_GetById(TargetNodeId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* The call applies only to peripheral nodes */
	if ((NODECLASS(SourceNodeId) != (u32)XPM_NODECLASS_DEVICE) ||
	    (NODESUBCLASS(SourceNodeId) != (u32)XPM_NODESUBCL_DEV_PERIPH)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Is subsystem allowed to use resource (slave)? */
	Status = XPm_IsAccessAllowed(SubsystemId, SourceNodeId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}
	const XPmRuntime_DeviceOps *DevOps = XPm_GetDevOps_ById(SourceNodeId);
	if (NULL == DevOps) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Periph = (XPm_Periph *)XPmDevice_GetById(SourceNodeId);
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if((NULL == Periph) || (NULL == Subsystem)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check whether the device has wake-up capability */
	Status = XPm_CheckCapabilities(&Periph->Device, (u32)PM_CAP_WAKEUP);
	if (XST_SUCCESS != Status) {
		Status = XST_NO_FEATURE;
		goto done;
	}
	Status = XPmRuntime_Periph_SetWakeProcId(Periph, TargetNodeId);
	if (XST_SUCCESS != Status) {
		Status = XST_FAILURE;
		goto done;
	}
	Status = XPmGicProxy_WakeEventSet(Periph, (u8)Enable);
	if (XST_SUCCESS != Status) {
		goto done;
	}


done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}


/****************************************************************************/
/**
 * @brief Handle a device-requirement update command.
 *
 * Resolves the indexed device ID, verifies subsystem access, and updates the
 * requested capabilities and quality of service.
 *
 * @param Cmd Pointer to the command containing device ID, capabilities, QoS,
 *            requesting subsystem, and response buffer.
 *
 * @return XST_SUCCESS on success; XPM_PM_NO_ACCESS when access is denied;
 *         otherwise an indexed-ID or requirement update error.
 *
 ****************************************************************************/
static XStatus XPm_DoSetRequirement(XPlmi_Cmd* Cmd){
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 IndexedDeviceId = Cmd->Payload[0];
	u32 DeviceId;
	u32 Capabilities = Cmd->Payload[1];
	u32 QoS = Cmd->Payload[2];

	/* Resolve the externally supplied index to its topology device ID. */
	Status = XPm_ResolveIndexedNodeId(IndexedDeviceId, &DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Enforce the subsystem's access policy before changing requirements. */
	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Apply the requested capabilities and quality-of-service setting. */
	Status = XPmDevice_SetRequirement(SubsystemId, DeviceId, Capabilities, QoS);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  Request all preallocated devices in a subsystem that match the
 * class, subclass, and type encoded in DeviceId. Used during a kexec-based
 * kernel reload (Linux sends the PM_DEV_ALL_PERIPH node with index
 * XPM_NODEIDX_DEV_ALL) after a bulk release, so devices needed for subsystem
 * bring-up (e.g. USB, eMMC) are re-requested.
 *
 * @param SubsystemId	Subsystem making the request.
 * @param DeviceId	Node whose class/subclass/type selects the devices.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
static XStatus XPm_RequestAllPreallocDevices(const u32 SubsystemId,
					     const u32 DeviceId)
{
	volatile XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem = NULL;
	u32 TargetClass = NODECLASS(DeviceId);
	u32 TargetSubclass = NODESUBCLASS(DeviceId);
	u32 TargetType = NODETYPE(DeviceId);
	u32 ReqmDeviceId;

	/* Ensure the subsystem is valid: in-range and not the PMC subsystem */
	if ((MAX_NUM_SUBSYSTEMS <= NODEINDEX(SubsystemId)) ||
	    (PM_SUBSYS_PMC == SubsystemId)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Retrieve target subsystem */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Request unallocated preallocated devices matching class/subclass/type */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		const XPm_Requirement *Reqm = NULL;

		if (NULL == ReqmNode->Data) {
			PmErr("Invalid requirement node\n\r");
			Status = XST_FAILURE;
			goto done;
		}
		Reqm = ReqmNode->Data;
		if (NULL == Reqm->Device) {
			Status = XST_FAILURE;
			goto done;
		}
		ReqmDeviceId = Reqm->Device->Node.Id;

		/*
		 * Skip devices that are already allocated, not preallocated,
		 * or that do not match class, subclass, and type.
		 */
		if ((1U == Reqm->Allocated) ||
		    (1U != PREALLOC((u32)Reqm->Flags)) ||
		    (TargetClass != NODECLASS(ReqmDeviceId)) ||
		    (TargetSubclass != NODESUBCLASS(ReqmDeviceId)) ||
		    (TargetType != NODETYPE(ReqmDeviceId))) {
			continue;
		}

		/*
		 * Re-request with the original prealloc capabilities. Use a
		 * secure command type so the request matches boot-time
		 * preallocation, independent of the IPI security of the
		 * bulk REQUEST_NODE from Linux.
		 */
		Status = XPmDevice_Request(SubsystemId, ReqmDeviceId,
					   Reqm->PreallocCaps,
					   Reqm->PreallocQoS,
					   XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			PmErr("Requesting prealloc device 0x%x failed.\n\r",
			      ReqmDeviceId);
			Status = XPM_ERR_DEVICE_REQ;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a device request command.
 *
 * Resolves the indexed device ID and requests the device with the supplied
 * capabilities, quality of service, and command security type. A device ID
 * with index XPM_NODEIDX_DEV_ALL bulk-requests matching preallocated devices.
 *
 * @param Cmd Pointer to the command payload, request metadata, and response.
 *
 * @return XST_SUCCESS on success, otherwise an indexed-ID resolution or device
 *         request error. The status is also written to the response.
 *
 ****************************************************************************/
static XStatus XPm_DoRequestDevice(XPlmi_Cmd* Cmd) {
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 IndexedDeviceId = Cmd->Payload[0];
	u32 DeviceId;
	u32 Capabilities = Cmd->Payload[1];
	u32 QoS = Cmd->Payload[2];
	u32 CmdType = Cmd->IpiReqType;

	/* Resolve the external device index before submitting the request. */
	Status = XPm_ResolveIndexedNodeId(IndexedDeviceId, &DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * Bulk-request path: Linux sends the PM_DEV_ALL_PERIPH node (device
	 * index XPM_NODEIDX_DEV_ALL) during kexec to re-request preallocated
	 * peripheral devices after a bulk release.
	 */
	if ((u32)XPM_NODEIDX_DEV_ALL == NODEINDEX(DeviceId)) {
		Status = XPm_RequestAllPreallocDevices(SubsystemId, DeviceId);
		goto done;
	}

	/* Request ownership with the caller's capabilities and QoS. */
	Status = XPmDevice_Request(SubsystemId, DeviceId, Capabilities,
				   QoS, CmdType);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  Release all devices requested by a subsystem that match the class,
 * subclass, and type encoded in DeviceId. Used during a kexec-based kernel
 * reload (Linux sends the PM_DEV_ALL_PERIPH node with index
 * XPM_NODEIDX_DEV_ALL) to bulk-release stale device ownership.
 *
 * @param SubsystemId	Subsystem making the request.
 * @param DeviceId	Node whose class/subclass/type selects the devices.
 * @param CmdType	Command type (secure/non-secure).
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If DeviceId uses the XPM_NODEIDX_DEV_ALL node index, all devices
 * matching the class, subclass, and type will be released. This
 * general-purpose behavior can release critical devices (e.g., processors)
 * by mistake, so use it with care and only when the usage is intended.
 *
 ****************************************************************************/
static XStatus XPm_ReleaseAllDevices(const u32 SubsystemId, const u32 DeviceId,
				     const u32 CmdType)
{
	volatile XStatus Status = XST_FAILURE;
	const XPm_Subsystem* Subsystem = NULL;
	u32 TargetClass = NODECLASS(DeviceId);
	u32 TargetSubclass = NODESUBCLASS(DeviceId);
	u32 TargetType = NODETYPE(DeviceId);
	u32 ReqmDeviceId;
	u32 Usage = 0U;

	/* Ensure the subsystem is valid: in-range and not the PMC subsystem */
	if ((MAX_NUM_SUBSYSTEMS <= NODEINDEX(SubsystemId)) ||
	    (PM_SUBSYS_PMC == SubsystemId)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Retrieve target subsystem */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Release devices matching the specified class, subclass, and type */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		const XPm_Requirement *Reqm = NULL;

		if (NULL == ReqmNode->Data) {
			PmErr("Invalid requirement node\n\r");
			Status = XST_FAILURE;
			goto done;
		}
		Reqm = ReqmNode->Data;
		if (NULL == Reqm->Device) {
			Status = XST_FAILURE;
			goto done;
		}
		ReqmDeviceId = Reqm->Device->Node.Id;

		/*
		 * Only release allocated devices that match class, subclass,
		 * and type
		 */
		if ((1U != Reqm->Allocated) ||
		    (TargetClass != NODECLASS(ReqmDeviceId)) ||
		    (TargetSubclass != NODESUBCLASS(ReqmDeviceId)) ||
		    (TargetType != NODETYPE(ReqmDeviceId))) {
			continue;
		}

		Status = XPmDevice_Release(SubsystemId, ReqmDeviceId, CmdType);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to release device 0x%x: 0x%x\n\r",
			      ReqmDeviceId, Status);
			Status = XPM_ERR_DEVICE_RELEASE;
			goto done;
		}

		Usage = XPmDevice_GetUsageStatus(Subsystem, Reqm->Device);
		if (0U == Usage) {
			XPmNotifier_Event(Reqm->Device->Node.Id,
					  (u32)EVENT_ZERO_USERS);
		}
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a device release command.
 *
 * Resolves the indexed device ID, bulk-releases matching devices for the
 * all-devices ID, or validates access and releases one device. A zero-user
 * notification is emitted after the last user releases a single device.
 *
 * @param Cmd Pointer to the command payload, requesting subsystem and command
 *            type, and response buffer.
 *
 * @return XST_SUCCESS on success, otherwise an ID resolution, access, or device
 *         release error. The status is also written to the response.
 *
 * @note A failed subsystem lookup returns XST_SUCCESS in the response and does
 *       not release the device.
 *
 ****************************************************************************/
static XStatus XPm_DoReleaseDevice(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 IndexedDeviceId = Cmd->Payload[0];
	u32 DeviceId;
	u32 CmdType = Cmd->IpiReqType;
	u32 Usage = 0U;
	const XPm_Subsystem* Subsystem = NULL;
	const XPm_Device* Device = NULL;

	Status = XPm_ResolveIndexedNodeId(IndexedDeviceId, &DeviceId);
	if (XST_SUCCESS != Status) {
		Cmd->Response[0] = (u32)Status;
		goto done;
	}

	/*
	 * Bulk-release path: Linux sends the PM_DEV_ALL_PERIPH node (device
	 * index XPM_NODEIDX_DEV_ALL) during kexec to release all peripheral
	 * devices owned by the subsystem at once.
	 */
	if ((u32)XPM_NODEIDX_DEV_ALL == NODEINDEX(DeviceId)) {
		Status = XPm_ReleaseAllDevices(SubsystemId, DeviceId, CmdType);
		goto done;
	}

	/* Resolve the requesting subsystem for usage accounting. */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	/* Validate the resolved device before checking its access policy. */
	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	/* Require subsystem access before releasing the individual device. */
	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Release ownership, then notify listeners if no users remain. */
	Status = XPmDevice_Release(SubsystemId, DeviceId, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Usage = XPmDevice_GetUsageStatus(Subsystem, Device);
	if (0U == Usage) {
		XPmNotifier_Event(Device->Node.Id, (u32)EVENT_ZERO_USERS);
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief Return the runtime-configuration skip mask for a boot device.
 *
 * @param DeviceId Device node ID to classify.
 *
 * @return The corresponding XPLMI_SKIP_BOOT_DEV_* mask for a supported QSPI,
 *         SD, OSPI, USB, or UFS boot device; zero for any other device.
 *
 ****************************************************************************/
static u32 XPm_GetBootDevSkipMask(u32 DeviceId)
{
	u32 Mask = 0U;

	/* Map each supported boot device to its runtime policy bit. */
	switch (DeviceId) {
	case PM_DEV_QSPI:
		Mask = XPLMI_SKIP_BOOT_DEV_QSPI_MASK;
		break;
	/* Distinguish the independently controlled SD boot interfaces. */
	case PM_DEV_SDIO_0:
		Mask = XPLMI_SKIP_BOOT_DEV_SD0_MASK;
		break;
	case PM_DEV_SDIO_1:
		Mask = XPLMI_SKIP_BOOT_DEV_SD1_MASK;
		break;
	/* Map the remaining external boot interfaces to their policy bits. */
	case PM_DEV_OSPI:
		Mask = XPLMI_SKIP_BOOT_DEV_OSPI_MASK;
		break;
	case PM_DEV_USB_0:
		Mask = XPLMI_SKIP_BOOT_DEV_USB_MASK;
		break;
	case PM_DEV_UFS:
		Mask = XPLMI_SKIP_BOOT_DEV_UFS_MASK;
		break;
	default:
		Mask = 0U;
		break;
	}

	return Mask;
}

/**
 * @brief Check if boot device release should be skipped.
 *
 * @param DeviceId	Device node ID.
 *
 * @return Non-zero if release should be skipped, 0 otherwise.
 */
static u32 XPm_ShouldSkipBootDevRelease(u32 DeviceId)
{
	u32 SkipMask = XPm_GetBootDevSkipMask(DeviceId);
	u32 Result = 0U;

	if (0U != SkipMask) {
		Result = (XPm_In32(XPLMI_RTCFG_SKIP_BOOT_DEV_RELEASE) & SkipMask);
	}

	return Result;
}

/**
 * @brief Request Device from PMC Subsystem (runtime strong override).
 *
 * Sets PmcAllocated so GetMaxCapabilities() accounts for PMC's usage,
 * then triggers FSM evaluation via UpdateStatus().  If the device is
 * currently UNUSED and DevOps exists, this causes ActionBringUpAll to
 * fire, which properly brings up power, enables clocks, and releases
 * resets through the FSM -- ensuring clock UseCount is tracked.
 *
 * If no DevOps exists (no subsystem registered requirements via CDO),
 * falls back to direct XPmDevice_BringUp() (power-only, same as boot
 * stub).
 *
 * When the device is already RUNNING (e.g. another subsystem holds it),
 * UpdateStatus() sees no state change needed and returns SUCCESS.
 *
 * UART_0/UART_1 are routed to the direct BringUp fallback:
 * ActionBringUpAll's Reset(RELEASE) pulses the UART reset and kills
 * the live console.  I2C_PMC is safe because its caller does a full
 * re-init.  Don't remove without adding explicit UART reconfigure at
 * the LPD-init request sites in xpm_api.c.
 *
 * @param DeviceId	Device node ID to request.
 *
 * @return XST_SUCCESS on successful request, error code on failure.
 */
XStatus XPm_PmcRequestDevice(u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_Device *Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Mark PMC ownership */
	Device->PmcAllocated = 1U;

	/*
	 * If DevOps exists, use the FSM path.  UpdateStatus() will
	 * evaluate GetMaxCapabilities() (which now includes
	 * PM_CAP_ACCESS from PmcAllocated) and trigger a
	 * UNUSED -> RUNNING transition via ActionBringUpAll if the
	 * device is not already running.
	 *
	 * ActionBringUpAll handles power + clocks + resets, ensuring
	 * clock UseCount is properly tracked for later release via
	 * ActionShutdown's symmetric SetClocks(0).
	 *
	 * If device is already RUNNING, UpdateStatus() sees same state
	 * and returns SUCCESS with no transition.
	 *
	 * UARTs excluded -- see function header.
	 */
	if ((NULL != XPm_GetDevOps_ById(DeviceId)) &&
	   ((u32)PM_DEV_UART_0 != (DeviceId)) &&
	   ((u32)PM_DEV_UART_1 != (DeviceId))) {
		Status = XPmDevice_UpdateStatus(Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DEVICE_CHANGE_STATE;
		}
	} else {
		/*
		 * No DevOps: no subsystem registered requirements.
		 * Fall back to direct BringUp (power-only).
		 * This is the boot-compatible path where CDO has
		 * already configured clocks and resets.
		 */
		Status = XPmDevice_BringUp(Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DEVICE_PWR_PARENT_UP;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/**
 * @brief Release a device owned by the PMC subsystem (runtime strong override).
 *
 * Clears PmcAllocated and lets the device FSM evaluate whether the
 * device should be powered down.  If other subsystems still hold
 * requirements, GetMaxCapabilities() returns non-zero and the device
 * stays in RUNNING state.
 *
 * If no DevOps exists (no subsystem ever registered a requirement for
 * this device via CDO), falls back to direct power-down since no
 * sharing conflict is possible.
 *
 * If the RTCA skip-boot-device-release policy has the corresponding
 * bit set, the release is skipped and XPM_PMC_BOOT_DEV_RETAINED is
 * returned (PmcAllocated stays set).
 *
 * UART_0/UART_1 excluded (defensive, symmetric with request side):
 * no in-tree caller releases UARTs today, but ActionShutdown would
 * kill the console if one ever does.  Keep in lockstep with request.
 *
 * @param DeviceId	Device node ID to release.
 *
 * @return XST_SUCCESS on successful release,
 *         XPM_PMC_BOOT_DEV_RETAINED if release was skipped per RTCA policy,
 *         error code on failure.
 */
XStatus XPm_PmcReleaseDevice(u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_Device *Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Check if boot device is to be retained by PMC */
	if (0U != XPm_ShouldSkipBootDevRelease(DeviceId)) {
		PmInfo("Boot dev 0x%x release skipped (retained)\r\n", DeviceId);
		Status = XPM_PMC_BOOT_DEV_RETAINED;
		goto done;
	}

	/* Check if device is already in unused state */
	if (Device->Node.State == (u8)XPM_DEVSTATE_UNUSED) {
		/* Clear PMC ownership */
		Device->PmcAllocated = 0U;
		Status = XST_SUCCESS;
		goto done;
	}

	/* Clear PMC ownership */
	Device->PmcAllocated = 0U;

	/*
	 * If DevOps exists, let the device FSM evaluate whether the
	 * device should be powered down.  GetMaxCapabilities() now
	 * returns 0 for the PmcAllocated contribution; if other
	 * subsystems still hold requirements, their capabilities keep
	 * the device in RUNNING state.
	 *
	 * If no DevOps exists, no subsystem ever registered a
	 * requirement for this device, so no sharing conflict is
	 * possible.  Fall back to direct power-down.
	 *
	 * UARTs excluded -- see function header.
	 */
	if ((NULL != XPm_GetDevOps_ById(DeviceId)) &&
	   ((u32)PM_DEV_UART_0 != (DeviceId)) &&
	   ((u32)PM_DEV_UART_1 != (DeviceId))) {
		Status = XPmDevice_UpdateStatus(Device);
		if (XST_SUCCESS != Status) {
			/* Restore PMC ownership on failed release */
			Device->PmcAllocated = 1U;
			DbgErr = XPM_INT_ERR_DEVICE_CHANGE_STATE;
		}
	} else {
		/* Direct power-down fallback (no FSM available) */
		if (NULL != Device->Power) {
			if (Device->Power->UseCount > 0U) {
				Device->WfPwrUseCnt = Device->Power->UseCount - (u16)1U;
			} else {
				Device->WfPwrUseCnt = 0U;
			}
			Status = Device->Power->HandleEvent(&Device->Power->Node,
							(u32)XPM_POWER_EVENT_PWR_DOWN);
			if (XST_SUCCESS == Status) {
				Device->Node.State = (u8)XPM_DEVSTATE_UNUSED;
			} else {
				/* Restore PMC ownership on failed power-down */
				Device->PmcAllocated = 1U;
				DbgErr = XPM_INT_ERR_PWRDN;
			}
		} else {
			Status = XST_SUCCESS;
		}
	}

done:
	/* Suppress error log when release was intentionally skipped per RTCA policy */
	if ((XStatus)XPM_PMC_BOOT_DEV_RETAINED != Status) {
		XPm_PrintDbgErr(Status, DbgErr);
	}
	return Status;
}

/**
 * @brief Adds a subsystem to the power management framework.
 *
 * This function is used to add a subsystem to the power management framework.
 * It takes a pointer to an XPlmi_Cmd structure as a parameter, which contains
 * the necessary information for adding the subsystem.
 *
 * @param Cmd Pointer to the XPlmi_Cmd structure containing the subsystem information.
 * @return XStatus Returns XST_SUCCESS if the subsystem is added successfully,
 *         or an error code if the operation fails.
 *
 */
static XStatus XPm_DoAddSubsystem(XPlmi_Cmd* Cmd)
{
	return XPm_AddSubsystem(Cmd);
}

/**
 * @brief  Handle PM_CLOCK_GETRATE command to retrieve a clock's rate.
 *
 * @param  Cmd	Pointer to the command structure
 *
 * @return XST_SUCCESS if successful else error code
 *
 ****************************************************************************/
static XStatus XPm_DoGetClockRate(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 IndexedClockId = Cmd->Payload[0];
	u32 ClockId;
	u32 *Rate = (u32*)&Cmd->Response[1];

	Status = XPm_ResolveIndexedNodeId(IndexedClockId, &ClockId);
	if (XST_SUCCESS != Status) {
		Cmd->Response[0] = (u32)Status;
		/*
		 * For getter APIs, a hole in the SCMI (INDEXED) ID space is
		 * not a failure — the API returns SUCCESS with the actual
		 * status in the response payload so the caller (e.g. TF-A
		 * SCMI server) can act accordingly.
		 */
		if (XPM_PM_INVALID_NODE == Status) {
			Status = XST_SUCCESS;
		}
		goto done;
	}

	Status = XPmClock_GetRate(ClockId, Rate);
	Cmd->Response[0] = (u32)Status;

done:
	return Status;
}

#ifdef XILPM_NG_ENABLE_CLK_SCMI
/**
 * @brief  Handle PM_CLOCK_PROGRAM_RATE command to set a clock's rate.
 *
 * Resolves the indexed node ID, checks subsystem permissions for the
 * target clock, and programs the requested rate.
 *
 * @param  Cmd	Pointer to the command structure
 *
 * @return XST_SUCCESS if successful else error code
 *
 ****************************************************************************/
static XStatus XPm_DoProgramClockRate(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 IndexedClockId = Cmd->Payload[0];
	u32 ClockId;
	u32 ClkRate = Cmd->Payload[1];
	u32 Flags = Cmd->Payload[2];

	/* Reject rates beyond the supported PLL operating range. */
	if (ClkRate > PLL_VCO_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Resolve the indexed clock before applying access policy. */
	Status = XPm_ResolveIndexedNodeId(IndexedClockId, &ClockId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Permit only subsystems authorized to control the clock. */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Program the validated rate and caller-provided flags. */
	Status = XPmClock_ProgramRate(ClockId, ClkRate, Flags);

done:
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/**
 * @brief  Handle PM_CLOCK_DESCRIBERATE command to describe supported rates.
 *
 * Resolves the indexed node ID and returns the rate description
 * (format, min/max/step or discrete list) for the specified clock.
 *
 * @param  Cmd	Pointer to the command structure
 *
 * @return XST_SUCCESS if successful else error code
 *
 ****************************************************************************/
static XStatus XPm_DoDescribeClockRate(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 IndexedClockId = Cmd->Payload[0];
	u32 ClockId;
	u32 RateIdx = Cmd->Payload[1];

	Status = XPm_ResolveIndexedNodeId(IndexedClockId, &ClockId);
	if (XST_SUCCESS != Status) {
		Cmd->Response[0] = (u32)Status;
		/*
		 * For getter APIs, a hole in the SCMI (INDEXED) ID space is
		 * not a failure — the API returns SUCCESS with the actual
		 * status in the response payload so the caller (e.g. TF-A
		 * SCMI server) can act accordingly.
		 */
		if (XPM_PM_INVALID_NODE == Status) {
			Status = XST_SUCCESS;
		}
		goto done;
	}

	Status = XPmClock_DescribeRate(ClockId, RateIdx, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
done:
	return Status;
}
#endif /* XILPM_NG_ENABLE_CLK_SCMI */

XStatus XPm_AddSubsystem(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->Payload[0];

	if (0U != XPlmi_IsPlmUpdateDone()) {
		/* Skip during PLM update */
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPmSubsystem_Add(SubsystemId);
	Cmd->Response[0] = (u32)Status;
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function links a node (dev/rst/subsys/regnode) to a subsystem.
 *	   Requirement assignment could be made by XPm_RequestDevice() or
 *	   XPm_SetRequirement() call.
 *
 * @param  Args		Node specific arguments
 * @param  NumArgs	Number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_DoAddRequirement(XPlmi_Cmd* Cmd)
{

	return XPm_AddRequirement(Cmd);
}

XStatus XPm_AddRequirement(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 *Payload = Cmd->Payload;
	u32 PayloadLen = Cmd->PayloadLen;
	XPm_Subsystem *Subsystem;

	if (0U != XPlmi_IsPlmUpdateDone()) {
		/* Skip during PLM update */
		Status = XST_SUCCESS;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(Payload[0]);
	if (NULL != Subsystem) {
		Status = XPmSubsystem_AddRequirement(Subsystem, Payload, PayloadLen);
	} else {
		Status = XPM_INVALID_SUBSYSID;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPm_AddDDRMemRegnForDefaultSubsystem(const XPm_MemCtrlrDevice *MCDev)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = PM_SUBSYS_DEFAULT;
	XPm_Subsystem *Subsystem;
	u32 DeviceId;
	XPm_Device *Device;
	u64 Address;
	u64 Size;

	DeviceId = MCDev->Device.Node.Id;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		/* do not error if default subsystem is not found */
		Status = XST_SUCCESS;
		goto done;
	}

	for (u32 Cnt = 0U; Cnt < MCDev->RegionCount; Cnt++) {
		u32 MemRegnIndx = XPmDevice_GetMemRegnCount();
		DeviceId = MEMREGN_DEVID(MemRegnIndx);
		Address = MCDev->Region[Cnt].Address;
		Size = MCDev->Region[Cnt].Size;

		PmDbg("DeviceId: (0x%x) MemRegnDeviceId: (0x%x)\r\n", MCDev->Device.Node.Id, DeviceId);
		Status = XPm_AddMemRegnDevice(DeviceId, Address, Size);
		if (XST_DEVICE_BUSY == Status) {
			/* Memory region already exists, skip requirement add */
			Status = XST_SUCCESS;
			continue;
		}
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Device = XPmDevice_GetById(DeviceId);
		if (NULL == Device) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		Status = XPmRequirement_Add(Subsystem, Device,
						(u32)REQUIREMENT_FLAGS(1U,
						(u32)REQ_ACCESS_SECURE_NONSECURE,
						(u32)REQ_NO_RESTRICTION),
						(u32)PM_CAP_ACCESS, XPM_DEF_QOS);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function is to link devices to the default subsystem.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static XStatus XPm_AddReqsDefaultSubsystem(XPm_Subsystem *Subsystem)
{
	// FIXME: Remove this implementation when we have a Xilpm_Runtime (Subsys)
	// enabled as user defined subsystems apply in that case
	XStatus Status = XST_FAILURE;
	u32 i = 0U, Prealloc = 0U, Capability = 0U;

	for (i = 0U; i < (u32)XPM_NODEIDX_DEV_MAX; i++) {
		/*
		 * Note: XPmDevice_GetByIndex() assumes that the caller
		 * is responsible for validating the Node ID attributes
		 * other than node index.
		 */
		XPm_Device *Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) || (1U != XPmDevice_IsRequestable(Device->Node.Id))) {
			continue;
		}
		/* Always prealloc all requestable devices */
		Prealloc = 1U;
		/* Always use maximum capability and secure if applies */
		Capability = XPM_MAX_CAPABILITY | (u32)PM_CAP_SECURE;

		/**
		 * Since default subsystem is hard-coded for now, add security policy
		 * for all peripherals as REQ_ACCESS_SECURE. This allows any device
		 * with a _master_ port to be requested in secure mode if the topology
		 * supports it.
		 */
		Status = XPmRequirement_Add(Subsystem, Device,
				REQUIREMENT_FLAGS(Prealloc,
					(u32)REQ_ACCESS_SECURE,
					(u32)REQ_NO_RESTRICTION),
				Capability, XPM_DEF_QOS);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/**
		 * Add PS Mem-Regns for Default Subsystem
		 * - The following Mem-Regns are an exact 1-to-1 mapping
		 * of their parent memory controller devices extracted
		 * from device topology.
		 *
		 * Supported PS Mem-Regns are:
		 * - OCM
		 * - TCM
		 *
		 * Note: DDR Mem-Regns are added in the later part
		 * 		of the boot sequence
		 * Note: This is an important fix to perform memory flushing
		 * 		during subsystem restarts
		*/
		u32 SubClass = NODESUBCLASS(Device->Node.Id);
		u32 Type = NODETYPE(Device->Node.Id);
		if (((u32)XPM_NODESUBCL_DEV_MEM == SubClass) &&
			(((u32)XPM_NODETYPE_DEV_OCM == Type) ||
			((u32)XPM_NODETYPE_DEV_TCM == Type))) {
			const XPm_MemDevice *MemDevice = (XPm_MemDevice *)Device;
			u64 StartAddress = (u64)MemDevice->StartAddress;
			u64 EndAddress = (u64)MemDevice->EndAddress;
			u64 Size = EndAddress - StartAddress + 1U;

			u32 MemRegnIndx = XPmDevice_GetMemRegnCount();
			u32 MemRegnDeviceId = MEMREGN_DEVID(MemRegnIndx);

			PmDbg("DeviceId: (0x%x) -> MemRegnDeviceId: (0x%x)\r\n", Device->Node.Id, MemRegnDeviceId);
			Status = XPm_AddMemRegnDevice(MemRegnDeviceId, StartAddress, Size);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			XPm_Device *MemRegnDevice = XPmDevice_GetById(MemRegnDeviceId);
			if (NULL == MemRegnDevice) {
				Status = XST_DEVICE_NOT_FOUND;
				goto done;
			}
			Status = XPmRequirement_Add(Subsystem, MemRegnDevice,
				REQUIREMENT_FLAGS(1U,
				(u32)REQ_ACCESS_SECURE_NONSECURE,
				(u32)REQ_NO_RESTRICTION),
				(u32)PM_CAP_ACCESS, XPM_DEF_QOS);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	/* Add reset permissions */
	Status = XPmReset_AddPermForGlobalResets(Subsystem);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
/*****************************************************************************/
/**
 * @brief	This function registers and enables power related interrupt
 *
 * @return	XST_SUCCESS on success and XST_FAILURE or other error code on
 *		failure.
 *
 *****************************************************************************/
static XStatus RegisterNEnablePwrIntr(void)
{
	XStatus Status = XST_FAILURE;

	Status = XPlmi_RegisterHandler(XPLMI_IOMODULE_PMC_PWR_MB, (GicIntHandler_t)&XPm_PwrIntrHandler,
				       (void *)XPLMI_IOMODULE_PMC_PWR_MB);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PMC_PWR_MB);

END:
	return Status;
}


/****************************************************************************/
/**
 * @brief Add and link the individual ADMA reset nodes missing from topology.
 *
 * Existing reset nodes (for example, nodes restored during In-Place Update)
 * are skipped. Each new reset uses its ADMA device base address and receives a
 * reset handle linking it to that device for permission checks.
 *
 * @return XST_SUCCESS when all channels exist or are added;
 *         XST_DEVICE_NOT_FOUND when an ADMA device is absent;
 *         XST_BUFFER_TOO_SMALL when a reset handle cannot be allocated;
 *         XST_FAILURE when an added reset cannot be retrieved;
 *         otherwise an error returned by XPmReset_AddNode.
 *
 * @note TODO: This is a temporary workaround. Remove this function when
 *       individual ADMA reset nodes are added from topology.
 *
 ****************************************************************************/
static XStatus XPm_AddAdmaResetNodes(void)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	XPm_Device *AdmaDevice;
	XPm_ResetNode *Rst;
	XPm_ResetHandle *RstHandle;
	const u32 AdmaDeviceIds[] = {
		PM_DEV_ADMA_0, PM_DEV_ADMA_1, PM_DEV_ADMA_2, PM_DEV_ADMA_3,
		PM_DEV_ADMA_4, PM_DEV_ADMA_5, PM_DEV_ADMA_6, PM_DEV_ADMA_7
	};
	const u32 AdmaResetIds[] = {
		PM_RST_ADMA_0, PM_RST_ADMA_1, PM_RST_ADMA_2, PM_RST_ADMA_3,
		PM_RST_ADMA_4, PM_RST_ADMA_5, PM_RST_ADMA_6, PM_RST_ADMA_7
	};
	const u32 NumAdmaChannels = (u32)ARRAY_SIZE(AdmaResetIds);

	/* Populate only the ADMA reset nodes absent from the topology. */
	for (i = 0U; i < NumAdmaChannels; i++) {
		/* Skip if reset node already exists (e.g. restored during IPU) */
		if (NULL != XPmReset_GetById(AdmaResetIds[i])) {
			continue;
		}

		/* Resolve the channel device that supplies the reset base address. */
		AdmaDevice = XPmDevice_GetById(AdmaDeviceIds[i]);
		if (NULL == AdmaDevice) {
			PmErr("ADMA device 0x%x not found\r\n", AdmaDeviceIds[i]);
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}

		/* Create the channel reset using the associated device address. */
		Status = XPmReset_AddNode(AdmaResetIds[i],
					  AdmaDevice->Node.BaseAddress,
					  0U, 1U, (u8)XPM_RSTOPS_CUSTOM, 0U, NULL);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to add ADMA reset node 0x%x, Status=0x%x\r\n",
			      AdmaResetIds[i], Status);
			goto done;
		}

		/* Link reset to device via RstHandle for permission checking */
		Rst = XPmReset_GetById(AdmaResetIds[i]);
		if (NULL == Rst) {
			PmErr("Failed to get ADMA reset node 0x%x\r\n", AdmaResetIds[i]);
			Status = XST_FAILURE;
			goto done;
		}

		/* Allocate the link used for device-based permission checks. */
		RstHandle = (XPm_ResetHandle *)XPm_AllocBytes(sizeof(XPm_ResetHandle));
		if (NULL == RstHandle) {
			PmErr("Failed to allocate RstHandle for ADMA reset 0x%x\r\n", AdmaResetIds[i]);
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		/* Insert the device link at the head of the reset's handle list. */
		RstHandle->Device = AdmaDevice;
		RstHandle->NextDevice = Rst->RstHandles;
		Rst->RstHandles = RstHandle;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_HookAfterPlmCdo(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;

	/* TODO: Remove this workaround when individual ADMA reset nodes are added from topology */
	Status = XPm_AddAdmaResetNodes();
	if (XST_SUCCESS != Status) {
		PmErr("Failed to add ADMA reset nodes\r\n");
		goto done;
	}

	/* Registers all  power related interrupt post PLM cdo loaded*/
	Status = RegisterNEnablePwrIntr();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * Memory-range table seed pass. Placed BEFORE the PLM update guard so it
	 * runs on both cold boot and in-place PLM update, and present-gated
	 * internally so it is harmless when the table is absent. This is the only
	 * DDR State trigger at cold boot: DDRMC is not requestable and its
	 * bring-up bypasses XPmDevice_ChangeState.
	 */
	XPm_MemTblRecomputeAll();

	if (0U != XPlmi_IsPlmUpdateDone()) {
		/* Skip during PLM update */
		Status = XST_SUCCESS;
		goto done;
	}

	/* If default subsystem is present, attempt to add its requirements */
	Subsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);
	if ((NULL != Subsystem) && ((u8)ONLINE == Subsystem->State)) {
		Status = XPm_AddReqsDefaultSubsystem(Subsystem);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to add requirements to default subsystem\n\r");
			goto done;
		}
	} else {
		PmInfo("Default subsystem not present\n\r");
	}
	Status = XST_SUCCESS;

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function activates subsystem by requesting all pre-alloc
 *	   devices which are essential for susbystem to be operational.
 *
 * @param  SubsystemId	ID of subsystem which is requesting to activate other
 *			subsystem
 * @param  TargetSubsystemId	ID of subsystem which needs activation
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This command is only allowed from PMC subsystem
 *
 ****************************************************************************/
static XStatus XPm_ActivateSubsystem(u32 SubsystemId, u32 TargetSubsystemId)
{
	// //XPM_EXPORT_CMD(PM_ACTIVATE_SUBSYSTEM, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = NULL;
	(void)SubsystemId;

	/* Return success if the target subsystem is PMC or ASU */
	if (PM_SUBSYS_PMC == TargetSubsystemId) {
		Status = XST_SUCCESS;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(TargetSubsystemId);
	if (NULL != Subsystem) {
		Status = XPmSubsystem_Activate(Subsystem);
	} else {
		Status = XPM_INVALID_SUBSYSID;
	}

done:
	PmInfo("Subsystem 0x%x activation status = 0x%x\n\r",
		TargetSubsystemId, Status);
	return Status;
}


/**
 * @brief PMC activate subsystem by subsystem ID
*/
XStatus XPm_PmcActivateSubsystem(u32 SubsystemId)	{
	return XPm_ActivateSubsystem(PM_SUBSYS_PMC, SubsystemId);
}

static XStatus XPmPower_GetStatus(u32 DeviceId, XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *Power;

	Power = XPmPower_GetById(DeviceId);
	if (NULL == Power) {
		goto done;
	}
	DeviceStatus->Status = Power->Node.State;
	DeviceStatus->Usage = 0;
	DeviceStatus->Requirement = 0;
	Status = XST_SUCCESS;
done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function is used to obtain information about the current state
 * of a device. The caller must pass a pointer to an XPm_DeviceStatus
 * structure, which must be pre-allocated by the caller.
 *
 * @param  SubsystemId	Subsystem ID.
 * @param  DeviceId	Device ID
 * @param  DeviceStatus	Pointer to the device status
 *
 * - Status - The current power state of the device
 *  - For CPU nodes:
 *   - 0 : if CPU is powered down,
 *   - 1 : if CPU is active (powered up),
 *   - 8 : if CPU is suspending (powered up)
 *  - For power islands and power domains:
 *   - 0 : if island is powered down,
 *   - 2 : if island is powered up
 *  - For slaves:
 *   - 0 : if slave is powered down,
 *   - 1 : if slave is powered up,
 *   - 9 : if slave is in retention
 *
 * - Requirement - Requirements placed on the device by the caller
 *
 * - Usage
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
static XStatus XPm_GetDeviceStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);

	switch(NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPmDevice_GetStatus(Subsystem, DeviceId, DeviceStatus);
		break;
	case (u32)XPM_NODECLASS_POWER:
		Status = XPmPower_GetStatus(DeviceId, DeviceStatus);
		break;
	case (u32)XPM_NODECLASS_SUBSYSTEM:
		Status = XPmSubsystem_GetStatus(Subsystem, DeviceStatus);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Register a subsystem to be notified about the device event
 *
 * @param  IpiMask      IPI mask of current subsystem
 * @param  SubsystemId  Subsystem to be notified
 * @param  NodeId     Node to which the event is related
 * @param  Event        Event in question
 * @param  Wake         Wake subsystem upon capturing the event if value 1
 * @param  Enable       Enable the registration for value 1, disable for value 0
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 ****************************************************************************/
static XStatus XPm_RegisterNotifier(const u32 SubsystemId, const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 Enable,
			 const u32 IpiMask)
{

	XStatus Status = XST_FAILURE;
	XPm_Subsystem* Subsystem = NULL;
	u32 __errl__ = 0;
	/* Validate SubsystemId */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		PmErr("Invalid SubsystemId=0x%x\n\r", SubsystemId);
		__errl__ = __LINE__;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Unregister all notifiers of the subsystem at once. Used during a
	 * kexec-based kernel reload to clear stale notifier registrations.
	 * NodeId == PM_ALL_NOTIFIER is only valid with Enable == 0.
	 */
	if (PM_ALL_NOTIFIER == NodeId) {
		if (0U != Enable) {
			Status = XST_INVALID_PARAM;
			__errl__ = __LINE__;
			goto done;
		}
		Status = XPmNotifier_UnregisterAll(Subsystem);
		__errl__ = __LINE__;
		goto done;
	}

	/* Only Event, Device and Power Nodes are supported */
	if (((u32)XPM_NODECLASS_EVENT != NODECLASS(NodeId)) &&
	    ((u32)XPM_NODECLASS_DEVICE != NODECLASS(NodeId)) &&
	    ((u32)XPM_NODECLASS_POWER != NODECLASS(NodeId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Validate other parameters */
	if ((((u32)XPM_NODECLASS_DEVICE == NODECLASS(NodeId)) ||
	     ((u32)XPM_NODECLASS_POWER == NODECLASS(NodeId))) &&
	     (((0U != Wake) && (1U != Wake)) ||
	      ((0U != Enable) && (1U != Enable)) ||
	      (((u32)EVENT_STATE_CHANGE != Event) &&
	       ((u32)EVENT_ZERO_USERS != Event) &&
	       ((u32)EVENT_CPU_IDLE_FORCE_PWRDWN != Event)))) {
		Status = XST_INVALID_PARAM;
		__errl__ = __LINE__;
		goto done;

	}
	if (((u32)XPM_NODECLASS_EVENT == NODECLASS(NodeId)) &&
	    (((0U != Wake) && (1U != Wake)) ||
	     ((0U != Enable) && (1U != Enable)))) {
		Status = XST_INVALID_PARAM;
		__errl__ = __LINE__;
		goto done;
	}

	if (0U == Enable) {
		Status = XPmNotifier_Unregister(Subsystem, NodeId, Event);
		__errl__ = __LINE__;
	} else {
		Status = XPmNotifier_Register(Subsystem, NodeId, Event, Wake,
					      IpiMask);
		__errl__ = __LINE__;
	}

done:
	if(XST_SUCCESS != Status){
		PmErr("NodeId =0x%x Line= %d Status = %d\n\r", NodeId, __errl__, Status);
	}
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a processor self-suspend command.
 */
/****************************************************************************/
static XStatus XPm_DoSelfSuspend(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 DeviceId = Cmd->Payload[0];
	u32 Latency = Cmd->Payload[1];
	u8 State = (u8)(Cmd->Payload[2] & 0xFFU);
	u32 AddrLow = Cmd->Payload[3];
	u32 AddrHigh = Cmd->Payload[4];
	Status = XPm_SelfSuspend(SubsystemId, DeviceId, Latency, State, AddrLow, AddrHigh);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/* XPmSubsystem_GetIPIMask() is now defined in xpm_subsystem.c */

maybe_unused static inline XStatus XPm_EnableDdrSr(const u32 SubsystemId)
{
	/*
	 * TODO: If subsystem is using DDR and NOC Power Domain is idle,
	 * enable self-refresh as post suspend requirement
	 */
	(void)SubsystemId;
	return XST_SUCCESS;
}
maybe_unused static inline XStatus XPm_DisableDdrSr(const u32 SubsystemId)
{
	/* TODO: If subsystem is using DDR, disable self-refresh */
	(void)SubsystemId;
	return XST_SUCCESS;
}
maybe_unused static inline void XPm_ClearScanClear(void)
{
	/* Unused function for versal_2ve_2vm */
}
/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to suspend a child
 * subsystem.
 *
 * @param SubsystemId	Subsystem ID
 * @param DeviceId	Processor device ID
 * @param Latency	Maximum wake-up latency requirement in us(microsecs)
 * @param State		Instead of specifying a maximum latency, a CPU can also
 *			explicitly request a certain power state.
 * @param AddressLow	Lower Address from which to resume when wake up.
 * @param AddressHigh	Higher Address from which to resume when wake up.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPm_SelfSuspend(const u32 SubsystemId, const u32 DeviceId,
			const u32 Latency, const u8 State,
			u32 AddrLow, u32 AddrHigh)
{
	// //XPM_EXPORT_CMD(PM_SELF_SUSPEND, XPLMI_CMD_ARG_CNT_FIVE, XPLMI_CMD_ARG_CNT_FIVE);
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;
	u64 Address = (u64)AddrLow + ((u64)AddrHigh << 32ULL);
	u32 CpuIdleFlag;
	XPm_Subsystem *Subsystem = NULL;

	/* TODO: Remove this warning fix hack when functionality is implemented */
	(void)Latency;

	Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	if ((NULL == Core) ||
	    (NODESUBCLASS(DeviceId) != (u32)XPM_NODESUBCL_DEV_CORE)) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	if ((PM_SUSPEND_STATE_SUSPEND_TO_RAM != State) &&
	    (PM_SUSPEND_STATE_CPU_OFF != State) &&
	    (PM_SUSPEND_STATE_CPU_IDLE != State)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	CpuIdleFlag = (State == PM_SUSPEND_STATE_CPU_IDLE) ? (1U) : (0U);

	Status = XPmCore_StoreResumeAddr(Core, (Address | 1U));
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (PM_SUSPEND_STATE_SUSPEND_TO_RAM == State) {
		Subsystem = XPmSubsystem_GetById(SubsystemId);
		if (NULL == Subsystem) {
			Status = XPM_INVALID_SUBSYSID;
			goto done;
		}
		/* Clear the pending suspend cb reason */
		Subsystem->PendCb.Reason = 0U;

		Status = XPmSubsystem_SetState(Subsystem, (u32)SUSPENDING);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/*
		 * If subsystem is using DDR and NOC Power Domain is idle,
		 * enable self-refresh as post suspend requirement
		 */
		Status = XPm_EnableDdrSr(SubsystemId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XPmCore_SetCPUIdleFlag(Core, CpuIdleFlag);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	ENABLE_WFI(Core->SleepMask);

	if (PM_SUSPEND_STATE_CPU_OFF == State) {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN;
	} else {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_SUSPENDING;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
static XStatus XPm_DoForcePowerdown(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 NodeId = Cmd->Payload[0];
	u32 Ack = Cmd->Payload[1];
	u32 CmdType = Cmd->IpiReqType;
	u32 IpiMask = Cmd->IpiMask;
	/**
	 *  Skip providing ack for the force power down command as it is
	 *  handled from the xilpm module itself.
	 */
	Cmd->AckInPLM = 0U;
	Status = XPm_ForcePowerdown(SubsystemId, NodeId, Ack, CmdType, IpiMask);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to Powerdown other
 * 	   processor or domain node or subsystem forcefully.
 *
 * @param SubsystemId	Subsystem ID
 * @param Node 		Processor or domain node or subsystem to be powered down
 * @param Ack		Ack request
 * @param CmdType	IPI command request type
 * @param IpiMask	IPI mask of initiator
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   The affected PUs are not notified about the upcoming powerdown,
 *         and PLM does not wait for their WFI interrupt.
 *
 ****************************************************************************/
static XStatus XPm_ForcePowerdown(u32 SubsystemId, const u32 NodeId, const u32 Ack,
			   const u32 CmdType, const u32 IpiMask)
{
//	//XPM_EXPORT_CMD(PM_FORCE_POWERDOWN, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = NULL;
	u32 NodeState = 0U;

	if ((u32)REQUEST_ACK_BLOCKING == Ack) {
		/* Disable IPI interrupt */
		PmOut32(IPI_PMC_IDR, IpiMask);
	}

	/* Validate access first */
	Status = XPm_IsForcePowerDownAllowed(SubsystemId, NodeId, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Retrieve target subsystem */
	Subsystem = XPmSubsystem_GetById(NodeId);
	if (NULL == Subsystem) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Subsystem->FrcPwrDwnReq.AckType = Ack;
	Subsystem->FrcPwrDwnReq.InitiatorIpiMask = IpiMask;
	NodeState = Subsystem->State;

	/* Restore multiboot register value*/
	XPlmi_RestoreMultiboot();

	if (0U != (Subsystem->Flags & (u8)SUBSYSTEM_IDLE_SUPPORTED)) {
		Status = XPmSubsystem_StartRecoveryTimer(Subsystem, CmdType);
		if (XST_DEVICE_NOT_FOUND == Status) {
			PmWarn("No recovery timer found, add runtime HB_MON node for recovery!\r\n");
		} else if (XST_SUCCESS != Status) {
			/*
			 * Error while requesting run time Healthy Boot
			 * Monitor node
			 */
			goto done;
		} else {
			/* Required by MISRA */
		}

		Status = XPmSubsystem_SetState(Subsystem,
					       (u8)PENDING_POWER_OFF);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_SubsystemIdleCores(Subsystem);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		Status = XPmSubsystem_ForcePwrDwn(NodeId);
		if (XST_SUCCESS != Status) {
			/*
			 * Checked here, as at the other three call sites, so the
			 * failing subsystem is named.  The status still reaches the
			 * acknowledge below and the common exit unchanged.
			 */
			PmErr("Force power down of subsystem 0x%x failed: 0x%x\r\n",
			      NodeId, Status);
		}
	}

done:
	/* Get current node state */
	if (NULL != Subsystem) {
		NodeState = Subsystem->State;
	}
	/* Process ack request */
	XPm_ProcessAckReq(Ack, IpiMask, Status, NodeId, NodeState);

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

static void XPm_CoreIdle(XPm_Core *Core)
{
	if (1U == Core->isCoreUp) {
		Core->Device.Node.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN;
	}
	XPmNotifier_Event(Core->Device.Node.Id,
			  (u8)EVENT_CPU_IDLE_FORCE_PWRDWN);
}

XStatus XPm_SubsystemIdleCores(const XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;

	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		const XPm_Requirement *Reqm = ReqmNode->Data;
		u32 DeviceId;
		u8 IsCoreIdleSupported = 0U;

		if ((NULL == Reqm) || (NULL == Reqm->Device)) {
			Status = XST_FAILURE;
			goto done;
		}
		DeviceId = Reqm->Device->Node.Id;

		/* Make sure core device is requested by the subsystem */
		if ((1U != Reqm->Allocated) ||
			((u32)XPM_NODESUBCL_DEV_CORE != NODESUBCLASS(DeviceId))) {
			continue;
		}
		XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
		if (NULL == Core) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		PmInfo("Check if core 0x%x supports idle callback\r\n", Core->Device.Node.Id);
		Status = XPmCore_GetCoreIdleSupport(Core, &IsCoreIdleSupported);
		if (XST_SUCCESS != Status) {
			if (XST_DEVICE_NOT_FOUND == Status) {
				u32 CoreType = NODETYPE(Core->Device.Node.Id);
				if ((u32)XPM_NODETYPE_DEV_CORE_RPU == CoreType) {
					PmDbg("-> RPU: No IdleCb for 0x%x, skipping\r\n",
							Core->Device.Node.Id);
					continue;
				}
				if ((u32)XPM_NODETYPE_DEV_CORE_APU == CoreType) {
					/* APU core without idle callback support */
					IsCoreIdleSupported = 1U;
					PmDbg("-> APU: No IdleCb for 0x%x, notifying anyway\r\n",
							Core->Device.Node.Id);
					goto cb;
				}
			}
			PmErr("Failed to get IdleCb Support for 0x%x: 0x%x\r\n",
					Core->Device.Node.Id, Status);
			goto done;
		}
cb:
		if (1U == IsCoreIdleSupported) {
			PmInfo("Sending IdleCb notification to 0x%x\r\n", Core->Device.Node.Id);
			XPm_CoreIdle(Core);
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  Function for processing ack request
 *
 * @param Ack		Acknowledgement type
 * @param IpiMask	IPI mask of initiator of request
 * @param Status	Return status
 * @param NodeId	Node ID of target subsystem/core
 * @param NodeState	State of target node
 *
 * @return None
 *
 * @note   None
 *
 ****************************************************************************/
static void XPm_ProcessAckReq(const u32 Ack, const u32 IpiMask, const XStatus Status,
		       const u32 NodeId, const u32 NodeState)
{
#ifdef XPLMI_IPI_DEVICE_ID
	if (0U == IpiMask) {
		goto done;
	}
	if ((u32)REQUEST_ACK_NON_BLOCKING == Ack) {
		/* Return acknowledge through callback */
		IPI_MESSAGE4(IpiMask, (u32)PM_ACKNOWLEDGE_CB, NodeId, (u32)Status,
			      NodeState);
		if (XST_SUCCESS != XPlmi_IpiTrigger(IpiMask)) {
			PmWarn("Error in IPI trigger from PMC to IpiMask: 0x%x\r\n", IpiMask);
		}
	}
	/**
	 * In either ACK case (NO_ACK, ACK_BLOCKING and ACK_NON_BLOCKING),
	 * Send response and enable PMC interrupt
	 **/
	IPI_RESPONSE1(IpiMask, (u32)Status);
	/* Clear interrupt status */
	PmOut32(IPI_PMC_ISR, IpiMask);
	/* Enable IPI interrupt */
	PmOut32(IPI_PMC_IER, IpiMask);

done:
	return;
#else
	(void)Ack;
	(void)IpiMask;
	(void)Status;
	(void)NodeId;
	(void)NodeState;
#endif /* XPLMI_IPI_DEVICE_ID */
}
/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to aborting suspend of a
 * child subsystem.
 *
 * @param SubsystemId	Subsystem ID
 * @param Reason	Abort reason
 * @param DeviceId	Processor device ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note        None
 *
 ****************************************************************************/
static XStatus XPm_AbortSuspend(const u32 SubsystemId, const u32 Reason,
			 const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);

	PmInfo("(%lu, %lu)\r\n", Reason, DeviceId);
	if (NULL == Subsystem) {
		PmErr("Invalid Subsystem Id\n\r");
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	} else if((NODECLASS(DeviceId) == (u32)XPM_NODECLASS_DEVICE) &&
	   (NODESUBCLASS(DeviceId) == (u32)XPM_NODESUBCL_DEV_CORE)) {
		Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
		if (NULL == Core) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;
	} else {
		PmErr("Invalid Device Id\n\r");
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if ((Reason < (u32)ABORT_REASON_MIN) || (Reason > (u32)ABORT_REASON_MAX)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DISABLE_WFI(Core->SleepMask);

	Status = XPmSubsystem_SetState(Subsystem, (u32)ONLINE);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a suspend-abort command.
 */
/****************************************************************************/
static XStatus XPm_DoAbortSuspend(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 Reason = Cmd->Payload[0];
	u32 DeviceId = Cmd->Payload[1];
	Status = XPm_AbortSuspend(SubsystemId, Reason, DeviceId);
	Cmd->Response[0] = (u32)Status;
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to start-up and wake-up
 * a child subsystem.  If the target subsystem has been loaded and is ready
 * to run, it will start running.  If the target subsystem is suspended, it
 * will resume.
 *
 * @param SubsystemId		Subsystem ID
 * @param  DeviceId	Processor device ID
 * @param  SetAddress 	Specifies whether to set the start address.
 * - 0 : do not set start address
 * - 1 : set start address
 * @param  Address	Address from which to resume when woken up.
 * @param  Ack		Ack request
 * @param  CmdType	IPI command request type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been received.
 *
 ****************************************************************************/
XStatus XPm_RequestWakeUp(u32 SubsystemId, const u32 DeviceId,
			  const u32 SetAddress, const u64 Address,
			  const u32 Ack,
			  const u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;
	u32 CoreSubsystemId, CoreDeviceId;
	const XPm_Power *Power;

	/* Warning Fix */
	(void) (Ack);

	/*Validate access first */
	Status = XPm_IsWakeAllowed(SubsystemId, DeviceId, CmdType);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	switch (NODECLASS(DeviceId))
	{
		case (u32)XPM_NODECLASS_SUBSYSTEM:
			CoreSubsystemId = DeviceId;
			Status = XPm_SubsystemPwrUp(CoreSubsystemId);
			break;
		case (u32)XPM_NODECLASS_DEVICE:
			CoreDeviceId = DeviceId;
			Core = (XPm_Core *)XPmDevice_GetById(CoreDeviceId);
			if (NULL == Core) {
				Status = XPM_ERR_WAKEUP;
				break;
			}
			if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(CoreDeviceId)) ||
			    ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(CoreDeviceId))) {
				/* Power up LPD if not powered up */
				Power = XPmPower_GetById(PM_POWER_LPD);
				if ((NULL != Power) && ((u8)XPM_POWER_STATE_OFF == Power->Node.State)) {
					Status = XPm_RestartCbWrapper(Power->Node.Id);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}

				/* If APU Cores are requested to wakeup, re-load FPD CDO */
				if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(CoreDeviceId)) {
					Power = XPmPower_GetById(PM_POWER_FPD);
					if ((NULL != Power) && ((u8)XPM_POWER_STATE_OFF == Power->Node.State)) {
						Status = XPm_RestartCbWrapper(Power->Node.Id);
						if (XST_SUCCESS != Status) {
							PmErr("Error during RestartCbWrapper: 0x%x\r\n", Status);
							goto done;
						}
					}

					/* clear GIC Proxy setup */
					XPmGicProxy_ClearWakeup();
				}
			}
			/* Make sure subsystem owns the core before requesting wakeup */
			Status = XPmDevice_Request(SubsystemId, DeviceId,
					     (u32)XPM_MAX_CAPABILITY,
					     (u32)XPM_DEF_QOS,
					     CmdType);
			if (XST_SUCCESS != Status) {
				PmErr("Failed to request core 0x%x for subsystem 0x%x\r\n",
					DeviceId, SubsystemId);
				goto done;
			}
			Status = XPmCore_RequestWakeup(Core, SetAddress, Address);
			if (XST_SUCCESS == Status) {
				Status = SetSubsystemState_ByCore(Core, (u32)ONLINE);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* If subsystem is using DDR, disable self-refresh */
	Status = XPm_DisableDdrSr(CoreSubsystemId);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
static XStatus XPm_DoWakeUp(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 DeviceId;
	u32 IndexedDeviceId = Cmd->Payload[0];
	/**  setAddress is encoded in the LSB bit of the low-word address */
	/**  we can do this because addresses are word alinghed ignore LSB */
	u32 SetAddress = Cmd->Payload[1] & 0x1U;
	u32 AddressLow = Cmd->Payload[1] & (~(u32)0x1U);
	u32 AddressHigh = Cmd->Payload[2];
	u64 Address = (u64)AddressLow + ((u64)AddressHigh << 32ULL);
	u32 Ack = Cmd->Payload[3];
	u32 CmdType =Cmd->IpiReqType;

	Status = XPm_ResolveIndexedNodeId(IndexedDeviceId, &DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_RequestWakeUp(SubsystemId, DeviceId, SetAddress, Address, Ack, CmdType);
	Cmd->Response[0] = (u32)Status;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/* Cleanup system activities after subsystem shutdown */
static XStatus XPmSubsystem_ShutdownSystemCleanup(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Reqm = NULL;
	const XPm_Subsystem *Subsystem = NULL;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Reqm = XPmDevice_FindRequirement(PM_DEV_ACPU_0_0, SubsystemId);
	if (NULL != Reqm) {
		/*
		 * Do APU GIC pulse reset:
		 * if all the cores are in power OFF state & FPD is ON
		 * Only applicable for APU Subsystem
		 */
		Status = ResetAPUGic(PM_DEV_ACPU_0_0);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* TODO: SMMU disable if required */
	}

	/* Flush LLC in CMN block before reloading subsystem Image */
	Status = XPmSubsystem_CmnFlush(SubsystemId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

/* Subsystem power up procedure */
static XStatus XPm_SubsystemPwrUp(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Activate the subsystem by requesting its pre-alloc devices */
	Status = XPmSubsystem_Activate(Subsystem);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Reload the subsystem image */
	Status = XPm_RestartCbWrapper(SubsystemId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function restarts the given subsystem.
 *
 * @param  SubsystemId	Subsystem ID to restart
 *
 * @return XST_SUCCESS if successful else appropriate return code.
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_RestartCbWrapper(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;

	if (NULL != PmRestartCb) {
		Status = PmRestartCb(SubsystemId, NULL);
	}

	return Status;
}


/****************************************************************************/
/**
 * @brief Set the state of the subsystem that owns a processor core.
 *
 * @param Core  Pointer to the processor core.
 * @param State Subsystem state to set.
 *
 * @return XST_SUCCESS on success, XST_DEVICE_NOT_FOUND for a NULL core,
 *         XPM_ERR_SUBSYS_NOTFOUND when no owner is found, or an error returned
 *         by XPmSubsystem_SetState.
 *
 ****************************************************************************/
static XStatus SetSubsystemState_ByCore(const XPm_Core* Core, const u32 State){
	XStatus Status = XST_FAILURE;
	/* Require a valid core before resolving its owning subsystem. */
	if (NULL == Core) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Resolve the subsystem currently associated with the core. */
	u32 CoreSubsystemId = XPmDevice_GetSubsystemIdOfCore((const XPm_Device *)Core);
	if (INVALID_SUBSYSID == CoreSubsystemId) {
		PmErr("Invalid Subsystem Id for core %d\n\r", Core->Device.Node.Id);
		Status = XPM_ERR_SUBSYS_NOTFOUND;
		goto done;
	}
	/* Look up the owner and apply the requested subsystem state. */
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(CoreSubsystemId);

	Status = XPmSubsystem_SetState(Subsystem, State);
done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function is used to perform self subsystem shutdown, self subsystem
 * 	   restart or system restart
 *
 * @param SubsystemId	Subsystem ID
 * @param  Type		Shutdown type
 * @param SubType	Shutdown subtype
 * @param CmdType	IPI command request type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been received.
 *
 ****************************************************************************/
XStatus XPm_SystemShutdown(u32 SubsystemId, u32 Type, u32 SubType, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	const XPm_ResetNode *Rst;
	XPm_Power *Fpd = XPmPower_GetById(PM_POWER_FPD);

	PmDbg("%s: SubsystemId=0x%x, Type=0x%x, SubType=0x%x\r\n",
		SubsystemId, Type, SubType);

	/* Accept only subsystem shutdown and reset operations. */
	if ((PM_SHUTDOWN_TYPE_SHUTDOWN != Type) &&
	    (PM_SHUTDOWN_TYPE_RESET != Type)) {
		Status = XPM_INVALID_TYPEID;
		goto done;
	}

	/* Resolve the caller before choosing the shutdown procedure. */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* For shutdown type the subtype is irrelevant: shut the caller down */
	if (PM_SHUTDOWN_TYPE_SHUTDOWN == Type) {
		if (0U != (SUBSYSTEM_IDLE_SUPPORTED & Subsystem->Flags)) {
			/* Defer shutdown until the subsystem's cores report idle. */
			Status = XPmSubsystem_SetState(Subsystem, (u8)PENDING_POWER_OFF);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = XPm_SubsystemIdleCores(Subsystem);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			/* Power down immediately and clean up shared system state. */
			Status = XPmSubsystem_ForcePwrDwn(SubsystemId);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = XPmSubsystem_ShutdownSystemCleanup(SubsystemId);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		/* We're done */
		goto done;
	}

	switch (SubType) {
	case PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM:
		/* Restore multiboot register value*/
		XPlmi_RestoreMultiboot();

		/*
		 * Hold a reference on FPD for the whole restart so that its use
		 * count cannot drop low enough for the power framework to
		 * switch the domain off. FPD is not meant to be power cycled by
		 * a subsystem restart.
		 */
		if (NULL != Fpd) {
			Fpd->UseCount++;
		}

		if (0U != (SUBSYSTEM_IDLE_SUPPORTED & Subsystem->Flags)) {
			/* Arm recovery monitoring before waiting for cores to idle. */
			Status = XPmSubsystem_StartRecoveryTimer(Subsystem, CmdType);
			if (XST_DEVICE_NOT_FOUND == Status) {
				PmWarn("No recovery timer found, add runtime HB_MON node for recovery!\r\n");
			} else if (XST_SUCCESS != Status) {
				/*
				 * Error while requesting run time Healthy Boot
				 * Monitor node
				 */
				goto fpd_restore;
			} else {
				/* Required by MISRA */
			}

			Status = XPmSubsystem_SetState(Subsystem, (u8)PENDING_RESTART);
			if (XST_SUCCESS != Status) {
				goto fpd_restore;
			}
			Status = XPm_SubsystemIdleCores(Subsystem);
			if (XST_SUCCESS != Status) {
				goto fpd_restore;
			}
		} else {
			/* Power down immediately and clean up shared system state. */
			Status = XPmSubsystem_ForcePwrDwn(SubsystemId);
			if (XST_SUCCESS != Status) {
				goto fpd_restore;
			}
			Status = XPmSubsystem_ShutdownSystemCleanup(SubsystemId);
			if (XST_SUCCESS != Status) {
				goto fpd_restore;
			}
			Status = XPm_SubsystemPwrUp(SubsystemId);
		}

fpd_restore:
		/* Restart finished or aborted: release the FPD reference */
		if ((NULL != Fpd) && (0U < Fpd->UseCount)) {
			Fpd->UseCount--;
		}
		break;
	case PM_SHUTDOWN_SUBTYPE_RST_SYSTEM:
		/*
		 * Validate the caller against the reset used for system reset.
		 * Caller subsystem may not be allowed to enact reset operation
		 * upon PM_RST_PMC. XPmReset_SystemReset uses PM_RST_PMC.
		 */
		Rst = XPmReset_GetById(PM_RST_PMC);
		if (NULL == Rst) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Status = XPmReset_IsOperationAllowed(SubsystemId, Rst, CmdType);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPmReset_SystemReset();
		break;
	default:
		Status = XPM_INVALID_TYPEID;
		break;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a subsystem shutdown or reset command.
 */
/****************************************************************************/
static XStatus XPm_DoSystemShutdown(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 Type = Cmd->Payload[0];
	u32 SubType = Cmd->Payload[1];
	u32 CmdType = Cmd->IpiReqType;
	Status = XPm_SystemShutdown(SubsystemId, Type, SubType, CmdType);
	/* Return the service status in-band even when command dispatch also fails. */
	Cmd->Response[0] = (u32)Status;
	if (XST_SUCCESS != Status) {
		PmErr("Subsystem: 0x%x, Type: 0x%x, SubType: 0x%x, Status: 0x%x\n\r",
			SubsystemId, Type, SubType, Status);
	}
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a maximum device-latency command.
 */
/****************************************************************************/
static XStatus XPm_DoSetMaxLatency(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 DeviceId = Cmd->Payload[0];
	u32 Latency = Cmd->Payload[1];
	Status = XPm_SetMaxLatency(SubsystemId, DeviceId, Latency);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  Set maximum allowed latency for the device
 *
 * @param  SubsystemId	Initiator of the request who must previously requested
 *			the device
 * @param  DeviceId	Device whose latency is specified
 * @param  Latency	Maximum allowed latency in micro sec
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code or
 * a reason code
 *
 ****************************************************************************/
static XStatus XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
		      const u32 Latency)
{
	XStatus Status = XPM_ERR_SET_LATENCY;

	PmInfo("(%x, %lu)\r\n", DeviceId, Latency);

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmDevice_SetMaxLatency(SubsystemId, DeviceId, Latency);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}



/****************************************************************************/
/**
 * @brief Handle a reset assert, release, or pulse command.
 *
 * Resolves the indexed reset ID and applies the requested reset action using
 * the caller's subsystem and command security type.
 *
 * @param Cmd Pointer to the command payload, request metadata, and response.
 *
 * @return XST_SUCCESS on success, otherwise an indexed-ID resolution, access,
 *         validation, or reset operation error.
 *
 ****************************************************************************/
static XStatus XPm_DoResetAssert(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 IndexedResetId = Cmd->Payload[0];
	u32 ResetId;
	u32 CmdType = Cmd->IpiReqType;
	u32 Action = Cmd->Payload[1];

	/* Resolve the external reset index before dispatching the action. */
	Status = XPm_ResolveIndexedNodeId(IndexedResetId, &ResetId);
	if (XST_SUCCESS != Status) {
		Cmd->Response[0] = (u32)Status;
		goto done;
	}

	/* Apply the requested action under the caller's security context. */
	Status = XPm_SetResetState(SubsystemId, ResetId, Action, CmdType);
	Cmd->Response[0] = (u32)Status;
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reset or de-reset a device. Alternatively a reset
 *	   pulse can be requested as well.
 *
 * @param SubsystemId	Subsystem ID
 * @param ResetId	Reset ID
 * @param Action	Reset action to be taken
 *			- PM_RESET_ACTION_RELEASE for Release Reset
 *			- PM_RESET_ACTION_ASSERT for Assert Reset
 *			- PM_RESET_ACTION_PULSE for Pulse Reset
 * @param CmdType	IPI command request type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   To de-reset a device, the subsystem must be using the device, and
 * all the upstream reset line(s) must be de-asserted.  To reset a device, the
 * subsystem must be the only user of the device, and all downstream devices
 * (in terms of reset dependency) must be already in reset state.  Otherwise,
 * this request will be denied.
 *
 ****************************************************************************/
static XStatus XPm_SetResetState(const u32 SubsystemId, const u32 ResetId,
			  const u32 Action, const u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	u32 SubClass = NODESUBCLASS(ResetId);
	u32 SubType = NODETYPE(ResetId);
	const XPm_ResetNode* Reset;

	Reset = XPmReset_GetById(ResetId);
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Only peripheral, debug and particular specified resets
	 * are allowed to control externally, on other masters.
	 */
	if ((((u32)XPM_NODESUBCL_RESET_PERIPHERAL == SubClass) && ((u32)XPM_NODETYPE_RESET_PERIPHERAL == SubType)) ||
	    (((u32)XPM_NODESUBCL_RESET_DBG == SubClass) && ((u32)XPM_NODETYPE_RESET_DBG == SubType))) {
		/* Check if subsystem is allowed to access requested reset */
		Status = XPm_IsAccessAllowed(SubsystemId, ResetId);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	} else {
		/*
		 * Only a few global resets are allowed to use permissions policy.
		 */
		Status = XPmReset_IsPermissionReset(ResetId);
		if ((XST_SUCCESS != Status) && (PM_SUBSYS_PMC != SubsystemId)) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}

		Status = XPmReset_IsOperationAllowed(SubsystemId, Reset, CmdType);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	}

	PmInfo("Subsystem 0x%x requests action %d on reset 0x%x\r\n",
		SubsystemId, Action, ResetId);

	Status = XPmReset_AssertbyId(ResetId, Action);

done:
	return Status;
}


/****************************************************************************/
/**
 * @brief Return the reset state in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoGetResetState(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 ResetId = Cmd->Payload[0];
	u32 State = 0;
	Status = XPmReset_GetStateById(ResetId, &State);
	Cmd->Response[1] = State;
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function initializes subsystem and releases unused devices
 *
 * @param SubsystemId  ID of the subsystem
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_DoInitFinalize(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem* Subsystem = XPmSubsystem_GetById(Cmd->SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}
	Status = XPmSubsystem_InitFinalize(Subsystem);
	Cmd->Response[0] = (u32)Status;
done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function is used to request the version and ID code of a chip
 *
 * @param  IDCode  Returns the chip ID code.
 * @param  Version Returns the chip version.
 *
 * @return XST_SUCCESS
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_GetChipID(u32* IDCode, u32 *Version)
{
	/* Read the chip ID code */
	PmIn32(PMC_TAP_IDCODE, *IDCode);

	/* Read the chip version */
	PmIn32(PMC_TAP_VERSION, *Version);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief Return the chip ID code and version in Response[1] and Response[2].
 */
/****************************************************************************/
static XStatus XPm_DoGetChipID(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;

	u32 IDCode = 0;
	u32 Version = 0;
	Status = XPm_GetChipID(&IDCode, &Version);
	Cmd->Response[1] = IDCode;
	Cmd->Response[2] = Version;
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a subsystem activation command.
 */
/****************************************************************************/
static XStatus XPm_DoActivateSubsystem(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 TargetSubsystemId = Cmd->Payload[0];
	Status = XPm_ActivateSubsystem(SubsystemId, TargetSubsystemId);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a pin-control request command.
 */
/****************************************************************************/
static XStatus XPm_DoPinCtrlRequest(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 PinId = Cmd->Payload[0];
	Status = XPm_PinCtrlRequest(SubsystemId, PinId);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function requests the pin.
 *
 * @param SubsystemId	Subsystem ID
 * @param PinId		ID of the pin node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_PinCtrlRequest(const u32 SubsystemId, const u32 PinId)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPin_Request(SubsystemId, PinId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function releases the pin.
 *
 * @param SubsystemId	Subsystem ID
 * @param PinId		ID of the pin node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_PinCtrlRelease(const u32 SubsystemId, const u32 PinId)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPin_Release(SubsystemId, PinId);

	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a pin-control release command.
 */
/****************************************************************************/
static XStatus XPm_DoPinCtrlRelease(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 PinId = Cmd->Payload[0];
	Status = XPm_PinCtrlRelease(SubsystemId, PinId);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function sets the pin function.
 *
 * @param SubsystemId	Subsystem ID
 * @param PinId			Pin node ID
 * @param FunctionId	Function for the pin
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If no change to the pin function setting is required (the pin is
 * already set up for this function), this call will be successful.
 * Otherwise, the request is denied unless the subsystem has already
 * requested this pin.
 *
 ****************************************************************************/
static XStatus XPm_SetPinFunction(const u32 SubsystemId,
	const u32 PinId, const u32 FunctionId)
{
	XStatus Status = XST_FAILURE;

	/* Check if subsystem is allowed to access or not */
	Status = XPm_IsAccessAllowed(SubsystemId, PinId);
	if(Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmPin_CheckPerms(SubsystemId, PinId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmPin_SetPinFunction(PinId, FunctionId);

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a pin-function selection command.
 */
/****************************************************************************/
static XStatus XPm_DoSetPinFunction(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 PinId = Cmd->Payload[0];
	u32 FunctionId = Cmd->Payload[1];
	Status = XPm_SetPinFunction(SubsystemId, PinId, FunctionId);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function reads the pin function.
 *
 * @param PinId			ID of the pin node
 * @param FunctionId	Address to store the function
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_GetPinFunction(const u32 PinId, u32 *const FunctionId)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPin_GetPinFunction(PinId, FunctionId);

	return Status;
}
/****************************************************************************/
/**
 * @brief Return the pin function in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoGetPinFunction(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 PinId = Cmd->Payload[0];
	Status = XPm_GetPinFunction(PinId, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function sets the pin parameter value.
 *
 * @param  SubsystemId	Subsystem ID
 * @param PinId			Pin node ID
 * @param ParamId		Pin parameter ID
 * @param ParamVal		Pin parameter value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If no change to the pin parameter setting is required (the pin
 * parameter is already set up for this value), this call will be successful.
 * Otherwise, the request is denied unless the subsystem has already
 * requested this pin.
 *
 ****************************************************************************/
static XStatus XPm_SetPinParameter(const u32 SubsystemId, const u32 PinId,
			const u32 ParamId,
			const u32 ParamVal)
{
	XStatus Status = XST_FAILURE;

	/* Check if subsystem is allowed to access or not */
	Status = XPm_IsAccessAllowed(SubsystemId, PinId);
	if(Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmPin_CheckPerms(SubsystemId, PinId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmPin_SetPinConfig(PinId, ParamId, ParamVal);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a pin-parameter configuration command.
 */
/****************************************************************************/
static XStatus XPm_DoSetPinParameter(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 PinId = Cmd->Payload[0];
	u32 ParamId = Cmd->Payload[1];
	u32 ParamVal = Cmd->Payload[2];
	Status = XPm_SetPinParameter(SubsystemId, PinId, ParamId, ParamVal);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function reads the pin parameter value.
 *
 * @param PinId		ID of the pin node
 * @param ParamId	Pin parameter ID
 * @param ParamVal	Address to store the pin parameter value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_GetPinParameter(const u32 PinId,
			const u32 ParamId,
			u32 * const ParamVal)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPin_GetPinConfig(PinId, ParamId, ParamVal);

	return Status;
}

/****************************************************************************/
/**
 * @brief Return the pin parameter value in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoGetPinParameter(XPlmi_Cmd * Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 PinId = Cmd->Payload[0];
	u32 ParamId = Cmd->Payload[1];
	Status = XPm_GetPinParameter(PinId, ParamId, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function performs driver-like IOCTL functions on shared system
 * devices.
 *
 * @param SubsystemId		ID of the subsystem
 * @param DeviceId		ID of the device
 * @param IoctlId		IOCTL function ID
 * @param Arg1			Argument 1
 * @param Arg2			Argument 2
 * @param Arg3			Argument 3
 * @param Response		Ioctl response
 * @param CmdType		IPI command request type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If no change to the pin parameter setting is required (the pin
 * parameter is already set up for this value), this call will be successful.
 * Otherwise, the request is denied unless the subsystem has already
 * requested this pin.
 *
 ****************************************************************************/
XStatus XPm_DevIoctl(const u32 SubsystemId, const u32 DeviceId,
			const pm_ioctl_id  IoctlId,
			const u32 Arg1,
			const u32 Arg2,
			const u32 Arg3,
			u32 *const Response,
			const u32 CmdType)
{
	XStatus Status = XST_FAILURE;

	Status = XPm_Ioctl(SubsystemId, DeviceId, IoctlId, Arg1, Arg2, Arg3,
			   Response, CmdType);
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}
/****************************************************************************/
/**
 * @brief  Command handler for the PM_IOCTL command. Decodes the IOCTL
 *         parameters from the command payload, resolves the indexed device
 *         ID and forwards the request to XPm_DevIoctl.
 *
 * @param  Cmd	Pointer to the PLM command structure carrying the payload
 *		and response buffer
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus XPm_DoDevIoctl(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 IndexedDeviceId = Cmd->Payload[0];
	u32 DeviceId;
	/*
	 * IoctlId is a wire-format command field (u32) decoded into the
	 * pm_ioctl_id enum. The explicit cast matches the other wire->enum
	 * decodes in xilpm_ng (XPm_PgoodSource, XPm_IsoCdoArgsFormat,
	 * XPm_NodeAccessTypes); the id is validated by XPm_Ioctl's switch.
	 * MISRA C:2025 Rule 10.5 (Advisory) deviation applies.
	 */
	pm_ioctl_id IoctlId = (pm_ioctl_id)Cmd->Payload[1];
	u32 Arg1 = Cmd->Payload[2];
	u32 Arg2 = Cmd->Payload[3];
	u32 Arg3 = Cmd->Payload[4];

	Status = XPm_ResolveIndexedNodeId(IndexedDeviceId, &DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_DevIoctl(SubsystemId, DeviceId, IoctlId, Arg1, Arg2, Arg3, &Cmd->Response[1], Cmd->IpiReqType);

done:
	Cmd->Response[0] = (u32)Status;
	return Status;

}

/****************************************************************************/
/**
 * @brief  This function sets the mode of PLL clock.
 *
 * @param SubsystemId	Subsystem ID
 * @param ClockId	ID of the clock node
 * @param Value		Pll mode value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_SetPllMode(const u32 SubsystemId, const u32 ClockId, const u32 Value)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode* Clock;

	if (!ISPLL(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check if subsystem is allowed to access requested pll or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Clock = (XPm_PllClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clock) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClockPll_SetMode(Clock, Value);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a PLL mode configuration command.
 */
/****************************************************************************/
static XStatus XPm_DoSetPllMode(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ClockId = Cmd->Payload[0];
	u32 Value = Cmd->Payload[1];
	Status = XPm_SetPllMode(SubsystemId, ClockId, Value);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the mode of PLL clock.
 *
 * @param ClockId	ID of the clock node
 * @param Value		Address to store mode value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetPllMode(const u32 ClockId, u32 *const Value)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode* Clock;

	if (!ISPLL(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Clock = (XPm_PllClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clock) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClockPll_GetMode(Clock, Value);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Return the PLL mode in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoGetPllMode(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 ClockId = Cmd->Payload[0];
	Status = XPm_GetPllMode(ClockId, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the parameter of PLL clock.
 *
 * @param SubsystemId	Subsystem ID
 * @param ClockId	ID of the clock node
 * @param ParmaId	ID of the parameter
 * @param Value		Value of the parameter
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_SetPllParameter(const u32 SubsystemId, const u32 ClockId, const u32 ParamId, const u32 Value)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode* Clock;

	if (!ISPLL(ClockId)) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	/* Check if subsystem is allowed to access requested clock or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Clock = (XPm_PllClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clock) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	Status = XPmClockPll_SetParam(Clock, ParamId, Value);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a PLL parameter configuration command.
 */
/****************************************************************************/
static XStatus XPm_DoSetPllParameter(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ClockId = Cmd->Payload[0];
	u32 ParamId = Cmd->Payload[1];
	u32 Value = Cmd->Payload[2];
	Status = XPm_SetPllParameter(SubsystemId, ClockId, ParamId, Value);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function reads the parameter of PLL clock.
 *
 * @param ClockId	ID of the clock node
 * @param ParmaId	ID of the parameter
 * @param Value		Address to store parameter value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetPllParameter(const u32 ClockId, const u32 ParamId, u32 *const Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_PllClockNode* Clock;

	if (!ISPLL(ClockId)) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	Clock = (XPm_PllClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clock) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	Status = XPmClockPll_GetParam(Clock, ParamId, Value);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief Return the PLL parameter value in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoGetPllParameter(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 ClockId = Cmd->Payload[0];
	u32 ParamId = Cmd->Payload[1];
	Status = XPm_GetPllParameter(ClockId, ParamId, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function queries information about the platform resources.
 *
 * @param Qid		The type of data to query
 * @param Arg1		Query argument 1
 * @param Arg2		Query argument 2
 * @param Arg3		Query argument 3
 * @param Output	Pointer to the output data
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
static XStatus XPm_Query(const u32 Qid, const u32 Arg1, const u32 Arg2,
		  const u32 Arg3, u32 *const Output)
{
	XStatus Status = XST_FAILURE;

	/* Warning Fix */
	(void) (Arg3);

	switch (Qid) {
	case (u32)XPM_QID_CLOCK_GET_NAME:
		Status = XPmClock_QueryName(Arg1,Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_TOPOLOGY:
		Status = XPmClock_QueryTopology(Arg1,Arg2,Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS:
		Status = XPmClock_QueryFFParams(Arg1,Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_MUXSOURCES:
		if (ISPLL(Arg1)) {
			Status = XPmClockPll_QueryMuxSources(Arg1,Arg2,Output);
		} else {
			Status = XPmClock_QueryMuxSources(Arg1,Arg2,Output);
		}
		break;
	case (u32)XPM_QID_CLOCK_GET_ATTRIBUTES:
		Status = XPmClock_QueryAttributes(Arg1,Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_NUM_CLOCKS:
		Status = XPmClock_GetNumClocks(Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_MAX_DIVISOR:
		Status = XPmClock_GetMaxDivisor(Arg1, Arg2, Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_NUM_PINS:
		Status = XPmPin_GetNumPins(Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTIONS:
		Status = XPmPinFunc_GetNumFuncs(Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS:
		Status = XPmPinFunc_GetNumFuncGroups(Arg1, Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_NAME:
		Status = XPmPinFunc_GetFuncName(Arg1, (char *)Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_GROUPS:
		Status = XPmPinFunc_GetFuncGroups(Arg1, Arg2, (u16 *)Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_PIN_GROUPS:
		Status = XPmPin_GetPinGroups(Arg1, Arg2, (u16 *)Output);
		break;
	case (u32)XPM_QID_PLD_GET_PARENT:
		Status = XPmPlDevice_GetParent(Arg1, Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_ATTRIBUTES:
		Status = XPmPin_QueryAttributes(Arg1, Output);
		break;
	case (u32)XPM_QID_GET_NODE_NAME:
		Status = XPm_GetNodeName(Arg1, Output);
		break;
	case (u32)XPM_QID_GET_NODE_COUNT:
		/* Arg1 specifies the node type: Clock, Reset, or Device */
		Status = XPm_GetNodeCount(Arg1, Output);
		break;
	case (u32)XPM_QID_GET_VID_POWER_STATE:
		/* Arg1 is the rail node id. Reply is the rail mode. */
		Status = XPmRail_GetVidPowerState(Arg1, Output);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a platform data query and return its output from Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoQueryData(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 Qid = Cmd->Payload[0];
	u32 Arg1 = Cmd->Payload[1];
	u32 Arg2 = Cmd->Payload[2];
	u32 Arg3 = Cmd->Payload[3];
	Status = XPm_Query(Qid, Arg1, Arg2, Arg3, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function enables or disables the clock.
 *
 * @param SubsystemId	Subsystem ID
 * @param ClockId	ID of the clock node
 * @param Enable	Enable (1) or disable (0)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   To enable a clock, the subsystem must be using the clock.  To
 * disable a clock, the subsystem must be the only user of the clock, and the
 * clock must not have any downstream clock(s) that are enabled.  Otherwise,
 * this request will be denied.
 *
 ****************************************************************************/
static XStatus XPm_SetClockState(const u32 SubsystemId, const u32 ClockId, const u32 Enable)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);
	u32 CurrState = 0U;

	if (NULL == Clk) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	/* HACK: Don't disable PLL clocks for now */
	if((Enable == 0U) && (ISPLL(ClockId)))
	{
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check if clock's state is already desired state */
	Status = XPm_GetClockState(ClockId, &CurrState);
	if ((XST_SUCCESS == Status) && (CurrState == Enable)) {
		goto done;
	}

	if (1U == Enable) {
		if (ISPLL(ClockId)) {
			/* HACK: Allow enabling of PLLs for now */
			goto bypass;
		} else if (ISOUTCLK(ClockId) &&
			   (0U != (Clk->Flags & CLK_FLAG_READ_ONLY))) {
			/* Allow enable operation for read-only clocks */
			goto bypass;
		} else {
			/* Required due to MISRA */
		}
	}

	/* Check if subsystem is allowed to access requested clock or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		goto done;
	}

bypass:
	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_SetGate((XPm_OutClockNode *)Clk, Enable);
	} else if (ISPLL(ClockId)) {
		u32 Mode;
		if (1U == Enable) {
			Mode = ((XPm_PllClockNode *)Clk)->PllMode;
		} else if (0U == Enable) {
			Mode = (u32)PM_PLL_MODE_RESET;
		} else {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Status = XPmClockPll_SetMode((XPm_PllClockNode *)Clk, Mode);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief Handle a clock-enable command.
 */
/****************************************************************************/
static XStatus XPm_DoClockEnable(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ClockId = Cmd->Payload[0];
	Status = XPm_SetClockState(SubsystemId, ClockId, 1);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief Handle a clock-disable command.
 *
 * @param Cmd Pointer to the command containing clock and subsystem IDs and the
 *            response buffer.
 *
 * @return XST_SUCCESS on success, otherwise an access, validation, or clock
 *         operation error. PLL disable requests currently return success
 *         without changing the PLL. The status is also in the response.
 *
 ****************************************************************************/
static XStatus XPm_DoClockDisable(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ClockId = Cmd->Payload[0];
	Status = XPm_SetClockState(SubsystemId, ClockId, 0);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the clock state.
 *
 * @param ClockId	ID of the clock node
 * @param Enable		Pointer to the clock state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetClockState(const u32 ClockId, u32 *const Enable)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk,
						(u32)TYPE_GATE, Enable);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_GetMode((XPm_PllClockNode *)Clk, Enable);
		if (*Enable == (u32)PM_PLL_MODE_RESET) {
			*Enable = 0;
		} else {
			*Enable = 1;
		}
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Command handler for the PM_CLOCK_GETSTATE command. Reads the clock
 *         ID from the command payload and returns the clock state in the
 *         command response buffer.
 *
 * @param  Cmd	Pointer to the PLM command structure carrying the payload
 *		and response buffer
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus XPm_DoGetClockState(XPlmi_Cmd*Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 ClockId = Cmd->Payload[0];
	Status = XPm_GetClockState(ClockId, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the divider value of the clock.
 *
 * @param SubsystemId	Subsystem ID.
 * @param ClockId	Clock node ID
 * @param Divider	Divider value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   To change the clock divider, the clock must be disabled.  Otherwise,
 * this request will be denied.
 *
 ****************************************************************************/
static XStatus XPm_SetClockDivider(const u32 SubsystemId, const u32 ClockId, const u32 Divider)
{
	volatile XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	if (NULL == Clk) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	/* Check if subsystem is allowed to access requested clock or not */
	volatile XStatus StatusTmp = XST_FAILURE;
	XSECURE_REDUNDANT_CALL(Status, StatusTmp, XPm_IsAccessAllowed, SubsystemId, ClockId);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_SetDivider((XPm_OutClockNode *)Clk, Divider);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_SetParam((XPm_PllClockNode *)Clk,
					      (u32)PM_PLL_PARAM_ID_FBDIV, Divider);
	} else {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  Command handler for the PM_CLOCK_SETDIVIDER command. Reads the
 *         clock ID and divider value from the command payload and sets the
 *         clock divider.
 *
 * @param  Cmd	Pointer to the PLM command structure carrying the payload
 *		and response buffer
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus XPm_DoSetClockDivider(XPlmi_Cmd *Cmd)
{
	volatile XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ClockId = Cmd->Payload[0];
	u32 Divider = Cmd->Payload[1];
	Status = XPm_SetClockDivider(SubsystemId, ClockId, Divider);
	Cmd->Response[0] = (u32)Status;
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function reads the clock divider.
 *
 * @param ClockId	ID of the clock node
 * @param Divider	Address to store the divider values
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_GetClockDivider(const u32 ClockId, u32 *const Divider)
{
	XStatus Status = XST_FAILURE;
	const XPm_ClockNode *Clk = NULL;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_GetClockData((const XPm_OutClockNode *)Clk,
						(u32)TYPE_DIV1, Divider);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_GetParam((const XPm_PllClockNode *)Clk,
					      (u32)PM_PLL_PARAM_ID_FBDIV, Divider);
	} else {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief Return the clock divider in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoGetClockDivider(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 ClockId = Cmd->Payload[0];
	Status = XPm_GetClockDivider(ClockId, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function sets the parent of the clock.
 *
 * @param SubsystemId	Subsystem ID.
 * @param ClockId	Clock node ID
 * @param ParentIdx	Parent clock index
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   To change the clock parent, the clock must be disabled.  Otherwise,
 * this request will be denied.
 *
 ****************************************************************************/
static XStatus XPm_SetClockParent(const u32 SubsystemId, const u32 ClockId, const u32 ParentIdx)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	/* Check if subsystem is allowed to access requested clock or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Set parent is allowed only on output clocks */
	if (!ISOUTCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_SetParent((XPm_OutClockNode *)Clk, ParentIdx);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Command handler for the PM_CLOCK_SETPARENT command. Reads the
 *         clock ID and parent index from the command payload and sets the
 *         clock parent.
 *
 * @param  Cmd	Pointer to the PLM command structure carrying the payload
 *		and response buffer
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus XPm_DoSetClockParent(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ClockId = Cmd->Payload[0];
	u32 ParentIdx = Cmd->Payload[1];
	Status = XPm_SetClockParent(SubsystemId, ClockId, ParentIdx);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the clock parent.
 *
 * @param ClockId	ID of the clock node
 * @param ParentIdx	Address to store the parent clock index
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_GetClockParent(const u32 ClockId, u32 *const ParentIdx)
{
	XStatus Status = XST_FAILURE;
	const XPm_ClockNode *Clk = NULL;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	/* Get parent is allowed only on output clocks */
	if (!ISOUTCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_GetClockData((const XPm_OutClockNode *)Clk, (u32)TYPE_MUX, ParentIdx);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
/****************************************************************************/
/**
 * @brief Return the clock parent index in Response[1].
 */
/****************************************************************************/
static XStatus XPm_DoGetClockParent(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 ClockId = Cmd->Payload[0];
	Status = XPm_GetClockParent(ClockId, &Cmd->Response[1]);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
/****************************************************************************/
/**
 * @brief  Request the usage of a device. A subsystem requests access to a
 * device and asserts its requirements on that device. The platform
 * management controller will enable access to the memory mapped region
 * containing the control registers of that device. For devices that can only
 * be used by one subsystem, any other subsystems will now be blocked from
 * accessing this device until it is released.
 *
 * @param SubsystemId	Target subsystem ID (can be the same subsystem)
 * @param Device		ID of the device
 * @param Capabilities		Capability requirements (1-hot)
 * @param QoS			Quality of Service (0-100) required
 * @param Ack			Ack request
 * @param CmdType		IPI command request type
 * @param Addr		    HandoffAddr, ClusterNum and LockstepVal
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_RequestDevice(const u32 SubsystemId, const u32 DeviceId,
			const u32 Capabilities, const u32 QoS, const u32 Ack,const u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	/* Warning Fix */
	(void) (Ack);

	Status = XPmDevice_Request(SubsystemId, DeviceId, Capabilities,
				   QoS, CmdType);

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function adds node entry to the access table
 *
 * @param Args		node specific arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
static XStatus XPm_DoSetNodeAccess(XPlmi_Cmd * Cmd)
{
	XStatus Status = XST_FAILURE;
	const u32 *Args = Cmd->Payload;
	u32 NumArgs = Cmd->Len;

	Status = XPmAccess_UpdateTable(Args, NumArgs);
	Cmd->Response[0] = (u32)Status;

	return Status;
}

#ifdef XPLMI_EM_LINK_NODES_SUPPORT
/*
 * GET_EVENT_STATUS_REGS staging: request/reply buffers live in DDR/PMC RAM and are
 * referenced by address in the IPI. We DMA them into local staging areas so
 * all packing/bounds checks operate on trusted PLM memory. Sized for versal_2ve_2vm
 * (~1.1 KB reply for 5 controllers); tune via build flags.
 *
 * The whole PM_GET_EVENT_STATUS_REGS handler is gated on XPLMI_EM_LINK_NODES_SUPPORT:
 * it depends on the error->node linkage backend (XPlmi_GetLinkedNodes, via
 * XPmAccess_GetEventStatusRegs) that only the platforms defining this macro provide.
 * Forks without it (e.g. versal_gen2_tr) compile the feature out entirely -
 * the command is neither registered (below) nor advertised (xpm_feature_check.c).
 */
#ifndef XPM_EVENT_STATUS_REQ_MAX_WORDS
/** Request staging buffer capacity, in 32-bit words */
#define XPM_EVENT_STATUS_REQ_MAX_WORDS		(64U)
#endif
/* XPM_EVENT_STATUS_REPLY_MAX_WORDS (xpm_access.h) sizes the reply staging
 * buffer below. The deferred-clear store is bounded separately (xpm_access.c). */

/** Fixed request-header words preceding the optional arg vector
 *  (word0=ReqSize, EventID, Mask, Flags) */
#define XPM_EVENT_STATUS_REQ_HDR_WORDS		(4U)

/****************************************************************************/
/**
 * @brief  EEMI handler for PM_GET_EVENT_STATUS_REGS (0x4F). Reads the raw register
 *         snapshot of every node linked to {EventID, Mask} into the caller's
 *         reply buffer for DDR EDAC source tracing.
 *
 * @param  Cmd  Command frame. Args (Payload):
 *              [0]=ReqBufAddrHi [1]=ReqBufAddrLo
 *              [2]=ReplyBufAddrHi [3]=ReplyBufAddrLo.
 *              Sizes live in word0 of each buffer.
 *
 * @return XST_SUCCESS if successful else an error code. Response:
 *         [0]=Status (snapshot delivery), [1]=RequestSizeConsumed,
 *         [2]=ReplySizeUsed, [3]=ClearStatus (deferred ISR-clear outcome;
 *         separate from [0] so a post-delivery clear failure never discards a
 *         valid snapshot).
 *
 ****************************************************************************/
static XStatus XPm_DoGetEventStatusRegs(XPlmi_Cmd * Cmd)
{
	volatile XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 CmdType = Cmd->IpiReqType;
	u64 ReqAddr;
	u64 ReplyAddr;
	static u32 ReqBuf[XPM_EVENT_STATUS_REQ_MAX_WORDS] = {0};
	static u32 ReplyBuf[XPM_EVENT_STATUS_REPLY_MAX_WORDS] = {0};
	u32 ReqSize = 0U;
	u32 ReqWords;
	u32 ReplyCap;
	u32 EventId;
	u32 Mask;
	u32 Flags;
	u32 ReqArgCnt = 0U;
	const u32 *ReqArgs = NULL;
	u32 ReplyUsed = 0U;
	u32 ReqConsumed = 0U;
	XStatus ClearStatus = XST_SUCCESS;	/* deferred-clear outcome (Response[3]) */

	/*
	 * The 4 address words are always present in the fixed IPI buffer. Do NOT
	 * gate on Cmd->Len here: EEMI PM calls arriving via TF-A carry no length in
	 * the command header (TF-A's PM_PACK_PAYLOAD sets only api/module/flag, so
	 * PLM computes Cmd->Len == 0 for every SMC/IPI PM command). Every other PM
	 * handler likewise reads Cmd->Payload[] directly. The caller-supplied
	 * addresses below are range-validated before any access.
	 */
	ReqAddr = ((u64)Cmd->Payload[0] << 32U) | (u64)Cmd->Payload[1];
	ReplyAddr = ((u64)Cmd->Payload[2] << 32U) | (u64)Cmd->Payload[3];

	/*
	 * Validate word0 (4 bytes) of each buffer BEFORE reading it so a
	 * validation-enabled build rejects a bad/disallowed address before PLM
	 * touches it (validate-before-touch). The full spans are validated below
	 * once the sizes are known. On default builds the macro is a no-op (see the
	 * platform note at the full-span validation).
	 */
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ReqAddr, (u64)sizeof(u32), Status,
				XPLMI_ERR_INVALID_ADDR_RANGE, done);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ReplyAddr, (u64)sizeof(u32), Status,
				XPLMI_ERR_INVALID_ADDR_RANGE, done);

	/* Read word0 of each buffer to learn the request/reply sizes. */
	Status = XPlmi_MemCpy64((u64)(UINTPTR)&ReqSize, ReqAddr, sizeof(u32));
	if (XST_SUCCESS != Status) {
		goto done;
	}
	Status = XPlmi_MemCpy64((u64)(UINTPTR)&ReplyCap, ReplyAddr, sizeof(u32));
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if ((ReqSize < (XPM_EVENT_STATUS_REQ_HDR_WORDS * (u32)sizeof(u32))) ||
	    (0U != (ReqSize & (u32)(sizeof(u32) - 1U))) ||
	    (ReqSize > (XPM_EVENT_STATUS_REQ_MAX_WORDS * (u32)sizeof(u32)))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	ReqWords = ReqSize / (u32)sizeof(u32);

	if ((0U != (ReplyCap & (u32)(sizeof(u32) - 1U))) ||
	    (ReplyCap < (u32)sizeof(u32))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * The PLM staging buffer (XPM_EVENT_STATUS_REPLY_MAX_WORDS) is the maximum
	 * reply the PLM can produce in one call. A caller ReplyCap larger than that
	 * is NOT an error: clamp the working capacity to the staging cap and let the
	 * overflow path return XST_BUFFER_TOO_SMALL + required size so the caller can
	 * resize and retry. REPLY_MAX_WORDS must stay >= the true worst-case reply,
	 * else a resize-and-retry loop cannot converge (always clamped, always over).
	 */
	if (ReplyCap > (XPM_EVENT_STATUS_REPLY_MAX_WORDS * (u32)sizeof(u32))) {
		ReplyCap = XPM_EVENT_STATUS_REPLY_MAX_WORDS * (u32)sizeof(u32);
	}

	/*
	 * Validate the FULL request/reply spans (once each) via the standard
	 * XPLMI_VERIFY_ADDR_RANGE macro, then DMA the request into staging. The
	 * macro is gated by PLM_ENABLE_ADDR_RANGE_VALIDATION and compiles to a no-op
	 * on builds where it is not set; on such builds the caller
	 * address is not range-checked here and hardware isolation (XMPU/XPPU) is
	 * the boundary, as with every other PLM IPI handler.
	 */
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ReqAddr, (u64)ReqSize, Status,
				XPLMI_ERR_INVALID_ADDR_RANGE, done);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ReplyAddr, (u64)ReplyCap, Status,
				XPLMI_ERR_INVALID_ADDR_RANGE, done);

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&ReqBuf[0], ReqAddr, ReqSize);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * Unpack the fixed request header (word0 = size, already consumed) and
	 * expose any trailing words as the optional per-event argument vector.
	 */
	EventId = ReqBuf[1];
	Mask = ReqBuf[2];
	Flags = ReqBuf[3];
	if (ReqWords > XPM_EVENT_STATUS_REQ_HDR_WORDS) {
		ReqArgs = &ReqBuf[XPM_EVENT_STATUS_REQ_HDR_WORDS];
		ReqArgCnt = ReqWords - XPM_EVENT_STATUS_REQ_HDR_WORDS;
	}
	/* Whole request buffer was read/consumed */
	ReqConsumed = ReqSize;

	/*
	 * Permission gate: the caller must have a live notifier for {EventId,Mask}.
	 * Per-aperture ACCESS_* is additionally enforced inside XPmAccess_ReadReg.
	 * Redundant check to harden the gate against glitch.
	 */
	{
		/* Two independent reads of the gate for glitch redundancy. */
		volatile XStatus GateChk = XPmNotifier_IsRegistered(SubsystemId,
								    EventId, Mask);
		volatile XStatus GateChkTmp = XPmNotifier_IsRegistered(SubsystemId,
								       EventId, Mask);
		/*
		 * Snapshot each volatile into a plain local on its own statement so
		 * the compound condition has no side effect (MISRA 13.5); the two
		 * reads above are preserved so the redundancy still holds.
		 */
		XStatus GateChkVal = GateChk;
		XStatus GateChkTmpVal = GateChkTmp;
		if ((XST_SUCCESS != GateChkVal) || (XST_SUCCESS != GateChkTmpVal)) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	}

	/* Seed reply word0 (capacity) into the staging area for the core */
	ReplyBuf[0] = ReplyCap;

	Status = XPmAccess_GetEventStatusRegs(SubsystemId, EventId, Mask, Flags,
					      ReqArgs, ReqArgCnt, ReplyBuf,
					      &ReplyUsed, CmdType);
	/*
	 * Snapshot the volatile Status once before testing it twice so the
	 * compound condition has no side effect (MISRA 13.5). This is a plain
	 * control-flow check, so a single read is correct. XST_BUFFER_TOO_SMALL
	 * is not a hard failure: it carries the required size back to the caller.
	 */
	XStatus StatusChk = Status;
	if ((XST_SUCCESS != StatusChk) && (XST_BUFFER_TOO_SMALL != StatusChk)) {
		goto done;
	}

	/*
	 * On success, DMA out only the used span (also mirror word0 = used size).
	 * On XST_BUFFER_TOO_SMALL, ReplyUsed carries the *required* size so the
	 * caller can resize and retry; nothing is written to the reply payload.
	 */
	if (XST_SUCCESS == Status) {
		u32 OutBytes = ReplyUsed;
		if (OutBytes > ReplyCap) {
			OutBytes = ReplyCap;
		}
		ReplyBuf[0] = ReplyUsed;
		Status = XPlmi_MemCpy64(ReplyAddr, (u64)(UINTPTR)&ReplyBuf[0],
					OutBytes);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/*
		 * The snapshot is now safely delivered to the caller. ONLY now
		 * apply the deferred ISR clears (recorded, write-permission
		 * verified, but NOT performed by XPmAccess_GetEventStatusRegs) so a
		 * clear-write failure can never destroy the reply the caller
		 * already received.
		 *
		 * Contract: snapshot delivery and ISR clear are
		 * SEPARATE outcomes. A post-DMA clear failure must NOT fail the
		 * whole command - that would make a strict caller discard a valid
		 * snapshot it already received. Keep Status (Response[0]) at
		 * XST_SUCCESS and report the clear outcome in Response[3]. The
		 * up-front per-aperture write-permission checks guarantee no clear
		 * is skipped for a PERMISSION reason, so no permission failure can
		 * leave a partial ISR; a mid-batch HW write failure can still
		 * leave a partially-cleared ISR, which the caller detects via
		 * Response[3] and may re-issue.
		 */
		ClearStatus = XPmAccess_ApplyEventClears(SubsystemId, CmdType);
	}

done:
	/*
	 * Report snapshot delivery and deferred-clear outcomes independently:
	 * [0]=Status, [1]=request bytes consumed, [2]=reply bytes used (or the
	 * required size on XST_BUFFER_TOO_SMALL), [3]=deferred ISR-clear outcome.
	 */
	Cmd->Response[0] = (u32)Status;
	Cmd->Response[1] = ReqConsumed;
	Cmd->Response[2] = ReplyUsed;
	Cmd->Response[3] = (u32)ClearStatus;
	return Status;
}
#endif /* XPLMI_EM_LINK_NODES_SUPPORT */

/****************************************************************************/
/**
 * @brief  Hook called after boot PDI loading is complete. On QEMU/COSIM
 *         platforms, creates the AIE device node if not already present.
 *         Initialises pin runtime ops and calculates clock rates.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 *
 * @note   This is the strong (runtime) override of the weak boot version
 *
 ****************************************************************************/
XStatus XPm_HookAfterBootPdi(void)
{
	XStatus Status = XST_FAILURE;
	u32 Platform = XPm_GetPlatform();

	/* TODO: Review where interrupts need to be enabled */
	/* Enable power related interrupts to PMC */

	XPm_RMW32(PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN, PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN_MASK,
		  PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN_MASK);

	if ((PLATFORM_VERSION_QEMU == Platform) ||
	    (PLATFORM_VERSION_COSIM == Platform)) {
		Status = XPm_CosimAddAieDeviceNode();
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	if (0U != XPlmi_IsPlmUpdateDone()){
		/* Skip during PLM update */
		Status = XST_SUCCESS;
		goto done;
	}

	/* Init Pin RuntimeOps */
	for (u32 i = 0; i < (u32)XPM_NODEIDX_STMIC_MAX; i++) {
		XPm_PinNode* Pin = XPmPin_GetByIndex(i);
		if (NULL == Pin) {
			continue;
		}
		if (XST_SUCCESS != XPmPin_RuntimeOps_Init(Pin)) {
			PmWarn("Pin RuntimeOps init failed for pin index %u\r\n", i);
		}
	}

	/* Calculate rate for all clocks */
	for (u32 i = 1U; i < (u32)XPM_NODEIDX_CLK_MAX; i++) {
		XPm_ClockNode *Clk = XPmClock_GetByIdx(i);
		if (NULL != Clk) {
			(void)XPmClock_CalculateRate(Clk);
		}
	}

	/* Register GPIO interrupt handlers for all configured RTCA registers */
	Status = XPm_GpioProcHandlerInit();
	if (XST_SUCCESS != Status) {
		PmErr("Failed to initialize GPIO handlers, Status: 0x%x\r\n",
			   Status);
	}

done:
	return Status;
}

/**
 * @brief Restore EVENT notifier EM actions after in-place PLM update
 *        by calling XPmNotifier_RestoreEmActions().
 *
 * @return XST_SUCCESS on success; otherwise the first error code
 *         returned by XPlmi_EmSetAction().
 */
XStatus XPmUpdate_RestoreNotifierEmActions(void)
{
	return XPmNotifier_RestoreEmActions();
}
