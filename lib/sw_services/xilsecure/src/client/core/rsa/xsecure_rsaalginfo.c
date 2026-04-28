/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file client/core/rsa/xsecure_rsaalginfo.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.7   tbk     02/05/26 Initial Release - Moved function implementation from header file
*
* </pre>
*
***************************************************************************************************/

/****************************************** Include Files *****************************************/
#include "xsecure_rsaalginfo.h"
#include "xsecure_alghelper.h"
#include "xsecure_plat_defs.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Function Definitions **************************************/

/** @addtogroup xsecure_rsa_client_apis XilSecure RSA Client APIs
 * @{
 */

/**************************************************************************************************/
/**
 *
 * This function returns the RSA crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm
 * 			 information
 *
 * @return	None
 *
 **************************************************************************************************/
void XSecure_RsaGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	XSecure_GetAlgInfo(AlgInfo, (u32)XSECURE_API_RSA_SIGN_VERIFY);
}

/** @} */
