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
	XPM_RPU_CORE_HALT(RpuCore->ResumeCfg);

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
	XPM_RPU_CORE_RUN(RpuCore->ResumeCfg);

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

	XPmRpuCore_AssignRegAddr(RpuCore, Id, BaseAddress);

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

	Val = XPm_PlatRpuGetOperMode(RpuCore);
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

	Val = XPm_PlatRpuGetOperMode(RpuCore);

	XPm_PlatRpuSetOperMode(RpuCore, Mode, &Val);

	XPm_GetCoreId(&Rpu0, &Rpu1, DeviceId);

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
					goto done;
				}
			} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
				Status = XPmDevice_IsRequested(Rpu1, PM_SUBSYS_DEFAULT);
				if (XST_SUCCESS == Status) {
					Status = XPmDevice_Release(PM_SUBSYS_DEFAULT, Rpu1,
								   XPLMI_CMD_SECURE);
					if (XST_SUCCESS != Status) {
						PmErr("Unable to release RPU 1 Core\n\r");
						goto done;
					}
				}
			} else {
				/* Required due to MISRA */
				PmDbg("Invalid RPU mode %d\r\n", Mode);
				Status = XST_INVALID_PARAM;
				goto done;
			}
		}
	}

	Status = XST_SUCCESS;

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

	Status = XPm_PlatRpuBootAddrConfig(RpuCore, BootAddr);

done:
	return Status;
}
