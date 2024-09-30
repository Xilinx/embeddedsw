/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_cmd.h
 *
 * This file contains declarations for xasufw_cmd.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/02/24 Initial release
 *       ma   04/18/24 Moved command handling related APIs from xasufw_sharedmem.c to this file
 *       ma   05/18/24 Added API to check resources availability before executing a command
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
#ifndef XASUFW_CMD_H
#define XASUFW_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_sharedmem.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_CommandQueueHandler(XAsu_ChannelQueueBuf *QueueBuf, u32 QueueId);
void XAsufw_CommandResponseHandler(XAsu_ChannelQueueBuf *QueueBuf, s32 Response);
s32 XAsufw_ValidateCommand(XAsu_ReqBuf *ReqBuf);
s32 XAsufw_CheckResources(XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_CMD_H */
/** @} */
