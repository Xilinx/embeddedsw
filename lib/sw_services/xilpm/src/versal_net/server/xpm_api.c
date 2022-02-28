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
	static u8 ApuClusterState[4U] = {0U};
	u8 ClusterId;
	(void)CmdType;
	(void)SubsystemId;
	(void)SetAddress;
	(void)Address;
	PmDbg("DeviceId %x\n",DeviceId);

	/*SPP_TBD: these register writes need to be moved to PSMFW when IPI is enables*/
	if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)){
		ClusterId = GET_APU_CLUSTER_ID(DeviceId);
		u32 LowAddress = (u32)(Address & 0xFFFFFFFCUL);
		u32 HighAddress = (u32)(Address >> 32UL);
		u32 CoreNum = GET_APU_CORE_NUM(DeviceId);
		u32 PcliCoreNum = (ClusterId * 4U) + CoreNum;
		u32 RstApuOffset = PSX_CRF_RST_APU0 + (ClusterId * RST_APU_REG_OFFSET);
		u32 ApuCoreOffset = (CoreNum * 8U);

		/* Skip cluster configuration if cluster is already configured */
		if (ApuClusterState[ClusterId] != XPM_A78_CLUSTER_CONFIGURED) {
			/* APU PSTATE, PREQ configuration */
			XPm_RMW32(GET_APU_PCLI_CLUSTER_REG(ClusterId,
				APU_PCLI_CLUSTER_PSTATE_OFFSET),
				APU_PCLI_CLUSTER_PSTATE_PSTATE_MASK,
				APU_CLUSTER_PSTATE_FULL_ON_VAL);
			XPm_RMW32(GET_APU_PCLI_CLUSTER_REG(ClusterId,
				APU_PCLI_CLUSTER_PREQ_OFFSET), APU_PCLI_CLUSTER_PREQ_PREQ_MASK,
				APU_PCLI_CLUSTER_PREQ_PREQ_MASK);
			/* ACPU clock config */
			XPm_RMW32(PSX_CRF_ACPU0_CLK_CTRL +
					(ClusterId * ACPU_CLK_CTRL_REG_OFFSET),
					ACPU0_CLK_CTRL_CLKACT_MASK, ACPU0_CLK_CTRL_CLKACT_VAL);
			/* APU cluster release cold & warm reset */
			XPm_RMW32(RstApuOffset, (APU_CLUSTER_WARM_RESET_MASK |
				APU_CLUSTER_COLD_RESET_MASK), 0U);

			Status = XPm_PollForMask(GET_APU_PCLI_CLUSTER_REG(ClusterId,
				APU_PCLI_CLUSTER_PACTIVE_OFFSET),
				APU_PCLI_CLUSTER_PACTIVE_PACCEPT_MASK, 1000U);
			if (Status != XST_SUCCESS) {
				PmErr("A78 Cluster PACCEPT timeout1..\n");
				goto done;
			}
			ApuClusterState[ClusterId] = XPM_A78_CLUSTER_CONFIGURED;
		}

		XPm_Out32(GET_APU_CLUSTER_REG(ClusterId,
			APU_CLUSTER_RVBARADDR0L_OFFSET) + ApuCoreOffset, LowAddress);
		XPm_Out32(GET_APU_CLUSTER_REG(ClusterId,
			APU_CLUSTER_RVBARADDR0H_OFFSET) + ApuCoreOffset, HighAddress);

		XPm_RMW32(GET_APU_PCLI_CORE_REG(PcliCoreNum, APU_PCLI_CORE_PSTATE_OFFSET),
			APU_PCLI_CORE_PSTATE_PSTATE_MASK, APU_CORE_PSTATE_FULL_ON_VAL);
		XPm_RMW32(GET_APU_PCLI_CORE_REG(PcliCoreNum, APU_PCLI_CORE_PREQ_OFFSET),
			APU_PCLI_CORE_PREQ_PREQ_MASK, APU_PCLI_CORE_PREQ_PREQ_MASK);

		/* APU core release warm reset */
		XPm_RMW32(RstApuOffset,
			(u32)(1U << (CoreNum + APU_CORE_WARM_RESET_SHIFT)), 0U);
		Status = XPm_PollForMask(GET_APU_PCLI_CORE_REG(PcliCoreNum,
				APU_PCLI_CORE_PACTIVE_OFFSET),
				APU_PCLI_CORE_PACTIVE_PACCEPT_MASK, 1000U);
		if (Status != XST_SUCCESS) {
			PmErr("A78 core PACCEPT timeout2..\n\r");
			goto done;
		}

	}

	if((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(DeviceId)){
		ClusterId = GET_RPU_CLUSTER_ID(DeviceId);
		if(((u32)0x13 == NODEINDEX(DeviceId)) || ((u32)0x15 == NODEINDEX(DeviceId))){
			XPm_RMW32((RPU_CLUSTER_BASEADDR+(ClusterId*0x10000)), 0x1, 0);
		}
		if(((u32)0x14 == NODEINDEX(DeviceId)) || ((u32)0x16 == NODEINDEX(DeviceId))){
			XPm_RMW32((RPU_CLUSTER_BASEADDR+(ClusterId*0x10000)+(1*0x100)), 0x1, 0);
		}
		Status = XST_SUCCESS;
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
	u32 RpuCfg0Addr = GET_RPU_CLUSTER_CORE_REG(ClusterNum, CoreNum,
				RPU_CLUSTER_CORE_CFG0_OFFSET);
	u32 VecTableAddr = GET_RPU_CLUSTER_CORE_REG(ClusterNum, CoreNum,
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
		u64 Tcm_Addr = (DeviceId == PM_DEV_CLUSTER0_RPU0_0) || (DeviceId == PM_DEV_CLUSTER1_RPU0_0)?XPM_R52_0A_TCMA_BASE_ADDR:
						XPM_R52_1A_TCMA_BASE_ADDR;
		u8 Ecc_Mask = (DeviceId == PM_DEV_CLUSTER0_RPU0_0) || (DeviceId == PM_DEV_CLUSTER1_RPU0_0)?XPM_R52_0_TCMA_ECC_DONE:
						XPM_R52_1_TCMA_ECC_DONE;
		u8 ClusterNum = GET_RPU_CLUSTER_ID(DeviceId);
		u32 LockstepVal;
		/*SPP_TBD: Need to find a place to clear RpuClusterState*/
		static u8 RpuClusterState[2U] = {0U};
		u8 CoreNum = GET_RPU_CORE_NUM(DeviceId);
		LockstepVal = XPm_In32((RPU_CLUSTER_BASEADDR+(ClusterNum*RPU_CLUSTER_OFFSET)+RPU_CLUSTER_CFG_OFFSET))&0x1;
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
		PmOut32((RPU_CLUSTER_BASEADDR+(Cluster*RPU_CLUSTER_OFFSET)+RPU_CLUSTER_CFG_OFFSET),LockStep);
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
