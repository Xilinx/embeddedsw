/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_API_PLAT_H_
#define XPM_PSM_API_PLAT_H_

#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ProcDeviceId {
	ACPU_0,
	ACPU_1,
	RPU0_0,
	RPU0_1,
	PROC_DEV_MAX,
};

XStatus XPm_CCIXEnEvent(u32 PowerId);
XStatus ReleaseDeviceLpd(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_PSM_API_PLAT_H_ */
