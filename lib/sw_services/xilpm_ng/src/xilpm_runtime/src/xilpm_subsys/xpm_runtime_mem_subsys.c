/******************************************************************************
 * Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xstatus.h"
#include "xpm_regs.h"
#include "xpm_requirement.h"
#include "cfu_apb.h"
#include "xpm_runtime_mem.h"

/********************* Macro / Struct Definitions *********************/

#define PL_MEM_REGN			(0x1U)
#define PL_MEM_REGN_FLAGS_SHIFT_64	(60U)
#define PL_MEM_REGN_FLAGS_MASK_64	((u64)0xF0000000U << 32)
#define PL_MEM_REGN_FLAGS(SZ_64BIT)	((u32)(((SZ_64BIT) & PL_MEM_REGN_FLAGS_MASK_64) >> PL_MEM_REGN_FLAGS_SHIFT_64))
#define IS_PL_MEM_REGN(SZ_64BIT)	((u32)PL_MEM_REGN == PL_MEM_REGN_FLAGS((u64)(SZ_64BIT)))

/* check whether PL EOS startup bit is asserted */
#define IS_PL_STARTUP_ASSERTED 		((XPlmi_In32(CFU_APB_CFU_FGCR) & CFU_APB_CFU_FGCR_EOS_MASK))

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
	const XPm_MemRegnDevice *MemRegnDevice = NULL;
	const XPm_Subsystem *Subsystem = NULL;
	XPm_Requirement *Reqm = NULL;

	/* IsPLMem is a mandatory out-parameter; reject NULL before dereferencing */
	if (NULL == IsPLMem) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	*IsPLMem = FALSE;

	if (RegionSize < 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/**
	 * Guard against RegionAddr + RegionSize overflowing the 64-bit address space
	 */
	if (RegionAddr > (U64_MAX_SIZE - RegionSize)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* store the provided region address in a local variable */
	u64 CurrentRegionAddr = RegionAddr;
	/* store the provided region size in a local variable */
	u64 CurrentRegionSize = RegionSize;
	/* indicates whether there's an overlapping of region or not */
	u8 IsOverlapMemRegion = FALSE;
	/* indicates the type of memory overlap */
	u8 OverlapMemType = MEM_TYPE_NONE;
	/* boolean condition to skip subsystem level checks */
	u8 SkipSubsysCheck = FALSE;

	/* check only non-PLM internal subsystems */
	if ((PM_SUBSYS_PMC != SubsystemId) && (PM_SUBSYS_ASU != SubsystemId)) {
		Subsystem = XPmSubsystem_GetById(SubsystemId);
		if (NULL == Subsystem) {
			Status = XPM_INVALID_SUBSYSID;
			goto done;
		}
	}
	else {
		SkipSubsysCheck = TRUE;
	}

	/**
	 * Mem-Regn address validity and/or Subsystem level access checking
	 * - For Built-in Subsystems (PMC & ASU), there are no requirements and hence we perform only
	 * physical address range checking for all the available Mem-Regns.
	 * - For other Subsystems (Default and User-Defined), we perform both, i.e
	 * address range checking and Subsystem level access checking.
	 * - Note: PL Mem-Regn is special condition where we check PLD Power and EOS bit
	 * and conclude entire access checking (PL has no parent controller node).
	 */
	u32 index = (u32)XPM_NODEIDX_DEV_MEM_REGN_MIN;
	while (index <= (u32)XPM_NODEIDX_DEV_MEM_REGN_MAX) {
		MemRegnDevice = (XPm_MemRegnDevice *)XPmDevice_GetMemRegnByIndex(index);
		if (NULL == MemRegnDevice) {
			index++;
			continue;
		}

		u64 StartAddress = MemRegnDevice->AddrRegion.Address;
		u64 MemSize = MemRegnDevice->AddrRegion.Size;
		u32 Flags = 0U;
		if (IS_PL_MEM_REGN(MemSize)) {
			Flags = PL_MEM_REGN_FLAGS(MemSize);
			/* zero-ing out upper flag bits[31:28] from 64bit size */
			MemSize &= ~PL_MEM_REGN_FLAGS_MASK_64;
		}
		/**
		 * Guard against zero-size mem-regn:
		 * MemSize=0 would underflow and produce an arbitrarily large address range
		 */
		if (MemSize < 1U) {
			index++;
			continue;
		}
		u64 EndAddress = StartAddress + MemSize - 1U;
		AddrRangeStatus Range = checkAddrRangeContainment(&CurrentRegionAddr, &CurrentRegionSize, StartAddress, EndAddress);

		/* skip to next device */
		if (ADDR_NOT_IN_RANGE == Range) {
			index++;
			continue;
		}

		/**
		 * skip iteration to failure for memory type mismatch during overlapping condition
		 * - only access to same memory type is allowed during overlapping condition
		 */
		if (PL_MEM_REGN == Flags) {
			if ((TRUE == IsOverlapMemRegion) && (MEM_TYPE_PL != OverlapMemType)) {
				PmErr("Mismatch Memory type access!\r\n");
				Status = XPM_ERR_MEM_ACCESS_TYPE;
				goto done;
			}
			*IsPLMem = TRUE;
			OverlapMemType = MEM_TYPE_PL;
		} else {
			if ((TRUE == IsOverlapMemRegion) && (MEM_TYPE_PL == OverlapMemType)) {
				PmErr("Mismatch Memory type access!\r\n");
				Status = XPM_ERR_MEM_ACCESS_TYPE;
				goto done;
			}
			OverlapMemType = MEM_TYPE_OCM_TCM;
		}

		if (FALSE == SkipSubsysCheck) {
			Reqm = FindReqm((const XPm_Device *)MemRegnDevice, Subsystem);
			if (NULL == Reqm) {
				/* Device not allocated to the subsystem */
				/**
				* NOTE: This assumes that no MemRegns are overlapping between subsystems.
				* In future if MemRegns sharing is supported, then the following assumption
				* needs to be updated accordingly.
				*/
				Status = XPM_FAILURE;
				goto done;
			}
		}

		if (ADDR_IN_RANGE == Range) {
			/* entire region is validated, we are done */
			Status = XPM_SUCCESS;
			break;
		}
		/* for overlapping condition, we continue the iteration */
		IsOverlapMemRegion = TRUE;

		/* Iterate to next Mem-Regn Device */
		index++;
	}

	/**
	 * If we encountered overlapping region(s) but didn't fully
	 * consume the range (no ADDR_IN_RANGE break), it's a failure.
	 * goto done to skip the PL power check (which would override Status).
	 */
	if ((TRUE == IsOverlapMemRegion) && (XPM_SUCCESS != Status)) {
		Status = XPM_FAILURE;
		goto done;
	}

	if (TRUE == *IsPLMem) {
		/* Address Range provided falls in PL Region */
		/**
		 * - Check whether PL Power Domain is up ( because no parent node for PL )
		 * Note: For any other Mem-Regn, power checking is done by retrieving
		 * their parent's controller node in XPm_IsMemAddressValid()
		 * - Check whether PL Startup has been asserted in hardware
		 */
		const XPm_Power *Power = XPmPower_GetById(PM_POWER_PLD);
		if ((NULL != Power) && ((u8)XPM_POWER_STATE_ON == Power->Node.State) && IS_PL_STARTUP_ASSERTED) {
			Status = XPM_SUCCESS;
		}
		else {
			Status = XPM_PM_NO_ACCESS;
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
