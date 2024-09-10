/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xrsa.c
* @addtogroup Overview
* @{
* This file contains implementation of the interface functions for RSA hardware engine.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   07/11/24 Initial release
*       ss   08/20/24 Added 64-bit address support
*       yog  08/25/24 Integrated FIH library
*
* </pre>
*
**************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xrsa.h"
#include "Rsa.h"
#include "xil_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
/* Definitions for peripheral RSA */
#define XASU_RSA_BASEADDR			0xEBF50000U

#define XRSA_RESET_REG_OFFSET		(0x40U)		/**< RSA reset register offset */

/**< Errors from IPcores library */
#define XRSA_KEY_PAIR_COMP_ERROR	(1U)		/**< RSA IPcores keypair compare error */
#define XRSA_RAND_GEN_ERROR		(2U)		/**< RSA IPcores random number gen error */

#define XRSA_TOTAL_PARAMS		(9U)		/**< RSA total no of parameters */

#define XRSA_MAX_KEY_SIZE_IN_BYTES	(512U)		/**< RSA max key size in bytes */
#define XRSA_MAX_PRIME_SIZE_IN_BYTES	(256U)		/**< RSA max prime size in bytes */
#define XRSA_PUBEXP_SIZE_IN_BYTES	(4U)		/**< RSA public exponent size in bytes */
#define XRSA_MAX_PARAM_SIZE_IN_BYTES	(XRSA_TOTAL_PARAMS * XRSA_MAX_KEY_SIZE_IN_BYTES)

#define XRSA_HALF_LEN(x)		((x) >> 1U)	/**< Calculate half size */
#define XRSA_BYTE_TO_BIT(x)		((x) << 3U)	/**< Byte to bit conversion */

#define XRSA_TOTIENT_IS_PRSNT		(1U)		/**< indicates totient is present */
#define XRSA_PRIME_NUM_IS_PRSNT		(2U)		/**< indicates prime num is present */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XRsa_UpdateStatus(s32 Status);

/************************************ Variable Definitions ***************************************/
u8 Rsa_Data[XRSA_MAX_PARAM_SIZE_IN_BYTES];

/*************************************************************************************************/
/**
 * @brief	This function performs the RSA Private CRT decrypt operation.
 *
 * @param	DmaPtr		DMA pointer used for DMA copy.
 * @param	Len		length of Input and Output data in bytes.
 * @param	InputDataAddr	address to buffer of input data.
 * @param	OutputDataAddr	address to buffer of output data.
 * @param	KeyParamAddr	address to all the parameters required for CRT operation.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- XASUFW_RSA_CRT_OP_ERROR on operation error.
 *		- XASUFW_RSA_RAND_GEN_ERROR on random number generation failure.
 *		- XASUFW_RSA_KEY_PAIR_COMP_ERROR on key pair comparison failure.
 *		- XASUFW_RSA_ERROR on other errors.
 *
 *************************************************************************************************/
s32 XRsa_CrtOp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
	       u64 KeyParamAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 SStatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = Rsa_Data;
	XAsu_RsaCrtKeyComp *KeyPtr = (XAsu_RsaCrtKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *PubExpoArr = (u8 *)KeyPtr + sizeof(XAsu_RsaCrtKeyComp);
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		XFIH_GOTO(END);
	}
	/* Release the RSA engine from reset */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from client address to local buffer*/
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from client address to local buffer*/
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaCrtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubKeyComp.PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness(InData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PubKeyComp.Modulus, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Prime1, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Prime2, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->DP, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->DQ, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->QInv, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	XFIH_CALL(RSA_ExpCrtQ, XFihVar, Status, InData, (u8 *)KeyPtr->Prime1, (u8 *)KeyPtr->Prime2,
		  (u8 *)KeyPtr->DP, (u8 *)KeyPtr->DQ, (u8 *)KeyPtr->QInv, PubExpoArr,
		  (u8 *)KeyPtr->PubKeyComp.Modulus, XRSA_BYTE_TO_BIT(Len), OutData);
	Status = XRsa_UpdateStatus(Status);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from local buffer to client address*/
	XFIH_CALL(XAsufw_DmaXfr, XFihVar, Status, DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr,
		  Len, 0U);

END:
	SStatus = Xil_SMemSet(Rsa_Data, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS, 0U,
			      XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/* Reset the RSA engine */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs the RSA Private Exponentiation decryption operation.
 *
 * @param	DmaPtr		DMA pointer used for DMA copy.
 * @param	Len		Length of Input and Output Data in bytes.
 * @param	InputDataAddr	address to buffer of input data.
 * @param	OutputDataAddr	address to buffer of output data.
 * @param	KeyParamAddr	address to the parameters required for RSA operation.
 * @param	ExpoAddr	address to exponential parameters required for RSA operation.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- XASUFW_RSA_PVT_OP_ERROR on operation error.
 *		- XASUFW_RSA_RAND_GEN_ERROR on random number generation failure.
 *		- XASUFW_RSA_KEY_PAIR_COMP_ERROR on key pair comparison failure.
 *		- XASUFW_RSA_ERROR on other errors.
 *
 *************************************************************************************************/
s32 XRsa_PvtExp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
		u64 KeyParamAddr, u64 ExpoAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 SStatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = Rsa_Data;
	XAsu_RsaPvtKeyComp *KeyPtr = (XAsu_RsaPvtKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *RRN = (u8 *)KeyPtr + sizeof(XAsu_RsaPvtKeyComp);
	u8 *RN = RRN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *PubExpoArr = RN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Release the RSA engine from reset */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from client address to local buffer*/
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from client address to local buffer*/
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaPvtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubKeyComp.PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness(InData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PubKeyComp.Modulus, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PvtExp, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_TOTIENT_IS_PRSNT) {
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient, Len);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_PRIME_NUM_IS_PRSNT) {
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient,
						 XRSA_HALF_LEN(Len));
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient +
						 XRSA_MAX_PRIME_SIZE_IN_BYTES,
						 XRSA_HALF_LEN(Len));
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	}

	if (ExpoAddr == 0U) {
		if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_TOTIENT_IS_PRSNT) {
			XFIH_CALL(RSA_ExpQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, NULL, NULL,
				  PubExpoArr, (u8 *)KeyPtr->PrimeCompOrTotient,
				  XRSA_BYTE_TO_BIT(Len), OutData);
		} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_PRIME_NUM_IS_PRSNT) {
			XFIH_CALL(RSA_ExpQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus,
				  (u8 *)KeyPtr->PrimeCompOrTotient,
				  (u8 *)KeyPtr->PrimeCompOrTotient +
				  XRSA_MAX_PRIME_SIZE_IN_BYTES, PubExpoArr, NULL,
				  XRSA_BYTE_TO_BIT(Len), OutData);
		} else {
			XFIH_CALL(RSA_ExpQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, NULL, NULL, PubExpoArr,
				  NULL, XRSA_BYTE_TO_BIT(Len), OutData);
		}
		Status = XRsa_UpdateStatus(Status);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, ExpoAddr, (u64)(UINTPTR)RRN, sizeof(XAsu_RsaRModN),
				       0U);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		Status = XAsufw_ChangeEndianness(RRN, Len);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		Status = XAsufw_ChangeEndianness(RN, Len);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_TOTIENT_IS_PRSNT) {
			XFIH_CALL(RSA_ExpoptQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, RN, RRN, NULL, NULL,
				  PubExpoArr, (u8 *)KeyPtr->PrimeCompOrTotient,
				  XRSA_BYTE_TO_BIT(Len), OutData);
		} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_PRIME_NUM_IS_PRSNT) {
			XFIH_CALL(RSA_ExpoptQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, RN, RRN,
				  (u8 *)KeyPtr->PrimeCompOrTotient,
				  (u8 *)KeyPtr->PrimeCompOrTotient +
				  XRSA_MAX_PRIME_SIZE_IN_BYTES, PubExpoArr, NULL,
				  XRSA_BYTE_TO_BIT(Len), OutData);
		} else {
			XFIH_CALL(RSA_ExpoptQ, XFihVar, Status, InData, (u8 *)KeyPtr->PvtExp,
				  (u8 *)KeyPtr->PubKeyComp.Modulus, RN, RRN,
				  NULL, NULL, PubExpoArr, NULL, XRSA_BYTE_TO_BIT(Len),
				  OutData);
		}
		Status = XRsa_UpdateStatus(Status);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	}

	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from local buffer to client address*/
	XFIH_CALL(XAsufw_DmaXfr, XFihVar, Status, DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr,
		  Len, 0U);

END:
	SStatus = Xil_SMemSet(Rsa_Data, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS, 0U,
			      XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/* Reset the RSA engine */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs the RSA Public Exponentiation encryption operation.
 *
 * @param	DmaPtr		DMA pointer used for DMA copy.
 * @param	Len		Length of Input and Output Data in bytes.
 * @param	InputDataAddr	address to buffer of input data.
 * @param	OutputDataAddr	address to buffer of output data.
 * @param	KeyParamAddr	address to the parameters required for RSA operation.
 * @param	ExpoAddr	address to exponential parameters required for RSA operation.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- XASUFW_RSA_PUB_OP_ERROR on operation error.
 *		- XASUFW_RSA_RAND_GEN_ERROR on random number generation failure.
 *		- XASUFW_RSA_KEY_PAIR_COMP_ERROR on key pair comparison failure.
 *		- XASUFW_RSA_ERROR on other errors.
 *
 *************************************************************************************************/
s32 XRsa_PubExp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
		u64 KeyParamAddr, u64 ExpoAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 SStatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = Rsa_Data;
	XAsu_RsaPubKeyComp *KeyPtr = (XAsu_RsaPubKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *RRN = (u8 *)KeyPtr + sizeof(XAsu_RsaPubKeyComp);
	u8 *PubExpoArr = RRN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Release the RSA engine from reset */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from client address to local buffer*/
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from client address to local buffer*/
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaPvtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness(InData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Modulus, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	if (ExpoAddr == 0) {
		rsaexp(InData, PubExpoArr, (u8 *)KeyPtr->Modulus, XRSA_BYTE_TO_BIT(Len), OutData);
	} else {
		Status = XAsufw_DmaXfr(DmaPtr, ExpoAddr, (u64)(UINTPTR)RRN, sizeof(XAsu_RsaRRModN),
				       0U);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		Status = XAsufw_ChangeEndianness(RRN, Len);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		rsaexpopt(InData, PubExpoArr, (u8 *)KeyPtr->Modulus, RRN, XRSA_BYTE_TO_BIT(Len),
			  OutData);
	}

	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DMA transfer from local buffer to client address*/
	XFIH_CALL(XAsufw_DmaXfr, XFihVar, Status, DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr,
		  Len, 0U);

END:
	SStatus = Xil_SMemSet(Rsa_Data, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS, 0U,
			      XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/* Reset the RSA engine */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function takes IPcores lib return status and map to respective ASUFW status.
 *
 * @param	Status	is the status which is to be updated.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_RSA_RAND_GEN_ERROR on random number generation failure.
 *		- XASUFW_RSA_KEY_PAIR_COMP_ERROR on key pair comparison failure.
 *		- XASUFW_RSA_ERROR on other errors.
 *
 *************************************************************************************************/
static s32 XRsa_UpdateStatus(s32 Status)
{
	s32 SStatus = XASUFW_FAILURE;

	if (Status == XRSA_RAND_GEN_ERROR) {
		SStatus = XASUFW_RSA_RAND_GEN_ERROR;
	} else if (Status == XRSA_KEY_PAIR_COMP_ERROR) {
		SStatus = XASUFW_RSA_KEY_PAIR_COMP_ERROR;
	} else if (Status != XASUFW_SUCCESS) {
		SStatus = XASUFW_RSA_ERROR;
	} else {
		SStatus = XASUFW_SUCCESS;
	}

	return SStatus;
}
