/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_ipihandler.c
* @addtogroup xsecure_apis XilSecure versal net platform handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure versalnet IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.1  kpt   01/13/2023 Initial release
* 5.2  vns   07/06/2023 Separated IPI commands of Update Crypto Status
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_defs.h"
#include "xil_util.h"
#include "xplmi_plat.h"
#include "xsecure_plat_ipihandler.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/

static int XSecure_UpdateCryptoMask(XSecure_CryptoStatusOp CryptoOp, u32 CryptoMask, u32 CryptoVal);

/*****************************************************************************/
/**
 * @brief   This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *	-	XST_SUCCESS - If the handler execution is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_PlatIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;
	u32 CryptoMask;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_UPDATE_HNIC_CRYPTO_STATUS):
		CryptoMask = XPLMI_SECURE_HNIC_AES_MASK;
		break;
	case XSECURE_API(XSECURE_API_UPDATE_CPM5N_CRYPTO_STATUS):
		CryptoMask = XPLMI_SECURE_CPM5N_AES_MASK;
		break;
	case XSECURE_API(XSECURE_API_UPDATE_PCIDE_CRYPTO_STATUS):
		CryptoMask = XPLMI_SECURE_PCIDE_AES_MASK;
		break;
	case XSECURE_API(XSECURE_API_UPDATE_PKI_CRYPTO_STATUS):
		CryptoMask = XPLMI_SECURE_PKI_CRYPTO_MASK;
		break;
	default:
		CryptoMask = 0U;
		break;
	}

	if (CryptoMask != 0U) {
		Status = XSecure_UpdateCryptoMask((XSecure_CryptoStatusOp)Pload[0],
					CryptoMask, (Pload[1U] & CryptoMask));
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sets or clears Crypto bit mask of given NodeId
 *
 * @param   CryptoOp	   - Operation to set or clear crypto bit mask
 * @param   CryptoMask	   - Crypto Mask of the module
 * @param   CryptoVal      - Crypto value to be updated
 *
 * @return
	-	XST_SUCCESS - If set or clear is successful
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_UpdateCryptoMask(XSecure_CryptoStatusOp CryptoOp, u32 CryptoMask, u32 CryptoVal)
{
	int Status = XST_FAILURE;

	if ((CryptoOp != XSECURE_CRYPTO_STATUS_SET) && (CryptoOp != XSECURE_CRYPTO_STATUS_CLEAR)) {
		goto END;
	}

	if (CryptoMask != 0U) {
		if (CryptoOp != XSECURE_CRYPTO_STATUS_SET) {
			XPlmi_UpdateCryptoStatus(CryptoMask, ~CryptoVal);
		}
		else {
			XPlmi_UpdateCryptoStatus(CryptoMask, CryptoVal);
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
