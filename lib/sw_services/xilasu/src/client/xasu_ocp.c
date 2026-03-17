/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasu_generic.h"

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
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
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
				   XASU_MODULE_OCP_ID,
				   (u8)(sizeof(XAsu_OcpCertParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, OcpCertClientParamPtr,
					(u32)(sizeof(XAsu_OcpCertParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform DevAk certificate generation
 *		operation.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
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
				   XASU_MODULE_OCP_ID,
				   (u8)(sizeof(XAsu_OcpCertParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, OcpCertClientParamPtr,
					(u32)(sizeof(XAsu_OcpCertParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform CSR(Certificate Signing Request)
 *		generation operation.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
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
				   XASU_MODULE_OCP_ID,
				   (u8)(sizeof(XAsu_OcpCertParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, OcpCertClientParamPtr,
					(u32)(sizeof(XAsu_OcpCertParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform DevAk attestation operation.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
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
	Header = XAsu_CreateHeader(XASU_OCP_DEVAK_ATTESTATION_CMD_ID, UniqueId,
				   XASU_MODULE_OCP_ID,
				   (u8)(sizeof(XAsu_OcpDevAkAttest) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, OcpDevAkAttestParamPtr,
					(u32)(sizeof(XAsu_OcpDevAkAttest)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to generate UDE challenge response.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	OcpUdeParamsPtr		Pointer to the XAsu_OcpUdeParams structure.
 *
 * @return
 *	- XST_SUCCESS, if IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 	- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_OcpUdeChallengeReq(XAsu_ClientParams *ClientParamPtr, XAsu_OcpUdeParams *OcpUdeParamsPtr)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate OCP client parameters. */
	if (OcpUdeParamsPtr == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if (OcpUdeParamsPtr->OcpUdeResponseAddr == 0U) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate the Nonce buffer to be non-zero. */
	Status = XAsu_IsBufferNonZero((u8 *)(UINTPTR)OcpUdeParamsPtr->NonceAddr,
		XASU_OCP_UDE_NONCE_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
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
	Header = XAsu_CreateHeader(XASU_OCP_UDE_CHALLENGE_REQ_CMD_ID, UniqueId,
				   XASU_MODULE_OCP_ID,
				   (u8)(sizeof(XAsu_OcpUdeParams) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, OcpUdeParamsPtr,
					(u32)(sizeof(XAsu_OcpUdeParams)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to ASUFW to perform UDE key encryption operation.
 *
 * @param	ClientParamPtr		Pointer to the XAsu_ClientParams structure which holds the
 *					client input parameters.
 * @param	OcpUdeKeyEnc		Pointer to XAsu_OcpUdeKeyEncrypt structure which holds the
 *					parameters of UDE key arguments.
 *
 * @return
 *	- XST_SUCCESS, if IPI request to ASUFW is sent successfully.
 *	- XASU_INVALID_ARGUMENT, if any argument is invalid.
 * 	- XASU_INVALID_UNIQUE_ID, if received Queue ID is invalid.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsu_OcpUdeKeysEncrypt(XAsu_ClientParams *ClientParamPtr, XAsu_OcpUdeKeyEncrypt *OcpUdeKeyEnc)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate OCP client parameter. */
	if (OcpUdeKeyEnc == NULL) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Validate client parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((OcpUdeKeyEnc->UdePvtKeyAddr == 0U) || (OcpUdeKeyEnc->UdeEncPvtKeyAddr == 0U)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	if ((OcpUdeKeyEnc->UdeKeyId != XASU_OCP_UDE_USER_KEY_0_ID) &&
	    (OcpUdeKeyEnc->UdeKeyId != XASU_OCP_UDE_USER_KEY_1_ID) &&
	    (OcpUdeKeyEnc->UdeKeyId != XASU_OCP_UDE_USER_KEY_2_ID)) {
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
	Header = XAsu_CreateHeader(XASU_OCP_UDE_PVT_KEYS_ENCRYPT_CMD_ID, UniqueId,
				   XASU_MODULE_OCP_ID,
				   (u8)(sizeof(XAsu_OcpUdeKeyEncrypt) / XASU_WORD_LEN_IN_BYTES),
				   ClientParamPtr->SecureFlag);

	/** Update request buffer and send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, OcpUdeKeyEnc,
					(u32)(sizeof(XAsu_OcpUdeKeyEncrypt)), Header);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to get the Hardware Unique Key.
 *
 * @param	ClientParamPtr is a pointer to XAsu_ClientParams which has parameters for the
 * 		client request.
 * @param	HukBuf is a pointer to a buffer to hold the 256-bit Hardware Unique Key.
 * @param	BufLen is the length of the buffer pointed to by HukBuf. It should not be less than
 * 		XASU_OCP_HUK_SIZE_IN_BYTES (32 bytes).
 *
 * @return
 *		- XST_SUCCESS, if the function finishes successfully.
 *		- XASU_INVALID_ARGUMENT, if invalid parameters used.
 *		- XASU_INVALID_UNIQUE_ID, if failed to get unique ID.
 *		- XST_FAILURE, if sending an IPI request to ASU fails.
 *
 *************************************************************************************************/
s32 XAsu_OcpGetHuk(XAsu_ClientParams *ClientParamPtr, u8 *HukBuf, u32 BufLen)
{
	s32 Status = XST_FAILURE;
	u32 Header;
	u8 UniqueId;

	/** Validate input parameters. */
	Status = XAsu_ValidateClientParameters(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((HukBuf == NULL) || (BufLen < XASU_OCP_HUK_SIZE_IN_BYTES)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	/** Generate a unique ID and register the callback function. */
	UniqueId = XAsu_RegCallBackNGetUniqueId(ClientParamPtr, HukBuf,
							XASU_OCP_HUK_SIZE_IN_BYTES, XASU_TRUE);
	if (UniqueId >= XASU_UNIQUE_ID_MAX) {
		Status = XASU_INVALID_UNIQUE_ID;
		goto END;
	}

	/** Create command header. */
	Header = XAsu_CreateHeader(XASU_OCP_GET_HUK_CMD_ID, UniqueId, XASU_MODULE_OCP_ID,
					   XASU_CMD_LEN_ZERO, ClientParamPtr->SecureFlag);

	/** Send an IPI request to ASUFW. */
	Status = XAsu_SendCmdToAsu(ClientParamPtr, NULL, 0U, Header);

END:
	return Status;
}
/** @} */
