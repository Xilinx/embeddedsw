/*******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
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
#define XNVM_BBRAM_BASE_ADDR			(0xF11f0000U)

/* BBRAM Controller Register Map definition */
#define XNVM_BBRAM_STATUS_REG			(0x00U)
#define XNVM_BBRAM_CTRL_REG			(0x04U)
#define XNVM_BBRAM_PGM_MODE_REG			(0x08U)
#define XNVM_BBRAM_AES_CRC_REG			(0x0CU)
#define XNVM_BBRAM_0_REG			(0x10U)
#define XNVM_BBRAM_1_REG			(0x14U)
#define XNVM_BBRAM_2_REG			(0x18U)
#define XNVM_BBRAM_3_REG			(0x1CU)
#define XNVM_BBRAM_4_REG			(0x20U)
#define XNVM_BBRAM_5_REG			(0x24U)
#define XNVM_BBRAM_6_REG			(0x28U)
#define XNVM_BBRAM_7_REG			(0x2CU)
#define XNVM_BBRAM_8_REG			(0x30U)
#define XNVM_BBRAM_SLV_ERR_CTRL_REG		(0x34U)
#define XNVM_BBRAM_SLV_ERR_ISR_REG		(0x38U)
#define XNVM_BBRAM_SLV_ERR_IMR_REG		(0x3CU)
#define XNVM_BBRAM_SLV_ERR_IER_REG		(0x40U)
#define XNVM_BBRAM_SLV_ERR_IDR_REG		(0x44U)
#define XNVM_BBRAM_MSW_LOCK_REG			(0x4CU)

/* BBRAM Controller STATUS register definition */
#define XNVM_BBRAM_STATUS_PGM_MODE_DONE		(0x01U << 0U)
#define XNVM_BBRAM_STATUS_ZEROIZED		(0x01U << 4U)
#define XNVM_BBRAM_STATUS_AES_CRC_DONE		(0x01U << 8U)
#define XNVM_BBRAM_STATUS_AES_CRC_PASS		(0x01U << 9U)

/* BBRAM Controller CTRL register definition */
#define XNVM_BBRAM_CTRL_START_ZEROIZE		(0x01U << 0U)

/* BBRAM Controller PGM_MODE register definition */
#define XNVM_EFUSE_PGM_MODE_PASSCODE		(0x757BDF0DU)

/* BBRAM Controller SLV_ERR_CTRL register definition */
#define XNVM_BBRAM_SLV_ERR_CTRL_ENABLE		(0x01U << 0U)

/* BBRAM Controller SLV_ERR_ISR register definition */
#define XNVM_BBRAM_SLV_ERR_ISR_ERR		(0x01U << 0U)

/* BBRAM Controller SLV_ERR_IMR register definition */
#define XNVM_BBRAM_SLV_ERR_IMR_ERR		(0x01U << 0U)

/* BBRAM Controller SLV_ERR_IER register definition */
#define XNVM_BBRAM_SLV_ERR_IER_ERR		(0x01U << 0U)

/* BBRAM Controller SLV_ERR_IDR register definition */
#define XNVM_BBRAM_SLV_ERR_IDR_ERR		(0x01U << 0U)

/* BBRAM Controller MSW_LOCK register definition */
#define XNVM_BBRAM_MSW_LOCK			(0x01U << 0U)

/* Timeout in term of number of times status register polled to check BBRAM
 * is set to programming mode
 */
#define XNVM_BBRAM_PGM_MODE_TIMEOUT_VAL		(0x1FFFFU)

/* Timeout in term of number of times status register polled to check BBRAM
 * AES CRC validation is complete
 */
#define XNVM_BBRAM_AES_CRC_DONE_TIMEOUT_VAL	(0x1FFFFU)

/* Timeout in term of number of times status register polled to check BBRAM
 * AES CRC validation is complete
 */
#define XNVM_BBRAM_ZEROIZE_TIMEOUT_VAL		(0x1FFFFU)

/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/
#define XNvm_BbramReadReg(Offset) \
	Xil_In32(XNVM_BBRAM_BASE_ADDR + Offset)

#define XNvm_BbramWriteReg(Offset, Data) \
	Xil_Out32(XNVM_BBRAM_BASE_ADDR + Offset, Data)

/*************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif