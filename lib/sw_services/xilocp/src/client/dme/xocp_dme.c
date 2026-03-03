/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_dme.c
* @addtogroup xocp_dme_client_apis XilOcp DME Client APIs
* @{
*
* This file contains the implementation of the client interface functions for DME API's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------------------------------
* 1.7   rpu  02/18/26 Initial release
*
* </pre>
*
* @note
*
***************************************************************************************************/

/*************************************** Include Files ********************************************/
#include "xocp_dme.h"

/************************************* Constant Definitions ***************************************/

/*************************************** Type Definitions *****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/************************************** Function Prototypes ***************************************/

/************************************** Variable Definitions **************************************/

/************************************** Function Definitions **************************************/

/**************************************************************************************************/
/**
 * @brief   This function sends IPI request to fill the DME structure and
 *          generates the response with signature
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   NonceAddr - pointer to 48 bytes buffer which holds the Nonce,
 *          which shall be used to fill one of the member of DME structure
 * @param   DmeStructResAddr - pointer to 224 bytes buffer, which is used to
 *          store the response DME structure of type XOcp_DmeResponseStructure
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - XST_FAILURE - Upon any failure
 *
 **************************************************************************************************/
int XOcp_GenDmeResp(XOcp_ClientInstance *InstancePtr, u64 NonceAddr,
	u64 DmeStructResAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_GENDMERESP command and send the request to Server */
	XOCP_PACK_PAYLOAD4(Payload, XOCP_API_GENDMERESP, NonceAddr,
				(NonceAddr >> XOCP_ADDR_HIGH_SHIFT),
				DmeStructResAddr,
				(DmeStructResAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}
/** @} */