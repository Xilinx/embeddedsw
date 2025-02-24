/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_runtime_mem.h"

/****************************************************************************/
/**
 * @brief  Iterate through all memory region/device nodes (DDR, OCM, [TCM & PL not added yet])
 * 		   and return memory region for given Subsystem
 *
 * @param  SubsystemId 			ID of given subsystem
 * @param  AddrRegnArray 		Pointer to array to store memory region
 * @param  AddrRegnArrayLen 	Length of the array
 * @param  NumOfRegions 		Counter for total number of regions
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
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
	u8 Flags;

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

	/* Iterate over all devices for particular subsystem */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
		DeviceId = Reqm->Device->Node.Id;
		SubClass = NODESUBCLASS(DeviceId);
		Type = NODETYPE(DeviceId);
		Flags = PL_NODE_FLAGS(DeviceId);
        if (((u32)XPM_NODESUBCL_DEV_MEM == SubClass) &&
			(((u32)XPM_NODETYPE_DEV_OCM == Type) || ((u32)XPM_NODETYPE_DEV_TCM == Type))) {
            /* OCM/TCM Memory Region */
			MemDevice  = (XPm_MemDevice *)Reqm->Device;
			AddrRegnArray[*NumOfRegions].Address = (u64)MemDevice->StartAddress;
			AddrRegnArray[*NumOfRegions].Size = (u64)MemDevice->EndAddress -
							    (u64)MemDevice->StartAddress;
			(*NumOfRegions)++;
        }
        else if ((u32)XPM_NODESUBCL_DEV_MEM_REGN == SubClass) {
			MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
            if ((u32)PL_MEMORY_REGION == Flags) {
				/* PL Memory Region */
				/* zero-ing out the upper 4-bits */
				AddrRegnArray[*NumOfRegions].Address = (MemRegnDevice->AddrRegion.Address) & (~(PL_NODE_FLAGS_MASK_BITS << PL_NODE_FLAGS_SHIFT));
            }
			else {
				/* DDR Memory Region */
				AddrRegnArray[*NumOfRegions].Address = MemRegnDevice->AddrRegion.Address;
			}
			AddrRegnArray[*NumOfRegions].Size = MemRegnDevice->AddrRegion.Size;
			(*NumOfRegions)++;
        }
		else {
			/* Required by MISRA */
		}
		if (AddrRegnArrayLen < *NumOfRegions) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
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
 * @param  SubsystemId          ID of given subsystem
 * @param  AddressofSubsystem   Address in a given subsystem space
 * @param  IsValidAddress       Reference Pointer to store boolean 1/0
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
XStatus XPm_IsAddressInSubsystem(const u32 SubsystemId, u64 AddressofSubsystem,
					u8 *IsValidAddress)
{
	XStatus Status = XST_FAILURE;
	u64 StartAddress;
	u64 EndAddress;
	u32 DeviceId;
	u32 SubClass;
	u32 Type;
	u8 Flags;
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

	/* Iterate over all devices for particular subsystem */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
		DeviceId = Reqm->Device->Node.Id;
		SubClass = NODESUBCLASS(DeviceId);
		Type = NODETYPE(DeviceId);
		Flags = PL_NODE_FLAGS(DeviceId);
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

				if ((u32)PL_MEMORY_REGION == Flags) {
					XPm_Power *Power = XPmPower_GetById(PM_POWER_PLD);
					/* check whether PL region is up or not */
					if(NULL != Power && (u8)XPM_DEVSTATE_PWR_ON == Power->Node.State) {
						*IsValidAddress = 1U;
						Status = XST_SUCCESS;
						goto done;
					}
					*IsValidAddress = 0U;
					break;
				}

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
		 * OCM/TCM devices.
		 */
		} else if (((u32)XPM_NODESUBCL_DEV_MEM == SubClass) &&
			(((u32)XPM_NODETYPE_DEV_OCM == Type) || ((u32)XPM_NODETYPE_DEV_TCM == Type))) {
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
	}

	Status = XST_SUCCESS;

done:
	return Status;
}