/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/slhdsa/xsecure_slhdsa_xmss.c
*
* This file contains definitions for SLH-DSA XMSS operations
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
#include "xsecure_slhdsa_wots.h"
#include "xsecure_slhdsa_addr.h"
#include "xsecure_slhdsa_hash.h"
#include "xsecure_slhdsa_xmss.h"

/************************************ Constant Definitions ****************************************/

/**************************************** Function Prototypes *************************************/

/************************************* Variable Definitions ***************************************/

/************************************* Function Definitions ***************************************/

/**************************************************************************************************/
/**
 * @brief	This function reconstructs the XMSS public key (tree root) using the provided XMSS
 *		signature, message, public seed, and address structure. It first computes the WOTS+
 *		public key from the WOTS+ signature, then uses the authentication path to compute
 *		the tree root.
 *
 * @par Algorithm Flow for SHAKE-256s (len=67, h'=8, n=32):
 *
 * Given: IdxLeaf=58, M=FORS_PK (32 bytes), SignTmp=XMSS_signature (2400 bytes)
 * Binary: IdxLeaf=58 = 0b00111010
 *
 * === XMSS SIGNATURE STRUCTURE ===
 * SignTmp layout (2400 bytes total):
 * ┌─────────────────┬──────────────────────────────────────────────────┐
 * │ WOTS+ Signature │ XMSS Authentication Path                         │
 * │ (len × n bytes) │ (h' × n bytes)                                   │
 * │ (67 × 32 = 2144)│ (8 × 32 = 256 bytes)                             │
 * └─────────────────┴──────────────────────────────────────────────────┘
 *
 * WOTS+ Sig: SignTmp[0:2143]      - Used to compute leaf node
 * AUTH Path: SignTmp[2144:2399]   - Used to compute tree root
 *
 * === INITIAL SETUP ===
 * Input indices: IdxLeaf = 58 (decimal) = 0b00111010 (binary)
 *
 * ADRS initialization:
 *   - setTypeAndClear(WOTS_HASH) for WOTS+ computation
 *   - setKeyPairAddress(58) for leaf identification
 *
 * === WOTS+ PUBLIC KEY COMPUTATION ===
 * Step 5: node[0] = wots_pkFromSig(SignTmp[0:2143], M, PK.seed, ADRS)
 * Result: 32-byte WOTS+ public key representing leaf 58
 *
 * === TREE TRAVERSAL SETUP ===
 * Switch ADRS to TREE type for authentication path processing
 * SignXmssOffset = &SignTmp[2144] (points to AUTH[0])
 *
 * === XMSS TREE TRAVERSAL TABLE ===
 * +---+------------+-----------+--------+----------+------------------+--------------------+
 * | k | TreeHeight | TreeIndex | Bit    | Position | Hash Operation   | AUTH Usage         |
 * +---+------------+-----------+--------+----------+------------------+--------------------+
 * | 0 |     1      |    29     | 0 EVEN | LEFT     | node || AUTH[0]  | SignXmssOffset[0]  |
 * | 1 |     2      |    14     | 1 ODD  | RIGHT    | AUTH[1] || node  | SignXmssOffset[32] |
 * | 2 |     3      |     7     | 0 EVEN | LEFT     | node || AUTH[2]  | SignXmssOffset[64] |
 * | 3 |     4      |     3     | 1 ODD  | RIGHT    | AUTH[3] || node  | SignXmssOffset[96] |
 * | 4 |     5      |     1     | 1 ODD  | RIGHT    | AUTH[4] || node  | SignXmssOffset[128]|
 * | 5 |     6      |     0     | 1 ODD  | RIGHT    | AUTH[5] || node  | SignXmssOffset[160]|
 * | 6 |     7      |     0     | 0 EVEN | LEFT     | node || AUTH[6]  | SignXmssOffset[192]|
 * | 7 |     8      |     0     | 0 EVEN | LEFT     | node || AUTH[7]  | SignXmssOffset[224]|
 * +---+------------+-----------+--------+----------+------------------+--------------------+
 *
 * === CALCULATION FORMULAS ===
 * TreeHeight = k + 1
 * TreeIndex = IdxLeaf >> (k + 1) = 58 >> (k + 1)
 * Bit = (IdxLeaf >> k) & 1 = (58 >> k) & 1
 *
 * === DETAILED BIT ANALYSIS ===
 * IdxLeaf = 58 = 0b00111010
 *
 * k=0: (58 >> 0) & 1 = 0 (EVEN) → node is LEFT child,  AUTH[0] is RIGHT sibling → node || AUTH[0]
 * k=1: (58 >> 1) & 1 = 1 (ODD)  → node is RIGHT child, AUTH[1] is LEFT sibling  → AUTH[1] || node
 * k=2: (58 >> 2) & 1 = 0 (EVEN) → node is LEFT child,  AUTH[2] is RIGHT sibling → node || AUTH[2]
 * k=3: (58 >> 3) & 1 = 1 (ODD)  → node is RIGHT child, AUTH[3] is LEFT sibling  → AUTH[3] || node
 * k=4: (58 >> 4) & 1 = 1 (ODD)  → node is RIGHT child, AUTH[4] is LEFT sibling  → AUTH[4] || node
 * k=5: (58 >> 5) & 1 = 1 (ODD)  → node is RIGHT child, AUTH[5] is LEFT sibling  → AUTH[5] || node
 * k=6: (58 >> 6) & 1 = 0 (EVEN) → node is LEFT child,  AUTH[6] is RIGHT sibling → node || AUTH[6]
 * k=7: (58 >> 7) & 1 = 0 (EVEN) → node is LEFT child,  AUTH[7] is RIGHT sibling → node || AUTH[7]
 *
 * === TREE STRUCTURE VISUALIZATION ===
 *
 * Height 8:                    [ROOT]
 *                             /      \
 * Height 7:            node_7        AUTH[7]
 *                     /      \
 * Height 6:      node_6      AUTH[6]
 *               /      \
 * Height 5: node_5    AUTH[5]
 *          /      \
 * Height 4: AUTH[4]  node_4
 *                   /      \
 * Height 3:   AUTH[3]    node_3
 *                       /      \
 * Height 2:       AUTH[2]    node_2
 *                           /      \
 * Height 1:           AUTH[1]    node_1
 *                               /      \
 * Height 0:                WOTS+[58]  AUTH[0]  <- Leaf level (IdxLeaf=58)
 *
 * Key:
 * - WOTS+[58] is at position 58 (EVEN), so it's the LEFT child
 * - AUTH[0] is the RIGHT sibling of WOTS+[58]
 * - Hash: WOTS+[58] || AUTH[0] (left || right)
 *
 * === ADDRESS PROGRESSION ===
 * Each iteration updates ADRS fields:
 * +---+------------------+-----------------------------+
 * | k | TreeHeight(k+1)  | TreeIndex(IdxLeaf>>(k+1))   |
 * +---+------------------+-----------------------------+
 * | 0 |        1         |      29 (58>>1)             |
 * | 1 |        2         |      14 (58>>2)             |
 * | 2 |        3         |       7 (58>>3)             |
 * | 3 |        4         |       3 (58>>4)             |
 * | 4 |        5         |       1 (58>>5)             |
 * | 5 |        6         |       0 (58>>6)             |
 * | 6 |        7         |       0 (58>>7)             |
 * | 7 |        8         |       0 (58>>8)             |
 * +---+------------------+-----------------------------+
 *
 * === MEMORY MANAGEMENT ===
 * Node buffer usage (in-place updates):
 * - Initial: Contains WOTS+ public key (leaf value)
 * - k=0: Contains parent of leaf 58
 * - k=1: Contains grandparent
 * - ...
 * - k=7: Contains final XMSS root (32 bytes)
 *
 * SignXmssOffset progression:
 * - k=0: &SignTmp[2144] → AUTH[0]
 * - k=1: &SignTmp[2176] → AUTH[1]
 * - k=2: &SignTmp[2208] → AUTH[2]
 * - ...
 * - k=7: &SignTmp[2368] → AUTH[7]
 *
 * === PERFORMANCE CHARACTERISTICS ===
 * Computational Complexity:
 * - Tree traversal: h' hash operations = 8 hashes
 * - Time complexity: O(h' × n) = O(8 × 32) = O(256) per XMSS verification
 *
 * Memory Usage:
 * - Input XMSS signature: 2400 bytes total
 *   * WOTS+ signature portion: 2144 bytes
 *   * Authentication path: 256 bytes (8 × 32)
 * - Working node buffer: 32 bytes (reused in-place)
 * - Authentication path access: Sequential, 32 bytes per iteration
 * - Total XMSS memory: O(signature_size + n)
 *
 * @param	IdxLeaf		Leaf index within the XMSS tree (0 to 2^h'-1).
 * @param	SignTmpAddr	64-bit address of XMSS signature (2400 bytes for SHAKE-256s).
 * @param	MAddr		64-bit address of message to verify (typically FORS public key, 32 bytes).
 * @param	PublicKeyAddr	64-bit address of public key seed for hash computations.
 * @param	Node		Buffer to store computed XMSS root (32 bytes).
 *
 * @return
 *		- XST_SUCCESS if XMSS public key computation succeeds.
 *		- ErrorCode if failure.
 *
 **************************************************************************************************/
int XSecure_SlhdsaXmssPkFromSign(u64 IdxLeaf, u64 SignTmpAddr, u64 MAddr,
                                 u64 PublicKeyAddr, u8* Node)
{
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();
	volatile int Status = XST_FAILURE;
	volatile u32 k;
	u64 SignXmssOffset = SignTmpAddr + (u64)((InstancePtr->Param->ChecksumParams.InputLenInDigits +
					InstancePtr->Param->ChecksumParams.ChecksumLenInDigits) *
					(u32)InstancePtr->Param->n);

	/** - Step 1: ADRS.setTypeAndClear(WOTS_HASH) -> compute WOTS+ pk from WOTS+ sig */
	XSecure_SlhdsaSetTypeClearNotKp(InstancePtr->Addr, XSECURE_SLH_DSA_ADRS_TYPE_WOTS_HASH);

	/** - Step 2: ADRS.setKeyPairAddress(idx) */
	XSecure_SlhdsaSetKeyPairAddress(InstancePtr->Addr, (u32)IdxLeaf);

	/** - Step 3: sig <- SIG_XMSS.getWOTSSig() (implicit: SignTmp points to WOTS+ sig) */
	/** - Step 4: AUTH <- SIG_XMSS.getXMSSAUTH() (implicit: SignXmssOffset points to AUTH) */

	/** - Step 5: node[0] <- wots_pkFromSig(sig, M, PK.seed, ADRS) */
	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_SlhdsaWotsPkFromSign,
		SignTmpAddr,				/* [in] WOTS+ signature address */
		MAddr,					/* [in] Message address */
		PublicKeyAddr,				/* [in] Public key address */
		Node);					/* [out] WOTS+ public key output */

	/** - Step 6: ADRS.setTypeAndClear(TREE) -> compute root from WOTS+ pk and AUTH */
	XSecure_SlhdsaSetTypeAndClear(InstancePtr->Addr, XSECURE_SLH_DSA_ADRS_TYPE_TREE);

	/** - Step 8: for k from 0 to h' - 1 do */
	for (k = 0; k < (u32)InstancePtr->Param->hprime; k++) {
		/** - Step 9: ADRS.setTreeHeight(k + 1) */
		XSecure_SlhdsaSetTreeHeight(InstancePtr->Addr, (u32)(k + XSECURE_VALUE_ONE));
		/*
		 * XMSS/SLH-DSA Tree Addressing: Uniform Node Methodology
		 * --------------------------------------------------------------------------
		 * In XMSS, each node in the binary hash tree is uniquely identified by its
		 * "tree height" (level) and "tree index" (position within that level).
		 * The leaf nodes are at height 0, their parents at height 1, and so on,
		 * up to the root at height h.
		 *
		 * The following addresses the correct node at each tree height by setting:
		 *
		 * the "tree height" field to k+1 (where k is the current layer/iteration)
		 * the "tree index" field to (IdxLeaf >> (k+1))
		 * This works for both odd and even layers, and for all standard binary tree
		 * traversals. At each height, right-shifting the leaf index divides the tree into
		 * blocks of 2^(k+1) leaves.
		 *
		 * Example: For IdxLeaf = 58,
		 * at k+1 = 2 (height 2), tree index = 58 >> 2 = 14 (which is the parent node index
		 *						     at height 2)
		 * at k+1 = 3 (height 3), tree index = 58 >> 3 = 7
		 * This approach cleanly generalizes node addressing for all heights, avoiding the
		 * need for special-case logic.
		 * Reference: NIST SPHINCS+/SLH-DSA and XMSS tree addressing logic.
		 */
		XSecure_SlhdsaSetTreeIndex(InstancePtr->Addr, (u32)((IdxLeaf >> (k +
							XSECURE_VALUE_ONE))));

		/** - Step 10: if floor(idx/2^k) is odd then */
		if (XSECURE_IS_ODD((u32)((IdxLeaf >> k)))) {
			/** - Step 14: ADRS.setTreeIndex((ADRS.getTreeIndex() - 1)/2) (implicit) */
			/** - Step 15: node[1] <- H(PK.seed, ADRS, AUTH[k] || node[0]) */
			XSECURE_TEMPORAL_CHECK(END,
				Status,
				XSecure_SlhdsaShake256sHashH,
				PublicKeyAddr,				/* [in] Public key seed address */
				SignXmssOffset,				/* [in] Authentication path element address */
				(u64)(UINTPTR)Node,			/* [in] Current node value address */
				Node);					/* [out] Updated node value */
		} else {
			/** - Step 11: ADRS.setTreeIndex(ADRS.getTreeIndex()/2) (implicit in logic) */
			/** - Step 12: node[1] <- H(PK.seed, ADRS, node[0] || AUTH[k]) */
			XSECURE_TEMPORAL_CHECK(END,
				Status,
				XSecure_SlhdsaShake256sHashH,
				PublicKeyAddr,				/* [in] Public key seed address */
				(u64)(UINTPTR)Node,			/* [in] Current node value address */
				SignXmssOffset,				/* [in] Authentication path element address */
				Node);					/* [out] Updated node value */
		}
		/** - Step 17: node[0] <- node[1] (Node already updated in-place) */
		SignXmssOffset += (u64)InstancePtr->Param->n;
	}
	/** - Step 18: end for */

	/** - Step 19: return node[0] */
	if (k != (u32)InstancePtr->Param->hprime) {
		Status = XSECURE_ERR_GLITCH_DETECTED;
	} else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
/** @} */
