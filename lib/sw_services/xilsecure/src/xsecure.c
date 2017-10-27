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
* 2.2   vns 09/18/17 Added APIs to support generic functionality
*                    for SHA3 and RSA hardware at linux level.
*                    Removed RSA-SHA2 authentication support
*                    for loading linux image and dtb from u-boot, as here it
*                    is using SHA2 hash and single RSA key pair authentication
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
#include "xsecure_sha.h"

XSecure_Aes SecureAes;
XSecure_Rsa Secure_Rsa;
XSecure_Sha3 Sha3Instance;
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

	case XSECURE_RSA_AES:

	default:
		Status = XSECURE_INVALID_FLAG;
	}
	return Status;
}

 /****************************************************************************/
 /**
 * This function access the xilsecure SHA3 hardware based on the flags provided
 * to calculate the SHA3 hash.
 *
 * @param	SrcAddrHigh  Higher 32-bit of the input or output address.
 * @param	SrcAddrLow   Lower 32-bit of the input or output address.
 * @param	SrcSize      Size of the data on which hash should be
 *		calculated.
 * @param	Flags: inputs for the operation requested
 *		BIT(0) - for initializing csudma driver and SHA3,
 *		(Here address and size inputs can be NULL)
 *		BIT(1) - To call Sha3_Update API which can be called multiple
 *		times when data is not contiguous.
 *		BIT(2) - to get final hash of the whole updated data.
 *		Hash will be overwritten at provided address with 48 bytes.
 *
 * @return	Returns Status XST_SUCCESS on success and error code on failure.
 *
 *****************************************************************************/
u32 XSecure_Sha3Hash(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize, u32 Flags)
{
	u32 Status = XST_SUCCESS;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32) | SrcAddrLow;

	switch (Flags & XSECURE_SHA3_MASK) {
	case XSECURE_SHA3_INIT:
		Status = XSecure_CsuDmaInit();
		if (Status != XST_SUCCESS) {
			return XSECURE_ERROR_CSUDMA_INIT_FAIL;
		}

		Status = XSecure_Sha3Initialize(&Sha3Instance, &CsuDma);
		if (Status != XST_SUCCESS) {
			return XSECURE_SHA3_INIT_FAIL;
		}
		XSecure_Sha3Start(&Sha3Instance);
		break;
	case XSECURE_SHA3_UPDATE:
		if (SrcSize % 4 != 0x00) {
			return XSECURE_SIZE_ERR;
		}
		XSecure_Sha3Update(&Sha3Instance, (u8 *)(UINTPTR)SrcAddr,
						SrcSize);
		break;
	case XSECURE_SHA3_FINAL:
		XSecure_Sha3Finish(&Sha3Instance, (u8 *)(UINTPTR)SrcAddr);
		break;
	default:
		Status = XSECURE_INVALID_FLAG;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This is the function to RSA decrypt or encrypt the provided data and load back
 * to memory
 *
 * @param	SrcAddrHigh	Higher 32-bit Linear memory space from where
 *		CSUDMA will read the data
 * @param	SrcAddrLow	Lower 32-bit Linear memory space from where
 *		CSUDMA will read the data
 * @param	WrSize	Number of bytes to be encrypted or decrypted.
 * @param	Flags:
 *		BIT(0) - Encryption/Decryption
 *			 0 - Rsa Private key Decryption
 *			 1 - Rsa Public key Encryption
 *
 * @return	Returns Status XST_SUCCESS on success and error code on failure.
 *
 * @note	Data to be encrypted/Decrypted + Modulus + Exponent
 *		Modulus and Data should always be key size
 *		Exponent : private key's exponent is key size while decrypting
 *		the signature and 32 bit for public key for encrypting the
 *		signature
 *		In this API we are not taking exponentiation value.
 *
 *****************************************************************************/
u32 XSecure_RsaCore(u32 SrcAddrHigh, u32 SrcAddrLow, u32 SrcSize,
				u32 Flags)
{
	u32 Status = XST_SUCCESS;
	u64 WrAddr = ((u64)SrcAddrHigh << 32) | SrcAddrLow;
	u8 *Modulus = (u8 *)(UINTPTR)(WrAddr + SrcSize);
	u8 *Exponent = (u8 *)(UINTPTR)(WrAddr + SrcSize
						+ SrcSize);

	Status = XSecure_RsaInitialize(&Secure_Rsa, Modulus, NULL, Exponent);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	switch (Flags & XSECURE_RSA_OPERATION) {
	case XSECURE_DEC:
		Status = XSecure_RsaPrivateDecrypt(&Secure_Rsa,
			(u8 *)(UINTPTR)WrAddr, SrcSize,(u8 *)(UINTPTR)WrAddr);
		break;
	case XSECURE_ENC:
		Status = XSecure_RsaPublicEncrypt(&Secure_Rsa,
			(u8 *)(UINTPTR)WrAddr, SrcSize,(u8 *)(UINTPTR)WrAddr);
		break;
	default:
		Status = XSECURE_INVALID_FLAG;
		break;
	}

END:
	return Status;
}
