/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_debug.h"
#include "xpm_rpucore.h"

static void TcmEccInit(const XPm_MemDevice *Tcm)
{
	u32 Size = Tcm->EndAddress - Tcm->StartAddress;
	u32 Base = Tcm->StartAddress;

	if (0U != Size) {
		s32 Status = XPlmi_EccInit(Base, Size);
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in EccInit of 0x%x\r\n", Status, Tcm->Device.Node.Id);
		}
	}
	return;
}

static XStatus HaltRpuCore(const XPm_Device *Rpu0, const XPm_Device *Rpu1,
			   const u32 Id, u32 *RpuMode)
{
	XStatus Status = XST_FAILURE;
	u32 Mode;
	XPm_RpuGetOperMode(Rpu0->Node.Id, &Mode);
	if (XPM_RPU_MODE_SPLIT == Mode) {
		if ((((PM_DEV_TCM_A_0A <= Id) && (PM_DEV_TCM_A_0C >= Id)) ||
			((PM_DEV_TCM_B_0A <= Id) && (PM_DEV_TCM_B_0C >= Id))) &&
			((u8)XPM_DEVSTATE_RUNNING != Rpu0->Node.State)) {
			Status = XPmRpuCore_Halt(Rpu0);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		if ((((PM_DEV_TCM_A_1A <= Id) && (PM_DEV_TCM_A_1C >= Id)) ||
			((PM_DEV_TCM_B_1A <= Id) && (PM_DEV_TCM_B_1C >= Id)) ) &&
		    ((u8)XPM_DEVSTATE_RUNNING !=  Rpu1->Node.State)) {
			Status = XPmRpuCore_Halt(Rpu1);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	if (XPM_RPU_MODE_LOCKSTEP == Mode)
	{
		if (((PM_DEV_TCM_A_0A <= Id) && (PM_DEV_TCM_B_1C >= Id)) &&
		     ((u8)XPM_DEVSTATE_RUNNING != Rpu0->Node.State)) {
			Status = XPmRpuCore_Halt(Rpu0);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = XPmRpuCore_Halt(Rpu0);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	*RpuMode = Mode;
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus HandleTcmDeviceState(XPm_Device* Device, u32 NextState)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *RpuA0Device = XPmDevice_GetById(PM_DEV_RPU_A_0);
	const XPm_Device *RpuA1Device = XPmDevice_GetById(PM_DEV_RPU_A_1);
	const XPm_Device *RpuB0Device = XPmDevice_GetById(PM_DEV_RPU_B_0);
	const XPm_Device *RpuB1Device = XPmDevice_GetById(PM_DEV_RPU_B_1);
	u32 Id = Device->Node.Id;
	u32 Mode;

	const XPm_Device *TempDev0,*TempDev1;
	if((Id >= PM_DEV_TCM_A_0A) && (Id <= PM_DEV_TCM_A_1C)){
		TempDev0 = RpuA0Device;
		TempDev1 = RpuA1Device;
	}else{
		TempDev0 = RpuB0Device;
		TempDev1 = RpuB1Device;
	}

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(Device);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Request the RPU clocks. Here both core having same RPU clock */
			Status = XPmClock_Request(TempDev0->ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* TCM is only accessible when the RPU is powered on and out of reset and is in halted state
			 * so bring up RPU too when TCM is requested*/
			Status = HaltRpuCore(TempDev0, TempDev1, Id, &Mode);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Tcm should be ecc initialized */
			TcmEccInit((XPm_MemDevice *)Device);
		}
		Status = XST_SUCCESS;
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     XPM_DEVEVENT_SHUTDOWN);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Release the RPU clocks. Here both core having same RPU clock */
			Status = XPmClock_Release(TempDev0->ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

done:
	return Status;
}