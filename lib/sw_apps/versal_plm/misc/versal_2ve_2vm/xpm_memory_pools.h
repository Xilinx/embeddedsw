/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_MEMORY_POOLS_H
#define XPM_MEMORY_POOLS_H

/* Memory pool size compile-time constants */
#define MAX_TOPO_POOL_SIZE     0xAD00U
#define MAX_SUBSYS_POOL_SIZE   0x400U
#define MAX_REQM_POOL_SIZE     0x3800U
#define MAX_DEVOPS_POOL_SIZE   0x1400U
#define MAX_OTHER_POOL_SIZE    0x2000U
#define MAX_BOARD_POOL_SIZE    0x400U

/**
 * @brief Aggregate size of the runtime pool group (SUBSYS + REQM + DEVOPS + OTHER).
 */
#define MAX_RUNTIME_POOL_TOTAL_SIZE  (MAX_SUBSYS_POOL_SIZE + MAX_REQM_POOL_SIZE + \
				MAX_DEVOPS_POOL_SIZE + MAX_OTHER_POOL_SIZE)
#endif /* XPM_MEMORY_POOLS_H */
