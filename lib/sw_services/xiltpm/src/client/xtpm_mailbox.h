/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm_mailbox.h
*
* This file contains declarations of xilTPM generic interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pre  03/09/26 Initial release
*
* </pre>
*
******************************************************************************/
#ifndef XTPM_MAILBOX_H
#define XTPM_MAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/**
* @addtogroup xtpm_mailbox_apis XilTPM mailbox APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/

/**
 * @cond xtpm_internal
 * @{
 */
/* Payload Packets */
#define XTPM_PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5)		\
	Payload[0U] = (u32)Arg0;						\
	Payload[1U] = (u32)Arg1;						\
	Payload[2U] = (u32)Arg2;						\
	Payload[3U] = (u32)Arg3;						\
	Payload[4U] = (u32)Arg4;						\
	Payload[5U] = (u32)Arg5;

#define XTPM_PACK_PAYLOAD0(Payload, ApiId) \
	XTPM_PACK_PAYLOAD(Payload, Header(0UL, (ApiId)), 0U, 0U, 0U, 0U, 0U)
#define XTPM_PACK_PAYLOAD1(Payload, ApiId, Arg1) \
	XTPM_PACK_PAYLOAD(Payload, Header(1UL, (ApiId)), (Arg1), 0U, 0U, 0U, 0U)
#define XTPM_PACK_PAYLOAD2(Payload, ApiId, Arg1, Arg2) \
	XTPM_PACK_PAYLOAD(Payload, Header(2UL, (ApiId)), (Arg1), (Arg2), 0U, 0U, 0U)
#define XTPM_PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2, Arg3) \
	XTPM_PACK_PAYLOAD(Payload, Header(3UL, (ApiId)), (Arg1), (Arg2), (Arg3), 0U, 0U)
#define XTPM_PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
	XTPM_PACK_PAYLOAD(Payload, Header(4UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), 0U)
#define XTPM_PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
	XTPM_PACK_PAYLOAD(Payload, Header(5UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), (Arg5))

#define XILTPM_MODULE_ID			(15U)

#define XTPM_ADDR_HIGH_SHIFT	(32U)	/**< Shift to get upper 32 bits of address */
/* 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT			XIPIPSU_MAX_MSG_LEN
/* 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		XIPIPSU_MAX_MSG_LEN
/**< Target PMC IPI interrupt mask */
#define XTPM_TARGET_IPI_INT_MASK	(0x00000002U)
/**< Module Id shift*/
#define XTPM_MODULE_ID_SHIFT		(8U)
/**< Length shift mask*/
#define XTPM_PAYLOAD_LEN_SHIFT		(16U)
/**< Module id mask*/
#define XILTPM_MODULE_ID_MASK		((u32)XILTPM_MODULE_ID << XTPM_MODULE_ID_SHIFT)

/**< Max size of shared memory used */
#define XTPM_SHARED_MEM_SIZE		(160U)

/**************************** Type Definitions *******************************/
/**< xilTPM client instance*/
typedef struct {
	XMailbox *MailboxPtr; /**< pointer to mailbox for IPI communication*/
	u32 SlrIndex;         /**< Slr index to trigger the slave PLM*/
} XTpm_ClientInstance;

/***************** Macros (Inline Functions) Definitions *********************/

static inline u32 Header(u32 Len, u32 ApiId)
{
	return ((Len << XTPM_PAYLOAD_LEN_SHIFT) |
		XILTPM_MODULE_ID_MASK | (ApiId));
}

/**
 * @}
 * @endcond
 */
/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XTpm_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen);
int XTpm_ClientInit(XTpm_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);

/** @} End of xtpm_mailbox_apis group */

#ifdef __cplusplus
}
#endif

#endif  /* XTPM_MAILBOX_H */
