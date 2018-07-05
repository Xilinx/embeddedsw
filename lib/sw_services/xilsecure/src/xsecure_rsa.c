/******************************************************************************
*
* Copyright (C) 2014 - 18 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure_rsa.c
*
* This file contains the implementation of the interface functions for RSA
* driver. Refer to the header file xsecure_sha.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ba   10/13/14 Initial release
* 1.1   ba   12/11/15 Added support for NIST approved SHA-3 in 2.0 silicon
* 2.0   vns  03/15/17 Fixed compilation warning, and corrected SHA2 padding
*                     verfication for silicon version other than 1.0
* 2.2   vns  07/06/17 Added doxygen tags
*       vns  17/08/17 Added APIs XSecure_RsaPublicEncrypt and
*                     XSecure_RsaPrivateDecrypt.As per functionality
*                     XSecure_RsaPublicEncrypt is same as XSecure_RsaDecrypt.
* 3.1   vns  11/04/18 Added support for 512, 576, 704, 768, 992, 1024, 1152,
*                     1408, 1536, 1984, 3072 key sizes, where previous verision
*                     has support only 2048 and 4096 key sizes.
* 3.2   vns  04/30/18 Added check for private RSA key decryption, such that only
*                     data to be decrypted should always be lesser than modulus
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_rsa.h"

/************************** Constant Definitions *****************************/

/* PKCS padding for SHA-3 in 1.0 Silicon */
static const u8 XSecure_Silicon1_TPadSha3[] = {0x30U, 0x41U, 0x30U, 0x0DU,
			0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U,
			0x02U, 0x02U, 0x05U, 0x00U, 0x04U, 0x30U };

/* PKCS padding scheme for SHA-2 */
static const u8 XSecure_Silicon1_TPadSha2[] = {0x30U, 0x31U, 0x30U, 0x0DU,
			0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U,
			0x02U, 0x01U, 0x05U, 0x00U, 0x04U, 0x20U };

/* PKCS padding for SHA-3 in 2.0 Silicon and onwards */
static const u8 XSecure_Silicon2_TPadSha3[] = {0x30U, 0x41U, 0x30U, 0x0DU,
			0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U,
			0x02U, 0x09U, 0x05U, 0x00U, 0x04U, 0x30U };

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static s32 XSecure_RsaOperation(XSecure_Rsa *InstancePtr, u8 *Input,
					u8 *Result);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief
 * This function initializes a specific Xsecure_Rsa instance so that it is
 * ready to be used.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Rsa instance.
 * @param	Mod		A character Pointer which contains the key
 *		Modulus of key size.
 * @param	ModExt		A Pointer to the pre-calculated exponential
 *		(R^2 Mod N) value.
 *		- NULL - if user doesn't have pre-calculated R^2 Mod N value,
 *		control will take care of this calculation internally.
 * @param	ModExpo		Pointer to the buffer which contains key
 *		exponent.
 *
 * @return	XST_SUCCESS if initialization was successful.
 *
 * @note	`Modulus`, `ModExt` and `ModExpo` are part of prtition signature
 * 		when authenticated boot image is generated by bootgen, else the
 *		all of them should be extracted from the key.
 *
 ******************************************************************************/
s32 XSecure_RsaInitialize(XSecure_Rsa *InstancePtr, u8 *Mod, u8 *ModExt,
							u8 *ModExpo)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Mod != NULL);
	Xil_AssertNonvoid(ModExpo != NULL);

	InstancePtr->BaseAddress = XSECURE_CSU_RSA_BASE;
	InstancePtr->Mod = Mod;
	InstancePtr->ModExt = ModExt;
	InstancePtr->ModExpo = ModExpo;
	InstancePtr->SizeInWords = XSECURE_RSA_4096_SIZE_WORDS;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function writes data to RSA RAM at a given offset.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	WrData		Pointer to the data to be written to RSA RAM
 * @param	RamOffset	Offset for the data to be written in RSA RAM
 *
 * @return	None
 *
 *
 ******************************************************************************/
static void XSecure_RsaWriteMem(XSecure_Rsa *InstancePtr, u32* WrData,
							u8 RamOffset)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	u32 Index = 0U;
	u32 DataOffset = 0U;
	u32 TmpIndex = 0U;
	u32 Data = 0U;

	/** Each of this loop will write 192 bits of data*/
	for (DataOffset = 0U; DataOffset < 22U; DataOffset++)
	{
		for (Index = 0U; Index < 6U; Index++)
		{
			TmpIndex = (DataOffset*6) + Index;
			/**
			* Exponent size is only 4 bytes
			* and rest of the data needs to be 0
			*/
			if((XSECURE_CSU_RSA_RAM_EXPO == RamOffset) &&
				(InstancePtr->EncDec == XSECURE_RSA_SIGN_ENC))
			{
				if(0U == TmpIndex )
				{
					Data = Xil_Htonl(*WrData);
				}
				else
				{
					Data = 0x0U;
				}
			}
			else
			{
				if(TmpIndex >= InstancePtr->SizeInWords)
				{
					Data = 0x0U;
				}
				else
				{
				/**
				* The RSA data in Image is in Big Endian.
				* So reverse it before putting in RSA memory,
				* becasue RSA h/w expects it in Little endian.
				*/

				Data = Xil_Htonl(WrData[(InstancePtr->SizeInWords - 1) - TmpIndex]);
				}
			}
			XSecure_WriteReg(InstancePtr->BaseAddress,
			(XSECURE_CSU_RSA_WR_DATA_0_OFFSET + (Index * 4)),
							Data);
		}
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_WR_ADDR_OFFSET,
				((RamOffset * 22) + DataOffset));
	}
}

/*****************************************************************************/
/**
 * @brief
 * This function reads back the resulting data from RSA RAM.
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance.
 * @param	RdData		Pointer to location where the decrypted data will
 * 		be written
 *
 * @return	None
 *
 *
 ******************************************************************************/
static void XSecure_RsaGetData(XSecure_Rsa *InstancePtr, u32 *RdData)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	u32 Index = 0U;
	u32 DataOffset = 0U;
	s32 TmpIndex = 0;

	/* Each of this loop will write 192 bits of data */
	for (DataOffset = 0U; DataOffset < 22U; DataOffset++)
	{
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_RD_ADDR_OFFSET,
				(XSECURE_CSU_RSA_RAM_RES_Y * 22) + DataOffset);

		Index = (DataOffset == 0U) ? 2: 0;
		for (; Index < 6; Index++)
		{
			TmpIndex = (InstancePtr->SizeInWords + 1) - ((DataOffset*6) + Index);
			if(TmpIndex < 0)
			{
				break;
			}
			/*
			 * The Signature digest is compared in Big endian.
			 * So because RSA h/w results in Little endian,
			 * reverse it after reading it from RSA memory,
			 */
			RdData[TmpIndex] = Xil_Htonl(XSecure_ReadReg(
						InstancePtr->BaseAddress,
			(XSECURE_CSU_RSA_RD_DATA_0_OFFSET+ (Index * 4))));
		}
	}

}

/*****************************************************************************/
/**
 * @brief
 * This function calculates the MINV value and put it into RSA core registers.
 *
 * @param	InstancePtr Pointer to XSeure_Rsa instance
 *
 * @return	None
 *
 * @note	MINV is the 32-bit value of `-M mod 2**32`,
 *		where M is LSB 32 bits of the original modulus.
 *
 ******************************************************************************/

static void XSecure_RsaMod32Inverse(XSecure_Rsa *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Calculate the MINV */
	u8 Count = 0U;
	u32 *ModPtr = (u32 *)(InstancePtr->Mod);
	u32 ModVal = Xil_Htonl(ModPtr[InstancePtr->SizeInWords - 1]);
	u32 Inv = 2U - ModVal;

	for (Count = 0U; Count < 4U; ++Count)
	{
		Inv = (Inv * (2U - ( ModVal * Inv ) ) );
	}

	Inv = -Inv;

	/* Put the value in MINV registers */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV0_OFFSET,
						(Inv & 0xFF ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV1_OFFSET,
						((Inv >> 8) & 0xFF ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV2_OFFSET,
						((Inv >> 16) & 0xFF ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV3_OFFSET,
						((Inv >> 24) & 0xFF ));
}

/*****************************************************************************/
/**
 * @brief
 * This function writes all the RSA data used for decryption (Modulus, Exponent)
 * at the corresponding offsets in RSA RAM.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Rsa instance.
 *
 * @return	None.
 *
 *
 ******************************************************************************/
static void XSecure_RsaPutData(XSecure_Rsa *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Initialize Modular exponentiation */
	XSecure_RsaWriteMem(InstancePtr, (u32 *)InstancePtr->ModExpo,
					XSECURE_CSU_RSA_RAM_EXPO);

	/* Initialize Modular. */
	XSecure_RsaWriteMem(InstancePtr, (u32 *)InstancePtr->Mod,
					XSECURE_CSU_RSA_RAM_MOD);

	if (InstancePtr->ModExt != NULL) {
	/* Initialize Modular extension (R*R Mod M) */
		XSecure_RsaWriteMem(InstancePtr, (u32 *)InstancePtr->ModExt,
					XSECURE_CSU_RSA_RAM_RES_Y);
	}

}

/*****************************************************************************/
/**
 * @brief
 * This function handles the RSA decryption from end to end.
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance.
 * @param	EncText		Pointer to the buffer which contains the input
 *		data to be decrypted.
 * @param	Result		Pointer to the buffer where resultant decrypted
 *		data to be stored		.
  *
 * @return	XST_SUCCESS if decryption was successful.
 *
 * @note	This API will be deprecated soon. Instead of this please use
 *		XSecure_RsaPublicEncrypt() API. This API can only support 4096
 *		key Size.
 *
 ******************************************************************************/
s32 XSecure_RsaDecrypt(XSecure_Rsa *InstancePtr, u8 *EncText, u8 *Result)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Result != NULL);

	volatile u32 Status = 0x0U;
	s32 ErrorCode = XST_SUCCESS;

	InstancePtr->EncDec = XSECURE_RSA_SIGN_ENC;

	/* Put Modulus, exponent, Mod extension in RSA RAM */
	XSecure_RsaPutData(InstancePtr);

	/* Initialize Digest */
	XSecure_RsaWriteMem(InstancePtr, (u32 *)EncText,
				XSECURE_CSU_RSA_RAM_DIGEST);

	/* Initialize MINV values from Mod. */
	XSecure_RsaMod32Inverse(InstancePtr);

	/* Start the RSA operation. */
	if (InstancePtr->ModExt != NULL) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_RSA_CONTROL_OFFSET,
			XSECURE_CSU_RSA_CONTROL_MASK);
	}
	else {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_CONTROL_OFFSET,
		XSECURE_CSU_RSA_CONTROL_4096 + XSECURE_CSU_RSA_CONTROL_EXP);
	}

	/* Check and wait for status */
	do
	{
		Status = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_RSA_STATUS_OFFSET);

		if(XSECURE_CSU_RSA_STATUS_ERROR ==
				((u32)Status & XSECURE_CSU_RSA_STATUS_ERROR))
		{
			ErrorCode = XST_FAILURE;
			goto END;
		}
	}while(XSECURE_CSU_RSA_STATUS_DONE !=
				((u32)Status & XSECURE_CSU_RSA_STATUS_DONE));


	/* Copy the result */
	XSecure_RsaGetData(InstancePtr, (u32 *)Result);

END:
	return ErrorCode;
}

/*****************************************************************************/
/**
 * @brief
 * This function verifies the RSA decrypted data provided is either matching
 *  with the provided expected hash by taking care of PKCS padding.
 *
 * @param	Signature	Pointer to the buffer which holds the decrypted
 *		RSA signature
 * @param	Hash		Pointer to the buffer which has hash
 *		calculated on the data to be authenticated.
 * @param	HashLen		Length of Hash used.
 *		- For SHA3 it should be 48 bytes
 *		- For SHA2 it should be 32 bytes
 *
 * @return	XST_SUCCESS if decryption was successful.
 *
 *
 ******************************************************************************/
u32 XSecure_RsaSignVerification(u8 *Signature, u8 *Hash, u32 HashLen)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(Signature != NULL);
	Xil_AssertNonvoid(Hash != NULL);

	u8 * Tpadding = (u8 *)XNULL;
	u32 Pad = XSECURE_FSBL_SIG_SIZE - 3U - 19U - HashLen;
	u8 * PadPtr = Signature;
	u32 sign_index;
	u32 Status = XST_SUCCESS;

	/* If Silicon version is not 1.0 then use the latest NIST approved SHA-3
	 * id for padding
	 */
	if (XGetPSVersion_Info() != XPS_VERSION_1)
	{
		if(XSECURE_HASH_TYPE_SHA3 == HashLen)
		{
			Tpadding = (u8 *)XSecure_Silicon2_TPadSha3;
		}
		else
		{
			Tpadding = (u8 *)XSecure_Silicon1_TPadSha2;
		}
	}
	else
	{
		if(XSECURE_HASH_TYPE_SHA3 == HashLen)
		{
			Tpadding = (u8 *)XSecure_Silicon1_TPadSha3;
		}
		else
		{
			Tpadding = (u8 *)XSecure_Silicon1_TPadSha2;
		}
	}

	/*
	 * Re-Create PKCS#1v1.5 Padding
	* MSB  ------------------------------------------------------------LSB
	* 0x0 || 0x1 || 0xFF(for 202 bytes) || 0x0 || T_padding || SHA384 Hash
	*/

	if (0x00U != *PadPtr)
	{
		Status = XST_FAILURE;
		goto ENDF;
	}
	PadPtr++;

	if (0x01U != *PadPtr)
	{
		Status = XST_FAILURE;
		goto ENDF;
	}
	PadPtr++;

	for (sign_index = 0U; sign_index < Pad; sign_index++)
	{
		if (0xFFU != *PadPtr)
		{
			Status = XST_FAILURE;
			goto ENDF;
		}
		PadPtr++;
	}

	if (0x00U != *PadPtr)
	{
		Status = XST_FAILURE;
		goto ENDF;
	}
	PadPtr++;

	for (sign_index = 0U; sign_index < 19U; sign_index++)
	{
		if (*PadPtr != Tpadding[sign_index])
		{
			Status = XST_FAILURE;
			goto ENDF;
		}
		PadPtr++;
	}

	for (sign_index = 0U; sign_index < HashLen; sign_index++)
	{
		if (*PadPtr != Hash[sign_index])
		{
			Status = XST_FAILURE;
			goto ENDF;
		}
		PadPtr++;
	}

ENDF:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
* This function handles the RSA signature encryption with public key components
* provide at XSecure_RsaInitialize() API.
*
* @param	InstancePtr	Pointer to the XSecure_Rsa instance.
* @param	Input		Pointer to the buffer which contains the input
*		data to be decrypted.
* @param	Size		Key size in bytes, Input size also should be
* 		same as Key size mentioned.Inputs supported are
* 		- XSECURE_RSA_4096_KEY_SIZE and
* 		- XSECURE_RSA_2048_KEY_SIZE
* @param	Result		Pointer to the buffer where resultant decrypted
*		data to be stored		.
*
* @return	XST_SUCCESS if encryption was successful.
*
* @note		Modulus of API XSecure_RsaInitialize() should also
* 		be same size of key size mentioned in this API and exponent
* 		should be 32 bit size.
*
******************************************************************************/
s32 XSecure_RsaPublicEncrypt(XSecure_Rsa *InstancePtr, u8 *Input, u32 Size,
					u8 *Result)
{
	s32 ErrorCode = XST_SUCCESS;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Result != NULL);
	Xil_AssertNonvoid(Input != NULL);
	Xil_AssertNonvoid((Size == XSECURE_RSA_512_KEY_SIZE) ||
		(Size == XSECURE_RSA_576_KEY_SIZE) ||
		(Size == XSECURE_RSA_704_KEY_SIZE) ||
		(Size == XSECURE_RSA_768_KEY_SIZE) ||
		(Size == XSECURE_RSA_992_KEY_SIZE) ||
		(Size == XSECURE_RSA_1024_KEY_SIZE) ||
		(Size == XSECURE_RSA_1152_KEY_SIZE) ||
		(Size == XSECURE_RSA_1408_KEY_SIZE) ||
		(Size == XSECURE_RSA_1536_KEY_SIZE) ||
		(Size == XSECURE_RSA_1984_KEY_SIZE) ||
		(Size == XSECURE_RSA_2048_KEY_SIZE) ||
		(Size == XSECURE_RSA_3072_KEY_SIZE) ||
		(Size == XSECURE_RSA_4096_KEY_SIZE));

	/* Setting for RSA signature encryption with public key */
	InstancePtr->EncDec = XSECURE_RSA_SIGN_ENC;
	InstancePtr->SizeInWords = Size/4;

	ErrorCode = XSecure_RsaOperation(InstancePtr, Input, Result);

	return ErrorCode;

}

/*****************************************************************************/
/**
 * @brief
* This function handles the RSA signature decryption with private key components
* provide at XSecure_RsaInitialize() API.
*
* @param	InstancePtr	Pointer to the XSecure_Rsa instance.
* @param	Input		Pointer to the buffer which contains the input
*		data to be decrypted.
* @param	Size		Key size in bytes, Input size also should be same as
* 		Key size mentioned.
* 		Inputs supported are XSECURE_RSA_4096_KEY_SIZE and
* 		XSECURE_RSA_2048_KEY_SIZE*
* @param	Result		Pointer to the buffer where resultant decrypted
*		data to be stored		.
*
* @return	XST_SUCCESS if decryption was successful.
*               else returns an error code
*			- XSECURE_RSA_DATA_VALUE_ERROR - if input data is
*               greater than modulus
*                       - XST_FAILURE - on RSA operation failure
*
* @note		Modulus and Exponent in XSecure_RsaInitialize() API should also
* 			be same as key size mentioned in this API.
*
******************************************************************************/
s32 XSecure_RsaPrivateDecrypt(XSecure_Rsa *InstancePtr, u8 *Input, u32 Size,
				u8 *Result)
{
	s32 ErrorCode;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Result != NULL);
	Xil_AssertNonvoid(Input != NULL);
	Xil_AssertNonvoid((Size == XSECURE_RSA_512_KEY_SIZE) ||
			(Size == XSECURE_RSA_576_KEY_SIZE) ||
			(Size == XSECURE_RSA_704_KEY_SIZE) ||
			(Size == XSECURE_RSA_768_KEY_SIZE) ||
			(Size == XSECURE_RSA_992_KEY_SIZE) ||
			(Size == XSECURE_RSA_1024_KEY_SIZE) ||
			(Size == XSECURE_RSA_1152_KEY_SIZE) ||
			(Size == XSECURE_RSA_1408_KEY_SIZE) ||
			(Size == XSECURE_RSA_1536_KEY_SIZE) ||
			(Size == XSECURE_RSA_1984_KEY_SIZE) ||
			(Size == XSECURE_RSA_2048_KEY_SIZE) ||
			(Size == XSECURE_RSA_3072_KEY_SIZE) ||
			(Size == XSECURE_RSA_4096_KEY_SIZE));
	/*
	 * Input data should always be smaller than modulus
	 * here we are checking only MSB byte
	 */
	if (*(InstancePtr->Mod) < *Input) {
		return XSECURE_RSA_DATA_VALUE_ERROR;
	}

	/* Setting to perform RSA signature decryption with private key */
	InstancePtr->EncDec = XSECURE_RSA_SIGN_DEC;
	InstancePtr->SizeInWords = Size/4;

	ErrorCode = XSecure_RsaOperation(InstancePtr, Input, Result);

	return ErrorCode;
}

/*****************************************************************************/
/**
 * @brief
* This function handles the all RSA operations with provided inputs.
*
* @param	InstancePtr	Pointer to the XSecure_Rsa instance.
* @param	Input		Pointer to the buffer which contains the input
*		data to be decrypted.
* @param	Result		Pointer to the buffer where resultant decrypted
*		data to be stored		.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
static s32 XSecure_RsaOperation(XSecure_Rsa *InstancePtr, u8 *Input,
							u8 *Result)
{
	volatile u32 Status = 0x0U;
	s32 ErrorCode = XST_SUCCESS;
	u32 RsaType = XSECURE_CSU_RSA_CONTROL_4096;

	/* Put Modulus, exponent, Mod extension in RSA RAM */
	XSecure_RsaPutData(InstancePtr);

	/* Initialize Digest */
	XSecure_RsaWriteMem(InstancePtr, (u32 *)Input,
				XSECURE_CSU_RSA_RAM_DIGEST);

	/* Initialize MINV values from Mod. */
	XSecure_RsaMod32Inverse(InstancePtr);

	switch(InstancePtr->SizeInWords) {
		case XSECURE_RSA_512_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_512;
			break;
		case XSECURE_RSA_576_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_576;
			break;
		case XSECURE_RSA_704_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_704;
			break;
		case XSECURE_RSA_768_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_768;
			break;
		case XSECURE_RSA_992_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_992;
			break;
		case XSECURE_RSA_1024_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_1024;
			break;
		case XSECURE_RSA_1152_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_1152;
			break;
		case XSECURE_RSA_1408_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_1408;
			break;
		case XSECURE_RSA_1536_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_1536;
			break;
		case XSECURE_RSA_1984_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_1984;
			break;
		case XSECURE_RSA_2048_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_2048;
			break;
		case XSECURE_RSA_3072_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_3072;
			break;
		case XSECURE_RSA_4096_SIZE_WORDS:
			RsaType = XSECURE_CSU_RSA_CONTROL_4096;
			break;
		default:
			ErrorCode = XST_FAILURE;
			goto END;
	}

	/* Start the RSA operation. */
	if (InstancePtr->ModExt != NULL) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_RSA_CONTROL_OFFSET,
			RsaType + XSECURE_CSU_RSA_CONTROL_EXP_PRE);
	}
	else {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_CONTROL_OFFSET,
				RsaType + XSECURE_CSU_RSA_CONTROL_EXP);
	}

	/* Check and wait for status */
	do
	{
		Status = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_RSA_STATUS_OFFSET);

		if(XSECURE_CSU_RSA_STATUS_ERROR ==
				((u32)Status & XSECURE_CSU_RSA_STATUS_ERROR))
		{
			ErrorCode = XST_FAILURE;
			goto END;
		}
	}while(XSECURE_CSU_RSA_STATUS_DONE !=
				((u32)Status & XSECURE_CSU_RSA_STATUS_DONE));


	/* Copy the result */
	XSecure_RsaGetData(InstancePtr, (u32 *)Result);

END:
	return ErrorCode;

}
