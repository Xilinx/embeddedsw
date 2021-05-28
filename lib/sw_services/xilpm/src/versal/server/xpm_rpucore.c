/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xil_io.h"
#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_psm.h"
#include "xpm_prot.h"

static XStatus XPmRpuCore_ProtControl(const XPm_Requirement *Reqm,
				      u32 Enable)
{
	const XPm_RpuCore *Rpu = (XPm_RpuCore *)Reqm->Device;

	return XPmProt_PpuControl(Reqm, Rpu->RpuBaseAddr, Enable);
}

XStatus XPmRpuCore_Halt(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)Device;

	/* RPU should be in reset state before putting it into halt state */
	Status = XPmDevice_Reset(&RpuCore->Core.Device, PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Put RPU in  halt state */
	PmRmw32(RpuCore->ResumeCfg, XPM_RPU_NCPUHALT_MASK,
		~XPM_RPU_NCPUHALT_MASK);

	/* Release reset for all resets attached to this core */
	Status = XPmDevice_Reset(&RpuCore->Core.Device, PM_RESET_ACTION_RELEASE);

done:
	return Status;
}

static XStatus XPmRpuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)Core;

	Status = XPmCore_WakeUp(Core, SetAddress, Address);
	if (XST_SUCCESS != Status) {
		PmErr("Status = %x\r\n", Status);
		goto done;
	}

	/* Put RPU in running state from halt state */
	PmRmw32(RpuCore->ResumeCfg, XPM_RPU_NCPUHALT_MASK,
		XPM_RPU_NCPUHALT_MASK);

done:
	return Status;
}

static XStatus XPmRpuCore_PwrDwn(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;

	Status = XPmRpuCore_Halt((XPm_Device *)Core);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmCore_PwrDwn(Core);

done:
	return Status;
}

static struct XPm_CoreOps RpuOps = {
	.RequestWakeup = XPmRpuCore_WakeUp,
	.PowerDown = XPmRpuCore_PwrDwn,
};


XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&RpuCore->Core, Id, Power, Clock, Reset, (u8)Ipi,
			      &RpuOps);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	/* Protection handler for RPU cores */
	RpuCore->Core.Device.HandleProtection = &XPmRpuCore_ProtControl;

	RpuCore->RpuBaseAddr = BaseAddress[0];

	if (PM_DEV_RPU0_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU0_0_PWR_CTRL_MASK;
		RpuCore->Core.PwrDwnMask = XPM_RPU_0_CPUPWRDWNREQ_MASK;
	} else {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU0_1_PWR_CTRL_MASK;
		RpuCore->Core.PwrDwnMask = XPM_RPU_1_CPUPWRDWNREQ_MASK;
	}

done:
	return Status;
}

void XPm_RpuGetOperMode(const u32 DeviceId, u32 *Mode)
{
	u32 Val;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	PmIn32(RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET, Val);
	Val &= XPM_RPU_SLSPLIT_MASK;
	if (0U == Val) {
		*Mode = XPM_RPU_MODE_LOCKSTEP;
	} else {
		*Mode = XPM_RPU_MODE_SPLIT;
	}
}

void XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode)
{
	u32 Val;
	XStatus Status;
	const XPm_Subsystem *DefSubsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);

	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	if (NULL == RpuCore)  {
		PmErr("Unable to get RPU Core for Id: 0x%x\n\r", DeviceId);
		goto done;
	}
	PmIn32(RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET, Val);
	if (Mode == XPM_RPU_MODE_SPLIT) {
		Val |= XPM_RPU_SLSPLIT_MASK;
		Val &= ~XPM_RPU_TCM_COMB_MASK;
		Val &= ~XPM_RPU_SLCLAMP_MASK;
	} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
		Val &= ~XPM_RPU_SLSPLIT_MASK;
		Val |= XPM_RPU_TCM_COMB_MASK;
		Val |= XPM_RPU_SLCLAMP_MASK;
	} else {
		/* Required by MISRA */
	}

	PmOut32(RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET, Val);

	/* Add or remove R50_1 core in default subsystem according to its mode */
	if (NULL != DefSubsystem) {
		Status = XPmDevice_IsRequested(PM_DEV_RPU0_0,
					       PM_SUBSYS_DEFAULT);
		if ((XST_SUCCESS == Status) &&
		    ((u8)ONLINE == DefSubsystem->State)) {
			if (Mode == XPM_RPU_MODE_SPLIT) {
				Status = XPmDevice_Request(PM_SUBSYS_DEFAULT,
							   PM_DEV_RPU0_1,
							   (u32)PM_CAP_ACCESS,
							   XPM_MAX_QOS,
							   XPLMI_CMD_SECURE);
				if (XST_SUCCESS != Status) {
					PmErr("Unable to request RPU 1 Core\n\r");
				}
			} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
				Status = XPmDevice_IsRequested(PM_DEV_RPU0_1,
						PM_SUBSYS_DEFAULT);
				if (XST_SUCCESS == Status) {
					Status = XPmDevice_Release(PM_SUBSYS_DEFAULT,
								   PM_DEV_RPU0_1,
								   XPLMI_CMD_SECURE);
					if (XST_SUCCESS != Status) {
						PmErr("Unable to release RPU 1 Core\n\r");
					}
				}
			} else {
				/* Required due to MISRA */
				PmDbg("Invalid RPU mode %d\r\n", Mode);
			}
		}
	}

done:
	return;
}

XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	/* CFG_VINITHI_MASK mask is common for both processors */
	if (XPM_RPU_BOOTMEM_LOVEC == BootAddr) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			~XPM_RPU_VINITHI_MASK);
		Status = XST_SUCCESS;
	} else if (XPM_RPU_BOOTMEM_HIVEC == BootAddr) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
			XPM_RPU_VINITHI_MASK);
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

XStatus XPm_RpuTcmCombConfig(const u32 DeviceId, const u32 Config)
{
	XStatus Status = XST_FAILURE;
	u32 Address;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	Address = RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET;
	if (Config == XPM_RPU_TCM_SPLIT) {
		PmRmw32(Address, XPM_RPU_TCM_COMB_MASK,
			~XPM_RPU_TCM_COMB_MASK);
		Status = XST_SUCCESS;
	} else if (Config == XPM_RPU_TCM_COMB) {
		PmRmw32(Address, XPM_RPU_TCM_COMB_MASK,
			XPM_RPU_TCM_COMB_MASK);
		Status = XST_SUCCESS;
	} else {
		Status = XST_INVALID_PARAM;
	}

	return Status;
}

XStatus XPm_RpuRstComparators(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = NULL;

	RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	if(RpuCore == NULL) {
		PmInfo("Invalid Device Id: 0x%x\n\r", DeviceId);
		goto done;
	}

	PmOut32(RpuCore->RpuBaseAddr + RPU_ERR_INJ_OFFSET, 0x0);
	Status = XST_SUCCESS;

done:
	return Status;
}
