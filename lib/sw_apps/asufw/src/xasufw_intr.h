/**************************************************************************************************
* Copyright (c) 2024 -2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_intr.h
 *
 * This file contains declarations for xasufw_intr.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   12/12/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_INTR_H_
#define XASUFW_INTR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
void XAsufw_HandlePendingInterrupts(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_INTR_H_ */
/** @} */