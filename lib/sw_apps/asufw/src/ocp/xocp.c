/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
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
#include "xasu_ocp_common.h"
#include "xasu_ocpinfo.h"
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
#include "xasu_def.h"
#include "x509_asufw.h"
#include "xasufw_perf.h"
#include "xasufw_memory.h"
#include "xkeymanager.h"

#ifdef XASU_OCP_ENABLE
/********************************** Constant Definitions *****************************************/
_Static_assert(sizeof(XOcp_CdoData) <= XASUFW_OCP_CDO_DATA_SIZE,
	       "OCP CDO data block size exceeds the OCP CDO data block size limit");

/************************************ Macro Definitions ******************************************/
#define XOCP_DICE_CDI_SIZE_IN_BYTES	(48U)		/**< CDI size in bytes */
#define XOCP_TRNG_DF_LENGTH		(2U)		/**< TRNG DF length to generate ECC pvt
							key */
#define XOCP_TRNG_DRBG_SEED_LIFE	(7U)		/**< User config seed life */
#define XOCP_DEV_KEY_RANDOM_BUF_SIZE	(64U)		/**< Maximum device private key size */

#define XOCP_SUBSYS_EVENT_MASK		(0x01U)		/**< Subsystem event mask */
#define XOCP_SUBSYS_EVENT_SHIFT		(1U)		/**< Subsystem event size */
#define XOCP_X509_CERT_FIELD_LEN	(4U)		/**< Certificate size field length */

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/
static XOcp_DeviceKeys DevIkData;	/**< Device identity key data */
static XOcp_DeviceKeys DevAkData[XOCP_MAX_USER_OCP_SUBSYS];	/**< Device attestation key data */
static u8 AsuCdiValid = (u8)XASU_STATUS_FAIL;	/**< ASU CDI validation flag */

/************************************ Function Prototypes ****************************************/
static s32 XOcp_GenerateDevIkAkPvtKey(const XOcp_PrivateKeyGen *PvtKeyInfo);
static s32 XOcp_GenerateSwCdi(u32 SubsystemId, XAsufw_Dma *DmaPtr, u8 *SwCdi, u32 CdiLen);
static s32 XOcp_GetAsuCdiAddr(u32 *AsuCdiAddr);
static s32 XOcp_GenerateDevIk(XAsufw_Dma *DmaPtr);
static s32 XOcp_GetSubsystemHashAddr(u32 SubsystemId, u32 *SwHashAddr);
static s32 XOcp_GenerateDevAk(u32 SubsysIdx, XAsufw_Dma *DmaPtr);
static s32 XOcp_GetSubsytemIndex(u32 SubsystemId, u32 *SubsystemIdx);
static s32 XOcp_StoreDeviceKeyPairInVault(XOcp_DeviceKeys *DevKey, const u8 *PvtKey,
					  const u8 *PubKey);
static s32 XOcp_GetDeviceKeyPvtKeyPtr(const XOcp_DeviceKeys *DevKey, const u8 **PvtKeyPtr);
static s32 XOcp_GetDeviceKeyPubKeyPtr(const XOcp_DeviceKeys *DevKey, const u8 **PubKeyPtr);

/*************************************************************************************************/
/**
 * @brief	This function provides OCP event mask after getting from PLM.
 *
 * @param	EventMask	Pointer to store event mask.
 *
 * @return
 *	- XASUFW_SUCCESS, if event mask retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_SEND_IPI_REQ_FAIL, if IPI request is failed.
 *	- XASUFW_OCP_READ_IPI_RESP_FAIL, if IPI read response is failed.
 *
 *************************************************************************************************/
s32 XOcp_GetOcpEventMaskFromPlm(u32 *EventMask)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XOCP_GET_OCP_EVENT_MASK_PAYLOAD_SIZE];
	u32 Response[XOCP_GET_OCP_EVENT_MASK_PAYLOAD_RESP_SIZE] = {0U};

	/** Validate input parameter. */
	if (EventMask == NULL) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Prepare IPI request payload. */
	Payload[XASUFW_BUFFER_INDEX_ZERO] =
			XASUFW_PLM_IPI_HEADER(XOCP_GET_OCP_EVENT_MASK_CMD_LEN,
					      XASUFW_PLM_CMD_ID_GET_OCP_EVENT_MASK,
					      XASUFW_PLM_ASU_MODULE_ID);

	/** Send IPI request to PLM to get OCP event mask. */
	Status = XAsufw_SendIpiToPlm(Payload, XOCP_GET_OCP_EVENT_MASK_PAYLOAD_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_SEND_IPI_REQ_FAIL;
		goto END;
	}

	/** Read response received from PLM. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm(Response, XOCP_GET_OCP_EVENT_MASK_PAYLOAD_RESP_SIZE);
	if ((Status != XASUFW_SUCCESS) ||
	    (Response[XASUFW_BUFFER_INDEX_ZERO] != (u32)XASUFW_SUCCESS)) {
		Status = XASUFW_OCP_READ_IPI_RESP_FAIL;
		goto END;
	}

	*EventMask = Response[XASUFW_BUFFER_INDEX_ONE];

END:
	return Status;
}

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
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_DEVIK_GENERATION_FAIL, if DevIk generation is failed.
 *	- XASUFW_OCP_DEVAK_GENERATION_FAIL, if DevAk generation is failed.
 *
 *************************************************************************************************/
s32 XOcp_GenerateDeviceKeys(XAsufw_Dma *DmaPtr, u32 EventMask)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Idx = XASUFW_VALUE_ONE;
	u32 EvtMask = EventMask;

	/** Validate input parameter. */
	if (DmaPtr == NULL) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Generate DevIK key pair. */
	if (((EvtMask & XOCP_SUBSYS_EVENT_MASK) == XOCP_SUBSYS_EVENT_MASK)) {
		/**
		 * Reset ASU CDI validation flag as it is needed to generate CDI for each ASUFW/PLM
		 * update.
		 */
		AsuCdiValid = (u8)XASU_STATUS_FAIL;
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

	Status = XASUFW_SUCCESS;

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
 *	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if DevAk private key cannot be fetched from the
 *	  ASU key vault.
 *
 *************************************************************************************************/
s32 XOcp_AttestWithDevAk(XAsufw_Dma *DmaPtr, const XAsu_OcpDevAkAttest *OcpAttestParam,
			 u32 SubsystemId)
{
	/**
	 * Capture the start time of the OCP attestation operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_OCP_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XEcc *EccInstance = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	u32 SubsysIdx = 0U;
	const u8 *DevAkPvtKey = NULL;

	/** Validate input parameter. */
	if ((DmaPtr == NULL) || (OcpAttestParam == NULL) || (SubsystemId == 0U)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Validate attestation parameters. */
	Status = XAsu_OcpValidateAttestParams(OcpAttestParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Get subsystem index using subsystem ID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GetSubsytemIndex(SubsystemId, &SubsysIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_SUBSYSTEM_INDEX;
		goto END;
	}

	/** Check whether DevAk key is ready or not. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (DevAkData[SubsysIdx].IsKeyReady != (u8)XASU_TRUE) {
		Status = XASUFW_OCP_DEVAK_NOT_READY;
		goto END;
	}

	/** Fetch the DevAk private key from the ASU key vault. */
	Status = XOcp_GetDeviceKeyPvtKeyPtr(&DevAkData[SubsysIdx], &DevAkPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
		goto END;
	}

	/* Generate the signature using DevAk private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GenerateSignature(EccInstance, DmaPtr, XASU_ECC_NIST_P384,
					XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
					(u64)(UINTPTR)DevAkPvtKey,
					NULL, OcpAttestParam->DataAddr,
					OcpAttestParam->DataLen, OcpAttestParam->SignatureAddr);

	/**
	 * Measure and print the performance time for the OCP attestation operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_OCP_ID);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns DevIk structure address.
 *
 * @return
 *	- Address of DevIk structure.
 *
 *************************************************************************************************/
XOcp_DeviceKeys* XOcp_GetDevIk(void)
{
	return &DevIkData;
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
 *	- XASUFW_OCP_GET_ASU_CDI_FAIL, if ASU CDI is not retrieved successfully.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- XASUFW_OCP_GEN_DEVIK_PVT_KEY_FAIL, if DevIk private key generation is failed.
 *	- XASUFW_OCP_GEN_DEVIK_PUB_KEY_FAIL, if DevIk public key generation is failed.
 *	- XASUFW_OCP_DEVIK_STORE_IN_VAULT_FAIL, if storing the DevIk key pair in the ASU key vault
 *	  fails.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateDevIk(XAsufw_Dma *DmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihClr = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XOcp_PrivateKeyGen PvtKeyInfo;
	u32 AsuCdiAddr = 0U;
	u8 PersonalString[XOCP_PERSONAL_STRING_LEN] = {0U};
	u8 PvtKey[XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES] = {0U};
	u8 PubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};

	/** Get ASU CDI seed. */
	Status = XOcp_GetAsuCdiAddr(&AsuCdiAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GET_ASU_CDI_FAIL);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Copy Device DNA as personalized string. */
	Status = Xil_SMemCpy(PersonalString, XOCP_PERSONAL_STRING_LEN, (u8 *)EFUSE_CACHE_DNA_0,
			     EFUSE_CACHE_DNA_SIZE, EFUSE_CACHE_DNA_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Initially set IsKeyReady to false. */
	DevIkData.IsKeyReady = (u8)XASU_FALSE;

	/** Generate DevIk private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	PvtKeyInfo.CdiAddr = AsuCdiAddr;
	PvtKeyInfo.CdiLength = XOCP_DICE_CDI_SIZE_IN_BYTES;
	PvtKeyInfo.PerStringAddr = (u32)(UINTPTR)PersonalString;
	PvtKeyInfo.PvtKeyAddr = (u32)(UINTPTR)PvtKey;
	Status = XOcp_GenerateDevIkAkPvtKey(&PvtKeyInfo);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_DEVIK_PVT_KEY_FAIL);
		goto END_CLR;
	}

	/** Generate DevIk public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, XASU_ECC_NIST_P384,
					XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
					(u64)(UINTPTR)PvtKey,
					(u64)(UINTPTR)PubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_DEVIK_PUB_KEY_FAIL);
		goto END_CLR;
	}

	/** Store the DevIk key pair in the ASU key vault. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_StoreDeviceKeyPairInVault(&DevIkData, PvtKey, PubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DEVIK_STORE_IN_VAULT_FAIL);
		goto END_CLR;
	}

	/** Set IsKeyReady to true. */
	DevIkData.IsKeyReady = (u8)XASU_TRUE;

END_CLR:
	/** Zeroize the transient private and public key buffers once the pair is stored. */
	XFIH_CALL(Xil_SecureZeroize, XFihClr, ClearStatus, PvtKey,
		  XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihClr, ClearStatus, PubKey,
		  XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

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
 *	- XASUFW_OCP_MAX_SUBSYSTEMS_EXCEEDED, if number of subsystems in CDO exceeds maximum.
 *	- XASUFW_OCP_INVALID_SUBSYSTEM_INDEX, if subsystem index is invalid.
 *	- XASUFW_OCP_GEN_DEVAK_PVT_KEY_FAIL, if DevAk private key generation is failed.
 *	- XASUFW_OCP_GEN_DEVAK_PUB_KEY_FAIL, if DevAk public key generation is failed.
 *	- XASUFW_OCP_GEN_SUBSYSTEM_CDI_FAIL, if subsystem CDI generation is failed.
 *	- XASUFW_OCP_DEVAK_STORE_IN_VAULT_FAIL, if storing the DevAk key pair in the ASU key vault
 *	  fails.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateDevAk(u32 SubsysIdx, XAsufw_Dma *DmaPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihClr = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XOcp_PrivateKeyGen PvtKeyInfo;
	u8 SwCdi[XOCP_DICE_CDI_SIZE_IN_BYTES] = {0U};
	const XOcp_CdoData *CdoData = (XOcp_CdoData *)XASUFW_OCP_CDO_DATA_ADDR;
	const u8 *PersStr = NULL;
	u8 PvtKey[XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES] = {0U};
	u8 PubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};

	/** Validate number of subsystems provided in CDO. */
	if (CdoData->NumSubsys > XOCP_MAX_USER_OCP_SUBSYS) {
		Status = XASUFW_OCP_MAX_SUBSYSTEMS_EXCEEDED;
		goto END;
	}

	/**
	 * Generate DevAk key pair only if subsystem index doesn't exceed the maximum number of
	 * DevAk supported.
	 */
	if (SubsysIdx >= XOCP_MAX_USER_OCP_SUBSYS) {
		Status = XASUFW_OCP_INVALID_SUBSYSTEM_INDEX;
		goto END;
	}

	/** Initially set IsKeyReady to false. */
	DevAkData[SubsysIdx].IsKeyReady = (u8)XASU_FALSE;

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
	PvtKeyInfo.PvtKeyAddr = (u32)(UINTPTR)PvtKey;
	Status = XOcp_GenerateDevIkAkPvtKey(&PvtKeyInfo);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_DEVAK_PVT_KEY_FAIL);
		goto END_CLR;
	}

	/** Generate DevAk public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, XASU_ECC_NIST_P384,
					XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
					(u64)(UINTPTR)PvtKey,
					(u64)(UINTPTR)PubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GEN_DEVAK_PUB_KEY_FAIL);
		goto END_CLR;
	}

	/** Store the DevAk key pair in the ASU key vault. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_StoreDeviceKeyPairInVault(&DevAkData[SubsysIdx], PvtKey, PubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DEVAK_STORE_IN_VAULT_FAIL);
		goto END_CLR;
	}

	/** Set IsKeyReady to true. */
	DevAkData[SubsysIdx].IsKeyReady = (u8)XASU_TRUE;

END_CLR:
	/** Zeroize the transient private key, public key and subsystem CDI buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihClr, ClearStatus, PvtKey,
		  XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihClr, ClearStatus, PubKey,
		  XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihClr, ClearStatus, SwCdi,
		  XOCP_DICE_CDI_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

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
	Status = XRsa_EccGeneratePvtKey(XASU_ECC_NIST_P384, XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
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
 *	- XASUFW_OCP_MAX_SUBSYSTEMS_EXCEEDED, if number of subsystems in CDO exceeds maximum.
 *
 *************************************************************************************************/
static s32 XOcp_GetSubsytemIndex(u32 SubsystemId, u32 *SubsystemIdx)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XOcp_CdoData *CdoData = (XOcp_CdoData *)XASUFW_OCP_CDO_DATA_ADDR;
	u32 Idx;

	/** Validate number of subsystems provided in CDO. */
	if (CdoData->NumSubsys > XOCP_MAX_USER_OCP_SUBSYS) {
		Status = XASUFW_OCP_MAX_SUBSYSTEMS_EXCEEDED;
		goto END;
	}

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
 * @brief	This function stores a device key pair (private and public keys) in the ASU key
 *		vault and records the resulting key identifiers in the device key structure. Any
 *		previously stored key pair for the same device key is removed first so that key
 *		regeneration does not exhaust the sub-vault or leave stale keys behind.
 *
 * @param	DevKey		Pointer to the device key structure holding the key identifiers.
 * @param	PvtKey		Pointer to the ECC private key buffer to be stored.
 * @param	PubKey		Pointer to the ECC public key buffer (X and Y coordinates) to be
 *				stored.
 *
 * @return
 *	- XASUFW_SUCCESS, if the key pair is stored successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_KEYMANAGER_UPDATE_PVT_KEY_FAIL, if private key storage fails.
 *
 *************************************************************************************************/
static s32 XOcp_StoreDeviceKeyPairInVault(XOcp_DeviceKeys *DevKey, const u8 *PvtKey,
					  const u8 *PubKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(DeleteStatus, XASUFW_FAILURE);
	XAsu_KeyManagerParams KmParams = {0U};

	/**
	 * Delete any previously stored key pair before storing the new one. In case key deletion
	 * fails, print the error message and continue storing the new key pair in the vault.
	 */
	if (DevKey->PvtKeyId != 0U) {
		DeleteStatus = XKeyManager_DeleteKey(DevKey->PvtKeyId,
						     XKEYMANAGER_ASU_SUBSYSTEM_ID);
		if (DeleteStatus != XASUFW_SUCCESS) {
			XAsufw_Printf(DEBUG_GENERAL,
				"Deleting private key from vault failed: 0x%x\n", DeleteStatus);
		}

		/** Reset the private key ID so OCP does not use the stale private key. */
		DevKey->PvtKeyId = 0U;
	}

	if (DevKey->PubKeyId != 0U) {
		ASSIGN_VOLATILE(DeleteStatus, XASUFW_FAILURE);
		DeleteStatus = XKeyManager_DeleteKey(DevKey->PubKeyId,
						     XKEYMANAGER_ASU_SUBSYSTEM_ID);
		if (DeleteStatus != XASUFW_SUCCESS) {
			XAsufw_Printf(DEBUG_GENERAL,
				"Deleting public key from vault failed: 0x%x\n", DeleteStatus);
		}

		/** Reset the public key ID so OCP does not use the stale public key. */
		DevKey->PubKeyId = 0U;
	}

	/** Set up the device key metadata for ASU key vault storage. */
	KmParams.KeyMetadata.Length = XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES;
	KmParams.KeyMetadata.EpochTime = 0U;
	KmParams.KeyMetadata.UsageCount = XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE;
	KmParams.KeyMetadata.VaultId = XKEYMANAGER_ASU_VAULT_ID;
	KmParams.KeyMetadata.KeyUseCase = XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE;

	/** Store the private key in the ECC private sub-vault. */
	KmParams.KeyMetadata.KeyType = (u8)XASU_ECC_PVT_SUBVAULT_ID;
	KmParams.KeyObjectAddr = (u64)(UINTPTR)PvtKey;
	Status = XKeyManager_StoreKeyInAsuVault(&KmParams, &DevKey->PvtKeyId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_UPDATE_PVT_KEY_FAIL);
		goto END;
	}

	/** Store the public key in the ECC public sub-vault. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	KmParams.KeyMetadata.KeyUseCase = XASU_KEYMANAGER_ECC_PUB_SIGN_VER_USE_CASE;
	KmParams.KeyMetadata.KeyType = (u8)XASU_ECC_PUB_SUBVAULT_ID;
	KmParams.KeyObjectAddr = (u64)(UINTPTR)PubKey;
	Status = XKeyManager_StoreKeyInAsuVault(&KmParams, &DevKey->PubKeyId);
	if (Status != XASUFW_SUCCESS) {
		/* Clean up the already stored private key if public key storage fails. */
		ASSIGN_VOLATILE(DeleteStatus, XASUFW_FAILURE);
		DeleteStatus = XKeyManager_DeleteKey(DevKey->PvtKeyId,
						     XKEYMANAGER_ASU_SUBSYSTEM_ID);
		if (DeleteStatus != XASUFW_SUCCESS) {
			XAsufw_Printf(DEBUG_GENERAL,
				"Deleting new stored private key failed: 0x%x\n", DeleteStatus);
		}

		/** Reset the private key ID so OCP does not use the stale private key. */
		DevKey->PvtKeyId = 0U;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function fetches a device private key from the ASU key vault.
 *
 * @param	DevKey		Pointer to the device key structure holding the private key
 *				identifier.
 * @param	PvtKeyPtr	Pointer that is updated with the address of the private key material
 *				within the vault.
 *
 * @return
 *	- XASUFW_SUCCESS, if the private key is fetched successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XOcp_GetDeviceKeyPvtKeyPtr(const XOcp_DeviceKeys *DevKey, const u8 **PvtKeyPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XKeyManager_EccPvtKeyObject *PvtKeyObj = NULL;

	/** Fetch the private key object from the ASU vault for a signature-generation use case. */
	PvtKeyObj = (const XKeyManager_EccPvtKeyObject *)XKeyManager_GetKeyObjectPtr(
			DevKey->PvtKeyId, XKEYMANAGER_ASU_SUBSYSTEM_ID,
			XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE, (u8)TRUE);
	if (PvtKeyObj != NULL) {
		*PvtKeyPtr = PvtKeyObj->PrivateKey;
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function fetches a device public key from the ASU key vault.
 *
 * @param	DevKey		Pointer to the device key structure holding the public key
 *				identifier.
 * @param	PubKeyPtr	Pointer that is updated with the address of the public key material
 *				(X and Y coordinates) within the vault.
 *
 * @return
 *	- XASUFW_SUCCESS, if the public key is fetched successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XOcp_GetDeviceKeyPubKeyPtr(const XOcp_DeviceKeys *DevKey, const u8 **PubKeyPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XKeyManager_EccPubKeyObject *PubKeyObj = NULL;

	/** Fetch the public key object from the ASU vault for a signature-verification use case. */
	PubKeyObj = (const XKeyManager_EccPubKeyObject *)XKeyManager_GetKeyObjectPtr(
			DevKey->PubKeyId, XKEYMANAGER_ASU_SUBSYSTEM_ID,
			XASU_KEYMANAGER_ECC_PUB_SIGN_VER_USE_CASE, (u8)TRUE);
	if (PubKeyObj != NULL) {
		*PubKeyPtr = PubKeyObj->PublicKey;
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function fetches the DevIk public key from the ASU key vault.
 *
 * @param	PubKeyPtr	Pointer that is updated with the address of the DevIk public key
 *				material (X and Y coordinates) within the vault.
 *
 * @return
 *	- XASUFW_SUCCESS, if the DevIk public key is fetched successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_KEY_MGMT_NOT_READY, if the DevIk key pair is not generated.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *
 *************************************************************************************************/
s32 XOcp_GetDevIkPublicKey(const u8 **PubKeyPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameter. */
	if (PubKeyPtr == NULL) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Ensure the DevIk key pair has been generated. */
	if (DevIkData.IsKeyReady != (u8)XASU_TRUE) {
		Status = XASUFW_OCP_KEY_MGMT_NOT_READY;
		goto END;
	}

	/** Fetch the DevIk public key from the ASU vault. */
	Status = XOcp_GetDeviceKeyPubKeyPtr(&DevIkData, PubKeyPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the X.509 certificate and DevIK CSR for device keys.
 *
 * @param	SubsystemId		Subsystem ID for which certificate is to be generated.
 * @param	OcpCertParam		Pointer to the OCP certificate parameters.
 * @param	PlatData		Pointer to platform specific data.
 * @param	IsCsr			Indicates if this is a Certificate Signing Request(CSR).
 *
 * @return
 *	- XASUFW_SUCCESS, if certificate is generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_INVALID_PARAM, if input parameter is invalid.
 *	- XASUFW_OCP_KEY_MGMT_NOT_READY, if DevIk pair is not generated.
 *	- XASUFW_OCP_DEVAK_NOT_READY, if DevAk pair is not generated.
 *	- XASUFW_OCP_INVALID_SUBSYSTEM_INDEX, if subsystem index is invalid.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if zeroize memset is failed.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- XASUFW_OCP_X509_CERT_GEN_FAIL, if certificate generation is failed.
 *	- XASUFW_OCP_INVALID_BUF_SIZE, if buffer size is not sufficient.
 *	- XASUFW_DMA_COPY_FAIL, if DMA transfer is failed.
 *	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if device key cannot be fetched from the vault.
 *
 *************************************************************************************************/
s32 XOcp_GetX509Cert(u32 SubsystemId, const XAsu_OcpCertParams *OcpCertParam, void *PlatData,
		     u8 IsCsr)
{
	/**
	 * Capture the start time of the OCP X.509 certificate generation operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_OCP_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	X509_Config CertCfg;
	XOcp_CdoData *CdoData = (XOcp_CdoData *)XASUFW_OCP_CDO_DATA_ADDR;
	const X509_PlatData *PlatDataPtr = (X509_PlatData *)PlatData;
	XOcp_CertData CertData;
	u32 SubsysIdx = 0U;
	u32 CertSize = 0U;
	const u8 *IssuerPvtKey = NULL;
	const u8 *IssuerPubKey = NULL;
	const u8 *SubjectPubKey = NULL;
	u8 CertBuf[X509_CERTIFICATE_MAX_SIZE_IN_BYTES];

	/** Validate OCP certificate parameters. */
	Status = XAsu_OcpValidateCertParams(OcpCertParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Validate platform data and CSR flag parameter. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((PlatData == NULL) || ((IsCsr != XASU_FALSE) && (IsCsr != XASU_TRUE))) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Initialize certificate data. */
	CertData.DevKeyType = OcpCertParam->DevKeySel;
	CertData.CertAddr = (u64)(UINTPTR)CertBuf;
	CertData.CertMaxSize = X509_CERTIFICATE_MAX_SIZE_IN_BYTES;

	/** Subsystem ID should be ASU subsystem ID for DevIk. */
	if ((CertData.DevKeyType == (u32)XOCP_DEVIK) && (SubsystemId != XASUFW_SUBSYTEM_ID)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Check if DevIk is ready or not. */
	if (DevIkData.IsKeyReady != (u32)XASU_TRUE) {
		Status = XASUFW_OCP_KEY_MGMT_NOT_READY;
		goto END;
	}

	/** Fetch the DevIk private key from the ASU key vault. */
	Status = XOcp_GetDeviceKeyPvtKeyPtr(&DevIkData, &IssuerPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
		goto END;
	}

	/** Fetch the DevIk public key from the ASU key vault. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GetDeviceKeyPubKeyPtr(&DevIkData, &IssuerPubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
		goto END;
	}

	/** Set X.509 certificate configurations. */
	CertCfg.PubKeyInfo.PubKeyType = X509_PUB_KEY_ECC;
	CertCfg.PubKeyInfo.EccCurveType = X509_ECC_CURVE_TYPE_384;
	CertCfg.IsCsr = IsCsr;
	CertCfg.IssuerPubKeyLen = XAsu_DoubleCurveLength(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	CertCfg.IssuerPublicKey = IssuerPubKey;
	CertCfg.IssuerPrvtKey = IssuerPvtKey;
	CertCfg.PlatformData = PlatData;

	if (CertData.DevKeyType == (u32)XOCP_DEVIK) {
		/**
		 * Get user configurations for DevIk certificate.
		 * For DevIk, subsystem shall be ASU and subsystem index 0.
		 */
		CertCfg.UserCfg = &CdoData->AsuSubsysData.UserCfg;

		/** Set subject public key for DevIk certificate. */
		CertCfg.PubKeyInfo.SubjectPubKeyLen =
			XAsu_DoubleCurveLength(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
		CertCfg.PubKeyInfo.SubjectPublicKey = IssuerPubKey;
		CertCfg.IsSelfSigned = (u8)XASU_TRUE;
	} else {
		/** Get subsystem index using subsystem ID. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XOcp_GetSubsytemIndex(SubsystemId, &SubsysIdx);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_OCP_INVALID_SUBSYSTEM_INDEX;
			goto END;
		}

		/** Check if DevAk is ready or not. */
		if (DevAkData[SubsysIdx].IsKeyReady != (u8)XASU_TRUE) {
			Status = XASUFW_OCP_DEVAK_NOT_READY;
			goto END;
		}

		/** Fetch the DevAk public key from the ASU key vault to use as the subject key. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XOcp_GetDeviceKeyPubKeyPtr(&DevAkData[SubsysIdx], &SubjectPubKey);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED;
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
		CertCfg.PubKeyInfo.SubjectPubKeyLen =
			XAsu_DoubleCurveLength(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
		CertCfg.PubKeyInfo.SubjectPublicKey = SubjectPubKey;
		CertCfg.IsSelfSigned = (u8)XASU_FALSE;
	}

	/** Generate X.509 certificate. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenerateX509Cert(CertData.CertAddr, CertData.CertMaxSize,
				       &CertSize, &CertCfg);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_X509_CERT_GEN_FAIL);
		goto END;
	}

	/** Validate Certificate buffer length */
	if (OcpCertParam->CertBufLen < CertSize) {
		Status = XASUFW_OCP_INVALID_BUF_SIZE;
		goto END;
	}

	/** Copy generated certificate to destination address using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(PlatDataPtr->DmaPtr, CertData.CertAddr, OcpCertParam->CertBufAddr,
			       CertSize, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	/** Copy actual size of generated certificate to destination address using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(PlatDataPtr->DmaPtr, (u64)(UINTPTR)&CertSize,
			       (u64)(UINTPTR)OcpCertParam->CertActualSize,
			       XOCP_X509_CERT_FIELD_LEN, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	ReturnStatus = XASUFW_OCP_CERT_GENERATION_SUCCESS;

	/**
	 * Measure and print the performance time for the OCP X.509 certificate generation
	 * operation, if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_OCP_ID);

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
 * @brief	This function sends IPI request to PLM to get ASU CDI.
 *
 * @param	XOcpAsuCdi	Pointer to the buffer to store the ASU CDI.
 *
 * @return
 *	- XASUFW_SUCCESS, if ASU CDI address is retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_SEND_IPI_REQ_FAIL, if IPI request is failed.
 *	- XASUFW_OCP_READ_IPI_RESP_FAIL, if IPI response read is failed.
 *
 *************************************************************************************************/
static inline s32 XOcp_GetAsuCdiFromPlm(const u8 *XOcpAsuCdi)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Payload[XOCP_ASU_CDI_TX_PAYLOAD_SIZE];
	s32 Response = XASUFW_FAILURE;

	/** Prepare IPI request payload. */
	Payload[XASUFW_BUFFER_INDEX_ZERO] = XASUFW_PLM_IPI_HEADER(XOCP_ASU_CDI_TX_ID_CMD_LEN,
			XASUFW_PLM_CMD_ID_ASU_CDI_TX_ID, XASUFW_PLM_ASU_MODULE_ID);
	Payload[XASUFW_BUFFER_INDEX_ONE] = (u32)(UINTPTR)XOcpAsuCdi;

	/** Send IPI request to PLM to get ASU CDI. */
	Status = XAsufw_SendIpiToPlm(Payload, XOCP_ASU_CDI_TX_PAYLOAD_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_SEND_IPI_REQ_FAIL;
		goto END;
	}

	/** Read response received from PLM. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm((u32 *)(UINTPTR)&Response,
					   XOCP_ASU_CDI_TX_PAYLOAD_RESP_SIZE);
	if ((Status != XASUFW_SUCCESS) || (Response != XASUFW_SUCCESS)) {
		Status = XASUFW_OCP_READ_IPI_RESP_FAIL;
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provides ASU CDI address.
 *
 * @param	AsuCdiAddr	Pointer to the variable to store ASU CDI address.
 *
 * @return
 *	- XASUFW_SUCCESS, if ASU CDI address is retrieved successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_GET_ASU_CDI_FROM_PLM_FAIL, if getting ASU CDI fails.
 *
 *************************************************************************************************/
static s32 XOcp_GetAsuCdiAddr(u32 *AsuCdiAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	static const u8 XOcpAsuCdi[XOCP_DICE_CDI_SIZE_IN_BYTES] = {0U};

	/** Send IPI request to PLM if ASU CDI is not available. */
	if (AsuCdiValid != (u8)XASU_STATUS_PASS) {
		Status = XOcp_GetAsuCdiFromPlm(XOcpAsuCdi);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_OCP_GET_ASU_CDI_FROM_PLM_FAIL;
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

	/** Mark ASU CDI as valid. */
	AsuCdiValid = (u8)XASU_STATUS_PASS;

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
		Status = XASUFW_OCP_SEND_IPI_REQ_FAIL;
		goto END;
	}

	/** Read response received from PLM. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ReadIpiRespFromPlm((u32 *)(UINTPTR)&Response,
					   XOCP_SUBSYS_HASH_TX_PAYLOAD_RESP_SIZE);
	if ((Status != XASUFW_SUCCESS) || (Response != XASUFW_SUCCESS)) {
		Status = XASUFW_OCP_READ_IPI_RESP_FAIL;
		goto END;
	}

	/** Provide subsystem hash address to caller. */
	*SwHashAddr = (u32)SwHash;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates and returns a Hardware Unique Key (HUK).
 *
 * @param	HukBuf		Pointer to the output buffer where HUK will be written.
 *				Buffer must be at least XASU_OCP_HUK_SIZE_IN_BYTES (32 bytes).
 *
 * @return
 *	- XASUFW_SUCCESS, if HUK generation is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XOcp_GetHuk(u8 *HukBuf)
{
	CREATE_VOLATILE(Status, XASUFW_OCP_INVALID_PARAM);
	/**
	 * Temporary fixed HUK (256-bit AES key) for development purposes.
	 * TODO: This will be replaced with a proper key derivation implementation.
	 */
	static const u8 TempHuk[XASU_OCP_HUK_SIZE_IN_BYTES] = {
		0x2F, 0xA8, 0x71, 0xD4, 0x5B, 0xE9, 0x3C, 0x16,
		0x8A, 0x4D, 0xF2, 0x67, 0xB5, 0x39, 0xC1, 0x8E,
		0x7C, 0x29, 0xE6, 0x54, 0xA3, 0x1F, 0xDB, 0x68,
		0x95, 0x42, 0xCE, 0x7A, 0xB7, 0x3D, 0xF4, 0x81
	};

	/** Validate input parameters. */
	if (HukBuf == NULL) {
		goto END;
	}

	/** Copy the HUK to the output buffer. */
	Status = Xil_SMemCpy(HukBuf, XASU_OCP_HUK_SIZE_IN_BYTES, TempHuk, XASU_OCP_HUK_SIZE_IN_BYTES,
					XASU_OCP_HUK_SIZE_IN_BYTES);

END:
	return Status;
}

#endif /* XASU_OCP_ENABLE */
/** @} */
