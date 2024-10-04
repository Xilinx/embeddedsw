/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat.c
* This file contains versal specific code for xilsecure server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   bm   07/06/22 Initial release
* 5.2   yog  07/10/23 Added support of unaligned data sizes for Versal Net
*	vss  09/11/2023 Fixed Coverity warning EXPRESSION_WITH_MAGIC_NUMBERS
*	ss   04/05/2024 Fixed doxygen warnings
* 5.4   yog  04/29/2024 Fixed doxygen warnings.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in Xilsecure server
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_sss.h"
#include "xsecure_sha_hw.h"
#include "xsecure_sha.h"
#include "xsecure_defs.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/* XSecure_SssLookupTable[Input source][Resource] */
const u8 XSecure_SssLookupTable[XSECURE_SSS_MAX_SRCS][XSECURE_SSS_MAX_SRCS] = {
	/*+----+-----+-----+-----+-----+-----+-----+--------+
	*|DMA0| DMA1| PTPI| AES | SHA | SBI | PZM |Invalid |
	*+----+-----+-----+-----+-----+-----+-----+--------+
	* 0x00 = INVALID value
	*/
	{0x0DU, 0x00U, 0x00U, 0x06U, 0x00U, 0x0BU, 0x03U, 0x00U}, /* DMA0 */
	{0x00U, 0x09U, 0x00U, 0x07U, 0x00U, 0x0EU, 0x04U, 0x00U}, /* DMA1 */
	{0x0DU, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* PTPI */
	{0x0EU, 0x05U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* AES  */
	{0x0CU, 0x07U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SHA  */
	{0x05U, 0x0BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SBI  */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* PZI  */
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
	}
};

/************************** Function Prototypes ******************************/

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

	/** Update SSS mask value */
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
 *
 * @brief	This function validates whether size of the data is word aligned
 *		and if it is the last chunk size should be qword aligned.
 *
 * @param	Size		Size of data in bytes.
 * @param	IsLastChunk	Last chunk indication
 *
 * @return
 *		 - XST_SUCCESS  On successful valdation
 *		 - XSECURE_AES_UNALIGNED_SIZE_ERROR  If unaligned size is given as input
 *
 ******************************************************************************/
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;

	/** Validate the size is 4-byte aligned or not */
	if ((Size % XSECURE_WORD_SIZE) != 0x00U) {
		Status = (int)XSECURE_AES_UNALIGNED_SIZE_ERROR;
		goto END;
	}
	/** Validate the size is 16-byte aligned when it is last chunk */
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
 * @param	BaseAddress	Not applicable for versal
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration
 *		 - XSECURE_AES_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, const XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress)
{
	int Status = XST_FAILURE;
	(void)BaseAddress;

	if ((PmcDmaPtr == NULL) || (AesDmaCfg == NULL) || (Size == 0U)) {
		Status = (int)XSECURE_AES_INVALID_PARAM;
		goto END;
	}
	/** Enable PMC DMA Src and Dst channels for byte swapping.*/
	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		XSecure_AesPmcDmaCfgEndianness(PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
	}

	/**
	 * Sets the start address and size for both src and dest channels
	 * as per the configuration
	 */
	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
			((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XSecure_AesPmcDmaCfgEndianness(PmcDmaPtr,
			XPMCDMA_DST_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
	}

	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XPmcDma_64BitTransfer(PmcDmaPtr, XPMCDMA_DST_CHANNEL,
			(u32)AesDmaCfg->DestDataAddr, (u32)(AesDmaCfg->DestDataAddr >> XSECURE_ADDR_HIGH_SHIFT),
			Size / XSECURE_WORD_SIZE, AesDmaCfg->IsLastChunkDest);
	}

	if (AesDmaCfg->SrcChannelCfg == TRUE) {
		XPmcDma_64BitTransfer(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
			(u32)AesDmaCfg->SrcDataAddr, (u32)(AesDmaCfg->SrcDataAddr >> XSECURE_ADDR_HIGH_SHIFT),
			Size / XSECURE_WORD_SIZE, AesDmaCfg->IsLastChunkSrc);
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

	if ((AesDmaCfg->DestChannelCfg == TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		/* Wait for the DEST DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(PmcDmaPtr,
			XPMCDMA_DST_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Acknowledge the transfer has completed */
		XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_DST_CHANNEL,
			XPMCDMA_IXR_DONE_MASK);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This is a helper function to enable/disable byte swapping feature
 * 		of PMC DMA
 *
 * @param	InstancePtr	Pointer to the XPmcDma instance
 * @param	Channel		Channel Type
 *				- XPMCDMA_SRC_CHANNEL
 *				- XPMCDMA_DST_CHANNEL
 * @param	EndianType
 *			- 1 : Enable Byte Swapping
 *			- 0 : Disable Byte Swapping
 *
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
/** @} */
