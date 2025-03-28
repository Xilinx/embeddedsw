/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_runtime_mem.h"

static u8 XPm_IsValidMemRegnDDRorPL(u64 AddrToCheck, u32 Flags)
{
	u8 IsValid = 0U;

	/* Check whether PL mem or not */
	if (PL_MEM_REGN == Flags) {
		const XPm_Power *Power = XPmPower_GetById(PM_POWER_PLD);
		/* check whether PL region is up or not */
		if (NULL != Power && (u8)XPM_POWER_STATE_ON == Power->Node.State) {
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

/****************************************************************************/
/**
 * @brief  Iterate through all memory region/device nodes (DDR, OCM, TCM, PL)
 * 		   and return memory region for given Subsystem
 *
 * @param  SubsystemId		ID of given subsystem
 * @param  AddrRegnArray	Pointer to array to store memory region
 * @param  AddrRegnArrayLen	Length of the array
 * @param  NumOfRegions		Counter for total number of regions
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
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

	/* Iterate over all devices for particular subsystem */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
		DeviceId = Reqm->Device->Node.Id;

		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN != NODESUBCLASS(DeviceId)) {
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
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Iterate over all the memory regions of a given subsystem and return
 *          given address falls within the range or not
 *
 * @param  SubsystemId   ID of given subsystem
 * @param  AddrToCheck   Address to check
 * @param  IsValid       Reference Pointer to store boolean 1/0
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
XStatus XPm_IsAddressInSubsystem(u32 SubsystemId, u64 AddrToCheck, u8 *IsValid)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;
	const XPm_Requirement *Reqm;
	const XPm_MemRegnDevice *MemRegnDevice;
	const XPm_MemDevice *MemDevice;
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

	/*
	* Validate the address to confirm its inclusion within the
	* memory region devices.
	*
	* - OCM / TCM
	*/
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
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
				* The memory controller should be in running state.
				*/
				if ((u8)XPM_DEVSTATE_RUNNING == MemDevice->Device.Node.State) {
					*IsValid = 1U;
				}
				/* Match found, finished regardless of controller is running */
				Status = XST_SUCCESS;
				goto done;
			}
		}
	}

	/*
	* Validate the address to confirm its inclusion within the
	* memory region devices.
	*
	* - PL
	* - DDR
	*/
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
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
	}

	/* Status should be success irrespective of whether the address is valid or not */
	Status = XST_SUCCESS;

done:
	return Status;
}
