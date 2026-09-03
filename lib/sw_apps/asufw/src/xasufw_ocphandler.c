/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       rmv  08/17/26 Added validation of client provided buffers
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
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
#include "xasu_sharedmem.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_memvalidation.h"
#include "xocp.h"
#include "xsha_hw.h"
#include "xocp_ude.h"
#include "xoverlay.h"
#include "xkeymanager.h"
#include "xecc.h"
#include "xil_util.h"

#ifdef XASU_OCP_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_OcpDevIkCsrX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpDevIkX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpDevAkX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpUserX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpDevAkAttestation(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpUdeChallengeReq(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_OcpGetHuk(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
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
	static const XAsufw_ModuleCmd XAsufw_OcpCmds[XASU_OCP_MAX_CMDS] = {
		[XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDevIkX509CertGen),
		[XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDevAkX509CertGen),
		[XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDevIkCsrX509CertGen),
		[XASU_OCP_DEVAK_ATTESTATION_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpDevAkAttestation),
		[XASU_OCP_UDE_CHALLENGE_REQ_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpUdeChallengeReq),
		[XASU_OCP_RESERVED_CMD_ID] =
			XASUFW_MODULE_COMMAND(NULL),
		[XASU_OCP_GET_HUK_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpGetHuk),
		[XASU_OCP_GET_USER_X509_CERT_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_OcpUserX509CertGen),
	};

	/**
	 * The XAsufw_OcpResources contains the required resources for each supported
	 * command.
	 */
	static XAsufw_ResourcesRequired XAsufw_OcpResources[XASU_OCP_MAX_CMDS] = {
		[XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_SHA3_RESOURCE_MASK |
			XASUFW_ECC_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK |	XASUFW_SHA3_RESOURCE_MASK |
			XASUFW_ECC_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_SHA3_RESOURCE_MASK |
			XASUFW_ECC_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_OCP_DEVAK_ATTESTATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_ECC_RESOURCE_MASK |
			XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_OCP_UDE_CHALLENGE_REQ_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_AES_RESOURCE_MASK |
			XASUFW_SHA2_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK |
			XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_OCP_RESERVED_CMD_ID] = 0U,
		[XASU_OCP_GET_HUK_CMD_ID] = XASUFW_OCP_RESOURCE_MASK,
		[XASU_OCP_GET_USER_X509_CERT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_OCP_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_SHA3_RESOURCE_MASK |
			XASUFW_ECC_RESOURCE_MASK | XASUFW_KEYMANAGER_RESOURCE_MASK |
			XASUFW_OVERLAY_RESOURCE_MASK,
	};

	/** The XAsufw_OcpAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_OcpAccessPermBuf[XASU_OCP_MAX_CMDS] = {
		[XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID),
		[XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID),
		[XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID),
		[XASU_OCP_DEVAK_ATTESTATION_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_DEVAK_ATTESTATION_CMD_ID),
		[XASU_OCP_UDE_CHALLENGE_REQ_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_UDE_CHALLENGE_REQ_CMD_ID),
		[XASU_OCP_RESERVED_CMD_ID] = 0U,
		[XASU_OCP_GET_HUK_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_GET_HUK_CMD_ID),
		[XASU_OCP_GET_USER_X509_CERT_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_OCP_GET_USER_X509_CERT_CMD_ID),
	};

	XAsufw_OcpModule.Id = XASU_MODULE_OCP_ID;
	XAsufw_OcpModule.Cmds = XAsufw_OcpCmds;
	XAsufw_OcpModule.ResourcesRequired = XAsufw_OcpResources;
	XAsufw_OcpModule.CmdCnt = XASU_OCP_MAX_CMDS;
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
		Status = XASUFW_OCP_MODULE_REGISTRATION_FAIL;
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
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem id is not retrieved.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDevIkCsrX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpCertParams *OcpCertParam = (const XAsu_OcpCertParams *)ReqBuf->Arg;
	X509_PlatData PlatData;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_OcpCertParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided certificate and certificate size buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpCertParam->CertBufAddr,
				   OcpCertParam->CertBufLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpCertParam->CertActualSizeAddr,
				   XASUFW_WORD_LEN_IN_BYTES);

	/** Assign DMA and SHA instance pointers. */
	PlatData.DmaPtr = XAsufw_OcpModule.AsuDmaPtr;
	PlatData.ShaPtr = XAsufw_OcpModule.ShaPtr;

	/** Generate DevIk CSR(Certificate Signing Request) for ASU subsystem. */
	Status = XOcp_GetX509Cert(XASUFW_SUBSYTEM_ID, OcpCertParam, (void *)(UINTPTR)&PlatData,
				  XASU_TRUE);

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
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem id is not retrieved.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDevIkX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpCertParams *OcpCertParam = (const XAsu_OcpCertParams *)ReqBuf->Arg;
	X509_PlatData PlatData;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_OcpCertParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided certificate and certificate size buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpCertParam->CertBufAddr,
				   OcpCertParam->CertBufLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpCertParam->CertActualSizeAddr,
				   XASUFW_WORD_LEN_IN_BYTES);

	/** Assign DMA and SHA instance pointers. */
	PlatData.DmaPtr = XAsufw_OcpModule.AsuDmaPtr;
	PlatData.ShaPtr = XAsufw_OcpModule.ShaPtr;

	/** Generate DevIk certificate for ASU subsystem. */
	Status = XOcp_GetX509Cert(XASUFW_SUBSYTEM_ID, OcpCertParam, (void *)(UINTPTR)&PlatData,
				  XASU_FALSE);

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
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem id is not retrieved.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDevAkX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpCertParams *OcpCertParam = (const XAsu_OcpCertParams *)ReqBuf->Arg;
	X509_PlatData PlatData;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_OcpCertParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided certificate and certificate size buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpCertParam->CertBufAddr,
				   OcpCertParam->CertBufLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpCertParam->CertActualSizeAddr,
				   XASUFW_WORD_LEN_IN_BYTES);

	/** Assign DMA and SHA instance pointers. */
	PlatData.DmaPtr = XAsufw_OcpModule.AsuDmaPtr;
	PlatData.ShaPtr = XAsufw_OcpModule.ShaPtr;

	/** Generate DevAk certificate for given subsystem. */
	Status = XOcp_GetX509Cert(SubsystemId, OcpCertParam, (void *)(UINTPTR)&PlatData,
				  XASU_FALSE);

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
 * @brief	This function is handler for user X.509 certificate generation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if user certificate is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem id is not retrieved.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if clearing the certificate generation working data fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpUserX509CertGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	XFih_Var XFihOcp = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	const XAsu_OcpUserCertParams *OcpUserCertParam =
		(const XAsu_OcpUserCertParams *)ReqBuf->Arg;
	XOcp_UserCertData *CertData = XOcp_GetUserCertData();
	XOcp_X509CertGenParams CertGenParams;
	X509_PlatData PlatData;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u32 CertSize = 0U;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_OcpUserCertParams);

	/** Validate the user certificate request parameters. */
	Status = XAsu_OcpValidateUserCertParams(OcpUserCertParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Assign DMA and SHA instance pointers. */
	PlatData.DmaPtr = XAsufw_OcpModule.AsuDmaPtr;
	PlatData.ShaPtr = XAsufw_OcpModule.ShaPtr;

	/** Clear the certificate generation working data. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(CertData, sizeof(XOcp_UserCertData), 0U, sizeof(XOcp_UserCertData));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Copy the user certificate configuration from the user memory to ASU memory. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(PlatData.DmaPtr, OcpUserCertParam->UserCertCfgAddr,
			       (u64)(UINTPTR)&CertData->UserCfg,
			       (u32)sizeof(XAsu_X509UserCertCfg), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Validate the user certificate configuration. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_OcpValidateUserCertCfg(&CertData->UserCfg);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Fetch the subject public key from the key vault. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	CertData->SubjectPubKeyObj.CurveType = XASU_ECC_NIST_P384;
	Status = XKeyManager_UpdateKeyObjFromVault(PlatData.DmaPtr,
			CertData->UserCfg.SubjectPubKeyId, (u64)(UINTPTR)&CertData->SubjectPubKeyObj,
			SubsystemId,
			XKeyManager_GetEccPubKeyUseCase(CertData->UserCfg.CertCfg.KeyUsage),
			XKEYMANAGER_RSA_OP_NONE, XASU_ECC_PUB_SUBVAULT_ID);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_INVALID_PUB_KEY_ID);
		goto END;
	}

	/** Fetch the issuer private key from the key vault. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	CertData->IssuerPvtKeyObj.CurveType = XASU_ECC_NIST_P384;
	Status = XKeyManager_UpdateKeyObjFromVault(PlatData.DmaPtr,
			CertData->UserCfg.IssuerPvtKeyId, (u64)(UINTPTR)&CertData->IssuerPvtKeyObj,
			SubsystemId, XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE,
			XKEYMANAGER_RSA_OP_NONE, XASU_ECC_PVT_SUBVAULT_ID);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_INVALID_SIGN_KEY_ID);
		goto END;
	}

	/** Fetch the issuer public key from the key vault to verify the signature. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	CertData->IssuerPubKeyObj.CurveType = XASU_ECC_NIST_P384;
	Status = XKeyManager_UpdateKeyObjFromVault(PlatData.DmaPtr,
			CertData->UserCfg.IssuerPubKeyId, (u64)(UINTPTR)&CertData->IssuerPubKeyObj,
			SubsystemId, XASU_KEYMANAGER_ECC_PUB_SIGN_VER_USE_CASE,
			XKEYMANAGER_RSA_OP_NONE, XASU_ECC_PUB_SUBVAULT_ID);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_INVALID_PUB_KEY_ID);
		goto END;
	}

	/** Verify the issuer public and private keys are a key pair. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(XEcc_Pwct, XFihOcp, Status, XEcc_GetInstance(XASU_XECC_0_DEVICE_ID),
		  PlatData.DmaPtr, CertData->IssuerPvtKeyObj.CurveType,
		  CertData->IssuerPvtKeyObj.KeyLen, CertData->IssuerPvtKeyObj.KeyAddr,
		  CertData->IssuerPubKeyObj.KeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_ISSUER_KEY_PAIR_MISMATCH);
		goto END;
	}

	/**
	 * Populate the certificate generation parameters with the fetched key material. A user
	 * certificate is issued by a distinct issuer, so it is neither self-signed nor a CA
	 * certificate.
	 */
	CertGenParams.CertFields = &CertData->UserCfg.CertCfg;
	CertGenParams.SubjectPubKey = (const u8 *)(UINTPTR)CertData->SubjectPubKeyObj.KeyAddr;
	CertGenParams.SubjectPubKeyLen = XAsu_DoubleCurveLength(CertData->SubjectPubKeyObj.KeyLen);
	CertGenParams.IssuerPubKey = (const u8 *)(UINTPTR)CertData->IssuerPubKeyObj.KeyAddr;
	CertGenParams.IssuerPubKeyLen = XAsu_DoubleCurveLength(CertData->IssuerPubKeyObj.KeyLen);
	CertGenParams.IssuerPvtKey = (const u8 *)(UINTPTR)CertData->IssuerPvtKeyObj.KeyAddr;
	CertGenParams.CertBufAddr = OcpUserCertParam->CertBufAddr;
	CertGenParams.CertBufMaxSize = OcpUserCertParam->CertBufLen;
	CertGenParams.IsSelfSigned = (u32)XASU_FALSE;
	CertGenParams.IsCaCert = (u32)XASU_FALSE;

	/** Generate the user certificate for the public key owned by the requesting subsystem. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GetUserX509Cert(&CertGenParams, &CertSize, (void *)(UINTPTR)&PlatData);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Copy the actual size of the generated certificate to the destination using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(PlatData.DmaPtr, (u64)(UINTPTR)&CertSize,
			       OcpUserCertParam->CertActualSizeAddr, (u32)sizeof(CertSize), 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	ReturnStatus = XASUFW_OCP_CERT_GENERATION_SUCCESS;

END:
	/** Zeroize the certificate generation working data. */
	XFIH_CALL(Xil_SecureZeroize, XFihOcp, ClearStatus, (u8 *)(UINTPTR)CertData,
		  (u32)sizeof(XOcp_UserCertData));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

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
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem id is not retrieved.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpDevAkAttestation(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpDevAkAttest *OcpAttestParam = (const XAsu_OcpDevAkAttest *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_OcpDevAkAttest);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided attestation data and signature buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpAttestParam->DataAddr,
				   OcpAttestParam->DataLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpAttestParam->SignatureAddr,
				   OcpAttestParam->SignatureBufLen);

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
 * @brief	This function is a handler for UDE challenge request.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem id is not retrieved.
 *	- XASUFW_OCP_UDE_CHALLENGE_RESPONSE_FAIL, if UDE challenge response operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpUdeChallengeReq(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_OcpUdeParams *OcpUdeParamsPtr = (const XAsu_OcpUdeParams *)(UINTPTR)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_OcpUdeParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided nonce and UDE response buffers. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpUdeParamsPtr->NonceAddr,
				   XASU_OCP_UDE_NONCE_SIZE_IN_BYTES);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, OcpUdeParamsPtr->OcpUdeResponseAddr,
				   sizeof(XAsu_OcpUdeResponse));

	Status = XOcp_GenerateUdeResponse(XAsufw_OcpModule.AsuDmaPtr, OcpUdeParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_CHALLENGE_RESPONSE_FAIL);
	}

END:
	if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	XAsufw_OcpModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for Hardware Unique Key (HUK) generation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if HUK generation is successful.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpGetHuk(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u8 *HukBuf = (u8 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
					(XASUFW_RESP_DATA_OFFSET * XASUFW_WORD_LEN_IN_BYTES);

	/** Generate and return HUK in response buffer. */
	Status = XOcp_GetHuk(HukBuf);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_INVALID_PARAM);
	}

	if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

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
 *	- XASUFW_OVERLAY_ACTIVATE_FAIL, if overlay activation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource fails.
 *
 *************************************************************************************************/
static s32 XAsufw_OcpResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;

	/** Allocate OCP resource. */
	XAsufw_AllocateResource(XASUFW_OCP, XASUFW_OCP, ReqId);

	/** HUK command only needs OCP resource, no DMA or other resources. */
	if (CmdId == XASU_OCP_GET_HUK_CMD_ID) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/** Allocate DMA resource. */
	XAsufw_OcpModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_OCP, ReqId);
	if (XAsufw_OcpModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	/** Allocate overlay resource. */
	XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_OCP, ReqId);

	if ((CmdId == XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID) ||
	    (CmdId == XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID) ||
	    (CmdId == XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID) ||
	    (CmdId == XASU_OCP_GET_USER_X509_CERT_CMD_ID)) {
		/** Allocate SHA3 resource which are dependent on SHA3 HW. */
		XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_OCP, ReqId);
		XAsufw_OcpModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	} else if (CmdId == XASU_OCP_UDE_CHALLENGE_REQ_CMD_ID) {
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

	/** Allocate key manager resource for user certificate generation. */
	if (CmdId == XASU_OCP_GET_USER_X509_CERT_CMD_ID) {
		XAsufw_AllocateResource(XASUFW_KEYMANAGER, XASUFW_OCP, ReqId);
	}

	/** Activate RSA-ECC overlay. */
	Status = XOverlay_Activate(XOVERLAY_ALGO_RSA_ECC, XAsufw_OcpModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		if (XAsufw_ReleaseResource(XASUFW_OCP, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

END:
	return Status;
}
#endif /* XASU_OCP_ENABLE */
/** @} */
