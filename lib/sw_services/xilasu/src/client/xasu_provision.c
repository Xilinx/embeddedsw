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

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to provision a Unique Device Endorsement
 *		(UDE) key pair.
 *
 *		The UDE key pair (ECC NIST P-384) is generated inside the ASU so the private key is
 *		never exposed outside the ASU. The private key is encrypted with the PUF KEK and
 *		programmed into the requested eFUSE slot by the ASU (via the PLM), and a self-signed
 *		X.509 certificate over the generated public key is returned to the caller at
 *		ParamsPtr->CertBufAddr with its actual size at ParamsPtr->CertActualSizeAddr.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	ParamsPtr	Pointer to the XAsu_ProvisionUdeParams structure which holds the UDE
 *				key provisioning input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *	- XASU_INVALID_UNIQUE_ID, if the received Queue ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_ProvisionUde(XAsu_ClientParams *ClientParamPtr, XAsu_ProvisionUdeParams *ParamsPtr)
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

	/** Validate the UDE provisioning input parameters. */
	Status = XAsu_ProvisionValidateUdeParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function, providing the caller's
	 * CertActualSizeAddr as the response buffer so the certificate size returned by the server
	 * is copied there during the callback.
	 */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)ParamsPtr->CertActualSizeAddr,
						XASU_PROVISION_UDE_CERT_ACTUAL_SIZE_IN_BYTES,
						XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_UDE_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID,
				   (u8)(sizeof(XAsu_ProvisionUdeParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ParamsPtr,
				   (u32)(sizeof(XAsu_ProvisionUdeParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to generate and return the Device Provisioning
 *		Certificate (DPC) for the calling subsystem (SAS section 12.16.2).
 *
 *		The ASU checks the internal exportable key vault for the subsystem's provisioning
 *		authentication key pair (ECC NIST P-384) and provisioning wrapping key pair (ECIES
 *		over NIST P-384). Any pair that is missing is generated with the TRNG and stored in
 *		the exportable vault. A UDE-signed X.509 certificate is generated over each public
 *		key, the DPC container (Device DNA, both certificates and a UDE signature over them)
 *		is assembled and returned to the caller at ParamsPtr->DpcBufAddr with its actual size
 *		at ParamsPtr->DpcActualSizeAddr.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 * @param	ParamsPtr	Pointer to the XAsu_DevProvisionCertParams structure which holds the
 *				DPC generation input arguments.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 *	- XASU_INVALID_UNIQUE_ID, if the received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_GetDevProvisionCert(XAsu_ClientParams *ClientParamPtr,
			     XAsu_DevProvisionCertParams *ParamsPtr)
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

	/** Validate the DPC generation input parameters. */
	Status = XAsu_ProvisionValidateDpcParams(ParamsPtr);
	if (Status != XST_SUCCESS) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/**
	 * Generate a unique ID and register the callback function, providing the caller's
	 * DpcActualSizeAddr as the response buffer so the DPC size returned by the server is
	 * copied there during the callback.
	 */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr,
						(u8 *)(UINTPTR)ParamsPtr->DpcActualSizeAddr,
						XASU_PROVISION_DPC_ACTUAL_SIZE_IN_BYTES,
						XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_GET_DPC_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID,
				   (u8)(sizeof(XAsu_DevProvisionCertParams) /
					XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, ParamsPtr,
				   (u32)(sizeof(XAsu_DevProvisionCertParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to ASUFW to delete the calling subsystem's
 *		provisioning key pairs backing the Device Provisioning Certificate (SAS section
 *		12.16.3).
 *
 *		The provisioning authentication and wrapping key pairs are removed from the internal
 *		exportable key vault so that the next XAsu_GetDevProvisionCert call regenerates them.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 *
 * @return
 *	- XST_SUCCESS, if the IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_UNIQUE_ID, if the received Queue ID is invalid.
 *
 *************************************************************************************************/
s32 XAsu_DeleteDevProvisionCert(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, NULL, 0U, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_PROVISION_DELETE_DPC_CMD_ID, UniqueId,
				   XASU_MODULE_PROVISION_ID, 0U,
				   ClientParamPtr->SecureFlag);

	/** Send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}

/** @} */
