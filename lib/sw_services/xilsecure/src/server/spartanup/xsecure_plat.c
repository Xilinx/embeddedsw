/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat.c
* This file contains versal specific code for spartan ultrascale plus.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.5   kpt   08/16/22 Initial release
*       vss   04/07/25 Initialized AesKeyLookupTbl array.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_sss.h"
#include "xsecure_sha_hw.h"
#include "xsecure_sha.h"
#include "xsecure_aes.h"

/************************** Constant Definitions *****************************/

#define XSECURE_ADDR_HIGH_SHIFT (32U)

/************************** Variable Definitions *****************************/

const XSecure_AesKeyLookup AesKeyLookupTbl [XSECURE_MAX_KEY_SOURCES] =
{
	/* BH_KEY */
	[0U] = {
		.RegOffset = XSECURE_AES_BH_KEY_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_BH_KEY,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = TRUE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_BH_KEY_MASK
	},

	/* BH_RED_KEY */
	[1U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_BH_RD_KEY,
		.UsrWrAllowed = FALSE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_BH_RED,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK
	},

	/* EFUSE_KEY */
	[2U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_KEY,
		.UsrWrAllowed = FALSE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = TRUE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK
	},

	/* EFUSE_RED_KEY */
	[3U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_RED_KEY,
		.UsrWrAllowed = FALSE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_EFUSE_RED,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK
	},

	/* KUP_KEY */
	[4U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_KUP_KEY,
		.UsrWrAllowed = FALSE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK
	},

	/* FAMILY_KEY */
	[5U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_FAMILY_KEY,
		.UsrWrAllowed = FALSE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_INVALID_CFG
	},

	/* PUF_KEY */
	[6U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_PUF_KEY,
		.UsrWrAllowed = FALSE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK
	},

	/* USER_KEY_0 */
	[7U] = {
		.RegOffset = XSECURE_AES_USER_KEY_0_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_0,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK
	},

	/* USER_KEY_1 */
	[8U] = {
		.RegOffset = XSECURE_AES_USER_KEY_1_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_1,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK
	},

	/* USER_KEY_2 */
	[9U] = {
		.RegOffset = XSECURE_AES_USER_KEY_2_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_2,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK
	},

	/* USER_KEY_3 */
	[10U] = {
		.RegOffset = XSECURE_AES_USER_KEY_3_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_3,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK
	},

	/* USER_KEY_4 */
	[11U] = {
		.RegOffset = XSECURE_AES_USER_KEY_4_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_4,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK
	},

	/* USER_KEY_5 */
	[12U] = {
		.RegOffset = XSECURE_AES_USER_KEY_5_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_5,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK
	},

	/* USER_KEY_6 */
	[13U] = {
		.RegOffset = XSECURE_AES_USER_KEY_6_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_6,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK
	},

	/* USER_KEY_7 */
	[14U] = {
		.RegOffset = XSECURE_AES_USER_KEY_7_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_7,
		.UsrWrAllowed = TRUE,
		.DecAllowed = TRUE,
		.EncAllowed = TRUE,
		.KeyDecSrcAllowed = FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_7_MASK
	}
};

/* XSecure_SssLookupTable[Input source][Resource] */
const u8 XSecure_SssLookupTable
		[XSECURE_SSS_MAX_SRCS][XSECURE_SSS_MAX_SRCS] = {
	/*+----+-----+-----+-----+-----+-----+-----+--------+
	*|DMA0 | AES | SHA | SBI |Invalid |
	*+----+-----+-----+-----+-----+-----+-----+--------+
	* 0x00 = INVALID value
	*/
	{0x05U, 0x09U, 0x00U, 0x0AU}, /* DMA0 */
	{0x05U, 0x00U, 0x00U, 0x00U}, /* AES  */
	{0x05U, 0x00U, 0x00U, 0x0AU}, /* SHA  */
	{0x05U, 0x00U, 0x00U, 0x00U}, /* SBI  */
	{0x00U, 0x00U, 0x00U, 0x00U}, /* Invalid  */
};

/*
* The configuration table for devices
*/
const XSecure_ShaConfig ShaConfigTable[XSECURE_SHA_NUM_OF_INSTANCES] =
{
	{
		XSECURE_SSS_SHA,
		XSECURE_SHA_BASE_ADDRESS,
		XSECURE_SHA_DEVICE_ID,
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
 *	-	Mask - Mask value of corresponding InputSrc and OutputSrc
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
		if ((RegVal & XSECURE_SSS_SHA_MASK) == XSECURE_SSS_SHA_DMA0_VAL) {
			Mask |= XSECURE_SSS_SHA_MASK;
		}
		if ((RegVal & XSECURE_SSS_AES_MASK) == XSECURE_SSS_AES_DMA0_VAL) {
			Mask |= XSECURE_SSS_AES_MASK;
		}
		if ((RegVal & XSECURE_SSS_DMA0_MASK) != 0U) {
			Mask |= XSECURE_SSS_DMA0_MASK;
		}
	}

	return Mask;
}
/*****************************************************************************/
/**
 *
 * @brief      This function validates the size
 *
 * @param      Size            Size of data in bytes.
 * @param      IsLastChunk     Last chunk indication
 *
 * @return
 *     -       XST_SUCCESS on successful valdation
 *     -       Error code on failure
 *
 ******************************************************************************/
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;

	/* Validate the Size */
	if ((Size % XSECURE_WORD_SIZE) != 0x00U) {
		Status = (int)XSECURE_AES_UNALIGNED_SIZE_ERROR;
		goto END;
	}
	/* Validate the size based on last chunk */
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
 * @brief       This function configures the PMC DMA channels and transfers data
 *
 * @param       PmcDmaPtr       Pointer to the XPmcDma instance.
 * @param       AesDmaCfg       DMA SRC and DEST channel configuration
 * @param       Size            Size of data in bytes.
 * @param	BaseAddress     Not applicable for versal
 *
 * @return
 *	-	XST_SUCCESS on successful configuration
 *	-	Error code on failure
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
 * @param	InstancePtr  Pointer to the XPmcDma instance
 * @param	Channel 	 Channel Type
 *			- XPMCDMA_SRC_CHANNEL
 *			 -XPMCDMA_DST_CHANNEL
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

/*******************************************************************************/
/**
 * @brief	This function validates the SHA Mode and initialize SHA instance.
 *
 * @param	InstancePtr - Pointer to the SHA instance.
 * @param	ShaMode - SHA Mode.
 *
 * @return
 *		XST_SUCCESS - Upon Success.
 *		XST_FAILURE - Upon Failure.
 *		XSECURE_SHA_INVALID_MODE_ERROR
 ********************************************************************************/
int XSecure_ShaValidateModeAndCfgInstance(XSecure_Sha * const InstancePtr,
	XSecure_ShaMode ShaMode)
{
	volatile int Status = XST_FAILURE;

	/** Initializes the SHA instance based on SHA Mode */
	switch(ShaMode) {
        case XSECURE_SHA3_256:
            InstancePtr->ShaDigestSize = (u32)XSECURE_SHA3_256_HASH_LEN;
            InstancePtr->ShaMode = (u32)SHA256;
            break;
		/** SHAKE-256 Mode */
		case XSECURE_SHAKE_256:
			InstancePtr->ShaDigestSize = (u32)XSECURE_SHAKE_256_HASH_LEN;
			InstancePtr->ShaMode = SHAKE256;
			break;
		/** SHA invalid mode */
		case XSECURE_SHA_INVALID_MODE:
		default:
			Status = (int)XSECURE_SHA_INVALID_PARAM;
			break;
	}

	if (Status == XSECURE_SHA_INVALID_PARAM) {
		goto END;
	}
	else {
		Status = XST_SUCCESS;
	}
END:
	return Status;
}

/*******************************************************************************************/
/**
* @brief	This function validate SHA input data size.
*
* @param	Size - Input data size in bytes.
*
* @return
*		XST_SUCCESS - Upon Success.
*
 *******************************************************************************************/
int XSecure_ValidateShaDataSize(const u32 Size)
{
	int Status = XST_FAILURE;

	if (Size % XSECURE_WORD_SIZE == 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*******************************************************************************************/
/**
* @brief	This function transfer data to SHA engine from DMA
*
* @param    DmaPtr - Pointer to XPmcDma
* @param    DataAddr - input data address
* @param	Size - Input data size in words.
* @param    IsLastUpdate - Last update
*
* @return
*		XST_SUCCESS - Upon Success.
*
 *******************************************************************************************/
int XSecure_ShaDmaXfer(XPmcDma *DmaPtr, u64 DataAddr, u32 Size, u8 IsLastUpdate)
{
	int Status = XST_FAILURE;

	if (DmaPtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	if ((IsLastUpdate != TRUE) && (IsLastUpdate != FALSE)) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	XPmcDma_Transfer(DmaPtr, XPMCDMA_SRC_CHANNEL,
		DataAddr, (u32)Size/XSECURE_WORD_SIZE, IsLastUpdate);
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures SSS to AES engine
 *
 * @param	DmaPtr	Pointer to the DMA instance
 * @param 	SssInstance Pointer to SSS instance
 *
 * @return
 *	-	XST_SUCCESS on successful configuration
 *	-	Error code on failure
 *
 ******************************************************************************/
int XSecure_CfgSssAes(XPmcDma *DmaPtr, const XSecure_Sss *SssInstance)
{
	int Status = XST_FAILURE;

#ifndef SDT
	if (DmaPtr->Config.DeviceId == (u16)PMCDMA_0_DEVICE_ID) {
#else
	if (DmaPtr->Config.BaseAddress == PMCDMA_0_DEVICE_ID) {
#endif
		Status = XSecure_SssAes(SssInstance,
				XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to set the Data context bit
 * 		of the corresponding IPI channel if the previous data context is lost.
 *
 * @param	InstancePtr		Pointer to the XSecure_Aes instance
 *
 *
 ******************************************************************************/
void XSecure_AesSetDataContext(XSecure_Aes *InstancePtr) {
	(void)InstancePtr;
}

/*****************************************************************************/
/**
 * @brief	This function is used to set the Data context bit of the
 * 		corresponding IPI channel if the previous data context is lost.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha instance
 *
 ******************************************************************************/
void XSecure_ShaSetDataContext(XSecure_Sha *InstancePtr)
{
	(void)InstancePtr;
}

/***************************************************************************/
/**
 * @brief	This function checks if the EXPORT control eFuse is
 * 		programmed and PL loading is done
 *
 * @return
 *	-	XST_SUCCESS - When crypto accelerators are enabled
 *	-	XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED - When crypto accelerators
 *		are disabled
 *
 ******************************************************************************/
int XSecure_CryptoCheck(void)
{
	/* Not applicable for spartan ultrascal plus */
	return XST_SUCCESS;
}
