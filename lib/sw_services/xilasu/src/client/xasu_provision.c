/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provision.c
 *
 * This file contains the implementation of the client interface functions for the ASU
 * Provisioning interface.
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
 * @addtogroup xasu_provision_client_apis Provisioning Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_def.h"
#include "xasu_provision.h"
#include "xasu_provision_common.h"
#include "xasu_provisioninfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to program a USER eFUSE key.
 *
 *		The key to be programmed can be sourced from the AKV, generated internally by the
 *		ASU using the TRNG (benign provisioning), or supplied directly by the user. It is
 *		always stored in the eFUSE in black (PUF KEK encrypted) form.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUserEfuseParams structure which holds
 *				the USER eFUSE key provisioning input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *	- XASU_INVALID_UNIQUE_ID, if the received Queue ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionUserEfuse(XAsu_ClientParams *ClientParamPtr,
			    XAsu_ProvisionUserEfuseParams *ParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate the provisioning input parameter pointer. */
	if (ParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the USER eFUSE provisioning input parameters. */
	Status = XAsu_ProvisionValidateUserEfuseParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_USER_EFUSE_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID,
				   (u8)(sizeof(XAsu_ProvisionUserEfuseParams) /
					XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ParamsPtr,
				  (u32)(sizeof(XAsu_ProvisionUserEfuseParams)), Header);

END:
	return Status;
}
/** @} */
