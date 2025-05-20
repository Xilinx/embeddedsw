/**************************************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_hmachandler.h
*
* This file contains declarations for xasufw_hmachandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  01/02/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_HMACHANDLER_H_
#define XASUFW_HMACHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_hmacinfo.h"

#ifdef XASU_HMAC_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_HmacInit(void);
#endif /* XASU_HMAC_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_HMACHANDLER_H_ */
/** @} */
