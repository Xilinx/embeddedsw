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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xecdsa_rsa_hw.h
*
* This header file contains RSA ECDSA core hardware register offsets of versal.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     02/14/19 First Release
* </pre>
*
******************************************************************************/
#ifndef XSECURE_ECDSA_RSA_H_
#define XSECURE_ECDSA_RSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/* ECDSA RSA core's base Address */
#define XSECURE_ECDSA_RSA_BASEADDR			(0xF1200000U)
					/**< Base address of ECDSA RSA core */

/* Register offsets */
/**
 * Register: RAM data register
 */
#define XSECURE_ECDSA_RSA_RAM_DATA_OFFSET		(0x00000000U)

/**
 * Register: RAM address register
 */
#define XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET		(0x00000004U)
#define XSECURE_ECDSA_RSA_RAM_ADDR_WRRD_B_MASK		(0x80000000U)
#define XSECURE_ECDSA_RSA_RAM_ADDR_ADDR_MASK		(0x7FFFFFFFU)

/**
 * Register: Control register
 */
#define XSECURE_ECDSA_RSA_CTRL_OFFSET			(0x00000008U)

#define XSECURE_ECDSA_RSA_CTRL_CLR_DATA_BUF_SHIFT	(7U)
#define XSECURE_ECDSA_RSA_CTRL_CLR_DATA_BUF_MASK	(0x00000080U)

#define XSECURE_ECDSA_RSA_CTRL_CLR_DONE_ABORT_SHIFT	(6U)
#define XSECURE_ECDSA_RSA_CTRL_CLR_DONE_ABORT_MASK	(0x00000040U)

#define XSECURE_ECDSA_RSA_CTRL_OPCODE_MASK		(0x00000007U)

/**
 * Register: Status Register
 */
#define XSECURE_ECDSA_RSA_STATUS_OFFSET			(0x0000000CU)

#define XSECURE_ECDSA_RSA_STATUS_PROG_CNT_SHIFT		(3U)
#define XSECURE_ECDSA_RSA_STATUS_PROG_CNT_MASK		(0x000000F8U)

#define XSECURE_ECDSA_RSA_STATUS_ERR_SHIFT		(2U)
#define XSECURE_ECDSA_RSA_STATUS_ERR_MASK		(0x00000004U)

#define XSECURE_ECDSA_RSA_STATUS_BUSY_SHIFT		(1U)
#define XSECURE_ECDSA_RSA_STATUS_BUSY_MASK		(0x00000002U)

#define XSECURE_ECDSA_RSA_STATUS_DONE_MASK		(0x00000001U)

/**
 * Register: MINV value register
 */
#define XSECURE_ECDSA_RSA_MINV_OFFSET			(0x00000010U)

/**
 * Register: Key Length register
 */
#define XSECURE_ECDSA_RSA_KEY_LEN_OFFSET		(0x00000020U)

#define XSECURE_ECDSA_RSA_KEY_LEN_BIN_PRIME_SHIFT	(15U)
#define XSECURE_ECDSA_RSA_KEY_LEN_BIN_PRIME_MASK	(0x00008000U)

#define XSECURE_ECDSA_RSA_KEY_LEN_MASK			(0x00007FFFU)

/**
 * Register: CFG 0
 */
#define XSECURE_ECDSA_RSA_CFG0_OFFSET			(0x00000028U)

#define XSECURE_ECDSA_RSA_CFG0_QSEL_SHIFT		(6U)
#define XSECURE_ECDSA_RSA_CFG0_QSEL_MASK		(0x000000C0U)

#define XSECURE_ECDSA_RSA_CFG0_MULTI_PASS_MASK		(0x0000003FU)

/**
 * Register: CFG 1
 */
#define XSECURE_ECDSA_RSA_CFG1_OFFSET			(0x0000002CU)

#define XSECURE_ECDSA_RSA_CFG1_MONT_DIGIT_MASK		(0x000000FFU)

/**
 * Register: CFG 2
 */
#define XSECURE_ECDSA_RSA_CFG2_OFFSET			(0x00000030U)
#define XSECURE_ECDSA_RSA_CFG2_MEM_LOC_SIZE_MASK	(0x0000001FU)

/**
 * Register: CFG 3
 */
#define XSECURE_ECDSA_RSA_CFG3_OFFSET			(0x00000034U)

#define XSECURE_ECDSA_RSA_CFG3_MONT_MOD_SHIFT		(4U)
#define XSECURE_ECDSA_RSA_CFG3_MONT_MOD_MASK		(0x000000F0U)

#define XSECURE_ECDSA_RSA_CFG3_SCRATCH_MASK		(0x0000000FU)

/**
 * Register: XSECURE_ECDSA_RSA_CFG4
 */
#define XSECURE_ECDSA_RSA_CFG4_OFFSET			(0x00000038U)
#define XSECURE_ECDSA_RSA_CFG4_START_ADDR_MASK		(0x000000FFU)

/**
 * Register: XSECURE_ECDSA_RSA_CFG5
 */
#define XSECURE_ECDSA_RSA_CFG5_OFFSET			(0x0000003CU)

#define XSECURE_ECDSA_RSA_CFG5_NO_GROUPS_MASK		(0x0000001FU)

/**
 * Register: XSECURE_ECDSA_RSA_RESET
 */
#define XSECURE_ECDSA_RSA_RESET_OFFSET			(0x00000040U)

/**
 * Register: XSECURE_ECDSA_RSA_APB_SLAVE_ERR_CTRL
 */
#define XSECURE_ECDSA_RSA_APB__ERR_CTRL_OFFSET		(0x00000044U)

/**
 * Register: XSECURE_ECDSA_RSA_RSA_CFG
 */
#define XSECURE_ECDSA_RSA_CFG_OFFSET			(0x00000058U)

#define XSECURE_ECDSA_RSA_CFG_RD_ENDIANNESS_MASK	(0x00000002U)
#define XSECURE_ECDSA_RSA_RSA_CFG_WR_ENDIANNESS_MASK	(0x00000001U)

/**
 * Register: XSECURE_ECDSA_RSA_ECO
 */
#define XSECURE_ECDSA_RSA_ECO_OFFSET			(0x00000060U)

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ECDSA_RSA_H_ */
