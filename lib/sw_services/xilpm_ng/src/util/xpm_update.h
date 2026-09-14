/******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XPM_UPDATE_H
#define XPM_UPDATE_H

#define XPM_TOPOLOGY_DS_ID 0x1
#define XPM_ALLOC_TOPO_MEM_DS_ID 0x2
#define XPM_DEVICETABLE_DS_ID 0x3
/** @brief Topology layout version for the enlarged, relocated pool. */
#define XPM_TOPOLOGY_DS_VERSION 0x2
/** @brief Reject saved pointers from the previous topology pool layout. */
#define XPM_TOPOLOGY_DS_LCVERSION 0x2

#define XPM_SUBSYSTEMPOOL_DS_ID 0x4
#define XPM_SUBSYSTEMPOOL_DS_VERSION 0x1
#define XPM_SUBSYSTEMPOOL_DS_LCVERSION 0x1

#define XPM_BOARD_POOL_DS_ID 0x5
#define XPM_BOARD_POOL_DS_VERSION 0x1
#define XPM_BOARD_POOL_DS_LCVERSION 0x1

#define XPM_PLACE_IN(SECTION) __attribute__((section(SECTION)))
#define XPM_INIT_DATA(SECTION) XPM_PLACE_IN(".RefTable." #SECTION)

#endif /*XPM_UPDATE_H*/
