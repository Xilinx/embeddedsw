/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_H_
#define XPM_MEM_H_

#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTCA_IMAGE_STORE_ADDR_HIGH 	((u64)XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH))
#define RTCA_IMAGE_STORE_ADDR_LOW 	((u64)XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW))
#define RTCA_IMAGE_STORE_ADDR_SIZE 	(XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_SIZE))

#define IS_BUILTIN_SUBSYSTEM(ID) 	(((u32)PM_SUBSYS_DEFAULT == (ID)) || \
					 ((u32)PM_SUBSYS_PMC == (ID)) || \
					 ((u32)PM_SUBSYS_ASU == (ID)))

#define IS_MEM_DEV_TYPE(Type)		(((u32)XPM_NODETYPE_DEV_OCM == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_XRAM == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_L2CACHE == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_DDR == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_TCM == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_EFUSE == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_HBM == (Type)))

#define IS_MEM_DEV_OCM(ID)		(((u32)XPM_NODECLASS_DEVICE == NODECLASS(ID)) && \
					 ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(ID)) && \
					 ((u32)XPM_NODETYPE_DEV_OCM == NODETYPE(ID)))

#define IS_MEM_DEV_TCM(ID)		(((u32)XPM_NODECLASS_DEVICE == NODECLASS(ID)) && \
					 ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(ID)) && \
					 ((u32)XPM_NODETYPE_DEV_TCM == NODETYPE(ID)))

#define IS_MEM_DEV_DDRMC(ID)		(((u32)XPM_NODECLASS_DEVICE == NODECLASS(ID)) && \
					 ((u32)XPM_NODESUBCL_DEV_MEM_CTRLR == NODESUBCLASS(ID)) && \
					 ((u32)XPM_NODETYPE_DEV_DDR == NODETYPE(ID)))

/* TODO: add correct definitions of these after topology cdo is updated */
#define XPM_NODEIDX_DEV_DDRMC_MIN_INT_1	XPM_NODEIDX_DEV_DDRMC_0
#define XPM_NODEIDX_DEV_DDRMC_MAX_INT_1	XPM_NODEIDX_DEV_DDRMC_3
#define XPM_NODEIDX_DEV_DDRMC_MIN_INT_2	XPM_NODEIDX_DEV_DDRMC_4
#define XPM_NODEIDX_DEV_DDRMC_MAX_INT_2	XPM_NODEIDX_DEV_DDRMC_7

#define XPM_NODEIDX_DEV_DDRMC_MIN	XPM_NODEIDX_DEV_DDRMC_MIN_INT_1
#define XPM_NODEIDX_DEV_DDRMC_MAX	XPM_NODEIDX_DEV_DDRMC_MAX_INT_2

/* Total count of DDRMCs: update this if above macros change */
#define MAX_PLAT_DDRMC_COUNT		\
	((XPM_NODEIDX_DEV_DDRMC_MAX_INT_1 - XPM_NODEIDX_DEV_DDRMC_MIN_INT_1 + 1) +\
	 (XPM_NODEIDX_DEV_DDRMC_MAX_INT_2 - XPM_NODEIDX_DEV_DDRMC_MIN_INT_2 + 1))
typedef struct XPm_MemCtrlrDevice XPm_MemCtrlrDevice;
typedef struct XPm_MemRegnDevice XPm_MemRegnDevice;
typedef struct XPm_AddrRegion XPm_AddrRegion;
typedef struct XPm_MemDevice XPm_MemDevice;
/* Generic memory device with associated memory address */
struct XPm_MemDevice {
	XPm_Device Device;	/**< Device: Base class */
	u32 StartAddress;
	u32 EndAddress;
};
struct XPm_AddrRegion {
	u64 Address;
	u64 Size;
};
/* DDR Memory regions device */
struct XPm_MemRegnDevice {
    XPm_Device Device;	/**< Device: Base class */
	XPm_AddrRegion AddrRegion;	/**< Memory regions */
};

/* DDR Memory controller device */
struct XPm_MemCtrlrDevice {
	XPm_Device Device;	/**< Device: Base class */
	struct XPm_AddrRegion Region[2];/**< DDRMC Address regions (max: 2) */
	u8 RegionCount;		/**< DDRMC Address range count */
	u8 IntlvSize;		/**< DDRMC Interleave size in bytes */
	u8 IntlvIndex;		/**< DDRMC Interleave order index */
	struct XPm_PlDeviceNode *PlDevice;	/**< Parent PL device */
};

/**
 * @enum AddrRangeStatus
 * @brief Status codes for address-range containment checks (inclusive bounds).
 *
 * Indicates whether the tested range is fully contained within a bounding range.
 * Used as the return type of IsAddrWithinRange().
 *
 * Values:
 *  - ADDR_IN_RANGE     (0x3C): The tested range is fully contained.
 *  - ADDR_OUT_OF_RANGE (0xC3): The tested range is not fully contained.
 *
 * Note: These are non-boolean sentinels; compare explicitly with the enumerators.
 */
typedef enum {
	ADDR_IN_RANGE		= 0x3CU,
	ADDR_OUT_OF_RANGE 	= 0xC3U
} AddrRangeStatus;

/************************** Static Inline Functions ******************************/

/**
 * @brief  Checks whether region [RegionStart, RegionEnd] is fully contained
 *		within range [StartAddr, EndAddr] (inclusive).
 * @param	RegionStart  Start address of the region to test
 * @param	RegionEnd    End address of the region to test
 * @param	StartAddr    Start address of the bounding range
 * @param	EndAddr      End address of the bounding range
 * @return	ADDR_IN_RANGE if contained, otherwise ADDR_OUT_OF_RANGE
 * @note	Caller must ensure RegionStart <= RegionEnd and StartAddr <= EndAddr.
 */
inline AddrRangeStatus IsAddrWithinRange(u64 RegionStart, u64 RegionEnd, u64 StartAddr, u64 EndAddr)

{
	return ((RegionStart >= StartAddr) && (RegionStart <= EndAddr) &&
			(RegionEnd >= StartAddr) && (RegionEnd <= EndAddr))
			? ADDR_IN_RANGE : ADDR_OUT_OF_RANGE;
}


/************************** Function Prototypes ******************************/
XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress);
XStatus XPm_AddMemRegnDevice(u32 DeviceId, u64 Address, u64 Size);
XStatus XPm_IsMemRegnAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize, u8 *IsPLMem);
XStatus XPm_IsMemAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
