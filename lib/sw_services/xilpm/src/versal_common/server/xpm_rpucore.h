/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_RPUCORE_H_
#define XPM_RPUCORE_H_

#include "xpm_core.h"
#include "xpm_rpucore_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The RPU core class.
 */
struct XPm_RpuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 ResumeCfg;
	u32 RpuBaseAddr; /**< Base address of RPU module */
	u32 ClusterBaseAddr; /**< Base address of RPU module */
	u32 PcilIsr; /*pcil isr register*/
	SAVE_REGION()
};

/************************** Function Prototypes ******************************/
XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset);

XStatus XPm_RpuGetOperMode(const u32 DeviceId, u32 *Mode);
XStatus XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode);
XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr);
XStatus XPmRpuCore_Halt(const XPm_Device *Device);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RPUCORE_H_ */
