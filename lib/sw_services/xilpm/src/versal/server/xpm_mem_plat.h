/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_PLAT_H_
#define XPM_MEM_PLAT_H_

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

#define XPM_NODEIDX_DEV_DDRMC_MIN	XPM_NODEIDX_DEV_DDRMC_0
#define XPM_NODEIDX_DEV_DDRMC_MAX	XPM_NODEIDX_DEV_DDRMC_3

/* Periodicity of HBM Stack Temperature Monitoring Task */
#define HBM_TEMP_MON_PERIOD		(250U)

typedef struct XPm_MemDevice XPm_MemDevice;

/************************** Function Prototypes ******************************/
XStatus XPmDDRDevice_IsInSelfRefresh(void);
void XPm_AssignDdrFsm(XPm_MemDevice *MemDevice);
XStatus XPmMem_HBMTempMonInitTask(void);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_PLAT_H_ */
