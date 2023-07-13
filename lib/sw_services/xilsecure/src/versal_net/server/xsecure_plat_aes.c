/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_aes.c
* This file contains versalnet specific code for xilsecure aes server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     06/20/23 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_plat_aes.h"
#include "xsecure_error.h"
#include "xsecure_aes_core_hw.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static int XSecure_AesEcbDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function sets and initializes AES in ECB mode
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	KeySrc		- Key Source for decryption of the data
 * @param	KeySize		- Size of the AES key to be used for decryption is
 *		 		- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 *
 * @return
 *  -   XST_SUCCESS - On Success
 *	-	ErrorCode   - On Failure
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
 * @param	InstancePtr Pointer to the XSecure_Aes instance
 * @param   EcbModeFlag Flag to enable/disable ECB mode
 *
 * @return
 *  -   XST_SUCCESS - On Success
 *	-	ErrorCode   - On Failure
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
 *          in ECB mode
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	KeyAddr		- Address of the key to be programmed in XSECURE_AES_USER_KEY_7
 * @param	KeySize		- Size of the AES key to be used for decryption is
 *		 		- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *				  decrypted
 * @param	OutDataAddr	Address of output buffer where the decrypted data
 *				  to be updated
 * @param	Size    Size of data to be decrypted in bytes, whereas number of bytes shall be aligned as below
 *                  - 16 byte aligned when it is not the last chunk
 *                  - 4 byte aligned when the data is the last chunk
 *
 * @return
 *  -   XST_SUCCESS - On Success
 *	-	ErrorCode   - On Failure
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
