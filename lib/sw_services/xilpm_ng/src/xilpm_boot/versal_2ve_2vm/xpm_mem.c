/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_alloc.h"
#include "xpm_debug.h"

XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(&MemDevice->Device, Id, BaseAddress, Power, Clock,
				Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MemDevice->StartAddress = MemStartAddress;
	MemDevice->EndAddress = MemEndAddress;


done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Add mem-range device to internal data-structure map
 *
 * @param  MemRegnDevice 	Pointer to store (struct XPm_MemRegnDevice)
 * @param  Id 				Device Id
 * @param  Address 			Address associated with device
 * @param  Size 			Length of the range
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
static XStatus XPmMemRegnDevice_Init(XPm_MemRegnDevice *MemRegnDevice, u32 Id,
			     u64 Address, u64 Size)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power;
	const u32 PowerId = PM_POWER_NOC;

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Status = XPmDevice_Init(&MemRegnDevice->Device, Id, 0U, Power,
				NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MemRegnDevice->AddrRegion.Address = Address;
	MemRegnDevice->AddrRegion.Size = Size;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Add mem-range device to internal data-structure map
 *
 * @param  DeviceId 	Variable stored device id extracted from cdo
 * @param  Address 		Address associated with device
 * @param  Size 		Length of the range
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
XStatus XPm_AddMemRegnDevice(u32 DeviceId, u64 Address, u64 Size)
{
	XStatus Status = XST_FAILURE;
	XPm_MemRegnDevice *MemRegnDevice;

	MemRegnDevice  = (XPm_MemRegnDevice *)XPm_AllocBytes(sizeof(XPm_MemRegnDevice));
	if (NULL == MemRegnDevice) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	Status = XPmMemRegnDevice_Init(MemRegnDevice, DeviceId, Address, Size);

done:
	return Status;
}
