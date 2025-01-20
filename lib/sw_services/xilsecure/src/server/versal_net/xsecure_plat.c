/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat.c
*
* This file contains Versal Net specific code for Xilsecure server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.0   bm      07/06/22 Initial release
* 5.2   yog     07/10/23 Added support of unaligned data sizes for Versal Net
*       kpt     07/09/23 Added XSecure_GetRandomNum function
*       yog     08/07/23 Moved functions from xsecure_trng.c to xsecure_plat.c
*       kpt     08/29/23 Added volatile keyword to avoid compiler optimization
* 5.3   kpt     12/14/23 Place TRNG in reset when there is a failure
        kpt     01/09/24 Updated option for non-blocking trng reseed
* 5.4   yog     04/29/24 Fixed doxygen warnings.
*       kpt     06/30/24 Added XSecure_MemCpyAndChangeEndianness
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in Xilsecure server
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_sha_hw.h"
#include "xsecure_sss.h"
#include "xsecure_sha.h"
#include "xsecure_plat_kat.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/

#define XSECURE_AES_ADDRESS			  (0xF11E0000U) /**< AES BaseAddress */
#define XSECURE_SHA_ADDRESS			  (0xF1210000U) /**< SHA BaseAddress */
#define XSECURE_RSA_ECDSA_RSA_ADDRESS (0xF1200000U) /**< RSA ECDSA BaseAddress */

/************************** Variable Definitions *****************************/

/* XSecure_SssLookupTable[Input source][Resource] */
const u8 XSecure_SssLookupTable[XSECURE_SSS_MAX_SRCS][XSECURE_SSS_MAX_SRCS] = {
	/*+----+-----+-----+-----+-----+-----+-----+--------+
	*|DMA0| DMA1| PTPI| AES | SHA3_0 | SBI | SHA3_1 |Invalid |
	*+----+-----+-----+-----+-----+-----+-----+--------+
	* 0x00 = INVALID value
	*/
	{0x0DU, 0x00U, 0x00U, 0x06U, 0x00U, 0x0BU, 0x03U, 0x00U}, /* DMA0 */
	{0x00U, 0x09U, 0x00U, 0x07U, 0x00U, 0x0EU, 0x04U, 0x00U}, /* DMA1 */
	{0x0DU, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* PTPI */
	{0x0EU, 0x05U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* AES  */
	{0x0CU, 0x07U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SHA3_0 */
	{0x05U, 0x0BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SBI  */
	{0x0AU, 0x0FU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SHA3_1 */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* Invalid */
};

/*
* The configuration table for devices
*/
const XSecure_ShaConfig ShaConfigTable[XSECURE_SHA_NUM_OF_INSTANCES] =
{
	{
		XSECURE_SSS_SHA3_0,
		XSECURE_SHA3_0_BASE_ADDRESS,
		XSECURE_SHA3_0_DEVICE_ID,
	},
	{
		XSECURE_SSS_SHA3_1,
		XSECURE_SHA3_1_BASE_ADDRESS,
		XSECURE_SHA3_1_DEVICE_ID,
	}
};

/************************** Function Prototypes ******************************/

static void XSecure_UpdateEcdsaCryptoStatus(u32 Op);
static int XSecure_AesPmcDmaByteXfer(XPmcDma *PmcDmaPtr,
       const XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress);
static int XSecure_AesPmcDmaWordXfer(XPmcDma *PmcDmaPtr,
	const XSecure_AesDmaCfg *AesDmaCfg, u32 Size);
static void XSecure_AesDataEndiannessChange(u64 Address, u32 Size);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function masks the secure stream switch value
 *
 * @param	InputSrc	Input source to be selected for the resource
 * @param	OutputSrc	Output source to be selected for the resource
 * @param	Value		Register Value of SSS cfg register
 *
 * @return
 *		 - Mask  Mask value of corresponding InputSrc and OutputSrc
 *
 * @note	InputSrc, OutputSrc are of type XSecure_SssSrc
 *
 *****************************************************************************/
 u32 XSecure_SssMask(XSecure_SssSrc InputSrc, XSecure_SssSrc OutputSrc,
							u32 Value)
{
	u32 Mask = 0U;
	u32 RegVal = Value;

	if ((InputSrc == XSECURE_SSS_DMA0) || (OutputSrc == XSECURE_SSS_DMA0)) {
		if ((RegVal & XSECURE_SSS_SBI_MASK) == XSECURE_SSS_SBI_DMA0_VAL) {
			Mask |= XSECURE_SSS_SBI_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_0_MASK) == XSECURE_SSS_SHA3_0_DMA0_VAL) {
			Mask |= XSECURE_SSS_SHA3_0_MASK;
		}
		if ((RegVal & XSECURE_SSS_AES_MASK) == XSECURE_SSS_AES_DMA0_VAL) {
			Mask |= XSECURE_SSS_AES_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_1_MASK) == XSECURE_SSS_SHA3_1_DMA0_VAL) {
			Mask |= XSECURE_SSS_SHA3_1_MASK;
		}
		if ((RegVal & XSECURE_SSS_DMA0_MASK) != 0U) {
			Mask |= XSECURE_SSS_DMA0_MASK;
		}
	}
	if ((InputSrc == XSECURE_SSS_DMA1) || (OutputSrc == XSECURE_SSS_DMA1)) {
		if ((RegVal & XSECURE_SSS_SBI_MASK) == XSECURE_SSS_SBI_DMA1_VAL) {
			Mask |= XSECURE_SSS_SBI_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_0_MASK) == XSECURE_SSS_SHA3_0_DMA1_VAL) {
			Mask |= XSECURE_SSS_SHA3_0_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_1_MASK) == XSECURE_SSS_SHA3_1_DMA1_VAL) {
			Mask |= XSECURE_SSS_SHA3_1_MASK;
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

/*****************************************************************************/
/**
 * @brief	This function updates TRNG crypto indicator
 *
 * @param	Op	To set or clear the bit
 *
 *****************************************************************************/
void XSecure_UpdateTrngCryptoStatus(u32 Op)
{
#ifdef VERSALNET_PLM
	XPlmi_UpdateCryptoStatus(XPLMI_SECURE_TRNG_MASK, (XPLMI_SECURE_TRNG_MASK & ~Op));
#else
	(void)Op;
#endif
}

/*****************************************************************************/
/**
 * @brief	This function updates RSA crypto indicator
 *
 *****************************************************************************/
void XSecure_SetRsaCryptoStatus()
{
#ifdef VERSALNET_PLM
	XPlmi_UpdateCryptoStatus(XPLMI_SECURE_RSA_MASK, XPLMI_SECURE_RSA_MASK);
#endif
}

/*****************************************************************************/
/**
 * @brief	This function updates the crypto indicator bit of AES, SHA and ECC
 *
 * @param	BaseAddress	Base address of the core
 * @param	Op		To set or clear the bit
 *
 *****************************************************************************/
void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op)
{
#ifdef VERSALNET_PLM
	if (BaseAddress == XSECURE_AES_ADDRESS) {
		XPlmi_UpdateCryptoStatus(XPLMI_SECURE_AES_MASK, (XPLMI_SECURE_AES_MASK & ~Op));
	}
	else if (BaseAddress == XSECURE_RSA_ECDSA_RSA_ADDRESS) {
		XSecure_UpdateEcdsaCryptoStatus(Op);
	}
	else if (BaseAddress == XSECURE_SHA_ADDRESS) {
		XPlmi_UpdateCryptoStatus(XPLMI_SECURE_SHA3_384_MASK, (XPLMI_SECURE_SHA3_384_MASK & ~Op));
	}
	else {
		/* Do Nothing */
	}
#else
	(void)BaseAddress;
	(void)Op;
#endif
}

/*****************************************************************************/
/**
 * @brief	This function updates ECC crypto indicator
 *
 * @param	Op	To set or clear the bit
 *
 *****************************************************************************/
static void XSecure_UpdateEcdsaCryptoStatus(u32 Op)
{
#ifdef VERSALNET_PLM
	u32 RsaInUseFlag = 0U;


	if (Op == XSECURE_SET_BIT) {
		RsaInUseFlag = XPlmi_GetCryptoStatus(XPLMI_SECURE_RSA_MASK);
		if (RsaInUseFlag == 0U) {
			XPlmi_UpdateCryptoStatus(XPLMI_SECURE_ECDSA_MASK, XPLMI_SECURE_ECDSA_MASK);
		}
	}
	else {
		/* Clear both RSA and ECDSA bits */
		XPlmi_UpdateCryptoStatus(XPLMI_SECURE_ECDSA_MASK, 0U);
		XPlmi_UpdateCryptoStatus(XPLMI_SECURE_RSA_MASK, 0U);
	}
#else
	(void)Op;
#endif
}

/*****************************************************************************/
/**
 * @brief	This function validates whether all the intermediate updates to
 * 		AES engine are Q-WORD aligned or not.
 *
 * @param	Size		Size of data in bytes.
 * @param	IsLastChunk	Last chunk indication
 *
 * @return
 *		XST_SUCCESS If the data is q-word aligned for intermediate updates
 *		XST_FAILURE If the data is not q-word aligned for intermediate updates
 *
 ******************************************************************************/
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;

	/**
	  * AES engine expect all intermediate updates shall be
	  * 16-byte aligned when it is not last chunk of data.
	  * Throw an error if it is not 16 byte aligned.
	  */
	if ((IsLastChunk != TRUE) &&
		((Size % XSECURE_QWORD_SIZE) != 0x00U)) {
		Status = (int)XSECURE_AES_UNALIGNED_SIZE_ERROR;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sets the SRC and DEST channel endianness
 * 		configurations of PMC DMA and transfers data.
 *
 * @param	PmcDmaPtr	Pointer to the XPmcDma instance.
 * @param	AesDmaCfg	DMA SRC and DEST channel configuration
 * @param	Size		Size of data in bytes.
 * @param	BaseAddress	AES BaseAddress
 *
 * @return
 * 		 - XST_SUCCESS  On successful configuration
 * 		 - XSECURE_AES_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress)
{
	int Status = XST_FAILURE;
	u32 ExtraBytes;
	u32 LastChunkSrc;
	u32 AesMode;
	static u32 NonQWordDataAligned = 0U;

	/* Validate input parameters*/
	if ((PmcDmaPtr == NULL) || (AesDmaCfg == NULL) || (Size == 0U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}

	AesMode = XSecure_ReadReg(BaseAddress, XSECURE_AES_MODE_OFFSET);
	LastChunkSrc = AesDmaCfg->IsLastChunkSrc;
	/* Transfers QWord aligned data */
	if (Size >= XSECURE_QWORD_SIZE) {
		/* Set the src lastchunk to 0 if size of data is non-qword aligned */
		if ((Size % XSECURE_QWORD_SIZE) != 0U) {
			AesDmaCfg->IsLastChunkSrc = 0U;
		}
		Status = XSecure_AesPmcDmaWordXfer(PmcDmaPtr, AesDmaCfg, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	/* Transfers remaining extra bytes when size of the data(AAD/Paylod) is non-qword aligned*/
	if ((Size % XSECURE_QWORD_SIZE) != 0U) {
		AesDmaCfg->IsLastChunkSrc = (u8)LastChunkSrc;
		Status = XSecure_AesPmcDmaByteXfer(PmcDmaPtr, AesDmaCfg, Size, BaseAddress);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		/* Wait for the DEST DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(PmcDmaPtr, XPMCDMA_DST_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Acknowledge the transfer has completed */
		XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_DST_CHANNEL, XPMCDMA_IXR_DONE_MASK);

		/* Change the endianness of extra bytes if data(AAD/Payload) is non-qword aligned*/
		if ((Size % XSECURE_QWORD_SIZE) != 0U) {
			/*
			 * If payload is non-qword aligned, GCM Tag will be generated with changed endianness.
			 * So setting the flag to TRUE when payload is non-qword aligned
			 * so that we can change the endianness when GCM Tag is received
			 */
			if ((AesDmaCfg->DestChannelCfg == TRUE) && (AesDmaCfg->SrcChannelCfg == TRUE)
					&& (AesMode == XSECURE_AES_MODE_ENC)) {
				NonQWordDataAligned = TRUE;
			}
			ExtraBytes = Size % XSECURE_QWORD_SIZE;
			XSecure_AesDataEndiannessChange((AesDmaCfg->DestDataAddr +
				(u64)(UINTPTR)(Size - ExtraBytes)), ExtraBytes);
		}
		/* If Payload size is non-qword aligned */
		if (NonQWordDataAligned == (u32)TRUE) {
			/* Change the endianness for GCM Tag*/
			if ((AesDmaCfg->DestChannelCfg == TRUE) && (AesDmaCfg->SrcChannelCfg == FALSE)) {
				XSecure_AesDataEndiannessChange(AesDmaCfg->DestDataAddr, Size);
				NonQWordDataAligned = (u32)FALSE;
			}
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sets the SRC and DEST channel endianness configurations of PMC DMA
 * 		and transfers qword aligned data
 *
 * @param	PmcDmaPtr	Pointer to the XPmcDma instance.
 * @param	AesDmaCfg	DMA SRC and DEST channel configuration
 * @param	Size		Size of data in bytes.
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_AesPmcDmaWordXfer(XPmcDma *PmcDmaPtr,
	const XSecure_AesDmaCfg *AesDmaCfg, u32 Size)
{
	int Status = XST_FAILURE;
	/* Enable PMC DMA Src and Dst channels for byte swapping */
	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		XSecure_AesPmcDmaCfgEndianness(PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
	}

	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
			((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XSecure_AesPmcDmaCfgEndianness(PmcDmaPtr,
			XPMCDMA_DST_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
	}

	/* Transfer the data for src and dest channels as per configuration */
	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XPmcDma_64BitTransfer(PmcDmaPtr, XPMCDMA_DST_CHANNEL,
			(u32)AesDmaCfg->DestDataAddr, (u32)(AesDmaCfg->DestDataAddr >> 32U),
			((Size / XSECURE_QWORD_SIZE) * XSECURE_WORD_SIZE), AesDmaCfg->IsLastChunkDest);
	}

	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		XPmcDma_64BitTransfer(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
			(u32)AesDmaCfg->SrcDataAddr, (u32)(AesDmaCfg->SrcDataAddr >> 32U),
			((Size / XSECURE_QWORD_SIZE) * XSECURE_WORD_SIZE), AesDmaCfg->IsLastChunkSrc);
	}

	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		/* Wait for the SRC DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Acknowledge the transfer has completed */
		XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
			XPMCDMA_IXR_DONE_MASK);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function changes the endianness of particular size
 *
 * @param	Address	Endianness change data stored address
 * @param	Size	Size of the data
 *
 *
 ******************************************************************************/
static void XSecure_AesDataEndiannessChange(u64 Address, u32 Size)
{
	u32 Index;
	u32 RevIndex = Size;
	u8 EndianessChange[XSECURE_QWORD_SIZE];

	XSecure_MemCpy64((u64)(UINTPTR)EndianessChange, Address, Size);
	for (Index = 0; Index < Size; Index++) {
		XSecure_OutByte64((u64)(UINTPTR)(Address + Index),
			EndianessChange[RevIndex - 1U]);
		RevIndex--;
	}
}

/*****************************************************************************/
/**
 *
 * @brief	This function sets the SRC and DEST channel endianness configurations of PMC DMA
 * 		and transfers non-qword aligned data
 *
 * @param	PmcDmaPtr	Pointer to the XPmcDma instance.
 * @param	AesDmaCfg	DMA SRC and DEST channel configuration
 * @param	Size		Size of data in bytes.
 * @param	BaseAddress	Aes BaseAddress
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_AesPmcDmaByteXfer(XPmcDma *PmcDmaPtr,
	const XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress)
{
	int Status = XST_FAILURE;
	u8 EndianessChange[XSECURE_QWORD_SIZE];
	u32 RevIndex;
	u32 ExtraBytes;
	u32 Index;

	/*
	 * Disable PMC DMA Src and Dst channels for byte swapping and
	 * AES data_swap for transferring extra bytes of non-qword aligned data
	 */
	XSecure_WriteReg(BaseAddress,
				XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_DISABLE_BYTE_SWAP);
	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		XSecure_AesPmcDmaCfgEndianness(PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	}

	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XSecure_AesPmcDmaCfgEndianness(PmcDmaPtr,
			XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	}

	/*
	 * As we disabled AES data_swap and DMA endianness,
	 * change the endianness of the extra bytes
	 */
	ExtraBytes = Size % XSECURE_QWORD_SIZE;
	RevIndex = ExtraBytes;
	for (Index = 0; Index < ExtraBytes; Index++) {
		EndianessChange[Index] = XSecure_InByte64((AesDmaCfg->SrcDataAddr +
					(u64)(UINTPTR)(Size - ExtraBytes) + (RevIndex-1U)));
		RevIndex--;
	}

	/* Transfer the data for src and dest channels as per configuration */
	if ((AesDmaCfg->DestChannelCfg == TRUE)
			&& ((u32) AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XCsuDma_ByteAlignedTransfer(PmcDmaPtr,
				XPMCDMA_DST_CHANNEL, (u64)(UINTPTR)(AesDmaCfg->DestDataAddr +
				(u64)(UINTPTR)(Size - ExtraBytes)), ExtraBytes, AesDmaCfg->IsLastChunkDest);
	}

	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		XCsuDma_ByteAlignedTransfer(PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, (u64)(UINTPTR)EndianessChange, ExtraBytes,
				AesDmaCfg->IsLastChunkSrc);
	}

	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		/* Wait for the SRC DMA completion.*/
		Status = XPmcDma_WaitForDoneTimeout(PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/*Acknowledge the transfer has completed*/
		XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
			XPMCDMA_IXR_DONE_MASK);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This is a helper function to enable/disable byte swapping feature
 * 		of PMC DMA
 *
 * @param	InstancePtr	Pointer to the XPmcDma instance
 * @param	Channel		Channel type
 *				- XPMCDMA_SRC_CHANNEL
 *				- XPMCDMA_DST_CHANNEL
 * @param	EndianType	Endianness type
 *				- 1 : Enable Byte Swapping
 *				- 0 : Disable Byte Swapping
 *
 ******************************************************************************/
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel, u8 EndianType)
{
	XPmcDma_Configure ConfigValues = {0U};

	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	/* Updates the XPmcDma_Configure structure with PmcDma's channel values */
	XPmcDma_GetConfig(InstancePtr, Channel, &ConfigValues);
	ConfigValues.EndianType = EndianType;
	/* Updates the PmcDma's channel with XPmcDma_Configure structure values */
	XPmcDma_SetConfig(InstancePtr, Channel, &ConfigValues);
}

/*****************************************************************************/
/**
 * @brief	This function generates Random number of given size
 *
 * @param	Output	is pointer to the output buffer
 * @param	Size	is the number of random bytes to be read
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_FAILURE  On Failure
 *
 *****************************************************************************/
int XSecure_GetRandomNum(u8 *Output, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u8 *RandBufPtr = Output;
	u32 TotalSize = Size;
	u32 RandBufSize = XTRNGPSX_SEC_STRENGTH_IN_BYTES;
	u32 Index = 0U;
	u32 NoOfGenerates = (Size + XTRNGPSX_SEC_STRENGTH_IN_BYTES - 1U) >> 5U;
	XTrngpsx_Instance *TrngInstance = XSecure_GetTrngInstance();

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_ECCRandInit);

	for (Index = 0U; Index < NoOfGenerates; Index++) {
		if (Index == (NoOfGenerates - 1U)) {
			RandBufSize = TotalSize;
		}

		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_Generate, TrngInstance,
					RandBufPtr, RandBufSize, FALSE);
		RandBufPtr += RandBufSize;
		TotalSize -= RandBufSize;
	}

END:
	if (Status != XST_SUCCESS) {
		Status |= XTrngpsx_Uninstantiate(TrngInstance);
		XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the trng in HRNG mode if it is not initialized
 *		and it is applicable only for VersalNet
 *
 * @return
 *		 - XST_SUCCESS  On Successful initialization
 *		 - XST_FAILURE  On Failure
 *
 *****************************************************************************/
int XSecure_ECCRandInit(void)
{
	volatile int Status = XST_FAILURE;
	XTrngpsx_Instance *TrngInstance = XSecure_GetTrngInstance();

	if ((XPlmi_IsKatRan(XPLMI_SECURE_TRNG_KAT_MASK) != TRUE) ||
		(TrngInstance->ErrorState != XTRNGPSX_HEALTHY)) {
		Status = XTrngpsx_PreOperationalSelfTests(TrngInstance);
		if (Status != XST_SUCCESS) {
			XPlmi_ClearKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
			goto END;
		}
		else {
			XPlmi_SetKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
		}
	}
	if ((TrngInstance->UserCfg.Mode != XTRNGPSX_HRNG_MODE) ||
		(TrngInstance->State == XTRNGPSX_UNINITIALIZED_STATE )) {
		Status = XSecure_TrngInitNCfgHrngMode();
		if(Status != XST_SUCCESS) {
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function can copy the content of memory from 64-bit address
 *		to 32-bit address and change endianness of destination data
 *
 * @param	DestAddress	is the address of the destination where content of
 *				SrcAddr memory should be copied.
 * @param	SrcAddress	is the address of the source where copy should
 *				start from.
 * @param	Length		is size of memory to be copied in bytes.
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_FAILURE  On failure
 *
 *****************************************************************************/
int XSecure_MemCpyAndChangeEndianness(u64 DestAddress, u64 SrcAddress, u32 Length)
{
	volatile int Status = XST_FAILURE;
	u8 *Buf = (u8*)(UINTPTR)DestAddress;

	Status = XPlmi_MemCpy64(DestAddress, SrcAddress, Length);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SReverseData(Buf, Length);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initialize and configures the TRNG into HRNG mode of operation.
 *
 * @return
 *		 - XST_SUCCESS  Upon success.
 *		 - XST_FAILURE  On failure.
 *
 *****************************************************************************/
int XSecure_TrngInitNCfgHrngMode(void)
{
	volatile int Status = XST_FAILURE;
	XTrngpsx_UserConfig UsrCfg;
	XTrngpsx_Instance *TrngInstance = XSecure_GetTrngInstance();

	if (TrngInstance->State != XTRNGPSX_UNINITIALIZED_STATE ) {
		Status = XTrngpsx_Uninstantiate(TrngInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
	}
	/* Initiate TRNG */
	UsrCfg.Mode = XTRNGPSX_HRNG_MODE;
	UsrCfg.AdaptPropTestCutoff = XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF;
	UsrCfg.RepCountTestCutoff = XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF;
	UsrCfg.DFLength = XSECURE_TRNG_USER_CFG_DF_LENGTH ;
	UsrCfg.SeedLife = XSECURE_TRNG_USER_CFG_SEED_LIFE ;
	UsrCfg.IsBlocking = FALSE;
	Status = XTrngpsx_Instantiate(TrngInstance, NULL, 0U, NULL, &UsrCfg);
	if (Status != XST_SUCCESS) {
		(void)XTrngpsx_Uninstantiate(TrngInstance);
		XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
		goto END;
	}
	XSecure_UpdateTrngCryptoStatus(XSECURE_SET_BIT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the common trng instance
 *
 * @return
 *		 - Pointer to the XSecure_TrngInstance instance
 *
 *****************************************************************************/
XTrngpsx_Instance *XSecure_GetTrngInstance(void)
{
	static XTrngpsx_Instance TrngInstance = {0U};

	return &TrngInstance;
}
/** @} */
