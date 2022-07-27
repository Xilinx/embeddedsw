/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEM_PLAT_H_
#define XPM_MEM_PLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IS_MEM_DEV_TYPE(Type)		(((u32)XPM_NODETYPE_DEV_OCM == Type) || \
					 ((u32)XPM_NODETYPE_DEV_DDR == Type) || \
					 ((u32)XPM_NODETYPE_DEV_TCM == Type))

typedef struct XPm_MemDevice XPm_MemDevice;

/************************** Function Prototypes ******************************/
maybe_unused static inline void XPm_AssignDdrFsm(XPm_MemDevice *MemDevice)
{
	(void)MemDevice;
	return;
}
maybe_unused static inline void XPm_AssignTcmFsm(XPm_MemDevice *MemDevice)
{
	(void)MemDevice;
	return;
}

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
