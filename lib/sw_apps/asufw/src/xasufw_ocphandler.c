/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_ocphandler.c
 *
 * This file contains the OCP module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  07/16/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW server functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "x509_asufw.h"
#include "x509_cert.h"
#include "xasu_ocp_common.h"
#include "xasu_ocpinfo.h"
#include "xasufw_cmd.h"
#include "xasufw_ipi.h"
#include "xasufw_modules.h"
#include "xasufw_ocphandler.h"
#include "xasufw_queuescheduler.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xocp.h"
#include "xsha_hw.h"
#include "xocp_dme.h"

#ifdef XASU_OCP_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XOCP_X509_CERT_FIELD_LEN		(4U)	/**< Certificate size field length */

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_OcpDevIkCsrX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpDevIkX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpDevAkX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpDevAkAttestation(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpDmeChallengeReq(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpDmePvtKeysEncrypt(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_OcpModule;	/**< ASUFW OCP module instance */

/*************************************************************************************************/
/**
 * @brief	This function initializes OCP module.
 *
 * @return
 *	- XASUFW_SUCCESS, if OCP module is initialized successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_X509_MODULE_INIT_FAIL, if X.509 module is not initialized.
 *	- XASUFW_OCP_MODULE_REGISTRATION_FAIL, if module registration is failed.
 *
 *************************************************************************************************/
s32 XAsufw_OcpInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_OcpCmds array contains the list of commands supported by OCP module. */
	static const XAsufw_ModuleCmd XAsufw_OcpCmds[] = {
		[XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDevIkX509CertGen),
		[XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDevAkX509CertGen),
		[XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDevIkCsrX509CertGen),
		[XASU_OCP_DEVAK_ATTESTATION_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDevAkAttestation),
		[XASU_OCP_DME_CHALLENGE_REQ_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDmeChallengeReq),
		[XASU_OCP_DME_PVT_KEYS_ENCRYPT_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDmePvtKeysEncrypt),
	};

	/**
	 * The XAsufw_OcpResources contains the required resources for each supported
	 * command.
	 */
	static XAsufw_ResourcesRequired XAsufw_OcpResources[XASUFW_ARRAY_SIZE(XAsufw_OcpCmds)] = {
		[XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_SHA3_RESOURCE_MASK |
			XASUFW_ECC_RESOURCE_MASK,
		[XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK |	XASUFW_SHA3_RESOURCE_MASK |
			XASUFW_ECC_RESOURCE_MASK,
		[XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_SHA3_RESOURCE_MASK |
			XASUFW_ECC_RESOURCE_MASK,
		[XASU_OCP_DEVAK_ATTESTATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_OCP_DME_CHALLENGE_REQ_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_AES_RESOURCE_MASK |
			XASUFW_SHA2_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_OCP_DME_PVT_KEYS_ENCRYPT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK,
	};

	/** The XAsufw_OcpAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_OcpAccessPermBuf[XASUFW_ARRAY_SIZE(XAsufw_OcpCmds)] = {
		[XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID),
		[XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID),
		[XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID),
		[XASU_OCP_DEVAK_ATTESTATION_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_DEVAK_ATTESTATION_CMD_ID),
		[XASU_OCP_DME_CHALLENGE_REQ_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_DME_CHALLENGE_REQ_CMD_ID),
		[XASU_OCP_DME_PVT_KEYS_ENCRYPT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_DME_PVT_KEYS_ENCRYPT_CMD_ID),
	};

	XAsufw_OcpModule.Id = XASU_MODULE_OCP_ID;
	XAsufw_OcpModule.Cmds = XAsufw_OcpCmds;
	XAsufw_OcpModule.ResourcesRequired = XAsufw_OcpResources;
	XAsufw_OcpModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_OcpCmds);
	XAsufw_OcpModule.ResourceHandler = XAsufw_OcpResourceHandler;
	XAsufw_OcpModule.AsuDmaPtr = NULL;
	XAsufw_OcpModule.ShaPtr = NULL;
	XAsufw_OcpModule.AccessPermBufferPtr = XAsufw_OcpAccessPermBuf;

	/** Initialize X.509 module. */
	Status = X509_CfgInitialize();
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_X509_MODULE_INIT_FAIL);
		goto END;
	}

	/** Register OCP module. */
	Status = XAsufw_ModuleRegister(&XAsufw_OcpModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_MODULE_REGISTRATION_FAIL);
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is handler for DevIk CSR(Certificate Signing Request)
 *		generation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if DevIk CSR is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *	- XASUFW_OCP_X509_DEVIK_CSR_GEN_FAIL, if CSR generation is failed.
 *	- XASUFW_OCP_INVALID_BUF_SIZE, if buffer size is not sufficient.
 *	- XASUFW_DMA_COPY_FAIL, if DMA transfer is failed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDevIkCsrX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpCertParams *OcpCertParam = (const XAsu_OcpCertParams *)ReqBuf->Arg;
	X509_PlatData PlatData;
	XOcp_CertData CertData;
	u32 CertSize;
	u8 CertBuf[X509_CERTIFICATE_MAX_SIZE_IN_BYTES];

	/** Validate client parameter. */
	Status = XAsu_OcpValidateCertParams(OcpCertParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Assign DMA and SHA instance pointers. */
	PlatData.DmaPtr = XAsufw_OcpModule.AsuDmaPtr;
	PlatData.ShaPtr = XAsufw_OcpModule.ShaPtr;

	/** Initialized certificate data. */
	CertData.DevKeyType = OcpCertParam->DevKeySel;
	CertData.CertAddr = (u64)(UINTPTR)CertBuf;
	CertData.CertMaxSize = X509_CERTIFICATE_MAX_SIZE_IN_BYTES;
	CertData.CertActualSize = &CertSize;

	/** Generate DevIk CSR(Certificate Signing Request) for ASU subsystem. */
	Status = XOcp_GetX509Cert(XASUFW_SUBSYTEM_ID, &CertData, (void *)(UINTPTR)&PlatData,
				  XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_X509_DEVIK_CSR_GEN_FAIL);
		goto END;
	}

	/** Copy generated CSR to destination address using DMA. */
	if (OcpCertParam->CertBufLen < CertSize) {
		Status = XASUFW_OCP_INVALID_BUF_SIZE;
		goto END;
	}

	Status = XAsufw_DmaXfr(XAsufw_OcpModule.AsuDmaPtr, (u64)(UINTPTR)CertBuf,
			       OcpCertParam->CertBufAddr, CertSize, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_DMA_COPY_FAIL);
		goto END;
	}

	/** Copy actual size of CSR to destination address using DMA. */
	Status = XAsufw_DmaXfr(XAsufw_OcpModule.AsuDmaPtr, (UINTPTR)&CertSize,
			       (UINTPTR)OcpCertParam->CertActualSize, XOCP_X509_CERT_FIELD_LEN, 0U);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_OcpModule.AsuDmaPtr = NULL;
	XAsufw_OcpModule.ShaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is handler for DevIk certificate generation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if DevIk certificate is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *	- XASUFW_OCP_X509_DEVIK_CERT_GEN_FAIL, if DevIk certificate generation is failed.
 *	- XASUFW_OCP_INVALID_BUF_SIZE, if buffer size is not sufficient.
 *	- XASUFW_DMA_COPY_FAIL, if DMA transfer is failed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDevIkX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpCertParams *OcpCertParam = (const XAsu_OcpCertParams *)ReqBuf->Arg;
	X509_PlatData PlatData;
	XOcp_CertData CertData;
	u32 CertSize;
	u8 CertBuf[X509_CERTIFICATE_MAX_SIZE_IN_BYTES];

	/** Validate client parameter. */
	Status = XAsu_OcpValidateCertParams(OcpCertParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Assign DMA and SHA instance pointers. */
	PlatData.DmaPtr = XAsufw_OcpModule.AsuDmaPtr;
	PlatData.ShaPtr = XAsufw_OcpModule.ShaPtr;

	/** Initialized certificate data. */
	CertData.DevKeyType = OcpCertParam->DevKeySel;
	CertData.CertAddr = (u64)(UINTPTR)CertBuf;
	CertData.CertMaxSize = X509_CERTIFICATE_MAX_SIZE_IN_BYTES;
	CertData.CertActualSize = &CertSize;

	/** Generate DevIk certificate for ASU subsystem. */
	Status = XOcp_GetX509Cert(XASUFW_SUBSYTEM_ID, &CertData, (void *)(UINTPTR)&PlatData,
				  XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_X509_DEVIK_CERT_GEN_FAIL);
		goto END;
	}

	/** Copy generated certificate to destination address using DMA. */
	if (OcpCertParam->CertBufLen < CertSize) {
		Status = XASUFW_OCP_INVALID_BUF_SIZE;
		goto END;
	}

	Status = XAsufw_DmaXfr(XAsufw_OcpModule.AsuDmaPtr, (u64)(UINTPTR)CertBuf,
			       OcpCertParam->CertBufAddr, CertSize, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_DMA_COPY_FAIL);
		goto END;
	}

	/** Copy actual size of generated certificate to destination address using DMA. */
	Status = XAsufw_DmaXfr(XAsufw_OcpModule.AsuDmaPtr, (UINTPTR)&CertSize,
			       (UINTPTR)OcpCertParam->CertActualSize, XOCP_X509_CERT_FIELD_LEN, 0U);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_OcpModule.AsuDmaPtr = NULL;
	XAsufw_OcpModule.ShaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is handler for DevAk certificate generation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if DevAk certificate is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_INVALID_SUBSYSTEM_ID, if subsystem id is not retrieved.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *	- XASUFW_OCP_X509_DEVAK_CERT_GEN_FAIL, if DevAk certificate generation is failed.
 *	- XASUFW_OCP_INVALID_BUF_SIZE, if buffer size is not sufficient.
 *	- XASUFW_DMA_COPY_FAIL, if DMA transfer is failed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDevAkX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpCertParams *OcpCertParam = (const XAsu_OcpCertParams *)ReqBuf->Arg;
	X509_PlatData PlatData;
	XOcp_CertData CertData;
	u32 CertSize;
	u8 CertBuf[X509_CERTIFICATE_MAX_SIZE_IN_BYTES];
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Validate client parameter. */
	Status = XAsu_OcpValidateCertParams(OcpCertParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsufw_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_INVALID_SUBSYSTEM_ID);
		goto END;
	}

	/** Assign DMA and SHA instance pointers. */
	PlatData.DmaPtr = XAsufw_OcpModule.AsuDmaPtr;
	PlatData.ShaPtr = XAsufw_OcpModule.ShaPtr;

	/** Initialized certificate data. */
	CertData.DevKeyType = OcpCertParam->DevKeySel;
	CertData.CertAddr = (u64)(UINTPTR)CertBuf;
	CertData.CertMaxSize = X509_CERTIFICATE_MAX_SIZE_IN_BYTES;
	CertData.CertActualSize = &CertSize;

	/** Generate DevAk certificate for given subsystem. */
	Status = XOcp_GetX509Cert(SubsystemId, &CertData, (void *)(UINTPTR)&PlatData, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_X509_DEVAK_CERT_GEN_FAIL);
		goto END;
	}

	/** Copy generated certificate to destination address using DMA. */
	if (OcpCertParam->CertBufLen < CertSize) {
		Status = XASUFW_OCP_INVALID_BUF_SIZE;
		goto END;
	}

	Status = XAsufw_DmaXfr(XAsufw_OcpModule.AsuDmaPtr, (u64)(UINTPTR)CertBuf,
			       OcpCertParam->CertBufAddr, CertSize, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_DMA_COPY_FAIL);
		goto END;
	}

	/** Copy actual size of generated certificate to destination address using DMA. */
	Status = XAsufw_DmaXfr(XAsufw_OcpModule.AsuDmaPtr, (UINTPTR)&CertSize,
			       (UINTPTR)OcpCertParam->CertActualSize, XOCP_X509_CERT_FIELD_LEN, 0U);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_OcpModule.AsuDmaPtr = NULL;
	XAsufw_OcpModule.ShaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is handler for DevAk attestation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if DevAk is attested successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_INVALID_SUBSYSTEM_ID, if subsystem id is not retrieved.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDevAkAttestation(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpDevAkAttest *OcpAttestParam = (const XAsu_OcpDevAkAttest *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Validate client parameter. */
	Status = XAsu_OcpValidateAttestParams(OcpAttestParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsufw_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_INVALID_SUBSYSTEM_ID);
		goto END;
	}

	/** DevAk attestation. */
	Status = XOcp_AttestWithDevAk(XAsufw_OcpModule.AsuDmaPtr, OcpAttestParam, SubsystemId);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_OcpModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for DME challenge request.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_OCP_DME_CHALLENGE_RESPONSE_FAIL, if DME challenge response operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDmeChallengeReq(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpDmeParams *OcpDmeParamsPtr = (const XAsu_OcpDmeParams *)(UINTPTR)ReqBuf->Arg;
	(void)ReqId;

	Status = XOcp_GenerateDmeResponse(XAsufw_OcpModule.AsuDmaPtr, OcpDmeParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DME_CHALLENGE_RESPONSE_FAIL);
	}

	if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	XAsufw_OcpModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for DME private keys encryption.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_OCP_DME_KEY_ENCRYPT_FAIL, in case of DME key encryption failure.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDmePvtKeysEncrypt(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpDmeKeyEncrypt *OcpDmeKeyEnc = (const XAsu_OcpDmeKeyEncrypt *)ReqBuf->Arg;
	(void)ReqId;

	Status = XOcp_EncryptDmeKeys(XAsufw_OcpModule.AsuDmaPtr, OcpDmeKeyEnc);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DME_KEY_ENCRYPT_FAIL);
	}

	if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_OcpModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for OCP module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation is failed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;

	/** Allocate DMA resource. */
	XAsufw_OcpModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_OCP, ReqId);
	if (XAsufw_OcpModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}
	/** Allocate OCP resource. */
	XAsufw_AllocateResource(XASUFW_OCP, XASUFW_OCP, ReqId);

	if (CmdId != XASU_OCP_DME_PVT_KEYS_ENCRYPT_CMD_ID) {
		if ((CmdId == XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID) ||
		    (CmdId == XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID) ||
		    (CmdId == XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID)) {
			/** Allocate SHA3 resource which are dependent on SHA3 HW. */
			XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_OCP, ReqId);
			XAsufw_OcpModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
		} else if (CmdId == XASU_OCP_DME_CHALLENGE_REQ_CMD_ID) {
			/** Allocate SHA2 resource which are dependent on SHA2 HW. */
			XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_OCP, ReqId);

			/** Allocate AES resource. */
			XAsufw_AllocateResource(XASUFW_AES, XASUFW_OCP, ReqId);
		} else {
			/* Do Nothing */
		}
		/** Allocate TRNG resource which are dependent on TRNG HW. */
		XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_OCP, ReqId);

		/** Allocate ECC resource. */
		XAsufw_AllocateResource(XASUFW_ECC, XASUFW_OCP, ReqId);
	} else {
		/** Allocate AES resource. */
		XAsufw_AllocateResource(XASUFW_AES, XASUFW_OCP, ReqId);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
#endif /* XASU_OCP_ENABLE */
/** @} */
