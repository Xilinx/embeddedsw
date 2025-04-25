/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/******************************************************************************/
/**
*
* @file xsecure_lms.h
*
* This file contains structures, constants and defines used in LMS and HSS
* provides interface to LMS operations
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
#ifndef XSECURE_LMS_H_
#define XSECURE_LMS_H_

#include "xil_types.h"
#include "xsecure_lms_ots.h"

#define XSECURE_IS_ODD(Value)		((Value & 0x00000001U) == 0x00000001U)

/************************** Constant Definitions ****************************/

/**
 * Max supported types, used for creating lookup table for parameters for types
 * supported number should be max valid + 1 (as first slot will be for invalid parameters)
 * XSecure_LmsType from 0 (unsupported) to XSECURE_LMS_SHAKE_M32_H20
 */
#define XSECURE_LMS_TYPE_MAX_SUPPORTED			(9U)

/* Common lengths - they remain same irrespective of place they are used */
#define XSECURE_LMS_TYPE_SIZE				(4U)	/** Length of type field */
#define XSECURE_LMS_M_BYTE_FIELD_SIZE			(32U)

/* Public key related sizes */
#define XSECURE_LMS_PUB_KEY_FIXED_FIELD_SIZE		(XSECURE_LMS_TYPE_SIZE + \
							XSECURE_LMS_OTS_TYPE_SIZE + \
							XSECURE_LMS_I_FIELD_SIZE)
#define XSECURE_LMS_PUB_KEY_T_FIELD_SIZE		(XSECURE_LMS_M_BYTE_FIELD_SIZE)
#define XSECURE_LMS_PUB_KEY_TOTAL_SIZE			(XSECURE_LMS_PUB_KEY_FIXED_FIELD_SIZE + \
							XSECURE_LMS_PUB_KEY_T_FIELD_SIZE)

/* Sizes related to tmp buffer used to calculate root public key from leaf */
#define	XSECURE_MERKLE_TREE_NODE_NUMBER_SIZE		(XSECURE_LMS_Q_FIELD_SIZE)
#define	XSECURE_LMS_D_LEAF_FIELD_SIZE			(2U)
#define	XSECURE_LMS_D_INTR_FIELD_SIZE			(2U)
#define	XSECURE_LMS_D_FIELD_SIZE			(2U)

/* Used for leaf node processing */
#define XSECURE_LMS_PUB_KEY_TMP_BUFFER_LEAF_TOTAL_SIZE	(XSECURE_LMS_I_FIELD_SIZE + \
							XSECURE_MERKLE_TREE_NODE_NUMBER_SIZE + \
							XSECURE_LMS_D_LEAF_FIELD_SIZE + \
							XSECURE_LMS_M_BYTE_FIELD_SIZE)

/* Used for internal nodes processing */
#define XSECURE_LMS_PUB_KEY_TMP_BUFFER_INTR_TOTAL_SIZE	(XSECURE_LMS_I_FIELD_SIZE + \
							XSECURE_MERKLE_TREE_NODE_NUMBER_SIZE + \
							XSECURE_LMS_D_INTR_FIELD_SIZE + \
							XSECURE_LMS_M_BYTE_FIELD_SIZE + \
							XSECURE_LMS_M_BYTE_FIELD_SIZE)

/* Offsets */

/* LMS public key related offsets */
#define XSECURE_LMS_PUB_KEY_TYPE_OFFSET			(0U)
#define XSECURE_LMS_PUB_KEY_OTS_TYPE_OFFSET		(XSECURE_LMS_PUB_KEY_TYPE_OFFSET + \
							XSECURE_LMS_TYPE_SIZE)
#define XSECURE_LMS_PUB_KEY_I_OFFSET			(XSECURE_LMS_PUB_KEY_OTS_TYPE_OFFSET + \
							XSECURE_LMS_OTS_TYPE_SIZE)
#define XSECURE_LMS_PUB_KEY_T_OFFSET			(XSECURE_LMS_PUB_KEY_I_OFFSET + \
							XSECURE_LMS_I_FIELD_SIZE)

/* LMS Signature offsets */
#define XSECURE_LMS_SIGNATURE_Q_FIELD_OFFSET		(0U)
#define XSECURE_LMS_SIGNATURE_OTS_FIELD_OFFSET		(XSECURE_LMS_SIGNATURE_Q_FIELD_OFFSET + \
							XSECURE_LMS_Q_FIELD_SIZE)

/***************************** Type Definitions******************************/
/**
 * @brief Supported LMS Types from standard
 */
typedef enum {
	XSECURE_LMS_RESERVED = 0x00000000U,
	XSECURE_LMS_SHA256_M32_H5 = 0x00000005U,		/** 'M' = 32, 'H' =  SHA2-256, 'h' = 5 */
	XSECURE_LMS_SHA256_M32_H10 = 0x00000006U,		/** 'M' = 32, 'H' =  SHA2-256, 'h' = 10 */
	XSECURE_LMS_SHA256_M32_H15 = 0x00000007U,		/** 'M' = 32, 'H' =  SHA2-256, 'h' = 15 */
	XSECURE_LMS_SHA256_M32_H20 = 0x00000008U,		/** 'M' = 32, 'H' =  SHA2-256, 'h' = 20 */
	XSECURE_LMS_SHAKE_M32_H5 = 0x0000000FU,			/** 'M' = 32, 'H' =  SHAKE-256, 'h' = 5 */
	XSECURE_LMS_SHAKE_M32_H10 = 0x00000010U,		/** 'M' = 32, 'H' =  SHAKE-256, 'h' = 10 */
	XSECURE_LMS_SHAKE_M32_H15 = 0x00000011U,		/** 'M' = 32, 'H' =  SHAKE-256, 'h' = 15 */
	XSECURE_LMS_SHAKE_M32_H20 = 0x00000012U,		/** 'M' = 32, 'H' =  SHAKE-256, 'h' = 20 */
	XSECURE_LMS_NOT_SUPPORTED
} XSecure_LmsType;

/**
 * @brief Different Heights supported for Merkle Tree, 'h'=25 not supported
 */
typedef enum {
	XSECURE_LMS_TREE_HEIGHT_UNSUPPORTED = 0U,
	XSECURE_LMS_TREE_HEIGHT_H5 = 5U,
	XSECURE_LMS_TREE_HEIGHT_H10 = 10U,
	XSECURE_LMS_TREE_HEIGHT_H15 = 15U,
	XSECURE_LMS_TREE_HEIGHT_H20 = 20U
} XSecure_LmsTreeHeight;

/**
 * @brief Structure for maintaining parameters for each LMS Type
 */
typedef struct {
	XSecure_ShaMode H; /** HASH function for selected type */
	u32 m;	/** The number of bytes of the output of the hash function, same as 'n' in LMS OTS */
	XSecure_LmsTreeHeight h; /** Height of Merkle tree */
	u32 mh;	/** Product of m and h, used in signature length determination */
} XSecure_LmsParam;

/**
 * @brief LMS Public key structure to access root value
 *
 * Size = 4 + 4 + 16 + H Len (32) = 56 Bytes
 */
typedef union XSecure_LmsPublicKey_ {
	u8 Buff[XSECURE_LMS_PUB_KEY_TOTAL_SIZE];
	struct {
		/**
		 * @brief LmsType @ref  XSecure_LmsType
		 * Size - 4 bytes, 0 to 3 in public key
		 */
		XSecure_LmsType LmsType;
		/**
		 * @brief OtsType @ref XSecure_LmsOtsType
		 * Size - 4 bytes, 4 to 7 bytes in public key
		 */
		XSecure_LmsOtsType OtsType;
		/**
		 * @brief I - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 8 to 23 bytes in public key
		 */
		u8 I[XSECURE_LMS_I_FIELD_SIZE];
		/**
		 * @brief T[1] - H(I||u32str(r)||u16str(D_INTR)||T[2*r]||T[2*r+1])
		 *
		 * 'H' is a hash function, PLM supports only SHA2-256 and SHAKE-256
		 * both are of 32Byte output len
		 *
		 * 'r' is the node number, same as 'q', goes from 0 on left most
		 * leaf to right most (2^h -1), in a single tree
		 * D_INTR is a constant @ref XSECURE_D_INTR
		 */
		u8 T[XSECURE_LMS_PUB_KEY_T_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;
} XSecure_LmsPublicKey;

/**
 * @brief Used to store data which is repeatedly sent to SHA engine,
 * during LMS root value calculation
 *
 * Size = 16(I) + 4(node_number/2) + 2(D_INTR) + (32 * 2)
 */
typedef union XSecure_LmsPubKeyTmp_ {
	u8 Buff[XSECURE_LMS_PUB_KEY_TMP_BUFFER_INTR_TOTAL_SIZE];
	struct {
		/**
		 * I - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 0 to 15 bytes
		 */
		u8 I[XSECURE_LMS_I_FIELD_SIZE];
		/**
		 * (node number/2), used to get unique hash o/p for same data for every node
		 * Size - 4 bytes, 16 to 20 bytes
		 */
		u32 half_node_number;
		/**
		 * D	- @ref XSECURE_D_INTR or @ref XSECURE_D_PLEAF
		 * Size - 2 bytes, 21st & 22nd byte
		 */
		u8	D[XSECURE_LMS_D_FIELD_SIZE];
		/**
		 * Tmp - Used to store previous hash output and adjacent
		 * node value in right order to send to SHA engine
		 * Size - 32 * 2 = 64 bytes, 23 to 85 bytes
		 */
		u8	Tmp[XSECURE_LMS_M_BYTE_FIELD_SIZE + XSECURE_LMS_M_BYTE_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;
} XSecure_LmsPubKeyTmp;

/***************************** Function Prototypes ******************************************/
int XSecure_LmsLookupParamSet(XSecure_LmsType Type, XSecure_LmsParam** Parameters);

#endif /* XSECURE_LMS_H_ */
/** @} */
