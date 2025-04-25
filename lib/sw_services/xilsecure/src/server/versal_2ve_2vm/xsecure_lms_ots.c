/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/******************************************************************************/
/**
*
* @file xsecure_lms_ots.c
*
* This file consists definitions for LMS OTS operations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- ----------------------------------------------------------
* 5.4   kal  07/24/24  Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "xsecure_sha.h"
#include "xil_io.h"
#include "xsecure_lms_ots.h"

/************************** Constant Definitions ******************************/

/**************************** Type Definitions ********************************/

/***************** Macros (Inline Functions) Definitions **********************/
#define XSECURE_BYTE_QUO(i) 		((i)/8U) /** To Pick the byte in array */
#define XSECURE_BYTE_REM(i)  		((i)%8U) /** Inside the byte interested
						value has to be right shifted */
#define XSECURE_BIT_MASK(i)		((1U << i) - 1U) /** Mask out rest of bits,
					only take w width back to caller invocations
					to get signature is equal to number of w width
					digits in a digest to get that,
					(2^w - 1), example w=8, Mask = 256 */
#define	XSECURE_BITS_PER_BYTE		(8U)
#define XSECURE_LOWER_16_BIT_MASK	(0x0000FFFFU)

/************************** Function Prototypes *******************************/

/************************** Variable Definitions ******************************/
/******************************************************************************/
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
 * this cant be checked here
 *
 * @param	Array - Byte array from where the digit needs to be picked
 * @param	ArrayIndex - When array is divided into array of digits,
 * 				i represents the index of digit
 * @param[	w	- Width in bits, for each digit allowed values
 *
 * @return	-	digit value.
*******************************************************************************/
u32 XSecure_LmsOtsCoeff(u8 const* const Arr, const u32 ArrayIndex, const u32 w)
{

	/*
	 * (2^w - 1) AND ( byte(S, floor(i * w / 8)) >> (8 - (w * (i % (8 / w)) + w)) )
	 * mask	  &  (w width value in byte >> to lower most bits)
	 * to extract the digit,
	 */

	u32 Result;

	u32 Product = ArrayIndex * w;

	Result = Arr[XSECURE_BYTE_QUO(Product)] >> (XSECURE_BITS_PER_BYTE - w - XSECURE_BYTE_REM(Product));

	Result &= XSECURE_BIT_MASK(w);

	return Result;
}

/******************************************************************************/
/**
* @brief	This function calculates the checksum for a given array
*
* @param	Array 	- Byte array ffor which checksum needs to be calculated
* @param	ArrayLen- Length of input array
* @param	w	- Width in bits, for each digit allowed values
* @param	ls	- Number of bits to left-shift the calculated checksum
* @param	Checksum- Pointer to address, where computed checksum needs to be copied
*
* @return
*	-	@ref XST_SUCCESS - Valid type is passed, and parameters are assigned
*	-	@ref XST_FAILURE - If not a valid type is passed
*******************************************************************************/
int XSecure_LmsOtsComputeChecksum(const u8* const Array,
				const u32 ArrayLen,
				const u32 w,
				const u32 ls,
				u32* const Checksum)
{

	int Status = XST_FAILURE;
	u32 Sum = 0x0U;
	u32 MaxDigitValue = XSECURE_BIT_MASK(w);
	u32 MaxIndex = 0U;
	u32 Index;

	if (0U == ArrayLen) {
		Status = (u32)XSECURE_LMS_OTS_CHECKSUM_BUFF_INVALID_LEN_ERROR;
		*Checksum = 0x0;
		goto END;
	}

	MaxIndex = (ArrayLen * XSECURE_BITS_PER_BYTE)/w;

	for (Index = 0U; Index < MaxIndex; Index++) {
		Sum += (MaxDigitValue - XSecure_LmsOtsCoeff(Array, Index, w));
	}

	/* If loop counter is glitched, the signature verification fails */

	*Checksum = (u32)(XSECURE_LOWER_16_BIT_MASK & (Sum << ls));
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
* @brief	This function returns the parameters for the type of LMS OTS type
*
* @param	Type 	XSecure_LmsOtsType, type of OTS algorithm selected
* @param	Parameters Pointer to array location where all predefined
* 		parameter values are present @ref XSecure_LmsOtsParam
*
* @return
*	-	@ref XST_SUCCESS - Valid type is passed, and parameters are assigned
*	-	@ref XST_FAILURE - If not a valid type is passed
*******************************************************************************/
int XSecure_LmsOtsLookupParamSet(XSecure_LmsOtsType Type,
	XSecure_LmsOtsParam** Parameters)
{
	int Status = XST_FAILURE;

	/**
	 * @brief lookup table for 'n', 'w', 'p', 'ls' & Signature lengths
	 */
	static XSecure_LmsOtsParam XSecure_LmsOtsParamLookup[XSECURE_LMS_OTS_TYPE_MAX_SUPPORTED] = {
		/* XSECURE_LMS_OTS_RESERVED, XSECURE_LMS_OTS_NOT_SUPPORTED, default*/
		[0U] = {
			.H = XSECURE_SHA_INVALID_MODE,
			.n = 0U,
			.w = XSECURE_LMS_OTS_W_NOT_SUPPORTED,
			.u = XSECURE_LMS_OTS_U_NOT_SUPPORTED,
			.v = XSECURE_LMS_OTS_V_NOT_SUPPORTED,
			.ls = XSECURE_LMS_OTS_LS_NOT_SUPPORTED,
			.p = XSECURE_LMS_OTS_P_NOT_SUPPORTED,
			.NoOfInvSign = 0U,
			.SignLen = 0U
		},
		/* XSECURE_LMS_OTS_SHA256_N32_W2 */
		[1U] = {
			.H = XSECURE_SHA2_256,
			.n = XSECURE_SHA2_256_HASH_LEN,
			.w = XSECURE_LMS_OTS_W2,
			.u = XSECURE_LMS_OTS_W2_U,
			.v = XSECURE_LMS_OTS_W2_V,
			.ls = XSECURE_LMS_OTS_W2_LS,
			.p = XSECURE_LMS_OTS_W2_P,
			.NoOfInvSign = 3U,
			.SignLen = (XSECURE_LMS_OTS_TYPE_SIZE + (XSECURE_LMS_N_FIELD_SIZE * ((u32)XSECURE_LMS_OTS_W2_P + 1U)))	/* 4292U */
		},
		/* XSECURE_LMS_OTS_SHA256_N32_W4 */
		[2U] = {
			.H = XSECURE_SHA2_256,
			.n = XSECURE_SHA2_256_HASH_LEN,
			.w = XSECURE_LMS_OTS_W4,
			.u = XSECURE_LMS_OTS_W4_U,
			.v = XSECURE_LMS_OTS_W4_V,
			.ls = XSECURE_LMS_OTS_W4_LS,
			.p = XSECURE_LMS_OTS_W4_P,
			.NoOfInvSign = 15U,
			.SignLen = (XSECURE_LMS_OTS_TYPE_SIZE + (XSECURE_LMS_N_FIELD_SIZE * ((u32)XSECURE_LMS_OTS_W4_P + 1U)))	/* 2180U */
		},
		/* XSECURE_LMS_OTS_SHA256_N32_W8 */
		[3U] = {
			.H = XSECURE_SHA2_256,
			.n = XSECURE_SHA2_256_HASH_LEN,
			.w = XSECURE_LMS_OTS_W8,
			.u = XSECURE_LMS_OTS_W8_U,
			.v = XSECURE_LMS_OTS_W8_V,
			.ls = XSECURE_LMS_OTS_W8_LS,
			.p = XSECURE_LMS_OTS_W8_P,
			.NoOfInvSign = 255U,
			.SignLen = (XSECURE_LMS_OTS_TYPE_SIZE + (XSECURE_LMS_N_FIELD_SIZE * ((u32)XSECURE_LMS_OTS_W8_P + 1U)))	/* 1124U */
		},
		/* XSECURE_LMS_OTS_SHAKE_N32_W2 */
		[4U] = {
			.H = XSECURE_SHAKE_256,
			.n = XSECURE_SHAKE_256_HASH_LEN,
			.w = XSECURE_LMS_OTS_W2,
			.u = XSECURE_LMS_OTS_W2_U,
			.v = XSECURE_LMS_OTS_W2_V,
			.ls = XSECURE_LMS_OTS_W2_LS,
			.p = XSECURE_LMS_OTS_W2_P,
			.NoOfInvSign = 3U,
			.SignLen = (XSECURE_LMS_OTS_TYPE_SIZE + (XSECURE_LMS_N_FIELD_SIZE * ((u32)XSECURE_LMS_OTS_W2_P + 1U)))	/* 4292U */
		},
		/* XSECURE_LMS_OTS_SHAKE_N32_W4 */
		[5U] = {
			.H = XSECURE_SHAKE_256,
			.n = XSECURE_SHAKE_256_HASH_LEN,
			.w = XSECURE_LMS_OTS_W4,
			.u = XSECURE_LMS_OTS_W4_U,
			.v = XSECURE_LMS_OTS_W4_V,
			.ls = XSECURE_LMS_OTS_W4_LS,
			.p = XSECURE_LMS_OTS_W4_P,
			.NoOfInvSign = 15U,
			.SignLen = (XSECURE_LMS_OTS_TYPE_SIZE + (XSECURE_LMS_N_FIELD_SIZE * ((u32)XSECURE_LMS_OTS_W4_P + 1U)))	/* 2180U */
		},
		/* XSECURE_LMS_OTS_SHAKE_N32_W8 */
		[6U] = {
			.H = XSECURE_SHAKE_256,
			.n = XSECURE_SHAKE_256_HASH_LEN,
			.w = XSECURE_LMS_OTS_W8,
			.u = XSECURE_LMS_OTS_W8_U,
			.v = XSECURE_LMS_OTS_W8_V,
			.ls = XSECURE_LMS_OTS_W8_LS,
			.p = XSECURE_LMS_OTS_W8_P,
			.NoOfInvSign = 255U,
			.SignLen = (XSECURE_LMS_OTS_TYPE_SIZE + (XSECURE_LMS_N_FIELD_SIZE * ((u32)XSECURE_LMS_OTS_W8_P + 1U)))	/* 1124U */
		}
	};

	/* Redundant type for temporal check */
	volatile XSecure_LmsOtsType TmpType = (XSecure_LmsOtsType)XSECURE_ALLFS;
	*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[0U];

	/* 'w' = 1' variations and 24 bit hash outputs are not supported in this library */

	switch (Type) {
		/* SHA-256 options */
		case XSECURE_LMS_OTS_SHA256_N32_W2: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[1U];
			TmpType = XSECURE_LMS_OTS_SHA256_N32_W2;
			break;
		}
		case XSECURE_LMS_OTS_SHA256_N32_W4: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[2U];
			TmpType = XSECURE_LMS_OTS_SHA256_N32_W4;
			break;
		}
		case XSECURE_LMS_OTS_SHA256_N32_W8: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[3U];
			TmpType = XSECURE_LMS_OTS_SHA256_N32_W8;
			break;
		}

		/* SHAKE 256 options */
		case XSECURE_LMS_OTS_SHAKE_N32_W2: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[4U];
			TmpType = XSECURE_LMS_OTS_SHAKE_N32_W2;
			break;
		}
		case XSECURE_LMS_OTS_SHAKE_N32_W4: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[5U];
			TmpType = XSECURE_LMS_OTS_SHAKE_N32_W4;
			break;
		}
		case XSECURE_LMS_OTS_SHAKE_N32_W8: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[6U];
			TmpType = XSECURE_LMS_OTS_SHAKE_N32_W8;
			break;
		}

		/* default, and other not supported options */
		case XSECURE_LMS_OTS_RESERVED: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[0U];
			TmpType = XSECURE_LMS_OTS_RESERVED;
			Status = (u32)XSECURE_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
		case XSECURE_LMS_OTS_NOT_SUPPORTED: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[0U];
			TmpType = XSECURE_LMS_OTS_NOT_SUPPORTED;
			Status = (u32)XSECURE_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
		default: {
			*Parameters = (XSecure_LmsOtsParam*)&XSecure_LmsOtsParamLookup[0U];
			Status = (u32)XSECURE_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
	}

	/* Two possible glitches, replacing this condition with a NOP, we will update error and exit
		flipping condition, we update error and exit, so both cases it is OK. */
	if(TmpType != Type) {
		/* Update register to be taken care by caller */
		Status = (u32)XSECURE_LMS_OTS_TYPE_LOOKUP_GLITCH_ERROR;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
/** @} */
