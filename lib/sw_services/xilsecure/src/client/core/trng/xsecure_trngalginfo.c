/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file client/core/trng/xsecure_trngalginfo.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.7   tbk     04/16/26 Initial Release
*
* </pre>
*
***************************************************************************************************/

/****************************************** Include Files *****************************************/
#include "xsecure_trngalginfo.h"
#include "xsecure_alghelper.h"
#include "xsecure_plat_defs.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Function Definitions **************************************/

/** @addtogroup xsecure_trng_client_apis XilSecure TRNG Client APIs
 * @{
 */

/**************************************************************************************************/
/**
 *
 * This function returns the TRNG crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 **************************************************************************************************/
void XSecure_TrngGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	XSecure_GetAlgInfo(AlgInfo, (u32)XSECURE_API_TRNG_GENERATE);
}

/** @} */
