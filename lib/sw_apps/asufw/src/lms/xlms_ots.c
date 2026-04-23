/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xlms_ots.c
*
* This file consists definitions for LMS OTS operations for ASUFW.
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

/*************************************** Include Files *******************************************/
/**
* @addtogroup xlms_server_apis LMS Server APIs
* @{
*/
#include "xil_io.h"
#include "xlms_ots.h"
#include "xlms.h"
#include "xasufw_status.h"
#include "xasu_shainfo.h"
#include "xasufw_util.h"

#ifdef XASU_LMS_ENABLE

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XLMS_BYTE_QUO(i) 		((i)/8U) /**< To Pick the byte in array */
#define XLMS_BYTE_REM(i)  		((i)%8U) /**< Inside the byte interested
						value has to be right shifted */
#define XLMS_BIT_MASK(i)		((1U << (i)) - 1U) /**< Mask out rest of bits,
					only take w width back to caller invocations
					to get signature is equal to number of w width
					digits in a digest to get that,
					(2^w - 1), example w=8, Mask = 256 */
#define	XLMS_BITS_PER_BYTE		(8U)	/**< Bits per byte */
#define XLMS_LOWER_16_BIT_MASK	(0x0000FFFFU)	/**< Lower 16 bit mask */

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
/**
 * @brief Global lookup table for 'n', 'w', 'p', 'ls' & Signature lengths
 */
static const XLms_OtsParam XLms_OtsParamLookup[XLMS_OTS_TYPE_MAX_SUPPORTED] = {
	/* XLMS_OTS_RESERVED, XLMS_OTS_NOT_SUPPORTED, default*/
	[XLMS_OTS_PARAM_IDX_RESERVED] = {
		.HashAlgId = XLMS_HASH_MODE_UNSUPPORTED,
		.HashOutputBytes = 0U,
		.w = XLMS_OTS_W_NOT_SUPPORTED,
		.u = XLMS_OTS_U_NOT_SUPPORTED,
		.v = XLMS_OTS_V_NOT_SUPPORTED,
		.ls = XLMS_OTS_LS_NOT_SUPPORTED,
		.p = XLMS_OTS_P_NOT_SUPPORTED,
		.NoOfInvSign = 0U,
		.SignLen = 0U
	},
	/* XLMS_OTS_SHA256_N32_W2 */
	[XLMS_OTS_PARAM_IDX_SHA256_W2] = {
		.HashAlgId = XASU_SHA_MODE_256,
		.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
		.w = XLMS_OTS_W2,
		.u = XLMS_OTS_W2_U,
		.v = XLMS_OTS_W2_V,
		.ls = XLMS_OTS_W2_LS,
		.p = XLMS_OTS_W2_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W2,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N_FIELD_SIZE * ((u32)XLMS_OTS_W2_P + 1U))) /* 4292U */
	},
	/* XLMS_OTS_SHA256_N32_W4 */
	[XLMS_OTS_PARAM_IDX_SHA256_W4] = {
		.HashAlgId = XASU_SHA_MODE_256,
		.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
		.w = XLMS_OTS_W4,
		.u = XLMS_OTS_W4_U,
		.v = XLMS_OTS_W4_V,
		.ls = XLMS_OTS_W4_LS,
		.p = XLMS_OTS_W4_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W4,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N_FIELD_SIZE * ((u32)XLMS_OTS_W4_P + 1U))) /* 2180U */
	},
	/* XLMS_OTS_SHA256_N32_W8 */
	[XLMS_OTS_PARAM_IDX_SHA256_W8] = {
		.HashAlgId = XASU_SHA_MODE_256,
		.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
		.w = XLMS_OTS_W8,
		.u = XLMS_OTS_W8_U,
		.v = XLMS_OTS_W8_V,
		.ls = XLMS_OTS_W8_LS,
		.p = XLMS_OTS_W8_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W8,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N_FIELD_SIZE * ((u32)XLMS_OTS_W8_P + 1U))) /* 1124U */
	},
	/* XLMS_OTS_SHAKE_N32_W2 */
	[XLMS_OTS_PARAM_IDX_SHAKE_W2] = {
		.HashAlgId = XASU_SHA_MODE_SHAKE256,
		.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
		.w = XLMS_OTS_W2,
		.u = XLMS_OTS_W2_U,
		.v = XLMS_OTS_W2_V,
		.ls = XLMS_OTS_W2_LS,
		.p = XLMS_OTS_W2_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W2,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N_FIELD_SIZE * ((u32)XLMS_OTS_W2_P + 1U))) /* 4292U */
	},
	/* XLMS_OTS_SHAKE_N32_W4 */
	[XLMS_OTS_PARAM_IDX_SHAKE_W4] = {
		.HashAlgId = XASU_SHA_MODE_SHAKE256,
		.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
		.w = XLMS_OTS_W4,
		.u = XLMS_OTS_W4_U,
		.v = XLMS_OTS_W4_V,
		.ls = XLMS_OTS_W4_LS,
		.p = XLMS_OTS_W4_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W4,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N_FIELD_SIZE * ((u32)XLMS_OTS_W4_P + 1U))) /* 2180U */
	},
	/* XLMS_OTS_SHAKE_N32_W8 */
	[XLMS_OTS_PARAM_IDX_SHAKE_W8] = {
		.HashAlgId = XASU_SHA_MODE_SHAKE256,
		.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
		.w = XLMS_OTS_W8,
		.u = XLMS_OTS_W8_U,
		.v = XLMS_OTS_W8_V,
		.ls = XLMS_OTS_W8_LS,
		.p = XLMS_OTS_W8_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W8,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N_FIELD_SIZE * ((u32)XLMS_OTS_W8_P + 1U))) /* 1124U */
	},
	/* XLMS_OTS_SHA256_N32_W1 */
	[XLMS_OTS_PARAM_IDX_SHA256_W1] = {
		.HashAlgId = XASU_SHA_MODE_256,
		.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
		.w = XLMS_OTS_W1,
		.u = XLMS_OTS_W1_U,
		.v = XLMS_OTS_W1_V,
		.ls = XLMS_OTS_W1_LS,
		.p = XLMS_OTS_W1_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W1,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N_FIELD_SIZE * ((u32)XLMS_OTS_W1_P + 1U))) /* 8516U */
	},
	/* XLMS_OTS_SHAKE_N32_W1 */
	[XLMS_OTS_PARAM_IDX_SHAKE_W1] = {
		.HashAlgId = XASU_SHA_MODE_SHAKE256,
		.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
		.w = XLMS_OTS_W1,
		.u = XLMS_OTS_W1_U,
		.v = XLMS_OTS_W1_V,
		.ls = XLMS_OTS_W1_LS,
		.p = XLMS_OTS_W1_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W1,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N_FIELD_SIZE * ((u32)XLMS_OTS_W1_P + 1U))) /* 8516U */
	},
	/* XLMS_OTS_SHA256_N24_W1 */
	[XLMS_OTS_PARAM_IDX_SHA256_N24_W1] = {
		.HashAlgId = XASU_SHA_MODE_256,
		.HashOutputBytes = XLMS_N24_FIELD_SIZE,
		.w = XLMS_OTS_W1,
		.u = XLMS_OTS_N24_W1_U,
		.v = XLMS_OTS_N24_W1_V,
		.ls = XLMS_OTS_N24_W1_LS,
		.p = XLMS_OTS_N24_W1_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W1,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N24_FIELD_SIZE * ((u32)XLMS_OTS_N24_W1_P + 1U))) /* 4828U */
	},
	/* XLMS_OTS_SHA256_N24_W2 */
	[XLMS_OTS_PARAM_IDX_SHA256_N24_W2] = {
		.HashAlgId = XASU_SHA_MODE_256,
		.HashOutputBytes = XLMS_N24_FIELD_SIZE,
		.w = XLMS_OTS_W2,
		.u = XLMS_OTS_N24_W2_U,
		.v = XLMS_OTS_N24_W2_V,
		.ls = XLMS_OTS_N24_W2_LS,
		.p = XLMS_OTS_N24_W2_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W2,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N24_FIELD_SIZE * ((u32)XLMS_OTS_N24_W2_P + 1U))) /* 2452U */
	},
	/* XLMS_OTS_SHA256_N24_W4 */
	[XLMS_OTS_PARAM_IDX_SHA256_N24_W4] = {
		.HashAlgId = XASU_SHA_MODE_256,
		.HashOutputBytes = XLMS_N24_FIELD_SIZE,
		.w = XLMS_OTS_W4,
		.u = XLMS_OTS_N24_W4_U,
		.v = XLMS_OTS_N24_W4_V,
		.ls = XLMS_OTS_N24_W4_LS,
		.p = XLMS_OTS_N24_W4_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W4,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N24_FIELD_SIZE * ((u32)XLMS_OTS_N24_W4_P + 1U))) /* 1252U */
	},
	/* XLMS_OTS_SHA256_N24_W8 */
	[XLMS_OTS_PARAM_IDX_SHA256_N24_W8] = {
		.HashAlgId = XASU_SHA_MODE_256,
		.HashOutputBytes = XLMS_N24_FIELD_SIZE,
		.w = XLMS_OTS_W8,
		.u = XLMS_OTS_N24_W8_U,
		.v = XLMS_OTS_N24_W8_V,
		.ls = XLMS_OTS_N24_W8_LS,
		.p = XLMS_OTS_N24_W8_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W8,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N24_FIELD_SIZE * ((u32)XLMS_OTS_N24_W8_P + 1U))) /* 652U */
	},
	/* XLMS_OTS_SHAKE_N24_W1 */
	[XLMS_OTS_PARAM_IDX_SHAKE_N24_W1] = {
		.HashAlgId = XASU_SHA_MODE_SHAKE256,
		.HashOutputBytes = XLMS_N24_FIELD_SIZE,
		.w = XLMS_OTS_W1,
		.u = XLMS_OTS_N24_W1_U,
		.v = XLMS_OTS_N24_W1_V,
		.ls = XLMS_OTS_N24_W1_LS,
		.p = XLMS_OTS_N24_W1_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W1,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N24_FIELD_SIZE * ((u32)XLMS_OTS_N24_W1_P + 1U))) /* 4828U */
	},
	/* XLMS_OTS_SHAKE_N24_W2 */
	[XLMS_OTS_PARAM_IDX_SHAKE_N24_W2] = {
		.HashAlgId = XASU_SHA_MODE_SHAKE256,
		.HashOutputBytes = XLMS_N24_FIELD_SIZE,
		.w = XLMS_OTS_W2,
		.u = XLMS_OTS_N24_W2_U,
		.v = XLMS_OTS_N24_W2_V,
		.ls = XLMS_OTS_N24_W2_LS,
		.p = XLMS_OTS_N24_W2_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W2,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N24_FIELD_SIZE * ((u32)XLMS_OTS_N24_W2_P + 1U))) /* 2452U */
	},
	/* XLMS_OTS_SHAKE_N24_W4 */
	[XLMS_OTS_PARAM_IDX_SHAKE_N24_W4] = {
		.HashAlgId = XASU_SHA_MODE_SHAKE256,
		.HashOutputBytes = XLMS_N24_FIELD_SIZE,
		.w = XLMS_OTS_W4,
		.u = XLMS_OTS_N24_W4_U,
		.v = XLMS_OTS_N24_W4_V,
		.ls = XLMS_OTS_N24_W4_LS,
		.p = XLMS_OTS_N24_W4_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W4,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N24_FIELD_SIZE * ((u32)XLMS_OTS_N24_W4_P + 1U))) /* 1252U */
	},
	/* XLMS_OTS_SHAKE_N24_W8 */
	[XLMS_OTS_PARAM_IDX_SHAKE_N24_W8] = {
		.HashAlgId = XASU_SHA_MODE_SHAKE256,
		.HashOutputBytes = XLMS_N24_FIELD_SIZE,
		.w = XLMS_OTS_W8,
		.u = XLMS_OTS_N24_W8_U,
		.v = XLMS_OTS_N24_W8_V,
		.ls = XLMS_OTS_N24_W8_LS,
		.p = XLMS_OTS_N24_W8_P,
		.NoOfInvSign = XLMS_OTS_NO_OF_INV_SIGN_W8,
		.SignLen = (XLMS_OTS_TYPE_SIZE + (XLMS_N24_FIELD_SIZE * ((u32)XLMS_OTS_N24_W8_P + 1U))) /* 652U */
	}
};

/*************************************************************************************************/
/**
 * @brief	This function extracts the digits from the array for bytes
 *
 * @note
 *		S (represented in bits) = 0x1234
 *	[0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0]
 *				 ^
 *				  |
 *			coef(S, 7, 1)
 *
 *	[1,			2,			3,			4]
 *				 ^
 *				 |
 * 			coef(S, 0, 4)
 *
 * If i is larger than the number of w-bit values in S, then coef(S, i, w) is undefined,
 * this can't be checked here
 *
 * @param	Arr	Byte array from where the digit needs to be picked
 * @param	ArrayIndex	When array is divided into array of digits,
 * 				i represents the index of digit
 * @param	w	Width in bits, for each digit allowed values
 *
 * @return	-	digit value.
 *
 *************************************************************************************************/
u32 XLms_OtsCoeff(u8 const* const Arr, const u32 ArrayIndex, const u32 w)
{

	/**
	* (2^w - 1) AND ( byte(S, floor(i * w / 8)) >> (8 - (w * (i % (8 / w)) + w)) )
	* mask & (w width value in byte >> to lower most bits) to extract the digit,
	*/

	u32 Result = 0U;

	u32 Product = ArrayIndex * w;

	Result = Arr[XLMS_BYTE_QUO(Product)] >> (XLMS_BITS_PER_BYTE - w - XLMS_BYTE_REM(Product));

	Result &= XLMS_BIT_MASK(w);

	return Result;
}

/*************************************************************************************************/
/**
* @brief	This function calculates the checksum for a given array
*
* @param	Array	Byte array for which checksum needs to be calculated
* @param	ArrayLen	Length of input array
* @param	w	Width in bits, for each digit allowed values
* @param	ls	Number of bits to left-shift the calculated checksum
* @param	Checksum Pointer to address, where computed checksum needs to be copied
*
* @return
*	-	XASUFW_SUCCESS - Valid type is passed, and parameters are assigned
*	-	XASUFW_LMS_OTS_CHECKSUM_BUFF_INVALID_LEN_ERROR - If buffer length is 0
*
**************************************************************************************************/
s32 XLms_OtsComputeChecksum(const u8* const Array, const u32 ArrayLen, const u32 w, const u32 ls,
				u32* const Checksum)
{

	s32 Status = XASUFW_FAILURE;
	u32 Sum = 0x0U;
	u32 MaxDigitValue = XLMS_BIT_MASK(w);
	u32 MaxIndex = 0U;
	u32 Index;

	if (0U == ArrayLen) {
		Status = XASUFW_LMS_OTS_CHECKSUM_BUFF_INVALID_LEN_ERROR;
		*Checksum = 0x0;
		goto END;
	}

	MaxIndex = (ArrayLen * XLMS_BITS_PER_BYTE)/w;

	for (Index = 0U; Index < MaxIndex; Index++) {
		Sum += (MaxDigitValue - XLms_OtsCoeff(Array, Index, w));
	}

	/* If loop counter is glitched, the signature verification fails */

	*Checksum = (u32)(XLMS_LOWER_16_BIT_MASK & (Sum << ls));
	Status = XASUFW_SUCCESS;
END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function returns the parameters for the type of LMS OTS type
*
* @param	Type 	XLms_OtsType, type of OTS algorithm selected
* @param	Parameters Pointer to array location where all predefined
* 		parameter values are present @ref XLms_OtsParam
*
* @return
*	-	XASUFW_SUCCESS - Valid type is passed, and parameters are assigned
*	-	XASUFW_LMS_OTS_TYPE_UNSUPPORTED_ERROR - If not a valid type is passed
*	-	XASUFW_LMS_TYPE_LOOKUP_GLITCH_ERROR - If glitch detected
*
**************************************************************************************************/
s32 XLms_OtsLookupParamSet(XLms_OtsType Type, const XLms_OtsParam** Parameters)
{
	s32 Status = XASUFW_FAILURE;
	/* Redundant type for temporal check */
	volatile XLms_OtsType TmpType = (XLms_OtsType)XASUFW_ALLFS;
	*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_RESERVED];

	switch (Type) {
		/* SHA-256 options */
		case XLMS_OTS_SHA256_N32_W1: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHA256_W1];
			TmpType = XLMS_OTS_SHA256_N32_W1;
			break;
		}
		case XLMS_OTS_SHA256_N32_W2: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHA256_W2];
			TmpType = XLMS_OTS_SHA256_N32_W2;
			break;
		}
		case XLMS_OTS_SHA256_N32_W4: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHA256_W4];
			TmpType = XLMS_OTS_SHA256_N32_W4;
			break;
		}
		case XLMS_OTS_SHA256_N32_W8: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHA256_W8];
			TmpType = XLMS_OTS_SHA256_N32_W8;
			break;
		}

		/* SHAKE 256 options */
		case XLMS_OTS_SHAKE_N32_W1: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHAKE_W1];
			TmpType = XLMS_OTS_SHAKE_N32_W1;
			break;
		}
		case XLMS_OTS_SHAKE_N32_W2: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHAKE_W2];
			TmpType = XLMS_OTS_SHAKE_N32_W2;
			break;
		}
		case XLMS_OTS_SHAKE_N32_W4: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHAKE_W4];
			TmpType = XLMS_OTS_SHAKE_N32_W4;
			break;
		}
		case XLMS_OTS_SHAKE_N32_W8: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHAKE_W8];
			TmpType = XLMS_OTS_SHAKE_N32_W8;
			break;
		}

		/* default, and other not supported options */
		case XLMS_OTS_RESERVED: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_RESERVED];
			TmpType = XLMS_OTS_RESERVED;
			Status = XASUFW_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
		case XLMS_OTS_NOT_SUPPORTED: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_RESERVED];
			TmpType = XLMS_OTS_NOT_SUPPORTED;
			Status = XASUFW_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
		/* N=24 variants defined in NIST SP 800-208 */
		case XLMS_OTS_SHA256_N24_W1: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHA256_N24_W1];
			TmpType = XLMS_OTS_SHA256_N24_W1;
			break;
		}
		case XLMS_OTS_SHA256_N24_W2: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHA256_N24_W2];
			TmpType = XLMS_OTS_SHA256_N24_W2;
			break;
		}
		case XLMS_OTS_SHA256_N24_W4: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHA256_N24_W4];
			TmpType = XLMS_OTS_SHA256_N24_W4;
			break;
		}
		case XLMS_OTS_SHA256_N24_W8: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHA256_N24_W8];
			TmpType = XLMS_OTS_SHA256_N24_W8;
			break;
		}
		case XLMS_OTS_SHAKE_N24_W1: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHAKE_N24_W1];
			TmpType = XLMS_OTS_SHAKE_N24_W1;
			break;
		}
		case XLMS_OTS_SHAKE_N24_W2: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHAKE_N24_W2];
			TmpType = XLMS_OTS_SHAKE_N24_W2;
			break;
		}
		case XLMS_OTS_SHAKE_N24_W4: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHAKE_N24_W4];
			TmpType = XLMS_OTS_SHAKE_N24_W4;
			break;
		}
		case XLMS_OTS_SHAKE_N24_W8: {
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_SHAKE_N24_W8];
			TmpType = XLMS_OTS_SHAKE_N24_W8;
			break;
		}
		default: {
		/* default, and other not supported options */
			*Parameters = (const XLms_OtsParam*)&XLms_OtsParamLookup[XLMS_OTS_PARAM_IDX_RESERVED];
			Status = XASUFW_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
	}

	/* Two possible glitches, replacing this condition with a NOP, we will update error and exit
		flipping condition, we update error and exit, so both cases it is OK. */
	if(TmpType != Type) {
		/* Update register to be taken care by caller */
		Status = XASUFW_LMS_TYPE_LOOKUP_GLITCH_ERROR;
		goto END;
	}

	Status = XASUFW_SUCCESS;
END:
	return Status;
}
#endif /* XASU_LMS_ENABLE */
/** @} */
