/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xlms.c
*
* This file contains definitions and interface used in LMS for ASUFW.
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
#include "xlms.h"
#include "xasufw_status.h"
#include "xasu_shainfo.h"
#include "xasufw_util.h"

#ifdef XASU_LMS_ENABLE

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
* @brief	This function returns the parameters for the type of LMS type
*
* @param	Type 		XLms_Type, type of OTS algorithm selected
* @param	Parameters 	Pointer to array location where all predefined
* 				parameter values are present in XLms_Param
*
* @return
*	- XASUFW_SUCCESS - Valid type is passed, and parameters are assigned
*	- XASUFW_LMS_TYPE_UNSUPPORTED_ERROR - If not a valid type is passed
*	- XASUFW_LMS_TYPE_LOOKUP_GLITCH_ERROR - If glitch detected
*
**************************************************************************************************/
s32 XLms_LookupParamSet(XLms_Type Type, const XLms_Param** Parameters)
{
	s32 Status = XASUFW_FAILURE;
	/* Redundant type for temporal check */
	volatile XLms_Type TmpType = (XLms_Type)XASUFW_ALLFS;
	static const XLms_Param XLms_ParamLookup[XLMS_TYPE_MAX_SUPPORTED] = {
		/* XLMS_RESERVED, XLMS_NOT_SUPPORTED, default */
		[XLMS_PARAM_IDX_RESERVED] = {
			.HashAlgId = XLMS_HASH_MODE_UNSUPPORTED,
			.HashOutputBytes = 0U,
			.TreeHeight = XLMS_TREE_HEIGHT_UNSUPPORTED,
			.SignatureLenBytes = 0U
		},
		/* XLMS_SHA256_M32_HEIGHT_5 */
		[XLMS_PARAM_IDX_SHA256_HEIGHT_5] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_5,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_5) /* 160U */
		},
		/* XLMS_SHA256_M32_HEIGHT_10 */
		[XLMS_PARAM_IDX_SHA256_HEIGHT_10] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_10,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_10) /* 320U */
		},
		/* XLMS_SHA256_M32_HEIGHT_15 */
		[XLMS_PARAM_IDX_SHA256_HEIGHT_15] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_15,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_15) /* 480U */
		},
		/* XLMS_SHA256_M32_HEIGHT_20 */
		[XLMS_PARAM_IDX_SHA256_HEIGHT_20] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_20,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_20) /* 640U */
		},
		/* XLMS_SHA256_M32_HEIGHT_25 */
		[XLMS_PARAM_IDX_SHA256_HEIGHT_25] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_25,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_25) /* 800U */
		},

		/* XLMS_SHAKE_M32_HEIGHT_5 */
		[XLMS_PARAM_IDX_SHAKE_HEIGHT_5] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_5,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_5) /* 160U */
		},
		/* XLMS_SHAKE_M32_HEIGHT_10 */
		[XLMS_PARAM_IDX_SHAKE_HEIGHT_10] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_10,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_10) /* 320U */
		},
		/* XLMS_SHAKE_M32_HEIGHT_15 */
		[XLMS_PARAM_IDX_SHAKE_HEIGHT_15] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_15,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_15) /* 480U */
		},
		/* XLMS_SHAKE_M32_HEIGHT_20 */
		[XLMS_PARAM_IDX_SHAKE_HEIGHT_20] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_20,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_20) /* 640U */
		},
		/* XLMS_SHAKE_M32_HEIGHT_25 */
		[XLMS_PARAM_IDX_SHAKE_HEIGHT_25] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XASU_SHA_SHAKE_256_HASH_LEN,
			.TreeHeight = XLMS_TREE_HEIGHT_25,
			.SignatureLenBytes = (XASU_SHA_SHAKE_256_HASH_LEN * (u32)XLMS_TREE_HEIGHT_25) /* 800U */
		},

		/* M=24 SHA-256 variants */
		[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_5] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_5,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_5) /* 120U */
		},
		[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_10] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_10,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_10) /* 240U */
		},
		[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_15] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_15,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_15) /* 360U */
		},
		[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_20] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_20,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_20) /* 480U */
		},
		[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_25] = {
			.HashAlgId = XASU_SHA_MODE_256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_25,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_25) /* 600U */
		},

		/* M=24 SHAKE-256 variants */
		[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_5] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_5,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_5) /* 120U */
		},
		[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_10] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_10,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_10) /* 240U */
		},
		[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_15] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_15,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_15) /* 360U */
		},
		[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_20] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_20,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_20) /* 480U */
		},
		[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_25] = {
			.HashAlgId = XASU_SHA_MODE_SHAKE256,
			.HashOutputBytes = XLMS_N24_FIELD_SIZE,
			.TreeHeight = XLMS_TREE_HEIGHT_25,
			.SignatureLenBytes = (XLMS_N24_FIELD_SIZE * (u32)XLMS_TREE_HEIGHT_25) /* 600U */
		}
	};
	*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_RESERVED];

	switch (Type) {
		/* SHA-256 options */
		case XLMS_SHA256_M32_HEIGHT_5: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_HEIGHT_5];
			TmpType = XLMS_SHA256_M32_HEIGHT_5;
			break;
		}
		case XLMS_SHA256_M32_HEIGHT_10: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_HEIGHT_10];
			TmpType = XLMS_SHA256_M32_HEIGHT_10;
			break;
		}
		case XLMS_SHA256_M32_HEIGHT_15: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_HEIGHT_15];
			TmpType = XLMS_SHA256_M32_HEIGHT_15;
			break;
		}
		case XLMS_SHA256_M32_HEIGHT_20: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_HEIGHT_20];
			TmpType = XLMS_SHA256_M32_HEIGHT_20;
			break;
		}
		case XLMS_SHA256_M32_HEIGHT_25: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_HEIGHT_25];
			TmpType = XLMS_SHA256_M32_HEIGHT_25;
			break;
		}


		/* SHAKE 256 options */
		case XLMS_SHAKE_M32_HEIGHT_5: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_HEIGHT_5];
			TmpType = XLMS_SHAKE_M32_HEIGHT_5;
			break;
		}
		case XLMS_SHAKE_M32_HEIGHT_10: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_HEIGHT_10];
			TmpType = XLMS_SHAKE_M32_HEIGHT_10;
			break;
		}
		case XLMS_SHAKE_M32_HEIGHT_15: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_HEIGHT_15];
			TmpType = XLMS_SHAKE_M32_HEIGHT_15;
			break;
		}
		case XLMS_SHAKE_M32_HEIGHT_20: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_HEIGHT_20];
			TmpType = XLMS_SHAKE_M32_HEIGHT_20;
			break;
		}
		case XLMS_SHAKE_M32_HEIGHT_25: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_HEIGHT_25];
			TmpType = XLMS_SHAKE_M32_HEIGHT_25;
			break;
		}
		/* default, and other not supported options */
		case XLMS_RESERVED: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_RESERVED];
			TmpType = XLMS_RESERVED;
			Status = XASUFW_LMS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
		case XLMS_NOT_SUPPORTED: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_RESERVED];
			TmpType = XLMS_NOT_SUPPORTED;
			Status = XASUFW_LMS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
		/* SHA-256 M=24 options (NIST SP 800-208) */
		case XLMS_SHA256_M24_HEIGHT_5: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_5];
			TmpType = XLMS_SHA256_M24_HEIGHT_5;
			break;
		}
		case XLMS_SHA256_M24_HEIGHT_10: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_10];
			TmpType = XLMS_SHA256_M24_HEIGHT_10;
			break;
		}
		case XLMS_SHA256_M24_HEIGHT_15: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_15];
			TmpType = XLMS_SHA256_M24_HEIGHT_15;
			break;
		}
		case XLMS_SHA256_M24_HEIGHT_20: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_20];
			TmpType = XLMS_SHA256_M24_HEIGHT_20;
			break;
		}
		case XLMS_SHA256_M24_HEIGHT_25: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHA256_M24_HEIGHT_25];
			TmpType = XLMS_SHA256_M24_HEIGHT_25;
			break;
		}

		/* SHAKE-256 M=24 options (NIST SP 800-208) */
		case XLMS_SHAKE_M24_HEIGHT_5: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_5];
			TmpType = XLMS_SHAKE_M24_HEIGHT_5;
			break;
		}
		case XLMS_SHAKE_M24_HEIGHT_10: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_10];
			TmpType = XLMS_SHAKE_M24_HEIGHT_10;
			break;
		}
		case XLMS_SHAKE_M24_HEIGHT_15: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_15];
			TmpType = XLMS_SHAKE_M24_HEIGHT_15;
			break;
		}
		case XLMS_SHAKE_M24_HEIGHT_20: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_20];
			TmpType = XLMS_SHAKE_M24_HEIGHT_20;
			break;
		}
		case XLMS_SHAKE_M24_HEIGHT_25: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_SHAKE_M24_HEIGHT_25];
			TmpType = XLMS_SHAKE_M24_HEIGHT_25;
			break;
		}
		default: {
			*Parameters = &XLms_ParamLookup[XLMS_PARAM_IDX_RESERVED];
			Status = XASUFW_LMS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}
	}

	/* Two possible glitches, replacing this condition with a NOP,
	 * we will update error and exit flipping condition,
	 * we update error and exit, so both cases it is OK.
	 */
	if(TmpType != Type) {
		/* Update register to be taken care by caller */
		Status = XASUFW_LMS_TYPE_LOOKUP_GLITCH_ERROR;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	/* No matter success or failure, call happened successfully */
	return Status;
}
#endif /* XASU_LMS_ENABLE */
/** @} */
