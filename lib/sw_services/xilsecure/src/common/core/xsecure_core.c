/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_core.c
* This file contains core specific common code for versalgen.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   kpt   08/17/24 Initial release
*       pre   03/02/25 Removed data context setting for AES and SHA
* 5.5   hj    03/20/25 Change AesKeyLookupTbl initialisation format
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_common_apis Xilsecure Common Apis
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_core.h"
#include "xsecure_aes.h"
#include "xsecure_sha.h"
#include "xsecure_sha_common.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/** This lookup table array defines properties of Key sources.*/
const XSecure_AesKeyLookup AesKeyLookupTbl [XSECURE_MAX_KEY_SOURCES] =
{
	/* BBRAM_KEY */
	[0U] = {
	  .RegOffset = XSECURE_AES_INVALID_CFG,
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_BBRAM_KEY,
	  .UsrWrAllowed = FALSE,
	  .DecAllowed = TRUE,
	  .EncAllowed = TRUE,
	  .KeyDecSrcAllowed = TRUE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearVal = XSECURE_AES_INVALID_CFG
	},

	/* BBRAM_RED_KEY */
	[1U] = {
	  .RegOffset = XSECURE_AES_INVALID_CFG,
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_BBRAM_RD_KEY,
	  .UsrWrAllowed = FALSE,
	  .DecAllowed = TRUE,
	  .EncAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_BBRAM_RED,
	  .KeyClearVal = XSECURE_AES_KEY_CLEAR_BBRAM_RED_KEY_MASK
	},

	/* BH_KEY */
	[2U] = {
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
	[3U] = {
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
	[4U] = {
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
	[5U] = {
	  .RegOffset = XSECURE_AES_INVALID_CFG,
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_RED_KEY,
	  .UsrWrAllowed = FALSE,
	  .DecAllowed = TRUE,
	  .EncAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_EFUSE_RED,
	  .KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK
	},

	/* EFUSE_USER_KEY_0 */
	[6U] = {
	  .RegOffset = XSECURE_AES_INVALID_CFG,
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_USR_KEY0,
	  .UsrWrAllowed = FALSE,
	  .DecAllowed = TRUE,
	  .EncAllowed = TRUE,
	  .KeyDecSrcAllowed = TRUE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_0_MASK
	},

	/* EFUSE_USER_KEY_1 */
	[7U] = {
	  .RegOffset = XSECURE_AES_INVALID_CFG,
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_USR_KEY1,
	  .UsrWrAllowed = FALSE,
	  .DecAllowed = TRUE,
	  .EncAllowed = TRUE,
	  .KeyDecSrcAllowed = TRUE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_1_MASK
	},

	/* EFUSE_USER_RED_KEY_0 */
	[8U] = {
	  .RegOffset = XSECURE_AES_INVALID_CFG,
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY0,
	  .UsrWrAllowed = FALSE,
	  .DecAllowed = TRUE,
	  .EncAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED,
	  .KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_0_MASK
	},

	/* EFUSE_USER_RED_KEY_1 */
	[9U] = {
	  .RegOffset = XSECURE_AES_INVALID_CFG,
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY1,
	  .UsrWrAllowed = FALSE,
	  .DecAllowed = TRUE,
	  .EncAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED,
	  .KeyClearVal = XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_1_MASK
	},

	/* KUP_KEY */
	[10U] = {
	  .RegOffset = XSECURE_AES_INVALID_CFG,
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_KUP_KEY,
	  .UsrWrAllowed = FALSE,
	  .DecAllowed = TRUE,
	  .EncAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearVal = XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK
	},

	/* PUF_KEY */
	[11U] = {
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
	[12U] = {
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
	[13U] {
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
	[14U] = {
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
	[15U] = {
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
	[16U] = {
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
	[17U] = {
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
	[18U] = {
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
	[19U] = {
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
/***************** Macros (Inline Functions) Definitions **********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function configures SSS to AES engine
 *
 * @param	DmaPtr		Pointer to the DMA instance
 * @param 	SssInstance	Pointer to SSS instance
 *
 * @return
 *		 - XST_SUCCESS  On successful configuration
 *		 - XST_FAILURE  On failure
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
	else {
		Status = XSecure_SssAes(SssInstance,
				XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}

	return Status;
}
/** @} */
