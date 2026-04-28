/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file client/versal_net/xsecure_keyunwrapalginfo.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.7   tbk     04/09/26 Initial Release
*
* </pre>
*
***************************************************************************************************/

/****************************************** Include Files *****************************************/
#include "xsecure_keyunwrapalginfo.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Function Definitions **************************************/

/** @addtogroup xsecure_keyunwrap_client_apis XilSecure Key Unwrap Client APIs
 * @{
 */

/**************************************************************************************************/
/**
 *
 * This function returns the Key Unwrap crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 **************************************************************************************************/
void XSecure_KeyUnwrapGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	XSecure_GetAlgInfo(AlgInfo, (u32)XSECURE_API_KEY_UNWRAP);
}

/** @} */
