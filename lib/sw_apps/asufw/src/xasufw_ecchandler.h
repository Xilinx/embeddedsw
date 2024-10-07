/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xasufw_ecchandler.h
* This file contains declarations for xasufw_ecchandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  08/19/24 Initial release
*       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/

#ifndef XASU_ECCHANDLER_H_
#define XASU_ECCHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_EccInit(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_ECCHANDLER_H_ */
/** @} */
