/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xsecure_lms.c
*
* This file contains definitions and interface used in LMS.
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
#include "xil_io.h"
#include "xsecure_lms.h"

/************************** Constant Definitions ******************************/

/**************************** Type Definitions ********************************/

/***************** Macros (Inline Functions) Definitions **********************/

/************************** Function Prototypes *******************************/

/************************** Variable Definitions ******************************/

/************************** Function Definitions ******************************/

/******************************************************************************/
/**
* @brief	This function returns the parameters for the type of LMS type
*
* @param	Type 		XSecure_LmsType, type of OTS algorithm selected
* @param	Parameters 	Pointer to array location where all predefined
* 				parameter values are present @ref XSecure_LmsParam
*
* @return
*	-	@ref XST_SUCCESS - Valid type is passed, and parameters are assigned
*	-	@ref XST_FAILURE - If not a valid type is passed
*******************************************************************************/
int XSecure_LmsLookupParamSet(XSecure_LmsType Type,
	XSecure_LmsParam** Parameters)
{
	int Status = XST_FAILURE;

	static XSecure_LmsParam XSecure_LmsParamLookup[XSECURE_LMS_TYPE_MAX_SUPPORTED] = {
		/* XSECURE_LMS_RESERVED, XSECURE_LMS_NOT_SUPPORTED, default */
		[0U] = {
			.H = XSECURE_SHA_INVALID_MODE,
			.m = 0U,
			.h = XSECURE_LMS_TREE_HEIGHT_UNSUPPORTED,
			.mh = 0U
		},
		/* XSECURE_LMS_SHA256_M32_H5 */
		[1U] = {
			.H = XSECURE_SHA2_256,
			.m = XSECURE_SHA2_256_HASH_LEN,
			.h = XSECURE_LMS_TREE_HEIGHT_H5,
			.mh = (XSECURE_SHA2_256_HASH_LEN * (u32)XSECURE_LMS_TREE_HEIGHT_H5) /** 160U */
		},
		/* XSECURE_LMS_SHA256_M32_H10 */
		[2U] = {
			.H = XSECURE_SHA2_256,
			.m = XSECURE_SHA2_256_HASH_LEN,
			.h = XSECURE_LMS_TREE_HEIGHT_H10,
			.mh = (XSECURE_SHA2_256_HASH_LEN * (u32)XSECURE_LMS_TREE_HEIGHT_H10) /** 320U */
		},
		/* XSECURE_LMS_SHA256_M32_H15 */
		[3U] = {
			.H = XSECURE_SHA2_256,
			.m = XSECURE_SHA2_256_HASH_LEN,
			.h = XSECURE_LMS_TREE_HEIGHT_H15,
			.mh = (XSECURE_SHA2_256_HASH_LEN * (u32)XSECURE_LMS_TREE_HEIGHT_H15) /** 480U */
		},
		/* XSECURE_LMS_SHA256_M32_H20 */
		[4U] = {
			.H = XSECURE_SHA2_256,
			.m = XSECURE_SHA2_256_HASH_LEN,
			.h = XSECURE_LMS_TREE_HEIGHT_H20,
			.mh = (XSECURE_SHA2_256_HASH_LEN * (u32)XSECURE_LMS_TREE_HEIGHT_H20) /** 640U */
		},
		/* XSECURE_LMS_SHAKE_M32_H5 */
		[5U] = {
			.H = XSECURE_SHAKE_256,
			.m = XSECURE_SHAKE_256_HASH_LEN,
			.h = XSECURE_LMS_TREE_HEIGHT_H5,
			.mh = (XSECURE_SHAKE_256_HASH_LEN * (u32)XSECURE_LMS_TREE_HEIGHT_H5) /** 160U */
		},
		/* XSECURE_LMS_SHAKE_M32_H10 */
		[6U] = {
			.H = XSECURE_SHAKE_256,
			.m = XSECURE_SHAKE_256_HASH_LEN,
			.h = XSECURE_LMS_TREE_HEIGHT_H10,
			.mh = (XSECURE_SHAKE_256_HASH_LEN * (u32)XSECURE_LMS_TREE_HEIGHT_H10) /** 320U */
		},
		/* XSECURE_LMS_SHAKE_M32_H15 */
		[7U] = {
			.H = XSECURE_SHAKE_256,
			.m = XSECURE_SHAKE_256_HASH_LEN,
			.h = XSECURE_LMS_TREE_HEIGHT_H15,
			.mh = (XSECURE_SHAKE_256_HASH_LEN * (u32)XSECURE_LMS_TREE_HEIGHT_H15) /** 480U */
		},
		/* XSECURE_LMS_SHAKE_M32_H20 */
		[8U] = {
			.H = XSECURE_SHAKE_256,
			.m = XSECURE_SHAKE_256_HASH_LEN,
			.h = XSECURE_LMS_TREE_HEIGHT_H20,
			.mh = (XSECURE_SHAKE_256_HASH_LEN * (u32)XSECURE_LMS_TREE_HEIGHT_H20) /** 640U */
		}
	};

	/* Redundant type for temporal check */
	volatile XSecure_LmsType TmpType = (XSecure_LmsType)XSECURE_ALLFS;
	*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[0U];

	/* 'h'=25 variations and 24 bit hash outputs are not supported till date in this library */

	switch (Type) {
		/* SHA-256 options */
		case XSECURE_LMS_SHA256_M32_H5: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[1U];
			TmpType = XSECURE_LMS_SHA256_M32_H5;
			break;
		}
		case XSECURE_LMS_SHA256_M32_H10: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[2U];
			TmpType = XSECURE_LMS_SHA256_M32_H10;
			break;
		}
		case XSECURE_LMS_SHA256_M32_H15: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[3U];
			TmpType = XSECURE_LMS_SHA256_M32_H15;
			break;
		}
		case XSECURE_LMS_SHA256_M32_H20: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[4U];
			TmpType = XSECURE_LMS_SHA256_M32_H20;
			break;
		}

		/* SHAKE 256 options */
		case XSECURE_LMS_SHAKE_M32_H5: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[5U];
			TmpType = XSECURE_LMS_SHAKE_M32_H5;
			break;
		}
		case XSECURE_LMS_SHAKE_M32_H10: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[6U];
			TmpType = XSECURE_LMS_SHAKE_M32_H10;
			break;
		}
		case XSECURE_LMS_SHAKE_M32_H15: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[7U];
			TmpType = XSECURE_LMS_SHAKE_M32_H15;
			break;
		}
		case XSECURE_LMS_SHAKE_M32_H20: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[8U];
			TmpType = XSECURE_LMS_SHAKE_M32_H20;
			break;
		}

		/* default, and other not supported options */
		case XSECURE_LMS_RESERVED: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[0U];
			TmpType = XSECURE_LMS_RESERVED;
			Status = (u32)XSECURE_LMS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
		case XSECURE_LMS_NOT_SUPPORTED: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[0U];
			TmpType = XSECURE_LMS_NOT_SUPPORTED;
			Status = (u32)XSECURE_LMS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
		default: {
			*Parameters = (XSecure_LmsParam*)&XSecure_LmsParamLookup[0U];
			Status = (u32)XSECURE_LMS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
	}

	/* Two possible glitches, replacing this condition with a NOP,
	 * we will update error and exit flipping condition,
	 * we update error and exit, so both cases it is OK.
	 */
	if(TmpType != Type) {
		/* Update register to be taken care by caller */
		Status = (u32)XSECURE_LMS_TYPE_LOOKUP_GLITCH_ERROR;
		goto END;
	}

	Status = (u32)XST_SUCCESS;

END:
	/* No matter success or failure, call happened successfully */
	return Status;
}


/** @} */
