/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*****************************************************************************/

/*****************************************************************************/
/**
*
* @file common/core/xsecure_core.c
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
* 5.7   tbk   04/29/2026 Repack AesKeyLookupTbl entries (drop RegOffset and
*                       compute via XSecure_GetKeyRegOffset, narrow
*                       KeyDecSrcSelVal to u8, replace KeyClearVal with
*                       KeyClearBitPos referencing renamed
*                       XSECURE_AES_KEY_CLEAR_*_BIT_POS macros) for memory
*                       footprint reduction
* 5.8   tvp   09/15/26  Guard AES key lookup table definition
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_common_apis XilSecure Common Apis
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_core.h"
#include "xsecure_aes.h"
#include "xsecure_sha.h"
#include "xsecure_sha_common.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

#if !(defined(VERSAL_2VE_2VM) && defined(VERSAL_2VP_P))
/** This lookup table array defines properties of Key sources.*/
const XSecure_AesKeyLookup AesKeyLookupTbl [XSECURE_MAX_KEY_SOURCES] =
{
	/** - BBRAM_KEY */
	[0U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_BBRAM_KEY,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = TRUE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_INVALID_CFG
	},

	/** - BBRAM_RED_KEY */
	[1U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_BBRAM_RD_KEY,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_BBRAM_RED,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_BBRAM_RED_KEY_BIT_POS
	},

	/** - BH_KEY */
	[2U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_BH_KEY,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = TRUE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_BH_KEY_BIT_POS
	},

	/** - BH_RED_KEY */
	[3U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_BH_RD_KEY,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_BH_RED,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_BH_RED_KEY_BIT_POS
	},

	/** - EFUSE_KEY */
	[4U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_KEY,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = TRUE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_EFUSE_KEY_BIT_POS
	},

	/** - EFUSE_RED_KEY */
	[5U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_RED_KEY,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_EFUSE_RED,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_BIT_POS
	},

	/** - EFUSE_USER_KEY_0 */
	[6U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_USR_KEY0,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = TRUE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_0_BIT_POS
	},

	/** - EFUSE_USER_KEY_1 */
	[7U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_USR_KEY1,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = TRUE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_1_BIT_POS
	},

	/** - EFUSE_USER_RED_KEY_0 */
	[8U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY0,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_0_BIT_POS
	},

	/** - EFUSE_USER_RED_KEY_1 */
	[9U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY1,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_1_BIT_POS
	},

	/** - KUP_KEY */
	[10U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_KUP_KEY,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_KUP_KEY_BIT_POS
	},

	/** - PUF_KEY */
	[11U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_PUF_KEY,
	  .UsrWrAllowed = FALSE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_PUF_KEY_BIT_POS
	},

	/** - USER_KEY_0 */
	[12U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_0,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_USER_KEY_0_BIT_POS
	},

	/** - USER_KEY_1 */
	[13U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_1,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_USER_KEY_1_BIT_POS
	},

	/** - USER_KEY_2 */
	[14U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_2,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_USER_KEY_2_BIT_POS
	},

	/** - USER_KEY_3 */
	[15U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_3,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_USER_KEY_3_BIT_POS
	},

	/** - USER_KEY_4 */
	[16U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_4,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_USER_KEY_4_BIT_POS
	},

	/** - USER_KEY_5 */
	[17U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_5,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_USER_KEY_5_BIT_POS
	},

	/** - USER_KEY_6 */
	[18U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_6,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_USER_KEY_6_BIT_POS
	},

	/** - USER_KEY_7 */
	[19U] = {
	  .KeySrcSelVal = XSECURE_AES_KEY_SEL_USR_KEY_7,
	  .UsrWrAllowed = TRUE,
	  .KeyDecSrcAllowed = FALSE,
	  .KeyDecSrcSelVal = XSECURE_AES_INVALID_CFG,
	  .KeyClearBitPos = XSECURE_AES_KEY_CLEAR_USER_KEY_7_BIT_POS
	}
};
#endif
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
