/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_kat_ipihandler.c
* @addtogroup xsecure_apis XilSecure versal net KAT handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure KAT IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kpt   07/15/2022 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_plat_kat.h"
#include "xsecure_kat_ipihandler.h"
#include "xsecure_plat_kat_ipihandler.h"
#include "xsecure_defs.h"
#include "xsecure_error.h"
#include "xil_util.h"
#include "xsecure_init.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/
/************************** Function Prototypes *****************************/
#ifndef PLM_SECURE_EXCLUDE
static int XSecure_TrngKat(void);
#endif
static int XSecure_UpdateKatStatus(XSecure_KatId KatOp, XSecure_KatId KatId);

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *	-	XST_SUCCESS - If the handler execution is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_KatPlatIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	switch (Pload[0U] & XSECURE_API_ID_MASK) {
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_TRNG_KAT):
		Status = XSecure_TrngKat();
		break;
#endif
	case XSECURE_API(XSECURE_API_KAT_CLEAR):
	case XSECURE_API(XSECURE_API_KAT_SET):
		Status = XSecure_UpdateKatStatus(Pload[1U], Pload[2U]);
		break;
	default:
		/* Common IPI handler for versal devices */
		Status = XSecure_KatIpiHandler(Cmd);
		break;
	}

	return Status;
}

#ifndef PLM_SECURE_EXCLUDE
/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_TrngPreOperationalSelfTests Server API
 *
 * @return	- XST_SUCCESS - If the KAT is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_TrngKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_TrngPreOperationalSelfTests(TrngInstance);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_SECURE_TRNG_KAT_MASK);

	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief       This function sets or clears KAT mask of given KatId
 *
 * @return
	-	XST_SUCCESS - If set or clear is successful
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_UpdateKatStatus(XSecure_KatId KatOp, XSecure_KatId KatId) {
	int Status = XST_FAILURE;
	u32 KatMask = 0U;

	if ((KatOp != XSECURE_API_KAT_CLEAR) && (KatOp != XSECURE_API_KAT_SET)) {
			goto END;
	}

	switch((u32)KatId) {
		case XSECURE_API_CPM5N_AES_XTS:
			KatMask = XPLMI_SECURE_CPM5N_AES_XTS_KAT_MASK;
			break;
		case XSECURE_API_CPM5N_AES_PCI_IDE:
			KatMask = XPLMI_SECURE_CPM5N_PCI_IDE_KAT_MASK;
			break;
		case XSECURE_API_NICSEC_KAT:
			KatMask = XPLMI_SECURE_NICSEC_KAT_MASK;
			break;
		default:
			XSecure_Printf(XSECURE_DEBUG_GENERAL,"Invalid KATId for operation");
			break;
	}
	if (KatMask != 0U) {
		/* Update KAT status in RTC area */
		XSecure_KatOp(KatOp, KatMask);
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
