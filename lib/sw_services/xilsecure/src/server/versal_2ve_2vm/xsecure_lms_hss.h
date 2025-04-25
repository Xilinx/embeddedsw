/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/******************************************************************************/
/**
*
* @file xsecure_lms_hss.h

* @addtogroup LMS
* @{
* @details
* This file contains structures, constants and defines used in HSS
* provides interface to LMS operations
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
*  1.0  sak  08/015/23 Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/
#ifndef XSECURE_LMS_HSS_H_
#define XSECURE_LMS_HSS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_lms.h"

/************************** Constant Definitions ****************************/
#define XSECURE_LMS_HSS_MIN_LEVELS_SUPPORTED	(1U)	/** during KAT we support only one tree */
#define XSECURE_LMS_HSS_MAX_LEVELS_SUPPORTED	(2U)	/** max supported by ROM */
#define XSECURE_LMS_HSS_LEVELS_FIELD_SIZE	(4U)	/** Level field size */
/** public key size */
#define XSECURE_HSS_PUBLIC_KEY_TOTAL_SIZE	(XSECURE_LMS_HSS_LEVELS_FIELD_SIZE +\
						 XSECURE_LMS_PUB_KEY_TOTAL_SIZE)
/* signature list related */
#define XSECURE_HSS_SIGN_LIST_LEVEL_FIELD_OFFSET	(0U)		/** level field offset */
/** Offset of Signature-0 in HSS Signature */
#define XSECURE_HSS_SIGN_LIST_SIGN_0_OFFSET		(XSECURE_HSS_SIGN_LIST_LEVEL_FIELD_OFFSET + \
							XSECURE_LMS_HSS_LEVELS_FIELD_SIZE)
/** Offset of Public key-1 in HSS Signature */
#define XSECURE_HSS_SIGN_LIST_PUB_KEY_1_OFFSET(Sign0Len_)	(XSECURE_HSS_SIGN_LIST_SIGN_0_OFFSET + Sign0Len_)
/** Offset of Signature Npsk in HSS Signature */
#define XSECURE_HSS_SIGN_LIST_SIGN_NPSK_OFFSET(Sign0Len_)	(XSECURE_HSS_SIGN_LIST_PUB_KEY_1_OFFSET(Sign0Len_) +\
								 XSECURE_LMS_PUB_KEY_TOTAL_SIZE)

/* public key related offsets */
#define XSECURE_HSS_PUBLIC_KEY_LEVEL_FIELD_OFFSET		(0U)

#define XSECURE_HSS_PUBLIC_KEY_LMS_FIELD_OFFSET			(XSECURE_HSS_PUBLIC_KEY_LEVEL_FIELD_OFFSET +\
								 XSECURE_LMS_HSS_LEVELS_FIELD_SIZE)

#define XSECURE_HSS_PUBLIC_KEY_LMS_OTS_FIELD_OFFSET		(XSECURE_HSS_PUBLIC_KEY_LMS_FIELD_OFFSET + \
								XSECURE_LMS_TYPE_SIZE)

/***************************** Type Definitions******************************/
/**
 * @brief Used to parse HSS public key fields
 */
typedef struct {
	u32 Levels;

	XSecure_LmsPublicKey LmsPubKey;
}__attribute__((__packed__)) XSecure_LmsHssPublicKey;

/**
 * @brief Used to parse HSS Signature list
 */
typedef struct {
	u32 Levels;
	u8* SignList;
} XSecure_LmsHssSignature;

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_LMS_HSS_H_ */
/** @} */
