/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_hw.h"
#include "xplmi_modules.h"
#include "xplmi_sysmon.h"
#include "xplmi_util.h"
#include "xpm_api.h"
#include "xpm_apucore.h"
#include "xpm_asucore.h"
#include "xpm_common.h"
#include "xpm_clock.h"
#include "xpm_debug.h"
#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_domain_iso.h"
#include "xpm_err.h"
#include "xpm_mem.h"
#include "xpm_nodeid.h"
#include "xpm_npdomain.h"
#include "xpm_periph.h"
#include "xpm_pin.h"
#include "xpm_pldomain.h"
#include "xpm_pll.h"
#include "xpm_pmc.h"
#include "xpm_pmcdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_regs.h"
#include "xpm_repair.h"
#include "xpm_reset.h"
#include "xpm_rpucore.h"
#include "xsysmonpsv.h"
#include "xpm_pldevice.h"
#include "xpm_rail.h"
#include "xpm_regulator.h"
#include "xpm_aie.h"
#include "xpm_alloc.h"
#include "xpm_regnode.h"
#include "xpm_aiedevice.h"
#ifdef XILPM_RUNTIME
#include "xpm_subsystem.h"
#endif
#include "xpm_aie.h"

#define PM_SUBSYS_INVALID		   (0xFFFFFFFFU) /* Invalid Subsystem ID */
/* Macro to typecast PM API ID */
/*
 * Macro for exporting xilpm command details. Use in the first line of commands
 * used in CDOs.
 */
#define XPM_EXPORT_CMD(CmdIdVal, MinArgCntVal, MaxArgCntVal) \
	XPLMI_EXPORT_CMD(CmdIdVal, XPLMI_MODULE_XILPM_ID, MinArgCntVal, MaxArgCntVal)

/**
 * The XPlmi_PmCmds array is a static array of type XPlmi_ModuleCmd, which is used to store
 * the module commands for the Versal platform. It has a size of PM_API_MAX, which represents
 * the maximum number of module commands supported.
 */
static XPlmi_ModuleCmd XPlmi_PmCmds[PM_API_MAX] = {NULL};

static XStatus XPm_DoIgnoreCommand(XPlmi_Cmd* Cmd);
static XStatus XPm_DoBisr(XPlmi_Cmd* Cmd);
static XStatus XPm_DoApplyTrim(XPlmi_Cmd* Cmd);
static XStatus XPm_AddNodeParent(XPlmi_Cmd *Cmd);
static XStatus XPm_AddNodeName(XPlmi_Cmd *Cmd);
static XStatus XPm_IsoControl(XPlmi_Cmd *Cmd);
static XStatus XPm_DoInitNode(XPlmi_Cmd *Cmd);
static XStatus XPm_AddNodeIsolation(const u32 *Args, u32 NumArgs);
static XStatus XPm_AddNodeRegnode(const u32 *Args, u32 NumArgs);
/*********************************************************************/

u32 ResetReason;
void (*PmRequestCb)(const u32 SubsystemId, const XPmApiCbId_t EventId, u32 *Payload) = NULL;
int (*PmRestartCb)(u32 ImageId, u32 *FuncId) = NULL;
extern u8 __xpm_bss_start[];
extern u8 __xpm_bss_end[];

/**
 * @brief Get the pointer to the array of PM commands
 *
 * This function returns a pointer to the array of PM commands.
 *
 * @return Pointer to the array of PM commands
 */
XPlmi_ModuleCmd* XPm_GetPmCmds(void){
	return XPlmi_PmCmds;
}
/**
 * The XPlmi_PmAccessPermBuff array is a static array used to store the access permissions
 * for various PM API functions. It is used in the Versal Common Server module of the Xilinx
 * Power Management Interface (XPMI) library.
 */
static XPlmi_AccessPerm_t XPlmi_PmAccessPermBuff[PM_API_MAX] =
{
	XPLMI_ALL_IPI_FULL_ACCESS(PM_GET_API_VERSION),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_GET_NODE_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_GET_OP_CHARACTERISTIC),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_REGISTER_NOTIFIER),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_REQUEST_SUSPEND),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_SELF_SUSPEND),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_FORCE_POWERDOWN),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_ABORT_SUSPEND),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_REQUEST_WAKEUP),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_SET_WAKEUP_SOURCE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_SYSTEM_SHUTDOWN),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_REQUEST_NODE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_RELEASE_NODE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_SET_REQUIREMENT),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_SET_MAX_LATENCY),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_RESET_ASSERT),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_RESET_GET_STATUS),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_INIT_FINALIZE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_GET_CHIPID),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PINCTRL_REQUEST),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PINCTRL_RELEASE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PINCTRL_GET_FUNCTION),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PINCTRL_SET_FUNCTION),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PINCTRL_CONFIG_PARAM_GET),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PINCTRL_CONFIG_PARAM_SET),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_IOCTL),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_QUERY_DATA),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_CLOCK_ENABLE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_CLOCK_DISABLE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_CLOCK_GETSTATE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_CLOCK_SETDIVIDER),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_CLOCK_GETDIVIDER),
	XPLMI_ALL_IPI_NO_ACCESS(PM_CLOCK_SETRATE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_CLOCK_SETPARENT),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_CLOCK_GETPARENT),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PLL_SET_PARAMETER),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PLL_GET_PARAMETER),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PLL_SET_MODE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_PLL_GET_MODE),
	XPLMI_ALL_IPI_NO_ACCESS(PM_ADD_SUBSYSTEM),
	XPLMI_ALL_IPI_NO_ACCESS(PM_DESTROY_SUBSYSTEM),
	XPLMI_ALL_IPI_NO_ACCESS(PM_DESCRIBE_NODES),
	XPLMI_ALL_IPI_NO_ACCESS(PM_ADD_NODE),
	XPLMI_ALL_IPI_NO_ACCESS(PM_ADD_NODE_PARENT),
	XPLMI_ALL_IPI_NO_ACCESS(PM_ADD_NODE_NAME),
	XPLMI_ALL_IPI_NO_ACCESS(PM_ADD_REQUIREMENT),
	XPLMI_ALL_IPI_NO_ACCESS(PM_SET_CURRENT_SUBSYSTEM),
	XPLMI_ALL_IPI_NO_ACCESS(PM_INIT_NODE),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_FEATURE_CHECK),
	XPLMI_ALL_IPI_NO_ACCESS(PM_ISO_CONTROL),
	XPLMI_ALL_IPI_FULL_ACCESS(PM_ACTIVATE_SUBSYSTEM),
	XPLMI_ALL_IPI_NO_ACCESS(PM_SET_NODE_ACCESS),
	XPLMI_ALL_IPI_NO_ACCESS(PM_BISR),
	XPLMI_ALL_IPI_NO_ACCESS(PM_APPLY_TRIM),
	XPLMI_ALL_IPI_NO_ACCESS(PM_NOC_CLOCK_ENABLE),
	XPLMI_ALL_IPI_NO_ACCESS(PM_IF_NOC_CLOCK_ENABLE),
	XPLMI_ALL_IPI_NO_ACCESS(PM_FORCE_HOUSECLEAN),
	XPLMI_ALL_IPI_NO_ACCESS(PM_HNICX_NPI_DATA_XFER),
};

static XPlmi_Module XPlmi_Pm =
{
	XPLMI_MODULE_XILPM_ID,
	XPlmi_PmCmds,
	PM_API(PM_API_MAX),
	NULL,
	XPlmi_PmAccessPermBuff,
	NULL,
};

/**
 * Dummy Runtime function which can be overridden by the Runtime Library
 * We need to define this function here to avoid linking error in case
 */
XStatus __attribute__((weak, noinline)) XPm_RuntimeInit(void) {
	PmInfo("No XilPm Runtime Library.\n\r");
	return XST_SUCCESS;
};

XStatus __attribute__((weak, noinline)) XPm_HookAfterPlmCdo(void)
{
	XStatus Status = XST_SUCCESS;
	return Status;
}

XStatus __attribute__((weak, noinline)) XPm_AddDDRMemRegnForDefaultSubsystem(const XPm_MemCtrlrDevice *MCDev) {
	(void)(MCDev);
	return XST_SUCCESS;
};

static inline u32 XPmSubsystem_GetSubSysIdByIpiMask_Core(u32 IpiMask) {
	u32 SubsystemId = PM_SUBSYS_INVALID;
	s32 FirstSet = 0;

	if (0U == IpiMask) {
		/** This is the error case */
		PmErr("Invalid IPI Mask: 0x%x\n\r", IpiMask);
		goto done;
	}

	FirstSet = __builtin_ffs((s32)IpiMask) - 1;
	switch (BIT((u32)FirstSet)) {
		case ASU_IPI_MASK:
			SubsystemId = PM_SUBSYS_ASU;
			break;
		case PMC_IPI_MASK:
			SubsystemId = PM_SUBSYS_PMC;
			break;
		default:
			SubsystemId = PM_SUBSYS_DEFAULT + (u32)FirstSet;
			break;
	}

done:
	return SubsystemId;
}

u32 __attribute__((weak, noinline)) XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask) {
	volatile u32 SubsystemId = PM_SUBSYS_INVALID;
	volatile u32 SubsystemIdTmp = PM_SUBSYS_INVALID;

	XSECURE_REDUNDANT_CALL(SubsystemId, SubsystemIdTmp, XPmSubsystem_GetSubSysIdByIpiMask_Core, IpiMask);
	if (SubsystemId != SubsystemIdTmp) {
		PmErr("Glitched detected");
		SubsystemId = PM_SUBSYS_INVALID;
	}

	return SubsystemId;
}

XStatus __attribute__((weak, noinline)) XPm_SystemShutdown(u32 SubsystemId, u32 Type, u32 SubType, u32 CmdType)
{
	(void)SubsystemId;
	(void)Type;
	(void)SubType;
	(void)CmdType;
	PmWarn("Ignoring CMD: PM_SYSTEM_SHUTDOWN. SubsystemId=0x%x, Type=0x%x, SubType=0x%x\n\r",
		SubsystemId, Type, SubType);
	return XST_SUCCESS;
}

XStatus __attribute__((weak, noinline)) XPm_PmcActivateSubsystem(u32 SubsystemId)
{
	(void)SubsystemId;
	/*
	 * This function is used to activate the subsystem.
	 * The default boot implementation does nothing and returns success.
	 */
	return XST_SUCCESS;
}

XStatus __attribute__((weak, noinline)) XPm_AddSubsystem(XPlmi_Cmd* Cmd) {
	PmWarn("Ignoring CMD: PM_ADD_SUBSYSTEM. SubsystemId=0x%x\n\r", Cmd->Payload[0]);
	return XST_SUCCESS;
}

XStatus __attribute__((weak, noinline)) XPm_AddRequirement(XPlmi_Cmd* Cmd) {
	PmWarn("Ignoring CMD: PM_ADD_REQUIREMENT. SubsystemId=0x%x DeviceId=0x%x\n\r",
	Cmd->Payload[0], Cmd->Payload[1]);
	return XST_SUCCESS;
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
static XStatus XPm_SetClockRate(XPlmi_Cmd* Cmd)
{
	XPM_EXPORT_CMD(PM_CLOCK_SETRATE, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	if (2 > Cmd->Len) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	u32 IpiMask =Cmd->IpiMask;
	u32 ClockId = Cmd->Payload[0];
	u32 ClkRate = Cmd->Payload[1];

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
	Cmd->Response[0] = (u32) Status;
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

/**
 * XPm_DoIgnoreCommand - This function just ignore all commands.
 *
 * @Cmd: Pointer to the command structure.
 *
 * This function sets the response of the command to XST_SUCCESS and returns
 * XST_SUCCESS. It is used to handle commands that should be ignored.
 *
 * Return: XST_SUCCESS on success.
 */
static XStatus XPm_DoIgnoreCommand(XPlmi_Cmd* Cmd)
{
	Cmd->Response[0] = XST_SUCCESS;
	return XST_SUCCESS;
}

/**
 * XPm_DoWarningCommand - This function just ignore all commands with a warning.
 *
 * @Cmd: Pointer to the command structure.
 *
 * This function sets the response of the command to XST_SUCCESS and returns
 * XST_SUCCESS. It is used to handle commands that should be ignored.
 *
 * Return: XST_SUCCESS on success.
 */
static XStatus XPm_DoWarningCommand(XPlmi_Cmd* Cmd)
{
	Cmd->Response[0] = XST_SUCCESS;
	PmWarn("Ignoring command: CMDID: 0x%x\n\r", Cmd->CmdId);
	return XST_SUCCESS;
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

	/* Zeroized xpm_bss_data session during none In-place update*/
	if (XPlmi_IsPlmUpdateDone() != (u8)TRUE) {
		u32 __xpm_bss_len = (u32)(__xpm_bss_end - __xpm_bss_start);
		Status = Xil_SMemSet(__xpm_bss_start, __xpm_bss_len , 0U, __xpm_bss_len);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to zeroize xpm_bss_data session\r\n");
			goto done;
		}
		/* Initialize clock topology templates */
		Status = XPmClock_InitGenericTopology();
		if (XST_SUCCESS != Status) {
			PmErr("Failed to initialize clock topology templates\r\n");
			goto done;
		}
	}

	PmInfo("Initializing XilPM Boot Library\n\r");

	/* Initializing XPLmi_PmCmds array*/
	XPlmi_PmCmds[PM_ADD_NODE].Handler = (XPlmi_CmdHandler)XPm_AddNode;
	XPlmi_PmCmds[PM_ADD_REQUIREMENT].Handler = (XPlmi_CmdHandler)XPm_AddRequirement;
	XPlmi_PmCmds[PM_ADD_SUBSYSTEM].Handler = (XPlmi_CmdHandler)XPm_AddSubsystem;
	XPlmi_PmCmds[PM_ADD_NODE_PARENT].Handler = (XPlmi_CmdHandler)XPm_AddNodeParent;
	XPlmi_PmCmds[PM_ADD_NODE_NAME].Handler = (XPlmi_CmdHandler)XPm_AddNodeName;
	XPlmi_PmCmds[PM_CLOCK_SETRATE].Handler = (XPlmi_CmdHandler)XPm_SetClockRate;
	XPlmi_PmCmds[PM_INIT_NODE].Handler = (XPlmi_CmdHandler)XPm_DoInitNode;
	XPlmi_PmCmds[PM_ISO_CONTROL].Handler = (XPlmi_CmdHandler)XPm_IsoControl;
	XPlmi_PmCmds[PM_BISR].Handler = (XPlmi_CmdHandler)XPm_DoBisr;
	XPlmi_PmCmds[PM_APPLY_TRIM].Handler = (XPlmi_CmdHandler)XPm_DoApplyTrim;
	XPlmi_PmCmds[PM_HNICX_NPI_DATA_XFER].Handler = (XPlmi_CmdHandler)XPm_DoWarningCommand;
	XPlmi_PmCmds[PM_SET_NODE_ACCESS].Handler = (XPlmi_CmdHandler)XPm_DoIgnoreCommand;
	/* Init runtime submodule */
	Status = XPm_RuntimeInit();
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/* Check last reset reason */
	(void)XPm_CheckLastResetReason();

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
	u32 PsPlPorMask = CRP_RST_PS_PL_POR_MASK |
			CRP_RST_PS_PS_POR_MASK |
			CRP_RST_PS_PL_SRST_MASK;
	u32 DefaultDomainIsoMask = PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK \
		| PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_MASK;
	if ((0U != (ResetReason & SysResetMask)) && ((u32)XPlmi_IsPlmUpdateDone() != 1U)) {
		/* Assert PL and PS POR */
		PmRmw32(CRP_RST_PS, PsPlPorMask, PsPlPorMask);

		/* Assert NOC POR, NPI Reset, Sys Resets */
		PmRmw32(CRP_RST_NONPS, NoCResetsMask, NoCResetsMask);
		/* Re-enable all domain isolation in PMC_GLOBAL*/
		PmOut32(PMC_GLOBAL_DOMAIN_ISO_CNTRL, DefaultDomainIsoMask);
	}
	/*
	 * Clear DomainInitStatusReg in case of internal PMC_POR. Since PGGS0
	 * value is not cleared in case of internal POR.
	 */
	if (0U != (ResetReason & PmcIPORMask)) {
		XPm_Out32(XPM_DOMAIN_INIT_STATUS_REG, 0);
	}

	PmRequestCb = RequestCb;
	(void)XPlmi_ModuleRegister(&XPlmi_Pm);

	Status = XST_SUCCESS;
	PmRestartCb = RestartCb;

done:
	return Status;
}

/**
 * @brief Performs Built-In Self-Repair (BISR) for a given Tag ID
 *
 * This function performs Built-In Self-Repair (BISR) for a given Tag ID.
 * It checks if the Tag ID is supported and calls the appropriate repair function.
 *
 * @param Cmd Pointer to the XPlmi_Cmd structure containing the command payload
 * @return XStatus Status of the BISR operation
 */
XStatus XPm_DoBisr(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 TagId = Cmd->Payload[0];

	Status = XPmBisr_Repair(TagId);

	Cmd->Response[0] = (u32)Status;
	return Status;
}

/**
 * @brief Applies trim operation based on the given trim type.
 *
 * This function is used to apply trim operation based on the given trim type.
 *
 * @param Cmd Pointer to the XPlmi_Cmd structure containing the command payload.
 * @return Status of the trim operation. Returns XST_SUCCESS on success, or an error code on failure.
 */
XStatus XPm_DoApplyTrim(XPlmi_Cmd* Cmd)
{
	XStatus Status = XST_FAILURE;
	u32 TrimType = Cmd->Payload[0];
	Status = XPm_PldApplyTrim(TrimType);

	Cmd->Response[0] = (u32)Status;
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
XStatus XPm_IsoControl(XPlmi_Cmd *Cmd)
	//u32 NodeId, u32 Enable)
{
	//XPM_EXPORT_CMD(PM_ISO_CONTROL, XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	XStatus Status = XST_FAILURE;
	if (2 > Cmd->Len) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	u32 NodeId = Cmd->Payload[0];
	u32 Enable =  Cmd->Payload[1];

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
	Cmd->Response[0] = (u32)Status;
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
static XStatus XPm_AddNodeParent(XPlmi_Cmd *Cmd)
{
	//XPM_EXPORT_CMD(PM_ADD_NODE_PARENT, XPLMI_CMD_ARG_CNT_TWO, XPLMI_UNLIMITED_ARG_CNT);
	XStatus Status = XST_FAILURE;
	const u32* Args = Cmd->Payload;
	u32 NumArgs = Cmd->Len;
	u32 Id = Args[0];
	const u32 *Parents;
	u32 NumParents;

	if (NumArgs < 2U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NumParents = NumArgs-1U;
	Parents = &Args[1];

	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		/* Skip during PLM update */
		Status = XST_SUCCESS;
		goto done;
	}

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

	case (u32)XPM_NODETYPE_DEV_CORE_ASU:
		XPm_AsuCore *AsuCore = (XPm_AsuCore *)XPm_AllocBytes(sizeof(XPm_AsuCore));
		Status = XPmAsuCore_Init(AsuCore, DeviceId, Ipi, BaseAddr, Power, NULL, NULL);
		/* @TODO Just allocate memory for ASU node for now and return SUCCESS */
		Status = XST_SUCCESS;
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
	} else if ((u32)XPM_NODEIDX_DEV_AIE == NODEINDEX(DeviceId)) {
		Status = XPmAie_AddPeriphNode(Args, PowerId);
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

static XStatus AddPlDevice(const u32 *Args, u32 NumArgs, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Index;
	XPm_Power *Power;
	u32 BaseAddr = 0;
	XPm_PlDevice *PlDevice;

	DeviceId = Args[0];
	if (3U <= NumArgs) {
		BaseAddr = Args[2];
	}

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
 * @brief  Add mem-range device to internal data-structure map
 *
 * @param  Args		CDO command arguments
 * @param  NumArgs	Total number of arguments
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
static XStatus AddMemRegnDevice(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	u32 Index;
	u64 Address;
	u64 size;

	if (5U != NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DeviceId = Args[0];
	Type = NODETYPE(DeviceId);
	Index = NODEINDEX(DeviceId);

	Address = ((u64)Args[1]) | (((u64)Args[2]) << 32);
	size = ((u64)Args[3]) | (((u64)Args[4]) << 32);

	if ((u32)XPM_NODEIDX_DEV_MEM_REGN_MAX < Index) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	if ((u32)XPM_NODETYPE_DEV_MEM_REGN == Type) {
		Status = XPm_AddMemRegnDevice(DeviceId, Address, size);
	} else {
		Status = XST_INVALID_PARAM;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Add device security, virtualization and coherency attributes
 *
 * @param  Args		CDO command arguments
 * @param  NumArgs	Total number of arguments
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
static XStatus AddDevAttributes(const u32 *Args, const u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_DeviceAttr *DevAttr = NULL;
	XPm_Device *Dev = XPmDevice_GetById(Args[ARG_IDX_DEVATTR_DEVICE_ID]);

	/* Check for device presence and sufficient arguments */
	if ((NULL == Dev) || (NumArgs < DEVATTR_ARG_MIN_LEN)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DevAttr = (XPm_DeviceAttr *)XPm_AllocBytes(sizeof(XPm_DeviceAttr));
	if (NULL == DevAttr) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	/* Store the security attributes */
	DevAttr->SecurityBaseAddr = Args[ARG_IDX_DEVATTR_SEC_BASEADDR];
	DevAttr->Security[0].Offset = (u16)((Args[ARG_IDX_DEVATTR_SEC_0_OFFSET] >> DEVATTR_SEC_OFFSET) & DEVATTR_SEC_MASK);
	DevAttr->Security[0].Mask = (u16)(Args[ARG_IDX_DEVATTR_SEC_0_MASK] & DEVATTR_SEC_MASK);
	DevAttr->Security[1].Offset = (u16)((Args[ARG_IDX_DEVATTR_SEC_1_OFFSET] >> DEVATTR_SEC_OFFSET) & DEVATTR_SEC_MASK);
	DevAttr->Security[1].Mask = (u16)(Args[ARG_IDX_DEVATTR_SEC_1_MASK] & DEVATTR_SEC_MASK);

	/* Check for the coherency and virtualization attributes */
	if (NumArgs > DEVATTR_ARG_MIN_LEN) {
		if (NumArgs < DEVATTR_ARG_MAX_LEN) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		/* Store the coherency and virtualization attributes */
		DevAttr->CohVirtBaseAddr = Args[ARG_IDX_DEVATTR_COHVIR_BASEADDR];
		DevAttr->Coherency.Offset = (u16)((Args[ARG_IDX_DEVATTR_COH_OFFSET] >> DEVATTR_COH_OFFSET) & DEVATTR_COH_MASK);
		DevAttr->Coherency.Mask = (u16)(Args[ARG_IDX_DEVATTR_COH_MASK] & DEVATTR_COH_MASK);
		DevAttr->Virtualization.Offset = (u16)((Args[ARG_IDX_DEVATTR_VIR_OFFSET] >> DEVATTR_VIR_OFFSET) & DEVATTR_VIR_MASK);
		DevAttr->Virtualization.Mask = (u16)(Args[ARG_IDX_DEVATTR_VIR_MASK] & DEVATTR_VIR_MASK);
	}

	Dev->DevAttr = DevAttr;

	Status = XST_SUCCESS;

done:
	return Status;
}


static XStatus AddAieDevice(const u32 *Args)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId = Args[0];
	u32 Index = NODEINDEX(DeviceId);
	u32 BaseAddr = Args[2];
	XPm_AieDevice *AieDevice;

	if ((u32)XPM_NODEIDX_DEV_AIE_MAX <= Index) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Note: This function is executed as part of pm_add_node cmd triggered
	 * through CDO. Since there's a possibility of the same RM (hence CDO)
	 * being executed multiple times, we should not error out on addition
	 * of same node multiple times. Memory is allocated only if node is not
	 * present in database.
	 */
	AieDevice = (XPm_AieDevice *)XPmDevice_GetById(DeviceId);
	if (NULL == AieDevice) {
		AieDevice = (XPm_AieDevice *)XPm_AllocBytes(sizeof(XPm_AieDevice));
		if (NULL == AieDevice) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	} else {
		PmInfo("0x%x Device is already added\r\n", DeviceId);
	}

	Status = XPmAieDevice_Init(AieDevice, DeviceId, BaseAddr, NULL, NULL, NULL);

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

	/*
	 * Memory region device node does not have any power node as parent.
	 * So, skipping this check even if the ADD_NODE command has more than
	 * one argument.
	 */
	if (((u32)XPM_NODESUBCL_DEV_MEM_REGN != SubClass) && (NumArgs > 1U)) {
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
		Status = AddPlDevice(Args, NumArgs ,PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_MEM_CTRLR:
		Status = AddMemCtrlrDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_MEM_REGN:
		Status = AddMemRegnDevice(Args, NumArgs);
		break;
	case (u32)XPM_NODESUBCL_DEV_AIE:
	/* PowerId is not passed by topology */
		Status = AddAieDevice(Args);
		break;
	default:
		Status = XST_NO_FEATURE;
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
XStatus XPm_AddNodeName(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	const u32* Args = Cmd->Payload;
	u32 NumArgs = Cmd->Len;
	u32 NodeId;
	char Name[MAX_NAME_BYTES] = {0};
	u32 i = 0U, j = 0U;
	const u32 CopySize = 4U;

	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		Status = XST_SUCCESS;
		goto done;
	}
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
	XPm_Rail *Rail;

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
	case (u32)XPM_NODETYPE_POWER_RAIL:

		Rail = (XPm_Rail *)XPmPower_GetById(PowerId);
		if (NULL == Rail) {
			Rail = (XPm_Rail *)XPm_AllocBytes(sizeof(XPm_Rail));
			if (NULL == Rail) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
		}
		Status = XPmRail_Init(Rail, PowerId, Args, NumArgs);
		break;
	case (u32)XPM_NODETYPE_POWER_REGULATOR:
		{
			XPm_Regulator *Regulator;
			Regulator = (XPm_Regulator *)XPmRegulator_GetById(PowerId);
			if (NULL == Regulator) {
				Regulator = (XPm_Regulator *)XPm_AllocBytesBoard(sizeof(XPm_Regulator));
				if (NULL == Regulator) {
					Status = XST_BUFFER_TOO_SMALL;
					goto done;
				}
			}
			Status = XPmRegulator_Init(Regulator, PowerId, Args, NumArgs);
			break;
		}
	case (u32)XPM_NODETYPE_POWER_DOMAIN_ME:
		XPm_AieDomain * AieDomain = (XPm_AieDomain *)XPm_AllocBytes(sizeof(XPm_AieDomain));
		if (NULL == AieDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmAieDomain_Init(AieDomain, PowerId, BitMask, PowerParent,
				&Args[3U], (NumArgs - 3U));
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
static XStatus XPm_AddNodeMio(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 MioId;
	u32 BaseAddress;
	XPm_PinNode *MioPin;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	MioId = Args[0];
	BaseAddress = Args[1];


	if ((u32)XPM_NODESUBCL_PIN != NODESUBCLASS(MioId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (((u32)XPM_NODETYPE_LPD_MIO != NODETYPE(MioId)) &&
	    ((u32)XPM_NODETYPE_PMC_MIO != NODETYPE(MioId))) {
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
XStatus XPm_AddNode(XPlmi_Cmd *Cmd)
{
	XStatus Status = XST_FAILURE;
	const u32* Args = Cmd->Payload;
	u32 NumArgs = Cmd->Len;
	u32 Id = Args[0];

	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		/* Skip during PLM update */
		Status = XST_SUCCESS;
		goto done;
	}

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
	case (u32)XPM_NODECLASS_STMIC:
		Status = XPm_AddNodeMio(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_ISOLATION:
		Status = XPm_AddNodeIsolation(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_REGNODE:
		Status = XPm_AddNodeRegnode(Args, NumArgs);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	Cmd->Response[0] = (u32)Status;
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



/**
 * XPm_PmcReleaseDevice - Release a device in the PMC.
 *
 * This function is a placeholder and does not actually release the device.
 * The XILPM boot does not support releasing devices.
 *
 * @param DeviceId: The ID of the device to be released.
 *
 * @return XST_SUCCESS always, as the function does not perform any action.
 */
XStatus XPm_PmcReleaseDevice(const u32 DeviceId)
{
	/* XILPM boot does not support Releasing device*/
	(void)DeviceId;
	return XST_SUCCESS;
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
 * @brief  This function is used to obtain information about the current state
 * of a device. The caller must pass a pointer to an XPm_DeviceStatus
 * structure, which must be pre-allocated by the caller.
 *
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
XStatus XPm_PmcGetDeviceState(const u32 DeviceId, u32 *const DeviceState)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;

	switch(NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPmDevice_GetState(DeviceId, DeviceState);
		break;
	case (u32)XPM_NODECLASS_POWER:
		Status = XPmPower_GetState(DeviceId, DeviceState);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/**
 * @brief Request Device from PMC Subsystem.
*/
XStatus XPm_PmcRequestDevice(const u32 DeviceId) {
	XStatus Status = XST_FAILURE;
	XPm_Device *Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	Status = XPmDevice_BringUp(Device);
done:
	return Status;
}

/**
 * @brief PMC request wakeup of CPU core
*/
XStatus XPm_PmcWakeAllCores(void) {
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;
	xil_printf("%s %d\n\r", __func__, __LINE__);
	for (u32 Idx = (u32)XPM_NODEIDX_DEV_MIN + 1U;
	     Idx < (u32)XPM_NODEIDX_DEV_MAX; Idx++) {
		Core = (XPm_Core *)XPmDevice_GetByIndex(Idx);
		if (NULL == Core) {
			xil_printf(".");
			continue;
		}
		/** Only wake up when  NODE type of CoreID is XPM_NODETYPE_DEV_CORE_APU or XPM_NODETYPE_DEV_CORE_RPU */
		if ((XPM_NODETYPE_DEV_CORE_APU != NODETYPE(Core->Device.Node.Id)) &&
		    (XPM_NODETYPE_DEV_CORE_RPU != NODETYPE(Core->Device.Node.Id))) {
			continue;
		}
		xil_printf("CoreId: 0x%x\n\r", Core->Device.Node.Id);
		if (XPM_DEVSTATE_RUNNING != Core->Device.Node.State) {
			xil_printf("..Waking up the core above\n\r");
			Status = XPm_PmcWakeUpCore(Core->Device.Node.Id, 0, 0);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

/**
 * @brief PMC request wakeup of CPU core
*/
XStatus XPm_PmcWakeUpCore(const u32 CoreId , const u32 SetAddress, const u64 Address) {
		XStatus Status = XST_FAILURE;
	XPm_Core *Core = (XPm_Core*)XPmDevice_GetById(CoreId);
	if (NULL == Core) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Status = XPmCore_RequestWakeup(Core, SetAddress, Address);
done:
	if (XST_SUCCESS != Status) {
		PmErr("Failed to wake up core 0x%x. Status = 0x%x\n\r", CoreId, Status);
	}
	return Status;
}

static XStatus PldInitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_PlDevice *PlDevice = NULL;

	PlDevice = (XPm_PlDevice *)XPmDevice_GetById(NodeId);
	if (NULL == PlDevice) {
		DbgErr = XPM_INT_ERR_INVALID_NODE;
		goto done;
	}

	switch (Function) {
	case (u32)FUNC_INIT_START:
		Status = XPmPlDevice_InitStart(PlDevice, Args, NumArgs);
		break;
	case (u32)FUNC_INIT_FINISH:
		Status = XPmPlDevice_InitFinish(PlDevice, Args, NumArgs);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPmDomainIso_ProcessPending();
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_DOMAIN_ISO;
			goto done;
		}
		break;
	case (u32)FUNC_MEM_CTRLR_MAP:
		Status = XPmPlDevice_MemCtrlrMap(PlDevice, Args, NumArgs);
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_FUNC;
		break;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}


static XStatus PwrDomainInitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PwrDomainNode;

	PwrDomainNode = (XPm_PowerDomain *)XPmPower_GetById(NodeId);
	if (NULL == PwrDomainNode) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	switch (NODEINDEX(NodeId)) {
	case (u32)XPM_NODEIDX_POWER_LPD:
	case (u32)XPM_NODEIDX_POWER_FPD:
	case (u32)XPM_NODEIDX_POWER_NOC:
	case (u32)XPM_NODEIDX_POWER_PLD:
	case (u32)XPM_NODEIDX_POWER_ME2:
		Status = XPmPowerDomain_InitDomain(PwrDomainNode, Function,
						   Args, NumArgs);
		break;
	default:
		Status = XPM_INVALID_PWRDOMAIN;
		break;
	}

	/*
	 * Call LPD init to initialize required components
	 */
	if (((u32)XPM_NODEIDX_POWER_LPD == NODEINDEX(NodeId)) &&
	    ((u32)FUNC_INIT_FINISH == Function)
	    /* TODO: Comment out below condition once INIT_START call added for LPD */
	    /* && (XST_SUCCESS == Status) */) {
		/*
		 * Mark domain init status bit in DomainInitStatusReg
		 */
		XPm_RMW32(XPM_DOMAIN_INIT_STATUS_REG,0x2,0x2);
		/**
		 * PLM needs to request UART0 and UART1, otherwise XilPM
		 * will turn it off when it is not used by other processor.
		 * During such scenario when PLM/other application tries to
		 * print debug message, system may not work properly.
		 */
#if (XPLMI_UART_NUM_INSTANCES > 0U)
		Status = XPm_PmcRequestDevice(PM_DEV_UART_0);
		if (XST_SUCCESS != Status) {
			PmErr("Error %d in request UART_0\r\n", Status);
			goto done;
		}
#endif

#if (XPLMI_UART_NUM_INSTANCES > 1U)
		Status = XPm_PmcRequestDevice(PM_DEV_UART_1);
		if (XST_SUCCESS != Status) {
			PmErr("Error %d in request UART_1\r\n", Status);
			goto done;
		}
#endif

		/**
		 * PLM needs to request PMC IPI, else XilPM will reset IPI
		 * when it is not used by other processor. Because of that PLM
		 * hangs when it tires to communicate through IPI.
		 */
		Status = XPm_PmcRequestDevice(PM_DEV_IPI_PMC);
		if (XST_SUCCESS != Status) {
			PmErr("Error %d in request IPI PMC\r\n", Status);
		}
		XPlmi_LpdInit();

#ifdef XPLMI_IPI_DEVICE_ID

	Status = XPlmi_IpiInit(XPmSubsystem_GetSubSysIdByIpiMask, NULL);
		if (XST_SUCCESS != Status) {
			PmErr("Error %u in IPI initialization\r\n", Status);
		}
#else
		PmWarn("IPI is not enabled in design\r\n");
#endif /* XPLMI_IPI_DEVICE_ID */
	}
done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x in InitNode for NodeId: 0x%x Function: 0x%x\r\n",
		       Status, NodeId, Function);
	}
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function allows to initialize the node.
 *
 * @param  NodeId	Supported power domain nodes, PLD
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
XStatus XPm_InitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (((u32)XPM_NODECLASS_POWER == NODECLASS(NodeId)) &&
	    ((u32)XPM_NODESUBCL_POWER_DOMAIN == NODESUBCLASS(NodeId)) &&
	    ((u32)XPM_NODEIDX_POWER_MAX > NODEINDEX(NodeId))) {
		Status = PwrDomainInitNode(NodeId, Function, Args, NumArgs);
	} else if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(NodeId)) &&
		  ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(NodeId)) &&
		  ((u32)XPM_NODEIDX_DEV_PLD_MAX > NODEINDEX(NodeId))) {
		Status = PldInitNode(NodeId, Function, Args, NumArgs);
	} else {
		Status = XPM_PM_INVALID_NODE;
		DbgErr = XPM_INT_ERR_INITNODE;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus XPm_DoInitNode(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	const u32 *Args = Cmd->Payload;
	u32 NumArgs = Cmd->Len;
	Status = XPm_InitNode(Args[0U], Args[1U], &Args[2U], NumArgs - 2U);
	Cmd->Response[0U] = (u32)Status;
	if (XST_SUCCESS != Status) {
		PmErr("PM_INIT_NODE 0x%x 0x%x Failure, Err: 0x%x\r\n", Args[0U], Args[1U], Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add isolation node to isolation topology database
 *
 * @param Args		pointer to isolation node arguments or payload
 * @param NumArgs	number of arguments or words in payload
 *
 * Format of payload/args (word aligned):
 *
 * +--------------------------------------------------------------------+
 * |Isolation Node ID(Node id include  class , subclass, type and Index)|
 * +--------------------------------------------+-----------------------+
 * |               rsvd[31:8]                   |      Format[7:0]      |
 * +--------------------------------------------+-----------------------+
 * |              Format specific payload (can be multiple words)       |
 * |                               ...                                  |
 * +--------------------------------------------+-----------------------+
 * |               rsvd[31:8]                   |      Format[7:0]      |
 * +--------------------------------------------+-----------------------+
 * |              Format specific payload (can be multiple words)       |
 * |                               ...                                  |
 * +--------------------------------------------------------------------+
 * |                               .                                    |
 * |                               .                                    |
 * |                               .                                    |
 * +--------------------------------------------------------------------+
 * Format entry for single word isolation control:
 * +--------------------------------------------+-----------------------+
 * |               rsvd[31:8]                   |      Format[7:0]      |
 * +--------------------------------------------+-----------------------+
 * |                           BaseAddress                              |
 * +--------------------------------------------------------------------+
 * |                           Mask                                     |
 * +--------------------------------------------------------------------+
 *
 * Format entry for power domain dependencies:
 * +----------------+----------------------------+----------------------+
 * | rsvd[31:16]    | Dependencies Count[15:8]   |      Format[7:0]     |
 * +-------------- -+ ---------------------------+----------------------+
 * |                   NodeID of Dependency0                            |
 * +--------------------------------------------------------------------+
 * |                           ...                                      |
 * +--------------------------------------------------------------------+
 *
 * Format entry for AFI interface control:
 * +--------------------------------------------+-----------------------+
 * |               rsvd[31:8]                   |      Format[7:0]      |
 * +--------------------------------------------+-----------------------+
 * |                           BaseAddress                              |
 * +--------------------------------------------------------------------+
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeIsolation(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	if (3U > NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Start at beginning of the payload*/
	u32 Index = 0U;
	/* NodeID is always at the first word in payload*/
	u32 NodeId = Args[Index++];
	u32 IsoFormat = 0U;

	u32 Dependencies[PM_ISO_MAX_NUM_DEPENDENCIES] = {0U};
	u32 BaseAddr = 0U, Mask = 0U, NumDependencies = 0U;

	XPm_IsoCdoArgsFormat Format = SINGLE_WORD_ACTIVE_LOW;
	while(Index < NumArgs) {
		/* Extract format and number of dependencies*/
		Format = (XPm_IsoCdoArgsFormat)CDO_ISO_ARG_FORMAT(Args[Index]);
		NumDependencies = CDO_ISO_DEP_COUNT(Args[Index++]);

		switch (Format){
		case SINGLE_WORD_ACTIVE_LOW:
		case SINGLE_WORD_ACTIVE_HIGH:
		case PSM_SINGLE_WORD_ACTIVE_LOW:
		case PSM_SINGLE_WORD_ACTIVE_HIGH:
			IsoFormat = Format;
			/* Extract BaseAddress and Mask*/
			BaseAddr = Args[Index++];
			Mask = Args[Index++];
			break;
		case PM_ISO_FORMAT_AFI_FM:
			IsoFormat = Format;
			BaseAddr = Args[Index++];
			Mask = AFI_FM_PORT_EN_MASK;
			break;
		case PM_ISO_FORMAT_AFI_FS:
			IsoFormat = Format;
			/* Extract BaseAddress */
			BaseAddr = Args[Index++];
			Mask = AFI_FS_PORT_EN_MASK;
			break;
		case PM_ISO_FORMAT_CHI_FPD:
			IsoFormat = Format;
			/* Extract BaseAddress */
			BaseAddr = Args[Index++];
			Mask = CHI_FPD_PORT_EN_MASK;
			break;
		case PM_ISO_FORMAT_ACP_APU:
			IsoFormat = Format;
			/* Extract BaseAddress */
			BaseAddr = Args[Index++];
			Mask = ACP_APU_GET_MASK(NodeId);
			break;
		case PM_ISO_FORMAT_PS_DTI:
			IsoFormat = Format;
			/* Extract BaseAddress */
			BaseAddr = Args[Index++];
			Mask = PS_DTI_GET_MASK(NodeId);
			break;
		case POWER_DOMAIN_DEPENDENCY:
			/* Format power domain dependencies*/
			/* To save space in PMC RAM we statically allocate Dependencies list*/
			if (NumDependencies > PM_ISO_MAX_NUM_DEPENDENCIES){
				Status = XPM_INT_ERR_ISO_MAX_DEPENDENCIES;
				goto done;
			}else {
				for (u32 i = 0U ; i < NumDependencies; i++){
					Dependencies[i] = Args[Index++];
				}
			}
			break;
		default:
			Status = XPM_INT_ERR_ISO_INVALID_FORMAT;
			goto done;
		}
	}

	Status = XPmDomainIso_NodeInit(NodeId, BaseAddr, Mask, IsoFormat, Dependencies,
					NumDependencies);
done:
	return Status;
}


static XStatus XPm_AddNodeRegnode(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId, PowerId;
	u32 BaseAddress;
	XPm_Power *Power = NULL;
	XPm_RegNode *Regnode = NULL;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeId = Args[0];
	BaseAddress = Args[1];
	PowerId = Args[2];

	if ((((u32)XPM_NODESUBCL_REGNODE_PREDEF != NODESUBCLASS(NodeId)) &&
	    ((u32)XPM_NODESUBCL_REGNODE_USERDEF != NODESUBCLASS(NodeId))) ||
	    ((((u32)XPM_NODETYPE_REGNODE_PREDEF_PGGS != NODETYPE(NodeId)) &&
	    ((u32)XPM_NODETYPE_REGNODE_PREDEF_GGS != NODETYPE(NodeId)) &&
	    ((u32)XPM_NODETYPE_REGNODE_GENERIC != NODETYPE(NodeId))))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Regnode = (XPm_RegNode *)XPm_AllocBytes(sizeof(XPm_RegNode));
	if (NULL == Regnode) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	XPmRegNode_Init(Regnode, NodeId, BaseAddress, Power);

	Status = XST_SUCCESS;

done:
	return Status;
}
XStatus __attribute__((weak, noinline)) XPm_HookAfterBootPdi(void)
{
	/* TODO: Review where interrupts need to be enabled */
	/* Enable power related interrupts to PMC */
	XPm_RMW32(PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN, PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN_MASK,
		  PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ_EN_MASK);

	return XST_SUCCESS;
}
