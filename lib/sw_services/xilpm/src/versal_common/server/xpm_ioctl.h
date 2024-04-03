/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_IOCTL_H_
#define XPM_IOCTL_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"
#include "xpm_subsystem.h"
#include "xpm_ioctl_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GGS_REGS	(4U)

#define PMC_PGGS_REGS	(2U)

/* Tap delay bypass */
#define TAPDLY_BYPASS_OFFSET			(0x0000003CU)
#define XPM_TAP_DELAY_MASK			(0x4U)

/* SD DLL control */
#define SD0_DLL_CTRL_OFFSET			(0x00000448U)
#define SD1_DLL_CTRL_OFFSET			(0x000004C8U)
#define XPM_SD_DLL_RST_MASK			(0x4U)

/* SD ITAPDLY */
#define ITAPDLY_OFFSET				(0x0000F0F8U)
#define XPM_SD_ITAPCHGWIN_MASK			(0x200U)
#define XPM_SD_ITAPDLYENA_MASK			(0x100U)
#define XPM_SD_ITAPDLYSEL_MASK			(0xFFU)

/* SD OTAPDLY */
#define OTAPDLY_OFFSET				(0x0000F0FCU)
#define XPM_SD_OTAPDLYENA_MASK			(0x40U)
#define XPM_SD_OTAPDLYSEL_MASK			(0x3FU)

/* Permissions related macros */
#define IOCTL_PERM_READ_SHIFT_NS		(0U)
#define IOCTL_PERM_WRITE_SHIFT_NS		(1U)
#define IOCTL_PERM_READ_IDX			(IOCTL_PERM_READ_SHIFT_NS)
#define IOCTL_PERM_WRITE_IDX			(IOCTL_PERM_WRITE_SHIFT_NS)

#define IOCTL_PERM_READ_SHIFT_S			(IOCTL_PERM_READ_SHIFT_NS + MAX_NUM_SUBSYSTEMS)
#define IOCTL_PERM_WRITE_SHIFT_S		(IOCTL_PERM_WRITE_SHIFT_NS + MAX_NUM_SUBSYSTEMS)

#define GGS_MAX					(XPM_NODEIDX_DEV_GGS_3)


XStatus XPm_Ioctl(const u32 SubsystemId, const u32 DeviceId, const pm_ioctl_id IoctlId,
	      const u32 Arg1, const u32 Arg2, const u32 Arg3,
	      u32 *const Response, const u32 CmdType);
XStatus XPmIoctl_AddRegPermission(const XPm_Subsystem *Subsystem, u32 DeviceId,
				  u32 Operations);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_IOCTL_H_ */
