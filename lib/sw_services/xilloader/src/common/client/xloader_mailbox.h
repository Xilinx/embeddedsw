/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_mailbox.h
 *
 * This file contains declarations of xilmailbox generic interface APIs
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xloader_mailbox_apis XilLoader Mailbox APIs
 * @{
 */

#ifndef XLOADER_MAILBOX_H
#define XLOADER_MAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xilmailbox.h"
#include "xparameters.h"
#include "xloader_defs.h"

/************************************ Constant Definitions ***************************************/

#define XILLOADER_MODULE_ID			(7U) /**< Module id for xilloader */
#define PAYLOAD_ARG_CNT				(8U)
						/**< 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		   	(8U)
						/**< 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define XLOADER_TARGET_IPI_INT_MASK	(0x00000002U) /**< Target PMC IPI interrupt mask */
#define XLOADER_MODULE_ID_SHIFT		(8U) /**< Module id shift */
#define XLOADER_PAYLOAD_LEN_SHIFT	(16U) /**< Length shift mask */
#define XILLOADER_MODULE_ID_MASK	((u32)XILLOADER_MODULE_ID << XLOADER_MODULE_ID_SHIFT)
										/**< Module id mask*/
#define XLOADER_SHARED_MEM_SIZE		(160U)
						/**< Max size of shared memory used to store the CDO command */

/************************************** Type Definitions *****************************************/

typedef struct {
	XMailbox *MailboxPtr; /**< Pointer to mailbox for IPI communication */
	u32 Response[RESPONSE_ARG_CNT];
} XLoader_ClientInstance; /**< Xilloader client instance */

/*************************** Macros (Inline Functions) Definitions *******************************/

static inline u32 PACK_XLOADER_HEADER(u32 Len, u32 ApiId)
{
	return ((Len << XLOADER_PAYLOAD_LEN_SHIFT) | XILLOADER_MODULE_ID_MASK | (ApiId));
}

/************************************ Function Prototypes ****************************************/

int XLoader_ProcessMailbox(XLoader_ClientInstance *ClientPtr, u32 *MsgPtr, u32 MsgLen);
int XLoader_ClientInit(XLoader_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_MAILBOX_H */