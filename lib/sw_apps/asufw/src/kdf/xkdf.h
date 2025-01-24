/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xkdf.h
*
* This file contains declarations for xkdf.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ma   01/15/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xkdf_server_apis KDF Server APIs
* @{
*/

#ifndef XKDF_H
#define XKDF_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xsha.h"
#include "xasufw_dma.h"
#include "xasu_kdfinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Macro Definitions ******************************************/

/************************************ Type Definitions *******************************************/

/************************************ Function Prototypes ****************************************/
s32 XKdf_Compute(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, const XAsu_KdfParams *KdfParams);

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* XKDF_H_ */
/** @} */
