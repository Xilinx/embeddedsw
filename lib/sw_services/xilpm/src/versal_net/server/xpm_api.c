/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xplmi_error_node.h"
#include "xplmi_hw.h"
#include "xplmi_modules.h"
#include "xplmi_sysmon.h"
#include "xplmi_util.h"
#include "xpm_api.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_common.h"
#include "xpm_defs.h"
#include "xpm_err.h"
#include "xpm_nodeid.h"
#include "xpm_psm.h"
#include "xpm_regs.h"
#include "xpm_subsystem.h"
#include "xsysmonpsv.h"
#include "xpm_ioctl.h"
#include "xpm_ipi.h"
#include "xpm_psm_api.h"
#include "xpm_powerdomain.h"
#include "xpm_pmcdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_pldomain.h"
#include "xpm_npdomain.h"
#include "xpm_pll.h"
#include "xpm_reset.h"
#include "xpm_domain_iso.h"
#include "xpm_pmc.h"
#include "xpm_apucore.h"
#include "xpm_rpucore.h"
#include "xpm_psm.h"
#include "xpm_periph.h"
#include "xpm_mem.h"
#include "xpm_debug.h"

/* Macro to typecast PM API ID */
#define PM_API(ApiId)			((u32)ApiId)

#define PM_IOCTL_FEATURE_BITMASK ( \
	(1ULL << (u64)IOCTL_GET_APU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_SET_APU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_GET_RPU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_SET_RPU_OPER_MODE) | \
	(1ULL << (u64)IOCTL_RPU_BOOT_ADDR_CONFIG) | \
	(1ULL << (u64)IOCTL_SET_TAPDELAY_BYPASS) | \
	(1ULL << (u64)IOCTL_SD_DLL_RESET) | \
	(1ULL << (u64)IOCTL_SET_SD_TAPDELAY) | \
	(1ULL << (u64)IOCTL_SET_PLL_FRAC_MODE) | \
	(1ULL << (u64)IOCTL_GET_PLL_FRAC_MODE) | \
	(1ULL << (u64)IOCTL_SET_PLL_FRAC_DATA) | \
	(1ULL << (u64)IOCTL_GET_PLL_FRAC_DATA) | \
	(1ULL << (u64)IOCTL_WRITE_GGS) | \
	(1ULL << (u64)IOCTL_READ_GGS) | \
	(1ULL << (u64)IOCTL_WRITE_PGGS) | \
	(1ULL << (u64)IOCTL_READ_PGGS) | \
	(1ULL << (u64)IOCTL_SET_BOOT_HEALTH_STATUS) | \
	(1ULL << (u64)IOCTL_OSPI_MUX_SELECT) | \
	(1ULL << (u64)IOCTL_USB_SET_STATE) | \
	(1ULL << (u64)IOCTL_GET_LAST_RESET_REASON))

#define PM_QUERY_FEATURE_BITMASK ( \
	(1ULL << (u64)XPM_QID_CLOCK_GET_NAME) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_TOPOLOGY) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_MUXSOURCES) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_ATTRIBUTES) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_NUM_CLOCKS) | \
	(1ULL << (u64)XPM_QID_CLOCK_GET_MAX_DIVISOR))

u32 ResetReason;
static XPlmi_ModuleCmd XPlmi_PmCmds[PM_API_MAX];
static XPlmi_Module XPlmi_Pm =
{
	XPLMI_MODULE_XILPM_ID,
	XPlmi_PmCmds,
	PM_API(PM_API_MAX),
	NULL,
	NULL,
};

void (*PmRequestCb)(const u32 SubsystemId, const XPmApiCbId_t EventId, u32 *Payload);

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

static int XPm_ProcessCmd(XPlmi_Cmd * Cmd)
{
	//changed to support minimum boot time xilpm

	u32 ApiResponse[XPLMI_CMD_RESP_SIZE-1] = {0};
	u32 CmdId = Cmd->CmdId & 0xFFU;
	XStatus Status = XST_FAILURE;
	const u32 *Pload = Cmd->Payload;
	u32 SetAddress;
	u64 Address;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 Len = Cmd->Len;
	PmDbg("Processing Cmd: 0x%x\r\n",Cmd->CmdId);

	switch (CmdId) {
	case PM_API(PM_GET_CHIPID):
		Status = XPm_GetChipID(&ApiResponse[0], &ApiResponse[1]);
		break;
	case PM_API(PM_GET_API_VERSION):
		Status = XPm_GetApiVersion(ApiResponse);
		break;
	case PM_API(PM_INIT_NODE):
		Status = XPm_InitNode(Pload[0], Pload[1], &Pload[2], Len-2U);
		break;
	case PM_API(PM_SELF_SUSPEND):
		Status = XPm_SelfSuspend(SubsystemId, Pload[0],
					 Pload[1], (u8)Pload[2],
					 Pload[3], Pload[4]);
		break;
	case PM_API(PM_ISO_CONTROL):
		Status = XPm_IsoControl(Pload[0], Pload[1]);
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
	case PM_API(PM_BISR):
		Status =  XPmBisr_Repair(Pload[0]);
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
	case PM_API(PM_APPLY_TRIM):
		Status = XPm_PldApplyTrim(Pload[0]);
		break;
	case PM_API(PM_ADD_SUBSYSTEM):
		Status = XPm_AddSubsystem(Pload[0]);
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
	case PM_API(PM_FEATURE_CHECK):
		Status = XPm_FeatureCheck(Pload[0], ApiResponse);
		break;
	case PM_API(PM_REQUEST_NODE):
		Status = XPm_RequestDevice(SubsystemId, Pload[0], Pload[1],
					   Pload[2], Pload[3], Cmd->IpiReqType);
		break;
	case PM_API(PM_RELEASE_NODE):
		Status = XPm_ReleaseDevice(SubsystemId, Pload[0], Cmd->IpiReqType);
		break;
	case PM_API(PM_GET_NODE_STATUS):
		Status = XPm_GetDeviceStatus(SubsystemId, Pload[0], (XPm_DeviceStatus *)ApiResponse);
		break;
	case PM_API(PM_SET_REQUIREMENT):
		Status = XPm_SetRequirement(SubsystemId, Pload[0], Pload[1], Pload[2], Pload[3]);
		break;
	case PM_API(PM_SET_MAX_LATENCY):
		Status = XPm_SetMaxLatency(SubsystemId, Pload[0],
					   Pload[1]);
		break;
	case PM_API(PM_QUERY_DATA):
		Status = XPm_Query(Pload[0], Pload[1], Pload[2],
				   Pload[3], ApiResponse);
		break;
	case PM_API(PM_IOCTL):
		Status = XPm_DevIoctl(SubsystemId, Pload[0], (pm_ioctl_id) Pload[1], Pload[2],
				      Pload[3], Pload[4], ApiResponse, Cmd->IpiReqType);
		break;
	default:
		PmErr("CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	if (XST_SUCCESS == Status) {
		Cmd->ResumeHandler = NULL;
	} else {
		PmErr("Error 0x%x while processing command 0x%x\r\n",
				Status, Cmd->CmdId);
		PmDbg("Command payload: 0x%x, 0x%x, 0x%x, 0x%x\r\n",
			Pload[0], Pload[1], Pload[2], Pload[3]);
		goto done;
	}

	/* First word of the response is status */
	Cmd->Response[0] = (u32)Status;
	Status = Xil_SecureMemCpy(&Cmd->Response[1], sizeof(ApiResponse),
			ApiResponse, sizeof(ApiResponse));
	if (XST_SUCCESS != Status) {
		PmErr("Error 0x%x while copying the Cmd 0x%x return payload\r\n",
				Status, Cmd->CmdId);
	}

done:
	return Status;
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
	//changed to support minimum boot time xilpm

	XStatus Status = XST_FAILURE;
	(void)RequestCb;
	(void)RestartCb;
	u32 i;
	PmDbg("%s\n",__func__);
	for (i = 1; i < XPlmi_Pm.CmdCnt; i++) {
		XPlmi_PmCmds[i].Handler = XPm_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Pm);

	XPm_PsmModuleInit();
	Status = XPmSubsystem_Add(PM_SUBSYS_PMC);

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
	XStatus Status = XST_FAILURE;
	u32 Mask = 0;

	(void)SubsystemId;
	(void)Latency;
	(void)State;
	(void)AddrLow;
	(void)AddrHigh;

	/* TODO: Store resume address */

	/* TODO: Remove hard coded mask when available from topology */
	switch (DeviceId) {
	case PM_DEV_ACPU_0_0:
		Mask = APU0_CORE0_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_0_1:
		Mask = APU0_CORE1_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_0_2:
		Mask = APU0_CORE2_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_0_3:
		Mask = APU0_CORE3_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_1_0:
		Mask = APU1_CORE0_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_1_1:
		Mask = APU1_CORE1_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_1_2:
		Mask = APU1_CORE2_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_1_3:
		Mask = APU1_CORE3_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_2_0:
		Mask = APU2_CORE0_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_2_1:
		Mask = APU2_CORE1_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_2_2:
		Mask = APU2_CORE2_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_2_3:
		Mask = APU2_CORE3_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_3_0:
		Mask = APU3_CORE0_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_3_1:
		Mask = APU3_CORE1_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_3_2:
		Mask = APU3_CORE2_PWRDWN_MASK;
		break;
	case PM_DEV_ACPU_3_3:
		Mask = APU3_CORE3_PWRDWN_MASK;
		break;
	default:
		break;
	}

	ENABLE_WFI(Mask);

	XPm_Out32(PSMX_GLOBAL_SCAN_CLEAR_TRIGGER, 0U);
	XPm_Out32(PSMX_GLOBAL_MEM_CLEAR_TRIGGER, 0U);

	Status = XST_SUCCESS;

	return Status;
}

XStatus XPm_HookAfterPlmCdo(void)
{

	return XST_SUCCESS;

}

/****************************************************************************/
/**
 * @brief  This function allows to control isolation nodes.
 *
 * @param  NodeId	Isolation node id
 * @param  Enable	0: disable isolation
 * 			1: enable isolation
 * 			2: disable isolation immediately
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   none
 *
 ****************************************************************************/
XStatus XPm_IsoControl(u32 NodeId, u32 Enable)
{
	XStatus Status = XST_FAILURE;

	if (((u32)XPM_NODECLASS_ISOLATION != NODECLASS(NodeId)) ||
	    ((u32)XPM_NODEIDX_ISO_MAX <= NODEINDEX(NodeId))) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Status = XPmDomainIso_Control(NODEINDEX(NodeId), Enable);

done:
	if (XST_SUCCESS != Status) {
		PmErr("%s Err Code: 0x%x\r\n",__func__,Status);
	}
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function allows to initialize the node.
 *
 * @param  NodeId	Supported power domain nodes, PLD, AIE & Protection
 * nodes
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
	//changed to support minimum boot time xilpm

	XStatus Status = XST_FAILURE;
	(void)NodeId;
	(void)Function;
	(void)Args;
	(void)NumArgs;
	PmDbg("NodeId: %x,Function %x\n",NodeId,Function);
	if((XPM_NODEIDX_POWER_LPD == NODEINDEX(NodeId))&&(FUNC_INIT_FINISH == Function)){
		/*
		 * Mark domain init status bit in DomainInitStatusReg
		 */
		XPm_RMW32(XPM_DOMAIN_INIT_STATUS_REG,0x2,0x2);
		XPlmi_LpdInit();
	#ifdef XPAR_XIPIPSU_0_DEVICE_ID
		Status = XPlmi_IpiInit(XPmSubsystem_GetSubSysIdByIpiMask);
		if (XST_SUCCESS != Status) {
			PmErr("Error %u in IPI initialization\r\n", Status);
		}
	#else
		PmWarn("IPI is not enabled in design\r\n");
	#endif
	}else{
		PmErr("UnSupported Node %x\n",NodeId);
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Status = XST_SUCCESS;
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
 * @param ClusterId A78 or R52 Cluster Id
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
	//changed to support minimum boot time xilpm
	XStatus Status=XST_FAILURE;
	/* Warning Fix */
	(void) (Ack);
	(void)CmdType;
	(void)SubsystemId;
	(void)SetAddress;
	(void)Address;
	u32 Payload[2], Idx;
	PmDbg("DeviceId %x\n",DeviceId);

	/*TBD: move this to XPmCore_StoreResumeAddr when topology cdo is ready*/
	/* Set reset address */
	if (1U == SetAddress) {
		for (Idx = 0U; Idx < ARRAY_SIZE(ProcDevList); Idx++) {
			if(ProcDevList[Idx]==DeviceId){
				/* Store the resume address to PSM reserved RAM location */
				PsmToPlmEvent->ResumeAddress[Idx] = Address;
			}
		}
	}

	if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)) ||
		((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(DeviceId))){
		Payload[0] = PSM_API_DIRECT_PWR_UP;
		Payload[1] = DeviceId;
		Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	}

	if((u32)XPM_NODETYPE_DEV_CORE_PSM == NODETYPE(DeviceId)) {
		XPm_RMW32(CRL_PSM_RST_MODE_ADDR, CRL_PSM_RST_WAKEUP_MASK,
						CRL_PSM_RST_WAKEUP_MASK);
		Status = XPm_PollForMask(PSMX_GLOBAL_REG_GLOBAL_CNTRL,
						 PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
						 XPM_MAX_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
				goto done;
		}
		Status = XPm_GetPsmToPlmEventAddr();
		if (XST_SUCCESS != Status) {
				goto done;
		}

	}
done:
	if (XST_SUCCESS != Status) {
		PmErr("Err Code 0x%x\n\r",Status);
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
	//changed to support minimum boot time xilpm
	PmDbg("SubsystemId %x\n",SubsystemId);
	(void)SubsystemId;
	(void)Type;
	(void)SubType;
	(void)CmdType;
	//this service is not supported at boot time
	PmErr("unsupported service\n");
	return XST_FAILURE;

}

static void XPm_RpuCoreConfig(u8 ClusterNum, u8 CoreNum, u64 HandoffAddr,
		u32 *RstRpuMask)
{
	u32 RpuCfg0Addr = XPM_GET_RPU_CLUSTER_CORE_REG(ClusterNum, CoreNum,
				RPU_CLUSTER_CORE_CFG0_OFFSET);
	u32 VecTableAddr = XPM_GET_RPU_CLUSTER_CORE_REG(ClusterNum, CoreNum,
				RPU_CLUSTER_CORE_VECTABLE_OFFSET);
	u32 Address = (u32)(HandoffAddr & 0xFFFFFFE0U);

	/* Disable address remap */
	PmRmw32(RpuCfg0Addr, RPU_CLUSTER_CORE_CFG0_REMAP_MASK, 0U);
	PmRmw32(RpuCfg0Addr, RPU_CLUSTER_CORE_CFG0_CPU_HALT_MASK, 1U);
	/* Configure Vector Table Address */
	PmRmw32(VecTableAddr, RPU_CLUSTER_CORE_VECTABLE_MASK,
				Address);
	*RstRpuMask |= GetRpuRstMask(RPU_CORE0A_POR_MASK, ClusterNum, CoreNum) |
			GetRpuRstMask(RPU_CORE0A_RESET_MASK, ClusterNum, CoreNum);
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

	if(XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(DeviceId)){
		static u8 EccInitDone[2]={0};
		u32 RstRpuMask = 0U;
		u64 Tcm_Addr = (DeviceId == PM_DEV_RPU_A_0) || (DeviceId == PM_DEV_RPU_B_0)?XPM_R52_0A_TCMA_BASE_ADDR:
						XPM_R52_1A_TCMA_BASE_ADDR;
		u8 Ecc_Mask = (DeviceId == PM_DEV_RPU_A_0) || (DeviceId == PM_DEV_RPU_B_0)?XPM_R52_0_TCMA_ECC_DONE:
						XPM_R52_1_TCMA_ECC_DONE;
		u8 ClusterNum = GET_RPU_CLUSTER_ID(DeviceId);
		u32 LockstepVal;
		/*SPP_TBD: Need to find a place to clear RpuClusterState*/
		static u8 RpuClusterState[2U] = {0U};
		u8 CoreNum = GET_RPU_CORE_NUM(DeviceId);
		LockstepVal = XPm_In32((RPU_CLUSTER_BASEADDR+(ClusterNum*RPU_CLUSTER_OFFSET)+XPM_RPU_CLUSTER_CFG_OFFSET))&0x1;
		/* Skip cluster configuration if cluster is already configured */
		if (RpuClusterState[ClusterNum] != XPM_R52_CLUSTER_CONFIGURED) {
			RstRpuMask = (RPU_A_TOPRESET_MASK | RPU_A_DBGRST_MASK) << ClusterNum;
			if (LockstepVal != 1) {
				RstRpuMask |= RPU_A_DCLS_TOPRESET_MASK << ClusterNum;
			}
			PmRmw32(CRL_RST_RPU_ADDR, RstRpuMask, 0U);
			RpuClusterState[ClusterNum] = XPM_R52_CLUSTER_CONFIGURED;
		}
		/* Configure Lockstep and bring cluster, cores out of reset */
		if (LockstepVal != 0) {
			XPm_RpuCoreConfig(ClusterNum, CoreNum, 0, &RstRpuMask);
		}
		else {
			XPm_RpuCoreConfig(ClusterNum, XPM_RPU_CORE0, 0,
					&RstRpuMask);
			XPm_RpuCoreConfig(ClusterNum, XPM_RPU_CORE1, 0,
					&RstRpuMask);
		}
		PmRmw32(CRL_RST_RPU_ADDR, RstRpuMask, 0U);
		if ((EccInitDone[ClusterNum] & Ecc_Mask) !=
					Ecc_Mask) {
			XPlmi_EccInit(Tcm_Addr +
				(ClusterNum * XPM_R52_TCM_CLUSTER_OFFSET),
							XPM_R52_TCM_TOTAL_LENGTH);
			EccInitDone[ClusterNum] |= Ecc_Mask;
		}

	}
	Status = XST_SUCCESS;

	if(XST_SUCCESS != Status){
		PmErr("Err Code 0x%x\n",Status);
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
	case (u32)XPM_QID_PINCTRL_GET_NUM_PINS:
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTIONS:
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS:
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_NAME:
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_GROUPS:
	case (u32)XPM_QID_PINCTRL_GET_PIN_GROUPS:
	case (u32)XPM_QID_PLD_GET_PARENT:
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
XStatus XPm_DevIoctl(const u32 SubsystemId, const u32 DeviceId, const pm_ioctl_id  IoctlId,
		     const u32 Arg1, const u32 Arg2, const u32 Arg3, u32 *const Response,
		     const u32 CmdType)
{
	XStatus Status;

	Status = XPm_Ioctl(SubsystemId, DeviceId, IoctlId, Arg1, Arg2, Arg3,
			   Response, CmdType);
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

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

	if ((NODEINDEX(PowerId) >= (u32)XPM_NODEIDX_POWER_MAX)) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else {
		/* Required by MISRA */
	}

	BitMask = BITNMASK(Shift, Width);

	if ((NODEINDEX(ParentId) != (u32)XPM_NODEIDX_POWER_MIN)) {
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
		Power = (XPm_Power *)XPm_AllocBytes(sizeof(XPm_Power));
		if (NULL == Power) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPower_Init(Power, PowerId, BitMask,
			PowerParent);
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
		Status = XST_INVALID_PARAM;
		break;
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
	XStatus Status = XST_FAILURE;
	u32 NodeId;
	char Name[MAX_NAME_BYTES] = {0};
	u32 i=0U, j=0U;
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
	u8 TopologyType, NumCustomNodes=0, NumParents, ClkFlags;

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
	u32 GicProxyMask;
	u32 GicProxyGroup;
	XPm_Periph *PeriphDevice;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];
	GicProxyMask = Args[3];
	GicProxyGroup = Args[4];

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	PeriphDevice = (XPm_Periph *)XPm_AllocBytes(sizeof(XPm_Periph));
	if (NULL == PeriphDevice) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Status = XPmPeriph_Init(PeriphDevice, DeviceId, BaseAddr, Power, NULL, NULL,
				GicProxyMask, GicProxyGroup);

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

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_OCM:
	case (u32)XPM_NODETYPE_DEV_TCM:
	case (u32)XPM_NODETYPE_DEV_DDR:
		Device = (XPm_MemDevice *)XPm_AllocBytes(sizeof(XPm_MemDevice));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmMemDevice_Init(Device, DeviceId, BaseAddr, Power, NULL, NULL, StartAddr, EndAddr);
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
	u32 PowerId = 0U;

	if (1U > NumArgs) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DeviceId = Args[0];
	SubClass = NODESUBCLASS(DeviceId);

	if (1U < NumArgs) {
		/*
		 * Check for Num Args < 3U as device specific (except PLDevice)
		 * AddNode functions currently don't implement any NumArgs checks
		 */
		if (3U > NumArgs) {
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
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	/*TBD: add device security, virtualization and coherency attributes */

done:
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

	u32 Dependencies[PM_ISO_MAX_NUM_DEPENDENCIES] = {0U};
	u32 BaseAddr = 0U, Mask = 0U, NumDependencies = 0U;
	XPm_IsoPolarity Polarity = ACTIVE_HIGH;
	u8 Psm = (u8)0;

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
			/* Format for isolation control by single word*/
			if (Format == SINGLE_WORD_ACTIVE_LOW || \
			    Format == PSM_SINGLE_WORD_ACTIVE_LOW){
				Polarity = ACTIVE_LOW;
			}

			if (Format == PSM_SINGLE_WORD_ACTIVE_HIGH || \
			    Format == PSM_SINGLE_WORD_ACTIVE_LOW){
				Psm =  (u8)1;
			}

			/* Extract BaseAddress and Mask*/
			BaseAddr = Args[Index++];
			Mask = Args[Index++];
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

	Status = XPmDomainIso_NodeInit(NodeId, BaseAddr, Mask, \
				       Psm, Polarity, Dependencies, NumDependencies);
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
XStatus XPm_AddNode(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 Id = Args[0];

	switch (NODECLASS(Id)) {
	case (u32)XPM_NODECLASS_POWER:
		Status = XPm_AddNodePower(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_ISOLATION:
		Status = XPm_AddNodeIsolation(Args, NumArgs);
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
	default:
		Status = XST_INVALID_PARAM;
		break;
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
	XStatus Status = XST_FAILURE;
	Status = XPmSubsystem_Add(SubsystemId);

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
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
	PmDbg("ImageId %x SubsystemId %x\n",ImageId,SubsystemId);
	return SubsystemId;

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
	//changed to support minimum boot time xilpm
	PmDbg("%s: DeviceId %x\n",__func__,DeviceId);

	(void)DeviceId;
	(void)BaseAddr;
	//this service is not supported at boot time
	PmErr("unsupported service\n");
	return XST_FAILURE;
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
	XStatus Status = XST_FAILURE;

	if (NULL == Version) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	*Version = PM_VERSION;

	Status = XST_SUCCESS;
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
 * @brief  This function reset or de-reset a device. Alternatively a reset
 * 	   pulse can be requested as well.
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
 * @note   None
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
	case PM_API(PM_GET_OP_CHARACTERISTIC):
	case PM_API(PM_REQUEST_SUSPEND):
	case PM_API(PM_SELF_SUSPEND):
	case PM_API(PM_FORCE_POWERDOWN):
	case PM_API(PM_ABORT_SUSPEND):
	case PM_API(PM_REQUEST_WAKEUP):
	case PM_API(PM_SET_WAKEUP_SOURCE):
	case PM_API(PM_SYSTEM_SHUTDOWN):
	case PM_API(PM_REQUEST_NODE):
	case PM_API(PM_RELEASE_NODE):
	case PM_API(PM_SET_REQUIREMENT):
	case PM_API(PM_SET_MAX_LATENCY):
	case PM_API(PM_RESET_ASSERT):
	case PM_API(PM_RESET_GET_STATUS):
	case PM_API(PM_INIT_FINALIZE):
	case PM_API(PM_GET_CHIPID):
	case PM_API(PM_PINCTRL_REQUEST):
	case PM_API(PM_PINCTRL_RELEASE):
	case PM_API(PM_PINCTRL_GET_FUNCTION):
	case PM_API(PM_PINCTRL_SET_FUNCTION):
	case PM_API(PM_PINCTRL_CONFIG_PARAM_GET):
	case PM_API(PM_PINCTRL_CONFIG_PARAM_SET):
	case PM_API(PM_CLOCK_ENABLE):
	case PM_API(PM_CLOCK_DISABLE):
	case PM_API(PM_CLOCK_GETSTATE):
	case PM_API(PM_CLOCK_SETDIVIDER):
	case PM_API(PM_CLOCK_GETDIVIDER):
	case PM_API(PM_CLOCK_SETPARENT):
	case PM_API(PM_CLOCK_GETPARENT):
	case PM_API(PM_CLOCK_GETRATE):
	case PM_API(PM_PLL_SET_PARAMETER):
	case PM_API(PM_PLL_GET_PARAMETER):
	case PM_API(PM_PLL_SET_MODE):
	case PM_API(PM_PLL_GET_MODE):
	case PM_API(PM_ADD_SUBSYSTEM):
	case PM_API(PM_DESTROY_SUBSYSTEM):
	case PM_API(PM_DESCRIBE_NODES):
	case PM_API(PM_ADD_NODE):
	case PM_API(PM_ADD_NODE_PARENT):
	case PM_API(PM_ADD_NODE_NAME):
	case PM_API(PM_ADD_REQUIREMENT):
	case PM_API(PM_INIT_NODE):
	case PM_API(PM_FEATURE_CHECK):
		*Version = XST_API_BASE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_QUERY_DATA):
		Version[0] = XST_API_QUERY_DATA_VERSION;
		Version[1] = (u32)(PM_QUERY_FEATURE_BITMASK);
		Version[2] = (u32)(PM_QUERY_FEATURE_BITMASK >> 32);
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_REGISTER_NOTIFIER):
		*Version = XST_API_REG_NOTIFIER_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_IOCTL):
		Version[0] = XST_API_PM_IOCTL_VERSION;
		Version[1] = (u32)(PM_IOCTL_FEATURE_BITMASK);
		Version[2] = (u32)(PM_IOCTL_FEATURE_BITMASK >> 32);
		Status = XST_SUCCESS;
		break;
	default:
		*Version = 0U;
		Status = XPM_NO_FEATURE;
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
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem* Subsystem = NULL;
	const XPm_Device* Device = NULL;
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

	Status = XPmDevice_Release(SubsystemId, DeviceId, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Usage = XPmDevice_GetUsageStatus(Subsystem, Device);
	if (0U == Usage) {
		/* TODO: Send notifier event */
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
