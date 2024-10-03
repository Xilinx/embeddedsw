/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sha2handler.h
 *
 * This file contains function declarations, macro and structure defines related to SHA2 module in
 * ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   04/18/24 Initial release
 *       ma   06/14/24 Add support for SHAKE256 XOF and also modify SHA APIs to take DMA pointer
 *                     for every update
 *       ma   07/08/24 Add task based approach at queue level
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_SHA2HANDLER_H
#define XASUFW_SHA2HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_cmd.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_Sha2Init(void);
s32 XAsufw_Sha2Operation(XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_SHA2HANDLER_H */
/** @} */
