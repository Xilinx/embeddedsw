/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsa_hash.h
*
* This file contains function declarations for SLH-DSA hash operations including message hashing,
* F function, chain operations, and other cryptographic hash computations required by the SLH-DSA
* algorithm
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
#ifndef XSECURE_SLH_DSA_HASH_H_
#define XSECURE_SLH_DSA_HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xsecure_slhdsa_instance.h"
#include "xil_types.h"

/***************************************** Type Definitions ***************************************/
/**
 * @brief Structure to maintain SLH-DSA chain configuration parameters
 */
typedef struct {
	u32 StartIdx;		/**< Start index for chain operation */
	u32 Steps;		/**< Number of steps in the chain */
} XSecure_SlhdsaChainConfig;

int XSecure_SlhdsaShake256sHashMsg(void);

int XSecure_SlhdsaShake256sHashF(const u64 PublicKeyAddr, const u64 InputAddr,
				 u8 * const Output);

int XSecure_SlhdsaShake256sChain(const u64 InputAddr,
				 const XSecure_SlhdsaChainConfig * const ChainConfig,
				 const u64 PublicKeyAddr, u8 * const Output);

int XSecure_SlhdsaShake256sHashTl(const u64 PublicKeyAddr, const u8 * const Input, u32 InputLen,
				  u8 * const Output);

int XSecure_SlhdsaShake256sHashH(const u64 PublicKeyAddr, const u64 Data1Addr,
				 const u64 Data2Addr, u8 * const Output);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLH_DSA_HASH_H_ */
/** @} */
