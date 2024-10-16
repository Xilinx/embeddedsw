/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_hmac.h
*
* This file contains APIs which calculate the HMAC on provided data and key.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   vns 05/30/22 Initial release
*       kpt 07/24/22 Moved XSecure_HmacKat into xsecure_kat_plat.c
* 5.4   yog 04/29/24 Fixed doxygen grouping and doxygen warnings.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_hmac_apis Xilsecure HMAC APIs
* @{
*/
/**@cond xsecure_internal
 * @{
 */
#ifndef XSECURE_HMAC_H
#define XSECURE_HMAC_H

#ifdef __cplusplus
extern "C" {
#endif

/************************** Include Files ***********************************/

#include "xsecure_sha.h"

/************************** Constant Definitions ****************************/

/************************** Type Definitions ********************************/
/** Stores the resultant HMAC */
typedef struct {
	u8 Hash[XSECURE_HASH_SIZE_IN_BYTES]; /**< Hash value*/
} XSecure_HmacRes;

/**
 * The HMAC driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */
typedef struct {
	XSecure_Sha3 *Sha3InstPtr; /**< Sha3 instance pointer*/
	u8 IPadRes[XSECURE_SHA3_BLOCK_LEN]; /**< Ipad resultant value*/
	u8 OPadRes[XSECURE_SHA3_BLOCK_LEN]; /**< Opad resultant value*/
} XSecure_Hmac;

/************************** Function Prototypes ******************************/
int XSecure_HmacInit(XSecure_Hmac *InstancePtr,
					XSecure_Sha3 *Sha3InstancePtr,
					u64 KeyAddr, u32 KeyLen);
int XSecure_HmacUpdate(XSecure_Hmac *InstancePtr, u64 DataAddr, u32 Len);
int XSecure_HmacFinal(XSecure_Hmac *InstancePtr, XSecure_HmacRes *Hmac);

/**
 * @}
 * @endcond
 */
#ifdef __cplusplus
extern "C" }
#endif

#endif /* XSECURE_HMAC_H_ */
/** @} */
