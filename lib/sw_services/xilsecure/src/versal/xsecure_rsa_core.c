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
* This file contains the implementation of the versal specific RSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/18/19 Initial Release.
* 4.1   vns  08/23/19 Updated Status variables with XST_FAILURE and added
*                     to while loops.
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_rsa.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XSecure_RsaPutData(XSecure_Rsa *InstancePtr);
static void XSecure_RsaZeroize(XSecure_Rsa *InstancePtr);
static void XSecure_RsaWriteMem(XSecure_Rsa *InstancePtr, u32* WrData,
							u8 RamOffset);
static void XSecure_RsaMod32Inverse(XSecure_Rsa *InstancePtr);
static void XSecure_RsaGetData(XSecure_Rsa *InstancePtr, u32 *RdData);

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

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	InstancePtr->BaseAddress = XSECURE_ECDSA_RSA_BASEADDR;
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
*		data to be stored.
* @param	EncDecFlag	Flag to inform the operation to be performed
* 		is either encryption/decryption
* @param	KeySize		Size of the key in bytes.
*
* @return	XST_SUCCESS on success.
*
******************************************************************************/
u32 XSecure_RsaOperation(XSecure_Rsa *InstancePtr, u8 *Input,
			u8 *Result, u8 EncDecFlag, u32 KeySize)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ErrorCode = (u32)XST_FAILURE;
	u32 Timeout = 0U;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Input != NULL);
	Xil_AssertNonvoid(Result != NULL);
	Xil_AssertNonvoid((EncDecFlag == XSECURE_RSA_SIGN_ENC) ||
			(EncDecFlag == XSECURE_RSA_SIGN_DEC));
	Xil_AssertNonvoid((KeySize == XSECURE_RSA_4096_KEY_SIZE) ||
			(KeySize == XSECURE_RSA_3072_KEY_SIZE) ||
			(KeySize == XSECURE_RSA_2048_KEY_SIZE));

	InstancePtr->EncDec = EncDecFlag;
	InstancePtr->SizeInWords = KeySize/4;

	/* Reset core */
	XSecure_ReleaseReset(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RESET_OFFSET);

	/* Setting Key length */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_KEY_LEN_OFFSET,
		(InstancePtr->SizeInWords * 32) &
		XSECURE_ECDSA_RSA_KEY_LEN_MASK);

	/* configuring endianness for data */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_CFG_OFFSET,
		XSECURE_ECDSA_RSA_RSA_CFG_WR_ENDIANNESS_MASK |
		XSECURE_ECDSA_RSA_CFG_RD_ENDIANNESS_MASK);

	/* Put Modulus, exponent, Mod extension in RSA RAM */
	XSecure_RsaPutData(InstancePtr);

	/* Initialize Digest */
	XSecure_RsaWriteMem(InstancePtr, (u32 *)Input,
				XSECURE_CSU_RSA_RAM_DIGEST);

	/* Initialize MINV values from Mod. */
	XSecure_RsaMod32Inverse(InstancePtr);

	/* Configurations */
	switch (InstancePtr->SizeInWords) {
		/* For 2048 key */
		case XSECURE_RSA_2048_SIZE_WORDS:
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG0_OFFSET,
					XSECURE_ECDSA_RSA_CFG0_2048_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG1_OFFSET,
					XSECURE_ECDSA_RSA_CFG1_2048_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG2_OFFSET,
					XSECURE_ECDSA_RSA_CFG2_2048_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG5_OFFSET,
					XSECURE_ECDSA_RSA_CFG5_2048_VALUE);

			ErrorCode = XST_SUCCESS;
			break;
			/* For 3072 key */
		case XSECURE_RSA_3072_SIZE_WORDS:
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG0_OFFSET,
					XSECURE_ECDSA_RSA_CFG0_3072_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG1_OFFSET,
					XSECURE_ECDSA_RSA_CFG1_3072_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG2_OFFSET,
					XSECURE_ECDSA_RSA_CFG2_3072_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG5_OFFSET,
					XSECURE_ECDSA_RSA_CFG5_3072_VALUE);
			ErrorCode = XST_SUCCESS;
			break;
			/* For 4096 key */
		case XSECURE_RSA_4096_SIZE_WORDS:
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG0_OFFSET,
					XSECURE_ECDSA_RSA_CFG0_4096_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG1_OFFSET,
					XSECURE_ECDSA_RSA_CFG1_4096_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG2_OFFSET,
					XSECURE_ECDSA_RSA_CFG2_4096_VALUE);
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_CFG5_OFFSET,
					XSECURE_ECDSA_RSA_CFG5_4096_VALUE);
			ErrorCode = XST_SUCCESS;
			break;
		default:
			ErrorCode = XST_FAILURE;
			break;
	}

	if (ErrorCode == XST_FAILURE) {
		goto END;
	}

	/* Start the RSA operation. */
	if (InstancePtr->ModExt != NULL) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_CTRL_OFFSET,
			XSECURE_CSU_RSA_CONTROL_EXP_PRE);
	}
	else {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_CTRL_OFFSET,
				XSECURE_CSU_RSA_CONTROL_EXP);
	}

	/* Check and wait for status */
	do {
		Status = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_STATUS_OFFSET);

		if(XSECURE_CSU_RSA_STATUS_ERROR ==
			((u32)Status & XSECURE_CSU_RSA_STATUS_ERROR))
		{
			ErrorCode = XST_FAILURE;
			goto END;
		}
		if (XSECURE_CSU_RSA_STATUS_DONE ==
		((u32)Status & XSECURE_CSU_RSA_STATUS_DONE)) {
			ErrorCode = XST_SUCCESS;
			break;
		}
		Timeout = Timeout + 1U;
	}while(Timeout < XSECURE_TIMEOUT_MAX);

	/* Time out occured */
	if (Timeout == XSECURE_TIMEOUT_MAX) {
		ErrorCode = XST_FAILURE;
		goto END;

	}

	/* Copy the result */
	XSecure_RsaGetData(InstancePtr, (u32 *)Result);

	/* Zeroize RSA memory space */
	XSecure_RsaZeroize(InstancePtr);

END:
	/* Revert configuring endianness for data */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_CFG_OFFSET, 0);

	/* Reset core */
	XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

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
 * @param	RdData		Pointer to location where the RSA output data
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

	/* Each of this loop will write 192 bits of data */
	for (DataOffset = 0U; DataOffset < 22U; DataOffset++) {

		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
			(XSECURE_CSU_RSA_RAM_RES_Y * 22U) + DataOffset);

		for (Index = 0; Index < 6U; Index++) {
			TmpIndex = (InstancePtr->SizeInWords - 1U) -
					((DataOffset*6U) + Index);

			if(TmpIndex < 0) {
				break;
			}
			RdData[TmpIndex] =
				XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RAM_DATA_OFFSET);
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
	/* Calculate the MINV */
	u8 Count;
	u32 *ModPtr;
	u32 ModVal;
	u32 Inv;

	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	ModPtr = (u32 *)(InstancePtr->Mod);
	ModVal = Xil_Htonl(ModPtr[InstancePtr->SizeInWords - 1]);
	Inv = (u32)2U - ModVal;

	for (Count = 0U; Count < 4U; ++Count) {
		Inv = (Inv * (2U - ( ModVal * Inv ) ) );
	}

	Inv = ~Inv + 1U;

	/* Put the value in MINV registers */

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_MINV_OFFSET, (Inv));
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

	/** Each of this loop will write 192 bits of data*/
	for (DataOffset = 0U; DataOffset < 22U; DataOffset++) {
		for (Index = 0U; Index < 6U; Index++) {
			TmpIndex = (DataOffset*6U) + Index;
			/**
			* Exponent size is only 4 bytes
			* and rest of the data needs to be 0
			*/
			if((XSECURE_CSU_RSA_RAM_EXPO == RamOffset) &&
			  (InstancePtr->EncDec == XSECURE_RSA_SIGN_ENC)) {
				if(0U == TmpIndex ) {
					Data = *WrData;
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
					Data = WrData[(InstancePtr->SizeInWords - 1) - TmpIndex];
				}
			}
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_RAM_DATA_OFFSET,
							Data);
		}

		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
			((RamOffset * (u8)22) + DataOffset) |
			XSECURE_ECDSA_RSA_RAM_ADDR_WRRD_B_MASK);
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

	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CTRL_OFFSET,
			XSECURE_ECDSA_RSA_CTRL_CLR_DATA_BUF_MASK);
	do {
		for (DataOffset = 0U; DataOffset < 22U; DataOffset++) {
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_RAM_DATA_OFFSET,
				((RamOffset * 22) + DataOffset));
		}
		RamOffset++;
	} while(RamOffset <= XSECURE_CSU_RSA_RAM_RES_Q);

}
