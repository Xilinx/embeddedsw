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
/*****************************************************************************/
/**
*
* @file xpuf_hw.h
*
* This file contains PUF hardware interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  08/01/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPUF_HW_H
#define XPUF_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/

#include "xil_io.h"

/*************************** Constant Definitions *****************************/
/*
 * PMC_GLOBAL Base Address
 */
#define XPUF_PMC_GLOBAL_BASEADDR			(0xF1110000)

/* Register: PMC_GLOBAL_PMC_BOOT_ERR */
#define XPUF_PMC_GLOBAL_PMC_BOOT_ERR_OFFSET		(0x00020100)

/* Register: XPUF_PMC_GLOBAL_PUF_CMD */
#define XPUF_PMC_GLOBAL_PUF_CMD_OFFSET			(0x00040000)

/* Register: XPUF_PMC_GLOBAL_PUF_CFG0 */
#define XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET			(0x00040004)

/* Register: XPUF_PMC_GLOBAL_PUF_CFG1 */
#define XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET			(0x00040008)

/* Register: XPUF_PMC_GLOBAL_PUF_SHUT */
#define XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET			(0x0004000C)

/* Register: XPUF_PMC_GLOBAL_PUF_STATUS */
#define XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET		(0x00040010)

/* Register: XPUF_PMC_GLOBAL_PUF_WORD */
#define XPUF_PMC_GLOBAL_PUF_WORD_OFFSET			(0x00040018)

/* Register: XPUF_PMC_GLOBAL_PUF_SYN_ADDR */
#define XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET		(0x00040020)

/* Register: XPUF_PMC_GLOBAL_PUF_AUX */
#define XPUF_PMC_GLOBAL_PUF_AUX_OFFSET			(0x00040024)

/* Register: XPUF_PMC_GLOBAL_PUF_CHASH */
#define XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET		(0x00040028)

/* Register: XPUF_PMC_GLOBAL_PUF_CLEAR */
#define XPUF_PMC_GLOBAL_PUF_CLEAR_OFFSET		(0x0004002C)

/* Register: XPUF_PMC_GLOBAL_PUF_ID_0 */
#define XPUF_PMC_GLOBAL_PUF_ID_0_OFFSET			(0x00040030)

/* Register: XPUF_PMC_GLOBAL_PUF_SEG_STATUS_0 */
#define XPUF_PMC_GLOBAL_PUF_SEG_STATUS_0_OFFSET		(0x00040050)

/* PUF COMMAND register definition */
#define XPUF_CMD_PAUSE				(0x00U)
#define XPUF_CMD_PROVISION			(0x01U)
#define XPUF_CMD_REGENERATION			(0x02U)
#define XPUF_CMD_STOP				(0x03U)
#define XPUF_CMD_TEST_MODE_ENABLE		(0x01U << 31U)

/* PUF CFG0 register definition */
#define XPUF_CFG0_GLOBAL_FILTER_ENABLE		(0x01U)
#define XPUF_CFG0_HASH_SEL			(0x02U)

/* PUF CFG1 register definition */
#define XPUF_CFG1_INIT_VAL_4K			(0x0C230090U)
#define XPUF_CFG1_INIT_VAL_12K			(0x00230150U)

/* PUF Registration mode */
#define XPUF_REGISTRATION_MODE_4K		(0x0U)
#define XPUF_REGISTRATION_MODE_12K		(0x1U)

/* PUF SHUT register definition */
#define XPUF_SHUTTER_OPEN_TIME_MASK		(0xFFFFFFU)
#define XPUF_SHUTTER_OPEN_SET_SHIFT		(24U)

/* PUF STATUS register definition */
#define XPUF_STATUS_SYNDROME_WORD_RDY		(0x01U << 0U)
#define XPUF_STATUS_ID_ZEROIZED			(0x01U << 1U)
#define XPUF_STATUS_ID_RDY			(0x01U << 2U)
#define XPUF_STATUS_KEY_RDY			(0x01U << 3U)
#define XPUF_STATUS_PUF_DONE			(0x01U << 30)

/* PUF TM STATUS register definition */
#define XPUF_TEST_MODE_STATUS_DONE		(0x01U)

/* PUF TM TR register definition */
#define XPUF_TEST_MODE_RESULT_FRR_TEST_PASSED	(0x01U << 0U)
#define XPUF_TEST_MODE_RESULT_ER_TEST_MASK	(0xFFU << 16U)
#define XPUF_TEST_MODE_RESULT_US_MASK		(0x03U << 24U)

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
 * 		u32 XilPuf_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 * ******************************************************************************/

#define XPuf_ReadReg(BaseAddress, RegOffset) \
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
 * 		void XilPuf_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 * ******************************************************************************/

#define XPuf_WriteReg(BaseAddress, RegOffset, Data) \
		Xil_Out32(((BaseAddress) + (u32)(RegOffset)), (u32)(Data))


#ifdef __cplusplus
}
#endif

#endif /* XPUF_HW_H */
