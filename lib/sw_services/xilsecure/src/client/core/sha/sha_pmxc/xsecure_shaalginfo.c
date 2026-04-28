/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file client/core/sha/sha_pmxc/xsecure_shaalginfo.c
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
#include "xsecure_shaalginfo.h"
#include "xsecure_alghelper.h"
#include "xsecure_plat_defs.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Function Definitions **************************************/

/** @addtogroup xsecure_sha3_client_apis XilSecure SHA Client APIs
 * @{
 */

/**************************************************************************************************/
/**
 *
 * This function returns the SHA crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 **************************************************************************************************/
void XSecure_ShaGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
	XSecure_GetAlgInfo(AlgInfo, (u32)XSECURE_API_SHA3_OPERATION);
#else
	XSecure_GetAlgInfo(AlgInfo, (u32)XSECURE_API_SHA3_UPDATE);
#endif
}

/** @} */
