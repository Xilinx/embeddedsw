/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_ALGINFO_H_
#define XASUFW_ALGINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_TRNG_MAJOR_VERSION		(1U) /**< Major version number of TRNG module */
#define XASUFW_TRNG_MINOR_VERSION		(0U) /**< Minor version number of TRNG module */

#define XASUFW_SHA2_MAJOR_VERSION		(1U) /**< Major version number of SHA2 module */
#define XASUFW_SHA2_MINOR_VERSION		(0U) /**< Minor version number of SHA2 module */

#define XASUFW_SHA3_MAJOR_VERSION		(1U) /**< Major version number of SHA3 module */
#define XASUFW_SHA3_MINOR_VERSION		(0U) /**< Minor version number of SHA3 module */

#define XASUFW_ECC_MAJOR_VERSION		(1U) /**< Major version number of ECC module */
#define XASUFW_ECC_MINOR_VERSION		(0U) /**< Minor version number of ECC module */

#define XASUFW_RSA_MAJOR_VERSION		(1U) /**< Major version number of RSA module */
#define XASUFW_RSA_MINOR_VERSION		(0U) /**< Minor version number of RSA module */

#define XASUFW_AES_MAJOR_VERSION		(1U) /**< Major version number of AES module */
#define XASUFW_AES_MINOR_VERSION		(0U) /**< Minor version number of AES module */

#define XASUFW_HMAC_MAJOR_VERSION		(1U) /**< Major version number of HMAC module */
#define XASUFW_HMAC_MINOR_VERSION		(0U) /**< Minor version number of HMAC module */

#define XASUFW_KDF_MAJOR_VERSION		(1U) /**< Major version number of KDF module */
#define XASUFW_KDF_MINOR_VERSION		(0U) /**< Minor version number of KDF module */

#define XASUFW_ECIES_MAJOR_VERSION		(1U) /**< Major version number of ECIES module */
#define XASUFW_ECIES_MINOR_VERSION		(0U) /**< Minor version number of ECIES module */

#define XASUFW_KEYWRAP_MAJOR_VERSION		(1U) /**< Major version number of Keywrap module */
#define XASUFW_KEYWRAP_MINOR_VERSION		(0U) /**< Minor version number of Keywrap module */

#define XASUFW_NIST_COMPLIANT			(0xFFU) /**< Indicates that the algorithm is
								NIST compliant */
#define XASUFW_NIST_NON_COMPLIANT		(0x00U) /**< Indicates that the algorithm is
								either not NIST compliant or
								NIST compliance is not
								applicable*/

#define XASUFW_ALG_BUILD_VERSION(Major, Minor)	((((u32)Major) << XASUFW_TWO_BYTE_SHIFT_VALUE) \
						| (Minor)) /**< Concatenate 16-bit Major version
							and Minor version of crypto module */
/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_ALGINFO_H_ */
/** @} */