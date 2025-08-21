/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_update.h"
#include "xplmi_update.h"
#include "xpm_node.h"
#include "xpm_alloc.h"
#ifdef XILPM_RUNTIME
#include "xpm_subsystem.h"
#endif
#include "xpm_clock.h"
#include "xpm_memory_pools.h"

extern u8 __xpm_init_data_start[];
extern u8 __xpm_topo_start[];
extern u8 __xpm_subsys_start[];
extern u8 __xpm_board_bss_start[];

EXPORT_DS(XPmInitData, \
	XPLMI_MODULE_XILPM_ID, XPM_DEVICETABLE_DS_ID, \
	XPM_TOPOLOGY_DS_VERSION, XPM_TOPOLOGY_DS_LCVERSION, \
	XPM_NODEIDX_DEV_MAX * sizeof(void*) + sizeof(u32) +
	XPM_NODEIDX_DEV_PLD_MAX * sizeof(void*) + sizeof(u32) +
	XPM_NODEIDX_DEV_AIE_MAX * sizeof(void*) + sizeof(u32) +
	XPM_NODEIDX_DEV_VIRT_MAX * sizeof(void*) + sizeof(u32) +
	XPM_NODEIDX_DEV_HB_MON_MAX * sizeof(void*) + sizeof(u32) +
	XPM_NODEIDX_DEV_MEM_REGN_MAX * sizeof(void*) + sizeof(u32) +
	XPM_NODEIDX_MONITOR_MAX * sizeof(void*) +
	XPM_NODEIDX_ISO_MAX * sizeof(void*) +
	XPM_NODEIDX_STMIC_MAX * sizeof(void*) + sizeof(u32) +
	XPM_NODEIDX_POWER_MAX * sizeof(void*) + sizeof(u32) +
	XPM_NODEIDX_RST_MAX * sizeof(void*) + sizeof(u32) +
	5 * sizeof(XPm_ClkTopology) +
#ifdef XILPM_RUNTIME
	sizeof(XPm_SubsystemMgr) +
	sizeof(void*) + /* XPmRuntime_DeviceOpsList */
	sizeof(void*) + /* XPm_RegNode* */
	sizeof(void*) +/* XPmRuntime_ResetList* */
#endif
	XPM_NODEIDX_CLK_MAX * sizeof(void*) + sizeof(u32) +
    6 * sizeof(XPm_AllocablePool_t), \
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