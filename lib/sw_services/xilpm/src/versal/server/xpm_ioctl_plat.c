/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi.h"
#include "xpm_defs.h"
#include "xpm_ioctl.h"
#include "xpm_rpucore.h"
#include "xpm_pmc.h"
#include "xpm_power.h"
#include "xpm_regs.h"
#include "xpm_aie.h"
#include "xpm_api.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_access.h"
#include "xpm_aiedevice.h"
#include "xpm_requirement.h"
#include "xpm_common.h"

static XStatus XPm_GetRegProbeCounterLpd(u32 CounterIdx, u32 ReqType, u32 *Reg,
					 u32 *ReqTypeOffset)
{
	XStatus Status = XST_INVALID_PARAM;

	if (PROBE_COUNTER_CPU_R5_MAX_IDX < CounterIdx) {
		goto done;
	}

	if (PROBE_COUNTER_LPD_MAX_REQ_TYPE < ReqType) {
		goto done;
	} else if ((PROBE_COUNTER_CPU_R5_MAX_REQ_TYPE < ReqType) &&
			(PROBE_COUNTER_LPD_MAX_IDX < CounterIdx)) {
		goto done;
	} else {
		/* Required due to MISRA */
		PmDbg("[%d] Unknown else case\r\n", __LINE__);
	}

	*Reg = CORESIGHT_LPD_ATM_BASE;
	*ReqTypeOffset = (ReqType * PROBE_COUNTER_LPD_REQ_TYPE_OFFSET);
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPm_GetRegProbeCounterFpd(u32 CounterIdx, u32 ReqType, u32 *Reg,
					 u32 *ReqTypeOffset)
{
	XStatus Status = XST_INVALID_PARAM;
	const u32 FpdReqTypeOffset[] = {
		PROBE_COUNTER_FPD_RD_REQ_OFFSET,
		PROBE_COUNTER_FPD_RD_RES_OFFSET,
		PROBE_COUNTER_FPD_WR_REQ_OFFSET,
		PROBE_COUNTER_FPD_WR_RES_OFFSET,
	};

	if (PROBE_COUNTER_FPD_MAX_IDX < CounterIdx) {
		goto done;
	}

	if (PROBE_COUNTER_FPD_MAX_REQ_TYPE < ReqType) {
		goto done;
	}

	*Reg = CORESIGHT_FPD_ATM_BASE;
	*ReqTypeOffset = FpdReqTypeOffset[ReqType];
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPm_GetRegByProbeCounterType(u32 CounterIdx, u32 Arg1, u32 *Reg,
					    u32 ReqTypeOffset, u8 Write)
{
	XStatus Status = XST_FAILURE;
	u32 ProbeCounterType = (Arg1 >> PROBE_COUNTER_TYPE_SHIFT) & PROBE_COUNTER_TYPE_MASK;

	switch (ProbeCounterType) {
	case XPM_PROBE_COUNTER_TYPE_LAR_LSR:
		if (1U == Write) {
			*Reg += PROBE_COUNTER_LAR_OFFSET;
		} else {
			*Reg += PROBE_COUNTER_LSR_OFFSET;
		}
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_MAIN_CTL:
		*Reg += ReqTypeOffset + PROBE_COUNTER_MAIN_CTL_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_CFG_CTL:
		*Reg += ReqTypeOffset + PROBE_COUNTER_CFG_CTL_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_STATE_PERIOD:
		*Reg += ReqTypeOffset + PROBE_COUNTER_STATE_PERIOD_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_PORT_SEL:
		*Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_PORT_SEL_OFFSET);
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_SRC:
		*Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_SRC_OFFSET);
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_VAL:
		if (1U == Write) {
			/* This type doesn't support write operation */
			goto done;
		}
		*Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_VAL_OFFSET);
		Status = XST_SUCCESS;
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
XStatus XPm_ProbeCounterAccess(u32 DeviceId, u32 Arg1, u32 Value,
				  u32 *const Response, u8 Write)
{
	XStatus Status = XST_INVALID_PARAM;
	const XPm_Power *Power;
	u32 Reg;
	u32 CounterIdx;
	u32 ReqType;
	u32 ReqTypeOffset;

	CounterIdx = Arg1 & PROBE_COUNTER_IDX_MASK;
	ReqType = ((Arg1 >> PROBE_COUNTER_REQ_TYPE_SHIFT) &
		   PROBE_COUNTER_REQ_TYPE_MASK);

	switch (NODEINDEX(DeviceId)) {
	case (u32)XPM_NODEIDX_POWER_LPD:
		Status = XPm_GetRegProbeCounterLpd(CounterIdx, ReqType, &Reg, &ReqTypeOffset);
		break;
	case (u32)XPM_NODEIDX_POWER_FPD:
		Status = XPm_GetRegProbeCounterFpd(CounterIdx, ReqType, &Reg, &ReqTypeOffset);
		break;
	default:
		Status = XPM_PM_INVALID_NODE;
		break;
	}

	if (XST_SUCCESS != Status) {
		goto done;
	}

	Power = XPmPower_GetById(DeviceId);
	if ((NULL == Power) || ((u8)XPM_POWER_STATE_ON != Power->Node.State)) {
		goto done;
	}

	Status = XPm_GetRegByProbeCounterType(CounterIdx, Arg1, &Reg,
					      ReqTypeOffset, Write);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (0U == Write) {
		if (NULL == Response) {
			Status = XST_FAILURE;
			goto done;
		}
		PmIn32(Reg, *Response);
	} else {
		PmOut32(Reg, Value);
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}

XStatus XPm_AieISRClear(u32 SubsystemId, u32 AieDeviceId, u32 Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Aie = NULL;
	u32 IntrClear = 0x0U;
	u32 IdCode = XPm_GetIdCode();
	u32 PlatformVersion = XPm_GetPlatformVersion();

	if (PM_DEV_AIE != AieDeviceId) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	Aie = XPmDevice_GetById(AieDeviceId);
	if (NULL == Aie) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Only needed for XCVC1902 ES1 devices */
	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		/* Check whether given subsystem has access to the device */
		Status = XPm_IsAccessAllowed(SubsystemId, AieDeviceId);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
		/* Unlock the AIE PCSR register to allow register writes */
		XPm_UnlockPcsr(Aie->Node.BaseAddress);

		/* Clear ISR */
		IntrClear = Value & ME_NPI_ME_ISR_MASK;
		XPm_Out32(Aie->Node.BaseAddress + ME_NPI_ME_ISR_OFFSET, IntrClear);

		/* Re-lock the AIE PCSR register for protection */
		XPm_LockPcsr(Aie->Node.BaseAddress);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_AieOperation(u32 SubsystemId, u32 Id, pm_ioctl_id IoctlId, u32 Part, u32 Ops)
{
	XStatus Status = XST_FAILURE;
	(void)Id;
	(void)SubsystemId;

	u32 NumArgs = 4U;
	u32 ArgBuf[4U];
	ArgBuf[0] = Id;
	ArgBuf[1] = (u32)IoctlId;
	ArgBuf[2] = Part;
	ArgBuf[3] = Ops;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs,
				    (u32)NO_HEADER_CMDTYPE, NULL);
	if (XST_DEVICE_NOT_FOUND != Status){
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	/* To-Do: Add Permission Check */

	Status = Aie_Operations(Part, Ops);

done:
	return Status;
}

XStatus XPm_GetQos(const u32 DeviceId, pm_ioctl_id IoctlId, u32 *Response)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;

	u32 NumArgs = 2U;
	u32 ArgBuf[2U];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = (u32)IoctlId;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs,
				    (u32)NO_HEADER_CMDTYPE, Response);
	if (XST_DEVICE_NOT_FOUND != Status){
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if (IS_DEV_AIE(DeviceId)) {
		XPmAieDevice_QueryDivider(Device, Response);
		Status = XST_SUCCESS;
	} else {
		/* Device not supported */
		Status = XST_INVALID_PARAM;
	}

done:
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
			ReadPerm =  &PggsReadPermissions[RegNum];
			WritePerm = &PggsWritePermissions[RegNum];
			break;
		case (u32)XPM_NODETYPE_DEV_GGS:
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

XStatus XPmIoctl_IsOperationAllowed(u32 RegNum, u32 SubsystemId,
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
