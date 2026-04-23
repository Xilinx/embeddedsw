/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xlms_hss.h
*
* This file contains structures, constants and defines used in HSS
* provides interface to LMS operations for ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 1.0   ss   01/07/26 Initial release
*
* </pre>
*
* @note
*
**************************************************************************************************/
/**
* @addtogroup xlms_server_apis LMS Server APIs
* @{
*/
#ifndef XLMS_HSS_H_
#define XLMS_HSS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xlms.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

#ifdef XASU_LMS_ENABLE

/************************************ Constant Definitions ***************************************/
#define XLMS_HSS_MIN_LEVELS_SUPPORTED	(1U)	/**< during KAT we support only one tree */
#define XLMS_HSS_MAX_LEVELS_SUPPORTED	(2U)	/**< max supported */
#define XLMS_HSS_LEVELS_FIELD_SIZE	(4U)	/**< Level field size */
/** public key size */
#define XLMS_HSS_PUBLIC_KEY_TOTAL_SIZE	(XLMS_HSS_LEVELS_FIELD_SIZE +\
						 XLMS_PUB_KEY_TOTAL_SIZE)
/* signature list related */
#define XLMS_HSS_SIGN_LIST_LEVEL_FIELD_OFFSET		(0U)	/**< level field offset */
/** Offset of Signature-0 in HSS Signature */
#define XLMS_HSS_SIGN_LIST_SIGN_0_OFFSET		(XLMS_HSS_SIGN_LIST_LEVEL_FIELD_OFFSET + \
							XLMS_HSS_LEVELS_FIELD_SIZE)
/** Offset of Public key-1 in HSS Signature */
#define XLMS_HSS_SIGN_LIST_PUB_KEY_1_OFFSET(Sign0Len_)	(XLMS_HSS_SIGN_LIST_SIGN_0_OFFSET + Sign0Len_)
/** Offset of Signature Npsk in HSS Signature */
#define XLMS_HSS_SIGN_LIST_SIGN_NPSK_OFFSET(Sign0Len_)	(XLMS_HSS_SIGN_LIST_PUB_KEY_1_OFFSET(Sign0Len_) +\
								 XLMS_PUB_KEY_TOTAL_SIZE)

/* public key related offsets */
#define XLMS_HSS_PUBLIC_KEY_LEVEL_FIELD_OFFSET		(0U)
						/**< HSS public key level field offset */

#define XLMS_HSS_PUBLIC_KEY_LMS_FIELD_OFFSET			(XLMS_HSS_PUBLIC_KEY_LEVEL_FIELD_OFFSET +\
								 XLMS_HSS_LEVELS_FIELD_SIZE)
						/**< HSS public key LMS field offset */

#define XLMS_HSS_PUBLIC_KEY_LMS_OTS_FIELD_OFFSET		(XLMS_HSS_PUBLIC_KEY_LMS_FIELD_OFFSET + \
								XLMS_TYPE_SIZE)
						/**< HSS public key OTS field offset */

/************************************** Type Definitions *****************************************/
/**
 * @brief Used to parse HSS public key fields
 */
typedef struct {
	u32 Levels;	/**< Levels supported */
	XLms_PublicKey LmsPubKey;	/**< LMS public key */
}__attribute__((__packed__)) XLms_HssPublicKey;

/**
 * @brief Used to parse HSS Signature list
 */
typedef struct {
	u32 Levels;	/**< Levels supported */
	u8* SignList;	/**< Pointer to signature list */
} XLms_HssSignature;

#endif /* XASU_LMS_ENABLE */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XLMS_HSS_H_ */
/** @} */
