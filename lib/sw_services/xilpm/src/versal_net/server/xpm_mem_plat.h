/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_PLAT_H_
#define XPM_MEM_PLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

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
