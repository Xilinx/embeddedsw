/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsa_wots.h
*
* This file consists of declarations for SLH-DSA WOTS+ operations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_slhdsa_server_apis XilSecure SLHDSA Server APIs
* @{
*/
#ifndef XSECURE_SLH_DSA_WOTS_H_
#define XSECURE_SLH_DSA_WOTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xsecure_slhdsa_instance.h"
#include "xil_types.h"

/*************************************** Function Prototypes **************************************/
int XSecure_SlhdsaWotsPkFromSign(u64 WotsSignAddr, u64 MAddr, u64 PublicKeyAddr, u8 * const WotsPk);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLH_DSA_WOTS_H_ */
/** @} */
