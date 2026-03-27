/******************************************************************************
 * Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_UPDATE_DATA_H_
#define XPM_UPDATE_DATA_H_
#include "xpm_debug.h"
#include "xpm_common.h"
#ifdef __cplusplus
extern "C" {
#endif
/****************** Data struct Version **************/
/**
 * Version 2: Added regnode/access table save/restore for IPU.
 * LCVERSION bumped to 2 to reject IPU from older PLM that lacks
 * XPM_REGNODES_DS_ID support - regnode state cannot be preserved
 * across such updates.
 */
#define XPM_DATA_STRUCT_VERSION 0x2
/**
 * @def XPM_DATA_STRUCT_LCVERSION
 * @brief Lowest compatible version for IPU data structure
 */
#define XPM_DATA_STRUCT_LCVERSION 0x2
/****************** Data struct IDs **************/
#define XPM_BYTEBUFFER_DS_ID		1U
#define XPM_BYTEBUFFER_ADDR_DS_ID	2U
#define XPM_ALLNODES_DS_ID		3U
#define XPM_NUMNODES_DS_ID		4U
#define XPM_EVENTSEQ_DS_ID		5U
#define XPM_PENDINGEVENT_DS_ID		6U
#define XPM_POSEMPTYSPACE_DS_ID		7U
#define XPM_PMNOTIFIERS_DS_ID		8U
#define XPM_SCHEDULERTASK_DS_ID		9U
#define XPM_ALLSAVEREGIONSINFO_DS_ID	10U
#define XPM_PREVNUMSAVEREGIONINFO_DS_ID 11U
#define XPM_GICPROXYGROUPS_DS_ID	12U
/**
 * @def XPM_REGNODES_DS_ID
 * @brief Data structure ID for regnode and access table state
 */
#define XPM_REGNODES_DS_ID		13U

#ifdef __cplusplus
}
#endif

#endif /* XPM_UPDATE_DATA_H_ */
