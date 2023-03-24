/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
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
*
* </pre>
*
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
* @param    None.
*
* @return   None.
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
* @param    None.
*
* @return   None.
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
* @param    Addr is address of ragne to be invalidated.
* @param    Len is the length in bytes to be invalidated.
*
* @return   None.
*
* @note
*
****************************************************************************/
#define Xil_L1DCacheInvalidateRange(Addr, Len) \
				riscv_invalidate_dcache_range((Addr), (Len))

/****************************************************************************/
/**
*
* @brief    Invalidate the L1 data cache for the given address range.
*           If the bytes specified by the address (Addr) are cached by the
*           L1 data cache, the cacheline containing that byte is invalidated.
*           If the cacheline is modified (dirty), the modified contents are lost.
*
* @param    Addr: address of ragne to be invalidated.
* @param    Len: length in bytes to be invalidated.
*
* @return   None.
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
* @param    Addr: the starting address of the range to be flushed.
* @param    Len: length in byte to be flushed.
*
* @return   None.
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
* @param   Addr: the starting address of the range to be flushed.
* @param   Len: length in byte to be flushed.
*
* @return   None.
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
* @return   None.
*
****************************************************************************/
#define Xil_L1DCacheFlush() riscv_flush_dcache()

/****************************************************************************/
/**
* @brief    Flush the entire L2 data cache. If any cacheline is dirty, the
*           cacheline will be written to system memory. The entire data cache
*           will be invalidated.
*
* @return   None.
*
****************************************************************************/
#define Xil_L2CacheFlush() riscv_flush_dcache()

/****************************************************************************/
/**
*
* @brief    Invalidate the instruction cache for the given address range.
*
* @param    Addr is address of ragne to be invalidated.
* @param    Len is the length in bytes to be invalidated.
*
* @return   None.
*
****************************************************************************/
#define Xil_L1ICacheInvalidateRange(Addr, Len) \
				riscv_invalidate_icache_range((Addr), (Len))

/****************************************************************************/
/**
*
* @brief    Invalidate the entire instruction cache.
*
* @param    None
*
* @return   None.
*
****************************************************************************/
#define Xil_L1ICacheInvalidate() riscv_invalidate_icache()


/****************************************************************************/
/**
*
* @brief    Enable the L1 data cache.
*
* @return   None.
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
* @return   None.
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
* @return   None.
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
* @return   None.
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
* @param    None
*
* @return   None.
*
****************************************************************************/
#define Xil_DCacheEnable()

/****************************************************************************/
/**
*
* @brief    Enable the instruction cache.
*
* @param    None
*
* @return   None.
*
* @note
*
*
****************************************************************************/
#define Xil_ICacheEnable()

/****************************************************************************
*
* @brief    Invalidate the entire Data cache.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
#define Xil_DCacheInvalidate() riscv_invalidate_dcache()


/****************************************************************************
*
* @brief    Invalidate the Data cache for the given address range.
*           If the bytes specified by the address (adr) are cached by the
*           Data cache, the cacheline containing that byte is invalidated.
*           If the cacheline is modified (dirty), the modified contents are
*           lost and are NOT written to system memory before the line is
*           invalidated.
*
* @param	Addr: Start address of ragne to be invalidated.
* @param	Len: Length of range to be invalidated in bytes.
*
* @return	None.
*
****************************************************************************/
#define Xil_DCacheInvalidateRange(Addr, Len) \
				riscv_invalidate_dcache_range((Addr), (Len))


/****************************************************************************
*
* @brief    Flush the entire Data cache.
*
* @param	None.
*
* @return	None.
*
****************************************************************************/
#define Xil_DCacheFlush() riscv_flush_dcache()

/****************************************************************************
* @brief     Flush the Data cache for the given address range.
*            If the bytes specified by the address (adr) are cached by the
*            Data cache, the cacheline containing that byte is invalidated.
*            If the cacheline is modified (dirty), the written to system
*            memory first before the before the line is invalidated.
*
* @param	Addr: Start address of range to be flushed.
* @param	Len: Length of range to be flushed in bytes.
*
* @return	None.
*
****************************************************************************/
#define Xil_DCacheFlushRange(Addr, Len) \
				riscv_flush_dcache_range((Addr), (Len))


/****************************************************************************
*
* @brief    Invalidate the entire instruction cache.
*
* @param	None.
*
* @return	None.
*
****************************************************************************/
#define Xil_ICacheInvalidate() riscv_invalidate_icache()


/****************************************************************************
*
* @brief     Invalidate the instruction cache for the given address range.
*            If the bytes specified by the address (adr) are cached by the
*            Data cache, the cacheline containing that byte is invalidated.
*            If the cacheline is modified (dirty), the modified contents are
*            lost and are NOT written to system memory before the line is
*            invalidated.
*
* @param	Addr: Start address of ragne to be invalidated.
* @param	Len: Length of range to be invalidated in bytes.
*
* @return	None.
*
****************************************************************************/
#define Xil_ICacheInvalidateRange(Addr, Len) \
				riscv_invalidate_icache_range((Addr), (Len))

#define Xil_DCacheDisable(void)
#define Xil_ICacheDisable(void)

#ifdef __cplusplus
}
#endif

#endif

#endif
/**
* @} End of "addtogroup riscv_cache_apis".
*/
