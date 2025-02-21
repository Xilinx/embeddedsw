/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_error_manager.h
 *
 * This file contains declarations for xasufw_error_manager.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   02/21/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_ERROR_MANAGER_H
#define XASUFW_ERROR_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/** This typedef is used to select error type. */
typedef enum {
	XASUFW_FATAL_ERROR = 1, /**< ASUFW Fatal Error */
    XASUFW_NON_FATAL_ERROR, /**< ASUFW Non-Fatal Error */
} XAsufw_ErrorType;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
void XAsufw_ErrorManagerInit(void);
void XAsufw_SendErrorToPlm(XAsufw_ErrorType ErrorType, s32 ErrorCode);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_ERROR_MANAGER_H */
/** @} */
