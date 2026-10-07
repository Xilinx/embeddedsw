/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_cache.h
*
* @addtogroup riscv_cache_apis RISC-V Cache APIs
* @{
*
* The xil_cache.h file contains cache related driver functions (or macros)
* that can be used to access the device.  The user should refer to the
* hardware device specification for more details of the device operation.
* The functions in this header file can be used across all Xilinx supported
* processors.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 9.00  sa   08/29/22 Initial release
* 9.1   ml   01/09/24 Fix comment for Xil_L2CacheInvalidateRange.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/

#ifndef XIL_CACHE_H
#define XIL_CACHE_H

#if defined XENV_VXWORKS
/* VxWorks environment */
#error "Unknown processor / architecture. Must be PPC for VxWorks."
#else
/* standalone environment */

#include "riscv_interface.h"
#include "xil_types.h"
#include "xparameters.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/**
*
* @brief    Invalidate the entire L1 data cache. If the cacheline is modified
*            (dirty), the modified contents are lost.
*
* @note
*
****************************************************************************/
#define Xil_L1DCacheInvalidate() riscv_invalidate_dcache()

/****************************************************************************/
/**
*
* @brief    Invalidate the entire L2 data cache. If the cacheline is modified
*           (dirty),the modified contents are lost.
*
* @note
*
****************************************************************************/
#define Xil_L2CacheInvalidate() riscv_invalidate_dcache()

/****************************************************************************/
/**
*
* @brief    Invalidate the L1 data cache for the given address range.
*           If the bytes specified by the address (Addr) are cached by the L1
*           data cache, the cacheline containing that byte is invalidated.If
*           the cacheline is modified (dirty), the modified contents are lost.
*
* @param[in] Addr address of ragne to be invalidated.
* @param[in] Len the length in bytes to be invalidated.
*
* @note
*
****************************************************************************/
#define Xil_L1DCacheInvalidateRange(Addr, Len) \
				riscv_invalidate_dcache_range((Addr), (Len))

/****************************************************************************/
/**
*
* @brief    Invalidate the L2 data cache for the given address range.
*           If the bytes specified by the address (Addr) are cached by the
*           L1 data cache, the cacheline containing that byte is invalidated.
*           If the cacheline is modified (dirty), the modified contents are lost.
*
* @param[in] Addr address of ragne to be invalidated.
* @param[in] Len length in bytes to be invalidated.
*
* @note
* Processor must be in real mode.
****************************************************************************/
#define Xil_L2CacheInvalidateRange(Addr, Len) \
				riscv_invalidate_dcache_range((Addr), (Len))

/****************************************************************************/
/**
* @brief   Flush the L1 data cache for the given address range.
*          If the bytes specified by the address (Addr) are cached by the
*          data cache, and is modified (dirty), the cacheline will be written
*          to system memory.The cacheline will also be invalidated.
*
* @param[in] Addr the starting address of the range to be flushed.
* @param[in] Len length in byte to be flushed.
*
****************************************************************************/
#define Xil_L1DCacheFlushRange(Addr, Len) \
				riscv_flush_dcache_range((Addr), (Len))

/****************************************************************************/
/**
* @brief    Flush the L2 data cache for the given address range.
*           If the bytes specified by the address (Addr) are cached by the
*           data cache, and is modified (dirty), the cacheline will be
*           written to system memory. The cacheline will also be invalidated.
*
* @param[in] Addr the starting address of the range to be flushed.
* @param[in] Len length in byte to be flushed.
*
****************************************************************************/
#define Xil_L2CacheFlushRange(Addr, Len) \
				riscv_flush_dcache_range((Addr), (Len))

/****************************************************************************/
/**
* @brief    Flush the entire L1 data cache. If any cacheline is dirty, the
*           cacheline will be written to system memory. The entire data cache
*           will be invalidated.
*
****************************************************************************/
#define Xil_L1DCacheFlush() riscv_flush_dcache()

/****************************************************************************/
/**
* @brief    Flush the entire L2 data cache. If any cacheline is dirty, the
*           cacheline will be written to system memory. The entire data cache
*           will be invalidated.
*
****************************************************************************/
#define Xil_L2CacheFlush() riscv_flush_dcache()

/****************************************************************************/
/**
*
* @brief    Invalidate the instruction cache for the given address range.
*
* @param[in] Addr address of ragne to be invalidated.
* @param[in] Len the length in bytes to be invalidated.
*
****************************************************************************/
#define Xil_L1ICacheInvalidateRange(Addr, Len) \
				riscv_invalidate_icache_range((Addr), (Len))

/****************************************************************************/
/**
*
* @brief    Invalidate the entire instruction cache.
*
****************************************************************************/
#define Xil_L1ICacheInvalidate() riscv_invalidate_icache()


/****************************************************************************/
/**
*
* @brief    Enable the L1 data cache.
*
* @note     This is processor specific.
*
****************************************************************************/
#define Xil_L1DCacheEnable()

/****************************************************************************/
/**
*
* @brief    Disable the L1 data cache.
*
* @note     This is processor specific.
*
****************************************************************************/
#define Xil_L1DCacheDisable()

/****************************************************************************/
/**
*
* @brief    Enable the instruction cache.
*
* @note     This is processor specific.
*
****************************************************************************/
#define Xil_L1ICacheEnable()

/****************************************************************************/
/**
*
* @brief    Disable the L1 Instruction cache.
*
* @note     This is processor specific.
*
****************************************************************************/
#define Xil_L1ICacheDisable()

/****************************************************************************/
/**
*
* @brief    Enable the data cache.
*
****************************************************************************/
#define Xil_DCacheEnable()

/****************************************************************************/
/**
*
* @brief    Enable the instruction cache.
*
* @note
*
****************************************************************************/
#define Xil_ICacheEnable()

#define Xil_DCacheInvalidate() riscv_invalidate_dcache() /**< Invalidate the entire data cache. */


/**
 * @brief Invalidate the data cache for the given address range.
 * @param[in] Addr Start address of range to be invalidated.
 * @param[in] Len  Length of range to be invalidated in bytes.
 */
#define Xil_DCacheInvalidateRange(Addr, Len) \
				riscv_invalidate_dcache_range((Addr), (Len))


#define Xil_DCacheFlush() riscv_flush_dcache() /**< Flush the entire data cache. */

/**
 * @brief Flush the data cache for the given address range.
 * @param[in] Addr Start address of range to be flushed.
 * @param[in] Len  Length of range to be flushed in bytes.
 */
#define Xil_DCacheFlushRange(Addr, Len) \
				riscv_flush_dcache_range((Addr), (Len))


#define Xil_ICacheInvalidate() riscv_invalidate_icache() /**< Invalidate the entire instruction cache. */


/**
 * @brief Invalidate the instruction cache for the given address range.
 * @param[in] Addr Start address of range to be invalidated.
 * @param[in] Len  Length of range to be invalidated in bytes.
 */
#define Xil_ICacheInvalidateRange(Addr, Len) \
				riscv_invalidate_icache_range((Addr), (Len))

#define Xil_DCacheDisable(void) /**< Disable the data cache (no-op for RISC-V, cache cannot be disabled). */
#define Xil_ICacheDisable(void) /**< Disable the instruction cache (no-op for RISC-V, cache cannot be disabled). */

#ifdef __cplusplus
}
#endif

#endif

#endif
/**
* @} End of "addtogroup riscv_cache_apis".
*/
