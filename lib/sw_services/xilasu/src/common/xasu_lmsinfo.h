/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_lmsinfo.h
 *
 * This file contains the LMS definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   01/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_LMSINFO_H_
#define XASU_LMSINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* LMS module command IDs */
#define XASU_LMS_SIGN_VERIFY_SHA2_CMD_ID	(0U) /**< Command ID for LMS signature verification
							with SHA2 */
#define XASU_LMS_SIGN_VERIFY_SHA3_CMD_ID	(1U) /**< Command ID for LMS signature verification
							with SHA3 */
#define XASU_HSS_SIGN_VERIFY_SHA2_CMD_ID	(2U) /**< Command ID for HSS signature verification
							with SHA2 */
#define XASU_HSS_SIGN_VERIFY_SHA3_CMD_ID	(3U) /**< Command ID for HSS signature verification
							with SHA3 */
#define XASU_LMS_KAT_CMD_ID			(4U) /**< Command ID for LMS KAT */

/* LMS Type values from RFC 8554 */
#define XASU_LMS_RESERVED			(0x00000000U) /**< Reserved type */
#define XASU_LMS_SHA256_M32_HEIGHT_5		(0x00000005U) /**< SHA-256, M=32, H=5 */
#define XASU_LMS_SHA256_M32_HEIGHT_10		(0x00000006U) /**< SHA-256, M=32, H=10 */
#define XASU_LMS_SHA256_M32_HEIGHT_15		(0x00000007U) /**< SHA-256, M=32, H=15 */
#define XASU_LMS_SHA256_M32_HEIGHT_20		(0x00000008U) /**< SHA-256, M=32, H=20 */
#define XASU_LMS_SHAKE_M32_HEIGHT_5		(0x0000000FU) /**< SHAKE-256, M=32, H=5 */
#define XASU_LMS_SHAKE_M32_HEIGHT_10		(0x00000010U) /**< SHAKE-256, M=32, H=10 */
#define XASU_LMS_SHAKE_M32_HEIGHT_15		(0x00000011U) /**< SHAKE-256, M=32, H=15 */
#define XASU_LMS_SHAKE_M32_HEIGHT_20		(0x00000012U) /**< SHAKE-256, M=32, H=20 */

/* LMS OTS Type values from RFC 8554 */
#define XASU_LMS_OTS_RESERVED			(0x00000000U) /**< Reserved OTS type */
#define XASU_LMS_OTS_SHA256_N32_W1		(0x00000001U) /**< SHA-256, n=32, w=1 */
#define XASU_LMS_OTS_SHA256_N32_W2		(0x00000002U) /**< SHA-256, n=32, w=2 */
#define XASU_LMS_OTS_SHA256_N32_W4		(0x00000003U) /**< SHA-256, n=32, w=4 */
#define XASU_LMS_OTS_SHA256_N32_W8		(0x00000004U) /**< SHA-256, n=32, w=8 */
#define XASU_LMS_OTS_SHAKE_N32_W1		(0x0000000BU) /**< SHAKE-256, n=32, w=1 */
#define XASU_LMS_OTS_SHAKE_N32_W2		(0x0000000CU) /**< SHAKE-256, n=32, w=2 */
#define XASU_LMS_OTS_SHAKE_N32_W4		(0x0000000DU) /**< SHAKE-256, n=32, w=4 */
#define XASU_LMS_OTS_SHAKE_N32_W8		(0x0000000EU) /**< SHAKE-256, n=32, w=8 */

/* LMS Public key size */
#define XASU_LMS_PUB_KEY_SIZE			(56U) /**< LMS public key size in bytes
							(4 + 4 + 16 + 32) */

/* LMS common field sizes */
#define XASU_LMS_TYPE_SIZE			(4U) /**< Size of LMS type field */
#define XASU_LMS_OTS_TYPE_SIZE			(4U) /**< Size of LMS OTS type field */
#define XASU_LMS_I_FIELD_SIZE			(16U) /**< Size of I field (tree identifier) */
#define XASU_LMS_M_FIELD_SIZE			(32U) /**< Size of M field (hash output) */

/* HSS related sizes */
#define XASU_HSS_MAX_LEVELS			(8U) /**< Maximum HSS levels supported */
#define XASU_HSS_NUM_LEVELS_SIZE		(4U) /**< Size of HSS levels field */

/* Pre-hashed message flag */
#define XASU_LMS_MSG_NOT_PREHASHED		(0U) /**< Message is not pre-hashed */
#define XASU_LMS_MSG_PREHASHED			(1U) /**< Message is pre-hashed */

/** @} */
/************************************** Type Definitions *****************************************/

/**
 * This structure contains LMS/HSS signature verification parameters.
 */
typedef struct {
	u64 MsgAddr;		/**< Address of the message to be verified */
	u64 SignatureAddr;	/**< Address of the LMS signature */
	u64 PublicKeyAddr;	/**< Address of the LMS public key */
	u32 MsgLen;		/**< Length of the message in bytes */
	u32 SignatureLen;	/**< Length of the signature in bytes */
	u32 PublicKeyLen;	/**< Length of the public key in bytes */
	u8 PreHashedMsg;	/**< Flag indicating if message is pre-hashed
				     (XASU_LMS_MSG_NOT_PREHASHED / XASU_LMS_MSG_PREHASHED) */
	u8 ShaType;		/**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode;		/**< SHA Mode (XASU_SHA_MODE_SHA2_256 / XASU_SHA_MODE_SHAKE256) */
	u8 Reserved;		/**< Reserved for alignment */
} XAsu_LmsHssSignVerifyParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_LMSINFO_H_ */
