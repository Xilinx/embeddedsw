/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_rsa_common.c
 *
 * This file contains the RSA function definitions which are common across client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/04/25 Initial release
 *       yog  01/28/26 Added RSA key ID validation.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_rsa_common_apis RSA Common APIs
 * @{
*/

/***************************** Include Files *****************************************************/
#include "xasu_rsa_common.h"
#include "xasu_keymanager_common.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates input parameters that are common to all RSA padding
 * 		algorithms.
 *
 * @param	RsaParamsPtr	Pointer to XAsu_RsaParams structure that holds the input parameters
 *				for RSA.
 * @return
 * 	- XST_SUCCESS, if input validation is successful.
 * 	- XST_FAILURE, if input validation fails.
 *
 *************************************************************************************************/
s32 XAsu_RsaValidateInputParams(const XAsu_RsaParams *RsaParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	/** Validate that the addresses of all input buffers are non-zero. */
	if (RsaParamsPtr->InputDataAddr == 0U) {
		goto END;
	}

	Status = XAsu_KmValidateKeyAddrNdKeyId(RsaParamsPtr->KeyCompAddr, RsaParamsPtr->KeyId);

END:
	return Status;
}
/** @} */
