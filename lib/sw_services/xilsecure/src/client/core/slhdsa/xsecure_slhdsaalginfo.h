/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_slhdsaalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.8   tbk     06/17/26 Initial Release
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_SLHDSAALGINFO_H
#define XSECURE_SLHDSAALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/

/****************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void XSecure_SlhdsaGetCryptoAlgInfo(Xil_CryptoAlgInfo *AlgInfo);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLHDSAALGINFO_H */
