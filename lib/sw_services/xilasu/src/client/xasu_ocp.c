/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocp.c
 *
 * This file contains the implementation of the client interface functions for
 * OCP driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  08/04/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ocp_client_apis OCP Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_def.h"
#include "xasu_ocp.h"
#include "xasu_ocp_common.h"
#include "xasu_ocpinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform DevIk certificate generation
 *		operation.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	OcpCertClientParamPtr	Pointer to XAsu_OcpCertParams structure which holds the
 *					parameters of OCP input arguments.
 *
 * @return
 *	- XST_SUCCESS, if IPI request to ASUFW is sent successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_OcpGetDevIkX509Certificate(XAsu_ClientParams *ClientParamPtr,
				    XAsu_OcpCertParams *OcpCertClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate OCP client parameter. */
	if (OcpCertClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameter. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate OCP client parameter inputs. */
	Status = XAsu_OcpValidateCertParams(OcpCertClientParamPtr);
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
	Header = XAsu_CreateHeader(XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID, UniqueId,
				   XASU_MODULE_OCP_ID, 0U, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, OcpCertClientParamPtr,
					(u32)(sizeof(XAsu_OcpCertParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform DevAk certificate generation
 *		operation.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	OcpCertClientParamPtr	Pointer to XAsu_OcpCertParams structure which holds the
 *					parameters of OCP input arguments.
 *
 * @return
 *	- XST_SUCCESS, if IPI request to ASUFW is sent successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_OcpGetDevAkX509Certificate(XAsu_ClientParams *ClientParamPtr,
				    XAsu_OcpCertParams *OcpCertClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate OCP client parameter. */
	if (OcpCertClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameter. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate OCP client parameter inputs. */
	Status = XAsu_OcpValidateCertParams(OcpCertClientParamPtr);
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
	Header = XAsu_CreateHeader(XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID, UniqueId,
				   XASU_MODULE_OCP_ID, 0U, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, OcpCertClientParamPtr,
					(u32)(sizeof(XAsu_OcpCertParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform CSR(Certificate Signing Request)
 *		generation operation.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	OcpCertClientParamPtr	Pointer to XAsu_OcpCertParams structure which holds the
 *					parameters of OCP input arguments.
 *
 * @return
 *	- XST_SUCCESS, if IPI request to ASUFW is sent successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_OcpGetDevIkCsr(XAsu_ClientParams *ClientParamPtr,
			XAsu_OcpCertParams *OcpCertClientParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate OCP client parameter. */
	if (OcpCertClientParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameter. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate OCP client parameter inputs. */
	Status = XAsu_OcpValidateCertParams(OcpCertClientParamPtr);
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
	Header = XAsu_CreateHeader(XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID, UniqueId,
				   XASU_MODULE_OCP_ID, 0U, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, OcpCertClientParamPtr,
					(u32)(sizeof(XAsu_OcpCertParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform DevAk attestation operation.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	OcpDevAkAttestParamPtr	Pointer to XAsu_OcpDevAkAttest structure which holds the
 *					parameters of OCP input arguments.
 *
 * @return
 *	- XST_SUCCESS, if IPI request to ASUFW is sent successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_OcpDevAkAttestation(XAsu_ClientParams *ClientParamPtr,
			     XAsu_OcpDevAkAttest *OcpDevAkAttestParamPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate OCP client parameter. */
	if (OcpDevAkAttestParamPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate OCP client parameter inputs. */
	Status = XAsu_OcpValidateAttestParams(OcpDevAkAttestParamPtr);
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
	Header = XAsu_CreateHeader(XASU_OCP_DEVAK_ATTESTATION_CMD_ID, UniqueId, XASU_MODULE_OCP_ID,
				   0U, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, OcpDevAkAttestParamPtr,
					(u32)(sizeof(XAsu_OcpDevAkAttest)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to generate DME challenge response.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	OcpDmeResponseAddr	Address of the XAsu_OcpDmeResponse structure.
 *
 * @return
 *	- XST_SUCCESS, if IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 	- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_OcpDmeChallengeReq(XAsu_ClientParams *ClientParamPtr, XAsu_OcpDmeParams *OcpDmeParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate OCP client parameters. */
	if ((OcpDmeParamsPtr->NonceAddr == 0U) || (OcpDmeParamsPtr->OcpDmeResponseAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

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
	Header = XAsu_CreateHeader(XASU_OCP_DME_CHALLENGE_REQ_CMD_ID, UniqueId, XASU_MODULE_OCP_ID,
				   0U, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, OcpDmeParamsPtr,
					(u32)(sizeof(XAsu_OcpDmeParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform DevAk attestation operation.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	OcpDmeKeyEnc		Pointer to XAsu_OcpDmeKeyEncrypt structure which holds the
 *					parameters of DME key arguments.
 *
 * @return
 *	- XST_SUCCESS, if IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 	- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_OcpDmeKeysEncrypt(XAsu_ClientParams *ClientParamPtr, XAsu_OcpDmeKeyEncrypt *OcpDmeKeyEnc)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate OCP client parameter. */
	if (OcpDmeKeyEnc == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((OcpDmeKeyEnc->DmePvtKeyAddr == 0U) || (OcpDmeKeyEnc->DmeEncPvtKeyAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((OcpDmeKeyEnc->DmeKeyId != XASU_OCP_DME_USER_KEY_0_ID) &&
	    (OcpDmeKeyEnc->DmeKeyId != XASU_OCP_DME_USER_KEY_1_ID) &&
	    (OcpDmeKeyEnc->DmeKeyId != XASU_OCP_DME_USER_KEY_2_ID)) {
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
	Header = XAsu_CreateHeader(XASU_OCP_DME_PVT_KEYS_ENCRYPT_CMD_ID, UniqueId, XASU_MODULE_OCP_ID,
				   0U, ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_UpdateQueueBufferNSendIpi(ClientParamPtr, OcpDmeKeyEnc,
					(u32)(sizeof(XAsu_OcpDmeKeyEncrypt)), Header);

END:
	return Status;
}
/** @} */
