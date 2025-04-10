/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xhkdf.h
*
* This file contains declarations for xhkdf.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   LP   04/07/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xhkdf_server_apis HKDF Server APIs
* @{
*/

#ifndef XHKDF_H_
#define XHKDF_H_

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
/**
* @brief This structure contains HKDF params info
*/
typedef struct {
	XAsu_KdfParams KdfParams; /**< Kdf Parameters for HKDF operation */
	u64 SaltAddr; /**< Address of the buffer holding salt which is optional */
	u32 SaltLen; /**< Length of the Salt */
} XAsu_HkdfParams;

/************************************ Function Prototypes ****************************************/
s32 XHkdf_Generate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, const XAsu_HkdfParams *HkdfParams);

#ifdef __cplusplus
}
#endif

#endif /* XHKDF_H_ */
/** @} */
