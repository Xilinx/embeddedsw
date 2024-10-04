/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_ipihandler.c
* This file contains versal specific code for xilsecure server ipi handler.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.3   kpt   03/12/24 Initial release
*       kpt   03/30/24 Fixed Branch past initialization
*	ss    04/05/24 Fixed doxygen warnings
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in Xilsecure server
* @{
*/
/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xplmi_dma.h"
#include "xplmi.h"
#include "xsecure_defs.h"
#include "xsecure_error.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa.h"
#endif
#include "xsecure_init.h"
#include "xsecure_plat_ipihandler.h"

/************************** Function Prototypes *****************************/
#ifndef PLM_RSA_EXCLUDE
static int XSecure_RsaDecrypt(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
#endif

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  If the handler execution is successful
 *		 - XST_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_PlatIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Call the respective API handler according to API ID */
	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
		/**   - @ref XSecure_RsaDecrypt */
		Status = XSecure_RsaDecrypt(Cmd->Payload[0], Cmd->Payload[1],
						Cmd->Payload[2], Cmd->Payload[3]);
		break;
#endif
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function handler calls XSecure_RsaInitialize and
 * 		XSecure_RsaPrivateDecrypt server APIs
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the XSecure_RsaInParam
 * 				structure
 * @param	SrcAddrHigh	- Higher 32 bit address of the XSecure_RsaInParam
 * 				structure
 * @param	DstAddrLow	- Lower 32 bit address of the output data
 * 				where decrypted data to be stored
 * @param	DstAddrHigh	- Higher 32 bit address of the output data
 * 				where decrypted data to be stored
 *
 * @return
 *		 - XST_SUCCESS  If the Rsa decryption is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaDecrypt(u32 SrcAddrLow, u32 SrcAddrHigh,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	u64 Modulus;
	u64 PrivateExp;
	XSecure_RsaInParam RsaParams;
	XSecure_Rsa *XSecureRsaInstPtr = XSecure_GetRsaInstance();

	Status = XPlmi_MemCpy64((UINTPTR)&RsaParams, Addr, sizeof(RsaParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Modulus = RsaParams.KeyAddr;
	PrivateExp = RsaParams.KeyAddr + RsaParams.Size;

	Status = XSecure_RsaInitialize_64Bit(XSecureRsaInstPtr, Modulus, 0U,
			PrivateExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_RsaPrivateDecrypt_64Bit(XSecureRsaInstPtr,
			RsaParams.DataAddr, RsaParams.Size, DstAddr);

END:
	return Status;
}
#endif
/** @} */
