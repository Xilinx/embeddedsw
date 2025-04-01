/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasu_rsainfo.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "Rsa.h"
#include "xil_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
/* Definitions for peripheral RSA */
#define XASU_RSA_BASEADDR		(0xEBF50000U)	/**< RSA base address */
#define XRSA_RESET_REG_OFFSET		(0x40U)		/**< RSA reset register offset */

/* Errors from IPcores library */
#define XRSA_KEY_PAIR_COMP_ERROR	(1)		/**< RSA IPcores keypair compare error */
#define XRSA_RAND_GEN_ERROR		(2)		/**< RSA IPcores random number generation error */

#define XRSA_MAX_PRIME_SIZE_IN_BYTES	(256U)		/**< RSA max prime size in bytes */
#define XRSA_PUBEXP_SIZE_IN_BYTES	(4U)		/**< RSA public exponent size in bytes */

#define XRSA_HALF_LEN(x)		((x) >> 1U)	/**< Calculate half value */
#define XRSA_BYTE_TO_BIT(x)		((s32)((x) << 3U)) /**< Byte to bit conversion */

#define XRSA_NO_PRIME_NO_TOT_PRSNT	(0U)		/**< Indicates no prime num or totient
								is present as parameter for private decryption operation */
#define XRSA_TOTIENT_IS_PRSNT		(1U)		/**< Indicates totient is present as
								parameter for private decryption operation */
#define XRSA_PRIME_NUM_IS_PRSNT		(2U)		/**< Indicates prime num is present as
								parameter for private decryption operation */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XRsa_UpdateStatus(s32 Status);
static s32 XRsa_ValidatePubExp(u8 *BuffAddr);
static s32 XRsa_ValidateModulus(u8 *BuffAddr, u8 *InputData, u32 Len);

/************************************ Variable Definitions ***************************************/
/** RSA data block memory allocated in ASU RAM. */
static u8  Rsa_Data[XRSA_MAX_PARAM_SIZE_IN_BYTES] __attribute__ ((section (".rsa_data_block")));

/*************************************************************************************************/
/**
 * @brief	This function performs RSA decryption using CRT algorithm for the provided
 * 		message by using private key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	Len		Length of Input and Output data in bytes.
 * @param	InputDataAddr	Address of the input data buffer.
 * @param	OutputDataAddr	Address of the output data buffer.
 * @param	KeyParamAddr	Address to all the parameters required for private decrypt
 * 				operation using CRT algorithm.
 *
 * @return
 *		- XASUFW_SUCCESS, if RSA decryption using CRT algorithm is successful.
 *		- XASUFW_FAILURE, if RSA decryption using CRT algorithm fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_RSA_ZEROIZE_MEMSET_FAIL, if zeroize memset fails.
 *		- XASUFW_RSA_DMA_COPY_FAIL, if DMA copy fails.
 *		- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change error occurs.
 *		- XASUFW_RSA_MEM_COPY_FAIL, if memory copy fails.
 * 		- Also, this function can return termination error codes from 0x9CU to 0x9FU
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_CrtOp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
	       u64 KeyParamAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = XRsa_GetDataBlockAddr();
	XAsu_RsaCrtKeyComp *KeyPtr = (XAsu_RsaCrtKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *PubExpoArr = (u8 *)KeyPtr + sizeof(XAsu_RsaCrtKeyComp);
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U) ||
		(DmaPtr == NULL)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	Status = XAsu_RsaValidateKeySize(Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset. */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Copy input data to server memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy key parameters to server memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaCrtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy public exponent to pointer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubKeyComp.PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_ValidatePubExp(PubExpoArr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PUB_EXP_INVALID_VALUE;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_IsBufferNonZero((u8 *)KeyPtr->PubKeyComp.Modulus, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MOD_DATA_IS_ZERO;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	SStatus = XRsa_ValidateModulus((u8 *)KeyPtr->PubKeyComp.Modulus, InData, Len);
	if (SStatus != XASUFW_SUCCESS) {
		Status = SStatus;
		goto END;
	}

	/**
	 * Endianness change from BE to LE for following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - first prime number.
	 * - second prime number.
	 * - derived value of first prime number.
	 * - derived value of second prime number.
	 * - Inverse of derived value of second prime number.
	 */

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(InData, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PubKeyComp.Modulus, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Prime1, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Prime2, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->DP, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->DQ, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->QInv, XRSA_HALF_LEN(Len));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Perform private decryption operation using CRT algorithm. */
	XFIH_CALL(RSA_ExpCrtQ, XFihVar, Status, InData, (u8 *)KeyPtr->Prime1, (u8 *)KeyPtr->Prime2,
		  (u8 *)KeyPtr->DP, (u8 *)KeyPtr->DQ, (u8 *)KeyPtr->QInv, PubExpoArr,
		  (u8 *)KeyPtr->PubKeyComp.Modulus, XRSA_BYTE_TO_BIT(Len), OutData);
	if (Status != XASUFW_SUCCESS) {
		Status = XRsa_UpdateStatus(Status);
		goto END;
	}

	/** Endianness change from LE to BE for output data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Copy output data to user memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr, Len, 0U);

END:
	/** Zeroize local copy of all the parameters. */
	ASSIGN_VOLATILE(SStatus, XASUFW_FAILURE);
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)InData, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
					XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Set the RSA engine under reset. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA decryption for the provided message by using private key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	Len		Length of Input and Output Data in bytes.
 * @param	InputDataAddr	Address of the input data buffer.
 * @param	OutputDataAddr	Address of the output data buffer.
 * @param	KeyParamAddr	Address to the parameters required for RSA operation.
 * @param	ExpoAddr	Address to exponential parameters required for RSA operation.
 *
 * @return
 *		- XASUFW_SUCCESS, if RSA decryption is successful.
 *		- XASUFW_FAILURE, if RSA decryption fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_RSA_ZEROIZE_MEMSET_FAIL, if zeroize memset fails.
 *		- XASUFW_RSA_DMA_COPY_FAIL, if DMA copy fails.
 *		- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change error occurs.
 *		- XASUFW_RSA_MEM_COPY_FAIL, if memory copy fails.
 *		- XASUFW_RSA_INVALID_PRIME_TOT_FLAG, if invalid prime/totient flag.
 *		- Also, this function can return termination error codes from 0x9CU to 0x9EU and 0xA0U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PvtExp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
		u64 KeyParamAddr, u64 ExpoAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 *InData = XRsa_GetDataBlockAddr();
	XAsu_RsaPvtKeyComp *KeyPtr = (XAsu_RsaPvtKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *RRN = (u8 *)KeyPtr + sizeof(XAsu_RsaPvtKeyComp);
	u8 *RN = RRN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *PubExpoArr = RN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U) ||
		(DmaPtr == NULL)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	Status = XAsu_RsaValidateKeySize(Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset. */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Copy input data to server memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy key parameters to server memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaPvtKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy public exponent to pointer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubKeyComp.PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_ValidatePubExp(PubExpoArr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PUB_EXP_INVALID_VALUE;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_IsBufferNonZero((u8 *)KeyPtr->PubKeyComp.Modulus, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MOD_DATA_IS_ZERO;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	SStatus = XRsa_ValidateModulus((u8 *)KeyPtr->PubKeyComp.Modulus, InData, Len);
	if (SStatus != XASUFW_SUCCESS) {
		Status = SStatus;
		goto END;
	}


	/**
	 * Endianness change from BE to LE for following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - Private exponent.
	 * - Prime number or totient.
	 * - Pre calculated exponent values if available (R mod N,R square mod N).
	 */

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(InData, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PubKeyComp.Modulus, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PvtExp, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_TOTIENT_IS_PRSNT) {
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient, Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
	} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_PRIME_NUM_IS_PRSNT) {
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient,
						 XRSA_HALF_LEN(Len));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->PrimeCompOrTotient +
						 XRSA_MAX_PRIME_SIZE_IN_BYTES,
						 XRSA_HALF_LEN(Len));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
	} else if (KeyPtr->PrimeCompOrTotientPrsnt == XRSA_NO_PRIME_NO_TOT_PRSNT) {
		Status = XASUFW_SUCCESS;
	} else {
		Status = XASUFW_RSA_INVALID_PRIME_TOT_FLAG;
		goto END;
	}

	/**
	 * Perform private exponentiation operation by calculating exponentiation values or with
	 * pre calculated exponentiation values and with totient or prime numbers or without
	 * totient and prime numbers based on available parameters.
	 */
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
	} else {
		/* DMA transfer of pre-calculated modulus values from client address to server
		memory if available */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, ExpoAddr, (u64)(UINTPTR)RRN, sizeof(XAsu_RsaRModN),
				       0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_DMA_COPY_FAIL;
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(RRN, Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(RN, Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
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

	}
	if (Status != XASUFW_SUCCESS) {
		Status = XRsa_UpdateStatus(Status);
		goto END;
	}

	/** Endianness change from LE to BE for output data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Copy output data to user memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr, Len, 0U);

END:
	/** Zeroize local copy of all the parameters. */
	ASSIGN_VOLATILE(SStatus, XASUFW_FAILURE);
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)InData, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
					XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Set the RSA engine under reset. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA encryption for the provided message by using public key.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	Len		Length of Input and Output Data in bytes.
 * @param	InputDataAddr	Address of the input data buffer.
 * @param	OutputDataAddr	Address of the output data buffer.
 * @param	KeyParamAddr	Address to the parameters required for RSA operation.
 * @param	ExpoAddr	Address to exponential parameters required for RSA operation.
 *
 * @return
 *		- XASUFW_SUCCESS, if RSA encryption is successful.
 *		- XASUFW_FAILURE, if RSA encryption fails.
 *		- XASUFW_RSA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_RSA_ZEROIZE_MEMSET_FAIL, if zeroize memset fails.
 *		- XASUFW_RSA_DMA_COPY_FAIL, if DMA copy fails.
 *		- XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, if endianness change error occurs.
 *		- XASUFW_RSA_MEM_COPY_FAIL, if memory copy fails.
 *		- Also, this function can return termination error codes from 0x9CU to 0x9EU and 0xA1U
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XRsa_PubExp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
		u64 KeyParamAddr, u64 ExpoAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);
	u8 *InData = XRsa_GetDataBlockAddr();
	XAsu_RsaPubKeyComp *KeyPtr = (XAsu_RsaPubKeyComp *)(InData + XRSA_MAX_KEY_SIZE_IN_BYTES);
	u8 *RRN = (u8 *)KeyPtr + sizeof(XAsu_RsaPubKeyComp);
	u8 *PubExpoArr = RRN + XRSA_MAX_KEY_SIZE_IN_BYTES;
	u8 *OutData = PubExpoArr + XRSA_MAX_KEY_SIZE_IN_BYTES;

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (KeyParamAddr == 0U) || (OutputDataAddr == 0U) ||
		(DmaPtr == NULL)) {
			Status = XASUFW_RSA_INVALID_PARAM;
			goto END;
	}

	Status = XAsu_RsaValidateKeySize(Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset. */
	XAsufw_CryptoCoreReleaseReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, 0U,
			     XRSA_MAX_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Copy input data to server memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr, (u64)(UINTPTR)InData, Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy key parameters to server memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, KeyParamAddr, (u64)(UINTPTR)KeyPtr,
			       sizeof(XAsu_RsaPubKeyComp), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy public exponent to pointer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(PubExpoArr, XRSA_MAX_KEY_SIZE_IN_BYTES, &(KeyPtr->PubExp),
			     XRSA_PUBEXP_SIZE_IN_BYTES, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_ValidatePubExp(PubExpoArr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PUB_EXP_INVALID_VALUE;
		goto END;
	}

	/**
	 * Endianness change from BE to LE for following components
	 * - Input Data.
	 * - Public exponent.
	 * - Modulus.
	 * - Pre calculated exponent value if available (R square mod N).
	 */

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(InData, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(PubExpoArr, XRSA_PUBEXP_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness((u8 *)KeyPtr->Modulus, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/**
	 * Perform public exponentiation operation by calculating exponentiation values or with
	 * pre calculated exponentiation values based on available parameters.
	 */
	if (ExpoAddr == 0U) {
		rsaexp(InData, PubExpoArr, (u8 *)KeyPtr->Modulus, XRSA_BYTE_TO_BIT(Len), OutData);
	} else {
		/* DMA transfer of pre-calculated modulus values from client address to server
			memory */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, ExpoAddr, (u64)(UINTPTR)RRN, sizeof(XAsu_RsaRRModN),
				       0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_DMA_COPY_FAIL;
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_ChangeEndianness(RRN, Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
			goto END;
		}
		rsaexpopt(InData, PubExpoArr, (u8 *)KeyPtr->Modulus, RRN, XRSA_BYTE_TO_BIT(Len),
			  OutData);
	}

	/** Endianness change from LE to BE for output data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(OutData, Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/** Copy output data to user memory using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OutData, OutputDataAddr, Len, 0U);

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)InData, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
					XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	/** Set the RSA engine under reset. */
	XAsufw_CryptoCoreSetReset(XASU_RSA_BASEADDR, XRSA_RESET_REG_OFFSET);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to get RSA data block address.
 *
 * @return
 *		- Address of the data block.
 *
 *************************************************************************************************/
u8 *XRsa_GetDataBlockAddr(void)
{
	return Rsa_Data;
}

/*************************************************************************************************/
/**
 * @brief	This function maps the status returned from IP cores to the respective error
 * 		from xasufw_status.h.
 *
 * @param	Status	Status returned from IPcores library.
 *
 * @return
 *		- XASUFW_RSA_RAND_GEN_ERROR, if random number generation fails.
 *		- XASUFW_RSA_KEY_PAIR_COMP_ERROR, if key pair comparison fails.
 *		- XASUFW_RSA_ERROR, if other errors occur.
 *
 *************************************************************************************************/
static s32 XRsa_UpdateStatus(s32 Status)
{
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);

	/* Updates status which relates to ASUFW_RSA module. */
	if (Status == XRSA_RAND_GEN_ERROR) {
		SStatus = XASUFW_RSA_RAND_GEN_ERROR;
	} else if (Status == XRSA_KEY_PAIR_COMP_ERROR) {
		SStatus = XASUFW_RSA_KEY_PAIR_COMP_ERROR;
	} else if (Status != XASUFW_SUCCESS) {
		SStatus = XASUFW_RSA_ERROR;
	}

	return SStatus;
}

/*************************************************************************************************/
/**
 * @brief	This function validates public exponent value.
 *
 * @param	BuffAddr	Buffer address of public exponent.
 *
 * @return
 *		- XASUFW_SUCCESS, if valid public exponent value is given.
 *		- XASUFW_FAILURE, if invalid public exponent value is given.
 *
 *************************************************************************************************/
static s32 XRsa_ValidatePubExp(u8 *BuffAddr)
{
	s32 Status = XASUFW_FAILURE;
	s32 PubExpVal = 0U;

	PubExpVal = BuffAddr[XRSA_PUBEXP_SIZE_IN_BYTES - 4U] << 24U |
			BuffAddr[XRSA_PUBEXP_SIZE_IN_BYTES - 3U] << 16U |
				BuffAddr[XRSA_PUBEXP_SIZE_IN_BYTES - 2U] << 8U |
					BuffAddr[XRSA_PUBEXP_SIZE_IN_BYTES - 1U];

	if ((PubExpVal != 0U) && (PubExpVal != 1U) && (PubExpVal != 3U)) {
		Status = XASUFW_SUCCESS;
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks the modulus data is valid or not
 *
 * @param	BuffAddr	Pointer to the buffer whose value needs to be checked.
 * @param	InputData	Pointer to the buffer whose value needs to be checked with.
 * @param       Len		Length of the buffers
 *
 * @return
 *		- XASUFW_SUCCESS, if modulus data is greater than input data.
 *		- XASUFW_RSA_MOD_DATA_INVALID, if modulus data less than input data.
 *		- XASUFW_RSA_MOD_DATA_INPUT_DATA_EQUAL, if modulus data equal to input data.
 *
 *************************************************************************************************/
static s32 XRsa_ValidateModulus(u8 *BuffAddr, u8 *InputData, u32 Len)
{
	s32 Status = XASUFW_FAILURE;
	volatile u32 Index = 0U;

	for (Index = 0U; Index < Len; Index++) {
		if (BuffAddr[Index] > InputData[Index]) {
			Status = XASUFW_SUCCESS;
			break;
		}
		if (BuffAddr[Index] < InputData[Index]) {
			Status = XASUFW_RSA_MOD_DATA_INVALID;
			break;
		}
	}

	if (Index == Len) {
		Status = XASUFW_RSA_MOD_DATA_INPUT_DATA_EQUAL;
	}

	return Status;
}
/** @} */
