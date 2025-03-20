/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xplmi_cmd.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_requirement.h"
#include "xpm_device.h"
#include "xpm_feature_check.h"
#include "xpm_periph.h"
#include "xpm_gic_proxy.h"
#include "xsysmonpsv.h"
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
#include "xpm_runtime_pin.h"
#include "xpm_runtime_pll.h"
#include "xpm_runtime_core.h"
#include "xpm_runtime_power.h"
#include "xpm_runtime_periph.h"
#include "xpm_access.h"
#include "xpm_power_handlers.h"

#define XPm_RegisterWakeUpHandler(GicId, SrcId, NodeId)	\
	{ \
		Status = XPlmi_GicRegisterHandler(GicId, SrcId, \
				XPm_DispatchWakeHandler, (void *)(NodeId)); \
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
static XStatus XPm_DoRegisterNotifier(XPlmi_Cmd* Cmd);
static XStatus XPm_DoGetOpCharacteristic(XPlmi_Cmd* Cmd);
static XStatus XPm_DoSelfSuspend(XPlmi_Cmd* Cmd);
static XStatus XPm_DoForcePowerdown(XPlmi_Cmd* Cmd);

static XStatus XPm_GetLatency(const u32 DeviceId, u32 *Latency);
static XStatus XPm_GetTemperature(u32 const DeviceId, u32 *Result);

static XStatus XPm_GetOpCharacteristic(u32 const DeviceId, u32 const Type, u32 *Result);
static XStatus XPm_RegisterNotifier(const u32 SubsystemId, const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 Enable,
			 const u32 IpiMask);
static XStatus XPm_SetWakeUpSource(const u32 SubsystemId, const u32 TargetNodeId,
			    const u32 SourceNodeId, const u32 Enable);
static void XPm_CoreIdle(XPm_Core *Core);
static XStatus XPm_DoAbortSuspend(XPlmi_Cmd* Cmd);

static XStatus XPm_RequestHBMonDevice(const u32 SubsystemId, const u32 CmdType);
static XStatus XPm_DoWakeUp(XPlmi_Cmd* Cmd);
static XStatus XPm_SubsystemPwrUp(const u32 SubsystemId);
static XStatus SetSubsystemState_ByCore(XPm_Core* Core, const u32 State);
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

/* Defined in xilpm-boot */
extern XPm_Device *PmDevices[(u32)XPM_NODEIDX_DEV_MAX];
extern int (*PmRestartCb)(u32 ImageId, u32 *FuncId);

/**
 * @brief Initializes the Xilinx Power Management (XPM) runtime.
 *
 * This function initializes the XPM runtime and performs any necessary setup
 * for power management operations. It should be called before any other XPM
 * runtime functions are used.
 *
 * @return XST_SUCCESS if initialization is successful, otherwise an error code.
 */
XStatus XPm_RuntimeInit(void)
{
	XStatus Status = XST_FAILURE;
	const char *Banner = XILPM_RUNTIME_BANNER_STRING;
	PmInfo("Initializing XilPm Runtime Library [%s]\r\n", Banner);

	/* Store multiboot register value*/
	XPlmi_StoreMultiboot();

	/* Initializing XPLmi_PmCmds array*/
	XPlmi_ModuleCmd* XPlmi_PmCmds =  XPm_GetPmCmds();
	XPlmi_PmCmds[PM_GET_API_VERSION].Handler = (XPlmi_CmdHandler)XPm_DoGetApiVersion;
	XPlmi_PmCmds[PM_GET_NODE_STATUS].Handler = (XPlmi_CmdHandler)XPm_DoGetNodeStatus;
	XPlmi_PmCmds[PM_GET_OP_CHARACTERISTIC].Handler = (XPlmi_CmdHandler)XPm_DoGetOpCharacteristic;
	XPlmi_PmCmds[PM_REGISTER_NOTIFIER].Handler = (XPlmi_CmdHandler)XPm_DoRegisterNotifier;
	XPlmi_PmCmds[PM_SELF_SUSPEND].Handler = (XPlmi_CmdHandler)XPm_DoSelfSuspend;
	XPlmi_PmCmds[PM_FORCE_POWERDOWN].Handler = (XPlmi_CmdHandler)XPm_DoForcePowerdown;
	XPlmi_PmCmds[PM_ABORT_SUSPEND].Handler = (XPlmi_CmdHandler)XPm_DoAbortSuspend;
	XPlmi_PmCmds[PM_REQUEST_WAKEUP].Handler = (XPlmi_CmdHandler)XPm_DoWakeUp;
	XPlmi_PmCmds[PM_SET_WAKEUP_SOURCE].Handler = (XPlmi_CmdHandler)XPm_DoSetWakeUpSource;
	XPlmi_PmCmds[PM_SYSTEM_SHUTDOWN].Handler = (XPlmi_CmdHandler)XPm_DoSystemShutdown;
	XPlmi_PmCmds[PM_REQUEST_NODE].Handler = (XPlmi_CmdHandler)XPm_DoRequestDevice;
	XPlmi_PmCmds[PM_RELEASE_NODE].Handler = (XPlmi_CmdHandler)XPm_DoReleaseDevice;
	XPlmi_PmCmds[PM_SET_REQUIREMENT].Handler = (XPlmi_CmdHandler)XPm_DoSetRequirement;
	XPlmi_PmCmds[PM_SET_MAX_LATENCY].Handler = (XPlmi_CmdHandler)XPm_DoSetMaxLatency;
	XPlmi_PmCmds[PM_RESET_ASSERT].Handler = (XPlmi_CmdHandler)XPm_DoResetAssert;
	XPlmi_PmCmds[PM_RESET_GET_STATUS].Handler = (XPlmi_CmdHandler)XPm_DoGetResetState;
	XPlmi_PmCmds[PM_INIT_FINALIZE].Handler = (XPlmi_CmdHandler)XPm_DoInitFinalize;
	XPlmi_PmCmds[PM_GET_CHIPID].Handler = (XPlmi_CmdHandler)XPm_DoGetChipID;
	XPlmi_PmCmds[PM_PINCTRL_REQUEST].Handler = (XPlmi_CmdHandler)XPm_DoPinCtrlRequest;
	XPlmi_PmCmds[PM_PINCTRL_RELEASE].Handler = (XPlmi_CmdHandler)XPm_DoPinCtrlRelease;
	XPlmi_PmCmds[PM_PINCTRL_GET_FUNCTION].Handler = (XPlmi_CmdHandler)XPm_DoGetPinFunction;
	XPlmi_PmCmds[PM_PINCTRL_SET_FUNCTION].Handler = (XPlmi_CmdHandler)XPm_DoSetPinFunction;
	XPlmi_PmCmds[PM_PINCTRL_CONFIG_PARAM_GET].Handler = (XPlmi_CmdHandler)XPm_DoGetPinParameter;
	XPlmi_PmCmds[PM_PINCTRL_CONFIG_PARAM_SET].Handler = (XPlmi_CmdHandler)XPm_DoSetPinParameter;
	XPlmi_PmCmds[PM_PLL_SET_MODE].Handler = (XPlmi_CmdHandler)XPm_DoSetPllMode;
	XPlmi_PmCmds[PM_PLL_GET_MODE].Handler = (XPlmi_CmdHandler)XPm_DoGetPllMode;
	XPlmi_PmCmds[PM_PLL_SET_PARAMETER].Handler = (XPlmi_CmdHandler)XPm_DoSetPllParameter;
	XPlmi_PmCmds[PM_PLL_GET_PARAMETER].Handler = (XPlmi_CmdHandler)XPm_DoGetPllParameter;
	XPlmi_PmCmds[PM_IOCTL].Handler = (XPlmi_CmdHandler)XPm_DoDevIoctl;
	XPlmi_PmCmds[PM_QUERY_DATA].Handler = (XPlmi_CmdHandler)XPm_DoQueryData;
	XPlmi_PmCmds[PM_CLOCK_ENABLE].Handler = (XPlmi_CmdHandler)XPm_DoClockEnable;
	XPlmi_PmCmds[PM_CLOCK_DISABLE].Handler = (XPlmi_CmdHandler)XPm_DoClockDisable;
	XPlmi_PmCmds[PM_CLOCK_SETDIVIDER].Handler = (XPlmi_CmdHandler)XPm_DoSetClockDivider;
	XPlmi_PmCmds[PM_CLOCK_GETDIVIDER].Handler = (XPlmi_CmdHandler)XPm_DoGetClockDivider;
	XPlmi_PmCmds[PM_CLOCK_SETPARENT].Handler = (XPlmi_CmdHandler)XPm_DoSetClockParent;
	XPlmi_PmCmds[PM_CLOCK_GETPARENT].Handler = (XPlmi_CmdHandler)XPm_DoGetClockParent;
	XPlmi_PmCmds[PM_CLOCK_GETSTATE].Handler = (XPlmi_CmdHandler)XPm_DoGetClockState;
	XPlmi_PmCmds[PM_ADD_SUBSYSTEM].Handler = (XPlmi_CmdHandler)XPm_DoAddSubsystem;
	XPlmi_PmCmds[PM_ADD_REQUIREMENT].Handler = (XPlmi_CmdHandler)XPm_DoAddRequirement;
	XPlmi_PmCmds[PM_ACTIVATE_SUBSYSTEM].Handler = (XPlmi_CmdHandler)XPm_DoActivateSubsystem;
	XPlmi_PmCmds[PM_FEATURE_CHECK].Handler = (XPlmi_CmdHandler)XPm_DoFeatureCheck;
	XPlmi_PmCmds[PM_SET_NODE_ACCESS].Handler = (XPlmi_CmdHandler)XPm_DoSetNodeAccess;
	Status = XPmSubsystem_ModuleInit();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_RegisterWakeUpHandlers();

done:
	return Status;
}

XStatus XPm_GicProxyWakeUp(const u32 PeriphIdx)
{
	XStatus Status = XST_FAILURE;

	const XPmRuntime_Periph *Periph = (XPmRuntime_Periph *)XPmDevice_GetByIndex(PeriphIdx);
	if ((NULL == Periph) || (0U == Periph->WakeProcId)) {
		goto done;
	}

	XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(Periph->WakeProcId);

	/* Do not process anything if core is already running */
	if ((u8)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	Status = Core->CoreOps->RequestWakeup(Core, 0, 0);

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
int XPm_RegisterWakeUpHandlers(void)
{
	int Status = XST_FAILURE;

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

END:
	return Status;
}

static XStatus XPm_DoGetOpCharacteristic(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId = Cmd->Payload[0];
	u32 Type = Cmd->Payload[1];
	u32 *Result = &Cmd->Response[1];
	Status = XPm_GetOpCharacteristic(DeviceId, Type, Result);
	Cmd->Response[0] = (u32)Status;
	return Status;

}


static XStatus XPm_GetOpCharacteristic(u32 const DeviceId, u32 const Type, u32 *Result)
{
	XStatus Status = XST_FAILURE;

	switch(Type) {
	case (u32)PM_OPCHAR_TYPE_TEMP:
		Status = XPm_GetTemperature(DeviceId, Result);
		break;
	case (u32)PM_OPCHAR_TYPE_LATENCY:
		Status = XPm_GetLatency(DeviceId, Result);
		break;
	case (u32)PM_OPCHAR_TYPE_POWER:
		Status = XST_NO_FEATURE;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}
static XStatus XPm_GetTemperature(u32 const DeviceId, u32 *Result)
{
	XStatus Status = XST_FAILURE;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		goto done;
	}

	/*
	 * TODO - need to implement getting temperature, beside the
	 * temperature of entire SoC.
	 */
	if ((u32)XPM_NODETYPE_DEV_SOC != NODETYPE(DeviceId)) {
		Status = XST_NO_FEATURE;
		goto done;
	}

	*Result = XSysMonPsv_ReadDeviceTemp(SysMonInstPtr,
					    XSYSMONPSV_VAL_VREF_MAX);
	Status = XST_SUCCESS;

done:
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
static XStatus XPm_GetLatency(const u32 DeviceId, u32 *Latency)
{
	XStatus Status = XST_SUCCESS;

	switch (NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		if ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(DeviceId)) {
			Status = XPmCore_GetWakeupLatency(DeviceId, Latency);
		} else {
			Status = XPmDevice_GetWakeupLatency(DeviceId, Latency);
		}
		break;
	case (u32)XPM_NODECLASS_POWER:
		Status = XPmPower_GetWakeupLatency(DeviceId, Latency);
		break;
	case (u32)XPM_NODECLASS_CLOCK:
		if ((u32)XPM_NODESUBCL_CLOCK_PLL == NODESUBCLASS(DeviceId)) {
			Status = XPmClockPll_GetWakeupLatency(DeviceId, Latency);
		} else {
			Status = XST_INVALID_PARAM;
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

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

static XStatus XPm_DoFeatureCheck(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 ApiId = Cmd->Payload[0];
	u32 *Version = &Cmd->Response[1];
	Status = XPm_FeatureCheck(ApiId, Version);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
static XStatus XPm_DoGetApiVersion(XPlmi_Cmd* Cmd)
{
	PmInfo("XPm_DoGetApiVersion\n\r");
	Cmd->Response[1] = PM_VERSION;
	Cmd->Response[0] = XST_SUCCESS;
	return XST_SUCCESS;
}

static XStatus XPm_DoGetNodeStatus(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 NodeId = Cmd->Payload[0];
	/* TODO: FIXME fix casting bellow */
	Status = XPm_GetDeviceStatus(SubsystemId, NodeId, (XPm_DeviceStatus*)(&Cmd->Response[1]));
	Cmd->Response[0] = (u32)Status;
	return Status;
}
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
	XPmRuntime_DeviceOps *DevOps = XPm_GetDevOps_ById(SourceNodeId);
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
	XPmGicProxy_WakeEventSet(Periph, (u8)Enable);


done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}


static XStatus XPm_DoSetRequirement(XPlmi_Cmd* Cmd){
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 DeviceId = Cmd->Payload[0];
	u32 Capabilities = Cmd->Payload[1];
	u32 QoS = Cmd->Payload[2];
	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}
	Status = XPmDevice_SetRequirement(SubsystemId, DeviceId, Capabilities, QoS);
done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	Cmd->Response[0] = (u32)Status;
	return Status;
}

static XStatus XPm_DoRequestDevice(XPlmi_Cmd* Cmd) {
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 DeviceId = Cmd->Payload[0];
	u32 Capabilities = Cmd->Payload[1];
	u32 QoS = Cmd->Payload[2];
	u32 CmdType = Cmd->IpiReqType;

	Status = XPmDevice_Request(SubsystemId, DeviceId, Capabilities,
				   QoS, CmdType);
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	Cmd->Response[0] = (u32)Status;
	return Status;
}

static XStatus XPm_DoReleaseDevice(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 DeviceId = Cmd->Payload[0];
	u32 CmdType = Cmd->IpiReqType;
	u32 Usage = 0U;
	const XPm_Subsystem* Subsystem = NULL;
	const XPm_Device* Device = NULL;
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}
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

XStatus XPm_AddSubsystem(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->Payload[0];
	Status = XPmSubsystem_Add(SubsystemId);
	Cmd->Response[0] = (u32)Status;
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
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
	Status = XPmSubsystem_AddReqm(Payload[0], Payload, PayloadLen);
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
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPmRequirement_Add(Subsystem, XPmDevice_GetById(DeviceId),
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

XStatus XPm_AddPSMemRegnForDefaultSubsystem(void) {
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = PM_SUBSYS_DEFAULT;
	XPm_Subsystem *Subsystem;
	const XPm_MemDevice *MemDevice;
	u32 DeviceId;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		/* do not error if default subsystem is not found */
		Status = XST_SUCCESS;
		goto done;
	}

	for (u32 index = 0U; index < (u32)XPM_NODEIDX_DEV_MAX; index++) {
		/*
		 * Note: XPmDevice_GetByIndex() assumes that the caller
		 * is responsible for validating the Node ID attributes
		 * other than node index.
		 */
		XPm_Device *Device = XPmDevice_GetByIndex(index);
		if ((NULL == Device) || (1U != XPmDevice_IsRequestable(Device->Node.Id))) {
			continue;
		}
		DeviceId = Device->Node.Id;

		u32 SubClass = NODESUBCLASS(DeviceId);
		u32 Type = NODETYPE(DeviceId);
		if (((u32)XPM_NODESUBCL_DEV_MEM == SubClass) &&
		(((u32)XPM_NODETYPE_DEV_OCM == Type) || ((u32)XPM_NODETYPE_DEV_TCM == Type))) {
			MemDevice  = (XPm_MemDevice *)Device;
			u64 StartAddress = (u64)MemDevice->StartAddress;
			u64 EndAddress = (u64)MemDevice->EndAddress;
			/* TODO: Remove cond when TCM end-addr is fixed in topology */
			if ((u32)XPM_NODETYPE_DEV_TCM == Type) {
				EndAddress -= 1U;
			}
			u64 Size = EndAddress - StartAddress + 1U;

			u32 MemRegnIndx = XPmDevice_GetMemRegnCount();
			u32 MemRegnDeviceId = MEMREGN_DEVID(MemRegnIndx);

			PmDbg("DeviceId: (0x%x) MemRegnDeviceId: (0x%x)\r\n", DeviceId, MemRegnDeviceId);
			Status = XPm_AddMemRegnDevice(MemRegnDeviceId, StartAddress, Size);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			Status = XPmRequirement_Add(Subsystem, XPmDevice_GetById(MemRegnDeviceId),
				REQUIREMENT_FLAGS(1U,
				(u32)REQ_ACCESS_SECURE_NONSECURE,
				(u32)REQ_NO_RESTRICTION),
				(u32)PM_CAP_ACCESS, XPM_DEF_QOS);
			if (XST_SUCCESS != Status) {
				goto done;
			}
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
		Capability = XPM_MAX_CAPABILITY | PM_CAP_SECURE;

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
	int Status = XST_FAILURE;

	Status = XPlmi_RegisterHandler(XPLMI_IOMODULE_PMC_PWR_MB, (GicIntHandler_t)(void *)XPm_PwrIntrHandler,
				       (void *)XPLMI_IOMODULE_PMC_PWR_MB);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPlmi_PlmIntrEnable(XPLMI_IOMODULE_PMC_PWR_MB);

END:
	return Status;
}


XStatus XPm_HookAfterPlmCdo(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	/* Registers all  power related interrupt post PLM cdo loaded*/
	Status = RegisterNEnablePwrIntr();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* If default subsystem is present, attempt to add its requirements */
	Subsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);

	if (((u32)1U == XPmSubsystem_GetMaxSubsysIdx()) && (NULL != Subsystem) && ((u8)ONLINE == Subsystem->State)) {
		Status = XPm_AddReqsDefaultSubsystem(Subsystem);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to add requirements to default subsystem\n\r");
			goto done;
		}
		Status = XPm_AddPSMemRegnForDefaultSubsystem();
		if (XST_SUCCESS != Status) {
			PmErr("Failed to add PS Mem Regn requirements to default subsystem\n\r");
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
	if (PM_SUBSYS_PMC == TargetSubsystemId || PM_SUBSYS_ASU == TargetSubsystemId) {
		Status = XST_SUCCESS;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(TargetSubsystemId);
	if (NULL != Subsystem) {
		Status = Subsystem->Ops->Activate(Subsystem);
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
XStatus XPm_PmcActivateSubsystem(const u32 SubsystemId)	{
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
		Status = Subsystem->Ops->GetStatus(Subsystem, DeviceStatus);
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
XStatus XPm_DoSelfSuspend(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 DeviceId = Cmd->Payload[0];
	u32 Latency = Cmd->Payload[1];
	u8 State = (u8)(Cmd->Payload[2] & 0xFF);
	u32 AddrLow = Cmd->Payload[3];
	u32 AddrHigh = Cmd->Payload[4];
	Status = XPm_SelfSuspend(SubsystemId, DeviceId, Latency, State, AddrLow, AddrHigh);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
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
	/* Unused function for versal_gen2 */
}
maybe_unused static inline XStatus XPm_PlatCmnFlush(const u32 SubsystemId)
{
	(void)SubsystemId;
        return XST_SUCCESS;
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

		Status = Subsystem->Ops->SetState(Subsystem, (u32)SUSPENDING);
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

	XPm_ClearScanClear();

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
XStatus XPm_ForcePowerdown(u32 SubsystemId, const u32 NodeId, const u32 Ack,
			   const u32 CmdType, const u32 IpiMask)
{
//	//XPM_EXPORT_CMD(PM_FORCE_POWERDOWN, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	u32 NodeState = 0U;
	const XPm_Power *Power;

	if ((u32)REQUEST_ACK_BLOCKING == Ack) {
		/* Disable IPI interrupt */
		PmOut32(IPI_PMC_IDR, IpiMask);
	}

	/* Validate access first */
	Status = XPm_IsForcePowerDownAllowed(SubsystemId, NodeId, CmdType);
	if (XST_SUCCESS != Status) {
		goto process_ack;
	}

	/* Retrieve target subsystem */
	Subsystem = XPmSubsystem_GetById(NodeId);
	if (NULL == Subsystem) {
		Status = XST_INVALID_PARAM;
		goto process_ack;
	}
	Subsystem->FrcPwrDwnReq.AckType = Ack;
	Subsystem->FrcPwrDwnReq.InitiatorIpiMask = IpiMask;
	NodeState = Subsystem->State;

	/* Restore multiboot register value*/
	XPlmi_RestoreMultiboot();

	if (0U != (Subsystem->Flags & (u8)SUBSYSTEM_IDLE_SUPPORTED)) {
		Status = XPm_RequestHBMonDevice(NodeId, CmdType);
		if (XST_DEVICE_NOT_FOUND == Status) {
			PmWarn("Add runtime HB_MON node for recovery\r\n");
		} else if (XST_SUCCESS != Status) {
			/*
			 * Error while requesting run time Healthy Boot
			 * Monitor node
			 */
			goto done;
		} else {
			/* Required by MISRA */
		}

		Status = Subsystem->Ops->SetState(Subsystem,
					       (u8)PENDING_POWER_OFF);
		if (XST_SUCCESS != Status) {
			goto process_ack;
		}
		NodeState = Subsystem->State;

		Status = XPm_SubsystemIdleCores(Subsystem);
		if (XST_SUCCESS != Status) {
			goto process_ack;
		}
		goto done;
	} else {
		Status = XPmSubsystem_ForcePwrDwn(NodeId);
		goto done;
	}

process_ack:
	XPm_ProcessAckReq(Ack, IpiMask, Status, NodeId, NodeState);

done:
#ifdef XPLMI_IPI_DEVICE_ID
	if ((u32)REQUEST_ACK_BLOCKING != Ack) {
		/* Write response */
		IPI_RESPONSE1(IpiMask, (u32)Status);
		/* Clear interrupt status */
		PmOut32(IPI_PMC_ISR, IpiMask);
		PmOut32(IPI_PMC_IER, IpiMask);
	}
#endif /* XPLMI_IPI_DEVICE_ID */

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

static void XPm_CoreIdle(XPm_Core *Core)
{
	Core->Device.Node.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN;
	XPmNotifier_Event(Core->Device.Node.Id,
			  (u8)EVENT_CPU_IDLE_FORCE_PWRDWN);
}

static XStatus XPm_RequestHBMonDevice(const u32 SubsystemId, const u32 CmdType)
{
	u32 DeviceIdx;
	XStatus Status = XST_DEVICE_NOT_FOUND;
	const XPm_Device *Device;
	XPm_Requirement *Reqm = NULL;

	/* Request run time Healthy Boot Monitor node if it is added */
	for (DeviceIdx = (u32)XPM_NODEIDX_DEV_HB_MON_0;
	     DeviceIdx < (u32)XPM_NODEIDX_DEV_HB_MON_MAX; DeviceIdx++) {
		Device = XPmDevice_GetHbMonDeviceByIndex(DeviceIdx);
		if (NULL == Device) {
			continue;
		}
		Reqm = XPmDevice_FindRequirement(Device->Node.Id, SubsystemId);
		/* Skip if boot time healthy boot monitor node found */
		if ((NULL == Reqm) || (1U == PREALLOC((u32)Reqm->Flags))) {
			continue;
		}
		Status = XPmDevice_Request(SubsystemId, Device->Node.Id,
					   (u32)PM_CAP_ACCESS, Reqm->PreallocQoS,
					   CmdType);
		break;
	}

	return Status;
}

XStatus XPm_SubsystemIdleCores(const XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm;
	u32 DeviceId;

	LIST_FOREACH(Subsystem->Requirements, ReqmNode){
		Reqm = ReqmNode->Data;
		DeviceId = Reqm->Device->Node.Id;
		/**
		 * PSM is required to be up when any application processor is
		 * running. In case of default subsystem, PSM is part of
		 * pre-alloc. So PSM might be powered down during force power
		 * down of subsystem. Currently there is no user option to
		 * force power down default subsystem because every processor
		 * is part of default subsystem and we don't allow force power
		 * down of own subsystem. However, if we want to use
		 * XPmSubsystem_ForcePwrDwn() from other cases (e.g. subsystem
		 * restart) then PSM power down will happen. So skip PSM power
		 * down from XPmSubsystem_ForcePwrDwn().
		 */
		if ((1U == Reqm->Allocated) &&
		    ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(DeviceId))) {
			XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
			if (NULL == Core) {
				Status = XST_INVALID_PARAM;
				goto done;
			}
			u8 IsCoreIdleSupported = 0;
			Status = XPmCore_GetCoreIdleSupport(Core, &IsCoreIdleSupported);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			if (1U == IsCoreIdleSupported) {
				XPm_CoreIdle(Core);
			} else if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)) &&
				   (1U == Core->isCoreUp)) {
				XPm_CoreIdle(Core);
			} else {
				/* Required by MISRA */
			}
			Status = XST_SUCCESS;

		}
	}

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
void XPm_ProcessAckReq(const u32 Ack, const u32 IpiMask, const int Status,
		       const u32 NodeId, const u32 NodeState)
{
#ifdef XPLMI_IPI_DEVICE_ID
	if (0U == IpiMask) {
		goto done;
	}

	if ((u32)REQUEST_ACK_BLOCKING == Ack) {
		/* Return status immediately */
		IPI_RESPONSE1(IpiMask, (u32)Status);
		/* Clear interrupt status */
		PmOut32(IPI_PMC_ISR, IpiMask);
		/* Enable IPI interrupt */
		PmOut32(IPI_PMC_IER, IpiMask);
	} else if ((u32)REQUEST_ACK_NON_BLOCKING == Ack) {
		/* Return acknowledge through callback */
		IPI_MESSAGE4(IpiMask, (u32)PM_ACKNOWLEDGE_CB, NodeId, (u32)Status,
			      NodeState);
		if (XST_SUCCESS != XPlmi_IpiTrigger(IpiMask)) {
			PmWarn("Error in IPI trigger\r\n");
		}
	} else {
		/* No returning of the acknowledge */
	}
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
XStatus XPm_AbortSuspend(const u32 SubsystemId, const u32 Reason,
			 const u32 DeviceId)
{
	//XPM_EXPORT_CMD(PM_ABORT_SUSPEND, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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

	Status = Subsystem->Ops->SetState(Subsystem, (u32)ONLINE);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
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
	//XPM_EXPORT_CMD(PM_REQUEST_WAKEUP, XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);
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
			if ((NULL == Core) ||
			    (NULL == Core->CoreOps->RequestWakeup)) {
				Status = XPM_ERR_WAKEUP;
				break;
			}
			if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(CoreDeviceId)) ||
			    ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(CoreDeviceId)) ||
			    ((u32)XPM_NODETYPE_DEV_CORE_PSM == NODETYPE(CoreDeviceId))) {
				/* Power up LPD if not powered up */
				Power = XPmPower_GetById(PM_POWER_LPD);
				if ((NULL != Power) && ((u8)XPM_POWER_STATE_OFF == Power->Node.State)) {
					Status = XPm_RestartCbWrapper(Power->Node.Id);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
			}

			Status = Core->CoreOps->RequestWakeup(Core, SetAddress, Address);
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
	u32 DeviceId = Cmd->Payload[0];
	/**  setAddress is encoded in the LSB bit of the low-word address */
	/**  we can do this because addresses are word alinghed ignore LSB */
	u32 SetAddress = Cmd->Payload[1] & 0x1U;
	u32 AddressLow = Cmd->Payload[1] & (~(u32)0x1U);
	u32 AddressHigh = Cmd->Payload[2];
	u64 Address = (u64)AddressLow + ((u64)AddressHigh << 32ULL);
	u32 Ack = Cmd->Payload[3];
	u32 CmdType =Cmd->IpiReqType;
	Status = XPm_RequestWakeUp(SubsystemId, DeviceId, SetAddress, Address, Ack, CmdType);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
static XStatus XPm_SubsystemPwrUp(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Activate the subsystem by requesting its pre-alloc devices */
	Status = Subsystem->Ops->Activate(Subsystem);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Flush LLC in CMN block before reloading subsystem Image. */
	Status = XPm_PlatCmnFlush(SubsystemId);
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
int XPm_RestartCbWrapper(const u32 SubsystemId)
{
	int Status = XST_FAILURE;

	if (NULL != PmRestartCb) {
		Status = PmRestartCb(SubsystemId, NULL);
	}

	return Status;
}


static XStatus SetSubsystemState_ByCore(XPm_Core* Core, const u32 State){
	XStatus Status = XST_FAILURE;
	if (NULL == Core) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	u32 CoreSubsystemId = XPmDevice_GetSubsystemIdOfCore((XPm_Device *)Core);
	if (INVALID_SUBSYSID == CoreSubsystemId) {
		PmErr("Invalid Subsystem Id for core %d\n\r", Core->Device.Node.Id);
		Status = XPM_ERR_SUBSYS_NOTFOUND;
		goto done;
	}
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(CoreSubsystemId);

	Status = Subsystem->Ops->SetState(Subsystem, State);
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
	//XPM_EXPORT_CMD(PM_SYSTEM_SHUTDOWN, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	const XPm_ResetNode *Rst;

	if ((PM_SHUTDOWN_TYPE_SHUTDOWN != Type) &&
	    (PM_SHUTDOWN_TYPE_RESET != Type)) {
		Status = XPM_INVALID_TYPEID;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Restore multiboot register value*/
	XPlmi_RestoreMultiboot();

	/* For shutdown type the subtype is irrelevant: shut the caller down */
	if (PM_SHUTDOWN_TYPE_SHUTDOWN == Type) {
		/* Idle the subsystem first */
		Status = Subsystem->Ops->Idle(Subsystem);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_SUBSYS_IDLE;
			goto done;
		}
		/* Release devices and power down cores */
		Status = XPmSubsystem_ForceDownCleanup(SubsystemId);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_CLEANUP;
			goto done;
		}

		/* Clear the pending suspend cb reason */
		Subsystem->PendCb.Reason = 0U;

		Status = Subsystem->Ops->SetState(Subsystem, (u32)POWERED_OFF);
		goto done;
	}

	switch (SubType) {
	case PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM:
#if 0
		/* FIXME: Disable idle callback support for now */
		if (0U != (SUBSYSTEM_IDLE_SUPPORTED & Subsystem->Flags)) {
			Status = XPm_RequestHBMonDevice(SubsystemId, CmdType);
			if (XST_DEVICE_NOT_FOUND == Status) {
				PmWarn("Add runtime HB_MON node for recovery\r\n");
			} else if (XST_SUCCESS != Status) {
				/*
				 * Error while requesting run time Healthy Boot
				 * Monitor node
				 */
				goto done;
			} else {
				/* Required by MISRA */
			}

			Status = Subsystem->Ops->SetState(Subsystem, (u8)PENDING_RESTART);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = XPm_SubsystemIdleCores(Subsystem);
			if (XST_SUCCESS != Status) {
				goto done;
			}
#endif

		Status = XPmSubsystem_ForcePwrDwn(SubsystemId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPm_SubsystemPwrUp(SubsystemId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case PM_SHUTDOWN_SUBTYPE_RST_SYSTEM:
		/*
		 * Caller subystem may not be allowed to enact reset operation
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

static XStatus XPm_DoSystemShutdown(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 Type = Cmd->Payload[0];
	u32 SubType = Cmd->Payload[1];
	u32 CmdType = Cmd->IpiReqType;
	Status = XPm_SystemShutdown(SubsystemId, Type, SubType, CmdType);
	Cmd->Response[0] = (u32)Status;
	if (XST_SUCCESS != Status) {
		PmErr("Subsystem: 0x%x, Type: 0x%x, SubType: 0x%x, Status: 0x%x\n\r",
			SubsystemId, Type, SubType, Status);
	}
	return Status;
}
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
XStatus XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
		      const u32 Latency)
{
	//XPM_EXPORT_CMD(PM_SET_MAX_LATENCY, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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



static XStatus XPm_DoResetAssert(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ResetId = Cmd->Payload[0];
	u32 CmdType = Cmd->IpiReqType;
	u32 Action = Cmd->Payload[1];
	Status = XPm_SetResetState(SubsystemId, ResetId, Action, CmdType);
	Cmd->Response[0] = (u32)Status;
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
XStatus XPm_SetResetState(const u32 SubsystemId, const u32 ResetId,
			  const u32 Action, const u32 CmdType)
{
	//XPM_EXPORT_CMD(PM_RESET_ASSERT, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	u32 SubClass = NODESUBCLASS(ResetId);
	u32 SubType = NODETYPE(ResetId);
	XPm_ResetNode* Reset;

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

	Status = XPmReset_AssertbyId(ResetId, Action);

done:
	return Status;
}


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
	Status = Subsystem->Ops->InitFinalize(Subsystem);
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
XStatus XPm_GetChipID(u32* IDCode, u32 *Version)
{
	/* Read the chip ID code */
	PmIn32(PMC_TAP_IDCODE, *IDCode);

	/* Read the chip version */
	PmIn32(PMC_TAP_VERSION, *Version);

	return XST_SUCCESS;
}

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
static XStatus XPm_DoActivateSubsystem(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 TargetSubsystemId = Cmd->Payload[0];
	Status = XPm_ActivateSubsystem(SubsystemId, TargetSubsystemId);
	Cmd->Response[0] = (u32)Status;
	return Status;
}
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
XStatus XPm_PinCtrlRequest(const u32 SubsystemId, const u32 PinId)
{
	//XPM_EXPORT_CMD(PM_PINCTRL_REQUEST, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
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
XStatus XPm_PinCtrlRelease(const u32 SubsystemId, const u32 PinId)
{
	//XPM_EXPORT_CMD(PM_PINCTRL_RELEASE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;

	Status = XPmPin_Release(SubsystemId, PinId);

	return Status;
}

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
XStatus XPm_SetPinFunction(const u32 SubsystemId,
	const u32 PinId, const u32 FunctionId)
{
	//XPM_EXPORT_CMD(PM_PINCTRL_SET_FUNCTION, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
XStatus XPm_GetPinFunction(const u32 PinId, u32 *const FunctionId)
{
	//XPM_EXPORT_CMD(PM_PINCTRL_GET_FUNCTION, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;

	Status = XPmPin_GetPinFunction(PinId, FunctionId);

	return Status;
}
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
XStatus XPm_SetPinParameter(const u32 SubsystemId, const u32 PinId,
			const u32 ParamId,
			const u32 ParamVal)
{
	//XPM_EXPORT_CMD(PM_PINCTRL_CONFIG_PARAM_SET, XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);
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
XStatus XPm_GetPinParameter(const u32 PinId,
			const u32 ParamId,
			u32 * const ParamVal)
{
	//XPM_EXPORT_CMD(PM_PINCTRL_CONFIG_PARAM_GET, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;

	Status = XPmPin_GetPinConfig(PinId, ParamId, ParamVal);

	return Status;
}

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
	//XPM_EXPORT_CMD(PM_IOCTL, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_FIVE);
	XStatus Status = XST_FAILURE;

	Status = XPm_Ioctl(SubsystemId, DeviceId, IoctlId, Arg1, Arg2, Arg3,
			   Response, CmdType);
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}
static XStatus XPm_DoDevIoctl(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 DeviceId = Cmd->Payload[0];
	pm_ioctl_id IoctlId = Cmd->Payload[1];
	u32 Arg1 = Cmd->Payload[2];
	u32 Arg2 = Cmd->Payload[3];
	u32 Arg3 = Cmd->Payload[4];
	Status = XPm_DevIoctl(SubsystemId, DeviceId, IoctlId, Arg1, Arg2, Arg3, &Cmd->Response[1], Cmd->IpiReqType);
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
	//XPM_EXPORT_CMD(PM_PLL_SET_MODE, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
	//XPM_EXPORT_CMD(PM_PLL_GET_MODE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
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
	//XPM_EXPORT_CMD(PM_PLL_SET_PARAMETER, XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);
	XStatus Status = XST_FAILURE;
	const XPm_PllClockNode* Clock;

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
	//XPM_EXPORT_CMD(PM_PLL_GET_PARAMETER, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
XStatus XPm_Query(const u32 Qid, const u32 Arg1, const u32 Arg2,
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
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

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
XStatus XPm_SetClockState(const u32 SubsystemId, const u32 ClockId, const u32 Enable)
{
	//XPM_EXPORT_CMD(PM_CLOCK_ENABLE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	//XPM_EXPORT_CMD(PM_CLOCK_DISABLE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);
	u32 CurrState = 0U;

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
static XStatus XPm_DoClockEnable(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ClockId = Cmd->Payload[0];
	Status = XPm_SetClockState(SubsystemId, ClockId, 1);
	Cmd->Response[0] = (u32)Status;
	return Status;
}

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
 * @param State		Pointer to the clock state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetClockState(const u32 ClockId, u32 *const State)
{
	//XPM_EXPORT_CMD(PM_CLOCK_GETSTATE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk,
						(u32)TYPE_GATE, State);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_GetMode((XPm_PllClockNode *)Clk, State);
		if (*State == (u32)PM_PLL_MODE_RESET) {
			*State = 0;
		} else {
			*State = 1;
		}
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

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
XStatus XPm_SetClockDivider(const u32 SubsystemId, const u32 ClockId, const u32 Divider)
{
	//XPM_EXPORT_CMD(PM_CLOCK_SETDIVIDER, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	const XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	if (0U == Divider) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check if subsystem is allowed to access requested clock or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
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

static XStatus XPm_DoSetClockDivider(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
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
XStatus XPm_GetClockDivider(const u32 ClockId, u32 *const Divider)
{
	//XPM_EXPORT_CMD(PM_CLOCK_GETDIVIDER, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;
	const XPm_ClockNode *Clk = NULL;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk,
						(u32)TYPE_DIV1, Divider);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_GetParam((XPm_PllClockNode *)Clk,
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
XStatus XPm_SetClockParent(const u32 SubsystemId, const u32 ClockId, const u32 ParentIdx)
{
	//XPM_EXPORT_CMD(PM_CLOCK_SETPARENT, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
XStatus XPm_GetClockParent(const u32 ClockId, u32 *const ParentIdx)
{
	//XPM_EXPORT_CMD(PM_CLOCK_GETPARENT, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
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

	Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk, (u32)TYPE_MUX, ParentIdx);

done:
	return Status;
}
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
	//XPM_EXPORT_CMD(PM_REQUEST_NODE, XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);
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
	u32 NodeId;
	XPm_NodeAccess *NodeEntry;
	const u32 *Args = Cmd->Payload;
	u32 NumArgs = Cmd->Len;
	/* SET_NODE_ACCESS <NodeId: Arg0> <Arg 1,2> <Arg 3,4> ... */
	if ((NumArgs < 3U) || ((NumArgs % 2U) == 0U)) {
		Status = XST_FAILURE;
		goto done;
	}

	NodeId = Args[0];

	/* TODO: Check if NodeId is present in database */

	NodeEntry = (XPm_NodeAccess *)XPm_AllocBytesOthers(sizeof(XPm_NodeAccess));
	if (NULL == NodeEntry) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	NodeEntry->Id = NodeId;
	NodeEntry->Aperture = NULL;
	NodeEntry->NextNode = NULL;

	Status = XPmAccess_UpdateTable(NodeEntry, Args, NumArgs);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
XStatus XPm_HookAfterBootPdi(void)
{
	/* TODO: Review where interrupts need to be enabled */
	/* Enable power related interrupts to PMC */
	XPm_RMW32(PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN, PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN_MASK,
		  PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN_MASK);
	/* Init Pin RuntimeOps */
	for (int i = 0; i < XPM_NODEIDX_STMIC_MAX; i++) {
		XPm_PinNode* Pin = XPmPin_GetByIndex(i);
		if (NULL == Pin) {
			continue;
		}
		XPmPin_RuntimeOps_Init(Pin);
	}
	return XST_SUCCESS;
}
