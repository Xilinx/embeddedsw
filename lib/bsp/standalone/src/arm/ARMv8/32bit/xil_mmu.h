/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xil_mmu.h
*
* @addtogroup a53_32_mmu_apis Cortex A53 32bit Processor MMU Handling
* @{
* MMU functions equip users to enable MMU, disable MMU and modify default
* memory attributes of MMU table as per the need.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.2	pkp  28/05/15 First release
* 7.1	mus  30/07/19 Added constant definitions for memory attributes.
* 9.0   ml   03/03/23 Add description to fix doxygen warnings.
* 9.1   bl   10/11/23 Add API Xil_MemMap
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/

#ifndef XIL_MMU_H
#define XIL_MMU_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************** Include Files *********************************/

#include "xil_types.h"

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/
/* Memory type */
#define NORM_NONCACHE 0x11DE2 	/**< Normal Non-cacheable */
#define STRONG_ORDERED 0xC02	/**< Strongly ordered */
#define DEVICE_MEMORY 0xC06	/**< Device memory */
#define RESERVED 0x0		/**< reserved memory */

#define NORM_WT_CACHE 0x16DEA /**< Normal write-through cacheable shareable */

#define NORM_WB_CACHE 0x15DE6 /**< Normal write back cacheable shareable */

#define SHAREABLE (0x1 << 16) /**< shareability attribute */
#define NON_SHAREABLE	(~(0x1 << 16)) /**< Non-shareability attribute */

#define EXECUTE_NEVER ((0x1 << 4) | (0x1 << 0)) /**< Execution type */

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/** @cond nocomments */
void Xil_SetTlbAttributes(UINTPTR Addr, u32 attrib);
/** @endcond */
/**
 * @brief Map a physical address range.
 * @param[in] PhysAddr Physical address to be mapped.
 * @param[in] size Size of the region in bytes.
 * @param[in] flags Memory attributes/flags to apply.
 * @return PhysAddr unchanged on success, or NULL on address-range overflow
 *         when flags is nonzero.
 */
void* Xil_MemMap(UINTPTR PhysAddr, size_t size, u32 flags);
void Xil_EnableMMU(void);
void Xil_DisableMMU(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIL_MMU_H */
/**
* @} End of "addtogroup a53_32_mmu_apis".
*/
