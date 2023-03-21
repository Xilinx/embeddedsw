/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.1   am   03/09/23 Moved payload length macros to xilmailbox.h file
*       am   03/21/23 Match the shared memory size in secure library to reuse for customer
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

#define HEADER(len, ApiId) ((len << 16U) | (XILSECURE_MODULE_ID << 8U) | ((u32)ApiId))
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

/**
 * @}
 * @endcond
 */
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_ProcessMailbox(XMailbox *MailboxPtr, u32 *MsgPtr, u32 MsgLen);
int XSecure_ClientInit(XSecure_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_MAILBOX_H */
