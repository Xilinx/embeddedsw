/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_client.h
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
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       vns  09/30/24 Added support for asynchronous communication
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_client_info Client APIs AND Error Codes
 * @{
*/
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

typedef void (*XAsuClient_ResponseHandler) (void *CallBackRefPtr, u32 Status);
                                            /**< Response handler */


/** @brief This structure contains client parameters information. */
typedef struct {
	u8 Priority;    /**< Task Priority */
	u8 Reserved;    /**< Reserved */
	u16 Reserved2;  /**< Reserved */
	XAsuClient_ResponseHandler CallBackFuncPtr;  /**< Call Back function pointer */
	void *CallBackRefPtr;   /**< Call Back reference pointer */
} XAsu_ClientParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/
/**
 * @brief   This function gets the unique ID from the provided header
 *
 * @param   Header		Header of the response/request buffer.
 *
 * @return
 * 			- Unique ID	Returns the unique ID
 *
 *************************************************************************************************/
inline u8 XAsu_GetUniqueId(u32 Header)
{
	return (u8)((Header & XASU_UNIQUE_REQ_ID_MASK) >> XASU_UNIQUE_REQ_ID_SHIFT);
}

/*************************************************************************************************/
/**
 * @brief	This function formats the command header with module ID, command ID and command
 * 		length.
 *
 * @param	CmdId		Command ID to be updated.
 * @param	UniqueId	Unique ID of the request.
 * @param	ModuleId	Module ID to be updated.
 * @param	CommandLen	Length of the command.
 *
 * @return
 * 	- Header	Command Header.
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
s32 XAsu_ClientInit(u32 BaseAddress);
s32 XAsu_ValidateClientParameters(XAsu_ClientParams *ClientParamPtr);
s32 XAsu_UpdateQueueBufferNSendIpi(XAsu_ClientParams *ClientParam, void *ReqBuffer,
				   u32 Size, u32 Header);
u8 XAsu_RegCallBackNGetUniqueId(XAsu_ClientParams *ClientParamPtr, u8 *RespBufferPtr, u32 Size);
/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_CLIENT_H */
/** @} */
