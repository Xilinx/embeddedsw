/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/versal_2vp_p/xsecure_plat.c
*
* This file contains Versal_2vp_p specific code for Xilsecure server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date   Changes
* ----- ----- -------- ----------------------------------------------------------------------------
* 5.7   tvp   02/19/26 Initial release
*       tvp   02/23/26 Use XSecure_ShaPlatConfig, platform specific SHA configurations
*       tvp   02/23/26 Add SHAKE256 SLH-DSA Chaining algorithm support
*       rpu   04/22/26 Fix XSecure_GetRandomNum for non-word-aligned sizes
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in XilSecure server
* @{
*/
/**************************************** Include Files *******************************************/
#include "xsecure_sss.h"
#include "xsecure_sha.h"
#include "xsecure_aes.h"
#include "xsecure_init.h"
#include "xplmi.h"
#include "xsecure_defs.h"
#include "xil_sutil.h"

/************************************* Constant Definitions ***************************************/

/************************************* Variable Definitions ***************************************/

/* XSecure_SssLookupTable[Input source][Resource] */
const u8 XSecure_SssLookupTable[XSECURE_SSS_MAX_SRCS][XSECURE_SSS_MAX_SRCS] = {
	/*+----+------+------+------+------+-----+------+--------+
	*|DMA0| DMA1  | PTPI | AES  | SHA3 | SBI | SHA2 | Invalid|
	*+----+-------+------+------+------+-----+------+--------+
	* 0x00 = INVALID value
	*/
	{0x0DU, 0x00U, 0x00U, 0x06U, 0x00U, 0x0BU, 0x03U, 0x00U}, /* DMA0 */
	{0x00U, 0x09U, 0x00U, 0x07U, 0x00U, 0x0EU, 0x04U, 0x00U}, /* DMA1 */
	{0x0DU, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* PTPI */
	{0x0EU, 0x05U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* AES  */
	{0x0CU, 0x07U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SHA3 */
	{0x05U, 0x0BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SBI  */
	{0x0AU, 0x0FU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SHA2 */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* Invalid */
};

/* The configuration table for devices */
const XSecure_ShaConfig ShaConfigTable[XSECURE_SHA_NUM_OF_INSTANCES] =
{
	{
		XSECURE_SSS_SHA3,
		XSECURE_SHA3_BASE_ADDRESS,
		XSECURE_SHA3_DEVICE_ID,
	},
	{
		XSECURE_SSS_SHA2,
		XSECURE_SHA2_BASE_ADDRESS,
		XSECURE_SHA2_DEVICE_ID,
	}
};

/************************************* Function Definitions ***************************************/

/**************************************************************************************************/
/**
 * @brief	This function masks the secure stream switch value.
 *
 * @param	InputSrc	Input source to be selected for the resource.
 * @param	OutputSrc	Output source to be selected for the resource.
 * @param	Value		Register Value of SSS cfg register.
 *
 * @return
 *		- Mask Mask value of corresponding InputSrc and OutputSrc.
 *
 * @note	InputSrc, OutputSrc are of type XSecure_SssSrc.
 *
 **************************************************************************************************/
 u32 XSecure_SssMask(XSecure_SssSrc InputSrc, XSecure_SssSrc OutputSrc, u32 Value)
{
	u32 Mask = 0U;
	u32 RegVal = Value;

	if ((InputSrc == XSECURE_SSS_DMA0) || (OutputSrc == XSECURE_SSS_DMA0)) {
		if ((RegVal & XSECURE_SSS_SBI_MASK) == XSECURE_SSS_SBI_DMA0_VAL) {
			Mask |= XSECURE_SSS_SBI_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_MASK) == XSECURE_SSS_SHA3_DMA0_VAL) {
			Mask |= XSECURE_SSS_SHA3_MASK;
		}
		if ((RegVal & XSECURE_SSS_AES_MASK) == XSECURE_SSS_AES_DMA0_VAL) {
			Mask |= XSECURE_SSS_AES_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA2_MASK) == XSECURE_SSS_SHA2_DMA0_VAL) {
			Mask |= XSECURE_SSS_SHA2_MASK;
		}
		if ((RegVal & XSECURE_SSS_DMA0_MASK) != 0U) {
			Mask |= XSECURE_SSS_DMA0_MASK;
		}
	}
	if ((InputSrc == XSECURE_SSS_DMA1) || (OutputSrc == XSECURE_SSS_DMA1)) {
		if ((RegVal & XSECURE_SSS_SBI_MASK) == XSECURE_SSS_SBI_DMA1_VAL) {
			Mask |= XSECURE_SSS_SBI_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_MASK) == XSECURE_SSS_SHA3_DMA1_VAL) {
			Mask |= XSECURE_SSS_SHA3_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA2_MASK) == XSECURE_SSS_SHA2_DMA1_VAL) {
			Mask |= XSECURE_SSS_SHA2_MASK;
		}
		if ((RegVal & XSECURE_SSS_AES_MASK) == XSECURE_SSS_AES_DMA1_VAL) {
			Mask |= XSECURE_SSS_AES_MASK;
		}
		if ((RegVal & XSECURE_SSS_DMA1_MASK) != 0U) {
			Mask |= XSECURE_SSS_DMA1_MASK;
		}
	}

	return Mask;
}

/**************************************************************************************************/
/**
 * @brief	This function updates TRNG crypto indicator.
 *
 * @param	Op		To set or clear the bit.
 *
 **************************************************************************************************/
void XSecure_UpdateTrngCryptoStatus(u32 Op)
{
	(void)Op;
	/** - Not applicable for Versal_2vp_p */
}

/**************************************************************************************************/
/**
 * @brief	This function updates RSA crypto indicator.
 *
 **************************************************************************************************/
void XSecure_SetRsaCryptoStatus(void)
{
	/** - Not applicable for Versal_2vp_p */
}

/**************************************************************************************************/
/**
 * @brief	This function updates the crypto indicator bit of AES, SHA and ECC.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	Op		To set or clear the bit.
 *
 **************************************************************************************************/
void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op)
{
	(void)BaseAddress;
	(void)Op;
}

/**************************************************************************************************/
/**
 * @brief	This function configures DMA Byte Swap based on the user input.
 *
 * @param	Op	Enable/Disable byte swapping.
 *
 **************************************************************************************************/
void XSecure_ConfigureDmaByteSwap(u32 Op)
{
	XSecure_Aes *AesInstance = XSecure_GetAesInstance();

	AesInstance->DmaSwapEn = Op;
}

/**************************************************************************************************/
/**
 *
 * @brief	This function validates if intermediate updates are 16-bytes aligned or not.
 *
 * @param	Size		Size of data in bytes.
 * @param	IsLastChunk	Last chunk indication.
 *
 * @return
 *		- XST_SUCCESS If update is 16-bytes aligned.
 *		- XSECURE_AES_UNALIGNED_SIZE_ERROR If update is not 16-bytes aligned.
 *
 **************************************************************************************************/
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;

	/**
	 * - AES engine expect all intermediate updates shall be 16-byte aligned when it is not last
	 *   chunk of data. Throw an error if it is not 16 byte aligned.
	 */
	if ((IsLastChunk != TRUE) && ((Size % XSECURE_QWORD_SIZE) != 0x00U)) {
		Status = (int)XSECURE_AES_UNALIGNED_SIZE_ERROR;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************************************/
/**
 *
 * @brief	This function sets the SRC and DEST channel endianness configurations of PMC DMA and
 * 		transfers data.
 *
 * @param	PmcDmaPtr	Pointer to the XPmcDma instance.
 * @param	AesDmaCfg	DMA SRC and DEST channel configuration.
 * @param	Size		Size of data in bytes.
 * @param	BaseAddress	AES BaseAddress.
 *
 * @return
 *		 - XST_SUCCESS On successful configuration.
 * 		 - XSECURE_AES_INVALID_PARAM If any input parameter is invalid.
 *		 - XST_FAILURE On failure.
 *
 **************************************************************************************************/
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, XSecure_AesDmaCfg *AesDmaCfg, u32 Size,
				UINTPTR BaseAddress)
{
	int Status = XST_FAILURE;
	XSecure_Aes *InstancePtr = XSecure_GetAesInstance();
	(void)BaseAddress;

	if ((PmcDmaPtr == NULL) || (AesDmaCfg == NULL) || (Size == 0U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	/** - Enable PMC DMA Src and Dst channels for byte swapping. */
	if ((AesDmaCfg->SrcChannelCfg == TRUE) &&
		(InstancePtr->DmaSwapEn == XSECURE_DISABLE_BYTE_SWAP)) {
		XSecure_AesPmcDmaCfgEndianness(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
						XSECURE_ENABLE_BYTE_SWAP);
	}

	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
			(InstancePtr->DmaSwapEn == XSECURE_DISABLE_BYTE_SWAP) &&
			((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XSecure_AesPmcDmaCfgEndianness(PmcDmaPtr, XPMCDMA_DST_CHANNEL,
						XSECURE_ENABLE_BYTE_SWAP);
	}

	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XPmcDma_ByteAlignedTransfer(PmcDmaPtr,XPMCDMA_DST_CHANNEL, AesDmaCfg->DestDataAddr,
						Size, AesDmaCfg->IsLastChunkDest);
	}

	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		XPmcDma_ByteAlignedTransfer(PmcDmaPtr,XPMCDMA_SRC_CHANNEL, AesDmaCfg->SrcDataAddr,
						Size, AesDmaCfg->IsLastChunkSrc);
	}

	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		/** - Wait for the SRC DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(PmcDmaPtr, XPMCDMA_SRC_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - Acknowledge the transfer has completed */
		XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, XPMCDMA_IXR_DONE_MASK);
	}

	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		/** - Wait for the DEST DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(PmcDmaPtr, XPMCDMA_DST_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - Acknowledge the transfer has completed */
		XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_DST_CHANNEL, XPMCDMA_IXR_DONE_MASK);
	}

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This is a helper function to enable/disable byte swapping feature of PMC DMA.
 *
 * @param	InstancePtr	Pointer to the XPmcDma instance.
 * @param	Channel		Channel Type (XPMCDMA_SRC_CHANNEL or XPMCDMA_DST_CHANNEL)
 * @param	EndianType	1 to enable byte swapping, 0 to disable byte swapping
 *
 **************************************************************************************************/
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr, XPmcDma_Channel Channel, u8 EndianType)
{
	XPmcDma_Configure ConfigValues = {0U};

	/** - Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	/** - Updates the XPmcDma_Configure structure with PmcDma's channel values */
	XPmcDma_GetConfig(InstancePtr, Channel, &ConfigValues);
	ConfigValues.EndianType = EndianType;

	/** - Updates the PmcDma's channel with XPmcDma_Configure structure values */
	XPmcDma_SetConfig(InstancePtr, Channel, &ConfigValues);
}

/**************************************************************************************************/
/**
 * @brief	This function generates random number of given size.
 *
 * @param	Output	Pointer to the output buffer.
 * @param	Size	Number of random bytes to be read.
 *
 * @return
 *		 - XST_SUCCESS On Success.
 *  		 - XST_FAILURE On Failure.
 * 		 - XST_INVALID_PARAM - Invalid input parameter (size is 0 or output is NULL)
 *		 - XSECURE_ERR_GLITCH_DETECTED - Error when glitch is detected.
 *
 **************************************************************************************************/
int XSecure_GetRandomNum(u8 *Output, u32 Size)
{
	volatile int Status = XST_FAILURE;
	volatile int ZeroizeStatus = XST_FAILURE;
	u8 *RandBufPtr = NULL;
	u32 RandBufSize = XTRNGPSX_SEC_STRENGTH_IN_BYTES;
	volatile u32 Index = 0U;
	u32 TotalSize = 0U;
	u32 NoOfGenerates = 0U;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();
	u8 TmpRandBuf[XTRNGPSX_SEC_STRENGTH_IN_BYTES] = {0};

	/** - Validate input parameters */
	if ((Size == 0U) || (Output == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	TotalSize = Size;
	NoOfGenerates = XIL_SCEILDIV(u32, Size, XTRNGPSX_SEC_STRENGTH_IN_BYTES);
	RandBufPtr = Output;

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_ECCRandInit);

	for (Index = 0U; Index < NoOfGenerates; Index++) {
		if (Index == (NoOfGenerates - 1U)) {
			RandBufSize = TotalSize;
			if (RandBufSize < XTRNGPSX_SEC_STRENGTH_IN_BYTES) {
				XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_Generate, TrngInstance,
						       (u8 *)&TmpRandBuf,
						       XTRNGPSX_SEC_STRENGTH_IN_BYTES, FALSE);
				Status = XST_FAILURE;
				Status = Xil_SMemCpy(RandBufPtr, RandBufSize, TmpRandBuf,
						     XTRNGPSX_SEC_STRENGTH_IN_BYTES, RandBufSize);
				Index = NoOfGenerates;
				break;
			}
		}

		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_Generate, TrngInstance, RandBufPtr,
					RandBufSize, FALSE);
		RandBufPtr += RandBufSize;
		TotalSize -= RandBufSize;
	}

	/** - Verify loop index is within expected bounds */
	if (Index != NoOfGenerates) {
		Status = (int)XSECURE_ERR_GLITCH_DETECTED;
	}

END:
	/** - Zeroise temporary buffer */
	ZeroizeStatus = Xil_SecureZeroize(TmpRandBuf, XTRNGPSX_SEC_STRENGTH_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = ZeroizeStatus;
	}

	if (Status != XST_SUCCESS) {
		Status |= XSecure_Uninstantiate(TrngInstance);
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function initializes the trng in HRNG mode if it is not initialized.
 *
 * @return
 *		- XST_SUCCESS On Successful initialization.
 *      	- XST_FAILURE On Failure.
 *
 **************************************************************************************************/
int XSecure_ECCRandInit(void)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();

	if ((XPlmi_IsKatRan(XPLMI_SECURE_TRNG_KAT_MASK) != TRUE) ||
		(!XSecure_TrngIsHealthy(TrngInstance))) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_PreOperationalSelfTests, TrngInstance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			XPlmi_ClearKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
			Status = (int)XSECURE_ERR_IN_TRNG_SELF_TESTS;
			goto END;
		}

		XPlmi_SetKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
	}

	if (((XSecureTrng_Mode)TrngInstance->UserCfg.Mode != XSECURE_TRNG_HRNG_MODE) ||
		(XSecure_TrngIsUninitialized(TrngInstance))) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_TrngInitNCfgMode,
				XSECURE_TRNG_HRNG_MODE, NULL, 0, NULL);
		if((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = (int)XSECURE_ERR_TRNG_INIT_N_CONFIG;
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function validates the SHA Mode and initialize SHA instance.
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	ShaMode		SHA Mode.
 *
 * @return
 *		- XST_SUCCESS Upon Success.
 *		- XST_FAILURE Upon Failure.
 *		- XSECURE_SHA_INVALID_MODE_ERROR In case of invalid sha mode config.
 *
 **************************************************************************************************/
int XSecure_ShaValidateModeAndCfgInstance(XSecure_Sha * const InstancePtr, XSecure_ShaMode ShaMode)
{
	volatile int Status = XST_FAILURE;
	XSecure_ShaPlatConfig *ShaPlatConfig = (XSecure_ShaPlatConfig *)InstancePtr->ShaPlatConfig;

	/** - Initializes the SHA instance based on SHA Mode */
	switch(ShaMode) {
		/** - SHA2-256 Mode */
		case XSECURE_SHA2_256:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHA2_256_HASH_LEN;
			ShaPlatConfig->ShaMode = (u32)SHA256;
			break;
		/** - SHA2-384 Mode */
		case XSECURE_SHA2_384:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHA2_384_HASH_LEN;
			ShaPlatConfig->ShaMode = (u32)SHA384;
			break;
		/** - SHA2-512 Mode */
		case XSECURE_SHA2_512:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHA_512_HASH_LEN;
			ShaPlatConfig->ShaMode = (u32)SHA512;
			break;
		/** - SHA3-256 Mode */
		case XSECURE_SHA3_256:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHA3_256_HASH_LEN;
			ShaPlatConfig->ShaMode = (u32)SHA256;
			break;
		/** - SHA3-384 Mode */
		case XSECURE_SHA3_384:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHA3_384_HASH_LEN;
			ShaPlatConfig->ShaMode = (u32)SHA384;
			break;
		/** - SHAKE-512 Mode */
		case XSECURE_SHA3_512:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHA_512_HASH_LEN;
			ShaPlatConfig->ShaMode = (u32)SHA512;
			break;
		/** - SHAKE-256 Mode */
		case XSECURE_SHAKE_256:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHAKE_256_HASH_LEN;
			ShaPlatConfig->ShaMode = SHAKE256;
			break;
		/** - SHAKE-256 SLH-DSA chaining Mode */
		case XSECURE_SHAKE_256_SLH_DSA_CHAIN:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHAKE_256_HASH_LEN;
			ShaPlatConfig->ShaMode = SHAKE256_SLH_DSA_CHAIN;
			break;
		/** - SHA invalid mode */
		case XSECURE_SHA_INVALID_MODE:
		default:
			Status = (int)XSECURE_SHA_INVALID_PARAM;
			break;
	}

	if (Status == XSECURE_SHA_INVALID_PARAM) {
		goto END;
	} else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function transfer data to SHA engine from DMA
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	DataAddr	Input data address.
 * @param	Size		Input data size in bytes.
 * @param	IsLastUpdate	Last update flag.
 *
 * @return
 *		 - XST_SUCCESS Upon Success.
 *		 - XST_FAILURE Upon Failure.
 *
 **************************************************************************************************/
int XSecure_ShaDmaXfer(void *InstancePtr, u64 DataAddr, u32 Size, u8 IsLastUpdate)
{
	int Status = XST_FAILURE;
	XSecure_Sha *ShaInstancePtr = (XSecure_Sha *)InstancePtr;

	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	if ((IsLastUpdate != TRUE) && (IsLastUpdate != FALSE)) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	XPmcDma_ByteAlignedTransfer(ShaInstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL, DataAddr, (u32)Size,
					(u8)IsLastUpdate);

	/** - Wait for PMC DMA done bit to be set. */
	Status = XPmcDma_WaitForDoneTimeout(ShaInstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/** - Acknowledge the transfer has completed. */
	XPmcDma_IntrClear(ShaInstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL, XPMCDMA_IXR_DONE_MASK);

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function can copy the content of memory from 64-bit address to 32-bit address
 * 		and change endianness of destination data.
 *
 * @param	DestAddress	The address of the destination where content of SrcAddr memory
 * 				should be copied.
 * @param	SrcAddress	The address of the source where copy should start from.
 * @param	Length		Size of memory to be copied in bytes.
 *
 * @return
 *		 - XST_SUCCESS On success and error code on failure.
 *		 - XST_FAILURE In case of any failure.
 *
 **************************************************************************************************/
int XSecure_MemCpyAndChangeEndianness(u64 DestAddress, u64 SrcAddress, u32 Length)
{
	volatile int Status = XST_FAILURE;
	u8 *Buf = (u8*)(UINTPTR)DestAddress;

	Status = XPlmi_MemCpy64(DestAddress, SrcAddress, Length);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = Xil_SReverseData(Buf, Length);

END:
	return Status;
}
/** @} */
