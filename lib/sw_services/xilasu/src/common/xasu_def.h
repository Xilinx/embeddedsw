/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_DEF_H
#define XASU_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
/* Module ID */
#define XASU_MODULE_TRNG_ID	(0U) /**< TRNGs module ID */
#define XASU_MODULE_SHA2_ID	(1U) /**< SHA2 module ID */
#define XASU_MODULE_SHA3_ID	(2U) /**< SHA3 module ID */
#define XASU_MODULE_ECC_ID	(3U) /**< ECC module ID */
#define XASU_MODULE_RSA_ID	(4U) /**< RSA module ID */
#define XASU_MODULE_AES_ID	(5U) /**< AES module ID */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_DEF_H */
/** @} */
