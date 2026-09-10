/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_runtime_periph.h"
#include "xpm_update.h"
static XPmRuntime_PeriphList *PeriphList XPM_INIT_DATA(PeriphList) = NULL;

/****************************************************************************/
/**
 * @brief Find a peripheral association by device pointer.
 *
 * @param Device Peripheral device to find.
 *
 * @return Pointer to the first matching association, or NULL when absent.
 *
 ****************************************************************************/
static XPmRuntime_Periph *XPmRuntime_Periph_Find(const XPm_Periph *Device)
{
	XPmRuntime_Periph *Periph = NULL;

	/* Match associations by device pointer identity. */
	LIST_FOREACH(PeriphList, PeriphNode) {
		if ((NULL != PeriphNode->Data) &&
		    (PeriphNode->Data->Device == Device)) {
			/* Retain the first association in list order. */
			Periph = PeriphNode->Data;
			break;
		}
	}

	return Periph;
}

/****************************************************************************/
/**
 * @brief  Get the wake processor ID associated with a peripheral device
 *
 * @param  Device: Peripheral device to look up in the peripheral list
 * @param  WakeProcId: Pointer that receives the wake processor ID on success;
 *                     unchanged when no association is found
 *
 * @return XST_SUCCESS if found; XST_INVALID_PARAM for NULL arguments;
 *         XST_DEVICE_NOT_FOUND if the peripheral list is empty; XST_FAILURE
 *         if the list contains no association for @p Device.
 *
 ****************************************************************************/
XStatus XPmRuntime_Periph_GetWakeProcId(const XPm_Periph * Device, u32 *WakeProcId)
{
	XStatus Status = XST_FAILURE;
	if ((NULL == Device) || (NULL == WakeProcId)) {
		Status =  XST_INVALID_PARAM;
		goto done;
	}
	/* The setter creates the association list on first use */
	if (NULL == PeriphList) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	/* Match associations by device pointer identity */
	LIST_FOREACH(PeriphList, PeriphNode) {
		if ((NULL != PeriphNode->Data) &&
		    (PeriphNode->Data->Device == Device)) {
			*WakeProcId = PeriphNode->Data->WakeProcId;
			Status = XST_SUCCESS;
			goto done;
		}
	}

	/* Leave the output untouched when no association matches */
done:
	return Status;
}
/****************************************************************************/
/**
 * @brief Associate a wake processor ID with a peripheral device.
 *
 * Updates an existing association when one is present. Otherwise, lazily
 * creates the peripheral list, allocates a new association, and prepends it to
 * the list. Allocated list storage persists for the firmware lifetime.
 *
 * @param Device Peripheral device to associate with @p WakeProcId.
 * @param WakeProcId Wake processor node ID to store.
 *
 * @return XST_SUCCESS when the association is stored; XST_INVALID_PARAM when
 *         @p Device is NULL; XST_BUFFER_TOO_SMALL when list or entry allocation
 *         fails.
 *
 ****************************************************************************/
XStatus XPmRuntime_Periph_SetWakeProcId(XPm_Periph * Device, u32 WakeProcId)
{
	XStatus Status = XST_FAILURE;
	XPmRuntime_Periph *Periph = NULL;
	/* Reject a NULL association key before touching the list */
	if (NULL == Device) {
		Status =  XST_INVALID_PARAM;
		goto done;
	}
	/* Lazily create storage for the first association */
	if (NULL == PeriphList) {
		PeriphList = Make_XPmRuntime_PeriphList();
		if (NULL == PeriphList) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}
	Periph = XPmRuntime_Periph_Find(Device);
	/* Allocate a persistent entry when the device is not registered */
	if (NULL == Periph) {
		Periph = (XPmRuntime_Periph*)XPm_AllocBytesDevOps(sizeof(XPmRuntime_Periph));
		if (NULL == Periph) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Periph->Device = Device;
		Periph->WakeProcId = WakeProcId;
		/* Prepend may also fail while allocating its list node */
		LIST_PREPEND(PeriphList, Periph);
	} else {
		/* Replace the wake processor for an existing association */
		Periph->WakeProcId = WakeProcId;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}
