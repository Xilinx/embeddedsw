/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_update.h"
#include "xplmi_update.h"
#include "xpm_node.h"
#include "xpm_alloc.h"
#include "xpm_regs.h"
#include "xplmi_modules.h"
#include "xcframe.h"
#ifdef XILPM_RUNTIME
#include "xpm_subsystem.h"
#include "xpm_access.h"
#include "xpm_gic_proxy.h"
#include "xpm_ioctl.h"
#endif
#include "xpm_clock.h"
#include "xpm_memory_pools.h"

extern const u8 __xpm_init_data_start[];
extern const u8 __xpm_init_data_size[];
extern const u8 __xpm_topo_start[];
#ifdef XILPM_RUNTIME
extern const u8 __xpm_subsys_start[];
#endif
extern const u8 __xpm_board_bss_start[];

/** @brief Export .xpm_initialized_data section for IPU save/restore. */
EXPORT_DS(XPmInitData, \
		XPLMI_MODULE_XILPM_ID, XPM_DEVICETABLE_DS_ID, \
		XPM_TOPOLOGY_DS_VERSION, XPM_TOPOLOGY_DS_LCVERSION, \
		(u32)(UINTPTR)__xpm_init_data_size, \
		(u32)(UINTPTR)__xpm_init_data_start);

EXPORT_DS(XPmTopoData, \
	XPLMI_MODULE_XILPM_ID, XPM_TOPOLOGY_DS_ID, \
	XPM_TOPOLOGY_DS_VERSION, XPM_TOPOLOGY_DS_LCVERSION, \
	MAX_TOPO_POOL_SIZE,
	(u32)(UINTPTR)__xpm_topo_start);

#ifdef XILPM_RUNTIME
EXPORT_DS(XPmSubsysData, \
	XPLMI_MODULE_XILPM_ID, XPM_SUBSYSTEMPOOL_DS_ID, \
	XPM_SUBSYSTEMPOOL_DS_VERSION, XPM_SUBSYSTEMPOOL_DS_LCVERSION, \
	MAX_SUBSYS_POOL_SIZE+
	MAX_REQM_POOL_SIZE +
	MAX_DEVOPS_POOL_SIZE +
	MAX_OTHER_POOL_SIZE,
	(u32)(UINTPTR)__xpm_subsys_start);
#endif

EXPORT_DS(XPmBoardBssData, \
	XPLMI_MODULE_XILPM_ID, XPM_BOARD_POOL_DS_ID, \
	XPM_BOARD_POOL_DS_VERSION, XPM_BOARD_POOL_DS_LCVERSION, \
	MAX_BOARD_POOL_SIZE,
	(u32)(UINTPTR)__xpm_board_bss_start);
