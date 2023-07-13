/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_keyunwrap.c
* This file contains versalnet specific code for xilsecure key unwrap.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     06/30/23 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#ifndef PLM_RSA_EXCLUDE

#include "xsecure_plat_rsa.h"
#include "xsecure_rsa.h"
#include "xsecure_plat_aes.h"
#include "xsecure_keyunwrap.h"
#include "xsecure_init.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

#define XSECURE_KEY_STORE_ADDR                 (0x10000000U)  /**< Key store address */
#define XSECURE_SHARED_KEY_STORE_SIZE_OFFSET   (8U)           /**< Key size offset */
#define XSECURE_SHARED_KEY_STORE_BITMAP_OFFSET (12U)          /**< Bitmap offset */
#define XSECURE_KEY_STORE_BITMAP_MASK        (0xFFFFFFFFU)    /**< Bitmap mask */
#define XSECURE_MAX_KEY_STORE_CAPACTIY       (32U)            /**< Maximum key store capacity */

/************************** Function Prototypes ******************************/

static int XSecure_GetFreeKeySlot(u32 *KeySlotPtr, u64 SharedKeyStoreAddr);
static void XSecure_MarkKeySlotOccupied(u32 KeySlotId, u64 SharedKeyStoreAddr);
static u32 XSecure_GetKeyStoreAddr(void);

/************************** Variable Definitions *****************************/

typedef struct {
	u32 KeyStoreTag; /**< Key store tag */
	u32 VersionNum;  /**< Version number */
	u32 KeyStoreCapacity; /**< Key store capacity */
	u32 BitMap; /**< Bitmap to indicate free key slot */
}XSecure_KeyStoreHdr;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function returns key store address
 *
 ******************************************************************************/
static u32 XSecure_GetKeyStoreAddr(void)
{
	return XSECURE_KEY_STORE_ADDR;
}

/*****************************************************************************/
/**
 * @brief	This function returns the free key slot id from shared key store address
 *
 * @param	KeySlotPtr is pointer to the keyslot where key and metadata needs to be stored.
 * @param	SharedKeyStoreAddr is address of shared key store between secure shell and PMC.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
 *
 ******************************************************************************/
static int XSecure_GetFreeKeySlot(u32 *KeySlotPtr, u64 SharedKeyStoreAddr)
{
	int Status = XST_FAILURE;
	static u32 UpdateKeyFreeSlot = 0U;
	u32 BitMap = XSecure_In64(SharedKeyStoreAddr + XSECURE_SHARED_KEY_STORE_BITMAP_OFFSET);
	u32 KeyStoreSize = XSecure_In64(SharedKeyStoreAddr + XSECURE_SHARED_KEY_STORE_SIZE_OFFSET);
	u32 BitMapMask = 0U;


	if ((KeyStoreSize > XSECURE_MAX_KEY_STORE_CAPACTIY) || (KeyStoreSize == 0U)) {
		Status = (int)XSECURE_ERR_KEY_STORE_SIZE;
		goto END;
	}

	BitMapMask = ~(u32)((u64)XSECURE_KEY_STORE_BITMAP_MASK << KeyStoreSize);
	if ((BitMap & BitMapMask) == BitMapMask) {
		Status = (int)XSECURE_ERR_NO_FREE_KEY_SLOT;
		goto END;
	}

	while (((BitMap >> UpdateKeyFreeSlot) & 0x01U) != 0x00U) {
		UpdateKeyFreeSlot++;
		if (UpdateKeyFreeSlot > KeyStoreSize) {
			UpdateKeyFreeSlot = 0U;
		}
	}
	*KeySlotPtr = UpdateKeyFreeSlot;
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the KEY slot id as in use
 *
 * @param	KeySlotId is key slot id that needs to be updated in bitmap.
 * @param	SharedKeyStoreAddr is address of shared key store between secure shell and PMC.
 *
 ******************************************************************************/
static void XSecure_MarkKeySlotOccupied(u32 KeySlotId,  u64 SharedKeyStoreAddr)
{
	u32 BitMap = XSecure_In64(SharedKeyStoreAddr + XSECURE_SHARED_KEY_STORE_BITMAP_OFFSET);

	BitMap |= (1 << KeySlotId);
	XSecure_Out64(SharedKeyStoreAddr + XSECURE_SHARED_KEY_STORE_BITMAP_OFFSET, BitMap);
}

/*****************************************************************************/
/**
 * @brief	This function unwraps the given wrapped key and stores it along
 *              with metadata in Shared address between PMC and secure shell.
 *
 * @param	KeyWrapData is pointer to the XSecure_KeyWrapData instance.
 * @param   DmaPtr is ponter to DMA instance which is used for AES and SHA
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
 *
 ******************************************************************************/
int XSecure_KeyUnwrap(XSecure_KeyWrapData *KeyWrapData, XPmcDma *DmaPtr)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatusTmp = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u64 KeyWrapAddr = 0U;
	u64 DstKeySlotAddr = 0U;
	u32 EncryptedKeySize = 0U;
	XSecure_AesKeySize AesKeySize;
	XSecure_RsaOaepParam OaepParam = {0U};
	XSecure_Rsa *RsaInstancePtr = XSecure_GetRsaInstance();
	XSecure_Aes *AesInstPtr = XSecure_GetAesInstance();
	XSecure_Sha3 *ShaInstancePtr = XSecure_GetSha3Instance();
	XSecure_RsaKey *PrivKey = XSecure_GetRsaPrivateKey();
	u64 SharedKeyStoreAddr = XSecure_GetKeyStoreAddr();
	u8 EphAesKey[XSECURE_AES_KEY_SIZE_256BIT_BYTES];
	u32 KeySlotVal = 0U;

	if ((KeyWrapData == NULL) || (PrivKey == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	if (KeyWrapData->TotalWrappedKeySize <= XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES) {
		Status = (int)XSECURE_ERR_KEY_WRAP_SIZE_MISMATCH;
		goto END;
	}

	EncryptedKeySize = KeyWrapData->TotalWrappedKeySize - XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES;
	if ((EncryptedKeySize != XSECURE_AES_KEY_SIZE_256BIT_BYTES) &&
		(EncryptedKeySize != XSECURE_AES_KEY_SIZE_128BIT_BYTES)) {
		Status = (int)XSECURE_ERR_AES_KEY_SIZE_NOT_SUPPORTED;
		goto END;
	}

	KeyWrapAddr = KeyWrapData->KeyWrapAddr;

	/** Check for free key slot */
	Status = XSecure_GetFreeKeySlot(&KeySlotVal, SharedKeyStoreAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Get destination key slot address by using free key slot value */
	DstKeySlotAddr = (SharedKeyStoreAddr + sizeof(XSecure_KeyStoreHdr) + (KeySlotVal * sizeof(XSecure_KeyMetaData)) +
				(KeySlotVal * XSECURE_AES_KEY_SIZE_256BIT_BYTES));

	/* Decode the wrapped AES ephemeral key using RSA OAEP decrypt */
	Status = XSecure_RsaInitialize(RsaInstancePtr, PrivKey->Modulus, PrivKey->ModExt, PrivKey->Exponent);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Initialize(ShaInstancePtr, DmaPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	OaepParam.InputDataAddr = KeyWrapAddr;
	OaepParam.OutputDataAddr = (u64)(UINTPTR)EphAesKey;
	OaepParam.ShaInstancePtr = (void *)ShaInstancePtr;
	OaepParam.ShaType = XSECURE_SHA3_384;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_RsaOaepDecrypt, RsaInstancePtr, &OaepParam);
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

	/**< Unwrap the AES customer managed key using AES ephemeral key */
	Status = XSecure_AesInitialize(AesInstPtr, DmaPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	KeyWrapAddr = KeyWrapAddr + XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES;
	if (OaepParam.OutputDataSize != EncryptedKeySize) {
		Status = (int)XSECURE_ERR_KEY_WRAP_SIZE_MISMATCH;
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_AesEcbDecrypt, AesInstPtr, (u64)(UINTPTR)EphAesKey,
			       AesKeySize, KeyWrapAddr, DstKeySlotAddr, EncryptedKeySize);

	/* Update the key slot with metadata */
	DstKeySlotAddr += EncryptedKeySize;
	XSecure_MemCpy64(DstKeySlotAddr, (u64)(UINTPTR)&KeyWrapData->KeyMetaData, sizeof(XSecure_KeyMetaData));
	XSecure_MarkKeySlotOccupied(KeySlotVal, SharedKeyStoreAddr);

END:
	/* Clear the ephemeral AES key after the usage */
	XSECURE_TEMPORAL_IMPL(SStatus, SStatusTmp, Xil_SMemSet, EphAesKey, XSECURE_AES_KEY_SIZE_256BIT_BYTES,
			      0U, XSECURE_AES_KEY_SIZE_256BIT_BYTES);
	if ((SStatus != XST_SUCCESS) || (SStatusTmp != XST_SUCCESS)) {
		if (Status == XST_SUCCESS) {
			Status = (SStatus | SStatusTmp);
		}
	}

return Status;
}

#endif
