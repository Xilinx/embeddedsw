/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/versal_2vp/xsecure_plat_kat_ipihandler.c
*
* This file contains the Xilsecure KAT IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  11/14/25 Initial release
* 5.8   mb   09/24/26 Moved XSECURE_API_TRNG_KAT to XSecure_TrngIpiHandler
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

/*************************************** Constant Definitions *************************************/

/***************************************** Function Prototypes ************************************/

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

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Common IPI handler for versal devices */
	Status = XSecure_KatIpiHandler(Cmd);

END:
	return Status;
}
/** @} */
