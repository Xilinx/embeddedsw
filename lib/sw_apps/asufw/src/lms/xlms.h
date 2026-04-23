/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xlms.h
*
* This file contains structures, constants and defines used in LMS and HSS
* provides interface to LMS operations for ASUFW.
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
#ifndef XLMS_H_
#define XLMS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xlms_ots.h"

#ifdef XASU_LMS_ENABLE
/************************************ Constant Definitions ***************************************/
/**
 * Max supported types, used for creating lookup table for parameters for types
 * supported number should be max valid + 1 (as first slot will be for invalid parameters)
 * XLms_Type from 0 (unsupported) to XLMS_SHAKE_M32_HEIGHT_25
 */
#define XLMS_TYPE_MAX_SUPPORTED		(21U)

/** Lookup table indices for LMS parameter sets */
#define XLMS_PARAM_IDX_RESERVED		(0U)	/**< Index for reserved/unsupported type */
#define XLMS_PARAM_IDX_SHA256_HEIGHT_5	(1U)	/**< Index for SHA256_M32_HEIGHT_5 */
#define XLMS_PARAM_IDX_SHA256_HEIGHT_10	(2U)	/**< Index for SHA256_M32_HEIGHT_10 */
#define XLMS_PARAM_IDX_SHA256_HEIGHT_15	(3U)	/**< Index for SHA256_M32_HEIGHT_15 */
#define XLMS_PARAM_IDX_SHA256_HEIGHT_20	(4U)	/**< Index for SHA256_M32_HEIGHT_20 */
#define XLMS_PARAM_IDX_SHA256_HEIGHT_25	(5U)	/**< Index for SHA256_M32_HEIGHT_25 */
#define XLMS_PARAM_IDX_SHAKE_HEIGHT_5	(6U)	/**< Index for SHAKE_M32_HEIGHT_5 */
#define XLMS_PARAM_IDX_SHAKE_HEIGHT_10	(7U)	/**< Index for SHAKE_M32_HEIGHT_10 */
#define XLMS_PARAM_IDX_SHAKE_HEIGHT_15	(8U)	/**< Index for SHAKE_M32_HEIGHT_15 */
#define XLMS_PARAM_IDX_SHAKE_HEIGHT_20	(9U)	/**< Index for SHAKE_M32_HEIGHT_20 */
#define XLMS_PARAM_IDX_SHAKE_HEIGHT_25	(10U)	/**< Index for SHAKE_M32_HEIGHT_25 */
#define XLMS_PARAM_IDX_SHA256_M24_HEIGHT_5  (11U) /**< Index for SHA256_M24_HEIGHT_5 */
#define XLMS_PARAM_IDX_SHA256_M24_HEIGHT_10 (12U) /**< Index for SHA256_M24_HEIGHT_10 */
#define XLMS_PARAM_IDX_SHA256_M24_HEIGHT_15 (13U) /**< Index for SHA256_M24_HEIGHT_15 */
#define XLMS_PARAM_IDX_SHA256_M24_HEIGHT_20 (14U) /**< Index for SHA256_M24_HEIGHT_20 */
#define XLMS_PARAM_IDX_SHA256_M24_HEIGHT_25 (15U) /**< Index for SHA256_M24_HEIGHT_25 */
#define XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_5   (16U) /**< Index for SHAKE_M24_HEIGHT_5 */
#define XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_10  (17U) /**< Index for SHAKE_M24_HEIGHT_10 */
#define XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_15  (18U) /**< Index for SHAKE_M24_HEIGHT_15 */
#define XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_20  (19U) /**< Index for SHAKE_M24_HEIGHT_20 */
#define XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_25  (20U) /**< Index for SHAKE_M24_HEIGHT_25 */

/** Domain separator constants as per RFC 8554 */
#define XLMS_D_PBLC_VALUE	(0x8080U)	/**< Domain separator for OTS public key calc */
#define XLMS_D_MESG_VALUE	(0x8181U)	/**< Domain separator for message hashing */
#define XLMS_D_LEAF_VALUE	(0x8282U)	/**< Domain separator for leaf node */
#define XLMS_D_INTR_VALUE	(0x8383U)	/**< Domain separator for internal node */
/** Domain separator byte values (both bytes are identical per RFC 8554) */
#define XLMS_D_PBLC_BYTE	(0x80U)		/**< D_PBLC byte value */
#define XLMS_D_MESG_BYTE	(0x81U)		/**< D_MESG byte value */
#define XLMS_D_LEAF_BYTE	(0x82U)		/**< D_LEAF byte value */
#define XLMS_D_INTR_BYTE	(0x83U)		/**< D_INTR byte value */

/** Hash mode value for unsupported types */
#define XLMS_HASH_MODE_UNSUPPORTED	(0xFFU)	/**< Unsupported hash mode marker */

/** Merkle tree root node number */
#define XLMS_ROOT_NODE_NUM		(1U)	/**< Root node number in Merkle tree */

/* Common lengths - they remain same irrespective of place they are used */
#define XLMS_TYPE_SIZE		(4U)	/**< Length of type field */
#define XLMS_M_BYTE_FIELD_SIZE	(32U)	/**< Length of M byte field */

/* Public key related sizes */
#define XLMS_PUB_KEY_FIXED_FIELD_SIZE		(XLMS_TYPE_SIZE + \
							XLMS_OTS_TYPE_SIZE + \
							XLMS_I_FIELD_SIZE)
							/**< Length of fixed part of public key */
#define XLMS_PUB_KEY_T_FIELD_SIZE		(XLMS_M_BYTE_FIELD_SIZE)
							/**< Length of T field of public key */
#define XLMS_PUB_KEY_TOTAL_SIZE			(XLMS_PUB_KEY_FIXED_FIELD_SIZE + \
							XLMS_PUB_KEY_T_FIELD_SIZE)
							/**< Length of total public key */

/* Sizes related to tmp buffer used to calculate root public key from leaf */
#define	XLMS_MERKLE_TREE_NODE_NUMBER_SIZE	(XLMS_Q_FIELD_SIZE)
						/**< Length of Merkle tree node number */
#define	XLMS_D_LEAF_FIELD_SIZE			(2U)
						/**< Length of D leaf field */
#define	XLMS_D_INTR_FIELD_SIZE			(2U)
						/**< Length of D internal field */
#define	XLMS_D_FIELD_SIZE			(2U)	/**< Length of D field */

/** Used for internal nodes processing */
#define XLMS_PUB_KEY_TMP_BUFFER_INTR_TOTAL_SIZE	(XLMS_I_FIELD_SIZE + \
							XLMS_MERKLE_TREE_NODE_NUMBER_SIZE + \
							XLMS_D_INTR_FIELD_SIZE + \
							XLMS_M_BYTE_FIELD_SIZE + \
							XLMS_M_BYTE_FIELD_SIZE)

/* Offsets */

/* LMS public key related offsets */
#define XLMS_PUB_KEY_TYPE_OFFSET	(0U)	/**< Offset of LmsType field in public key */
#define XLMS_PUB_KEY_OTS_TYPE_OFFSET	(XLMS_PUB_KEY_TYPE_OFFSET + \
					XLMS_TYPE_SIZE)	/**< Offset of OtsType field in public key */
#define XLMS_PUB_KEY_I_OFFSET		(XLMS_PUB_KEY_OTS_TYPE_OFFSET + \
					XLMS_OTS_TYPE_SIZE)	/**< Offset of I field in public key */
#define XLMS_PUB_KEY_T_OFFSET		(XLMS_PUB_KEY_I_OFFSET + \
					XLMS_I_FIELD_SIZE)	/**< Offset of T field in public key */

/* LMS Signature offsets */
#define XLMS_SIGNATURE_Q_FIELD_OFFSET		(0U)	/**< Offset of Q field in LMS Signature */
#define XLMS_SIGNATURE_OTS_FIELD_OFFSET		(XLMS_SIGNATURE_Q_FIELD_OFFSET + \
							XLMS_Q_FIELD_SIZE)	/**< Offset of OTS field in LMS Signature */

/************************************** Type Definitions *****************************************/
/**
 * @brief Supported LMS Types from standard
 */
typedef enum {
	XLMS_RESERVED = 0x00000000U,		/**< Reserved */
	XLMS_SHA256_M32_HEIGHT_5 = 0x00000005U,
					/**< 'M' = 32, 'H' =  SHA2-256, 'h' = 5 */
	XLMS_SHA256_M32_HEIGHT_10 = 0x00000006U,
					/**< 'M' = 32, 'H' =  SHA2-256, 'h' = 10 */
	XLMS_SHA256_M32_HEIGHT_15 = 0x00000007U,
					/**< 'M' = 32, 'H' =  SHA2-256, 'h' = 15 */
	XLMS_SHA256_M32_HEIGHT_20 = 0x00000008U,
					/**< 'M' = 32, 'H' =  SHA2-256, 'h' = 20 */
	XLMS_SHA256_M32_HEIGHT_25 = 0x00000009U,
					/**< 'M' = 32, 'H' =  SHA2-256, 'h' = 25 */
	XLMS_SHA256_M24_HEIGHT_5 = 0x0000000AU,
				/**< 'M' = 24, 'H' =  SHA2-256, 'h' = 5 */
	XLMS_SHA256_M24_HEIGHT_10 = 0x0000000BU,
				/**< 'M' = 24, 'H' =  SHA2-256, 'h' = 10 */
	XLMS_SHA256_M24_HEIGHT_15 = 0x0000000CU,
				/**< 'M' = 24, 'H' =  SHA2-256, 'h' = 15 */
	XLMS_SHA256_M24_HEIGHT_20 = 0x0000000DU,
				/**< 'M' = 24, 'H' =  SHA2-256, 'h' = 20 */
	XLMS_SHA256_M24_HEIGHT_25 = 0x0000000EU,
				/**< 'M' = 24, 'H' =  SHA2-256, 'h' = 25 */
	XLMS_SHAKE_M32_HEIGHT_5 = 0x0000000FU,
				/**< 'M' = 32, 'H' =  SHAKE-256, 'h' = 5 */
	XLMS_SHAKE_M32_HEIGHT_10 = 0x00000010U,
				/**< 'M' = 32, 'H' =  SHAKE-256, 'h' = 10 */
	XLMS_SHAKE_M32_HEIGHT_15 = 0x00000011U,
				/**< 'M' = 32, 'H' =  SHAKE-256, 'h' = 15 */
	XLMS_SHAKE_M32_HEIGHT_20 = 0x00000012U,
				/**< 'M' = 32, 'H' =  SHAKE-256, 'h' = 20 */
	XLMS_SHAKE_M32_HEIGHT_25 = 0x00000013U,
				/**< 'M' = 32, 'H' =  SHAKE-256, 'h' = 25 */
	XLMS_SHAKE_M24_HEIGHT_5 = 0x00000014U,
				/**< 'M' = 24, 'H' =  SHAKE-256, 'h' = 5 */
	XLMS_SHAKE_M24_HEIGHT_10 = 0x00000015U,
				/**< 'M' = 24, 'H' =  SHAKE-256, 'h' = 10 */
	XLMS_SHAKE_M24_HEIGHT_15 = 0x00000016U,
				/**< 'M' = 24, 'H' =  SHAKE-256, 'h' = 15 */
	XLMS_SHAKE_M24_HEIGHT_20 = 0x00000017U,
				/**< 'M' = 24, 'H' =  SHAKE-256, 'h' = 20 */
	XLMS_SHAKE_M24_HEIGHT_25 = 0x00000018U,
				/**< 'M' = 24, 'H' =  SHAKE-256, 'h' = 25 */
	XLMS_NOT_SUPPORTED	/**< Not Supported */
} XLms_Type;

/** Enumeration for supported LMS tree heights. */
typedef enum {
	XLMS_TREE_HEIGHT_UNSUPPORTED = 0U,	/**< Unsupported tree height */
	XLMS_TREE_HEIGHT_5 = 5U,	/**< Supported tree height 5*/
	XLMS_TREE_HEIGHT_10 = 10U,	/**< Supported tree height 10*/
	XLMS_TREE_HEIGHT_15 = 15U,	/**< Supported tree height 15*/
	XLMS_TREE_HEIGHT_20 = 20U,	/**< Supported tree height 20*/
	XLMS_TREE_HEIGHT_25 = 25U	/**< Supported tree height 25*/
} XLms_TreeHeight;

/**
 * @brief Structure for maintaining parameters for each LMS Type
 */
typedef struct {
	u32 HashAlgId; /**< HASH function for selected type */
	u32 HashOutputBytes;	/**< The number of bytes of the output of the hash function, same as 'n' in LMS OTS */
	XLms_TreeHeight TreeHeight; /**< Height of Merkle tree */
	u32 SignatureLenBytes;	/**< Product of HashOutputBytes and TreeHeight, used in signature length determination */
} XLms_Param;

/**
 * @brief LMS Public key structure to access root value
 *
 * Size = 4 + 4 + 16 + H Len (32) = 56 Bytes
 */
typedef union {
	u8 Buff[XLMS_PUB_KEY_TOTAL_SIZE]; /**< Raw buffer for LMS public key. */
	struct {
		/**
		 * @brief LmsType XLms_Type
		 * Size - 4 bytes, 0 to 3 in public key
		 */
		XLms_Type LmsType;
		/**
		 * @brief OtsType XLms_OtsType
		 * Size - 4 bytes, 4 to 7 bytes in public key
		 */
		XLms_OtsType OtsType;
		/**
		 * @brief PubKeyTreeId - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 8 to 23 bytes in public key
		 */
		u8 PubKeyTreeId[XLMS_I_FIELD_SIZE];
		/**
		 * @brief T[1] - H(I||u32str(r)||u16str(D_INTR)||T[2*r]||T[2*r+1])
		 *
		 * 'H' is a hash function, supports only SHA2-256 and SHAKE-256
		 * both are of 32Byte output len
		 *
		 * 'r' is the node number, same as 'q', goes from 0 on left most
		 * leaf to right most (2^h -1), in a single tree
		 * D_INTR is a constant XLMS_D_INTR
		 */
		u8 T[XLMS_PUB_KEY_T_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;	/**< Fields of the public key structure */
} XLms_PublicKey;

/**
 * @brief Used to store data which is repeatedly sent to SHA engine,
 * during LMS root value calculation
 *
 * Size = 16(I) + 4(node_number/2) + 2(D_INTR) + (32 * 2)
 */
typedef union {
	u8 Buff[XLMS_PUB_KEY_TMP_BUFFER_INTR_TOTAL_SIZE]; /**< Raw buffer for intermediate public key data. */
	struct {
		/**
		 * MerkleTreeId - Merkle Tree's unique identifier (uniformly random 16-byte string)
		 * Size - 16 bytes, 0 to 15 bytes
		 */
		u8 MerkleTreeId[XLMS_I_FIELD_SIZE];
		/**
		 * (node number/2), used to get unique hash o/p for same data for every node
		 * Size - 4 bytes, 16 to 20 bytes
		 */
		u32 half_node_number;
		/**
		 * D - XLMS_D_INTR or XLMS_D_PLEAF
		 * Size - 2 bytes, 21st & 22nd byte
		 */
		u8 D[XLMS_D_FIELD_SIZE];
		/**
		 * Tmp - Used to store previous hash output and adjacent
		 * node value in right order to send to SHA engine
		 * Size - 32 * 2 = 64 bytes, 23 to 85 bytes
		 */
		u8 Tmp[XLMS_M_BYTE_FIELD_SIZE + XLMS_M_BYTE_FIELD_SIZE];
	}__attribute__((__packed__)) Fields;	/**< Fields of chain temporary buffer structure */
} XLms_PubKeyTmp;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XLms_LookupParamSet(XLms_Type Type, const XLms_Param** Parameters);
#endif /* XASU_LMS_ENABLE */

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XLMS_H_ */
/** @} */
