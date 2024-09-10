/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_config.h
 * @addtogroup Overview
 * @{
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
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_CONFIG_H
#define XASUFW_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xparameters.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_CONFIG_ENABLE		(0x1U) /**< To enable any configration */
#define XASUFW_CONFIG_DISABLE		(0x0U) /**< To disable any configration */

/* Counter measures configurations options */
#define XASUFW_ECC_CM_CONFIG		(XASUFW_CONFIG_ENABLE) /**< Counter Measure configuration
									for ECC */
#define XASUFW_AES_CM_CONFIG		(XASUFW_CONFIG_ENABLE) /**< Counter Measure configuration
									for AES */
/**
 * @name Supported ECC curves
 * @{
 */
/**< Macros to enable / disable support of NIST and Brainpool curves */
#define XASUFW_RSA_ECC_SUPPORT_NIST_P521	(XASUFW_CONFIG_ENABLE) /**< P521 curve support \
configuration */
#define XASUFW_RSA_ECC_SUPPORT_NIST_P192	(XASUFW_CONFIG_ENABLE) /**< P192 curve support \
configuration */
#define XASUFW_RSA_ECC_SUPPORT_NIST_P224	(XASUFW_CONFIG_ENABLE) /**< P224 curve support \
configuration */
#define XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P256	(XASUFW_CONFIG_ENABLE) /**< Brainpool P256 curve \
support configuration */
#define XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P320	(XASUFW_CONFIG_ENABLE) /**< Brainpool P320 curve \
support configuration */
#define XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P384	(XASUFW_CONFIG_ENABLE) /**< Brainpool P384 curve \
support configuration */
#define XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P512	(XASUFW_CONFIG_ENABLE) /**< Brainpool P512 curve \
support configuration */
/** @} */

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
//#define ASUFW_PRINT
//#define ASUFW_DEBUG
//#define ASUFW_DEBUG_INFO
#define ASUFW_DEBUG_DETAILED

/** Run TRNG in DRBG mode */
//#define XASUFW_TRNG_ENABLE_DRBG_MODE

/** Run TRNG in PTRNG mode */
//#define XASUFW_TRNG_ENABLE_PTRNG_MODE

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_CONFIG_H */
/** @} */
