/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provision_common.c
 *
 * This file contains the Provisioning code which is common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kp   06/27/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_provision_common_apis Provisioning Common APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_provision_common.h"
#include "xasu_ocpinfo.h"

/********************************* Constant Definitions ******************************************/

/*********************************** Macros Definitions ******************************************/

/*********************************** Type Definitions ********************************************/

/********************************* Variable Definitions ******************************************/

/********************************** Function Prototypes ******************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the USER eFUSE key provisioning parameters.
 *
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure which
 *				holds the USER eFUSE key provisioning input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the parameters are validated successfully.
 *	- XST_FAILURE, if the parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateUserEfuseParams(const XAsu_ProvisionUserEfuseParams *ParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (ParamsPtr == NULL) {
		goto END;
	}

	/** Validate the target eFUSE key identifier. */
	if ((ParamsPtr->EfuseKeyId != XASU_PROVISION_EFUSE_USER_KEY_0) &&
	    (ParamsPtr->EfuseKeyId != XASU_PROVISION_EFUSE_USER_KEY_1)) {
		goto END;
	}

	/** Validate the key length (only 256-bit keys are supported by USER eFUSE keys). */
	if (ParamsPtr->KeyLen != XASU_PROVISION_USER_EFUSE_KEY_SIZE_IN_BYTES) {
		goto END;
	}

	/**
	 * Validate the key source, its corresponding KeyId/KeyAddr fields, and the key
	 * source against the destination eFUSE.
	 */
	switch (ParamsPtr->KeySource) {
		case XASU_PROVISION_KEY_SOURCE_ASU_GENERATED:
			/** ASU generates the key; no KeyId/KeyAddr must be supplied. */
			if ((ParamsPtr->KeyId != 0U) || (ParamsPtr->KeyAddr != 0U)) {
				goto END;
			}
			break;
		case XASU_PROVISION_KEY_SOURCE_VAULT:
			/** Vault source: rejected for eFUSE0; KeyId non-zero, KeyAddr zero. */
			if ((ParamsPtr->EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_0) ||
			    (ParamsPtr->KeyId == 0U) || (ParamsPtr->KeyAddr != 0U)) {
				goto END;
			}
			break;
		case XASU_PROVISION_KEY_SOURCE_USER_BUFFER:
			/** Buffer source: rejected for eFUSE0; KeyAddr non-zero, KeyId zero. */
			if ((ParamsPtr->EfuseKeyId == XASU_PROVISION_EFUSE_USER_KEY_0) ||
			    (ParamsPtr->KeyAddr == 0U) || (ParamsPtr->KeyId != 0U)) {
				goto END;
			}
			break;
		default:
			goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the Unique Device Endorsement (UDE) key provisioning
 *		parameters.
 *
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUdeParams structure which holds the UDE
 *				key provisioning input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the parameters are validated successfully.
 *	- XST_FAILURE, if the parameters validation fails.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionValidateUdeParams(const XAsu_ProvisionUdeParams *ParamsPtr)
{
	volatile s32 Status = XST_FAILURE;

	if (ParamsPtr == NULL) {
		goto END;
	}

	/** Validate the certificate output buffer address and its actual-size output address. */
	if ((ParamsPtr->CertBufAddr == 0U) || (ParamsPtr->CertActualSizeAddr == 0U)) {
		goto END;
	}

	/** Validate the certificate output buffer size. */
	if (ParamsPtr->CertBufSize == 0U) {
		goto END;
	}

	/** Validate the UDE key type (only ECC NIST P-384 is supported currently). */
	if (ParamsPtr->UdeKeyType != XASU_PROVISION_UDE_KEY_TYPE_ECC_P384) {
		goto END;
	}

	/** Validate the target UDE eFUSE slot. */
	if ((ParamsPtr->UdeSlot != XASU_OCP_UDE_USER_KEY_0_ID) &&
	    (ParamsPtr->UdeSlot != XASU_OCP_UDE_USER_KEY_1_ID) &&
	    (ParamsPtr->UdeSlot != XASU_OCP_UDE_USER_KEY_2_ID)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
