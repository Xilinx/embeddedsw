/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsa_fors.c
*
* This file contains definitions for SLH-DSA FORS operations
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
/*************************************** Include Files ********************************************/
#include "xsecure_slhdsa_addr.h"
#include "xsecure_slhdsa_hash.h"
#include "xsecure_slhdsa_fors.h"
#include "xsecure_utils.h"

/************************************ Constant Definitions ****************************************/

/**************************************** Function Prototypes *************************************/

/************************************* Variable Definitions ***************************************/

/************************************* Function Definitions ***************************************/

/**************************************************************************************************/
/**
 * @brief	Computes the base 2^b representation of X, converts a byte string into an array of
 *		integers in the range [0, 2^b - 1].
 *
 * @param	BaseB Output array to store the base 2^b representation.
 * @param	X Input byte string (must have at least (OutLen*b)/8 bytes).
 * @param	b The base parameter (number of bits per output element).
 * @param	OutLen The number of output elements to generate.
 *
 **************************************************************************************************/
void XSecure_SlhdsaBase2b(u32 * const BaseB, const u8 * const X, const u32 b, const u32 OutLen)
{
	u32 In = XSECURE_ZERO;        /* Step 1: in <- 0 */
	u32 Bits = XSECURE_ZERO;      /* Step 2: bits <- 0 */
	u32 Total = XSECURE_ZERO;     /* Step 3: total <- 0 */
	volatile u32 Out;

	/* Step 4: for out from 0 to out_len - 1 do */
	for (Out = 0U; Out < OutLen; Out++) {
		/* Step 5: while bits < b do */
		while (Bits < b) {
			/* Step 6: total <- (total << 8) + X[in] */
			Total = ((Total << XSECURE_BYTE_IN_BITS) + X[In]);
			/* Step 7: in <- in+1 */
			In++;
			/* Step 8: bits <- bits + 8 */
			Bits += XSECURE_BYTE_IN_BITS;
		}
		/* Step 9: end while */

		/* Step 10: bits <- bits - b */
		Bits -= b;

		/* Step 11: baseb[out] <- (total >> bits) mod 2^b */
		BaseB[Out] = ((Total >> Bits) & ((XSECURE_VALUE_ONE << b) - XSECURE_VALUE_ONE));
	}
	/* Step 12: end for */
	/* Step 13: return baseb (implicit through BaseB output parameter) */
}

/**************************************************************************************************/
/**
 * @brief	Computes a FORS public key from a FORS signature.
 *
 * Implements Algorithm 17 (fors_pkFromSig) from FIPS 205.
 * This function reconstructs the FORS public key using the provided signature,
 * message digest, public seed, and address structure as specified in SLH-DSA.
 *
 * @par Algorithm Flow for SHAKE-256s (k=22, a=14, n=32):
 *
 * === SIGNATURE STRUCTURE ===
 * Each tree signature:
 * [secret_value (32B)] || [auth[0] (32B)] || [auth[1] (32B)] || ... || [auth[13] (32B)]
 * Total per tree: 32 + (14 × 32) = 480 bytes
 *
 * Complete FORS signature layout:
 * ┌─────────────┬─────────────┬─────────────┬─────────────┐
 * │ Tree 0 Sig  │ Tree 1 Sig  │     ...     │ Tree 21 Sig │
 * │ (480 bytes) │ (480 bytes) │             │ (480 bytes) │
 * └─────────────┴─────────────┴─────────────┴─────────────┘
 * Total: 22 × 480 = 10,560 bytes
 *
 * Given: BaseB[5] = 12345 (from base_2b conversion), Binary: 0b11000000111001
 *
 * === ADDRESS STRUCTURE PROGRESSION ===
 * Initial: ADRS.setTreeAddress(i), ADRS.setType(FORS_TREE)
 *
 * For each j iteration:
 * ┌─────────────────────────────────────────────────────────┐
 * │ Step 9: ADRS.setTreeHeight(j+1)                         │
 * │ Step 11/14: ADRS.setTreeIndex(calculated_parent_index)  │
 * │ Used in: H(PK.seed, ADRS, left_input || right_input)    │
 * └─────────────────────────────────────────────────────────┘
 *
 * Final: ADRS.setType(FORS_ROOTS) for Step 24
 *
 * === INITIAL SETUP ===
 * Tree Index (i=5): Idx = (5 << 14) + 12345 = 81920 + 12345 = 94265
 * NodePtr = &Data1[160], SignPtr = &SignFors[2400]
 *
 * === FORS Tree Hierarchy (for SHAKE-256s): ===
 *
 * Height 0 (Leaves):     360,448 nodes (22 trees × 16,384 leaves each)
 * Height 1:              180,224 nodes (360,448 ÷ 2)
 * Height 2:               90,112 nodes (180,224 ÷ 2)
 * Height 3:               45,056 nodes
 * ...
 * Height 13:                 44 nodes
 * Height 14 (Roots):         22 nodes (1 root per FORS tree)
 *
 * Each level has half the nodes of the level below.
 *
 * Tree 5 Range: Global indices 81920 to 98303
 *
 * Height 0: Leaves 81920-98303 (16,384 leaves)
 * Height 1: Parents 40960-49151 (8,192 nodes) ✓ (81920>>1 to 98303>>1)
 * Height 2: Parents 20480-24575 (4,096 nodes) ✓ (40960>>1 to 49151>>1)
 * ...
 * Height 14: Root = 5 (single root node)
 *
 * Leaf 94265 progression:
 * Height 0: 94265 (leaf)
 * Height 1: 47132 (parent) ✓
 * Height 2: 23566 (grandparent)
 * Height 3: 11783
 * ...
 * Height 14: 5 (Tree 5's root)
 *
 * === LEAF COMPUTATION ===
 * Step 4-6: TreeHeight=0, TreeIndex=94265
 *           NodePtr = F(PK.seed, ADRS, SignPtr[0:31])
 *           SignPtr advances to auth[0]
 *
 * === TREE TRAVERSAL TABLE ===
 * +----+------------+-----------+------+-------+------------------+----------+
 * | j  | TreeHeight | TreeIndex | Bit  | Path  | Hash Operation   | Alg Step |
 * +----+------------+-----------+------+-------+------------------+----------+
 * | 0  |     1      |   47132   |  1   | ODD   | auth[0] || node  | Step 15  |
 * | 1  |     2      |   23566   |  0   | EVEN  | node || auth[1]  | Step 12  |
 * | 2  |     3      |   11783   |  0   | EVEN  | node || auth[2]  | Step 12  |
 * | 3  |     4      |    5891   |  1   | ODD   | auth[3] || node  | Step 15  |
 * | 4  |     5      |    2945   |  1   | ODD   | auth[4] || node  | Step 15  |
 * | 5  |     6      |    1472   |  1   | ODD   | auth[5] || node  | Step 15  |
 * | 6  |     7      |     736   |  0   | EVEN  | node || auth[6]  | Step 12  |
 * | 7  |     8      |     368   |  0   | EVEN  | node || auth[7]  | Step 12  |
 * | 8  |     9      |     184   |  0   | EVEN  | node || auth[8]  | Step 12  |
 * | 9  |    10      |      92   |  0   | EVEN  | node || auth[9]  | Step 12  |
 * | 10 |    11      |      46   |  0   | EVEN  | node || auth[10] | Step 12  |
 * | 11 |    12      |      23   |  1   | ODD   | auth[11] || node | Step 15  |
 * | 12 |    13      |      11   |  1   | ODD   | auth[12] || node | Step 15  |
 * | 13 |    14      |       5   |  1   | ODD   | auth[13] || node | Step 15  |
 * +----+------------+-----------+------+-------+------------------+----------+
 *
 * === CALCULATION FORMULAS ===
 * TreeIndex = Idx >> (j+1) = 94265 >> (j+1)
 * Bit = (BaseB[i] >> j) & 1 = (12345 >> j) & 1
 *
 * === MEMORY LAYOUT ===
 * +--------+------------------+------------------------+
 * | Tree i | NodePtr Location | SignPtr Range (bytes)  |
 * +--------+------------------+------------------------+
 * |   0    |   Data1[0:31]    |     [0:479]            |
 * |   1    |   Data1[32:63]   |     [480:959]          |
 * |   2    |   Data1[64:95]   |     [960:1439]         |
 * |   3    |   Data1[96:127]  |     [1440:1919]        |
 * |   4    |   Data1[128:159] |     [1920:2399]        |
 * |   5    |   Data1[160:191] |     [2400:2879] <-     |
 * |  ...   |       ...        |        ...             |
 * |  21    |   Data1[672:703] |     [10080:10559]      |
 * +--------+------------------+------------------------+
 *
 * === FINAL COMPRESSION ===
 * Step 24: PkFors = T_k(PK.seed, forspkADRS, Data1[0:703])
 *          Input:  704 bytes (22 roots × 32 bytes)
 *          Output: 32 bytes (Final FORS public key)
 *
 * === PERFORMANCE CHARACTERISTICS ===
 * Computational Complexity:
 * - Hash operations: k × (1 + a) = 22 × 15 = 330 hash calls
 * - Time complexity: O(k × a × n)
 *
 * Memory Usage:
 * - Temporary storage: 704 bytes (22 roots)
 * - Signature input: 10,560 bytes
 * - Working memory: Constant (reuses NodePtr)
 *
 * @param	SignForsAddr	64-bit address of the FORS signature data (10,560 bytes for SHAKE-256s).
 * @param	Md		Pointer to the message digest.
 * @param	PublicKeySeedAddr 64-bit address of the public key seed.
 * @param	PkFors		Pointer to the buffer where the computed FORS public key will be
 *				stored.
 *
 * @return
 *		- XST_SUCCESS if the public key is computed successfully.
 *		- XSECURE_SLH_DSA_FORS_COMPUTE_LEAF_ERROR if FORS leaf computation fails.
 *		- XSECURE_SLH_DSA_SIGN_VERIFY_FORS_NODE_ERROR if FORS tree node computation fails.
 *		- XSECURE_SLH_DSA_FORS_ROOT_ERROR if FORS root computation fails.
 *
 **************************************************************************************************/
int XSecure_SlhdsaForsPkFromSig(const u64 SignForsAddr, const u8 * const Md,
				const u64 PublicKeySeedAddr, u8 * const PkFors)
{
	volatile int Status = XST_FAILURE;
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();
	u8* NodePtr = InstancePtr->Data1;
	volatile u32 TreeIdx;
	volatile u32 NodeIdx;
	u64 SignPtrAddr = SignForsAddr;
	/**
	 * Used to represent Global lead address across all FORS Trees, range: 0 to 360,448; 0 to
	 * 16,383 for first tree
	 */
	u32 Idx;

	/* Step 1: indices <- base_2b(md, a, k) */
	(void)XSecure_SlhdsaBase2b(InstancePtr->Data4, Md, (u32)InstancePtr->Param->a,
				   (u32)InstancePtr->Param->k);

	/* 2: for i from 0 to k - 1 do */
	for (TreeIdx = 0; TreeIdx < (u32)InstancePtr->Param->k; TreeIdx++) {

		/* Step 4: ADRS.setTreeHeight(0) */
		XSecure_SlhdsaSetTreeHeight(InstancePtr->Addr, XSECURE_ZERO);

		/* Step 5: ADRS.setTreeIndex(TreeIdx * 2^a + indices[TreeIdx]) */
		Idx = ((TreeIdx << (u32)InstancePtr->Param->a) + InstancePtr->Data4[TreeIdx]);
		XSecure_SlhdsaSetTreeIndex(InstancePtr->Addr, Idx);

		/* Step 6: node[0] <- F(PK.seed, ADRS, sk = current location of signature) */
		XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_SlhdsaShake256sHashF,
			PublicKeySeedAddr,		/* [in] Public key seed address */
			SignPtrAddr,			/* [in] FORS signature element address */
			NodePtr);			/* [out] Computed leaf node */

		SignPtrAddr += (u64)InstancePtr->Param->n;

		/* Step 8: for NodeIdx from 0 to a-1 do */
		for (NodeIdx = 0U; NodeIdx < (u32)InstancePtr->Param->a; NodeIdx++) {
			/* Step 9: ADRS.setTreeHeight(NodeIdx+1) */
			XSecure_SlhdsaSetTreeHeight(InstancePtr->Addr, NodeIdx + XSECURE_VALUE_ONE);

			/*
			 * Step 11: ADRS.setTreeIndex(ADRS.getTreeIndex()/2)
			 * Step 14: ADRS.setTreeIndex((ADRS.getTreeIndex() - 1)/2)
			 */
			XSecure_SlhdsaSetTreeIndex(InstancePtr->Addr, Idx >> (NodeIdx +
						XSECURE_VALUE_ONE));

			/* Step 10/13: even/odd index logic */
			if (!XSECURE_IS_ODD((u32)(InstancePtr->Data4[TreeIdx] >> NodeIdx))) {
				/* Step 12: node[1] <- H(PK.seed, ADRS, node[0] || auth[NodeIdx]) */
				XSECURE_TEMPORAL_CHECK(END,
					Status,
					XSecure_SlhdsaShake256sHashH,
					PublicKeySeedAddr,	/* [in] Public key seed address */
					(u64)(UINTPTR)NodePtr,	/* [in] Current node value address */
					SignPtrAddr,		/* [in] Auth path element address */
					NodePtr);		/* [out] Updated node value */
			} else {
				/* Step 15: node[1] <- H(PK.seed, ADRS, auth[NodeIdx] || node[0]) */
				XSECURE_TEMPORAL_CHECK(END,
					Status,
					XSecure_SlhdsaShake256sHashH,
					PublicKeySeedAddr,	/* [in] Public key seed address */
					SignPtrAddr,		/* [in] Auth path element address */
					(u64)(UINTPTR)NodePtr,	/* [in] Current node value address */
					NodePtr);		/* [out] Updated node value */
			}
			/* Step 17: node[0] <- node[1] (NodePtr already updated) */
			SignPtrAddr += (u64)InstancePtr->Param->n;
		}

		if (NodeIdx != (u32)InstancePtr->Param->a) {
			Status = XSECURE_ERR_GLITCH_DETECTED;
			goto END;
		}

		/* Step 19: root[TreeIdx] <- node[0] (NodePtr points to root for this tree) */
		NodePtr += (u32)InstancePtr->Param->n;
	}

	if (TreeIdx != (u32)InstancePtr->Param->k) {
		Status = XSECURE_ERR_GLITCH_DETECTED;
		goto END;
	}

	XSecure_SlhdsaSetTypeClearNotKp(InstancePtr->Addr, XSECURE_SLH_DSA_ADRS_TYPE_FORS_ROOTS);

	/* Step 24: pk <- T_k(PK.seed, forspkADRS, root) */
	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_SlhdsaShake256sHashTl,
		PublicKeySeedAddr,				/* [in] Public key seed address */
		InstancePtr->Data1,				/* [in] FORS roots array */
		((u32)InstancePtr->Param->k * (u32)InstancePtr->Param->n),
								/* [in] Length of roots data */
		PkFors);					/* [out] FORS public key output */

END:
	return Status;
}
/** @} */
