/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/slhdsa/xsecure_slhdsa_wots.c
*
* This file consists of definitions for SLH-DSA WOTS+ operations
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
#include "xsecure_slhdsa_wots.h"
#include "xsecure_slhdsa_fors.h"

/************************************ Constant Definitions ****************************************/

/**************************************** Function Prototypes *************************************/
static void XSecure_ComputeChecksum(const XSecure_ChecksumParam * const ChecksumParams,
				    const u8 * const M, u32 * const Output);

/************************************* Variable Definitions ***************************************/

/************************************* Function Definitions ***************************************/

/**************************************************************************************************/
/**
 * @brief	This function computes WOTS+ checksum and combines with base-w message
 *		representation to create the complete message array used for WOTS+ signature
 *		verification.
 *
 * @param	ChecksumParams	Pointer to checksum parameters structure containing:
 * @param	M		Input message (n bytes, typically 32 bytes for SHAKE-256s)
 * @param	Output		Output array to store combined message and checksum digits (len1 +
 *				len2 elements, typically 67 for SHAKE-256s)
 *
 **************************************************************************************************/
static void XSecure_ComputeChecksum(const XSecure_ChecksumParam * const ChecksumParams,
				    const u8 * const M, u32 * const Output)
{
	u32 Csum = XSECURE_ZERO;  /* Step 1: csum <- 0 */
	volatile u32 Idx;
	/** - Buffer to hold checksum as bytes (max 4 bytes needed) */
	u8 ChecksumBytes[XSECURE_SLHDSA_CSUM_SIZE_IN_BYTES * XSECURE_VALUE_TWO] = {0U};

	/** - Step 2: msg <- base_2b(M, lgw, len1) - Convert message to base-w */
	XSecure_SlhdsaBase2b(Output, M, ChecksumParams->DigitWidth, ChecksumParams->InputLenInDigits);

	/** - Step 3-5: Compute checksum */
	/** - for i from 0 to len1 - 1 do */
	for (Idx = 0U; Idx < ChecksumParams->InputLenInDigits; Idx++) {
		/** - Step 4: csum <- csum + w - 1 - msg[i] */
		/** - w = 2^lgw, so w-1 = (1 << lgw) - 1 */
		Csum += ((u32)((XSECURE_VALUE_ONE << ChecksumParams->DigitWidth) -
					XSECURE_VALUE_ONE) - Output[Idx]);
	}
	/** - Step 5: end for */

	/** - Step 6: csum <- csum << ((8 - ((len2 * lgw) mod 8)) mod 8) */
	/** - For lgw=4, this shifts left by 4 bits */
	Csum = (Csum << ChecksumParams->ChecksumShift);

	/** - Convert checksum to bytes for base_2b conversion */
	/** - toByte(csum, ceil(len2*lgw/8)) */
	for (Idx = 0U; Idx < ChecksumParams->ChecksumSizeInBytes; Idx++) {
		/** - Store checksum in big-endian format */
		ChecksumBytes[ChecksumParams->ChecksumSizeInBytes - XSECURE_VALUE_ONE - Idx] =
			(u8)((Csum >> (Idx * XSECURE_BYTE_IN_BITS)) & 0xFFU);
	}

	/** - Step 7: msg <- msg || base_2b(toByte(csum, ceil(len2*lgw/8)), lgw, len2) */
	/** - Convert checksum to base-w and append to message array */
	XSecure_SlhdsaBase2b(&Output[ChecksumParams->InputLenInDigits], ChecksumBytes,
			     ChecksumParams->DigitWidth, ChecksumParams->ChecksumLenInDigits);
}

/**************************************************************************************************/
/**
 * @brief	This function reconstructs the WOTS+ public key from the provided WOTS+ signature
 *		and message. It computes the checksum, extends each signature chain to derive the
 *		public key elements, and then hashes all elements to produce the final WOTS+ public
 *		key.
 *
 * @par Algorithm Flow for SHAKE-256s (len=67, w=16, n=32):
 *
 * Given: M=FORS_PK (32 bytes), WotsSign=WOTS_signature (2144 bytes)
 * Input message: M = [0x3A, 0x7F, 0x92, 0x1C, ...] (32 bytes)
 *
 * === WOTS+ SIGNATURE STRUCTURE ===
 * WotsSign layout (2144 bytes total):
 * ┌──────────────┬──────────────┬─────┬──────────────┬──────────────┬─────┬──────────────┐
 * │ sig[0]       │ sig[1]       │ ... │ sig[63]      │ sig[64]      │ ... │ sig[66]      │
 * │ (32 bytes)   │ (32 bytes)   │     │ (32 bytes)   │ (32 bytes)   │     │ (32 bytes)   │
 * │ Chain 0      │ Chain 1      │     │ Chain 63     │ Checksum 0   │     │ Checksum 2   │
 * └──────────────┴──────────────┴─────┴──────────────┴──────────────┴─────┴──────────────┘
 *
 * Message chains: sig[0] to sig[63] (64 chains for 32-byte message)
 * Checksum chains: sig[64] to sig[66] (3 chains for checksum)
 * Total: 67 chains × 32 bytes = 2144 bytes
 *
 * === INITIAL SETUP ===
 * ADRS initialization:
 *   - Type already set to WOTS_HASH for chain computations
 *   - KeyPairAddress inherited from caller
 *
 * === STEP 1-7: MESSAGE PROCESSING AND CHECKSUM ===
 * XSecure_SlhdsaWotsCheckSum(M, Data2[]) computes:
 *
 * Step 1: csum = 0
 * Step 2: msg = base_w(M, w, len1) - Convert message to base-16 digits
 *
 * Message conversion (first 4 bytes example):
 * M[0] = 0x3A = 0011 1010 → base_w[0] = 3, base_w[1] = 10
 * M[1] = 0x7F = 0111 1111 → base_w[2] = 7, base_w[3] = 15
 * M[2] = 0x92 = 1001 0010 → base_w[4] = 9, base_w[5] = 2
 * M[3] = 0x1C = 0001 1100 → base_w[6] = 1, base_w[7] = 12
 * ...continuing for all 32 bytes → 64 base-16 digits
 *
 * Step 3-5: Compute checksum
 * for i from 0 to len1-1: csum += w-1-msg[i]
 * csum += (15-3) + (15-10) + (15-7) + (15-15) + (15-9) + (15-2) + ... = sum
 *
 * Step 6-7: Convert checksum to base-16 and append
 * csum_base_w = base_w(csum, w, len2) → 3 additional base-16 digits
 * Final msg array: [msg[0]...msg[63], csum[0], csum[1], csum[2]]
 *
 * === WOTS+ CHAIN COMPUTATION TABLE (WITH HW ACCELERATION) ===
 * +-----+----------+----------+-------------+------------------------+------------------+
 * | i   | msg[i]   | Chain    | Iterations  | HW Chain Operation     | Result Location  |
 * +-----+----------+----------+-------------+------------------------+------------------+
 * | 0   | 3        | 0        | 15-3 = 12   | HW: sig[0] →(×12)→     | Data1[0:31]      |
 * | 1   | 10       | 1        | 15-10 = 5   | HW: sig[1] →(×5)→      | Data1[32:63]     |
 * | 2   | 7        | 2        | 15-7 = 8    | HW: sig[2] →(×8)→      | Data1[64:95]     |
 * | 3   | 15       | 3        | 15-15 = 0   | SKIP: sig[3] copied    | Data1[96:127]    |
 * | 4   | 9        | 4        | 15-9 = 6    | HW: sig[4] →(×6)→      | Data1[128:159]   |
 * | ... | ...      | ...      | ...         | ...                    | ...              |
 * | 63  | msg[63]  | 63       | 15-msg[63]  | HW: sig[63] →(×n)→     | Data1[2016:2047] |
 * | 64  | csum[0]  | 64       | 15-csum[0]  | HW: sig[64] →(×n)→     | Data1[2048:2079] |
 * | 65  | csum[1]  | 65       | 15-csum[1]  | HW: sig[65] →(×n)→     | Data1[2080:2111] |
 * | 66  | csum[2]  | 66       | 15-csum[2]  | HW: sig[66] →(×n)→     | Data1[2112:2143] |
 * +-----+----------+----------+-------------+------------------------+------------------+
 *
 * === ADDRESS PROGRESSION ===
 * For each chain i:
 * +-----+------------------+-----------------------------+
 * | i   | ChainAddress(i)  | HW Chain Operation          |
 * +-----+------------------+-----------------------------+
 * | 0   |        0         | HW: chain=0, iter=3→15      |
 * | 1   |        1         | HW: chain=1, iter=10→15     |
 * | 2   |        2         | HW: chain=2, iter=7→15      |
 * | ... |       ...        | ...                         |
 * | 66  |       66         | HW: chain=66, iter=x→15     |
 * +-----+------------------+-----------------------------+
 *
 * === FINAL PUBLIC KEY COMPUTATION ===
 * Step 12-13: Switch ADRS to WOTS_PK type
 * XSecure_SlhdsaSetTypeClearNotKp(WOTS_PK)
 *
 * Step 15: Compute final WOTS+ public key
 * WotsPk = Tlen(PK.seed, ADRS, Data1[0:2143])
 *
 * XSecure_SlhdsaShake256sHashTl computes:
 * - Input: All 67 public key elements (Data1, 2144 bytes total)
 * - Hash: SHAKE-256(PK.seed || ADRS || Data1[0:2143])
 * - Output: 32-byte WOTS+ public key (WotsPk)
 *
 * === MEMORY MANAGEMENT ===
 * Buffer Usage:
 * - Data1[2144]: Stores all 67 public key elements (67 × 32 bytes)
 * - Data2[67]: Stores base-w representation + checksum
 * - WotsPk[32]: Output buffer for final public key
 * - TmpPtr: Tracks current position in signature buffer (increments by 32)
 *
 * Memory Layout during execution:
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ Data1: [pk_elem[0]][pk_elem[1]]...[pk_elem[66]] (2144 bytes)            │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Data2: [msg[0]][msg[1]]...[msg[63]][csum[0]][csum[1]][csum[2]] (67 vals)│
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * Memory Usage:
 * - Input signature: 2144 bytes
 * - Working buffers: 2144 + 67 = 2211 bytes
 * - Output: 32 bytes
 * - Total working memory: ~2.2 KB
 *
 * @param	WotsSignAddr	64-bit address of WOTS+ signature (2144 bytes for SHAKE-256s).
 * @param	MAddr		64-bit address of message to verify (typically FORS public key, n bytes = 32 bytes).
 * @param	PublicKeyAddr	64-bit address of public key seed for hash computations (n bytes = 32 bytes).
 * @param	WotsPk		Buffer to store computed WOTS+ public key (n bytes = 32 bytes).
 *
 * @return
 *		- XST_SUCCESS if WOTS+ public key computation succeeds.
 *		- ErrorCode Otherwise.
 *
 **************************************************************************************************/
int XSecure_SlhdsaWotsPkFromSign(u64 WotsSignAddr, u64 MAddr,
                                 u64 PublicKeyAddr, u8 * const WotsPk)
{
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();
	volatile int Status = XST_FAILURE;
	u32 TmpPtr = XSECURE_ZERO;
	u32 Idx;
	u32 TotalLength;
	XSecure_SlhdsaChainConfig ChainConfig;

	/** - Step 1: csum <- 0 (handled inside WotsCheckSum) */
	/** - Step 2: msg <- base_2b(M, lgw, len1) (handled inside WotsCheckSum) */
	/** - Step 3-7: Compute checksum and append to msg (handled inside WotsCheckSum) */
	XSecure_ComputeChecksum(&InstancePtr->Param->ChecksumParams, (const u8 *)(UINTPTR)MAddr,
			InstancePtr->Data4);

	/** - Step 8: for i from 0 to len-1 do */
	/** - Use embedded parameter for total length instead of direct Param->len */
	TotalLength = InstancePtr->Param->ChecksumParams.InputLenInDigits +
			InstancePtr->Param->ChecksumParams.ChecksumLenInDigits;
	for (Idx = 0U; Idx < TotalLength; Idx++) {
		/** - Step 9: ADRS.setChainAddress(i) */
		XSecure_SlhdsaSetChainAddress(InstancePtr->Addr, Idx);

		/*
		 * Step 10: tmp[i] <- chain(sig[i], msg[i], w-1-msg[i], PK.seed, ADRS)
		 * chain function computes the hash chain for each signature element
		 */
		ChainConfig.StartIdx = InstancePtr->Data4[Idx];
		ChainConfig.Steps = InstancePtr->Param->NoOfInvSign - InstancePtr->Data4[Idx];

		XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_SlhdsaShake256sChain,
			WotsSignAddr + TmpPtr,			/* [in] Input Data address for HASH function */
			&ChainConfig,				/* [in] Chain configuration */
			PublicKeyAddr,				/* [in] Public key address */
			(InstancePtr->Data1 + TmpPtr));		/* [out] pointer to output buffer */

		TmpPtr += (u32)InstancePtr->Param->n;
	}
	/** - Step 11: end for */

	/** - Step 12: wotspkADRS <- ADRS (current ADRS used) */
	/** - Step 13: wotspkADRS.setTypeAndClear(WOTS_PK) */
	XSecure_SlhdsaSetTypeClearNotKp(InstancePtr->Addr, XSECURE_SLH_DSA_ADRS_TYPE_WOTS_PK);

	/*
	 * Step 14: wotspkADRS.setKeyPairAddress(ADRS.getKeyPairAddress()) (implicit, not needed as
	 * keypair is unchanged)
	 */

	/*
	 * Step 15: pksig <- Tlen(PK.seed, wotspkADRS, tmp)
	 * Tlen is a hash function mapping all tmp[] to the final WOTS+ public key
	 */
	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_SlhdsaShake256sHashTl,
		PublicKeyAddr,				/* [in] Public key seed address */
		InstancePtr->Data1,			/* [in] Input data buffer (tmp array) */
		TmpPtr,					/* [in] Size of input data */
		WotsPk);				/* [out] WOTS+ public key output buffer */

	/** - Step 16: return pksig */

END:
	return Status;
}
/** @} */
