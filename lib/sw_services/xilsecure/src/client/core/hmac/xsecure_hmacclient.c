/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file client/core/hmac/xsecure_hmacclient.c
*
* This file contains the implementation of HMAC client interface functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   vss  04/29/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_hmac_client_apis XilSecure HMAC Client APIs
* @{
*/
/*************************************** Include Files ********************************************/
#include "xsecure_hmacclient.h"
#include "xsecure_generic.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/

/************************************ Variable Definitions ****************************************/

/**************************************************************************************************/
/**
 *
 * @brief	This function sends IPI request for HMAC operation.
 *
 * @param	InstancePtr	Pointer to the client instance.
 * @param	ParamsPtr	Pointer to the XSecure_HmacParams structure.
 *
 * @return
 *		 - XST_SUCCESS	On Success.
 *		 - XST_INVALID_PARAM	If any input parameter is invalid.
 *		 - XST_FAILURE	If there is a failure.
 *
 **************************************************************************************************/
int XSecure_HmacOperation(XSecure_ClientInstance *InstancePtr, const XSecure_HmacParams *ParamsPtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 Addr = (u64)(UINTPTR)ParamsPtr;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) || (ParamsPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSECURE_PACK_PAYLOAD3(Payload, XSECURE_API_HMAC_OPERATION, (u32)Addr,
			      (u32)(Addr >> XSECURE_ADDR_HIGH_SHIFT),
			      (u32)sizeof(XSecure_HmacParams));

	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}
/** @} */
