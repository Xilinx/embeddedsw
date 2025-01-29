/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_ipihandler.c
*
* This file contains the Xilsecure RSA IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   03/23/21 Initial release
*      bm    05/13/21 Updated code to use common crypto instance
* 4.6  gm    07/16/21 Added 64-bit address support
* 4.7  kpt   03/18/21 Replaced XPlmi_DmaXfr with XPlmi_MemCpy64
* 5.0  kpt   07/24/22 Moved XSecure_RsaKat into xsecure_kat_plat_ipihanlder.c
*      dc    08/22/22 Fixed RSA key accesses address based on RSA key size
* 5.1  yog   05/03/23 Fixed MISRA C violation of Rule 10.3
* 5.3  kpt   03/22/24 Fixed Branch past initialization
*      ss    04/05/24 Fixed doxygen warnings
* 5.4  yog   04/29/24 Fixed doxygen warnings.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_server_apis XilSecure RSA Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifndef PLM_RSA_EXCLUDE
#include "xplmi_dma.h"
#include "xsecure_defs.h"
#include "xsecure_rsa.h"
#include "xsecure_rsa_ipihandler.h"
#include "xsecure_init.h"
#include "xplmi.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static int XSecure_RsaEncrypt(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_RsaSignVerify(u32 SrcAddrLow, u32 SrcAddrHigh);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd	is pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  If the handler execution is successful
 *		 - XST_INVALID_PARAM  If Cmd is NULL or API ID is invalid
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_RsaIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	/** Call the respective API handler according to API ID */
	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
		/**    - XSecure_RsaEncrypt */
		Status = XSecure_RsaEncrypt(Pload[0], Pload[1],
						Pload[2], Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
		/**    - XSecure_RsaSignVerify */
		Status = XSecure_RsaSignVerify(Pload[0], Pload[1]);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler calls XSecure_RsaInitialize and
 * 		XSecure_RsaPublicEncrypt server APIs
 *
 * @param	SrcAddrLow	Lower 32 bit address of the XSecure_RsaInParam
 * 				structure
 * @param	SrcAddrHigh	Higher 32 bit address of the XSecure_RsaInParam
 * 				structure
 * @param	DstAddrLow	Lower 32 bit address of the output data
 * 				where encrypted data to be stored
 * @param	DstAddrHigh	Higher 32 bit address of the output data
 * 				where encrypted data to be stored
 *
 * @return
 *		 - XST_SUCCESS  If the Rsa encryption is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaEncrypt(u32 SrcAddrLow, u32 SrcAddrHigh,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	u64 Modulus;
	u64 PublicExp;
	XSecure_RsaInParam RsaParams;
	XSecure_Rsa *XSecureRsaInstPtr = XSecure_GetRsaInstance();

	Status = XPlmi_MemCpy64((UINTPTR)&RsaParams, Addr, sizeof(RsaParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Modulus = RsaParams.KeyAddr;
	PublicExp = RsaParams.KeyAddr + RsaParams.Size;

	Status = XSecure_RsaInitialize_64Bit(XSecureRsaInstPtr, Modulus, 0U,
			PublicExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_RsaPublicEncrypt_64Bit(XSecureRsaInstPtr,
			RsaParams.DataAddr, RsaParams.Size, DstAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler extracts the payload params with respect
 * 		to XSECURE_API_RSA_SIGN_VERIFY IPI command and calls
 * 		XSecure_RsaSignVerification_64Bit server API.
 *
 * @param	SrcAddrLow	Lower 32 bit address of the
 * 				XSecure_RsaSignParams structure
 * @param	SrcAddrHigh	Higher 32 bit address of the
 * 				XSecure_RsaSignParams structure
 *
 * @return
 *		 - XST_SUCCESS  If the Rsa sign verification is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaSignVerify(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_RsaSignParams SignParams;

	Status = XPlmi_MemCpy64((UINTPTR)&SignParams, Addr, sizeof(SignParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_RsaSignVerification_64Bit(SignParams.SignAddr,
			SignParams.HashAddr,
			SignParams.Size);

END:
	return Status;
}

#endif
/** @} */
