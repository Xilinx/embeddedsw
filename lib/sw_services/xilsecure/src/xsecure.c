/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure.c
*
* This file contains the implementation of the interface functions for secure
* library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -------------------------------------------------------
* 1.0   DP  02/15/17 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"
#include "xsecure_aes.h"
#include "xsecure.h"
#include "xsecure_rsa.h"
#include "xsecure_sha2.h"

XSecure_Aes SecureAes;
XSecure_Rsa Secure_Rsa;
u32 Iv[XSECURE_IV_LEN];
u32 Key[XSECURE_KEY_LEN];

XCsuDma CsuDma = {0U};

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This function is used to initialize the DMA driver
 *
 * @param	None
 *
 * @return	returns the error code on any error
 *		returns XST_SUCCESS on success
 *
 *****************************************************************************/
u32 XSecure_CsuDmaInit(void)
{
	u32 Status;
	s32 SStatus;
	XCsuDma_Config * CsuDmaConfig;

	CsuDmaConfig = XCsuDma_LookupConfig(XSECURE_CSUDMA_DEVICEID);
	if (NULL == CsuDmaConfig) {
		Status = XSECURE_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}

	SStatus = XCsuDma_CfgInitialize(&CsuDma, CsuDmaConfig,
			CsuDmaConfig->BaseAddress);
	if (SStatus != XST_SUCCESS) {
		Status = XSECURE_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * Converts the char into the equivalent nibble.
 *	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
 *
 * @param	InChar is input character. It has to be between 0-9,a-f,A-F
 * @param	Num is the output nibble.
 *
 * @return
 *		- XST_SUCCESS no errors occured.
 *		- ERROR when input parameters are not valid
 *
 * @note	None.
 *
 *****************************************************************************/
static u32 XSecure_ConvertCharToNibble(char InChar, u8 *Num) {
	/* Convert the char to nibble */
	if ((InChar >= '0') && (InChar <= '9'))
		*Num = InChar - '0';
	else if ((InChar >= 'a') && (InChar <= 'f'))
		*Num = InChar - 'a' + 10;
	else if ((InChar >= 'A') && (InChar <= 'F'))
		*Num = InChar - 'A' + 10;
	else
		return XSECURE_STRING_INVALID_ERROR;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0xab, 0xc1, 0x23}
 *
 * @param	Str is a Input String. Will support the lower and upper
 *		case values. Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 *
 * @return
 *		- XST_SUCCESS no errors occured.
 *		- ERROR when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 * @note	None.
 *
 *****************************************************************************/
static u32 XSecure_ConvertStringToHex(char *Str, u32 *Buf, u8 Len)
{
	u32 Status = XST_SUCCESS;
	u8 ConvertedLen = 0;
	u8 Index = 0;
	u8 Nibble[XSECURE_MAX_NIBBLES];
	u8 NibbleNum;

	while (ConvertedLen < Len) {
		/* Convert char to nibble */
		for (NibbleNum = 0;
		NibbleNum < XSECURE_ARRAY_LENGTH(Nibble); NibbleNum++) {
			Status = XSecure_ConvertCharToNibble(
				Str[ConvertedLen++], &Nibble[NibbleNum]);

			if (Status != XST_SUCCESS)
				/* Error converting char to nibble */
				return XSECURE_STRING_INVALID_ERROR;

		}

		Buf[Index++] = Nibble[0] << 28 | Nibble[1] << 24 |
				Nibble[2] << 20 | Nibble[3] << 16 |
				Nibble[4] << 12 | Nibble[5] << 8 |
				Nibble[6] << 4 | Nibble[7];
	}
	return XST_SUCCESS;
}


/*****************************************************************************/
/** This is the function to decrypt the encrypted data and load back to memory
 *
 * @param	WrSize: Number of bytes that the encrypted image contains
 *
 * @param       WrAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *              will read the data
 *
 * @param       WrAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XSecure_Decrypt(u32 WrSize, u32 SrcAddrHigh, u32 SrcAddrLow)
{
	u32 Status = XST_SUCCESS;
	u64 WrAddr;
	u8 Index;

	WrSize *= XSECURE_WORD_LEN;
	WrAddr = ((u64)SrcAddrHigh << 32) | SrcAddrLow;
	XSecure_ConvertStringToHex((char *)(UINTPTR)WrAddr + WrSize,
					Key, XSECURE_KEY_STR_LEN);
	XSecure_ConvertStringToHex(
		(char *)(UINTPTR)WrAddr + XSECURE_KEY_STR_LEN + WrSize,
				Iv, XSECURE_IV_STR_LEN);

	/* Xilsecure expects Key in big endian form */
	for (Index = 0; Index < XSECURE_ARRAY_LENGTH(Key); Index++)
		Key[Index] = Xil_Htonl(Key[Index]);
	for (Index = 0; Index < XSECURE_ARRAY_LENGTH(Iv); Index++)
		Iv[Index] = Xil_Htonl(Iv[Index]);

	/* Initialize the Aes driver so that it's ready to use */
	XSecure_AesInitialize(&SecureAes, &CsuDma, XSECURE_CSU_AES_KEY_SRC_KUP,
			                           (u32 *)Iv, (u32 *)Key);
	Status = XSecure_AesDecrypt(&SecureAes,
				(u8 *)(UINTPTR)WrAddr,
			(u8 *)(UINTPTR)WrAddr, WrSize - XSECURE_GCM_TAG_LEN);

	return Status;
}

/****************************************************************************/
/**
*
* This function generates RSA hash on data provided by using XilSecure library
*
* @return
*		- XST_FAILURE if the authentication failed.
*
* @note		None.
*
****************************************************************************/
static u32 XSecure_RsaHashGn(u8 *Mod, u8 *Exp, u8 *Sig, u8 *RsaHash)
{
	u8 RsaSha3Array[XSECURE_FSBL_SIG_SIZE];
	u32 Status = XST_SUCCESS;

	/*
	 * Initialize the Rsa driver so that it's ready to use
	 * Look up the configuration in the config table and then initialize it.
	 */

	XSecure_RsaInitialize(&Secure_Rsa, Mod, NULL, Exp);

	Status = XSecure_RsaDecrypt(&Secure_Rsa, (u8 *)Sig, RsaSha3Array);
	if (Status != XST_SUCCESS)
		goto END;

	memcpy(RsaHash, RsaSha3Array +
		   XSECURE_ARRAY_LENGTH(RsaSha3Array) - XSECURE_HASH_TYPE_SHA2,
		   XSECURE_HASH_TYPE_SHA2);

END:
	return Status;
}


/*****************************************************************************/
/** This is the function to authenticate an images.
 *
 * @param	WrSize: Number of bytes of the image to be authenticated
 *
 * @param       WrAddrHigh: Higher 32-bit Linear memory space where image
 *              exits
 *
 * @param       WrAddrLow: Lower 32-bit Linear memory space where authenticated
 *              image exists
 *
 * @return	error status based on implemented functionality
 *		(SUCCESS by default).
 *
 *****************************************************************************/
static u32 XSecure_Auth(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow) {
	u32 Status = XST_SUCCESS;
	u8 Sha2Hash[32];
	u64 WrAddr;
	u8 RsaHash[32];
	u8 *SigAddr;
	u8 *PubKeyAddr;
	u32 Offset;

	if (WrSize % XSECURE_WORD_LEN)
		Offset = (WrSize/XSECURE_WORD_LEN + 1) * XSECURE_WORD_LEN;
	else
		Offset = WrSize;

	WrAddr = ((u64)WrAddrHigh << 32) | WrAddrLow;
	SigAddr = (u8 *)(UINTPTR)(WrAddr +  Offset);
	PubKeyAddr = (u8 *)(UINTPTR)(WrAddr + Offset + XSECURE_FSBL_SIG_SIZE);

	/* Calculate Hash on the given signature */
	Status = XSecure_RsaHashGn(PubKeyAddr,
				   PubKeyAddr + XSECURE_MOD_LEN,
				   SigAddr, RsaHash);

	if(Status != XST_SUCCESS)
		goto END;

	sha_256((u8 *)(UINTPTR)WrAddr, WrSize, Sha2Hash);

	/* Compare Sha2 Hash with RSA Hash */
	if (memcmp(Sha2Hash, RsaHash, XSECURE_ARRAY_LENGTH(RsaHash)))
		Status = XSECURE_AUTH_FAIL;

END:
	return Status;
}

/*****************************************************************************/
/** This is the function to authenticate or decrypt or both for secure images
  * based on flags
 *
 * @param	WrSize: Number of bytes that the secure image contains
 *
 * @param       SrcAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *              will read the data
 *
 * @param       SrcAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data
 * @param	flags:
 *		 1 - Decrypt the image.
 *		 0 - Authenticate the image.
 *		 0 - Authenticated and decrypt the image.
 * NOTE -
 *  The current implementation supports only decryption of images
 *
 * @return	error or success based on implementation in secure libraries
 *
 *****************************************************************************/
u32 XSecure_RsaAes(u32 SrcAddrHigh, u32 SrcAddrLow, u32 WrSize, u32 Flags)
{
	u32 Status = XST_SUCCESS;

	switch (Flags & XSECURE_MASK) {
	case XSECURE_AES:
		Status = XSecure_CsuDmaInit();
		if (Status != XST_SUCCESS) {
			return XSECURE_ERROR_CSUDMA_INIT_FAIL;
		}
		Status = XSecure_Decrypt(WrSize, SrcAddrHigh, SrcAddrLow);
		break;
	case XSECURE_RSA:
		Status = XSecure_Auth(WrSize, SrcAddrHigh, SrcAddrLow);
		break;
	case XSECURE_RSA_AES:
		break;
	default:
		Status = XSECURE_INVALID_FLAG;
	}
	return Status;
}
