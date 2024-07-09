/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_API_PLAT_H_
#define XPM_PSM_API_PLAT_H_

#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PMC subsystem uses LPD devices like IPI, PSM_PROC and UART0. When no one
 * using LPD devices, its minimum use count will be 3 as these devices are
 * requested by PMC subsystem.
 */
#define MIN_LPD_USE_COUNT			(3U)

enum ProcDeviceId {
	ACPU_0,
	ACPU_1,
	RPU0_0,
	RPU0_1,
	PROC_DEV_MAX,
};

XStatus XPm_CCIXEnEvent(u32 PowerId);
XStatus ReleaseDeviceLpd(void);
XStatus XPm_DirectPwrUp(const u32 DeviceId);
XStatus XPm_DirectPwrDwn(const u32 DeviceId);

#ifdef __cplusplus
}
#endif

#endif /* XPM_PSM_API_PLAT_H_ */
