/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_efuse_hw.h
*
* This file contains NVM library eFUSE controller register definitions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   kal  08/16/2019 Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/
#ifndef XNVM_EFUSE_HW_H
#define XNVM_EFUSE_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_io.h"

/*************************** Constant Definitions *****************************/

/* CRP base address definition */
#define XNVM_CRP_BASE_ADDR				(0xF1260000U)

/* CRP eFUSE Clock Control Register Map */
#define XNVM_CRP_EFUSE_REF_CLK_REG_OFFSET		(0x00000134U)

#define XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT		(2U)
#define XNVM_CRP_EFUSE_REF_CLK_IN			(0x01 <<\
						XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT)
#define XNVM_CRP_EFUSE_REF_CLK_IRO_CLK_BY_4		(0x00 <<\
						XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT)

#define XNVM_CRP_EFUSE_REF_CLK_SELSRC			(XNVM_CRP_EFUSE_REF_CLK_IN)


/* eFUSE Controller base address definition */
#define XNVM_EFUSE_BASE_ADDR				(0xF1240000U)

/* eFUSE COntroller Register Map definition */
#define XNVM_EFUSE_WR_LOCK_REG_OFFSET			(0x00000000U)
#define XNVM_EFUSE_CFG_REG_OFFSET			(0x00000004U)
#define XNVM_EFUSE_STATUS_REG_OFFSET			(0x00000008U)
#define XNVM_EFUSE_PGM_ADDR_REG_OFFSET			(0x0000000CU)
#define XNVM_EFUSE_RD_ADDR_REG_OFFSET			(0x00000010U)
#define XNVM_EFUSE_RD_DATA_REG_OFFSET			(0x00000014U)
#define XNVM_EFUSE_TPGM_REG_OFFSET			(0x00000018U)
#define XNVM_EFUSE_TRD_REG_OFFSET			(0x0000001CU)
#define XNVM_EFUSE_TSU_H_PS_REG_OFFSET			(0x00000020U)
#define XNVM_EFUSE_TSU_H_PS_CS_REG_OFFSET		(0x00000024U)
#define XNVM_EFUSE_TRDM_REG_OFFSET			(0x00000028U)
#define XNVM_EFUSE_TSU_H_CS_REG_OFFSET			(0x0000002CU)
#define XNVM_EFUSE_ISR_REG_OFFSET			(0x00000030U)
#define XNVM_EFUSE_IMR_REG_OFFSET			(0x00000034U)
#define XNVM_EFUSE_IER_REG_OFFSET			(0x00000038U)
#define XNVM_EFUSE_IDR_REG_OFFSET			(0x0000003CU)
#define XNVM_EFUSE_CACHE_LOAD_REG_OFFSET		(0x00000040U)
#define XNVM_EFUSE_PGM_LOCK_REG_OFFSET			(0x00000044U)
#define XNVM_EFUSE_AES_CRC_REG_OFFSET			(0x00000048U)
#define XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET		(0x0000004CU)
#define XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET		(0x00000050U)
#define XNVM_EFUSE_PD_REG_OFFSET			(0x00000054U)
#define XNVM_EFUSE_ALG_OSC_SW_1LP_REG_OFFSET		(0x00000060U)
#define XNVM_EFUSE_ECO_REG_OFFSET			(0x000000FCU)
#define XNVM_EFUSE_TEST_CTRL_REG_OFFSET			(0x00000100U)
#define XNVM_EFUSE_TEST_PARITY_E0_A_REG_OFFSET		(0x00000104U)
#define XNVM_EFUSE_TEST_PARITY_E0_B_REG_OFFSET		(0x00000108U)
#define XNVM_EFUSE_TEST_BITS_ADDR_REG_OFFSET		(0x00000140U)
#define XNVM_EFUSE_TEST_BITS_DATA_REG_OFFSET		(0x00000144U)


/*  EFUSE_CACHE Base Address */
#define XNVM_EFUSE_CACHE_BASEADDR			(0xF1250000U)

/* EFUSE ROWS */
#define XNVM_EFUSE_TBITS_ROW				(0U)
#define XNVM_EFUSE_PUF_CHASH_ROW			(42U)
#define XNVM_EFUSE_PUF_AUX_ROW				(41U)

/* EFUSE COLUMNS */
#define XNVM_EFUSE_TBITS_0_COLUMN			(28U)
#define XNVM_EFUSE_TBITS_1_COLUMN                       (29U)
#define XNVM_EFUSE_TBITS_2_COLUMN                       (30U)
#define XNVM_EFUSE_TBITS_3_COLUMN			(31U)

/* eFUSE Controller WR_LOCK register definition */
#define XNVM_EFUSE_WR_UNLOCK_PASSCODE			(0xDF0DU)

/* eFUSE Controller CFG register definition */
#define XNVM_EFUSE_CFG_ENABLE_PGM			(0x01U << 1U)
#define XNVM_EFUSE_CFG_MARGIN_RD			(0x01U << 2U)
#define XNVM_EFUSE_CFG_NORMAL_RD			(0x00U << 2U)
#define XNVM_EFUSE_CFG_ENABLE_SLV_ERR			(0x01U << 5U)

/* eFUSE Controller STATUS register definition */
#define XNVM_EFUSE_STATUS_TBIT_0			(0x01U << 0U)
#define XNVM_EFUSE_STATUS_TBIT_1			(0x01U << 1U)
#define XNVM_EFUSE_STATUS_TBIT_2			(0x01U << 2U)
#define XNVM_EFUSE_STATUS_CACHE_LOAD			(0x01U << 4U)
#define XNVM_EFUSE_STATUS_CACHE_DONE			(0x01U << 5U)
#define XNVM_EFUSE_STATUS_AES_CRC_DONE			(0x01U << 6U)
#define XNVM_EFUSE_STATUS_AES_CRC_PASS			(0x01U << 7U)
#define XNVM_EFUSE_STATUS_AES_USR_KEY0_CRC_DONE 	(0x01U << 8U)
#define XNVM_EFUSE_STATUS_AES_USR_KEY0_CRC_PASS 	(0x01U << 9U)
#define XNVM_EFUSE_STATUS_AES_USR_KEY1_CRC_DONE 	(0x01U << 10U)
#define XNVM_EFUSE_STATUS_AES_USR_KEY1_CRC_PASS 	(0x01U << 11U)

/* eFUSE Controller PGM_ADDR register definition */
#define XNVM_EFUSE_ADDR_COLUMN_SHIFT			(0U)
#define XNVM_EFUSE_ADDR_ROW_SHIFT			(5U)
#define XNVM_EFUSE_ADDR_PAGE_SHIFT			(13U)

/* eFUSE Controller TPGM register definition */
#define XNVM_EFUSE_TPGM_MASK				(0x0000FFFFU)

/* eFUSE Controller TRD register definition */
#define XNVM_EFUSE_TRD_MASK				(0x000000FFU)

/* eFUSE Controller TSU_H_PS register definition */
#define XNVM_EFUSE_TSU_H_PS_MASK			(0x000000FFU)

/* eFUSE Controller TRDM register definition */
#define XNVM_EFUSE_TRDM_MASK				(0x000000FFU)

/* eFUSE Controller TSU_H_CS register definition */
#define XNVM_EFUSE_TSU_H_CS_MASK			(0x000000FFU)

/* eFUSE Controller ISR register definition */
#define XNVM_EFUSE_ISR_MASK				(0x80007FFFU)

/* eFUSE Controller TBITS PGM ENABLE MASK and Shift */
#define XNVM_EFUSE_TBITS_PRGRMG_EN_MASK			(0x01U << 5U)
#define XNVM_EFUSE_TBITS_MASK				(0xFU)
#define XNVM_EFUSE_TBITS_SHIFT				(27U)

/* eFUse Cache load mask */
#define XNVM_EFUSE_CACHE_LOAD_MASK			(0x01U)

/* eFuse ISR registers masks */
#define XNVM_EFUSE_ISR_PGM_DONE				(0x01 << 0U)
#define XNVM_EFUSE_ISR_PGM_ERROR			(0x01 << 1U)
#define XNVM_EFUSE_ISR_RD_DONE				(0x01 << 2U)
#define XNVM_EFUSE_ISR_RD_ERROR				(0x01 << 3U)
#define XNVM_EFUSE_ISR_CACHE_ERROR			(0x01 << 4U)
#define XNVM_EFUSE_ISR_MAIN_FSM_ERROR			(0x01 << 5U)
#define XNVM_EFUSE_ISR_CACHE_FSM_ERROR			(0x01 << 6U)
#define XNVM_EFUSE_ISR_RD_ON_CACHE_LD			(0x01 << 7U)
#define XNVM_EFUSE_ISR_MAIN_REQ_ERROR			(0x01 << 8U)
#define XNVM_EFUSE_ISR_CACHE_REQ_ERROR			(0x01 << 9U)
#define XNVM_EFUSE_ISR_CACHE_APB_SLV_ERROR		(0x01 << 10U)
#define XNVM_EFUSE_ISR_CACHE_PARITY_EOR			(0x01 << 11U)
#define XNVM_EFUSE_ISR_CACHE_PARITY_EOS			(0x01 << 12U)
#define XNVM_EFUSE_ISR_CACHE_PARITY_E1			(0x01 << 13U)
#define XNVM_EFUSE_ISR_CACHE_PARITY_E2			(0x01 << 14U)
#define XNVM_EFUSE_ISR_APB_SLV_ERROR			(0x01 << 31U)


/* eFUSE Controller PGM_LOCK register definition */
#define XNVM_EFUSE_PGM_LOCK_SPK_ID			(0x01U << 0U)

/* eFUSE Controller PD register definition */
#define XNVM_EFUSE_PD_ENABLE				(0x01U << 0U)

/* PS Reference clock definition in Hz. */
#define XNVM_PS_REF_CLK_FREQ				(33.333333e6) /* Hz */

/* Timeout in term of number of times status register polled to check eFUSE
 * read operation complete
 */
#define XNVM_EFUSE_RD_TIMEOUT_VAL			(100U)

/* Timeout in term of number of times status register polled to check eFUSE
 * programming operation complete
 */
#define XNVM_EFUSE_PGM_TIMEOUT_VAL			(100U)

/* Timeout in term of number of times status register polled to check eFuse
 * Cache load is done
 */
#define XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL		(0x1FFFFFFFU)

/***************** Macros (Inline Functions) Definitions ********************/

/*****************************************************************************/
/*
 *
 * This macro reads the given register.
 *
 * @param	BaseAddress is the Xilinx base address of the eFuse or Bbram
 *		controller.
 * @param	RegOffset is the register offset of the register.
 *
 * @return	The 32-bit value of the register.
 *
 * @note	C-style signature:
 * 		u32 Xnvm_Efuse_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 * ******************************************************************************/

#define XNvm_Efuse_ReadReg(BaseAddress, RegOffset) \
		Xil_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/*
 *
 * This macro writes the value into the given register.
 *
 * @param	BaseAddress is the Xilinx base address of the eFuse or Bbram
 *		controller.
 * @param	RegOffset is the register offset of the register.
 * @param	Data is the 32-bit value to write to the register.
 *
 * @return	None.
 *
 * @note	C-style signature:
 * 		void XNvm_Efuse_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 * ******************************************************************************/

#define XNvm_Efuse_WriteReg(BaseAddress, RegOffset, Data) \
		Xil_Out32(((BaseAddress) + (u32)(RegOffset)), (u32)(Data))


#ifdef __cplusplus
}
#endif

#endif		/* XNVM_EFUSE_HW_H */
