/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RPUCORE_PLAT_H_
#define XPM_RPUCORE_PLAT_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_CLUSTER_CFG_OFFSET		(0x0U)
#define XPM_RPU_SLSPLIT_MASK		BIT32(0)
#define XPM_CORE_CFG0_OFFSET		(0x0U)
#define XPM_RPU_TCMBOOT_MASK		BIT32(4)
#define XPM_CORE_VECTABLE_OFFSET	(0x10U)
#define XPM_RPU_CORE_HALT(ResumeCfg)		PmRmw32(ResumeCfg, XPM_RPU_CPUHALT_MASK,\
							XPM_RPU_CPUHALT_MASK)
#define XPM_RPU_CORE_RUN(ResumeCfg)		PmRmw32(ResumeCfg, XPM_RPU_CPUHALT_MASK,\
							~XPM_RPU_CPUHALT_MASK)

typedef struct XPm_RpuCore XPm_RpuCore;

/************************** Function Prototypes ******************************/
void XPmRpuCore_AssignRegAddr(struct XPm_RpuCore *RpuCore, const u32 Id, const u32 *BaseAddress);
XStatus XPm_PlatRpuSetOperMode(const struct XPm_RpuCore *RpuCore, const u32 Mode, u32 *Val);
XStatus XPm_PlatRpuBootAddrConfig(const struct XPm_RpuCore *RpuCore, const u32 BootAddr);
u32 XPm_PlatRpuGetOperMode(const struct XPm_RpuCore *RpuCore);
void XPm_GetCoreId(u32 *Rpu0, u32 *Rpu1, const u32 DeviceId);
void XPmRpuCore_SetTcmBoot(const u32 DeviceId, const u8 TcmBootFlag);
XStatus XPm_PlatRpucoreHalt(XPm_Core *Core);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RPUCORE_PLAT_H_ */
