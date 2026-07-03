/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file xsecure_mldsaalginfo.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.8   tbk     06/17/26 Initial Release
*
* </pre>
*
***************************************************************************************************/

/****************************************** Include Files *****************************************/
#include "xsecure_mldsaalginfo.h"
#include "xsecure_alghelper.h"
#include "xsecure_plat_defs.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Function Definitions **************************************/

/**************************************************************************************************/
/**
 *
 * This function returns the MLDSA crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 **************************************************************************************************/
void XSecure_MldsaGetCryptoAlgInfo(Xil_CryptoAlgInfo *AlgInfo)
{
	XSecure_GetAlgInfo(AlgInfo, (u32)XSECURE_API_MLDSA_SIGN_VERIFY);
}
