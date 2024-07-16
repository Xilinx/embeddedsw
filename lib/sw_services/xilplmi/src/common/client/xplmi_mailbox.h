/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_mailbox.h
 *
 * This file contains declarations of xilmailbox generic interface APIs
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       am   04/10/24 Fixed doxygen warning
 *       pre  07/10/24 Added SLR index in client data structure
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xplmi_mailbox_apis XilPlmi Mailbox APIs
 * @{
 */

#ifndef XPLMI_MAILBOX_H
#define XPLMI_MAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xilmailbox.h"
#include "xparameters.h"

/************************************ Constant Definitions ***************************************/

#define XILPLMI_MODULE_ID			(1U) /**< Module ID for xilplmi */
#define PAYLOAD_ARG_CNT			   	(8U)
						/**< 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		   	(8U)
						/**< 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define XPLMI_TARGET_IPI_INT_MASK	(0x00000002U) /**< Target PMC IPI interrupt mask */
#define XPLMI_MODULE_ID_SHIFT		(8U) /**< Module Id shift */
#define XPLMI_PAYLOAD_LEN_SHIFT		(16U) /**< Length shift mask */
#define XILPLMI_MODULE_ID_MASK		((u32)XILPLMI_MODULE_ID << XPLMI_MODULE_ID_SHIFT)
										/**< Module id mask*/
#define XPLMI_SHARED_MEM_SIZE		(160U)
						/**< Max size of shared memory used to store the CDO command */

/************************************** Type Definitions *****************************************/

typedef struct {
	XMailbox *MailboxPtr; /**< pointer to mailbox for IPI communication*/
	u32 Response[RESPONSE_ARG_CNT]; /**< Response payload */
	u32 SlrIndex; /**< SLR index number */
} XPlmi_ClientInstance; /**< xilplmi client instance*/

/*************************** Macros (Inline Functions) Definitions *******************************/

/*****************************************************************************/
/**
 * @brief	This function prepares xilplmi command header
 *
 * @param	Len is Length of Payload
 * @param	ApiId is to check the supported features
 *
 * @return	xilplmi command header
 *
 *****************************************************************************/
static inline u32 PACK_XPLMI_HEADER(u32 Len, u32 ApiId)
{
	return ((Len << XPLMI_PAYLOAD_LEN_SHIFT) | XILPLMI_MODULE_ID_MASK | (ApiId));
}

/************************************ Function Prototypes ****************************************/

int XPlmi_ProcessMailbox(XPlmi_ClientInstance *ClientPtr, u32 *MsgPtr, u32 MsgLen);
int XPlmi_ClientInit(XPlmi_ClientInstance* InstancePtr, XMailbox* MailboxPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_MAILBOX_H */