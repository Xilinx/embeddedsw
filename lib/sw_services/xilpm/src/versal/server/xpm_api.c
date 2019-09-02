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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_api.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_pldomain.h"
#include "xpm_pll.h"
#include "xpm_powerdomain.h"
#include "xpm_pmcdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_npdomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_psm.h"
#include "xpm_pmc.h"
#include "xpm_periph.h"
#include "xpm_mem.h"
#include "xpm_apucore.h"
#include "xpm_rpucore.h"
#include "xpm_power.h"
#include "xpm_pin.h"
#include "xplmi.h"
#include "xplmi_modules.h"
#include "xpm_aie.h"
#include "xpm_prot.h"
#include "xpm_regs.h"
#include "xpm_ipi.h"
#include "xsysmonpsv.h"
#include "xpm_notifier.h"

u32 ResetReason;
u32 SysmonAddresses[XPM_NODEIDX_MONITOR_MAX];

extern int XLoader_RestartImage(u32 SubsystemId);

void (* PmRequestCb)(u32 SubsystemId, const u32 EventId, u32 *Payload);

static XPlmi_ModuleCmd XPlmi_PmCmds[PM_API_MAX+1];
static XPlmi_Module XPlmi_Pm =
{
	XPLMI_MODULE_XILPM_ID,
	XPlmi_PmCmds,
	PM_API_MAX+1,
};

static int XPm_ProcessCmd(XPlmi_Cmd * Cmd)
{
	u32 ApiResponse[XPLMI_CMD_RESP_SIZE-1] = {0, 0, 0};
	u32 Status = XST_SUCCESS;
	u32 SubsystemId = INVALID_SUBSYSID;
	u32 *Pload = Cmd->Payload;
	u32 Len = Cmd->Len;
	u32 SetAddress;
	u64 Address;

	PmDbg("Processing Cmd %x\r\n", Cmd->CmdId);

	if((Cmd->CmdId & 0xFF) != PM_SET_CURRENT_SUBSYSTEM) {
		SubsystemId = XPmSubsystem_GetCurrent();
		if(SubsystemId != INVALID_SUBSYSID) {
                        PmDbg("Using current subsystemId: 0x%x\n\r",
								SubsystemId);
                } else if(Cmd->IpiMask == 0 && Cmd->SubsystemId) {
			SubsystemId = Cmd->SubsystemId;
                        PmDbg("Using subsystemId passed by PLM: 0x%x\n\r",
								SubsystemId);
		} else if(Cmd->IpiMask) {
			SubsystemId = XPmSubsystem_GetSubSysIdByIpiMask(Cmd->IpiMask);
			PmDbg("Using subsystemId mapped to IPI: 0x%x\n\r", SubsystemId);
		}
		if(SubsystemId == INVALID_SUBSYSID) {
			PmInfo("Failure: Invalid SubsystemId\n\r");
			Status = XPM_INVALID_SUBSYSID;
			goto done;
		}
	}

	switch (Cmd->CmdId & 0xFF) {
		case PM_GET_CHIPID:
			Status = XPm_GetChipID(&ApiResponse[0], &ApiResponse[1]);
			break;
		case PM_GET_API_VERSION:
			Status = XPm_GetApiVersion(ApiResponse);
			break;
		case PM_REQUEST_WAKEUP:
			/* setAddress is encoded in the 1st bit of the low-word address */
			SetAddress = Pload[1] & 0x1U;
			/* addresses are word-aligned, ignore bit 0 */
			Address = ((u64) Pload[2]) << 32ULL;
			Address += Pload[1] & ~0x1U;
			Status = XPm_RequestWakeUp(SubsystemId, Pload[0],
						   SetAddress, Address,
						   Pload[3]);
			break;
		case PM_FORCE_POWERDOWN:
			Status = XPm_ForcePowerdown(SubsystemId, Pload[0], Pload[1]);
			break;
		case PM_SYSTEM_SHUTDOWN:
			Status = XPm_SystemShutdown(SubsystemId, Pload[0], Pload[1]);
			break;
		case PM_SELF_SUSPEND:
			Status = XPm_SelfSuspend(SubsystemId, Pload[0],
						 Pload[1], Pload[2], Pload[3],
						 Pload[4]);
			break;
		case PM_REQUEST_SUSPEND:
			Status = XPm_RequestSuspend(SubsystemId, Pload[0], Pload[1], Pload[2], Pload[3]);
			break;
		case PM_ABORT_SUSPEND:
			Status = XPm_AbortSuspend(SubsystemId, Pload[0], Pload[1]);
			break;
		case PM_SET_WAKEUP_SOURCE:
			Status = XPm_SetWakeupSource(SubsystemId, Pload[0], Pload[1], Pload[2]);
			break;
		case PM_CLOCK_SETPARENT:
			Status = XPm_SetClockParent(SubsystemId, Pload[0], Pload[1]);
			break;
		case PM_CLOCK_GETPARENT:
			Status = XPm_GetClockParent(Pload[0], ApiResponse);
			break;
		case PM_CLOCK_ENABLE:
			Status = XPm_SetClockState(SubsystemId, Pload[0], 1);
			break;
		case PM_CLOCK_DISABLE:
			Status = XPm_SetClockState(SubsystemId, Pload[0], 0);
			break;
		case PM_CLOCK_GETSTATE:
			Status = XPm_GetClockState(Pload[0], ApiResponse);
			break;
		case PM_CLOCK_SETDIVIDER:
			Status = XPm_SetClockDivider(SubsystemId, Pload[0], Pload[1]);
			break;
		case PM_CLOCK_GETDIVIDER:
			Status = XPm_GetClockDivider(Pload[0], ApiResponse);
			break;
		case PM_PLL_SET_PARAMETER:
			Status = XPm_SetPllParameter(SubsystemId, Pload[0], Pload[1], Pload[2]);
			break;
		case PM_PLL_GET_PARAMETER:
			Status = XPm_GetPllParameter(Pload[0], Pload[1], ApiResponse);
			break;
		case PM_PLL_SET_MODE:
			Status = XPm_SetPllMode(SubsystemId, Pload[0], Pload[1]);
			break;
		case PM_PLL_GET_MODE:
			Status = XPm_GetPllMode(Pload[0], ApiResponse);
			break;
		case PM_REQUEST_NODE:
			Status = XPm_RequestDevice(SubsystemId, Pload[0],
						   Pload[1], Pload[2],
						   Pload[3]);
			break;
		case PM_RELEASE_NODE:
			Status = XPm_ReleaseDevice(SubsystemId, Pload[0]);
			break;
		case PM_SET_REQUIREMENT:
			Status = XPm_SetRequirement(SubsystemId, Pload[0], Pload[1], Pload[2], Pload[3]);
			break;
		case PM_SET_MAX_LATENCY:
			Status = XPm_SetMaxLatency(SubsystemId, Pload[0],
						   Pload[1]);
			break;
		case PM_GET_NODE_STATUS:
			Status = XPm_GetDeviceStatus(SubsystemId, Pload[0], (XPm_DeviceStatus *)ApiResponse);
			break;
		case PM_QUERY_DATA:
			Status = XPm_Query(Pload[0], Pload[1], Pload[2],
					   Pload[3], ApiResponse);
			break;
		case PM_RESET_ASSERT:
			Status = XPm_SetResetState(SubsystemId, Cmd->IpiMask,
						   Pload[0], Pload[1]);
			break;
		case PM_RESET_GET_STATUS:
			Status = XPm_GetResetState(Pload[0], ApiResponse);
			break;
		case PM_ADD_SUBSYSTEM:
			Status = XPm_AddSubsystem(Pload[0]);
			break;
		case PM_DESTROY_SUBSYSTEM:
			Status = XPm_DestroySubsystem(Pload[0]);
			break;
		case PM_PINCTRL_REQUEST:
			Status = XPm_PinCtrlRequest(SubsystemId, Pload[0]);
			break;
		case PM_PINCTRL_RELEASE:
			Status = XPm_PinCtrlRelease(SubsystemId, Pload[0]);
			break;
		case PM_PINCTRL_GET_FUNCTION:
			Status = XPm_GetPinFunction(Pload[0], ApiResponse);
			break;
		case PM_PINCTRL_SET_FUNCTION:
			Status = XPm_SetPinFunction(SubsystemId, Pload[0], Pload[1]);
			break;
		case PM_PINCTRL_CONFIG_PARAM_GET:
			Status = XPm_GetPinParameter(Pload[0], Pload[1], ApiResponse);
			break;
		case PM_PINCTRL_CONFIG_PARAM_SET:
			Status = XPm_SetPinParameter(SubsystemId, Pload[0], Pload[1], Pload[2]);
			break;
		case PM_IOCTL:
			Status = XPm_DevIoctl(SubsystemId, Pload[0], Pload[1],
					      Pload[2], Pload[3], ApiResponse);
			break;
		case PM_INIT_FINALIZE:
			Status = XPm_InitFinalize(SubsystemId);
			break;
		case PM_DESCRIBE_NODES:
			Status = XPm_DescribeNodes(Len);
			break;
		case PM_ADD_NODE:
			Status = XPm_AddNode(&Pload[0], Len);
			break;
		case PM_ADD_NODE_PARENT:
			Status = XPm_AddNodeParent(&Pload[0], Len);
			break;
		case PM_ADD_NODE_NAME:
			Status = XPm_AddNodeName(&Pload[0], Len);
			break;
		case PM_ADD_REQUIREMENT:
			Status = XPm_AddRequirement(Pload[0], Pload[1], Pload[2], &Pload[3], Len-3);
			break;
		case PM_SET_CURRENT_SUBSYSTEM:
			Status = XPm_SetCurrentSubsystem(Pload[0]);
			break;
		case PM_INIT_NODE:
			Status = XPm_InitNode(Pload[0], Pload[1], &Pload[2], Len-2);
			break;
		case PM_FEATURE_CHECK:
			Status = XPm_FeatureCheck(Pload[0], ApiResponse);
			break;
		case PM_ISO_CONTROL:
			Status = XPm_IsoControl(Pload[0], Pload[1]);
			break;
		case PM_GET_OP_CHARACTERISTIC:
			Status = XPm_GetOpCharacteristic(Pload[0], Pload[1],
							 ApiResponse);
			break;
		case PM_REGISTER_NOTIFIER:
			Status = XPm_RegisterNotifier(SubsystemId, Pload[0],
						      Pload[1], Pload[2],
						      Pload[3], Cmd->IpiMask);
			break;
		default:
			PmErr("CMD: INVALID PARAM\n\r");
			Status = XST_INVALID_PARAM;
			break;
	}

	/* First word of the response is status */
	Cmd->Response[0] = Status;
	memcpy(&Cmd->Response[1], ApiResponse, sizeof(ApiResponse));

	/*
	 * XPM_QID_CLOCK_GET_NAME and XPM_QID_PINCTRL_GET_FUNCTION_NAME store part
	 * of their clock names in Status variable which is stored in response.
	 * So this value should not be treated as error code.
	 * Consider error only if clock name is not found.
	 */
	if ((PM_QUERY_DATA == (Cmd->CmdId & 0xFF)) &&
	   ((XPM_QID_CLOCK_GET_NAME == Pload[0]) ||
	   (XPM_QID_PINCTRL_GET_FUNCTION_NAME == Pload[0]))) {
		if (!Cmd->Response) {
			Status = XPM_INVALID_NAME;
		} else {
			Status = XST_SUCCESS;
		}
	}

	if (Status == XST_SUCCESS) {
		Cmd->ResumeHandler = NULL;
	} else {
		PmErr("Error %d while processing command 0x%x\r\n", Status, Cmd->CmdId);
		PmDbg("Command payload: 0x%x, 0x%x, 0x%x, 0x%x\r\n",
			Pload[0], Pload[1], Pload[2], Pload[3]);
	}
done:
	if(Status != XST_SUCCESS) {
		PmErr("Err Code: 0x%x\n\r", Status);
	}
	return Status;
}


/****************************************************************************/
/**
 * @brief  Initialize XilLibPM library
 *
 * @param  IpiInst		IPI instance
 * @param  RequestCb 	Pointer to the request calbback handler
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_Init(void (* const RequestCb)(u32 SubsystemId, const u32 EventId, u32 *Payload))
{
	XStatus Status = XST_SUCCESS;
	unsigned int i;
	u32 Version;
	u32 PmcIPORMask = (CRP_RESET_REASON_ERR_POR_MASK |
			   CRP_RESET_REASON_SLR_POR_MASK |
			   CRP_RESET_REASON_SW_POR_MASK);
	u32 SysResetMask = (CRP_RESET_REASON_SLR_SYS_MASK |
			    CRP_RESET_REASON_SW_SYS_MASK |
			    CRP_RESET_REASON_ERR_SYS_MASK |
			    CRP_RESET_REASON_DAP_SYS_MASK);
	u32 IsolationIdx[] = {
		XPM_NODEIDX_ISO_VCCAUX_VCCRAM,
		XPM_NODEIDX_ISO_VCCRAM_SOC,
		XPM_NODEIDX_ISO_VCCAUX_SOC,
		XPM_NODEIDX_ISO_PL_SOC,
		XPM_NODEIDX_ISO_PMC_SOC,
		XPM_NODEIDX_ISO_PMC_SOC_NPI,
		XPM_NODEIDX_ISO_PMC_PL,
		XPM_NODEIDX_ISO_PMC_LPD,
		XPM_NODEIDX_ISO_LPD_SOC,
		XPM_NODEIDX_ISO_LPD_PL,
		XPM_NODEIDX_ISO_LPD_CPM,
		XPM_NODEIDX_ISO_FPD_SOC,
		XPM_NODEIDX_ISO_FPD_PL,
	};

	PmInfo("Initializing LibPM\n\r");

	PmRequestCb = RequestCb;

	/* Register command handlers with eFSBL */
	for(i=1; i<XPlmi_Pm.CmdCnt; i++)
		XPlmi_PmCmds[i].Handler = XPm_ProcessCmd;
	XPlmi_ModuleRegister(&XPlmi_Pm);

	XPm_PsmModuleInit();

	PmIn32(PMC_TAP_VERSION, Version);
	PlatformVersion = ((Version & PMC_TAP_VERSION_PLATFORM_VERSION_MASK) >>
                        PMC_TAP_VERSION_PLATFORM_VERSION_SHIFT);
	Platform = ((Version & PMC_TAP_VERSION_PLATFORM_MASK) >>
                        PMC_TAP_VERSION_PLATFORM_SHIFT);

	/* Read and store the reset reason value */
	PmIn32(CRP_RESET_REASON, ResetReason);

	if (0 != (ResetReason & SysResetMask)) {
		/* Clear the system reset bits of reset_reason register */
		PmOut32(CRP_RESET_REASON, (ResetReason & SysResetMask));

		/* Enable domain isolations after system reset */
		for (i = 0; i < ARRAY_SIZE(IsolationIdx); i++) {
			Status = XPmDomainIso_Control(IsolationIdx[i], TRUE);
			if (Status != XST_SUCCESS) {
				goto done;
			}
		}
	}

	/*
	 * Clear DomainInitStatusReg in case of internal PMC_POR. Since PGGS0
	 * value is not cleared in case of internal POR.
	 */
	if (ResetReason & PmcIPORMask) {
		XPm_Out32(XPM_DOMAIN_INIT_STATUS_REG, 0);
	}

done:
	return Status;
}


XStatus XPm_HookAfterPlmCdo()
{
	XStatus Status = XST_SUCCESS;

	// On Boot, Update PMC SAT0 & SAT1 sysmon trim
	XPmPowerDomain_ApplyAmsTrim(SysmonAddresses[XPM_NODEIDX_MONITOR_SYSMON_PMC_0], PM_POWER_PMC, 0);
	XPmPowerDomain_ApplyAmsTrim(SysmonAddresses[XPM_NODEIDX_MONITOR_SYSMON_PMC_1], PM_POWER_PMC, 1);

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
int XPm_GetChipID(u32* IDCode, u32 *Version)
{
	/* Read the chip ID code */
	PmIn32(PMC_TAP_IDCODE, *IDCode);

	/* Read the chip version */
	PmIn32(PMC_TAP_VERSION, *Version);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  This function is used to request the version number of the API
 * running on the power management controller.
 *
 * @param  Version Returns the API 32-bit version number.
 * Returns 0 if no firmware present.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetApiVersion(u32 *Version)
{
	*Version = PM_VERSION;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  This function configures the
 * platform resources for the new subsystem.
 *
 * @param  SubSystemCdo	Pointer to the subsystem CDO
 * @param  NotifyCb		Pointer to the notify callback handler
 * @param  SubsystemId	Address to store the new subsystem ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   The provided address must be in an address space which is
 * accessible by the callee.  There will be no change if the subsystem CDO
 * is incompatible or if the required resources are not available, so no
 * clean-up will be necessary
 *
 ****************************************************************************/
XStatus XPm_AddSubsystem(u32 SubsystemId)
{
	XStatus Status;
	Status = XPmSubsystem_Add(SubsystemId);

	if (Status != XST_SUCCESS) {
		PmErr("Failed to configure platform resources\n\r");
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows to set current subsystem id.
 *
 * @param  SubsystemId	Address to store the new subsystem ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   none
 *
 ****************************************************************************/
XStatus XPm_SetCurrentSubsystem(u32 SubsystemId)
{
	XStatus Status;
	Status = XPmSubsystem_SetCurrent(SubsystemId);

	if (Status != XST_SUCCESS) {
		PmErr("Unable to set current subsystem. Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows to initialize the node.
 *
 * @param  NodeId	Supported power domain nodes only
 * @param  Function	Function id
 * @param  Args		Arguments speicifc to function
 * @param  NumArgs  Number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   none
 *
 ****************************************************************************/
XStatus XPm_InitNode(u32 NodeId, u32 Function, u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_SUCCESS;
	XPm_PowerDomain *PwrDomainNode;

	if ((XPM_NODECLASS_POWER != NODECLASS(NodeId)) ||
	    (XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(NodeId)) ||
	    (XPM_NODEIDX_POWER_MAX <= NODEINDEX(NodeId))) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	PwrDomainNode = (XPm_PowerDomain *)XPmPower_GetById(NodeId);
	if (NULL == PwrDomainNode) {
		PmErr("Unable to find Power Domain for given Node Id\n\r");
		Status = XPM_PM_INVALID_NODE;
                goto done;
	}

	switch (NODEINDEX(NodeId)) {
	case XPM_NODEIDX_POWER_PMC:
	case XPM_NODEIDX_POWER_LPD:
	case XPM_NODEIDX_POWER_FPD:
	case XPM_NODEIDX_POWER_NOC:
	case XPM_NODEIDX_POWER_PLD:
	case XPM_NODEIDX_POWER_ME:
	case XPM_NODEIDX_POWER_CPM:
		Status = XPmPowerDomain_InitDomain(PwrDomainNode, Function, Args, NumArgs);
		break;
	default:
		Status = XPM_INVALID_PWRDOMAIN;
		PmErr("Unrecognized Power Domain: 0x%x\n\r", NODEINDEX(NodeId));
		break;
	}

	/*
	 * Call LPD init to initialize required components
	 */
	if ((NODEINDEX(NodeId) == XPM_NODEIDX_POWER_LPD) &&
		(Function == FUNC_INIT_FINISH) &&
		(XST_SUCCESS == Status)) {
		XPlmi_LpdInit();
	}

done:
	if (Status != XST_SUCCESS) {
		PmErr("Unable to initialize node. Returned: 0x%x\n\r", Status);
	}
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function allows to control isolation nodes.
 *
 * @param  Isoaltion NodeId	Supported isoaltion nodes only
 * @param  Enable/Disable
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   none
 *
 ****************************************************************************/
XStatus XPm_IsoControl(u32 NodeId, u32 Enable)
{
	XStatus Status = XST_SUCCESS;

	if ((XPM_NODECLASS_ISOLATION != NODECLASS(NodeId)) ||
	    (XPM_NODEIDX_ISO_MAX <= NODEINDEX(NodeId))) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Status = XPmDomainIso_Control(NODEINDEX(NodeId), Enable);

done:
	if (Status != XST_SUCCESS) {
		PmErr("Unable to initialize node. Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function releases all the resources of a subsystem.  The
 * subsystem ID will become invalid.
 *
 * @param  SubsystemId	Subsystem ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_DestroySubsystem(u32 SubsystemId)
{
	XStatus Status;
	Status = XPmSubsystem_Destroy(SubsystemId);

	if (Status != XST_SUCCESS) {
		PmErr("Unable release node. Returned: 0x%x\n\r", Status);
	}
	return Status;
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
	XStatus Status = XST_SUCCESS;
	XPm_Core *Core;

	PmInfo("(%lu, %lu)\r\n", Reason, DeviceId);

	if((NODECLASS(DeviceId) == XPM_NODECLASS_DEVICE) &&
	   (NODESUBCLASS(DeviceId) == XPM_NODESUBCL_DEV_CORE)) {
		Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
		Core->Device.Node.State = XPM_DEVSTATE_RUNNING;
		Status = XPM_PM_ABORT_SUSPEND;
	} else {
		PmErr("Invalid Device Id\n\r");
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	DISABLE_WFI(Core->SleepMask);

	Status = XPmSubsystem_SetState(SubsystemId, ONLINE);

done:
	if (Status != XST_SUCCESS) {
		PmErr("Unable to abort suspend child subsystem. Returned: 0x%x\n\r", Status);
	}
	return Status;
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
	XStatus Status = XST_SUCCESS;
	XPm_Core *Core;
	u64 Address = (u64)AddrLow + ((u64)AddrHigh << 32ULL);

	/* TODO: Remove this warning fix hack when functionality is implemented */
	(void)Latency;

	if((NODECLASS(DeviceId) == XPM_NODECLASS_DEVICE) &&
	   (NODESUBCLASS(DeviceId) == XPM_NODESUBCL_DEV_CORE)) {
		Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
		Core->ResumeAddr = Address | 1U;
		Core->Device.Node.State = XPM_DEVSTATE_SUSPENDING;
	} else {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	ENABLE_WFI(Core->SleepMask);

	if (PM_SUSPEND_STATE_SUSPEND_TO_RAM == State) {
		Status = XPmSubsystem_SetState(SubsystemId, SUSPENDING);
	}

done:
	if (Status != XST_SUCCESS) {
		PmErr("Unable to Self Suspend child subsystem. Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to send suspend request
 * to another subsystem.  If the target subsystem accepts the request, it
 * needs to initiate its own self suspend.
 *
 * @param SubsystemId		Subsystem ID
 * @param TargetSubsystemId	Target subsystem ID (cannot be the same subsystem)
 * @param Ack			Ack request
 * @param Latency		Desired wakeup latency
 * @param State			Desired power state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been sent.
 *
 ****************************************************************************/
XStatus XPm_RequestSuspend(const u32 SubsystemId, const u32 TargetSubsystemId,
			   const u32 Ack, const u32 Latency, const u32 State)
{
	XStatus Status = XST_SUCCESS;
	u32 IpiMask = 0;
	u32 Payload[5] = {0};
	/* Warning Fix */
	(void) (Ack);

	IpiMask = XPmSubsystem_GetIPIMask(TargetSubsystemId);
	if (0 == IpiMask) {
		PmErr("Unable to fetch IpiMask for given TargetSubsystem\r\n");
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if (SubsystemId == TargetSubsystemId) {
		Status = XPM_INVALID_SUBSYSID;
		PmErr("Cannot Suspend yourself\n\r");
		goto done;
	}

	/* TODO: Check if current subsystem has access to request target subsystem */
	/* Failure in this case should return XPM_PM_NO_ACCESS */

	/* TODO: Target subsystem must be active to get the suspend request */

	/* TODO: Check if other subsystem has sent suspend request to target subsystem */
	/* Failure in this case should return XPM_PM_DOUBLE_REQ */

	Payload[0] = XPM_INIT_SUSPEND_CB;
	Payload[1] = SUSPEND_REASON_SUBSYSTEM_REQ;
	Payload[2] = Latency;
	Payload[3] = State;
	/* Payload[4] is for timeout which is not considered */
	Payload[4] = 0U;

	/* Send the suspend request via callback */
	if (PmRequestCb) {
		(*PmRequestCb)(IpiMask, XPM_INIT_SUSPEND_CB, Payload);
	}

done:
	if (Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

int XPm_GicProxyWakeUp(const u32 PeriphIdx)
{
	int Status = XST_FAILURE;

	XPm_Periph *Periph = (XPm_Periph *)XPmDevice_GetByIndex(PeriphIdx);
	if ((NULL == Periph) || (0 == Periph->WakeProcId)) {
		goto done;
	}

	Status = XPm_RequestWakeUp(PM_SUBSYS_PMC, Periph->WakeProcId, 0, 0, 0);

done:
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
			  const u32 Ack)
{
	XStatus Status;
	XPm_Core *Core;
	u32 CoreSubsystemId;
	/* Warning Fix */
	(void) (Ack);

	/*Validate access first */
	Status = XPm_IsWakeAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
	if(Core->CoreOps->RequestWakeup) {
		Status = Core->CoreOps->RequestWakeup(Core, SetAddress, Address);
		if (XST_SUCCESS == Status) {
			CoreSubsystemId = XPmDevice_GetSubsystemIdOfCore((XPm_Device *)Core);
			XPmSubsystem_SetState(CoreSubsystemId, ONLINE);
		}
	} else {
		Status = XPM_ERR_WAKEUP;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to Powerdown other
 * 		processor or domain node forcefully. To powerdown whole
 * 		subsystem, this function needs to be called for all processors
 * 		of target subsystem, which in turn will power down the whole
 * 		subsystem
 *
 * @param SubsystemId	Subsystem ID
 * @param  Node 		Processor or domain node to be powered down
 * @param  Ack			Ack request
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   The affected PUs are not notified about the upcoming powerdown,
 *          and PMU does not wait for their WFI interrupt.
 *
 ****************************************************************************/
XStatus XPm_ForcePowerdown(u32 SubsystemId, const u32 NodeId, const u32 Ack)
{
	XStatus Status = XST_SUCCESS;
	XPm_Core *Core;
	XPm_Device *Device;
	XPm_Power *Power;
	u32 TargetSubsystemId;
	XPm_Requirement *Reqm;
	int i;

	/* Warning Fix */
	(void) (Ack);

	/*Validate access first */
	Status = XPm_IsForcePowerDownAllowed(SubsystemId, NodeId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

        if ((NODECLASS(NodeId) == XPM_NODECLASS_DEVICE) &&
	    (NODESUBCLASS(NodeId) == XPM_NODESUBCL_DEV_CORE)) {
		Core = (XPm_Core *)XPmDevice_GetById(NodeId);
		if (Core && Core->CoreOps && Core->CoreOps->PowerDown) {
			Status = Core->CoreOps->PowerDown(Core);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			/* Disable the direct wake in case of force power down */
			DISABLE_WAKE(Core->SleepMask);
		}

		Reqm = Core->Device.Requirements;
		while (Reqm) {
			if (TRUE == Reqm->Allocated) {
				TargetSubsystemId = Reqm->Subsystem->Id;
				if (XST_SUCCESS == XPmSubsystem_IsAllProcDwn(TargetSubsystemId)) {
					/* Idle the subsystem */
					Status = XPmSubsystem_Idle(TargetSubsystemId);
					if(XST_SUCCESS != Status) {
						Status = XPM_ERR_SUBSYS_IDLE;
						goto done;
					}

					Status = XPmSubsystem_ForceDownCleanup(TargetSubsystemId);
					if(XST_SUCCESS != Status) {
						Status = XPM_ERR_CLEANUP;
						goto done;
					}

					XPmSubsystem_SetState(TargetSubsystemId, OFFLINE);
				}
				XPmRequirement_Clear(Reqm);
			}
			Reqm = Reqm->NextSubsystem;
		}

	} else if (XPM_NODECLASS_POWER == NODECLASS(NodeId)) {
		if (XPM_NODESUBCL_POWER_ISLAND == NODESUBCLASS(NodeId)) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}
		VERIFY(XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(NodeId));

		/*
		 * PMC power domain can not be powered off.
		 */
		if (XPM_NODEIDX_POWER_PMC == NODEINDEX(NodeId)) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}

		/*
		 * Release devices belonging to the power domain.
		 */
		for (i = 1; i < XPM_NODEIDX_DEV_MAX; i++) {
			/*
			 * Note: XPmDevice_GetByIndex() assumes that the caller
			 * is responsible for validating the Node ID attributes
			 * other than node index.
			 */
			Device = XPmDevice_GetByIndex(i);
			if ((NULL == Device) ||
			    (XPM_DEVSTATE_UNUSED == Device->Node.State)) {
				continue;
			}

			/*
			 * Check power topology of this device to identify
			 * if it belongs to the power domain.
			 */
			Power = Device->Power;
			while (NULL != Power) {
				if (NodeId == Power->Node.Id) {
					Status = XPmRequirement_Release(
					    Device->Requirements, RELEASE_DEVICE);
					if (XST_SUCCESS != Status) {
						Status = XPM_PM_INVALID_NODE;
						goto done;
					}
				}
				Power = Power->Parent;
			}
		}
	} else {
		Status = XPM_PM_INVALID_NODE;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to shutdown self or restart
 * 			self, Ps or system
 *
 * @param SubsystemId		Subsystem ID
 * @param  Type				Shutdown type
 * @param SubType			Shutdown subtype
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been received.
 *
 ****************************************************************************/
XStatus XPm_SystemShutdown(u32 SubsystemId, const u32 Type, const u32 SubType)
{
        XStatus Status = XST_SUCCESS;
	XPm_Subsystem *Subsystem;
	XPm_Requirement *Reqm;
	XPm_Device *Device;
	XPm_Core *Core = NULL;

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

	/* For shutdown type the subtype is irrelevant: shut the caller down */
	if (PM_SHUTDOWN_TYPE_SHUTDOWN == Type) {
		Reqm = Subsystem->Requirements;
		while (NULL != Reqm) {
			if (TRUE == Reqm->Allocated) {
				Device = Reqm->Device;
				if (XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Device->Node.Id)) {
					Core = (XPm_Core *)XPmDevice_GetById(Device->Node.Id);
					if (Core->CoreOps->PowerDown) {
						Status = Core->CoreOps->PowerDown(Core);
						if (XST_SUCCESS != Status) {
							goto done;
						}
					}
				}
			}
			Reqm = Reqm->NextDevice;
		}
		/* Idle the subsystem */
		Status = XPmSubsystem_Idle(SubsystemId);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_SUBSYS_IDLE;
			goto done;
		}
		Status = XPmSubsystem_ForceDownCleanup(SubsystemId);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_CLEANUP;
			goto done;
		}

		XPmSubsystem_SetState(SubsystemId, OFFLINE);
		goto done;
	}

	VERIFY(PM_SHUTDOWN_TYPE_RESET == Type);

	switch (SubType) {
	case PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM:
		Status = XPmSubsystem_Restart(SubsystemId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XLoader_RestartImage(SubsystemId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case PM_SHUTDOWN_SUBTYPE_RST_PS_ONLY:
		/* TODO */
		break;
	case PM_SHUTDOWN_SUBTYPE_RST_SYSTEM:
		Status = XPmReset_SystemReset();
		break;
	default:
		Status = XPM_INVALID_TYPEID;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to to set wake up
 * source
 *
 * @param SubsystemId Initiator of the request
 * @param TargetNodeId  Core to be woken-up (currently must be same as initiator)
 * @param SourceNodeId  Source of the wake-up (Device that generates interrupt)
 * @param Enable      Flag stating should event be enabled or disabled
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been received.
 *
 ****************************************************************************/
XStatus XPm_SetWakeupSource(const u32 SubsystemId, const u32 TargetNodeId,
			    const u32 SourceNodeId, const u32 Enable)
{
	int Status = XST_SUCCESS;
	XPm_Periph *Periph = NULL;
	XPm_Subsystem *Subsystem;

	/* Check if given target node is valid */
	if(NODECLASS(TargetNodeId) != XPM_NODECLASS_DEVICE ||
	   NODESUBCLASS(TargetNodeId) != XPM_NODESUBCL_DEV_CORE)
	{
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* The call applies only to peripheral nodes */
	if (NODECLASS(SourceNodeId) != XPM_NODECLASS_DEVICE ||
	    NODESUBCLASS(SourceNodeId) != XPM_NODESUBCL_DEV_PERIPH) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Is subsystem allowed to use resource (slave)? */
	Status = XPm_IsAccessAllowed(SubsystemId, SourceNodeId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Periph = (XPm_Periph *)XPmDevice_GetById(SourceNodeId);
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if(!Periph || !Subsystem) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check whether the device has wake-up capability */
	Status = XPm_CheckCapabilities(&Periph->Device, PM_CAP_WAKEUP);
	if (XST_SUCCESS != Status) {
		Status = XST_NO_FEATURE;
		goto done;
	}

	Periph->WakeProcId = TargetNodeId;

	if (NULL != Periph->PeriphOps->SetWakeupSource) {
		Periph->PeriphOps->SetWakeupSource(Periph, Enable);
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
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
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_RequestDevice(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 Ack)
{
	XStatus Status;

	/* Warning Fix */
	(void) (Ack);

	Status = XPmDevice_Request(SubsystemId, DeviceId, Capabilities,
				   QoS);

	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used by a subsystem to release the usage of a
 * device. This will tell the platform management controller that the device
 * is no longer needed, allowing the device to be placed into an inactive
 * state.
 *
 * @param SubsystemId	Subsystem ID
 * @param  DeviceId	ID of the device.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_ReleaseDevice(const u32 SubsystemId, const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem* Subsystem = NULL;
	XPm_Device* Device = NULL;
	u32 Usage = 0U;

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

	Status = XPmDevice_Release(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Usage = XPmDevice_GetUsageStatus(Subsystem, Device);
	if (0U == Usage) {
		XPmNotifier_Event(Device, EVENT_ZERO_USERS);
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used by a subsystem to announce a change in
 * requirements for a specific device which is currently in use.
 *
 * @param SubsystemId	Subsystem ID.
 * @param DeviceId	ID of the device.
 * @param Capabilities	Capabilities required
 * @param QoS		Quality of Service (0-100) required.
 * @param Ack		Ack request
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If this function is called after the last awake CPU within the
 * subsystem calls RequestSuspend, the requirement change shall be performed
 * after the CPU signals the end of suspend to the platform management
 * controller, (e.g. WFI interrupt).
 *
 ****************************************************************************/
XStatus XPm_SetRequirement(const u32 SubsystemId, const u32 DeviceId,
			   const u32 Capabilities, const u32 QoS, const u32 Ack)
{
	XStatus Status = XST_FAILURE;

	/* Warning Fix */
	(void) (Ack);

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmDevice_SetRequirement(SubsystemId, DeviceId,
					  Capabilities, QoS);
done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
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
int XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
		      const u32 Latency)
{
	int Status = XPM_ERR_SET_LATENCY;

	PmInfo("(%x, %lu)\r\n", DeviceId, Latency);

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmDevice_SetMaxLatency(SubsystemId, DeviceId, Latency);

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
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
 *   - 2 : if CPU is suspending (powered up)
 *  - For power islands and power domains:
 *   - 0 : if island is powered down,
 *   - 1 : if island is powered up
 *  - For slaves:
 *   - 0 : if slave is powered down,
 *   - 1 : if slave is powered up,
 *   - 2 : if slave is in retention
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
XStatus XPm_GetDeviceStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;


	switch(NODECLASS(DeviceId)) {
	case XPM_NODECLASS_DEVICE:
		Status = XPmDevice_GetStatus(SubsystemId, DeviceId, DeviceStatus);
		break;
	case XPM_NODECLASS_POWER:
		Status = XPmPower_GetStatus(SubsystemId, DeviceId, DeviceStatus);
		break;
	case XPM_NODECLASS_SUBSYSTEM:
		Status = XPmSubsystem_GetStatus(SubsystemId, DeviceId,
						DeviceStatus);
		break;
	default:
		Status = XST_INVALID_PARAM;
	}

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
	u32 Status = XST_SUCCESS;

	/* Warning Fix */
	(void) (Arg3);

	switch (Qid) {
		case XPM_QID_CLOCK_GET_NAME:
			Status = XPmClock_QueryName(Arg1,Output);
			break;
		case XPM_QID_CLOCK_GET_TOPOLOGY:
			Status = XPmClock_QueryTopology(Arg1,Arg2,Output);
			break;
		case XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS:
			Status = XPmClock_QueryFFParams(Arg1,Output);
			break;
		case XPM_QID_CLOCK_GET_MUXSOURCES:
			if (ISPLL(Arg1)) {
				Status = XPmClockPll_QueryMuxSources(Arg1,Arg2,Output);
			} else {
				Status = XPmClock_QueryMuxSources(Arg1,Arg2,Output);
			}
			break;
		case XPM_QID_CLOCK_GET_ATTRIBUTES:
			Status = XPmClock_QueryAttributes(Arg1,Output);
			break;
		case XPM_QID_PINCTRL_GET_NUM_PINS:
			Status = XPmPin_GetNumPins(Output);
			break;
		case XPM_QID_PINCTRL_GET_NUM_FUNCTIONS:
			Status = XPmPinFunc_GetNumFuncs(Output);
			break;
		case XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS:
			Status = XPmPinFunc_GetNumFuncGroups(Arg1, Output);
			break;
		case XPM_QID_PINCTRL_GET_FUNCTION_NAME:
			Status = XPmPinFunc_GetFuncName(Arg1, (char *)Output);
			break;
		case XPM_QID_PINCTRL_GET_FUNCTION_GROUPS:
			Status = XPmPinFunc_GetFuncGroups(Arg1, Arg2, (u16 *)Output);
			break;
		case XPM_QID_PINCTRL_GET_PIN_GROUPS:
			Status = XPmPin_GetPinGroups(Arg1, Arg2, (u16 *)Output);
			break;
		case XPM_QID_CLOCK_GET_NUM_CLOCKS:
			Status = XPmClock_GetNumClocks(Output);
			break;
		case XPM_QID_CLOCK_GET_MAX_DIVISOR:
			Status = XPmClock_GetMaxDivisor(Arg1, Arg2, Output);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}
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
	XStatus Status = XST_SUCCESS;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);
	u32 CurrState;

	/* HACK: Don't disable PLL clocks for now */
	if(Enable == 0 && ISPLL(ClockId))
	{
		return Status;
	}

	/* Check if clock's state is already desired state */
	Status = XPm_GetClockState(ClockId, &CurrState);
	if ((XST_SUCCESS == Status) && (CurrState == Enable)) {
		goto done;
	}

	/* HACK: Allow enabling of PLLs for now */
	if ((TRUE == Enable) && (ISPLL(ClockId))) {
		goto bypass;
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
		if (1 == Enable) {
			Mode = ((XPm_PllClockNode *)Clk)->PllMode;
		} else if (0 == Enable) {
			Mode = PM_PLL_MODE_RESET;
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
	XStatus Status = XST_SUCCESS;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk,
						TYPE_GATE, State);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_GetMode((XPm_PllClockNode *)Clk, State);
		if (*State == PM_PLL_MODE_RESET) {
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
	XStatus Status = XST_SUCCESS;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

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
					      PM_PLL_PARAM_ID_FBDIV, Divider);
	} else {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
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
	XStatus Status = XST_SUCCESS;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk,
						TYPE_DIV1, Divider);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_GetParam((XPm_PllClockNode *)Clk,
					      PM_PLL_PARAM_ID_FBDIV, Divider);
	} else {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
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
	XStatus Status = XST_SUCCESS;
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
	XStatus Status = XST_SUCCESS;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	/* Get parent is allowed only on output clocks */
	if (!ISOUTCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk, TYPE_MUX, ParentIdx);

done:
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
	XStatus Status = XST_SUCCESS;
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
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%xn\r", Status);
	}
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
	XStatus Status = XST_SUCCESS;
	XPm_PllClockNode* Clock;

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
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
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
	XStatus Status = XST_SUCCESS;
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
	XStatus Status = XST_SUCCESS;
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
 * @brief  This function reset or de-reset a device.
 *
 * @param SubsystemId	Subsystem ID
 * @param ResetId	Reset ID
 * @param IpiMask	IPI Mask currently being used
 * @param Action	Reset (true) or de-reset (false) the device
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
XStatus XPm_SetResetState(const u32 SubsystemId, const u32 IpiMask,
			  const u32 ResetId, const u32 Action)
{
	int Status = XST_SUCCESS;
	XPm_ResetNode* Reset;
	u32 SubClass = NODESUBCLASS(ResetId);
	u32 SubType = NODETYPE(ResetId);

	Reset = XPmReset_GetById(ResetId);
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * XSDB is a privileged master, allow unrestricted access.
	 */
	if (XSDB_IPI_INT_MASK != IpiMask) {
		/*
		 * Only peripheral and debug resets
		 * are allowed to control externally, on other masters.
		 */
		if (XPM_NODESUBCL_RESET_PERIPHERAL == SubClass) {
			if (XPM_NODETYPE_RESET_PERIPHERAL != SubType) {
				Status = XPM_PM_NO_ACCESS;
				goto done;
			}
		} else if (XPM_NODESUBCL_RESET_DBG == SubClass) {
			if (XPM_NODETYPE_RESET_DBG != SubType) {
				Status = XPM_PM_NO_ACCESS;
				goto done;
			}
		} else {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}

		/* Check if subsystem is allowed to access requested reset */
		Status = XPm_IsAccessAllowed(SubsystemId, ResetId);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	}

	Status = Reset->Ops->SetState(Reset, Action);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the device reset state.
 *
 * @param  ResetId		Reset ID
 * @param State		Pointer to the reset state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetResetState(const u32 ResetId, u32 *const State)
{
	int Status = XST_SUCCESS;
	XPm_ResetNode* Reset;

	Reset = XPmReset_GetById(ResetId);
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	*State = Reset->Ops->GetState(Reset);

done:
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
	XStatus Status;

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
	XStatus Status;

	Status = XPmPin_Release(SubsystemId, PinId);

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
	XStatus Status = XST_FAILURE;

	Status = XPmPin_GetPinFunction(PinId, FunctionId);

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
	XStatus Status = XST_FAILURE;

	Status = XPmPin_GetPinConfig(PinId, ParamId, ParamVal);

	return Status;
}

/****************************************************************************/
/**
 * @brief  Enable/Disable tap delay bypass
 *
 * @param  DeviceId	ID of the device
 * @param  Type		Type of tap delay to enable/disable (QSPI)
 * @param  Value	Enable/Disable
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_SetTapdelayBypass(const u32 DeviceId, const u32 Type,
				 const u32 Value)
{
	int Status = XST_SUCCESS;
	XPm_Device *Device = XPmDevice_GetById(DeviceId);
	u32 BaseAddress;

	if (NULL == Device) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/* QSPI base address */
	BaseAddress = Device->Node.BaseAddress;

	if (((XPM_TAPDELAY_BYPASS_ENABLE != Value) &&
	    (XPM_TAPDELAY_BYPASS_DISABLE != Value)) ||
	    (XPM_TAPDELAY_QSPI != Type)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PmRmw32(BaseAddress + TAPDLY_BYPASS_OFFSET, XPM_TAP_DELAY_MASK,
		Value << Type);

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function resets DLL logic for the SD device.
 *
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Reset type
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_SdDllReset(const u32 DeviceId, const u32 Type)
{
	int Status = XST_SUCCESS;
	XPm_Device *Device = XPmDevice_GetById(PM_DEV_PMC_PROC);
	u32 BaseAddress;
	u32 Offset;

	if (NULL == Device) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/* PMC_IOU_SLCR base address */
	BaseAddress = Device->Node.BaseAddress;

	if (PM_DEV_SDIO_0 == DeviceId) {
		Offset = SD0_CTRL_OFFSET;
	} else if (PM_DEV_SDIO_1 == DeviceId) {
		Offset = SD1_CTRL_OFFSET;
	} else {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	switch (Type) {
	case XPM_DLL_RESET_ASSERT:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			XPM_SD_DLL_RST_MASK);
		break;
	case XPM_DLL_RESET_RELEASE:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			~XPM_SD_DLL_RST_MASK);
		break;
	case XPM_DLL_RESET_PULSE:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			XPM_SD_DLL_RST_MASK);
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			~XPM_SD_DLL_RST_MASK);
		break;
	default:
		Status = XPM_INVALID_TYPEID;
		break;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets input/output tap delay for the SD device.
 *
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Type of tap delay to set (input/output)
 * @param  Value	Value to set for the tap delay
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_SetSdTapDelay(const u32 DeviceId, const u32 Type,
			     const u32 Value)
{
	int Status = XST_SUCCESS;
	XPm_Device *Device = XPmDevice_GetById(DeviceId);
	u32 BaseAddress;

	if (NULL == Device) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/* SD0/1 base address */
	BaseAddress = Device->Node.BaseAddress;

	Status = XPm_SdDllReset(DeviceId, XPM_DLL_RESET_ASSERT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	switch (Type) {
	case XPM_TAPDELAY_INPUT:
		PmRmw32(BaseAddress + ITAPDLY_OFFSET, XPM_SD_ITAPCHGWIN_MASK,
			XPM_SD_ITAPCHGWIN_MASK);
		PmRmw32(BaseAddress + ITAPDLY_OFFSET, XPM_SD_ITAPDLYENA_MASK,
			XPM_SD_ITAPDLYENA_MASK);
		PmRmw32(BaseAddress + ITAPDLY_OFFSET, XPM_SD_ITAPDLYSEL_MASK,
			Value);
		PmRmw32(BaseAddress + ITAPDLY_OFFSET, XPM_SD_ITAPCHGWIN_MASK,
			~XPM_SD_ITAPCHGWIN_MASK);
		break;
	case XPM_TAPDELAY_OUTPUT:
		PmRmw32(BaseAddress + OTAPDLY_OFFSET, XPM_SD_OTAPDLYENA_MASK,
			XPM_SD_OTAPDLYENA_MASK);
		PmRmw32(BaseAddress + OTAPDLY_OFFSET, XPM_SD_OTAPDLYSEL_MASK,
			Value);
		break;
	default:
		Status = XPM_INVALID_TYPEID;
		break;
	}

	Status = XPm_SdDllReset(DeviceId, XPM_DLL_RESET_RELEASE);

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function performs read/write operation on probe counter
 *         registers of LPD/FPD.
 *
 * @param  DeviceId	DeviceId of the LPD/FPD
 * @param  Arg1		- Counter Number (0 to 7 bit)
 *			- Register Type (8 to 15 bit)
 *                         0 - LAR_LSR access (Request Type is ignored and
 *                                             Counter Number is ignored)
 *                         1 - Main Ctl       (Counter Number is ignored)
 *                         2 - Config Ctl     (Counter Number is ignored)
 *                         3 - State Period   (Counter Number is ignored)
 *                         4 - PortSel
 *                         5 - Src
 *                         6 - Val
 *                      - Request Type (16 to 23 bit)
 *                         0 - Read Request
 *                         1 - Read Response
 *                         2 - Write Request
 *                         3 - Write Response
 *                         4 - lpd Read Request (For LPD only)
 *                         5 - lpd Read Response (For LPD only)
 *                         6 - lpd Write Request (For LPD only)
 *                         7 - lpd Write Response (For LPD only)
 * @param  Value	Register value to write (if Write flag is 1)
 * @param  Response	Value of register read (if Write flag is 0)
 * @param  Write	Operation type (0 - Read, 1 - Write)
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_ProbeCounterAccess(u32 DeviceId, u32 Arg1, u32 Value,
				  u32 *const Response, u8 Write)
{
	int Status = XST_INVALID_PARAM;
	XPm_Power *Power;
	u32 Reg;
	u8 CounterIdx;
	u8 ReqType;
	u32 ReqTypeOffset;
	u32 FpdReqTypeOffset[] = {
		PROBE_COUNTER_FPD_RD_REQ_OFFSET,
		PROBE_COUNTER_FPD_RD_RES_OFFSET,
		PROBE_COUNTER_FPD_WR_REQ_OFFSET,
		PROBE_COUNTER_FPD_WR_RES_OFFSET,
	};

	CounterIdx = Arg1 & PROBE_COUNTER_IDX_MASK;
	ReqType = ((Arg1 >> PROBE_COUNTER_REQ_TYPE_SHIFT) &
		   PROBE_COUNTER_REQ_TYPE_MASK);
	switch (NODEINDEX(DeviceId)) {
	case XPM_NODEIDX_POWER_LPD:
		if (CounterIdx > PROBE_COUNTER_CPU_R5_MAX_IDX) {
			goto done;
		}
		if (ReqType > PROBE_COUNTER_LPD_MAX_REQ_TYPE) {
			goto done;
		} else if ((ReqType > PROBE_COUNTER_CPU_R5_MAX_REQ_TYPE) &&
			   (CounterIdx > PROBE_COUNTER_LPD_MAX_IDX)) {
			goto done;
		}
		Reg = CORESIGHT_LPD_ATM_BASE;
		ReqTypeOffset = (ReqType * PROBE_COUNTER_LPD_REQ_TYPE_OFFSET);
		break;
	case XPM_NODEIDX_POWER_FPD:
		if (CounterIdx > PROBE_COUNTER_FPD_MAX_IDX) {
			goto done;
		}
		if (ReqType > PROBE_COUNTER_FPD_MAX_REQ_TYPE) {
			goto done;
		}
		Reg = CORESIGHT_FPD_ATM_BASE;
		ReqTypeOffset = FpdReqTypeOffset[ReqType];
		break;
	default:
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Power = XPmPower_GetById(DeviceId);
	if ((NULL == Power) || (XPM_POWER_STATE_ON != Power->Node.State)) {
		goto done;
	}

	switch ((Arg1 >> PROBE_COUNTER_TYPE_SHIFT) & PROBE_COUNTER_TYPE_MASK) {
	case XPM_PROBE_COUNTER_TYPE_LAR_LSR:
		if (TRUE == Write) {
			Reg += PROBE_COUNTER_LAR_OFFSET;
		} else {
			Reg += PROBE_COUNTER_LSR_OFFSET;
		}
		break;
	case XPM_PROBE_COUNTER_TYPE_MAIN_CTL:
		Reg += ReqTypeOffset + PROBE_COUNTER_MAIN_CTL_OFFSET;
		break;
	case XPM_PROBE_COUNTER_TYPE_CFG_CTL:
		Reg += ReqTypeOffset + PROBE_COUNTER_CFG_CTL_OFFSET;
		break;
	case XPM_PROBE_COUNTER_TYPE_STATE_PERIOD:
		Reg += ReqTypeOffset + PROBE_COUNTER_STATE_PERIOD_OFFSET;
		break;
	case XPM_PROBE_COUNTER_TYPE_PORT_SEL:
		Reg += (ReqTypeOffset + (CounterIdx * 20) +
			PROBE_COUNTER_PORT_SEL_OFFSET);
		break;
	case XPM_PROBE_COUNTER_TYPE_SRC:
		Reg += (ReqTypeOffset + (CounterIdx * 20) +
			PROBE_COUNTER_SRC_OFFSET);
		break;
	case XPM_PROBE_COUNTER_TYPE_VAL:
		if (TRUE == Write) {
			/* This type doesn't support write operation */
			goto done;
		}
		Reg += (ReqTypeOffset + (CounterIdx * 20) +
			PROBE_COUNTER_VAL_OFFSET);
		break;
	default:
		goto done;
	}

	if (FALSE == Write) {
		if (NULL == Response) {
			goto done;
		}
		PmIn32(Reg, *Response);
	} else {
		PmOut32(Reg, Value);
	}
	Status = XST_SUCCESS;

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function selects/returns the AXI interface to OSPI device
 *
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Reset type
 * @param  Response	Output Response (0 - DMA, 1 - LINEAR)
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_OspiMuxSelect(const u32 DeviceId, const u32 Type, u32 *Response)
{
	int Status = XST_SUCCESS;
	XPm_Device *Device;
	u32 BaseAddress;
	u32 Offset;

	(void)DeviceId;

	Device = XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Device) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	/* PMC_IOU_SLCR base address */
	BaseAddress = Device->Node.BaseAddress;
	Offset = XPM_OSPI_MUX_SEL_OFFSET;

	switch (Type) {
	case XPM_OSPI_MUX_SEL_DMA:
		PmRmw32(BaseAddress + Offset, XPM_OSPI_MUX_SEL_MASK,
				~XPM_OSPI_MUX_SEL_MASK);
		break;
	case XPM_OSPI_MUX_SEL_LINEAR:
		PmRmw32(BaseAddress + Offset, XPM_OSPI_MUX_SEL_MASK,
				XPM_OSPI_MUX_SEL_MASK);
		break;
	case XPM_OSPI_MUX_GET_MODE:
		if (NULL == Response) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		PmIn32(BaseAddress + Offset, *Response);
		*Response = (((*Response) & XPM_OSPI_MUX_SEL_MASK) >>
			XPM_OSPI_MUX_SEL_SHIFT);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
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
 * @param Response		Ioctl response
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
			const u32 IoctlId,
			const u32 Arg1,
			const u32 Arg2, u32 *const Response)
{
	XStatus Status = XPM_ERR_IOCTL;

	switch (IoctlId) {
	case IOCTL_GET_RPU_OPER_MODE:
		if ((DeviceId != PM_DEV_RPU0_0) &&
		    (DeviceId != PM_DEV_RPU0_1)) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
		XPm_RpuGetOperMode(DeviceId, Response);
		Status = XST_SUCCESS;
		break;
	case IOCTL_SET_RPU_OPER_MODE:
		if ((DeviceId != PM_DEV_RPU0_0) &&
		    (DeviceId != PM_DEV_RPU0_1)) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
		XPm_RpuSetOperMode(DeviceId, Arg1);
		Status = XST_SUCCESS;
		break;
	case IOCTL_RPU_BOOT_ADDR_CONFIG:
		if ((PM_DEV_RPU0_0 != DeviceId) &&
		    (PM_DEV_RPU0_1 != DeviceId)) {
			goto done;
		}
		Status = XPm_RpuBootAddrConfig(DeviceId, Arg1);
		break;
	case IOCTL_TCM_COMB_CONFIG:
		if ((PM_DEV_RPU0_0 != DeviceId) &&
		    (PM_DEV_RPU0_1 != DeviceId)) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
		Status = XPm_RpuTcmCombConfig(DeviceId, Arg1);
		break;
	case IOCTL_SET_TAPDELAY_BYPASS:
		if (PM_DEV_QSPI != DeviceId) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}

		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_SetTapdelayBypass(DeviceId, Arg1, Arg2);
		break;
	case IOCTL_SD_DLL_RESET:
		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_SdDllReset(DeviceId, Arg1);
		break;
	case IOCTL_SET_SD_TAPDELAY:
		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_SetSdTapDelay(DeviceId, Arg1, Arg2);
		break;
	case IOCTL_WRITE_GGS:
		if (Arg1 >= GGS_NUM_REGS) {
			goto done;
		}
		PmOut32((GGS_BASEADDR + (Arg1 << 2)), Arg2);
		Status = XST_SUCCESS;
		break;
	case IOCTL_READ_GGS:
		if (Arg1 >= GGS_NUM_REGS) {
			goto done;
		}
		PmIn32((GGS_BASEADDR + (Arg1 << 2)), *Response);
		Status = XST_SUCCESS;
		break;
	case IOCTL_WRITE_PGGS:
		if (Arg1 >= PGGS_NUM_REGS) {
			goto done;
		}
		PmOut32((PGGS_BASEADDR + (Arg1 << 2)), Arg2);
		Status = XST_SUCCESS;
		break;
	case IOCTL_READ_PGGS:
		if (Arg1 >= PGGS_NUM_REGS) {
			goto done;
		}
		PmIn32((PGGS_BASEADDR + (Arg1 << 2)), *Response);
		Status = XST_SUCCESS;
		break;
	case IOCTL_SET_BOOT_HEALTH_STATUS:
		PmRmw32(GGS_BASEADDR + GGS_4_OFFSET,
			XPM_BOOT_HEALTH_STATUS_MASK, Arg1);
		Status = XST_SUCCESS;
		break;
	case IOCTL_PROBE_COUNTER_READ:
		Status = XPm_ProbeCounterAccess(DeviceId, Arg1, Arg2,
						Response, FALSE);
		break;
	case IOCTL_PROBE_COUNTER_WRITE:
		Status = XPm_ProbeCounterAccess(DeviceId, Arg1, Arg2,
						Response, TRUE);
		break;
	case IOCTL_OSPI_MUX_SELECT:
		if (PM_DEV_OSPI != DeviceId) {
			goto done;
		}

		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPm_OspiMuxSelect(DeviceId, Arg1, Response);
		break;
	default:
		/* Not supported yet */
		Status = XPM_ERR_IOCTL;
		goto done;
		break;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
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
int XPm_InitFinalize(const u32 SubsystemId)
{
	return XPmSubsystem_InitFinalize(SubsystemId);
}

/****************************************************************************/
/**
 * @brief  This function provides topology information
 *
 * @param  Args		topology information data
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_DescribeNodes(u32 NumArgs)
{
	int Status = XST_SUCCESS;
	/*u32 NumPwrNodes, NumClkNodes, NumRstNodes, NumMioNodes, NumDevices;*/

	if(NumArgs < 3) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Uncomment when AllocNodes will implement */
	/*NumPwrNodes = (Args[0] >> 16) & 0xFFFF;
	NumClkNodes = Args[0] & 0xFFFF;
	NumRstNodes = (Args[1] >> 16) & 0xFFFF;
	NumMioNodes = Args[1] & 0xFFFF;
	NumDevices = Args[2] & 0xFFFF;

	if(NumClkNodes != 0)
		Status = XPmClock_AllocNodes(NumClkNodes);
	if(NumPwrNodes != 0)
		Status = XPmPower_AllocNodes(NumPwrNodes);
	if(NumRstNodes != 0)
		Status = XPmReset_AllocNodes(NumRstNodes);
	if (NumMioNodes != 0)
		Status = XPmMio_AllocNodes(NumMioNodes);
	if (NumDevices != 0)
		Status = XPmDevice_Alloc(NumDevices);*/
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows adding parent to any node or device
 *
 * @param  Args		Parent ids
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_AddNodeParent(u32 *Args, u32 NumArgs)
{
	int Status = XST_SUCCESS;
	u32 Id = Args[0];
	u32 *Parents;
	u32 NumParents;

	if (NumArgs < 2) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NumParents = NumArgs-1;
	Parents = &Args[1];

	switch ((Id >> NODE_CLASS_SHIFT) & NODE_CLASS_MASK_BITS) {
		case XPM_NODECLASS_POWER:
			Status = XPmPower_AddParent(Id, Parents, NumParents);
			break;
		case XPM_NODECLASS_CLOCK:
			if (ISPLL(Id)) {
				Status = XPmClockPll_AddParent(Id, Parents, NumParents);
			} else {
				Status = XPmClock_AddParent(Id, Parents, NumParents);
			}
			break;
		case XPM_NODECLASS_RESET:
			break;
		case XPM_NODECLASS_MEMIC:
			break;
		case XPM_NODECLASS_STMIC:
			break;
		case XPM_NODECLASS_DEVICE:
			Status = XPmDevice_AddParent(Id, Parents, NumParents);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function adds sub nodes for clocks having custom topology
 *
 * @param  Args		topology node arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddClockSubNode(u32 *Args, u32 NumArgs)
{
	int Status = XST_SUCCESS;
	u32 ClockId, ControlReg, Type, Flags;
	u8 Param1, Param2;

	if (NumArgs < 5) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	ClockId = Args[0];
	if (ISOUTCLK(ClockId)) {
		Type = Args[1];
		ControlReg = Args[2];
		Param1 =  Args[3] & 0xFF;
		Param2 =  (Args[3] >> 8) & 0xFF;
		Flags = Args[4];
		Status = XPmClock_AddSubNode(ClockId, Type, ControlReg, Param1, Param2, Flags);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add clock node to clock topology database
 *
 * @param  Args		clock arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeClock(u32 *Args, u32 NumArgs)
{
	int Status = XST_SUCCESS;
	u32 ClockId, ControlReg;
	u32 PowerDomainId;
	u8 TopologyType, NumCustomNodes=0, NumParents, ClkFlags;

	if (NumArgs < 4) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	ClockId = Args[0];

	if (NODETYPE(ClockId) == XPM_NODETYPE_CLOCK_SUBNODE) {
		Status = XPm_AddClockSubNode(Args, NumArgs);
		goto done;
	}
	if (ISOUTCLK(ClockId) || ISREFCLK(ClockId) || ISPLL(ClockId)) {
		ControlReg = Args[1];
		TopologyType = Args[2] & 0xFF;
		NumCustomNodes = (Args[2] >> 8) & 0xFF;
		NumParents = (Args[2] >> 16) & 0xFF;
		ClkFlags = (Args[2] >> 24) & 0xFF;
		PowerDomainId = Args[3];
		if (ISPLL(ClockId)) {
			u16 *Offsets = (u16 *)&Args[4];
			Status = XPmClockPll_AddNode(ClockId, ControlReg,
						     TopologyType, Offsets,
						     PowerDomainId, ClkFlags);
		} else {
			Status = XPmClock_AddNode(ClockId, ControlReg,
						  TopologyType, NumCustomNodes,
						  NumParents, PowerDomainId,
						  ClkFlags);
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
 * @brief  This function adds node name
 *
 * @param  Args		name
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_AddNodeName(u32 *Args, u32 NumArgs)
{
	int Status = XST_SUCCESS;
	u32 NodeId;
	char Name[MAX_NAME_BYTES] = {0};
	u32 i=0, j=0;

	if (NumArgs == 0) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	NodeId = Args[0];
	if (ISOUTCLK(NodeId) || ISREFCLK(NodeId) || ISPLL(NodeId)) {
		for (i = 1; i < NumArgs; i++,j = j+4) {
			memcpy(Name+j,&Args[i],4);
		}
		Status = XPmClock_AddClkName(NodeId, Name);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add power node to power topology database
 *
 * @param  Args		power arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodePower(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 PowerId;
	u32 PowerType;
	u8 Width;
	u8 Shift;
	u32 BitMask;
	u32 ParentId;
	XPm_Power *Power;
	XPm_Power *PowerParent = NULL;
	XPm_PsFpDomain *PsFpDomain;
	XPm_PmcDomain *PmcDomain;
	XPm_PsLpDomain *PsLpDomain;
	XPm_NpDomain *NpDomain;
	XPm_PlDomain *PlDomain;
	XPm_AieDomain *AieDomain;
	XPm_CpmDomain *CpmDomain;

	if (NumArgs < 3) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PowerId = Args[0];
	PowerType = NODETYPE(PowerId);
	Width = (Args[1] >> 8) & 0xFFU;
	Shift = Args[1] & 0xFFU;
	ParentId = Args[2];

	if (NODECLASS(PowerId) != XPM_NODECLASS_POWER) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else if (NODEINDEX(PowerId) >= XPM_NODEIDX_POWER_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else {
		/* Required by MISRA */
	}

	BitMask = BITNMASK(Shift, Width);

	if (ParentId != XPM_NODEIDX_POWER_MIN) {
		if (NODECLASS(ParentId) != XPM_NODECLASS_POWER) {
			Status = XST_INVALID_PARAM;
			goto done;
		} else if (NODEINDEX(ParentId) >= XPM_NODEIDX_POWER_MAX) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		} else {
			/* Required by MISRA */
		}

		PowerParent = XPmPower_GetById(ParentId);
		if (NULL == PowerParent) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
	}

	switch (PowerType) {
		case XPM_NODETYPE_POWER_ISLAND:
			Power = (XPm_Power *)XPm_AllocBytes(sizeof(XPm_Power));
			if (NULL == Power) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmPower_Init(Power, PowerId, BitMask,
				PowerParent);
			break;
		case XPM_NODETYPE_POWER_DOMAIN_PMC:
			PmcDomain =
				(XPm_PmcDomain *)XPm_AllocBytes(sizeof(XPm_PmcDomain));
			if (NULL == PmcDomain) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmPmcDomain_Init((XPm_PmcDomain *)PmcDomain, PowerId);
			break;
		case XPM_NODETYPE_POWER_DOMAIN_PS_FULL:
			PsFpDomain =
				(XPm_PsFpDomain *)XPm_AllocBytes(sizeof(XPm_PsFpDomain));
			if (NULL == PsFpDomain) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmPsFpDomain_Init(PsFpDomain, PowerId,
						    BitMask, PowerParent, &Args[3], (NumArgs - 3));
			break;
		case XPM_NODETYPE_POWER_DOMAIN_PS_LOW:
			PsLpDomain =
				(XPm_PsLpDomain *)XPm_AllocBytes(sizeof(XPm_PsLpDomain));
			if (NULL == PsLpDomain) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmPsLpDomain_Init(PsLpDomain, PowerId,
						    BitMask, PowerParent,
						    &Args[3], (NumArgs - 3));
			break;
		case XPM_NODETYPE_POWER_DOMAIN_NOC:
			NpDomain = (XPm_NpDomain *)XPm_AllocBytes(sizeof(XPm_NpDomain));
			if (NULL == NpDomain) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmNpDomain_Init(NpDomain, PowerId, 0x00000000,
						  PowerParent);
			break;
		case XPM_NODETYPE_POWER_DOMAIN_PL:
			PlDomain = (XPm_PlDomain *)XPm_AllocBytes(sizeof(XPm_PlDomain));
			if (NULL == PlDomain) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmPlDomain_Init(PlDomain, PowerId, 0x00000000,
						  PowerParent, &Args[3], (NumArgs - 3));
			break;
		case XPM_NODETYPE_POWER_DOMAIN_CPM:
			CpmDomain = (XPm_CpmDomain *)XPm_AllocBytes(sizeof(XPm_CpmDomain));
			if (NULL == CpmDomain) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmCpmDomain_Init(CpmDomain, PowerId, 0x00000000, PowerParent,
						   &Args[3], (NumArgs - 3));
			break;
		case XPM_NODETYPE_POWER_DOMAIN_ME:
			AieDomain = (XPm_AieDomain *)XPm_AllocBytes(sizeof(XPm_AieDomain));
			if (NULL == AieDomain) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmAieDomain_Init(AieDomain, PowerId, BitMask, PowerParent);
			break;
		default:
			Status = XST_INVALID_PARAM;
			goto done;
			break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add reset node to reset topology database
 *
 * @param  Args		reset arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeReset(u32 *Args, u32 NumArgs)
{
	int Status = XST_SUCCESS;
	u32 ResetId, ControlReg;
	u8 Shift, Width, ResetType, NumParents;
	u32 *Parents;

	if (NumArgs < 4) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	ResetId = Args[0];
	ControlReg = Args[1];
	Shift = Args[2] & 0xFF;
	Width = (Args[2] >> 8) & 0xFF;
	ResetType = (Args[2] >> 16) & 0xFF;
	NumParents = (Args[2] >> 24) & 0xFF;
	Parents = &Args[3];

	Status = XPmReset_AddNode(ResetId, ControlReg, Shift, Width, ResetType, NumParents, Parents);

done:
	return Status;
}

static XStatus AddProcDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	u32 Index;

	XPm_Psm *Psm;
	XPm_Pmc *Pmc;
	XPm_ApuCore *ApuCore;
	XPm_RpuCore *RpuCore;
	XPm_Power *Power;
	u32 BaseAddr[MAX_BASEADDR_LEN];
	u32 Ipi;

	DeviceId = Args[0];
	BaseAddr[0] = Args[2];
	Ipi = Args[3];
	BaseAddr[1] = Args[4];
	BaseAddr[2] = Args[5];

	Type = NODETYPE(DeviceId);
	Index = NODEINDEX(DeviceId);

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (Index >= XPM_NODEIDX_DEV_MAX) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
		case XPM_NODETYPE_DEV_CORE_PSM:
			Psm = (XPm_Psm *)XPm_AllocBytes(sizeof(XPm_Psm));
			if (NULL == Psm) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmPsm_Init(Psm, Ipi, BaseAddr, Power, 0, 0);
			break;
		case XPM_NODETYPE_DEV_CORE_APU:
			ApuCore = (XPm_ApuCore *)XPm_AllocBytes(sizeof(XPm_ApuCore));
			if (NULL == ApuCore) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmApuCore_Init(ApuCore, DeviceId, Ipi, BaseAddr, Power, 0, 0);
			break;
		case XPM_NODETYPE_DEV_CORE_RPU:
			RpuCore = (XPm_RpuCore *)XPm_AllocBytes(sizeof(XPm_RpuCore));
			if (NULL == RpuCore) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmRpuCore_Init(RpuCore, DeviceId, Ipi, BaseAddr, Power, 0, 0);
			break;
		case XPM_NODETYPE_DEV_CORE_PMC:
			Pmc = (XPm_Pmc *)XPm_AllocBytes(sizeof(XPm_Pmc));
			if (NULL == Pmc) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmPmc_Init(Pmc, DeviceId, 0, BaseAddr, Power, 0, 0);
			break;
		default:
			break;
	}

done:
	return Status;
}

static XStatus AddPeriphDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	u32 Index;
	u32 GicProxyMask;
	u32 GicProxyGroup;

	XPm_Periph *Device;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];
	GicProxyMask = Args[3];
	GicProxyGroup = Args[4];

	Type = NODETYPE(DeviceId);
	Index = NODEINDEX(DeviceId);

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (Index >= XPM_NODEIDX_DEV_MAX) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
		default:
			Device = (XPm_Periph *)XPm_AllocBytes(sizeof(XPm_Periph));
			if (NULL == Device) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmPeriph_Init(Device, DeviceId, BaseAddr,
						Power, NULL, NULL, GicProxyMask,
						GicProxyGroup);
			break;
	}

done:
	return Status;
}

static XStatus AddMemDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	u32 Index;

	XPm_MemDevice *Device;
	XPm_Power *Power;
	u32 BaseAddr;
	u32 StartAddr;
	u32 EndAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];
	StartAddr = Args[3];
	EndAddr = Args[4];

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Type = NODETYPE(DeviceId);
	Index = NODEINDEX(DeviceId);

	if (Index >= XPM_NODEIDX_DEV_MAX) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
		case XPM_NODETYPE_DEV_OCM:
		case XPM_NODETYPE_DEV_L2CACHE:
		case XPM_NODETYPE_DEV_DDR:
		case XPM_NODETYPE_DEV_TCM:
		case XPM_NODETYPE_DEV_EFUSE:
			Device = (XPm_MemDevice *)XPm_AllocBytes(sizeof(XPm_MemDevice));
			if (NULL == Device) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmMemDevice_Init(Device, DeviceId, BaseAddr, Power, NULL, NULL, StartAddr, EndAddr);
			break;
		default:
			break;
	}

done:
	return Status;
}

static XStatus AddMemCtrlrDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	XPm_Device *Device;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Type = NODETYPE(DeviceId);

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
		case XPM_NODETYPE_DEV_DDR:
			Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
			if (NULL == Device) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmDevice_Init(Device, DeviceId, BaseAddr,
						Power, NULL, NULL);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

done:
	return Status;
}

static XStatus AddPhyDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	XPm_Device *Device;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Type = NODETYPE(DeviceId);

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
		case XPM_NODETYPE_DEV_GT:
			Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
			if (NULL == Device) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmDevice_Init(Device, DeviceId, BaseAddr,
						Power, NULL, NULL);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function adds device node to device topology database
 *
 * @param  Args		device specific arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddDevice(u32 *Args, u32 NumArgs)
{
	int Status = XST_SUCCESS;
	u32 DeviceId;
	u32 SubClass;
	u32 PowerId;

	if (NumArgs < 3) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DeviceId = Args[0];
	SubClass = NODESUBCLASS(DeviceId);
	PowerId = Args[1];

	if (NULL == XPmPower_GetById(PowerId)) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	switch (SubClass) {
		case XPM_NODESUBCL_DEV_CORE:
			Status = AddProcDevice(Args, PowerId);
			break;
		case XPM_NODESUBCL_DEV_PERIPH:
			Status = AddPeriphDevice(Args, PowerId);
			break;
		case XPM_NODESUBCL_DEV_MEM:
			Status = AddMemDevice(Args, PowerId);
			break;
		case XPM_NODESUBCL_DEV_MEM_CTRLR:
			Status = AddMemCtrlrDevice(Args, PowerId);
			break;
		case XPM_NODESUBCL_DEV_PHY:
			Status = AddPhyDevice(Args, PowerId);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add memic node to the topology database
 *
 * @param Args		MEMIC arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeMemIc(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 MemIcId;
	u32 BaseAddress;

	if (NumArgs < 3) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	MemIcId = Args[0];
	BaseAddress = Args[2];

	if (XPM_NODECLASS_MEMIC != NODECLASS(MemIcId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (XPM_NODESUBCL_MEMIC_NOC != NODESUBCLASS(MemIcId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmNpDomain_MemIcInit(MemIcId, BaseAddress);

done:
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function add monitor node to the topology database
 *
 * @param Args		arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeMonitor(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 NodeId, BaseAddress, NodeType;

	if (NumArgs < 3) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeId = Args[0];
	BaseAddress = Args[2];

	if (XPM_NODECLASS_MONITOR != NODECLASS(NodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (XPM_NODESUBCL_MONITOR_SYSMON != NODESUBCLASS(NodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeType = NODETYPE(NodeId);

	if (((XPM_NODETYPE_MONITOR_SYSMON_PMC != NodeType) &&
	    (XPM_NODETYPE_MONITOR_SYSMON_PS != NodeType)
		&& (XPM_NODETYPE_MONITOR_SYSMON_NPD != NodeType)) ||
	    (XPM_NODEIDX_MONITOR_MAX <= NODEINDEX(NodeId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	SysmonAddresses[NODEINDEX(NodeId)] = BaseAddress;
	Status = XST_SUCCESS;
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add xmpu/xppu node to the topology database
 *
 * @param Args		Node arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeProt(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 NodeId;
	u32 BaseAddress;
	u32 SubClass;
	XPm_Prot *ProtNode;

	if (NumArgs < 3) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeId = Args[0];
	BaseAddress = Args[2];
	SubClass = NODESUBCLASS(NodeId);

	if (XPM_NODECLASS_PROTECTION != NODECLASS(NodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((XPM_NODESUBCL_PROT_XPPU != SubClass) && (XPM_NODESUBCL_PROT_XMPU != SubClass)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	switch (SubClass) {
		case XPM_NODESUBCL_PROT_XPPU:
			ProtNode = (XPm_Prot *)XPm_AllocBytes(sizeof(XPm_ProtPpu));
			if (NULL == ProtNode) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmProt_Init(ProtNode, NodeId, BaseAddress);
			break;
		case XPM_NODESUBCL_PROT_XMPU:
			ProtNode = (XPm_Prot *)XPm_AllocBytes(sizeof(XPm_ProtMpu));
			if (NULL == ProtNode) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			Status = XPmProt_Init(ProtNode, NodeId, BaseAddress);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add mio pin node to the topology database
 *
 * @param  Args		mio arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeMio(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 MioId;
	u32 BaseAddress;
	XPm_PinNode *MioPin;

	if (NumArgs < 3) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	MioId = Args[0];
	BaseAddress = Args[1];

	if (XPM_NODECLASS_STMIC != NODECLASS(MioId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (XPM_NODESUBCL_PIN != NODESUBCLASS(MioId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((XPM_NODETYPE_LPD_MIO != NODETYPE(MioId)) &&
	    (XPM_NODETYPE_PMC_MIO != NODETYPE(MioId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	MioPin = (XPm_PinNode *)XPm_AllocBytes(sizeof(XPm_PinNode));
	if (NULL == MioPin) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	Status = XPmPin_Init(MioPin, MioId, BaseAddress);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows adding node to clock, power, reset, mio
 * 			or device topology
 *
 * @param  Args		Node specific arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_AddNode(u32 *Args, u32 NumArgs)
{
	int Status = XST_SUCCESS;
	u32 Id = Args[0];

	switch (NODECLASS(Id)) {
		case XPM_NODECLASS_POWER:
			Status = XPm_AddNodePower(Args, NumArgs);
			break;
		case XPM_NODECLASS_CLOCK:
				Status = XPm_AddNodeClock(Args, NumArgs);
			break;
		case XPM_NODECLASS_RESET:
			Status = XPm_AddNodeReset(Args, NumArgs);
			break;
		case XPM_NODECLASS_MEMIC:
			Status = XPm_AddNodeMemIc(Args, NumArgs);
			break;
		case XPM_NODECLASS_STMIC:
			Status = XPm_AddNodeMio(Args, NumArgs);
			break;
		case XPM_NODECLASS_DEVICE:
			Status = XPm_AddDevice(Args, NumArgs);
			break;
		case XPM_NODECLASS_PROTECTION:
			Status = XPm_AddNodeProt(Args, NumArgs);
			break;
		case XPM_NODECLASS_MONITOR:
			Status = XPm_AddNodeMonitor(Args, NumArgs);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function links a device to a subsystem so requirement
 *         assignment could be made by XPm_RequestDevice() or
 *         XPm_SetRequirement() call.
 *
 * @param  SubsystemId	Subsystem Id
 * @param  DeviceId 	Device Id
 * @param  Flags 		Bit0-2 - No restriction/ Shared/Time Shared/Nonshared/ - 0,1,2,3
 *						Bit 3 -Secure(1)/Nonsecure(0) (Device mode)
 * @param  Params 		Optional: XPPU- master id mask for peripherals
 * @param  NumParams 	Number of params
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_AddRequirement(const u32 SubsystemId, const u32 DeviceId, u32 Flags, u32 *Params, u32 NumParams)
{
	XStatus Status = XST_INVALID_PARAM;
	XPm_Device *Device = NULL;
	XPm_Subsystem *Subsystem;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (Subsystem == NULL || Subsystem->State != ONLINE) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	Status = XPmRequirement_Add(Subsystem, Device, Flags, Params, NumParams);
done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function gets operating characteristics of a device
 *
 * @param  DeviceId  Targeted device Id.
 * @param  Type      Type of the operating characteristics:
 *                       power, temperature, and latency
 * @param  Result    Returns the value of operating characteristic type
 *
 * @return XST_SUCCESS if successful else either XST_NO_FEATURE or XST_FAILURE.
 *
 * @note   Temperature reported in Celsius (signed Q8.7 format)
 *
 ****************************************************************************/
XStatus XPm_GetOpCharacteristic(u32 const DeviceId, u32 const Type, u32 *Result)
{
	XStatus Status = XST_FAILURE;
	static XSysMonPsv SysMonInst;
	XSysMonPsv *SysMonInstPtr = &SysMonInst;
	XSysMonPsv_Config *ConfigPtr;

	if (XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		goto done;
	}

	/*
	 * TODO - need to implement getting power and latency operating
	 * characteristics.
	 */
	if (PM_OPCHAR_TYPE_TEMP != Type) {
		Status = XST_NO_FEATURE;
		goto done;
	}

	/*
	 * TODO - need to implement getting temperature, beside the
	 * temperature of entire SoC.
	 */
	if (XPM_NODETYPE_DEV_SOC != NODETYPE(DeviceId)) {
		Status = XST_NO_FEATURE;
		goto done;
	}

	/* Initialize the SysMon driver. */
	ConfigPtr = XSysMonPsv_LookupConfig();
	if (ConfigPtr == NULL) {
		goto done;
	}

	XSysMonPsv_CfgInitialize(SysMonInstPtr, ConfigPtr);

	*Result = XSysMonPsv_ReadDeviceTemp(SysMonInstPtr,
					    XSYSMONPSV_VAL_VREF_MAX);
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Register a subsystem to be notified about the device event
 *
 * @param  IpiMask      IPI mask of current subsystem
 * @param  SubsystemId  Subsystem to be notified
 * @param  DeviceId     Device to which the event is related
 * @param  Event        Event in question
 * @param  Wake         Wake subsystem upon capturing the event if value 1
 * @param  Enable       Enable the registration for value 1, disable for value 0
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 ****************************************************************************/
int XPm_RegisterNotifier(const u32 SubsystemId, const u32 DeviceId,
			 const u32 Event, const u32 Wake, const u32 Enable,
			 const u32 IpiMask)
{
	int Status = XST_FAILURE;
	XPm_Subsystem* Subsystem = NULL;
	XPm_Device* Device = NULL;

	/* Validate SubsystemId */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	/* Validate DeviceId */
	Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		goto done;
	}

	/* Validate other parameters */
	if ((0U != Wake && 1U != Wake) || (0U != Enable && 1U != Enable) ||
	    (EVENT_STATE_CHANGE != Event && EVENT_ZERO_USERS != Event)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (0U == Enable) {
		XPmNotifier_Unregister(Subsystem, Device, Event);
		Status = XST_SUCCESS;
	} else {
		Status = XPmNotifier_Register(Subsystem, Device, Event, Wake,
					      IpiMask);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns supported version of the given API.
 *
 * @param  ApiId	API ID to check
 * @param  Version	Supported version number
 *
 * @return XST_SUCCESS if successful else XST_NO_FEATURE.
 *
 * @note   None
 *
 ****************************************************************************/
int XPm_FeatureCheck(const u32 ApiId, u32 *const Version)
{
	int Status = XST_SUCCESS;

	if (NULL == Version) {
		Status = XPM_ERR_VERSION;
		goto done;
	}

	switch (ApiId) {
	case PM_GET_API_VERSION:
	case PM_GET_NODE_STATUS:
	case PM_GET_OP_CHARACTERISTIC:
	case PM_REQUEST_SUSPEND:
	case PM_SELF_SUSPEND:
	case PM_FORCE_POWERDOWN:
	case PM_ABORT_SUSPEND:
	case PM_REQUEST_WAKEUP:
	case PM_SET_WAKEUP_SOURCE:
	case PM_SYSTEM_SHUTDOWN:
	case PM_REQUEST_NODE:
	case PM_RELEASE_NODE:
	case PM_SET_REQUIREMENT:
	case PM_SET_MAX_LATENCY:
	case PM_RESET_ASSERT:
	case PM_RESET_GET_STATUS:
	case PM_PINCTRL_REQUEST:
	case PM_PINCTRL_RELEASE:
	case PM_PINCTRL_GET_FUNCTION:
	case PM_PINCTRL_SET_FUNCTION:
	case PM_PINCTRL_CONFIG_PARAM_GET:
	case PM_PINCTRL_CONFIG_PARAM_SET:
	case PM_IOCTL:
	case PM_QUERY_DATA:
	case PM_CLOCK_ENABLE:
	case PM_CLOCK_DISABLE:
	case PM_CLOCK_GETSTATE:
	case PM_CLOCK_SETDIVIDER:
	case PM_CLOCK_GETDIVIDER:
	case PM_CLOCK_SETPARENT:
	case PM_CLOCK_GETPARENT:
	case PM_PLL_SET_PARAMETER:
	case PM_PLL_GET_PARAMETER:
	case PM_PLL_SET_MODE:
	case PM_PLL_GET_MODE:
	case PM_ADD_SUBSYSTEM:
	case PM_DESTROY_SUBSYSTEM:
	case PM_DESCRIBE_NODES:
	case PM_ADD_NODE:
	case PM_ADD_NODE_PARENT:
	case PM_ADD_NODE_NAME:
	case PM_ADD_REQUIREMENT:
	case PM_SET_CURRENT_SUBSYSTEM:
	case PM_INIT_NODE:
	case PM_FEATURE_CHECK:
		*Version = XST_API_BASE_VERSION;
		break;
	default:
		*Version = 0U;
		Status = XPM_NO_FEATURE;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}
