/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_cache.h
*
* @addtogroup r5_cache_apis Cortex R5 Processor Cache Functions
*
* Cache functions provide access to cache related operations such as flush
*  and invalidate for instruction and data caches. It gives option to perform
* the cache operations on a single cacheline, a range of memory and an entire
* cache.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 5.00 	pkp  02/20/14 First release
* 6.2   mus  01/27/17 Updated to support IAR compiler
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/
#ifndef XIL_CACHE_H
#define XIL_CACHE_H

#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *@cond nocomments
 */

#if defined (__GNUC__)
#define asm_inval_dc_line_mva_poc(param) __asm__ __volatile__("mcr " \
		XREG_CP15_INVAL_DC_LINE_MVA_POC :: "r" (param))

#define asm_clean_inval_dc_line_sw(param) __asm__ __volatile__("mcr " \
		XREG_CP15_CLEAN_INVAL_DC_LINE_SW :: "r" (param))

#define asm_clean_inval_dc_line_mva_poc(param) __asm__ __volatile__("mcr " \
		XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC :: "r" (param))

#define asm_inval_ic_line_mva_pou(param) __asm__ __volatile__("mcr " \
		XREG_CP15_INVAL_IC_LINE_MVA_POU :: "r" (param))
#elif defined (__ICCARM__)
#define asm_inval_dc_line_mva_poc(param) __asm volatile("mcr " \
		XREG_CP15_INVAL_DC_LINE_MVA_POC :: "r" (param))

#define asm_clean_inval_dc_line_sw(param) __asm volatile("mcr " \
		XREG_CP15_CLEAN_INVAL_DC_LINE_SW :: "r" (param))

#define asm_clean_inval_dc_line_mva_poc(param) __asm volatile("mcr " \
		XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC :: "r" (param))

#define asm_inval_ic_line_mva_pou(param) __asm volatile("mcr " \
		XREG_CP15_INVAL_IC_LINE_MVA_POU :: "r" (param))
#endif

/**
 *@endcond
 */

#if defined (ARMR52)
void Xil_DCacheEnable(void) __attribute__((__section__(".boot")));
void Xil_DCacheDisable(void) __attribute__((__section__(".boot")));
void Xil_DCacheInvalidate(void) __attribute__((__section__(".boot")));
void Xil_DCacheFlush(void) __attribute__((__section__(".boot")));
void Xil_ICacheEnable(void) __attribute__((__section__(".boot")));
void Xil_ICacheDisable(void) __attribute__((__section__(".boot")));
void Xil_ICacheInvalidate(void) __attribute__((__section__(".boot")));
#else
void Xil_DCacheEnable(void);
void Xil_DCacheDisable(void);
void Xil_DCacheInvalidate(void);
void Xil_DCacheFlush(void);
void Xil_ICacheEnable(void);
void Xil_ICacheDisable(void);
void Xil_ICacheInvalidate(void);
#endif

/** @brief Invalidate a range of data cache lines. */
void Xil_DCacheInvalidateRange(INTPTR adr, u32 len);
/** @brief Flush a range of data cache lines. */
void Xil_DCacheFlushRange(INTPTR adr, u32 len);
/** @brief Invalidate a single data cache line. */
void Xil_DCacheInvalidateLine(INTPTR adr);
/** @brief Flush a single data cache line. */
void Xil_DCacheFlushLine(INTPTR adr);
/**
 * @brief Clean and store a single data cache line.
 * @param[in] adr Address of the cache line to store.
 */
void Xil_DCacheStoreLine(INTPTR adr);

/** @brief Invalidate a range of instruction cache lines. */
void Xil_ICacheInvalidateRange(INTPTR adr, u32 len);
/** @brief Invalidate a single instruction cache line. */
void Xil_ICacheInvalidateLine(INTPTR adr);

#ifdef __cplusplus
}
#endif

#endif
/**
* @} End of "addtogroup r5_cache_apis".
*/
