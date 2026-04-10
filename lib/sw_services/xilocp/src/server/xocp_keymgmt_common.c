/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_keymgmt_common.c
*
* This file contains the XilOcp KeyMgmt Server APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.7   rmv  01/30/26 Refactor OCP library
*       rpu  03/11/26 Validate input parameters
*       rpu  02/18/26 Fixed Doxygen warnings
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xilocp_keymgmt_server_apis XilOcp KeyMgmt Server APIs
 * @{
 */
/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP_KEY_MGMT
#include "xil_types.h"
#include "xocp_generic.h"
#include "xocp_common.h"
#include "xsecure_init.h"
#include "xsecure_kat.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_plat_kat.h"
#include "xocp_keymgmt_common.h"
#include "xocp_plat.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function validates the DICE CDI stored in PMC global register.
 *
 * @return
 *      - XST_SUCCESS, if DICE CDI is validated successfully.
 *      - XOCP_DICE_CDI_SEED_ZERO, if DICE CDI seed value is zero.
 *      - XOCP_DICE_CDI_PARITY_ERROR, if DICE CDI parity is not zero.
 *      - XOCP_ERR_GLITCH_DETECTED, if DICE CDI size is invalid.
 *
 ******************************************************************************/
int XOcp_ValidateDiceCdi(void)
{
	volatile int Status = (int)XOCP_DICE_CDI_SEED_ZERO;
	volatile u32 Index;
	volatile u32 CdiParity = 0U;
	volatile u32 CdiParityTmp = 0U;
	XOcp_RegSpace* XOcp_Reg = XOcp_GetRegSpace();

	/** - Upon DICE CDI SEED zeroize, if CDI valid bit is not cleared in Versal Net.
	 *  Check whether DICE CDI SEED is non zero or not.
	 */
	for (Index = 0U; Index < XOCP_CDI_SIZE_IN_WORDS; Index++) {
		if (XPlmi_In32(XOcp_Reg->DiceCdiSeedAddr +
			(Index * XSECURE_WORD_LEN)) != 0x0U) {
			CdiParity = XPlmi_In32(XOcp_Reg->DiceCdiSeedParityAddr) &
				XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_ERROR_MASK;
			CdiParityTmp = XPlmi_In32(XOcp_Reg->DiceCdiSeedParityAddr) &
				XOCP_PMC_GLOBAL_DICE_CDI_SEED_PARITY_ERROR_MASK;
			if ((CdiParity != 0x0U) || (CdiParityTmp != 0x0U)) {
				Status = (int)XOCP_DICE_CDI_PARITY_ERROR;
				goto END;
			} else {
				Status = XST_SUCCESS;
				break;
			}
		}
	}
	if (Index > XOCP_CDI_SIZE_IN_WORDS) {
		Status = (int)XOCP_ERR_GLITCH_DETECTED;
	} else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the seed for DEVAK key generation.
 *
 * @param	CdiAddr holds the address of key to be used for HMAC.
 * @param	CdiLen specifies the length of the key.
 * @param	DataAddr holds the data address to be updated to HMAC.
 * @param	DataLen specifies the length of the data to be updated to HMAC.
 * @param	Out is a pointer of type XSecure_HmacRes where the resultant gets
 *		updated.
 *
 * @return
 *	- XST_SUCCESS, if DevAk seed generated successfully.
 *	- XST_FAILURE, in case of failure.
 *
 ******************************************************************************/
int XOcp_KeyGenDevAkSeed(u32 CdiAddr, u32 CdiLen, u32 DataAddr, u32 DataLen, XSecure_HmacRes *Out)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int RetStatus = XST_GLITCH_ERROR;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XSecure_Hmac HmacInstance;

	/** - Validate input parameters */
	if ((Out == NULL) || (CdiAddr == 0U) || (CdiLen == 0U) || (DataAddr == 0U) ||
		(DataLen == 0U)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	if (XPlmi_IsKatRan(XPLMI_SECURE_SHA3_KAT_MASK) != TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp,
				XSecure_Sha3Kat, Sha3InstPtr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status |= StatusTmp;
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_SHA3_KAT_MASK);
	}

	if (XPlmi_IsKatRan(XPLMI_SECURE_HMAC_KAT_MASK) != TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp,
				XSecure_HmacKat, Sha3InstPtr);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status |= StatusTmp;
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_HMAC_KAT_MASK);
	}

	Status = XST_FAILURE;
	Status = XSecure_HmacInit(&HmacInstance, Sha3InstPtr, CdiAddr, CdiLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_HmacUpdate(&HmacInstance, (u64)DataAddr, DataLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_HmacFinal(&HmacInstance, Out);
	RetStatus = Status;

END:
	if ((RetStatus != XST_SUCCESS) && (Status != XST_SUCCESS)) {
		RetStatus = Status;
	}

	return RetStatus;
}

#endif /* PLM_OCP_KEY_MGMT */
/** @} */
