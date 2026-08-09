/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_runtime_periph.h"
#include "xpm_update.h"
static XPmRuntime_PeriphList *PeriphList XPM_INIT_DATA(PeriphList) = NULL;
/****************************************************************************/
/**
 * @brief  Get the wake processor ID associated with a peripheral device
 *
 * @param  Device: Peripheral device to look up in the peripheral list
 * @param  WakeProcId: Pointer that receives the wake processor ID
 *
 * @return XST_SUCCESS if found, XST_INVALID_PARAM for NULL arguments,
 *         XST_DEVICE_NOT_FOUND if the peripheral list is empty
 *
 ****************************************************************************/
maybe_unused static XStatus XPmRuntime_Periph_GetWakeProcId(const XPm_Periph * Device, u32 *WakeProcId)
{
	XStatus Status = XST_FAILURE;
	if ((NULL == Device) || (NULL == WakeProcId)) {
		Status =  XST_INVALID_PARAM;
		goto done;
	}
	if (NULL == PeriphList) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	LIST_FOREACH(PeriphList, PeriphNode) {
		if (PeriphNode->Data->Device == Device) {
			*WakeProcId = PeriphNode->Data->WakeProcId;
			Status = XST_SUCCESS;
			goto done;
		}
	}

done:
	return Status;
}
XStatus XPmRuntime_Periph_SetWakeProcId(XPm_Periph * Device, u32 WakeProcId)
{
	XStatus Status = XST_FAILURE;
	XPmRuntime_Periph *Periph = NULL;
	if (NULL == Device) {
		Status =  XST_INVALID_PARAM;
		goto done;
	}
	if (NULL == PeriphList) {
		PeriphList = Make_XPmRuntime_PeriphList();
		if (NULL == PeriphList) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}
	LIST_FOREACH(PeriphList, PeriphNode) {
		if (PeriphNode->Data->Device == Device) {
			Periph = PeriphNode->Data;
			break;
		}
	}
	if (NULL == Periph) {
		Periph = (XPmRuntime_Periph*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_Periph));
		if (NULL == Periph) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Periph->Device = Device;
		Periph->WakeProcId = WakeProcId;
		LIST_PREPEND(PeriphList, Periph);
	} else {
		Periph->WakeProcId = WakeProcId;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}
