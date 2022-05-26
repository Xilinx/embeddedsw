/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xil_io.h"
#include "xpm_rpucore.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_psm.h"

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
	PmRmw32(RpuCore->ResumeCfg, XPM_RPU_CPUHALT_MASK,
		XPM_RPU_CPUHALT_MASK);

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
	PmRmw32(RpuCore->ResumeCfg, XPM_RPU_CPUHALT_MASK,
		~XPM_RPU_CPUHALT_MASK);

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

	RpuCore->RpuBaseAddr = BaseAddress[0];
	RpuCore->ClusterBaseAddr = BaseAddress[1];

	if (PM_DEV_RPU_A_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_A_0_PWR_CTRL_MASK;
		RpuCore->Core.WakeUpMask = XPM_RPU_A_0_WAKEUP_MASK;
	} else if (PM_DEV_RPU_A_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_A_1_PWR_CTRL_MASK;
		RpuCore->Core.WakeUpMask = XPM_RPU_A_1_WAKEUP_MASK;
	} else if (PM_DEV_RPU_B_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_B_0_PWR_CTRL_MASK;
		RpuCore->Core.WakeUpMask = XPM_RPU_B_0_WAKEUP_MASK;
	} else if (PM_DEV_RPU_B_1 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU_B_1_PWR_CTRL_MASK;
		RpuCore->Core.WakeUpMask = XPM_RPU_B_1_WAKEUP_MASK;
	}

done:
	return Status;
}

XStatus XPm_RpuGetOperMode(const u32 DeviceId, u32 *Mode)
{
	XStatus Status = XST_FAILURE;
	u32 Val;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	if (NULL == RpuCore) {
		*Mode = XPM_INVAL_OPER_MODE;
		goto done;
	}

	PmIn32(RpuCore->ClusterBaseAddr + XPM_CLUSTER_CFG_OFFSET, Val);
	Val &= XPM_RPU_SLSPLIT_MASK;
	if (0U == Val) {
		*Mode = XPM_RPU_MODE_LOCKSTEP;
	} else {
		*Mode = XPM_RPU_MODE_SPLIT;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode)
{
	u32 Val;
	XStatus Status = XST_FAILURE;
	u32 Rpu0, Rpu1;
	const XPm_Subsystem *DefSubsystem = XPmSubsystem_GetById(PM_SUBSYS_DEFAULT);
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	if (NULL == RpuCore)  {
		PmErr("Unable to get RPU Core for Id: 0x%x\n\r", DeviceId);
		goto done;
	}
	PmIn32(RpuCore->ClusterBaseAddr + XPM_CLUSTER_CFG_OFFSET, Val);
	if (Mode == XPM_RPU_MODE_SPLIT) {
		Val |= XPM_RPU_SLSPLIT_MASK;
	} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
		Val &= ~XPM_RPU_SLSPLIT_MASK;
	} else {
		/* Required by MISRA */
	}

	PmOut32(RpuCore->ClusterBaseAddr + XPM_CLUSTER_CFG_OFFSET, Val);

	if (PM_DEV_RPU_A_0 == DeviceId || PM_DEV_RPU_A_1 == DeviceId) {
		Rpu0 = PM_DEV_RPU_A_0;
		Rpu1 = PM_DEV_RPU_A_1;
	} else {
		Rpu0 = PM_DEV_RPU_B_0;
		Rpu1 = PM_DEV_RPU_B_1;
	}

	/* Add or remove R50_1 core in default subsystem according to its mode */
	if (NULL != DefSubsystem) {
		Status = XPmDevice_IsRequested(Rpu0, PM_SUBSYS_DEFAULT);
		if ((XST_SUCCESS == Status) &&
		    ((u8)ONLINE == DefSubsystem->State)) {
			if (Mode == XPM_RPU_MODE_SPLIT) {
				Status = XPmDevice_Request(PM_SUBSYS_DEFAULT, Rpu1,
							   (u32)PM_CAP_ACCESS,
							   XPM_MAX_QOS,
							   XPLMI_CMD_SECURE);
				if (XST_SUCCESS != Status) {
					PmErr("Unable to request RPU 1 Core\n\r");
				}
			} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
				Status = XPmDevice_IsRequested(Rpu1, PM_SUBSYS_DEFAULT);
				if (XST_SUCCESS == Status) {
					Status = XPmDevice_Release(PM_SUBSYS_DEFAULT, Rpu1,
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
	return Status;
}

XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	if (NULL == RpuCore) {
		PmErr("Unable to get RPU Core for Id: 0x%x\n\r", DeviceId);
		goto done;
	}

	if (0U == BootAddr) {
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK,
			XPM_RPU_TCMBOOT_MASK);
	} else {
		PmOut32(RpuCore->RpuBaseAddr + XPM_CORE_VECTABLE_OFFSET, BootAddr);
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK, 0U);
	}

done:
	return Status;
}
