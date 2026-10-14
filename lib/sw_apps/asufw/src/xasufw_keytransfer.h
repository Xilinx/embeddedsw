/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_keytransfer.h
 *
 * This file contains declarations for xasufw_keytransfer.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog   06/25/26 Initial release
 *       kp   09/23/26 Key transfer takes one red/black form via XASU_EFUSE_*
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_KEYTRANSFER_H_
#define XASUFW_KEYTRANSFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_PmcKeyTransfer(u32 *PufKekGenStatusPtr, u32 KeyForm);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_KEYTRANSFER_H_ */
/** @} */
