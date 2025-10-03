/******************************************************************************
 * Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xstatus.h"
#include "xpm_requirement.h"

/********************* Macro / Struct Definitions *********************/

#define PL_MEM_REGN			(0x1U)
#define PL_MEM_REGN_FLAGS_SHIFT_64	(60U)
#define PL_MEM_REGN_FLAGS_MASK_64	((u64)0xF0000000U << 32)
#define PL_MEM_REGN_FLAGS(SZ_64BIT)	((u32)(((SZ_64BIT) & PL_MEM_REGN_FLAGS_MASK_64) >> PL_MEM_REGN_FLAGS_SHIFT_64))
#define IS_PL_MEM_REGN(SZ_64BIT)	((u32)PL_MEM_REGN == PL_MEM_REGN_FLAGS((u64)(SZ_64BIT)))

/********************* Static Function Definitions *********************/

/****************************************************************************/
/**
 * @brief  Checks whether given address range (addr + size) is contained
 * 		in any of the mem-regns for given subsystem
 *
 * @param  Subsystem 	Points to the subsystem struct
 * @param  RegionAddr 	Start Address of the range
 * @param  RegionSize 	Size of the range
 * @param  IsPLMem 	If valid, indicates whether PL or Non-PL Mem-Regn
 * @return Status of the operation : XPM_FAILURE, XPM_SUCCESS
 *
 ****************************************************************************/
static XStatus IsAddressInMemRegn(const XPm_Subsystem *Subsystem, u64 RegionAddr, u64 RegionSize, u8 *IsPLMem) {
	XStatus Status = XPM_FAILURE;
	const XPm_MemRegnDevice *MemRegnDevice = NULL;
	const XPm_Requirement *Reqm = NULL;
	u32 DeviceId;
	volatile AddrRangeStatus RangeCheck = ADDR_OUT_OF_RANGE;
	volatile AddrRangeStatus RangeCheckTmp = ADDR_OUT_OF_RANGE;

	/* Iterate over all mem-regns for particular subsystem */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		Reqm = ReqmNode->Data;
		DeviceId = Reqm->Device->Node.Id;

		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN != NODESUBCLASS(DeviceId)) {
			continue;
		}

		MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
		u64 StartAddress = MemRegnDevice->AddrRegion.Address;
		u64 MemSize = MemRegnDevice->AddrRegion.Size;
		u32 Flags = 0U;
		if (IS_PL_MEM_REGN(MemSize)) {
			Flags = PL_MEM_REGN_FLAGS(MemSize);
			/* zero-ing out upper flag bits[31:28] from 64bit size */
			MemSize &= ~PL_MEM_REGN_FLAGS_MASK_64;
		}
		u64 EndAddress = StartAddress + MemSize - 1U;

		XSECURE_REDUNDANT_CALL(RangeCheck, RangeCheckTmp, IsAddrWithinRange,
				RegionAddr, (RegionAddr + RegionSize - 1U), StartAddress, EndAddress);
		if ((ADDR_IN_RANGE== RangeCheck) && (ADDR_IN_RANGE == RangeCheckTmp)) {
			/* Check whether PL Mem or not */
			if (PL_MEM_REGN == Flags) {
				*IsPLMem = 1U;
			}
			Status = XPM_SUCCESS;
			break;
		}
	}
	return Status;
}
/*************************************************************************************************/

/****************************************************************************/
/**
 * @brief  Checks whether given address range (addr + size) is valid
 * 		in any Mem-Regn of given subsystem id.
 *
 * @param  SubsystemId 	The Subsystem to check for given addr range
 * @param  RegionAddr 	Start Address of the range
 * @param  RegionSize 	Size of the range
 * @param  IsPLMem 	If address range valid, indicates whether PL or Non-PL Mem-Regn
 *
 * @return Status of the operation : XPM_INVALID_SUBSYSID, XST_INVALID_PARAM, XPM_FAILURE,
 * 				XPM_SUCCESS
 *
 ****************************************************************************/
XStatus XPm_IsMemRegnAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize, u8 *IsPLMem) {
	volatile XStatus Status = XPM_FAILURE;
	const XPm_Subsystem *Subsystem = NULL;

	if (RegionSize < 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/**
	 * For built-in subsystems, we don't have any Mem-Regns
	 * and the below algorithm checking is not required!
	*/
	if (IS_BUILTIN_SUBSYSTEM(SubsystemId)) {
		Status = XPM_SUCCESS;
		goto done;
	}

	*IsPLMem = 0U;

	/**
	 * Check whether given address range is within ( or equal to ) the boundary
	 * of any the Mem-Regn for given subsystem
	 */
	Status = IsAddressInMemRegn(Subsystem, RegionAddr, RegionSize, IsPLMem);

	if (XPM_SUCCESS != Status) {
		PmInfo("Address Range not in Mem-Regn [0x%x]\r\n", SubsystemId);
		goto done;
	}
	if (1U == *IsPLMem) {
		/* Address Range provided falls in PL Region */
		/**
		 * Check whether PL Power Domain is up ( because no parent node for PL )
		 * Note: For any other Mem-Regn, power checking is done by retrieving
		 * their parent's controller node in XPm_IsMemAddressValid()
		 */
		const XPm_Power *Power = XPmPower_GetById(PM_POWER_PLD);
		if ((NULL != Power) && ((u8)XPM_POWER_STATE_ON == Power->Node.State)) {
			Status = XPM_SUCCESS;
		}
		else {
			Status = XPM_FAILURE;
		}
	}

done:
	return Status;
}