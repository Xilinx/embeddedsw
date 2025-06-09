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
#include "xasu_eccinfo.h"
#include "xasufw_hw.h"
#include "xasufw_ipi.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
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

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/
static XOcp_DeviceKeys DevIkData;	/**< Device identity key data */

/************************************ Function Prototypes ****************************************/
static s32 XOcp_GenerateDevIkAkPvtKey(const XOcp_PrivateKeyGen *PvtKeyInfo);
static s32 XOcp_GetAsuCdiAddr(u32 *AsuCdiAddr);
static s32 XOcp_GenerateDevIk(XAsufw_Dma *DmaPtr);

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

	/** Enable TRNG to default mode. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_EnableDefaultMode(TrngInstance);

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
#endif /* XASU_OCP_ENABLE */
/** @} */
