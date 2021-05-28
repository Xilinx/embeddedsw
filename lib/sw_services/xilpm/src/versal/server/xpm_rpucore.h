/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_RPUCORE_H_
#define XPM_RPUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_PROC_RPU_HIVEC_ADDR		0xFFFC0000U
#define XPM_RPU_NCPUHALT_MASK		BIT(0)
#define XPM_RPU_VINITHI_MASK		BIT(2)
#define XPM_RPU_SLSPLIT_MASK		BIT(3)
#define XPM_RPU_TCM_COMB_MASK		BIT(6)
#define XPM_RPU_SLCLAMP_MASK		BIT(4)
#define XPM_RPU0_0_PWR_CTRL_MASK	BIT(4)
#define XPM_RPU0_1_PWR_CTRL_MASK	BIT(5)
#define XPM_RPU_0_CPUPWRDWNREQ_MASK  BIT(0)
#define XPM_RPU_1_CPUPWRDWNREQ_MASK  BIT(0)

typedef struct XPm_RpuCore XPm_RpuCore;

/**
 * The RPU core class.
 */
struct XPm_RpuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 ResumeCfg;
	u32 RpuBaseAddr; /**< Base address of RPU module */
};

/************************** Function Prototypes ******************************/
XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset);

void XPm_RpuGetOperMode(const u32 DeviceId, u32 *Mode);
void XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode);
XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr);
XStatus XPm_RpuTcmCombConfig(const u32 DeviceId, const u32 Config);
XStatus XPmRpuCore_Halt(const XPm_Device *Device);
XStatus XPm_RpuRstComparators(const u32 DeviceId);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RPUCORE_H_ */
