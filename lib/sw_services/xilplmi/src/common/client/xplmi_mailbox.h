/**************************************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
 * 1.01  pre  07/10/24 Added SLR index in client data structure
 * 1.02  obs  02/18/25 Fixed IPI message length
 * 2.4   gnr  03/18/26 Updated the Payload assignments with XPLMI_PACK_PAYLOAD macros
 *       tbk  02/24/26 Added XPLMI_PACK_PAYLOAD macros
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xilplmi_mailbox_apis XilPlmi Mailbox APIs
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
#define PAYLOAD_ARG_CNT			   	XIPIPSU_MAX_MSG_LEN
						/**< 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		   	XIPIPSU_MAX_MSG_LEN
						/**< 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define XPLMI_TARGET_IPI_INT_MASK	(0x00000002U) /**< Target PMC IPI interrupt mask */
#define XPLMI_MODULE_ID_SHIFT		(8U) /**< Module Id shift */
#define XPLMI_PAYLOAD_LEN_SHIFT		(16U) /**< Length shift mask */
#define XILPLMI_MODULE_ID_MASK		((u32)XILPLMI_MODULE_ID << XPLMI_MODULE_ID_SHIFT)
										/**< Module id mask*/
#define XPLMI_SHARED_MEM_SIZE		(160U)
						/**< Max size of shared memory used to store the CDO command */
/**@cond xplmi_internal
 * @{
 */
/**< Payload Packets */
#define XPLMI_PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)      \
    Payload[0U] = (u32)Arg0;                        \
    Payload[1U] = (u32)Arg1;                        \
    Payload[2U] = (u32)Arg2;                        \
    Payload[3U] = (u32)Arg3;                        \
    Payload[4U] = (u32)Arg4;                        \
    Payload[5U] = (u32)Arg5;                        \
    Payload[6U] = (u32)Arg6;

#define XPLMI_PACK_PAYLOAD0(Payload, ApiId) \
    XPLMI_PACK_PAYLOAD(Payload, PACK_XPLMI_HEADER(0UL, (ApiId)), 0U, 0U, 0U, 0U, 0U, 0U)
#define XPLMI_PACK_PAYLOAD1(Payload, ApiId, Arg1) \
    XPLMI_PACK_PAYLOAD(Payload, PACK_XPLMI_HEADER(1UL, (ApiId)), (Arg1), 0U, 0U, 0U, 0U, 0U)
#define XPLMI_PACK_PAYLOAD2(Payload, ApiId, Arg1, Arg2) \
    XPLMI_PACK_PAYLOAD(Payload, PACK_XPLMI_HEADER(2UL, (ApiId)), (Arg1), (Arg2), 0U, 0U, 0U, 0U)
#define XPLMI_PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2, Arg3) \
    XPLMI_PACK_PAYLOAD(Payload, PACK_XPLMI_HEADER(3UL, (ApiId)), (Arg1), (Arg2), (Arg3), 0U, 0U, 0U)
#define XPLMI_PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
    XPLMI_PACK_PAYLOAD(Payload, PACK_XPLMI_HEADER(4UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), 0U, 0U)
#define XPLMI_PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
    XPLMI_PACK_PAYLOAD(Payload, PACK_XPLMI_HEADER(5UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), (Arg5), 0U)
#define XPLMI_PACK_PAYLOAD6(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) \
    XPLMI_PACK_PAYLOAD(Payload, PACK_XPLMI_HEADER(6UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), (Arg5), (Arg6))
/** @}
 * @endcond
 */
/************************************** Type Definitions *****************************************/

/**
 * This structure contains the client instance data for XilPlmi.
 * The user is required to allocate an instance of this structure and
 * initialize it using XPlmi_ClientInit() before using it with other APIs.
 */
typedef struct {
	XMailbox *MailboxPtr; /**< Pointer to mailbox for IPI communication */
	u32 Response[RESPONSE_ARG_CNT]; /**< Response payload buffer */
	u32 SlrIndex; /**< SLR index number */
} XPlmi_ClientInstance;

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
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
static inline u32 PACK_XPLMI_HEADER(u32 Len, u32 ApiId)
{
	(void)Len;
	return (XILPLMI_MODULE_ID_MASK | (ApiId));
}
#else
static inline u32 PACK_XPLMI_HEADER(u32 Len, u32 ApiId)
{
	return ((Len << XPLMI_PAYLOAD_LEN_SHIFT) | XILPLMI_MODULE_ID_MASK | (ApiId));
}
#endif

/************************************ Function Prototypes ****************************************/

int XPlmi_ProcessMailbox(XPlmi_ClientInstance *ClientPtr, u32 *MsgPtr, u32 MsgLen);
int XPlmi_ClientInit(XPlmi_ClientInstance* InstancePtr, XMailbox* MailboxPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_MAILBOX_H */

/** @} end of xilplmi_mailbox_apis group*/
