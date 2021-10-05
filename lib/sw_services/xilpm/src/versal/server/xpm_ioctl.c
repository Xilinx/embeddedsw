/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_ioctl.h"
#include "xpm_rpucore.h"
#include "xpm_pmc.h"
#include "xpm_power.h"
#include "xpm_psm.h"
#include "xpm_regs.h"
#include "sleep.h"
#include "xpm_aie.h"
#include "xpm_api.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_periph.h"
#include "xpm_requirement.h"
#include "xplmi.h"

static u32 PsmGgsValues[GGS_REGS] = {0U};

static u32 PggsReadPermissions[PSM_PGGS_REGS + PMC_PGGS_REGS];
static u32 PggsWritePermissions[PSM_PGGS_REGS + PMC_PGGS_REGS];
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
static XStatus XPm_ProbeCounterAccess(u32 DeviceId, u32 Arg1, u32 Value,
				  u32 *const Response, u8 Write)
{
	XStatus Status = XST_INVALID_PARAM;
	const XPm_Power *Power;
	u32 Reg;
	u32 CounterIdx;
	u32 ReqType;
	u32 ReqTypeOffset;
	const u32 FpdReqTypeOffset[] = {
		PROBE_COUNTER_FPD_RD_REQ_OFFSET,
		PROBE_COUNTER_FPD_RD_RES_OFFSET,
		PROBE_COUNTER_FPD_WR_REQ_OFFSET,
		PROBE_COUNTER_FPD_WR_RES_OFFSET,
	};

	CounterIdx = Arg1 & PROBE_COUNTER_IDX_MASK;
	ReqType = ((Arg1 >> PROBE_COUNTER_REQ_TYPE_SHIFT) &
		   PROBE_COUNTER_REQ_TYPE_MASK);

	switch (NODEINDEX(DeviceId)) {
	case (u32)XPM_NODEIDX_POWER_LPD:
		if (PROBE_COUNTER_CPU_R5_MAX_IDX < CounterIdx) {
			goto done;
		}

		if (PROBE_COUNTER_LPD_MAX_REQ_TYPE < ReqType) {
			goto done;
		} else if ((ReqType > PROBE_COUNTER_CPU_R5_MAX_REQ_TYPE) &&
			   (CounterIdx > PROBE_COUNTER_LPD_MAX_IDX)) {
			goto done;
		} else {
			/* Required due to MISRA */
			PmDbg("[%d] Unknown else case\r\n", __LINE__);
		}

		Reg = CORESIGHT_LPD_ATM_BASE;
		ReqTypeOffset = (ReqType * PROBE_COUNTER_LPD_REQ_TYPE_OFFSET);
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_FPD:
		if (PROBE_COUNTER_FPD_MAX_IDX < CounterIdx) {
			goto done;
		}

		if (PROBE_COUNTER_FPD_MAX_REQ_TYPE < ReqType) {
			goto done;
		}

		Reg = CORESIGHT_FPD_ATM_BASE;
		ReqTypeOffset = FpdReqTypeOffset[ReqType];
		Status = XST_SUCCESS;
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

	switch ((Arg1 >> PROBE_COUNTER_TYPE_SHIFT) & PROBE_COUNTER_TYPE_MASK) {
	case XPM_PROBE_COUNTER_TYPE_LAR_LSR:
		if (1U == Write) {
			Reg += PROBE_COUNTER_LAR_OFFSET;
		} else {
			Reg += PROBE_COUNTER_LSR_OFFSET;
		}
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_MAIN_CTL:
		Reg += ReqTypeOffset + PROBE_COUNTER_MAIN_CTL_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_CFG_CTL:
		Reg += ReqTypeOffset + PROBE_COUNTER_CFG_CTL_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_STATE_PERIOD:
		Reg += ReqTypeOffset + PROBE_COUNTER_STATE_PERIOD_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_PORT_SEL:
		Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_PORT_SEL_OFFSET);
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_SRC:
		Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_SRC_OFFSET);
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_VAL:
		if (1U == Write) {
			/* This type doesn't support write operation */
			goto done;
		}
		Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_VAL_OFFSET);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

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

/****************************************************************************/
/**
 * @brief  This function sets the controller into either D3 or D0 state
 *
 * @param  DeviceId	DeviceId of the device
 * @param  ReqState	Requested State (0 - D0, 3 - D3)
 * @param  TimeOut	TimeOut value in micro secs to wait for D3/D0 entry
 *
 * @return XST_SUCCESS if successful else error code
 *
 ****************************************************************************/
static XStatus XPm_USBDxState(const u32 DeviceId, const u32 ReqState,
			  const u32 TimeOut)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc;
	u32 BaseAddress;
	u32 Offset;
	u32 CurState;
	u32 LocalTimeOut;

	(void)DeviceId;
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
	Offset = XPM_USB_PWR_REQ_OFFSET;
	PmOut32(BaseAddress + Offset, ReqState);

	/* current power state */
	Offset = XPM_USB_CUR_PWR_OFFSET;
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
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Reset type
 * @param  Response	Output Response (0 - DMA, 1 - LINEAR)
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static XStatus XPm_OspiMuxSelect(const u32 DeviceId, const u32 Type, u32 *Response)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc;
	u32 BaseAddress;
	u32 Offset;

	(void)DeviceId;

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


static XStatus XPm_ReadPggs(u32 PggsNum, u32 *Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	const XPm_Psm *Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);

	if ((NULL == Pmc) || (NULL == Psm)) {
		goto done;
	}

	if ((PSM_PGGS_REGS + PMC_PGGS_REGS) <= PggsNum) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Map PGGS0-1 to PMC_GLOBAL_PGGS3-4 and PGGS2-3 to PSM_GLOBAL_PGGS0-1 */
	if (2U > PggsNum) {
		PmIn32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PGGS3_OFFSET +
		       (PggsNum << 2U)), *Value);
	} else {
		const XPm_Power *Lpd = XPmPower_GetById(PM_POWER_LPD);
		if ((u8)XPM_POWER_STATE_ON != Lpd->Node.State) {
			goto done;
		}
		PmIn32((Psm->PsmGlobalBaseAddr + PSM_GLOBAL_PGGS0_OFFSET) +
		       ((PggsNum - 2U) << 2U), *Value);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPm_WritePggs(u32 PggsNum, u32 Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	const XPm_Psm *Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);

	if ((NULL == Pmc) || (NULL == Psm)) {
		goto done;
	}

	if ((PSM_PGGS_REGS + PMC_PGGS_REGS) <= PggsNum) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Map PGGS0-1 to PMC_GLOBAL_PGGS3-4 and PGGS2-3 to PSM_GLOBAL_PGGS0-1 */
	if (2U > PggsNum) {
		PmOut32((Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_PGGS3_OFFSET +
			(PggsNum << 2U)), Value);
	} else {
		const XPm_Power *Lpd = XPmPower_GetById(PM_POWER_LPD);
		if ((u8)XPM_POWER_STATE_ON != Lpd->Node.State) {
			goto done;
		}
		PmOut32((Psm->PsmGlobalBaseAddr + PSM_GLOBAL_PGGS0_OFFSET) +
			((PggsNum - 2U) << 2U), Value);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPm_ReadGgs(u32 GgsNum, u32 *Value)
{
	XStatus Status = XST_FAILURE;

	if (GGS_REGS <= GgsNum) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	*Value = PsmGgsValues[GgsNum];

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPm_WriteGgs(u32 GgsNum, u32 Value)
{
	XStatus Status = XST_FAILURE;

	if (GGS_REGS <= GgsNum) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PsmGgsValues[GgsNum] = Value;

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPm_AieISRClear(u32 SubsystemId, u32 AieDeviceId, u32 Value)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Aie = NULL;
	u32 IntrClear = 0x0U;
	u32 IdCode = XPm_GetIdCode();
	u32 PlatformVersion = XPm_GetPlatformVersion();

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
		XPmAieDomain_UnlockPcsr(Aie->Node.BaseAddress);

		/* Clear ISR */
		IntrClear = Value & ME_NPI_ME_ISR_MASK;
		XPm_Out32(Aie->Node.BaseAddress + ME_NPI_ME_ISR_OFFSET, IntrClear);

		/* Re-lock the AIE PCSR register for protection */
		XPmAieDomain_LockPcsr(Aie->Node.BaseAddress);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_Ioctl(const u32 SubsystemId, const u32 DeviceId, const pm_ioctl_id IoctlId,
	      const u32 Arg1, const u32 Arg2, u32 *const Response, const u32 CmdType)
{
	XStatus Status = XPM_ERR_IOCTL;
	const XPm_Pmc *Pmc;

	switch (IoctlId) {
	case IOCTL_GET_RPU_OPER_MODE:
		if ((PM_DEV_RPU0_0 != DeviceId) &&
		    (PM_DEV_RPU0_1 != DeviceId)) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
		XPm_RpuGetOperMode(DeviceId, Response);
		Status = XST_SUCCESS;
		break;
	case IOCTL_SET_RPU_OPER_MODE:
		if ((PM_DEV_RPU0_0 != DeviceId) &&
		    (PM_DEV_RPU0_1 != DeviceId)) {
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
		Status = XPmIoctl_IsOperationAllowed(Arg1, SubsystemId,
						     GgsWritePermissions,
						     (u32)XPM_NODETYPE_DEV_GGS,
						     CmdType);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_WriteGgs(Arg1, Arg2);
		break;
	case IOCTL_READ_GGS:
		Status = XPmIoctl_IsOperationAllowed(Arg1, SubsystemId,
						     GgsReadPermissions,
						     (u32)XPM_NODETYPE_DEV_GGS,
						     CmdType);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_ReadGgs(Arg1, Response);
		break;
	case IOCTL_WRITE_PGGS:
		Status = XPmIoctl_IsOperationAllowed(Arg1, SubsystemId,
						     PggsWritePermissions,
						     (u32)XPM_NODETYPE_DEV_PGGS,
						     CmdType);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_WritePggs(Arg1, Arg2);
		break;
	case IOCTL_READ_PGGS:
		Status = XPmIoctl_IsOperationAllowed(Arg1, SubsystemId,
						     PggsReadPermissions,
						     (u32)XPM_NODETYPE_DEV_PGGS,
						     CmdType);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_ReadPggs(Arg1, Response);
		break;
	case IOCTL_SET_BOOT_HEALTH_STATUS:
		Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
		if (NULL == Pmc) {
			Status = XST_FAILURE;
			goto done;
		}
		PmRmw32(Pmc->PmcGlobalBaseAddr + PMC_GLOBAL_GGS4_OFFSET,
			XPM_BOOT_HEALTH_STATUS_MASK, Arg1);
		Status = XST_SUCCESS;
		break;
	case IOCTL_PROBE_COUNTER_READ:
		Status = XPm_ProbeCounterAccess(DeviceId, Arg1, Arg2,
						Response, 0U);
		break;
	case IOCTL_PROBE_COUNTER_WRITE:
		Status = XPm_ProbeCounterAccess(DeviceId, Arg1, Arg2,
						Response, 1U);
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
	case IOCTL_USB_SET_STATE:
		if (PM_DEV_USB_0 != DeviceId) {
			goto done;
		}

		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_USBDxState(DeviceId, Arg1, Arg2);
		break;
	case IOCTL_GET_LAST_RESET_REASON:
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
		Status = XST_SUCCESS;
		break;
	case IOCTL_AIE_ISR_CLEAR:
		if (PM_DEV_AIE != DeviceId) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
		Status = XPm_AieISRClear(SubsystemId, DeviceId, Arg1);
		break;
	case IOCTL_SET_PLL_FRAC_MODE:
	case IOCTL_GET_PLL_FRAC_MODE:
	case IOCTL_SET_PLL_FRAC_DATA:
	case IOCTL_GET_PLL_FRAC_DATA:
	case IOCTL_ULPI_RESET:
	case IOCTL_AFI:
	case IOCTL_REGISTER_SGI:
	case IOCTL_SET_FEATURE_CONFIG:
	case IOCTL_GET_FEATURE_CONFIG:
	case IOCTL_SET_SGMII_MODE:
	default:
		/* Not supported yet */
		Status = XPM_ERR_IOCTL;
		break;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}

	return Status;
}
