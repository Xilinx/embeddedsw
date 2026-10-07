/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_cache.h
*
* @addtogroup a9_cache_apis Cortex A9 Processor Cache Functions
*
* Cache functions provide access to cache related operations such as flush
* and invalidate for instruction and data caches. It gives option to perform
* the cache operations on a single cacheline, a range of memory and an entire
* cache.
*
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  01/29/10 First release
* 3.04a sdm  01/02/12 Remove redundant dsb/dmb instructions in cache maintenance
*		      APIs.
* 6.8   aru  09/06/18 Removed compilation warnings for ARMCC toolchain.
* 9.0   ml   03/03/23 Updated function prototypes.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/


#ifndef XIL_CACHE_H
#define XIL_CACHE_H

/**
*@cond nocomments
*/
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__

#define asm_cp15_inval_dc_line_mva_poc(param) __asm__ __volatile__("mcr " \
			XREG_CP15_INVAL_DC_LINE_MVA_POC :: "r" (param));

#define asm_cp15_clean_inval_dc_line_mva_poc(param) __asm__ __volatile__("mcr " \
			XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC :: "r" (param));

#define asm_cp15_inval_ic_line_mva_pou(param) __asm__ __volatile__("mcr " \
			XREG_CP15_INVAL_IC_LINE_MVA_POU :: "r" (param));

#define asm_cp15_inval_dc_line_sw(param) __asm__ __volatile__("mcr " \
			XREG_CP15_INVAL_DC_LINE_SW :: "r" (param));

#define asm_cp15_clean_inval_dc_line_sw(param) __asm__ __volatile__("mcr " \
			XREG_CP15_CLEAN_INVAL_DC_LINE_SW :: "r" (param));

#elif defined (__ICCARM__)

#define asm_cp15_inval_dc_line_mva_poc(param) __asm volatile ("mcr " \
			XREG_CP15_INVAL_DC_LINE_MVA_POC :: "r" (param));

#define asm_cp15_clean_inval_dc_line_mva_poc(param) __asm volatile ("mcr " \
			XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC :: "r" (param));

#define asm_cp15_inval_ic_line_mva_pou(param) __asm volatile ("mcr " \
			XREG_CP15_INVAL_IC_LINE_MVA_POU :: "r" (param));

#define asm_cp15_inval_dc_line_sw(param) __asm volatile ("mcr " \
			XREG_CP15_INVAL_DC_LINE_SW :: "r" (param));

#define asm_cp15_clean_inval_dc_line_sw(param) __asm volatile ("mcr " \
			XREG_CP15_CLEAN_INVAL_DC_LINE_SW :: "r" (param));

#endif

/**
*@endcond
*/

void Xil_DCacheEnable(void);
void Xil_DCacheDisable(void);
void Xil_DCacheInvalidate(void);
/** @brief Invalidate a range of data cache lines. */
void Xil_DCacheInvalidateRange(INTPTR adr, u32 len);
void Xil_DCacheFlush(void);
/** @brief Flush a range of data cache lines. */
void Xil_DCacheFlushRange(INTPTR adr, u32 len);

void Xil_ICacheEnable(void);
void Xil_ICacheDisable(void);
void Xil_ICacheInvalidate(void);
/** @brief Invalidate a range of instruction cache lines. */
void Xil_ICacheInvalidateRange(INTPTR adr, u32 len);

#ifdef __cplusplus
}
#endif

/**
* @} End of "addtogroup a9_cache_apis".
*/
#endif
