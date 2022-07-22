/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_H_
#define XPM_MEM_H_

#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_NODEIDX_DEV_DDRMC_MIN	XPM_NODEIDX_DEV_DDRMC_0
#define XPM_NODEIDX_DEV_DDRMC_MAX	XPM_NODEIDX_DEV_DDRMC_3

/* Periodicity of HBM Stack Temperature Monitoring Task */
#define HBM_TEMP_MON_PERIOD		(250U)

typedef struct XPm_MemDevice XPm_MemDevice;
typedef struct XPm_MemCtrlrDevice XPm_MemCtrlrDevice;

/* Generic memory device with associated memory address */
struct XPm_MemDevice {
	XPm_Device Device;	/**< Device: Base class */
	u32 StartAddress;
	u32 EndAddress;
};

/* DDR Memory controller device */
struct XPm_MemCtrlrDevice {
	XPm_Device Device;	/**< Device: Base class */
	struct XPm_AddrRegion {
		u64 Address;
		u64 Size;
	} Region[2];		/**< DDRMC Address regions (max: 2) */
	struct XPm_PlDeviceNode *PlDevice;	/**< Parent PL device */
	u8 RegionCount;		/**< DDRMC Address range count */
	u8 IntlvSize;		/**< DDRMC Interleave size in bytes */
	u8 IntlvIndex;		/**< DDRMC Interleave order index */
};

/************************** Function Prototypes ******************************/
XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress);
XStatus XPmDDRDevice_IsInSelfRefresh(void);
XStatus XPmMem_HBMTempMonInitTask(void);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
