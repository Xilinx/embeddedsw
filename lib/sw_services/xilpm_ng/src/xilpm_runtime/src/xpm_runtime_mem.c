/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_runtime_mem.h"
#include "xpm_mem.h"
#include "xpm_subsystem.h"

struct XPm_ExclusionRegion {
	u64 StartAddress;
	u64 EndAddress;
};

/*********************************************************************************/
/**
 * @brief  Checks whether address check is performed for built-in subsystems
 *
 * @param  SubsystemId 	checks whether given subsystem is accessible without
 * 				subsystem dependencies
 * @param  RegionAddr 	N/A
 * @param  RegionSize 	N/A
 * @param  IsPLMem 	N/A
 *
 * @return  Status of the operation: XPM_SUCCESS, XST_INVALID_PARAM
 *
 * @note:  Weak Symbol - Runtime function which can be overridden by SUBSYS module
 *
 *********************************************************************************/
XStatus __attribute__((weak, noinline)) XPm_IsMemRegnAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize, u8 *IsPLMem) {

	volatile XStatus Status = XPM_FAILURE;
	(void)(RegionAddr);
	(void)(RegionSize);
	(void)(IsPLMem);

	/**
	 * Supported Subsystem for eemi:
	 * - PM_SUBSYS_DEFAULT
	 * - PM_SUBSYS_PMC
	 * - PM_SUBSYS_ASU
	 */
	if (!IS_BUILTIN_SUBSYSTEM(SubsystemId)) {
		PmErr("Feature not supported for [0x%x]\r\n", SubsystemId);
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPM_SUCCESS;

done:
	return Status;
}

/**
 * @brief  Checks whether a given address range has any overlap with an exclusion range
 *
 * @param  RegionStart	Start Address of the region to check
 * @param  RegionEnd	End Address of the region to check
 * @param  ExclStart	Start Address of the exclusion range
 * @param  ExclEnd	End Address of the exclusion range
 *
 * @return ADDR_IN_RANGE if any overlap exists, ADDR_NOT_IN_RANGE otherwise
 *
 * @note   Detects all four overlap geometries:
 *         - Region start falls inside exclusion (partial overlap at start)
 *         - Region end falls inside exclusion (partial overlap at end)
 *         - Region fully contained within exclusion
 *         - Region fully contains exclusion (neither endpoint falls inside exclusion)
 *         Default initializer is ADDR_IN_RANGE for fail-safe (deny by default).
 */
static inline AddrRangeStatus checkRangeOverlapExclusion(u64 RegionStart, u64 RegionEnd, u64 ExclStart, u64 ExclEnd) {
	volatile AddrRangeStatus Range = ADDR_IN_RANGE;

	if (((RegionStart >= ExclStart) && (RegionStart <= ExclEnd)) ||
	    ((RegionEnd >= ExclStart) && (RegionEnd <= ExclEnd))) {
		Range = ADDR_IN_RANGE;
	}
	else if ((RegionStart < ExclStart) && (RegionEnd > ExclEnd)) {
		/* Region fully contains the exclusion range (neither endpoint inside exclusion) */
		Range = ADDR_IN_RANGE;
	}
	else {
		Range = ADDR_NOT_IN_RANGE;
	}

	return Range;
}

/**
 * @brief  Iterates over the exclusion list and checks whether a given address
 *         range overlaps with or is contained within any excluded region
 *
 * @param  RegionStartAddr	Start Address of the region to validate
 * @param  RegionEndAddr	End Address of the region to validate
 * @param  ExcludedList		Pointer to the array of exclusion regions
 * @param  ExcludedListSize	Number of entries in the exclusion list
 *
 * @return XPM_SUCCESS if region does not overlap any exclusion entry,
 *         XPM_FAILURE otherwise
 */
static inline XStatus checkExclusionList(u64 RegionStartAddr, u64 RegionEndAddr,
							struct XPm_ExclusionRegion *XPm_ExcludedList, u32 ExcludedListSize) {
	volatile XStatus Status = XPM_FAILURE;
	volatile u32 index = 0U;

	for (index=0U; index<ExcludedListSize; index++) {
		u64 excludedStartAddress = XPm_ExcludedList[index].StartAddress;
		u64 excludedEndAddress = XPm_ExcludedList[index].EndAddress;
		u64 excludedSize = excludedEndAddress - excludedStartAddress + 1ULL;

		/* size for any exclusion region must be non-zero */
		if (0x0ULL == (excludedEndAddress - excludedStartAddress)) {
			continue;
		}

		volatile AddrRangeStatus isRangeOverlapExclusion = ADDR_NOT_IN_RANGE;
		volatile AddrRangeStatus isRangeOverlapExclusionTmp = ADDR_NOT_IN_RANGE;
		volatile AddrRangeStatus isExclusionInRange = ADDR_NOT_IN_RANGE;
		volatile AddrRangeStatus isExclusionInRangeTmp = ADDR_NOT_IN_RANGE;

		/* condition to check whether there is any overlap of given address range (address + size) with excluded range */
		/* redundancy in checker to address security concerns */
		XSECURE_REDUNDANT_CALL(isRangeOverlapExclusion, isRangeOverlapExclusionTmp, checkRangeOverlapExclusion,\
		RegionStartAddr, RegionEndAddr, excludedStartAddress, excludedEndAddress);
		if (isRangeOverlapExclusion != isRangeOverlapExclusionTmp) {
			Status = XPM_FAILURE;
			goto done;
		}

		/* condition to check whether entire excluded range falls in given address range (address + size) */
		/* redundancy in checker to address security concerns */
		/**
		 * NOTE: checkRangeOverlapExclusion above also handles all four overlap geometries
		 * directly; this second check is intentional redundancy for fault-injection protection.
		 */
		isExclusionInRange = checkAddrRangeContainment(&excludedStartAddress, &excludedSize,\
								RegionStartAddr, RegionEndAddr);
		/* restore the excluded range values ( as they might be modified by checkAddrRangeContainment()) */
		excludedStartAddress = XPm_ExcludedList[index].StartAddress;
		excludedSize = XPm_ExcludedList[index].EndAddress - XPm_ExcludedList[index].StartAddress + 1ULL;
		isExclusionInRangeTmp = checkAddrRangeContainment(&excludedStartAddress, &excludedSize,\
								RegionStartAddr, RegionEndAddr);
		if (isExclusionInRange != isExclusionInRangeTmp) {
			Status = XPM_FAILURE;
			goto done;
		}

		/* If there is any overlap of provided range and excluded range, not allowed to access! */
		/**
		 * Excluded Range: |--------------------|
		 * Ex Range1:  |-------| 			(given range overlaps excluded range partially at the start)
		 * Ex Range2:                       |-------| 	(given range overlaps excluded range partially at the end)
		 * Ex Range3:             |-------| 		(given range completely inside excluded range)
		 * Ex Range4: |-------------------------------| (given range fully contains excluded range)
		 */
		if ((ADDR_IN_RANGE == isRangeOverlapExclusion) || (ADDR_NOT_IN_RANGE != isExclusionInRange)) {
			PmInfo("Range falls in Exclusion List [%d]\r\n", index);
			Status = XPM_FAILURE;
			goto done;
		}
	}

	/* check whether entire loop was consumed or not ( to address security concerns ) */
	if (index != ExcludedListSize) {
		Status = XPM_FAILURE;
		goto done;
	}
	Status = XPM_SUCCESS;

done:
	return Status;
}

/**
 * @brief Checks whether a given address range is not in any of the exclusion regions
 * @param RegionAddr 	Range Start Address
 * @param RegionSize 	Range Size
 *
 * @note This application for Subsystem != PM_SUBSYS_PMC
 * @return ==XPM_FAILURE (if range in exclusion list), ==XPM_SUCCESS (if range not in exclusion list)
 *************************************************************************************/
static XStatus IsRangeOutsideExclusionList(u64 RegionAddr, u64 RegionSize) {
	volatile XStatus Status = XPM_FAILURE;
	volatile XStatus StatusTmp = XPM_FAILURE;
	u64 imgStoreRegionAddress = 0x0ULL;
	u64 imgStoreRegionSize    = 0x0ULL;

	if (RegionSize < 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	u64 RegionStartAddr = RegionAddr;
	u64 RegionEndAddr = RegionAddr + (RegionSize - 1ULL);

	enum ExclusionRegionList {
		PMC_RAM = 0x0,
		ASU_RAM = 0x1,
		PPU_INSTR_CNTLR_MEM = 0x2,
		PPU_DATA_CNTLR_MEM = 0x3,
		RTCA_IMAGE_STORE_REGN = 0x4
	};

	struct XPm_ExclusionRegion XPm_ExcludedList[] = {
		[PMC_RAM] = {XPLMI_PMCRAM_BASEADDR, (XPLMI_PMCRAM_BASEADDR + XPLMI_TOTAL_CHUNK_SIZE - 1ULL)},
		[ASU_RAM] = {XPLMI_ASU_RAM_BASE_ADDR, XPLMI_ASU_RAM_HIGH_ADDR},
		[PPU_INSTR_CNTLR_MEM] = {PPU_RAM_INSTR_CNTLR_BASEADDR, PPU_RAM_INSTR_CNTLR_HIGHADDR},
		[PPU_DATA_CNTLR_MEM] = {PPU_RAM_DATA_CNTLR_BASEADDR, PPU_RAM_DATA_CNTLR_HIGHADDR},
		[RTCA_IMAGE_STORE_REGN] = {0x0ULL, 0x0ULL}
	};
	/** extract RTCA Image Store Address Range
	 * - 32-bit higher store address
	 * - 32-bit lower store address
	 * - 32-bit size
	 *
	 * Compute range from user-defined Image store address and size
	*/
	imgStoreRegionAddress = (RTCA_IMAGE_STORE_ADDR_HIGH << 32U) | RTCA_IMAGE_STORE_ADDR_LOW;
	imgStoreRegionSize = (u64)(RTCA_IMAGE_STORE_ADDR_SIZE);
	/* only initialize array parameters if size is non-zero */
	if (imgStoreRegionSize >= 1U) {
		XPm_ExcludedList[RTCA_IMAGE_STORE_REGN].StartAddress = imgStoreRegionAddress;
		XPm_ExcludedList[RTCA_IMAGE_STORE_REGN].EndAddress = imgStoreRegionAddress + imgStoreRegionSize - 1ULL;

		/* check security concerns for address overflow */
		if (XPm_ExcludedList[RTCA_IMAGE_STORE_REGN].StartAddress > XPm_ExcludedList[RTCA_IMAGE_STORE_REGN].EndAddress) {
			PmErr("RTCA Image Store Address Range Overflow\r\n");
			Status = XST_INVALID_PARAM;
			goto done;
		}
	}

	/* redundancy iteration check to address security concerns */
	XSECURE_REDUNDANT_CALL(Status, StatusTmp, checkExclusionList, RegionStartAddr, RegionEndAddr, XPm_ExcludedList, ARRAY_SIZE(XPm_ExcludedList));
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		Status = XPM_FAILURE;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Checks whether given address range (addr + size) is read/write accessible
 * 		by a given subsystem id.
 *
 * @param  SubsystemId 	The Subsystem to check for given addr range
 * @param  RegionAddr 	Start Address of the range
 * @param  RegionSize 	Size of the range
 *
 * @return Status of the operation : XPM_INVALID_SUBSYSID, XST_INVALID_PARAM, XPM_FAILURE,
 * 				XPM_PM_NO_ACCESS, XPM_SUCCESS
 *
 ****************************************************************************/
XStatus XPm_IsMemAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize) {
	volatile XStatus Status = XPM_FAILURE;
	const XPm_MemDevice *MemDevice = NULL;
	const XPm_MemCtrlrDevice *MCDev = NULL;
	/* store the provided region address in a local variable */
	u64 CurrentRegionAddr = RegionAddr;
	/* store the provided region size in a local variable */
	u64 CurrentRegionSize = RegionSize;
	/* indicates whether there's an overlapping of region or not */
	u8 IsOverlapMemRegion = FALSE;
	/* indicates the type of memory overlap */
	u8 OverlapMemType = MEM_TYPE_NONE;
	u32 DeviceId;

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
	if ((INVALID_SUBSYSID == SubsystemId) ||
		((MAX_NUM_SUBSYSTEMS) <= NODEINDEX(SubsystemId))) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/**
	 * For any subsystem other than PMC, we must check exclusion list
	 * If range is in exclusion list, it's not allowed and we return
	 */
	Status = IsRangeOutsideExclusionList(RegionAddr, RegionSize);
	if (XPM_SUCCESS != Status) {
		if (PM_SUBSYS_PMC == SubsystemId) {
			/* If range is in exclusion list and caller is PMC_SUBSYS, then it's accessible */
			Status = XPM_SUCCESS;
		}
		goto done;
	}

	/**
	 * XPm_IsMemRegnAddressValid() for boot and runtime-eemi, check whether given subsystem is accessible during boot-time
	 * XPm_IsMemRegnAddressValid() for runtime-subsys, check whether given address range falls in mem-regn
	 * 	- For runtime, IsPLMem flag is used to also check PL Power Domain ( IsPLMem flag is don't care for boot & eemi )
	 */
	u8 IsPLMem = FALSE;
	Status = XPm_IsMemRegnAddressValid(SubsystemId, RegionAddr, RegionSize, &IsPLMem);
	if (XPM_SUCCESS != Status) {
		goto done;
	}
	/** For PL Mem-Regn, we have already checked the PLD Power Domain in XPm_IsMemRegnAddressValid()
	 *  Hence, for XPM_SUCCESS and IsPLMem, we can skip rest everything! ( as PL doesn't have parent node )
	*/
	if (IsPLMem) {
		goto done;
	}

	/** manually over-write the status to failure
	 *  this is for Non-PL Mem-Regns ( as we further check parent controller's state )
	*/
	Status = XPM_FAILURE;

	for (u32 idx = (u32)XPM_NODEIDX_DEV_MIN; idx <= (u32)XPM_NODEIDX_DEV_MAX; idx++) {
		const XPm_Device *Device = XPmDevice_GetByIndex(idx);
		if (NULL == Device) {
			continue;
		}
		DeviceId = Device->Node.Id;

		/** supported Mem-Regn Devices
		 * - OCM
		 * - TCM
		 * - DDR
		 *
		 * note: PL Mem-Regn is supported but it doesn't have parent device
		*/
		if (IS_MEM_DEV_OCM(DeviceId) ||
		    IS_MEM_DEV_TCM(DeviceId)) {
			MemDevice = (XPm_MemDevice *)Device;
			if (NULL == MemDevice) {
				continue;
			}

			u64 StartAddress = (u64)MemDevice->StartAddress;
			u64 EndAddress = (u64)MemDevice->EndAddress;
			AddrRangeStatus Range = checkAddrRangeContainment(&CurrentRegionAddr, &CurrentRegionSize, StartAddress, EndAddress);

			/* skip to next device */
			if (ADDR_NOT_IN_RANGE == Range) {
				continue;
			}

			/**
			 * skip iteration to failure for memory type mismatch during overlapping condition
			 * - only access to same memory type is allowed during overlapping condition
			 */
			if ((TRUE == IsOverlapMemRegion) && (MEM_TYPE_OCM_TCM != OverlapMemType)) {
				PmErr("Mismatch Memory type access!\r\n");
				Status = XPM_ERR_MEM_ACCESS_TYPE;
				goto done;
			}

			/**
			 * check power state of the given Memory Controller Device
			 */
			if ((u8)XPM_DEVSTATE_RUNNING == MemDevice->Device.Node.State) {
				Status = XPM_SUCCESS;
			}
			else {
				PmErr("Node [0x%x] not running...!\r\n", DeviceId);
				Status = XPM_PM_NO_ACCESS;
				/* if any part of the region is not accessible, we are done */
				goto done;
			}

			if (ADDR_IN_RANGE == Range) {
				/* entire region is validated, we are done */
				goto done;
			}
			/* for overlapping condition, we continue the iteration */
			IsOverlapMemRegion = TRUE;
			OverlapMemType = MEM_TYPE_OCM_TCM;
		}
		else if (IS_MEM_DEV_DDRMC(DeviceId)) {
			MCDev = (XPm_MemCtrlrDevice *)Device;
			if (NULL == MCDev) {
				continue;
			}

			/**
			 * Save remaining chunks of region
			 * This is needed to restore chunk region for interleaved controllers
			*/
			u64 savedIntlvAddr = CurrentRegionAddr;
			u64 savedIntlvSize = CurrentRegionSize;

			for (u32 Cnt = 0U; Cnt < MCDev->RegionCount; Cnt++) {
				u64 StartAddress = MCDev->Region[Cnt].Address;
				u64 RegionSz = MCDev->Region[Cnt].Size;
				/**
				 * Guard against zero-size DDR region
				 * Size=0 would underflow and produce an arbitrarily large address range
				 */
				if (RegionSz < 1U) {
					continue;
				}
				u64 EndAddress = StartAddress + RegionSz - 1U;
				AddrRangeStatus Range = checkAddrRangeContainment(&CurrentRegionAddr, &CurrentRegionSize, StartAddress, EndAddress);

				/* skip to next device */
				if (ADDR_NOT_IN_RANGE == Range) {
					continue;
				}

				/**
				 * skip iteration to failure for memory type mismatch during overlapping condition
				 * - only access to same memory type is allowed during overlapping condition
				 */
				if ((TRUE == IsOverlapMemRegion) && (MEM_TYPE_DDR != OverlapMemType)) {
					PmErr("Mismatch Memory type access!\r\n");
					Status = XPM_ERR_MEM_ACCESS_TYPE;
					goto done;
				}

				/**
				 * check power state of the given Memory Controller Device
				 */
				if ((u8)XPM_DEVSTATE_RUNNING == MCDev->Device.Node.State) {
					Status = XPM_SUCCESS;
				}
				else {
					PmErr("Node [0x%x] not running...!\r\n", DeviceId);
					Status = XPM_PM_NO_ACCESS;
					/* if any part of the region is not accessible, we are done */
					goto done;
				}

				if (ADDR_IN_RANGE == Range) {
					/*
					* If the address is valid and the memory controller is non-interleaved
					* or it's the last interleaved controller, then we are done checking everything
					*/
					if (MCDev->IntlvIndex <= 1U) {
						goto done;
					}
					/* interleaved: break inner loop, continue to paired controller */
					break;
				}

				/* for overlapping, continue to next region within same controller */
				IsOverlapMemRegion = TRUE;
				OverlapMemType = MEM_TYPE_DDR;
			}

			/**
			 * Restore the region until it's either:
			 * - Last interleaved controller
			 * - or it's non-interleaved controller with overlap
			 */
			if ((XPM_SUCCESS == Status) && (MCDev->IntlvIndex > 1U)) {
				/*
				* If the address is valid and the memory controller is interleaved,
				* then keep going to check for paired memory controller(s).
				*/
				/* restore the region condition for the next interleaved controller */
				CurrentRegionAddr = savedIntlvAddr;
				CurrentRegionSize = savedIntlvSize;
				/* reset memory overlap flag */
				IsOverlapMemRegion = FALSE;
			}
		}
		else {
			/* Required by MISRA */
		}
	}

	/**
	 * If we encountered overlapping region(s) then:
	 * We haven't exhausted all the memory chunks of provided Region
	 * in entire iteration, hence it's a failure!
	 */
	if (TRUE == IsOverlapMemRegion) {
		Status = XPM_FAILURE;
	}

done:
	return Status;
}
