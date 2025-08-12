/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
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
	if ((RsaParamsPtr->InputDataAddr == 0U) ||
	    (RsaParamsPtr->KeyCompAddr == 0U)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
