/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_alloc.h"
#include "xpm_debug.h"
#include "xpm_regs.h"
#include "xpm_common.h"

/******************************************************************/
/**
 * @brief  MemRegn checking feature not supported in BOOT
 *
 * @param  SubsystemId 	N/A
 * @param  RegionAddr 	N/A
 * @param  RegionSize 	N/A
 *
 * @return Status of the operation: XPM_SUCCESS, XST_INVALID_PARAM
 *
 * @note  Weak Symbol - Stub implementation for BOOT-only builds.
 *
 ******************************************************************/
XStatus __attribute__((weak, noinline)) XPm_IsMemAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize) {
	volatile XStatus Status = XPM_FAILURE;
	(void)SubsystemId;
	(void)RegionAddr;
	(void)RegionSize;

	Status = XPM_NO_FEATURE;
	return Status;
}

/*************************************************************************************************/

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
 * @brief  Find an existing memory region device by address
 *
 * @param  Address	Address to search for
 *
 * @return Pointer to existing device if found, NULL otherwise
 *
 ****************************************************************************/
static XPm_MemRegnDevice *XPm_FindMemRegnByAddress(u64 Address)
{
	XPm_MemRegnDevice *MemRegnDevice;
	const XPm_Device *Device;

	/*
	 * Memory region devices are stored in PmMemRegnDevices array,
	 * not in the main PmDevices array.
	 */
	for (u32 Idx = 0U; Idx < (u32)XPM_NODEIDX_DEV_MEM_REGN_MAX; Idx++) {
		Device = XPmDevice_GetMemRegnByIndex(Idx);
		if (NULL == Device) {
			continue;
		}
		MemRegnDevice = (XPm_MemRegnDevice *)Device;
		if (MemRegnDevice->AddrRegion.Address == Address) {
			return MemRegnDevice;
		}
	}
	return NULL;
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
 * @note   If a memory region with the same address already exists (e.g., during
 *         repeated PLD loads in segmented configuration), this function returns
 *         XST_DEVICE_BUSY without allocating new memory. This prevents memory
 *         pool exhaustion. If size differs, it updates the size and logs a warning.
 *         Caller should treat XST_DEVICE_BUSY as success and skip requirement add.
 *
 *         The check uses Address (not DeviceId) because DeviceId is generated
 *         from an incrementing counter and is always unique on each call.
 *
 ****************************************************************************/
XStatus XPm_AddMemRegnDevice(u32 DeviceId, u64 Address, u64 Size)
{
	XStatus Status = XST_FAILURE;
	XPm_MemRegnDevice *MemRegnDevice;

	/*
	 * Check if a memory region with the same address already exists.
	 * This can happen during repeated PLD loads in segmented configuration.
	 */
	MemRegnDevice = XPm_FindMemRegnByAddress(Address);
	if (NULL != MemRegnDevice) {
		/* Memory region already exists */
		if (MemRegnDevice->AddrRegion.Size != Size) {
			PmWarn("MemRegn 0x%x size changed: 0x%x%08x -> 0x%x%08x\r\n",
				MemRegnDevice->Device.Node.Id,
				(u32)(MemRegnDevice->AddrRegion.Size >> 32),
				(u32)MemRegnDevice->AddrRegion.Size,
				(u32)(Size >> 32), (u32)Size);
			MemRegnDevice->AddrRegion.Size = Size;
		}
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	MemRegnDevice = (XPm_MemRegnDevice *)XPm_AllocBytes(sizeof(XPm_MemRegnDevice));
	if (NULL == MemRegnDevice) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	Status = XPmMemRegnDevice_Init(MemRegnDevice, DeviceId, Address, Size);

done:
	return Status;
}
