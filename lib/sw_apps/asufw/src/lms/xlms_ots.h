/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xlms_ots.h
*
* This file contains structures, constants and defines used in LMS OTS and
* provides interface to LMS OTS operations for ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 1.0   ss   01/07/26 Initial release
*
* </pre>
*
* @note
*
**************************************************************************************************/
/**
* @addtogroup xlms_server_apis LMS Server APIs
* @{
*/
#ifndef XLMS_OTS_H_
#define XLMS_OTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

#ifdef XASU_LMS_ENABLE
/************************************ Constant Definitions ***************************************/

/**
 * Max supported types, used for creating lookup table for parameters for types supported
 * number should be max valid + 1 (as first slot will be for invalid parameters)
 * XLms_OtsType
 */
#define	XLMS_OTS_TYPE_MAX_SUPPORTED	(17U)

/** Lookup table indices for LMS OTS parameter sets */
#define XLMS_OTS_PARAM_IDX_RESERVED	(0U)	/**< Index for reserved/unsupported type */
#define XLMS_OTS_PARAM_IDX_SHA256_W2	(1U)	/**< Index for SHA256_N32_W2 */
#define XLMS_OTS_PARAM_IDX_SHA256_W4	(2U)	/**< Index for SHA256_N32_W4 */
#define XLMS_OTS_PARAM_IDX_SHA256_W8	(3U)	/**< Index for SHA256_N32_W8 */
#define XLMS_OTS_PARAM_IDX_SHAKE_W2	(4U)	/**< Index for SHAKE_N32_W2 */
#define XLMS_OTS_PARAM_IDX_SHAKE_W4	(5U)	/**< Index for SHAKE_N32_W4 */
#define XLMS_OTS_PARAM_IDX_SHAKE_W8	(6U)	/**< Index for SHAKE_N32_W8 */
#define XLMS_OTS_PARAM_IDX_SHA256_W1	(7U)	/**< Index for SHA256_N32_W1 */
#define XLMS_OTS_PARAM_IDX_SHAKE_W1	(8U)	/**< Index for SHAKE_N32_W1 */
#define XLMS_OTS_PARAM_IDX_SHA256_N24_W1 (9U)	/**< Index for SHA256_N24_W1 */
#define XLMS_OTS_PARAM_IDX_SHA256_N24_W2 (10U)	/**< Index for SHA256_N24_W2 */
#define XLMS_OTS_PARAM_IDX_SHA256_N24_W4 (11U)	/**< Index for SHA256_N24_W4 */
#define XLMS_OTS_PARAM_IDX_SHA256_N24_W8 (12U)	/**< Index for SHA256_N24_W8 */
#define XLMS_OTS_PARAM_IDX_SHAKE_N24_W1  (13U)	/**< Index for SHAKE_N24_W1 */
#define XLMS_OTS_PARAM_IDX_SHAKE_N24_W2  (14U)	/**< Index for SHAKE_N24_W2 */
#define XLMS_OTS_PARAM_IDX_SHAKE_N24_W4  (15U)	/**< Index for SHAKE_N24_W4 */
#define XLMS_OTS_PARAM_IDX_SHAKE_N24_W8  (16U)	/**< Index for SHAKE_N24_W8 */

/**
 * Number of hash invocations for signature verification (2^w - 1)
 * These values represent the maximum hash chain iterations per Winternitz digit
 */
#define XLMS_OTS_NO_OF_INV_SIGN_W1	(1U)	/**< NoOfInvSign for W1: 2^1 - 1 = 1 */
#define XLMS_OTS_NO_OF_INV_SIGN_W2	(3U)	/**< NoOfInvSign for W2: 2^2 - 1 = 3 */
#define XLMS_OTS_NO_OF_INV_SIGN_W4	(15U)	/**< NoOfInvSign for W4: 2^4 - 1 = 15 */
#define XLMS_OTS_NO_OF_INV_SIGN_W8	(255U)	/**< NoOfInvSign for W8: 2^8 - 1 = 255 */

/* ********************* Sizes ********************************************** */

/** common lengths - they remain same irrespective of place they are used */
#define XLMS_OTS_TYPE_SIZE		(4U) /**< Length of type field */
#define XLMS_I_FIELD_SIZE		(16U) /**< Length of I field */
#define XLMS_Q_FIELD_SIZE		(4U) /**< Length of q field */
#define XLMS_N_FIELD_SIZE		(32U) /**< Maximum supported hash output bytes (n).
						  Used for buffer allocation only.
						  Runtime uses HashOutputBytes from params. */
#define XLMS_N24_FIELD_SIZE		(24U) /**< Hash output bytes for n=24 variants */
#define XLMS_C_FIELD_SIZE		(32U) /**< 32 byte random value for
						every msg to be authenticated */
#define XLMS_D_MESG_FIELD_SIZE		(2U) /**< Size of XLMS_D_MESG */
#define XLMS_D_PBLC_FIELD_SIZE		(2U) /**< Size of XLMS_D_PBLC */

/** Digest and Checksum related */
#define XLMS_DIGEST_SIZE			(XLMS_N_FIELD_SIZE)
						/**< 'n' bytes, output of SHA operation */
#define XLMS_CHECKSUM_FIELD_SIZE		(2U) /**< Length of checksum field after digest */

/** Max OTS signature length (for W1 variant: 4 + 32 * 266 = 8516) */
#define XLMS_OTS_MAX_SIGN_LEN		(8516U)

/** Digest of data to be authenticated concatenated with checksum value */
#define XLMS_DIGEST_CHECKSUM_SIZE		(XLMS_DIGEST_SIZE + \
						XLMS_CHECKSUM_FIELD_SIZE)

/** Prefix fields length, before sending actual data to sha engine for digest */
#define XLMS_MESSAGE_TO_DIGEST_PREFIX_SIZE	(XLMS_I_FIELD_SIZE + \
							XLMS_Q_FIELD_SIZE + \
							XLMS_D_MESG_FIELD_SIZE + \
							XLMS_C_FIELD_SIZE)

/** OTS public key sizes */
#define XLMS_OTS_PUB_KEY_FIXED_FIELD_SIZE		(XLMS_OTS_TYPE_SIZE + \
							XLMS_I_FIELD_SIZE + \
							XLMS_Q_FIELD_SIZE)
#define XLMS_OTS_PUB_KEY_K_FIELD_SIZE		(XLMS_N_FIELD_SIZE) /**< 'n' bytes */

/** OTS Public key total size */
#define XLMS_OTS_PUB_KEY_TOTAL_SIZE		(XLMS_OTS_PUB_KEY_FIXED_FIELD_SIZE + \
							XLMS_OTS_PUB_KEY_K_FIELD_SIZE)

/* Sizes related to fields in tmp buffer used for storing
 * intermediate values during hash chains for each
 * 'n' bytes from LMS OTS signature verification */
#define XLMS_OTS_SIGN_VERIF_TMP_BUFF_i_SIZE	(2U) /**< 'i' Field, this causes different
						hash output for hash position in signature */
#define XLMS_OTS_SIGN_VERIF_TMP_BUFF_j_SIZE	(1U) /**< 'j' Field, this causes different
						hash output for per iteration (inner loop) */
#define XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE	(XLMS_N_FIELD_SIZE)
						/**< Size of individual hash chain placed
						in sequence in signature */

/** Total size of buffer */
/* 1 reserved byte added at start of buffer, to manage word
 * aligned sha finish copies to buffer
 */
#define	XLMS_OTS_SIGN_VERIF_TMP_BUFF_TOTAL_SIZE		(XLMS_I_FIELD_SIZE + \
							XLMS_Q_FIELD_SIZE + \
							XLMS_OTS_SIGN_VERIF_TMP_BUFF_i_SIZE + \
							XLMS_OTS_SIGN_VERIF_TMP_BUFF_j_SIZE + \
							XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE)


/* Sizes related to fields in tmp buffer used for collecting
 * concatenated result of hash chain for each
 * digit processed in signature during verification */

/** Max possible size as supported, XLMS_OTS_W1 has the largest 'p' */
#define XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_z_SIZE		((u32)XLMS_OTS_W1_P *\
								 XLMS_N_FIELD_SIZE)

/** Total MAX Size of buffer used in OTS verification */
#define XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_TOTAL_SIZE	(XLMS_I_FIELD_SIZE + \
							XLMS_Q_FIELD_SIZE + \
							XLMS_D_PBLC_FIELD_SIZE + \
							XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_z_SIZE)

/* ************************ offsets ***************************************** */

/* OTS signature fields offset */

/** Offset of 'Type' field in OTS Signature buffer */
#define XLMS_OTS_SIGN_TYPE_FIELD_OFFSET			(0U)
/** Offset of 'C' field in OTS Signature buffer */
#define XLMS_OTS_SIGN_C_FIELD_OFFSET			(XLMS_OTS_SIGN_TYPE_FIELD_OFFSET + \
							XLMS_OTS_TYPE_SIZE)

/* Offsets of fields in tmp buffer used for storing intermediate
 * values during hash chains for each
 *'n' bytes from LMS OTS signature verification
 */

/** Offset of 'I' field in temporary buffer, where inner loops
 * result are stored, for reprocessing during OTS operations
 */
#define XLMS_OTS_SIGN_VERIF_TMP_BUFF_I_OFFSET		(1U)
							/** Offset of 'q' field in temporary buffer,
							 * where inner loops result are stored,
							 * for reprocessing during OTS operations
							 */
#define XLMS_OTS_SIGN_VERIF_TMP_BUFF_Q_OFFSET		(XLMS_OTS_SIGN_VERIF_TMP_BUFF_I_OFFSET + \
								XLMS_I_FIELD_SIZE)
							/** Offset of 'i' field in temporary buffer,
							 * where inner loops result are stored,
							 * for reprocessing during OTS operations */
#define XLMS_OTS_SIGN_VERIF_TMP_BUFF_I_CALC_OFFSET	(XLMS_OTS_SIGN_VERIF_TMP_BUFF_Q_OFFSET + \
								XLMS_Q_FIELD_SIZE)
							/** Offset of 'j' field in temporary buffer,
							 * where inner loops result are stored,
							 * for reprocessing during OTS operations */
#define XLMS_OTS_SIGN_VERIF_TMP_BUFF_j_OFFSET		(XLMS_OTS_SIGN_VERIF_TMP_BUFF_I_CALC_OFFSET + \
								XLMS_OTS_SIGN_VERIF_TMP_BUFF_i_SIZE)
							/** Offset of 'Y' field in temporary buffer,
							 * where inner loops result are stored,
							 * for reprocessing during OTS operations */
#define XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_OFFSET		(XLMS_OTS_SIGN_VERIF_TMP_BUFF_j_OFFSET + \
							XLMS_OTS_SIGN_VERIF_TMP_BUFF_j_SIZE)


/* Offsets of fields in tmp buffer used for collecting concatenated result of hash chain for each
   digit processed in signature during verification */

/** Offset of 'I' field in temporary buffer, where inner loops
 * result is concatenated during OTS operations
 */
#define XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_I_OFFSET	(0U)

/** Offset of 'q' field in temporary buffer, where inner loops
 * result is concatenated during OTS operations
 */
#define XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_Q_OFFSET	(XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_I_OFFSET + \
								XLMS_I_FIELD_SIZE)
/** Offset of D_PBLC field in temporary buffer, where inner loops
 * result is concatenated during OTS operations
 */
#define XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_D_PBLC_OFFSET	(XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_Q_OFFSET + \
								XLMS_Q_FIELD_SIZE)
/** Offset of 'z' field in temporary buffer, where inner loops
 * result is concatenated during OTS operations
 */
#define XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_Z_OFFSET	(XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_D_PBLC_OFFSET + \
								XLMS_D_PBLC_FIELD_SIZE)

/************************************** Type Definitions *****************************************/
/**
 * @brief Types of LMS_OTS supported
 *
 */
typedef enum {
	XLMS_OTS_RESERVED 	= 0x00000000U,	/**< Reserved */
	XLMS_OTS_SHA256_N32_W1 	= 0x00000001U,	/**< 'n' = 32, 'H' = SHA2-256, 'w' = 1 */
	XLMS_OTS_SHA256_N32_W2 	= 0x00000002U,	/**< 'n' = 32, 'H' = SHA2-256, 'w' = 2 */
	XLMS_OTS_SHA256_N32_W4 	= 0x00000003U,	/**< 'n' = 32, 'H' = SHA2-256, 'w' = 4 */
	XLMS_OTS_SHA256_N32_W8 	= 0x00000004U,	/**< 'n' = 32, 'H' = SHA2-256, 'w' = 8 */
	XLMS_OTS_SHA256_N24_W1 	= 0x00000005U,	/**< 'n' = 24, 'H' = SHA2-256, 'w' = 1 */
	XLMS_OTS_SHA256_N24_W2 	= 0x00000006U,	/**< 'n' = 24, 'H' = SHA2-256, 'w' = 2 */
	XLMS_OTS_SHA256_N24_W4 	= 0x00000007U,	/**< 'n' = 24, 'H' = SHA2-256, 'w' = 4 */
	XLMS_OTS_SHA256_N24_W8 	= 0x00000008U,	/**< 'n' = 24, 'H' = SHA2-256, 'w' = 8 */
	XLMS_OTS_SHAKE_N32_W1 	= 0x00000009U,	/**< 'n' = 32, 'H' = SHAKE-256, 'w' = 1 */
	XLMS_OTS_SHAKE_N32_W2 	= 0x0000000AU,	/**< 'n' = 32, 'H' = SHAKE-256, 'w' = 2 */
	XLMS_OTS_SHAKE_N32_W4 	= 0x0000000BU,	/**< 'n' = 32, 'H' = SHAKE-256, 'w' = 4 */
	XLMS_OTS_SHAKE_N32_W8 	= 0x0000000CU,	/**< 'n' = 32, 'H' = SHAKE-256, 'w' = 8 */
	XLMS_OTS_SHAKE_N24_W1 	= 0x0000000DU,	/**< 'n' = 24, 'H' = SHAKE-256, 'w' = 1 */
	XLMS_OTS_SHAKE_N24_W2 	= 0x0000000EU,	/**< 'n' = 24, 'H' = SHAKE-256, 'w' = 2 */
	XLMS_OTS_SHAKE_N24_W4 	= 0x0000000FU,	/**< 'n' = 24, 'H' = SHAKE-256, 'w' = 4 */
	XLMS_OTS_SHAKE_N24_W8 	= 0x00000010U,	/**< 'n' = 24, 'H' = SHAKE-256, 'w' = 8 */
	XLMS_OTS_NOT_SUPPORTED			/**< Not Supported */
} XLms_OtsType;

/**
 * @brief Number of bits per digit
 * Possible values are as supported
 */
typedef enum {
	XLMS_OTS_W_NOT_SUPPORTED	= 0U,	/**< Not Supported */
	XLMS_OTS_W1 		= 1U,	/**< 'w' = 1 */
	XLMS_OTS_W2 		= 2U,	/**< 'w' = 2 */
	XLMS_OTS_W4 		= 4U,	/**< 'w' = 4 */
	XLMS_OTS_W8 		= 8U	/**< 'w' = 8 */
} XLms_OtsWIndex;

/**
 * @brief Number of 'w' width digits possible in message Digest, excluding checksum
 * Possible values are as supported
 */
typedef enum {
	XLMS_OTS_U_NOT_SUPPORTED	= 0U,	/**< Not Supported */
	XLMS_OTS_W1_U 		= 256U,	/**< 'u' corresponding to n=32, 'w' = 1 */
	XLMS_OTS_W2_U 		= 128U,	/**< 'u' corresponding to n=32, 'w' = 2 */
	XLMS_OTS_W4_U 		= 64U,	/**< 'u' corresponding to n=32, 'w' = 4 */
	XLMS_OTS_W8_U 		= 32U,	/**< 'u' corresponding to n=32, 'w' = 8 */
	XLMS_OTS_N24_W1_U 	= 192U,	/**< 'u' corresponding to n=24, 'w' = 1 */
	XLMS_OTS_N24_W2_U 	= 96U,	/**< 'u' corresponding to n=24, 'w' = 2 */
	XLMS_OTS_N24_W4_U 	= 48U,	/**< 'u' corresponding to n=24, 'w' = 4 */
	XLMS_OTS_N24_W8_U 	= 24U	/**< 'u' corresponding to n=24, 'w' = 8 */
} XLms_OtsuIndex;

/**
 * @brief Is the number of 'w' width digits possible in checksum
 * Possible values are as supported
 */
typedef enum {
	XLMS_OTS_V_NOT_SUPPORTED	= 0U,	/**< Not Supported */
	XLMS_OTS_W1_V 		= 9U,	/**< 'v' corresponding to n=32, 'w' = 1 */
	XLMS_OTS_W2_V 		= 5U,	/**< 'v' corresponding to n=32, 'w' = 2 */
	XLMS_OTS_W4_V 		= 3U,	/**< 'v' corresponding to n=32, 'w' = 4 */
	XLMS_OTS_W8_V 		= 2U,	/**< 'v' corresponding to n=32, 'w' = 8 */
	XLMS_OTS_N24_W1_V 	= 8U,	/**< 'v' corresponding to n=24, 'w' = 1 */
	XLMS_OTS_N24_W2_V 	= 5U,	/**< 'v' corresponding to n=24, 'w' = 2 */
	XLMS_OTS_N24_W4_V 	= 3U,	/**< 'v' corresponding to n=24, 'w' = 4 */
	XLMS_OTS_N24_W8_V 	= 2U	/**< 'v' corresponding to n=24, 'w' = 8 */
} XLms_OtsvIndex;

/**
 * @brief No.of bits to left-shift in Checksum, in the 16 bit value
 * Possible values are as supported
 */
typedef enum {
	XLMS_OTS_W1_LS 		= 7U,	/**< 'ls' corresponding to n=32, 'w' = 1 */
	XLMS_OTS_W2_LS 		= 6U,	/**< 'ls' corresponding to 'w' = 2 */
	XLMS_OTS_W4_LS 		= 4U,	/**< 'ls' corresponding to 'w' = 4 */
	XLMS_OTS_W8_LS 		= 0U,	/**< 'ls' corresponding to 'w' = 8 */
	XLMS_OTS_N24_W1_LS 	= 8U,	/**< 'ls' corresponding to n=24, 'w' = 1 */
	XLMS_OTS_N24_W2_LS 	= 6U,	/**< 'ls' corresponding to n=24, 'w' = 2 */
	XLMS_OTS_N24_W4_LS 	= 4U,	/**< 'ls' corresponding to n=24, 'w' = 4 */
	XLMS_OTS_N24_W8_LS 	= 0U,	/**< 'ls' corresponding to n=24, 'w' = 8 */
	XLMS_OTS_LS_NOT_SUPPORTED = 16U	/**< Not Supported */
} XLms_OtslsIndex;

/**
 * @brief Is a sum of U + V, it is number of 'w' width digits possible in digest + checksum buffer
 * Possible values are as supported
 */
typedef enum {
	XLMS_OTS_P_NOT_SUPPORTED	= 0U,	/**< Not Supported */
	XLMS_OTS_W1_P 		= 265U,	/**< 'p' corresponding to n=32, 'w' = 1 */
	XLMS_OTS_W2_P 		= 133U,	/**< 'p' corresponding to n=32, 'w' = 2 */
	XLMS_OTS_W4_P 		= 67U,	/**< 'p' corresponding to n=32, 'w' = 4 */
	XLMS_OTS_W8_P 		= 34U,	/**< 'p' corresponding to n=32, 'w' = 8 */
	XLMS_OTS_N24_W1_P 	= 200U,	/**< 'p' corresponding to n=24, 'w' = 1 */
	XLMS_OTS_N24_W2_P 	= 101U,	/**< 'p' corresponding to n=24, 'w' = 2 */
	XLMS_OTS_N24_W4_P 	= 51U,	/**< 'p' corresponding to n=24, 'w' = 4 */
	XLMS_OTS_N24_W8_P 	= 26U	/**< 'p' corresponding to n=24, 'w' = 8 */
} XLms_OtspIndex;

/**
 * @brief LMS OTS parameters
 */
typedef struct {
	u32 HashAlgId;		/**< HASH function for selected type */
	u32 HashOutputBytes;	/**< The number of bytes of the output of the hash function */
	XLms_OtsWIndex w;	/**< The width (in bits) of the Winternitz coefficients; that is,
				the number of bits from the hash or checksum that is used with a
				single Winternitz chain.  It is a member of the set
				{ 1, 2, 4, 8 }. */
	XLms_OtsuIndex u;	/**< Represent the number of w-bit fields required to
				contain the hash of the message */
	XLms_OtsvIndex v;	/**< Represent the number of w-bit fields required to
				contain the checksum byte strings */
	XLms_OtslsIndex ls; /**< The number of left-shift bits used in the checksum function Cksm */
	XLms_OtspIndex p;	/**< The number of 'n'-byte string elements that make up the LM-OTS signature */
	u32 NoOfInvSign; /**< Number of invocations to get to signature per digit (2^w - 1) */
	u32 SignLen;	/**< Number of bytes in signature */
} XLms_OtsParam;

/**
 * @brief LMS OTS Public key structure
 *
 * Size = 4 + 16 + 4 + H Len (32) = 56 Bytes
 */
typedef union {
	u8 Buff[XLMS_OTS_PUB_KEY_TOTAL_SIZE]; /**< Raw buffer for OTS public key. */
	struct {
		/**
		 * @brief Type XLms_OtsType
		 * Size - 4 bytes, 0 to 3 bytes in public key
		 */
		XLms_OtsType Type;
		/**
		 * @brief OtsTreeId - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 4 to 19 bytes in public key
		 */
		u8 OtsTreeId[XLMS_I_FIELD_SIZE];
		/**
		 * @brief q - The leaf number q, goes from 0 on left most leaf to right most (2^h -1), in a single tree
		 * Size - 4 bytes, 20 to 23 bytes in public key
		 */
		u32 q;
		/**
		 * @brief K - H(I || u32str(q) || u16str(D_PBLC) || y[0] || ... || y[p-1])
		 * H is a hash function, supports only SHA2-256 and SHAKE-256 both are of 32Byte output len
		 * I & q remain same as described above
		 * D_PBLC is a constant XLMS_D_PBLC
		 * y[0] to y[p-1] is calculated from private key
		 */
		u8 K[XLMS_OTS_PUB_KEY_K_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;	/**< Fields of LMS OTS public key structure */
} XLms_OtsPublicKey;

/* ********************************************************************************************** */
/* ********************************************************************************************** */
/* LMS OTS Signature related */
/* ********************************************************************************************** */
/* ********************************************************************************************** */
/**
 * @brief Temporary buffer, used in LMS OTS signature verification, used in hash chain to arrive at values
 * which will be concatenated and hashed to get public value
 *
 * Size = 1 + 16 + 4 + 2 + 1 + H Len (32) = 56 Bytes
 */
typedef union {
	u8 Buff[1U + XLMS_OTS_SIGN_VERIF_TMP_BUFF_TOTAL_SIZE]; /**< Raw buffer for OTS signature verification. */
	struct {
		/**
		 * 1 reserved byte added at start of buffer, to manage word aligned sha finish copies to buffer
		 */
		u8 Reserved;
		/**
		 * TreeId - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 1 to 16 bytes
		 */
		u8 TreeId[XLMS_I_FIELD_SIZE];
		/**
		 * LeafIndex - The leaf number q, goes from 0 on left most leaf to right most (2^h -1), in a single tree
		 * Size - 4 bytes, 17 to 20 bytes
		 */
		u32 LeafIndex;
		/**
		 * DigitPosition - Digit position in (Digest || Checksum), 0 to (p-1) digits.
		 * Size - 2 Bytes, 21 to 22 bytes
		 */
		u16 DigitPosition;
		/**
		 * j - Index for inner loop during signature verification, starts at digit value in (Digest || Checksum) ends at (2^w - 1).
		 * Size - 1 Byte, 23nd byte
		 */
		u8 j;
		/**
		 * y[] - 'n' bytes picked from LMS OTS signatures. y[0] to y[p-1], each 'n' sized bytes will be
		 * copied here and passed into HASH engine along with other fields.
		 * Size - 32 Bytes, 24 to 55 bytes
		 */
		u8 y[XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE];
	}__attribute__((__packed__)) Fields;	/**< Fields of LMS OTS hash per digit structure */
} XLms_OtsHashPerDigit;

/**
 * @brief Structure used when validating LMS OTS, once hash chain is completed,
 * result needs to be stored so that they can be further concatenated and hashed
 * to get OTS public key, this provides a way to access members and fill data in a structured way
 *
 * Size = 16(I) + 4(q) + 2(D_PBLC) + (p * H Len (32))
 */
typedef union {
	u8 Buff[XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_TOTAL_SIZE]; /**< Raw buffer for OTS sign-to-public-key hash. */
	struct {
		/**
		 * ChainTreeId - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 0 to 15 bytes
		 */
		u8 ChainTreeId[XLMS_I_FIELD_SIZE];
		/**
		 * q - The leaf number q, goes from 0 on left most leaf to right most (2^h -1), in a single tree
		 * Size - 4 bytes, 16 to 19 bytes
		 */
		u32 q;
		/**
		 * D_PBLC - XLMS_D_PBLC
		 * Size - 2 Bytes, 20 to 21 bytes
		 */
		u8 D_PBLC[2U];
		/**
		 * z - Each z is a XLMS_N_FIELD_SIZE byte length, and ranges from 0 to p-1
		 * Size - (32 Bytes * p)
		 */
		u8 z[XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_z_SIZE];
	}__attribute__((__packed__)) Fields;	/**< Fields of LMS OTS Sign to Public Key Hash */
} XLms_OtsSignToPubKeyHash;

/**
 * @brief Partial LMS OTS Signature structure
 *
 * struct Size = 4 + 32
 * Sign size = 4 + 32 + (p * n) = 4 + n * (p+1)
 */
typedef struct {
	/**
	 * @brief Type XLms_OtsType
	 * Size - 4 bytes, 0 to 3 bytes in LMS OTS signature
	 */
	XLms_OtsType Type;
	/**
	 * @brief C = A uniformly random 'n'-byte string
	 * Size - 32 bytes, 4 to 35 bytes in LMS OTS signature
	 */
	u8 C[XLMS_C_FIELD_SIZE];
	/* Here rest of the bytes of signature represented by y */
} XLms_OtsSignature;

/* ********************************************************************************************** */
/* ********************************************************************************************** */
/* Digest related */
/* ********************************************************************************************** */
/* ********************************************************************************************** */
/**
 * @brief Temporary buffer, used to send prefix fields to calculate digest for data to be authenticate
 *
 * Size = I (16) + q (4) + D_MESG (2) + C (32) = 54 Bytes
 */
typedef union {
	u8 Buff[XLMS_MESSAGE_TO_DIGEST_PREFIX_SIZE]; /**< Raw buffer for digest prefix fields. */
	struct {
		/**
		 * DigestTreeId - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 0 to 15 bytes
		 */
		u8 DigestTreeId[XLMS_I_FIELD_SIZE];
		/**
		 * q - The leaf number q, goes from 0 on left most leaf to right most (2^h -1), in a single tree
		 * Size - 4 bytes, 16 to 19 bytes
		 */
		u32 q;
		/**
		 * D_MESG - XLMS_D_MESG
		 * Size - 2 Bytes, 20 to 21 bytes
		 */
		u8 D_MESG[XLMS_D_MESG_FIELD_SIZE];
		/**
		 * C - Randomizer per data
		 * Size - 32 Byte, 22 to 53 bytes
		 */
		u8 C[XLMS_C_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;	/**< Fields of LMS data digest fixed */
} XLms_DataDigestFixedFields;

/**
 * @brief Digest for message to be authenticated, along with checksum
 *
 * Size = Digest (32) + Check sum (2) = 34 Bytes
 */
typedef union {
	u8 Buff[XLMS_DIGEST_CHECKSUM_SIZE]; /**< Raw buffer for digest and checksum. */
	struct {
		/**
		 * Digest - Digest of data to be authenticated
		 * Size - 32 bytes, 0 to 31 bytes
		 */
		u8 Digest[XLMS_DIGEST_SIZE];
		/**
		 * Checksum - Checksum on Digest
		 * Size - 2 bytes, 32nd & 33rd byte
		 */
		u8 Checksum[XLMS_CHECKSUM_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;	/**< Fields of the digest structure */
} XLms_DataDigest;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
u32 XLms_OtsCoeff(u8 const* const Arr, const u32 ArrayIndex, const u32 w);
s32 XLms_OtsComputeChecksum(const u8* const Array, const u32 ArrayLen,
				const u32 w, const u32 ls, u32* const Checksum);
s32 XLms_OtsLookupParamSet(XLms_OtsType Type,
	const XLms_OtsParam** Parameters);
#endif /* XASU_LMS_ENABLE */

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XLMS_OTS_H_ */
/** @} */
