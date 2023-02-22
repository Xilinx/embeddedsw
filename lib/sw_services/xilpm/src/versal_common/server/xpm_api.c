/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_hw.h"
#include "xplmi_ipi.h"
#include "xplmi_modules.h"
#include "xplmi_scheduler.h"
#include "xplmi_sysmon.h"
#include "xplmi_util.h"
#include "xpm_api.h"
#include "xpm_apucore.h"
#include "xpm_common.h"
#include "xpm_clock.h"
#include "xpm_cpmdomain.h"
#include "xpm_debug.h"
#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_err.h"
#include "xpm_ioctl.h"
#include "xpm_ipi.h"
#include "xpm_mem.h"
#include "xpm_nodeid.h"
#include "xpm_notifier.h"
#include "xpm_npdomain.h"
#include "xpm_periph.h"
#include "xpm_pldomain.h"
#include "xpm_pll.h"
#include "xpm_pmc.h"
#include "xpm_pmcdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psm.h"
#include "xpm_psm_api.h"
#include "xpm_regs.h"
#include "xpm_requirement.h"
#include "xpm_reset.h"
#include "xpm_rpucore.h"
#include "xpm_subsystem.h"
#include "xsysmonpsv.h"
#include "xpm_pldevice.h"

/* Macro to typecast PM API ID */
#define PM_API(ApiId)			((u32)ApiId)

/*
 * Macro for exporting xilpm command details. Use in the first line of commands
 * used in CDOs.
 */
#define XPM_EXPORT_CMD(CmdIdVal, MinArgCntVal, MaxArgCntVal) \
    XPLMI_EXPORT_CMD(CmdIdVal, XPLMI_MODULE_XILPM_ID, MinArgCntVal, MaxArgCntVal)

#define PM_GET_OP_CHAR_FEATURE_BITMASK ( \
		(1U << (u32)PM_OPCHAR_TYPE_TEMP) | \
		(1U << (u32)PM_OPCHAR_TYPE_LATENCY))

u32 ResetReason;

void (*PmRequestCb)(const u32 SubsystemId, const XPmApiCbId_t EventId, u32 *Payload);

static XPlmi_ModuleCmd XPlmi_PmCmds[PM_API_MAX];
static XPlmi_Module XPlmi_Pm =
{
	XPLMI_MODULE_XILPM_ID,
	XPlmi_PmCmds,
	PM_API(PM_API_MAX),
	NULL,
#ifdef VERSAL_NET
	NULL,
#endif
	NULL,
};
static int (*PmRestartCb)(u32 ImageId, u32 *FuncId);


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
	XPM_EXPORT_CMD(PM_ACTIVATE_SUBSYSTEM, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;

	/* Return error if request is not from PMC subsystem */
	if (PM_SUBSYS_PMC != SubsystemId) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Configure target subsystem */
	Status = XPmSubsystem_Configure(TargetSubsystemId);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the rate of the clock.
 *
 * @param IpiMask	IpiMask of subsystem
 * @param ClockId	Clock node ID
 * @param ClkRate	Clock rate
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_SetClockRate(const u32 IpiMask, const u32 ClockId, const u32 ClkRate)
{
	XPM_EXPORT_CMD(PM_CLOCK_SETRATE, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	/* Set rate is allow only for the request come from CDO,
	 * So by use of IpiMask check that request come from CDO or not,
	 * If request comes from CDO then IpiMask will 0x00U.
	 */
	if (0U != IpiMask) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Set rate is allowed only for ref clocks */
	if (!ISREFCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_SetRate(Clk, ClkRate);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function gets the rate of the clock.
 *
 * @param ClockId	Clock node ID
 * @param ClkRate	Pointer to store clock rate.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_GetClockRate(const u32 ClockId, u32 *ClkRate)
{
	XPM_EXPORT_CMD(PM_CLOCK_GETRATE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_SUCCESS;
	const XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	if (NULL == Clk) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Get rate is allowed only for ref clocks */
	if (!ISREFCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_GetRate(Clk, ClkRate);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function links a device to a subsystem with the given policies.
 *         It is a helper function serving the handling of subsystem <-> device
 *         requirements for PM_ADD_REQ command.
 *         There are a few special cases to be handled here, such as:
 *           - Requirements for PGGS/GGS nodes
 *		* PM_ADD_NODE is called internally if the node is not present
 *           - Requirements for Healthy Boot Monitoring nodes
 *              * PM_ADD_NODE is called internally if the node is not present
 *           - Generic requirements for all the device nodes
 *              * Node must be present in the database through topology CDO
 *              or otherwise
 *
 * @param  SubsystemId  Subsystem Id
 * @param  DeviceId     Device Id
 * @param  ReqFlags     Bit[0:1] - No-restriction(0)/Shared(1)/Time-Shared(2)/Nonshared(3)
 *                      Bit[2] - Secure(1)/Nonsecure(0) (Device mode)
 *                      Bit[3:5] - Reserved
 *                      Bit[6] - Pre-alloc flag
 * @param  Args		Node specific arguments
 *			- Pre-alloc capability bits
 *			- Quality of Service
 * @param  NumArgs	Total number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddDevRequirement(XPm_Subsystem *Subsystem, u32 DeviceId,
				     u32 ReqFlags, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;

	Status = XPm_PlatAddDevRequirement(Subsystem, DeviceId, ReqFlags, Args, NumArgs);

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function links a node (dev/rst/subsys/regnode) to a subsystem.
 *         Requirement assignment could be made by XPm_RequestDevice() or
 *         XPm_SetRequirement() call.
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
static XStatus XPm_AddRequirement(const u32 *Args, const u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 SubsysId, DevId, Flags;
	XPm_ResetNode *Rst = NULL;
	XPm_Subsystem *Subsys, *TarSubsys;

	/* Check the minimum basic arguments required for this command */
	if (3U > NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Parse the basic arguments */
	SubsysId = Args[0];
	DevId = Args[1];
	Flags = Args[2];

	Subsys = XPmSubsystem_GetById(SubsysId);
	if ((NULL == Subsys) || ((u8)ONLINE != Subsys->State)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	switch (NODECLASS(DevId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPm_AddDevRequirement(Subsys, DevId, Flags, Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_SUBSYSTEM:
		TarSubsys = XPmSubsystem_GetById(DevId);
		if (NULL == TarSubsys) {
			Status = XPM_INVALID_SUBSYSID;
			goto done;
		}
		Status = XPmSubsystem_AddPermission(Subsys, TarSubsys, Flags);
		break;
	case (u32)XPM_NODECLASS_RESET:
		Rst = XPmReset_GetById(DevId);
		if (NULL == Rst) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		Status = XPmReset_AddPermission(Rst, Subsys, Flags);
		break;
	default:
		Status = XPm_PlatAddRequirement(Args, NumArgs);
		break;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

static int XPm_ProcessCmd(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	u32 ApiResponse[XPLMI_CMD_RESP_SIZE-1] = {0};
	const XPm_Subsystem *Subsystem = NULL;
	u32 SubsystemId = Cmd->SubsystemId;
	const u32 *Pload = Cmd->Payload;
	u32 CmdId = Cmd->CmdId & 0xFFU;
	u32 Len = Cmd->Len;
	const u32 CopySize = sizeof(ApiResponse);
	u32 SetAddress;
	u64 Address;

	PmDbg("Processing Cmd: 0x%x, SubsysId: 0x%x, IpiMask: 0x%x\r\n",
					Cmd->CmdId, SubsystemId, Cmd->IpiMask);

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (((NULL == Subsystem) || (Subsystem->State == (u8)OFFLINE)) &&
	    (XST_SUCCESS != IsOnSecondarySLR(SubsystemId))) {
		/* Subsystem must not be offline here */
		PmErr("Subsystem 0x%x is not present or offline\r\n", SubsystemId);
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if (XST_SUCCESS == IsOnSecondarySLR(SubsystemId)) {
		/*
		 * If API has been forwarded the Subsystem ID is set to 0 by
		 * default. This is a temporary solution to set the
		 * Subsystem ID on a secondary SLR.
		 */
		SubsystemId = PM_SUBSYS_PMC;
	}

	switch (CmdId) {
	case PM_API(PM_SET_WAKEUP_SOURCE):
		Status = XPm_SetWakeUpSource(SubsystemId, Pload[0], Pload[1], Pload[2]);
		break;
	case PM_API(PM_QUERY_DATA):
		Status = XPm_Query(Pload[0], Pload[1], Pload[2],
				   Pload[3], ApiResponse);
		break;
	case PM_API(PM_GET_CHIPID):
		Status = XPm_GetChipID(&ApiResponse[0], &ApiResponse[1]);
		break;
	case PM_API(PM_GET_API_VERSION):
		Status = XPm_GetApiVersion(ApiResponse);
		break;
	case PM_API(PM_REQUEST_WAKEUP):
		/* setAddress is encoded in the 1st bit of the low-word address */
		SetAddress = Pload[1] & 0x1U;
		/* addresses are word-aligned, ignore bit 0 */
		Address = ((u64) Pload[2]) << 32ULL;
		Address += Pload[1] & (~(u64)0x1U);
		Status = XPm_RequestWakeUp(SubsystemId, Pload[0],
					   SetAddress, Address,
					   Pload[3], Cmd->IpiReqType);
		break;
	case PM_API(PM_FORCE_POWERDOWN):
		Status = XPm_ForcePowerdown(SubsystemId, Pload[0], Pload[1],
					    Cmd->IpiReqType, Cmd->IpiMask);
		/**
		 *  Skip providing ack for the force power down command as it is
		 *  handled from the xilpm module itself.
		 */
		Cmd->AckInPLM = (u8)FALSE;
		break;
	case PM_API(PM_SYSTEM_SHUTDOWN):
		Status = XPm_SystemShutdown(SubsystemId, Pload[0], Pload[1], Cmd->IpiReqType);
		break;
	case PM_API(PM_SELF_SUSPEND):
		Status = XPm_SelfSuspend(SubsystemId, Pload[0],
					 Pload[1], (u8)Pload[2],
					 Pload[3], Pload[4]);
		break;
	case PM_API(PM_REQUEST_SUSPEND):
		Status = XPm_RequestSuspend(SubsystemId, Pload[0], Pload[1],
					    Pload[2], Pload[3], Cmd->IpiReqType);
		break;
	case PM_API(PM_ABORT_SUSPEND):
		Status = XPm_AbortSuspend(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_API(PM_CLOCK_SETRATE):
		Status = XPm_SetClockRate(Cmd->IpiMask, Pload[0], Pload[1]);
		break;
	case PM_API(PM_CLOCK_GETRATE):
		Status = XPm_GetClockRate(Pload[0], ApiResponse);
		break;
	case PM_API(PM_CLOCK_SETPARENT):
		Status = XPm_SetClockParent(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_API(PM_CLOCK_GETPARENT):
		Status = XPm_GetClockParent(Pload[0], ApiResponse);
		break;
	case PM_API(PM_CLOCK_ENABLE):
		Status = XPm_SetClockState(SubsystemId, Pload[0], 1);
		break;
	case PM_API(PM_CLOCK_DISABLE):
		Status = XPm_SetClockState(SubsystemId, Pload[0], 0);
		break;
	case PM_API(PM_CLOCK_GETSTATE):
		Status = XPm_GetClockState(Pload[0], ApiResponse);
		break;
	case PM_API(PM_CLOCK_SETDIVIDER):
		Status = XPm_SetClockDivider(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_API(PM_CLOCK_GETDIVIDER):
		Status = XPm_GetClockDivider(Pload[0], ApiResponse);
		break;
	case PM_API(PM_PLL_SET_PARAMETER):
		Status = XPm_SetPllParameter(SubsystemId, Pload[0], Pload[1], Pload[2]);
		break;
	case PM_API(PM_PLL_GET_PARAMETER):
		Status = XPm_GetPllParameter(Pload[0], Pload[1], ApiResponse);
		break;
	case PM_API(PM_PLL_SET_MODE):
		Status = XPm_SetPllMode(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_API(PM_PLL_GET_MODE):
		Status = XPm_GetPllMode(Pload[0], ApiResponse);
		break;
	case PM_API(PM_RESET_ASSERT):
		Status = XPm_SetResetState(SubsystemId, Pload[0], Pload[1],
					   Cmd->IpiReqType);
		break;
	case PM_API(PM_RESET_GET_STATUS):
		Status = XPm_GetResetState(Pload[0], ApiResponse);
		break;
	case PM_API(PM_REQUEST_NODE):
		Status = XPm_RequestDevice(SubsystemId, Pload[0], Pload[1],
				Pload[2], Pload[3], Cmd->IpiReqType);
		break;
	case PM_API(PM_RELEASE_NODE):
		Status = XPm_ReleaseDevice(SubsystemId, Pload[0], Cmd->IpiReqType);
		break;
	case PM_API(PM_SET_REQUIREMENT):
		Status = XPm_SetRequirement(SubsystemId, Pload[0], Pload[1], Pload[2], Pload[3]);
		break;
	case PM_API(PM_SET_MAX_LATENCY):
		Status = XPm_SetMaxLatency(SubsystemId, Pload[0],
				Pload[1]);
		break;
	case PM_API(PM_GET_NODE_STATUS):
		Status = XPm_GetDeviceStatus(SubsystemId, Pload[0], (XPm_DeviceStatus *)ApiResponse);
		break;
	case PM_API(PM_FEATURE_CHECK):
		Status = XPm_FeatureCheck(Pload[0], ApiResponse);
		break;
	case PM_API(PM_ADD_SUBSYSTEM):
		Status = XPm_AddSubsystem(Pload[0]);
		break;
	case PM_API(PM_DESTROY_SUBSYSTEM):
		Status = XPm_DestroySubsystem(Pload[0]);
		break;
	case PM_API(PM_DESCRIBE_NODES):
		Status = XPm_DescribeNodes(Len);
		break;
	case PM_API(PM_ADD_NODE):
		Status = XPm_AddNode(&Pload[0], Len);
		break;
	case PM_API(PM_ADD_NODE_PARENT):
		Status = XPm_AddNodeParent(&Pload[0], Len);
		break;
	case PM_API(PM_ADD_NODE_NAME):
		Status = XPm_AddNodeName(&Pload[0], Len);
		break;
	case PM_API(PM_ADD_REQUIREMENT):
		Status = XPm_AddRequirement(&Pload[0], Len);
		break;
	case PM_API(PM_INIT_FINALIZE):
		Status = XPm_InitFinalize(SubsystemId);
		break;
	case PM_API(PM_ACTIVATE_SUBSYSTEM):
		Status = XPm_ActivateSubsystem(SubsystemId, Pload[0]);
		break;
	case PM_API(PM_ISO_CONTROL):
		Status = XPm_IsoControl(Pload[0], Pload[1]);
		break;
	case PM_API(PM_GET_OP_CHARACTERISTIC):
		Status = XPm_GetOpCharacteristic(Pload[0], Pload[1],
						 ApiResponse);
		break;
	case PM_API(PM_REGISTER_NOTIFIER):
		Status = XPm_RegisterNotifier(SubsystemId, Pload[0],
					      Pload[1], Pload[2],
					      Pload[3], Cmd->IpiMask);
		break;
	case PM_API(PM_IOCTL):
		Status = XPm_DevIoctl(SubsystemId, Pload[0], (pm_ioctl_id) Pload[1],
				      Pload[2], Pload[3], Pload[4],
				      ApiResponse, Cmd->IpiReqType);
		break;
	default:
		Status = XPm_PlatProcessCmd(Cmd, ApiResponse);
		break;
	}

	if (XST_SUCCESS == Status) {
		Cmd->ResumeHandler = NULL;
	} else {
		PmErr("Error 0x%x while processing command 0x%x\r\n",
				Status, Cmd->CmdId);
		PmDbg("Command payload: 0x%x, 0x%x, 0x%x, 0x%x\r\n",
			Pload[0], Pload[1], Pload[2], Pload[3]);
	}

	/* First word of the response is status */
	Cmd->Response[0] = (u32)Status;
	Status = Xil_SMemCpy(&Cmd->Response[1], CopySize, ApiResponse, CopySize, CopySize);
	if (XST_SUCCESS != Status) {
		PmErr("Error 0x%x while copying the Cmd 0x%x return payload\r\n",
				Status, Cmd->CmdId);
		goto done;
	}

	/* Restore the CmdHandler status */
	Status = (int)Cmd->Response[0];

done:
	if (XST_SUCCESS != Status) {
		PmErr("Err Code: 0x%x\r\n", Status);
	}
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

static void XPm_CheckLastResetReason(void)
{
	u32 RegVal;

	/* Read LAST_RESET_REASON_REG register value for checking CRP_RESET_REASON */
	PmIn32(LAST_RESET_REASON_REG, RegVal);

	/* Mask out CRP_RESET_REASON value */
	ResetReason = RegVal & (CRP_RESET_REASON_MASK);

	return;
}

/****************************************************************************/
/**
 * @brief  Initialize XilPM library
 *
 * @param  IpiInst	IPI instance
 * @param  RequestCb 	Pointer to the request calbback handler
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_Init(void (*const RequestCb)(const u32 SubsystemId, const XPmApiCbId_t EventId, u32 *Payload),
		 int (*const RestartCb)(u32 ImageId, u32 *FuncId))
{
	XStatus Status = XST_FAILURE;
	u32 i;

	PmInfo("Initializing XilPM Library\n\r");

	/* Check last reset reason */
	XPm_CheckLastResetReason();

	u32 PmcIPORMask = (CRP_RESET_REASON_ERR_POR_MASK |
			   CRP_RESET_REASON_SLR_POR_MASK |
			   CRP_RESET_REASON_SW_POR_MASK);
	u32 SysResetMask = (CRP_RESET_REASON_SLR_SYS_MASK |
			    CRP_RESET_REASON_SW_SYS_MASK |
			    CRP_RESET_REASON_ERR_SYS_MASK |
			    CRP_RESET_REASON_DAP_SYS_MASK);
	u32 NoCResetsMask = CRP_RST_NONPS_NOC_POR_MASK |
			CRP_RST_NONPS_NPI_RESET_MASK |
			CRP_RST_NONPS_NOC_RESET_MASK |
			CRP_RST_NONPS_SYS_RST_1_MASK |
			CRP_RST_NONPS_SYS_RST_2_MASK |
			CRP_RST_NONPS_SYS_RST_3_MASK;

	if (0U != (ResetReason & SysResetMask)) {

		XPm_DisableSkipHC();

		/* Assert PL and PS POR */
		PmRmw32(CRP_RST_PS, CRP_RST_PS_PL_POR_MASK | CRP_RST_PS_PS_POR_MASK,
					CRP_RST_PS_PL_POR_MASK | CRP_RST_PS_PS_POR_MASK);

		/* Assert NOC POR, NPI Reset, Sys Resets */
		PmRmw32(CRP_RST_NONPS, NoCResetsMask, NoCResetsMask);

		Status = XPm_PlatInit();
		if (XST_SUCCESS != Status) {
			goto done;
		}

	}

	/*
	 * Clear DomainInitStatusReg in case of internal PMC_POR. Since PGGS0
	 * value is not cleared in case of internal POR.
	 */
	if (0U != (ResetReason & PmcIPORMask)) {
		XPm_Out32(XPM_DOMAIN_INIT_STATUS_REG, 0);
	}

	PmRequestCb = RequestCb;

	/* Register command handlers with eFSBL */
	for (i = 1; i < XPlmi_Pm.CmdCnt; i++) {
		XPlmi_PmCmds[i].Handler = XPm_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Pm);

	XPm_PsmModuleInit();

	Status = XPm_RegisterWakeUpHandlers();
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XPmSubsystem_Add(PM_SUBSYS_PMC);

	PmRestartCb = RestartCb;

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
	XPM_EXPORT_CMD(PM_ISO_CONTROL, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;

	if (((u32)XPM_NODECLASS_ISOLATION != NODECLASS(NodeId)) ||
	    ((u32)XPM_NODEIDX_ISO_MAX <= NODEINDEX(NodeId))) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Status = XPmDomainIso_Control(NODEINDEX(NodeId), Enable);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
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
	XPM_EXPORT_CMD(PM_GET_API_VERSION, XPLMI_CMD_ARG_CNT_ZERO, XPLMI_CMD_ARG_CNT_ZERO);

	*Version = PM_VERSION;
	return XST_SUCCESS;
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
XStatus XPm_SetWakeUpSource(const u32 SubsystemId, const u32 TargetNodeId,
			    const u32 SourceNodeId, const u32 Enable)
{
	XPM_EXPORT_CMD(PM_SET_WAKEUP_SOURCE, XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);
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

	Periph->WakeProcId = TargetNodeId;

	if (NULL != Periph->PeriphOps->SetWakeupSource) {
		Periph->PeriphOps->SetWakeupSource(Periph, (u8)(Enable));
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
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
	default:
		Status = XPm_PlatQuery(Qid, Arg1, Arg2, Arg3, Output);
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
	XPM_EXPORT_CMD(PM_CLOCK_ENABLE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XPM_EXPORT_CMD(PM_CLOCK_DISABLE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
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
	XPM_EXPORT_CMD(PM_CLOCK_GETSTATE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
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
	XPM_EXPORT_CMD(PM_CLOCK_SETDIVIDER, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
	XPM_EXPORT_CMD(PM_CLOCK_GETDIVIDER, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
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
	XPM_EXPORT_CMD(PM_CLOCK_SETPARENT, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
	XPM_EXPORT_CMD(PM_CLOCK_GETPARENT, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
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
	XPM_EXPORT_CMD(PM_PLL_SET_PARAMETER, XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);
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
	XPM_EXPORT_CMD(PM_PLL_GET_PARAMETER, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
	XPM_EXPORT_CMD(PM_PLL_SET_MODE, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
	XPM_EXPORT_CMD(PM_PLL_GET_MODE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
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
	XPM_EXPORT_CMD(PM_DESCRIBE_NODES, XPLMI_CMD_ARG_CNT_THREE,
		XPLMI_CMD_ARG_CNT_THREE);
	XStatus Status = XST_FAILURE;

	if(NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* TODO: missing implementation */
	Status = XST_SUCCESS;

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
XStatus XPm_AddNodeParent(const u32 *Args, u32 NumArgs)
{
	XPM_EXPORT_CMD(PM_ADD_NODE_PARENT, XPLMI_CMD_ARG_CNT_TWO, XPLMI_UNLIMITED_ARG_CNT);
	XStatus Status = XST_FAILURE;
	u32 Id = Args[0];
	const u32 *Parents;
	u32 NumParents;

	if (NumArgs < 2U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NumParents = NumArgs-1U;
	Parents = &Args[1];

	switch (NODECLASS(Id)) {
	case (u32)XPM_NODECLASS_POWER:
		Status = XPmPower_AddParent(Id, Parents, NumParents);
		break;
	case (u32)XPM_NODECLASS_CLOCK:
		if (ISPLL(Id)) {
			Status = XPmClockPll_AddParent(Id, Parents, (u8)NumParents);
		} else {
			Status = XPmClock_AddParent(Id, Parents, (u8)NumParents);
		}
		break;
	case (u32)XPM_NODECLASS_RESET:
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_MEMIC:
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_STMIC:
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_DEVICE:
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
static XStatus XPm_AddNodeReset(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 ResetId, ControlReg;
	u8 Shift, Width, ResetType, NumParents;
	const u32 *Parents;

	if (NumArgs < 4U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	ResetId = Args[0];
	ControlReg = Args[1];
	Shift = (u8)(Args[2] & 0xFFU);
	Width = (u8)((Args[2] >> 8U) & 0xFFU);
	ResetType = (u8)((Args[2] >> 16U) & 0xFFU);
	NumParents = (u8)((Args[2] >> 24U) & 0xFFU);
	Parents = &Args[3];

	Status = XPmReset_AddNode(ResetId, ControlReg, Shift, Width, ResetType, NumParents, Parents);

done:
	return Status;
}

static XStatus AddProcDevice(const u32 *Args, u32 PowerId)
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

	if (Index >= (u32)XPM_NODEIDX_DEV_MAX) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_CORE_PSM:
		Psm = (XPm_Psm *)XPm_AllocBytes(sizeof(XPm_Psm));
		if (NULL == Psm) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPsm_Init(Psm, Ipi, BaseAddr, Power, NULL, NULL);
		break;
	case (u32)XPM_NODETYPE_DEV_CORE_APU:
		ApuCore = (XPm_ApuCore *)XPm_AllocBytes(sizeof(XPm_ApuCore));
		if (NULL == ApuCore) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmApuCore_Init(ApuCore, DeviceId, Ipi, BaseAddr, Power, NULL, NULL);
		break;
	case (u32)XPM_NODETYPE_DEV_CORE_RPU:
		RpuCore = (XPm_RpuCore *)XPm_AllocBytes(sizeof(XPm_RpuCore));
		if (NULL == RpuCore) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmRpuCore_Init(RpuCore, DeviceId, Ipi, BaseAddr, Power, NULL, NULL);
		break;
	case (u32)XPM_NODETYPE_DEV_CORE_PMC:
		Pmc = (XPm_Pmc *)XPm_AllocBytes(sizeof(XPm_Pmc));
		if (NULL == Pmc) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPmc_Init(Pmc, DeviceId, 0, BaseAddr, Power, NULL, NULL);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus AddPeriphDevice(const u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	u32 GicProxyMask;
	u32 GicProxyGroup;
	XPm_Periph *PeriphDevice;
	XPm_Device *Device;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];
	GicProxyMask = Args[3];
	GicProxyGroup = Args[4];
	Type = NODETYPE(DeviceId);

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	if (((u32)XPM_NODETYPE_DEV_GGS == Type) ||
	    ((u32)XPM_NODETYPE_DEV_PGGS == Type)) {
		Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		Status = XPmVirtDev_DeviceInit(Device, DeviceId, Power);
	} else if ((u32)XPM_NODETYPE_DEV_HB_MON == Type) {
		Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		Status = XPmHbMonDev_Init(Device, DeviceId, Power);
	} else {
		PeriphDevice = (XPm_Periph *)XPm_AllocBytes(sizeof(XPm_Periph));
		if (NULL == PeriphDevice) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		Status = XPmPeriph_Init(PeriphDevice, DeviceId, BaseAddr, Power, NULL, NULL,
					GicProxyMask, GicProxyGroup);
	}

done:
	return Status;
}

static XStatus AddMemDevice(const u32 *Args, u32 PowerId)
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

	if ((u32)XPM_NODEIDX_DEV_MAX <= Index) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	if (IS_MEM_DEV_TYPE(Type)) {
		Device = (XPm_MemDevice *)XPm_AllocBytes(sizeof(XPm_MemDevice));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmMemDevice_Init(Device, DeviceId, BaseAddr, Power, NULL, NULL, StartAddr, EndAddr);
	} else {
		Status = XST_INVALID_PARAM;
	}

done:
	return Status;
}
static XStatus AddPlDevice(const u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Index;
	XPm_Power *Power;
	u32 BaseAddr;
	XPm_PlDevice *PlDevice;

	DeviceId = Args[0];
	BaseAddr = Args[2];

	Index = NODEINDEX(DeviceId);

	Power = XPmPower_GetById(PowerId);

	if ((u32)XPM_NODEIDX_DEV_PLD_MAX <= Index) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/*
	 * Note: This function is executed as part of pm_add_node cmd triggered
	 * through CDO. Since there's a possibility of the same RM (hence CDO)
	 * being executed multiple times, we should not error out on addition
	 * of same node multiple times. Memory is allocated only if node is not
	 * present in database. Since PLD0 represents static image and
	 * not RM, we shouldn't allow it to be re-added.
	 */
	PlDevice = (XPm_PlDevice *)XPmDevice_GetById(DeviceId);
	if (NULL == PlDevice) {
		PlDevice = (XPm_PlDevice *)XPm_AllocBytes(sizeof(XPm_PlDevice));
		if (NULL == PlDevice) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	} else {
		if ((u32)XPM_NODEIDX_DEV_PLD_0 == Index) {
			Status = XST_DEVICE_BUSY;
			goto done;
		}
		PmInfo("0x%x Device is already added\r\n", DeviceId);
	}

	Status = XPmPlDevice_Init(PlDevice, DeviceId, BaseAddr, Power, NULL, NULL);

done:
	return Status;
}

static XStatus AddMemCtrlrDevice(const u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	XPm_Device *Device;
	XPm_MemCtrlrDevice *MemCtrlr;
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
	case (u32)XPM_NODETYPE_DEV_HBM:
		Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmDevice_Init(Device, DeviceId, BaseAddr,
					Power, NULL, NULL);
		break;
	case (u32)XPM_NODETYPE_DEV_DDR:
		MemCtrlr = (XPm_MemCtrlrDevice *)XPm_AllocBytes(sizeof(XPm_MemCtrlrDevice));
		if (NULL == MemCtrlr) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmDevice_Init(&MemCtrlr->Device, DeviceId, BaseAddr,
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
static XStatus XPm_AddDevice(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 SubClass;
	u32 PowerId = 0;

	if (NumArgs < 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DeviceId = Args[0];
	SubClass = NODESUBCLASS(DeviceId);

	if (NumArgs > 1U) {
		/*
		 * Check for Num Args < 3U as device specific (except PLDevice)
		 * AddNode functions currently don't implement any NumArgs checks
		 */
		if (NumArgs < 3U) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		PowerId = Args[1];
		if (NULL == XPmPower_GetById(PowerId)) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
	}

	switch (SubClass) {
	case (u32)XPM_NODESUBCL_DEV_CORE:
		Status = AddProcDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_PERIPH:
		Status = AddPeriphDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_MEM:
		Status = AddMemDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_PL:
		Status = AddPlDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_MEM_CTRLR:
		Status = AddMemCtrlrDevice(Args, PowerId);
		break;
	default:
		Status = XPm_PlatAddDevice(Args, NumArgs);
		break;
	}

	if (NumArgs > 6U) {
		Status = AddDevAttributes(Args, NumArgs);
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
static XStatus XPm_AddClockSubNode(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 ClockId, ControlReg, Type, Flags;
	u8 Param1, Param2;

	if (5U > NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	ClockId = Args[0];
	if (ISOUTCLK(ClockId)) {
		Type = Args[1];
		ControlReg = Args[2];
		Param1 =  (u8)(Args[3] & 0xFFU);
		Param2 =  (u8)((Args[3] >> 8U) & 0xFFU);
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
static XStatus XPm_AddNodeClock(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 ClockId, ControlReg;
	u32 PowerDomainId;
	u8 TopologyType, NumCustomNodes = 0U, NumParents, ClkFlags;

	if (4U > NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	ClockId = Args[0];

	if (NODETYPE(ClockId) == (u32)XPM_NODETYPE_CLOCK_SUBNODE) {
		Status = XPm_AddClockSubNode(Args, NumArgs);
		goto done;
	}
	if (ISOUTCLK(ClockId) || ISREFCLK(ClockId) || ISPLL(ClockId)) {
		ControlReg = Args[1];
		TopologyType = (u8)(Args[2] & 0xFFU);
		NumCustomNodes = (u8)((Args[2] >> 8U) & 0xFFU);
		NumParents = (u8)((Args[2] >> 16U) & 0xFFU);
		ClkFlags = (u8)((Args[2] >> 24U) & 0xFFU);
		PowerDomainId = Args[3];
		if (ISPLL(ClockId)) {
			const u16 *Offsets = (u16 *)&Args[4];
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
XStatus XPm_AddNodeName(const u32 *Args, u32 NumArgs)
{
	XPM_EXPORT_CMD(PM_ADD_NODE_NAME, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_FIVE);
	XStatus Status = XST_FAILURE;
	u32 NodeId;
	char Name[MAX_NAME_BYTES] = {0};
	u32 i = 0U, j = 0U;
	const u32 CopySize = 4U;

	if (0U == NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	NodeId = Args[0];
	if (ISOUTCLK(NodeId) || ISREFCLK(NodeId) || ISPLL(NodeId)) {
		for (i = 1U; i < NumArgs; i++) {
			Status = Xil_SMemCpy(&Name[j], CopySize, (char *)((UINTPTR)&Args[i]), CopySize, CopySize);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			j += 4U;
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
static XStatus XPm_AddNodePower(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
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
	XPm_CpmDomain *CpmDomain;

	if (1U > NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PowerId = Args[0];
	PowerType = NODETYPE(PowerId);
	Width = (u8)(Args[1] >> 8) & 0xFFU;
	Shift = (u8)(Args[1] & 0xFFU);
	ParentId = Args[2];

	if ((NODEINDEX(PowerId) >= (u32)XPM_NODEIDX_POWER_MAX) &&
	    ((u32)XPM_NODETYPE_POWER_REGULATOR != PowerType)) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else {
		/* Required by MISRA */
	}

	BitMask = BITNMASK(Shift, Width);

	if ((ParentId != (u32)XPM_NODEIDX_POWER_MIN) &&
	    ((u32)XPM_NODETYPE_POWER_RAIL != PowerType) &&
#ifdef VERSAL_ENABLE_DOMAIN_CONTROL_GPIO
		((u32)XPM_NODETYPE_POWER_DOMAIN_CTRL != PowerType) &&
#endif
	    ((u32)XPM_NODETYPE_POWER_REGULATOR != PowerType)) {
		if (NODECLASS(ParentId) != (u32)XPM_NODECLASS_POWER) {
			Status = XST_INVALID_PARAM;
			goto done;
		} else if (NODEINDEX(ParentId) >= (u32)XPM_NODEIDX_POWER_MAX) {
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
	case (u32)XPM_NODETYPE_POWER_ISLAND:
	case (u32)XPM_NODETYPE_POWER_ISLAND_XRAM:
		Power = (XPm_Power *)XPm_AllocBytes(sizeof(XPm_Power));
		if (NULL == Power) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPower_Init(Power, PowerId, BitMask,
			PowerParent);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		if (9U <= NumArgs) {
			XPmPower_SetPsmRegInfo(Power, Args);
		}
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_PMC:
		PmcDomain =
			(XPm_PmcDomain *)XPm_AllocBytes(sizeof(XPm_PmcDomain));
		if (NULL == PmcDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPmcDomain_Init((XPm_PmcDomain *)PmcDomain, PowerId,
					   PowerParent);
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_PS_FULL:
		PsFpDomain =
			(XPm_PsFpDomain *)XPm_AllocBytes(sizeof(XPm_PsFpDomain));
		if (NULL == PsFpDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPsFpDomain_Init(PsFpDomain, PowerId,
					    BitMask, PowerParent, &Args[3], (NumArgs - 3U));
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_PS_LOW:
		PsLpDomain =
			(XPm_PsLpDomain *)XPm_AllocBytes(sizeof(XPm_PsLpDomain));
		if (NULL == PsLpDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPsLpDomain_Init(PsLpDomain, PowerId,
					    BitMask, PowerParent,
					    &Args[3], (NumArgs - 3U));
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_NOC:
		NpDomain = (XPm_NpDomain *)XPm_AllocBytes(sizeof(XPm_NpDomain));
		if (NULL == NpDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmNpDomain_Init(NpDomain, PowerId, 0x00000000,
					  PowerParent);
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_PL:
		PlDomain = (XPm_PlDomain *)XPm_AllocBytes(sizeof(XPm_PlDomain));
		if (NULL == PlDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPlDomain_Init(PlDomain, PowerId, 0x00000000,
					  PowerParent, &Args[3], (NumArgs - 3U));
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_CPM:
		CpmDomain = (XPm_CpmDomain *)XPm_AllocBytes(sizeof(XPm_CpmDomain));
		if (NULL == CpmDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmCpmDomain_Init(CpmDomain, PowerId, 0x00000000, PowerParent,
					   &Args[3], (NumArgs - 3U));
		break;
	default:
		Status = XPm_PlatAddNodePower(Args, NumArgs);
		break;
	}

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
static XStatus XPm_AddNodeMonitor(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId, BaseAddress, NodeType;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeId = Args[0];
	BaseAddress = Args[2];


	if ((u32)XPM_NODESUBCL_MONITOR_SYSMON != NODESUBCLASS(NodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeType = NODETYPE(NodeId);

	if ((((u32)XPM_NODETYPE_MONITOR_SYSMON_PMC != NodeType) &&
	    ((u32)XPM_NODETYPE_MONITOR_SYSMON_PS != NodeType) &&
	    ((u32)XPM_NODETYPE_MONITOR_SYSMON_CPM5N != NodeType) &&
	    ((u32)XPM_NODETYPE_MONITOR_SYSMON_NPD != NodeType)) ||
	    ((u32)XPM_NODEIDX_MONITOR_MAX <= NODEINDEX(NodeId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPm_SetSysmonNode(NodeId, BaseAddress);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows adding node to clock, power, reset, mio
 *			or device topology
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
XStatus XPm_AddNode(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Id = Args[0];

	switch (NODECLASS(Id)) {
	case (u32)XPM_NODECLASS_POWER:
		Status = XPm_AddNodePower(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_CLOCK:
		Status = XPm_AddNodeClock(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_RESET:
		Status = XPm_AddNodeReset(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPm_AddDevice(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_MONITOR:
		Status = XPm_AddNodeMonitor(Args, NumArgs);
		break;
	default:
		Status = XPm_PlatAddNode(Args, NumArgs);
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function determines the base address of the devices.
 *
 * @param  DeviceId	Device Id from the topology cdo
 * @param  BaseAddr	Pointer to store base address
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetDeviceBaseAddr(u32 DeviceId, u32 *BaseAddr)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	const XPm_ApuCore *ApuCore;
	const XPm_RpuCore *RpuCore;
	const XPm_MemDevice *MemDev;
	const XPm_Pmc *PmcCore;
	const XPm_Psm *PsmCore;
	u32 SubClass = NODESUBCLASS(DeviceId);
	u32 Type = NODETYPE(DeviceId);

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if (NULL == BaseAddr) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_DEV_CORE == SubClass) {
		/* Get base address using CORE device subclass */
		if ((u32)XPM_NODETYPE_DEV_CORE_APU == Type) {
			/* Using APU core */
			ApuCore = (XPm_ApuCore *)Device;
			*BaseAddr = ApuCore->FpdApuBaseAddr;
		} else if ((u32)XPM_NODETYPE_DEV_CORE_RPU == Type) {
			/* Using RPU core */
			RpuCore = (XPm_RpuCore *)Device;
			*BaseAddr = RpuCore->RpuBaseAddr;
		} else if ((u32)XPM_NODETYPE_DEV_CORE_PMC == Type) {
			/* using PMC core */
			PmcCore = (XPm_Pmc *)Device;
			*BaseAddr = PmcCore->PmcGlobalBaseAddr;
		} else if ((u32)XPM_NODETYPE_DEV_CORE_PSM == Type) {
			/* using PSM core */
			PsmCore = (XPm_Psm *)Device;
			*BaseAddr = PsmCore->PsmGlobalBaseAddr;
		} else {
			/* Required by MISRA */
		}
	} else if (((u32)XPM_NODESUBCL_DEV_MEM == SubClass) && IS_MEM_DEV_TYPE(Type)) {
		/* Get base address using MEM device subclass */
		MemDev = (XPm_MemDevice *)Device;
		*BaseAddr = MemDev->StartAddress;
	} else {
		/* Get base address using node class*/
		*BaseAddr = Device->Node.BaseAddress;
	}

	Status = XST_SUCCESS;

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
XStatus XPm_RegisterNotifier(const u32 SubsystemId, const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 Enable,
			 const u32 IpiMask)
{
	XPM_EXPORT_CMD(PM_REGISTER_NOTIFIER, XPLMI_CMD_ARG_CNT_FOUR,
		XPLMI_CMD_ARG_CNT_FOUR);
	XStatus Status = XST_FAILURE;
	XPm_Subsystem* Subsystem = NULL;

	/* Register/Unregister on secondary SLRs */
	Status = XPmNotifier_PlatHandleSsit(SubsystemId, NodeId, Event, Enable);
	if ((XST_SUCCESS != Status) ||
	    (/* XPLMI_SSIT_MASTER_SLR_INDEX */ 0U != XPm_PlatGetSlrIndex())) {
		/* Return on secondary SLRs, regardless of status */
		goto done;
	}

	/* Validate SubsystemId */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	/* Only Event, Device and Power Nodes are supported */
	if (((u32)XPM_NODECLASS_EVENT != NODECLASS(NodeId)) &&
	    ((u32)XPM_NODECLASS_DEVICE != NODECLASS(NodeId)) &&
	    ((u32)XPM_NODECLASS_POWER != NODECLASS(NodeId))) {
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
		goto done;
	}
	if (((u32)XPM_NODECLASS_EVENT == NODECLASS(NodeId)) &&
	    (((0U != Wake) && (1U != Wake)) ||
	     ((0U != Enable) && (1U != Enable)))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (0U == Enable) {
		Status = XPmNotifier_Unregister(Subsystem, NodeId, Event);
	} else {
		Status = XPmNotifier_Register(Subsystem, NodeId, Event, Wake,
					      IpiMask);
	}

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
XStatus XPm_SetResetState(const u32 SubsystemId, const u32 ResetId,
			  const u32 Action, const u32 CmdType)
{
	XPM_EXPORT_CMD(PM_RESET_ASSERT, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
	if ((((u32)XPM_NODESUBCL_RESET_PERIPHERAL == SubClass) &&
	     ((u32)XPM_NODETYPE_RESET_PERIPHERAL == SubType)) ||
	    (((u32)XPM_NODESUBCL_RESET_DBG == SubClass) &&
	     ((u32)XPM_NODETYPE_RESET_DBG == SubType))) {
		/* Check if subsystem is allowed to access requested reset */
		Status = XPm_IsAccessAllowed(SubsystemId, ResetId);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	} else {
		/*
		 * Only a certain list of resets is allowed to
		 * use permissions policy.
		 *
		 * If with in this list, then check reset to
		 * permission policy for access.
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
	XPM_EXPORT_CMD(PM_RESET_GET_STATUS, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;
	const XPm_ResetNode* Reset;

	Reset = XPmReset_GetById(ResetId);
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	*State = Reset->Ops->GetState(Reset);

	Status = XST_SUCCESS;

done:
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
	XPM_EXPORT_CMD(PM_REQUEST_NODE, XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);
	XStatus Status = XST_FAILURE;

	/* Warning Fix */
	(void) (Ack);

	u32 NumArgs = 3U;
	u32 ArgBuf[3U];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = Capabilities;
	ArgBuf[2] = QoS;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_REQUEST_NODE, ArgBuf, NumArgs,
				    CmdType, NULL);
	if (XST_DEVICE_NOT_FOUND != Status){
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	Status = XPmDevice_Request(SubsystemId, DeviceId, Capabilities,
				   QoS, CmdType);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
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
 * @param CmdType	IPI command request type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_ReleaseDevice(const u32 SubsystemId, const u32 DeviceId,
			  const u32 CmdType)
{
	XPM_EXPORT_CMD(PM_RELEASE_NODE, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem* Subsystem = NULL;
	const XPm_Device* Device = NULL;
	u32 Usage = 0U;

	u32 NumArgs = 1U;
	u32 ArgBuf[1U];
	ArgBuf[0] = DeviceId;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_RELEASE_NODE, ArgBuf, NumArgs,
				    CmdType, NULL);
	if (XST_DEVICE_NOT_FOUND != Status) {
		/* API is forwarded, nothing else to be done */
		goto done;
	}

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
	XPM_EXPORT_CMD(PM_SET_REQUIREMENT, XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);
	XStatus Status = XST_FAILURE;

	/* Warning Fix */
	(void) (Ack);

	u32 NumArgs = 3U;
	u32 ArgBuf[3U];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = Capabilities;
	ArgBuf[2] = QoS;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_SET_REQUIREMENT, ArgBuf, NumArgs,
				    NO_HEADER_CMDTYPE, NULL);
	if (XST_DEVICE_NOT_FOUND != Status) {
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmDevice_SetRequirement(SubsystemId, DeviceId,
					  Capabilities, QoS);
done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
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
XStatus XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
		      const u32 Latency)
{
	XPM_EXPORT_CMD(PM_SET_MAX_LATENCY, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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
XStatus XPm_GetDeviceStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;

	switch(NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPmDevice_GetStatus(SubsystemId, DeviceId, DeviceStatus);
		break;
	case (u32)XPM_NODECLASS_POWER:
		Status = XPmPower_GetStatus(SubsystemId, DeviceId, DeviceStatus);
		break;
	case (u32)XPM_NODECLASS_SUBSYSTEM:
		Status = XPmSubsystem_GetStatus(SubsystemId, DeviceId,
						DeviceStatus);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns supported version of the given API.
 *
 * @param  ApiId	API ID to check
 * @param  Version	pointer to array of 4 words
 *  - version[0] - EEMI API version number
 *  - version[1] - lower 32-bit bitmask of IOCTL or QUERY ID
 *  - version[2] - upper 32-bit bitmask of IOCTL or Query ID
 *  - Only PM_FEATURE_CHECK version 2 supports 64-bit bitmask
 *  - i.e. version[1] and version[2]
 * @return XST_SUCCESS if successful else XST_NO_FEATURE.
 *
 * @note   Remove CDO-only commands from versioning as it is for internal
 * use only, so no need to consider for versioing.
 *
 ****************************************************************************/
XStatus XPm_FeatureCheck(const u32 ApiId, u32 *const Version)
{
	XStatus Status = XST_FAILURE;

	if (NULL == Version) {
		Status = XPM_ERR_VERSION;
		goto done;
	}

	switch (ApiId) {
	case PM_API(PM_GET_API_VERSION):
	case PM_API(PM_GET_NODE_STATUS):
	case PM_API(PM_REQUEST_SUSPEND):
	case PM_API(PM_ABORT_SUSPEND):
	case PM_API(PM_REQUEST_WAKEUP):
	case PM_API(PM_SET_WAKEUP_SOURCE):
	case PM_API(PM_SYSTEM_SHUTDOWN):
	case PM_API(PM_SET_REQUIREMENT):
	case PM_API(PM_SET_MAX_LATENCY):
	case PM_API(PM_RESET_ASSERT):
	case PM_API(PM_RESET_GET_STATUS):
	case PM_API(PM_INIT_FINALIZE):
	case PM_API(PM_GET_CHIPID):
	case PM_API(PM_CLOCK_ENABLE):
	case PM_API(PM_CLOCK_DISABLE):
	case PM_API(PM_CLOCK_GETSTATE):
	case PM_API(PM_CLOCK_SETDIVIDER):
	case PM_API(PM_CLOCK_GETDIVIDER):
	case PM_API(PM_CLOCK_SETPARENT):
	case PM_API(PM_CLOCK_GETPARENT):
	case PM_API(PM_CLOCK_SETRATE):
	case PM_API(PM_CLOCK_GETRATE):
	case PM_API(PM_PLL_SET_PARAMETER):
	case PM_API(PM_PLL_GET_PARAMETER):
	case PM_API(PM_PLL_SET_MODE):
	case PM_API(PM_PLL_GET_MODE):
		*Version = XST_API_BASE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_FEATURE_CHECK):
		*Version = XST_API_PM_FEATURE_CHECK_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_REGISTER_NOTIFIER):
		*Version = XST_API_REG_NOTIFIER_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_SELF_SUSPEND):
		*Version = XST_API_SELF_SUSPEND_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_FORCE_POWERDOWN):
		*Version = XST_API_FORCE_POWERDOWN_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_REQUEST_NODE):
		*Version = XST_API_REQUEST_NODE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_RELEASE_NODE):
		*Version = XST_API_RELEASE_NODE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_GET_OP_CHARACTERISTIC):
		Version[0] = XST_API_GET_OP_CHAR_VERSION;
		Version[1] = (u32)(PM_GET_OP_CHAR_FEATURE_BITMASK);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPm_PlatFeatureCheck(ApiId, Version);
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
	XPM_EXPORT_CMD(PM_ADD_SUBSYSTEM, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;
	Status = XPmSubsystem_Add(SubsystemId);

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
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
	XPM_EXPORT_CMD(PM_DESTROY_SUBSYSTEM, XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);
	XStatus Status = XST_FAILURE;
	Status = XPmSubsystem_Destroy(SubsystemId);

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
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
XStatus XPm_InitFinalize(const u32 SubsystemId)
{
	return XPmSubsystem_InitFinalize(SubsystemId);
}

/****************************************************************************/
/**
 * @brief  This function determines the Subsystem Id based upon power domains.
 *
 * @param  ImageId	ImageId of the CDO
 *
 * @return Subsystem Id to be used by PLM
 *
 * @note   None
 *
 ****************************************************************************/
u32 XPm_GetSubsystemId(u32 ImageId)
{
	u32 Class = NODECLASS(ImageId);
	u32 SubClass = NODESUBCLASS(ImageId);
	u32 SubsystemId;

	if (((u32)XPM_NODECLASS_DEVICE == Class) &&
	    (((u32)XPM_NODESUBCL_DEV_PL == SubClass) ||
	    ((u32)XPM_NODESUBCL_DEV_AIE == SubClass))) {
		/* Use PMC Subsystem Id for PLD images */
		SubsystemId = PM_SUBSYS_PMC;
	} else if ((u32)XPM_NODECLASS_POWER == Class) {
		/* Use PMC Subsystem Id for power domain CDOs */
		SubsystemId = PM_SUBSYS_PMC;
	} else {
		/* Use given Image Id as Subsystem Id */
		SubsystemId = ImageId;
	}

	return SubsystemId;
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
	XPM_EXPORT_CMD(PM_ABORT_SUSPEND, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;

	PmInfo("(%lu, %lu)\r\n", Reason, DeviceId);
	if (NULL == XPmSubsystem_GetById(SubsystemId)) {
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

	if ((Reason < ABORT_REASON_MIN) || (Reason > ABORT_REASON_MAX)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DISABLE_WFI(Core->SleepMask);

	Status = XPmSubsystem_SetState(SubsystemId, (u32)ONLINE);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
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
	XPM_EXPORT_CMD(PM_SELF_SUSPEND, XPLMI_CMD_ARG_CNT_FIVE, XPLMI_CMD_ARG_CNT_FIVE);
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

		Status = XPmSubsystem_SetState(SubsystemId, (u32)SUSPENDING);
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
	Core->Device.Node.State = (u8)XPM_DEVSTATE_SUSPENDING;

	XPm_ClearScanClear();

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
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
 * @param CmdType		IPI command request type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been sent.
 *
 ****************************************************************************/
XStatus XPm_RequestSuspend(const u32 SubsystemId, const u32 TargetSubsystemId,
			   const u32 Ack, const u32 Latency, const u32 State,
			   const u32 CmdType)
{
	XPM_EXPORT_CMD(PM_REQUEST_SUSPEND, XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);
	XStatus Status = XST_FAILURE;
	u32 IpiMask = 0;
	u32 Payload[5] = {0};
	/* Warning Fix */
	(void) (Ack);
	const XPm_Subsystem *TargetSubsystem = NULL;

	IpiMask = XPmSubsystem_GetIPIMask(TargetSubsystemId);
	if (0U == IpiMask) {
		PmErr("Unable to fetch IpiMask for given TargetSubsystem\r\n");
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if (SubsystemId == TargetSubsystemId) {
		Status = XPM_INVALID_SUBSYSID;
		PmErr("Cannot Suspend yourself\n\r");
		goto done;
	}

	Status = XPmSubsystem_IsOperationAllowed(SubsystemId, TargetSubsystemId,
						 SUB_PERM_SUSPEND_MASK,
						 CmdType);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		PmErr("Subsystem %x not allowed to suspend Target %x\n", SubsystemId, TargetSubsystemId);
		goto done;
	}

	TargetSubsystem = XPmSubsystem_GetById(TargetSubsystemId);
	if (NULL == TargetSubsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if (0U != TargetSubsystem->PendCb.Reason) {
		Status = XPM_PEND_SUSP_CB_FOUND;
		goto done;
	}

	/* TODO: Target subsystem must be active to get the suspend request */

	/* TODO: Check if other subsystem has sent suspend request to target subsystem */
	/* Failure in this case should return XPM_PM_DOUBLE_REQ */

	Payload[0] = (u32)PM_INIT_SUSPEND_CB;
	Payload[1] = (u32)SUSPEND_REASON_PU_REQ;
	Payload[2] = Latency;
	Payload[3] = State;
	/* Payload[4] is for timeout which is not considered */
	Payload[4] = 0U;

	/* Send the suspend request via callback */
	XPmNotifier_NotifyTarget(IpiMask, Payload);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPm_GicProxyWakeUp(const u32 PeriphIdx)
{
	XStatus Status = XST_FAILURE;

	const XPm_Periph *Periph = (XPm_Periph *)XPmDevice_GetByIndex(PeriphIdx);
	if ((NULL == Periph) || (0U == Periph->WakeProcId)) {
		goto done;
	}

	const XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(Periph->WakeProcId);

	/* Do not process anything if core is already running */
	if ((u8)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPm_RequestWakeUp(PM_SUBSYS_PMC, Periph->WakeProcId, 0, 0, 0,
				   XPLMI_CMD_SECURE);

done:
	return Status;
}


static XStatus XPm_SubsystemPwrUp(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;

	/* Activate the subsystem by requesting its pre-alloc devices */
	Status = XPmSubsystem_Configure(SubsystemId);
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
	XPM_EXPORT_CMD(PM_REQUEST_WAKEUP, XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);
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
			CoreSubsystemId = XPmDevice_GetSubsystemIdOfCore((XPm_Device *)Core);
			if (INVALID_SUBSYSID == CoreSubsystemId) {
				Status = XPM_ERR_SUBSYS_NOTFOUND;
				break;
			}
			Status = Core->CoreOps->RequestWakeup(Core, SetAddress, Address);
			if (XST_SUCCESS == Status) {
				Status = XPmSubsystem_SetState(CoreSubsystemId, (u32)ONLINE);
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

static void XPm_CoreIdle(XPm_Core *Core)
{
	ENABLE_WFI(Core->SleepMask);
	Core->Device.Node.State = (u8)XPM_DEVSTATE_PENDING_PWR_DWN;
	XPmNotifier_Event(Core->Device.Node.Id,
			  (u8)EVENT_CPU_IDLE_FORCE_PWRDWN);
}

static XStatus XPm_SubsystemIdleCores(const XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Reqm;
	u32 DeviceId;

	Reqm = Subsystem->Requirements;
	while (NULL != Reqm) {
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
		    ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(DeviceId)) &&
		    (PM_DEV_PSM_PROC != DeviceId)) {
			XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
			if (NULL == Core) {
				Status = XST_INVALID_PARAM;
				goto done;
			}
			if ((1U == Core->isCoreUp) &&
			    (1U == Core->IsCoreIdleSupported)) {
				XPm_CoreIdle(Core);
				Status = XST_SUCCESS;
			} else {
				Status = XPmCore_ForcePwrDwn(DeviceId);
			}
		}
		Reqm = Reqm->NextDevice;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Handler for force power down timer
 *
 * @param Data	Node ID of subsystem/core for which power down event came
 *
 * @return XST_SUCCESS in case of success and error code in case of failure
 *
 * @note   none
 *
 ****************************************************************************/
int XPm_ForcePwrDwnCb(void *Data)
{
	int Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;
	u32 NodeId = (u32)Data;
	const XPm_Core *Core;

	if ((u32)XPM_NODECLASS_SUBSYSTEM == NODECLASS(NodeId)) {
		Subsystem = XPmSubsystem_GetById(NodeId);
		if (NULL == Subsystem) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		if ((u8)PENDING_POWER_OFF != Subsystem->State) {
			Status = XST_SUCCESS;
			goto done;
		}

		Status = XPmSubsystem_ForcePwrDwn(NodeId);
	} else {
		Core = (XPm_Core *)XPmDevice_GetById(NodeId);
		if (NULL == Core) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		if ((u8)XPM_DEVSTATE_PENDING_PWR_DWN !=
		    Core->Device.Node.State) {
			Status = XST_SUCCESS;
			goto done;
		}
		Status = XPmCore_ProcessPendingForcePwrDwn(NodeId);
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
	XPM_EXPORT_CMD(PM_FORCE_POWERDOWN, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	u32 NodeState = 0U;
	const XPm_Power *Power;

	if ((u32)REQUEST_ACK_BLOCKING == Ack) {
		/* Disable IPI interrupt */
		PmOut32(IPI_PMC_IDR, IpiMask);
	}

	/*Validate access first */
	Status = XPm_IsForcePowerDownAllowed(SubsystemId, NodeId, CmdType);
	if (XST_SUCCESS != Status) {
		goto process_ack;
	}

	if ((NODECLASS(NodeId) == (u32)XPM_NODECLASS_DEVICE) &&
	    (NODESUBCLASS(NodeId) == (u32)XPM_NODESUBCL_DEV_CORE)) {
		XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(NodeId);
		if (NULL == Core) {
			Status = XST_INVALID_PARAM;
			goto process_ack;
		}
		if ((1U == Core->isCoreUp) && (1U == Core->IsCoreIdleSupported)) {
			Core->FrcPwrDwnReq.AckType = Ack;
			Core->FrcPwrDwnReq.InitiatorIpiMask = IpiMask;
			XPm_CoreIdle(Core);
			Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_XILPM_ID,
							XPm_ForcePwrDwnCb,
							NULL,
							XPM_PWR_DWN_TIMEOUT,
							XPLM_TASK_PRIORITY_1,
							(void *)NodeId,
							XPLMI_NON_PERIODIC_TASK);
			goto done;
		} else {
			Status = XPmCore_ForcePwrDwn(NodeId);
			NodeState = Core->Device.Node.State;
		}
	} else if ((u32)XPM_NODECLASS_POWER == NODECLASS(NodeId)) {
		Power = XPmPower_GetById(NodeId);
		if (NULL == Power) {
			goto process_ack;
		}
		Status = XPmPower_ForcePwrDwn(NodeId);
		NodeState = Power->Node.State;
	} else if ((u32)XPM_NODECLASS_SUBSYSTEM == NODECLASS(NodeId)) {
		Subsystem = XPmSubsystem_GetById(NodeId);
		if (NULL == Subsystem) {
			Status = XST_INVALID_PARAM;
			goto process_ack;
		}

		if (0U != (Subsystem->Flags & (u8)SUBSYSTEM_IDLE_SUPPORTED)) {
			Status = XPmSubsystem_SetState(Subsystem->Id,
						       (u8)PENDING_POWER_OFF);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			Subsystem->FrcPwrDwnReq.AckType = Ack;
			Subsystem->FrcPwrDwnReq.InitiatorIpiMask = IpiMask;
			Status = XPm_SubsystemIdleCores(Subsystem);
			if (XST_SUCCESS != Status) {
				NodeState = Subsystem->State;
				goto process_ack;
			}
			Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_XILPM_ID,
							XPm_ForcePwrDwnCb,
							NULL,
							XPM_PWR_DWN_TIMEOUT,
							XPLM_TASK_PRIORITY_1,
							(void *)NodeId,
							XPLMI_NON_PERIODIC_TASK);
			goto done;
		} else {
			Status = XPmSubsystem_ForcePwrDwn(NodeId);
			goto done;
		}
	} else {
		Status = XPM_PM_INVALID_NODE;
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

/****************************************************************************/
/**
 * @brief  This function can be used to send restart CPU idle callback to
 * 	   subsystem to idle cores before subsystem restart
 *
 * @param SubsystemId	Subsystem ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note  None
 *
 ****************************************************************************/
XStatus XPm_IdleRestartHandler(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);

	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if (0U != (SUBSYSTEM_IDLE_SUPPORTED & Subsystem->Flags)) {
		Status = XPmSubsystem_SetState(SubsystemId, (u8)PENDING_RESTART);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPm_SubsystemIdleCores(Subsystem);

		/* TODO: Need to add recovery handling if subsystem not responding */
	} else {
		Status = XPm_SystemShutdown(SubsystemId, PM_SHUTDOWN_TYPE_RESET,
					    PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM,
					    XPLMI_CMD_SECURE);
	}

done:
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
 * @param CmdType			IPI command request type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been received.
 *
 ****************************************************************************/
XStatus XPm_SystemShutdown(u32 SubsystemId, const u32 Type, const u32 SubType,
			   const u32 CmdType)
{
	XPM_EXPORT_CMD(PM_SYSTEM_SHUTDOWN, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
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

	/* For shutdown type the subtype is irrelevant: shut the caller down */
	if (PM_SHUTDOWN_TYPE_SHUTDOWN == Type) {
		/* Idle the subsystem first */
		Status = XPmSubsystem_Idle(SubsystemId);
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

		Status = XPmSubsystem_SetState(SubsystemId, (u32)POWERED_OFF);
		goto done;
	}

	switch (SubType) {
	case PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM:
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
	XPM_EXPORT_CMD(PM_IOCTL, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_FIVE);
	XStatus Status;

	Status = XPm_Ioctl(SubsystemId, DeviceId, IoctlId, Arg1, Arg2, Arg3,
			   Response, CmdType);
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}
