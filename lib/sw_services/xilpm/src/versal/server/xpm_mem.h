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
