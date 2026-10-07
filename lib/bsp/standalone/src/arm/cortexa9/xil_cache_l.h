/******************************************************************************
* Copyright (c) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_cache_l.h
*
* Contains L1 and L2 specific functions for the ARM cache functionality
* used by xcache.c. This functionality is being made available here for
* more sophisticated users.
*
* @addtogroup a9_cache_apis
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  01/24/10 First release
* 6.8   aru  09/06/18 Removed compilation warnings for ARMCC toolchain.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/
#ifndef XIL_CACHE_MACH_H
#define XIL_CACHE_MACH_H

#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif

/************************** Function Prototypes ******************************/

/** @brief Invalidate a single data cache line. */
void Xil_DCacheInvalidateLine(u32 adr);
/** @brief Flush a single data cache line. */
void Xil_DCacheFlushLine(u32 adr);
/**
 * @brief Store (clean) a single data cache line.
 * @param[in] adr Address of the cache line to store.
 */
void Xil_DCacheStoreLine(u32 adr);
/**
 * @brief Invalidate a single instruction cache line.
 * @param[in] adr Address of the instruction cache line to invalidate.
 */
void Xil_ICacheInvalidateLine(u32 adr);

void Xil_L1DCacheEnable(void); /**< Enable the L1 data cache. */
void Xil_L1DCacheDisable(void); /**< Disable the L1 data cache. */
void Xil_L1DCacheInvalidate(void); /**< Invalidate the entire L1 data cache. */
/**
 * @brief Invalidate a single L1 data cache line.
 * @param[in] adr Address of the L1 data cache line to invalidate.
 */
void Xil_L1DCacheInvalidateLine(u32 adr);
/**
 * @brief Invalidate a range of L1 data cache lines.
 * @param[in] adr Start address of the range to invalidate.
 * @param[in] len Length in bytes of the range to invalidate.
 */
void Xil_L1DCacheInvalidateRange(u32 adr, u32 len);
void Xil_L1DCacheFlush(void); /**< Flush (clean+invalidate) the entire L1 data cache. */
/**
 * @brief Flush a single L1 data cache line.
 * @param[in] adr Address of the L1 data cache line to flush.
 */
void Xil_L1DCacheFlushLine(u32 adr);
/**
 * @brief Flush a range of L1 data cache lines.
 * @param[in] adr Start address of the range to flush.
 * @param[in] len Length in bytes of the range to flush.
 */
void Xil_L1DCacheFlushRange(u32 adr, u32 len);
/**
 * @brief Store (clean) a single L1 data cache line.
 * @param[in] adr Address of the L1 data cache line to store.
 */
void Xil_L1DCacheStoreLine(u32 adr);

void Xil_L1ICacheEnable(void); /**< Enable the L1 instruction cache. */
void Xil_L1ICacheDisable(void); /**< Disable the L1 instruction cache. */
void Xil_L1ICacheInvalidate(void); /**< Invalidate the entire L1 instruction cache. */
/**
 * @brief Invalidate a single L1 instruction cache line.
 * @param[in] adr Address of the L1 instruction cache line to invalidate.
 */
void Xil_L1ICacheInvalidateLine(u32 adr);
/**
 * @brief Invalidate a range of L1 instruction cache lines.
 * @param[in] adr Start address of the range to invalidate.
 * @param[in] len Length in bytes of the range to invalidate.
 */
void Xil_L1ICacheInvalidateRange(u32 adr, u32 len);

void Xil_L2CacheEnable(void); /**< Enable the L2 cache. */
void Xil_L2CacheDisable(void); /**< Disable the L2 cache. */
void Xil_L2CacheInvalidate(void); /**< Invalidate the entire L2 cache. */
/**
 * @brief Invalidate a single L2 cache line.
 * @param[in] adr Address of the L2 cache line to invalidate.
 */
void Xil_L2CacheInvalidateLine(u32 adr);
/**
 * @brief Invalidate a range of L2 cache lines.
 * @param[in] adr Start address of the range to invalidate.
 * @param[in] len Length in bytes of the range to invalidate.
 */
void Xil_L2CacheInvalidateRange(u32 adr, u32 len);
void Xil_L2CacheFlush(void); /**< Flush (clean+invalidate) the entire L2 cache. */
/**
 * @brief Flush a single L2 cache line.
 * @param[in] adr Address of the L2 cache line to flush.
 */
void Xil_L2CacheFlushLine(u32 adr);
/**
 * @brief Flush a range of L2 cache lines.
 * @param[in] adr Start address of the range to flush.
 * @param[in] len Length in bytes of the range to flush.
 */
void Xil_L2CacheFlushRange(u32 adr, u32 len);
/**
 * @brief Store (clean) a single L2 cache line.
 * @param[in] adr Address of the L2 cache line to store.
 */
void Xil_L2CacheStoreLine(u32 adr);

#ifdef __cplusplus
}
#endif

#endif
/**
* @} End of "addtogroup a9_cache_apis".
*/
