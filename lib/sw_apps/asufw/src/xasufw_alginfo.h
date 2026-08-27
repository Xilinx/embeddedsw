/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_alginfo.h
 *
 * This file contains version information of crypto algorithms in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   05/20/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_ALGINFO_H_
#define XASUFW_ALGINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif
#include "xasufw_config.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_TRNG_MAJOR_VERSION		(2U) /**< Major version number of TRNG module */
#define XASUFW_TRNG_MINOR_VERSION		(1U) /**< Minor version number of TRNG module */

#define XASUFW_SHA2_MAJOR_VERSION		(3U) /**< Major version number of SHA2 module */
#define XASUFW_SHA2_MINOR_VERSION		(0U) /**< Minor version number of SHA2 module */

#define XASUFW_SHA3_MAJOR_VERSION		(3U) /**< Major version number of SHA3 module */
#define XASUFW_SHA3_MINOR_VERSION		(0U) /**< Minor version number of SHA3 module */

#define XASUFW_ECC_MAJOR_VERSION		(3U) /**< Major version number of ECC module */
#define XASUFW_ECC_MINOR_VERSION		(0U) /**< Minor version number of ECC module */

#define XASUFW_RSA_MAJOR_VERSION		(3U) /**< Major version number of RSA module */
#define XASUFW_RSA_MINOR_VERSION		(0U) /**< Minor version number of RSA module */

#define XASUFW_AES_MAJOR_VERSION		(3U) /**< Major version number of AES module */
#define XASUFW_AES_MINOR_VERSION		(0U) /**< Minor version number of AES module */

#define XASUFW_HMAC_MAJOR_VERSION		(2U) /**< Major version number of HMAC module */
#define XASUFW_HMAC_MINOR_VERSION		(1U) /**< Minor version number of HMAC module */

#define XASUFW_KDF_MAJOR_VERSION		(2U) /**< Major version number of KDF module */
#define XASUFW_KDF_MINOR_VERSION		(1U) /**< Minor version number of KDF module */

#define XASUFW_ECIES_MAJOR_VERSION		(2U) /**< Major version number of ECIES module */
#define XASUFW_ECIES_MINOR_VERSION		(1U) /**< Minor version number of ECIES module */

#define XASUFW_KEYWRAP_MAJOR_VERSION		(2U) /**< Major version number of Keywrap module */
#define XASUFW_KEYWRAP_MINOR_VERSION		(1U) /**< Minor version number of Keywrap module */

#define XASUFW_OCP_MAJOR_VERSION		(2U) /**< Major version number of OCP module */
#define XASUFW_OCP_MINOR_VERSION		(0U) /**< Minor version number of OCP module */

#define XASUFW_KEYMANAGER_MAJOR_VERSION		(2U) /**< Major version number of Keymanager module */
#define XASUFW_KEYMANAGER_MINOR_VERSION		(0U) /**< Minor version number of Keymanager module */

#define XASUFW_LMS_MAJOR_VERSION		(2U) /**< Major version number of LMS module */
#define XASUFW_LMS_MINOR_VERSION		(0U) /**< Minor version number of LMS module */

#define XASUFW_MLDSA_MAJOR_VERSION		(1U) /**< Major version number of ML-DSA module */
#define XASUFW_MLDSA_MINOR_VERSION		(0U) /**< Minor version number of ML-DSA module */

#define XASUFW_MLKEM_MAJOR_VERSION		(1U) /**< Major version number of ML-KEM module */
#define XASUFW_MLKEM_MINOR_VERSION		(0U) /**< Minor version number of ML-KEM module */

#define XASUFW_NIST_COMPLIANT			(0xFFU) /**< Indicates that the algorithm is
								NIST compliant */
#define XASUFW_NIST_NON_COMPLIANT		(0x00U) /**< Indicates that the algorithm is
								either not NIST compliant or
								NIST compliance is not
								applicable*/

#define XASUFW_ALG_BUILD_VERSION(Major, Minor)	((((u32)Major) << XASUFW_TWO_BYTE_SHIFT_VALUE) \
						| (Minor)) /**< Concatenate 16-bit Major version
							and Minor version of crypto module */

/**
 * @name Feature Capability Bit Positions for TRNG Module (ID 0)
 * @{
 */
#define XASUFW_TRNG_CAP_DRBG_MODE_BIT_POS	(0U) /**< Bit position for DRBG mode */
#define XASUFW_TRNG_CAP_PTRNG_MODE_BIT_POS	(1U) /**< Bit position for PTRNG mode */
/** @} */

/**
 * @name Feature Capability Bits for TRNG Module (ID 0)
 * @{
 * TRNG is always enabled. These bits indicate optional sub-features.
 * Each macro resolves to the capability bit if the feature is enabled, or 0 if disabled.
 */
#ifdef XASU_TRNG_ENABLE_DRBG_MODE
#define XASUFW_TRNG_CAP_DRBG_MODE \
	((u16)1U << XASUFW_TRNG_CAP_DRBG_MODE_BIT_POS) /**< DRBG mode */
#else
#define XASUFW_TRNG_CAP_DRBG_MODE	(0U) /**< DRBG mode disabled */
#endif

#ifdef XASUFW_TRNG_ENABLE_PTRNG_MODE
#define XASUFW_TRNG_CAP_PTRNG_MODE \
	((u16)1U << XASUFW_TRNG_CAP_PTRNG_MODE_BIT_POS) /**< PTRNG mode */
#else
#define XASUFW_TRNG_CAP_PTRNG_MODE	(0U) /**< PTRNG mode disabled */
#endif
/** @} */

/**
 * @name Feature Capability Bit Positions for ECC Module (ID 3)
 * @{
 */
#define XASUFW_ECC_CAP_CM_ENABLED_BIT_POS	(0U)  /**< Bit position for counter-measures */
#define XASUFW_ECC_CAP_NIST_P192_BIT_POS	(1U)  /**< Bit position for NIST P-192 */
#define XASUFW_ECC_CAP_NIST_P224_BIT_POS	(2U)  /**< Bit position for NIST P-224 */
#define XASUFW_ECC_CAP_NIST_P256_BIT_POS	(3U)  /**< Bit position for NIST P-256 */
#define XASUFW_ECC_CAP_NIST_P384_BIT_POS	(4U)  /**< Bit position for NIST P-384 */
#define XASUFW_ECC_CAP_NIST_P521_BIT_POS	(5U)  /**< Bit position for NIST P-521 */
#define XASUFW_ECC_CAP_EDWARD_P25519_BIT_POS	(6U)  /**< Bit position for Edwards 25519 */
#define XASUFW_ECC_CAP_EDWARD_P448_BIT_POS	(7U)  /**< Bit position for Edwards 448 */
#define XASUFW_ECC_CAP_BRAINPOOL_P256_BIT_POS	(8U)  /**< Bit position for Brainpool P-256 */
#define XASUFW_ECC_CAP_BRAINPOOL_P320_BIT_POS	(9U)  /**< Bit position for Brainpool P-320 */
#define XASUFW_ECC_CAP_BRAINPOOL_P384_BIT_POS	(10U) /**< Bit position for Brainpool P-384 */
#define XASUFW_ECC_CAP_BRAINPOOL_P512_BIT_POS	(11U) /**< Bit position for Brainpool P-512 */
/** @} */

/**
 * @name Feature Capability Bits for ECC Module (ID 3)
 * @{
 * ECC is always enabled. These bits indicate which curves are supported.
 * Each macro resolves to the capability bit if the feature is enabled, or 0 if disabled.
 */
#ifdef XASU_ECC_CM_ENABLE
#define XASUFW_ECC_CAP_CM_ENABLED \
	((u16)1U << XASUFW_ECC_CAP_CM_ENABLED_BIT_POS) /**< Counter-measures */
#else
#define XASUFW_ECC_CAP_CM_ENABLED	(0U) /**< Counter-measures disabled */
#endif

#ifdef XASU_ECC_SUPPORT_NIST_P192
#define XASUFW_ECC_CAP_NIST_P192 \
	((u16)1U << XASUFW_ECC_CAP_NIST_P192_BIT_POS) /**< NIST P-192 */
#else
#define XASUFW_ECC_CAP_NIST_P192	(0U) /**< NIST P-192 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_NIST_P224
#define XASUFW_ECC_CAP_NIST_P224 \
	((u16)1U << XASUFW_ECC_CAP_NIST_P224_BIT_POS) /**< NIST P-224 */
#else
#define XASUFW_ECC_CAP_NIST_P224	(0U) /**< NIST P-224 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_NIST_P256
#define XASUFW_ECC_CAP_NIST_P256 \
	((u16)1U << XASUFW_ECC_CAP_NIST_P256_BIT_POS) /**< NIST P-256 */
#else
#define XASUFW_ECC_CAP_NIST_P256	(0U) /**< NIST P-256 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_NIST_P384
#define XASUFW_ECC_CAP_NIST_P384 \
	((u16)1U << XASUFW_ECC_CAP_NIST_P384_BIT_POS) /**< NIST P-384 */
#else
#define XASUFW_ECC_CAP_NIST_P384	(0U) /**< NIST P-384 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_NIST_P521
#define XASUFW_ECC_CAP_NIST_P521 \
	((u16)1U << XASUFW_ECC_CAP_NIST_P521_BIT_POS) /**< NIST P-521 */
#else
#define XASUFW_ECC_CAP_NIST_P521	(0U) /**< NIST P-521 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_EDWARD_P25519
#define XASUFW_ECC_CAP_EDWARD_P25519 \
	((u16)1U << XASUFW_ECC_CAP_EDWARD_P25519_BIT_POS) /**< Ed25519 */
#else
#define XASUFW_ECC_CAP_EDWARD_P25519	(0U) /**< Ed25519 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_EDWARD_P448
#define XASUFW_ECC_CAP_EDWARD_P448 \
	((u16)1U << XASUFW_ECC_CAP_EDWARD_P448_BIT_POS) /**< Ed448 */
#else
#define XASUFW_ECC_CAP_EDWARD_P448	(0U) /**< Ed448 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_BRAINPOOL_P256
#define XASUFW_ECC_CAP_BRAINPOOL_P256 \
	((u16)1U << XASUFW_ECC_CAP_BRAINPOOL_P256_BIT_POS) /**< BP P-256 */
#else
#define XASUFW_ECC_CAP_BRAINPOOL_P256	(0U) /**< BP P-256 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_BRAINPOOL_P320
#define XASUFW_ECC_CAP_BRAINPOOL_P320 \
	((u16)1U << XASUFW_ECC_CAP_BRAINPOOL_P320_BIT_POS) /**< BP P-320 */
#else
#define XASUFW_ECC_CAP_BRAINPOOL_P320	(0U) /**< BP P-320 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_BRAINPOOL_P384
#define XASUFW_ECC_CAP_BRAINPOOL_P384 \
	((u16)1U << XASUFW_ECC_CAP_BRAINPOOL_P384_BIT_POS) /**< BP P-384 */
#else
#define XASUFW_ECC_CAP_BRAINPOOL_P384	(0U) /**< BP P-384 disabled */
#endif

#ifdef XASU_ECC_SUPPORT_BRAINPOOL_P512
#define XASUFW_ECC_CAP_BRAINPOOL_P512 \
	((u16)1U << XASUFW_ECC_CAP_BRAINPOOL_P512_BIT_POS) /**< BP P-512 */
#else
#define XASUFW_ECC_CAP_BRAINPOOL_P512	(0U) /**< BP P-512 disabled */
#endif
/** @} */

/**
 * @name Feature Capability Bit Positions for RSA Module (ID 4)
 * @{
 */
#define XASUFW_RSA_CAP_PADDING_BIT_POS		(0U) /**< Bit position for padding support */
#define XASUFW_RSA_CAP_2048_KEYGEN_BIT_POS	(1U) /**< Bit position for 2048-bit keygen */
#define XASUFW_RSA_CAP_3072_KEYGEN_BIT_POS	(2U) /**< Bit position for 3072-bit keygen */
#define XASUFW_RSA_CAP_4096_KEYGEN_BIT_POS	(3U) /**< Bit position for 4096-bit keygen */
/** @} */

/**
 * @name Feature Capability Bits for RSA Module (ID 4)
 * @{
 * RSA is always enabled. These bits indicate optional sub-features.
 * Each macro resolves to the capability bit if the feature is enabled, or 0 if disabled.
 */
#ifdef XASU_RSA_PADDING_ENABLE
#define XASUFW_RSA_CAP_PADDING \
	((u16)1U << XASUFW_RSA_CAP_PADDING_BIT_POS) /**< Padding support */
#else
#define XASUFW_RSA_CAP_PADDING		(0U) /**< Padding disabled */
#endif

#ifdef XASU_RSA_2048_KEYGEN_ENABLE
#define XASUFW_RSA_CAP_2048_KEYGEN \
	((u16)1U << XASUFW_RSA_CAP_2048_KEYGEN_BIT_POS) /**< 2048-bit keygen */
#else
#define XASUFW_RSA_CAP_2048_KEYGEN	(0U) /**< 2048-bit keygen disabled */
#endif

#ifdef XASU_RSA_3072_KEYGEN_ENABLE
#define XASUFW_RSA_CAP_3072_KEYGEN \
	((u16)1U << XASUFW_RSA_CAP_3072_KEYGEN_BIT_POS) /**< 3072-bit keygen */
#else
#define XASUFW_RSA_CAP_3072_KEYGEN	(0U) /**< 3072-bit keygen disabled */
#endif

#ifdef XASU_RSA_4096_KEYGEN_ENABLE
#define XASUFW_RSA_CAP_4096_KEYGEN \
	((u16)1U << XASUFW_RSA_CAP_4096_KEYGEN_BIT_POS) /**< 4096-bit keygen */
#else
#define XASUFW_RSA_CAP_4096_KEYGEN	(0U) /**< 4096-bit keygen disabled */
#endif
/** @} */

/**
 * @name Feature Capability Bit Positions for AES Module (ID 5)
 * @{
 */
#define XASUFW_AES_CAP_CM_ENABLED_BIT_POS	(0U) /**< Bit position for counter-measures */
/** @} */

/**
 * @name Feature Capability Bits for AES Module (ID 5)
 * @{
 * AES is always enabled. These bits indicate optional sub-features.
 * Each macro resolves to the capability bit if the feature is enabled, or 0 if disabled.
 */
#ifdef XASU_AES_CM_ENABLE
#define XASUFW_AES_CAP_CM_ENABLED \
	((u16)1U << XASUFW_AES_CAP_CM_ENABLED_BIT_POS) /**< Counter-measures */
#else
#define XASUFW_AES_CAP_CM_ENABLED	(0U) /**< Counter-measures disabled */
#endif
/** @} */

/**
 * @name Compile-time aggregate feature capabilities per module.
 * Each XASUFW_*_CAP_* macro resolves to its bit value or 0
 * based on the build configuration. The aggregate macros
 * below OR them into a single compile-time constant.
 * @{
 */
#define XASUFW_TRNG_FEATURE_CAPS \
	(XASUFW_TRNG_CAP_DRBG_MODE | XASUFW_TRNG_CAP_PTRNG_MODE)
		/**< Aggregate feature capabilities for TRNG module */

#define XASUFW_ECC_FEATURE_CAPS \
	(XASUFW_ECC_CAP_CM_ENABLED | XASUFW_ECC_CAP_NIST_P192 | \
	XASUFW_ECC_CAP_NIST_P224 | XASUFW_ECC_CAP_NIST_P256 | \
	XASUFW_ECC_CAP_NIST_P384 | XASUFW_ECC_CAP_NIST_P521 | \
	XASUFW_ECC_CAP_EDWARD_P25519 | XASUFW_ECC_CAP_EDWARD_P448 | \
	XASUFW_ECC_CAP_BRAINPOOL_P256 | XASUFW_ECC_CAP_BRAINPOOL_P320 | \
	XASUFW_ECC_CAP_BRAINPOOL_P384 | XASUFW_ECC_CAP_BRAINPOOL_P512)
		/**< Aggregate feature capabilities for ECC module */

#define XASUFW_RSA_FEATURE_CAPS \
	(XASUFW_RSA_CAP_PADDING | XASUFW_RSA_CAP_2048_KEYGEN | \
	XASUFW_RSA_CAP_3072_KEYGEN | XASUFW_RSA_CAP_4096_KEYGEN)
		/**< Aggregate feature capabilities for RSA module */

#define XASUFW_AES_FEATURE_CAPS \
	(XASUFW_AES_CAP_CM_ENABLED)
		/**< Aggregate feature capabilities for AES module */
/** @} */

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_ALGINFO_H_ */
/** @} */