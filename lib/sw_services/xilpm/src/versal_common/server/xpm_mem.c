/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_debug.h"

static const XPm_StateCap XPmMemDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = PM_CAP_ACCESS | PM_CAP_CONTEXT,
	},
};

static const XPm_StateTran XPmMemDevTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	},
};

static XStatus HandleMemDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(Device);
		} else {
			Status = XST_SUCCESS;
		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     XPM_DEVEVENT_SHUTDOWN);
		} else {
			Status = XST_SUCCESS;
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static const XPm_DeviceFsm XPmMemDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = HandleMemDeviceState,
};

static void TcmEccInit(const XPm_MemDevice *Tcm, u32 Mode)
{
	u32 Size = Tcm->EndAddress - Tcm->StartAddress + 1U;
	u32 Id = Tcm->Device.Node.Id;
	u32 Base = Tcm->StartAddress;

	Base -= XPm_CombTcm(Id,Mode);

	if (0U != Size) {
		s32 Status = XPlmi_EccInit(Base, Size);
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in EccInit of 0x%x\r\n", Status, Tcm->Device.Node.Id);
		}
	}
	return;
}

static XStatus HandleTcmDeviceState(XPm_Device* const Device, u32 const NextState)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Rpu0Device = NULL,*Rpu1Device = NULL;
	u32 Id = Device->Node.Id;
	u32 Mode;

	Status = XPm_GetRpuDevice(&Rpu0Device, &Rpu1Device, Id);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if ((NULL == Rpu0Device) || (NULL == Rpu1Device))
	{
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(Device);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Request the RPU clocks. Here both core having same RPU clock */
			Status = XPmClock_Request(Rpu0Device->ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* TCM is only accessible when the RPU is powered on and out of reset and is in halted state
			 * so bring up RPU too when TCM is requested*/
			Status = HaltRpuCore(Rpu0Device, Rpu1Device, Id, &Mode);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Tcm should be ecc initialized */
			TcmEccInit((XPm_MemDevice *)Device, Mode);
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
			Status = XPmClock_Release(Rpu0Device->ClkHandles);
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

static const XPm_DeviceFsm XPmTcmDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = HandleTcmDeviceState,
};

XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress)
{
	XStatus Status = XST_FAILURE;
	u32 Type = NODETYPE(Id);

	Status = XPmDevice_Init(&MemDevice->Device, Id, BaseAddress, Power, Clock,
				Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MemDevice->StartAddress = MemStartAddress;
	MemDevice->EndAddress = MemEndAddress;

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_DDR:
		XPm_AssignDdrFsm(MemDevice);
		break;
	case (u32)XPM_NODETYPE_DEV_TCM:
		MemDevice->Device.DeviceFsm = &XPmTcmDeviceFsm;
		break;
	default:
		MemDevice->Device.DeviceFsm = &XPmMemDeviceFsm;
		break;
	}

	if (NULL == MemDevice->Device.DeviceFsm) {
		MemDevice->Device.DeviceFsm = &XPmMemDeviceFsm;
	}

done:
	return Status;
}

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

static u8 XPm_IsValidMemRegnDDRorPL(u64 AddrToCheck, u32 Flags)
{
	u8 IsValid = 0U;

	/* Check whether PL mem or not */
	if (PL_MEM_REGN == Flags) {
		const XPm_Power *Power = XPmPower_GetById(PM_POWER_PLD);
		/* check whether PL region is up or not */
		if ((NULL != Power) && ((u8)XPM_POWER_STATE_ON == Power->Node.State)) {
			IsValid = 1U;
			goto done;
		}
	}
	/*
	 * Iterate through all DDRMC nodes and verify whether the specified address falls within
	 * any of their associated memory regions.
	 */
	for (u32 i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; i <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX; i++) {
#ifdef PM_NODEIDX_DEV_DDRMC_MAX_INT_1
		if (((u32)XPM_NODEIDX_DEV_DDRMC_MAX_INT_1 + 1U) == i) {
			i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN_INT_2;
		}
#endif
		const XPm_MemCtrlrDevice *MCDev = (XPm_MemCtrlrDevice *)XPmDevice_GetById(DDRMC_DEVID(i));
		if (NULL == MCDev) {
			continue;
		}

		for (u32 Cnt = 0U; Cnt < MCDev->RegionCount; Cnt++) {
			u64 StartAddress = MCDev->Region[Cnt].Address;
			u64 EndAddress = MCDev->Region[Cnt].Address + MCDev->Region[Cnt].Size - 1U;
			if ((AddrToCheck >= StartAddress) &&
				(AddrToCheck <= EndAddress)) {
				/*
				 * the memory controller should be in running state
				 * for the address to be accessible
				 */
				if ((u8)XPM_DEVSTATE_RUNNING != MCDev->Device.Node.State) {
					IsValid = 0U;
					goto done;
				}
				IsValid = 1U;
				break;
			}
		}
		/*
		 * If the address is valid and the memory controller is interleaved,
		 * then keep going to check for paired memory controller.
		 */
		if ((1U == IsValid) && (0U == MCDev->IntlvIndex)) {
			break;
		}
	}

done:
	return IsValid;
}

XStatus XPm_IsAddressInSubsystem(u32 SubsystemId, u64 AddrToCheck, u8 *IsValid)
{
	XStatus Status = XST_FAILURE;
	const XPm_MemRegnDevice *MemRegnDevice;
	const XPm_Subsystem *Subsystem;
	const XPm_MemDevice *MemDevice;
	const XPm_Requirement *Reqm;
	u64 StartAddress;
	u64 EndAddress;
	u64 Size;
	u32 DeviceId;
	u32 SubClass;
	u32 Type;

	*IsValid = 0U;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Reqm = Subsystem->Requirements;

	/*
	* Validate the address to confirm its inclusion within the
	* memory region devices.
	*
	* - OCM / TCM
	*/
	while (NULL != Reqm) {
		DeviceId = Reqm->Device->Node.Id;
		SubClass = NODESUBCLASS(DeviceId);
		Type = NODETYPE(DeviceId);
		if (((u32)XPM_NODESUBCL_DEV_MEM == SubClass) &&
			(((u32)XPM_NODETYPE_DEV_OCM == Type) || ((u32)XPM_NODETYPE_DEV_TCM == Type))) {
			MemDevice  = (XPm_MemDevice *)Reqm->Device;
			StartAddress = (u64)MemDevice->StartAddress;
			EndAddress = (u64)MemDevice->EndAddress;
			if ((AddrToCheck >= StartAddress) && (AddrToCheck <= EndAddress)) {
				/*
				* The memory controller should be in running state
				* for the address to be accessible
				*/
				if ((u8)XPM_DEVSTATE_RUNNING == MemDevice->Device.Node.State) {
					*IsValid = 1U;
				}
				/* Match found, finished regardless of controller is running */
				Status = XST_SUCCESS;
				goto done;
			}
		}
		Reqm = Reqm->NextDevice;
	}

	Reqm = Subsystem->Requirements;

	/*
	* Validate the address to confirm its inclusion within the
	* memory region devices.
	*
	* - PL
	* - DDR
	*/
	while (NULL != Reqm) {
		DeviceId = Reqm->Device->Node.Id;
		SubClass = NODESUBCLASS(DeviceId);
		Type = NODETYPE(DeviceId);
		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN == SubClass) {
			u32 Flags = 0U;
			MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
			StartAddress = MemRegnDevice->AddrRegion.Address;
			Size = MemRegnDevice->AddrRegion.Size;
			if (IS_PL_MEM_REGN(Size)) {
				Flags = PL_MEM_REGN_FLAGS(Size);
				/* zero-ing out upper flag bits[31:28] from 64bit size */
				Size &= ~PL_MEM_REGN_FLAGS_MASK_64;
			}
			EndAddress = StartAddress + Size - 1U;

			/** Check if given DDR/PL address is valid
			 * Conditions:
			 * - whether the PL region is up or not (PL)
			 * - whether given DDR controller is running or not (DDR)
			 */
			if ((AddrToCheck >= StartAddress) && (AddrToCheck <= EndAddress)) {
				*IsValid = XPm_IsValidMemRegnDDRorPL(AddrToCheck, Flags);
				/* We're done in either case */
				break;
			}
		}
		Reqm = Reqm->NextDevice;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_GetAddrRegnForSubsystem(u32 SubsystemId, XPm_AddrRegion *AddrRegnArray,
				    u32 AddrRegnArrayLen, u32 *NumOfRegions)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;
	const XPm_Requirement *Reqm;
	const XPm_MemRegnDevice *MemRegnDevice;
	u64 Address, Size;
	u32 DeviceId;

	*NumOfRegions = 0U;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Reqm = Subsystem->Requirements;

	/* Iterate over all devices for particular subsystem */
	while (NULL != Reqm) {
		DeviceId = Reqm->Device->Node.Id;

		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN != NODESUBCLASS(DeviceId)) {
			Reqm = Reqm->NextDevice;
			continue;
		}

		if (AddrRegnArrayLen <= *NumOfRegions) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
		Address = MemRegnDevice->AddrRegion.Address;
		Size = MemRegnDevice->AddrRegion.Size;

		if (IS_PL_MEM_REGN(Size)) {
			/* Zero-ing out upper flag bits[31:28] from 64bit size for PL */
			Size &= ~PL_MEM_REGN_FLAGS_MASK_64;
		}
		AddrRegnArray[*NumOfRegions].Address = Address;
		AddrRegnArray[*NumOfRegions].Size = Size;
		(*NumOfRegions)++;

		Reqm = Reqm->NextDevice;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}
