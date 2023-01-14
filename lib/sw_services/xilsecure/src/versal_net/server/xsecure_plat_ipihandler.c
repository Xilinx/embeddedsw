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

static int XSecure_UpdateCryptoStatus(XSecure_CryptoStatusOp CryptoOp, u32 NodeId, u32 CryptoMask);

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

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_UPDATE_CRYPTO_STATUS):
		Status = XSecure_UpdateCryptoStatus((XSecure_CryptoStatusOp)Pload[0U], Pload[1U], Pload[2U]);
	default:
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sets or clears Crypto bit mask of given NodeId
 *
 * @param   CryptoOp	   - Operation to set or clear crypto bit mask
 * @param   NodeId		   - Nodeid of the module
 * @param   CryptoVal      - Mask to set crypto bits
 *
 * @return
	-	XST_SUCCESS - If set or clear is successful
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_UpdateCryptoStatus(XSecure_CryptoStatusOp CryptoOp, u32 NodeId, u32 CryptoVal)
{
	int Status = XST_FAILURE;
	u32 Val = 0U;
	u32 CryptoMask = 0U;

	if ((CryptoOp != XSECURE_CRYPTO_STATUS_SET) && (CryptoOp != XSECURE_CRYPTO_STATUS_CLEAR)) {
		goto END;
	}

	/** TODO - Validate NodeId */

	switch(NodeId) {
		case XSECURE_DDR_0_NODE_ID:
		case XSECURE_DDR_1_NODE_ID:
		case XSECURE_DDR_2_NODE_ID:
		case XSECURE_DDR_3_NODE_ID:
		case XSECURE_DDR_4_NODE_ID:
		case XSECURE_DDR_5_NODE_ID:
		case XSECURE_DDR_6_NODE_ID:
		case XSECURE_DDR_7_NODE_ID:
			/* Read value from DDR counters */
			break;
		case XSECURE_HNIC_NODE_ID:
			Val = CryptoVal & XPLMI_SECURE_HNIC_AES_MASK;
			CryptoMask = XPLMI_SECURE_HNIC_AES_MASK;
			break;
		case XSECURE_CPM5N_NODE_ID:
			Val = CryptoVal & XPLMI_SECURE_CPM5N_AES_MASK;
			CryptoMask = XPLMI_SECURE_CPM5N_AES_MASK;
			break;
		case XSECURE_PCIDE_NODE_ID:
			Val = CryptoVal & XPLMI_SECURE_PCIDE_AES_MASK;
			CryptoMask = XPLMI_SECURE_PCIDE_AES_MASK;
			break;
		case XSECURE_PKI_NODE_ID:
			Val = CryptoVal & XPLMI_SECURE_PKI_CRYPTO_MASK;
			CryptoMask = XPLMI_SECURE_PKI_CRYPTO_MASK;
			break;
		default:
			XSecure_Printf(XSECURE_DEBUG_GENERAL,"Invalid NodeId for operation");
			break;
	}
	if (CryptoMask != 0U) {
		if (CryptoOp != XSECURE_CRYPTO_STATUS_SET) {
			XPlmi_UpdateCryptoStatus(CryptoMask, ~Val);
		}
		else {
			XPlmi_UpdateCryptoStatus(CryptoMask, Val);
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
