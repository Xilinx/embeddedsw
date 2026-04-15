/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/slhdsa/xsecure_slhdsa_addr.h
*
* This file contains the SLH-DSA address structure (ADRS) and manipulation functions.
* The ADRS provides domain separation for hash functions across different SLH-DSA operations
* including FORS, WOTS+, XMSS, and Hypertree.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*
* </pre>
*
* @note
* All multi-byte values are stored in big-endian order within the ADRS structure.
*
***************************************************************************************************/

#ifndef XSECURE_SLH_DSA_ADDR_H_
#define XSECURE_SLH_DSA_ADDR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_io.h"
#include "xsecure_utils.h"

/***************************************** Constant Definitions ***********************************/
/** @cond xsecure_internal
 * @{
 */

#define XSECURE_IS_ODD(Value)		((Value & 0x00000001U) == 0x00000001U) /**< Odd check */

/* ADRS word index macros */
#define XSECURE_SLH_DSA_ADDR_WORD_LAYER		(0U)	/**< Layer address word index */
#define XSECURE_SLH_DSA_ADDR_WORD_TREE_2	(1U)	/**< Tree address high bits word index */
#define XSECURE_SLH_DSA_ADDR_WORD_TREE_1	(2U)	/**< Tree address middle bits word index */
#define XSECURE_SLH_DSA_ADDR_WORD_TREE_0	(3U)	/**< Tree address low bits word index */
#define XSECURE_SLH_DSA_ADDR_WORD_TYPE		(4U)	/**< Type field word index */
#define XSECURE_SLH_DSA_ADDR_WORD_KEYPAIR	(5U)	/**< Keypair address word index */
#define XSECURE_SLH_DSA_ADDR_WORD_CHAIN_HEIGHT	(6U)	/**< Chain/tree height word index */
#define XSECURE_SLH_DSA_ADDR_WORD_HASH_INDEX	(7U)	/**< Hash/tree index word index */

/* Address type identifiers for SLH_DSA operations */
#define XSECURE_SLH_DSA_ADRS_TYPE_WOTS_HASH	(0U)	/**< WOTS+ hash address type */
#define XSECURE_SLH_DSA_ADRS_TYPE_WOTS_PK	(1U)	/**< WOTS+ public key address type */
#define XSECURE_SLH_DSA_ADRS_TYPE_TREE		(2U)	/**< XMSS tree address type */
#define XSECURE_SLH_DSA_ADRS_TYPE_FORS_TREE	(3U)	/**< FORS tree address type */
#define XSECURE_SLH_DSA_ADRS_TYPE_FORS_ROOTS	(4U)	/**< FORS roots address type */

/* Address size definitions for SLH_DSA */
#define XSECURE_SLH_DSA_ADDR_SIZE_IN_WORDS	(8U)	/**< ADRS size in words */
#define XSECURE_SLH_DSA_ADDR_SIZE_IN_BYTES	(32U)	/**< ADRS size in bytes */

/**
 * Represents an address structure that allows access to the same data as either an array of 8
 * 32-bit words or as an array of 32 bytes.
 *
 * Mapping between word and byte indexes:
 *
 *   | Word Index | Byte Index Range | Field Name         |
 *   |------------|------------------|--------------------|
 *   |    0       |   0  -  3        | Layer Address      |
 *   |    1       |   4  -  7        | Reserved/Unused    |
 *   |    2       |   8  - 11        | Tree Address High  |
 *   |    3       |  12  - 15        | Tree Address Low   |
 *   |    4       |  16  - 19        | Type               |
 *   |    5       |  20  - 23        | Key Pair Address   |
 *   |    6       |  24  - 27        | Chain/Tree Height  |
 *   |    7       |  28  - 31        | Hash/Tree Index    |
 *
 */
typedef union {
	u32 AdrsWord[XSECURE_SLH_DSA_ADDR_SIZE_IN_WORDS];	/**< ADRS as 32-bit words */
	u8 AdrsByte[XSECURE_SLH_DSA_ADDR_SIZE_IN_BYTES];	/**< ADRS as byte array */
} ADRS;

/**************************************************************************************************/
/**
 * @brief	This function converts an n-byte string to a 64-bit integer value using big-endian
 * 		byte order. It processes bytes from index 0 to n-1, with each byte contributing to
 * 		the result as: total = 256 * total + X[i].
 *
 * @param	X Pointer to the byte string to convert. Must not be NULL.
 * @param	n Number of bytes in the string to convert (must be <= 8 for u64 result).
 *
 * @return
 * 		- 64-bit unsigned integer value representing the byte string in big-endian order.
 *
 **************************************************************************************************/
static inline u64 XSecure_BytesToU64(const u8 *X, u32 n)
{
	u64 total = 0ULL;
	u32 i;

	for (i = 0U; i < n; i++) {
		total = (total << XSECURE_BYTE_IN_BITS) + (u64)X[i];
	}

	return total;
}

/**************************************************************************************************/
/**
 * @brief	This function clears the SLH_DSA address structure by setting all words to zero.
 *
 * @param	InstancePtr Pointer to the ADRS structure to be cleared.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaClearAddress(ADRS * const InstancePtr)
{
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_LAYER] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TREE_2] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TREE_1] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TREE_0] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TYPE] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_KEYPAIR] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_CHAIN_HEIGHT] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_HASH_INDEX] = (u32)XSECURE_ZERO;
}

/**************************************************************************************************/
/**
 * @brief	Sets the layer address field in the ADRS structure. Keeps rest of places unchanged.
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	LayerAddr Value to set for layer address.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetLayerAddress(ADRS * const InstancePtr, const u32 LayerAddr)
{
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_LAYER] = Xil_Htonl(LayerAddr);
}

/**************************************************************************************************/
/**
 * @brief	Sets the tree address fields (high/low) in the ADRS structure.
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	TreeAddress 64-bit tree address value.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetTreeAddress(ADRS * const InstancePtr, const u64 TreeAddress)
{
	/* word LAYER, TYPE, KEYPAIR, CHAIN_HEIGHT, HASH_INDEX remains unchanged */
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TREE_2] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TREE_1] = Xil_Htonl((u32)(TreeAddress >>
									    XSECURE_WORD_IN_BITS));
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TREE_0] = Xil_Htonl((u32)(TreeAddress &
									    XSECURE_ALLFS));
}

/**************************************************************************************************/
/**
 * @brief	Sets the type field and clears keypair, chain height, and hash index fields.
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	Type Value to set for type field.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetTypeAndClear(ADRS * const InstancePtr, const u32 Type)
{
	/* words LAYER, TREE_2, TREE_1, TREE_0 remains unchanged */
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TYPE] = Xil_Htonl(Type);
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_KEYPAIR] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_CHAIN_HEIGHT] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_HASH_INDEX] = (u32)XSECURE_ZERO;
}

/**************************************************************************************************/
/**
 * @brief	Sets the type field and clears chain height and hash index fields (not keypair).
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	Val Value to set for type field.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetTypeClearNotKp(ADRS * InstancePtr, u32 Val)
{
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_TYPE] = Xil_Htonl(Val);
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_CHAIN_HEIGHT] = (u32)XSECURE_ZERO;
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_HASH_INDEX] = (u32)XSECURE_ZERO;
}

/**************************************************************************************************/
/**
 * @brief	Sets the keypair address field in the ADRS structure.
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	Val Value to set for keypair address.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetKeyPairAddress(ADRS * const InstancePtr, const u32 Val)
{
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_KEYPAIR] = Xil_Htonl(Val);
}

/**************************************************************************************************/
/**
 * @brief	Sets the chain address field in the ADRS structure.
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	Val Value to set for chain address.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetChainAddress(ADRS * const InstancePtr, const u32 Val)
{
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_CHAIN_HEIGHT] = Xil_Htonl(Val);
}

/**************************************************************************************************/
/**
 * @brief	Sets the tree height field in the ADRS structure.
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	Val Value to set for tree height.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetTreeHeight(ADRS * const InstancePtr, const u32 Val)
{
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_CHAIN_HEIGHT] = Xil_Htonl(Val);
}

/**************************************************************************************************/
/**
 * @brief	Sets the hash address field in the ADRS structure.
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	Val Value to set for hash address.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetHashAddress(ADRS * const InstancePtr, const u32 Val)
{
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_HASH_INDEX] = Xil_Htonl(Val);
}

/**************************************************************************************************/
/**
 * @brief	Sets the tree index field in the ADRS structure.
 *
 * @param	InstancePtr Pointer to ADRS structure.
 * @param	Val Value to set for tree index.
 *
 **************************************************************************************************/
static inline void XSecure_SlhdsaSetTreeIndex(ADRS * const InstancePtr, const u32 Val)
{
	InstancePtr->AdrsWord[XSECURE_SLH_DSA_ADDR_WORD_HASH_INDEX] = Xil_Htonl(Val);
}

/**************************************************************************************************/
/**
 * @brief	This function computes the ceiling of the division of two unsigned 32-bit integers.
 *
 * @param	Num Dividend (numerator) value.
 * @param	Den Divisor (denominator) value. Must not be zero.
 *
 * @return
 * 		- Ceiling of Num divided by Den as an unsigned 32-bit integer.
 *
 **************************************************************************************************/
static inline u32 XSecure_SlhCeilDivU32(u32 Num, u32 Den)
{
	u32 quotient = Num / Den;
	u32 rem = Num % Den;

	return quotient + (u32)(rem != XSECURE_ZERO);
}

/**************************************************************************************************/
/**
 * @brief	This function computes the modulo of a 64-bit unsigned integer with a power of 2
 * 		(x mod (2^k)) using bitwise operations.
 *
 * @param	x The value to be reduced modulo 2^k.
 * @param	k The exponent of the power of 2 modulus (i.e., compute x mod 2^k).
 *
 * @return
 * 		- 0 if k is 0.
 * 		- x unchanged if k >= 64.
 * 		- x mod (2^k) otherwise, computed as x & ((1 << k) - 1).
 *
 **************************************************************************************************/
static inline u64 XSecure_ModPow2U64(u64 x, u64 k)
{
	u64 result;

	if (k == 0ULL) {
		result = 0ULL;
	} else if (k >= 64ULL) {
		result = x;
	} else {
		result = x & ((1ULL << k) - 1ULL);
	}

	return result;
}

/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLH_DSA_ADDR_H_ */
