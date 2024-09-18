/******************************************************************************
 * Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
#define XPM_DATA_STRUCT_VERSION 0x1
#define XPM_DATA_STRUCT_LCVERSION 0x1
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

#ifdef __cplusplus
}
#endif

#endif /* XPM_UPDATE_DATA_H_ */
