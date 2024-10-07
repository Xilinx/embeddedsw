/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aes_common.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the AES function definition which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   10/03/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/***************************** Include Files *****************************************************/
#include "xasu_aes_common.h"
#include "xasu_aesinfo.h"
#include "xasu_def.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates IV for given AES engine mode.
 *
 * @param	EngineMode	AES engine mode.
 * @param	IvAddr		Address of buffer holding IV.
 * @param	IvLen		Lenght of the IV in bytes.
 *
 * @return
 *		- Upon successful validation of IV, it returns XST_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
s32 XAsu_AesValidateIv(u8 EngineMode, u64 IvAddr, u32 IvLen)
{
	s32 Status = XST_FAILURE;

	/*
	 * IV Validation for respective AES engine modes
	 * AES Standard mode (ECB, CBC, CTR, CFB, OFB).
	 * AES MAC mode (GCM, CCM, GMAC, CMAC).
	 *
	 * |   Engine Mode     |   IvAddress    |   IvLength           |
	 * |-------------------|----------------|----------------------|
	 * | AES-ECB, AES-CMAC |     N/A        |      N/A             |
	 * | AES-GCM           |   Non-zero     |  Any non-zero Length |
	 * | Remaining modes   |   Non-zero     |  12 or 16 Bytes      |
	 */
	switch (EngineMode) {
		case XASU_AES_ECB_MODE:
		case XASU_AES_CMAC_MODE:
			Status = XST_SUCCESS;
			break;
		case XASU_AES_GCM_MODE:
			if ((IvAddr != 0U) && (IvLen != 0U)) {
				Status = XST_SUCCESS;
			}
			break;
		case XASU_AES_CBC_MODE:
		case XASU_AES_CFB_MODE:
		case XASU_AES_OFB_MODE:
		case XASU_AES_CTR_MODE:
			if ((IvAddr != 0U) && ((IvLen == XASU_AES_IV_SIZE_96BIT_IN_BYTES) ||
					       (IvLen == XASU_AES_IV_SIZE_128BIT_IN_BYTES))) {
				Status = XST_SUCCESS;
			}
			break;
		default:
			Status = XST_FAILURE;
			break;
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates Tag for given AES engine mode.
 *
 * @param	EngineMode	AES engine mode.
 * @param	TagAddr		Address of the Input/Output Tag.
 * @param	TagLen		Length of Tag in bytes and it will be zero for all AES
 *				standard modes like, ECB, CBC, OFB, CFB, CTR
 *
 * @return
 *		- Upon successful validation of Tag, it returns XST_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
s32 XAsu_AesValidateTag(u8 EngineMode, u64 TagAddr, u32 TagLen)
{
	s32 Status = XST_FAILURE;

	/*
	 * Tag Validation for respective AES engine modes
	 * AES Standard mode (ECB, CBC, CTR, CFB, OFB).
	 * AES MAC mode (GCM, CCM, GMAC, CMAC).
	 *
	 * |   Engine Mode       |   TagAddress   |   TagLength          |
	 * |---------------------|----------------|----------------------|
	 * | Standard mode       |     N/A        |      N/A             |
	 * | AES-GCM, CMAC       |   Non-zero     |  8<=TagLen<=16       |
	 * | AES-CCM             |   Non-zero     |  4,6,8,10,12,14,16   |
	 *
	 * NIST recommends using a tag length of atleast 64 bits to provide adequate protection
	 * against guessing attacks.
	 */
	switch (EngineMode) {
		case XASU_AES_CBC_MODE:
		case XASU_AES_CFB_MODE:
		case XASU_AES_OFB_MODE:
		case XASU_AES_CTR_MODE:
		case XASU_AES_ECB_MODE:
			Status = XST_SUCCESS;
			break;
		case XASU_AES_CCM_MODE:
			if ((TagAddr != 0U) && ((TagLen % XASU_AES_EVEN_MODULUS == 0U) &&
						(TagLen >= XASU_AES_RECOMMENDED_TAG_LENGTH_IN_BYTES) &&
						(TagLen <= XASU_AES_MAX_TAG_LENGTH_IN_BYTES))) {
				Status = XST_SUCCESS;
			}
			break;
		case XASU_AES_GCM_MODE:
		case XASU_AES_CMAC_MODE:
			if ((TagAddr != 0U) && ((TagLen >= XASU_AES_RECOMMENDED_TAG_LENGTH_IN_BYTES) &&
						(TagLen <= XASU_AES_MAX_TAG_LENGTH_IN_BYTES))) {
				Status = XST_SUCCESS;
			}
			break;
		default:
			Status = XST_FAILURE;
			break;
	}
	return Status;
}
/** @} */
