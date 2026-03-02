/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsa_ht.c
*
* This file consists of definitions for SLH-DSA hypertree operations
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
#include "xsecure_slhdsa_xmss.h"
#include "xsecure_slhdsa_ht.h"
#include "xsecure_utils.h"
#include "xsecure_defs.h"

/************************************ Constant Definitions ****************************************/

/**************************************** Function Prototypes *************************************/

/************************************* Variable Definitions ***************************************/

/************************************* Function Definitions ***************************************/

/**************************************************************************************************/
/**
 * @brief	This function verifies a SLH-DSA hypertree signature by traversing through multiple
 *		layers of XMSS trees, starting from the bottom layer and working up to the root
 *		layer, ultimately comparing the computed root with PK.root.
 *
 * @par Algorithm Flow for SHAKE-256s (d=8, h'=8, len=67, n=32):
 *
 * Given: IdxTree=0x123456789ABCDEF0, IdxLeaf=0x87654321
 *
 * === HYPERTREE STRUCTURE ===
 * SLH-DSA-SHAKE-256s uses a hypertree composed of d=8 layers of XMSS trees:
 *
 * Layer 7 (Top):    1 XMSS tree                    (contains PK.root)
 * Layer 6:          2^8 = 256 XMSS trees
 * Layer 5:          2^16 = 65,536 XMSS trees
 * Layer 4:          2^24 = 16,777,216 XMSS trees
 * Layer 3:          2^32 = 4,294,967,296 XMSS trees
 * Layer 2:          2^40 = 1,099,511,627,776 XMSS trees
 * Layer 1:          2^48 = 281,474,976,710,656 XMSS trees
 * Layer 0 (Bottom): 2^56 = 72,057,594,037,927,936 XMSS trees (FORS trees connect here)
 *
 * Each XMSS tree has height h'=8 (2^8 = 256 leaves)
 *
 * === TREE COUNT CALCULATION ===
 * At layer j, maximum tree index = 2^(56 - j×8) - 1
 * - Layer 0: Max tree index = 2^56 - 1 (so 2^56 possible trees)
 * - Layer 1: Max tree index = 2^48 - 1 (so 2^48 possible trees)
 * - Layer 2: Max tree index = 2^40 - 1 (so 2^40 possible trees)
 * - Layer 3: Max tree index = 2^32 - 1 (so 2^32 possible trees)
 * - Layer 4: Max tree index = 2^24 - 1 (so 2^24 possible trees)
 * - Layer 5: Max tree index = 2^16 - 1 (so 2^16 possible trees)
 * - Layer 6: Max tree index = 2^8 - 1  (so 2^8 possible trees)
 * - Layer 7: Max tree index = 2^0 - 1 = 0 (so 1 possible tree)
 *
 * === TREE DIVERSITY DEMONSTRATION ===
 * Starting with IdxTree = 0x123456789ABCDEF0:
 *
 * Layer 0→1: IdxLeaf = 0x00, IdxTree = 0x123456789ABCDE00 >> 8 = 0x123456789ABCDE
 * Layer 1→2: IdxLeaf = 0xDE, IdxTree = 0x123456789ABCDE >> 8 = 0x123456789ABC
 * Layer 2→3: IdxLeaf = 0xBC, IdxTree = 0x123456789ABC >> 8 = 0x123456789A
 * Layer 3→4: IdxLeaf = 0x9A, IdxTree = 0x123456789A >> 8 = 0x12345678
 * Layer 4→5: IdxLeaf = 0x78, IdxTree = 0x12345678 >> 8 = 0x123456
 * Layer 5→6: IdxLeaf = 0x56, IdxTree = 0x123456 >> 8 = 0x1234
 * Layer 6→7: IdxLeaf = 0x34, IdxTree = 0x1234 >> 8 = 0x12
 * Layer 7→END: IdxLeaf = 0x12, IdxTree = 0x12 >> 8 = 0x0 ✓
 *
 * === TRAVERSAL TABLE ===
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | j     | Layer    | IdxLeaf     | IdxTree           | XMSS Tree Accessed    | Description     |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | -     | 0 (init) | 0x21        | 0x123456789ABCDEF0| Tree F0 at Layer 0    | Initial tree    |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | 1     | 1        | 0xF0        | 0x123456789ABCDE  | Tree DE at Layer 1    | Parent of F0    |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | 2     | 2        | 0xDE        | 0x123456789ABC    | Tree BC at Layer 2    | Parent of DE    |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | 3     | 3        | 0xBC        | 0x123456789A      | Tree 9A at Layer 3    | Parent of BC    |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | 4     | 4        | 0x9A        | 0x12345678        | Tree 78 at Layer 4    | Parent of 9A    |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | 5     | 5        | 0x78        | 0x123456          | Tree 56 at Layer 5    | Parent of 78    |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | 6     | 6        | 0x56        | 0x1234            | Tree 34 at Layer 6    | Parent of 56    |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 * | 7     | 7        | 0x34        | 0x12              | Tree 12 at Layer 7    | Root layer      |
 * |       |          |             |                   | (leaf 0x34 in tree    | (final check    |
 * |       |          |             |                   |  0x12)                 | vs PK.root)    |
 * +-------+----------+-------------+-------------------+------------------------+----------------+
 *
 * === CALCULATION FORMULAS ===
 * Step 6: IdxLeaf = IdxTree & ((1 << h') - 1) = IdxTree & 0xFF
 * Step 7: IdxTree = IdxTree >> h' = IdxTree >> 8
 *
 * === DETAILED INDEX PROGRESSION ===
 * Starting with IdxTree = 0x123456789ABCDEF0:
 *
 * Layer 0→1: IdxLeaf = 0xF0 & 0xFF = 0xF0 (240), IdxTree = 0x123456789ABCDEF0 >> 8 = 0x123456789ABCDE
 * Layer 1→2: IdxLeaf = 0xDE & 0xFF = 0xDE (222), IdxTree = 0x123456789ABCDE >> 8 = 0x123456789ABC
 * Layer 2→3: IdxLeaf = 0xBC & 0xFF = 0xBC (188), IdxTree = 0x123456789ABC >> 8 = 0x123456789A
 * Layer 3→4: IdxLeaf = 0x9A & 0xFF = 0x9A (154), IdxTree = 0x123456789A >> 8 = 0x12345678
 * Layer 4→5: IdxLeaf = 0x78 & 0xFF = 0x78 (120), IdxTree = 0x12345678 >> 8 = 0x123456
 * Layer 5→6: IdxLeaf = 0x56 & 0xFF = 0x56 (86),  IdxTree = 0x123456 >> 8 = 0x1234
 * Layer 6→7: IdxLeaf = 0x34 & 0xFF = 0x34 (52),  IdxTree = 0x1234 >> 8 = 0x12
 * Layer 7→8: IdxLeaf = 0x12 & 0xFF = 0x12 (18),  IdxTree = 0x12 >> 8 = 0x0
 *
 * === TREE DIVERSITY DEMONSTRATION ===
 * This example shows access to different trees at each layer:
 * - Layer 0: Tree 0xF0 (240) out of 2^56 possible trees
 * - Layer 1: Tree 0xDE (222) out of 2^48 possible trees
 * - Layer 2: Tree 0xBC (188) out of 2^40 possible trees
 * - Layer 3: Tree 0x9A (154) out of 2^32 possible trees
 * - Layer 4: Tree 0x78 (120) out of 2^24 possible trees
 * - Layer 5: Tree 0x56 (86)  out of 2^16 possible trees
 * - Layer 6: Tree 0x34 (52)  out of 2^8 possible trees
 * - Layer 7: Tree 0x12 (18)  out of 1 possible tree (but shows non-zero until final shift)
 * - Layer 8: Tree 0x0  (0)   - The single root tree
 *
 * === SIGNATURE CONSUMPTION ===
 * SignHt progression through signature buffer:
 * +-------+------------------------+-------------------------+
 * | j     | SignHt Offset (bytes)  | XMSS Signature Used     |
 * +-------+------------------------+-------------------------+
 * | -     | 0                      | SIG_HT[0:2399]          |
 * | 1     | 2400                   | SIG_HT[2400:4799]       |
 * | 2     | 4800                   | SIG_HT[4800:7199]       |
 * | 3     | 7200                   | SIG_HT[7200:9599]       |
 * | 4     | 9600                   | SIG_HT[9600:11999]      |
 * | 5     | 12000                  | SIG_HT[12000:14399]     |
 * | 6     | 14400                  | SIG_HT[14400:16799]     |
 * | 7     | 16800                  | SIG_HT[16800:19199]     |
 * +-------+------------------------+-------------------------+
 *
 *
 * === XMSS VERIFICATION DETAILS ===
 * At each layer j:
 * 1. ADRS.setLayerAddress(j) - Identifies current layer
 * 2. ADRS.setTreeAddress(IdxTree) - Identifies specific XMSS tree
 * 3. xmss_pkFromSig() computes XMSS root using:
 *    - IdxLeaf: Position within the XMSS tree
 *    - SIGtmp: XMSS signature for this layer
 *    - M: Message (FORS_PK for layer 0, previous root for others)
 *    - PK.seed: Public seed
 *    - ADRS: Address structure for domain separation
 *
 * === FINAL VERIFICATION ===
 * Step 13: Compare computed root with PK.root
 * - Node contains final computed root (32 bytes)
 * - PK.root = PublicKey[32:63] (second 32 bytes of public key)
 * - Byte-by-byte comparison for constant-time verification
 *
 * === PERFORMANCE CHARACTERISTICS ===
 * Computational Complexity:
 * - XMSS verifications: d = 8 operations
 * - Total hash operations: d × (len + h') = 8 × 75 = 600 hash calls
 * - Memory usage: 2400 bytes working + 19200 bytes signature input
 * - Time complexity: O(d × (len + h') × n)
 *
 * @param	M		Message to verify (FORS public key, 32 bytes).
 * @param	SignHtAddr	64-bit address of hypertree signature (19,200 bytes for SHAKE-256s).
 * @param	PublicKeyAddr	64-bit address of public key buffer (PK.seed || PK.root, 64 bytes).
 * @param	HtIndices	Pointer to structure containing hypertree indices:
 *				- IdxTree: Tree index for hypertree verification
 *				- IdxLeaf: Leaf index for hypertree verification
 *
 * @return
 *		- XST_SUCCESS if signature verification succeeds.
 *		- XSECURE_SLHDSA_PK_ROOT_MISMATCH_ERROR if final root comparison fails.
 *		- ErrorCode Otherwise.
 *
 **************************************************************************************************/
int XSecure_SlhdsaHtVerify(const u8* const M,
                           const u64 SignHtAddr, const u64 PublicKeyAddr,
                           const XSecure_SlhdsaHtIndices * const HtIndices)
{
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();
	volatile int Status = XST_FAILURE;
	u8* Node = InstancePtr->Data3;
	volatile u32 LevelIdx;
	volatile u32 Idx;
	u32 Offset;
	u64 IdxTree = HtIndices->IdxTree;
	u64 IdxLeaf = HtIndices->IdxLeaf;
	u64 SignHtPtrAddr = SignHtAddr;

	/* Step 1: ADRS <- toByte(0, 32) */
	XSecure_SlhdsaClearAddress(InstancePtr->Addr);

	/* Step 2: ADRS.setTreeAddress(idxtree) */
	XSecure_SlhdsaSetTreeAddress(InstancePtr->Addr, IdxTree);

	/* Step 3: SIGtmp <- SIG_HT.getXMSSSignature(0) */
	Offset = (InstancePtr->Param->ChecksumParams.InputLenInDigits +
		  InstancePtr->Param->ChecksumParams.ChecksumLenInDigits +
		  (u32)InstancePtr->Param->hprime) * (u32)InstancePtr->Param->n;

	/* Step 4: node <- xmss_pkFromSig(idxleaf, SIGtmp, M, PK.seed, ADRS) */
	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_SlhdsaXmssPkFromSign,
		IdxLeaf,					/* [in] Leaf index within XMSS tree */
		SignHtPtrAddr,					/* [in] XMSS signature data address */
		(u64)(UINTPTR)M,				/* [in] Message address */
		PublicKeyAddr,					/* [in] Public key seed address */
		Node);						/* [out] Computed XMSS public key */

	/* Step 5: for LevelIdx from 1 to d-1 do */
	for (LevelIdx = 1U; LevelIdx < (u32)InstancePtr->Param->d; LevelIdx++) {
		/* Step 6: idxleaf <- idxtree mod 2^h', h' least significant bits from idxtree */
		IdxLeaf = IdxTree & (u64)((XSECURE_VALUE_ONE << (u32)InstancePtr->Param->hprime) -
				XSECURE_VALUE_ONE);

		/* Step 7: idxtree <- idxtree >> h', removing least significant h' bits from idxtree */
		IdxTree >>= (u32)InstancePtr->Param->hprime;

		/* Step 8: ADRS.setLayerAddress(LevelIdx) */
		XSecure_SlhdsaSetLayerAddress(InstancePtr->Addr, LevelIdx);

		/* Step 9: ADRS.setTreeAddress(idxtree) */
		XSecure_SlhdsaSetTreeAddress(InstancePtr->Addr, IdxTree);

		/* Step 10: SIGtmp <- SIG_HT.getXMSSSignature(LevelIdx) */
		SignHtPtrAddr += Offset;

		/* Step 11: node <- xmss_pkFromSig(idxleaf, SIGtmp, node, PK.seed, ADRS) */
		XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_SlhdsaXmssPkFromSign,
			IdxLeaf,					/* [in] Leaf index within XMSS tree */
			SignHtPtrAddr,					/* [in] XMSS signature data address */
			(u64)(UINTPTR)Node,				/* [in] Message/node address from previous layer */
			PublicKeyAddr,					/* [in] Public key seed address */
			Node);						/* [out] Computed XMSS public key */
	}

	if (LevelIdx != (u32)InstancePtr->Param->d) {
		Status = XSECURE_ERR_GLITCH_DETECTED;
		goto END;
	}

	/* Step 12: end for */

	/* Step 13: if node == PK.root then */
	Status = XSECURE_SLHDSA_PK_ROOT_MISMATCH_ERROR;
	for (Idx = 0U; Idx < (u32)InstancePtr->Param->n; Idx++) {
		if (Node[Idx] != XSecure_InByte64(PublicKeyAddr + (u64)InstancePtr->Param->n +
						  Idx)) {
			XSecure_Printf(XSECURE_DEBUG_GENERAL, "XSecure_SlhdsaHtVerify: Expected "
				       "root[%d] %x, Computed node[%d] %x\n\r", Idx,
				       XSecure_InByte64(PublicKeyAddr + (u64)InstancePtr->Param->n
							+ Idx), Idx, Node[Idx]);
			/* Step 15: else */
			Status = XSECURE_SLHDSA_PK_ROOT_MISMATCH_ERROR;
			goto END;
		}
	}

	if (Idx != (u32)InstancePtr->Param->n) {
		Status = XSECURE_ERR_GLITCH_DETECTED;
	} else {
		/* Step 14: return true */
		Status = XST_SUCCESS;
	}

END:
	/* Step 16/17: return false/end if */
	return Status;
}
/** @} */
