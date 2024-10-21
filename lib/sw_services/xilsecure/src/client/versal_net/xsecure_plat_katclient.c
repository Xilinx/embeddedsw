/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.2   am   04/01/23 Added XST_INVALID_PARAM error code for invalid parameters
*       am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
* 5.4   yog  04/29/24 Fixed doxygen warnings.
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
static int XSecure_UpdateKatStatus(XSecure_ClientInstance *InstancePtr,
		XSecure_KatOp KatOp, u32 KatMaskLen, u32 *KatMask, u32 ApiId);
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

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to set or clear kat mask of HNIC.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KatOp		Operation to set or clear KAT mask
 * @param	KatMaskLen	Length of the KAT mask
 * @param	KatMask		KAT mask
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_UpdateHnicKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp,
		u32 KatMaskLen, u32 *KatMask)
{
	volatile int Status = XST_FAILURE;

	if (KatMaskLen != XSECURE_MIN_KAT_MASK_LEN) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_UpdateKatStatus(InstancePtr, KatOp, KatMaskLen, KatMask,
				XSECURE_API_UPDATE_HNIC_KAT_STATUS);

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to set or clear kat mask of CPM5N.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KatOp		Operation to set or clear KAT mask
 * @param	KatMaskLen	Length of the KAT mask
 * @param	KatMask		KAT mask
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_UpdateCpm5NKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp,
		u32 KatMaskLen, u32 *KatMask)
{
	volatile int Status = XST_FAILURE;

	if (KatMaskLen != XSECURE_MIN_KAT_MASK_LEN) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_UpdateKatStatus(InstancePtr, KatOp, KatMaskLen,
			KatMask, XSECURE_API_UPDATE_CPM5N_KAT_STATUS);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to set or clear kat mask of PCIDE.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KatOp		Operation to set or clear KAT mask
 * @param	KatMaskLen	Length of the KAT mask
 * @param	KatMask		KAT mask
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_UpdatePcideKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp,
		u32 KatMaskLen, u32 *KatMask)
{
	volatile int Status = XST_FAILURE;

	if (KatMaskLen != XSECURE_MIN_KAT_MASK_LEN) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_UpdateKatStatus(InstancePtr, KatOp, KatMaskLen,
			 KatMask, XSECURE_API_UPDATE_PCIDE_KAT_STATUS);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets or clears KAT mask of PKI.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KatOp		Operation to set or clear KAT mask
 * @param	KatMaskLen	Length of the KAT mask
 * @param	KatMask		Pointer to the KAT mask
 *
 * @return
 *		 - XST_SUCCESS  If set or clear is successful
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_UpdatePkiKatStatus(XSecure_ClientInstance *InstancePtr, XSecure_KatOp KatOp,
	u32 KatMaskLen, u32 *KatMask)
{
	volatile int Status = XST_FAILURE;

	if (KatMaskLen != XSECURE_MAX_KAT_MASK_LEN) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_UpdateKatStatus(InstancePtr, KatOp, KatMaskLen,
			 KatMask, XSECURE_API_UPDATE_PKI_KAT_STATUS);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to set or clear kat mask of HNIC.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	KatOp		Operation to set or clear KAT mask
 * @param	KatMaskLen	Length of the KAT mask
 * @param	KatMask		KAT
 * @param	ApiId		IPI request API ID
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_INVALID_PARAM  If any input parameter is invalid.
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_UpdateKatStatus(XSecure_ClientInstance *InstancePtr,
		XSecure_KatOp KatOp, u32 KatMaskLen, u32 *KatMask, u32 ApiId)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];
	u32 Index;


	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if ((KatOp != XSECURE_API_KAT_SET) && (KatOp != XSECURE_API_KAT_CLEAR)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER((XSECURE_KAT_HDR_LEN + KatMaskLen), ApiId);
	Payload[1U] = (u32)KatOp;

	for (Index = 0U; Index < KatMaskLen; Index++) {
		Payload[2U + Index] = KatMask[Index];
	}

	/**
	 * Send an IPI request to the PLM by using the CDO command to update Kat status in
	 * XSecure_UpdateKatStatusIpiHandler API and returns the status of the IPI response.
	 */
	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));
END:
	return Status;
}
/** @} */
