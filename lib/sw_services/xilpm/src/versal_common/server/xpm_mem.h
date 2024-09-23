/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_H_
#define XPM_MEM_H_

#include "xpm_device.h"
#include "xpm_mem_plat.h"
#include "xpm_requirement.h"

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

#define MEMREGN_DEVID(IDX)		NODEID((u32)XPM_NODECLASS_DEVICE, \
					(u32)XPM_NODESUBCL_DEV_MEM_REGN, \
					(u32)XPM_NODETYPE_DEV_MEM_REGN, (IDX))

typedef struct XPm_MemCtrlrDevice XPm_MemCtrlrDevice;
typedef struct XPm_MemRegnDevice XPm_MemRegnDevice;
typedef struct XPm_AddrRegion XPm_AddrRegion;

/* Generic memory device with associated memory address */
struct XPm_MemDevice {
	XPm_Device Device;	/**< Device: Base class */
	u32 StartAddress;
	u32 EndAddress;
	SAVE_REGION()
};
struct XPm_AddrRegion {
	u64 Address;
	u64 Size;
};
/* DDR Memory regions device */
struct XPm_MemRegnDevice {
    XPm_Device Device;	/**< Device: Base class */
SAVE_REGION (
    XPm_AddrRegion AddrRegion;	/**< Memory regions */
)
};

/* DDR Memory controller device */
struct XPm_MemCtrlrDevice {
	XPm_Device Device;	/**< Device: Base class */
	SAVE_REGION(
	struct XPm_AddrRegion Region[2];/**< DDRMC Address regions (max: 2) */
	u8 RegionCount;		/**< DDRMC Address range count */
	u8 IntlvSize;		/**< DDRMC Interleave size in bytes */
	u8 IntlvIndex;		/**< DDRMC Interleave order index */
	)
	struct XPm_PlDeviceNode *PlDevice;	/**< Parent PL device */
};

/************************** Function Prototypes ******************************/
XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress);
XStatus XPm_AddMemRegnDevice(u32 DeviceId, u64 Address, u64 Size);
XStatus HaltRpuCore(const XPm_Device *Rpu0, const XPm_Device *Rpu1,
			   const u32 Id, u32 *RpuMode);
XStatus XPm_GetRpuDevice(const XPm_Device **Rpu0Device,const XPm_Device **Rpu1Device,
				const u32 Id);
u32 XPm_CombTcm(const u32 Id, const u32 Mode);
XStatus XPm_GetAddrRegnForSubsystem(const u32 SubsystemId,
				    XPm_AddrRegion *AddrRegnArray,
				    u32 AddrRegnArrayLen, u32 *NumOfRegions);
XStatus XPm_IsAddressInSubsystem(const u32 SubsystemId, u64 AddressofSubsystem,
				 u8 *IsValidAddress);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
