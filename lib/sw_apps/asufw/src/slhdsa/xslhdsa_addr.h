/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa_addr.h
*
* This file contains SLH-DSA address (ADRS) helper macros and inline functions for ASUFW.
*
* SLH-DSA domain separation addresses are 32-byte structures used throughout WOTS+, XMSS,
* FORS, and hash-instantiation paths. This header provides encode, field-set, and utility
* helpers aligned with NIST FIPS 205 address packing rules.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xslhdsa_server_apis SLH-DSA Server APIs
* @{
*/
#ifndef XSLHDSA_ADDR_H_
#define XSLHDSA_ADDR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_util.h"

/************************************ Constant Definitions ***************************************/

#define XSLHDSA_ADDR_WORDS	(8U)	/**< Number of 32-bit words in an SLH-DSA address. */
#define XSLHDSA_ADDR_BYTES	(32U)	/**< Total address size in bytes (8 words). */
#define XSLHDSA_ADDR_WORD_BYTES \
	(XSLHDSA_ADDR_BYTES / XSLHDSA_ADDR_WORDS)	/**< Bytes per ADRS word. */
#define XSLHDSA_ADDR_W_LAYER	(0U)	/**< Word index of the hypertree layer field. */
#define XSLHDSA_ADDR_W_TREE_PAD	(1U)	/**< Word index of unused tree-address padding. */
#define XSLHDSA_ADDR_W_TREE_HI	(2U)	/**< Word index of the tree ID high 32 bits. */
#define XSLHDSA_ADDR_W_TREE_LO	(3U)	/**< Word index of the tree ID low 32 bits. */
#define XSLHDSA_ADDR_W_TYPE	(4U)	/**< Word index of the ADRS type (domain separation). */
#define XSLHDSA_ADDR_W_KEYPAIR	(5U)	/**< Word index of the keypair / OTS index field. */
#define XSLHDSA_ADDR_W_HEIGHT	(6U)	/**< Word index of the height / chain field. */
#define XSLHDSA_ADDR_W_INDEX	(7U)	/**< Word index of the tree node / hash slot index. */
#define XSLHDSA_SHA2_ADDR_FIELD_BYTES	(22U)	/**< SHA2 compact ADRS bytes for F/H/T. */

/************************************** Type Definitions *****************************************/
/** FIPS 205 ADRS type field values (word 4 domain separation). */
typedef enum {
	XSLHDSA_ADRS_WOTS_HASH = 0, /**< WOTS+ hash address type. */
	XSLHDSA_ADRS_WOTS_PK, /**< WOTS+ public-key address type. */
	XSLHDSA_ADRS_XMSS_NODE, /**< XMSS tree node address type. */
	XSLHDSA_ADRS_FORS_TREE, /**< FORS tree address type. */
	XSLHDSA_ADRS_FORS_ROOTS, /**< FORS roots address type. */
	XSLHDSA_ADRS_WOTS_NONCE, /**< WOTS+ nonce address type. */
	XSLHDSA_ADRS_FORS_NONCE /**< FORS nonce address type. */
} XSlhDsa_AdrsType;
/** SLH-DSA address as a 32-byte FIPS 205 ADRS buffer. */
typedef struct {
	u8 byte[XSLHDSA_ADDR_BYTES]; /**< Address as a contiguous byte array. */
} XSlhDsa_Adrs;

/*************************** Macros (Inline Functions) Definitions *******************************/
/*************************************************************************************************/
/**
* @brief	This function stores a big-endian word in an SLH-DSA address.
*
* @param	Addr	Pointer to the address structure.
* @param	WordIdx	Word index within the address (0..7).
* @param	Val	32-bit host-endian value to store.
*
* @note
*	Stores Val as four big-endian bytes at word WordIdx. Word indices beyond the address
*	size are rejected so the write always stays inside the 32-byte address.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrPutWord(XSlhDsa_Adrs *Addr, u32 WordIdx, u32 Val)
{
	u32 Offset;

	if (WordIdx < XSLHDSA_ADDR_WORDS) {
		Offset = WordIdx * XSLHDSA_ADDR_WORD_BYTES;
		XAsufw_I2Osp(Val, XASUFW_WORD_LEN_IN_BYTES, &Addr->byte[Offset]);
	}
}
/*************************************************************************************************/
/**
* @brief	This function sets the ADRS type field.
*
* @param	Addr	Pointer to the address structure.
* @param	Kind	FIPS 205 ADRS type value.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetType(XSlhDsa_Adrs *Addr, XSlhDsa_AdrsType Kind)
{
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_TYPE, (u32)Kind);
}
/*************************************************************************************************/
/**
* @brief	This function sets the layer field of an SLH-DSA address.
*
* @param	Addr	Pointer to the address structure.
* @param	Layer	Hypertree layer index value.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetLayer(XSlhDsa_Adrs *Addr, u32 Layer)
{
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_LAYER, Layer);
}
/*************************************************************************************************/
/**
* @brief	This function sets the tree identifier field of an SLH-DSA address.
*
* @param	Addr	Pointer to the address structure.
* @param	TreeId	64-bit hypertree identifier.
*
* @note
*	Word 1 is cleared; tree high and low words are written to words 2 and 3.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetTree(XSlhDsa_Adrs *Addr, u64 TreeId)
{
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_TREE_PAD, 0U);
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_TREE_HI,
				 (u32)(TreeId >> XASUFW_BITS_IN_WORD));
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_TREE_LO, (u32)TreeId);
}
/*************************************************************************************************/
/**
* @brief	This function sets the address type and clears trailing fields.
*
* @param	Addr	Pointer to the address structure.
* @param	Kind	Address type (domain separation) value.
*
* @note
*	Clears keypair, height, and index words after setting the type.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetTypeClearTail(XSlhDsa_Adrs *Addr,
						      XSlhDsa_AdrsType Kind)
{
	XSlhDsa_AddrSetType(Addr, Kind);
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_KEYPAIR, 0);
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_HEIGHT, 0);
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_INDEX, 0);
}
/*************************************************************************************************/
/**
* @brief	This function sets the address type and clears auxiliary fields.
*
* @param	Addr	Pointer to the address structure.
* @param	Kind	Address type (domain separation) value.
*
* @note
*	Clears height and index words after setting the type; keypair is preserved.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetTypeClearAux(XSlhDsa_Adrs *Addr,
						     XSlhDsa_AdrsType Kind)
{
	XSlhDsa_AddrSetType(Addr, Kind);
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_HEIGHT, 0);
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_INDEX, 0);
}
/*************************************************************************************************/
/**
* @brief	This function sets the keypair index field of an SLH-DSA address.
*
* @param	Addr	Pointer to the address structure.
* @param	Index	Keypair or OTS index value.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetKeyPair(XSlhDsa_Adrs *Addr, u32 Index)
{
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_KEYPAIR, Index);
}
/*************************************************************************************************/
/**
* @brief	This function sets the height field of an SLH-DSA address.
*
* @param	Addr	Pointer to the address structure.
* @param	Height	Tree height or chain height value.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetHeight(XSlhDsa_Adrs *Addr, u32 Height)
{
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_HEIGHT, Height);
}
/*************************************************************************************************/
/**
* @brief	This function sets the index field of an SLH-DSA address.
*
* @param	Addr	Pointer to the address structure.
* @param	Index	Tree node or leaf index value.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetIndex(XSlhDsa_Adrs *Addr, u32 Index)
{
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_INDEX, Index);
}
/*************************************************************************************************/
/**
* @brief	This function sets the hash slot field of an SLH-DSA address.
*
* @param	Addr	Pointer to the address structure.
* @param	Slot	WOTS+ hash slot index.
*
* @note
*	Writes Slot to the index word position per FIPS 205 WOTS+ addressing.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetHashSlot(XSlhDsa_Adrs *Addr, u32 Slot)
{
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_INDEX, Slot);
}
/*************************************************************************************************/
/**
* @brief	This function sets the chain address field of an SLH-DSA address.
*
* @param	Addr	Pointer to the address structure.
* @param	Chain	WOTS+ chain index value.
*
* @note
*	Writes Chain to the height word position per FIPS 205 WOTS+ addressing.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetChainAddr(XSlhDsa_Adrs *Addr, u32 Chain)
{
	XSlhDsa_AddrPutWord(Addr, XSLHDSA_ADDR_W_HEIGHT, Chain);
}
/*************************************************************************************************/
/**
* @brief	This function sets the address type and clears auxiliary fields.
*
* @param	Addr	Pointer to the address structure.
* @param	Kind	Address type (domain separation) value.
*
* @note
*	Alias for XSlhDsa_AddrSetTypeClearAux(); keypair is preserved.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrSetTypeClearNotKp(XSlhDsa_Adrs *Addr,
						       XSlhDsa_AdrsType Kind)
{
	XSlhDsa_AddrSetTypeClearAux(Addr, Kind);
}
/*************************************************************************************************/
/**
* @brief	This function packs an SLH-DSA address into SHA2 compact form.
*
* @param	Addr	Pointer to the source address structure.
* @param	Out	Pointer to the 22-byte compact output buffer.
*
* @note
*	Selects 22 non-padding bytes per the SLH-DSA SHA2 address compression scheme.
*
*************************************************************************************************/
static inline void XSlhDsa_AddrPackSha2Compact(const XSlhDsa_Adrs *Addr, u8 *Out)
{
	static const u8 Gather[XSLHDSA_SHA2_ADDR_FIELD_BYTES] = {
			3,  8,  9,  10, 11, 12, 13, 14, 15, 19, 20, 21,
			22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
	const u8 *Raw = Addr->byte;
	u32 Idx;

	for (Idx = 0; Idx < XSLHDSA_SHA2_ADDR_FIELD_BYTES; Idx++) {
		Out[Idx] = Raw[Gather[Idx]];
	}
}

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/


#ifdef __cplusplus
}
#endif

#endif /* XSLHDSA_ADDR_H_ */
/** @} */
