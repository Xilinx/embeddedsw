/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_keyunwrap.c
* This file contains Versal Net specific code for Xilsecure key unwrap.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     06/30/23 Initial release
*       dd      10/11/23 MISRA-C violation Rule 10.4 fixed
*       dd      10/11/23 MISRA-C violation Rule 8.13 fixed
* 5.3   kpt     11/24/23 Replace Xil_SMemSet with Xil_SecureZeroize
*       kpt     12/13/23 Added support for RSA CRT
*       kpt     12/13/23 Added SHA384 MGF support for keyunwrap
*       kpt     12/19/23 Fix logical issue in updating keyslot value
* 5.3   ng      01/28/24 Added SDT support
*       ng      03/26/24 Fixed header include in SDT flow
* 5.4   yog     04/29/24 Fixed doxygen warnings.
*       kpt     06/13/24 Added RSA key generation support.
*       kpt     06/13/24 Added AES key unwrap with padding support.
*       kpt     06/13/24 Updated keyvault map.
*       kal     07/24/24 Code refactoring for versal_2ve_2vm
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_keyunwrap_server_apis XilSecure Key Unwrap Server APIs
* @{
*/
/***************************** Include Files *********************************/

#ifdef SDT
#include "xsecure_config.h"
#endif

#include "xparameters.h"
#ifndef PLM_RSA_EXCLUDE

#include "xsecure_plat_rsa.h"
#include "xsecure_rsa.h"
#include "xsecure_plat_aes.h"
#include "xsecure_keyunwrap.h"
#include "xsecure_init.h"
#include "xsecure_error.h"
#include "xplmi_plat.h"
#include "xplmi_dma.h"
#include "xsecure_plat_defs.h"

/************************** Constant Definitions *****************************/

#define XSECURE_MAX_KEY_STORE_CAPACTIY			(32U)            /**< Maximum key store capacity */
#define XSECURE_SHARED_KEY_STORE_SIZE_OFFSET	(8U)           /**< Key size offset */
#define XSECURE_KEY_STORE_KEY_OFFSET			(4U)             /**< Key offset from key slot status */
#define XSECURE_AES_256BIT_KEY_BLOCK_SIZE		(40U)            /**< AES 256-bit key block size */
#define XSECURE_AES_128BIT_KEY_BLOCK_SIZE		(24U)            /**< AES 128-bit key block size */
#define XSECURE_AES_KEY_SLOT_STATUS_FULL		(0x973AFB51U)		  /**< AES key status is full */
#define XSECURE_KEY_SLOT_KEY_STATUS_ADDR	 (XSECURE_KEY_STORE_ADDR + \
											 sizeof(XSecure_KeyStoreHdr))
											 /* Key slot status address */
#define XSECURE_KEY_SLOT_KEY_ADDR			 (XSECURE_KEY_SLOT_KEY_STATUS_ADDR + \
											 XSECURE_KEY_STORE_KEY_OFFSET) /* Key address */

#define XSECURE_KEY_STORE_KEY_WRAP_DATA_SIZE (XSECURE_KEY_STORE_KEY_OFFSET + \
											 sizeof(XSecure_KeyMetaData) + XSECURE_AES_KEY_SIZE_256BIT_BYTES)
											/* key wrap data size */

/************************** Function Prototypes ******************************/

static int XSecure_UpdateKeySlotStatusAddr(void);
static void XSecure_MarkKeySlotOccupied(u64 KeySlotStatusAddr);

/************************** Variable Definitions *****************************/

typedef struct {
	u32 KeyStoreTag; /**< Key store tag */
	u32 VersionNum;  /**< Version number */
	u32 KeyStoreCapacity; /**< Key store capacity */
}XSecure_KeyStoreHdr;

static u32 UpdatedFreeKeySlot = 0U;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function updates the key free slot status address
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XSECURE_ERR_KEY_STORE_SIZE  If key store size is invalid.
 *		 - XSECURE_ERR_NO_FREE_KEY_SLOT  If there is no free key slot.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_UpdateKeySlotStatusAddr(void)
{
	int Status = XST_FAILURE;
	u64 KeySlotStatusAddr = XSECURE_KEY_SLOT_KEY_STATUS_ADDR;
	u32 KeyStoreSize = XSecure_In64(XSECURE_KEY_STORE_ADDR + XSECURE_SHARED_KEY_STORE_SIZE_OFFSET);
	u32 KeySlotCnt = 0U;
	u32 KeySlotStatus = 0U;

	/* Check if the key store size is valid */
	if ((KeyStoreSize > XSECURE_MAX_KEY_STORE_CAPACTIY) || (KeyStoreSize == 0U)) {
		Status = (int)XSECURE_ERR_KEY_STORE_SIZE;
		goto END;
	}

	/* Calculate the starting address of the free key slot status */
	KeySlotStatusAddr = (u64)(XSECURE_KEY_SLOT_KEY_STATUS_ADDR +
					(UpdatedFreeKeySlot * XSECURE_KEY_STORE_KEY_WRAP_DATA_SIZE));

	/* Iterate through the key slots to find a free slot */
	while (KeySlotCnt < KeyStoreSize) {
		KeySlotStatus = XSecure_In64(KeySlotStatusAddr);
		if (KeySlotStatus != XSECURE_AES_KEY_SLOT_STATUS_FULL) {
			break;
		}

		/* Move to the next slot */
		KeySlotStatusAddr = KeySlotStatusAddr + XSECURE_KEY_STORE_KEY_WRAP_DATA_SIZE;
		UpdatedFreeKeySlot++;

		/* Wrap around if the end of the key store is reached */
		if (UpdatedFreeKeySlot >= KeyStoreSize) {
			UpdatedFreeKeySlot = 0U;
			KeySlotStatusAddr = (u64)XSECURE_KEY_SLOT_KEY_STATUS_ADDR;
		}
		KeySlotCnt++;
	}

	if (KeySlotCnt >= KeyStoreSize) {
		Status = (int)XSECURE_ERR_NO_FREE_KEY_SLOT;
		goto END;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the key slot id as in use
 *
 * @param	KeySlotStatusAddr	Key slot status address to be updated
 *
 ******************************************************************************/
static void XSecure_MarkKeySlotOccupied(u64 KeySlotStatusAddr)
{
	u32 KeyStoreSize = XSecure_In64(XSECURE_KEY_STORE_ADDR + XSECURE_SHARED_KEY_STORE_SIZE_OFFSET);

	XSecure_Out64(KeySlotStatusAddr, XSECURE_AES_KEY_SLOT_STATUS_FULL);
	UpdatedFreeKeySlot = UpdatedFreeKeySlot + 1U;
	if (UpdatedFreeKeySlot >= KeyStoreSize) {
		UpdatedFreeKeySlot = 0U;
	}
}

/*****************************************************************************/
/**
 * @brief	This function unwraps the given wrapped key and stores it along
 *              with metadata in Shared address between PMC and secure shell.
 *
 * @param	KeyWrapData is pointer to the XSecure_KeyWrapData instance.
 * @param	DmaPtr is pointer to DMA instance which is used for AES and SHA
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XST_INVALID_PARAM  If any input parameter is invalid.
 *		 - XSECURE_ERR_KEY_WRAP_SIZE_MISMATCH  If wrapped key size is invalid.
 *		 - XSECURE_ERR_AES_KEY_SIZE_NOT_SUPPORTED  If AES key size is invalid.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_KeyUnwrap(XSecure_KeyWrapData *KeyWrapData)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatusTmp = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u64 KeyWrapAddr = 0U;
	u64 DstKeySlotAddr = 0U;
	u64 KeySlotStatusAddr = 0U;
	u32 EncryptedKeySize = 0U;
	XSecure_AesKeySize AesKeySize;
	XSecure_RsaOaepParam OaepParam = {0U};
	XSecure_Aes *AesInstPtr = XSecure_GetAesInstance();
	u32 KeyInUseIdx = XSecure_GetRsaKeyInUseIdx();
	XSecure_RsaPrivKey *PrivKey = XSecure_GetRsaPrivateKey(KeyInUseIdx);
	u8 WrapRsaKey[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES];
	u8 EphAesKey[XSECURE_AES_KEY_SIZE_256BIT_BYTES];
	u8 WrapAesKey[XSECURE_AES_256BIT_KEY_BLOCK_SIZE];

	if ((KeyWrapData == NULL) || (PrivKey == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	if (KeyWrapData->TotalWrappedKeySize <= XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES) {
		Status = (int)XSECURE_ERR_KEY_WRAP_SIZE_MISMATCH;
		goto END;
	}

	EncryptedKeySize = KeyWrapData->TotalWrappedKeySize - XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES;
	if ((EncryptedKeySize != XSECURE_AES_256BIT_KEY_BLOCK_SIZE) &&
		(EncryptedKeySize != XSECURE_AES_128BIT_KEY_BLOCK_SIZE)) {
		Status = (int)XSECURE_ERR_AES_KEY_SIZE_NOT_SUPPORTED;
		goto END;
	}

	KeyWrapAddr = KeyWrapData->KeyWrapAddr;

	/** Check for free key slot and update Key slot status address */
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_UpdateKeySlotStatusAddr);

	/* Get destination key slot address by using free key slot value */
	DstKeySlotAddr = (u64)(XSECURE_KEY_SLOT_KEY_ADDR + (UpdatedFreeKeySlot * XSECURE_KEY_STORE_KEY_WRAP_DATA_SIZE));
	KeySlotStatusAddr = DstKeySlotAddr - XSECURE_KEY_STORE_KEY_OFFSET;

	/* Copy wrapped rsa key to local buffer */
	Status = XSecure_MemCpyAndChangeEndianness((u64)(UINTPTR)WrapRsaKey, KeyWrapAddr, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	OaepParam.InputDataAddr = (u64)(UINTPTR)WrapRsaKey;
	OaepParam.OutputDataAddr = (u64)(UINTPTR)EphAesKey;
	OaepParam.ShaInstancePtr = NULL;
	OaepParam.ShaType = XSECURE_SHA2_384;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_RsaOaepDecrypt, PrivKey, &OaepParam);
	if (OaepParam.OutputDataSize == XSECURE_AES_KEY_SIZE_256BIT_BYTES) {
		AesKeySize = XSECURE_AES_KEY_SIZE_256;
	}
	else if (OaepParam.OutputDataSize == XSECURE_AES_KEY_SIZE_128BIT_BYTES) {
		AesKeySize = XSECURE_AES_KEY_SIZE_128;
	}
	else {
		Status = (int)XSECURE_ERR_AES_KEY_SIZE_NOT_SUPPORTED;
		goto END;
	}

	KeyWrapAddr = KeyWrapAddr + XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES;
	Status = XPlmi_MemCpy64((u64)(UINTPTR)WrapAesKey, KeyWrapAddr, XSECURE_AES_256BIT_KEY_BLOCK_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_AesKeyUnwrap, AesInstPtr, EphAesKey,
			       AesKeySize, WrapAesKey, DstKeySlotAddr, EncryptedKeySize);

	/** Update the key slot with metadata */
	DstKeySlotAddr += (u64)(EncryptedKeySize - XSECURE_AES_64BIT_BLOCK_SIZE);
	XSecure_MemCpy64(DstKeySlotAddr, (u64)(UINTPTR)&KeyWrapData->KeyMetaData, sizeof(XSecure_KeyMetaData));
	XSecure_MarkKeySlotOccupied(KeySlotStatusAddr);

END:
	/** Clear the ephemeral AES key after the usage */
	XSECURE_TEMPORAL_IMPL(SStatus, SStatusTmp, Xil_SecureZeroize, EphAesKey, XSECURE_AES_KEY_SIZE_256BIT_BYTES);
	if ((SStatus != XST_SUCCESS) || (SStatusTmp != XST_SUCCESS)) {
		if (Status == XST_SUCCESS) {
			Status = (SStatus | SStatusTmp);
		}
	}

		/* Clear the ephemeral AES key after the usage */
	XSECURE_TEMPORAL_IMPL(SStatus, SStatusTmp, Xil_SecureZeroize, WrapAesKey, XSECURE_AES_256BIT_KEY_BLOCK_SIZE);
	if ((SStatus != XST_SUCCESS) || (SStatusTmp != XST_SUCCESS)) {
		if (Status == XST_SUCCESS) {
			Status = (SStatus | SStatusTmp);
		}
	}

	return Status;
}

#endif
/** @} */
