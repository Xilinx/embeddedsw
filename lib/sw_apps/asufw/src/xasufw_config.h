/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_config.h
 *
 * This header file contains ASUFW configuration macros for users.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  06/19/24 Initial release
 *       ma   07/01/24 Add print related macros and XASUFW_ENABLE_CAVP_TESTING macro
 *       am   07/04/24 Added XASUFW_AES_CM_CONFIG macro
 *       am   07/09/24 Fixed incorrect macro values for config enable and disable
 *       yog  07/11/24 Added macros to enable or disable support of NIST and Brainpool curves
 *       ma   07/26/24 Added XASUFW_TRNG_IN_PTRNG_MODE to run TRNG in PTRNG mode when enabled
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       ss   12/02/24 Added support for NIST curves P-256,P-384
 * 1.1   am   201/2/25 Added macros for efuse user keys
 *       yog  01/24/25 Renamed RSA_ECC macros to ECC
 *       ma   02/07/25 Moved TRNG DRBG configuration option to xilasu common code
 *       am   02/21/25 Added XASUFW_ENABLE_PERF_MEASUREMENT macro
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_CONFIG_H_
#define XASUFW_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xparameters.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_CONFIG_ENABLE		(0x1U) /**< To enable any configuration */
#define XASUFW_CONFIG_DISABLE		(0x0U) /**< To disable any configuration */

#define XASUFW_EFUSE_BLACK_KEY		(0X00000001U) /**< Efuse black key */
#define XASUFW_EFUSE_RED_KEY		(0X00000002U) /**< Efuse red key */

/** Configure key transfer for either red key or black key */
#define XASUFW_PMXC_EFUSE_USER_KEY_0 	(XASUFW_EFUSE_BLACK_KEY) /**< PMX eFuse user key 0
									configuration */
#define XASUFW_PMXC_EFUSE_USER_KEY_1 	(XASUFW_EFUSE_BLACK_KEY) /**< PMX eFuse user key 1
									configuration */

#define XASUFW_ENABLE_PERF_MEASUREMENT		(XASUFW_CONFIG_DISABLE) /**< To enable/disable
									performance measurement. */

/** SDK release version */
#define SDK_RELEASE_YEAR		2025	/**< Specifies the SDK release year */
#define SDK_RELEASE_QUARTER		2	/**< Specifies the SDK release quarter */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
/**
 * @name ASUFW Debug options
 *
 *  ASUFW supports an unconditional print
 *     - ASUFW_PRINT - This print level is used to print ASUFW banner and any mandatory prints
 *     - ASUFW_DEBUG - This print level is used to print basic information and error prints if any
 *     - ASUFW_DEBUG_INFO - This print level is used to print more debug information in addition
 *       to the basic information
 *     - ASUFW_DEBUG_DETAILED - This print level is used to print detailed debug prints
 */
//#define ASUFW_PRINT /**< ASUFW minimal prints */
//#define ASUFW_DEBUG /**< ASUFW general debug information prints */
//#define ASUFW_DEBUG_INFO /**< ASUFW more debug information prints */
#define ASUFW_DEBUG_DETAILED /**< ASUFW detailed debug information prints */

/** Run TRNG in PTRNG mode */
//#define XASUFW_TRNG_ENABLE_PTRNG_MODE

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_CONFIG_H_ */
/** @} */
