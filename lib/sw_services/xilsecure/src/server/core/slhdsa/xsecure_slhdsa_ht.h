/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/slhdsa/xsecure_slhdsa_ht.h
*
* This file contains function declarations for SLH-DSA hypertree signature verification operations
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
#ifndef XSECURE_SLH_DSA_HT_H_
#define XSECURE_SLH_DSA_HT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xsecure_slhdsa_instance.h"
#include "xil_types.h"

/***************************************** Type Definitions ***************************************/
/**
 * @brief Structure to maintain SLH-DSA hypertree indices
 */
typedef struct {
	u64 IdxTree;		/**< Tree index for hypertree verification */
	u64 IdxLeaf;		/**< Leaf index for hypertree verification */
} XSecure_SlhdsaHtIndices;

/**************************************** Function Prototypes *************************************/
int XSecure_SlhdsaHtVerify(const u8* const M,
			   const u64 SignHtAddr, const u64 PublicKeyAddr,
			   const XSecure_SlhdsaHtIndices * const HtIndices);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLH_DSA_HT_H_ */
