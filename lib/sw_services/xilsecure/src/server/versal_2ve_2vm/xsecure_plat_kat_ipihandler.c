/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_kat_ipihandler.c
*
* This file contains the Xilsecure KAT IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.4   kal  07/24/2024 Initial release
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis Xilsecure KAT Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_plat_kat.h"
#include "xsecure_kat_ipihandler.h"
#include "xsecure_plat_kat_ipihandler.h"
#include "xsecure_defs.h"
#include "xsecure_error.h"
#include "xil_sutil.h"
#include "xsecure_init.h"
#include "xplmi.h"
#include "xsecure_resourcehandling.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
#ifndef PLM_SECURE_EXCLUDE
static int XSecure_TrngKat(void);
#endif
static int XSecure_Sha2Kat(void);

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd	is pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  If the handler execution is successful
 *		 - XST_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_KatPlatIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	u32 *Pload = NULL;
	XSecure_Sha *XSecureSha2InstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Pload[0U] & XSECURE_API_ID_MASK) {
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_TRNG_KAT):
		Status = XSecure_TrngKat();
		break;
#endif
	case XSECURE_API(XSECURE_API_SHA2_KAT):
		/**   - @ref XSecure_ShaKat */
		/** SHA IPI event handling */
		Status = XSecure_IpiEventHandling(Cmd, XPLMI_SHA2_CORE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_Sha2Kat();
		break;
	default:
		/* Common IPI handler for versal devices */
		Status = XSecure_KatIpiHandler(Cmd);
		break;
	}
END:
	if (XSecureSha2InstPtr->ShaState == XSECURE_SHA_INITIALIZED) {
		SStatus = XSecure_MakeResFree(XPLMI_SHA2_CORE);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}
	return Status;
}

#ifndef PLM_SECURE_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function handler calls XSecure_TrngPreOperationalSelfTests
 * 		server API and updates KAT status to XPLMI_RTCFG_PLM_KAT_ADDR
 * 		address.
 *
 * @return
 *		 - XST_SUCCESS  If the KAT is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_TrngKat(void)
{
	volatile int Status = XST_FAILURE;
	XTrngpsx_Instance *TrngInstance = XSecure_GetTrngInstance();

	Status = XTrngpsx_PreOperationalSelfTests(TrngInstance);
	/* Update KAT status in to RTC area */
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_TRNG_KAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
	}

	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function handler calls XSecure_Sha2256Kat server API and
 * 		and updates KAT status to XPLMI_RTCFG_PLM_KAT_ADDR address.
 *
 * @return
 *		 - XST_SUCCESS  If the SHA2 KAT is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_Sha2Kat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);

	/* Run and update KAT status in to RTCA area */
	Status = XSecure_Sha2256Kat(ShaInstPtr);
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SHA2_256_KAT_MASK);
	} else {
		XPlmi_SetKatMask(XPLMI_SHA2_256_KAT_MASK);
	}

	return Status;
}
