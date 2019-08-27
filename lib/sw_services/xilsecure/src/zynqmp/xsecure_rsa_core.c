/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure_rsa_core.c
*
* This file contains the implementation of the ZynqMP specific RSA driver.
* Refer to the header file xsecure_rsa_core.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/12/19 Initial Release.
*       arc  03/20/19 modified default status value to XST_FAILURE
*                     for XSecure_RsaSignVerification()
*       mmd  03/15/19 Refactored the code
*       psl  03/26/19 Fixed MISRA-C violation
* 4.1   psl  08/05/19 Fixed MISRA-C violation
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_rsa_core.h"
#include "xsecure_rsa_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XSecure_RsaPutData(XSecure_Rsa *InstancePtr);
static void XSecure_RsaGetData(XSecure_Rsa *InstancePtr, u32 *RdData);
static void XSecure_RsaZeroize(XSecure_Rsa *InstancePtr);
static void XSecure_RsaWriteMem(XSecure_Rsa *InstancePtr, u32* WrData,
							u8 RamOffset);
static void XSecure_RsaMod32Inverse(XSecure_Rsa *InstancePtr);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief
* This function stores the base address of RSA core registers.
*
* @param	InstancePtr	Pointer to the XSecure_Rsa instance.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
u32 XSecure_RsaCfgInitialize(XSecure_Rsa *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;

	InstancePtr->BaseAddress = XSECURE_CSU_RSA_BASE;
	Status = (u32)XST_SUCCESS;

	return Status;
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
u32 XSecure_RsaOperation(XSecure_Rsa *InstancePtr, u8 *Input,
			u8 *Result, u8 EncDecFlag, u32 Size)
{
	u32 Status = (u32)XST_FAILURE;
	s32 ErrorCode = XST_FAILURE;
	u32 RsaType = XSECURE_CSU_RSA_CONTROL_4096;
	u32 TimeOut = 0U;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Input != NULL);
	Xil_AssertNonvoid(Result != NULL);
	Xil_AssertNonvoid((EncDecFlag == XSECURE_RSA_SIGN_ENC) ||
			(EncDecFlag == XSECURE_RSA_SIGN_DEC));
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

	InstancePtr->EncDec = EncDecFlag;
	InstancePtr->SizeInWords = Size/4U;
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
			ErrorCode = XST_INVALID_PARAM;
			break;
	}

	if(ErrorCode == XST_INVALID_PARAM) {
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
	do {
		Status = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_RSA_STATUS_OFFSET);

		if(XSECURE_CSU_RSA_STATUS_ERROR ==
				((u32)Status & XSECURE_CSU_RSA_STATUS_ERROR))
		{
			ErrorCode = XST_FAILURE;
			goto END;
		}
		if(XSECURE_CSU_RSA_STATUS_DONE ==
				((u32)Status & XSECURE_CSU_RSA_STATUS_DONE)){
			ErrorCode = XST_SUCCESS;
			break;
		}
		TimeOut = TimeOut + 1U;
	} while(TimeOut < XSECURE_TIMEOUT_MAX);

	if(TimeOut == XSECURE_TIMEOUT_MAX) {
		ErrorCode = XST_FAILURE;
		goto END;
	}
	/* Copy the result */
	XSecure_RsaGetData(InstancePtr, (u32 *)Result);

END:
	/* Zeroize RSA memory space */
	XSecure_RsaZeroize(InstancePtr);
	return (u32)ErrorCode;
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
 * This function reads back the resulting data from RSA RAM.
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance.
 * @param	RdData		Pointer to location where the decrypted data
 *		will be written
 *
 * @return	None
 *
 *
 ******************************************************************************/
static void XSecure_RsaGetData(XSecure_Rsa *InstancePtr, u32 *RdData)
{
	u32 Index;
	u32 DataOffset;
	s32 TmpIndex;

	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/** Total bits to be read 4096
	 * Each iteration of this loop reads 32 * 6 = 192 bits
	 * Therefore, for 4096, total iterations required = 4096/192 = 21.33 = 22
	 */
	for (DataOffset = 0U; DataOffset < 22U; DataOffset++)
	{
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_RD_ADDR_OFFSET,
				(XSECURE_CSU_RSA_RAM_RES_Y * 22U) + DataOffset);
		Index = (DataOffset == 0U) ? 2U: 0U;
		for (; Index < 6U; Index++)
		{
			TmpIndex = (InstancePtr->SizeInWords + 1U) -
					((DataOffset*6U) + Index);
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
			(XSECURE_CSU_RSA_RD_DATA_0_OFFSET+ (Index * 4U))));
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
	u8 Count;
	u32 *ModPtr = (u32 *)(InstancePtr->Mod);
	u32 ModVal = Xil_Htonl(ModPtr[InstancePtr->SizeInWords - 1]);
	u32 Inv = (u32)2U - ModVal;

	for (Count = 0U; Count < 4U; ++Count)
	{
		Inv = (Inv * (2U - ( ModVal * Inv ) ) );
	}

	Inv = ~Inv + 1U;

	/* Put the value in MINV registers */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV0_OFFSET,
						(Inv & 0xFFU ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV1_OFFSET,
						((Inv >> 8) & 0xFFU ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV2_OFFSET,
						((Inv >> 16) & 0xFFU ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV3_OFFSET,
						((Inv >> 24) & 0xFFU ));
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
	u32 Index;
	u32 DataOffset;
	u32 TmpIndex;
	u32 Data;

	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(WrData != NULL);

	/** Total bits to be written 4096 (RSA Key size)
	 * Each iteration of this loop writes 32 * 6 = 192 bits
	 * Therefore, for 4096, total iteration required = 4096/192 = 21.33 = 22
	 */
	for (DataOffset = 0U; DataOffset < 22U; DataOffset++)
	{
		for (Index = 0U; Index < 6U; Index++)
		{
			TmpIndex = (DataOffset*6U) + Index;
			/**
			* Exponent size is only 4 bytes
			* and rest of the data needs to be 0
			*/
			Data = 0U;
			if((XSECURE_CSU_RSA_RAM_EXPO == RamOffset) &&
			   (InstancePtr->EncDec == XSECURE_RSA_SIGN_ENC))
			{
				if(0U == TmpIndex)
				{
					Data = Xil_Htonl(*WrData);
				}
			}
			else
			{
				if(TmpIndex < InstancePtr->SizeInWords)
				{
					/**
					* The RSA data in Image is in Big Endian.
					* So reverse it before putting in RSA memory,
					* because RSA h/w expects it in Little endian.
					*/
					Data = Xil_Htonl(
					        WrData[(InstancePtr->SizeInWords - 1) - TmpIndex]);
				}
			}
			XSecure_WriteReg(InstancePtr->BaseAddress,
			(XSECURE_CSU_RSA_WR_DATA_0_OFFSET + (Index * 4U)),
							Data);
		}
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_WR_ADDR_OFFSET,
				((RamOffset * 22U) + DataOffset));
	}
}


/*****************************************************************************/
/**
 * @brief
 * This function clears whole RSA memory space. This function clears stored
 * exponent, modulus and exponentiation key components along with digest.
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance.
 *
 * @return	None.
 *
 *****************************************************************************/
static void XSecure_RsaZeroize(XSecure_Rsa *InstancePtr)
{

	u32 RamOffset = 0U;
	u32 DataOffset;

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_RSA_ZERO_OFFSET, 1U);
	do {
		for (DataOffset = 0U; DataOffset < 22U; DataOffset++) {
			XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_WR_ADDR_OFFSET,
				((RamOffset * 22U) + DataOffset));
		}
		RamOffset++;
	} while(RamOffset <= XSECURE_CSU_RSA_RAM_RES_Q);

}
