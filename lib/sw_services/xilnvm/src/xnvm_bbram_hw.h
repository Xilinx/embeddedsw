/*******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_bbram_hw.h
*
* This file contains NVM library BBRAM modules hardware register definitions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   mmd  04/01/2019 Initial release
* 2.1	am 	 08/19/2020 Resolved MISRA C violations.
*
* </pre>
*
* @note
*
*******************************************************************************/
#ifndef XNVM_BBRAM_HW_H
#define XNVM_BBRAM_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/

/*************************** Constant Definitions *****************************/
/* BBRAM Controller base address definition */
#define XNVM_BBRAM_BASE_ADDR			(0xF11F0000U)

/* BBRAM Controller Register Map definition */
#define XNVM_BBRAM_STATUS_REG			(0x00U)
#define XNVM_BBRAM_CTRL_REG			(0x04U)
#define XNVM_BBRAM_PGM_MODE_REG			(0x08U)
#define XNVM_BBRAM_AES_CRC_REG			(0x0CU)
#define XNVM_BBRAM_0_REG			(0x10U)
#define XNVM_BBRAM_8_REG			(0x30U)
#define XNVM_BBRAM_MSW_LOCK_REG			(0x4CU)

/* BBRAM Controller STATUS register definition */
#define XNVM_BBRAM_STATUS_PGM_MODE_DONE		((u32)0x01U << 0U)
#define XNVM_BBRAM_STATUS_ZEROIZED			((u32)0x01U << 4U)
#define XNVM_BBRAM_STATUS_AES_CRC_DONE		((u32)0x01U << 8U)
#define XNVM_BBRAM_STATUS_AES_CRC_PASS		((u32)0x01U << 9U)

/* BBRAM Controller CTRL register definition */
#define XNVM_BBRAM_CTRL_START_ZEROIZE		((u32)0x01U << 0U)

/* BBRAM Controller PGM_MODE register definition */
#define XNVM_EFUSE_PGM_MODE_PASSCODE		(0x757BDF0DU)

/* BBRAM Controller MSW_LOCK register definition */
#define XNVM_BBRAM_MSW_LOCK			((u32)0x01U << 0U)

/* Timeout in term of number of times status register polled to check BBRAM
 * is set to programming mode
 */
#define XNVM_BBRAM_PGM_MODE_TIMEOUT_VAL		(0x400U)

/* Timeout in term of number of times status register polled to check BBRAM
 * AES CRC validation is complete
 */
#define XNVM_BBRAM_AES_CRC_DONE_TIMEOUT_VAL	(0x400U)

/* Timeout in term of number of times status register polled to check BBRAM
 * AES CRC validation is complete
 */
#define XNVM_BBRAM_ZEROIZE_TIMEOUT_VAL		(0x400U)

/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif
