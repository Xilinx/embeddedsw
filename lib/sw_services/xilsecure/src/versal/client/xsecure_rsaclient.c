/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_rsaclient.c
*
* This file contains the implementation of the client interface functions for
* RSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
* 4.6   kpt  09/27/21 Fixed compilation warnings
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xsecure_defs.h"
#include "xsecure_ipi.h"
#include "xsecure_rsaclient.h"

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to Perform RSA decryption with
 * 		private key.
 *
 * @param	KeyAddr		- Address of the Key
 * @param	InDataAddr	- Address of the data which has to be decrypted
 * @param	Size		- Key size in bytes, Input size also should be
 * 				same as key size mentioned. Inputs supported are
 *				- XSECURE_RSA_4096_KEY_SIZE,
 *				- XSECURE_RSA_2048_KEY_SIZE
 *				- XSECURE_RSA_3072_KEY_SIZE
 *		OutDataAddr	- Address of the buffer where resultant decrypted
 *				data to be stored
 *
 * @return	- XST_SUCCESS - If the update is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_RsaPrivateDecrypt(const u64 KeyAddr, const u64 InDataAddr,
				const u32 Size, const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaInParam RsaParams __attribute__ ((aligned (64)));
	u64 BufferAddr;

	RsaParams.KeyAddr = KeyAddr;
	RsaParams.DataAddr = InDataAddr;
	RsaParams.Size = Size;
	BufferAddr = (u64)(UINTPTR)&RsaParams;

	Xil_DCacheFlushRange((INTPTR)BufferAddr, sizeof(RsaParams));

	Status = XSecure_ProcessIpiWithPayload4(XSECURE_API_RSA_PRIVATE_DECRYPT,
			(u32)BufferAddr, (u32)(BufferAddr >> 32),
			(u32)OutDataAddr, (u32)(OutDataAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to Perform RSA encryption with
 * 		public key.
 *
 * @param	KeyAddr		- Address of the Key
 * @param	InDataAddr	- Address of the data which has to be encrypted
 * 				with public key
 * @param	Size		- Key size in bytes, Input size also should be
 * 				same as key size mentioned. Inputs supported are
 *				- XSECURE_RSA_4096_KEY_SIZE,
 *				- XSECURE_RSA_2048_KEY_SIZE
 *				- XSECURE_RSA_3072_KEY_SIZE
 *		OutDataAddr	- Address of the buffer where resultant decrypted
 *				data to be stored
 *
 * @return	- XST_SUCCESS - If encryption was successful
 *		- XSECURE_RSA_INVALID_PARAM - On invalid arguments
 *		- XSECURE_RSA_STATE_MISMATCH_ERROR - If there is state mismatch
 *
 ******************************************************************************/
int XSecure_RsaPublicEncrypt(const u64 KeyAddr, const u64 InDataAddr,
                                const u32 Size, const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaInParam RsaParams __attribute__ ((aligned (64)));
	u64 BufferAddr;

	RsaParams.KeyAddr = KeyAddr;
	RsaParams.DataAddr = InDataAddr;
	RsaParams.Size = Size;
	BufferAddr = (u64)(UINTPTR)&RsaParams;

	Xil_DCacheFlushRange((INTPTR)BufferAddr, sizeof(RsaParams));

	Status = XSecure_ProcessIpiWithPayload4(XSECURE_API_RSA_PUBLIC_ENCRYPT,
			(u32)BufferAddr, (u32)(BufferAddr >> 32),
			(u32)OutDataAddr, (u32)(OutDataAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to Perform RSA sign verification
 *
 * @param	SignAddr 	- Address of the buffer which holds the
 * 				decrypted RSA signature.
 * @param	HashAddr	- Address of the HashAddr which has the
 * 				hash calculated on the data to be authenticated
 *
 * @return	- XST_SUCCESS - If decryption was successful
 *		- XSECURE_RSA_INVALID_PARAM - On invalid arguments
 *		- XST_FAILURE - In case of mismatch
 *
 *****************************************************************************/
int XSecure_RsaSignVerification(const u64 SignAddr, const u64 HashAddr,
								const u32 Size)
{
	volatile int Status = XST_FAILURE;
	XSecure_RsaSignParams SignParams __attribute__ ((aligned (64)));
	u64 BufAddr;

	SignParams.SignAddr = SignAddr;
	SignParams.HashAddr = HashAddr;
	SignParams.Size = Size;
	BufAddr = (u64)(UINTPTR)&SignParams;

	Xil_DCacheFlushRange((INTPTR)BufAddr, sizeof(SignParams));

	Status = XSecure_ProcessIpiWithPayload2(XSECURE_API_RSA_SIGN_VERIFY,
			(u32)BufAddr, (u32)(BufAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to Perform RSA KAT
 *
 * @param
 *
 * @return	- XST_SUCCESS - On success
 *		- XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR - When RSA KAT fails
 *		- XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR - Error when
 *					RSA data not matched with expected data
 *
 ******************************************************************************/
int XSecure_RsaKat(void)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload0(XSECURE_API_RSA_KAT);

	return Status;
}
