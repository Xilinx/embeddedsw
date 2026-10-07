/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_cache.h
*
* @addtogroup a53_64_cache_apis Cortex A53 64bit Processor Cache Functions
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
* 5.00 	pkp  05/29/14 First release
* 9.00  ml   03/03/23 Add description to fix doxygen warnings.
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

/************************** Constant Definitions *****************************/
#define L1_DATA_PREFETCH_CONTROL_MASK  0xE000
#define L1_DATA_PREFETCH_CONTROL_SHIFT  13

/**
 *@endcond
 */

/***************** Macros (Inline Functions) Definitions *********************/
#define Xil_DCacheFlushRange Xil_DCacheInvalidateRange /**< DCache range */
/************************** Function Prototypes ******************************/
void Xil_DCacheEnable(void);
void Xil_DCacheDisable(void);
void Xil_DCacheInvalidate(void);
/**
 * @brief Invalidate a range of data cache lines.
 * @param[in] adr Start address of the range to invalidate.
 * @param[in] len Length in bytes of the range to invalidate.
 */
void Xil_DCacheInvalidateRange(INTPTR adr, INTPTR len);
/**
 * @brief Invalidate a single data cache line.
 * @param[in] adr Address of the cache line to invalidate.
 */
void Xil_DCacheInvalidateLine(INTPTR adr);
void Xil_DCacheFlush(void);
/**
 * @brief Flush a single data cache line.
 * @param[in] adr Address of the cache line to flush.
 */
void Xil_DCacheFlushLine(INTPTR adr);

void Xil_ICacheEnable(void);
void Xil_ICacheDisable(void);
void Xil_ICacheInvalidate(void);
/**
 * @brief Invalidate a range of instruction cache lines.
 * @param[in] adr Start address of the range to invalidate.
 * @param[in] len Length in bytes of the range to invalidate.
 */
void Xil_ICacheInvalidateRange(INTPTR adr, INTPTR len);
/**
 * @brief Invalidate a single instruction cache line.
 * @param[in] adr Address of the cache line to invalidate.
 */
void Xil_ICacheInvalidateLine(INTPTR adr);
void Xil_ConfigureL1Prefetch(u8 num);
#ifdef __cplusplus
}
#endif

#endif
/**
* @} End of "addtogroup a53_64_cache_apis".
*/
