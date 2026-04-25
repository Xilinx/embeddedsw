/******************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file server/spartanup/xsecure_plat.c
* This file contains versal specific code for spartan ultrascale plus.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.5   kpt   08/16/22 Initial release
*       vss   04/07/25 Initialized AesKeyLookupTbl array.
*       vss   06/19/25 Removed dead code
* 5.6   aa    07/15/25 Typecast to essential datatype to avoid implicit conversions
*                      added explicit parenthesis for sub-expression and fix
*                      partially initialized array
*       mb    09/11/25 Added SHA3_384 mode check to calculate hash
* 5.7   tvp   02/23/26 Use XSecure_ShaPlatConfig, platform specific SHA configurations
*       mb    03/13/26 Add support for ECC curves for SPARTANUPLUSAES1 device
*       rpu   04/22/26 Fix XSecure_GetRandomNum for non-word-aligned sizes
*       mb    04/23/26 Remove unused macros
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in XilSecure server
* @{
*/

/***************************** Include Files *********************************/
#include "xsecure_sss.h"
#include "xsecure_sha_hw.h"
#include "xsecure_sha.h"
#include "xsecure_utils.h"
#include "xsecure_aes.h"
#include "xil_sutil.h"

/************************** Constant Definitions *****************************/

#define XSECURE_ADDR_HIGH_SHIFT (32U)	/**< High address shift for 64-bit addresses */

/** TRNG related macros for SPARTANUPLUSAES1 device */
#ifdef SPARTANUPLUSAES1
#define XSECURE_TRNGPSX_BASEADDR	XPAR_XTRNGPSX_0_BASEADDR /**< TRNGPSX base address */
#endif

/************************** Variable Definitions *****************************/

const XSecure_AesKeyLookup AesKeyLookupTbl [XSECURE_MAX_KEY_SOURCES] =
{
	/** - BH_KEY */
	[0U] = {
		.RegOffset = XSECURE_AES_BH_KEY_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_BH_KEY,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)TRUE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_BH_KEY_MASK
	},

	/** - BH_RED_KEY */
	[1U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_BH_RD_KEY,
		.UsrWrAllowed = (u8)FALSE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_BH_RED,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK
	},

	/** - EFUSE_KEY */
	[2U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_KEY,
		.UsrWrAllowed = (u8)FALSE,
		.KeyDecSrcAllowed = (u8)TRUE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK
	},

	/** - EFUSE_RED_KEY */
	[3U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_RED_KEY,
		.UsrWrAllowed = (u8)FALSE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_EFUSE_RED,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK
	},

	/** - KUP_KEY */
	[4U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_KUP_KEY,
		.UsrWrAllowed = (u8)FALSE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK
	},

	/** - FAMILY_KEY */
	[5U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_FAMILY_KEY,
		.UsrWrAllowed = (u8)FALSE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_INVALID_CFG
	},

	/** - PUF_KEY */
	[6U] = {
		.RegOffset = XSECURE_AES_INVALID_CFG,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_PUF_KEY,
		.UsrWrAllowed = (u8)FALSE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK
	},

	/** - USER_KEY_0 */
	[7U] = {
		.RegOffset = XSECURE_AES_USER_KEY_0_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_0,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK
	},

	/** - USER_KEY_1 */
	[8U] = {
		.RegOffset = XSECURE_AES_USER_KEY_1_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_1,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK
	},

	/** - USER_KEY_2 */
	[9U] = {
		.RegOffset = XSECURE_AES_USER_KEY_2_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_2,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK
	},

	/** - USER_KEY_3 */
	[10U] = {
		.RegOffset = XSECURE_AES_USER_KEY_3_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_3,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK
	},

	/** - USER_KEY_4 */
	[11U] = {
		.RegOffset = XSECURE_AES_USER_KEY_4_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_4,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK
	},

	/** - USER_KEY_5 */
	[12U] = {
		.RegOffset = XSECURE_AES_USER_KEY_5_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_5,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK
	},

	/** - USER_KEY_6 */
	[13U] = {
		.RegOffset = XSECURE_AES_USER_KEY_6_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_6,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)FALSE,
		.KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
		.KeyClearVal = XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK
	},

	/** - USER_KEY_7 */
	[14U] = {
		.RegOffset = XSECURE_AES_USER_KEY_7_0_OFFSET,
		.KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_7,
		.UsrWrAllowed = (u8)TRUE,
		.KeyDecSrcAllowed = (u8)FALSE,
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
	{0x05U, 0x09U, 0x00U, 0x0AU, 0x00U}, /* DMA0 */
	{0x05U, 0x00U, 0x00U, 0x00U, 0x00U}, /* AES  */
	{0x05U, 0x00U, 0x00U, 0x0AU, 0x00U}, /* SHA  */
	{0x05U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SBI  */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* Invalid  */
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
 *     -       XST_SUCCESS on successful validation
 *     -       Error code on failure
 *
 ******************************************************************************/
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk)
{
	int Status = XST_FAILURE;

	/** - Validate the Size */
	if ((Size % XSECURE_WORD_SIZE) != 0x00U) {
		Status = (int)XSECURE_AES_UNALIGNED_SIZE_ERROR;
		goto END;
	}
	/** - Validate the size based on last chunk */
	if ((IsLastChunk != (u8)TRUE) &&
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

	if ((AesDmaCfg->DestChannelCfg == (u8)TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		XPmcDma_64BitTransfer(PmcDmaPtr, XPMCDMA_DST_CHANNEL,
			(u32)AesDmaCfg->DestDataAddr, (u32)(AesDmaCfg->DestDataAddr >> XSECURE_ADDR_HIGH_SHIFT),
			Size / XSECURE_WORD_SIZE, AesDmaCfg->IsLastChunkDest);
	}

	if (AesDmaCfg->SrcChannelCfg == (u8)TRUE) {
		XPmcDma_64BitTransfer(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
			(u32)AesDmaCfg->SrcDataAddr, (u32)(AesDmaCfg->SrcDataAddr >> XSECURE_ADDR_HIGH_SHIFT),
			Size / XSECURE_WORD_SIZE, AesDmaCfg->IsLastChunkSrc);
	}

	if (AesDmaCfg->SrcChannelCfg == (u8)TRUE) {
		/** - Wait for the SRC DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - Acknowledge the transfer has completed */
		XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
			XPMCDMA_IXR_DONE_MASK);
	}

	if ((AesDmaCfg->DestChannelCfg == (u8)TRUE) &&
		((u32)AesDmaCfg->DestDataAddr != XSECURE_AES_NO_CFG_DST_DMA)) {
		/** - Wait for the DEST DMA completion. */
		Status = XPmcDma_WaitForDoneTimeout(PmcDmaPtr,
			XPMCDMA_DST_CHANNEL);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - Acknowledge the transfer has completed */
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
 * @param	Channel 	 Channel Type (XPMCDMA_SRC_CHANNEL or XPMCDMA_DST_CHANNEL)
 * @param	EndianType   1 to enable byte swapping, 0 to disable byte swapping
 *
 *
 ******************************************************************************/
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel, u8 EndianType)
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

/*******************************************************************************/
/**
 * @brief	This function validates the SHA Mode and initialize SHA instance.
 *
 * @param	InstancePtr  Pointer to the SHA instance.
 * @param	ShaMode  SHA Mode.
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
	XSecure_ShaPlatConfig *ShaPlatConfig = (XSecure_ShaPlatConfig *)InstancePtr->ShaPlatConfig;

	/** - Initializes the SHA instance based on SHA Mode */
	switch(ShaMode) {
        case XSECURE_SHA3_256:
            ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHA3_256_HASH_LEN;
            ShaPlatConfig->ShaMode = (u32)SHA256;
            break;
		case XSECURE_SHAKE_256:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHAKE_256_HASH_LEN;
			ShaPlatConfig->ShaMode = SHAKE256;
			break;
#ifdef SPARTANUPLUSAES1
		case XSECURE_SHA3_384:
			ShaPlatConfig->ShaDigestSize = (u32)XSECURE_SHA3_384_HASH_LEN;
			ShaPlatConfig->ShaMode = (u32)SHA384;
			break;
#endif
		case XSECURE_SHA_INVALID_MODE:
		default:
			Status = (int)XSECURE_SHA_INVALID_PARAM;
			break;
	}

	if (Status == (int)XSECURE_SHA_INVALID_PARAM) {
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
* @brief	This function stages input data through the DMA-safe PartialData
*		buffer and transfers it block-by-block to the SHA engine.
*
*		On Spartan UltraScale+, the DMA controller can only access a
*		restricted memory region. This function accumulates incoming data
*		in the PartialData buffer (which resides in DMA-reachable memory),
*		and DMA's complete blocks to the SHA engine one at a time. Any
*		remaining data less than one block is kept in PartialData for
*		the next call or for NIST padding during ShaFinish.
*
* @param	InstancePtr	Pointer to the SHA instance.
* @param	DataAddr	Address of the input data.
* @param	Size		Size of the input data in bytes.
* @param	IsLastUpdate	Flag indicating if this is the last data transfer.
*
* @return
*		- XST_SUCCESS - Upon successful data transfer.
*		- XST_FAILURE - Upon failure.
*		- XSECURE_SHA_INVALID_PARAM - Upon invalid parameter.
*
 *******************************************************************************************/
int XSecure_ShaDmaXfer(void *InstancePtr, u64 DataAddr, u32 Size, u8 IsLastUpdate)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha *ShaInstancePtr = (XSecure_Sha *)InstancePtr;
	u32 RemainingDataLen;
	u8 IsLastBlock;
	u32 PrevPartialLen;
	u8 *PartialData;
	u64 InDataAddr = DataAddr;
	u32 BlockLen;
	const XSecure_ShaPlatConfig *ShaPlatConfig;

	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	if ((IsLastUpdate != (u8)TRUE) && (IsLastUpdate != (u8)FALSE)) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	ShaPlatConfig = (const XSecure_ShaPlatConfig *)ShaInstancePtr->ShaPlatConfig;

	BlockLen = XSecure_ShaGetBlockLen(ShaPlatConfig->HashAlgo);
	if (BlockLen == 0U) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	PrevPartialLen = ShaInstancePtr->PartialLen;
	PartialData = ShaInstancePtr->PartialData;

	RemainingDataLen = Size + PrevPartialLen;
	IsLastBlock = (u8)FALSE;

	while (RemainingDataLen >= BlockLen) {
		if (PrevPartialLen != 0U) {
			if ((u64)(UINTPTR)&PartialData[PrevPartialLen] != InDataAddr) {
				XSecure_MemCpy64(
					(u64)(UINTPTR)&PartialData[PrevPartialLen],
					InDataAddr, BlockLen - PrevPartialLen);
			}
			InDataAddr += (BlockLen - PrevPartialLen);
		}
		else {
			if ((u64)(UINTPTR)PartialData != InDataAddr) {
				XSecure_MemCpy64(
					(u64)(UINTPTR)PartialData,
					InDataAddr, BlockLen);
			}
			InDataAddr += (u64)BlockLen;
		}

		RemainingDataLen -= BlockLen;

		if ((RemainingDataLen == 0U) &&
		    (IsLastUpdate == (u8)TRUE)) {
			IsLastBlock = (u8)TRUE;
		}

		XPmcDma_Transfer(ShaInstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL,
			(u64)(UINTPTR)PartialData,
			BlockLen / XSECURE_WORD_SIZE, IsLastBlock);

		Status = XPmcDma_WaitForDoneTimeout(ShaInstancePtr->DmaPtr,
				XPMCDMA_SRC_CHANNEL);
		if (Status != XST_SUCCESS) {
			if (Xil_SMemSet(ShaInstancePtr->PartialData,
				sizeof(ShaInstancePtr->PartialData), 0U,
				sizeof(ShaInstancePtr->PartialData)) != XST_SUCCESS) {
				Status = XST_FAILURE;
			}
			ShaInstancePtr->PartialLen = 0U;
			goto END;
		}

		XPmcDma_IntrClear(ShaInstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);
		PrevPartialLen = 0U;
	}

	/**
	 * - Store remaining data (< BlockLen) in PartialData for the next
	 *   call or for NIST padding in ShaFinish.
	 */
	if ((RemainingDataLen > 0U) &&
	    ((u64)(UINTPTR)&PartialData[PrevPartialLen] != InDataAddr)) {
		XSecure_MemCpy64(
			(u64)(UINTPTR)&PartialData[PrevPartialLen],
			InDataAddr, RemainingDataLen - PrevPartialLen);
	}
	ShaInstancePtr->PartialLen = RemainingDataLen;

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
	if (DmaPtr->Config.BaseAddress == (UINTPTR)PMCDMA_0_DEVICE_ID) {
#endif
		Status = XSecure_SssAes(SssInstance,
				XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}

	return Status;
}

/***************************************************************************/
/**
 * @brief	This function checks whether the requested crypto accelerator
 * 		is enabled for the specified core
 *
 * @param	CoreSrc	Crypto core source (AES, SHA, RSA)
 *
 * @return
 *	-	XST_SUCCESS - When crypto accelerators are enabled
 *	-	XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED - When crypto accelerators
 *		are disabled
 *	-	XSECURE_ERR_INVALID_CORE - When invalid core is specified
 *
 ******************************************************************************/
int XSecure_CryptoCheck(XSecure_CoreSrc CoreSrc)
{
	volatile int Status = XST_FAILURE;
	u32 ExportControl = XSecure_In32(XSECURE_EFUSE_CONTROLS_ADDRESS) &
			    XSECURE_EFUSE_EXPORT_CONTROL_MASK;
	u32 CryptoDisVal = XSecure_In32(XSECURE_PMC_GLOBAL_CRYPTO_DIS_ADDRESS);
	u32 StickyDisMask = 0U;

	/**
	 * Checks whether the specified crypto core (AES, SHA, or RSA) is enabled.
	 * - For AES: returns XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED if export control
	 *   is set or the AES sticky disable bit is set.
	 * - For SHA: returns XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED if the SHA
	 *   sticky disable bit is set.
	 * - For RSA: returns XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED if export control
	 *   is set or the RSA sticky disable bit is set.
	 *
	 */
	switch (CoreSrc) {
		/* AES Core */
		case XSECURE_CORE_AES:
			StickyDisMask = XSECURE_AES_CRYPTO_DIS_MASK;
			break;
		/* SHA Core */
		case XSECURE_CORE_SHA:
			StickyDisMask = XSECURE_SHA_CRYPTO_DIS_MASK;
			ExportControl = 0U;
			break;
#ifdef SPARTANUPLUSAES1
		/* RSA-ECC Core */
		case XSECURE_CORE_RSA_ECC:
			StickyDisMask = XSECURE_RSA_CRYPTO_DIS_MASK;
			break;
#endif
		default:
			Status = XSECURE_ERR_INVALID_CORE;
			goto END;
	}

	if ((ExportControl != 0U) || ((CryptoDisVal & StickyDisMask) == StickyDisMask)) {
		Status = XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

#ifdef SPARTANUPLUSAES1
/*****************************************************************************/
/**
 * @brief	This function initializes the trng in HRNG mode if it is not initialized
 *          	and performs health tests if TRNG is not healthy
 *
 * @return
 *		- XST_SUCCESS  On Successful initialization
 *      	- XST_FAILURE  On Failure
 *
 *****************************************************************************/
int XSecure_ECCRandInit(void)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XTrngpsx_Config *Config;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();

	/*
	 * Initialize the TRNGPSX driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
	if (TrngInstance->State <= (u32)XTRNGPSX_UNINITIALIZED_STATE) {
		Config = XTrngpsx_LookupConfig(XSECURE_TRNGPSX_BASEADDR);
		if (NULL == Config) {
			goto END;
		}

	/** - Initialize the TRNGPSX driver so that it is ready to use. */
		Status = XTrngpsx_CfgInitialize(TrngInstance, Config, Config->BaseAddress);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/** - Perform health tests if TRNG is not healthy */
	if(!XSecure_TrngIsHealthy(TrngInstance)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_PreOperationalSelfTests, TrngInstance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = (int)XSECURE_ERR_IN_TRNG_SELF_TESTS;
			goto END;
		}
	}

	/** - Initialize the TRNG in HRNG mode if it is not initialized */
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

/*****************************************************************************/
/**
 * @brief	This function generates random number of given size
 *
 * @param	Output	Pointer to the output buffer
 * @param	Size	Number of random bytes to be read
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *  		 - XST_FAILURE  On Failure
 * 		 - XST_INVALID_PARAM - Invalid input parameter (size is 0 or output is NULL)
 *		 - XSECURE_ERR_GLITCH_DETECTED Error when glitch is detected
 *
 *****************************************************************************/
int XSecure_GetRandomNum(u8 *Output, u32 Size)
{
	volatile int Status = XST_FAILURE;
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

		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_Generate, TrngInstance,
					RandBufPtr, RandBufSize, FALSE);
		RandBufPtr += RandBufSize;
		TotalSize -= RandBufSize;
	}

	if (Index != NoOfGenerates) {
		Status = (int)XSECURE_ERR_GLITCH_DETECTED;
	}

END:
	if (Status != XST_SUCCESS) {
		Status |= XSecure_Uninstantiate(TrngInstance);
	}

	return Status;
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
	(void)Op;
}
#endif /** SPARTANUPLUSAES1 */
/** @} */