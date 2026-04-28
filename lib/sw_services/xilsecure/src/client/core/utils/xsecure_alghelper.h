/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file client/core/utils/xsecure_alghelper.h
*
* This file contains function prototypes for algorithm helper functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.7   tbk     02/05/26 Initial Release
*
* </pre>
*
***************************************************************************************************/

#ifndef XSECURE_ALGHELPER_H
#define XSECURE_ALGHELPER_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xil_cryptoalginfo.h"
#include "xil_types.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/
int XSecure_GetAlgInfo(Xil_CryptoAlgInfo *AlgInfo, u32 CryptoAlg);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ALGHELPER_H */
