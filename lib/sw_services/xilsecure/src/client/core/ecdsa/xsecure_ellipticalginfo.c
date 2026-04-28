/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file client/core/ecdsa/xsecure_ellipticalginfo.c
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
#include "xsecure_ellipticalginfo.h"
#include "xsecure_alghelper.h"
#include "xsecure_plat_defs.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Function Definitions **************************************/

/** @addtogroup xsecure_ecdsa_client_apis XilSecure ECDSA Client APIs
 * @{
 */

/**************************************************************************************************/
/**
 *
 * This function returns the Elliptic crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 **************************************************************************************************/
void XSecure_EllipticGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	XSecure_GetAlgInfo(AlgInfo, (u32)XSECURE_API_ELLIPTIC_VERIFY_SIGN);
}

/** @} */
