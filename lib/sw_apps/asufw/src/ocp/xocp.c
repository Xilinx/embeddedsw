/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp.c
*
* This file contains the implementation of the interface functions for OCP functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- -------- ---------------------------------------------------------------------------
* 1.0   rmv  07/16/25 Initial release
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xocp_server_apis OCP server APIs
 * @{
 */
/************************************** Include Files ********************************************/
#include "x509_cert.h"
#include "xasu_eccinfo.h"
#include "xasufw_hw.h"
#include "xasufw_ipi.h"
#include "xasufw_status.h"
#include "xasufw_trnghandler.h"
#include "xasufw_util.h"
#include "xecc.h"
#include "xhmac.h"
#include "xil_types.h"
#include "xil_util.h"
#include "xocp.h"
#include "xrsa_ecc.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xtrng.h"
#include "xtrng_hw.h"

#ifdef XASU_OCP_ENABLE
/********************************** Constant Definitions *****************************************/

/************************************ Macro Definitions ******************************************/
#define XOCP_DICE_CDI_SIZE_IN_BYTES	(48U)		/**< CDI size in bytes */
#define XOCP_TRNG_DF_LENGTH		(2U)		/**< TRNG DF length to generate ECC pvt
							key */
#define XOCP_TRNG_DRBG_SEED_LIFE	(7U)		/**< User config seed life */
#define XOCP_DEV_KEY_RANDOM_BUF_SIZE	(64U)		/**< Maximum device private key size */

#define XOCP_SUBSYS_EVENT_MASK		(0x01U)		/**< Subsystem event mask */
#define XOCP_SUBSYS_EVENT_SHIFT		(1U)		/**< Subsystem event size */

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/
static XOcp_DeviceKeys DevIkData;	/**< Device identity key data */
static XOcp_DeviceKeys DevAkData[XOCP_MAX_OCP_SUBSYSTEMS];	/**< Device attestation key data */

/************************************ Function Prototypes ****************************************/
static s32 XOcp_GenerateDevIkAkPvtKey(const XOcp_PrivateKeyGen *PvtKeyInfo);
static s32 XOcp_GenerateSwCdi(u32 SubsystemId, XAsufw_Dma *DmaPtr, u8 *SwCdi, u32 CdiLen);
static s32 XOcp_GetAsuCdiAddr(u32 *AsuCdiAddr);
static s32 XOcp_GenerateDevIk(XAsufw_Dma *DmaPtr);
static s32 XOcp_GetSubsystemHashAddr(u32 SubsystemId, u32 *SwHashAddr);
static s32 XOcp_GenerateDevAk(u32 SubsysIdx, XAsufw_Dma *DmaPtr);
static s32 XOcp_GetSubsytemIndex(u32 SubsystemId, u32 *SubsystemIdx);

/*************************************************************************************************/
/**
 * @brief	This function generates device keys based on event mask.
 *
 * @param	DmaPtr		Pointer to allocated DMA resource.
 * @param	EventMask	Event mask representing subsystem state changes.
 *
 * @return
 *	- XASUFW_SUCCESS, if PLM event is handled successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_DEVIK_GENERATION_FAIL, if DevIk generation is failed.
 *	- XASUFW_OCP_DEVAK_GENERATION_FAIL, if DevAk generation is failed.
 *
 *************************************************************************************************/
s32 XOcp_GenerateDeviceKeys(XAsufw_Dma *DmaPtr, u32 EventMask)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Idx = XASUFW_VALUE_ONE;
	u32 EvtMask = EventMask;

	/** Generate DevIK key pair. */
	if (((EvtMask & XOCP_SUBSYS_EVENT_MASK) == XOCP_SUBSYS_EVENT_MASK)) {
		Status = XOcp_GenerateDevIk(DmaPtr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_OCP_DEVIK_GENERATION_FAIL);
			goto END;
		}
	}

	/** Generate DevAK key pair according to event mask for required subsystems. */
	EvtMask = EvtMask >> XOCP_SUBSYS_EVENT_SHIFT;
	while (EvtMask != 0x0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		if ((EvtMask & XOCP_SUBSYS_EVENT_MASK) == XOCP_SUBSYS_EVENT_MASK) {
			Status = XOcp_GenerateDevAk((Idx - XOCP_USER_SUBSYS_START_INDEX), DmaPtr);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_OCP_DEVAK_GENERATION_FAIL);
				goto END;
			}
		}
		EvtMask = EvtMask >> XOCP_SUBSYS_EVENT_SHIFT;
		Idx++;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allows user to sign his own data using device attestation key.
 *
 * @param	DmaPtr		Pointer to allocated DMA resource.
 * @param	OcpAttestParam	Pointer to the attestation information.
 * @param	SubsystemId	Subsystem ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if DevAk is attested successfully.
 *	- XASUFW_FAILURE, in case of failure
 *	- XASUFW_OCP_INVALID_PARAM, if parameter is invalid.
 *	- XASUFW_OCP_INVALID_SUBSYSTEM_INDEX, if subsystem index in invalid.
 *	- XASUFW_OCP_DEVAK_NOT_READY, if DevAk key is not generated.
 *	- XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL, if ephemeral key is not generated.
 *
 *************************************************************************************************/
s32 XOcp_AttestWithDevAk(XAsufw_Dma *DmaPtr, const XAsu_OcpDevAkAttest *OcpAttestParam,
			 u32 SubsystemId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XEcc *EccInstance = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	u8 EphemeralKey[XASU_ECC_P384_SIZE_IN_BYTES] = {0U};
	u32 SubsysIdx = 0U;

	/** Validate input parameter. */
	if ((DmaPtr == NULL) || (OcpAttestParam == NULL) || (SubsystemId == 0U)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Validate signature buffer. */
	if ((OcpAttestParam->SignatureAddr == 0U) ||
	    (OcpAttestParam->SignatureBufLen < XASU_ECC_P384_SIZE_IN_BYTES)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Get subsystem index using subsystem ID. */
	Status = XOcp_GetSubsytemIndex(SubsystemId, &SubsysIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_SUBSYSTEM_INDEX;
		goto END;
	}

	/** Check whether DevAk key is ready or not. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (DevAkData[SubsysIdx].IsDevAkKeyReady != (u8)XASU_TRUE) {
		Status = XASUFW_OCP_DEVAK_NOT_READY;
		goto END;
	}

	/** Generate ephemeral key using TRNG. */
	Status = XAsufw_TrngGetRandomNumbers(EphemeralKey, XASU_ECC_P384_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL);
		goto END;
	}

	/* Generate the signature using DevAk private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GenerateSignature(EccInstance, DmaPtr, XASU_ECC_NIST_P384,
					XASU_ECC_P384_SIZE_IN_BYTES,
					(u64)(UINTPTR)DevAkData[SubsysIdx].EccPvtKey,
					EphemeralKey, OcpAttestParam->DataAddr,
					OcpAttestParam->DataLen, OcpAttestParam->SignatureAddr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates a DevIk key pair(private and public key).
 *
 * @param	DmaPtr	Pointer to allocated DMA resource.
 *
 * @return
 *	- XASUFW_SUCCESS, if DevIk key pair is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_GET_ASU_CDI_FAIL, if ASU CDI is not retrieved successfully.
 *	- XASUFW_OCP_GEN_DEVIK_PVT_KEY_FAIL, if DevIk private key generation is failed.
 *	- XASUFW_OCP_GEN_DEVIK_PUB_KEY_FAIL, if DevIk public key generation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateDevIk(XAsufw_Dma *DmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XOcp_PrivateKeyGen PvtKeyInfo;
	u32 AsuCdiAddr = 0U;
	u8 PersonalString[XOCP_PERSONAL_STRING_LEN] = {0U};

	/** Validate input parameter. */
	if (DmaPtr == NULL) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Get ASU CDI seed. */
	Status = XOcp_GetAsuCdiAddr(&AsuCdiAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GET_ASU_CDI_FAIL);
		goto END;
	}

	/** Copy Device DNA as personalized string. */
	Status = Xil_SMemCpy(PersonalString, XOCP_PERSONAL_STRING_LEN, (u8 *)EFUSE_CACHE_DNA_0,
			     EFUSE_CACHE_DNA_SIZE, EFUSE_CACHE_DNA_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Initially set IsDevIkKeyReady to false. */
	DevIkData.IsDevIkKeyReady = (u8)XASU_FALSE;

	/** Generate DevIk private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	PvtKeyInfo.CdiAddr = AsuCdiAddr;
	PvtKeyInfo.CdiLength = XOCP_DICE_CDI_SIZE_IN_BYTES;
	PvtKeyInfo.PerStringAddr = (u32)(UINTPTR)PersonalString;
	PvtKeyInfo.PvtKeyAddr = (u32)(UINTPTR)DevIkData.EccPvtKey;
	Status = XOcp_GenerateDevIkAkPvtKey(&PvtKeyInfo);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_DEVIK_PVT_KEY_FAIL);
		goto END;
	}

	/** Generate DevIk public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, XASU_ECC_NIST_P384, XASU_ECC_P384_SIZE_IN_BYTES,
					(u64)(UINTPTR)DevIkData.EccPvtKey,
					(u64)(UINTPTR)DevIkData.EccX);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_DEVIK_PUB_KEY_FAIL);
		goto END;
	}
	DevIkData.PublicKeyLen = XAsu_DoubleCurveLength(XASU_ECC_P384_SIZE_IN_BYTES);

	/** Set IsDevIkKeyReady to true. */
	DevIkData.IsDevIkKeyReady = (u8)XASU_TRUE;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates DevAk key pair(private and public keys) for given subsystem
 *		index.
 *
 * @param	SubsysIdx	Subsystem index.
 * @param	DmaPtr		Pointer to allocated DMA resource.
 *
 * @return
 *	- XASUFW_SUCCESS, if DevAk key pair is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_INVALID_SUBSYSTEM_INDEX, if subsystem index is invalid.
 *	- XASUFW_OCP_GEN_DEVAK_PVT_KEY_FAIL, if DevAk private key generation is failed.
 *	- XASUFW_OCP_GEN_DEVAK_PUB_KEY_FAIL, if DevAk public key generation is failed.
 *	- XASUFW_OCP_GEN_SUBSYSTEM_CDI_FAIL, if subsystem CDI generation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateDevAk(u32 SubsysIdx, XAsufw_Dma *DmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XOcp_PrivateKeyGen PvtKeyInfo;
	u8 SwCdi[XOCP_DICE_CDI_SIZE_IN_BYTES];
	const XOcp_CdoData *CdoData = (XOcp_CdoData *)XOCP_CDO_DATA_ADDR;
	const u8 *PersStr = NULL;

	/** Validate input DMA parameter. */
	if (DmaPtr == NULL) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/**
	 * Generate DevAk key pair only if subsystem index doesn't exceed the maximum number of
	 * DevAk supported.
	 */
	if (SubsysIdx >= XOCP_MAX_OCP_SUBSYSTEMS) {
		Status = XASUFW_OCP_INVALID_SUBSYSTEM_INDEX;
		goto END;
	}

	/** Initially set IsDevAkKeyReady to false. */
	DevAkData[SubsysIdx].IsDevAkKeyReady = (u8)XASU_FALSE;

	/** Generate subsystem CDI. */
	Status = XOcp_GenerateSwCdi(CdoData->SubsysData[SubsysIdx].SubSystemID, DmaPtr, SwCdi,
				    XOCP_DICE_CDI_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_SUBSYSTEM_CDI_FAIL);
		goto END;
	}

	/** Assign an address of personalized string if it is available in ASU CDO. */
	if (CdoData->SubsysData[SubsysIdx].IsPersonalStringAvailable == (u32)XASU_TRUE) {
		PersStr = CdoData->SubsysData[SubsysIdx].PersonalString;
	}

	/** Generate DevAk Private Key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	PvtKeyInfo.CdiAddr = (u32)(UINTPTR)SwCdi;
	PvtKeyInfo.CdiLength = XOCP_DICE_CDI_SIZE_IN_BYTES;
	PvtKeyInfo.PerStringAddr = (u32)(UINTPTR)PersStr;
	PvtKeyInfo.PvtKeyAddr = (u32)(UINTPTR)DevAkData[SubsysIdx].EccPvtKey;
	Status = XOcp_GenerateDevIkAkPvtKey(&PvtKeyInfo);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_DEVAK_PVT_KEY_FAIL);
		goto END;
	}

	/** Generate DevAk public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, XASU_ECC_NIST_P384, XASU_ECC_P384_SIZE_IN_BYTES,
					(u64)(UINTPTR)DevAkData[SubsysIdx].EccPvtKey,
					(u64)(UINTPTR)DevAkData[SubsysIdx].EccX);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_DEVAK_PUB_KEY_FAIL);
		goto END;
	}
	DevAkData[SubsysIdx].PublicKeyLen = XAsu_DoubleCurveLength(XASU_ECC_P384_SIZE_IN_BYTES);


	/** Set IsDevAkKeyReady to true. */
	DevAkData[SubsysIdx].IsDevAkKeyReady = (u8)XASU_TRUE;

END:
	return Status;
}


/*************************************************************************************************/
/**
 * @brief	This function generates ECC DevIk/Ak private key for specified elliptical curve.
 *
 * @param	PvtKeyInfo	Pointer to structure containing information to generate ECC private
 *				key.
 *
 * @return
 *	- XASUFW_SUCCESS, if private key is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_TRNG_FAIL, if random number generation is failed.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if memset is failed.
 *	- XASUFW_OCP_TRNG_UNINSTANTIATE_FAIL, if TRNG uninstantiate is failed.
 *	- XASUFW_OCP_TRNG_INSTANTIATE_FAIL, if TRNG instantiate is failed.
 *	- XASUFW_OCP_GEN_ECC_PVT_KEY_FAIL, if private key generation is failed.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateDevIkAkPvtKey(const XOcp_PrivateKeyGen *PvtKeyInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XTrng_UserConfig UsrCfg;
	XTrng *TrngInstance = XTrng_GetInstance(XASU_XTRNG_0_DEVICE_ID);
	u8 RandBuf[XOCP_DEV_KEY_RANDOM_BUF_SIZE] = {0U};

	/** Uninstantiate TRNG instance to change the TRNG mode.  */
	Status = XTrng_Uninstantiate(TrngInstance);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_TRNG_UNINSTANTIATE_FAIL);
		goto END;
	}

	/** Zeroize TRNG user config structure instance. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(&UsrCfg, sizeof(UsrCfg), 0U, sizeof(UsrCfg));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Instantiate TRNG in DRBG mode to generate ECC private key */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	UsrCfg.DFLength = XOCP_TRNG_DF_LENGTH;
	UsrCfg.Mode = XTRNG_DRBG_MODE;
	UsrCfg.SeedLife = XOCP_TRNG_DRBG_SEED_LIFE;
	Status = XTrng_Instantiate(TrngInstance, (u8 *)(UINTPTR)PvtKeyInfo->CdiAddr,
			PvtKeyInfo->CdiLength, (u8 *)(UINTPTR)PvtKeyInfo->PerStringAddr, &UsrCfg);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_TRNG_INSTANTIATE_FAIL);
		goto END;
	}

	/**
	 * ECC private key is expecting 48-bytes so need to generate random numbers twice
	 * as TRNG supports 32-bytes random number generation at a time.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_Generate(TrngInstance, RandBuf, XTRNG_SEC_STRENGTH_IN_BYTES, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_TRNG_FAIL);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_Generate(TrngInstance, RandBuf + XTRNG_SEC_STRENGTH_IN_BYTES,
				XTRNG_SEC_STRENGTH_IN_BYTES, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_TRNG_FAIL);
		goto END;
	}

	/** Generate private key using RSA ECC engine. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePvtKey(XASU_ECC_NIST_P384, XASU_ECC_P384_SIZE_IN_BYTES,
					(u8 *)(UINTPTR)PvtKeyInfo->PvtKeyAddr,
					RandBuf, XOCP_DEV_KEY_RANDOM_BUF_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_ECC_PVT_KEY_FAIL);
		goto END;
	}

END:
	/** Enable TRNG to default mode. */
	Status = XAsufw_UpdateErrorStatus(Status, XTrng_EnableDefaultMode(TrngInstance));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gets the index of subsystem ID.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	SubsystemIdx	Pointer to store subsystem index.
 *
 * @return
 *	- XASUFW_SUCCESS, if subsystem index is retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XOcp_GetSubsytemIndex(u32 SubsystemId, u32 *SubsystemIdx)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XOcp_CdoData *CdoData = (XOcp_CdoData *)XOCP_CDO_DATA_ADDR;
	u32 Idx;

	/** Iterate through subsystem list and return an index if subsystem ID is matched. */
	for (Idx = 0U; Idx < CdoData->NumSubsys; Idx++) {
		if (SubsystemId == CdoData->SubsysData[Idx].SubSystemID) {
			*SubsystemIdx = Idx;
			Status = XASUFW_SUCCESS;
			goto END;
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the X.509 certificate and DevIK CSR for device keys.
 *
 * @param	SubsystemId	Subsystem ID for which certificate is to be generated.
 * @param	CertPtr		Pointer to the certificate data.
 * @param	PlatData	Pointer to platform specific data.
 * @param	IsCsr		Indicates if this is a Certificate Signing Request(CSR).
 *
 * @return
 *	- XASUFW_SUCCESS, if certificate is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_DEVICE_KEY_TYPE_INVALID, if device key type is invalid.
 *	- XASUFW_OCP_KEY_MGMT_NOT_READY, if DevIk pair is not generated.
 *	- XASUFW_OCP_DEVAK_NOT_READY, if DevAk pair is not generated.
 *	- XASUFW_OCP_INVALID_SUBSYSTEM_INDEX, if subsystem index is invalid.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if zeroize memset is failed.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- XASUFW_OCP_X509_CERT_GEN_FAIL, if certificate generation is failed.
 *
 *************************************************************************************************/
s32 XOcp_GetX509Cert(u32 SubsystemId, const XOcp_CertData *CertPtr, void *PlatData, u8 IsCsr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	X509_Config CertCfg;
	XOcp_CdoData *CdoData = (XOcp_CdoData *)XOCP_CDO_DATA_ADDR;
	u32 SubsysIdx = 0U;

	/** Validate platform data and CSR flag parameter. */
	if ((PlatData == NULL) || (CertPtr == NULL) || (CertPtr->CertAddr == 0U) ||
	    ((IsCsr != XASU_FALSE) && (IsCsr != XASU_TRUE))) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;

	}

	/** Validate device key type. */
	if (((IsCsr == XASU_TRUE) && (CertPtr->DevKeyType != (u32)XOCP_DEVIK)) ||
	    ((CertPtr->DevKeyType != (u32)XOCP_DEVAK) &&
	    (CertPtr->DevKeyType != (u32)XOCP_DEVIK))) {
		Status = XASUFW_OCP_DEVICE_KEY_TYPE_INVALID;
		goto END;
	}

	/** Subsystem ID should be ASU subsystem ID for DevIk. */
	if ((CertPtr->DevKeyType == (u32)XOCP_DEVIK) && (SubsystemId != XASUFW_SUBSYTEM_ID)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Check if DevIk is ready or not. */
	if (DevIkData.IsDevIkKeyReady != (u32)XASU_TRUE) {
		Status = XASUFW_OCP_KEY_MGMT_NOT_READY;
		goto END;
	}

	/** Set X.509 certificate configurations. */
	CertCfg.PubKeyInfo.PubKeyType = X509_PUB_KEY_ECC;
	CertCfg.PubKeyInfo.EccCurveType = X509_ECC_CURVE_TYPE_384;
	CertCfg.IsCsr = IsCsr;
	CertCfg.IssuerPubKeyLen = DevIkData.PublicKeyLen;
	CertCfg.IssuerPublicKey = (u8 *)DevIkData.EccX;
	CertCfg.IssuerPrvtKey = (u8 *)DevIkData.EccPvtKey;
	CertCfg.PlatformData = PlatData;

	if (CertPtr->DevKeyType == (u32)XOCP_DEVIK) {
		/**
		 * Get user configurations for DevIk certificate.
		 * For DevIk, subsystem shall be ASU and subsystem index 0.
		 */
		CertCfg.UserCfg = &CdoData->AsuSubsysData.UserCfg;

		/** Set subject public key for DevIk certificate. */
		CertCfg.PubKeyInfo.SubjectPubKeyLen = DevIkData.PublicKeyLen;
		CertCfg.PubKeyInfo.SubjectPublicKey = (u8 *)DevIkData.EccX;
		CertCfg.IsSelfSigned = (u8)XASU_TRUE;
	} else {
		/** Get subsystem index using subsystem ID. */
		Status = XOcp_GetSubsytemIndex(SubsystemId, &SubsysIdx);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_OCP_INVALID_SUBSYSTEM_INDEX;
			goto END;
		}

		/** Check if DevAk is ready or not. */
		if (DevAkData[SubsysIdx].IsDevAkKeyReady != (u8)XASU_TRUE) {
			Status = XASUFW_OCP_DEVAK_NOT_READY;
			goto END;
		}

		/** Get user configurations for DevAk certificate. */
		CertCfg.UserCfg = &CdoData->SubsysData[SubsysIdx].UserCfg;

		/** Update Issuer with CA issuer. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemSet(CertCfg.UserCfg->Issuer, X509_ISSUER_MAX_SIZE, 0U,
				     CertCfg.UserCfg->IssuerLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END;
		}
		CertCfg.UserCfg->IssuerLen = 0U;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(CertCfg.UserCfg->Issuer, X509_ISSUER_MAX_SIZE,
				     CdoData->AsuSubsysData.UserCfg.Issuer,
				     X509_ISSUER_MAX_SIZE,
				     CdoData->AsuSubsysData.UserCfg.IssuerLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}
		CertCfg.UserCfg->IssuerLen = CdoData->AsuSubsysData.UserCfg.IssuerLen;

		/** Set subject public key for DevAk certificate. */
		CertCfg.PubKeyInfo.SubjectPubKeyLen = DevAkData[SubsysIdx].PublicKeyLen;
		CertCfg.PubKeyInfo.SubjectPublicKey = (u8 *)DevAkData[SubsysIdx].EccX;
		CertCfg.IsSelfSigned = (u8)XASU_FALSE;
	}

	/** Generate X.509 certificate. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenerateX509Cert(CertPtr->CertAddr, CertPtr->CertMaxSize,
				       CertPtr->CertActualSize, &CertCfg);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_X509_CERT_GEN_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the subsystem CDI from the ASU CDI and subsystem hash.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	DmaPtr		Pointer to allocated DMA resource.
 * @param	SwCdi		Pointer to variable to store subsystem Cdi.
 * @param	CdiLen		Cdi length.
 *
 * @return
 *	- XASUFW_SUCCESS, if SW CDI is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_GET_ASU_CDI_FAIL, if ASU CDI is not retrieved successfully.
 *	- XASUFW_OCP_GET_SUBSYSTEM_HASH_FAIL, if subsystem hash is not retrieved successfully.
 *	- XASUFW_HMAC_INITIALIZATION_FAILED, if HMAC initialization is failed.
 *	- XASUFW_HMAC_UPDATE_FAILED, if HMAC update is failed.
 *	- XASUFW_HMAC_FINAL_FAILED, if HMAC final is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateSwCdi(u32 SubsystemId, XAsufw_Dma *DmaPtr, u8 *SwCdi, u32 CdiLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XHmac *XAsufw_HmacInstance = XHmac_GetInstance();
	XSha *Sha3Ptr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	u32 AsuCdiAddr = 0U;
	u32 SwHashAddr = 0U;
	u8 HmacOutput[XASU_SHA_384_HASH_LEN] = {0U};

	/** Validate the input parameter. */
	if (DmaPtr == NULL) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Get ASU CDI seed */
	Status = XOcp_GetAsuCdiAddr(&AsuCdiAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GET_ASU_CDI_FAIL);
		goto END;
	}

	/** Get the hash address of the subsystem. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GetSubsystemHashAddr(SubsystemId, &SwHashAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GET_SUBSYSTEM_HASH_FAIL);
		goto END;
	}

	/** Perform HMAC init operations. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XHmac_Init(XAsufw_HmacInstance, DmaPtr, Sha3Ptr, (u64)AsuCdiAddr,
			    XOCP_DICE_CDI_SIZE_IN_BYTES, XASU_SHA_MODE_384, CdiLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_INITIALIZATION_FAILED);
		goto END;
	}

	/** Perform HMAC update operations. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XHmac_Update(XAsufw_HmacInstance, DmaPtr, (u64)SwHashAddr,
			      XOCP_DICE_CDI_SIZE_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_UPDATE_FAILED);
		goto END;
	}

	/** Perform HMAC final operations. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XHmac_Final(XAsufw_HmacInstance, DmaPtr, (u32 *)HmacOutput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HMAC_FINAL_FAILED);
		goto END;
	}

	/** Copy HMAC output to subsystem CDI address. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(SwCdi, CdiLen, HmacOutput, XASU_SHA_384_HASH_LEN, CdiLen);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provides ASU CDI address.
 *
 * @param	AsuCdiAddr		Pointer to the variable to store ASU CDI address.
 *
 * @return
 *	- XASUFW_SUCCESS, if ASU CDI address is retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_SEND_IPI_REQ_FAIL, if IPI request is failed.
 *	- XASUFW_OCP_READ_IPI_RESP_FAIL, if IPI response read is failed.
 *
 *************************************************************************************************/
static s32 XOcp_GetAsuCdiAddr(u32 *AsuCdiAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XOCP_ASU_CDI_TX_PAYLOAD_SIZE];
	s32 Response = XASUFW_FAILURE;
	static const u8 XOcpAsuCdi[XOCP_DICE_CDI_SIZE_IN_BYTES] = {0U};
	static u8 AsuCdiValid = (u8)XASU_FALSE;

	/** Skip IPI request to PLM if ASU CDI is already available. */
	if (AsuCdiValid == (u8)XASU_TRUE) {
		Status = XASUFW_SUCCESS;
		goto RET;
	}

	/** Prepare IPI request payload. */
	Payload[XASUFW_BUFFER_INDEX_ZERO] = XASUFW_PLM_IPI_HEADER(XOCP_ASU_CDI_TX_ID_CMD_LEN,
			XASUFW_PLM_CMD_ID_ASU_CDI_TX_ID, XASUFW_PLM_ASU_MODULE_ID);
	Payload[XASUFW_BUFFER_INDEX_ONE] = (u32)(UINTPTR)XOcpAsuCdi;

	/** Send IPI request to PLM to get ASU CDI. */
	Status = XAsufw_SendIpiToPlm(Payload, XOCP_ASU_CDI_TX_PAYLOAD_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_SEND_IPI_REQ_FAIL);
		goto END;
	}

	/** Read response received from PLM. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm((u32 *)(UINTPTR)&Response,
					   XOCP_ASU_CDI_TX_PAYLOAD_RESP_SIZE);
	if ((Status != XASUFW_SUCCESS) || (Response != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_READ_IPI_RESP_FAIL);
		goto END;
	}

	/** Mark ASU CDI as valid. */
	AsuCdiValid = (u8)XASU_TRUE;

RET:
	/** Provide ASU CDI address to caller. */
	*AsuCdiAddr = (u32)XOcpAsuCdi;
END:
	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function provides the subsystem hash address.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	SwHashAddr	Pointer to the variable to store subsystem hash.
 *
 * @return
 *	- XASUFW_SUCCESS, if subsystem hash address is retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_SEND_IPI_REQ_FAIL, if IPI request is failed.
 *	- XASUFW_OCP_READ_IPI_RESP_FAIL, if IPI response read is failed.
 *
 *************************************************************************************************/
static s32 XOcp_GetSubsystemHashAddr(u32 SubsystemId, u32 *SwHashAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XOCP_SUBSYS_HASH_TX_PAYLOAD_SIZE];
	s32 Response = XASUFW_FAILURE;
	static u8 SwHash[XASU_SHA_384_HASH_LEN] = {0U};

	/** Prepare IPI request payload. */
	Payload[XASUFW_BUFFER_INDEX_ZERO] = XASUFW_PLM_IPI_HEADER(XOCP_SUBSYS_HASH_TX_CMD_LEN,
			XASUFW_PLM_CMD_ID_SUBSYSTEM_HASH_TX_ID, XASUFW_PLM_ASU_MODULE_ID);
	Payload[XASUFW_BUFFER_INDEX_ONE] = SubsystemId;
	Payload[XASUFW_BUFFER_INDEX_TWO] = (u32)(UINTPTR)SwHash;

	/** Send IPI request to PLM to get subsystem hash. */
	Status = XAsufw_SendIpiToPlm(Payload, XOCP_SUBSYS_HASH_TX_PAYLOAD_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_SEND_IPI_REQ_FAIL);
		goto END;
	}

	/** Read response received from PLM. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm((u32 *)(UINTPTR)&Response,
					   XOCP_SUBSYS_HASH_TX_PAYLOAD_RESP_SIZE);
	if ((Status != XASUFW_SUCCESS) || (Response != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_READ_IPI_RESP_FAIL);
		goto END;
	}

	/** Provide subsystem hash address to caller. */
	*SwHashAddr = (u32)SwHash;

END:
	return Status;
}
#endif /* XASU_OCP_ENABLE */
/** @} */
