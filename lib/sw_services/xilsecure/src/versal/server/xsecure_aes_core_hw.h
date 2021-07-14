/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes_core_hw.h
* This file contains AES core hardware definitions of versal.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/21/19 Initial release
* 4.2   vns  02/10/20 Added efuse cache DPA mask register address and mask
* 4.3   ana  06/04/20 Added XSECURE_AES_MODE_DEC macro
*       kpt  08/06/20 Added XSECURE_AES_KEY_DEC_RESET_MASK macro
*       am   09/24/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
* 4.5   har  03/02/21 Added register offset for AAD
* 4.6   har  07/14/21 Fixed doxygen warnings
*
* </pre>
*
* @endcond
******************************************************************************/

#ifndef XSECURE_AES_CORE_HW_H
#define XSECURE_AES_CORE_HW_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions ****************************/
#define XSECURE_AES_BASEADDR				(0xF11E0000U)
						/**< AES Base Address */

/**
 * @name  AES_STATUS register
 * @{
 */
/**< AES_STATUS register offset and definitions */
#define XSECURE_AES_STATUS_OFFSET			(0x00000000U)

#define XSECURE_AES_STATUS_CM_ENABLED_SHIFT		(12U)
#define XSECURE_AES_STATUS_CM_ENABLED_MASK		(0x00001000U)

#define XSECURE_AES_STATUS_BLK_KEY_DEC_DONE_MASK	(0x00000020U)

#define XSECURE_AES_STATUS_KEY_INIT_DONE_MASK		(0x00000010U)

#define XSECURE_AES_STATUS_GCM_TAG_PASS_MASK		(0x00000008U)

#define XSECURE_AES_STATUS_DONE_MASK			(0x00000004U)
/** @} */

#define XSECURE_AES_KEY_SEL_OFFSET			(0x00000004U)
				/**< AES_KEY_SEL register offset */

/**
 * @name  AES_KEY_LOAD register
 * @{
 */
/**< AES_KEY_LOAD register offset  and definitions */
#define XSECURE_AES_KEY_LOAD_OFFSET			(0x00000008U)
#define XSECURE_AES_KEY_LOAD_VAL_MASK			(0x00000001U)
/** @} */

/**
 * @name  AES_START_MSG register
 * @{
 */
/**< AES_START_MSG register offset  and definitions */
#define XSECURE_AES_START_MSG_OFFSET			(0x0000000CU)
#define XSECURE_AES_START_MSG_VAL_MASK			(0x00000001U)
/** @} */

#define XSECURE_AES_SOFT_RST_OFFSET			(0x00000010U)
				/**< AES_KEY_SOFT_RST register offset */

/**
 * @name  AES_KEY_CLEAR register
 * @{
 */
/**< AES_KEY_CLEAR register offset and definitions */
#define XSECURE_AES_KEY_CLEAR_OFFSET			(0x00000014U)

#define XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK		(0x00200000U)

#define XSECURE_AES_KEY_CLEAR_BBRAM_RED_KEY_MASK	(0x00100000U)

#define XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK		(0x00080000U)

#define XSECURE_AES_KEY_CLEAR_BH_KEY_MASK		(0x00040000U)

#define XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_1_MASK	(0x00020000U)

#define XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_0_MASK	(0x00010000U)

#define XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK	(0x00008000U)

#define XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_1_MASK	(0x00004000U)

#define XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_0_MASK	(0x00002000U)

#define XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK		(0x00001000U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_7_MASK		(0x00000800U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK		(0x00000400U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK		(0x00000200U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK		(0x00000100U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK		(0x00000080U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK		(0x00000040U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK		(0x00000020U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK		(0x00000010U)

#define XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK		(0x00000002U)

#define XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK	(0x00000001U)

#define XSECURE_AES_KEY_CLR_REG_CLR_MASK		(0x00000000U)

#define XSECURE_AES_KEY_CLEAR_ALL_KEYS_MASK		(0x003FFFF3U)
/** @} */

/**
 * @name  AES_MODE register
 * @{
 */
/**< AES_MODE register offset and definitions */
#define XSECURE_AES_MODE_OFFSET				(0x00000018U)

#define XSECURE_AES_MODE_ENC				(0x00000001U)
#define XSECURE_AES_MODE_DEC				(0x00000000U)
/** @} */

/**
 * @name  AES_KUP_WR register
 * @{
 */
/**< AES_KUP_WR register offset and definitions */
#define XSECURE_AES_KUP_WR_OFFSET			(0x0000001CU)

#define XSECURE_AES_KUP_WR_IV_SAVE_MASK			(0x00000002U)

#define XSECURE_AES_KUP_WR_KEY_SAVE_MASK		(0x00000001U)
/** @} */

#define XSECURE_AES_IV_0_OFFSET				(0x00000040U)
				/**< AES_IV_0 register offset */

#define XSECURE_AES_IV_3_OFFSET				(0x0000004CU)
				/**< AES_IV_3 register offset */

#define XSECURE_AES_KEY_SIZE_OFFSET			(0x00000050U)
				/**< AES_KEY_SIZE register offset */

/**
 * @name  AES_KEY_DEC register
 * @{
 */
/**< AES_KEY_DEC register offset and definitions */
#define XSECURE_AES_KEY_DEC_OFFSET			(0x00000058U)
#define XSECURE_AES_KEY_DEC_MASK			(0xFFFFFFFFU)
#define XSECURE_AES_KEY_DEC_RESET_MASK			(0X00000000U)
/** @} */

#define XSECURE_AES_KEY_DEC_TRIG_OFFSET			(0x0000005CU)
				/**< AES_KEY_DEC_TRIG register offset */

#define XSECURE_AES_KEY_DEC_SEL_OFFSET			(0x00000060U)
				/**< AES_KEY_DEC_SEL register offset */

#define XSECURE_AES_KEY_ZEROED_STATUS_OFFSET		(0x00000064U)
				/**< AES_KEY_ZEROED register offset */

#define XSECURE_AES_AAD_OFFSET				(0x0000006CU)
				/**< AES_AAD register offset */

/**
 * @name  AES_CM_EN register
 * @{
 */
/**< AES_CM_EN register offset and definitions */
#define XSECURE_AES_CM_EN_OFFSET			(0x0000007CU)
#define XSECURE_AES_CM_EN_VAL_MASK    			(0x00000001U)
/** @} */

/**
 * @name  AES_SPLIT_CFG register
 * @{
 */
/**< AES_SPLIT_CFG register offset and definitions */
#define XSECURE_AES_SPLIT_CFG_OFFSET			(0x00000080U)

#define XSECURE_AES_SPLIT_CFG_KEY_SPLIT			(0x00000002U)

#define XSECURE_AES_SPLIT_CFG_DATA_SPLIT		(0x00000001U)
#define XSECURE_AES_SPLIT_CFG_DATA_KEY_DISABLE		(0U)
/** @} */

/**
 * @name  AES_DATA_SWAP register
 * @{
 */
/**< AES_DATA_SWAP register offset and definitions */
#define XSECURE_AES_DATA_SWAP_OFFSET			(0x00000084U)
#define XSECURE_AES_DATA_SWAP_VAL_MASK			(0x00000001U)
#define XSECURE_AES_DATA_SWAP_VAL_DISABLE		(0x00000000U)
/** @} */

#define XSECURE_AES_BH_KEY_0_OFFSET			(0x000000F0U)
			/**< AES_BH_KEY_0 register offset */
#define XSECURE_AES_USER_KEY_0_0_OFFSET			(0x00000110U)
			/**< AES_USER_KEY_0_0 register offset */
#define XSECURE_AES_USER_KEY_1_0_OFFSET			(0x00000130U)
			/**< AES_USER_KEY_1_0 register offset */
#define XSECURE_AES_USER_KEY_2_0_OFFSET			(0x00000150U)
			/**< AES_USER_KEY_2_0 register offset */
#define XSECURE_AES_USER_KEY_3_0_OFFSET			(0x00000170U)
			/**< AES_USER_KEY_3_0 register offset */
#define XSECURE_AES_USER_KEY_4_0_OFFSET			(0x00000190U)
			/**< AES_USER_KEY_4_0 register offset */
#define XSECURE_AES_USER_KEY_5_0_OFFSET			(0x000001B0U)
			/**< AES_USER_KEY_5_0 register offset */
#define XSECURE_AES_USER_KEY_6_0_OFFSET			(0x000001D0U)
			/**< AES_USER_KEY_6_0 register offset */
#define XSECURE_AES_USER_KEY_7_0_OFFSET			(0x000001F0U)
			/**< AES_USER_KEY_7_0 register offset */

/**
 * @name  SECURITY_MISC_1 register in EFUSE_CACHE module
 * @{
 */
/**< SECURITY_MISC_1 register offset and definitions */
#define XSECURE_EFUSE_SECURITY_MISC1			(0xF12500E8U)
#define XSECURE_EFUSE_DPA_CM_DIS_MASK			(0xFFFF0000U)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AES_CORE_HW_H */
