/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_trnginfo.h
 *
 * This file contains the TRNG definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/23/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_TRNGINFO_H
#define XASU_TRNGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/* TRNG module command IDs */
#define XASU_TRNG_GET_RANDOM_BYTES_CMD_ID		0U /**< Command ID for TRNG Get Random Bytes cmd */
#define XASU_TRNG_KAT_CMD_ID					1U /**< Command ID for TRNG KAT command */
#define XASU_TRNG_GET_INFO_CMD_ID               2U /**< Command ID for TRNG Get Info command */

/* Internal purpose */
#define XASU_TRNG_DRBG_INSTANTIATE_CMD_ID       3U /**< Command ID for TRNG DRBG instantiate cmd */
#define XASU_TRNG_DRBG_RESEED_CMD_ID            4U /**< Command ID for TRNG DRBG reseed cmd */
#define XASU_TRNG_DRBG_GENERATE_CMD_ID          5U /**< Command ID for TRNG DRBG generate cmd */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_TRNGINFO_H */
/** @} */
