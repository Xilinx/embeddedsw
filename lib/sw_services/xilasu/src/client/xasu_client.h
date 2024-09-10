/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_client.h
 * @addtogroup Overview
 * @{
 *
 * This file contains declarations for xasu_client.c file in XilAsu library
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  06/04/24 Initial release
 *       ss   08/13/24 Changed XAsu_ClientInit function prototype and addded XAsu_ClientParams
 *                     structure
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASU_CLIENT_H
#define XASU_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasu_sharedmem.h"
#include "xilmailbox.h"

/************************************ Constant Definitions ***************************************/
#define XASU_PRIORITY_LOW                   1U  /**< LOW priority */
#define XASU_PRIORITY_HIGH                  0U  /**< High priority */

/************************************** Type Definitions *****************************************/

/*
 * This contains the queue information
 */
typedef struct {
	XAsu_ChannelQueue *ChannelQueue;
	u8 NextFreeIndex;
} XAsu_QueueInfo;

typedef struct {
	u8 Priority;    /**< Task Priority */
	u8 Reserved;    /**< Reserved */
	u16 Reserved2;  /**< Reserved */
	void *CallBackFuncPtr;  /**< Call Back function pointer */
	void *CallBackRefPtr;   /**< Call Back reference pointer */
} XAsu_ClientParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/
/**
 * @brief   This function formats the command header with Module ID, Command ID and Command Length
 *
 * @param   CmdId		Command ID to be updated
 * @param   UniqueId    Unique ID of the request
 * @param   ModuleId	Module ID to be updated
 * @param   CommandLen	Length of the command
 *
 * @return
 * 			- Header	Command Header.
 *
 *************************************************************************************************/
inline u32 XAsu_CreateHeader(u8 CmdId, u8 UniqueId, u8 ModuleId, u8 CommandLen)
{
	u32 Header = 0U;

	Header = (CmdId & XASU_COMMAND_ID_MASK) |
		 (UniqueId << XASU_UNIQUE_REQ_ID_SHIFT) |
		 (ModuleId << XASU_MODULE_ID_SHIFT) |
		 (CommandLen << XASU_COMMAND_LENGTH_SHIFT);

	return Header;
}

/************************************ Function Prototypes ****************************************/
XAsu_QueueInfo *XAsu_GetQueueInfo(u32 QueuePriority);
XAsu_ChannelQueueBuf *XAsu_GetChannelQueueBuf(XAsu_QueueInfo *QueueInfo);
s32 XAsu_UpdateQueueBufferNSendIpi(XAsu_QueueInfo *QueueInfo);
s32 XAsu_ClientInit(u8 DeviceId);
/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_CLIENT_H */
/** @} */