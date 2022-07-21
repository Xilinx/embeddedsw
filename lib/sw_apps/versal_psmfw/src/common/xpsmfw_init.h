/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_init.h
*
* This file contains PSM Firmware initialization functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_INIT_H
#define XPSMFW_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xpsmfw_plat.h"
#include "xpsmfw_debug.h"
#include "xil_io.h"
#include "xpsmfw_ipi_manager.h"

#define PMC_TAP_BASEADDR      			(0XF11A0000U)
#define PMC_TAP_VERSION    			((PMC_TAP_BASEADDR) + 0X00000004U)
#define PMC_TAP_VERSION_PLATFORM_SHIFT   	(24)
#define PMC_TAP_VERSION_PLATFORM_MASK    	(0X0F000000U)
#define PMC_TAP_IDCODE				((PMC_TAP_BASEADDR) + 0X00000000U)

#define PLATFORM_VERSION_SILICON		(0x0U)
#define PLATFORM_VERSION_SPP			(0x1U)
#define PLATFORM_VERSION_EMU			(0x2U)
#define PLATFORM_VERSION_QEMU			(0x3U)

#define PMC_TAP_IDCODE_SBFMLY_MASK		(0x001C0000U)
#define PMC_TAP_IDCODE_DEV_MASK			(0x0003F000U)
#define PMC_TAP_IDCODE_DEV_SBFMLY_MASK		(PMC_TAP_IDCODE_SBFMLY_MASK | PMC_TAP_IDCODE_DEV_MASK)

#define IDCODE_SBFMLY_S				(0x00080000U)
#define IDCODE_DEV_VC1902			(0x00028000U)
#define IDCODE_DEV_VM1802			(0x0002A000U)
#define IDCODE_DEV_SBFMLY_VC1902		(IDCODE_SBFMLY_S | IDCODE_DEV_VC1902)
#define IDCODE_DEV_SBFMLY_VM1802		(IDCODE_SBFMLY_S | IDCODE_DEV_VM1802)

u32 XPsmFw_GetPlatform(void);
u32 XPsmFw_GetIdCode(void);
XStatus XPsmFw_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_INIT_H_ */
