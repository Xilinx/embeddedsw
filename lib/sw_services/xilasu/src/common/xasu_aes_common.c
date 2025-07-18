/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aes_common.c
 *
 * This file contains the AES function definition which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   10/03/24 Initial release
 * 1.1   am   01/20/25 Added AES CCM Iv validation
 *       am   03/14/25 Renamed XAsu_AesValidateIv() to XAsu_AesValidateIvParams() and
 *                     XAsu_AesValidateTag() to XAsu_AesValidateTagParams()
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_aes_common_apis AES Common APIs
 * @{
*/

/***************************** Include Files *****************************************************/
#include "xasu_aes_common.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the IV parameters for the specified AES engine mode.
 *
 * @param	EngineMode	AES engine mode.
 * @param	IvAddr		Address of buffer holding IV.
 * @param	IvLen		Length of the IV in bytes.
 *
 * @return
 *		- XST_SUCCESS, if IV parameters are validated successfully.
 *		- XST_FAILURE, if IV parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_AesValidateIvParams(u8 EngineMode, u64 IvAddr, u32 IvLen)
{
	s32 Status = XST_FAILURE;

	/**
	 * IV Validation for respective AES engine modes
	 * AES Standard mode (ECB, CBC, CTR, CFB, OFB).
	 * AES MAC mode (GCM, CCM, GMAC, CMAC).
	 *
	 * |   Engine Mode     |   IvAddress    |   IvLength           |
	 * |-------------------|----------------|----------------------|
	 * | AES-ECB, AES-CMAC |     N/A        |      N/A             |
	 * | AES-GCM           |   Non-zero     |  Any non-zero Length |
	 * | AES-CCM           |   Non-zero     |  7 to 13 bytes       |
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
		case XASU_AES_CCM_MODE:
			if ((IvAddr != 0U) && ((IvLen >= XASU_AES_CCM_MIN_NONCE_LEN) &&
					(IvLen <= XASU_AES_CCM_MAX_NONCE_LEN))) {
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
 * @brief	This API Validates the tag parameters for the specified AES engine mode.
 *
 * @param	EngineMode	AES engine mode.
 * @param	TagAddr		Address of the Input/Output Tag buffer.
 * @param	TagLen		Length of the tag in bytes. It will be zero for all AES
 *				standard modes like, ECB, CBC, OFB, CFB, CTR
 *
 * @return
 *		- XST_SUCCESS, if the tag parameters are successfully validated.
 *		- XST_FAILURE, if the tag parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_AesValidateTagParams(u8 EngineMode, u64 TagAddr, u32 TagLen)
{
	s32 Status = XST_FAILURE;

	/**
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
	 * NIST recommends using a tag length of at least 64 bits to provide adequate protection
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
			if ((TagAddr != 0U) && (((TagLen % XASU_AES_EVEN_MODULUS) == 0U) &&
						(TagLen >= XASU_AES_MIN_TAG_LENGTH_IN_BYTES) &&
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
