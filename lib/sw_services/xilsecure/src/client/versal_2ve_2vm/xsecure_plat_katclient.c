/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_katclient.c
*
* This file contains the implementation of the client interface functions for
* KAT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   kpt  07/18/22 Initial release
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_client_apis XilSecure KAT Client APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_plat_katclient.h"
#include "xsecure_plat_defs.h"
#include "xil_sutil.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform TRNG KAT and health tests
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XST_INVALID_PARAM  If any input parameter is invalid.
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_TrngKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_KAT);
	Payload[1U] = (u32)XSECURE_API_TRNG_KAT;

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XSecure_TrngKat
	 * API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/** @} */
