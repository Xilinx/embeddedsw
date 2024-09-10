/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xasufw_rsahandler.h
* This file contains declarations for xasufw_rsahandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   08/20/24 Initial Release
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xrsa_server_apis RSA Server APIs
* @{
*/

#ifndef XASU_RSAHANDLER_H_
#define XASU_RSAHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
/**< RSA Command Structure Initialization function */
s32 XAsufw_RsaInit(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_RSAHANDLER_H_ */
/** @} */
