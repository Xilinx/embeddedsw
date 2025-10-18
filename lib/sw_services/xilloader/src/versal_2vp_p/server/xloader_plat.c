/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file versal_2vp/server/xloader_plat.c
*
* This file contains the versal_2vp specific code related to PDI image loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 2.3   sd  10/13/25 Initial release
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/*************************************** Include Files ********************************************/
#include "xloader.h"
#include "xilpdi.h"
#include "xpm_device.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_nodeid.h"
#include "xplmi.h"
#include "xplmi_hw.h"
#include "xplmi_util.h"
#include "xloader_plat.h"
#include "xplmi_err.h"
#include "xloader_ddr.h"
#include "xilpdi.h"
#ifdef PLM_OCP
#include "xsecure_trng.h"
#include "xocp.h"
#include "xloader_secure.h"
#ifdef PLM_OCP_KEY_MNGMT
#include "xocp_keymgmt.h"
#endif
#endif

/************************************ Constant Definitions ****************************************/
#define XLOADER_TCM_0			(0U) /**< TCM 0 */
#define XLOADER_TCM_1			(1U) /**< TCM 1 */
#define XLOADER_RPU_GLBL_CNTL		(0xFF9A0000U) /**< RPU global control */
#define XLOADER_TCMCOMB_MASK		(0x40U) /**< TCM combine mask */
#define XLOADER_TCMCOMB_SHIFT		(6U) /**< TCM combine shift */

#define PLM_VP1802_POR_SETTLE_TIME	(25000U) /**< Flag indicates POR settle time for VP1802 */

#ifdef PLM_OCP
#define XLOADER_PCR_MEASUREMENT_INDEX_MASK		(0xFFFF0000U) /**< Mask for PCR Measurement
									index */
#define XLOADER_PCR_MEASUREMENT_INDEX_SHIFT		(16U) /**< PCR Measurement index shift */

#ifdef PLM_OCP_KEY_MNGMT
#define XLOADER_INVALID_DEVAK_INDEX			(0xFFFFFFFFU) /**< INVALID DEVAK INDEX */
#endif

#define XLOADER_EFUSE_ROM_RSVD_CACHE_ADDRESS		(0xF1250090U) /**< ROM Reserved eFuse cache
									offset */
#define XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK	(0x00000200U) /**< AUTH_KEYS_TO_HASH eFuse
									bit mask */
#define XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_SHIFT	(9U) /**< AUTH_KEYS_TO_HASH eFuse bit
							       shift */
#endif
/**
 * @{
 * @cond DDR calibration errors
 */
#define DDRMC_OFFSET_CALIB_PTR					(0x8400U)
#define DDRMC_OFFSET_CALIB_ERR_SUB_STAGE			(0x8404U)
#define DDRMC_OFFSET_CALIB_ERR_RANK				(0x8408U)
#define DDRMC_OFFSET_CALIB_ERR					(0x840CU)
#define DDRMC_OFFSET_CALIB_ERR_DATA_NIBBLE_1			(0x8410U)
#define DDRMC_OFFSET_CALIB_ERR_DATA_NIBBLE_2			(0x8414U)
#define DDRMC_ARRAY_SIZE					(8U)
/**
 * @}
 * @endcond
 */

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/
static void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi);
static int XLoader_RequestTCM(u32 TcmId);
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu);
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len);
static int XLoader_DumpDdrmcRegisters(void);
#if (!defined(PLM_SECURE_EXCLUDE)) && (defined(PLM_OCP))
static int XLoader_RunSha3EngineKat(XilPdi* PdiPtr);
static int XLoader_SpkMeasurement(XLoader_SecureParams* SecureParams, XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendSpkHash(XSecure_Sha3Hash* SpkHash , u32 PcrNo, u32 DigestIndex,
				 u32 PdiType);
static int XLoader_SpkIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendSpkId(XSecure_Sha3Hash* SpkIdHash, u32 PcrInfo, u32 DigestIndex,
			       u32 PdiType);
static int XLoader_EncRevokeIdMeasurement(XLoader_SecureParams* SecurePtr,
					  XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendEncRevokeId(XSecure_Sha3Hash* RevokeIdHash, u32 PcrInfo, u32 DigestIndex,
				     u32 PdiType);
#endif

/************************************ Variable Definitions ****************************************/
#if defined(PLM_OCP)
/**************************************************************************************************/
/**
 * @brief	This function provides pointer to PrtitionHashTable.
 *
 * @return
 * 		- Pointer to PrtitionHashTable Info.
 *
 **************************************************************************************************/
XSecure_Sha3Hash* XLoader_GetPtrnHashTable(void)
{
	static XSecure_Sha3Hash PtrnHashTable[XIH_MAX_PRTNS]
		__attribute__ ((aligned(4U))) = {0}; /** < Partition Hash Storage */

	return &PtrnHashTable[0];
}
#endif

/**************************************************************************************************/
/**
 * @brief	This functions updates the partition data to SHA engine for measurement.
 *
 * @param	PdiPtr 		Pointer to the XilPdi structure.
 * 		DataAddr 	Address of Data for measure the hash.
 * 		DataLen		Length of data for DataMeasument.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
int XLoader_UpdateDataMeasurement(const XilPdi* PdiPtr, u64 DataAddr, u32 DataLen)
{
	int Status = XST_FAILURE;
#if defined(PLM_OCP)
	XLoader_ImageMeasureInfo ImageMeasureInfo = {0U};
	u32 PcrInfo = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].PcrInfo;
	u32 ChecksumType;
	u32 PrtnNum = PdiPtr->PrtnNum;
	ChecksumType = XilPdi_GetChecksumType(&PdiPtr->MetaHdr->PrtnHdr[PrtnNum]);
	/** Update data for measurement only if Authentication or ChecksumType is not SHA3 hash */
	if ((!XilPdi_IsAuthEnabled(&PdiPtr->MetaHdr->ImgHdrTbl)) &&
	    (ChecksumType != XIH_PH_ATTRB_HASH_SHA3)) {
		ImageMeasureInfo.DataAddr = DataAddr;
		ImageMeasureInfo.DataSize = DataLen;
		ImageMeasureInfo.PcrInfo = PcrInfo;
		ImageMeasureInfo.SubsystemID = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].ImgID;
		ImageMeasureInfo.Flags = XLOADER_MEASURE_UPDATE;

		/* Update the data for measurement */
		Status = XLoader_DataMeasurement(&ImageMeasureInfo);
	} else {
		/**
		 * - If authentication or checksum is enabled, SHA engine is used for partition
		 *   authentication or checksum calculation.
		 *   So, data should not be updated to SHA engine from this place.
		 */
		Status = XST_SUCCESS;
	}
#else
	(void)PdiPtr;
	(void)DataAddr;
	(void)DataLen;

	/** Nothing needs to be done in case, PLM_OCP is not defined */
	Status = XST_SUCCESS;
#endif
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function provides ImageInfoTbl pointer.
 *
 * @return
 * 		- Pointer to ImageInfoTbl.
 *
 **************************************************************************************************/
XLoader_ImageInfoTbl *XLoader_GetImageInfoTbl(void)
{
	/* Image Info Table */
	static XLoader_ImageInfoTbl ImageInfoTbl = {
		.Count = 0U,
		.IsBufferFull = FALSE,
	};

	return &ImageInfoTbl;
}

/**************************************************************************************************/
/**
 * @brief	This function provides pointer to PdiList.
 *
 * @return
 * 		- Pointer to PdiList.
 *
 **************************************************************************************************/
XLoader_ImageStore* XLoader_GetPdiList(void)
{
	static XLoader_ImageStore PdiList __attribute__ ((aligned(4U))) = {0U};

	return &PdiList;
}

/**************************************************************************************************/
/**
 * @brief	This function returns the ATFHandoffParams structure address to the caller.
 *
 * @return
 * 		- ATFHandoffParams structure address.
 *
 **************************************************************************************************/
XilPdi_ATFHandoffParams *XLoader_GetATFHandoffParamsAddr(void)
{
	static XilPdi_ATFHandoffParams ATFHandoffParams = {0U}; /**< Instance containing
								 ATF handoff params */
	/* Return ATF Handoff parameters structure address */
	return &ATFHandoffParams;
}

/**************************************************************************************************/
/**
 * @brief	This function provides pointer to BootPDI Info.
 *
 * @return
 * 		- pointer to BootPDI Info.
 *
 **************************************************************************************************/
XilBootPdiInfo* XLoader_GetBootPdiInfo(void)
{
	static XilBootPdiInfo BootPdiInfo
		__attribute__ ((aligned(4U))) = {0}; /** < BootPDI info Storage */

	return &BootPdiInfo;
}

/**************************************************************************************************/
/**
 * @brief	This function is used to start the subsystems in the PDI.
 *
 * @param	PdiPtr Pdi instance pointer.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_WAKEUP_A72_0 if waking up the A72-0 failed during handoff.
 *		- XLOADER_ERR_WAKEUP_A72_1 if waking up the A72-1 failed during handoff.
 *		- XLOADER_ERR_WAKEUP_R5_0 if waking up the R5-0 failed during handoff.
 *		- XLOADER_ERR_WAKEUP_R5_1 if waking up the R5-1 failed during handoff.
 *		- XLOADER_ERR_WAKEUP_R5_L if waking up the R5-L failed during handoff.
 *		- XLOADER_ERR_WAKEUP_PSM if waking up the PSM failed during handoff.
 *
 **************************************************************************************************/
int XLoader_StartImage(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;
	u64 HandoffAddr;
	u32 ExecState;
	u32 VInitHi;
	u32 DeviceId;
	u8 SetAddress = 1U;
	u32 ErrorCode;

	/** - Start Handoff to the cpus */
	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings & XIH_PH_ATTRB_DSTN_CPU_MASK;

		HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;
		ExecState = PdiPtr->HandoffParam[Index].CpuSettings & XIH_PH_ATTRB_A72_EXEC_ST_MASK;
		VInitHi = PdiPtr->HandoffParam[Index].CpuSettings & XIH_PH_ATTRB_HIVEC_MASK;
		Status = XST_FAILURE;
		/** - Wake up each processor */
		switch (CpuId)
		{
			case XIH_PH_ATTRB_DSTN_CPU_A72_0:
				/* APU Core configuration */
				XLoader_A72Config(CpuId, ExecState, VInitHi);
				DeviceId = PM_DEV_ACPU_0;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_A72_0;
				XLoader_Printf(DEBUG_INFO, "Request APU0 wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_A72_1:
				/* APU Core configuration */
				XLoader_A72Config(CpuId, ExecState, VInitHi);
				DeviceId = PM_DEV_ACPU_1;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_A72_1;
				XLoader_Printf(DEBUG_INFO, "Request APU1 wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_0:
				DeviceId = PM_DEV_RPU0_0;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_R5_0;
				XLoader_Printf(DEBUG_INFO, "Request RPU 0 wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_1:
				DeviceId = PM_DEV_RPU0_1;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_R5_1;
				XLoader_Printf(DEBUG_INFO, "Request RPU 1 wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_L:
				DeviceId = PM_DEV_RPU0_0;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_R5_L;
				XLoader_Printf(DEBUG_INFO, "Request RPU wakeup\r\n");
				break;
			case XIH_PH_ATTRB_DSTN_CPU_PSM:
				DeviceId = PM_DEV_PSM_PROC;
				ErrorCode = (u32)XLOADER_ERR_WAKEUP_PSM;
				SetAddress = 0U;
				XLoader_Printf(DEBUG_INFO, "Request PSM wakeup\r\n");
				break;

			default:
				Status = XST_SUCCESS;
				break;
		}
		if (Status != XST_SUCCESS) {
			Status = XPm_RequestWakeUp(PM_SUBSYS_PMC, DeviceId, SetAddress, HandoffAddr,
					0U, XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, Status);
				goto END;
			}
		}
	}

	Status = XST_SUCCESS;

END:
	/* Make Number of handoff CPUs to zero */
	PdiPtr->NoOfHandoffCpus = 0x0U;
	return Status;
}

#ifdef PLM_OCP
/**************************************************************************************************/
/**
 * @brief	This function measures the SPK by calculating SHA3 hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash  is pointer to the XSecure_Sha3Hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XLoader_SpkMeasurement(XLoader_SecureParams* SecurePtr,
	XSecure_Sha3Hash* Sha3Hash)
{
	int Status = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	u32 AuthType;
	u32 SpkLen = 0U;

	AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);

	if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
		SpkLen = XLOADER_SPK_SIZE - XOCP_WORD_LEN;
	} else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
		SpkLen = (XLOADER_ECDSA_P384_KEYSIZE + XLOADER_ECDSA_P384_KEYSIZE);
	} else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521) {
		SpkLen = (XLOADER_ECDSA_P521_KEYSIZE + XLOADER_ECDSA_P521_KEYSIZE);
	} else {
		/* MISRA-C compliance */
	}

	Status = XSecure_ShaDigest(ShaInstPtr, XSECURE_SHA3_384,
			(UINTPTR)&SecurePtr->AcPtr->Spk,
			SpkLen, (u64)(UINTPTR)Sha3Hash, XLOADER_SHA3_LEN);

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function extends the SPK Hash into specified PCR.
 *
 * @param	SpkHash SPK key hash measured.
 * @param	PcrInfo provides the PCR number and Measurement Index to be extended.
 * @param	DigestIndex Digest index in PCR log, applicable to SW PCR only.
 * @param       PdiType Full or Partial or Restore PDI.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XLoader_ExtendSpkHash(XSecure_Sha3Hash* SpkHash , u32 PcrNo, u32 DigestIndex,
				 u32 PdiType)
{
	int Status = XST_FAILURE;
	Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex,
			          (u64)(UINTPTR)&SpkHash->Hash, XLOADER_SHA3_LEN, PdiType);

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function measures the SPK ID by calculating SHA3 hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash  is pointer to the XSecure_Sha3Hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XLoader_SpkIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash)
{
	int Status = XST_FAILURE;
	XSecure_Sha *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	Status = XSecure_ShaDigest(Sha3InstPtr, XSECURE_SHA3_384, (UINTPTR)&SecurePtr->AcPtr->SpkId,
				   sizeof(SecurePtr->AcPtr->SpkId), (u64)(UINTPTR)Sha3Hash,
				   XLOADER_SHA3_LEN);

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function extends the Partition AC SPK ID into specified PCR.
 *
 * @param	SpkIdHash Partition AC SPK ID Hash.
 * @param	PcrInfo provides the PCR number and Measurement Index to be extended.
 * @param	DigestIndex Digest index in PCR log, applicable to SW PCR only.
 * @param       PdiType Full or Partial or Restore PDI.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XLoader_ExtendSpkId(XSecure_Sha3Hash* SpkIdHash, u32 PcrNo, u32 DigestIndex, u32 PdiType)
{
	int Status = XST_FAILURE;
	Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex,
				(u64)(UINTPTR)&SpkIdHash->Hash, XLOADER_SHA3_LEN, PdiType);

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function measures the Encryption Revoke ID by calculating SHA3 hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash  is pointer to the XSecure_Sha3Hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XLoader_EncRevokeIdMeasurement(XLoader_SecureParams* SecurePtr,
					  XSecure_Sha3Hash* Sha3Hash)
{
	int Status = XST_FAILURE;
	XSecure_Sha *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	Status = XSecure_ShaDigest(Sha3InstPtr, XSECURE_SHA3_384,
				   (UINTPTR)&SecurePtr->PrtnHdr->EncRevokeID,
				   sizeof(SecurePtr->PrtnHdr->EncRevokeID), (u64)(UINTPTR)Sha3Hash,
				   XLOADER_SHA3_LEN);

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function extends the Partition Header Revoke ID into specified PCR.
 *
 * @param	RevokeIdHash Partition Header Revocation ID Hash.
 * @param	PcrInfo provides the PCR number and Measurement Index to be extended.
 * @param	DigestIndex Digest index in PCR log, applicable to SW PCR only.
 * @param       PdiType Full or Partial or Restore PDI.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static int XLoader_ExtendEncRevokeId(XSecure_Sha3Hash* RevokeIdHash, u32 PcrNo, u32 DigestIndex,
				     u32 PdiType)
{
	int Status = XST_FAILURE;
	Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex, (u64)(UINTPTR)&RevokeIdHash->Hash,
				  XLOADER_SHA3_LEN, PdiType);

	return Status;
}
#endif
/**************************************************************************************************/
/**
 * @brief	This function measures the Secure Configuration that is SPK, SPK ID and Encryption
 * 		Revoke ID and extends to the specified PCR
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	PcrInfo provides the PCR number and Measurement Index to be extended.
 * @param	DigestIndex is pointer to the DigestIndex across the PCR.
 * @param	OverWrite TRUE or FALSE to overwrite the extended digest or not.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_ERR_SECURE_CONFIG_MEASUREMENT if error in Secure config measurement.
 *
 **************************************************************************************************/
int XLoader_SecureConfigMeasurement(XLoader_SecureParams* SecurePtr, u32 PcrInfo, u32 *DigestIndex,
				    u32 OverWrite)
{
	int Status = XLOADER_ERR_SECURE_CONFIG_MEASUREMENT;
#if defined(PLM_OCP)
	volatile u32 IsAuthenticated = SecurePtr->IsAuthenticated;
	volatile u32 IsAuthenticatedTmp = SecurePtr->IsAuthenticated;
	volatile u32 IsEncrypted = SecurePtr->IsEncrypted;
	volatile u32 IsEncryptedTmp = SecurePtr->IsEncrypted;
	u32 MeasureIdx = (PcrInfo & XOCP_PCR_MEASUREMENT_INDEX_MASK) >> 16U;
	u32 PcrNo = PcrInfo & XOCP_PCR_NUMBER_MASK;
	XSecure_Sha3Hash Sha3Hash = {0U};
	volatile u32 IsAuthKeysToHashEnabled = (XPlmi_In32(XLOADER_EFUSE_ROM_RSVD_CACHE_ADDRESS) &
			XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK) >>
			XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_SHIFT;;
	volatile u32 IsAuthKeysToHashEnabledTmp = (XPlmi_In32(XLOADER_EFUSE_ROM_RSVD_CACHE_ADDRESS) &
			XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK) >>
			XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_SHIFT;;

	if (PcrInfo == XOCP_PCR_INVALID_VALUE) {
                Status = XST_SUCCESS;
                goto END;
        }

	if (((IsAuthKeysToHashEnabled != 0U) || (IsAuthKeysToHashEnabledTmp != 0U)) &&
		((IsAuthenticated == (u8)TRUE) || (IsAuthenticatedTmp == (u8)TRUE))) {
		Status = XLoader_SpkMeasurement(SecurePtr, &Sha3Hash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XLoader_ExtendSpkHash(&Sha3Hash, PcrNo, MeasureIdx, OverWrite);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		MeasureIdx = MeasureIdx + 1U;

		Status = XLoader_SpkIdMeasurement(SecurePtr, &Sha3Hash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XLoader_ExtendSpkId(&Sha3Hash, PcrNo, MeasureIdx, OverWrite);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		MeasureIdx = MeasureIdx + 1;

	}
	if ((IsEncrypted == (u8)TRUE) || (IsEncryptedTmp == (u8)TRUE)) {
		if ((IsAuthenticated != (u8)TRUE) || (IsAuthenticatedTmp != (u8)TRUE)) {
			Status = XLoader_EncRevokeIdMeasurement(SecurePtr, &Sha3Hash);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XLoader_ExtendEncRevokeId(&Sha3Hash, PcrNo, MeasureIdx, OverWrite);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			MeasureIdx = MeasureIdx + 1;
		}
	}

	*DigestIndex = MeasureIdx;

	Status = XST_SUCCESS;
END:
	if (Status != XST_SUCCESS) {
		XPlmi_UpdateStatus(XLOADER_ERR_SECURE_CONFIG_MEASUREMENT, Status);
	}
#else
	(void)SecurePtr;
	(void)PcrInfo;
	(void)DigestIndex;
	(void)OverWrite;
	Status = XST_SUCCESS;
#endif
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function measures the partion hashes.
 *
 * @param	PdiPtr is pointer to XilPdi instance.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_ERR_DATA_MEASUREMENT if error in data measurement.
 *
 **************************************************************************************************/
int XLoader_MeasureNLoad(XilPdi* PdiPtr)
{
	volatile int Status = XST_FAILURE;
#ifdef PLM_OCP
	XLoader_ImageMeasureInfo ImageMeasureInfo = {0U};
	u32 PcrInfo = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].PcrInfo;
	u32 Index;
	XSecure_Sha3Hash *PtrnHashTablePtr;
	u32 ChecksumType;
	ChecksumType = XilPdi_GetChecksumType(&PdiPtr->MetaHdr->PrtnHdr[PdiPtr->PrtnNum]);
	PtrnHashTablePtr = XLoader_GetPtrnHashTable();
	PdiPtr->DigestIndex = (PcrInfo & XLOADER_PCR_MEASUREMENT_INDEX_MASK) >>
			       XLOADER_PCR_MEASUREMENT_INDEX_SHIFT;

	if (XilPdi_IsAuthEnabled(&PdiPtr->MetaHdr->ImgHdrTbl) ||
	    (ChecksumType == XIH_PH_ATTRB_HASH_SHA3)) {
		Status = XLoader_LoadImagePrtns(PdiPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		ImageMeasureInfo.PcrInfo = PcrInfo;
		ImageMeasureInfo.Flags = XLOADER_MEASURE_START;
		ImageMeasureInfo.SubsystemID = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].ImgID;
		Status = XLoader_DataMeasurement(&ImageMeasureInfo);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		for(Index = 0U; Index < PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].NoOfPrtns; Index++)
		{
			ImageMeasureInfo.DataSize = XLOADER_SHA3_LEN;
			ImageMeasureInfo.PcrInfo = PcrInfo;
			ImageMeasureInfo.SubsystemID = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].ImgID;
			ImageMeasureInfo.Flags = XLOADER_MEASURE_UPDATE;
			ImageMeasureInfo.DataAddr = (u64)(UINTPTR)PtrnHashTablePtr[Index].Hash;

			/* Update the data for measurement */
			XPlmi_Printf(DEBUG_INFO, "Partition Measurement started\r\n");
			Status = XLoader_DataMeasurement(&ImageMeasureInfo);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		ImageMeasureInfo.Flags = XLOADER_MEASURE_FINISH;
		Status = XLoader_DataMeasurement(&ImageMeasureInfo);
	} else {
		ImageMeasureInfo.PcrInfo = PcrInfo;
		ImageMeasureInfo.Flags = XLOADER_MEASURE_START;
		ImageMeasureInfo.SubsystemID = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].ImgID;
		Status = XLoader_DataMeasurement(&ImageMeasureInfo);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XLoader_LoadImagePrtns(PdiPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		ImageMeasureInfo.Flags = XLOADER_MEASURE_FINISH;
		Status = XLoader_DataMeasurement(&ImageMeasureInfo);
	}

END:
#else
	Status = XLoader_LoadImagePrtns(PdiPtr);
#endif
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function measures the data by calculating SHA3 hash.
 *
 * @param	ImageInfo Pointer to the XLoader_ImageMeasureInfo structure.
 *
 * @return
 * 		- XST_SUCCESS on successfully measuring the data.
 * 		- XLOADER_ERR_DATA_MEASUREMENT if error in data measurement.
 *
 **************************************************************************************************/
int XLoader_DataMeasurement(XLoader_ImageMeasureInfo *ImageInfo)
{
#ifdef PLM_OCP
	volatile int Status = (int)XLOADER_ERR_DATA_MEASUREMENT;
	u32 PcrNo;
	XSecure_Sha3Hash Sha3Hash;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
#ifndef PLM_SECURE_EXCLUDE
	XilPdi* PdiPtr = XLoader_GetPdiInstance();
#endif
#ifdef PLM_OCP_KEY_MNGMT
	u32 DevAkIndex[XOCP_MAX_KEYS_SUPPPORTED_PER_SUBSYSTEM] = {0U};

	Status = XOcp_GetSubSysDevAkIndex(ImageInfo->SubsystemID, DevAkIndex);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((ImageInfo->PcrInfo == XOCP_PCR_INVALID_VALUE) &&
		((DevAkIndex[XOCP_DEFAULT_DEVAK_KEY_INDEX] == XLOADER_INVALID_DEVAK_INDEX) &&
		(DevAkIndex[XOCP_KEYWRAP_DEVAK_KEY_INDEX] == XLOADER_INVALID_DEVAK_INDEX))) {
		Status = XST_SUCCESS;
		goto END;
	}
#endif

#ifndef PLM_SECURE_EXCLUDE
	Status = XLoader_RunSha3EngineKat(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif /* PLM_SECURE_EXCLUDE */

	switch(ImageInfo->Flags) {
	case XLOADER_MEASURE_START:
		Status = XSecure_ShaStart(Sha3InstPtr, XSECURE_SHA3_384);
		break;
	case XLOADER_MEASURE_UPDATE:
		Status = XSecure_ShaUpdate(Sha3InstPtr,
				ImageInfo->DataAddr, ImageInfo->DataSize);
		break;
	case XLOADER_MEASURE_FINISH:
		Status = XSecure_ShaFinish(Sha3InstPtr, (UINTPTR)&Sha3Hash, XLOADER_SHA3_LEN);
		break;
	default:
		XPlmi_Printf(DEBUG_INFO, "Please check provided case\r\n");
		break;
	}
	if (Status != XST_SUCCESS) {
		XPlmi_UpdateStatus(XLOADER_ERR_DATA_MEASUREMENT, Status);
		goto END;
	}

	if (ImageInfo->Flags == XLOADER_MEASURE_FINISH) {
#ifdef PLM_OCP_KEY_MNGMT
		if ((DevAkIndex[XOCP_DEFAULT_DEVAK_KEY_INDEX] != XLOADER_INVALID_DEVAK_INDEX) ||
			(DevAkIndex[XOCP_KEYWRAP_DEVAK_KEY_INDEX] != XLOADER_INVALID_DEVAK_INDEX)) {
			/* Generate DEVAK */
			Status = XOcp_GenSubSysDevAk(ImageInfo->SubsystemID,
						(u64)(UINTPTR)Sha3Hash.Hash);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
#endif /* PLM_OCP_KEY_MNGMT */
		if (ImageInfo->PcrInfo != XOCP_PCR_INVALID_VALUE) {
			PcrNo = ImageInfo->PcrInfo & XOCP_PCR_NUMBER_MASK;

			/* Extend SW PCR */
			Status = XOcp_ExtendSwPcr(PcrNo, *(u32 *)(ImageInfo->DigestIndex),
				(u64)(UINTPTR)Sha3Hash.Hash, XLOADER_SHA3_LEN,
				ImageInfo->OverWrite);
		}
	}

END:
	return Status;
#else
	(void)ImageInfo;
	XPlmi_Printf(DEBUG_INFO, "OCP Module is not initialized\r\n");
	return XST_SUCCESS;
#endif
}

/**************************************************************************************************/
/**
 * @brief	This function sets the handoff parameters to the ARM Trusted Firmware(ATF).
 * 		Some of the inputs for this are taken from image partition header. A pointer to
 * 		the structure containing these parameters is stored in the
 * 		PMC_GLOBAL.GLOBAL_GEN_STORAGE4 register, which ATF reads.
 *
 * @param
 * 		- PrtnHdr is pointer to Partition header details.
 *
 **************************************************************************************************/
void XLoader_SetATFHandoffParameters(const XilPdi_PrtnHdr *PrtnHdr)
{
	u32 PrtnAttrbs;
	u32 PrtnFlags;
	u32 LoopCount = 0U;
	XilPdi_ATFHandoffParams *ATFHandoffParams = XLoader_GetATFHandoffParamsAddr();

	PrtnAttrbs = PrtnHdr->PrtnAttrb;

	/**
	 * - Read partition header and deduce entry point and partition flags.
	 */
	PrtnFlags =
		(((PrtnAttrbs & XIH_PH_ATTRB_A72_EXEC_ST_MASK) >> XIH_ATTRB_A72_EXEC_ST_SHIFT_DIFF)
		 | ((PrtnAttrbs & XIH_PH_ATTRB_ENDIAN_MASK) >> XIH_ATTRB_ENDIAN_SHIFT_DIFF)
		 | ((PrtnAttrbs & XIH_PH_ATTRB_TZ_SECURE_MASK) << XIH_ATTRB_TR_SECURE_SHIFT_DIFF)
		 | ((PrtnAttrbs & XIH_PH_ATTRB_TARGET_EL_MASK) << XIH_ATTRB_TARGET_EL_SHIFT_DIFF));
	PrtnAttrbs &= XIH_PH_ATTRB_DSTN_CPU_MASK;
	/** - Update CPU number based on destination CPU */
	if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_A72_0) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A72_0;
	} else if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_A72_1) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A72_1;
	} else if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_NONE) {
		/*
		 * This is required for u-boot handoff to work when BOOTGEN_SUBSYSTEM_PDI is
		 * set to 0 in bootgen
		 */
		PrtnFlags &= (~(XIH_ATTRB_EL_MASK) | XIH_PRTN_FLAGS_EL_2)
					| XIH_PRTN_FLAGS_DSTN_CPU_A72_0;
	} else {
		/* MISRA-C compliance */
	}

	if (ATFHandoffParams->NumEntries == 0U) {
		/* Insert magic string */
		ATFHandoffParams->MagicValue[0U] = 'X';
		ATFHandoffParams->MagicValue[1U] = 'L';
		ATFHandoffParams->MagicValue[2U] = 'N';
		ATFHandoffParams->MagicValue[3U] = 'X';
	} else {
		for (; LoopCount < ATFHandoffParams->NumEntries; LoopCount++) {
			if (ATFHandoffParams->Entry[LoopCount].PrtnFlags == PrtnFlags) {
				break;
			}
		}
	}

	if ((ATFHandoffParams->NumEntries < XILPDI_MAX_ENTRIES_FOR_ATF) &&
	    (ATFHandoffParams->NumEntries == LoopCount)) {
		if((PrtnFlags & XIH_ATTRB_EL_MASK) != XIH_PRTN_FLAGS_EL_3) {
			ATFHandoffParams->NumEntries++;
			ATFHandoffParams->Entry[LoopCount].EntryPoint = PrtnHdr->DstnExecutionAddr;
			ATFHandoffParams->Entry[LoopCount].PrtnFlags = PrtnFlags;
		}
	}
}

/**************************************************************************************************/
/**
 * @brief	This function sets Aarch state and vector location for APU.
 *
 * @param	CpuId CPU ID.
 * @param	ExecState CPU execution state.
 * @param	VInitHi resembles highvec configuration for CPU.
 *
 **************************************************************************************************/
static void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi)
{
	u32 RegVal = Xil_In32(XLOADER_FPD_APU_CONFIG_0);
	u32 ExecMask = 0U;
	u32 VInitHiMask = 0U;

	switch(CpuId)
	{
		case XIH_PH_ATTRB_DSTN_CPU_A72_0:
			ExecMask = XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU0;
			VInitHiMask = XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU0;
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A72_1:
			ExecMask = XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU1;
			VInitHiMask = XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU1;
			break;
		default:
			break;
	}
	/** Set Aarch state 64 Vs 32 bit and vector location for 32 bit */
	if (ExecState == XIH_PH_ATTRB_A72_EXEC_ST_AA64) {
		RegVal |= ExecMask;
	} else {
		RegVal &= ~(ExecMask);
		if (VInitHi == XIH_PH_ATTRB_HIVEC_MASK) {
			RegVal |= VInitHiMask;
		} else {
			RegVal &= ~(VInitHiMask);
		}
	}
	/** Update the APU configuration */
	XPlmi_Out32(XLOADER_FPD_APU_CONFIG_0, RegVal);
}

/**************************************************************************************************/
/**
 * @brief	This function is used to get PdiSrc and PdiAddr for Secondary SD boot modes.
 *
 * @param	SecBootMode is the secondary boot mode value.
 * @param	PdiPtr Pointer to PDI instance.
 * @param	PdiSrc Pointer to the source of PDI.
 * @param	PdiAddr is the pointer to the address of the Pdi.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE on unsupported secondary bootmode.
 *
 **************************************************************************************************/
int XLoader_GetSDPdiSrcNAddr(u32 SecBootMode, XilPdi *PdiPtr, u32 *PdiSrc, u64 *PdiAddr)
{
	int Status = XST_FAILURE;
	(void)PdiPtr;

	/** - Get the PDI source address for the secondary boot device. */
	switch(SecBootMode)
	{
		case XIH_IHT_ATTR_SBD_SD_0:
#ifdef XLOADER_SD_0
			*PdiSrc = XLOADER_PDI_SRC_SD0 | XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr->ImgHdrTbl.SBDAddr << XLOADER_SD_ADDR_SHIFT);
#else
			*PdiSrc = XLOADER_PDI_SRC_SD0;
#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SD_1:
#ifdef XLOADER_SD_1
			*PdiSrc = XLOADER_PDI_SRC_SD1 | XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr->ImgHdrTbl.SBDAddr << XLOADER_SD_ADDR_SHIFT);
#else
			*PdiSrc = XLOADER_PDI_SRC_SD1;
#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SD_LS:
#ifdef XLOADER_SD_1
			*PdiSrc = XLOADER_PDI_SRC_SD1_LS | XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr->ImgHdrTbl.SBDAddr << XLOADER_SD_ADDR_SHIFT);
#else
			*PdiSrc = XLOADER_PDI_SRC_SD1_LS;
#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SD_0_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SD0;
			break;
		case XIH_IHT_ATTR_SBD_SD_1_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SD1;
			break;
		case XIH_IHT_ATTR_SBD_SD_LS_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SD1_LS;
			break;
		default:
			Status = (int)XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE;
			break;
	}

	if (Status != (int)XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function copies the elf partitions to specified destinations.
 *
 * @param	PdiPtr is pointer to XilPdi instance.
 * @param	PrtnHdr is pointer to the partition header.
 * @param	PrtnParams is pointer to the structure variable that contains
 * 		parameters required to process the partition.
 * @param	SecureParams is pointer to the instance containing security related params.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_INVALID_ELF_LOAD_ADDR if the load address of the elf is invalid.
 *		- XLOADER_ERR_PM_DEV_PSM_PROC if device requet for PSM is failed.
 *		- XLOADER_ERR_PM_DEV_IOCTL_RPU0_SPLIT if IOCTL call to set RPU0 in split mode fails.
 *		- XLOADER_ERR_PM_DEV_IOCTL_RPU1_SPLIT if IOCTL call to set RPU1 in split mode fails.
 *		- XLOADER_ERR_PM_DEV_IOCTL_RPU0_LOCKSTEP if IOCTL call to set RPU0 in lockstep mode
 *		  fails.
 *		- XLOADER_ERR_PM_DEV_IOCTL_RPU1_LOCKSTEP if IOCTL call to set RPU1 in lockstep mode
 *		  fails.
 *		- XLOADER_ERR_INVALID_TCM_ADDR on Invalid TCM address for A72 elfs.
 *
 **************************************************************************************************/
int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
		XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams)
{
	int Status = XST_FAILURE;
	u32 CapAccess = (u32)PM_CAP_ACCESS;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
	u32 Len = PrtnHdr->UnEncDataWordLen << XPLMI_WORD_LEN_SHIFT;
	u64 EndAddr = PrtnParams->DeviceCopy.DestAddr + Len - 1U;
	u32 ErrorCode;
	u32 Mode = 0U;
	u8 TcmComb;

	/**
	 * - Verify the load address.
	 */
	Status = XPlmi_VerifyAddrRange(PrtnParams->DeviceCopy.DestAddr, EndAddr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_ELF_LOAD_ADDR, Status);
		goto END;
	}
	PrtnParams->DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	/**
	 *
	 * - For PSM, PSM should be taken out of reset before loading.
	 *   PSM RAM should be ECC initialized.
	 *
	 * - For OCM, RAM should be ECC initialized.
	 *
	 * - R5 should be taken out of reset before loading. R5 TCM should be ECC initialized.
	 */
	if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_PSM) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_PSM_PROC, (CapAccess | CapContext),
				XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_PSM_PROC, 0);
			goto END;
		}
		goto END1;
	}

	if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0, IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_SPLIT, 0U, 0U, &Mode, XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_IOCTL_RPU0_SPLIT;
	} else if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1, IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_SPLIT, 0U, 0U, &Mode, XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_IOCTL_RPU1_SPLIT;
	} else if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0, IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_LOCKSTEP, 0U, 0U, &Mode, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_IOCTL_RPU0_LOCKSTEP, 0);
			goto END;
		}
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1, IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_LOCKSTEP, 0U, 0U, &Mode, XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_IOCTL_RPU1_LOCKSTEP;
	} else {
		/* MISRA-C compliance */
	}
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, 0);
		goto END;
	}

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) ||
			(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L)) {
		Status = XLoader_RequestTCM(XLOADER_TCM_0);
	}
	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) ||
			(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L)) {
		Status = XLoader_RequestTCM(XLOADER_TCM_1);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_GetLoadAddr(PrtnParams->DstnCpu, &PrtnParams->DeviceCopy.DestAddr, Len);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	if ((PrtnParams->DstnCpu != XIH_PH_ATTRB_DSTN_CPU_A72_0) &&
			(PrtnParams->DstnCpu != XIH_PH_ATTRB_DSTN_CPU_A72_1)) {
		goto END1;
	}

	EndAddr = PrtnParams->DeviceCopy.DestAddr + Len - 1U;
	if (((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_1_TCM_A_BASE_ADDR)
				&& (EndAddr <= XLOADER_R5_1_TCM_A_END_ADDR)) ||
			((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_1_TCM_B_BASE_ADDR)
			 && (EndAddr <= XLOADER_R5_1_TCM_B_END_ADDR))) {
		/* TCM 1 is in use */
		/* Only allow if TCM is in split mode */
		TcmComb = (u8)((XPlmi_In32(XLOADER_RPU_GLBL_CNTL) & XLOADER_TCMCOMB_MASK) >>
				XLOADER_TCMCOMB_SHIFT);
		if (TcmComb == (u8)FALSE) {
			Status = XLoader_RequestTCM(XLOADER_TCM_1);
		} else {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_TCM_ADDR, 0);
		}
	} else if (((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_0_TCM_A_BASE_ADDR) &&
				(EndAddr <= XLOADER_R5_0_TCM_A_END_ADDR)) ||
			((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_0_TCM_B_BASE_ADDR) &&
			 (EndAddr <= XLOADER_R5_0_TCM_B_END_ADDR))) {
		/* TCM 0 is in use */
		Status = XLoader_RequestTCM(XLOADER_TCM_0);
	} else if ((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_0_TCM_A_BASE_ADDR)
			&& (EndAddr <= XLOADER_R5_LS_TCM_END_ADDR)) {
		/* TCM COMB is in use */
		TcmComb = (u8)((XPlmi_In32(XLOADER_RPU_GLBL_CNTL) &
			XLOADER_TCMCOMB_MASK) >> XLOADER_TCMCOMB_SHIFT);
		if (TcmComb == (u8)TRUE) {
			Status = XLoader_RequestTCM(XLOADER_TCM_0);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XLoader_RequestTCM(XLOADER_TCM_1);
		} else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INVALID_TCM_ADDR, 0);
		}
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

END1:
	/**
	 * - Copy the partition to the load address.
	 */
	Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams->DeviceCopy, SecureParams);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A72_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A72_1)) {

		Status = XLoader_ClearATFHandoffParams(PdiPtr);
		if(Status != XST_SUCCESS){
			goto END;
		}
		/**
		 *  - Populate handoff parameters to ATF.
		 *  These correspond to the partitions of application
		 *  which ATF will be loading.
		 */
		XLoader_SetATFHandoffParameters(PrtnHdr);
	}

	if (PdiPtr->DelayHandoff == (u8)FALSE) {
		/* Update the handoff values */
		Status = XLoader_UpdateHandoffParam(PdiPtr);
	}

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function requests TCM_0_A, TCM_0_B, TCM_1_A and TCM_1_B depending upon
 * 		input param and R5-0 and R5-1 cores as required for TCMs.
 *
 * @param	TcmId denotes TCM_0 or TCM_1.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_PM_DEV_TCM_0_A if device request for TCM_0_A is failed.
 *		- XLOADER_ERR_PM_DEV_TCM_0_B if device request for TCM_0_B is failed.
 *		- XLOADER_ERR_PM_DEV_TCM_1_A if device request for TCM_1_A is failed.
 *		- XLOADER_ERR_PM_DEV_TCM_1_B if device request for TCM_1_B is failed.
 *
 **************************************************************************************************/
static int XLoader_RequestTCM(u32 TcmId)
{
	int Status = XST_FAILURE;
	u32 CapAccess = (u32)PM_CAP_ACCESS;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
	u32 ErrorCode;

	if (TcmId == XLOADER_TCM_0) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_0_A, (CapAccess | CapContext),
				XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			ErrorCode = (u32)XLOADER_ERR_PM_DEV_TCM_0_A;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_0_B, (CapAccess | CapContext),
				XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_TCM_0_B;
	} else if (TcmId == XLOADER_TCM_1) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_1_A, (CapAccess | CapContext),
				XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			ErrorCode = (u32)XLOADER_ERR_PM_DEV_TCM_1_A;
			goto END;
		}

		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_1_B, (CapAccess | CapContext),
				XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_TCM_1_B;
	}

END:
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, 0);
	}
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function is used to check whether cpu has handoff address stroed in the handoff
 * 		structure.
 *
 * @param	PdiPtr is pointer to XilPdi instance.
 * @param	DstnCpu is the cpu which needs to be checked.
 *
 * @return
 *		- XST_SUCCESS if the DstnCpu is successfully added to Handoff list.
 *		- XST_FAILURE if the DstnCpu is already added to Handoff list.
 *
 **************************************************************************************************/
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;

	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings & XIH_PH_ATTRB_DSTN_CPU_MASK;
		if (CpuId == DstnCpu) {
			goto END;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function is used to update the handoff parameters.
 *
 * @param	PdiPtr is pointer to XilPdi instance.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_NUM_HANDOFF_CPUS when number of CPUs exceed max count.
 *
 **************************************************************************************************/
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	u32 DstnCpu = XIH_PH_ATTRB_DSTN_CPU_NONE;
	u32 CpuNo = XLOADER_MAX_HANDOFF_CPUS;
	u32 PrtnNum = PdiPtr->PrtnNum;
	/* Assign the partition header to local variable */
	const XilPdi_PrtnHdr * PrtnHdr = &(PdiPtr->MetaHdr->PrtnHdr[PrtnNum]);

	/**
	 * - Get the destination CPU from the partition header.
	 */
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) && (DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_PSM)) {
		CpuNo = PdiPtr->NoOfHandoffCpus;
		/**
		 * - Validate the destination CPU.
		 */
		if (XLoader_CheckHandoffCpu(PdiPtr, DstnCpu) == XST_SUCCESS) {
			if (CpuNo >= XLOADER_MAX_HANDOFF_CPUS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_NUM_HANDOFF_CPUS, 0);
				goto END;
			}
			/**
			 * - Update the CPU settings.
			 */
			PdiPtr->HandoffParam[CpuNo].CpuSettings = XilPdi_GetDstnCpu(PrtnHdr) |
				XilPdi_GetA72ExecState(PrtnHdr) |
				XilPdi_GetVecLocation(PrtnHdr);
			PdiPtr->HandoffParam[CpuNo].HandoffAddr = PrtnHdr->DstnExecutionAddr;
			PdiPtr->NoOfHandoffCpus += 1U;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function updates the load address based on the destination CPU.
 *
 * @param	DstnCpu is destination CPU.
 * @param	LoadAddrPtr is the destination load address pointer.
 * @param	Len is the length of the partition.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_TCM_ADDR_OUTOF_RANGE if load address is out of range.
 *
 **************************************************************************************************/
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len)
{
	int Status = XST_FAILURE;
	u64 Address = *LoadAddrPtr;
	u32 Offset = 0U;

	/**
	 * - Validate the address is within the range of R5 TCMA or R5 TCMB.
	 */
	if (((Address < (XLOADER_R5_TCMA_LOAD_ADDRESS + XLOADER_R5_TCM_BANK_LENGTH)) ||
				((Address >= XLOADER_R5_TCMB_LOAD_ADDRESS) &&
				 (Address < (XLOADER_R5_TCMB_LOAD_ADDRESS +
					     XLOADER_R5_TCM_BANK_LENGTH))))) {
		if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
			Offset = XLOADER_R5_0_TCM_A_BASE_ADDR;
		} else if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
			Offset = XLOADER_R5_1_TCM_A_BASE_ADDR;
		} else {
			/* MISRA-C compliance */
		}

		if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) ||
				(DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1)) {
			if (((Address % XLOADER_R5_TCM_BANK_LENGTH) + Len) >
					XLOADER_R5_TCM_BANK_LENGTH) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
				goto END;
			}
		}
	}

	/**
	 * - Otherwise validate the address if the destination CPU is lockstep R5 and is within
	 *   the range of it TCM.
	 */
	if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) && (Address < XLOADER_R5_TCM_TOTAL_LENGTH)) {
		if (((Address % XLOADER_R5_TCM_TOTAL_LENGTH) + Len) > XLOADER_R5_TCM_TOTAL_LENGTH) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
			goto END;
		}
		Offset = XLOADER_R5_0_TCM_A_BASE_ADDR;
	}

	/**
	 * - Update the load address.
	 */
	Address += Offset;
	*LoadAddrPtr = Address;
	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function prints DDRMC register details.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Error code on failure.
 *
 **************************************************************************************************/
static int XLoader_DumpDdrmcRegisters(void)
{
	int Status = XST_FAILURE;
	u32 PcsrCtrl;
	u32 DevId;
	u8 Ub = 0U;
	u8 LoopCount;
	u32 BaseAddr;
	XPm_DeviceStatus DevStatus;

	XPlmi_Printf(DEBUG_PRINT_ALWAYS,"====DDRMC Register Dump Start======\n\r");

	Status = XLoader_DdrInit(XLOADER_PDI_SRC_DDR);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Error  0x%0x in requesting DDR.\n\r", Status);
		goto END;
	}

	for (DevId = PM_DEV_DDRMC_0; DevId <= PM_DEV_DDRMC_3; DevId++) {
		DevStatus.Status = (u32)XPM_DEVSTATE_UNUSED;
		/** Get DDRMC UB Base address */
		Status = XPm_GetDeviceStatus(PM_SUBSYS_PMC, DevId, &DevStatus);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (DevStatus.Status != XPM_DEVSTATE_RUNNING) {
			XPlmi_Printf(DEBUG_GENERAL, "DDRMC_%u is not enabled,"
					" Skipping its dump...\n\r", Ub);
			++Ub;
			continue;
		}
		Status = XPm_GetDeviceBaseAddr(DevId, &BaseAddr);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Error 0x%0x in getting DDRMC_%u addr\n",
				Status, Ub);
			goto END;
		}

		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "DDRMC_%u (UB 0x%08x)\n\r", Ub, BaseAddr);

		/** Read PCSR Control */
		PcsrCtrl = XPlmi_In32(BaseAddr + DDRMC_PCSR_CONTROL_OFFSET);

		/** Skip DDRMC dump if PComplete is zero */
		if (0U == (PcsrCtrl & DDRMC_PCSR_CONTROL_PCOMPLETE_MASK)) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PComplete not set\n\r");
			++Ub;
			continue;
		}

		Xloader_DdrmcRegisters DdrmcRegisters[DDRMC_ARRAY_SIZE] = {
			{"PCSR Status", DDRMC_PCSR_STATUS_OFFSET},
			{"PCSR Control", DDRMC_PCSR_CONTROL_OFFSET},
			{"CAL_PTR", DDRMC_OFFSET_CALIB_PTR},
			{"CAL_ERR_SUB_STAGE", DDRMC_OFFSET_CALIB_ERR_SUB_STAGE},
			{"CAL_ERR_RANK", DDRMC_OFFSET_CALIB_ERR_RANK},
			{"CAL_ERR", DDRMC_OFFSET_CALIB_ERR},
			{"CAL_ERROR_DATA_NIBBLE_1", DDRMC_OFFSET_CALIB_ERR_DATA_NIBBLE_1},
			{"CAL_ERROR_DATA_NIBBLE_2", DDRMC_OFFSET_CALIB_ERR_DATA_NIBBLE_2}
		};

		for (LoopCount=0U ; LoopCount<DDRMC_ARRAY_SIZE ; LoopCount++) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,"%s : 0x%x\n\r", DdrmcRegisters[LoopCount].RegStr,
			XPlmi_In32(BaseAddr + DdrmcRegisters[LoopCount].Offset));
		}

		++Ub;
	}
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "====DDRMC Register Dump End======\n\r");

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function checks if MJTAG workaround partition needs to be
 *		skipped.
 *
 * @param	PdiPtr is pointer to PDI instance.
 *
 * @return
 * 		- TRUE if MTAG workaround partition needs to be skipped, else FALSE.
 *
 **************************************************************************************************/
u8 XLoader_SkipMJtagWorkAround(XilPdi *PdiPtr)
{
	u32 RstReason;
	u8 Check = (u8)FALSE;

	if (PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].ImgID == PM_MISC_MJTAG_WA_IMG) {
		RstReason = XPlmi_In32(PMC_GLOBAL_PERS_GEN_STORAGE2);
		/**
		 * Skip MJTAG WA2 partitions if boot mode is JTAG and Reset Reason is not
		 * external POR
		 */
		if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_JTAG) ||
			(((RstReason & PERS_GEN_STORAGE2_ACC_RR_MASK) >> CRP_RESET_REASON_SHIFT) !=
				CRP_RESET_REASON_EXT_POR_MASK)) {
			Check = (u8)TRUE;
		}
	}

	return Check;
}

/**************************************************************************************************/
/**
 * @brief	This function checks if MJTAG workaround is required.
 *
 * @return
 * 		- XLOADER_ERR_DEFERRED_CDO_PROCESS on error while processing CDO but error is
 * 		  deferred till whole CDO processing is completed.
 *
 **************************************************************************************************/
int XLoader_ProcessDeferredError(void)
{
	int Status = XST_FAILURE;

	Status = XLoader_DumpDdrmcRegisters();
	Status = XPlmi_UpdateStatus(XLOADER_ERR_DEFERRED_CDO_PROCESS, Status);

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function check conditions and perform internal POR for VP1802 and VP1502 device
 * 		if required.
 *
 **************************************************************************************************/
void XLoader_PerformInternalPOR(void)
{
	u32 IdCode = XPlmi_In32(PMC_TAP_IDCODE) & PMC_TAP_IDCODE_SIREV_DVCD_MASK;
	u32 CrpResetReason = XPlmi_In32(CRP_RESET_REASON);
	u8 SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);
	u32 DnaBit = XPlmi_In32(EFUSE_CACHE_DNA_1) & EFUSE_CACHE_DNA_1_BIT25_MASK;
	PdiSrc_t BootMode = XLoader_GetBootMode();

	if ((IdCode != PMC_TAP_IDCODE_ES1_VP1802) && (IdCode != PMC_TAP_IDCODE_ES1_VP1502)) {
		/**
		 * - If the device is not an VP1802 Or VP1502, then return without
		 * performing IPOR.
		 */
		goto END;
	}

	if (SlrType != XLOADER_SSIT_MASTER_SLR) {
		/**
		 * - If the device is not an master SLR, then return without
		 * performing IPOR.
		 */
		goto END;
	}

	if ((BootMode == XLOADER_PDI_SRC_JTAG) || (BootMode == XLOADER_PDI_SRC_SMAP)) {
		/**
		 * - If the bootmode is JTAG or SMAP, then return without performing IPOR.
		 */
		goto END;
	}

	if (DnaBit == 0x00U) {
		/**
		 * - Efuse DNA_57 bit should be non-zero for IPOR.
		 */
		goto END;
	}

	/**
	 * - Perform IPOR, if all the pre-conditions are met for VP1502/VP1802 device.
	 */
	if (CrpResetReason == CRP_RESET_REASON_EXT_POR_MASK) {
		usleep(PLM_VP1802_POR_SETTLE_TIME);
		XPlmi_PORHandler();
	}

END:
	return;
}

/**************************************************************************************************/
/**
 * @brief      This Function does the following:
 *             - It resets the SHA instance 1.
 *             - It puts the SHA instance 1 into initialized state.
 *
 **************************************************************************************************/
void XLoader_ShaInstance1Reset(void)
{
	/** Not Applicable for Versal_2vp */
}

#ifdef PLM_OCP
/**************************************************************************************************/
/**
 * @brief	This function initializes the Trng instance.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_TRNG_INIT_FAIL if TRNG initialization fails.
 *
 **************************************************************************************************/
static int XLoader_InitTrngInstance(void)
{
	int Status = XST_FAILURE;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();
	XTrngpsv_Config *CfgPtr = NULL;

	CfgPtr = XTrngpsv_LookupConfig(0);
	if (CfgPtr == NULL) {
		Status = XLOADER_TRNG_INIT_FAIL;
		goto END;
	}

	Status = XTrngpsv_CfgInitialize(TrngInstance, CfgPtr, CfgPtr->BaseAddress);
END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function initializes the loader with platform specific
 * 		initializations.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
int XLoader_PlatInit(void)
{
	int Status = XST_FAILURE;

	Status = XLoader_InitTrngInstance();

	return Status;
}

#if (!defined(PLM_SECURE_EXCLUDE))
/*****************************************************************************/
/**
 * @brief	This Function does the following:
 *		- It clears KAT Status before loading PPDI
 *		- It runs KAT for SHA3 Instance 1 if it is not already run.
 *		- It updates KAT status in PdiPtr and also RTCA.
 *
 * @param	PdiPtr is PDI Instance pointer
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
*****************************************************************************/
static int XLoader_RunSha3EngineKat(XilPdi* PdiPtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *Sha3Instance = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	XLoader_ClearKatOnPPDI(PdiPtr, XPLMI_SECURE_SHA3_KAT_MASK);

	if (XPlmi_IsKatRan(XPLMI_SECURE_SHA3_KAT_MASK) != (u8)TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_Sha3Kat, Sha3Instance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}

		PdiPtr->PlmKatStatus |= XPLMI_SECURE_SHA3_KAT_MASK;

		/* Update KAT status */
		XPlmi_UpdateKatStatus(PdiPtr->PlmKatStatus);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
#endif
#else
/**************************************************************************************************/
/**
 * @brief	This function initializes the loader with platform specific
 * 		initializations.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
int XLoader_PlatInit(void)
{
	return XST_SUCCESS;
}
#endif /* PLM_OCP */
