/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xaes_hw.h
 *
 * This file Contains the macros for the AES hardware module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   06/26/24 Initial release
 *       am   08/24/24 Added macros related to split configuration register
 * 1.1   am   01/22/25 Added macros related to key transfer
 * 1.2   am   05/18/25 Removed duplicate macros
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XAES_HW_H_
#define XAES_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *****************************************************/

/************************** Constant Definitions *************************************************/
/**
 * @name Definitions for AES driver
 * @{
 */
#define XASU_XAES_NUM_INSTANCES				1U /**< Number of AES hardware instances */

#define XASU_XAES_0_DEVICE_ID				0U /**< AES0 device ID */
#define XASU_XAES_0_BASEADDR				0xEBE88000U /**< AES0 base address */
#define XASU_XKEY_0_BASEADDR				0xEBE8A000U /**< KEY0 vault base address */
/** @} */

/**
 * @name  AES_STATUS register
 * @{
 */
#define XAES_STATUS_OFFSET				(0x00000000U)
	/**< AES status register offset */
#define XAES_STATUS_BUSY_MASK				(0x00000001U)
	/**< AES engine is ready to accept the data from the stream swtich */
#define XAES_STATUS_READY_MASK				(0x00000002U)
	/**< AES engine is busy with an encrypt or decrypt operation */
/** @} */

/**
 * @name  AES_OPERATION register
 * @{
 */
#define XAES_OPERATION_OFFSET				(0x00000004U)
	/**< AES operation register offset */
#define XAES_KEY_LOAD_MASK				(0x00000001U)
	/**< Key load mask to load key into the AES engine */
#define XAES_IV_LOAD_MASK				(0x00000002U)
	/**< Iv load mask to load Iv into the AES engine */
#define XAES_INTMAC_LOAD_MASK				(0x00000004U)
	/**< INT_MAC load mask to load INT_MAC into the AES engine */
#define XAES_S0_LOAD_MASK				(0x00000008U)
	/**< S0_LOAD load mask to load S0_LOAD into the AES engine */
/** @} */

/**
 * @name  AES_SOFT_RST register
 * @{
 */
#define XAES_SOFT_RST_OFFSET				(0x0000000CU)
	/**< AES soft reset offset */
#define XAES_SOFT_RST_MASK				(0x00000001U)
	/**< Trigger soft reset to AES engine */
/** @} */

/**
 * @name  AES_IV_IN registers
 * @{
 */
#define XAES_IV_IN_0_OFFSET				(0x00000010U)
	/**<  IV input data bits 31 to 0 */
#define XAES_IV_IN_1_OFFSET				(0x00000014U)
	/**<  IV input data bits 63 to 32 */
#define XAES_IV_IN_2_OFFSET				(0x00000018U)
	/**<  IV input data bits 95 to 64 */
#define XAES_IV_IN_3_OFFSET				(0x0000001CU)
	/**<  IV input data bits 127 to 96 */
/** @} */

/**
 * @name  AES_IV_MASK_IN registers
 * @{
 */
#define XAES_IV_MASK_IN_0_OFFSET			(0x00000020U)
	/**<  IV_MASK input data bits 31 to 0 */
#define XAES_IV_MASK_IN_1_OFFSET			(0x00000024U)
	/**<  IV_MASK input data bits 63 to 32 */
#define XAES_IV_MASK_IN_2_OFFSET			(0x00000028U)
	/**<  IV_MASK input data bits 95 to 64 */
#define XAES_IV_MASK_IN_3_OFFSET			(0x0000002CU)
	/**<  IV_MASK input data bits 127 to 96 */
/** @} */

/**
 * @name  AES_IV_OUT registers
 * @{
 */
#define XAES_IV_OUT_0_OFFSET				(0x00000030U)
	/**<  IV output data bits 31 to 0 */
#define XAES_IV_OUT_1_OFFSET				(0x00000034U)
	/**<  IV output data bits 63 to 32 */
#define XAES_IV_OUT_2_OFFSET				(0x00000038U)
	/**<  IV output data bits 95 to 64 */
#define XAES_IV_OUT_3_OFFSET				(0x0000003CU)
	/**<  IV output data bits 127 to 96 */
/** @} */

/**
 * @name  AES_IV_MASK_OUT registers
 * @{
 */
#define XAES_IV_MASK_OUT_0_OFFSET			(0x00000040U)
	/**<  IV_MASK output data bits 31 to 0 */
#define XAES_IV_MASK_OUT_1_OFFSET			(0x00000044U)
	/**<  IV_MASK output data bits 63 to 32 */
#define XAES_IV_MASK_OUT_2_OFFSET			(0x00000048U)
	/**<  IV_MASK output data bits 95 to 64 */
#define XAES_IV_MASK_OUT_3_OFFSET			(0x0000004CU)
	/**<  IV_MASK output data bits 127 to 96 */
/** @} */

/**
 * @name  AES_KEY_DEC_TRIG register
 * @{
 */
#define XAES_KEY_DEC_TRIG_OFFSET			(0x0000005CU)
	/**<  Trigger key decryption register offset */
#define XAES_KEY_DEC_TRIG_MASK				(0x00000001U)
	/**<  Trigger key decryption mask */
#define XAES_KEY_DEC_TRIG_DISABLE			(0x00000000U)
	/**<  Key decryption clear mask */
/** @} */

/**
 * @name  AES_CM_EN register
 * @{
 */
#define XAES_CM_OFFSET					(0x00000070U)
	/**< AES counter measure offset */
#define XAES_CM_ENABLE_MASK				(0x00000007U)
	/**< Enables DPA counter measures in the AES engine */
#define XAES_CM_DISABLE					(0x00000000U)
	/**< Disables DPA counter measures in the AES engine */
/** @} */

/**
 * @name  AES_SPLIT_CFG register
 * @{
 */
#define XAES_SPLIT_CFG_OFFSET				(0x00000074U)
	/**< Offset of split configuration */
#define XAES_SPLIT_CFG_KEY_SPLIT_VALUE			(0x00000002U)
	/**< Key split value */
#define XAES_SPLIT_CFG_DATA_SPLIT_VALUE			(0x00000001U)
	/**< Data split value */
/** @} */

/**
 * @name  AES_MODE register
 * @{
 */
#define XAES_MODE_CONFIG_OFFSET				(0x00000078U)
	/**< AES mode configuration register offset */
#define XAES_MODE_CONFIG_ENC_DEC_MASK			(0x00000040U)
	/**< Encryption operation selection Mask */
#define XAES_MODE_CONFIG_ENGINE_MODE_MASK		(0x0000000FU)
	/**< AES engine mode mask */
#define XAES_MODE_CONFIG_AUTH_MASK			(0x00002000U)
	/**< Authentication of data enable mask */
#define XAES_MODE_CONFIG_AUTH_WITH_NO_PAYLOAD_MASK	(0x00004000U)
	/**< No payload with authentication data enable mask */
#define XAES_MODE_CONFIG_DISABLE			(0x00000000U)
	/**< Disable the mode configuration */
/** @} */

/**
 * @name  AES_MAC_OUT register
 * @{
 */
#define XAES_MAC_OUT_0_MASK				(0x00000080U)
	/**< MAC output data bits 31 to 0 */
#define XAES_MAC_OUT_1_MASK				(0x00000084U)
	/**< MAC output data bits 63 to 32 */
#define XAES_MAC_OUT_2_MASK				(0x00000088U)
	/**< MAC output data bits 95 to 64 */
#define XAES_MAC_OUT_3_MASK				(0x0000008CU)
	/**< MAC output data bits 127 to 96 */
#define XAES_MAC_MASK_OUT_0_MASK			(0x00000090U)
	/**< MAC_MASK output data bits 31 to 0 */
#define XAES_MAC_MASK_OUT_1_MASK			(0x00000094U)
	/**< MAC_MASK output data bits 63 to 32 */
#define XAES_MAC_MASK_OUT_2_MASK			(0x00000098U)
	/**< MAC_MASK output data bits 95 to 64 */
#define XAES_MAC_MASK_OUT_3_MASK			(0x0000009CU)
	/**< MAC_MASK output data bits 127 to 96 */
/** @} */

/**
 * @name  AES_INTERRUPT_STATUS register
 * @{
 */
#define XAES_INTERRUPT_STATUS_OFFSET			(0x00000104U)
	/**< AES interrupt status register offset */
#define XAES_INTERRUPT_ENABLE_OFFSET			(0x0000010CU)
	/**< AES interrupt enable register offset */
#define XAES_INTERRUPT_STATUS_DONE_MASK			(0x00000001U)
	/**< AES interrupt done mask */
/** @} */

/**
 * @name  AES_KEY_SEL register
 * @{
 */
#define XAES_KEY_SEL_OFFSET				(0x00000000U)
	/**< Key source selection register offset  */
#define XAES_KEY_SEL_EFUSE_KEY_RED_0_VALUE		(0xEF858201U)
	/**< Key source selection value for eFuse Red Key0 */
#define XAES_KEY_SEL_EFUSE_KEY_RED_1_VALUE		(0xEF858202U)
	/**< Key source selection value for eFuse Red Key1 */
#define XAES_KEY_SEL_USER_KEY_0_VALUE			(0xBF858201U)
	/**< Key source selection value for user Key0 */
#define XAES_KEY_SEL_USER_KEY_1_VALUE			(0xBF858202U)
	/**< Key source selection value for user Key1 */
#define XAES_KEY_SEL_USER_KEY_2_VALUE			(0xBF858204U)
	/**< Key source selection value for user Key2 */
#define XAES_KEY_SEL_USER_KEY_3_VALUE			(0xBF858208U)
	/**< Key source selection value for user Key3 */
#define XAES_KEY_SEL_USER_KEY_4_VALUE			(0xBF858210U)
	/**< Key source selection value for user Key4 */
#define XAES_KEY_SEL_USER_KEY_5_VALUE			(0xBF858220U)
	/**< Key source selection value for user Key5 */
#define XAES_KEY_SEL_USER_KEY_6_VALUE			(0xBF858240U)
	/**< Key source selection value for user Key6 */
#define XAES_KEY_SEL_USER_KEY_7_VALUE			(0xBF858280U)
	/**< Key source selection value for user Key7 */
#define XAES_KEY_SEL_PUF_KEY_VALUE			(0xDBDE8200U)
	/**< Key source selection value for PUF Key */
/** @} */

/**
 * @name  AES_KEY_CLEAR register
 * @{
 */
#define XAES_KEY_CLEAR_OFFSET				(0x00000004U)
	/**< AES key clear register offset */
#define XAES_KEY_CLEAR_ALL_KEYS_MASK			(0x00003FFFU)
	/**< All key clear mask */
#define XAES_KEY_CLR_AES_KEY_ZEROIZE_MASK		(0x00002000U)
	/**< Expanded key in AES engine clear mask */
#define XAES_KEY_CLEAR_EFUSE_KEY_RED_1_MASK		(0x00001000U)
	/**< Efuse red key 0 clear mask */
#define XAES_KEY_CLEAR_EFUSE_KEY_RED_0_MASK		(0x00000800U)
	/**< Efuse red key 1 clear mask */
#define XAES_KEY_CLEAR_PUF_KEY_MASK			(0x00000400U)
	/**< Puf key clear mask */
#define XAES_KEY_CLEAR_EFUSE_KEY_1_MASK			(0x00000200U)
	/**< Efuse key 1 clear mask */
#define XAES_KEY_CLEAR_EFUSE_KEY_0_MASK			(0x00000100U)
	/**< Efuse key 0 clear mask */
#define XAES_KEY_CLEAR_USER_KEY_7_MASK			(0x00000080U)
	/**< User key 7 clear mask */
#define XAES_KEY_CLEAR_USER_KEY_6_MASK			(0x00000040U)
	/**< User key 6 clear mask */
#define XAES_KEY_CLEAR_USER_KEY_5_MASK			(0x00000020U)
	/**< User key 5 clear mask */
#define XAES_KEY_CLEAR_USER_KEY_4_MASK			(0x00000010U)
	/**< User key 4 clear mask */
#define XAES_KEY_CLEAR_USER_KEY_3_MASK			(0x00000008U)
	/**< User key 3 clear mask */
#define XAES_KEY_CLEAR_USER_KEY_2_MASK			(0x00000004U)
	/**< User key 2 clear mask */
#define XAES_KEY_CLEAR_USER_KEY_1_MASK			(0x00000002U)
	/**< User key 1 clear mask */
#define XAES_KEY_CLEAR_USER_KEY_0_MASK			(0x00000001U)
	/**< User key 0 clear mask */
#define XAES_KEY_CLEAR_REG_CLR_MASK			(0x00000000U)
	/**< Clear mask for all keys */
/** @} */

/**
 * @name  KEY_ZEROED_STATUS register
 * @{
 */
#define XAES_KEY_ZEROED_STATUS_OFFSET			(0x00000008U)
	/**< AES key Zeroed register offset */
#define XAES_KEY_ZEROED_STATUS_RESET_VAL		(0x00003FFEU)
	/**< AES key Zeroed status reset value */
#define XAES_KEY_ZEROED_STATUS_PUF_KEY_MASK		(0x00002000U)
	/**< Puf key zeroed mask */
#define XAES_KEY_ZEROED_STATUS_EFUSE_RED_KEY_1_MASK	(0x00001000U)
	/**< eFuse red key 1 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_EFUSE_RED_KEY_0_MASK	(0x00000800U)
	/**< eFuse red key 0 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_EFUSE_KEY_1_MASK		(0x00000400U)
	/**< eFuse key 1 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_EFUSE_KEY_0_MASK		(0x00000200U)
	/**< eFuse key 0 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_USER_KEY_7_MASK		(0x00000100U)
	/**< User key 7 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_USER_KEY_6_MASK		(0x00000080U)
	/**< User key 6 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_USER_KEY_5_MASK		(0x00000040U)
	/**< User key 5 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_USER_KEY_4_MASK		(0x00000020U)
	/**< User key 4 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_USER_KEY_3_MASK		(0x00000010U)
	/**< User key 3 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_USER_KEY_2_MASK		(0x00000008U)
	/**< User key 2 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_USER_KEY_1_MASK		(0x00000004U)
	/**< User key 1 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_USER_KEY_0_MASK		(0x00000002U)
	/**< User key 0 zeroed mask */
#define XAES_KEY_ZEROED_STATUS_AES_KEY_MASK		(0x00000001U)
	/**< Expanded key in AES engine zeroed mask */
/** @} */

/**
 * @name  AES_USER_KEY register
 * @{
 */
#define XAES_USER_KEY_0_0_OFFSET			(0x00000064U)
	/**< AES User_key_0 word0 register offset */
#define XAES_USER_KEY_0_3_OFFSET			(0x00000070U)
	/**< AES User_key_0 word3 register offset */
#define XAES_USER_KEY_1_0_OFFSET			(0x00000084U)
	/**< AES User_key_1 word0 register offset */
#define XAES_USER_KEY_2_0_OFFSET			(0x000000A4U)
	/**< AES User_key_2 word0 register offset */
#define XAES_USER_KEY_3_0_OFFSET			(0x000000D4U)
	/**< AES User_key_3 word0 register offset */
#define XAES_USER_KEY_4_0_OFFSET			(0x000000F4U)
	/**< AES User_key_4 word0 register offset */
#define XAES_USER_KEY_5_0_OFFSET			(0x00000114U)
	/**< AES User_key_5 word0 register offset */
#define XAES_USER_KEY_6_0_OFFSET			(0x00000134U)
	/**< AES User_key_6 word0 register offset */
#define XAES_USER_KEY_7_0_OFFSET			(0x00000154U)
	/**< AES User_key_7 word0 register offset */
/** @} */

/**
 * @name  AES_KEY_SIZE register
 * @{
 */
#define XAES_KEY_SIZE_OFFSET				(0x00000174U)
	/**< Offset of AES key size selection register */
/** @} */

/**
 * @name  AES_KEY_DEC register
 * @{
 */
#define XAES_KEY_TO_BE_DEC_SIZE_OFFSET			(0x00000178U)
	/**< Offset of size of the key to be decrypted */
#define XAES_KEY_DEC_MODE_OFFSET			(0x0000017CU)
	/**< Offset of key decrypt indication to AES */
#define XAES_KEY_DEC_MODE_VALUE				(0xFFFFFFFFU)
	/**< Value to put the AES engine into a key decrypt operation mode */
#define XAES_KEY_DEC_MODE_DISABLE			(0x00000000U)
	/**< Disable the AES engine in key decrypt operation mode */
#define XAES_KEY_TO_BE_DEC_SEL_OFFSET			(0x00000180U)
	/**< Offset of selecting the key to be decrypted */
#define XAES_KEY_TO_BE_DEC_SEL_EFUSE_KEY_0_VALUE	(0xEF856601U)
	/**< Selecting the eFuse key0 for key decryption */
#define XAES_KEY_TO_BE_DEC_SEL_EFUSE_KEY_1_VALUE	(0xEF856602U)
	/**< Selecting the eFuse key1 for key decryption */
#define XAES_KEY_TO_BE_DEC_SEL_DISABLE			(0x00000000U)
	/**< Key to be decrypted reset value */
/** @} */

/**
 * @name  ASU_PMXC_KEY_TRANSFER_READY register
 * @{
 */
#define XAES_ASU_PMXC_KEY_TRANSFER_READY_OFFSET		(0x00000184U)
	/**< ASU PMXC key transfer ready offset */
#define XAES_ASU_PMXC_KEY_TRANSFER_READY_MASK		(0X00000001U)
	/**< ASU PMXC key transfer ready mask */
#define XAES_ASU_PMXC_KEY_TRANSFER_READY_DISABLE	(0x00000000U)
	/**< ASU PMXC key transfer ready disable */
/** @} */

/**
 * @name  EFUSE_KEY_BLACK_OR_RED register
 * @{
 */
#define XAES_EFUSE_KEY_0_BLACK_OR_RED_OFFSET		(0x00000188U)
	/**< Efuse key 0 black or red offset */
#define XAES_EFUSE_KEY_1_BLACK_OR_RED_OFFSET		(0x0000018CU)
	/**< Efuse key 1 black or red offset */
/** @} */

/**
 * @name  KV_INTERRUPT_STATUS register
 * @{
 */
#define XAES_KV_INTERRUPT_STATUS_OFFSET			(0x00000194U)
	/**< KV interrupt status offset */
#define XAES_KV_INTERRUPT_STATUS_DONE_MASK		(0x00000001U)
	/**< KV interrupt status done mask */
#define XAES_KV_INTERRUPT_STATUS_CLEAR_MASK		(0x00000001U)
	/**< KV interrupt status clear mask */
/** @} */

/**
 * @name  KEY_MASK register
 * @{
 */
#define XAES_KEY_MASK_0_OFFSET				(0x00000024U)
	/**< Offset of key mask0 */
#define XAES_KEY_MASK_3_OFFSET				(0x00000030U)
	/**< Offset of key mask3 */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XAES_HW_H_ */