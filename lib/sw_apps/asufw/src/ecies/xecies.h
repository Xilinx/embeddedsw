/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xecies.h
*
* This file contains declarations for xecies.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  02/20/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xecies_server_apis ECIES Server APIs
* @{
*/

#ifndef XECIES_H_
#define XECIES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xsha.h"
#include "xaes.h"
#include "xasufw_dma.h"
#include "xasu_eciesinfo.h"

#ifdef XASU_ECIES_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Macro Definitions ******************************************/

/************************************ Type Definitions *******************************************/

/************************************ Function Prototypes ****************************************/
s32 XEcies_Encrypt(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, XAes *AesInstancePtr,
		   const XAsu_EciesParams *EciesParams);
s32 XEcies_Decrypt(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, XAes *AesInstancePtr,
		   const XAsu_EciesParams *EciesParams);
#endif /* XASU_ECIES_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XECIES_H_ */
/** @} */
