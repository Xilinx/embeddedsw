/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes_ipihandler.c
* @addtogroup xsecure_apis XilSecure Versal AES handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure AES IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kal   03/04/2021 Initial release
*       bm    05/13/2021 Updated code to use common crypto instance
*       har   05/18/2021 Added check for key source for IPI calls
*       am    05/21/2021 Resolved Coverity violations
* 4.6   har   07/14/2021 Fixed doxygen warnings
*       kpt   07/15/2021 Added XSecure_AesInit in XSecure_AesWriteKey to avoid
*                        multiple calls from client
*       kal   08/16/2021 Fixed magic number usage comment
*       kal   08/18/2021 Fixed SW-BP-LOCK-RESOURCE review comments
*       har   09/14/2021 Added check for DecKeySrc in XSecure_AesKekDecrypt
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_aes.h"
#include "xsecure_aes_ipihandler.h"
#include "xsecure_defs.h"
#include "xil_util.h"
#include "xsecure_init.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/
#define XSECURE_AES_DEC_KEY_SRC_MASK	0x000000FFU
			/**< AES decrypt key source mask for KEK decryption */
#define XSECURE_AES_DST_KEY_SRC_MASK	0x0000FF00U
			/**< AES destination key source mask for KEK decryption */
#define XSECURE_AES_KEY_SIZE_MASK	0xFFFF0000U
			/**< Key Size mask */
#define XSECURE_PMCDMA_DEVICEID		PMCDMA_0_DEVICE_ID
			/**< AES destination key source mask for KEK decryption */

/************************** Function Prototypes *****************************/
static int XSecure_AesInit(void);
static int XSecure_AesOperationInit(u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_AesAadUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size);
static int XSecure_AesEncUpdate(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_AesEncFinal(u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_AesDecUpdate(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_AesDecFinal(u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_AesKeyZeroize(u32 KeySrc);
static int XSecure_AesKeyWrite(u8  KeySize, u8 KeySrc,
	u32 KeyAddrLow, u32 KeyAddrHigh);
static int XSecure_AesDecryptKek(u32 KeyInfo, u32 IvAddrLow, u32 IvAddrHigh);
static int XSecure_AesSetDpaCmConfig(u8 DpaCmCfg);
static int XSecure_AesExecuteDecKat(void);
static int XSecure_AesExecuteDecCmKat(void);

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_AES_INIT):
		Status = XSecure_AesInit();
		break;
	case XSECURE_API(XSECURE_API_AES_OP_INIT):
		Status = XSecure_AesOperationInit(Pload[0], Pload[1]);
		break;
	case XSECURE_API(XSECURE_API_AES_UPDATE_AAD):
		Status = XSecure_AesAadUpdate(Pload[0], Pload[1], Pload[2]);
		break;
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_UPDATE):
		Status = XSecure_AesEncUpdate(Pload[0], Pload[1], Pload[2],
				Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_FINAL):
		Status = XSecure_AesEncFinal(Pload[0], Pload[1]);
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_UPDATE):
		Status = XSecure_AesDecUpdate(Pload[0], Pload[1], Pload[2],
				Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_FINAL):
		Status = XSecure_AesDecFinal(Pload[0], Pload[1]);
		break;
	case XSECURE_API(XSECURE_API_AES_KEY_ZERO):
		Status = XSecure_AesKeyZeroize(Pload[0]);
		break;
	case XSECURE_API(XSECURE_API_AES_WRITE_KEY):
		Status = XSecure_AesKeyWrite((u8)Pload[0], (u8)Pload[1], Pload[2],
				Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_AES_KEK_DECRYPT):
		Status = XSecure_AesDecryptKek(Pload[0], Pload[1], Pload[2]);
		break;
	case XSECURE_API(XSECURE_API_AES_SET_DPA_CM):
		Status = XSecure_AesSetDpaCmConfig((u8)Pload[0]);
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_KAT):
		Status = XSecure_AesExecuteDecKat();
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_CM_KAT):
		Status = XSecure_AesExecuteDecCmKat();
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesInitialize Server API
 *
 * @return	- XST_SUCCESS - If the initialization is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesInit(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	Status = XSecure_AesInitialize(XSecureAesInstPtr, PmcDmaInstPtr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesEncryptInit or
 * 		XSecure_AesDecryptInit server API based on the Operation type
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the XSecure_AesInitOps
 * 				structure.
 * 		SrcAddrHigh	- Higher 32 bit address of the XSecure_AesInitOps
 * 				structure.
 *
 * @return	- XST_SUCCESS - If the initialization is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesOperationInit(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_AesInitOps AesParams;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)&AesParams, sizeof(AesParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((AesParams.KeySrc == XSECURE_AES_BBRAM_KEY) ||
		(AesParams.KeySrc == XSECURE_AES_BBRAM_RED_KEY) ||
		(AesParams.KeySrc == XSECURE_AES_EFUSE_KEY) ||
		(AesParams.KeySrc == XSECURE_AES_EFUSE_RED_KEY) ||
		(AesParams.KeySrc == XSECURE_AES_BH_KEY) ||
		(AesParams.KeySrc == XSECURE_AES_BH_RED_KEY)) {
		Status = XSECURE_AES_DEVICE_KEY_NOT_ALLOWED;
		goto END;
	}

	if (AesParams.OperationId == (u32)XSECURE_ENCRYPT) {
		Status = XSecure_AesEncryptInit(XSecureAesInstPtr,
				(XSecure_AesKeySrc)AesParams.KeySrc,
				(XSecure_AesKeySize)AesParams.KeySize,
				AesParams.IvAddr);
	}
	else {
		Status = XSecure_AesDecryptInit(XSecureAesInstPtr,
				(XSecure_AesKeySrc)AesParams.KeySrc,
				(XSecure_AesKeySize)AesParams.KeySize,
				AesParams.IvAddr);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesUpdateAad server API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the AAD data
 * 		SrcAddrHigh	- Higher 32 bit address of the AAD data
 *		Size		- AAD Size
 *
 * @return	- XST_SUCCESS - If the encrypt update is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesAadUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XSecure_AesUpdateAad(XSecureAesInstPtr, Addr, Size);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesEncryptUpdate server API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the
 * 				XSecure_AesInParams structure.
 * 		SrcAddrHigh	- Higher 32 bit address of the
 * 				XSecure_AesInParams structure.
 * 		DstAddrLow	- Lower 32 bit address of the Output buffer
 * 				where encrypted data to be stored
 * 		DstAddrHigh	- Higher 32 bit address of the output buffer
 * 				where encrypted data to be stored
 *
 * @return	- XST_SUCCESS - If the encrypt update is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesEncUpdate(u32 SrcAddrLow, u32 SrcAddrHigh,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_AesInParams InParams;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)&InParams, sizeof(InParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesEncryptUpdate(XSecureAesInstPtr, InParams.InDataAddr,
				DstAddr, InParams.Size, (u8)InParams.IsLast);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesEncryptFinal server API
 *
 * @param	DstAddrLow	- Lower 32 bit address of the GCM-TAG
 * 				to be stored.
 * 		DstAddrHigh	- Higher 32 bit address of the GCM-TAG
 * 				to be stored.
 *
 * @return	- XST_SUCCESS - If the encrypt final is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesEncFinal(u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XSecure_AesEncryptFinal(XSecureAesInstPtr, Addr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptUpdate server API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the
 * 				XSecure_AesInParams structure.
 * 		SrcAddrHigh	- Higher 32 bit address of the
 * 				XSecure_AesInParams structure.
 * 		DstAddrLow	- Lower 32 bit address of the Output buffer
 * 				where decrypted data to be stored
 * 		DstAddrHigh	- Higher 32 bit address of the output buffer
 * 				where decrypted data to be stored
 *
 * @return	- XST_SUCCESS - If the decrypt update is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesDecUpdate(u32 SrcAddrLow, u32 SrcAddrHigh,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_AesInParams InParams;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)&InParams, sizeof(InParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesDecryptUpdate(XSecureAesInstPtr, InParams.InDataAddr,
				DstAddr, InParams.Size, (u8)InParams.IsLast);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptFinal server API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the GCM-TAG
 * 		SrcAddrHigh	- Higher 32 bit address of the GCM-TAG
 *
 * @return	- XST_SUCCESS - If the decrypt final is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesDecFinal(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XSecure_AesDecryptFinal(XSecureAesInstPtr, Addr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesKeyZero server API
 *
 * @param	KeySrc	- Key source to be zeroized
 *
 * @return	- XST_SUCCESS - If the key zeroize is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesKeyZeroize(u32 KeySrc)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	if ((KeySrc == XSECURE_AES_KUP_KEY) ||
		(KeySrc == XSECURE_AES_EXPANDED_KEYS) ||
		((KeySrc >= XSECURE_AES_USER_KEY_0) &&
		(KeySrc <= XSECURE_AES_USER_KEY_7))) {
		Status = XSecure_AesKeyZero(XSecureAesInstPtr,
				(XSecure_AesKeySrc)KeySrc);
	}
	else {
		Status = (int)XSECURE_AES_INVALID_PARAM;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesWriteKey server API
 *
 * @param	KeySize		- Size of the key to specify 128/256 bit key
 *		KeySrc		- KeySrc to which key has to be written
 * 		KeyAddrLow	- Lower 32 bit address of the Key
 * 		KeyAddrHigh	- Higher 32 bit address of the Key
 *
 * @return	- XST_SUCCESS - If the key write is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesKeyWrite(u8  KeySize, u8 KeySrc,
			u32 KeyAddrLow, u32 KeyAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 KeyAddr = ((u64)KeyAddrHigh << 32U) | (u64)KeyAddrLow;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	if ((KeySrc == XSECURE_AES_BH_KEY) ||
		(KeySrc == XSECURE_AES_BH_RED_KEY)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_AesInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesWriteKey(XSecureAesInstPtr,
				(XSecure_AesKeySrc)KeySrc,
				(XSecure_AesKeySize)KeySize, KeyAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptKek server API
 *
 * @param	KeyInfo		- KeyInfo contains KeySize, KeyDst and KeySrc
 * 		IvAddrLow	- Lower 32 bit address of the IV
 * 		IvAddrHigh	- Higher 32 bit address of the IV
 *
 * @return	- XST_SUCCESS - If the decryption is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesDecryptKek(u32 KeyInfo, u32 IvAddrLow, u32 IvAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 IvAddr;
	XSecure_AesKeySrc DstKeySrc;
	XSecure_AesKeySize KeySize;
	XSecure_Aes *XSecureAesInstPtr;
	XSecure_AesKeySrc DecKeySrc = (XSecure_AesKeySrc)(KeyInfo &
		XSECURE_AES_DEC_KEY_SRC_MASK);

	if ((DecKeySrc != XSECURE_AES_EFUSE_USER_KEY_0) &&
		(DecKeySrc != XSECURE_AES_EFUSE_USER_KEY_1)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	IvAddr = ((u64)IvAddrHigh << 32U) | (u64)IvAddrLow;
	DstKeySrc = (XSecure_AesKeySrc)(KeyInfo &
		XSECURE_AES_DST_KEY_SRC_MASK);
	KeySize = (XSecure_AesKeySize)(KeyInfo &
		XSECURE_AES_KEY_SIZE_MASK);
	XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XSecure_AesKekDecrypt(XSecureAesInstPtr, DecKeySrc, DstKeySrc,
				IvAddr, KeySize);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesSetDpaCm server API
 *
 * @param	DpaCmCfg	- User DpaCmCfg configuration
 *
 * @return	- XST_SUCCESS - If the Set DpaCm is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesSetDpaCmConfig(u8 DpaCmCfg)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XSecure_AesSetDpaCm(XSecureAesInstPtr, DpaCmCfg);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptKat server API
 *
 * @return	- XST_SUCCESS - If the KAT is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesExecuteDecKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if ((XSecureAesInstPtr->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(XSecureAesInstPtr->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	Status = XSecure_AesInitialize(XSecureAesInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecryptKat(XSecureAesInstPtr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesExecuteDecCmKat
 * 		server API
 *
 * @return	- XST_SUCCESS - If the KAT is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesExecuteDecCmKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if ((XSecureAesInstPtr->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(XSecureAesInstPtr->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	Status = XSecure_AesInitialize(XSecureAesInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecryptCmKat(XSecureAesInstPtr);

END:
	return Status;
}
