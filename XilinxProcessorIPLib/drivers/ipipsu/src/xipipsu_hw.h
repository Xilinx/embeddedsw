/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XIPIPSU_HW_H_
#define XIPIPSU_HW_H_

/**
*
* @file xipipsu_hw.h
* @brief Defines the low-level hardware register macros for the IPIPSU driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   mjr  03/15/15 First release
* 2.1   kvn  05/05/16 Modified code for MISRA-C:2012 Compliance
* 2.5   sdd  12/17/18 Add the cpp extern macro
* 2.11  ma   01/17/22 Added XIPIPSU_BASE_ADDR macro
* 2.12  sd   02/24/22 Added support for VERSAL NET
*       sd   05/05/22 Added xparameters.h
* 2.14  sd   07/27/23 Update the target count
* 2.16  ma   09/10/24 Updated to support VERSAL_2VE_2VM platform
* 2.17  ht   11/08/24 Update description of XIPIPSU_MSG_BUF_SIZE
* 2.17  ht   04/24/25 Update IPI bitmask definitions for Versal Platforms
* 2.20  vn   09/10/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/**
* @addtogroup ipipsu_api IPIPSU APIs
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xparameters.h"

/************************** Constant Definitions *****************************/
/* Message RAM related params */
#if defined (VERSAL_NET) || defined (VERSAL_2VE_2VM)
#define XIPIPSU_MSG_RAM_BASE 0xEB3F0000U  /**< IPI message RAM base address. */
#elif defined (versal)
#define XIPIPSU_MSG_RAM_BASE 0xFF3F0000U  /**< IPI message RAM base address. */
#else
#define XIPIPSU_MSG_RAM_BASE 0xFF990000U  /**< IPI message RAM base address. */
#endif
#define XIPIPSU_MSG_BUF_SIZE 8U	/**< Message buffer size in 32-bit
				  * words (eight words, 32 bytes). */
#define XIPIPSU_MAX_BUFF_INDEX	7U /**< Maximum buffer index. */

/* EIGHT pairs of TWO buffers(msg+resp) of THIRTY TWO bytes each */
/** Buffer offset for group. */
#define XIPIPSU_BUFFER_OFFSET_GROUP	(8U * 2U * 32U)
/** Buffer offset for target. */
#define XIPIPSU_BUFFER_OFFSET_TARGET (32U * 2U)
/** Buffer offset for response. */
#define XIPIPSU_BUFFER_OFFSET_RESPONSE		(32U)

/* Number of IPI slots enabled on the device */
#ifndef SDT
/** Maximum number of targets. */
#define XIPIPSU_MAX_TARGETS    XPAR_XIPIPSU_NUM_TARGETS
#else
/** Maximum number of targets. */
#define XIPIPSU_MAX_TARGETS    XPAR_XIPIPSU_0_IPI_TARGET_COUNT
#endif

/* Register Offsets for each member  of IPI Register Set */
#define XIPIPSU_TRIG_OFFSET 0x00U /**< Offset for trigger register. */
#define XIPIPSU_OBS_OFFSET 0x04U  /**< Offset for observation register. */
#define XIPIPSU_ISR_OFFSET 0x10U  /**< Offset for the interrupt
				   * status register (ISR). */
#define XIPIPSU_IMR_OFFSET 0x14U  /**< Offset for interrupt mask register. */
#define XIPIPSU_IER_OFFSET 0x18U  /**< Offset for interrupt enable register. */
#define XIPIPSU_IDR_OFFSET 0x1CU  /**< Offset for interrupt disable register. */

#if defined (VERSAL_NET) || defined (VERSAL_2VE_2VM)
#define XIPIPSU_BASE_ADDR	0xEB300000U /**< Versal NET IPI base address. */
#define XIPIPSU_ISR_BASE	0xEB300010U /**< Versal NET ISR base address. */
#elif defined (versal)
#define XIPIPSU_BASE_ADDR	0xFF300000U /**< Versal IPI base address. */
#define XIPIPSU_ISR_BASE	0xFF300010U /**< Versal ISR base address. */
#endif
#define XIPIPSU_ECC_UE_MASK	0x40U  /**< Uncorrectable error mask. */

/* MASK of all valid IPI bits in above registers */
#if defined (VERSAL_NET) || defined (VERSAL_2VE_2VM)
#define XIPIPSU_ALL_MASK    0x0000FFFFU /**< All valid bit mask. */
#elif defined (versal)
#define XIPIPSU_ALL_MASK        0x000003FFU /**< All valid bit mask. */
#else
#define XIPIPSU_ALL_MASK	0x0F0F0301U /**< All valid bit mask. */
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIPIPSU_HW_H_ */
/** @} */
