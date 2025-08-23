/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_def.h
 *
 * This file contains the common definitions between server and client
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  06/04/24 Initial release
 *       am   08/01/24 Added macro for AES module Id.
 *       yog  01/02/25 Added macro for HMAC module ID.
 *       ma   01/15/25 Added macro for KDF module ID.
 *       rmv  08/11/25 Added macro for PLM event module ID.
 *       rmv  07/16/25 Added macro for OCP module ID.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_DEF_H_
#define XASU_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
#define XASU_TRUE		(TRUE)
#define XASU_FALSE		(FALSE)

#define XASU_CMD_SECURE			(0x0U) /**< Secure command */
#define XASU_CMD_NON_SECURE		(0x1U) /**< Non-secure command */

/* Module ID */
#define XASU_MODULE_TRNG_ID			(0U) /**< TRNGs module ID */
#define XASU_MODULE_SHA2_ID			(1U) /**< SHA2 module ID */
#define XASU_MODULE_SHA3_ID			(2U) /**< SHA3 module ID */
#define XASU_MODULE_ECC_ID			(3U) /**< ECC module ID */
#define XASU_MODULE_RSA_ID			(4U) /**< RSA module ID */
#define XASU_MODULE_AES_ID			(5U) /**< AES module ID */
#define XASU_MODULE_HMAC_ID			(6U) /**< HMAC module ID */
#define XASU_MODULE_KDF_ID			(7U) /**< KDF module ID */
#define XASU_MODULE_ECIES_ID			(8U) /**< ECIES module ID */
#define XASU_MODULE_KEYWRAP_ID			(9U) /**< Key wrap unwrap module ID */
#define XASU_MODULE_PLM_ID			(10U) /**< PLM module ID */
#define XASU_MODULE_OCP_ID			(11U) /**< OCP module ID */
#define XASU_MAX_MODULES			(12U) /**< Maximum supported modules in ASU */

#define XASU_ASU_DMA_MAX_TRANSFER_LENGTH	(0x1FFFFFFCU)
						/**< ASU DMA maximum transfer length in bytes. */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************** Type Definitions *****************************************/
/**
 * This structure contains data for cryptographic algorithms, including versioning
 * and NIST compliance status, for up to maximum modules in ASUFW.
 */
typedef struct {
	u32 Version;	/**< Version of the cryptographic algorithm */
				/**< [15:0]: Algorithm minor version
				     [31:16]: Algorithm major version */
	u8 NistStatus;	/**< NIST compliance status of the cryptographic algorithm */
				/**< 0xFF: Algorithm is NIST compliant
				     0x00: Algorithm is not NIST compliant or NIST compliance is not
					    applicable */
	u8 KatStatus;	/**< KAT status bit, 0 - KAT not run, 1 - KAT fail and 3 - KAT pass */
	u8 Reserved2;	/**< Reserved */
	u8 Reserved3;	/**< Reserved */
	u32 Reserved4;	/**< Reserved */
} XAsu_CryptoAlgInfo;

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_DEF_H_ */
/** @} */
