/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 4.1   kpt     01/07/20 Added MACRO
*                        XSECURE_ECDSA_RSA_CFG_REVERT_ENDIANNESS_MASK
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
#define XSECURE_ECDSA_RSA_CFG_REVERT_ENDIANNESS_MASK	(0U)

/**
 * Register: XSECURE_ECDSA_RSA_ECO
 */
#define XSECURE_ECDSA_RSA_ECO_OFFSET			(0x00000060U)

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ECDSA_RSA_H_ */
