/******************************************************************************
* Copyright (c) 2024 -2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RPUCORE_H_
#define XPM_RPUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif
#define XPM_CLUSTER_CFG_OFFSET		(0x0U)
#define XPM_RPU_SLSPLIT_MASK		BIT32(0)
#define XPM_CORE_CFG0_OFFSET		(0x0U)
#define XPM_RPU_TCMBOOT_MASK		BIT32(4)
#define XPM_CORE_VECTABLE_OFFSET	(0x10U)
#define XPM_RPU_CPUHALT_MASK	BIT(0)
#define XPM_RPU_CORE_HALT(ResumeCfg)		PmRmw32(ResumeCfg, XPM_RPU_CPUHALT_MASK,\
							XPM_RPU_CPUHALT_MASK)
#define XPM_RPU_CORE_RUN(ResumeCfg)		PmRmw32(ResumeCfg, XPM_RPU_CPUHALT_MASK,\
							~XPM_RPU_CPUHALT_MASK)
typedef struct XPm_RpuCore XPm_RpuCore;
/**
 * The RPU core class.
 */
struct XPm_RpuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 ResumeCfg;
	u32 RpuBaseAddr; /**< Base address of RPU module */
	u32 ClusterBaseAddr; /**< Base address of RPU module */
	u32 PcilIsr; /*pcil isr register*/
};


/************************** Function Prototypes ******************************/
XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset);

XStatus XPm_RpuGetOperMode(const u32 DeviceId, u32 *Mode);
XStatus XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode);
XStatus XPmRpuCore_ResetAndHalt(u32 CoreId);
void XPmRpuCore_AssignRegAddr(struct XPm_RpuCore *RpuCore, const u32 Id, const u32 *BaseAddress);
XStatus XPm_PlatRpuSetOperMode(const struct XPm_RpuCore *RpuCore, const u32 Mode, u32 *Val);
XStatus XPm_PlatRpuBootAddrConfig(const struct XPm_RpuCore *RpuCore, const u32 BootAddr);
u32 XPm_PlatRpuGetOperMode(const struct XPm_RpuCore *RpuCore);
void XPm_GetCoreId(u32 *Rpu0, u32 *Rpu1, const u32 DeviceId);
void XPmRpuCore_SetTcmBoot(const u32 DeviceId, const u8 TcmBootFlag);
XStatus XPmRpuCore_ReleaseReset(u32 CoreId);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RPUCORE_H_ */
