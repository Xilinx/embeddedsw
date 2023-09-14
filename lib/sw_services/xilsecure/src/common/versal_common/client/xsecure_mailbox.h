/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_mailbox.h
* @addtogroup xsecure_mailbox_apis XilSecure Mailbox APIs
* @{
*
* @cond xsecure_internal
* This file contains declarations of xilmailbox generic interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       har  04/14/21 Renamed XSecure_ConfigIpi as XSecure_SetIpi
*                     Added XSecure_InitializeIpi
*       am   05/22/21 Resolved MISRA C violation
* 4.6   har  07/14/21 Fixed doxygen warnings
* 4.7   kpt  01/13/22 Added macro XSECURE_SHARED_MEM_SIZE
*       am   03/08/22 Fixed MISRA C violations
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.2   am   03/09/23 Moved payload length macros to xilmailbox.h file
*       am   03/21/23 Match the shared memory size in secure library to reuse for customer
* 	yog  05/03/23 Fixed MISRA C violation of Rule 12.2
*       kal  09/14/23 Added XSecure_SetSlrIndex function
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XSECURE_MAILBOX_H
#define XSECURE_MAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/
/**@cond xsecure_internal
 * @{
 */
#define XILSECURE_MODULE_ID			(0x05U)
				/**< Module ID for xilsecure */

#define HEADER(len, ApiId) (((u32)len << 16U) | ((u32)XILSECURE_MODULE_ID << 8U) | ((u32)ApiId))
				/**< Header for XilSecure Commands */

#define PAYLOAD_ARG_CNT			(8U)
	/**< 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */

#define RESPONSE_ARG_CNT		(8U)
	/**< 1 for status + 3 for values + 3 for reserved + 1 for CRC */

#define XSECURE_TARGET_IPI_INT_MASK	(0x00000002U)
					/**< Target PMC IPI interrupt mask */

#define XSECURE_IPI_UNUSED_PARAM	(0U)
					/**< Unused param */

/* Maximum size of shared memory used to store the CDO command */
#define XSECURE_SHARED_MEM_SIZE		(160U)
					/**< Shared memory size */

/**************************** Type Definitions *******************************/
typedef struct {
	XMailbox *MailboxPtr;
	u32 SlrIndex;
} XSecure_ClientInstance;

/**< Enumeration constants for SlrIndex*/
typedef enum{
	XSECURE_SLR_INDEX_0 = 0,/**< SLR_INDEX_0 */
	XSECURE_SLR_INDEX_1,	/**< SLR_INDEX_1 */
	XSECURE_SLR_INDEX_2,	/**< SLR_INDEX_2 */
	XSECURE_SLR_INDEX_3	/**< SLR_INDEX_3 */
} XSecure_SlrIndex;

/**
 * @}
 * @endcond
 */
/***************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 * @brief	This function sets slr index in the Secure client instance.
 *
 * @param	InstancePtr	Pointer to XSecure_ClientInstance
 *
 * @param	SlrIndex	Slr index to be set in instance
 *
 * @return	XST_SUCCESS	On valid input SlrIndex.
 *		XST_FAILURE	On invalid SlrIndex.
 *
 * @Note	This function is applicable to only Versal
 *
 *******************************************************************************/
static inline int XSecure_SetSlrIndex(XSecure_ClientInstance *InstancePtr, u32 SlrIndex)
{
	int Status = XST_FAILURE;

	if (SlrIndex <= (u32)XSECURE_SLR_INDEX_3) {
		/**< Validate SlrIndex and assign it to instance pointer */
		InstancePtr->SlrIndex = SlrIndex;
		Status = XST_SUCCESS;
	}

	return Status;
}
/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen);
int XSecure_ClientInit(XSecure_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_MAILBOX_H */
