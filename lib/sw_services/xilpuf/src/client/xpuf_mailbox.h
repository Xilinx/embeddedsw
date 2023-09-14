/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_mailbox.h
* @addtogroup xpuf_mailbox_apis XilPuf Mailbox APIs
* @{
*
* @cond xpuf_internal
* This file contains declarations of xilmailbox generic interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 2.2   am   03/09/23 Moved payload length macros to xilmailbox.h file
*       kal  09/14/23 Added XPuf_SetSlrIndex function
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XPUF_MAILBOX_H
#define XPUF_MAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/

/**@cond xpuf_internal
 * @{
 */
#define XILPUF_MODULE_ID			(12U)

/* 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT			(8U)
/* 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT			(8U)
#define XPUF_TARGET_IPI_INT_MASK	(0x00000002U)
#define XPUF_MODULE_ID_SHIFT		(8U)
#define XPUF_PAYLOAD_LEN_SHIFT		(16U)
#define XILPUF_MODULE_ID_MASK		(XILPUF_MODULE_ID << XPUF_MODULE_ID_SHIFT)

/**************************** Type Definitions *******************************/
typedef struct {
	XMailbox *MailboxPtr;
	u32 SlrIndex;
} XPuf_ClientInstance;

/**< Enumeration constants for SlrIndex*/
typedef enum{
	XPUF_SLR_INDEX_0 = 0,	/**< SLR_INDEX_0 */
	XPUF_SLR_INDEX_1,	/**< SLR_INDEX_1 */
	XPUF_SLR_INDEX_2,	/**< SLR_INDEX_2 */
	XPUF_SLR_INDEX_3	/**< SLR_INDEX_3 */
} XPuf_SlrIndex;

/***************** Macros (Inline Functions) Definitions *********************/

static inline u32 PufHeader(u32 Len, u32 ApiId)
{
	return ((Len << XPUF_PAYLOAD_LEN_SHIFT) |
		XILPUF_MODULE_ID_MASK | (ApiId));
}

/******************************************************************************/
/**
 * @brief	This function sets slr index in the PUF client instance.
 *
 * @param	InstancePtr 	Pointer to XPuf_ClientInstance
 *
 * @param	SlrIndex 	Slr index to be set in instance
 *
 * @return	XST_SUCCESS	On valid input SlrIndex.
 *		XST_FAILURE	On invalid SlrIndex.
 *
 * @Note	This function is applicable to only Versal
 *
 *******************************************************************************/
static inline int XPuf_SetSlrIndex(XPuf_ClientInstance *InstancePtr, u32 SlrIndex)
{
	int Status = XST_FAILURE;

	if(SlrIndex <= (u32)XPUF_SLR_INDEX_3){
		/**< Validate SlrIndex and assign it to instance pointer */
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
int XPuf_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen);

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_MAILBOX_H */
