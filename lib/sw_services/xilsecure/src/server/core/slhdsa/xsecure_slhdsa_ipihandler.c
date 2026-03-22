/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file slhdsa/xsecure_slhdsa_ipihandler.c
*
* This file contains the xilsecure SLHDSA IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  03/20/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_slhdsa_server_apis XilSecure SLHDSA Server APIs
* @{
*/
/****************************************** Include Files *****************************************/
#include "xsecure_defs.h"
#include "xil_sutil.h"
#include "xsecure_slhdsa_ipihandler.h"
#include "xsecure_slhdsa.h"
#include "xsecure_error.h"
#include "xsecure_init.h"
#include "xplmi_dma.h"
#include "xsecure_resourcehandling.h"

/*************************************** Constant Definitions *************************************/

/*************************************** Function Prototypes **************************************/
static int XSecure_SlhdsaSignVerifyIpi(u32 SlhdsaParamAddrLow, u32 SlhdsaParamAddrHigh);

/*************************************** Function Definitions *************************************/

/**************************************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID.
 *
 * @param	Cmd	Pointer to the command structure.
 *
 * @return
 *		 - XST_SUCCESS If the handler execution is successful.
 *		 - XST_INVALID_PARAM If any input parameter is invalid.
 *		 - XST_FAILURE If there is a failure.
 *
 **************************************************************************************************/
int XSecure_SlhdsaIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	const u32 *Pload = NULL;
	XPlmi_CoreType Core =  XPLMI_SHA3_CORE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	if ((Cmd == NULL) || (Cmd->Payload == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** SHA IPI event handling */
	Status = XSecure_IpiEventHandling(Cmd, Core);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}


	Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_SLHDSA_SIGN_VERIFY):
		Status = XSecure_SlhdsaSignVerifyIpi(Pload[0U], Pload[1U]);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	if(ShaInstPtr->ShaState == XSECURE_SHA_INITIALIZED) {
		SStatus = XSecure_MakeResFree(Core);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function handler calls XSecure_SlhdsaVerify server API.
 *
 * @param	SlhdsaParamAddrLow	Lower 32 bit address of the XSecure_SlhdsaInputParams
 * 					structure.
 * @param	SlhdsaParamAddrHigh	Higher 32 bit address of the XSecure_SlhdsaInputParams
 * 					structure.
 *
 * @return
 *		 - XST_SUCCESS If the SLHDSA signature verification is successful.
 *		 - XST_FAILURE If there is a failure.
 *
 **************************************************************************************************/
static int XSecure_SlhdsaSignVerifyIpi(u32 SlhdsaParamAddrLow, u32 SlhdsaParamAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 SlhdsaParamAddr = ((u64)SlhdsaParamAddrHigh << XSECURE_ADDR_HIGH_SHIFT) |
				(u64)SlhdsaParamAddrLow;
	XSecure_SlhdsaInputParams SlhdsaParams;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	Status = XPlmi_MemCpy64((UINTPTR)&SlhdsaParams, SlhdsaParamAddr,
				sizeof(XSecure_SlhdsaInputParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_SlhdsaVerify(ShaInstPtr, &SlhdsaParams);

END:
	return Status;
}
/** @} */
