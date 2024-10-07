/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sss.c
 *
 * This file contains the code for Secure Stream Switch configuration in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   02/09/24 Initial release
 *       ma   02/16/24 Remove unwanted print statement
 *       ma   03/16/24 Added error codes at required places
 *       yog  08/25/24 Integrated FIH library
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_sss.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_SSS_MAX_SRCS         (XASUFW_SSS_INVALID + 1U) /**< SSS Maximum resources */
#define XASUFW_LOCAL_SSS_CFG        (0xEBE8E000U) /**< ASU_LOCAL SSS_CFG register base address */

#define XASUFW_SSS_DMA0_MASK        (0xFU) /**< SSS DMA0 Mask */
#define XASUFW_SSS_AES_MASK         (0xF0U) /**< SSS AES Mask */
#define XASUFW_SSS_SHA2_MASK        (0xF00U) /**< SSS SHA2 Mask */
#define XASUFW_SSS_SHA3_MASK        (0xF000U) /**< SSS SHA3 Mask */
#define XASUFW_SSS_PLI_MASK         (0xF0000U) /**< SSS PL Interface Mask */
#define XASUFW_SSS_DMA1_MASK        (0xF00000U) /**< SSS DMA1 Mask */

#define XASUFW_SSS_AES_DMA0_VAL     (0x50U) /**< SSS DMA0 Value for AES */
#define XASUFW_SSS_SHA2_DMA0_VAL    (0x500U) /**< SSS DMA0 Value for SHA2 */
#define XASUFW_SSS_SHA3_DMA0_VAL    (0x5000U) /**< SSS DMA0 Value for SHA3 */
#define XASUFW_SSS_PLI_DMA0_VAL     (0x50000U) /**< SSS DMA0 Value for PL Interface */

#define XASUFW_SSS_AES_DMA1_VAL     (0xC0U) /**< SSS DMA1 Value for AES */
#define XASUFW_SSS_SHA2_DMA1_VAL    (0xC00U) /**< SSS DMA1 Value for SHA2 */
#define XASUFW_SSS_SHA3_DMA1_VAL    (0xC000U) /**< SSS DMA1 Value for SHA3 */
#define XASUFW_SSS_PLI_DMA1_VAL     (0xC0000U) /**< SSS DMA1 Value for PL Interface */

#define XASUFW_SSS_DMA0_PLI_VAL     (0xAU) /**< SSS PLI Value for DMA0 */
#define XASUFW_SSS_AES_PLI_VAL      (0xA0U) /**< SSS PLI Value for AES */
#define XASUFW_SSS_SHA2_PLI_VAL     (0xA00U) /**< SSS PLI Value for SHA2 */
#define XASUFW_SSS_SHA3_PLI_VAL     (0xA000U) /**< SSS PLI Value for SHA3 */
#define XASUFW_SSS_DMA1_PLI_VAL     (0xA00000U) /**< SSS PLI Value for DMA1 */

#define XASUFW_SSS_DMA0_AES_VAL     (0x9U) /**< SSS AES Value for DMA0 */
#define XASUFW_SSS_PLI_AES_VAL      (0x90000U) /**< SSS AES Value for PL Interface */
#define XASUFW_SSS_DMA1_AES_VAL     (0x900000U) /**< SSS AES Value for DMA1 */

#define XASUFW_SSS_CFG_LEN_IN_BITS  (4U) /**< Length is bits */
#define XASUFW_SSS_SRC_SEL_MASK     (0xFU) /**< SSS Source selection Mask */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_SssMask(XAsufw_SssSrc Resource, XAsufw_SssSrc InputSrc,
			   XAsufw_SssSrc OutputSrc);
static s32 XAsufw_SssCfg(XAsufw_SssSrc Resource, XAsufw_SssSrc InputSrc, XAsufw_SssSrc OutputSrc);

/************************************ Variable Definitions ***************************************/
/** XAsufw_SssLookupTable[Input Source][Resource] */
const u8 XAsufw_SssLookupTable[XASUFW_SSS_MAX_SRCS][XASUFW_SSS_MAX_SRCS] = {
	/**
	 *+----+-----+-----+-----+-----+-----+-----+--------+
	 *| DMA0 | AES | SHA2 | SHA3 | PLI | DMA1 | Invalid |
	 *+----+-----+-----+-----+-----+-----+-----+--------+
	 * 0x00 = INVALID value
	*/
	{0x05U, 0x09U, 0x00U, 0x00U, 0x0AU, 0x00U, 0x00U}, /* DMA0 */
	{0x05U, 0x00U, 0x00U, 0x00U, 0x0AU, 0x0CU, 0x00U}, /* AES */
	{0x05U, 0x00U, 0x00U, 0x00U, 0x0AU, 0x0CU, 0x00U}, /* SHA2 */
	{0x05U, 0x00U, 0x00U, 0x00U, 0x0AU, 0x0CU, 0x00U}, /* SHA3  */
	{0x05U, 0x09U, 0x00U, 0x00U, 0x00U, 0x0CU, 0x00U}, /* PLI  */
	{0x00U, 0x09U, 0x00U, 0x00U, 0x0AU, 0x0CU, 0x00U}, /* DMA1  */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* Invalid */
};

/*************************************************************************************************/
/**
 * @brief	This function configures secure stream switch to perform DMA loopback operation
 *
 * @param	DmaResource	DMA resource for the DMA loopback operation
 *
 * @return
 *		- XASUFW_SUCCESS, if configuration of the switch is successful.
 *		- XASUFW_FAILURE, if configuration of the switch fails.
 *
 *************************************************************************************************/
s32 XAsufw_SssDmaLoopback(XAsufw_SssSrc DmaResource)
{
	s32 Status = XASUFW_FAILURE;

	if ((DmaResource != XASUFW_SSS_DMA0) && (DmaResource != XASUFW_SSS_DMA1)) {
		Status = XASUFW_SSS_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	Status = XAsufw_SssCfg(DmaResource, DmaResource, XASUFW_SSS_INVALID);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures secure stream switch to perform DMA loopback operation with
 * 		SHA hash calculation
 *
 * @param	DmaResource	DMA resource for the DMA loopback operation with hash
 * @param	ShaResource	Output SHA resource to be used for DMA loopback operation with hash
 *
 * @return
 *		- XASUFW_SUCCESS, if configuration of the switch is successful.
 *		- XASUFW_FAILURE, if configuration of the switch fails.
 *
 *************************************************************************************************/
s32 XAsufw_SssDmaLoopbackWithSha(XAsufw_SssSrc DmaResource, XAsufw_SssSrc ShaResource)
{
	s32 Status = XASUFW_FAILURE;

	if (((DmaResource != XASUFW_SSS_DMA0) && (DmaResource != XASUFW_SSS_DMA1)) ||
	    ((ShaResource != XASUFW_SSS_SHA2) && (ShaResource != XASUFW_SSS_SHA3))) {
		Status = XASUFW_SSS_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	Status = XAsufw_SssCfg(DmaResource, DmaResource, ShaResource);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures secure stream switch to perform DMA to PL interface and
 * 		back.
 *
 * @param	DmaResource	DMA resource to be used for DMA to PL interface and back
 *
 * @return
 *		- XASUFW_SUCCESS, if configuration of the switch is successful.
 *		- XASUFW_FAILURE, if configuration of the switch fails.
 *
 *************************************************************************************************/
s32 XAsufw_SssDmaToPli(XAsufw_SssSrc DmaResource)
{
	s32 Status = XASUFW_FAILURE;

	if ((DmaResource != XASUFW_SSS_DMA0) && (DmaResource != XASUFW_SSS_DMA1)) {
		Status = XASUFW_SSS_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	Status = XAsufw_SssCfg(DmaResource, XASUFW_SSS_PLI, XASUFW_SSS_PLI);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures secure stream switch to perform SHA hash calculation using DMA
 *
 * @param	ShaResource	SHA resource to be used for hash calculation using DMA
 * @param	DmaResource	Input DMA resource to be used for hash calculation using DMA
 *
 * @return
 *		- XASUFW_SUCCESS, if configuration of the switch is successful.
 *		- XASUFW_FAILURE, if configuration of the switch fails.
 *
 *************************************************************************************************/
s32 XAsufw_SssShaWithDma(XAsufw_SssSrc ShaResource, XAsufw_SssSrc DmaResource)
{
	s32 Status = XASUFW_FAILURE;

	if (((DmaResource != XASUFW_SSS_DMA0) && (DmaResource != XASUFW_SSS_DMA1)) ||
		((ShaResource != XASUFW_SSS_SHA2) && (ShaResource != XASUFW_SSS_SHA3))) {
		Status = XASUFW_SSS_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	Status = XAsufw_SssCfg(ShaResource, DmaResource, XASUFW_SSS_INVALID);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures secure stream switch to perform SHA2/SHA3 hash generation
 * 		for PL interface
 *
 * @param	ShaResource SHA resource to be used for SSS configuration
 *
 * @return
 *		- XASUFW_SUCCESS, if configuration of SSS with SHA for PL interface is successful.
 *		- XASUFW_FAILURE, if configuration of SSS with SHA for PL fails.
 *
 *************************************************************************************************/
s32 XAsufw_SssShaForPli(XAsufw_SssSrc ShaResource)
{
	s32 Status = XASUFW_FAILURE;

	if ((ShaResource != XASUFW_SSS_SHA2) && (ShaResource != XASUFW_SSS_SHA3)) {
		Status = XASUFW_SSS_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	Status = XAsufw_SssCfg(ShaResource, XASUFW_SSS_PLI, XASUFW_SSS_INVALID);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures secure stream switch to perform encryption/decryption
 * 		operation using DMA0/DMA1
 *
 * @param	DmaResource	Input DMA resource for the AES resource
 *
 * @return
 *		- XASUFW_SUCCESS, if configuration of the switch is successful.
 *		- XASUFW_FAILURE, if configuration of the switch fails.
 *
 *************************************************************************************************/
s32 XAsufw_SssAesWithDma(XAsufw_SssSrc DmaResource)
{
	s32 Status = XASUFW_FAILURE;

	if ((DmaResource != XASUFW_SSS_DMA0) && (DmaResource != XASUFW_SSS_DMA1)) {
		Status = XASUFW_SSS_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	Status = XAsufw_SssCfg(XASUFW_SSS_AES, DmaResource, DmaResource);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures secure stream switch to perform encryption/decryption
 * 		operation for PL interface.
 *
 * @return
 *		- XASUFW_SUCCESS, if configuration of SSS with AES for PL interface is successful.
 *		- XASUFW_FAILURE, if configuration of SSS with AES for PL fails.
 *
 *************************************************************************************************/
s32 XAsufw_SssAesForPli(void)
{
	return XAsufw_SssCfg(XASUFW_SSS_AES, XASUFW_SSS_PLI, XASUFW_SSS_PLI);
}

/*************************************************************************************************/
/**
 * @brief	This function masks the secure stream switch value.
 *
 * @param	Resource	Resource for which input and output paths to be configured.
 * @param	InputSrc	Input source for the SSS configuration.
 * @param	OutputSrc	Output source for the SSS configuration.
 *
 *************************************************************************************************/
static s32 XAsufw_SssMask(XAsufw_SssSrc Resource, XAsufw_SssSrc InputSrc,
			   XAsufw_SssSrc OutputSrc)
{
	s32 Status = XASUFW_FAILURE;
	u32 Mask = 0U;
	u32 RegVal = XAsufw_ReadReg(XASUFW_LOCAL_SSS_CFG);

	/** If DMA0 is input or output source, clear this configuration from other resources. */
	if ((Resource == XASUFW_SSS_DMA0) || (InputSrc == XASUFW_SSS_DMA0) ||
	    (OutputSrc == XASUFW_SSS_DMA0)) {
		if ((RegVal & XASUFW_SSS_AES_MASK) == XASUFW_SSS_AES_DMA0_VAL) {
			Mask |= XASUFW_SSS_AES_MASK;
		}
		if ((RegVal & XASUFW_SSS_SHA2_MASK) == XASUFW_SSS_SHA2_DMA0_VAL) {
			Mask |= XASUFW_SSS_SHA2_MASK;
		}
		if ((RegVal & XASUFW_SSS_SHA3_MASK) == XASUFW_SSS_SHA3_DMA0_VAL) {
			Mask |= XASUFW_SSS_SHA3_MASK;
		}
		if ((RegVal & XASUFW_SSS_PLI_MASK) == XASUFW_SSS_PLI_DMA0_VAL) {
			Mask |= XASUFW_SSS_PLI_MASK;
		}
		if ((RegVal & XASUFW_SSS_DMA0_MASK) != 0U) {
			Mask |= XASUFW_SSS_DMA0_MASK;
		}
	}

	/** If DMA1 is input or output source, clear this configuration from other resources. */
	if ((Resource == XASUFW_SSS_DMA1) || (InputSrc == XASUFW_SSS_DMA1) ||
	    (OutputSrc == XASUFW_SSS_DMA1)) {
		if ((RegVal & XASUFW_SSS_AES_MASK) == XASUFW_SSS_AES_DMA1_VAL) {
			Mask |= XASUFW_SSS_AES_MASK;
		}
		if ((RegVal & XASUFW_SSS_SHA2_MASK) == XASUFW_SSS_SHA2_DMA1_VAL) {
			Mask |= XASUFW_SSS_SHA2_MASK;
		}
		if ((RegVal & XASUFW_SSS_SHA3_MASK) == XASUFW_SSS_SHA3_DMA1_VAL) {
			Mask |= XASUFW_SSS_SHA3_MASK;
		}
		if ((RegVal & XASUFW_SSS_PLI_MASK) == XASUFW_SSS_PLI_DMA1_VAL) {
			Mask |= XASUFW_SSS_PLI_MASK;
		}
		if ((RegVal & XASUFW_SSS_DMA1_MASK) != 0U) {
			Mask |= XASUFW_SSS_DMA1_MASK;
		}
	}

	/** If PLI is input or output source, clear this configuration from other resources. */
	if ((Resource == XASUFW_SSS_PLI) || (InputSrc == XASUFW_SSS_PLI) ||
	    (OutputSrc == XASUFW_SSS_PLI)) {
		if ((RegVal & XASUFW_SSS_DMA0_MASK) == XASUFW_SSS_DMA0_PLI_VAL) {
			Mask |= XASUFW_SSS_DMA0_MASK;
		}
		if ((RegVal & XASUFW_SSS_AES_MASK) == XASUFW_SSS_AES_PLI_VAL) {
			Mask |= XASUFW_SSS_AES_MASK;
		}
		if ((RegVal & XASUFW_SSS_SHA2_MASK) == XASUFW_SSS_SHA2_PLI_VAL) {
			Mask |= XASUFW_SSS_SHA2_MASK;
		}
		if ((RegVal & XASUFW_SSS_SHA3_MASK) == XASUFW_SSS_SHA3_PLI_VAL) {
			Mask |= XASUFW_SSS_SHA3_MASK;
		}
		if ((RegVal & XASUFW_SSS_DMA1_MASK) == XASUFW_SSS_DMA1_PLI_VAL) {
			Mask |= XASUFW_SSS_DMA1_MASK;
		}
		if ((RegVal & XASUFW_SSS_PLI_MASK) != 0U) {
			Mask |= XASUFW_SSS_PLI_MASK;
		}
	}

	/** If AES is input or output source, clear this configuration from other resources. */
	if ((Resource == XASUFW_SSS_AES) || (InputSrc == XASUFW_SSS_AES) ||
	    (OutputSrc == XASUFW_SSS_AES)) {
		if ((RegVal & XASUFW_SSS_DMA0_MASK) == XASUFW_SSS_DMA0_AES_VAL) {
			Mask |= XASUFW_SSS_DMA0_MASK;
		}
		if ((RegVal & XASUFW_SSS_DMA1_MASK) == XASUFW_SSS_DMA1_AES_VAL) {
			Mask |= XASUFW_SSS_DMA1_MASK;
		}
		if ((RegVal & XASUFW_SSS_PLI_MASK) == XASUFW_SSS_PLI_AES_VAL) {
			Mask |= XASUFW_SSS_PLI_MASK;
		}
		if ((RegVal & XASUFW_SSS_AES_MASK) != 0U) {
			Mask |= XASUFW_SSS_AES_MASK;
		}
	}

	RegVal &= ~Mask;
	Status = XAsufw_SecureOut32(XASUFW_LOCAL_SSS_CFG, RegVal);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures the secure stream switch.
 *
 * @param	Resource	Resource for which input and output paths to be configured.
 * @param	InputSrc	Input source to be selected for the resource.
 * @param	OutputSrc	Output source to be selected for the resource.
 *
 * @return
 *		- XASUFW_SUCCESS, if configuration of the switch is successful.
 *		- XASUFW_SSS_INVALID_INPUT_PARAMETERS, if input parameters to SSS is invalid.
 *
 *************************************************************************************************/
static s32 XAsufw_SssCfg(XAsufw_SssSrc Resource, XAsufw_SssSrc InputSrc, XAsufw_SssSrc OutputSrc)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 InputSrcCfg = 0x0U;
	u32 OutputSrcCfg = 0x0U;
	volatile u32 InputSrcCfgRedundant = 0x0U;
	volatile u32 OutputSrcCfgRedundant = 0x0U;
	u32 SssCfg = 0x0U;
	u32 InputMask = 0x0U;
	u32 OutputMask = 0x0U;
	u32 Mask = 0x0U;
	u32 RegVal = 0x0U;
	u32 InputShift;
	u32 OutputShift;

	/** Validate the input arguments. */
	if ((Resource > XASUFW_SSS_INVALID) || (InputSrc > XASUFW_SSS_INVALID) ||
	    (OutputSrc > XASUFW_SSS_INVALID)) {
		Status = XASUFW_SSS_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	/** Clear the previous configuration on the given resource and input/output sources. */
	Status = XAsufw_SssMask(Resource, InputSrc, OutputSrc);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Configure the input source of the given resource with source mentioned by InputSrc. */
	InputShift = (XASUFW_SSS_CFG_LEN_IN_BITS * (u32)Resource);
	InputSrcCfg = (u32)XAsufw_SssLookupTable[Resource][InputSrc] << InputShift;

	/**
	 * SSS allows configuring only input source for any Resources connected to it. So to define
	 * output source of given Resource, configure given Resource as input to source mentioned by
	 * OutputSrc.
	 */
	OutputShift = (XASUFW_SSS_CFG_LEN_IN_BITS * (u32)OutputSrc);
	OutputSrcCfg = (u32)XAsufw_SssLookupTable[OutputSrc][Resource] << OutputShift;

	/** Recalculating to verify values. */
	InputSrcCfgRedundant = (u32)XAsufw_SssLookupTable[Resource][InputSrc] << InputShift;
	OutputSrcCfgRedundant = (u32)XAsufw_SssLookupTable[OutputSrc][Resource] << OutputShift;

	SssCfg = InputSrcCfg | OutputSrcCfg;
	InputMask = (u32)XASUFW_SSS_SRC_SEL_MASK << InputShift;
	OutputMask = (u32)XASUFW_SSS_SRC_SEL_MASK << OutputShift;
	Mask = InputMask | OutputMask;

	if ((SssCfg ^ (InputSrcCfgRedundant | OutputSrcCfgRedundant)) == 0U) {
		RegVal = XAsufw_ReadReg(XASUFW_LOCAL_SSS_CFG);
		RegVal &= ~Mask;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_SecureOut32(XASUFW_LOCAL_SSS_CFG, SssCfg | RegVal);
	}

END:
	return Status;
}
/** @} */
