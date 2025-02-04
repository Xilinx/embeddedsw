/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_rsa_common.c
 * @addtogroup Overview
 * @{
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
 * @brief	This function validates input parameters for padding.
 *
 * @param	RsaParamsPtr	Pointer which holds the parameters of RSA input arguments.
 *
 * @return
 *		- Upon successful validation of input parameters, it returns XST_SUCCESS.
 *		- XST_FAILURE on failure.
 *
 *************************************************************************************************/
s32 XAsu_RsaValidateInputParams(const XAsu_RsaParams *RsaParamsPtr)
{
	s32 Status = XST_FAILURE;

	if ((RsaParamsPtr->InputDataAddr == 0U) ||
	    (RsaParamsPtr->KeyCompAddr == 0U)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
