/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_H_
#define XPM_MEM_H_

#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_MEM_DEV_TYPE(Type)		(((u32)XPM_NODETYPE_DEV_OCM == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_XRAM == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_L2CACHE == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_DDR == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_TCM == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_EFUSE == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_HBM == (Type)))

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

/* DDR Memory controller device */
struct XPm_MemCtrlrDevice {
	XPm_Device Device;	/**< Device: Base class */
	struct XPm_AddrRegion Region[2];/**< DDRMC Address regions (max: 2) */
	u8 RegionCount;		/**< DDRMC Address range count */
	u8 IntlvSize;		/**< DDRMC Interleave size in bytes */
	u8 IntlvIndex;		/**< DDRMC Interleave order index */
	struct XPm_PlDeviceNode *PlDevice;	/**< Parent PL device */
};

/************************** Function Prototypes ******************************/
XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
