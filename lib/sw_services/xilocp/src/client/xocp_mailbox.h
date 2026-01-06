/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
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
* 1.0   am   12/21/22 Initial release
* 1.1   obs  02/18/25 Fixed IPI message length
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

/**@cond xocp_internal
 * @{
 */
#define XILOCP_MODULE_ID			(13U)

/* Payload Packets */
#define XOCP_PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)	\
	Payload[0U] = (u32)Arg0;						\
	Payload[1U] = (u32)Arg1;						\
	Payload[2U] = (u32)Arg2;						\
	Payload[3U] = (u32)Arg3;						\
	Payload[4U] = (u32)Arg4;						\
	Payload[5U] = (u32)Arg5;						\
	Payload[6U] = (u32)Arg6;

#define XOCP_PACK_PAYLOAD0(Payload, ApiId) \
	XOCP_PACK_PAYLOAD(Payload, OcpHeader(0UL, (ApiId)), 0U, 0U, 0U, 0U, 0U, 0U)
#define XOCP_PACK_PAYLOAD1(Payload, ApiId, Arg1) \
	XOCP_PACK_PAYLOAD(Payload, OcpHeader(1UL, (ApiId)), (Arg1), 0U, 0U, 0U, 0U, 0U)
#define XOCP_PACK_PAYLOAD2(Payload, ApiId, Arg1, Arg2) \
	XOCP_PACK_PAYLOAD(Payload, OcpHeader(2UL, (ApiId)), (Arg1), (Arg2), 0U, 0U, 0U, 0U)
#define XOCP_PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2, Arg3) \
	XOCP_PACK_PAYLOAD(Payload, OcpHeader(3UL, (ApiId)), (Arg1), (Arg2), (Arg3), 0U, 0U, 0U)
#define XOCP_PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
	XOCP_PACK_PAYLOAD(Payload, OcpHeader(4UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), 0U, 0U)
#define XOCP_PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
	XOCP_PACK_PAYLOAD(Payload, OcpHeader(5UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), (Arg5), 0U)
#define XOCP_PACK_PAYLOAD6(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) \
	XOCP_PACK_PAYLOAD(Payload, OcpHeader(6UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), (Arg5), (Arg6))

/** 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT			XIPIPSU_MAX_MSG_LEN
/** 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT			XIPIPSU_MAX_MSG_LEN
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
/** @}
 * @endcond
 */

/**************************** Type Definitions *******************************/
typedef struct {
	XMailbox *MailboxPtr;	/**< Mailbox pointer */
} XOcp_ClientInstance;

/***************** Macros (Inline Functions) Definitions *********************/
/** @cond xocp_internal
 * @{
 */
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
/** @} */