/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
extern const u8 __xpm_topo_start[];
#ifdef XILPM_RUNTIME
extern const u8 __xpm_subsys_start[];
#endif
extern const u8 __xpm_board_bss_start[];

#ifdef XILPM_RUNTIME
#define XPM_RUNTIME_EXTRA_SIZE ( \
	/* SubsysMgr */ \
	sizeof(XPm_SubsystemMgr) + \
	/* DevOpsList (XPmRuntime_DeviceOpsList) */ \
	sizeof(void*) + \
	/* PmRegnodes (XPm_RegNode*) */ \
	sizeof(void*) + \
	/* RuntimeResetList (XPmRuntime_ResetList*) */ \
	sizeof(void*) + \
	/* PmNodeAccessTable (XPm_NodeAccess) */ \
	(sizeof(void*) * XPM_NODE_ACCESS_TABLE_SIZE) + \
	/* RuntimeCoreList */ \
	sizeof(void*) + \
	/* PeriphList */ \
	sizeof(void*) + \
	/* XPm_GicProxyGroups[5] */ \
	(5U * sizeof(XPm_GicProxyGroup)) + \
	/* EventSeq[20] */ \
	(20U * sizeof(u8)) + \
	/* PendingEvent */ \
	sizeof(u32) + \
	/* PosEmptySpace */ \
	sizeof(u32) + \
	/* SchedulerTask */ \
	sizeof(u32) + \
	/* PmNotifiers[20] (7 u32 fields each) */ \
	(20U * 7U * sizeof(u32)))
#else
#define XPM_RUNTIME_EXTRA_SIZE (0U)
#endif

/* Base + boot extras broken out for clarity/alignment */
#define XPM_INITDATA_BASE_SIZE ( \
		XPM_NODEIDX_DEV_MAX * sizeof(void*) + sizeof(u32) + \
		XPM_NODEIDX_DEV_PLD_MAX * sizeof(void*) + sizeof(u32) + \
		XPM_NODEIDX_DEV_AIE_MAX * sizeof(void*) + sizeof(u32) + \
		XPM_NODEIDX_DEV_VIRT_MAX * sizeof(void*) + sizeof(u32) + \
		XPM_NODEIDX_DEV_HB_MON_MAX * sizeof(void*) + sizeof(u32) + \
		XPM_NODEIDX_DEV_MEM_REGN_MAX * sizeof(void*) + sizeof(u32) + \
		XPM_NODEIDX_MONITOR_MAX * sizeof(void*) + \
		XPM_NODEIDX_ISO_MAX * sizeof(void*) + \
		XPM_NODEIDX_STMIC_MAX * sizeof(void*) + sizeof(u32) + \
		XPM_NODEIDX_POWER_MAX * sizeof(void*) + sizeof(u32) + \
		XPM_NODEIDX_RST_MAX * sizeof(void*) + sizeof(u32) + \
		5 * sizeof(XPm_ClkTopology) + \
		/* Clock node list + metadata */ \
		XPM_NODEIDX_CLK_MAX * sizeof(void*) + sizeof(u32) + \
		/* Allocable pool descriptors (Topo/Subsys/Reqm/DevOps/Other/Board) */ \
		(6U * sizeof(XPm_AllocablePool_t)) )

#define XPM_BOOT_EXTRA_SIZE ( \
		/* ApuClusterState[4] */ \
		(4U * sizeof(u8)) + \
		/* CframeIns */ \
		sizeof(XCframe) + \
		/* SavedCfuDivider */ \
		sizeof(u32) + \
		/* PmRegulators pointer table */ \
		(XPM_NODEIDX_POWER_REGULATOR_MAX * sizeof(void*)) + \
		/* XPlmi_PmCmds */ \
		(PM_API_MAX * sizeof(XPlmi_ModuleCmd)) + \
		/* XPlmi_PmAccessPermBuff */ \
		(PM_API_MAX * sizeof(XPlmi_AccessPerm_t)) )

#define XPM_INITDATA_TOTAL_SIZE ( XPM_INITDATA_BASE_SIZE + XPM_BOOT_EXTRA_SIZE + XPM_RUNTIME_EXTRA_SIZE )
#define XPM_INITDATA_ALIGNED_SIZE ( XPM_INITDATA_TOTAL_SIZE + ((XPM_INITDATA_TOTAL_SIZE % 4U) ? (4U - (XPM_INITDATA_TOTAL_SIZE % 4U)) : 0U) )

EXPORT_DS(XPmInitData, \
		XPLMI_MODULE_XILPM_ID, XPM_DEVICETABLE_DS_ID, \
		XPM_TOPOLOGY_DS_VERSION, XPM_TOPOLOGY_DS_LCVERSION, \
		XPM_INITDATA_ALIGNED_SIZE, \
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
