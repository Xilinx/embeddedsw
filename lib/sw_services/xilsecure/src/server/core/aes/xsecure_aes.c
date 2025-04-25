/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_aes.c
*
* This file contains AES hardware interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 4.0   vns  04/24/2019 Initial release
* 4.1   vns  08/06/2019 Added AES encryption APIs
*       har  08/21/2019 Fixed MISRA C violations
*       vns  08/23/2019 Initialized status variables
* 4.2   har  01/03/2020 Added checks for return value of XSecure_SssAes
*       vns  02/10/2020 Added DPA CM enable/disable function
*       rpo  02/27/2020 Removed function prototype and static keyword of
*                       XSecure_AesKeyLoad, XSecure_AesWaitForDone functions
*       har  03/01/2020 Added code to soft reset once key decryption is done
*       rpo  03/23/2020 Replaced timeouts with WaitForEvent and code clean up
*       rpo  04/02/2020 Added Crypto KAT APIs
*                       Added support of release and set reset for AES
*       bvi  04/07/2020 Renamed csudma as pmcdma
*       vns  04/12/2020 Reset Versal key clear register in AES initialize call
* 4.3   ana  06/04/2020 Updated NextBlkLen in Xsecure_Aes structure wherever required
*                       Updated Aes state
*       kpt  06/29/2020 Added asserts for input arguments and minor
*                       enhancements on AES state
*       kpt  07/03/2020 Added type casting for the arguments in
*                       XPmcDma_64BitTransfer
*       kpt  07/08/2020 Removed dummy code and Status value reinitialized
*                       to XST_FAILURE
*       har  07/12/2020 Removed magic number from XSecure_AesKeyZero
*       har  07/21/2020 Corrected input parameters for config in
*                       XSecure_AesCfgKupKeynIv
*       kpt  08/06/2020 Replaced magic numbers with macro's
*       kpt  08/18/2020 Added volatile keyword to status variable in case of
*                       status reset
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       rpo  09/10/2020 Asserts are not compiled by default for
*                       secure libraries
*       rpo  09/21/2020 New error code added for crypto state mismatch
*       am   09/24/2020 Resolved MISRA C violations
*       har  09/30/2020 Added blind-write checks for XSecure_AesCfgKupKeynIv
*                       Deprecated Family Key support
*                       Replaced repetitive code for DMA configuration with
*                       XSecure_AesPmcDmaCfgByteSwap function
*       har  10/12/2020 Addressed security review comments
*       am   10/10/2020 Resolved Coverity warnings
* 4.5   am   11/24/2020 Resolved MISRA C and Coverity warnings
*       har  02/12/2021 Separated input validation checks for Instance pointer
*       har  03/02/2021 Added support for AES AAD
*       kpt  03/21/2021 Added volatile keyword for SStatus variable in
*                       XSecure_AesDecryptKat to avoid compiler optimization.
*       am   05/21/2021 Resolved MISRA C violations
* 4.6   har  07/14/2021 Fixed doxygen warnings
*       kpt  07/15/2021 Added 64bit support for XSecure_AesWriteKey
*       kal  08/19/2021 Renamed XSecure_AesPmcDmaCfgByteSwap to
*                       XSecure_AesPmcDmaCfgAndXfer
*       har  08/23/2021 Updated AAD size check
*       kpt  09/18/2021 Added redundancy in XSecure_AesSetDpaCm
* 4.7   har  01/03/2022 Updated Status and StatusTmp as volatile in
*                       XSecure_AesWriteKey()
*       har  01/20/2022 Added glitch checks for clearing keys in
*                       XSecure_AesWriteKey()
*       har  02/16/2022 Updated Status with ClearStatus only in case of success
* 5.0   kpt  07/24/2022 Moved XSecure_AesDecryptKat into XSecure_Kat.c and fixed bug in
*                       XSecure_AesDecryptCmKat
*       kpt  08/02/2022 Zeroized user key in XSecure_AesDecryptCmKat
*       kpt  08/19/2022 Added GMAC support
*       dc   08/26/2022 Optimized the code
* 5.1   kal  09/27/2022 Pass AesDmCfg structure as reference instead of value to
*                       XSecure_AesPmcDmaCfgAndXfer function
*       skg  10/13/2022 Added Encrypt/Decrypt update error handling check
* 5.2   yog  07/10/2023 Added support of unaligned data sizes for Versal Net
*       kpt  07/09/2023 Added AES ECB mode support for versalnet
*       ng   07/13/2023 Added SDT support
*       kpt  07/20/2023 Added volatile keyword for SStatus variable in XSecure_AesDecryptFinal
*       kpt  07/20/2023 Renamed XSecure_AesDpaCmDecryptKat to XSecure_AesDpaCmDecryptData
*	    kpt  07/27/2023 Initialize KeySizeInWords to zero to avoid invalid value in case of glitch
*       vss  09/11/2023 Fixed Coverity warning EXPRESSION_WITH_MAGIC_NUMBERS and MISRA-C Rule 10.1 violation
*       vss  09/11/2023 Fixed MISRA-C Rule 8.13 violation
*       vss  09/11/2023 Fixed MISRA-C Rule 10.3 and 10.4 violation
* 5.3   kpt  11/28/2023 Add support to clear AES PUF,RED,KUP keys
*       har  03/20/2024 Add support for non-word aligned data in XSecure_AesEncryptData
*                       and XSecure_AesDecryptData
*       kpt  03/22/2024 Fix overrun issue
* 5.4   yog  04/29/2024 Fixed doxygen warnings.
*	vss  10/23/2024 Removed AES duplicate code
*       vss  10/28/2024 Removed END label in XSecure_AesDecryptUpdate
*       vss  11/20/2024 Fix for data corruption of GCM tag when any other
*                       operation uses DMA0 after encrypt update.
*       vss  01/22/2025   Added status check in AesUpdate and CopyGcmTag functions.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_server_apis XilSecure AES Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_utils.h"
#include "xsecure_aes.h"
#include "xsecure_aes_core_hw.h"
#include "xil_sutil.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

#define XSECURE_KEK_DEC_ENABLE			(0x1U)	/**< Triggers decryption operation
											for black key */

#define XSECURE_AES_DISABLE_KUP_IV_UPDATE	(0x0U)	/**< Disables IV and Key save
											features for KUP */
#define XSECURE_AES_ENABLE_KUP_IV_UPDATE	(0x1U)	/**< Enables IV and Key save
											features for KUP */
#define XSECURE_AES_AAD_ENABLE			(0x1U)	/**< Enables authentication of
													data pushed in AES engine*/
#define XSECURE_AES_AAD_DISABLE			(0x0U)	/**< Disables authentication of
													data pushed in AES engine*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/

static int XSecure_AesWaitForDone(const XSecure_Aes *InstancePtr);
static int XSecure_AesKeyLoad(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize);
static int XSecure_AesKekWaitForDone(const XSecure_Aes *InstancePtr);
static int XSecure_AesOpInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	XSecure_AesKeySize KeySize, u64 IvAddr, u32 Mode);
static int XSecure_AesPmcDmaCfgAndXfer(const XSecure_Aes *InstancePtr,
	XSecure_AesDmaCfg *AesDmaCfg, u32 Size);
static int XSecureAesUpdate(const XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk);
static int XSecure_AesIvXfer(const XSecure_Aes *InstancePtr, u64 IvAddr);
static int XSecure_AesKeyLoadandIvXfer(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize, u64 IvAddr);
static int XSecure_ValidateAndUpdateData(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk);
static int XSecure_AesCopyGcmTag(const XSecure_Aes *InstancePtr,
	XSecure_AesDmaCfg* AesDmaCfg);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the AES instance pointer
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	PmcDmaPtr	Pointer to the XPmcDma instance
 *
 * @return
 *		 - XST_SUCCESS  If initialization was successful
 *		 - XSECURE_AES_INVALID_PARAM  For invalid parameter
 *		 - XST_FAILURE  On failure
 *
 * @note	All the inputs are accepted in little endian format, but AES
 *		engine accepts the data in big endianness, this will be taken
 *		care while passing data to AES engine
 *
 ******************************************************************************/
int XSecure_AesInitialize(XSecure_Aes *InstancePtr, XPmcDma *PmcDmaPtr)
{
	int Status = XST_FAILURE;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (PmcDmaPtr == NULL) ||
			(PmcDmaPtr->IsReady != (u32)(XIL_COMPONENT_IS_READY))) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	/** Initialize the instance */
	InstancePtr->BaseAddress = XSECURE_AES_BASEADDR;
	InstancePtr->PmcDmaPtr = PmcDmaPtr;
	InstancePtr->NextBlkLen = 0U;
	InstancePtr->IsGmacEn = FALSE;
#ifdef VERSAL_NET
	InstancePtr->IsEcbEn = (u32)FALSE;
#endif
	InstancePtr->DmaSwapEn = XSECURE_DISABLE_BYTE_SWAP;

	/* Clear all key zeroization register */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_CLEAR_OFFSET, XSECURE_AES_KEY_CLR_REG_CLR_MASK);

	/** Initialize SSS Instance */
	Status = XSecure_SssInitialize(&(InstancePtr->SssInstance));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	InstancePtr->AesState = XSECURE_AES_INITIALIZED;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables or disables DPA counter measures in AES engine
 * 		based on user input
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	DpaCmCfg	User choice to enable/disable DPA CM
 *				- TRUE - to enable AES DPA counter measure
 *					(Default setting)
 *				- FALSE - to disable AES DPA counter measure
 *
 * @return
 *		 - XST_SUCCESS  If configuration is success
 *		 - XSECURE_AES_INVALID_PARAM  For invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XSECURE_AES_DPA_CM_NOT_SUPPORTED  If DPA CM is disabled on chip
 *		(Enabling/Disabling in AES engine does not impact functionality)
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesSetDpaCm(const XSecure_Aes *InstancePtr, u32 DpaCmCfg)
{
	int Status = XST_FAILURE;
	u32 ReadReg;
	volatile u32 DpaCmCfgEn = DpaCmCfg;
	volatile u32 DpaCmCfgEnTmp = DpaCmCfg;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Chip has DPA CM support */
	if ((XSecure_In32(XSECURE_EFUSE_SECURITY_MISC1) &
		XSECURE_EFUSE_DPA_CM_DIS_MASK) != XSECURE_EFUSE_DPA_CM_DIS_MASK) {

		/** Set DPA counter measures as per the user input */
		if ((DpaCmCfgEn != FALSE) || (DpaCmCfgEnTmp != FALSE)) {
			DpaCmCfgEn = TRUE;
			DpaCmCfgEnTmp = TRUE;
		}
		XSecure_WriteReg(InstancePtr->BaseAddress,
						XSECURE_AES_CM_EN_OFFSET, (DpaCmCfgEn | DpaCmCfgEnTmp));

		/* Verify status of CM */
		ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_AES_STATUS_OFFSET);
		ReadReg = (ReadReg & XSECURE_AES_STATUS_CM_ENABLED_MASK) >>
					XSECURE_AES_STATUS_CM_ENABLED_SHIFT;
		if (ReadReg == (DpaCmCfgEn | DpaCmCfgEnTmp)) {
			Status = XST_SUCCESS;
		}
	}
	else {
		Status = (int)XSECURE_AES_DPA_CM_NOT_SUPPORTED;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function writes the key provided into the specified
 *		AES key registers
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	KeySrc		Key Source to be selected to which provided
 *				key should be updated by
 *				- XSECURE_AES_USER_KEY_0
 *				- XSECURE_AES_USER_KEY_1
 *				- XSECURE_AES_USER_KEY_2
 *				- XSECURE_AES_USER_KEY_3
 *				- XSECURE_AES_USER_KEY_4
 *				- XSECURE_AES_USER_KEY_5
 *				- XSECURE_AES_USER_KEY_6
 *				- XSECURE_AES_USER_KEY_7
 *				- XSECURE_AES_BH_KEY
 * @param	KeySize		A variable of type XSecure_AesKeySize, which holds
 *				the size of the input key to be written by
 *				- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	KeyAddr		Address of a buffer which should contain the key
 * 				to be written
 *
 * @return
 *		 - XST_SUCCESS  On successful key written on AES registers
 *		 - XSECURE_AES_INVALID_PARAM  For invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesWriteKey(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize, u64 KeyAddr)
{
	volatile int Status = XST_GLITCH_ERROR;
	volatile int ClearStatus = XST_FAILURE;
	volatile int ClearStatusTmp = XST_FAILURE;
	u32 Offset;
	volatile u32 Index = 0U;
	u32 Key[XSECURE_AES_KEY_SIZE_256BIT_WORDS];
	u32 KeySizeInWords = 0U;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (KeySrc >= XSECURE_MAX_KEY_SOURCES) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((AesKeyLookupTbl[KeySrc].UsrWrAllowed != TRUE) ||
		(KeyAddr == 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((XSECURE_AES_KEY_SIZE_128 != KeySize) &&
		 (XSECURE_AES_KEY_SIZE_256 != KeySize)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->AesState != XSECURE_AES_INITIALIZED)) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	if ((XSECURE_AES_BH_KEY == KeySrc) &&
			(XSECURE_AES_KEY_SIZE_128 == KeySize)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	Offset = AesKeyLookupTbl[KeySrc].RegOffset;
	if (Offset == XSECURE_AES_INVALID_CFG) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (XSECURE_AES_KEY_SIZE_128 == KeySize) {
		KeySizeInWords = XSECURE_AES_KEY_SIZE_128BIT_WORDS;
	}
	else {
		KeySizeInWords = XSECURE_AES_KEY_SIZE_256BIT_WORDS;
	}

	XSecure_MemCpy64((u64)(UINTPTR)Key, KeyAddr, KeySizeInWords *
				XSECURE_WORD_SIZE);

	Offset = Offset + (KeySizeInWords * XSECURE_WORD_SIZE) -
				XSECURE_WORD_SIZE;

	/** Write key into the specified AES key registers */
	for (Index = 0U; Index < KeySizeInWords; Index++) {
		XSecure_WriteReg(InstancePtr->BaseAddress, Offset,
					Xil_Htonl(Key[Index]));
		Offset = Offset - XSECURE_WORD_SIZE;
	}
	if ((Index == KeySizeInWords) && (KeySizeInWords != 0U)) {
		Status = XST_SUCCESS;
	}

END:
	ClearStatus = Xil_SecureZeroize((u8*)Key, XSECURE_AES_KEY_SIZE_256BIT_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8*)Key, XSECURE_AES_KEY_SIZE_256BIT_BYTES);
	if (Status == XST_SUCCESS) {
		Status = (ClearStatus | ClearStatusTmp);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the AES engine with
 *		Additional Authenticated Data(AAD).
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	AadAddr		Address of the additional authenticated data
 * @param	AadSize		Size of additional authenticated data in bytes,
 *				whereas number of bytes provided should be
 *				quad-word aligned(multiples of 16 bytes) for Versal
 *				For Versal Net, byte aligned data is
 *				accepted
 *
 * @return
 *		 - XST_SUCCESS  On successful update of AAD
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch occurs
 *		 - XST_FAILURE  On failure to update AAD
 *
 * @note	The API must be called after XSecure_AesEncryptInit() or
 *		XSecure_AesDecryptInit()
 *
 ******************************************************************************/
int XSecure_AesUpdateAad(XSecure_Aes *InstancePtr, u64 AadAddr, u32 AadSize)
{
	int Status = XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (AadAddr == 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (XSecure_AesIsEcbModeEn(InstancePtr) != FALSE) {
		Status = (int)XSECURE_AES_INVALID_MODE;
		goto END;
	}

#ifndef VERSAL_NET
	/* Validate Aad Size for versal*/
	if ((AadSize % XSECURE_QWORD_SIZE) != 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}
#endif
	/* Validate AES state */
	if (InstancePtr->AesState != XSECURE_AES_OPERATION_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}
	/** Enable AAD by writing to the register */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_AAD_OFFSET,
		XSECURE_AES_AAD_ENABLE);

	AesDmaCfg.SrcDataAddr = AadAddr;
	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.IsLastChunkSrc = (u8)InstancePtr->IsGmacEn;

	/*
	 * Enable destination channel swapping to read
	 * GMAC tag in correct order from hardware.
	 */
	if (InstancePtr->IsGmacEn == TRUE) {
		InstancePtr->AesState = XSECURE_AES_UPDATE_DONE;
#ifndef VERSAL_2VE_2VM
		XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
            XPMCDMA_DST_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
#endif
	}

	/** Configure DMA and transfer AAD to AES engine */
	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, &AesDmaCfg, AadSize);
	if (Status != XST_SUCCESS) {
		goto CLEAR;
	}

	if ((InstancePtr->IsGmacEn == TRUE) && (InstancePtr->OperationId) == XSECURE_ENCRYPT){
		Status = XSecure_AesCopyGcmTag(InstancePtr, &AesDmaCfg);
	}

	InstancePtr->IsGmacEn = FALSE;
CLEAR:
#ifndef VERSAL_2VE_2VM
	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
#endif
	/* Disable AAD */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_AAD_OFFSET,
		XSECURE_AES_AAD_DISABLE);

	if (Status != XST_SUCCESS) {
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the KEK/obfuscated key which is found
 * 		encrypted in boot header/eFUSE/BBRAM. It then places it in the
 * 		specified red key register.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	DecKeySrc	Select key source which holds KEK and needs to be
 *				decrypted
 * @param	DstKeySrc	Select the key in which decrypted red key should be
 * 				updated
 * @param	IvAddr		Address of IV holding buffer for decryption
 * 				of the key
 * @param	KeySize		A variable of type XSecure_AesKeySize, which
 * 				specifies the size of the key to be
 *				- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 *
 * @return
 *		 - XST_SUCCESS  On successful key decryption
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch occurs
 *		 - XST_FAILURE  If timeout has occurred
 *
 ******************************************************************************/
int XSecure_AesKekDecrypt(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc DecKeySrc, XSecure_AesKeySrc DstKeySrc, u64 IvAddr,
	XSecure_AesKeySize KeySize)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesKeySrc KeySrc;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (IvAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if (DecKeySrc >= XSECURE_MAX_KEY_SOURCES) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((XSECURE_AES_KEY_SIZE_128 != KeySize) &&
		 (XSECURE_AES_KEY_SIZE_256 != KeySize)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((AesKeyLookupTbl[DecKeySrc].KeyDecSrcAllowed != TRUE) ||
	    (AesKeyLookupTbl[DstKeySrc].KeyDecSrcSelVal ==
				XSECURE_AES_INVALID_CFG)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_RST;
	}

	if ((InstancePtr->AesState != XSECURE_AES_INITIALIZED)) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_RST;
	}

	XSecure_ReleaseReset(InstancePtr->BaseAddress,
		XSECURE_AES_SOFT_RST_OFFSET);

	KeySrc = XSECURE_AES_PUF_KEY;

	Status = XST_FAILURE;

	/** Load AES key to registers from PUF key */
	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	/* Enable Byte swap */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_ENABLE_BYTE_SWAP);

	Status = XST_FAILURE;

	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.SrcDataAddr = IvAddr;
	AesDmaCfg.IsLastChunkSrc = TRUE;

	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, &AesDmaCfg,
		XSECURE_SECURE_GCM_TAG_SIZE);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);

	/** Configure the AES engine for Key decryption and starts operation */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_DEC_OFFSET, XSECURE_AES_KEY_DEC_MASK);

	/* Decrypt selection */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_DEC_SEL_OFFSET,
		AesKeyLookupTbl[DstKeySrc].KeyDecSrcSelVal);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_SEL_OFFSET,
		AesKeyLookupTbl[DecKeySrc].KeySrcSelVal);

	/** Trigger key decryption operation */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_DEC_TRIG_OFFSET, XSECURE_KEK_DEC_ENABLE);

	Status = XST_FAILURE;

	/** Wait for AES Decryption completion. */
	Status = XSecure_AesKekWaitForDone(InstancePtr);

END_RST:
	/* Select key decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_DEC_OFFSET, XSECURE_AES_KEY_DEC_RESET_MASK);

	XSecure_SetReset(InstancePtr->BaseAddress,
		XSECURE_AES_SOFT_RST_OFFSET);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the AES engine for decryption
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	KeySrc		Key Source for decryption of the data
 * @param	KeySize		Size of the AES key to be used for decryption is
 *		 		- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		Address to the buffer holding IV
 *
 * @return
 *		 - XST_SUCCESS  On successful init
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XSECURE_AES_ZERO_PUF_KEY_NOT_ALLOWED  If keysrc is puf and
 *				puf key is zero
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	XSecure_AesKeySize KeySize, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;
	InstancePtr->OperationId = XSECURE_DECRYPT;

	Status = XSecure_AesOpInit(InstancePtr, KeySrc, KeySize, IvAddr, XSECURE_AES_MODE_DEC);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the AES engine for decryption with
 * 		provided data and stores the decrypted data at specified address
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *				  decrypted
 * @param	OutDataAddr	Address of output buffer where the decrypted
 *				  to be updated
 * @param	Size		Size of data to be decrypted in bytes, whereas number of bytes shall be
 *				aligned as below for Versal
 *				- 16 byte aligned when it is not the last chunk
 *				- 4 byte aligned when the data is the last chunk
 *				For Versal Net, byte aligned data is accepted
 * @param	IsLastChunk	If this is the last update of data to be decrypted,
 *				  this parameter should be set to TRUE otherwise FALSE
 *
 * @return
 *		 - XST_SUCCESS  On successful decryption of the data
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_UNALIGNED_SIZE_ERROR  If input IsLastChunk is invalid
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;

	Status = XSecure_ValidateAndUpdateData(InstancePtr, InDataAddr, OutDataAddr, Size, IsLastChunk);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function verifies the GCM tag provided for the data decrypted
 * 		till the point
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	GcmTagAddr	Address of a buffer which should holds GCM Tag
 *
 * @return
 *		 - XST_SUCCESS  On successful GCM tag verification
 *		 - XSECURE_AES_GCM_TAG_MISMATCH  User provided GCM tag does not
 *		match calculated tag
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_INVALID_MODE  If input mode is invalid
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 RegVal;
	volatile u32 RegValTmp;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (XSecure_AesIsEcbModeEn(InstancePtr) != FALSE) {
		Status = (int)XSECURE_AES_INVALID_MODE;
		goto END;
	}

	if (GcmTagAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XSECURE_AES_UPDATE_DONE) {
			Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
			goto END;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_ENABLE_BYTE_SWAP);

	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.SrcDataAddr = GcmTagAddr;
	AesDmaCfg.IsLastChunkSrc = FALSE;

	/** Update AES engine with the GCM Tag data to verify the GCM Tag */
	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, &AesDmaCfg,
		XSECURE_SECURE_GCM_TAG_SIZE);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = XST_FAILURE;

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesWaitForDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = XST_FAILURE;

	RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET);
	RegValTmp = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET);
	RegVal &= XSECURE_AES_STATUS_GCM_TAG_PASS_MASK;
	RegValTmp &= XSECURE_AES_STATUS_GCM_TAG_PASS_MASK;

	/** Verify whether GCM Tag is matched or not */
	if ((RegVal != XSECURE_AES_STATUS_GCM_TAG_PASS_MASK) ||
	   (RegValTmp != XSECURE_AES_STATUS_GCM_TAG_PASS_MASK)) {
		Status = (int)XSECURE_AES_GCM_TAG_MISMATCH;
		goto END_RST;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesGetNxtBlkLen(InstancePtr, &InstancePtr->NextBlkLen);

END_RST:
	InstancePtr->IsGmacEn = FALSE;
#ifndef VERSAL_2VE_2VM
	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		XSECURE_DISABLE_BYTE_SWAP);
#endif
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);
	InstancePtr->AesState = XSECURE_AES_INITIALIZED;
	if ((InstancePtr->NextBlkLen == 0U) || (Status != XST_SUCCESS)) {
		InstancePtr->NextBlkLen = 0U;
		SStatus = XSecure_AesKeyZero(InstancePtr, XSECURE_AES_KUP_KEY);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}

		SStatus = XST_FAILURE;
		SStatus = XSecure_AesKeyZero(InstancePtr, XSECURE_AES_EXPANDED_KEYS);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
		/*
		 * Issue a soft to reset to AES engine
		 */
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decrypts the  size (length) number of bytes of the
 * 		passed in InDataAddr (source) buffer and stores the decrypted data
 * 		in the OutDataAddr (destination) buffer and verifies GcmTagAddr
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *				  decrypted
 * @param	OutDataAddr	Address of output buffer where the decrypted to be
 *				  updated
 * @param	Size		Size of data to be decrypted in bytes, whereas number
 *				of bytes provided should be multiples of 4 for Versal
 *				For Versal Net, byte aligned data is acceptable
 * @param	GcmTagAddr	Address of a buffer which should contain GCM Tag
 *
 * @return
 *		 - XST_SUCCESS  On successful decryption and GCM tag verification
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_INVALID_MODE  If input mode is invalid
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_AesValidateSize(Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (XSecure_AesIsEcbModeEn(InstancePtr) != FALSE) {
		Status = (int)XSECURE_AES_INVALID_MODE;
		goto END;
	}

	if (GcmTagAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XSECURE_AES_OPERATION_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Update AES engine with encrypted data and get the decrypted data output */
	Status = XSecure_AesDecryptUpdate(InstancePtr, InDataAddr, OutDataAddr,
						Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	/** Verify GCM tag */
	Status = XSecure_AesDecryptFinal(InstancePtr, GcmTagAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the AES engine for encryption
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	KeySrc		Key Source for encryption
 * @param	KeySize		Size of the AES key to be used for encryption is
 *			 	- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		Address to the buffer holding IV
 *
 * @return
 *		 - XST_SUCCESS  On successful initialization
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XSECURE_AES_ZERO_PUF_KEY_NOT_ALLOWED  If keysrc is puf and
 *				puf key is zero
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	XSecure_AesKeySize KeySize, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;
	InstancePtr->OperationId = XSECURE_ENCRYPT;

	Status = XSecure_AesOpInit(InstancePtr, KeySrc, KeySize, IvAddr, XSECURE_AES_MODE_ENC);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the AES engine for encryption with
 * 		provided data and stores the decrypted data at specified address
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *				  encrypted
 * @param	OutDataAddr	Address of output buffer where the encrypted data
 *				  to be updated
 * @param	Size		Size of data to be decrypted in bytes, whereas number of bytes shall be
 *				aligned as below for Versal
 *				- 16 byte aligned when it is not the last chunk
 *				- 4 byte aligned when the data is the last chunk
 *				For Versal Net, byte aligned data is acceptable
 * @param	IsLastChunk	If this is the last update of data to be encrypted,
 *				this parameter should be set to TRUE otherwise FALSE
 *
 * @return
 *		 - XST_SUCCESS  On successful encryption of the data
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_UNALIGNED_SIZE_ERROR  If input IsLastChunk is invalid
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;

	Status = XSecure_ValidateAndUpdateData(InstancePtr, InDataAddr, OutDataAddr, Size, IsLastChunk);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the GCM tag for the encrypted data
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	GcmTagAddr	Address to the buffer of GCM tag size, where the API
 *				updates GCM tag
 *
 * @return
 *		 - XST_SUCCESS  On successful GCM tag updation
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_INVALID_MODE  If input mode is invalid
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (XSecure_AesIsEcbModeEn(InstancePtr) != FALSE) {
		Status = (int)XSECURE_AES_INVALID_MODE;
		goto END;
	}

	if (GcmTagAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}


	if (InstancePtr->AesState != XSECURE_AES_UPDATE_DONE) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Copying from local buffer which is already stored during encrypt update. */
	XSecure_MemCpy64(GcmTagAddr, (u64)(UINTPTR)InstancePtr->GcmTag, XSECURE_SECURE_GCM_TAG_SIZE);

	Status = XST_SUCCESS;
	InstancePtr->AesState = XSECURE_AES_INITIALIZED;
#ifndef VERSAL_2VE_2VM
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
			XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
#endif
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);

	SStatus = XSecure_AesKeyZero(InstancePtr, XSECURE_AES_KUP_KEY);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = XSecure_AesKeyZero(InstancePtr, XSECURE_AES_EXPANDED_KEYS);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
	XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);

	SStatus = Xil_SecureZeroize(InstancePtr->GcmTag, XSECURE_SECURE_GCM_TAG_SIZE);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function encrypts size (length) number of bytes of the passed
 * 		in InDataAddr (source) buffer and stores the encrypted data along
 * 		with its associated 16 byte tag in the OutDataAddr (destination)
 * 		buffer and GcmTagAddr (buffer) respectively
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	InDataAddr	Address of the data which needs to be encrypted
 * @param	OutDataAddr	Address of output buffer where the encrypted data to
 *				  be updated
 * @param	Size		Size of data to be encrypted in bytes, whereas number
 *				of bytes provided should be multiples of 4 for Versal
 *				For Versal Net, byte aligned data is acceptable
 * @param	GcmTagAddr	Address to the buffer of GCM tag size, where the API
 *				  updates GCM tag
 *
 * @return
 *		 - XST_SUCCESS  On successful encryption
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_INVALID_MODE  If input mode is invalid
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	volatile int Status = XST_FAILURE;

	/** Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (XSecure_AesIsEcbModeEn(InstancePtr) != FALSE) {
		Status = (int)XSECURE_AES_INVALID_MODE;
		goto END;
	}

	if (GcmTagAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_AesValidateSize(Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (InstancePtr->AesState != XSECURE_AES_OPERATION_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Update the data to AES engine */
	Status = XSecure_AesEncryptUpdate(InstancePtr, InDataAddr, OutDataAddr,
					Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	/** Get the GCM Tag */
	Status = XSecure_AesEncryptFinal(InstancePtr, GcmTagAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets AES engine to update key and IV during
 * 		decryption of secure header or footer of encrypted partition
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	Config		XSECURE_AES_ENABLE_KUP_IV_UPDATE to enable KUP and
 *				IV update,
 *				XSECURE_AES_DISABLE_KUP_IV_UPDATE to disable KUP and
 *				IV update
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesCfgKupKeyNIv(const XSecure_Aes *InstancePtr, u8 Config)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	/** Enable/disable the KUP and IV update as per the configuration provided */
	if ((Config != XSECURE_AES_DISABLE_KUP_IV_UPDATE) &&
		 (Config != XSECURE_AES_ENABLE_KUP_IV_UPDATE)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	if (Config == XSECURE_AES_DISABLE_KUP_IV_UPDATE) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KUP_WR_OFFSET,
			XSECURE_AES_DISABLE_KUP_IV_UPDATE);
	}
	else {
		Status = XSecure_SecureOut32((UINTPTR)(InstancePtr->BaseAddress +
			XSECURE_AES_KUP_WR_OFFSET),
			(XSECURE_AES_KUP_WR_KEY_SAVE_MASK |
			XSECURE_AES_KUP_WR_IV_SAVE_MASK));
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gives the AES next block length after decryption
 * 		of PDI block
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	Size		Pointer to a 32 bit variable where next block
 *				  length will be updated
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *
 ******************************************************************************/
int XSecure_AesGetNxtBlkLen(const XSecure_Aes *InstancePtr, u32 *Size)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Size == NULL)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	*Size = Xil_Htonl(XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_IV_3_OFFSET)) * XSECURE_WORD_SIZE;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function zeroizes the selected AES key storage register
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	KeySrc		Select the key source which needs to be zeroized
 *
 * @return
 *		 - XST_SUCCESS  When key zeroization is success
 *		 - XST_INVALID_PARAM  if key source is invalid
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XSECURE_AES_KEY_CLEAR_ERROR  AES key clear error
 *
 ******************************************************************************/
int XSecure_AesKeyZero(const XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc)
{
	int Status = XST_FAILURE;
	u32 Mask;
	u32 RstState = XSECURE_RESET_UNSET;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (KeySrc > XSECURE_AES_ALL_KEYS) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END_CLR;
	}

	if (InstancePtr->AesState == XSECURE_AES_UNINITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END_CLR;
	}

	if (KeySrc == XSECURE_AES_ALL_KEYS) {
		Mask = XSECURE_AES_KEY_CLEAR_ALL_KEYS_MASK;
	}
	else if (KeySrc == XSECURE_AES_PUF_RED_EXPANDED_KEYS) {
		Mask = XSECURE_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK;
	}
	else if (KeySrc == XSECURE_AES_EXPANDED_KEYS) {
		Mask = XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK;
	}
	else if (AesKeyLookupTbl[KeySrc].KeyClearVal != XSECURE_AES_INVALID_CFG) {
		Mask = AesKeyLookupTbl[KeySrc].KeyClearVal;
	}
	else {
		Status = XST_INVALID_PARAM;
		goto END_CLR;
	}

	RstState = XSecure_ReadReg(InstancePtr->BaseAddress,XSECURE_AES_SOFT_RST_OFFSET);
	if (RstState == XSECURE_RESET_SET){
		XSecure_ReleaseReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

	/** Zeroize the specified key source */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_CLEAR_OFFSET,
					 Mask);

	Status = (int)Xil_WaitForEvent(((InstancePtr)->BaseAddress +
				XSECURE_AES_KEY_ZEROED_STATUS_OFFSET),
				Mask,
				Mask,
				XSECURE_AES_TIMEOUT_MAX);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KEY_CLEAR_ERROR;
	}

END_CLR:
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_CLEAR_OFFSET,
		XSECURE_AES_KEY_CLR_REG_CLR_MASK);
	if (RstState == XSECURE_RESET_SET){
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates data and key to the AES core in split mode
 *              with DPACM enabled
 *
 * @param 	AesInstance	InstancePtr Pointer to the XSecure_Aes instance
 * @param 	KeyPtr		Key Pointer
 * @param 	DataPtr		Data Pointer
 * @param 	OutputPtr	Output where the decrypted data to be stored
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_AESKAT_INVALID_PARAM  Invalid Argument
 *		 - XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR  Error when AESDPACM key
 *							write fails
 *		 - XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR  Error when AESDPACM key
 *							load fails
 *		 - XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR  Error when AESDPACM sss
 *							configuration fails
 *		 - XSECURE_AESDPACM_KAT_FAILED_ERROR  Error when AESDPACM KAT fails
 *		 - XST_FAILURE  On failure
 *
 * @note
 *             This function is used during DPACM KAT where key and data are
 *             updated in split mode with DPACM enabled
 *
 *****************************************************************************/
int XSecure_AesDpaCmDecryptData(const XSecure_Aes *AesInstance,
	const u32 *KeyPtr, const u32 *DataPtr, u32 *OutputPtr)
{
	volatile int Status = XST_FAILURE;
	u32 Index;
	u32 ReadReg = 0U;

	if ((KeyPtr == NULL) ||
	   (DataPtr == NULL) ||
	   (OutputPtr == NULL) ||
	   (AesInstance->PmcDmaPtr == NULL)) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	XSecure_ReleaseReset(AesInstance->BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);

	/** Configure AES for Encryption */
	XSecure_WriteReg(AesInstance->BaseAddress,
		XSECURE_AES_MODE_OFFSET, XSECURE_AES_MODE_ENC);

	/** Configure AES engine in split mode to update data and key to aes core */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_SPLIT_CFG_OFFSET,
		(XSECURE_AES_SPLIT_CFG_KEY_SPLIT |
		XSECURE_AES_SPLIT_CFG_DATA_SPLIT));

	Status = XSecure_AesSetDpaCm(AesInstance, (u32)TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_CM_EN_OFFSET,
		XSECURE_AES_CM_EN_VAL_MASK);

	/** Write Key mask value */
	for (Index = 0U; Index < XSECURE_AES_KEY_SIZE_256BIT_WORDS; Index++) {
		XSecure_WriteReg(AesInstance->BaseAddress,
			XSECURE_AES_KEY_MASK_INDEX + (u32)(Index * XSECURE_WORD_SIZE),
			0x0U);
	}

	/** Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)KeyPtr);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR;
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesKeyLoad(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR;
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_SssAes(&AesInstance->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	if(Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR;
		goto END;
	}

	/** Start the message. */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_START_MSG_OFFSET,
		XSECURE_AES_START_MSG_VAL_MASK);

	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_DATA_SWAP_OFFSET,
		XSECURE_AES_DATA_SWAP_VAL_MASK);
#ifndef VERSAL_2VE_2VM
	/* Enable PMC DMA Src channel for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_MASK);

	/* Enable PMC DMA Dst channel for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_MASK);
#endif
	/** Configure the PMC DMA Tx/Rx for the incoming Block. */
	XPmcDma_Transfer(AesInstance->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
		(u64)(UINTPTR)OutputPtr, XSECURE_AES_DMA_SIZE,
		XSECURE_AES_DMA_LAST_WORD_DISABLE);

	XPmcDma_Transfer(AesInstance->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		(u64)(UINTPTR)DataPtr, XSECURE_AES_DMA_SIZE,
		XSECURE_AES_DMA_LAST_WORD_ENABLE);

	Status = XPmcDma_WaitForDoneTimeout(AesInstance->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesWaitForDone(AesInstance);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_FAILED_ERROR;
		goto END;
	}

	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_DATA_SWAP_OFFSET,
		XSECURE_AES_DATA_SWAP_VAL_DISABLE);

END:
	XPmcDma_IntrClear(AesInstance->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
		XPMCDMA_IXR_DONE_MASK);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(AesInstance->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		XPMCDMA_IXR_DONE_MASK);
#ifndef VERSAL_2VE_2VM
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_DISABLE);

	/* Disable PMC DMA Dst channel for byte swapping. */
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_DISABLE);
#endif
	/* Configure AES in split mode */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_SPLIT_CFG_OFFSET,
		XSECURE_AES_SPLIT_CFG_DATA_KEY_DISABLE);

	/* Copy the initial status of DPACM */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_CM_EN_OFFSET,
		ReadReg);

	/* AES reset */
	XSecure_SetReset(AesInstance->BaseAddress, XSECURE_AES_SOFT_RST_OFFSET);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function waits for AES engine completes key loading
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 *
 * @return
 *		 - XST_SUCCESS  If the AES engine completes key loading
 *		 - XST_FAILURE  If a timeout has occurred
 *
 ******************************************************************************/
static int XSecure_AesWaitKeyLoad(const XSecure_Aes *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);
	Status = (int)Xil_WaitForEvent(((InstancePtr)->BaseAddress +
					XSECURE_AES_STATUS_OFFSET),
			XSECURE_AES_STATUS_KEY_INIT_DONE_MASK,
			XSECURE_AES_STATUS_KEY_INIT_DONE_MASK,
			XSECURE_AES_TIMEOUT_MAX);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures and loads AES key from selected
 *		key source

 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	KeySrc		Variable is of type XSecure_AesKeySrc which
 *				mentions the key source to be loaded into AES engine
 * @param	KeySize		Size of the key selected
 *
 * @return
 *		 - XST_SUCCESS  On successful key load
 *		 - XST_FAILURE  If a timeout has occurred
 *
 ******************************************************************************/
static int XSecure_AesKeyLoad(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);
	XSecure_AssertNonvoid((KeySrc < XSECURE_MAX_KEY_SOURCES) &&
		(KeySrc >= XSECURE_AES_BBRAM_KEY));
	XSecure_AssertNonvoid((KeySize == XSECURE_AES_KEY_SIZE_128) ||
			  (KeySize == XSECURE_AES_KEY_SIZE_256));

	/* Load Key Size */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_SIZE_OFFSET,
		(u32)KeySize);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_SEL_OFFSET,
			AesKeyLookupTbl[KeySrc].KeySrcSelVal);

	/* Trig loading of key. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_LOAD_OFFSET,
			XSECURE_AES_KEY_LOAD_VAL_MASK);

	/* Wait for AES key loading.*/
	Status = XSecure_AesWaitKeyLoad(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function waits for AES completion
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 *
 * @return
 *		 - XST_SUCCESS  On successful key load
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_AesWaitForDone(const XSecure_Aes *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);

	Status = (int)Xil_WaitForEvent(((InstancePtr)->BaseAddress +
				XSECURE_AES_STATUS_OFFSET),
				XSECURE_AES_STATUS_DONE_MASK,
				XSECURE_AES_STATUS_DONE_MASK,
				XSECURE_AES_TIMEOUT_MAX);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function waits for AES key decryption completion
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_AesKekWaitForDone(const XSecure_Aes *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);

	Status = (int)Xil_WaitForEvent(((InstancePtr)->BaseAddress +
				XSECURE_AES_STATUS_OFFSET),
				XSECURE_AES_STATUS_BLK_KEY_DEC_DONE_MASK,
				XSECURE_AES_STATUS_BLK_KEY_DEC_DONE_MASK,
				XSECURE_AES_TIMEOUT_MAX);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes AES engine for encryption or decryption
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	KeySrc		Key Source for decryption of the data
 * @param	KeySize		Size of the AES key to be used for decryption
 *				- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		Address to the buffer holding IV
 *
 * @return
 *		 - XST_SUCCESS  On successful initialization
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_AesKeyLoadandIvXfer(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_ENABLE_BYTE_SWAP);

	if (XSecure_AesIsEcbModeEn(InstancePtr) == FALSE) {
		Status = XSecure_AesIvXfer(InstancePtr, IvAddr);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function transfers IV to AES engine
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	IvAddr		Address to the buffer holding IV
 *
 * @return
 *		 - XST_SUCCESS  On successful transfer
 *		 - XSECURE_AES_INVALID_PARAM  If Iv address is zero
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_AesIvXfer(const XSecure_Aes *InstancePtr, u64 IvAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U};

	/* Validate the input arguments */
	if (IvAddr == 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.SrcDataAddr = IvAddr;
	AesDmaCfg.IsLastChunkSrc = FALSE;
	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, &AesDmaCfg,
			XSECURE_SECURE_GCM_TAG_SIZE);
	if (InstancePtr->DmaSwapEn == XSECURE_DISABLE_BYTE_SWAP) {
		XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function configures the PMC DMA channels
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	AesDmaCfg	DMA SRC and DEST channel configuration
 * @param	Size		Size of data in bytes.
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_AesPmcDmaCfgAndXfer(const XSecure_Aes *InstancePtr,
	XSecure_AesDmaCfg *AesDmaCfg, u32 Size)
{
	int Status = XST_FAILURE;

	/* Configure the SSS for AES. */
	Status = XSecure_CfgSssAes(InstancePtr->PmcDmaPtr, &InstancePtr->SssInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesPlatPmcDmaCfgAndXfer(InstancePtr->PmcDmaPtr, AesDmaCfg, Size, InstancePtr->BaseAddress);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function enables or disables the GMAC configuration
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	IsGmacEn	User choice to enable/disable GMAC
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration
 *		 - XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  On state mismatch
 *
 * @note
 *	To generate GMAC on AAD data, this API must be called with IsGmacEn
 *	set to TRUE just before the last AAD update. After the last AAD update
 *	XSecure_AesEncryptFinal or XSecure_AesDecryptFinal must be called to
 *	generate or validate the GMAC tag.
 *
 ******************************************************************************/
int XSecure_AesGmacCfg(XSecure_Aes *InstancePtr, u32 IsGmacEn)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((IsGmacEn != TRUE) && (IsGmacEn != FALSE)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XSECURE_AES_OPERATION_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Enable/disable GMAC configuration */
	InstancePtr->IsGmacEn = IsGmacEn;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the AES engine with
 * 		provided input and output addresses.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	InDataAddr	Address of the input buffer.
 * @param	OutDataAddr	Address of output buffer.
 * @param	Size		Size of data to be updated by AES engine.
 * @param	IsLastChunk	If this is the last update of data to be processed,
 *				this parameter should be set to TRUE otherwise FALSE
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecureAesUpdate(const XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U, 0U, 0U, 0U, 0U, 0U};

	AesDmaCfg.SrcDataAddr = InDataAddr;
	AesDmaCfg.DestDataAddr = OutDataAddr;
	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.DestChannelCfg = TRUE;
	AesDmaCfg.IsLastChunkSrc = IsLastChunk;

	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, &AesDmaCfg, Size);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	if((InstancePtr->OperationId == XSECURE_ENCRYPT) && (IsLastChunk == TRUE)){
		Status = XSecure_AesCopyGcmTag(InstancePtr, &AesDmaCfg);
	}

#ifndef VERSAL_2VE_2VM
	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
#endif
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the AES engine with
 *		Additional Authenticated Data(AAD) and validate
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	AadAddr		Address of the additional authenticated data
 * @param	AadSize		Size of additional authenticated data in bytes,
 *				whereas number of bytes provided should be
 *				multiples of 4
 * @param	GcmTagAddr	GCM tag of the additional authenticated data
 *
 * @return
 *		- XST_SUCCESS  AAD updation and validation is success
 *		- XSECURE_AES_INVALID_PARAM  On invalid parameter
 *		- XSECURE_AES_STATE_MISMATCH_ERROR  On state mismatch
 *		- XSECURE_AES_GCM_TAG_MISMATCH  User provided GCM tag does not
 *			match calculated tag
 *		- XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesUpdateAadAndValidate(XSecure_Aes *InstancePtr, u64 AadAddr,
	u32 AadSize, u64 GcmTagAddr)
{
	volatile int Status = (u32)XST_FAILURE;
	XSecure_AesDmaCfg AesDmaCfg = {0U};
        volatile u32 GcmStatus = 0U;
        volatile u32 GcmStatusTmp = 0U;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (AadAddr == 0x00U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	/* Validate Aad Size for versal*/
	if ((AadSize % XSECURE_QWORD_SIZE) != 0x00U) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	/* Validate AES state */
	if (InstancePtr->AesState != XSECURE_AES_OPERATION_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Enable AAD */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_AAD_OFFSET,
		XSECURE_AES_AAD_ENABLE);

	AesDmaCfg.SrcDataAddr = AadAddr;
	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.IsLastChunkSrc = TRUE;

	/* Push the AAD to AES engine */
	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, &AesDmaCfg, AadSize);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	/* Disable AAD */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_AAD_OFFSET,
		XSECURE_AES_AAD_DISABLE);

	AesDmaCfg.SrcDataAddr = GcmTagAddr;
	AesDmaCfg.SrcChannelCfg = TRUE;
	AesDmaCfg.IsLastChunkSrc = FALSE;
	/* Push the GCM TAG to AES engine */
	Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, &AesDmaCfg,
		XSECURE_SECURE_GCM_TAG_SIZE);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = XST_FAILURE;
	/* Check Gcm Tag matching status */
	GcmStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET);
	GcmStatusTmp = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET);
	GcmStatus &= XSECURE_AES_STATUS_GCM_TAG_PASS_MASK;
	GcmStatusTmp &= XSECURE_AES_STATUS_GCM_TAG_PASS_MASK;

	if ((GcmStatus != XSECURE_AES_STATUS_GCM_TAG_PASS_MASK) ||
	   (GcmStatusTmp != XSECURE_AES_STATUS_GCM_TAG_PASS_MASK)) {
		Status = (int)XSECURE_AES_GCM_TAG_MISMATCH;
		goto END_RST;
	}

	Status = (u32)XST_SUCCESS;

END_RST:
	InstancePtr->AesState = XSECURE_AES_INITIALIZED;
	/* Soft Reset Aes*/
	XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);

END:
    return Status;
}


/*****************************************************************************/
/**
 * @brief	This function validates and updates the mode of the AES and loads
 *           key and transfers IV to the AES engine.
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	KeySrc		- Key Source for decryption of the data
 * @param	KeySize		- Size of the AES key to be used for decryption is
 *		 		- XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *				- XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		- Address to the buffer holding IV
 * @param   Mode        -
 *
 * @return
 *	-	XST_SUCCESS - On successful init
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *	-	XST_FAILURE - On failure to configure switch
 *
 ******************************************************************************/
static int XSecure_AesOpInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	XSecure_AesKeySize KeySize, u64 IvAddr, u32 Mode)
{
	volatile int Status = XST_FAILURE;
	volatile u32 KeyZeroedStatus = XSECURE_PUF_KEY_ZEROED_MASK;
	u32 IsKeySrcAllowed = FALSE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (KeySrc >= XSECURE_MAX_KEY_SOURCES) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if ((XSECURE_AES_KEY_SIZE_128 != KeySize) &&
		 (XSECURE_AES_KEY_SIZE_256 != KeySize)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}


	if (InstancePtr->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	KeyZeroedStatus = XSecure_ReadReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_ZEROED_STATUS_OFFSET);
	if ((KeySrc == XSECURE_AES_PUF_KEY) &&
		((KeyZeroedStatus & XSECURE_PUF_KEY_ZEROED_MASK) == XSECURE_PUF_KEY_ZEROED_MASK)) {
		Status = (int)XSECURE_AES_ZERO_PUF_KEY_NOT_ALLOWED;
		goto END;
	}

	if (Mode == XSECURE_AES_MODE_DEC) {
		IsKeySrcAllowed = AesKeyLookupTbl[KeySrc].DecAllowed;
	}
	else if (Mode == XSECURE_AES_MODE_ENC) {
		IsKeySrcAllowed = AesKeyLookupTbl[KeySrc].EncAllowed;
	}
	else {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	/* Key selected does not allow decryption or encryption */
	if (IsKeySrcAllowed == FALSE) {
		Status = XST_FAILURE;
		goto END;
	}

	if(InstancePtr->NextBlkLen == 0U) {
		XSecure_ReleaseReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

	/* Configure AES for decryption/encryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_MODE_OFFSET, Mode);

	Status = XSecure_AesKeyLoadandIvXfer(InstancePtr, KeySrc, KeySize, IvAddr);
	if (Status != XST_SUCCESS) {
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
		goto END;
	}
	InstancePtr->AesState = XSECURE_AES_OPERATION_INITIALIZED;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to update the data to AES engine and stores the
 *          resultant data at specified address
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *				  encrypted
 * @param	OutDataAddr	Address of output buffer where the encrypted data
 *				  to be updated
 * @param	Size    Size of data to be decrypted in bytes, whereas number of bytes shall be aligned as below
 *                  - 16 byte aligned when it is not the last chunk
 *                  - 4 byte aligned when the data is the last chunk
 * @param	IsLastChunk	 If this is the last update of data to be encrypted,
 *		 		  this parameter should be set to TRUE otherwise FALSE
 *
 * @return
 *	-	XST_SUCCESS - On successful encryption of the data
 *	-	XSECURE_AES_INVALID_PARAM - On invalid parameter
 *	-	XSECURE_AES_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_ValidateAndUpdateData(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	if (((IsLastChunk != TRUE) && (IsLastChunk != FALSE))) {
		Status = (int)XSECURE_AES_UNALIGNED_SIZE_ERROR;
		goto END;
	}
	/* Validate the size for last chunk */
	Status = XSecure_AesValidateSize(Size, IsLastChunk);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Validate the AES state */
	if ((InstancePtr->AesState != XSECURE_AES_OPERATION_INITIALIZED) &&
		(InstancePtr->AesState != XSECURE_AES_UPDATE_IN_PROGRESS)) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Enable AES Data swap */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_ENABLE_BYTE_SWAP);


	Status = XSecureAesUpdate(InstancePtr, InDataAddr, OutDataAddr, Size, IsLastChunk);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	if (IsLastChunk == TRUE) {
		InstancePtr->AesState = XSECURE_AES_UPDATE_DONE;
	}
	else {
		InstancePtr->AesState = XSECURE_AES_UPDATE_IN_PROGRESS;
	}

	if ((XSecure_AesIsEcbModeEn(InstancePtr) == TRUE) && (IsLastChunk == TRUE)) {
		/* Wait for AES Done for last chunk in ECB mode */
		Status = XSecure_AesWaitForDone(InstancePtr);
	}

END_RST:
	if (Status != XST_SUCCESS) {
		/*
		 * Issue a soft to reset to AES engine and
		 * set the AES state back to initialization state
		 */
		InstancePtr->NextBlkLen = 0U;
		InstancePtr->AesState = XSECURE_AES_INITIALIZED;
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used upon Last encrypt update. AES engine also sends GCM tag
 *          along with encrypted data,copy the GCM tag to local buffer to properly flush
 *          the DMA to avoid data corruption,if the same DMA is used for different operation
 *          before AES encrypt final.This is also used to store the generated GCM Tag during
 *          AES update AAD in GMAC mode.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 * @param	AesDmaCfg	DMA SRC and DEST channel configuration
 * @param	IsLastChunk	If this is the last update of data to be encrypted,
 *				  this parameter should be set to TRUE otherwise FALSE
 *
 * @return
 *	-	XST_SUCCESS - On successful copy of the GCM tag
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_AesCopyGcmTag(const XSecure_Aes *InstancePtr,
	XSecure_AesDmaCfg* AesDmaCfg)
{
	int Status = XST_FAILURE;

	if (XSecure_AesIsEcbModeEn(InstancePtr) != TRUE) {
		AesDmaCfg->DestDataAddr = (u64)(UINTPTR)InstancePtr->GcmTag;
		AesDmaCfg->DestChannelCfg = TRUE;
		AesDmaCfg->SrcChannelCfg = FALSE;

#if(!defined(VERSAL_PLM) && !(defined(__MICROBLAZE__)))
		/* Invalidate Cache before and after dma transfer to ensure cache coherency for a72 and r5 processors */
		Xil_DCacheInvalidateRange((UINTPTR)InstancePtr->GcmTag, XSECURE_SECURE_GCM_TAG_SIZE);
#endif
		Status = XSecure_AesPmcDmaCfgAndXfer(InstancePtr, AesDmaCfg,
			XSECURE_SECURE_GCM_TAG_SIZE);
		if(Status != XST_SUCCESS) {
			goto END;
		}

#if(!defined(VERSAL_PLM) && !(defined(__MICROBLAZE__)))
		Xil_DCacheInvalidateRange((UINTPTR)InstancePtr->GcmTag, XSECURE_SECURE_GCM_TAG_SIZE);
#endif
		/* Wait for AES Operation completion. */
		Status = XSecure_AesWaitForDone(InstancePtr);

	}
END:
	return Status;
}

/** @} */
