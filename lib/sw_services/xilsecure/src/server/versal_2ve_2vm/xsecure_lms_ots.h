/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xsecure_lms_ots.h
*
* This file contains structures, constants and defines used in LMS OTS and
* provides interface to LMS OTS operations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/
#ifndef XSECURE_LMS_OTS_H_
#define XSECURE_LMS_OTS_H_

#include "xil_types.h"
#include "xsecure_sha.h"

/************************** Constant Definitions ****************************/

/**
 * Max supported types, used for creating lookup table for parameters for types supported
 * number should be max valid + 1 (as first slot will be for invalid parameterXSecure_LmsOtsType
 * XSecure_LmsOtsType
 */
#define	XSECURE_LMS_OTS_TYPE_MAX_SUPPORTED	(7U)
#define XSECURE_ALLFS   (0xFFFFFFFFU)

/* ********************* Sizes ********************************************** */

/* common lengths - they remain same irrespective of place they are used */
#define XSECURE_LMS_OTS_TYPE_SIZE		(4U) /** length of type field */
#define XSECURE_LMS_I_FIELD_SIZE		(16U)/** length of I field */
#define XSECURE_LMS_Q_FIELD_SIZE		(4U) /** length of q field */
#define XSECURE_LMS_N_FIELD_SIZE		(32U)/** For now supported variants o/p length match */
#define XSECURE_LMS_C_FIELD_SIZE		(32U)/** 32 byte random value for every msg to be authenticated */
#define XSECURE_LMS_D_MESG_FIELD_SIZE		(2U)/** Size of @ref XSECURE_D_MESG */
#define XSECURE_LMS_D_PBLC_FIELD_SIZE		(2U)/** Size of @ref XSECURE_D_PBLC */

/* Digest and Checksum related */
#define XSECURE_LMS_DIGEST_SIZE			(XSECURE_LMS_N_FIELD_SIZE)/** 'n' bytes, output of sha op */
#define XSECURE_LMS_CHECKSUM_FIELD_SIZE		(2U)/** Length of checksum field after digest */

/** Digest of data to be authenticated concatenated with checksum value */
#define XSECURE_LMS_DIGEST_CHECKSUM_SIZE	(XSECURE_LMS_DIGEST_SIZE + \
						XSECURE_LMS_CHECKSUM_FIELD_SIZE)

/** Prefix fields length, before sending actual data to sha engine for digest */
#define XSECURE_LMS_MESSAGE_TO_DIGEST_PREFIX_SIZE	(XSECURE_LMS_I_FIELD_SIZE + \
							XSECURE_LMS_Q_FIELD_SIZE + \
							XSECURE_LMS_D_MESG_FIELD_SIZE + \
							XSECURE_LMS_C_FIELD_SIZE)

/* OTS public key sizes */
#define XSECURE_LMS_OTS_PUB_KEY_FIXED_FIELD_SIZE	(XSECURE_LMS_OTS_TYPE_SIZE + \
							XSECURE_LMS_I_FIELD_SIZE + \
							XSECURE_LMS_Q_FIELD_SIZE)
#define XSECURE_LMS_OTS_PUB_KEY_K_FIELD_SIZE		(XSECURE_LMS_N_FIELD_SIZE)/** 'n' bytes */

/** OTS Public key total size */
#define XSECURE_LMS_OTS_PUB_KEY_TOTAL_SIZE		(XSECURE_LMS_OTS_PUB_KEY_FIXED_FIELD_SIZE + \
							XSECURE_LMS_OTS_PUB_KEY_K_FIELD_SIZE)

/* Sizes related to fields in tmp buffer used for storing
 * intermediate values during hash chains for each
 * 'n' bytes from LMS OTS signature verification */
#define XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_i_SIZE	(2U) /** 'i' Field, this causes different hash
								output for hash position in signature */
#define XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_j_SIZE	(1U) /** 'j' Field, this causes different hash
								output for per iteration (inner loop) */
#define XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE	(XSECURE_LMS_N_FIELD_SIZE) /** Size of individual
								hash chain placed in sequence in signature */

/** Total size of buffer, @ref XSecure_LmsOtsHashPerDigit */
/* 1 reserved byte added at start of buffer, to manage word
 * aligned sha finish copies to buffer
 */
#define	XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_TOTAL_SIZE		(XSECURE_LMS_I_FIELD_SIZE + \
								XSECURE_LMS_Q_FIELD_SIZE + \
								XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_i_SIZE + \
								XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_j_SIZE + \
								XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE)


/* Sizes related to fields in tmp buffer used for collecting
 * concatenated result of hash chain for each
 * digit processed in signature during verification */
#define XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_CURR_z_SIZE(p)  ((u32)p * XSECURE_LMS_N_FIELD_SIZE)												/** Used for calculating the size of z array,
								based on p value */
#define XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_z_SIZE	((u32)XSECURE_LMS_OTS_W2_P *\
								 XSECURE_LMS_N_FIELD_SIZE)
								/** Max possible size as supported
 * 								only till @ref XSECURE_LMS_OTS_W2 */
/** Total MAX Size of buffer used in OTS verification */
#define XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_TOTAL_SIZE	(XSECURE_LMS_I_FIELD_SIZE + \
								XSECURE_LMS_Q_FIELD_SIZE + \
								XSECURE_LMS_D_PBLC_FIELD_SIZE + \
								XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_z_SIZE)

/** Total Size of buffer used in OTS verification, while considering current 'p' parameter */
#define XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_CURRENT_SIZE(p) (XSECURE_LMS_I_FIELD_SIZE +\
								XSECURE_LMS_Q_FIELD_SIZE +\
								XSECURE_LMS_D_PBLC_FIELD_SIZE +\
								XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_CURR_z_SIZE(p))

/* ************************ offsets ***************************************** */

/* Public key offsets */

/** Offset of 'I' field in OTS Public key */
#define XSECURE_LMS_OTS_PUBKEY_I_OFFSET				(0U)
/** Offset of 'q' field in OTS Public key */
#define XSECURE_LMS_OTS_PUBKEY_Q_OFFSET				(XSECURE_LMS_OTS_PUBKEY_I_OFFSET +\
								 XSECURE_LMS_I_FIELD_SIZE)


/* OTS signature fields offset */

/** Offset of 'Type' field in OTS Signature buffer */
#define XSECURE_LMS_OTS_SIGN_TYPE_FIELD_OFFSET			(0U)
/** Offset of 'C' field in OTS Signature buffer */
#define XSECURE_LMS_OTS_SIGN_C_FIELD_OFFSET			(XSECURE_LMS_OTS_SIGN_TYPE_FIELD_OFFSET + \
								XSECURE_LMS_OTS_TYPE_SIZE)
/** Offset of 'Y' field in OTS Signature buffer */
#define XSECURE_LMS_OTS_SIGN_Y_FIELD_OFFSET			(XSECURE_LMS_OTS_SIGN_C_FIELD_OFFSET + \
								XSECURE_LMS_C_FIELD_SIZE)


/* Offsets of fields in tmp buffer used for storing intermediate
 * values during hash chains for each
 *'n' bytes from LMS OTS signature verification
 */

/** Offset of 'I' field in temporary buffer, where inner loops
 * result are stored, for reprocessing during OTS operations
 */
#define XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_I_OFFSET		(1U)
								/** Offset of 'q' field in temporary buffer,
								 * where inner loops result are stored,
								 * for reprocessing during OTS operations
								 */
#define XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Q_OFFSET		(XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_I_OFFSET + \
								XSECURE_LMS_I_FIELD_SIZE)
								/** Offset of 'i' field in temporary buffer,
								 * where inner loops result are stored,
								 * for reprocessing during OTS operations */
#define XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_i_OFFSET		(XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Q_OFFSET + \
								XSECURE_LMS_Q_FIELD_SIZE)
								/** Offset of 'j' field in temporary buffer,
								 * where inner loops result are stored,
								 * for reprocessing during OTS operations */
#define XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_j_OFFSET		(XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_i_OFFSET + \
								XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_i_SIZE)
								/** Offset of 'Y' field in temporary buffer,
								 * where inner loops result are stored,
								 * for reprocessing during OTS operations */
#define XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_OFFSET		(XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_j_OFFSET + \
								XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_j_SIZE)


/* Offsets of fields in tmp buffer used for collecting concatenated result of hash chain for each
   digit processed in signature during verification */

/** Offset of 'I' field in temporary buffer, where inner loops
 * result is concatenated during OTS operations
 */
#define XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_I_OFFSET	(0U)

/** Offset of 'q' field in temporary buffer, where inner loops
 * result is concatenated during OTS operations
 */
#define XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_Q_OFFSET	(XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_I_OFFSET + \
								XSECURE_LMS_I_FIELD_SIZE)
/** Offset of D_PBLC field in temporary buffer, where inner loops
 * result is concatenated during OTS operations
 */
#define XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_D_PBLC_OFFSET	(XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_Q_OFFSET + \
								XSECURE_LMS_Q_FIELD_SIZE)
/** Offset of 'z' field in temporary buffer, where inner loops
 * result is concatenated during OTS operations
 */
#define XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_Z_OFFSET	(XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_D_PBLC_OFFSET + \
								XSECURE_LMS_D_PBLC_FIELD_SIZE)
/***************************** Type Definitions******************************/
/**
 * @brief Types of LMS_OTS supported
 *
 * @note Only 32 byte output is supported, i.e., 256 bit SHA or SHAKE are only supported, w=1 is not supported
 */
typedef enum {
	XSECURE_LMS_OTS_RESERVED 	= 0x00000000U,
	XSECURE_LMS_OTS_SHA256_N32_W2 	= 0x00000002U,		/** 'n' = 32, 'H' = SHA2-256, 'w' = 2 */
	XSECURE_LMS_OTS_SHA256_N32_W4 	= 0x00000003U,		/** 'n' = 32, 'H' = SHA2-256, 'w' = 4 */
	XSECURE_LMS_OTS_SHA256_N32_W8 	= 0x00000004U,		/** 'n' = 32, 'H' = SHA2-256, 'w' = 8 */
	XSECURE_LMS_OTS_SHAKE_N32_W2 	= 0x0000000AU,		/** 'n' = 32, 'H' = SHAKE-256, 'w' = 2 */
	XSECURE_LMS_OTS_SHAKE_N32_W4 	= 0x0000000BU,		/** 'n' = 32, 'H' = SHAKE-256, 'w' = 4 */
	XSECURE_LMS_OTS_SHAKE_N32_W8 	= 0x0000000CU,		/** 'n' = 32, 'H' = SHAKE-256, 'w' = 8 */
	XSECURE_LMS_OTS_NOT_SUPPORTED
} XSecure_LmsOtsType;

/**
 * @brief Number of bits per digit
 * Possible values are as supported by ROM, w=1 is not supported
 */
typedef enum {
	XSECURE_LMS_OTS_W_NOT_SUPPORTED	= 0U,
	XSECURE_LMS_OTS_W2 		= 2U,		/** 'w' = 2 */
	XSECURE_LMS_OTS_W4 		= 4U,		/** 'w' = 4 */
	XSECURE_LMS_OTS_W8 		= 8U		/** 'w' = 8 */
} XSecure_LmsOtsWIndex;

/**
 * @brief Number of 'w' width digits possible in message Digest, excluding checksum
 * Possible values are as supported by ROM
 */
typedef enum {
	XSECURE_LMS_OTS_U_NOT_SUPPORTED	= 0U,
	XSECURE_LMS_OTS_W2_U 		= 128U,		/** 'u' corrsponding to 'w' = 2 */
	XSECURE_LMS_OTS_W4_U 		= 64U,		/** 'u' corrsponding to 'w' = 4 */
	XSECURE_LMS_OTS_W8_U 		= 32U		/** 'u' corrsponding to 'w' = 8 */
} XSecure_LmsOtsuIndex;

/**
 * @brief Is the number of 'w' width digits possible in checksum
 * Possible values are as supported by ROM
 */
typedef enum {
	XSECURE_LMS_OTS_V_NOT_SUPPORTED	= 0U,
	XSECURE_LMS_OTS_W2_V 		= 5U,		/** 'v' corrsponding to 'w' = 2 */
	XSECURE_LMS_OTS_W4_V 		= 3U,		/** 'v' corrsponding to 'w' = 4 */
	XSECURE_LMS_OTS_W8_V 		= 2U		/** 'v' corrsponding to 'w' = 8 */
} XSecure_LmsOtsvIndex;

/**
 * @brief No.of bits to left-shift in Checksum, in the 16 bit value
 * Possible values are as supported by ROM
 */
typedef enum {
	XSECURE_LMS_OTS_W2_LS 		= 6U,	/** 'ls' corrsponding to 'w' = 2 */
	XSECURE_LMS_OTS_W4_LS 		= 4U,	/** 'ls' corrsponding to 'w' = 4 */
	XSECURE_LMS_OTS_W8_LS 		= 0U,	/** 'ls' corrsponding to 'w' = 8 */
	XSECURE_LMS_OTS_LS_NOT_SUPPORTED = 16U
} XSecure_LmsOtslsIndex;

/**
 * @brief Is a sum of U + V, it is number of 'w' width digits possible in digest + checksum buffer
 * Possible values are as supported by ROM
 */
typedef enum {
	XSECURE_LMS_OTS_P_NOT_SUPPORTED	= 0U,
	XSECURE_LMS_OTS_W2_P 		= 133U,		/** 'p' corrsponding to 'w' = 2 */
	XSECURE_LMS_OTS_W4_P 		= 67U,		/** 'p' corrsponding to 'w' = 4 */
	XSECURE_LMS_OTS_W8_P 		= 34U		/** 'p' corrsponding to 'w' = 8 */
} XSecure_LmsOtspIndex;

typedef struct {
	XSecure_ShaMode H;	/** HASH function for selected type */
	u32 n;			/** The number of bytes of the output of the hash function */
	XSecure_LmsOtsWIndex w;	/** The width (in bits) of the Winternitz coefficients; that is,
				the number of bits from the hash or checksum that is used with a
				single Winternitz chain.  It is a member of the set
				{ 1, 2, 4, 8 }. */
	XSecure_LmsOtsuIndex u;	/** Represent the number of w-bit fields required to
				contain the hash of the message */
	XSecure_LmsOtsvIndex v;	/** Represent the number of w-bit fields required to
				contain the checksum byte strings */
	XSecure_LmsOtslsIndex ls; /** The number of left-shift bits used in the checksum function Cksm */
	XSecure_LmsOtspIndex p;	/** The number of 'n'-byte string elements that make up the LM-OTS signature */
	u32 NoOfInvSign; /** Number of invocations to get to signature per digit (2^w - 1) */
	u32 SignLen;	/** Number of bytes in signature */
} XSecure_LmsOtsParam;

/**
 * @brief LMS OTS Public key structure
 *
 * Size = 4 + 16 + 4 + H Len (32) = 56 Bytes
 */
typedef union XSecure_LmsOtsPublicKey_ {
	u8 Buff[XSECURE_LMS_OTS_PUB_KEY_TOTAL_SIZE];
	struct {
		/**
		 * @brief Type @ref XSecure_LmsOtsType
		 * Size - 4 bytes, 0 to 3 bytes in public key
		 */
		XSecure_LmsOtsType Type;
		/**
		 * @brief I - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 4 to 19 bytes in public key
		 */
		u8 I[XSECURE_LMS_I_FIELD_SIZE];
		/**
		 * @brief q - The leaf number q, goes from 0 on left most leaf to right most (2^h -1), in a single tree
		 * Size - 4 bytes, 20 to 23 bytes in public key
		 */
		u32 q;
		/**
		 * @brief K - H(I || u32str(q) || u16str(D_PBLC) || y[0] || ... || y[p-1])
		 * H is a hash function, ROM supports only SHA2-256 and SHAKE-256 both are of 32Byte output len
		 * I & q remain same as described above
		 * D_PBLC is a constant @ref XSECURE_D_PBLC
		 * y[0] to y[p-1] is calculated from private key
		 */
		u8 K[XSECURE_LMS_OTS_PUB_KEY_K_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;
} XSecure_LmsOtsPublicKey;

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
typedef union XSecure_LmsOtsHashPerDigit_ {
	u8 Buff[1U + XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_TOTAL_SIZE];
	struct {
		/**
		 * 1 reserved byte added at start of buffer, to manage word aligned sha finish copies to buffer
		 */
		u8 Reserved;
		/**
		 * I - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 1 to 16 bytes
		 */
		u8 I[XSECURE_LMS_I_FIELD_SIZE];
		/**
		 * q - The leaf number q, goes from 0 on left most leaf to right most (2^h -1), in a single tree
		 * Size - 4 bytes, 17 to 20 bytes
		 */
		u32 q;
		/**
		 * i - Digit position in (Digest || Checksum), 0 to (p-1) digits.
		 * Size - 2 Bytes, 21 to 22 bytes
		 */
		u16 i;
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
		u8 y[XSECURE_LMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE];
	}__attribute__((__packed__)) Fields;
} XSecure_LmsOtsHashPerDigit;

/**
 * @brief Structure used when validating LMS OTS, once hash chain is completed,
 * result needs to be stored so that they can be further concatenated and hashed
 * to get OTS public key, this provides a way to access members and fill data in a structured way
 *
 * Size = 16(I) + 4(q) + 2(D_PBLC) + (p * H Len (32))
 */
typedef union XSecure_LmsOtsSignToPubKeyHash_ {
	u8 Buff[XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_TOTAL_SIZE];
	struct {
		/**
		 * I - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 0 to 15 bytes
		 */
		u8 I[XSECURE_LMS_I_FIELD_SIZE];
		/**
		 * q - The leaf number q, goes from 0 on left most leaf to right most (2^h -1), in a single tree
		 * Size - 4 bytes, 16 to 19 bytes
		 */
		u32 q;
		/**
		 * D_PBLC - @ref XSECURE_D_PBLC
		 * Size - 2 Bytes, 20 to 21 bytes
		 */
		u8 D_PBLC[2U];
		/**
		 * z - Each z is a @ref XSECURE_LMS_N_FIELD_SIZE byte length, and ranges from 0 to p-1
		 * Size - (32 Bytes * p)
		 */
		u8 z[XSECURE_LMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_z_SIZE];
	}__attribute__((__packed__)) Fields;
} XSecure_LmsOtsSignToPubKeyHash;

/**
 * @brief Partial LMS OTS Signature structure
 *
 * struct Size = 4 + 32
 * Sign size = 4 + 32 + (p * n) = 4 + n * (p+1)
 */
typedef struct XSecure_LmsOtsSignature_ {
	/**
	 * @brief Type @ref XSecure_LmsOtsType
	 * Size - 4 bytes, 0 to 3 bytes in LMS OTS signature
	 */
	XSecure_LmsOtsType Type;
	/**
	 * @brief C = A uniformly random 'n'-byte string
	 * Size - 32 bytes, 4 to 35 bytes in LMS OTS signature
	 */
	u8 C[XSECURE_LMS_C_FIELD_SIZE];
	/* Here rest of the bytes of signature represented by y */
} XSecure_LmsOtsSignature;

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
typedef union XSecure_LmsDataDigestFixedFields_ {
	u8 Buff[XSECURE_LMS_MESSAGE_TO_DIGEST_PREFIX_SIZE];
	struct {
		/**
		 * I - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 0 to 15 bytes
		 */
		u8 I[XSECURE_LMS_I_FIELD_SIZE];
		/**
		 * q - The leaf number q, goes from 0 on left most leaf to right most (2^h -1), in a single tree
		 * Size - 4 bytes, 16 to 19 bytes
		 */
		u32 q;
		/**
		 * D_MESG - @ref XSECURE_D_MESG
		 * Size - 2 Bytes, 20 to 21 bytes
		 */
		u8 D_MESG[XSECURE_LMS_D_MESG_FIELD_SIZE];
		/**
		 * C - Randomizer per data
		 * Size - 32 Byte, 22 to 53 bytes
		 */
		u8 C[XSECURE_LMS_C_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;
} XSecure_LmsDataDigestFixedFields;

/**
 * @brief Digest for message to be authenticated, along with checksum
 *
 * Size = Digest (32) + Check sum (2) = 34 Bytes
 */
typedef union XSecure_LmsDataDigest_ {
	u8 Buff[XSECURE_LMS_DIGEST_CHECKSUM_SIZE];
	struct {
		/**
		 * Digest - Digest of data to be authenticated
		 * Size - 32 bytes, 0 to 31 bytes
		 */
		u8 Digest[XSECURE_LMS_DIGEST_SIZE];
		/**
		 * Checksum - Checksum on Digest
		 * Size - 2 bytes, 32nd & 33rd byte
		 */
		u8 Checksum[XSECURE_LMS_CHECKSUM_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;
} XSecure_LmsDataDigest;

/***************************** Function Prototypes ******************************************/
u32 XSecure_LmsOtsCoeff(u8 const* const Arr, const u32 ArrayIndex, const u32 w);
int XSecure_LmsOtsComputeChecksum(const u8* const Array, const u32 ArrayLen,
				const u32 w, const u32 ls, u32* const Checksum);
int XSecure_LmsOtsLookupParamSet(XSecure_LmsOtsType Type,
	XSecure_LmsOtsParam** Parameters);

#endif /* XSECURE_LMS_OTS_H_ */
/** @} */
