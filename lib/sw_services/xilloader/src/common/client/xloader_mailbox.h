/**************************************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
 *       har  03/05/24 Fixed doxygen warnings
 * 1.01  pre  08/21/24 Added SlrIndex in XLoader_ClientInstance structure
 * 1.02  obs  02/18/25 Fixed IPI message length
 * 2.4   gnr  03/18/26 Updated the Payload assignments with XLOADER_PACK_PAYLOAD macros
 * 2.4   sms  04/16/26 Updated the Payload and Response buffer length parameters in the function prototypes
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
#define PAYLOAD_ARG_CNT				XIPIPSU_MAX_MSG_LEN
						/**< 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		   	XIPIPSU_MAX_MSG_LEN
						/**< 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define XLOADER_TARGET_IPI_INT_MASK	(0x00000002U) /**< Target PMC IPI interrupt mask */
#define XLOADER_MODULE_ID_SHIFT		(8U) /**< Module id shift */
#define XLOADER_PAYLOAD_LEN_SHIFT	(16U) /**< Length shift mask */
#define XILLOADER_MODULE_ID_MASK	((u32)XILLOADER_MODULE_ID << XLOADER_MODULE_ID_SHIFT)
										/**< Module id mask*/
#define XLOADER_SHARED_MEM_SIZE		(160U)
						/**< Max size of shared memory used to store the CDO command */

/**@cond xloader_internal
 * @{
 */
/**< Payload Packets */
#define XLOADER_PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)      \
    Payload[0U] = (u32)Arg0;                        \
    Payload[1U] = (u32)Arg1;                        \
    Payload[2U] = (u32)Arg2;                        \
    Payload[3U] = (u32)Arg3;                        \
    Payload[4U] = (u32)Arg4;                        \
    Payload[5U] = (u32)Arg5;                        \
    Payload[6U] = (u32)Arg6;

#define XLOADER_PACK_PAYLOAD0(Payload, ApiId) \
    XLOADER_PACK_PAYLOAD(Payload, PACK_XLOADER_HEADER(0UL, (ApiId)), 0U, 0U, 0U, 0U, 0U, 0U)
#define XLOADER_PACK_PAYLOAD1(Payload, ApiId, Arg1) \
    XLOADER_PACK_PAYLOAD(Payload, PACK_XLOADER_HEADER(1UL, (ApiId)), (Arg1), 0U, 0U, 0U, 0U, 0U)
#define XLOADER_PACK_PAYLOAD2(Payload, ApiId, Arg1, Arg2) \
    XLOADER_PACK_PAYLOAD(Payload, PACK_XLOADER_HEADER(2UL, (ApiId)), (Arg1), (Arg2), 0U, 0U, 0U, 0U)
#define XLOADER_PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2, Arg3) \
    XLOADER_PACK_PAYLOAD(Payload, PACK_XLOADER_HEADER(3UL, (ApiId)), (Arg1), (Arg2), (Arg3), 0U, 0U, 0U)
#define XLOADER_PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
    XLOADER_PACK_PAYLOAD(Payload, PACK_XLOADER_HEADER(4UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), 0U, 0U)
#define XLOADER_PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
    XLOADER_PACK_PAYLOAD(Payload, PACK_XLOADER_HEADER(5UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), (Arg5), 0U)
#define XLOADER_PACK_PAYLOAD6(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) \
    XLOADER_PACK_PAYLOAD(Payload, PACK_XLOADER_HEADER(6UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), (Arg5), (Arg6))
/** @}
 * @endcond
 */
/************************************** Type Definitions *****************************************/

/**
 * Structure to hold the client instance information for xilloader
 */
typedef struct {
	XMailbox *MailboxPtr; /**< Pointer to mailbox for IPI communication */
	u32 Response[RESPONSE_ARG_CNT];		/**< Buffer to store the response of the IPI */
	u32 SlrIndex; /**< SLR index number */
} XLoader_ClientInstance;

/*************************** Macros (Inline Functions) Definitions *******************************/
/*************************************************************************************************/
/**
 * @brief	This function creates the header for the command sent to xilloader module
 *
 * @param	Len		Length of Payload
 * @param	ApiId		API ID of the requested service
 *
 * @return
 *		Header of the command
 *
 **************************************************************************************************/
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
static inline u32 PACK_XLOADER_HEADER(u32 Len, u32 ApiId)
{
	(void)Len;
	return (XILLOADER_MODULE_ID_MASK | (ApiId));
}
#else
static inline u32 PACK_XLOADER_HEADER(u32 Len, u32 ApiId)
{
	return ((Len << XLOADER_PAYLOAD_LEN_SHIFT) | XILLOADER_MODULE_ID_MASK | (ApiId));
}
#endif

/************************************ Function Prototypes ****************************************/

int XLoader_ProcessMailbox(XLoader_ClientInstance *ClientPtr, u32 *MsgPtr, u32 MsgLen);
int XLoader_ClientInit(XLoader_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_MAILBOX_H */

/** @} end of xloader_mailbox_apis group */
