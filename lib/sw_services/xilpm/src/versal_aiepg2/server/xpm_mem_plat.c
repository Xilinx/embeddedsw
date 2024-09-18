/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_debug.h"
#include "xpm_rpucore.h"

XStatus HaltRpuCore(const XPm_Device *Rpu0, const XPm_Device *Rpu1,
			   const u32 Id, u32 *RpuMode)
{
	XStatus Status = XST_FAILURE;
	u32 Mode;
	XPm_RpuGetOperMode(Rpu0->Node.Id, &Mode);
	if (XPM_RPU_MODE_SPLIT == Mode) {
		if ((((PM_DEV_TCM_A_0A <= Id) && (PM_DEV_TCM_A_0C >= Id)) ||
			((PM_DEV_TCM_B_0A <= Id) && (PM_DEV_TCM_B_0C >= Id)) ||
			((PM_DEV_TCM_C_0A <= Id) && (PM_DEV_TCM_C_0C >= Id)) ||
			((PM_DEV_TCM_D_0A <= Id) && (PM_DEV_TCM_D_0C >= Id)) ||
			((PM_DEV_TCM_E_0A <= Id) && (PM_DEV_TCM_E_0C >= Id))) &&
			((u8)XPM_DEVSTATE_RUNNING != Rpu0->Node.State)) {
			Status = XPmRpuCore_Halt(Rpu0);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		if ((((PM_DEV_TCM_A_1A <= Id) && (PM_DEV_TCM_A_1C >= Id)) ||
			((PM_DEV_TCM_B_1A <= Id) && (PM_DEV_TCM_B_1C >= Id)) ||
			((PM_DEV_TCM_C_1A <= Id) && (PM_DEV_TCM_C_1C >= Id)) ||
			((PM_DEV_TCM_D_1A <= Id) && (PM_DEV_TCM_D_1C >= Id)) ||
			((PM_DEV_TCM_E_1A <= Id) && (PM_DEV_TCM_E_1C >= Id))) &&
		    ((u8)XPM_DEVSTATE_RUNNING !=  Rpu1->Node.State)) {
			Status = XPmRpuCore_Halt(Rpu1);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	if (XPM_RPU_MODE_LOCKSTEP == Mode)
	{
		if (((PM_DEV_TCM_A_0A <= Id) && (PM_DEV_TCM_E_1C >= Id)) &&
		     ((u8)XPM_DEVSTATE_RUNNING != Rpu0->Node.State)) {
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

XStatus XPm_GetRpuDevice(const XPm_Device **Rpu0Device,const XPm_Device **Rpu1Device,
		const u32 Id){
	XStatus Status = XST_FAILURE;
	/*warning fix*/
	(void)Rpu0Device;
	(void)Rpu1Device;

	if ((PM_DEV_TCM_A_0A <= Id) && (PM_DEV_TCM_A_1C >= Id)) {
		*Rpu0Device = XPmDevice_GetById(PM_DEV_RPU_A_0);
		*Rpu1Device = XPmDevice_GetById(PM_DEV_RPU_A_1);
	} else if ((PM_DEV_TCM_B_0A <= Id) && (PM_DEV_TCM_B_1C >= Id)) {
		*Rpu0Device = XPmDevice_GetById(PM_DEV_RPU_B_0);
		*Rpu1Device = XPmDevice_GetById(PM_DEV_RPU_B_1);
	} else if ((PM_DEV_TCM_C_0A <= Id) && (PM_DEV_TCM_C_1C >= Id)) {
		*Rpu0Device = XPmDevice_GetById(PM_DEV_RPU_C_0);
		*Rpu1Device = XPmDevice_GetById(PM_DEV_RPU_C_1);
	} else if ((PM_DEV_TCM_D_0A <= Id) && (PM_DEV_TCM_D_1C >= Id)) {
		*Rpu0Device = XPmDevice_GetById(PM_DEV_RPU_D_0);
		*Rpu1Device = XPmDevice_GetById(PM_DEV_RPU_D_1);
	} else if ((PM_DEV_TCM_E_0A <= Id) && (PM_DEV_TCM_E_1C >= Id)) {
		*Rpu0Device = XPmDevice_GetById(PM_DEV_RPU_E_0);
		*Rpu1Device = XPmDevice_GetById(PM_DEV_RPU_E_1);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

u32 XPm_CombTcm(const u32 Id, const u32 Mode)
{
	/*warning fix*/
	(void)Id;
	(void)Mode;
	return 0;
}
