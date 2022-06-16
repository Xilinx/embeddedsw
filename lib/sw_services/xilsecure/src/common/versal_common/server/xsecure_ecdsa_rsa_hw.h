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
* 4.1   kpt     01/07/20 Added macro XSECURE_ECDSA_RSA_CFG_REVERT_ENDIANNESS_MASK
* 4.3   ana     06/06/20 Renamed XSECURE_ECDSA_RSA_CFG_REVERT_ENDIANNESS_MASK as
*                        XSECURE_ECDSA_RSA_CFG_CLEAR_ENDIANNESS_MASK
*       am      09/24/20 Resolved MISRA C violations
*       har     10/12/20 Addressed security review comments
* 4.6   har     07/14/21 Fixed doxygen warnings
*
* </pre>
*
******************************************************************************/
#ifndef XSECURE_ECDSA_RSA_HW_H_
#define XSECURE_ECDSA_RSA_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define XSECURE_ECDSA_RSA_BASEADDR			(0xF1200000U)
					/**< Base address of ECDSA RSA core */

#define XSECURE_ECDSA_RSA_RAM_DATA_OFFSET		(0x00000000U)
					/**< Offset of RAM_DATA register */

/**
 * @name  RAM_ADDR register
 * @{
 */
/**< RAM address register offset and definitions */
#define XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET		(0x00000004U)
#define XSECURE_ECDSA_RSA_RAM_ADDR_WRRD_B_MASK		(0x80000000U)
/** @} */

/**
 * @name  CTRL register
 * @{
 */
/**< CTRL register offset and definitions */
#define XSECURE_ECDSA_RSA_CTRL_OFFSET			(0x00000008U)
#define XSECURE_ECDSA_RSA_CTRL_CLR_DATA_BUF_MASK	(0x00000080U)
/** @} */

#define XSECURE_ECDSA_RSA_STATUS_OFFSET			(0x0000000CU)
						/**< Offset of Status register */

#define XSECURE_ECDSA_RSA_MINV_OFFSET			(0x00000010U)
						/**< Offset of MINV register */

#define XSECURE_ECDSA_RSA_KEY_LEN_OFFSET		(0x00000020U)
							/**< Offset of Key Len register */

#define XSECURE_ECDSA_RSA_CFG0_OFFSET			(0x00000028U)
							/**< Offset of CFG0 register */

#define XSECURE_ECDSA_RSA_CFG1_OFFSET			(0x0000002CU)
							/**< Offset of CFG1 register */

#define XSECURE_ECDSA_RSA_CFG2_OFFSET			(0x00000030U)
							/**< Offset of CFG2 register */

#define XSECURE_ECDSA_RSA_CFG5_OFFSET			(0x0000003CU)
							/**< Offset of CFG5 register */

#define XSECURE_ECDSA_RSA_RESET_OFFSET			(0x00000040U)
							/**< Offset of RESET register */

/**
 * @name  CFG register
 * @{
 */
/**< CFG register offset and definitions */
#define XSECURE_ECDSA_RSA_CFG_OFFSET			(0x00000058U)
#define XSECURE_ECDSA_RSA_CFG_RD_ENDIANNESS_MASK	(0x00000002U)
#define XSECURE_ECDSA_RSA_RSA_CFG_WR_ENDIANNESS_MASK	(0x00000001U)
#define XSECURE_ECDSA_RSA_CFG_CLEAR_ENDIANNESS_MASK	(0U)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ECDSA_RSA_HW_H_ */
