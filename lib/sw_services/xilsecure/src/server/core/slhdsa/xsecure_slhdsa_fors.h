/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/slhdsa/xsecure_slhdsa_fors.h
*
* This file contains functions and defines for FORS (Forest Of Random Subsets) operations used in
* SLH-DSA signature verification
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
#ifndef XSECURE_SLH_DSA_FORS_H_
#define XSECURE_SLH_DSA_FORS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xsecure_slhdsa_instance.h"

/**************************************** Function Prototypes *************************************/
void XSecure_SlhdsaBase2b(u32 * const BaseB, const u8 * const X, const u32 b, const u32 OutLen);

int XSecure_SlhdsaForsPkFromSig(const u64 SignForsAddr, const u8 * const Md,
				const u64 PublicKeySeedAddr, u8 * const PkFors);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLH_DSA_FORS_H_ */
/** @} */
