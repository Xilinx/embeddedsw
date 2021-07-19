/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_ipihandler.c
*
* This file contains the xilsecure RSA IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   03/23/21 Initial release
*      bm    05/13/21 Updated code to use common crypto instance
* 4.6  gm    07/16/21 Added 64-bit address support
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_defs.h"
#include "xsecure_rsa.h"
#include "xsecure_rsa_ipihandler.h"
#include "xsecure_init.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static int XSecure_RsaDecrypt(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_RsaEncrypt(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_RsaSignVerify(u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_RsaKat(void);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_RsaIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
		Status = XSecure_RsaDecrypt(Pload[0], Pload[1],
						Pload[2], Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_RSA_PUBLIC_ENCRYPT):
		Status = XSecure_RsaEncrypt(Pload[0], Pload[1],
						Pload[2], Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_RSA_SIGN_VERIFY):
		Status = XSecure_RsaSignVerify(Pload[0], Pload[1]);
		break;
	case XSECURE_API(XSECURE_API_RSA_KAT):
		Status = XSecure_RsaKat();
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_RsaInitialize and
 * 		XSecure_RsaPrivateDecrypt server APIs
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the XSecure_RsaInParam
 * 				structure
 * 		SrcAddrHigh	- Higher 32 bit address of the XSecure_RsaInParam
 * 				structure
 * 		DstAddrLow	- Lower 32 bit address of the output data
 * 				where decrypted data to be stored
 * 		DstAddrHigh	- Higher 32 bit address of the output data
 * 				where decrypted data to be stored
 *
 * @return	- XST_SUCCESS - If the Rsa decryption is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaDecrypt(u32 SrcAddrLow, u32 SrcAddrHigh,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_RsaInParam RsaParams;
	XSecure_Rsa *XSecureRsaInstPtr = XSecure_GetRsaInstance();

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)&RsaParams, sizeof(RsaParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	u64 Modulus = RsaParams.KeyAddr;
	u64 PublicExp = RsaParams.KeyAddr + XSECURE_RSA_4096_KEY_SIZE;

	Status = XSecure_RsaInitialize_64Bit(XSecureRsaInstPtr, Modulus, 0U,
			PublicExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_RsaPrivateDecrypt_64Bit(XSecureRsaInstPtr,
			RsaParams.DataAddr, RsaParams.Size, DstAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_RsaInitialize and
 * 		XSecure_RsaPublicEncrypt server APIs
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the XSecure_RsaInParam
 * 				structure
 * 		SrcAddrHigh	- Higher 32 bit address of the XSecure_RsaInParam
 * 				structure
 * 		DstAddrLow	- Lower 32 bit address of the output data
 * 				where encrypted data to be stored
 * 		DstAddrHigh	- Higher 32 bit address of the output data
 * 				where encrypted data to be stored
 *
 * @return	- XST_SUCCESS - If the Rsa encryption is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaEncrypt(u32 SrcAddrLow, u32 SrcAddrHigh,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_RsaInParam RsaParams;
	XSecure_Rsa *XSecureRsaInstPtr = XSecure_GetRsaInstance();

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)&RsaParams, sizeof(RsaParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	u64 Modulus = RsaParams.KeyAddr;
	u64 PublicExp = RsaParams.KeyAddr + XSECURE_RSA_4096_KEY_SIZE;

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
 * @brief       This function handler calls XSecure_RsaSignVerification server
 * 		API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the
 * 				XSecure_RsaSignParams structure
 * 		SrcAddrHigh	- Higher 32 bit address of the
 * 				XSecure_RsaSignParams structure
 *
 * @return	- XST_SUCCESS - If the Rsa sign verification is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaSignVerify(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_RsaSignParams SignParams;

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)&SignParams, sizeof(SignParams),
			XPLMI_PMCDMA_0);
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

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_RsaPublicEncryptKat server
 * 		API
 *
 * @return	- XST_SUCCESS - If the Rsa Kat is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaKat(void)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_RsaPublicEncryptKat();

	return Status;
}