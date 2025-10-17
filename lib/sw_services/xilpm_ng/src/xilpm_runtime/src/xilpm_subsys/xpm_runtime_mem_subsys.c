/******************************************************************************
 * Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xstatus.h"
#include "xpm_regs.h"
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
 * @brief  Callback to flush given memory region using ABF
 *
 * @param  AddrRegn	Handle to memory region to be flushed
 *
 * @return XST_SUCCESS if successful, error code otherwise
 *
 ****************************************************************************/
static XStatus AddrRegnCmnFlushCb(const XPm_AddrRegion *const AddrRegion)
{
	XStatus Status = XST_FAILURE;
	const u32 MemAddr[] = {
		POR_HNF_NODE_INFO_U_HNF_NID8_BASEADDR,
		POR_HNF_NODE_INFO_U_HNF_NID72_BASEADDR,
		POR_HNF_NODE_INFO_U_HNF_NID136_BASEADDR,
		POR_HNF_NODE_INFO_U_HNF_NID200_BASEADDR,
	};

	if (NULL == AddrRegion) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/**
	 * Flush DDR and OCM addresses using ABF (Address based flush) for
	 * NID8, NID72, NID136 and NID200
	 */
	PmInfo("Region: Addr: 0x%x_%x, Size: 0x%x_%x\r\n",
		(u32)(AddrRegion->Address >> 32U), (u32)(AddrRegion->Address),
		(u32)(AddrRegion->Size >> 32U), (u32)(AddrRegion->Size));

	for (u32 Idx = 0U; Idx < ARRAY_SIZE(MemAddr); Idx++) {
		XPm_Out32(MemAddr[Idx] + ABF_LO_ADDR_U_HNF_NID_OFFSET_0,
			  (u32)(AddrRegion->Address));
		XPm_Out32(MemAddr[Idx] + ABF_LO_ADDR_U_HNF_NID_OFFSET_1,
			  (u32)(AddrRegion->Address >> 32U));
		XPm_Out32(MemAddr[Idx] + ABF_HI_ADDR_U_HNF_NID_OFFSET_0,
			  (u32)(AddrRegion->Address + AddrRegion->Size));
		XPm_Out32(MemAddr[Idx] + ABF_HI_ADDR_U_HNF_NID_OFFSET_1,
			  (u32)((AddrRegion->Address + AddrRegion->Size) >> 32U));
		XPm_Out32(MemAddr[Idx] + ABF_PR_U_HNF_NID_OFFSET, 0x1U);
		Status = XPm_PollForMask(MemAddr[Idx] + ABF_SR_U_HNF_NID_OFFSET, 0x1U,
					 XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Iterate through all memory region/device nodes (DDR, OCM, TCM, PL)
 *	   and apply the flush callback for each region
 *
 * @param  SubsystemId	ID of given subsystem
 * @param  CmnFlushCb	Callback to flush given memory region
 *
 * @return XST_SUCCESS if successful else appropriate return code
 *
 ****************************************************************************/
static XStatus SubsystemCmnFlush(u32 SubsystemId,
		XStatus (*CmnFlushCb)(const XPm_AddrRegion *const AddrRegion))
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;
	XPm_AddrRegion AddrRegn = { 0U };

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* Iterate over all devices for particular subsystem */
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		const XPm_MemRegnDevice *MemRegnDevice = NULL;
		const XPm_Requirement *Reqm = ReqmNode->Data;
		u32 DeviceId = Reqm->Device->Node.Id;
		u64 Address = 0U, Size = 0U;

		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN != NODESUBCLASS(DeviceId)) {
			continue;
		}
		MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
		Address = MemRegnDevice->AddrRegion.Address;
		Size = MemRegnDevice->AddrRegion.Size;

		if (IS_PL_MEM_REGN(Size)) {
			/* Zero-ing out upper flag bits[31:28] from 64bit size for PL */
			Size &= ~PL_MEM_REGN_FLAGS_MASK_64;
		}
		AddrRegn.Address = Address;
		AddrRegn.Size = Size;

		if (NULL != CmnFlushCb) {
			Status = CmnFlushCb(&AddrRegn);
			if (XST_SUCCESS != Status) {
				PmErr("Failed to flush region: Addr: 0x%x_%x, Size: 0x%x_%x\r\n",
					(u32)(AddrRegn.Address >> 32), (u32)(AddrRegn.Address),
					(u32)(AddrRegn.Size >> 32), (u32)(AddrRegn.Size));
				goto done;
			}
		}
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

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

	/**
	 * For built-in subsystems, we don't have any Mem-Regns
	 * and the below algorithm checking is not required!
	*/
	if (IS_BUILTIN_SUBSYSTEM(SubsystemId)) {
		Status = XPM_SUCCESS;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
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

/****************************************************************************/
/**
 * @brief  Flush all memory region/device nodes (DDR, OCM, TCM, PL) for the
 *	   given subsystem
 *
 * @param  SubsystemId	ID of given subsystem
 *
 * @return XST_SUCCESS if successful else appropriate return code
 *
 ****************************************************************************/
XStatus XPmSubsystem_CmnFlush(u32 SubsystemId)
{
	return SubsystemCmnFlush(SubsystemId, AddrRegnCmnFlushCb);
}
