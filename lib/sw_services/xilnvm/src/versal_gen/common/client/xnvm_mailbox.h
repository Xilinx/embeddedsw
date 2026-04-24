/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_mailbox.h
*
* This file contains declarations of xilmailbox generic interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/21 Initial release
* 1.1   kpt  01/13/21 Added macro XNVM_SHARED_MEM_SIZE
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 3.1   skg  10/04/22 Added SlrIndex as member to XNvm_ClientInstance
*       skg  10/28/22 Added comments for macros
* 3.2   am   03/09/23 Moved payload length macros to xilmailbox.h file
*       am   03/21/23 Match the shared memory size in secure library to reuse for customer
*       kal  09/14/23 Added XNvm_SetSlrIndex function
*       vss  09/19/23 Fixed MISRA-C 12.2 violation
* 3.3   tri  10/10/23 Fixed MISRA-C 5.5 violation
*       ng   11/22/23 Fixed doxygen grouping
* 3.4   obs  02/18/25 Fixed IPI message length
* 3.7   mb   04/23/26 Remove unused macros
*
* </pre>
*
******************************************************************************/
#ifndef XNVM_MAILBOX_H
#define XNVM_MAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/

/**
 * @cond xnvm_internal
 * @{
 */
/* Payload Packets */
#define XNVM_PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5)		\
	Payload[0U] = (u32)Arg0;						\
	Payload[1U] = (u32)Arg1;						\
	Payload[2U] = (u32)Arg2;						\
	Payload[3U] = (u32)Arg3;						\
	Payload[4U] = (u32)Arg4;						\
	Payload[5U] = (u32)Arg5;

#define XNVM_PACK_PAYLOAD0(Payload, ApiId) \
	XNVM_PACK_PAYLOAD(Payload, Header(0UL, (ApiId)), 0U, 0U, 0U, 0U, 0U)
#define XNVM_PACK_PAYLOAD1(Payload, ApiId, Arg1) \
	XNVM_PACK_PAYLOAD(Payload, Header(1UL, (ApiId)), (Arg1), 0U, 0U, 0U, 0U)
#define XNVM_PACK_PAYLOAD2(Payload, ApiId, Arg1, Arg2) \
	XNVM_PACK_PAYLOAD(Payload, Header(2UL, (ApiId)), (Arg1), (Arg2), 0U, 0U, 0U)
#define XNVM_PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2, Arg3) \
	XNVM_PACK_PAYLOAD(Payload, Header(3UL, (ApiId)), (Arg1), (Arg2), (Arg3), 0U, 0U)
#define XNVM_PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
	XNVM_PACK_PAYLOAD(Payload, Header(4UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), 0U)
#define XNVM_PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
	XNVM_PACK_PAYLOAD(Payload, Header(5UL, (ApiId)), (Arg1), (Arg2), (Arg3), (Arg4), (Arg5))

#define XILNVM_MODULE_ID			(11U)

#define XNVM_ADDR_HIGH_SHIFT	(32U)	/**< Shift to get upper 32 bits of address */
/* 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT			XIPIPSU_MAX_MSG_LEN
/* 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		XIPIPSU_MAX_MSG_LEN
/**< Target PMC IPI interrupt mask */
#define XNVM_TARGET_IPI_INT_MASK	(0x00000002U)
/**< Module Id shift*/
#define XNVM_MODULE_ID_SHIFT		(8U)
/**< Length shift mask*/
#define XNVM_PAYLOAD_LEN_SHIFT		(16U)
/**< Module id mask*/
#define XILNVM_MODULE_ID_MASK		((u32)XILNVM_MODULE_ID << XNVM_MODULE_ID_SHIFT)

/**< Max size of shared memory used to store the CDO command */
#define XNVM_SHARED_MEM_SIZE		(160U)

/**************************** Type Definitions *******************************/
/**< xilnvm client instance*/
typedef struct {
	XMailbox *MailboxPtr; /**< pointer to mailbox for IPI communication*/
	u32 SlrIndex;         /**< Slr index to trigger the slave PLM*/
} XNvm_ClientInstance;

/**< Enumeration constants for SlrIndex*/
typedef enum{
	XNVM_SLR_IDX_0 = 0,	/**< SLR_INDEX_0 */
	XNVM_SLR_IDX_1,	/**< SLR_INDEX_1 */
	XNVM_SLR_IDX_2,	/**< SLR_INDEX_2 */
	XNVM_SLR_IDX_3	/**< SLR_INDEX_3 */
} XNvm_SlrIndex;

/***************** Macros (Inline Functions) Definitions *********************/
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
static inline u32 Header(u32 Len, u32 ApiId)
{
	(void)Len;
	return (XILNVM_MODULE_ID_MASK | (ApiId));
}
#else
static inline u32 Header(u32 Len, u32 ApiId)
{
	return ((Len << XNVM_PAYLOAD_LEN_SHIFT) |
		XILNVM_MODULE_ID_MASK | (ApiId));
}
#endif
/******************************************************************************/
/**
 * @brief	This function sets slr index in the NVM client instance.
 *
 * @param	InstancePtr	Pointer to XNvm_ClientInstance
 *
 * @param	SlrIndex	Slr index to be set in instance
 *
 * @return
 * 		- XST_SUCCESS On valid input SlrIndex.
 * 		- XST_FAILURE On invalid SlrIndex.
 *
 * @note	This function is applicable to only Versal
 *
 *******************************************************************************/
static inline int XNvm_SetSlrIndex(XNvm_ClientInstance *InstancePtr, u32 SlrIndex)
{
	int Status = XST_FAILURE;

	if (SlrIndex <= (u32)XNVM_SLR_IDX_3) {
		/**
		 * Validate SlrIndex and assign it to instance pointer
		 */
		InstancePtr->SlrIndex = SlrIndex;
		Status = XST_SUCCESS;
	}

	return Status;
}

/**
 * @}
 * @endcond
 */
/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XNvm_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen);
int XNvm_ClientInit(XNvm_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_MAILBOX_H */
