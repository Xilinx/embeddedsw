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

XStatus XPm_IsoControl(u32 NodeId, u32 Enable)
{
	//changed to support minimum boot time xilpm

	XStatus Status = XST_FAILURE;
	(void)Enable;
	PmDbg("NodeId:%x, Enable: %x\r\n",NodeId,Enable);
	if(XPM_NODEIDX_ISO_FPD_PL_TEST == NODEINDEX(NodeId)){
		XPm_RMW32(0xF1120000,0x1,0);
	}else if(XPM_NODEIDX_ISO_FPD_PL == NODEINDEX(NodeId)){
		XPm_RMW32(0xF1120000,0x2,0);
	}else if(XPM_NODEIDX_ISO_LPD_PL_TEST == NODEINDEX(NodeId)){
		XPm_RMW32(0xF1120000,0x20,0);
	}else if(XPM_NODEIDX_ISO_LPD_PL == NODEINDEX(NodeId)){
		XPm_RMW32(0xF1120000,0x40,0);
	}else if(XPM_NODEIDX_ISO_PMC_PL_TEST == NODEINDEX(NodeId)){
		XPm_RMW32(0xF1120000,0x800,0);
	}else if(XPM_NODEIDX_ISO_PMC_PL == NODEINDEX(NodeId)){
		XPm_RMW32(0xF1120000,0x1000,0);
	}else{
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}
	Status = XST_SUCCESS;

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
	(void)SubsystemId;
	(void)DeviceId;
	(void)Capabilities;
	(void)QoS;
	(void)Ack;
	(void)CmdType;
	XStatus Status = XST_FAILURE;

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
	//changed to support minimum boot time xilpm
	PmDbg("SubsystemId %x,DeviceId %x\n",SubsystemId,DeviceId);
	(void)SubsystemId;
	(void)DeviceId;
	(void)CmdType;
	return XST_SUCCESS;

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
	/* Warning Fix */
	(void) (Arg3);
	//changed to support minimum boot time xilpm
	PmDbg("Qid %x\n",Qid);
	(void)Qid;
	(void)Arg1;
	(void)Arg2;
	(void)Output;
	//this service is not supported at boot time
	PmErr("unsupported service\n");
	return XST_FAILURE;

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
 * @param CmdType		IPI command request type
 * @param Cluster		Cluster Id of A78/R52
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
			const u32 Arg2, u32 *const Response,
			const u32 CmdType)
{
	//changed to support minimum boot time xilpm

	(void)SubsystemId;
	(void)DeviceId;
	(void)IoctlId;
	(void)Arg1;
	(void)Arg2;
	(void)Response;
	(void)CmdType;
	u32 LockStep;
	u32 Cluster = Arg2;
	PmDbg("IoctlId %x\n",IoctlId);
	if(XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(DeviceId)){
		LockStep = XPM_RPU_CLUSTER_LOCKSTEP_DISABLE;
		if (XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED != Arg1){
			LockStep = XPM_RPU_CLUSTER_LOCKSTEP_ENABLE;
		}
		PmOut32((RPU_CLUSTER_BASEADDR+(Cluster*RPU_CLUSTER_OFFSET)+XPM_RPU_CLUSTER_CFG_OFFSET),LockStep);
	}else if(XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)){
		LockStep = XPM_APU_CLUSTER_LOCKSTEP_DISABLE;
		if (XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED != Arg1){
			LockStep = XPM_APU_CLUSTER_LOCKSTEP_ENABLE;
		}
		XPm_RMW32(FPX_SLCR_APU_CTRL, (u32)(1U << Cluster),
			(u32)(LockStep << Cluster));
	}

	return XST_SUCCESS;
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
