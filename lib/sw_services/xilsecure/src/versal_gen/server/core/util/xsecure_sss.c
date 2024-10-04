/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_sss.c
* This file contains functions for SSS switch configurations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   har     03/26/20 Initial release
* 4.2   har     03/26/20 Updated file version to sync with library version
* 4.3   rpo     09/10/20 Asserts are not compiled by default for
*                        secure libraries
*       rpo     09/21/20 New error code added for crypto state mismatch
*       am      09/24/20 Resolved MISRA C violations
*       har     10/12/20 Addressed security review comments
*       bsv     10/19/20 Changed register writes to PMC SSS Cfg switch to mask
*                        writes
*       kpt     11/12/20 Fixed SSS Cfg issue
* 4.5   am      11/24/20 Resolved Coverity warnings
* 4.6   har     07/14/21 Fixed doxygen warnings
* 5.0   bm      07/06/22 Refactor versal and versal_net code
*       am      08/18/22 Added volatile keyword and reset the Status variable
*                        to failure before reuse
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_sss.h"
#include "xsecure_utils.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
static int XSecure_SssDmaSrc(u16 DmaId, XSecure_SssSrc *Resource);
static int XSecure_SssCfg (const XSecure_Sss *InstancePtr,
			   XSecure_SssSrc Resource, XSecure_SssSrc InputSrc,
			   XSecure_SssSrc OutputSrc);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function initializes the secure stream switch instance
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 *
 * @return
 *		 - XST_SUCCESS  If initialization was successful
 *		 - XSECURE_SSS_INVALID_PARAM  On invalid parameter
 *
 *****************************************************************************/
int XSecure_SssInitialize (XSecure_Sss *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Validates the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SSS_INVALID_PARAM;
		goto END;
	}

	InstancePtr->Address = XSECURE_SSS_ADDRESS;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures the secure stream switch for AES engine
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	InputSrc	Input DMA to be selected for AES engine
 * @param	OutputSrc	Output DMA to be selected for AES engine
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration of the switch
 *		 - XSECURE_SSS_INVALID_PARAM  On invalid parameter
 *		 - XST_FAILURE  On failure to configure switch
 *
 * @note	InputSrc, OutputSrc are of type XSecure_SssSrc
 *
 *****************************************************************************/
int XSecure_SssAes(const XSecure_Sss *InstancePtr,
	XSecure_SssSrc InputSrc, XSecure_SssSrc OutputSrc)
{
	volatile int Status = XST_FAILURE;
	u32 Mask = 0U;
	u32 RegVal;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SSS_INVALID_PARAM;
		goto END;
	}

	if ((InputSrc != XSECURE_SSS_DMA0) &&
		(InputSrc != XSECURE_SSS_DMA1)) {
		Status = (int)XSECURE_SSS_INVALID_PARAM;
		goto END;
	}

	if ((OutputSrc != XSECURE_SSS_DMA0) &&
		(OutputSrc != XSECURE_SSS_DMA1)) {
		Status = (int)XSECURE_SSS_INVALID_PARAM;
		goto END;
	}

	RegVal = Xil_In32(InstancePtr->Address);
	Mask = XSecure_SssMask(InputSrc, OutputSrc, RegVal);
	RegVal &= ~Mask;
	Status = XSecure_SecureOut32(InstancePtr->Address, RegVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_SssCfg(InstancePtr, XSECURE_SSS_AES,
			InputSrc, OutputSrc);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures the secure stream switch for SHA hardware
 *		engine
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	DmaId		Device ID of DMA which is to be used as an input to
 *				the SHA engine
 * @param	Resource	SHA resource for which input and output paths to be
 *				configured
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration of the switch
 *		 - XSECURE_SSS_INVALID_PARAM  On invalid parameter
 *		 - XST_FAILURE  On failure to configure switch
 *
 *****************************************************************************/
int XSecure_SssSha(const XSecure_Sss *InstancePtr, u16 DmaId,
		XSecure_SssSrc Resource)
{
	volatile int Status = XST_FAILURE;
	XSecure_SssSrc InputSrc = XSECURE_SSS_INVALID;
	u32 Mask = 0U;
	u32 RegVal;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) ||
		((DmaId != (u16)XSECURE_SSS_DMA0) &&
			(DmaId != (u16)XSECURE_SSS_DMA1))) {
		Status = (int)XSECURE_SSS_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_SssDmaSrc(DmaId, &InputSrc);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	RegVal = Xil_In32(InstancePtr->Address);
	Mask = XSecure_SssMask(InputSrc, XSECURE_SSS_INVALID, RegVal);
	RegVal &= ~Mask;
	Status = XSecure_SecureOut32(InstancePtr->Address, RegVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	/* configure the secure stream switch */
	Status = XSecure_SssCfg(InstancePtr, Resource, InputSrc,
			XSECURE_SSS_INVALID);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures secure stream switch to set DMA in
 *		loop back mode
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	DmaId		Device ID of DMA
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration of the switch
 *		 - XSECURE_SSS_INVALID_PARAM  On invalid parameter
 *		 - XST_FAILURE  On failure to configure switch
 *
 *****************************************************************************/
int XSecure_SssDmaLoopBack(const XSecure_Sss *InstancePtr, u16 DmaId)
{
	volatile int Status = XST_FAILURE;
	XSecure_SssSrc Resource = XSECURE_SSS_INVALID;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) ||
		((DmaId != (u16)XSECURE_SSS_DMA0) &&
			(DmaId != (u16)XSECURE_SSS_DMA1))) {
		Status = (int)XSECURE_SSS_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_SssDmaSrc(DmaId, &Resource);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	Status = XSecure_SssCfg(InstancePtr, Resource, Resource,
				XSECURE_SSS_INVALID);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the DMA source of type XSecure_SssSrc based
 * 		on the provided DMA device ID
*
 * @param	DmaId		Device ID of DMA
 * @param	Resource	DMA source is updated into the pointer
 *
 * @return
 *		 - XST_SUCCESS  If DMA ID is correct
 *		 - XST_FAILURE  On wrong DMA ID
 *
 *****************************************************************************/
static int XSecure_SssDmaSrc(u16 DmaId, XSecure_SssSrc *Resource)
{
	int Status = XST_FAILURE;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(Resource != NULL);
	XSecure_AssertNonvoid((DmaId == 0U) || (DmaId == 1U));

	if (DmaId == 0U) {
		*Resource = XSECURE_SSS_DMA0;
		Status = XST_SUCCESS;
	}
	else {
		*Resource = XSECURE_SSS_DMA1;
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures the secure stream switch
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	Resource	Resource for which input and output paths to be
 *				configured
 * @param	InputSrc	Input source to be selected for the resource
 * @param	OutputSrc	Output source to be selected for the resource
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration of the switch
 *		 - XST_FAILURE  On unsuccessful configuration of the switch
 *
 * @note	Resource, InputSrc, OutputSrc are of type XSecure_SssSrc
 *
 *****************************************************************************/
static int XSecure_SssCfg (const XSecure_Sss *InstancePtr,
	XSecure_SssSrc Resource, XSecure_SssSrc InputSrc,
	XSecure_SssSrc OutputSrc)
{
	int Status = XST_FAILURE;
	u32 InputSrcCfg = 0x0U;
	u32 OutputSrcCfg = 0x0U;
	volatile u32 InputSrcCfgRedundant = 0x0U;
	volatile u32 OutputSrcCfgRedundant = 0x0U;
	u32 SssCfg = 0x0U;
	u32 InputMask = 0U;
	u32 OutputMask = 0U;
	u32 Mask = 0U;
	u32 RegVal = 0U;
	u32 InputShift;
	u32 OutputShift;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);
	XSecure_AssertNonvoid((InputSrc >= XSECURE_SSS_DMA0) &&
		(InputSrc <= XSECURE_SSS_INVALID));
	XSecure_AssertNonvoid((OutputSrc >= XSECURE_SSS_DMA0) &&
		(OutputSrc <= XSECURE_SSS_INVALID));
	XSecure_AssertNonvoid((Resource >= XSECURE_SSS_DMA0) &&
		(Resource <= XSECURE_SSS_INVALID));

	/*
	 * Configure source of the input for given resource
	 * i.e Configure given InputSrc as a input for given Resource
	 */
	InputShift = (XSECURE_SSS_CFG_LEN_IN_BITS * (u32)Resource);
	InputSrcCfg = (u32) XSecure_SssLookupTable [Resource][InputSrc] <<
		InputShift;

	/*
	 * SSS allows configuring only input source for any Resources
	 * connected to it. So to define output source of given Resource,
	 *  configure given Resource as input to source mentioned by OutputSrc
	 */
	OutputShift = (XSECURE_SSS_CFG_LEN_IN_BITS * (u32)OutputSrc);
	OutputSrcCfg = (u32) XSecure_SssLookupTable [OutputSrc][Resource] <<
		OutputShift;

	/* Recalculating to verify values */
	InputSrcCfgRedundant = (u32) XSecure_SssLookupTable [Resource][InputSrc] <<
		InputShift;

	OutputSrcCfgRedundant = (u32) XSecure_SssLookupTable [OutputSrc][Resource] <<
		OutputShift;

	SssCfg = InputSrcCfg | OutputSrcCfg;
	InputMask = (u32)XSECURE_SSS_SRC_SEL_MASK << InputShift;
	OutputMask = (u32)XSECURE_SSS_SRC_SEL_MASK << OutputShift;
	Mask = InputMask | OutputMask;

	if ((SssCfg ^ (InputSrcCfgRedundant | OutputSrcCfgRedundant)) == 0U) {
		RegVal = Xil_In32(InstancePtr->Address);
		RegVal &= ~Mask;
		Status = XSecure_SecureOut32(InstancePtr->Address, SssCfg | RegVal);
	}

	return Status;
}
/** @} */
