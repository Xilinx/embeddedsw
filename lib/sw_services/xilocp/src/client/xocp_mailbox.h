/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_mailbox.h
* @addtogroup xocp_mailbox_apis XilOcp Mailbox APIs
* @{
*
* @cond xocp_internal
* This file contains declarations of xilmailbox generic interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XOCP_MAILBOX_H
#define XOCP_MAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**@cond xpuf_internal
 * @{
 */
#define XILOCP_MODULE_ID			(13U)

/** 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT			(8U)
/** 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT			(8U)
#define XOCP_TARGET_IPI_INT_MASK	(0x00000002U)
#define XOCP_MODULE_ID_SHIFT		(8U)
#define XOCP_PAYLOAD_LEN_SHIFT		(16U)
#define XILOCP_MODULE_ID_MASK		(XILOCP_MODULE_ID << XOCP_MODULE_ID_SHIFT)

#define XOCP_PAYLOAD_LEN_1U		(1U)
#define XOCP_PAYLOAD_LEN_2U		(2U)
#define XOCP_PAYLOAD_LEN_3U		(3U)
#define XOCP_PAYLOAD_LEN_4U		(4U)
#define XOCP_PAYLOAD_LEN_5U		(5U)
#define XOCP_PAYLOAD_LEN_6U		(6U)
#define XOCP_PAYLOAD_LEN_7U		(7U)

/**************************** Type Definitions *******************************/
typedef struct {
	XMailbox *MailboxPtr;
} XOcp_ClientInstance;

/***************** Macros (Inline Functions) Definitions *********************/
static inline u32 OcpHeader(u32 Len, u32 ApiId)
{
	return ((Len << XOCP_PAYLOAD_LEN_SHIFT) |
		XILOCP_MODULE_ID_MASK | (ApiId));
}

/**
 * @}
 * @endcond
 */
/************************** Variable Definitions *****************************/

/************************** Function Prototypes *****************************/
int XOcp_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen);

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_MAILBOX_H */