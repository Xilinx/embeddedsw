/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocp_common.c
 *
 * This file contains the OCP function definitions which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  08/12/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ocp_common_apis OCP Common APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_ocp_common.h"

/********************************* Constant Definitions ******************************************/

/*********************************** Macros Definitions ******************************************/

/*********************************** Type Definitions ********************************************/

/********************************* Variable Definitions ******************************************/

/****************************** Inline Function Definitions **************************************/

/********************************** Function Prototypes ******************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the OCP certificate parameters.
 *
 * @param	OcpCertParam	Pointer to XAsu_OcpCertParams structure which holds the parameters
 *				of OCP input argument.
 *
 * @return
 *	- XST_SUCCESS, if OCP certificate parameters are validated successfully.
 *	- XST_FAILURE, if OCP certificate parameters validation is failed.
 *
 *************************************************************************************************/
s32 XAsu_OcpValidateCertParams(const XAsu_OcpCertParams *OcpCertParam)
{
	s32 Status = XST_FAILURE;

	if ((OcpCertParam->CertBufAddr == 0U) || (OcpCertParam->CertBufLen == 0U) ||
	    (OcpCertParam->CertActualSize == 0U) ||
	    (OcpCertParam->DevKeySel >= (u32)XOCP_MAX_DEVICE_KEY)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
