/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_MEM_H
#define XPM_RUNTIME_MEM_H

#include "xpm_device.h"
#include "xstatus.h"
#include "xpm_err.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEMREGN_DEVID(IDX)		NODEID((u32)XPM_NODECLASS_DEVICE, \
					(u32)XPM_NODESUBCL_DEV_MEM_REGN, \
					(u32)XPM_NODETYPE_DEV_MEM_REGN, (IDX))

/* Maximum 64-bit unsigned value; used for address-range overflow guards */
#define U64_MAX_SIZE			(0xFFFFFFFFFFFFFFFFULL)

/* check whether given Subsystem Id is a built-in subsystem within PLM */
#define IS_BUILTIN_SUBSYSTEM(ID) 	(((u32)PM_SUBSYS_PMC == (ID)) || \
					((u32)PM_SUBSYS_ASU == (ID)) || \
					((u32)PM_SUBSYS_DEFAULT == (ID)))

#define RTCA_IMAGE_STORE_ADDR_HIGH 	((u64)XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH))
#define RTCA_IMAGE_STORE_ADDR_LOW 	((u64)XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW))
#define RTCA_IMAGE_STORE_ADDR_SIZE 	(XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_SIZE))


/* check whether given Node Id is a OCM memory device */
#define IS_MEM_DEV_OCM(ID)		(((u32)XPM_NODECLASS_DEVICE == NODECLASS(ID)) && \
					 ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(ID)) && \
					 ((u32)XPM_NODETYPE_DEV_OCM == NODETYPE(ID)))

/* check whether given Node Id is a TCM memory device */
#define IS_MEM_DEV_TCM(ID)		(((u32)XPM_NODECLASS_DEVICE == NODECLASS(ID)) && \
					 ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(ID)) && \
					 ((u32)XPM_NODETYPE_DEV_TCM == NODETYPE(ID)))

/* check whether given Node Id is a DDR memory controller device */
#define IS_MEM_DEV_DDRMC(ID)		(((u32)XPM_NODECLASS_DEVICE == NODECLASS(ID)) && \
					 ((u32)XPM_NODESUBCL_DEV_MEM_CTRLR == NODESUBCLASS(ID)) && \
					 ((u32)XPM_NODETYPE_DEV_DDR == NODETYPE(ID)))

typedef enum {
	MEM_TYPE_NONE 	 = 0x0U,
	MEM_TYPE_OCM_TCM = 0x1U,
	MEM_TYPE_DDR 	 = 0x2U,
	MEM_TYPE_PL 	 = 0x3U
} MemType;

/**
 * @enum AddrRangeStatus
 * @brief Status codes for address-range containment checks (inclusive bounds).
 *
 * Indicates whether the tested range is fully contained within a bounding range.
 * Used as the return type of checkAddrRangeContainment().
 *
 * Values:
 *  - ADDR_IN_RANGE      (0x3C): The tested range is fully contained.
 *  - ADDR_RANGE_OVERLAP (0xC2): The tested range partially overlaps.
 *  - ADDR_NOT_IN_RANGE  (0xC3): The tested range is not fully contained.
 *
 * Note: These are non-boolean sentinels; compare explicitly with the enumerators.
 */
typedef enum {
	ADDR_IN_RANGE		= 0x3CU,
	ADDR_RANGE_OVERLAP 	= 0xC2U,
	ADDR_NOT_IN_RANGE 	= 0xC3U
} AddrRangeStatus;

/************************** Static Functions ******************************/

/**
 * @brief  Checks whether given address range (addr + size) is contained
 * 		within specificed Memory Region or going beyond (for potential overlap)
 * @param  RegionStart 	Start Address of the Region to compare
 * @param  RegionSize 	Size of the Region to compare
 * @param  StartAddr 	Start Address of the Memory Range
 * @param  EndAddr 	End Address of the Memory Range
 *
 * @return ADDR_IN_RANGE if the address range is within the region,
 * 		ADDR_RANGE_OVERLAP if overlapping region (or ADDR_NOT_IN_RANGE otherwise)
 * @note   The assumption is that input address region is valid only if it
 * 		starts within a valid memory region.
 *************************************************************************************
 */
static inline AddrRangeStatus checkAddrRangeContainment(u64 *RegionStart, u64 *RegionSize, u64 StartAddr, u64 EndAddr) {
	volatile AddrRangeStatus Range = ADDR_NOT_IN_RANGE;
	u64 RegionEnd = *RegionStart + *RegionSize - 1ULL;

	/**
	 * Input:	   |==============|
	 *             RegionStart       RegionEnd
	 * Memory: 	|-----------------------|
	 *           StartAddr               EndAddr
	 */
	if (((*RegionStart >= StartAddr) && (*RegionStart <= EndAddr)) &&
	((RegionEnd >= StartAddr) && (RegionEnd <= EndAddr))) {
		Range = ADDR_IN_RANGE;
	}
	/**
	 * Input:	   |============================|
	 *             RegionStart                   RegionEnd
	 * Memory: 	|-----------------------|
	 *           StartAddr               EndAddr
	 */
	else if (((*RegionStart >= StartAddr) && (*RegionStart <= EndAddr)) &&
	(RegionEnd > EndAddr)) {
		Range = ADDR_RANGE_OVERLAP;
		/* calculate the remaining overlapped region */
		*RegionStart = EndAddr + 1ULL;
		*RegionSize = RegionEnd - EndAddr;
	}
	else {
		Range = ADDR_NOT_IN_RANGE;
	}

	return Range;
}

XStatus XPm_IsMemRegnAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize, u8 *IsPLMem);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_MEM_H */
