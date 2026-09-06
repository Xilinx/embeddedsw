/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_client.c
*
* This file contains the implementation of the client interface functions for
* OCP hardware interface APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*       am   01/10/23 Added client side API for dme
* 1.2   kpt  06/02/23 Updated XOcp_GetHwPcrLog
*       kal  06/02/23 Added client side API for SW PCR
* 1.3   kal  12/09/23 Added a check for DataAddr if size > 48 bytes SWPCR
*       am   02/06/24 Fixed Doxygen warning
* 1.7   rpu  02/18/26 Refactor OCP client library
* 1.8   tus  08/07/26 Added XOcp_GetClientLibVersion and
*                     XOcp_GetServerLibInfo
* </pre>
*
* @note
*
******************************************************************************/

/**
 * @addtogroup xocp_mailbox_apis XilOcp Mailbox APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xocp_client.h"
#include "xocp_ver.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* @brief    This function sets the instance of mailbox
*
* @param    InstancePtr - Pointer to the client instance
* @param    MailboxPtr - Pointer to the mailbox instance
*
* @return
*           - XST_SUCCESS - On successful initialization
*           - XST_FAILURE - On failure
*
******************************************************************************/
int XOcp_ClientInit(XOcp_ClientInstance* const InstancePtr,
	XMailbox* const MailboxPtr)
{
	int Status = XST_FAILURE;

	/**
	 * - Uses XMailbox instance to initiate the communication between
	 * client and server.
	 */
	if (InstancePtr != NULL) {
		InstancePtr->MailboxPtr = MailboxPtr;
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief    This function returns the XilOcp library version that the client is built with
*
* @param    Version - Pointer to hold the 32-bit library version, where the upper 16 bits hold
*           the major version and the lower 16 bits hold the minor version
*
* @return
*           - XST_SUCCESS - If the library version is read successfully
*           - XST_INVALID_PARAM - If Version is NULL
*
******************************************************************************/
int XOcp_GetClientLibVersion(u32 *Version)
{
	int Status = XST_FAILURE;

	if (Version == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	*Version = XOcp_GetLibVersion();
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief    This function sends an IPI request to get the XilOcp library version and capability
*           bits running on the PLM
*
* @param    InstancePtr - Pointer to the client instance
* @param    LibInfo - Pointer to Xil_LibInfo. LibVersion and Capability are filled on
*           success. The Version and NistStatus fields are reserved for XilOcp
*           and must be ignored.
*
* @return
*           - XST_SUCCESS - If the library information is read successfully
*           - XST_INVALID_PARAM - If any input parameter is invalid
*           - XST_FAILURE - On failure
*
******************************************************************************/
int XOcp_GetServerLibInfo(XOcp_ClientInstance *InstancePtr, Xil_LibInfo *LibInfo)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/**
	 * - Performs input parameter validation.
	 */
	if ((InstancePtr == NULL) || (LibInfo == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Query library info via the Features command using a valid OCP api-id as
	 *   the selector, so the command succeeds and the server returns the library
	 *   version/capability at fixed indices.
	 */
	XOCP_PACK_PAYLOAD1(Payload, (u32)XOCP_API_FEATURES, (u32)XOCP_API_GENDMERESP);

	/**
	 * - Sends the request to the server and copies the IPI payload after the status word.
	 */
	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT, Response);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * This transport does not shift the response (status stays at Response[0]),
	 * so the data starts at Response[1] and matches Xil_LibInfo. Copy it in one
	 * shot; the server does not emit Capability[1], so zero it afterwards.
	 * Version/NistStatus are reserved for XilOcp.
	 */
	Status = Xil_SMemCpy(LibInfo, sizeof(Xil_LibInfo), &Response[XIL_LIBINFO_RAW_RESP_BASE],
			(u32)(XIL_LIBINFO_RESP_WORDS * sizeof(u32)),
			(u32)(XIL_LIBINFO_RESP_WORDS * sizeof(u32)));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	LibInfo->Capability[1U] = 0U;

END:
	return Status;
}
/** @} */
