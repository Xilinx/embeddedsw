/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_aes.c
* This file contains Versal Net specific code for Xilsecure aes server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     06/20/23 Initial release
* 5.4   yog     04/29/24 Fixed doxygen warnings.
*       kpt     06/13/24 Added AES key unwrap with padding support.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_server_apis XilSecure AES Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_plat_aes.h"
#include "xsecure_error.h"
#include "xsecure_aes_core_hw.h"
#include "xplmi_dma.h"

/************************** Constant Definitions *****************************/

#define XSECURE_AES_KEY_WRAP_MAX_ROUNDS      (5U)             	/**< AES key wrap maximum rounds */
#define XSECURE_AES_256BIT_MAX_KEY_BLOCK_ROUNDS    (4U)         /**< AES 256-bit number of blocks */
#define XSECURE_AES_128BIT_MAX_KEY_BLOCK_ROUNDS    (2U)         /**< AES 128-bit number of blocks */
#define XSECURE_AES_KEY_WRAP_INIT_DEF_VAL		(0xA6A6A6A6A6A6A6A6U) /**< AES init default value */

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static int XSecure_AesEcbDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function sets and initializes AES in ECB mode
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	KeySrc		Key Source for decryption of the data
 * @param	KeySize		Size of the AES key to be used for decryption is
 *		 		- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On Failure
 *
 *****************************************************************************/
static int XSecure_AesEcbDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize)
{
	int Status = XST_FAILURE;

	Status = XSecure_AesEcbCfg(InstancePtr, XSECURE_AES_ECB_MODE_EN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesDecryptInit(InstancePtr, KeySrc, KeySize, 0x00U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets AES ECB mode
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	EcbModeFlag	Flag to enable/disable ECB mode
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  On state mismatch
 *
 *****************************************************************************/
int XSecure_AesEcbCfg(XSecure_Aes *InstancePtr, u32 EcbModeFlag)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	if ((EcbModeFlag != XSECURE_AES_ECB_MODE_EN) && (EcbModeFlag != XSECURE_AES_ECB_MODE_DIS)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_ECB_OFFSET, EcbModeFlag);
	InstancePtr->IsEcbEn = (u32)TRUE;
	Status = XST_SUCCESS;

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function writes AES key into XSECURE_AES_USER_KEY_7 and decrypts the data
 *		in ECB mode
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	KeyAddr		Address of the key to be programmed in XSECURE_AES_USER_KEY_7
 * @param	KeySize		Size of the AES key to be used for decryption is
 *				- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *				decrypted
 * @param	OutDataAddr	Address of output buffer where the decrypted data
 *				to be updated
 * @param	Size		Size of data to be decrypted in bytes, whereas number of bytes shall be aligned as below
 *				- 16 byte aligned when it is not the last chunk
 *				- 4 byte aligned when the data is the last chunk
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On Failure
 *
 *****************************************************************************/
int XSecure_AesEcbDecrypt(XSecure_Aes *InstancePtr, u64 KeyAddr, XSecure_AesKeySize KeySize, u64 InDataAddr,
			  u64 OutDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	/* Validate input arguments and write AES key */
	Status = XSecure_AesWriteKey(InstancePtr, XSECURE_AES_USER_KEY_7, KeySize, KeyAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Validate and put AES in ECB and decryption mode */
	Status = XSecure_AesEcbDecryptInit(InstancePtr, XSECURE_AES_USER_KEY_7, KeySize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesDecryptUpdate(InstancePtr, InDataAddr, OutDataAddr, Size, TRUE);

END:
	if (InstancePtr != NULL) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				 XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);
		XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_ECB_OFFSET, XSECURE_AES_ECB_MODE_DIS);
		/* Clear AES Key */
		SStatus = XSecure_AesKeyZero(InstancePtr, XSECURE_AES_USER_KEY_7);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
		InstancePtr->IsEcbEn = (u32)FALSE;
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
				 XSECURE_AES_SOFT_RST_OFFSET);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function unwraps the given AES wrapped key.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	EphAesKey	Address of the key to be programmed in XSECURE_AES_USER_KEY_7
 * @param	KeySize		Size of the AES key to be used for decryption is
 *		 		XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	AesWrapKey	Address of the wrapped key which needs to be
 *				decrypted
 * @param	OutDataAddr	Address of output buffer where the decrypted data
 *				to be updated
 * @param	Size		Size of data to be decrypted in bytes, whereas number of bytes shall be aligned as below
 *				- 16 byte aligned when it is not the last chunk
 *				- 4 byte aligned when the data is the last chunk
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_ERR_AES_KEY_SIZE_NOT_SUPPORTED  If AES key size is invalid.
 *		 - XSECURE_ERR_AES_KEY_UNWRAP_FAILED_ERROR  If key unwrap is failed.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_AesKeyUnwrap(XSecure_Aes *InstancePtr, u8 *EphAesKey, XSecure_AesKeySize KeySize, u8 *AesWrapKey,
			  u64 OutDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile int SStatusTmp = XST_FAILURE;
	u64 RoundVal = 0U;
	u64 DefInitVal = XSECURE_AES_KEY_WRAP_INIT_DEF_VAL;
	u32 Temp;
	u32 MaxRounds;
	u8 InitValue[XSECURE_AES_64BIT_BLOCK_SIZE];
	u8 DecInput[XSECURE_AES_64BIT_BLOCK_SIZE * 2U];
	u8 DecOut[XSECURE_AES_64BIT_BLOCK_SIZE * 2U];
	u32 AesBlkRoundNum;
	int AesRoundNum;

	MaxRounds = (Size / XSECURE_AES_64BIT_BLOCK_SIZE) - 1U;
	if ((MaxRounds != XSECURE_AES_256BIT_MAX_KEY_BLOCK_ROUNDS) &&
	   (MaxRounds != XSECURE_AES_128BIT_MAX_KEY_BLOCK_ROUNDS)) {
		Status = (int)XSECURE_ERR_AES_KEY_SIZE_NOT_SUPPORTED;
		goto END;
	}

	/* Validate input arguments and write AES key */
	Status = XSecure_AesWriteKey(InstancePtr, XSECURE_AES_USER_KEY_7, KeySize, (u64)(UINTPTR)EphAesKey);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Validate and put AES in ECB and decryption mode */
	Status = XSecure_AesEcbDecryptInit(InstancePtr, XSECURE_AES_USER_KEY_7, KeySize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy initial value*/
	Status = XPlmi_MemCpy64((u64)(UINTPTR)InitValue, (u64)(UINTPTR)AesWrapKey, XSECURE_AES_64BIT_BLOCK_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**< Unwrap the AES customer managed key using AES ephemeral key */
	for (AesRoundNum = (int)XSECURE_AES_KEY_WRAP_MAX_ROUNDS; AesRoundNum >= 0; AesRoundNum--) {
		for (AesBlkRoundNum = MaxRounds; AesBlkRoundNum >= 1U; AesBlkRoundNum--) {
			RoundVal = (u64)(XSECURE_AES_256BIT_MAX_KEY_BLOCK_ROUNDS * (u32)AesRoundNum + AesBlkRoundNum);
			/* XOR initial value with Round value*/
			for (Temp = 0U; Temp < 8U; Temp++) {
				InitValue[Temp] = (u8)(InitValue[Temp] ^ (RoundVal >> (8U * (7U - Temp)) & 0xFFU));
			}
			Status = XPlmi_MemCpy64((u64)(UINTPTR)DecInput, (u64)(UINTPTR)InitValue, XSECURE_AES_64BIT_BLOCK_SIZE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XPlmi_MemCpy64((u64)(UINTPTR)(DecInput + XSECURE_AES_64BIT_BLOCK_SIZE), (u64)(UINTPTR)(AesWrapKey + (AesBlkRoundNum * 8U)), XSECURE_AES_64BIT_BLOCK_SIZE);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			Status = XSecure_AesDecryptUpdate(InstancePtr, (u64)(UINTPTR)DecInput, (u64)(UINTPTR)DecOut, XSECURE_AES_64BIT_BLOCK_SIZE * 2U, TRUE);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			Status = XPlmi_MemCpy64((u64)(UINTPTR)InitValue, (u64)(UINTPTR)DecOut, XSECURE_AES_64BIT_BLOCK_SIZE);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			Status = XPlmi_MemCpy64((u64)(UINTPTR)(AesWrapKey + (AesBlkRoundNum * 8U)), (u64)(UINTPTR)(DecOut + XSECURE_AES_64BIT_BLOCK_SIZE), XSECURE_AES_64BIT_BLOCK_SIZE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

	XSECURE_TEMPORAL_IMPL(Status, SStatus, Xil_SMemCmp, InitValue, XSECURE_AES_64BIT_BLOCK_SIZE, (void*)(UINTPTR)&DefInitVal,
		XSECURE_AES_64BIT_BLOCK_SIZE, XSECURE_AES_64BIT_BLOCK_SIZE);
	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		Status = (int)XSECURE_ERR_AES_KEY_UNWRAP_FAILED_ERROR;
		goto END;
	}

	/* Copy key to destination */
	Status = XPlmi_MemCpy64(OutDataAddr, (u64)(UINTPTR)(AesWrapKey + XSECURE_AES_64BIT_BLOCK_SIZE), XSECURE_AES_64BIT_BLOCK_SIZE *
								MaxRounds);

END:
	if (InstancePtr != NULL) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				 XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);
		XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_ECB_OFFSET, XSECURE_AES_ECB_MODE_DIS);
		/* Clear AES Key */
		SStatus = XSecure_AesKeyZero(InstancePtr, XSECURE_AES_USER_KEY_7);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
		InstancePtr->IsEcbEn = (u32)FALSE;
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
				 XSECURE_AES_SOFT_RST_OFFSET);
	}

	XSECURE_TEMPORAL_IMPL(SStatus, SStatusTmp, Xil_SecureZeroize, DecOut, XSECURE_AES_64BIT_BLOCK_SIZE * 2U);
	if ((SStatus != XST_SUCCESS) || (SStatusTmp != XST_SUCCESS)) {
		if (Status == XST_SUCCESS) {
			Status = (SStatus | SStatusTmp);
		}
	}

	return Status;
}
/** @} */
