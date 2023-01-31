/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_PLAT_H_
#define XPM_MEM_PLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: add correct definitions of these after topology cdo is updated */
#define XPM_NODEIDX_DEV_DDRMC_MIN	0
#define XPM_NODEIDX_DEV_DDRMC_MAX	0
#define IS_MEM_DEV_TYPE(Type)		(((u32)XPM_NODETYPE_DEV_OCM == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_DDR == (Type)) || \
					 ((u32)XPM_NODETYPE_DEV_TCM == (Type)))

typedef struct XPm_MemDevice XPm_MemDevice;

/************************** Function Prototypes ******************************/
maybe_unused static inline void XPm_AssignDdrFsm(XPm_MemDevice *MemDevice)
{
	(void)MemDevice;
	return;
}

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
