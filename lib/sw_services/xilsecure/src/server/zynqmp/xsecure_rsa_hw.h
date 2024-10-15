/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_hw.h
* This file contains ZynqMP RSA hardware core register offsets.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/09/19 Initial release
* 4.1   kpt  01/07/20 Added Macro XSECURE_RSA_CTRL_CLR_DATA_BUF_MASK
*
* </pre>
*
******************************************************************************/
#ifndef XSECURE_RSA_HW_H
#define XSECURE_RSA_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

#define XSECURE_CSU_RSA_BASE			(0xFFCE0000U)
					/**< Base address of RSA core */

/** @name Register Map
 *
 * Register offsets for the RSA module.
 * @{
 */
#define XSECURE_CSU_RSA_WRITE_DATA_OFFSET	(0x00U)
						/**< RAM write data offset */
#define XSECURE_CSU_RSA_WRITE_ADDR_OFFSET	(0x04U)
						/**< RAM write address offset */
#define XSECURE_CSU_RSA_READ_DATA_OFFSET	(0x08U)
						/**< RAM data read offset */
#define XSECURE_CSU_RSA_READ_ADDR_OFFSET	(0x0CU)
						/**< RAM read offset */
#define XSECURE_CSU_RSA_CONTROL_OFFSET		(0x10U)
						/**< RSA Control Reg */

#define XSECURE_CSU_RSA_STATUS_OFFSET		(0x14U)
						/**< Status Register */

#define XSECURE_CSU_RSA_MINV0_OFFSET		(0x18U)
					/**< RSA MINV(Mod 32 Inverse) 0 */
#define XSECURE_CSU_RSA_MINV1_OFFSET		(0x1CU)
						/**< RSA MINV 1 */
#define XSECURE_CSU_RSA_MINV2_OFFSET		(0x20U) /**< RSA MINV 2 */
#define XSECURE_CSU_RSA_MINV3_OFFSET		(0x24U) /**< RSA MINV 3 */
#define XSECURE_CSU_RSA_ZERO_OFFSET			(0x28U) /**< RSA Zero offset */
#define XSECURE_CSU_RSA_WR_DATA_0_OFFSET	(0x2cU) /**< Write Data 0 */
#define XSECURE_CSU_RSA_WR_DATA_1_OFFSET	(0x30U) /**< Write Data 1 */
#define XSECURE_CSU_RSA_WR_DATA_2_OFFSET	(0x34U) /**< Write Data 2 */
#define XSECURE_CSU_RSA_WR_DATA_3_OFFSET	(0x38U) /**< Write Data 3 */
#define XSECURE_CSU_RSA_WR_DATA_4_OFFSET	(0x3cU) /**< Write Data 4 */
#define XSECURE_CSU_RSA_WR_DATA_5_OFFSET	(0x40U) /**< Write Data 5 */
#define XSECURE_CSU_RSA_WR_ADDR_OFFSET		(0x44U)
					/**< Write address in RSA RAM */

#define XSECURE_CSU_RSA_RD_DATA_0_OFFSET	(0x48U) /**< Read Data 0 */
#define XSECURE_CSU_RSA_RD_DATA_1_OFFSET	(0x4cU) /**< Read Data 1 */
#define XSECURE_CSU_RSA_RD_DATA_2_OFFSET	(0x50U) /**< Read Data 2 */
#define XSECURE_CSU_RSA_RD_DATA_3_OFFSET	(0x54U) /**< Read Data 3 */
#define XSECURE_CSU_RSA_RD_DATA_4_OFFSET	(0x58U) /**< Read Data 4 */
#define XSECURE_CSU_RSA_RD_DATA_5_OFFSET	(0x5cU) /**< Read Data 5 */
#define XSECURE_CSU_RSA_RD_ADDR_OFFSET		(0x60U)
						/**< Read address in RSA RAM */
#define XSECURE_RSA_CTRL_CLR_DATA_BUF_MASK	(1U) /**< RSA BUF Clr value */
/* @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_RSA_HW_H */
