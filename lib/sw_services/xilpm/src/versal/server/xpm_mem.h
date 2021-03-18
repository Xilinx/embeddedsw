/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_MEM_H_
#define XPM_MEM_H_

#include "xpm_node.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_OCM_MEM_REGN_TYPE(id)	((u32)XPM_NODETYPE_DEV_OCM_REGN == NODETYPE(id))
#define IS_DDR_MEM_REGN_TYPE(id)	((u32)XPM_NODETYPE_DEV_DDR_REGN == NODETYPE(id))

#define IS_MEM_REGN_TYPE(id)		(IS_OCM_MEM_REGN_TYPE(id) || IS_DDR_MEM_REGN_TYPE(id))

/**
 * IS_MEM_REGN - Returns true if given Node Id is of memory region node.
 */
#define IS_MEM_REGN(id)			(((u32)XPM_NODECLASS_DEVICE == NODECLASS(id)) && \
					 ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(id)) && \
					 (IS_MEM_REGN_TYPE(id)))

typedef struct XPm_MemDevice {
	XPm_Device Device; /**< Device: Base class */
	u32 StartAddress;
	u32 EndAddress;
} XPm_MemDevice;

/************************** Function Prototypes ******************************/
XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress);
XStatus XPmDDRDevice_IsInSelfRefresh(void);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
