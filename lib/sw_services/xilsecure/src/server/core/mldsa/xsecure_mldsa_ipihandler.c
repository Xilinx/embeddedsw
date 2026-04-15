/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/mldsa/xsecure_mldsa_ipihandler.c
*
* This file contains the xilsecure MLDSA IPI handlers implementation.
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
* @addtogroup xsecure_mldsa_server_apis XilSecure MLDSA Server APIs
* @{
*/
/****************************************** Include Files *****************************************/
#include "xsecure_defs.h"
#include "xil_sutil.h"
#include "xsecure_mldsa_ipihandler.h"
#include "xsecure_mldsa.h"
#include "xsecure_error.h"
#include "xplmi_dma.h"

/*************************************** Constant Definitions *************************************/

/*************************************** Function Prototypes **************************************/
static int XSecure_MldsaSignVerifyIpi(u32 MldsaParamAddrLow, u32 MldsaParamAddrHigh);
static int XSecure_MldsaSignGenerateIpi(u32 MldsaParamAddrLow, u32 MldsaParamAddrHigh);

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
int XSecure_MldsaIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	const u32 *Pload = NULL;

	if ((Cmd == NULL) || (Cmd->Payload == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_MLDSA_SIGN_VERIFY):
		Status = XSecure_MldsaSignVerifyIpi(Pload[0U], Pload[1U]);
		break;
	case XSECURE_API(XSECURE_API_MLDSA_SIGN_GENERATE):
		Status = XSecure_MldsaSignGenerateIpi(Pload[0U], Pload[1U]);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function handler calls XSecure_MldsaSignVerify server API.
 *
 * @param	MldsaParamAddrLow	Lower 32 bit address of the XSecure_MldsaSignVerifyParams
 * 					structure.
 * @param	MldsaParamAddrHigh	Higher 32 bit address of the XSecure_MldsaSignVerifyParams
 * 					structure.
 *
 * @return
 *		 - XST_SUCCESS If the MLDSA signature verification is successful.
 *		 - XST_FAILURE If there is a failure.
 *
 **************************************************************************************************/
static int XSecure_MldsaSignVerifyIpi(u32 MldsaParamAddrLow, u32 MldsaParamAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 MldsaParamAddr = ((u64)MldsaParamAddrHigh << XSECURE_ADDR_HIGH_SHIFT) |
				(u64)MldsaParamAddrLow;
	XSecure_MldsaSignVerifyParams MldsaParams;

	Status = XPlmi_MemCpy64((UINTPTR)&MldsaParams, MldsaParamAddr,
				sizeof(XSecure_MldsaSignVerifyParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_MldsaSignVerify(&MldsaParams);

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function handler calls XSecure_MldsaSignGenerate server API.
 *
 * @param	MldsaParamAddrLow	Lower 32 bit address of the XSecure_MldsaSignGenParams
 * 					structure.
 * @param	MldsaParamAddrHigh	Higher 32 bit address of the XSecure_MldsaSignGenParams
 * 					structure.
 *
 * @return
 *		 - XST_SUCCESS If the MLDSA signature generation is successful.
 *		 - XST_FAILURE If there is a failure.
 *
 **************************************************************************************************/
static int XSecure_MldsaSignGenerateIpi(u32 MldsaParamAddrLow, u32 MldsaParamAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 MldsaParamAddr = ((u64)MldsaParamAddrHigh << XSECURE_ADDR_HIGH_SHIFT) |
				(u64)MldsaParamAddrLow;
	XSecure_MldsaSignGenParams MldsaParams;

	Status = XPlmi_MemCpy64((UINTPTR)&MldsaParams, MldsaParamAddr,
				sizeof(XSecure_MldsaSignGenParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_MldsaSignGenerate(&MldsaParams);

END:
	return Status;
}
/** @} */
