/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.1   kpt   01/04/2023 Added API to set or clear KAT status for external modules
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

#define XSECURE_KAT_MAX_CMD_LEN     6U /**< Maximum command length*/

/************************** Function Prototypes *****************************/
#ifndef PLM_SECURE_EXCLUDE
static int XSecure_TrngKat(void);
#endif
static int XSecure_UpdateKatStatus(XSecure_KatOp KatOp, u32 NodeId, u32 CmdLen, u32 *KatMask);

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
	case XSECURE_API(XSECURE_API_UPDATE_KAT_STATUS):
		Status = XSecure_UpdateKatStatus(Pload[1U], Pload[2U], Cmd->Len, &Pload[3U]);
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
	XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_TrngPreOperationalSelfTests(TrngInstance);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_TRNG_KAT_MASK);

	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief       This function sets or clears KAT mask of given NodeId
 *
 * @param   KatOp		- Operation to set or clear KAT mask
 * @param   NodeId		- Nodeid of the module
 * @param   CmdLen  -    Length of the KAT mask
 * @param   KatMask     - Pointer to the KAT mask
 *
 * @return
	-	XST_SUCCESS - If set or clear is successful
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_UpdateKatStatus(XSecure_KatOp KatOp, u32 NodeId, u32 CmdLen, u32 *KatMask) {
	int Status = XST_FAILURE;
	u32 KatAddr = 0U;
	u32 KatVal[XSECURE_MAX_KAT_MASK_LEN] = {0U};
	u32 KatMaskLen = 0U;

	if ((KatOp != XSECURE_API_KAT_CLEAR) && (KatOp != XSECURE_API_KAT_SET)) {
			goto END;
	}

	if ((CmdLen <= XSECURE_KAT_HDR_LEN) || (CmdLen > XSECURE_KAT_MAX_CMD_LEN)) {
		goto END;
	}

	KatMaskLen = CmdLen - XSECURE_KAT_HDR_LEN;
	if (((KatMaskLen > XSECURE_MIN_KAT_MASK_LEN) && (NodeId != XSECURE_PKI_NODE_ID)) ||
			(KatMaskLen == 0U)) {
		goto END;
	}

	if ((KatMaskLen != XSECURE_MAX_KAT_MASK_LEN) && (NodeId == XSECURE_PKI_NODE_ID)) {
		goto END;
	}

	/** TODO - Validate NodeId */

	switch(NodeId) {
		case XSECURE_DDR_0_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_DDR_KAT_ADDR;
			KatMask[0U] &= XPLMI_DDR_0_KAT_MASK;
			break;
		case XSECURE_DDR_1_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_DDR_KAT_ADDR;
			KatMask[0U] &= XPLMI_DDR_1_KAT_MASK;
			break;
		case XSECURE_DDR_2_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_DDR_KAT_ADDR;
			KatMask[0U] &= XPLMI_DDR_2_KAT_MASK;
			break;
		case XSECURE_DDR_3_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_DDR_KAT_ADDR;
			KatMask[0U] &= XPLMI_DDR_3_KAT_MASK;
			break;
		case XSECURE_DDR_4_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_DDR_KAT_ADDR;
			KatMask[0U] &= XPLMI_DDR_4_KAT_MASK;
			break;
		case XSECURE_DDR_5_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_DDR_KAT_ADDR;
			KatMask[0U] &= XPLMI_DDR_5_KAT_MASK;
			break;
		case XSECURE_DDR_6_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_DDR_KAT_ADDR;
			KatMask[0U] &= XPLMI_DDR_6_KAT_MASK;
			break;
		case XSECURE_DDR_7_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_DDR_KAT_ADDR;
			KatMask[0U] &= XPLMI_DDR_7_KAT_MASK;
			break;
		case XSECURE_HNIC_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_HNIC_CPM5N_PCIDE_KAT_ADDR;
			KatMask[0U] &= XPLMI_HNIC_KAT_MASK;
			break;
		case XSECURE_CPM5N_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_HNIC_CPM5N_PCIDE_KAT_ADDR;
			KatMask[0U] &= XPLMI_CPM5N_KAT_MASK;
			break;
		case XSECURE_PCIDE_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_HNIC_CPM5N_PCIDE_KAT_ADDR;
			KatMask[0U] &= XPLMI_PCIDE_KAT_MASK;
			break;
		case XSECURE_PKI_NODE_ID:
			KatAddr = XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_0;
			KatMask[2U] &= XPLMI_PKI_KAT_MASK;
			break;
		default:
			XSecure_Printf(XSECURE_DEBUG_GENERAL,"Invalid NodeId for KAT operation");
			break;
	}
	if (KatAddr != 0U) {
		if (KatOp != XSECURE_API_KAT_CLEAR) {
			KatVal[0U] = KatMask[0U];
			if (NodeId == XSECURE_PKI_NODE_ID) {
				KatVal[1U] = KatMask[1U];
				KatVal[2U] = KatMask[2U];
			}
		}
		else {
			KatVal[0U] = ~KatMask[0U];
			if (NodeId == XSECURE_PKI_NODE_ID) {
				KatVal[1U] = ~KatMask[1U];
				KatVal[2U] = ~KatMask[2U];
			}
		}

		Status = Xil_SecureRMW32(KatAddr, KatMask[0U], KatVal[0U]);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (NodeId == XSECURE_PKI_NODE_ID) {
			Status = Xil_SecureRMW32(XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_1, KatMask[1U], KatVal[1U]);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = Xil_SecureRMW32(XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_2, KatMask[2U], KatVal[2U]);
			if (Status != XST_SUCCESS) {
				goto END;
			}

		}
		Status = XPlmi_CheckAndUpdateFipsState();
	}

END:
	return Status;
}
