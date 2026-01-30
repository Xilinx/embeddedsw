/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file versal_2vp/xsecure_plat_kat_ipihandler.c
*
* This file contains the Xilsecure KAT IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  11/14/25 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis XilSecure KAT Server APIs
* @{
*/
/****************************************** Include Files *****************************************/
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
#include "xsecure_trng.h"

/*************************************** Constant Definitions *************************************/

/***************************************** Function Prototypes ************************************/
#ifndef PLM_SECURE_EXCLUDE
static int XSecure_TrngKat(void);
#endif

/**************************************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID.
 *
 * @param 	Cmd	Pointer to the command structure.
 *
 * @return
 *		 - XST_SUCCESS If the handler execution is successful.
 *		 - XST_INVALID_PARAM If any input parameter is invalid.
 *		 - XST_FAILURE If there is a failure.
 *
 **************************************************************************************************/
int XSecure_KatPlatIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Pload[0U] & XSECURE_API_ID_MASK) {
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_TRNG_KAT):
		Status = XSecure_TrngKat();
#endif
		break;
	default:
		/* Common IPI handler for versal devices */
		Status = XSecure_KatIpiHandler(Cmd);
		break;
	}
END:
	return Status;
}

#ifndef PLM_SECURE_EXCLUDE
/**************************************************************************************************/
/**
 * @brief	This function handler calls XSecure_TrngPreOperationalSelfTests server API and
 * 		updates KAT status to XPLMI_RTCFG_PLM_KAT_ADDR address.
 *
 * @return
 *		 - XST_SUCCESS If the KAT is successful.
 *		 - XST_FAILURE If there is a failure.
 *
 **************************************************************************************************/
static int XSecure_TrngKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();

	Status = XSecure_PreOperationalSelfTests(TrngInstance);
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
/** @} */
