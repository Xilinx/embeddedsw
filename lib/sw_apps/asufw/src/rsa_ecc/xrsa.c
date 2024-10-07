/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xrsa.c
*
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
*       ss   09/26/24 Fixed doxygen comments
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xrsa_server_apis RSA Server APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "xrsa.h"
#include "Rsa.h"
#include "xil_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
/* Definitions for peripheral RSA */
#define XASU_RSA_BASEADDR		(0xEBF50000U)	/**< RSA base address */
#define XRSA_RESET_REG_OFFSET		(0x40U)		/**< RSA reset register offset */

/* Errors from IPcores library */
#define XRSA_KEY_PAIR_COMP_ERROR	(1)		/**< RSA IPcores keypair compare error */
#define XRSA_RAND_GEN_ERROR		(2)		/**< RSA IPcores random number generation
								error */

#define XRSA_TOTAL_PARAMS		(9U)		/**< RSA total no of parameters */

#define XRSA_MAX_KEY_SIZE_IN_BYTES	(512U)		/**< RSA max key size in bytes */
#define XRSA_MAX_PRIME_SIZE_IN_BYTES	(256U)		/**< RSA max prime size in bytes */
#define XRSA_PUBEXP_SIZE_IN_BYTES	(4U)		/**< RSA public exponent size in bytes */
#define XRSA_MAX_PARAM_SIZE_IN_BYTES	(XRSA_TOTAL_PARAMS * XRSA_MAX_KEY_SIZE_IN_BYTES) /**< Size
							of memory allocated for RSA parameters */

#define XRSA_HALF_LEN(x)		((x) >> 1U)	/**< Calculate half value */
#define XRSA_BYTE_TO_BIT(x)		((s32)((x) << 3U)) /**< Byte to bit conversion */

#define XRSA_TOTIENT_IS_PRSNT		(1U)		/**< Indicates totient is present as
								parameter for private decryption
								operation*/
#define XRSA_PRIME_NUM_IS_PRSNT		(2U)		/**< Indicates prime num is present as
								parameter for private decryption
								operation*/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XRsa_UpdateStatus(s32 Status);

/************************************ Variable Definitions ***************************************/
u8 Rsa_Data[XRSA_MAX_PARAM_SIZE_IN_BYTES]; /**< Memory allocated for RSA parameters */

/*************************************************************************************************/
/**
 * @brief	This function performs RSA decryption using CRT algorithm for the provided
 * 		message by using private key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	Len		Length of Input and Output data in bytes.
 * @param	InputDataAddr	Address of the input data buffer.
 * @param	OutputDataAddr	Address of the input data buffer.
 * @param	KeyParamAddr	Address to all the parameters required for private decrypt
 * 				operation using CRT algorithm.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
* 		- Also can return termination error codes from 0x9CU to 0x9FU ,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_CrtOp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
	       u64 KeyParamAddr)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XFih_VolatileAssign(XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = Rsa_Data;
	XAsu_RsaCrtKeyComp *KeyPtr = (XAsu_RsaCrtKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *PubExpoArr = (u8 *)KeyPtr + sizeof(XAsu_RsaCrtKeyComp);
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/** Release the RSA engine from reset. */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy input data to server memory using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy key parameters to server memory using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaCrtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Copy public exponent to pointer. */
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubKeyComp.PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Endianness change from BE to LE for following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - first prime number.
	 * - second prime numbe.
	 * - derived value of first prime number.
	 * - derived value of second prime number.
	 * - Inverse of derived value of second prime number.
	*/
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

	/** Perform private decryption operation using CRT algorithm. */
	XFIH_CALL(RSA_ExpCrtQ, XFihVar, Status, InData, (u8 *)KeyPtr->Prime1, (u8 *)KeyPtr->Prime2,
		  (u8 *)KeyPtr->DP, (u8 *)KeyPtr->DQ, (u8 *)KeyPtr->QInv, PubExpoArr,
		  (u8 *)KeyPtr->PubKeyComp.Modulus, XRSA_BYTE_TO_BIT(Len), OutData);
	Status = XRsa_UpdateStatus(Status);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Endianness change from LE to BE for output data. */
	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy output data to server memory using DMA. */
	XFIH_CALL(XAsufw_DmaXfr, XFihVar, Status, DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr,
		  Len, 0U);

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = Xil_SMemSet(Rsa_Data, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS, 0U,
			      XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Reset the RSA engine. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA decryption for the provided message by using
 * 		private key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	Len		Length of Input and Output Data in bytes.
 * @param	InputDataAddr	Address of the input data buffer.
 * @param	OutputDataAddr	Address of the input data buffer.
 * @param	KeyParamAddr	Address to the parameters required for RSA operation.
 * @param	ExpoAddr	Address to exponential parameters required for RSA operation.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- Also can return termination error codes from 0x9CU to 0x9EU and 0xA0U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PvtExp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
		u64 KeyParamAddr, u64 ExpoAddr)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XFih_VolatileAssign(XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = Rsa_Data;
	XAsu_RsaPvtKeyComp *KeyPtr = (XAsu_RsaPvtKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *RRN = (u8 *)KeyPtr + sizeof(XAsu_RsaPvtKeyComp);
	u8 *RN = RRN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *PubExpoArr = RN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/** Release the RSA engine from reset */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy input data to server memory using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy key parameters to server memory using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaPvtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Copy public exponent to pointer*/
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubKeyComp.PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Endianness change from BE to LE for following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - Private exponent.
	 * - Prime number or totient.
	 * - Pre calculated exponent values if available(R mod N,R square mod N).
	*/
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

	/** Perform private exponentiation operation by calculating exponentiation values or with
		pre calculated exponentiation values and with totient or prime numbers or without
		totient and prime numbers based on available parameters. */
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
		/* DMA transfer of pre-calculated modulus values from client address to server
		memory if available */
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

	/** Endianness change from LE to BE for output data. */
	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy output data to server memory using DMA. */
	XFIH_CALL(XAsufw_DmaXfr, XFihVar, Status, DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr,
		  Len, 0U);

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = Xil_SMemSet(Rsa_Data, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS, 0U,
			      XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Reset the RSA engine. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA encryption for the provided message by using
 * 		public key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	Len		Length of Input and Output Data in bytes.
 * @param	InputDataAddr	Address of the input data buffer.
 * @param	OutputDataAddr	Address of the input data buffer.
 * @param	KeyParamAddr	Address to the parameters required for RSA operation.
 * @param	ExpoAddr	Address to exponential parameters required for RSA operation.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- Also can return termination error codes from 0x9CU to 0x9EU and 0xA1U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PubExp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
		u64 KeyParamAddr, u64 ExpoAddr)
{
	s32 Status = XFih_VolatileAssign(XASUFW_FAILURE);
	s32 SStatus = XFih_VolatileAssign(XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = Rsa_Data;
	XAsu_RsaPubKeyComp *KeyPtr = (XAsu_RsaPubKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *RRN = (u8 *)KeyPtr + sizeof(XAsu_RsaPubKeyComp);
	u8 *PubExpoArr = RRN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/** Release the RSA engine from reset. */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy input data to server memory using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy key parameters to server memory using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaPvtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Copy public exponent to pointer*/
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Endianness change from BE to LE for following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - Pre calculated exponent value if available(R square mod N).
	*/
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

	/** Perform public exponentiation operation by calculating exponentiation values or with
		pre calculated exponentiation values based on available parameters. */
	if (ExpoAddr == 0U) {
		rsaexp(InData, PubExpoArr, (u8 *)KeyPtr->Modulus, XRSA_BYTE_TO_BIT(Len), OutData);
	} else {
		/* DMA transfer of pre-calculated modulus values from client address to server
			memory */
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

	/** Endianness change from LE to BE for output data. */
	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/** Copy output data to server memory using DMA. */
	XFIH_CALL(XAsufw_DmaXfr, XFihVar, Status, DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr,
		  Len, 0U);

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = Xil_SMemSet(Rsa_Data, XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS, 0U,
			      XRSA_MAX_KEY_SIZE_IN_BYTES * XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Reset the RSA engine. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function maps the status returned from IP cores to the respective error
 * 		from xasufw_status.h.
 *
 * @param	Status	Status returned from IPcores library.
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

	/* Updates status which relates to ASUFW_RSA module. */
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
/** @} */
