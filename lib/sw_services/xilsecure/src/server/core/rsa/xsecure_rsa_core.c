/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_core.c
*
* This file contains the implementation of the Versal specific RSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/18/19 Initial Release.
* 4.1   vns  08/23/19 Updated Status variables with XST_FAILURE and added
*                     to while loops.
* 4.2   kpt  01/07/20 Resolved CR-1049134 and added Macro's for all the
*                     Magic Numbers
*       har  03/19/20 Simplified calculation for Index
*       rpo  04/02/20 Added crypto KAT APIs
*                     Added function to update data length configuration
*       kpt  03/24/20 Added XSecure_RsaZeroizeVerify for
*                     RSA Zeroization Verification and modified Code for
*                     Zeroization
*       har  04/06/20 Moved PKCS padding related code for versal from the
*                     common directory
*       bvi  04/07/20 Renamed csudma as pmcdma
* 4.3   ana  06/04/20 Minor enhancement
*       har  07/12/20 Removed Magic number from XSecure_RsaPublicEncryptKat
*       rpo  09/01/20 Asserts are not compiled by default for secure libraries
*       rpo  09/10/20 Input validations are added
*       rpo  09/21/20 New error code added for crypto state mismatch
*       am   09/24/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
*       am   10/10/20 Resolved Coverity warnings
* 4.6   har  07/14/21 Fixed doxygen warnings
*       gm   07/16/21 Added support for 64-bit address
* 5.0   kpt  07/24/21 Moved XSecure_RsaPublicEncrypt KAT into xsecure_kat.c
*       kpt  08/03/22 Added volatile keyword to avoid compiler optimization of
*                     loop redundancy check
*       dc   08/26/22 Optimization of size by changing u8 to u32
* 5.2   kpt  08/20/23 Added XSecure_RsaEcdsaZeroizeAndVerifyRam
*	vss  09/18/23 Fixed compilation warning due to XSecure_RsaEcdsaZeroizeAndVerifyRam
* 5.4   yog  04/29/24 Fixed doxygen warnings.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_rsa_server_apis XilSecure RSA Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xsecure_rsa_core.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xil_sutil.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa.h"
#include "xsecure_error.h"
#include "xsecure_plat.h"

/************************** Constant Definitions ****************************/
/* PKCS padding for SHA-3 in Versal */
static const u8 XSecure_Silicon2_TPadSha3[] =
			{0x30U, 0x41U, 0x30U, 0x0DU,
			 0x06U, 0x09U, 0x60U, 0x86U,
			 0x48U, 0x01U, 0x65U, 0x03U,
			 0x04U, 0x02U, 0x09U, 0x05U,
			 0x00U, 0x04U, 0x30U };

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_TIMEOUT_MAX		(0x1FFFFFU)
					/**< Recommended software timeout */
#define SIZEOF_INT_IN_BYTES		(0x4U)
					/**< Size of integer om bytes */

/************************** Function Prototypes ******************************/

static void XSecure_RsaPutData(const XSecure_Rsa *InstancePtr);
static void XSecure_RsaWriteMem(const XSecure_Rsa *InstancePtr,
	u64 WrDataAddr, u8 RamOffset);
static void XSecure_RsaMod32Inverse(const XSecure_Rsa *InstancePtr);
static void XSecure_RsaGetData(const XSecure_Rsa *InstancePtr, u64 RdDataAddr);
static void XSecure_RsaDataLenCfg(const XSecure_Rsa *InstancePtr, u32 Cfg0, u32 Cfg1,
	u32 Cfg2, u32 Cfg5);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function stores the base address of RSA core registers
 *
 * @param	InstancePtr	 Pointer to the XSecure_Rsa instance
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_RSA_INVALID_PARAM  On invalid parameter
 *
******************************************************************************/
int XSecure_RsaCfgInitialize(XSecure_Rsa *InstancePtr)
{
	int Status = XST_FAILURE;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	/** Set RSA in use flag */
	XSecure_SetRsaCryptoStatus();

	InstancePtr->BaseAddress = XSECURE_ECDSA_RSA_BASEADDR;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handles the Public encryption and private decryption
 * 		of RSA operations with provided inputs
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance
 * @param	Input		Address of the buffer which contains the input
 *				data to be encrypted/decrypted
 * @param	Result		Address of buffer where resultant
 *				encrypted/decrypted data to be stored
 * @param	RsaOp		Flag to inform the operation to be performed
 * 				is either encryption/decryption
 * @param	KeySize		Size of the key in bytes
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_RSA_INVALID_PARAM  On invalid parameter
 *		 - XST_FAILURE  On failure
 *
******************************************************************************/
int XSecure_RsaOperation(XSecure_Rsa *InstancePtr, u64 Input,
	u64 Result, XSecure_RsaOps RsaOp, u32 KeySize)
{
	int Status = XST_FAILURE;
	volatile int ErrorCode = XST_FAILURE;
	u32 Events;

	/** Validate the input arguments */
	if (InstancePtr == NULL) {
		ErrorCode = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	if ((RsaOp != XSECURE_RSA_SIGN_ENC) && (RsaOp != XSECURE_RSA_SIGN_DEC)) {
		ErrorCode = (int)XSECURE_RSA_INVALID_PARAM;
		goto END_RST;
	}

	if ((KeySize != XSECURE_RSA_4096_KEY_SIZE) &&
		(KeySize !=XSECURE_RSA_3072_KEY_SIZE) &&
		(KeySize != XSECURE_RSA_2048_KEY_SIZE)) {
		ErrorCode = (int)XSECURE_RSA_INVALID_PARAM;
		goto END_RST;
	}

	InstancePtr->EncDec = (u8)RsaOp;
	InstancePtr->SizeInWords = KeySize/XSECURE_WORD_SIZE;

	/* Reset core */
	XSecure_ReleaseReset(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RESET_OFFSET);

	/* Setting Key length */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_KEY_LEN_OFFSET,
		(InstancePtr->SizeInWords * XSECURE_WORD_IN_BITS));

	/* configuring endianness for data */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_CFG_OFFSET,
		XSECURE_ECDSA_RSA_RSA_CFG_WR_ENDIANNESS_MASK |
		XSECURE_ECDSA_RSA_CFG_RD_ENDIANNESS_MASK);

	/** Put Modulus, exponent, Mod extension in RSA RAM */
	XSecure_RsaPutData(InstancePtr);

	/* Initialize Digest */
	XSecure_RsaWriteMem(InstancePtr, Input,
				XSECURE_RSA_RAM_DIGEST);

	/** Initialize MINV values from Mod. */
	XSecure_RsaMod32Inverse(InstancePtr);

	/* Configurations */
	switch (InstancePtr->SizeInWords) {
		/* For 2048 key */
		case XSECURE_RSA_2048_SIZE_WORDS:
			XSecure_RsaDataLenCfg(InstancePtr,
					XSECURE_ECDSA_RSA_CFG0_2048_VALUE,
					XSECURE_ECDSA_RSA_CFG1_2048_VALUE,
					XSECURE_ECDSA_RSA_CFG2_2048_VALUE,
					XSECURE_ECDSA_RSA_CFG5_2048_VALUE);
			ErrorCode = XST_SUCCESS;
			break;

			/* For 3072 key */
		case XSECURE_RSA_3072_SIZE_WORDS:
			XSecure_RsaDataLenCfg(InstancePtr,
					XSECURE_ECDSA_RSA_CFG0_3072_VALUE,
					XSECURE_ECDSA_RSA_CFG1_3072_VALUE,
					XSECURE_ECDSA_RSA_CFG2_3072_VALUE,
					XSECURE_ECDSA_RSA_CFG5_3072_VALUE);
			ErrorCode = XST_SUCCESS;
			break;

			/* For 4096 key */
		case XSECURE_RSA_4096_SIZE_WORDS:
			XSecure_RsaDataLenCfg(InstancePtr,
					XSECURE_ECDSA_RSA_CFG0_4096_VALUE,
					XSECURE_ECDSA_RSA_CFG1_4096_VALUE,
					XSECURE_ECDSA_RSA_CFG2_4096_VALUE,
					XSECURE_ECDSA_RSA_CFG5_4096_VALUE);
			ErrorCode = XST_SUCCESS;
			break;

		default:
			ErrorCode = XST_FAILURE;
			break;
	}

	if (ErrorCode == XST_FAILURE) {
		goto END_RST;
	}

	/** Start the RSA operation. */
	if (InstancePtr->ModExtAddr != 0U) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_CTRL_OFFSET,
			XSECURE_RSA_CONTROL_EXP_PRE);
	}
	else {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_CTRL_OFFSET,
				XSECURE_RSA_CONTROL_EXP);
	}

	ErrorCode = XST_FAILURE;

	/* Check and wait for status */
	Status = (int)Xil_WaitForEvents((InstancePtr->BaseAddress +
					XSECURE_ECDSA_RSA_STATUS_OFFSET),
					(XSECURE_RSA_STATUS_DONE |
					XSECURE_RSA_STATUS_ERROR),
					(XSECURE_RSA_STATUS_DONE |
					XSECURE_RSA_STATUS_ERROR),
					XSECURE_TIMEOUT_MAX,
					&Events);

	/* Time out occurred or RSA error observed*/
	if (Status != XST_SUCCESS) {
		ErrorCode = Status;
		goto END_RST;
	}

	if((Events & XSECURE_RSA_STATUS_ERROR) == XSECURE_RSA_STATUS_ERROR)
	{
		ErrorCode = XST_FAILURE;
		goto END_RST;
	}
	/* Copy the result */
	XSecure_RsaGetData(InstancePtr, Result);

	ErrorCode = XST_SUCCESS;
END_RST:
	/* Revert configuring endianness for data */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_CFG_OFFSET,
		XSECURE_ECDSA_RSA_CFG_CLEAR_ENDIANNESS_MASK);

	/* Zeroize and Verify RSA memory space */
	if (InstancePtr->EncDec == (u8)XSECURE_RSA_SIGN_DEC) {
		Status = XSecure_RsaZeroize(InstancePtr);
		ErrorCode |= Status;
	}
	/* Reset core */
	XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

END:
	return ErrorCode;
}

/*****************************************************************************/
/**
 * @brief	This function writes all the RSA data used for decryption
 * 		(Modulus, Exponent) at the corresponding offsets in RSA RAM
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance
 *
 ******************************************************************************/
static void XSecure_RsaPutData(const XSecure_Rsa *InstancePtr)
{
	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	/* Initialize Modular exponentiation */
	XSecure_RsaWriteMem(InstancePtr, InstancePtr->ModExpoAddr,
				XSECURE_RSA_RAM_EXPO);

	/* Initialize Modular. */
	XSecure_RsaWriteMem(InstancePtr, InstancePtr->ModAddr,
				XSECURE_RSA_RAM_MOD);

	if (InstancePtr->ModExtAddr != 0U) {
		/* Initialize Modular extension (R*R Mod M) */
		XSecure_RsaWriteMem(InstancePtr, InstancePtr->ModExtAddr,
				XSECURE_RSA_RAM_RES_Y);
	}
}

/*****************************************************************************/
/**
 * @brief	This function reads back the resulting data from RSA RAM
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance
 * @param	RdDataAddr	Address of the location where RSA output data
 *				will be written
 *
 ******************************************************************************/
static void XSecure_RsaGetData(const XSecure_Rsa *InstancePtr, u64 RdDataAddr)
{
	u32 Index;
	u32 DataOffset;
	int TmpIndex;
	u32 WrOffSet;

	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);
	XSecure_AssertVoid(RdDataAddr != 0U);

	TmpIndex = (int)(InstancePtr->SizeInWords) - 1;

	/** Read the data from RSA RAM buffer and store it in the destination address */
	for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT;
			DataOffset++) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
			(XSECURE_RSA_RAM_RES_Y * XSECURE_RSA_MAX_RD_WR_CNT)
							+ DataOffset);

		for (Index = 0U; Index < XSECURE_RSA_MAX_BUFF; Index++) {
			if(TmpIndex < 0) {
				goto END;
			}
			WrOffSet = SIZEOF_INT_IN_BYTES * (u32)TmpIndex;
			XSecure_OutWord64(
				(RdDataAddr + (u64)WrOffSet),
				XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RAM_DATA_OFFSET));
			TmpIndex --;
		}
	}

END: ;
}

/*****************************************************************************/
/**
 * @brief	This function calculates the MINV value and put it into RSA
 *		core registers
 *
 * @param	InstancePtr	Pointer to XSeure_Rsa instance
 *
 * @note	MINV is the 32-bit value of `-M mod 2**32`,
 *		where M is LSB 32 bits of the original modulus
 *
 ******************************************************************************/
static void XSecure_RsaMod32Inverse(const XSecure_Rsa *InstancePtr)
{
	/* Calculate the MINV */
	u32 Count;
	u32 ModVal;
	u32 Inv;
	u32 OffSet = 0U;


	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	OffSet = (InstancePtr->SizeInWords - 1U) * SIZEOF_INT_IN_BYTES;
	ModVal = XSecure_In64((InstancePtr->ModAddr + OffSet));
	ModVal = Xil_Htonl(ModVal);
	Inv = (u32)2U - ModVal;

	for (Count = 0U; Count < XSECURE_WORD_SIZE; ++Count) {
		Inv = Inv * (2U - (ModVal * Inv));
	}

	Inv = ~Inv + 1U;

	/** Store calculated MINV value in RSA registers */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_MINV_OFFSET, (Inv));
}

/*****************************************************************************/
/**
 * @brief	This function writes data to RSA RAM at a given offset
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	WrDataAddr	Address of the data to be written to RSA RAM
 * @param	RamOffset	Offset for the data to be written in RSA RAM
 *
 ******************************************************************************/
static void XSecure_RsaWriteMem(const XSecure_Rsa *InstancePtr,
	u64 WrDataAddr, u8 RamOffset)
{
	u32 Index;
	u32 DataOffset;
	u32 TmpIndex;
	u32 Data;
	u32 OffSet;

	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);
	XSecure_AssertVoid(WrDataAddr != 0U);

	for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT;
			DataOffset++) {
		for (Index = 0U; Index < XSECURE_RSA_MAX_BUFF; Index++) {
			TmpIndex = (DataOffset * XSECURE_RSA_MAX_BUFF) + Index;

			/*
			 * Exponent size is only 4 bytes
			 * and rest of the data needs to be 0
			 */
			if((XSECURE_RSA_RAM_EXPO == RamOffset) &&
			  (InstancePtr->EncDec == (u8)XSECURE_RSA_SIGN_ENC)) {
				if(0U == TmpIndex ) {
					Data = XSecure_In64(WrDataAddr);
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
					OffSet = ((InstancePtr->SizeInWords - 1U) - TmpIndex) * SIZEOF_INT_IN_BYTES;
					Data = XSecure_In64((WrDataAddr + OffSet));
				}
			}
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_RAM_DATA_OFFSET,
							Data);
		}

		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
			((u32)(RamOffset * (u32)XSECURE_RSA_MAX_RD_WR_CNT) + DataOffset) |
			XSECURE_ECDSA_RSA_RAM_ADDR_WRRD_B_MASK);
	}
}

/*****************************************************************************/
/**
 * @brief	This function clears whole RSA memory space. This function clears
 * 		stored exponent, modulus and exponentiation key components along
 *		with digest
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XSECURE_RSA_INVALID_PARAM  On invalid parameter
 *
 *****************************************************************************/
int XSecure_RsaZeroize(const XSecure_Rsa *InstancePtr)
{

	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	/** Zeroize and verify whole RSA RAM space */
	Status = XSecure_RsaEcdsaZeroizeAndVerifyRam((u32)InstancePtr->BaseAddress);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_MINV_OFFSET, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates data length configuration.
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance.
 * @param	Cfg0		QSEL, Multiplication passes.
 * @param	Cfg1		Number of Montgomery digits.
 * @param	Cfg2		Memory location size.
 * @param	Cfg5		Number of groups.
 *
******************************************************************************/
static void XSecure_RsaDataLenCfg(const XSecure_Rsa *InstancePtr, u32 Cfg0,
	u32 Cfg1, u32 Cfg2, u32 Cfg5)
{
	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CFG0_OFFSET,
			Cfg0);
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CFG1_OFFSET,
			Cfg1);
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CFG2_OFFSET,
			Cfg2);
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CFG5_OFFSET,
			Cfg5);
}

/*****************************************************************************/
/**
 * @brief	This function returns PKCS padding as per the silicon version
 *
 * @return
 *		 - XSecure_Silicon2_TPadSha3
 *
*****************************************************************************/
u8* XSecure_RsaGetTPadding(void)
{
	return (u8 *)XSecure_Silicon2_TPadSha3;
}
#endif

#if !defined(PLM_RSA_EXCLUDE) || !defined(PLM_ECDSA_EXCLUDE)

/*****************************************************************************/
/**
 * @brief	This function verifies whole RSA or ECDSA memory space.
 *
 * @param	BaseAddress	BaseAddress of RSA or ECDSA controller.
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On Failure
 *
 *****************************************************************************/
static int XSecure_RsaEcdsaZeroizeVerify(u32 BaseAddress)
{
	volatile int Status = XST_FAILURE;
	volatile u32 RamOffset = 0U;
	volatile u32 DataOffset;
	u32 Index;
	u32 Data = 0U;

	do {
		for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT; DataOffset++) {
			XSecure_WriteReg(BaseAddress, XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
				((RamOffset * (u8)XSECURE_RSA_MAX_RD_WR_CNT) + DataOffset));
			for (Index = 0U; Index < XSECURE_RSA_MAX_BUFF; Index++) {
				Data |= XSecure_ReadReg(BaseAddress,
						XSECURE_ECDSA_RSA_RAM_DATA_OFFSET);
			}
			if (Data != 0U) {
				Status = (int)XSECURE_RSA_ECDSA_ZEROIZE_ERROR;
				goto END;
			}
		}
		RamOffset++;
	} while (RamOffset <= XSECURE_RSA_RAM_RES_Q);

	if(((RamOffset - 1U) == XSECURE_RSA_RAM_RES_Q) &&
		(DataOffset == XSECURE_RSA_MAX_RD_WR_CNT)) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears and verifies whole RSA or ECDSA memory space.
 *
 * @param	BaseAddress	BaseAddress of ECDSA or RSA controller.
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On Failure
 *
 *****************************************************************************/
int XSecure_RsaEcdsaZeroizeAndVerifyRam(u32 BaseAddress)
{
	volatile u32 RamOffset = 0U;
	volatile u32 DataOffset;
	volatile int Status = XST_FAILURE;

	/** Clears whole RSA or ECDSA RAM space */
	do {
		for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT; DataOffset++) {
			XSecure_WriteReg(BaseAddress, XSECURE_ECDSA_RSA_CTRL_OFFSET,
				XSECURE_ECDSA_RSA_CTRL_CLR_DATA_BUF_MASK);
			XSecure_WriteReg(BaseAddress,
				XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
				((RamOffset * (u8)XSECURE_RSA_MAX_RD_WR_CNT) + DataOffset) |
				XSECURE_ECDSA_RSA_RAM_ADDR_WRRD_B_MASK);
		}
		RamOffset++;
	} while (RamOffset <= XSECURE_RSA_RAM_RES_Q);

	/** Verify whether whole RSA or ECDSA RAM space is zeroized or not */
	Status = XSecure_RsaEcdsaZeroizeVerify(BaseAddress);

	return Status;
}

#endif
/** @} */
