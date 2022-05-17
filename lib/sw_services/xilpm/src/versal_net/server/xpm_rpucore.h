/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_RPUCORE_H_
#define XPM_RPUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_RPU_A_0_PWR_CTRL_MASK	BIT(20)
#define XPM_RPU_A_1_PWR_CTRL_MASK	BIT(21)
#define XPM_RPU_B_0_PWR_CTRL_MASK	BIT(22)
#define XPM_RPU_B_1_PWR_CTRL_MASK	BIT(23)
#define XPM_RPU_A_0_WAKEUP_MASK		BIT(2)
#define XPM_RPU_A_1_WAKEUP_MASK		BIT(3)
#define XPM_RPU_B_0_WAKEUP_MASK		BIT(4)
#define XPM_RPU_B_1_WAKEUP_MASK		BIT(5)
#define XPM_CLUSTER_CFG_OFFSET		(0x0U)
#define XPM_RPU_SLSPLIT_MASK		BIT(0)
#define XPM_CORE_CFG0_OFFSET		(0x0U)
#define XPM_RPU_TCMBOOT_MASK		BIT(4)
#define XPM_CORE_VECTABLE_OFFSET	(0x10U)

typedef struct XPm_RpuCore XPm_RpuCore;

/**
 * The RPU core class.
 */
struct XPm_RpuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 ResumeCfg;
	u32 RpuBaseAddr; /**< Base address of RPU module */
	u32 ClusterBaseAddr; /**< Base address of RPU cluster */
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
