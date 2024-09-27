/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
	u32 Size = Tcm->EndAddress - Tcm->StartAddress;
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

XStatus XPm_IsAddressInSubsystem(const u32 SubsystemId, u64 AddressofSubsystem,
				 u8 *IsValidAddress)
{
	XStatus Status = XST_FAILURE;
	u64 StartAddress;
	u64 EndAddress;
	u32 DeviceId;
	u32 SubClass;
	u32 Type;
	const XPm_Subsystem *Subsystem;
	const XPm_Requirement *Reqm;
	const XPm_MemRegnDevice *MemRegnDevice;
	const XPm_MemCtrlrDevice *MCDev;
	const XPm_MemDevice *MemDevice;

	*IsValidAddress = 0U;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Reqm = Subsystem->Requirements;

	/* Iterate over all devices for particular subsystem */
	while (NULL != Reqm) {
		DeviceId = Reqm->Device->Node.Id;
		SubClass = NODESUBCLASS(DeviceId);
		Type = NODETYPE(DeviceId);
		/*
		 * Validate the address to confirm its inclusion within the
		 * memory region devices.
		 */
		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN == SubClass) {
			MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
			StartAddress = MemRegnDevice->AddrRegion.Address;
			EndAddress = MemRegnDevice->AddrRegion.Address +
				     MemRegnDevice->AddrRegion.Size;
			if ((AddressofSubsystem >= StartAddress) &&
				(AddressofSubsystem < EndAddress)) {
				/*
				 * Iterate through all DDRMC nodes and verify
				 * whether the specified address falls within
				 * any of their associated memory regions.
				 */
				for (u32 i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN;
				     i <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX; i++) {
#ifdef XPM_NODEIDX_DEV_DDRMC_MAX_INT_1
					if (((u32)XPM_NODEIDX_DEV_DDRMC_MAX_INT_1 + 1U) == i) {
						i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN_INT_2;
					}
#endif
					MCDev = (XPm_MemCtrlrDevice *)XPmDevice_GetById(DDRMC_DEVID(i));
					if (NULL == MCDev) {
						continue;
					}

					for (u32 Cnt = 0U; Cnt < MCDev->RegionCount; Cnt++) {
						StartAddress = MCDev->Region[Cnt].Address;
						EndAddress = MCDev->Region[Cnt].Address +
								MCDev->Region[Cnt].Size;
						if ((AddressofSubsystem >= StartAddress) &&
						    (AddressofSubsystem < EndAddress)) {
							/*
							 * the memory controller
							 * should be in running
							 * state
							 */
							if ((u8)XPM_DEVSTATE_RUNNING !=
							    MCDev->Device.Node.State) {
								*IsValidAddress = 0U;
								Status = XST_SUCCESS;
								goto done;
							}
							*IsValidAddress = 1U;
							break;
						}
					}
					if ((1U == *IsValidAddress) &&
					    (0U == MCDev->IntlvIndex)) {
						break;
					}
				}
				Status = XST_SUCCESS;
				goto done;
			}
		/*
		 * Validate the address to confirm its inclusion within the
		 * OCM devices.
		 */
		} else if (((u32)XPM_NODESUBCL_DEV_MEM == SubClass) &&
			   ((u32)XPM_NODETYPE_DEV_OCM == Type)) {
				MemDevice  = (XPm_MemDevice *)Reqm->Device;
				StartAddress = (u64)MemDevice->StartAddress;
				EndAddress = (u64)MemDevice->EndAddress;
				if ((AddressofSubsystem >= StartAddress) &&
					(AddressofSubsystem < EndAddress)) {
					/*
					 * The memory controller should be in
					 * running state.
					 */
					if ((u8)XPM_DEVSTATE_RUNNING ==
						MemDevice->Device.Node.State) {
						*IsValidAddress = 1U;
					}
					Status = XST_SUCCESS;
					goto done;
				}
		} else {
			/* Required by MISRA */
		}
		Reqm = Reqm->NextDevice;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_GetAddrRegnForSubsystem(const u32 SubsystemId, XPm_AddrRegion *AddrRegnArray,
				    u32 AddrRegnArrayLen, u32 *NumOfRegions)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;
	const XPm_Requirement *Reqm;
	const XPm_MemRegnDevice *MemRegnDevice;
	const XPm_MemDevice *MemDevice;
	u32 DeviceId;
	u32 SubClass;
	u32 Type;

	*NumOfRegions = 0U;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	if (0U >= AddrRegnArrayLen) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Reqm = Subsystem->Requirements;

	/* Iterate over all devices for particular subsystem */
	while (NULL != Reqm) {
		DeviceId = Reqm->Device->Node.Id;
		SubClass = NODESUBCLASS(DeviceId);
		Type = NODETYPE(DeviceId);
		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN == SubClass) {
			MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
			AddrRegnArray[*NumOfRegions].Address = MemRegnDevice->AddrRegion.Address;
			AddrRegnArray[*NumOfRegions].Size = MemRegnDevice->AddrRegion.Size;
			(*NumOfRegions)++;
		} else if (((u32)XPM_NODESUBCL_DEV_MEM == SubClass) &&
			   ((u32)XPM_NODETYPE_DEV_OCM == Type)) {
			MemDevice  = (XPm_MemDevice *)Reqm->Device;
			AddrRegnArray[*NumOfRegions].Address = (u64)MemDevice->StartAddress;
			AddrRegnArray[*NumOfRegions].Size = (u64)MemDevice->EndAddress -
							    (u64)MemDevice->StartAddress;
			(*NumOfRegions)++;
		} else {
			/* Required by MISRA */
		}
		if (AddrRegnArrayLen < *NumOfRegions) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Reqm = Reqm->NextDevice;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}
