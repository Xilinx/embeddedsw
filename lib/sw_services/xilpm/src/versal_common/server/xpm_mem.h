/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_H_
#define XPM_MEM_H_

#include "xpm_device.h"
#include "xpm_mem_plat.h"
#include "xpm_requirement.h"
#include "cfu_apb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enable MEM-REGN Checking Feature */
#ifdef VERSAL_NET
#define XPM_ENABLE_MEM_REGN_CHECKING
#else
/**
 * By default, feature is disabled for versal architecture (to save space)
 */
// #define XPM_ENABLE_MEM_REGN_CHECKING
#endif

#define RTCA_IMAGE_STORE_ADDR_HIGH 	((u64)XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH))
#define RTCA_IMAGE_STORE_ADDR_LOW 	((u64)XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW))
#define RTCA_IMAGE_STORE_ADDR_SIZE 	(XPm_In32((u32)XPLMI_RTCFG_IMG_STORE_SIZE))

#if defined(XPM_ENABLE_MEM_REGN_CHECKING)
#define IS_BUILTIN_SUBSYSTEM(ID) 	(((u32)PM_SUBSYS_PMC == (ID)) || \
					 ((u32)PM_SUBSYS_DEFAULT == (ID)))
#define IS_PL_STARTUP_ASSERTED 		((XPlmi_In32(CFU_APB_CFU_FGCR) & CFU_APB_CFU_FGCR_EOS_MASK))
#endif

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

#define MEMREGN_DEVID(IDX)		NODEID((u32)XPM_NODECLASS_DEVICE, \
					(u32)XPM_NODESUBCL_DEV_MEM_REGN, \
					(u32)XPM_NODETYPE_DEV_MEM_REGN, (IDX))

#define PL_MEM_REGN			(0x1U)
#define PL_MEM_REGN_FLAGS_SHIFT_64	(60U)
#define PL_MEM_REGN_FLAGS_MASK_64	((u64)0xF0000000U << 32)
#define PL_MEM_REGN_FLAGS(SZ_64BIT)	((u32)(((SZ_64BIT) & PL_MEM_REGN_FLAGS_MASK_64) >> PL_MEM_REGN_FLAGS_SHIFT_64))
/* zero-ing out upper flag bits[31:28] from 64bit size */
#define PL_MEM_SIZE(SZ_64BIT)		((SZ_64BIT) & ~PL_MEM_REGN_FLAGS_MASK_64)
/** Check whether PL Flag is set or not
 * This can be done by reading [31:28] bits of upper 64bit value,
 * where if Bits[31:28] == 0x1, then it's a PL memory ( or non-PL Memory otherwise )
*/
#define IS_PL_MEM_REGN(SZ_64BIT)	((u32)PL_MEM_REGN == PL_MEM_REGN_FLAGS((u64)(SZ_64BIT)))

/* Macros with higher Hamming distance */
#define ADDR_IN_RANGE      		(0x3CU)
#define ADDR_NOT_IN_RANGE  		(0xC2U)
#define ADDR_RANGE_OVERLAP 		(0xC3U)

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
	u32 DdrMc_MainAddr; /**< DDRMC Main Address */
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
XStatus XPm_IsMemAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
