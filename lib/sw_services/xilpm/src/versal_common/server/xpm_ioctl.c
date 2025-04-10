/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi.h"
#include "xpm_apucore.h"
#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_ioctl.h"
#include "xpm_rpucore.h"
#include "xpm_pmc.h"
#include "xpm_power.h"
#include "xpm_psm.h"
#include "xpm_regs.h"
#include "sleep.h"
#include "xpm_api.h"
#include "xpm_device.h"
#include "xpm_requirement.h"

static u32 PsmGgsValues[GGS_REGS] = {0U};

static u32 PggsReadPermissions[PMC_PGGS_REGS];
static u32 PggsWritePermissions[PMC_PGGS_REGS];
static u32 GgsReadPermissions[GGS_REGS];
static u32 GgsWritePermissions[GGS_REGS];

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
static XStatus XPm_SetTapdelayBypass(const u32 DeviceId, const u32 Type,
				 const u32 Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);
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

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
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
static XStatus XPm_SdDllReset(const u32 DeviceId, const u32 Type)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	u32 BaseAddress;
	u32 Offset;

	if (NULL == Pmc) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/* PMC_IOU_SLCR base address */
	BaseAddress = Pmc->PmcIouSlcrBaseAddr;

	if (PM_DEV_SDIO_0 == DeviceId) {
		Offset = SD0_DLL_CTRL_OFFSET;
	} else if (PM_DEV_SDIO_1 == DeviceId) {
		Offset = SD1_DLL_CTRL_OFFSET;
	} else {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	switch (Type) {
	case XPM_DLL_RESET_ASSERT:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			XPM_SD_DLL_RST_MASK);
		Status = XST_SUCCESS;
		break;
	case XPM_DLL_RESET_RELEASE:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			~XPM_SD_DLL_RST_MASK);
		Status = XST_SUCCESS;
		break;
	case XPM_DLL_RESET_PULSE:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			XPM_SD_DLL_RST_MASK);
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			~XPM_SD_DLL_RST_MASK);
		Status = XST_SUCCESS;
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
 * @brief  This function sets input/output tap delay for the SD device.
 *
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Type of tap delay to set (input/output)
 * @param  Value	Value to set for the tap delay
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static XStatus XPm_SetSdTapDelay(const u32 DeviceId, const u32 Type,
			     const u32 Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);
	u32 BaseAddress;

	if (NULL == Device) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/*Check for Type */
	if ((XPM_TAPDELAY_INPUT != Type) && (XPM_TAPDELAY_OUTPUT != Type)) {
		Status = XPM_INVALID_TYPEID;
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
		/*No action taken because we check for type earlier in the function
		 * but present as part of defensive programming in case we reach here */
		break;
	}

	Status = XPm_SdDllReset(DeviceId, XPM_DLL_RESET_RELEASE);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the controller into either D3 or D0 state
 *
 * @param  SubsystemId	SubsystemId of the subsystem
 * @param  DeviceId	DeviceId of the device
 * @param  ReqState	Requested State (0 - D0, 3 - D3)
 * @param  TimeOut	TimeOut value in micro secs to wait for D3/D0 entry
 *
 * @return XST_SUCCESS if successful else error code
 *
 ****************************************************************************/
static XStatus XPm_USBDxState(const u32 SubsystemId, const u32 DeviceId,
			      const u32 ReqState, const u32 TimeOut)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc;
	u32 BaseAddress;
	u32 Offset;
	u32 CurState;
	u32 LocalTimeOut;

	if (!IS_DEV_USB(DeviceId)) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	LocalTimeOut = TimeOut;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Only (D0 == 0U) or (D3 == 3U) states allowed */
	if ((0U != ReqState) && (3U != ReqState)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* PMC_IOU_SLCR base address */
	BaseAddress = Pmc->PmcIouSlcrBaseAddr;;

	/* power state request */
	Offset = XPm_GetUsbPwrReqOffset(DeviceId);
	PmOut32(BaseAddress + Offset, ReqState);

	/* current power state */
	Offset = XPm_GetUsbCurrPwrOffset(DeviceId);
	PmIn32(BaseAddress + Offset, CurState);

	while((CurState != ReqState) && (0U < LocalTimeOut)) {
		LocalTimeOut--;
		PmIn32(BaseAddress + Offset, CurState);
		usleep(1U);
	}

	Status = ((LocalTimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function selects/returns the AXI interface to OSPI device
 *
 * @param  SubsystemId	SubsystemId of the subsystem
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Reset type
 * @param  Response	Output Response (0 - DMA, 1 - LINEAR)
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static XStatus XPm_OspiMuxSelect(const u32 SubsystemId, const u32 DeviceId,
				 const u32 Type, u32 *Response)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc;
	u32 BaseAddress;
	u32 Offset;

	if (PM_DEV_OSPI != DeviceId) {
		goto done;
	}

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* PMC_IOU_SLCR base address */
	BaseAddress = Pmc->PmcIouSlcrBaseAddr;
	Offset = XPM_OSPI_MUX_SEL_OFFSET;

	switch (Type) {
	case XPM_OSPI_MUX_SEL_DMA:
		PmRmw32(BaseAddress + Offset, XPM_OSPI_MUX_SEL_MASK,
			~XPM_OSPI_MUX_SEL_MASK);
		Status = XST_SUCCESS;
		break;
	case XPM_OSPI_MUX_SEL_LINEAR:
		PmRmw32(BaseAddress + Offset, XPM_OSPI_MUX_SEL_MASK,
			XPM_OSPI_MUX_SEL_MASK);
		Status = XST_SUCCESS;
		break;
	case XPM_OSPI_MUX_GET_MODE:
		if (NULL == Response) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		PmIn32(BaseAddress + Offset, *Response);
		*Response = (((*Response) & XPM_OSPI_MUX_SEL_MASK) >>
			     XPM_OSPI_MUX_SEL_SHIFT);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus XPmIoctl_IsRegRequested(u32 SubsystemId, u32 Register, u32 Type)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Reqm = NULL;
	u32 DeviceId;
	u32 NodeClass = (u32)XPM_NODECLASS_DEVICE;
	u32 NodeSubClass = (u32)XPM_NODESUBCL_DEV_PERIPH;
	u32 RegNum = Register;

	switch (Type) {
		case (u32)XPM_NODETYPE_DEV_PGGS:
			/*
			 * +1 is needed as the first PGGS Node ID is after the last
			 * GGS Node ID.
			 */
			RegNum += (u32)GGS_MAX + 1U;
			break;
		case (u32)XPM_NODETYPE_DEV_GGS:
			break;
		default:
			Status = XPM_INVALID_TYPEID;
			break;
	}

	if (XPM_INVALID_TYPEID == Status) {
		goto done;
	}

	DeviceId = NODEID(NodeClass, NodeSubClass, Type, RegNum);

	if (NULL == XPmDevice_GetById(DeviceId)) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	if (NULL == XPmSubsystem_GetById(SubsystemId)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Reqm = XPmDevice_FindRequirement(DeviceId, SubsystemId);
	if ((NULL == Reqm) || (1U != Reqm->Allocated)) {
		Status = XPM_ERR_DEVICE_STATUS;
		goto done;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmIoctl_IsOperationAllowed(u32 RegNum, u32 SubsystemId,
		const u32 *Perms, u32 Type, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	u32 PermissionMask = 0;
	u32 SecureMask;
	u32 NonSecureMask;

	/*
	 * RegNum is validated later in each operation so do not need to
	 * validate here in case of PMC subsystem.
	 */
	if (PM_SUBSYS_PMC == SubsystemId) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Other subsystems have RegNum validated in this function. */
	Status = XPmIoctl_IsRegRequested(SubsystemId, RegNum, Type);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PermissionMask = Perms[RegNum];
	SecureMask = ((u32)1U << SUBSYS_TO_S_BITPOS(SubsystemId));
	NonSecureMask = ((u32)1U << SUBSYS_TO_NS_BITPOS(SubsystemId));

	/* Have Target check if Host can enact the operation */
	if ((XPLMI_CMD_SECURE == CmdType) &&
			(SecureMask == (PermissionMask & SecureMask))) {
		Status = XST_SUCCESS;
	} else if (NonSecureMask == (PermissionMask & NonSecureMask)) {
		Status = XST_SUCCESS;
	} else {
		Status = XPM_PM_NO_ACCESS;
	}

done:
	return Status;
}

static XStatus XPm_ReadPggs(const u32 SubsystemId, const u32 PggsNum,
			    u32 *const Value, const u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	if (NULL == Pmc) {
		goto done;
	}

	if (PMC_PGGS_REGS <= PggsNum) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmIoctl_IsOperationAllowed(PggsNum, SubsystemId,
					     PggsReadPermissions,
					     (u32)XPM_NODETYPE_DEV_PGGS,
					     CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Map PGGS0-1 to PMC_GLOBAL_PGGS3-4 */
	PmIn32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PGGS3_OFFSET +
	       (PggsNum << 2U)), *Value);

done:
	return Status;
}

static XStatus XPm_WritePggs(const u32 SubsystemId, const u32 PggsNum,
			     const u32 Value, const u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	if (NULL == Pmc) {
		goto done;
	}

	if (PMC_PGGS_REGS <= PggsNum) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmIoctl_IsOperationAllowed(PggsNum, SubsystemId,
					     PggsWritePermissions,
					     (u32)XPM_NODETYPE_DEV_PGGS,
					     CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Map PGGS0-1 to PMC_GLOBAL_PGGS3-4 */
	PmOut32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PGGS3_OFFSET +
		(PggsNum << 2U)), Value);

done:
	return Status;
}

static XStatus XPm_ReadGgs(const u32 SubsystemId, const u32 GgsNum,
			   u32 *const Value, const u32 CmdType)
{
	XStatus Status = XST_FAILURE;

	if (GGS_REGS <= GgsNum) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmIoctl_IsOperationAllowed(GgsNum, SubsystemId,
					     GgsReadPermissions,
					     (u32)XPM_NODETYPE_DEV_GGS,
					     CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	*Value = PsmGgsValues[GgsNum];

done:
	return Status;
}

static XStatus XPm_WriteGgs(const u32 SubsystemId, const u32 GgsNum,
			    const u32 Value, const u32 CmdType)
{
	XStatus Status = XST_FAILURE;

	if (GGS_REGS <= GgsNum) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmIoctl_IsOperationAllowed(GgsNum, SubsystemId,
					     GgsWritePermissions,
					     (u32)XPM_NODETYPE_DEV_GGS,
					     CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PsmGgsValues[GgsNum] = Value;

done:
	return Status;
}

static XStatus XPm_SetBootHealth(const u32 Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		goto done;
	}

	PmRmw32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_GGS4_OFFSET,
		XPM_BOOT_HEALTH_STATUS_MASK, Value);

	Status = XST_SUCCESS;

done:
	return Status;
}

static void XPm_GetLastResetReason(u32 *const Response)
{
	if (CRP_RESET_REASON_SW_POR_MASK ==
			(ResetReason & CRP_RESET_REASON_SW_POR_MASK)) {
		*Response = XPM_RESET_REASON_SW_POR;
	} else if (CRP_RESET_REASON_SLR_POR_MASK ==
			(ResetReason & CRP_RESET_REASON_SLR_POR_MASK)) {
		*Response = XPM_RESET_REASON_SLR_POR;
	} else if (CRP_RESET_REASON_ERR_POR_MASK ==
			(ResetReason & CRP_RESET_REASON_ERR_POR_MASK)) {
		*Response = XPM_RESET_REASON_ERR_POR;
	} else if (CRP_RESET_REASON_DAP_SYS_MASK ==
			(ResetReason & CRP_RESET_REASON_DAP_SYS_MASK)) {
		*Response = XPM_RESET_REASON_DAP_SRST;
	} else if (CRP_RESET_REASON_SW_SYS_MASK ==
			(ResetReason & CRP_RESET_REASON_SW_SYS_MASK)) {
		*Response = XPM_RESET_REASON_SW_SRST;
	} else if (CRP_RESET_REASON_ERR_SYS_MASK ==
			(ResetReason & CRP_RESET_REASON_ERR_SYS_MASK)) {
		*Response = XPM_RESET_REASON_ERR_SRST;
	} else if (CRP_RESET_REASON_SLR_SYS_MASK ==
			(ResetReason & CRP_RESET_REASON_SLR_SYS_MASK)) {
		*Response = XPM_RESET_REASON_SLR_SRST;
	} else if (CRP_RESET_REASON_EXTERNAL_POR_MASK ==
			(ResetReason & CRP_RESET_REASON_EXTERNAL_POR_MASK)) {
		*Response = XPM_RESET_REASON_EXT_POR;
	} else {
		*Response = XPM_RESET_REASON_INVALID;
	}
}

static XStatus XPm_TapDelayOper(const pm_ioctl_id IoctlId, const u32 SubsystemId,
				const u32 DeviceId, const u32 Arg1, const u32 Arg2)
{
	XStatus Status = XST_FAILURE;

	if ((IOCTL_SET_TAPDELAY_BYPASS == IoctlId) && (PM_DEV_QSPI != DeviceId)) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (IOCTL_SET_TAPDELAY_BYPASS == IoctlId) {
		Status = XPm_SetTapdelayBypass(DeviceId, Arg1, Arg2);
	} else if (IOCTL_SD_DLL_RESET == IoctlId) {
		Status = XPm_SdDllReset(DeviceId, Arg1);
	} else if (IOCTL_SET_SD_TAPDELAY == IoctlId) {
		Status = XPm_SetSdTapDelay(DeviceId, Arg1, Arg2);
	} else {
		Status = XST_INVALID_PARAM;
	}

done:
	return Status;
}

XStatus XPm_Ioctl(const u32 SubsystemId, const u32 DeviceId, const pm_ioctl_id IoctlId,
	      const u32 Arg1, const u32 Arg2, const u32 Arg3,
	      u32 *const Response, const u32 CmdType)
{
	XStatus Status = XPM_ERR_IOCTL;
	const XPm_Requirement *Reqm;

	Status = XPm_ValidateDeviceId(IoctlId, DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	switch (IoctlId) {
	case IOCTL_GET_RPU_OPER_MODE:
		Status = XPm_RpuGetOperMode(DeviceId, Response);
		break;
	case IOCTL_SET_RPU_OPER_MODE:
		Status = XPm_RpuSetOperMode(DeviceId, Arg1);
		break;
	case IOCTL_RPU_BOOT_ADDR_CONFIG:
		Status = XPm_RpuBootAddrConfig(DeviceId, Arg1);
		break;
	case IOCTL_GET_APU_OPER_MODE:
		Status = XPm_ApuGetOperMode(DeviceId, Response);
		break;
	case IOCTL_SET_APU_OPER_MODE:
		Status = XPm_ApuSetOperMode(DeviceId, Arg1);
		break;
	case IOCTL_TCM_COMB_CONFIG:
		Status = XPm_RpuTcmCombConfig(DeviceId, Arg1);
		break;
	case IOCTL_AIE_ISR_CLEAR:
		Status = XPm_AieISRClear(SubsystemId, DeviceId, Arg1);
		break;
	case IOCTL_AIE_OPS:
	case IOCTL_AIE2PS_OPS:
		Status = XPm_AieOperation(SubsystemId, DeviceId, IoctlId, Arg1,
					  Arg2, Arg3);
		break;
	case IOCTL_READ_REG:
		Status = XPmAccess_ReadReg(SubsystemId, DeviceId, IoctlId,
					   Arg1, Arg2,
					   Response, CmdType);
		break;
	case IOCTL_MASK_WRITE_REG:
		Status = XPmAccess_MaskWriteReg(SubsystemId, DeviceId, IoctlId,
						Arg1, Arg2, Arg3,
						CmdType);
		break;
	case IOCTL_GET_QOS:
		Status = XPm_GetQos(DeviceId, IoctlId, Response);
		break;
	case IOCTL_SET_TAPDELAY_BYPASS:
	case IOCTL_SD_DLL_RESET:
	case IOCTL_SET_SD_TAPDELAY:
		Status = XPm_TapDelayOper(IoctlId, SubsystemId, DeviceId,
					  Arg1, Arg2);
		break;
	case IOCTL_WRITE_GGS:
		Status = XPm_WriteGgs(SubsystemId, Arg1, Arg2, CmdType);
		break;
	case IOCTL_READ_GGS:
		Status = XPm_ReadGgs(SubsystemId, Arg1, Response, CmdType);
		break;
	case IOCTL_WRITE_PGGS:
		Status = XPm_WritePggs(SubsystemId, Arg1, Arg2, CmdType);
		break;
	case IOCTL_READ_PGGS:
		Status = XPm_ReadPggs(SubsystemId, Arg1, Response, CmdType);
		break;
	case IOCTL_SET_BOOT_HEALTH_STATUS:
		Status = XPm_SetBootHealth(Arg1);
		break;
	case IOCTL_OSPI_MUX_SELECT:
		Status = XPm_OspiMuxSelect(SubsystemId, DeviceId, Arg1, Response);
		break;
	case IOCTL_USB_SET_STATE:
		Status = XPm_USBDxState(SubsystemId, DeviceId, Arg1, Arg2);
		break;
	case IOCTL_GET_LAST_RESET_REASON:
		XPm_GetLastResetReason(Response);
		Status = XST_SUCCESS;
		break;
	case IOCTL_SET_PLL_FRAC_MODE:
		Status = XPm_SetPllMode(SubsystemId, Arg1, Arg2);
		break;
	case IOCTL_GET_PLL_FRAC_MODE:
		Status = XPm_GetPllMode(Arg1, Response);
		break;
	case IOCTL_SET_PLL_FRAC_DATA:
		Status = XPm_SetPllParameter(SubsystemId, Arg1, (u32)PM_PLL_PARAM_ID_DATA, Arg2);
		break;
	case IOCTL_GET_PLL_FRAC_DATA:
		Status = XPm_GetPllParameter(Arg1, (u32)PM_PLL_PARAM_ID_DATA, Response);
		break;
	case IOCTL_PREPARE_DDR_SHUTDOWN:
		/* Put the DDR_0 device into self refresh */
		Reqm = XPmDevice_FindRequirement(DeviceId, SubsystemId);
		if ((NULL == Reqm) || (1U != Reqm->Allocated)) {
			Status = XPM_ERR_DEVICE_STATUS;
			goto done;
		}
		Status = XPmDevice_SetRequirement(SubsystemId, DeviceId,
					(u32)PM_CAP_CONTEXT, 0U);
		break;
	case IOCTL_GET_SSIT_TEMP:
		Status = XPm_GetSsitTemp(DeviceId, IoctlId, Arg1, Response);
		break;
	case IOCTL_ULPI_RESET:
	case IOCTL_AFI:
	case IOCTL_REGISTER_SGI:
	case IOCTL_SET_FEATURE_CONFIG:
	case IOCTL_GET_FEATURE_CONFIG:
	case IOCTL_SET_SD_CONFIG:
	case IOCTL_SET_GEM_CONFIG:
	case IOCTL_SET_USB_CONFIG:
	default:
		Status = XPM_ERR_IOCTL;
		break;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}

XStatus XPmIoctl_AddRegPermission(const XPm_Subsystem *Subsystem, u32 DeviceId,
		u32 Operations)
{
	XStatus Status = XST_FAILURE;
	u32 RegNum = NODEINDEX(DeviceId);
	u32 SubsystemId = Subsystem->Id;
	u32 Type = NODETYPE(DeviceId);
	u32 *ReadPerm, *WritePerm;
	const u32 AddNodeArgs[5U] = { DeviceId, PM_POWER_PMC, 0, 0, 0};
	const XPm_Device *Device;

	/* Ensure device is added before trying to use it. */
	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPm_AddNode(AddNodeArgs, ARRAY_SIZE(AddNodeArgs));
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Device = XPmDevice_GetById(DeviceId);
		if (NULL == Device) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
	}

	if (PM_SUBSYS_PMC == SubsystemId) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	switch (Type) {
		case (u32)XPM_NODETYPE_DEV_PGGS:
			/* Normalize to permissions indices range for PGGS */
			RegNum -= (u32)XPM_NODEIDX_DEV_PGGS_0;
			if (RegNum >= ARRAY_SIZE(PggsReadPermissions)) {
				Status = XST_INVALID_PARAM;
				goto done;
			}
			ReadPerm =  &PggsReadPermissions[RegNum];
			WritePerm = &PggsWritePermissions[RegNum];
			break;
		case (u32)XPM_NODETYPE_DEV_GGS:
			if (RegNum >= ARRAY_SIZE(GgsReadPermissions)) {
				Status = XST_INVALID_PARAM;
				goto done;
			}
			ReadPerm =  &GgsReadPermissions[RegNum];
			WritePerm = &GgsWritePermissions[RegNum];
			break;
		default:
			Status = XPM_INVALID_TYPEID;
			break;
	}

	if (XPM_INVALID_TYPEID == Status) {
		goto done;
	}

	*ReadPerm |= PERM_BITMASK(Operations, IOCTL_PERM_READ_SHIFT_NS,
			SUBSYS_TO_NS_BITPOS(SubsystemId));
	*ReadPerm |= PERM_BITMASK(Operations, IOCTL_PERM_READ_SHIFT_S,
			SUBSYS_TO_S_BITPOS(SubsystemId));
	*WritePerm |= PERM_BITMASK(Operations, IOCTL_PERM_WRITE_SHIFT_NS,
			SUBSYS_TO_NS_BITPOS(SubsystemId));
	*WritePerm |= PERM_BITMASK(Operations, IOCTL_PERM_WRITE_SHIFT_S,
			SUBSYS_TO_S_BITPOS(SubsystemId));

	Status = XST_SUCCESS;

done:
	return Status;
}
