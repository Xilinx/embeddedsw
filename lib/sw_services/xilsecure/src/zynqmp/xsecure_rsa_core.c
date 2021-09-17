/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
*       kal  08/27/19 Changed default status to XST_FAILURE
* 4.2   kpt  01/07/20 Resolved CR-1049134 and Added Macros for all
*                     the Magic Numbers
*       har  03/19/20 Simplified calculation for index
*       kpt  03/24/20 Added XSecure_RsaZeroizeVerify for
*                     RSA Zeroize Verification
*       har  04/06/20 Moved PKCS padding related code for zynqmp here from the
*                     common directory
*       har  10/12/20 Addressed security review comments
* 4.6   kal  08/11/21 Added EXPORT CONTROL eFuse check in RsaCfgInitialize
*       am   09/17/21 Resolved compiler warnings
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_rsa_core.h"
#include "xsecure_rsa_hw.h"
#include "xplatform_info.h"
#include "xsecure_cryptochk.h"

/************************** Constant Definitions *****************************/
/* PKCS padding for SHA-3 in 1.0 Silicon */
static const u8 XSecure_Silicon1_TPadSha3[] = {0x30U, 0x41U, 0x30U, 0x0DU,
			0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U,
			0x02U, 0x02U, 0x05U, 0x00U, 0x04U, 0x30U };

/* PKCS padding for SHA-3 in 2.0 Silicon and onwards */
static const u8 XSecure_Silicon2_TPadSha3[] = {0x30U, 0x41U, 0x30U, 0x0DU,
			0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U,
			0x02U, 0x09U, 0x05U, 0x00U, 0x04U, 0x30U };

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_TIMEOUT_MAX             (0x1FFFFFU)

/************************** Function Prototypes ******************************/

static void XSecure_RsaPutData(XSecure_Rsa *InstancePtr);
static void XSecure_RsaGetData(XSecure_Rsa *InstancePtr, u32 *RdData);
static u32 XSecure_RsaZeroize(XSecure_Rsa *InstancePtr);
static u32 XSecure_RsaZeroizeVerify(XSecure_Rsa *InstancePtr);
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

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	InstancePtr->BaseAddress = XSECURE_CSU_RSA_BASE;

END:
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
	u32 ErrorCode = XST_FAILURE;
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
	InstancePtr->SizeInWords = Size/XSECURE_WORD_SIZE;
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
			ErrorCode = (u32)XST_INVALID_PARAM;
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
			ErrorCode = (u32)XST_FAILURE;
			goto END;
		}
		if(XSECURE_CSU_RSA_STATUS_DONE ==
				((u32)Status & XSECURE_CSU_RSA_STATUS_DONE)){
			ErrorCode = (u32)XST_SUCCESS;
			break;
		}
		TimeOut = TimeOut + 1U;
	} while(TimeOut < XSECURE_TIMEOUT_MAX);

	if(TimeOut == XSECURE_TIMEOUT_MAX) {
		ErrorCode = (u32)XST_FAILURE;
		goto END;
	}
	/* Copy the result */
	XSecure_RsaGetData(InstancePtr, (u32 *)Result);

END:
	/* Zeroize and Verify RSA memory space */
	if (InstancePtr->EncDec == XSECURE_RSA_SIGN_DEC) {
		Status = XSecure_RsaZeroize(InstancePtr);
		ErrorCode |= Status;
	}

	return ErrorCode;
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
	TmpIndex = (s32)(InstancePtr->SizeInWords) - 1;
	for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT; DataOffset++)
	{
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_RD_ADDR_OFFSET,
				(XSECURE_CSU_RSA_RAM_RES_Y * XSECURE_RSA_MAX_RD_WR_CNT)
								+ DataOffset);
		Index = (DataOffset == 0U) ? 2U: 0U;
		for (; Index < XSECURE_RSA_MAX_BUFF; Index++)
		{
			if(TmpIndex < 0)
			{
				goto END;
			}
			/*
			 * The Signature digest is compared in Big endian.
			 * So because RSA h/w results in Little endian,
			 * reverse it after reading it from RSA memory,
			 */
			RdData[TmpIndex] = Xil_Htonl(XSecure_ReadReg(
				InstancePtr->BaseAddress,
				(XSECURE_CSU_RSA_RD_DATA_0_OFFSET +
				(u16)(Index * XSECURE_WORD_SIZE))));
			TmpIndex--;

		}
	}
END: ;
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

	for (Count = 0U; Count < XSECURE_WORD_SIZE; ++Count)
	{
		Inv = (Inv * (2U - ( ModVal * Inv ) ) );
	}

	Inv = ~Inv + 1U;

	/* Put the value in MINV registers */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV0_OFFSET,
						(Inv & XSECURE_RSA_BYTE_MASK ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV1_OFFSET,
						((Inv >> XSECURE_RSA_BYTE_SHIFT)
							& XSECURE_RSA_BYTE_MASK ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV2_OFFSET,
						((Inv >> XSECURE_RSA_HWORD_SHIFT)
							& XSECURE_RSA_BYTE_MASK ));
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV3_OFFSET,
						((Inv >> XSECURE_RSA_SWORD_SHIFT)
							& XSECURE_RSA_BYTE_MASK ));
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
	for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT; DataOffset++)
	{
		for (Index = 0U; Index < XSECURE_RSA_MAX_BUFF; Index++)
		{
			TmpIndex = (DataOffset * XSECURE_RSA_MAX_BUFF) + Index;
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
			(XSECURE_CSU_RSA_WR_DATA_0_OFFSET + (Index * XSECURE_WORD_SIZE)),
							Data);
		}
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_WR_ADDR_OFFSET,
				((RamOffset * XSECURE_RSA_MAX_RD_WR_CNT) + DataOffset));
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
 * @return	XST_SUCCESS On Zeroization Success
 * 			XSECURE_RSA_ZEROIZE_ERROR On Zeroization Failure.
 *
 *****************************************************************************/
static u32 XSecure_RsaZeroize(XSecure_Rsa *InstancePtr)
{

	u32 RamOffset = (u32)XSECURE_CSU_RSA_RAM_EXPO;
	u32 DataOffset;
	u32 Index;
	u32 Status = (u32)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->RsaState == XSECURE_RSA_INITIALIZED);

	/**
	 * Each iteration of this loop writes Zero
	 * in to one of the six RSA Write Buffers
	 */

	for (Index = 0; Index < XSECURE_RSA_MAX_BUFF; Index++) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
		(XSECURE_CSU_RSA_WR_DATA_0_OFFSET + (Index * XSECURE_WORD_SIZE)),
		0U);
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_CSU_RSA_ZERO_OFFSET, XSECURE_RSA_CTRL_CLR_DATA_BUF_MASK);
	do {
		for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT;
			DataOffset++) {

			XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_WR_ADDR_OFFSET,
				((RamOffset * XSECURE_RSA_MAX_RD_WR_CNT) + DataOffset));

		}

		RamOffset++;
	} while (RamOffset <= XSECURE_CSU_RSA_RAM_RES_Q);

	Status = XSecure_RsaZeroizeVerify(InstancePtr);

	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV0_OFFSET,
		0U);
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV1_OFFSET,
		0U);
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV2_OFFSET,
		0U);
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_RSA_MINV3_OFFSET,
		0U);

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function verifies the Zeroization of RSA memory space.
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance.
 *
 * @return	XST_SUCCESS On Success
 * 			XSECURE_RSA_ZEROIZE_ERROR On Zeroize Verify Failure
 *
 *****************************************************************************/
static u32 XSecure_RsaZeroizeVerify(XSecure_Rsa *InstancePtr)
{
	u32 RamOffset = (u32)XSECURE_CSU_RSA_RAM_EXPO;
	u32 DataOffset;
	u32 Index;
	u32 Data = 0U;
	u32 Status = (u32)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);

	do {

		for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT;
			DataOffset++) {

			XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_RSA_RD_ADDR_OFFSET,
				((RamOffset * (u8)XSECURE_RSA_MAX_RD_WR_CNT) +
				DataOffset));
			for (Index = 0U; Index < XSECURE_RSA_MAX_BUFF; Index++) {
				Data |= XSecure_ReadReg(InstancePtr->BaseAddress,
					(XSECURE_CSU_RSA_RD_DATA_0_OFFSET +
					(u16)(Index * XSECURE_WORD_SIZE)));
			}

			if (Data != 0U) {
				Status = (u32)XSECURE_RSA_ZEROIZE_ERROR;
				goto END;
			}
		}

		RamOffset++;
	} while (RamOffset <= XSECURE_CSU_RSA_RAM_RES_Q);

	if (((RamOffset - 1U) == XSECURE_CSU_RSA_RAM_RES_Q) &&
		(DataOffset == XSECURE_RSA_MAX_RD_WR_CNT)) {
		Status = (u32)XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function returns PKCS padding as per the silicon version
 *
 * @param       None
 *
 * @return      XSecure_Silicon2_TPadSha3 if Silicon version is not 1.0
 *              XSecure_Silicon1_TPadSha3 if Silicon version is 1.0
 *
 *****************************************************************************/
u8* XSecure_RsaGetTPadding(void)
{
	u8* Tpadding = (u8 *)XNULL ;

	/* If Silicon version is not 1.0 then use the latest NIST approved SHA-3
	 * id for padding
	 */
	if(XGetPSVersion_Info() != (u32)XPS_VERSION_1) {
		Tpadding = (u8*)XSecure_Silicon2_TPadSha3;
	}
	else {
		Tpadding = (u8*)XSecure_Silicon1_TPadSha3;
	}

	return Tpadding;
}
