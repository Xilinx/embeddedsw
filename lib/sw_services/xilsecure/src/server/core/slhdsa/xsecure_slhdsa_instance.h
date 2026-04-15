/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/slhdsa/xsecure_slhdsa_instance.h
*
* This file contains structures, constants and declarations used in SLH-DSA instance management and
* provides interface to SLH-DSA verification operations
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
#ifndef XSECURE_SLH_DSA_INSTANCE_H_
#define XSECURE_SLH_DSA_INSTANCE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xsecure_slhdsa_addr.h"
#include "xil_types.h"
#include "xsecure_sha.h"

/***************************************** Constant Definitions ***********************************/
#define XSECURE_SLHDSA_N_LEN			32U	/**< Output length in bytes for
							  SLH-DSA-SHAKE-256s */
#define XSECURE_SLHDSA_SHAKE_256S_SIGN_LEN	29792U	/**< Total signature length in bytes for
							  SLH-DSA-SHAKE-256s */
#define XSECURE_SLHDSA_MAX_DATA1_LEN_IN_BYTES	(2144U)	/**< Maximum length of Data1 buffer in
							  bytes (for WOTS+ chain values) */
#define XSECURE_SLHDSA_MAX_DATA2_LEN_IN_BYTES	(99U)	/**< Maximum length of Data2 buffer in
							  bytes (for hash outputs and base
							  conversions) */
#define XSECURE_SLHDSA_MAX_DATA3_LEN_IN_BYTES	(32U)	/**< Maximum length of Data3 buffer in
							  bytes (for single hash outputs) */
#define XSECURE_SLHDSA_MAX_DATA4_LEN_IN_WORDS	(67U)	/**< Maximum length of Data4 buffer in
							  words (for storing digits) */
#define XSECURE_SLHDSA_NO_OF_INV_SIGN		(15U)	/**< Number of invalid signature
							  invocations allowed */
#define XSECURE_SLHDSA_CSUM_INPUT_LEN_IN_DIGITS	(64U)	/**< Length of checksum input in
							  digits for WOTS+ */
#define XSECURE_SLHDSA_CSUM_LEN_IN_DIGITS	(3U)	/**< Length of checksum in digits */
#define XSECURE_SLHDSA_CSUM_SHIFT		(4U)	/**< Shift value for checksum
							  calculation */
#define XSECURE_SLHDSA_CSUM_SIZE_IN_BYTES	(2U)	/**< Size of checksum in bytes */

/******************************************* Type Definitions *************************************/
/**
 * @enum XSecure_Slhdsa_n
 * @brief Output length in bytes (n).
 */
typedef enum {
	XSECURE_SLH_DSA_N_16 = 16U,			/**< 16 bytes */
	XSECURE_SLH_DSA_N_24 = 24U,			/**< 24 bytes */
	XSECURE_SLH_DSA_N_32 = XSECURE_SLHDSA_N_LEN,	/**< 32 bytes */
	XSECURE_SLH_DSA_N_UNSUPPORTED = 0U
} XSecure_Slhdsa_n;

/**
 * @enum XSecure_Slhdsa_h
 * @brief Merkle tree height (h).
 */
typedef enum {
	XSECURE_SLH_DSA_H_63 = 63U,		/**< 63 */
	XSECURE_SLH_DSA_H_64 = 64U,		/**< 64 */
	XSECURE_SLH_DSA_H_66 = 66U,		/**< 66 */
	XSECURE_SLH_DSA_H_68 = 68U,		/**< 68 */
	XSECURE_SLH_DSA_H_UNSUPPORTED = 0U
} XSecure_Slhdsa_h;

/**
 * @enum XSecure_Slhdsa_d
 * @brief Number of subtrees (d).
 */
typedef enum {
	XSECURE_SLH_DSA_D_7 = 7U,		/**< 7 */
	XSECURE_SLH_DSA_D_8 = 8U,		/**< 8 */
	XSECURE_SLH_DSA_D_17 = 17U,		/**< 17 */
	XSECURE_SLH_DSA_D_22 = 22U,		/**< 22 */
	XSECURE_SLH_DSA_D_UNSUPPORTED = 0U
} XSecure_Slhdsa_d;

/**
 * @enum XSecure_Slhdsa_hprime
 * @brief Subtree height (h').
 */
typedef enum {
	XSECURE_SLH_DSA_HPRIME_3 = 3U,		/**< 3 */
	XSECURE_SLH_DSA_HPRIME_4 = 4U,		/**< 4 */
	XSECURE_SLH_DSA_HPRIME_8 = 8U,		/**< 8 */
	XSECURE_SLH_DSA_HPRIME_9 = 9U,		/**< 9 */
	XSECURE_SLH_DSA_HPRIME_UNSUPPORTED = 0U
} XSecure_Slhdsa_hprime;

/**
 * @enum XSecure_Slhdsa_a
 * @brief Winternitz parameter (a).
 */
typedef enum {
	XSECURE_SLH_DSA_A_6 = 6U,		/**< 6 */
	XSECURE_SLH_DSA_A_8 = 8U,		/**< 8 */
	XSECURE_SLH_DSA_A_9 = 9U,		/**< 9 */
	XSECURE_SLH_DSA_A_12 = 12U,		/**< 12 */
	XSECURE_SLH_DSA_A_14 = 14U,		/**< 14 */
	XSECURE_SLH_DSA_A_UNSUPPORTED = 0U
} XSecure_Slhdsa_a;

/**
 * @enum XSecure_Slhdsa_k
 * @brief Number of layers (k).
 */
typedef enum {
	XSECURE_SLH_DSA_K_14 = 14U,		/**< 14 */
	XSECURE_SLH_DSA_K_17 = 17U,		/**< 17 */
	XSECURE_SLH_DSA_K_22 = 22U,		/**< 22 */
	XSECURE_SLH_DSA_K_33 = 33U,		/**< 33 */
	XSECURE_SLH_DSA_K_35 = 35U,		/**< 35 */
	XSECURE_SLH_DSA_K_UNSUPPORTED = 0U
} XSecure_Slhdsa_k;

/**
 * @enum XSecure_Slhdsa_lgw
 * @brief log2(Winternitz parameter) (lgw).
 */
typedef enum {
	XSECURE_SLH_DSA_LGW_4 = 4U,		/**< 4 */
	XSECURE_SLH_DSA_LGW_UNSUPPORTED = 0U
} XSecure_Slhdsa_lgw;

/**
 * @enum XSecure_Slhdsa_m
 * @brief Message digest length (m).
 */
typedef enum {
	XSECURE_SLH_DSA_M_30 = 30U,		/**< 30 */
	XSECURE_SLH_DSA_M_34 = 34U,		/**< 34 */
	XSECURE_SLH_DSA_M_39 = 39U,		/**< 39 */
	XSECURE_SLH_DSA_M_42 = 42U,		/**< 42 */
	XSECURE_SLH_DSA_M_47 = 47U,		/**< 47 */
	XSECURE_SLH_DSA_M_49 = 49U,		/**< 49 */
	XSECURE_SLH_DSA_M_UNSUPPORTED = 0U
} XSecure_Slhdsa_m;

/**
 * @brief Structure to maintain checksum parameters for WOTS+ operations
 */
typedef struct {
	u32 DigitWidth;			/**< Width of each digit in bits */
	u32 InputLenInDigits;		/**< Length of input in digits */
	u32 ChecksumLenInDigits;	/**< Length of checksum in digits */
	u32 ChecksumShift;		/**< Shift value for checksum calculation */
	u32 ChecksumSizeInBytes;	/**< Size of checksum in bytes */
} XSecure_ChecksumParam;

/**
 * This structure encapsulates all the parameters required to configure an instance of the SLH_DSA
 * algorithm.
 */
typedef struct {
	XSecure_Slhdsa_n n;             /**< ALL: Output length parameter (n): core hash output and
					  secret string size (in bytes). Used by FORS, XMSS, WOTS+,
					  and HT. Typical: 16, 24, 32. */
	XSecure_Slhdsa_h h;             /**< HT: Merkle tree height (h): total height of the
					  hypertree combining all XMSS layers. */
	XSecure_Slhdsa_d d;             /**< HT: Number of XMSS layers (d): depth of hypertree,
					  determines how many XMSS trees are stacked. */
	XSecure_Slhdsa_hprime hprime;   /**< XMSS: Subtree height (h'): height of each individual
					  XMSS subtree, h' = h/d. */
	XSecure_Slhdsa_a a;             /**< FORS: Height of each FORS tree (a): number of bits to
					  select a leaf; t = 2^a leaves per tree. */
	XSecure_Slhdsa_k k;             /**< FORS: Number of FORS trees (k): determines number of
					  signature components from FORS layer. */
	XSecure_Slhdsa_lgw lgw;         /**< WOTS+: log2(Winternitz parameter) (lgw): lgw = log2(w),
					  used for WOTS+ length calculations in XMSS layers. */
	XSecure_Slhdsa_m m;             /**< FORS: Message digest length (m): length of hash of
					  message to be signed (in bits), typically >= k * a for
					  FORS tree selection. */
	u32 NoOfInvSign;                /**< WOTS+: w - 1 = (2 ^ lgw) - 1, used in WOTS+ signature
					  generation within XMSS trees. */
	u32 SignLen;                    /**< ALL: Total signature size in bytes (SignLen):
					  (1 + k(1 + a) + h + d * len) * n. Includes FORS signature,
					  WOTS+ signatures, and HT authentication paths. */
	XSecure_ChecksumParam ChecksumParams;  /**< WOTS+: Embedded checksum parameters for
						 XSecure_ComputeChecksum used in WOTS+ within XMSS
						 layers */
} XSecure_SlhdsaParam;

/**
 * Main SLH-DSA instance structure containing all required components for signature verification.
 * This structure encapsulates the complete SLH-DSA verification context including address
 * management, parameter configuration, hardware instances, and data buffers.
 */
typedef struct {
	/**< Address structure for domain separation and tree navigation in hash functions */
	ADRS* Addr;

	/**< Pointer to SLH-DSA parameter set configuration (SHAKE-256s, etc.) */
	XSecure_SlhdsaParam* Param;

	/**< Pointer to SHA instance to be used for SLH-DSA Driver */
	XSecure_Sha* ShaInstance;

	/**< Data instance containing all working buffers for signature verification */
	u64 DataAddr;           /**< Address of the input data to be verified */
	u32 DataLen;            /**< Length of the input data in bytes */
	u64 SignatureAddr;      /**< Address of the signature buffer */
	u32 SignatureLen;	/**< Length of the signature */
	u64 PublicKeyAddr;      /**< Address of the public key buffer */
	/**
	 * General-purpose buffer for SLH-DSA intermediate computations.
	 *
	 * Size: 67 * 32 = 2,144 bytes (16-byte aligned)
	 *
	 * Usage:
	 * - FORS: Store 22 tree roots (704 bytes)
	 * - WOTS+: Store 67 chain values (2,144 bytes)
	 * - XMSS: Store single node value (32 bytes)
	 * - Hypertree: Store layer root (32 bytes)
	 */
	u8* Data1;
	u32 Data1Len;		/**< Current length of Data1 buffer in bytes */
	/**
	 * Secondary buffer for hash outputs and base conversions.
	 *
	 * Size: 99 bytes (4-byte aligned)
	 *
	 * Usage:
	 * - Message digest: Store 47-byte digest
	 * - Hash operations: Store concatenated inputs (up to 64 bytes)
	 */
	u8* Data2;
	/**
	 * Tertiary buffer for single hash outputs.
	 *
	 * Size: 32 bytes (4-byte aligned)
	 *
	 * Usage:
	 * - Single hash outputs (F, H, T functions)
	 * - Tree node computations
	 * - Public key derivation results
	 * - Serialized address structure
	 */
	u8* Data3;
	/**
	 * Generic buffer for storing digits
	 *
	 * Size: 268 bytes
	 *
	 * Usage:
	 * - in WOTS: 67 * 4 = 268 Bytes
	 * - in FORS: k (22) * 4 = 88 Bytes
	 */
	u32* Data4;

} XSecure_SlhdsaInstance;

/**************************************** Function Prototypes *************************************/
XSecure_SlhdsaInstance* XSecure_SlhdsaGetInstance(void);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLH_DSA_INSTANCE_H_ */
