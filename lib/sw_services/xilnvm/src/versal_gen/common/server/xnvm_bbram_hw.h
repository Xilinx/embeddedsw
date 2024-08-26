/*******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_bbram_hw.h
*
* @addtogroup xnvm_bbram_hw_reg XilNvm BBRAM Hw reg
* @{
*
* @cond xnvm_internal
* This file contains NVM library BBRAM modules hardware register definitions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   mmd  04/01/2019 Initial release
* 2.1	am 	 08/19/2020 Resolved MISRA C violations.
* 2.4   kal  07/13/2021 Fixed doxygen warnings
* 3.1   skg  10/23/2022 Added comments for macros
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/
#ifndef XNVM_BBRAM_HW_H
#define XNVM_BBRAM_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/

/*************************** Constant Definitions *****************************/
/**
 * @name BBRAM Controller Base Address
 */
#define XNVM_BBRAM_BASE_ADDR			(0xF11F0000U)
						/**< BBRAM Base Address */
/** @} */

/**
 * @name BBRAM Controller Registers
 */
/**< BBRAM Control Register offsets */
#define XNVM_BBRAM_STATUS_REG			(0x00U)
#define XNVM_BBRAM_CTRL_REG			(0x04U)
#define XNVM_BBRAM_PGM_MODE_REG			(0x08U)
#define XNVM_BBRAM_AES_CRC_REG			(0x0CU)
#define XNVM_BBRAM_0_REG			(0x10U)
#define XNVM_BBRAM_8_REG			(0x30U)
#define XNVM_BBRAM_MSW_LOCK_REG			(0x4CU)
/** @} */

/**
 * @name  STATUS register
 */
/**< BBRAM Status Register definition */
#define XNVM_BBRAM_STATUS_PGM_MODE_DONE		((u32)0x01U << 0U)
#define XNVM_BBRAM_STATUS_ZEROIZED			((u32)0x01U << 4U)
#define XNVM_BBRAM_STATUS_AES_CRC_DONE		((u32)0x01U << 8U)
#define XNVM_BBRAM_STATUS_AES_CRC_PASS		((u32)0x01U << 9U)
/** @} */

/**
 * @name  CTRL register
 */
/**< BBRAM Ctrl Start Zeroize mask */
#define XNVM_BBRAM_CTRL_START_ZEROIZE		((u32)0x01U << 0U)
/** @} */

/**
 * @name  PGM_MODE register
 */
/**< PGM_MODE Passcode */
#define XNVM_EFUSE_PGM_MODE_PASSCODE		(0x757BDF0DU)
/** @} */

/**
 * @name  MSW_LOCK register definition
 */
/**< BBRAM MSW LOCK Mask */
#define XNVM_BBRAM_MSW_LOCK			((u32)0x01U << 0U)
/** @} */

/**
 * @name  Timeout in term of number of times status register polled
 */
/**< PGM_MODE Timeout */
#define XNVM_BBRAM_PGM_MODE_TIMEOUT_VAL		(0x400U)

/**< CRC_DONE Timeout */
#define XNVM_BBRAM_AES_CRC_DONE_TIMEOUT_VAL	(0x400U)

/**< ZEROIZE Timeout */
#define XNVM_BBRAM_ZEROIZE_TIMEOUT_VAL		(0x400U)
/** @} */

/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif
