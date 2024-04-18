/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 4.7   am    03/08/2022 Fixed MISRA C violations
*       kpt   03/18/2022 Replaced XPlmi_Dmaxfr with XPlmi_MemCpy64
* 5.0   kpt   07/24/2022 Moved XSecure_AesExecuteDecKat, XSecure_AesExecuteDecCMKat
*                        into xsecure_kat_plat_ipihandler.c
*       kpt   08/19/2022 Added GMAC support
* 5.1   skg   12/16/2022 Added XSecure_AesEncrypt/DecryptInitUpdateFinal
*	yog   05/03/2023 Fixed MISRA C violation of Rule 10.3
*       vss	  07/14/2023 Added support for IpiChannel check
*       vss   09/11/2023 Fixed MISRA-C Rule 10.3 and 10.4 violation
* 5.3	vss  10/03/23 Added single API support for AES AAD and GMAC operations
*       mb    03/12/24   Added AES INIT call inside AES Operation INIT API
*	ss    04/05/24   Fixed doxygen warnings
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
#include "xplmi.h"
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
static int XSecure_AesOperationInit(u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_AesAadUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size, u32 IsGmacEn);
static int XSecure_AesEncUpdate(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_AesEncFinal(u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_AesDecUpdate(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_AesDecFinal(u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_AesDecryptKek(u32 KeyInfo, u32 IvAddrLow, u32 IvAddrHigh);
static int XSecure_AesSetDpaCmConfig(u8 DpaCmCfg);
static int XSecure_IsKeySrcValid(u32 KeySrc);
static int XSecure_AesIsDataContextLost(void);
static void XSecure_MakeAesFree(void);
static int XSecure_AesConfig(u32 OperationId, u32 KeySrc, u32 KeySize, u64 IvAddr);
/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *	-	XST_SUCCESS - If the handler execution is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	/*
	 * Storing the Ipimask value when a request for the resource comes and
	 * if the resource is free
	 */
	if (XSecureAesInstPtr->IsResourceBusy == (u32)XSECURE_RESOURCE_FREE) {
		XSecureAesInstPtr->IpiMask = Cmd->IpiMask;
	}
	else {
		/* Check if request comes from same IPI channel or not */
		if (XSecureAesInstPtr->IpiMask != Cmd->IpiMask) {
			Status = XST_DEVICE_BUSY;
			goto END;
		}
	}

	/** Call the respective API handler according to API ID */
	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_AES_INIT):
		/**   - @ref XSecure_AesInit */
		Status = XSecure_AesInit();
		break;
	case XSECURE_API(XSECURE_API_AES_OP_INIT):
		/**   - @ref XSecure_AesOperationInit */
		Status = XSecure_AesOperationInit(Pload[0], Pload[1]);
		break;
	case XSECURE_API(XSECURE_API_AES_UPDATE_AAD):
		/**   - @ref XSecure_AesAadUpdate */
		Status = XSecure_AesAadUpdate(Pload[0], Pload[1], Pload[2], Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_UPDATE):
		/**   - @ref XSecure_AesEncUpdate */
		Status = XSecure_AesEncUpdate(Pload[0], Pload[1], Pload[2],
				Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_FINAL):
		/**   - @ref XSecure_AesEncFinal */
		Status = XSecure_AesEncFinal(Pload[0], Pload[1]);
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_UPDATE):
		/**   - @ref XSecure_AesDecUpdate */
		Status = XSecure_AesDecUpdate(Pload[0], Pload[1], Pload[2],
				Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_FINAL):
		/**   - @ref XSecure_AesDecFinal */
		Status = XSecure_AesDecFinal(Pload[0], Pload[1]);
		break;
	case XSECURE_API(XSECURE_API_AES_KEY_ZERO):
		/**   - @ref XSecure_AesKeyZeroize */
		Status = XSecure_AesKeyZeroize(Pload[0]);
		break;
	case XSECURE_API(XSECURE_API_AES_WRITE_KEY):
		/**   - @ref XSecure_AesKeyWrite */
		Status = XSecure_AesKeyWrite((u8)Pload[0], (u8)Pload[1], Pload[2],
				Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_AES_KEK_DECRYPT):
		/**   - @ref XSecure_AesDecryptKek */
		Status = XSecure_AesDecryptKek(Pload[0], Pload[1], Pload[2]);
		break;
	case XSECURE_API(XSECURE_API_AES_SET_DPA_CM):
		/**   - @ref XSecure_AesSetDpaCmConfig */
		Status = XSecure_AesSetDpaCmConfig((u8)Pload[0]);
		break;
	case XSECURE_API(XSECURE_API_AES_PERFORM_OPERATION):
		/**   - @ref XSecure_AesPerformOperation */
		Status = XSecure_AesPerformOperation(Pload[0], Pload[1]);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesInitialize Server API
 *
 * @return
 *	-	XST_SUCCESS - If the initialization is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesInit(void)
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
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesEncryptInit or
 * 		XSecure_AesDecryptInit server API based on the Operation type
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the XSecure_AesInitOps
 * 				structure.
 * @param	SrcAddrHigh	- Higher 32 bit address of the XSecure_AesInitOps
 * 				structure.
 *
 * @return
 *	-	XST_SUCCESS - If the initialization is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesOperationInit(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_AesInitOps AesParams;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XSecureAesInstPtr->IsResourceBusy = (u32)XSECURE_RESOURCE_BUSY;

	if (XSecureAesInstPtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = XSecure_AesInit();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/* Clear previous aes data context flag */
	if(XSecureAesInstPtr->PreviousAesIpiMask == XSecureAesInstPtr->IpiMask) {
		 XSecureAesInstPtr->DataContextLost = (u32)XSECURE_DATA_CONTEXT_AVAILABLE;
	}

	Status =  XPlmi_MemCpy64((u64)(UINTPTR)&AesParams, Addr, sizeof(AesParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesConfig(AesParams.OperationId, AesParams.KeySrc, AesParams.KeySize, AesParams.IvAddr);

END:
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesGmacCfg and XSecure_AesUpdateAad
 *				server API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the AAD data
 * @param	SrcAddrHigh	- Higher 32 bit address of the AAD data
 * @param	Size		- AAD Size
 * @param     	IsGmacEn    	- User choice to enable/disable GMAC
 *
 * @return
 *	-	XST_SUCCESS - If the AAD update is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesAadUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size, u32 IsGmacEn)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	/**
	 * Update GMAC configuration as per the user input and
	 * calculate AAD if GMAC is enabled.
	 */
	Status = XSecure_AesGmacCfg(XSecureAesInstPtr, IsGmacEn);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesUpdateAad(XSecureAesInstPtr, Addr, Size);

END:
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesEncryptUpdate server API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the
 * 				XSecure_AesInParams structure.
 * @param	SrcAddrHigh	- Higher 32 bit address of the
 * 				XSecure_AesInParams structure.
 * @param	DstAddrLow	- Lower 32 bit address of the Output buffer
 * 				where encrypted data to be stored
 * @param	DstAddrHigh	- Higher 32 bit address of the output buffer
 * 				where encrypted data to be stored
 *
 * @return
 *	-	XST_SUCCESS - If the encrypt update is successful
 *	-	ErrorCode - If there is a failure
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

	Status =  XPlmi_MemCpy64((u64)(UINTPTR)&InParams, Addr, sizeof(InParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	/** Ensure previous data context is not lost for the corresponding IPI channel */
	Status = XSecure_AesIsDataContextLost();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesEncryptUpdate(XSecureAesInstPtr, InParams.InDataAddr,
				DstAddr, InParams.Size, (u8)InParams.IsLast);

END:
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesEncryptFinal server API
 *
 * @param	DstAddrLow	- Lower 32 bit address of the GCM-TAG
 * 				to be stored.
 * @param	DstAddrHigh	- Higher 32 bit address of the GCM-TAG
 * 				to be stored.
 *
 * @return
 *	-	XST_SUCCESS - If the encrypt final is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesEncFinal(u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	/** Ensure previous data context is not lost for the corresponding IPI channel */
	Status = XSecure_AesIsDataContextLost();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Generates GCM tag */
	Status = XSecure_AesEncryptFinal(XSecureAesInstPtr, Addr);

	XSecure_MakeAesFree();
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptUpdate server API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the
 * 				XSecure_AesInParams structure.
 * @param	SrcAddrHigh	- Higher 32 bit address of the
 * 				XSecure_AesInParams structure.
 * @param	DstAddrLow	- Lower 32 bit address of the Output buffer
 * 				where decrypted data to be stored
 * @param	DstAddrHigh	- Higher 32 bit address of the output buffer
 * 				where decrypted data to be stored
 *
 * @return
 *	-	XST_SUCCESS - If the decrypt update is successful
 *	-	ErrorCode - If there is a failure
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

	Status =  XPlmi_MemCpy64((u64)(UINTPTR)&InParams, Addr, sizeof(InParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesIsDataContextLost();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Update data in AES engine which needs to be decrypted */
	Status = XSecure_AesDecryptUpdate(XSecureAesInstPtr, InParams.InDataAddr,
				DstAddr, InParams.Size, (u8)InParams.IsLast);

END:
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptFinal server API
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the GCM-TAG
 * @param	SrcAddrHigh	- Higher 32 bit address of the GCM-TAG
 *
 * @return
 *	-	XST_SUCCESS - If the decrypt final is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesDecFinal(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XSecure_AesIsDataContextLost();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Verify the GCM tag provided for the data decrypted */
	Status = XSecure_AesDecryptFinal(XSecureAesInstPtr, Addr);

	XSecure_MakeAesFree();
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesKeyZero server API
 *
 * @param	KeySrc	- Key source to be zeroized
 *
 * @return
 *	-	XST_SUCCESS - If the key zeroize is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesKeyZeroize(u32 KeySrc)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	/* Validate key source to allow clearing of keysources other than device keys */
	if ((KeySrc == (u32)XSECURE_AES_KUP_KEY) ||
		(KeySrc == (u32)XSECURE_AES_EXPANDED_KEYS) ||
		((KeySrc >= (u32)XSECURE_AES_USER_KEY_0) &&
		(KeySrc <= (u32)XSECURE_AES_USER_KEY_7)) ||
		(KeySrc == (u32)XSECURE_AES_PUF_KEY)) {
		Status = XSecure_AesKeyZero(XSecureAesInstPtr,
				(XSecure_AesKeySrc)KeySrc);
	}
	else {
		Status = (int)XSECURE_AES_INVALID_PARAM;
	}

	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesWriteKey server API
 *
 * @param	KeySize		- Size of the key to specify 128/256 bit key
 * @param	KeySrc		- KeySrc to which key has to be written
 * @param	KeyAddrLow	- Lower 32 bit address of the Key
 * @param	KeyAddrHigh	- Higher 32 bit address of the Key
 *
 * @return
 *	-	XST_SUCCESS - If the key write is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesKeyWrite(u8  KeySize, u8 KeySrc,
			u32 KeyAddrLow, u32 KeyAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 KeyAddr = ((u64)KeyAddrHigh << 32U) | (u64)KeyAddrLow;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	/* User not allowed to write key in boot header */
	if ((KeySrc == (u32)XSECURE_AES_BH_KEY) ||
		(KeySrc == (u32)XSECURE_AES_BH_RED_KEY)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_AesInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesWriteKey(XSecureAesInstPtr,
				(XSecure_AesKeySrc)KeySrc,
				(XSecure_AesKeySize)KeySize, KeyAddr);

END:
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptKek server API
 *
 * @param	KeyInfo		- KeyInfo contains KeySize, KeyDst and KeySrc
 * @param	IvAddrLow	- Lower 32 bit address of the IV
 * @param	IvAddrHigh	- Higher 32 bit address of the IV
 *
 * @return
 *	-	XST_SUCCESS - If the decryption is successful
 *	-	ErrorCode - If there is a failure
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

	/* Check for valid decryption key source */
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
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesSetDpaCm server API
 *
 * @param	DpaCmCfg	- User DpaCmCfg configuration
 *
 * @return
 *	-	XST_SUCCESS - If the Set DpaCm is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesSetDpaCmConfig(u8 DpaCmCfg)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	if (XPlmi_IsKatRan(XPLMI_SECURE_AES_CMKAT_MASK) != TRUE) {
		Status = (int)XSECURE_ERR_KAT_NOT_EXECUTED;
		goto END;
	}

	Status = XSecure_AesSetDpaCm(XSecureAesInstPtr, DpaCmCfg);
END:
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}
/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesPerformOperation
 *
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the XSecure_AesDataBlockParams
 * 				structure.
 * @param	SrcAddrHigh	- Higher 32 bit address of the XSecure_AesDataBlockParams
 * 				structure.
 *
 * @return
 *	-	XST_SUCCESS - If the initialization is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_AesPerformOperation(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_AesDataBlockParams AesParams;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status =  XPlmi_MemCpy64((u64)(UINTPTR)&AesParams, Addr, sizeof(AesParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesConfig(AesParams.OperationId, AesParams.KeySrc, AesParams.KeySize, AesParams.IvAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (AesParams.IsUpdateAadEn == TRUE) {
		/**< Update AAD */
		Status = XSecure_AesAadUpdate((u32)AesParams.AadAddr, (u32)(AesParams.AadAddr >> 32U),
								 AesParams.AadSize, AesParams.IsGmacEnable);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (AesParams.OperationId == (u32)XSECURE_ENCRYPT) {
		/**< Based on user's choice GCM/GMAC is performed */
		if (AesParams.IsGmacEnable == FALSE) {
			Status = XSecure_AesEncryptData(XSecureAesInstPtr, AesParams.InDataAddr,
					AesParams.OutDataAddr, AesParams.Size, AesParams.GcmTagAddr);
		}
		else {
			Status = XSecure_AesEncryptFinal(XSecureAesInstPtr,AesParams.GcmTagAddr);
		}

	}
	else {
		/**< Based on user's choice GCM/GMAC is performed */
		if (AesParams.IsGmacEnable == FALSE) {
			Status = XSecure_AesDecryptData(XSecureAesInstPtr, AesParams.InDataAddr,
					AesParams.OutDataAddr, AesParams.Size, AesParams.GcmTagAddr);
		}
		else {
			Status = XSecure_AesDecryptFinal(XSecureAesInstPtr,AesParams.GcmTagAddr);
		}
	}


END:
	if (Status != XST_SUCCESS) {
		XSecure_MakeAesFree();
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function checks for valid KeySrc
 *
 *
 * @param	KeySrc	- Aes key source
 *
 * @return
 *	-	XST_SUCCESS - If KeySrc is Valid
 *	-	XSECURE_AES_DEVICE_KEY_NOT_ALLOWED - If KeySrc is Invalid
 *
 ******************************************************************************/
 static int XSecure_IsKeySrcValid(u32 KeySrc)
 {
	 int Status = XST_FAILURE;

	 if ((KeySrc == (u32)XSECURE_AES_BBRAM_KEY) ||
		(KeySrc == (u32)XSECURE_AES_BBRAM_RED_KEY) ||
		(KeySrc == (u32)XSECURE_AES_EFUSE_KEY) ||
		(KeySrc == (u32)XSECURE_AES_EFUSE_RED_KEY) ||
		(KeySrc == (u32)XSECURE_AES_BH_KEY) ||
		(KeySrc == (u32)XSECURE_AES_BH_RED_KEY)) {
		Status = (int)XSECURE_AES_DEVICE_KEY_NOT_ALLOWED;
	}
	else{
		Status = XST_SUCCESS;
	}

	return Status;
 }
/*****************************************************************************/
/**
 * @brief       This function is used to mark the resource as free
 *
 ******************************************************************************/
static void XSecure_MakeAesFree(void)
{
	XSecure_Aes *InstancePtr = XSecure_GetAesInstance();

	InstancePtr->IsResourceBusy = (u32)XSECURE_RESOURCE_FREE;
	InstancePtr->IpiMask = XSECURE_IPI_MASK_DEF_VAL;
}
/*****************************************************************************/
/**
 * @brief       This function is used to check whether any previous data
 * 		context is lost for the corresponding ipi channel or
 * @return
 *      -       XST_SUCCESS - If the context is available
 *      -       XST_DATA_LOST - If the context is lost
 *
 ******************************************************************************/
static int XSecure_AesIsDataContextLost(void)
{
	const XSecure_Aes *InstancePtr = XSecure_GetAesInstance();
	int Status = XST_SUCCESS;
	if (InstancePtr->PreviousAesIpiMask == InstancePtr->IpiMask) {
		if (InstancePtr->DataContextLost != (u32)XSECURE_DATA_CONTEXT_AVAILABLE) {
			Status = XST_DATA_LOST;
		}
	}
	return Status;
}
/*****************************************************************************/
/**
 * @brief       This function is used to validate key source and initialise the encryption/decryption
 *
 * @param	OperationId - Decides whether oepration is encryption/decryption
 * @param	KeySrc - Aes key source
 * @param	KeySize - Aes key size
 * @param	IvAddr - Iv address
 *
 * @return
 *      -       XST_SUCCESS - If successfully initialised
 *      -       XST_FAILURE - Failure in initialisation
 *
 ******************************************************************************/
static int XSecure_AesConfig(u32 OperationId, u32 KeySrc, u32 KeySize, u64 IvAddr)
{
	int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	Status = XSecure_IsKeySrcValid(KeySrc);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**< Selecting the AES Encryption/Decryption operation */
	if (OperationId == (u32)XSECURE_ENCRYPT) {
		if (XPlmi_IsKatRan(XPLMI_SECURE_AES_ENC_KAT_MASK) != TRUE) {
			Status = (int)XSECURE_ERR_KAT_NOT_EXECUTED;
			goto END;
		}
		Status = XSecure_AesEncryptInit(XSecureAesInstPtr,
					(XSecure_AesKeySrc)KeySrc,
					(XSecure_AesKeySize)KeySize,
					IvAddr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		if (XPlmi_IsKatRan(XPLMI_SECURE_AES_DEC_KAT_MASK) != TRUE) {
			Status = (int)XSECURE_ERR_KAT_NOT_EXECUTED;
			goto END;
		}
		Status = XSecure_AesDecryptInit(XSecureAesInstPtr,
				(XSecure_AesKeySrc)KeySrc,
				(XSecure_AesKeySize)KeySize,
				IvAddr);
	}
END:
	return Status;
}
