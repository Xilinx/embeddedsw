/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file client/core/lms_hss/xsecure_lmsalginfo.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.7   tvp     04/29/26 Initial Release - Moved function implementation from header file
*
* </pre>
 *
***************************************************************************************************/

/****************************************** Include Files *****************************************/
#include "xsecure_lmsalginfo.h"
#include "xsecure_alghelper.h"
#include "xsecure_plat_defs.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Function Definitions **************************************/

/** @addtogroup xsecure_lms_client_apis XilSecure LMS Client APIs
 * @{
 */

/**************************************************************************************************/
/**
 *
 * This function returns the LMS crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 **************************************************************************************************/
void XSecure_LmsGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	XSecure_GetAlgInfo(AlgInfo, (u32)XSECURE_API_LMS_SIGN_VERIFY);
}

/** @} */
