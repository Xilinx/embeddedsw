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

#define XASU_ASU_DMA_MAX_TRANSFER_LENGTH	(0x1FFFFFFCU)
						/** < ASU DMA maximum transfer rate in bytes. */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_DEF_H_ */
/** @} */
