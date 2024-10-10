/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_gic.h
*
* This file contains default headers and definitions used by GIC module
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	av	19/02/2020 	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_GIC_H_
#define XPSMFW_GIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* GIC Handler Table Structure */
typedef void (*GicHandlerFunction_t)(void);

/**
 * @brief Structure to hold GIC P2 handler information
 */
struct GicP2HandlerTable_t {
        u32 Mask; /**< The mask value for the GIC P2 handler */
	GicHandlerFunction_t CpmHandler; /**< The handler function for CPM */
	GicHandlerFunction_t Cpm5Handler; /**< The handler function for CPM5 */
};

XStatus XPsmFw_DispatchGicP2Handler(u32 GicP2Status, u32 GicP2IntMask);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_GIC_H */
