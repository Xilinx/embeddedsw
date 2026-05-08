/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/lms_hss/xsecure_lms_ipihandler.c
*
* This file contains the Xilsecure LMS IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------------------------------
* 5.7   vss  04/29/26 Initial release
*
* </pre>
*
*
***************************************************************************************************/
/**
* @addtogroup xsecure_lms_server_apis XilSecure LMS Server APIs
* @{
*/
/****************************************** Include Files *****************************************/
#include "xsecure_defs.h"
#include "xil_sutil.h"
#include "xplmi_plat.h"
#include "xplmi_dma.h"
#include "xsecure_lms_ipihandler.h"
#include "xsecure_lms_core.h"
#include "xsecure_sha.h"
#include "xsecure_error.h"
#include "xsecure_plat_defs.h"
#include "xsecure_init.h"
#include "xsecure_resourcehandling.h"

/*************************************** Function Prototypes **************************************/
static int XSecure_LmsSignVerifyIpi(XPlmi_Cmd *Cmd);

/**************************************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param	Cmd	Pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  On successful execution
 *		 - XST_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  If there is a failure
 *
 **************************************************************************************************/
int XSecure_LmsIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	if ((Cmd == NULL) || (Cmd->Payload == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_LMS_SIGN_VERIFY):
		Status = XSecure_LmsSignVerifyIpi(Cmd);
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
 * @brief	This function handler dispatches the request to either
 *		XSecure_LmsSignatureVerification (single-tree LMS) or
 *		XSecure_HssSignatureVerification (HSS, up to two levels)
 *		based on the public key length supplied by the client.
 *		PubKeyLen == XSECURE_LMS_PUB_KEY_TOTAL_SIZE      -> LMS
 *		PubKeyLen == XSECURE_HSS_PUBLIC_KEY_TOTAL_SIZE   -> HSS
 *
 * @param	Cmd	Pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  On successful execution
 *		 - XSECURE_LMS_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  If verification fails
 *
 **************************************************************************************************/
static int XSecure_LmsSignVerifyIpi(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	const u32 *Pload = Cmd->Payload;
	u32 SubsystemId = Cmd->SubsystemId;
	u32 ParamsLow =  Pload[0U];
	u32 ParamsHigh = Pload[1U];
	u32 ParamsSize = Pload[2U];
	u64 ParamsAddr = ((u64)ParamsHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)ParamsLow;
	XSecure_LmsParams Params;
	XSecure_LmsSignVerifyParams LmsSignVerifyParams;
	XPlmi_CoreType Core;
	XSecure_Sha *ShaInstPtr = NULL;
	XPmcDma *PmcDmaPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
	XSecure_HssInitParams HssInitParams;

	/**
	 * - Validate the size passed by the client matches the expected
	 *   parameter structure size and is 8-byte aligned
	 */
	if ((ParamsSize != (u32)sizeof(XSecure_LmsParams)) ||
	    ((ParamsSize & XSECURE_IPI_PARAMS_SIZE_ALIGN_MASK) != 0U)) {
		Status = XSECURE_LMS_INVALID_PARAM;
		goto END;
	}

	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ParamsAddr, sizeof(Params), Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);

	Xil_MemCpyFrom64To32Addr((u32)(UINTPTR)&Params, ParamsAddr,
				 (u32)sizeof(XSecure_LmsParams));

	/**
	 * - Validate internal address fields in the copied structure
	 */
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Params.DataAddr, Params.DataLen, Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Params.LmsSignAddr, Params.LmsSignLen, Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Params.ExpectedPubKeyAddr, Params.PubKeyLen, Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);

	if (Params.HashAlgo == (u32)XSECURE_SHA2_256) {
		ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA2_DEVICE_ID);
		Core =  XPLMI_SHA2_CORE;
	}
	else if (Params.HashAlgo == (u32)XSECURE_SHAKE_256) {
		ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA3_DEVICE_ID);
		Core =  XPLMI_SHA3_CORE;
	}
	else {
		Status = XSECURE_LMS_INVALID_PARAM;
		goto END;
	}

	if ((ShaInstPtr == NULL) || (PmcDmaPtr == NULL)) {
		Status = XST_FAILURE;
		goto END;
	}

	/** - SHA IPI event handling */
	Status = XSecure_IpiEventHandling(Cmd, Core);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	/** - Set the SHA MODE and start the SHA engine */
	Status = XSecure_ShaStart(ShaInstPtr, (XSecure_ShaMode)Params.HashAlgo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	LmsSignVerifyParams.Data = (u8 *)(UINTPTR)Params.DataAddr;
	LmsSignVerifyParams.DataLen = Params.DataLen;
	LmsSignVerifyParams.PreHashedMsg = Params.PreHashedMsg;
	LmsSignVerifyParams.LmsSign = (u8 *)(UINTPTR)Params.LmsSignAddr;
	LmsSignVerifyParams.LmsSignLen = Params.LmsSignLen;
	LmsSignVerifyParams.ExpectedPubKey = (u8 *)(UINTPTR)Params.ExpectedPubKeyAddr;
	LmsSignVerifyParams.PubKeyLen = Params.PubKeyLen;

	/**
	 * - Dispatch based on public key length:
	 *   - LMS single-tree if PubKeyLen == XSECURE_LMS_PUB_KEY_TOTAL_SIZE
	 *   - HSS multi-tree   if PubKeyLen == XSECURE_HSS_PUBLIC_KEY_TOTAL_SIZE
	 */
	if (LmsSignVerifyParams.PubKeyLen == (u32)XSECURE_LMS_PUB_KEY_TOTAL_SIZE) {
		Status = XSecure_LmsSignatureVerification(ShaInstPtr, PmcDmaPtr,
				&LmsSignVerifyParams);
	}
	else if (LmsSignVerifyParams.PubKeyLen == (u32)XSECURE_HSS_PUBLIC_KEY_TOTAL_SIZE) {
		HssInitParams.PublicKey = LmsSignVerifyParams.ExpectedPubKey;
		HssInitParams.PublicKeyLen = LmsSignVerifyParams.PubKeyLen;
		HssInitParams.SignBuff = LmsSignVerifyParams.LmsSign;
		HssInitParams.SignatureLen = LmsSignVerifyParams.LmsSignLen;

		Status = XSecure_HssSignatureVerification(ShaInstPtr, PmcDmaPtr,
				&HssInitParams, LmsSignVerifyParams.Data,
				LmsSignVerifyParams.DataLen);
	}
	else {
		Status = XSECURE_LMS_INVALID_PARAM;
	}

	if (Status != XST_SUCCESS) {
		(void)XSecure_ShaInitialize(ShaInstPtr, PmcDmaPtr);
		goto END;
	}

END:
	if (ShaInstPtr != NULL) {
		if(ShaInstPtr->ShaState == XSECURE_SHA_INITIALIZED) {
			SStatus = XSecure_MakeResFree(Core);
			if (Status == XST_SUCCESS) {
				Status = SStatus;
			}
		}
	}

	return Status;
}

/** @} */
