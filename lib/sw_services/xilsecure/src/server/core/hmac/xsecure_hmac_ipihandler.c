/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/hmac/xsecure_hmac_ipihandler.c
*
* This file contains the implementation of HMAC IPI handler
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   vss  04/29/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_hmac_server_apis HMAC Server APIs in XilSecure
* @{
*/
/****************************************** Include Files *****************************************/
#include "xsecure_hmac_ipihandler.h"
#include "xsecure_hmac.h"
#include "xsecure_defs.h"
#include "xsecure_plat_defs.h"
#include "xsecure_init.h"
#include "xsecure_error.h"
#include "xplmi_plat.h"
#include "xplmi.h"
#include "xsecure_resourcehandling.h"
#include "xil_sutil.h"

/*************************************** Constant Definitions *************************************/

/****************************************** Type Definitions **************************************/

/********************************* Macros (Inline Functions) Definitions **************************/

/************************************** Function Prototypes ***************************************/
static int XSecure_HmacOperation(u32 SubsystemId, u32 SrcAddrLow, u32 SrcAddrHigh,
				 u32 ParamsSize);

/************************************ Variable Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param	Cmd	Pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_INVALID_PARAM  If Cmd is NULL or API ID is invalid
 *		 - XST_FAILURE  On failure
 *
 **************************************************************************************************/
int XSecure_HmacIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	const u32 *Pload = NULL;
	const XSecure_Sha3 *Sha3Instance = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	if ((Cmd == NULL) || (Cmd->Payload == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - SHA IPI event handling */
	Status = XSecure_IpiEventHandling(Cmd, XPLMI_SHA3_CORE);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Pload = Cmd->Payload;

	/** - Call the respective API handler according to API ID */
	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_HMAC_OPERATION):
		/**   - XSecure_HmacOperation */
		Status = XSecure_HmacOperation(Cmd->SubsystemId, Pload[0U], Pload[1U],
					       Pload[2U]);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	if (Sha3Instance->ShaState == XSECURE_SHA_INITIALIZED) {
		SStatus = XSecure_MakeResFree(XPLMI_SHA3_CORE);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function performs HMAC operation.
 *
 * @param	SubsystemId	Subsystem ID
 * @param	SrcAddrLow	Lower 32 bit address of the
 *				XSecure_HmacParams structure
 * @param	SrcAddrHigh	Higher 32 bit address of the
 *				XSecure_HmacParams structure
 * @param	ParamsSize	Size in bytes of the XSecure_HmacParams
 *				structure as reported by the client
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XSECURE_HMAC_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  On failure
 *
 **************************************************************************************************/
static int XSecure_HmacOperation(u32 SubsystemId, u32 SrcAddrLow, u32 SrcAddrHigh,
				 u32 ParamsSize)
{
	volatile int Status = XST_FAILURE;
	volatile int ClrStatus = XST_FAILURE;
	XSecure_HmacParams Params;
	XSecure_Hmac HmacInstance;
	XSecure_HmacRes Res;
	XSecure_Sha3 *Sha3Instance = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XPmcDma *PmcDma = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
	u64 ParamAddr = (((u64)SrcAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)SrcAddrLow);

	/**
	 * - Validate the size passed by the client matches the expected
	 *   parameter structure size and is 8-byte aligned
	 */
	if ((ParamsSize != (u32)sizeof(XSecure_HmacParams)) ||
	    ((ParamsSize & XSECURE_IPI_PARAMS_SIZE_ALIGN_MASK) != 0U)) {
		Status = XSECURE_HMAC_INVALID_PARAM;
		goto END;
	}

	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ParamAddr, sizeof(XSecure_HmacParams), Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);

	Xil_MemCpyFrom64To32Addr((u32)(UINTPTR)&Params, ParamAddr,
				 (u32)sizeof(XSecure_HmacParams));

	/**
	 * - Validate internal address fields in the copied structure
	 */
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Params.KeyAddr, Params.KeyLen, Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Params.MsgAddr, Params.MsgLen, Status,
				XSECURE_ERR_INVALID_ADDR_RANGE, END);

	/**
	 * - Ensure the client supplied output buffer is large enough to hold
	 *   the HMAC result before validating its address range
	 */
	if (Params.OutputLen < XSECURE_SHA_384_HASH_SIZE_IN_BYTES) {
		Status = XSECURE_HMAC_INVALID_PARAM;
		goto END;
	}
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, Params.OutputAddr, Params.OutputLen,
				Status, XSECURE_ERR_INVALID_ADDR_RANGE, END);

	Status = XSecure_ShaInitialize(Sha3Instance, PmcDma);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_HmacInit(&HmacInstance, Sha3Instance, Params.KeyAddr, Params.KeyLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_HmacUpdate(&HmacInstance, Params.MsgAddr, Params.MsgLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_HmacFinal(&HmacInstance, &Res);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_MemCpyFrom32To64Addr(Params.OutputAddr, (u32)(UINTPTR)Res.Hash,
				 XSECURE_SHA_384_HASH_SIZE_IN_BYTES);

END:
	ClrStatus = Xil_SecureZeroize((u8*)&HmacInstance, sizeof(XSecure_Hmac));
	if (Status == XST_SUCCESS) {
		Status = ClrStatus;
	}

	ClrStatus = Xil_SecureZeroize((u8*)&Res, sizeof(XSecure_HmacRes));
	if (Status == XST_SUCCESS) {
		Status = ClrStatus;
	}

	return Status;
}
/** @} */
