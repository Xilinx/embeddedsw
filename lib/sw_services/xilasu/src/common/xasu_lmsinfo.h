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
#define XASU_HSS_KAT_CMD_ID			(4U) /**< Command ID for HSS KAT */
#define XASU_LMS_MAX_CMDS			(5U) /**< Maximum number of commands supported by LMS module */

/* LMS/HSS Public key sizes per NIST SP 800-208 */
#define XASU_LMS_PUB_KEY_SIZE			(48U) /**< Minimum LMS public key size (N=24) */
#define XASU_LMS_MAX_PUB_KEY_SIZE		(60U) /**< Maximum HSS public key size (N=32) */

/* Hash output lengths per NIST LMS parameter sets */
#define XASU_LMS_HASH_LEN_N24			(24U) /**< Hash output length for N=24 parameter sets */
#define XASU_LMS_HASH_LEN_N32			(32U) /**< Hash output length for N=32 parameter sets */

/**
 * LMS/HSS Signature size bounds per NIST SP 800-208.
 * Min LMS sig: N=24, W=8, H=5 → q[4] + LMOTS[652] + type[4] + path[24*5=120] = 780
 * Max HSS sig (2-level): Nspk[4] + LMS_max[9324] + pub_N32[56] + LMS_max[9324] = 18708
 */
#define XASU_LMS_MIN_SIGNATURE_SIZE		(780U) /**< Minimum NIST-valid LMS signature size */
#define XASU_LMS_MAX_SIGNATURE_SIZE		(18708U) /**< Maximum NIST-valid HSS signature size
								(2-level, N=32, W=1, H=25) */

/* Pre-hashed message flag */
#define XASU_LMS_MSG_NOT_PREHASHED		(0U) /**< Message is not pre-hashed */
#define XASU_LMS_MSG_PREHASHED			(1U) /**< Message is pre-hashed */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure represents an LMS/HSS key object. */
typedef struct {
	u64 PubKeyAddr;     /**< Address of the public key data */
	u32 PubKeyId;       /**< Key identifier from key management (used if PubKeyAddr == 0) */
	u32 PubKeyLen;      /**< Length of the public key in bytes */
} XAsu_LmsHssKeyObject;

/**
 * This structure contains LMS/HSS signature verification parameters.
 */
typedef struct {
	XAsu_LmsHssKeyObject LmsHssKeyObj;		/**< Key object for the public key */
	u64 MsgAddr;		/**< Address of the message to be verified */
	u64 SignatureAddr;	/**< Address of the LMS signature */
	u32 MsgLen;		/**< Length of the message in bytes */
	u32 SignatureLen;	/**< Length of the signature in bytes */
	u8 PreHashedMsg;	/**< Flag indicating if message is pre-hashed
				     (XASU_LMS_MSG_NOT_PREHASHED / XASU_LMS_MSG_PREHASHED) */
	u8 ShaType;		/**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode;		/**< SHA Mode (XASU_SHA_MODE_SHA2_256 / XASU_SHA_MODE_SHAKE256) */
	u8 Reserved[5];		/**< Reserved for alignment */
} XAsu_LmsHssSignVerifyParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_LMSINFO_H_ */
