/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_2ve_2vm/xloader_plat.c
*
* This file contains the versal_2ve_2vm specific code related to PDI image loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       bm   07/13/2022 Retain critical data structures after In-Place PLM Update
*       bm   07/18/2022 Shutdown modules gracefully during update
*       dc   07/20/2022 Added support for data measurement
*       dc   09/04/2022 Initialized TRNG
*       is   09/12/2022 Remove PM_CAP_SECURE capability when requesting PSM_PROC,
*                       TCM memory banks
* 1.01  ng   11/11/2022 Updated doxygen comments
*       dc   12/27/2022 Added SHA1 instance
*       kal  01/05/2023 Added PCR Extend functions for secure images
*       dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.02  ng   04/27/2023 Added support for cluster flags in ATF handoff params
*       sk   05/23/2023 Made Status variable volatile to avoid optimization
*       kal  06/04/2023 Added SW PCR extend support
*       sk   06/12/2023 Removed PDI Inst DS export,Added Bootpdiinfo storage &
*                       DS export to handle in-place update flow,
*                       Removed XLoader_GetPdiInstance function definition
*       bm   06/13/2023 Log PLM error before deferring
*       sk   07/06/2023 Added Jtag DAP config support for Non-Secure Debug
*       kpt  07/10/2023 Added IPI support to read DDR crypto status
*       sk   07/09/2023 Enable TCM Boot based on PH Attribute
*                       Removed XLoader_GetLoadAddr targeting TCM Memory
*       ng   06/26/2023 Added support for system device tree flow
*       dd   08/11/2023 Updated doxygen comments
*       sk   08/11/2023 Added error code for default case in XLoader_StartImage
*       ng   02/14/2024 removed int typecast for errors
* 1.03  am   01/31/2024 Fixed internal security review comments of XilOcp library
*       kpt  01/22/2024 Added support to extend secure state to SWPCR
*       mss  03/06/2024 Removed code which was overwriting partition header
*                       Destination Execution Address
*       sk   03/13/2024 Fixed doxygen comments format
*       mss  04/12/2024 Added code to dump DDRMC error logs
*       har  06/07/2024 Updated condition to check if optional data is not found
*       kal  06/29/2024 Update InPlace update to load LPD and PSM
*       pre  12/09/2024 use PMC RAM for Metaheader instead of PPU1 RAM
*       obs  12/10/2024 Fixed GCC Warnings
*       ma   01/07/2025 Added support for ASU handoff
*       sk   02/04/2024 Reset Status before call to XLoader_PrtnCopy
*       tri  03/01/2025 Added XLOADER_MEASURE_LAST case in XLoader_DataMeasurement
*                       for Versal 2VE and 2VM Devices
*       sk   03/05/2025 Reset Status before use in XLoader_ProcessElf
*       sk   03/05/2025 Added temporal check in XLoader_EncRevokeIdMeasurement
*       sk   03/05/2025 Added ASU destination CPU attribute
*       tri  03/13/2025 Added XLoader_MeasureNLoad support
*       sk   03/17/2025 Added support for all 5 RPU clusters
*       sk   03/22/2025 Updated status variable as volatile in XLoader_StartImage
*       sk   03/29/2025 Added redundancy for handoff address
* 1.04  obs  08/01/2025 Updated status with valid error code in XLoader_DataMeasurement API
*       tvp  07/28/2025 Added comment for better code readability
*       rmv  07/17/2025 Call XOcp_StoreSubsysDigest() to store subsystem digest for ASUFW
*       tvp  08/13/2025 Code refactoring for Platform specific TRNG functions
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_update.h"
#include "xloader.h"
#include "xpm_device.h"
#include "xpm_api.h"
#include "xpm_nodeid.h"
#include "xpm_rpucore.h"
#include "xloader_plat.h"
#include "xplmi_update.h"
#include "xplmi.h"
#include "xilpdi.h"
#include "xplmi_gic_interrupts.h"
#ifdef PLM_OCP
#include "xocp.h"
#ifdef PLM_OCP_ASUFW_KEY_MGMT
#include "xocp_plat.h"
#endif
#include "xsecure_sha.h"
#include "xsecure_init.h"
#endif
#include "xplmi_scheduler.h"
#include "xplmi_plat.h"
#include "xplmi_hw.h"
#include "xtrngpsx.h"
#include "xloader_ddr.h"
#include "xsecure_plat.h"
#include "xpm_rpucore.h"
#include "xpm_apucore.h"
/************************** Constant Definitions *****************************/
#define XLOADER_IMAGE_INFO_VERSION	(1U) /**< Image version information */
#define XLOADER_IMAGE_INFO_LCVERSION	(1U) /**< Image lowest compatible version information */
#define XLOADER_PDI_LIST_VERSION 	(1U) /**< PDI version list */
#define XLOADER_PDI_LIST_LCVERSION 	(1U) /**< PDI lowest compatible version list */
#define XLOADER_ATF_HANDOFF_PARAMS_VERSION 	(1U) /**< ATF handoff parameters version */
#define XLOADER_ATF_HANDOFF_PARAMS_LCVERSION 	(1U) /**< ATF handoff parameters lowest compatible version */
#define XLOADER_BOOTPDI_INFO_PARAMS_VERSION 	(2U) /**< BootPDI info version */
#define XLOADER_BOOTPDI_INFO_PARAMS_LCVERSION 	(2U) /**< BootPDI info lowest compatible version */
#define XLOADER_CMD_GET_DDR_DEVICE_ID (0U) /**< DDR device id */
#define XLOADER_DDR_CRYPTO_MAIN_OFFSET   (0X40000U) /**< DDR crypto block offset from ub */
#define XLOADER_DDR_PERF_MON_CNT0_OFFSET (0X868U)   /**< Counter 0 offset */
#define XLOADER_DDR_PERF_MON_CNT1_OFFSET (0X86CU)   /**< Counter 1 offset */
#define XLOADER_DDR_PERF_MON_CNT2_OFFSET (0X870U)   /**< Counter 2 offset */
#define XLOADER_DDR_PERF_MON_CNT3_OFFSET (0X874U)   /**< Counter 3 offset */
#ifndef PLM_SECURE_EXCLUDE
#define XLOADER_CMD_CONFIG_JTAG_STATE_FLAG_INDEX	(0U)
        /**< Index in the Payload of ConfigureJtagState command where JTAG state flag is present */
#define XLOADER_CMD_CONFIG_JTAG_STATE_FLAG_MASK		(0x03U) /**< Mask for JTAG state flag */
#define XLOADER_CONFIG_JTAG_STATE_FLAG_ENABLE		(0x03U) /**< Value of JTAG state flag if enabled */
#define XLOADER_CONFIG_JTAG_STATE_FLAG_DISABLE		(0x00U) /**< Value of JTAG state flag if disabled */
#endif
#define XLOADER_TRNG_DEVICE_ID				(0U)
#define XLOADER_PCR_MEASUREMENT_INDEX_MASK 		(0xFFFF0000U)  /**< Mask for PCR Measurement index */
#define XLOADER_PCR_MEASUREMENT_INDEX_SHIFT		(16U)		/**< Shift for PCR measurement index */
#define XLOADER_EFUSE_ROM_RSVD_CACHE_ADDRESS		(0xF1250090U)	/**< ROM Reserved eFuse cache offset */
#define XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK	(0x00000200U)	/**< AUTH_KEYS_TO_HASH eFuse bit mask */
#define XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_SHIFT	(9U)		/**< AUTH_KEYS_TO_HASH eFuse bit shift */
/**
 * @{
 * @cond DDR calibration errors
 */
#define DDRMC_OFFSET_CAL_POINTER				(0x38280U)
#define DDRMC_OFFSET_CAL_ERROR_SUB_STAGE		(0x38284U)
#define DDRMC_OFFSET_CAL_ERROR_RANK				(0x38288U)
#define DDRMC_OFFSET_CAL_ERROR					(0x3828CU)
#define DDRMC_OFFSET_CAL_ERROR_DATA_OCTAD_8_0	(0x38290U)
#define DDRMC_OFFSET_CAL_ERROR_DATA_OCTAD_11_9	(0x38294U)
#define DDRMC_OFFSET_CAL_ERROR_PHY_OCTAD_8_0	(0x38298U)
#define DDRMC_OFFSET_CAL_ERROR_PHY_OCTAD_11_9	(0x3829CU)
#define DDRMC_OFFSET_CAL_ERROR_DATA_LOC_1		(0x382A0U)
#define DDRMC_OFFSET_CAL_ERROR_PHY_LOC_1		(0x382C8U)
#define DDRMC_ARRAY_SIZE						(10U)
/**
 * @}
 * @endcond
 */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu,
	const u32 DstnCluster);
static int XLoader_InitTrngInstance(void);
#if (!defined(PLM_SECURE_EXCLUDE)) && (defined(PLM_OCP))
static int XLoader_SpkMeasurement(XLoader_SecureParams* SecureParams,
	XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendSpkHash(XSecure_Sha3Hash* SpkHash , u32 PcrNo, u32 DigestIndex, u32 OverWrite);
static int XLoader_SpkIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendSpkId(XSecure_Sha3Hash* SpkIdHash, u32 PcrInfo, u32 DigestIndex, u32 OverWrite);
static int XLoader_EncRevokeIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendEncRevokeId(XSecure_Sha3Hash* RevokeIdHash, u32 PcrInfo, u32 DigestIndex, u32 OverWrite);
#endif

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function provides ImageInfoTbl pointer
 *
 * @return	Pointer to ImageInfoTbl
 *
 *****************************************************************************/
XLoader_ImageInfoTbl *XLoader_GetImageInfoTbl(void)
{
	/* Image Info Table */
	static XLoader_ImageInfoTbl ImageInfoTbl __attribute__ ((aligned(4U))) = {
		.Count = 0U,
		.IsBufferFull = FALSE,
	};

	EXPORT_LOADER_DS(ImageInfoTbl, XLOADER_IMAGE_INFO_DS_ID,
		XLOADER_IMAGE_INFO_VERSION, XLOADER_IMAGE_INFO_LCVERSION,
		sizeof(ImageInfoTbl), (u32)(UINTPTR)&ImageInfoTbl);

	EXPORT_LOADER_DS(ImageInfoAddr, XLOADER_IMAGE_INFO_PTR_DS_ID,
		XLOADER_IMAGE_INFO_VERSION, XLOADER_IMAGE_INFO_LCVERSION,
		XPLMI_IMAGE_INFO_TBL_BUFFER_LEN , (u32)(UINTPTR)((XLoader_ImageInfo *) XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR) );
	return &ImageInfoTbl;
}

/*****************************************************************************/
/**
 * @brief	This function provides pointer to PdiList
 *
 * @return	pointer to PdiList
 *
 *****************************************************************************/
XLoader_ImageStore* XLoader_GetPdiList(void)
{
	static XLoader_ImageStore PdiList __attribute__ ((aligned(4U))) = {0};

	EXPORT_LOADER_DS(PdiList, XLOADER_PDI_LIST_DS_ID,
		XLOADER_PDI_LIST_VERSION, XLOADER_PDI_LIST_LCVERSION,
		sizeof(PdiList), (u32)(UINTPTR)&PdiList);

	return &PdiList;
}

/****************************************************************************/
/**
* @brief	This function returns the ATFHandoffParams structure address to
*           the caller.
*
* @return	Returns ATFHandoffParams structure address
*
*****************************************************************************/
XilPdi_ATFHandoffParams *XLoader_GetATFHandoffParamsAddr(void)
{

	/* Instance containing ATF handoff params */
	static XilPdi_ATFHandoffParams ATFHandoffParams
		__attribute__ ((aligned(4U))) = {0};

	EXPORT_LOADER_DS(ATFHandoffParams, XLOADER_ATF_HANDOFF_PARAMS_DS_ID,
		XLOADER_ATF_HANDOFF_PARAMS_VERSION,
		XLOADER_ATF_HANDOFF_PARAMS_LCVERSION,
		sizeof(ATFHandoffParams), (u32)(UINTPTR)&ATFHandoffParams);

	/* Return ATF Handoff parameters structure address */
	return &ATFHandoffParams;
}

/*****************************************************************************/
/**
 * @brief	This function provides pointer to BootPDI Info
 *
 * @return	pointer to BootPDI Info
 *
 *****************************************************************************/
XilBootPdiInfo* XLoader_GetBootPdiInfo(void)
{
	static XilBootPdiInfo BootPdiInfo
		__attribute__ ((aligned(4U))) = {0}; /** < BootPDI info Storage */

	EXPORT_LOADER_DS(BootPdiInfo, XLOADER_BOOTPDI_INFO_DS_ID,
		XLOADER_BOOTPDI_INFO_PARAMS_VERSION, XLOADER_BOOTPDI_INFO_PARAMS_LCVERSION,
		sizeof(BootPdiInfo), (u32)(UINTPTR)&BootPdiInfo);

	return &BootPdiInfo;
}

/*****************************************************************************/
/**
 * @brief	This function is used to start the subsystems in the PDI.
 *
 * @param	PdiPtr Pdi instance pointer
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_WAKEUP_R52_0 if waking up the R52_0 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_R52_1 if waking up the R52_1 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A78_0 if waking up the A78_0 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A78_1 if waking up the A78_1 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A78_2 if waking up the A78_2 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A78_3 if waking up the A78_3 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_ASU if waking up the ASU failed during handoff.
 *
 *****************************************************************************/
int XLoader_StartImage(XilPdi *PdiPtr)
{
	volatile int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;
	u32 DeviceId;
	u32 ClusterId;
	volatile u64 HandoffAddr;
	u8 SetAddress = 1U;
	u32 ErrorCode;
	u32 RequestWakeup = FALSE;

	/** - Start Handoff to the cpus */
	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;
		ClusterId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CLUSTER_MASK;
		ClusterId >>= XIH_PH_ATTRB_DSTN_CLUSTER_SHIFT;
		HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;
		/* Redundant assignment to handle glitch attacks */
		HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;
		Status = XST_FAILURE;
		RequestWakeup = FALSE;

		/** - Wake up each processor */
		switch (CpuId)
		{
			case XIH_PH_ATTRB_DSTN_CPU_R52_0:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_4) {
					Status = XLOADER_ERR_WAKEUP_R52_0;
				} else {
					RequestWakeup = TRUE;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_R52_0;
				DeviceId = XLOADER_GET_RPU0_DEVICE_ID(ClusterId);
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d"
						" R52_0 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_R52_1:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_4) {
					Status = XLOADER_ERR_WAKEUP_R52_1;
				} else {
					RequestWakeup = TRUE;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_R52_1;
				DeviceId = XLOADER_GET_RPU1_DEVICE_ID(ClusterId);
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d"
						" R52_1 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_A78_0:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_3) {
					Status = XLOADER_ERR_WAKEUP_A78_0;
				} else {
					RequestWakeup = TRUE;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_A78_0;
				DeviceId = PM_DEV_ACPU_0_0 + (ClusterId*4);
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d "
						" A78_0 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_A78_1:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_3) {
					Status = XLOADER_ERR_WAKEUP_A78_1;
				} else {
					RequestWakeup = TRUE;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_A78_1;
				DeviceId = PM_DEV_ACPU_0_0 + (ClusterId*4) +
						XLOADER_APU_CORE1;
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d "
						" A78_1 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_A78_2:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_3) {
					Status = XLOADER_ERR_WAKEUP_A78_2;
				} else {
					RequestWakeup = TRUE;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_A78_2;
				DeviceId = PM_DEV_ACPU_0_0 + (ClusterId*4) +
						XLOADER_APU_CORE2;
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d "
						" A78_2 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_A78_3:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_3) {
					Status = XLOADER_ERR_WAKEUP_A78_3;
				} else {
					RequestWakeup = TRUE;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_A78_3;
				DeviceId = PM_DEV_ACPU_0_0 + (ClusterId*4) +
						XLOADER_APU_CORE3;
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d "
						" A78_3 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_ASU:
				RequestWakeup = TRUE;
				ErrorCode = XLOADER_ERR_WAKEUP_ASU;
				DeviceId = PM_DEV_ASU;
				XLoader_Printf(DEBUG_INFO, "Request ASU wakeup\r\n");
				break;

			default:
				Status = XLOADER_ERR_INVALID_CPUID;
				break;
		}
		if (RequestWakeup == TRUE) {
			Status = XPm_PmcWakeUpCore(DeviceId, SetAddress, HandoffAddr);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, Status);
				goto END;
			}
		}
		else {
			Status = XPlmi_UpdateStatus(Status, 0U);
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	/**
	 * - Make Number of handoff CPUs to zero.
	 */
	PdiPtr->NoOfHandoffCpus = 0x0U;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the partion hashes from the hash block
 *
 * @param	PdiPtr is pointer to XilPdi instance
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_DATA_MEASUREMENT if error in data measurement.
 *
 *****************************************************************************/
int XLoader_MeasureNLoad(XilPdi* PdiPtr)
{
	volatile int Status = XST_FAILURE;
	XLoader_ImageMeasureInfo ImageMeasureInfo = {0U};
	u32 PrtnNum = PdiPtr->PrtnNum;
	u32 NoOfPrtns = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].NoOfPrtns;
	u32 PcrInfo = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].PcrInfo;
	u32 SubsystemID = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].ImgID;
	XLoader_HashBlock *HBPtr = XLoader_GetHashBlockInstance();
	u32 Index;

	/** Load the image partitions */
	Status = XLoader_LoadImagePrtns(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifdef PLM_OCP_ASUFW_KEY_MGMT
	if ((PcrInfo == XOCP_PCR_INVALID_VALUE) && (XOcp_IsOcpSubsystem(SubsystemID) == FALSE)) {
#else
	if ((PcrInfo == XOCP_PCR_INVALID_VALUE)) {
#endif
		Status = XST_SUCCESS;
		goto END;
	}

	PdiPtr->DigestIndex = (PcrInfo & XLOADER_PCR_MEASUREMENT_INDEX_MASK) >>
						XLOADER_PCR_MEASUREMENT_INDEX_SHIFT;

	ImageMeasureInfo.PcrInfo = PcrInfo;
	ImageMeasureInfo.Flags = XLOADER_MEASURE_START;
	ImageMeasureInfo.SubsystemID = SubsystemID;
	ImageMeasureInfo.DigestIndex = &PdiPtr->DigestIndex;
	if ((PdiPtr->PdiType == XLOADER_PDI_TYPE_PARTIAL) ||
		(PdiPtr->PdiType == XLOADER_PDI_TYPE_IPU)) {
		ImageMeasureInfo.OverWrite = TRUE;
	}
	else {
		ImageMeasureInfo.OverWrite = FALSE;
	}

	/** Start the measurement with the partition hash's present in HashBlock */
	Status = XLoader_DataMeasurement(&ImageMeasureInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	for (Index = 0U; Index < NoOfPrtns; Index++) {
		ImageMeasureInfo.DataAddr = (u64)(UINTPTR)&HBPtr->HashData[PrtnNum].PrtnHash;
		ImageMeasureInfo.DataSize = XLOADER_SHA3_LEN;
		ImageMeasureInfo.PcrInfo = PcrInfo;
		ImageMeasureInfo.SubsystemID = PdiPtr->MetaHdr->ImgHdr[PdiPtr->ImageNum].ImgID;
		/**
		 * For versal_2ve_2vm, sha_pmxc driver requires notification
		 * before the last ShaUpate.
		 * Set the flag accordingly for the last partition.
		 */
		if((NoOfPrtns - Index) > 1U) {
			ImageMeasureInfo.Flags = XLOADER_MEASURE_UPDATE;
		}
		else {
			/** Set Last word of DMA, by setting IsLastUpdate true */
			ImageMeasureInfo.Flags = XLOADER_MEASURE_LAST;
		}
		/** Update the data for measurement */
		XPlmi_Printf(DEBUG_INFO, "Partition Measurement started\r\n");
		Status = XLoader_DataMeasurement(&ImageMeasureInfo);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		PrtnNum++;
	}
	ImageMeasureInfo.Flags = XLOADER_MEASURE_FINISH;
	Status = XLoader_DataMeasurement(&ImageMeasureInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function sets the handoff parameters to the ARM Trusted
* 			Firmware(ATF). Some of the inputs for this are taken from image
* 			partition header. A pointer to the structure containing these
* 			parameters is stored in the PMC_GLOBAL.GLOBAL_GEN_STORAGE4
* 			register, which ATF reads.
*
* @param	PrtnHdr is pointer to Partition header details
*
*
*****************************************************************************/
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
		(((PrtnAttrbs & XIH_PH_ATTRB_A78_EXEC_ST_MASK)
				>> XIH_ATTRB_A78_EXEC_ST_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_ENDIAN_MASK)
				>> XIH_ATTRB_ENDIAN_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_TZ_SECURE_MASK)
				<< XIH_ATTRB_TR_SECURE_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_TARGET_EL_MASK)
				<< XIH_ATTRB_TARGET_EL_SHIFT_DIFF));

	/** - Update cluster number based on destination cluster number. */
	PrtnFlags |= (PrtnAttrbs & XIH_PH_ATTRB_DSTN_CLUSTER_MASK)
							>> XIH_PRTN_FLAGS_DSTN_CLUSTER_SHIFT_DIFF;

	PrtnAttrbs &= XIH_PH_ATTRB_DSTN_CPU_MASK;
	/** - Update CPU number based on destination CPU */
	if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_A78_0) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A78_0;
	}
	else if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_A78_1) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A78_1;
	}
	else if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_A78_2) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A78_2;
	}
	else if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_A78_3) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A78_3;
	}
	else if (PrtnAttrbs == XIH_PH_ATTRB_DSTN_CPU_NONE) {
		/*
		 * This is required for u-boot handoff to work
		 * when BOOTGEN_SUBSYSTEM_PDI is set to 0 in bootgen
		 */
		PrtnFlags &= (~(XIH_ATTRB_EL_MASK) | XIH_PRTN_FLAGS_EL_2)
					| XIH_PRTN_FLAGS_DSTN_CPU_A78_0;
	}
	else {
		/* MISRA-C compliance */
	}

	if (ATFHandoffParams->NumEntries == 0U) {
		/* Insert magic string */
		ATFHandoffParams->MagicValue[0U] = 'X';
		ATFHandoffParams->MagicValue[1U] = 'L';
		ATFHandoffParams->MagicValue[2U] = 'N';
		ATFHandoffParams->MagicValue[3U] = 'X';
	}
	else {
		for (; LoopCount < ATFHandoffParams->NumEntries;
			LoopCount++) {
			if (ATFHandoffParams->Entry[LoopCount].PrtnFlags ==
					PrtnFlags) {
				break;
			}
		}
	}

	if ((ATFHandoffParams->NumEntries < XILPDI_MAX_ENTRIES_FOR_ATF) &&
		(ATFHandoffParams->NumEntries == LoopCount)) {
		if((PrtnFlags & XIH_ATTRB_EL_MASK) != XIH_PRTN_FLAGS_EL_3) {
			ATFHandoffParams->NumEntries++;
			ATFHandoffParams->Entry[LoopCount].EntryPoint =
					PrtnHdr->DstnExecutionAddr;
			ATFHandoffParams->Entry[LoopCount].PrtnFlags = PrtnFlags;
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function is used to get PdiSrc and PdiAddr for Secondary
 *			SD boot modes
 *
 * @param	SecBootMode is the secondary boot mode value
 * @param	PdiPtr Pointer to the PDI instance
 * @param	PdiSrc is pointer to the source of PDI
 * @param	PdiAddr is the pointer to the address of the Pdi
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE on unsupported secondary
 * 			bootmode.
 *
 *****************************************************************************/
int XLoader_GetSDPdiSrcNAddr(u32 SecBootMode, XilPdi *PdiPtr, u32 *PdiSrc,
		u64 *PdiAddr)
{
	int Status = XST_FAILURE;
	(void)PdiPtr;

	/**
	 * - Get the PDI source address for the secondary boot device.
	 */
	switch(SecBootMode)
	{
		case XIH_IHT_ATTR_SBD_SDLS_B0:
		#ifdef XLOADER_SD_0
			*PdiSrc = XLOADER_PDI_SRC_SDLS_B0 |
				XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr->ImgHdrTbl.SBDAddr
				<< XLOADER_SD_ADDR_SHIFT);
		#else
			*PdiSrc = XLOADER_PDI_SRC_SDLS_B0;
		#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SD_B1:
		#ifdef XLOADER_SD_0
			*PdiSrc = XLOADER_PDI_SRC_SD_B1 |
				XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr->ImgHdrTbl.SBDAddr
				<< XLOADER_SD_ADDR_SHIFT);
		#else
			*PdiSrc = XLOADER_PDI_SRC_SD_B1;
		#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SDLS_B1:
		#ifdef XLOADER_SD_0
			*PdiSrc = XLOADER_PDI_SRC_SDLS_B1 |
				XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr->ImgHdrTbl.SBDAddr
				<< XLOADER_SD_ADDR_SHIFT);
		#else
			*PdiSrc = XLOADER_PDI_SRC_SDLS_B1;
		#endif
			*PdiAddr = 0U;
			break;
		case XIH_IHT_ATTR_SBD_SDLS_B0_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SDLS_B0;
			break;
		case XIH_IHT_ATTR_SBD_SD_B1_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SD_B1;
			break;
		case XIH_IHT_ATTR_SBD_SDLS_B1_RAW:
			*PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_SDLS_B1;
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

/*****************************************************************************/
/**
 * @brief	This function copies the elf partitions to specified destinations.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	PrtnHdr is pointer to the partition header
 * @param	PrtnParams is pointer to the structure variable that contains
 *			parameters required to process the partition
 * @param	SecureParams is pointer to the instance containing security related
 *			params
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_INVALID_ELF_LOAD_ADDR if load address of the elf is
 * 			invalid.
 * 			- XLOADER_ERR_INVALID_R52_CLUSTER if invalid R52 cluster is
 * 			selected.
 *
 *****************************************************************************/
int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams)
{
	volatile int Status = XST_FAILURE;
	u32 Len = PrtnHdr->UnEncDataWordLen << XPLMI_WORD_LEN_SHIFT;
	u64 EndAddr = PrtnParams->DeviceCopy.DestAddr + Len - 1U;
	u32 DstnCluster = 0U;
	u32 ClusterLockstep = 0U;
	u32 DeviceId;
	u32 Mode = 0U;
	u32 TcmBootFlag = FALSE;

		/** Check if TCM Boot Bit is set */
	if ((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_TCM_BOOT_MASK) == XIH_PH_ATTRB_TCM_BOOT_ENABLED) {
		XPlmi_Printf(DEBUG_DETAILED, "TCM Boot Enabled\n");
		TcmBootFlag = TRUE;
	}

	PrtnParams->DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);
	/**
	 * - Verify the load address.
	 */
	Status = XPlmi_VerifyAddrRange(PrtnParams->DeviceCopy.DestAddr, EndAddr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_ELF_LOAD_ADDR,
				Status);
		goto END;
	}

	DstnCluster = XilPdi_GetDstnCluster(PrtnHdr) >>
			XIH_PH_ATTRB_DSTN_CLUSTER_SHIFT;
	ClusterLockstep = XilPdi_GetClusterLockstep(PrtnHdr);

	/**
	 * - For OCM, RAM should be ECC initialized
	 *
	 * - R5 should be taken out of reset before loading.
	 * R5 TCM should be ECC initialized
	 */
	if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_ASU) {
		Status = XPm_PmcRequestDevice(PM_DEV_ASU);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_ASU_PROC, 0);
			goto END;
		}
	}

	if ((XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED == ClusterLockstep) &&
		((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_1))){
		Mode = XPM_RPU_MODE_SPLIT;
	}else if ((XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED != ClusterLockstep) &&
		((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_1))){
		Mode = XPM_RPU_MODE_LOCKSTEP;
	}else if (XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED == ClusterLockstep){
		Mode = XPM_APU_MODE_SPLIT;
	}else {
		Mode = XPM_APU_MODE_LOCKSTEP;
	}

	switch (PrtnParams->DstnCpu)
	{
		case XIH_PH_ATTRB_DSTN_CPU_R52_0:
			if (DstnCluster > XIH_ATTRB_DSTN_CLUSTER_4) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_R52_CLUSTER, 0U);
				goto END;
			}
			DeviceId = XLOADER_GET_RPU0_DEVICE_ID(DstnCluster);
			Status = XPm_RpuSetOperMode(DeviceId, Mode);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			XPmRpuCore_SetTcmBoot(DeviceId, (u8)TcmBootFlag);
			break;
		case XIH_PH_ATTRB_DSTN_CPU_R52_1:
			if (DstnCluster > XIH_ATTRB_DSTN_CLUSTER_4) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_R52_CLUSTER, 0U);
				goto END;
			}
			DeviceId = XLOADER_GET_RPU1_DEVICE_ID(DstnCluster);
			Status = XPm_RpuSetOperMode(DeviceId, Mode);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			XPmRpuCore_SetTcmBoot(DeviceId, (u8)TcmBootFlag);
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_0:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster * 4);
			Status = XPm_ApuSetOperMode(DeviceId, Mode);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_1:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE1;
			Status = XPm_ApuSetOperMode(DeviceId, Mode);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_2:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE2;
			Status = XPm_ApuSetOperMode(DeviceId, Mode);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_3:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE3;
			Status = XPm_ApuSetOperMode(DeviceId, Mode);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		default:
			Status=XST_SUCCESS;
			break;
	}

	if (XST_SUCCESS != Status) {
		goto END;
	}

	/**
	 * - Copy the partition to the load address.
	 */
	Status = XST_FAILURE;
	Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams->DeviceCopy, SecureParams);
	if (XST_SUCCESS != Status) {
			goto END;
	}

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_1) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_2) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_3)
		) {

		Status = XST_FAILURE;
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
		Status = XST_FAILURE;
		Status = XLoader_UpdateHandoffParam(PdiPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	 This function reads DDR crypto performance counters of given DDR device id
 *
 * @param	 Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success.
 *          - ErrorCode on Failure.
 *
 *****************************************************************************/
int XLoader_ReadDdrCryptoPerfCounters(XPlmi_Cmd *Cmd)
{
	int Status  = XST_FAILURE;
	u32 BaseAddr = 0U;
	u32 PcsrCtrl = 0U;
	u32 DevId = Cmd->Payload[XLOADER_CMD_GET_DDR_DEVICE_ID];

	/* Validate the given device id */
	if ((NODESUBCLASS(DevId) != (u32)XPM_NODESUBCL_DEV_MEM_CTRLR) ||
		(NODETYPE(DevId) != XPM_NODETYPE_DEV_DDR)) {
		Status = XLOADER_ERR_DDR_DEVICE_ID;
		goto END;
	}

	/** Get DDRMC UB Base address */
	Status = XPm_GetDeviceBaseAddr(DevId, &BaseAddr);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	/** Read PCSR control status */
	PcsrCtrl = XPlmi_In32(BaseAddr + DDRMC_PCSR_CONTROL_OFFSET);
	if (0U == (PcsrCtrl & DDRMC_PCSR_CONTROL_PCOMPLETE_MASK)) {
		Status = XLOADER_ERR_PCOMPLETE_NOT_DONE;
		goto END;
	}

	/** Read DDR crypto counters */
	Cmd->Response[1U] = XPlmi_In32(BaseAddr + XLOADER_DDR_CRYPTO_MAIN_OFFSET + XLOADER_DDR_PERF_MON_CNT0_OFFSET);
	Cmd->Response[2U] = XPlmi_In32(BaseAddr + XLOADER_DDR_CRYPTO_MAIN_OFFSET + XLOADER_DDR_PERF_MON_CNT1_OFFSET);
	Cmd->Response[3U] = XPlmi_In32(BaseAddr + XLOADER_DDR_CRYPTO_MAIN_OFFSET + XLOADER_DDR_PERF_MON_CNT2_OFFSET);
	Cmd->Response[4U] = XPlmi_In32(BaseAddr + XLOADER_DDR_CRYPTO_MAIN_OFFSET + XLOADER_DDR_PERF_MON_CNT3_OFFSET);

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function is used to check whether cpu has handoff address
 * 			stored in the handoff structure.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	DstnCpu is the cpu which needs to be checked
 * @param	DstnCluster is the cluster which needs to be checked
 *
 * @return
 * 			- XST_SUCCESS if the DstnCpu is successfully added to Handoff list.
 * 			- XST_FAILURE if the DstnCpu is already added to Handoff list
 *
 *****************************************************************************/
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu,
		const u32 DstnCluster)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;
	u32 ClusterId;

	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;
		ClusterId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CLUSTER_MASK;
		if ((CpuId == DstnCpu) && (ClusterId == DstnCluster)) {
			goto END;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function is used to update the handoff parameters.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_NUM_HANDOFF_CPUS if number of CPUs exceeds max count.
 *
 *****************************************************************************/
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	u32 DstnCpu = XIH_PH_ATTRB_DSTN_CPU_NONE;
	u32 DstnCluster = XIH_PH_ATTRB_DSTN_CLUSTER_0;
	u32 CpuNo = XLOADER_MAX_HANDOFF_CPUS;
	u32 PrtnNum = PdiPtr->PrtnNum;
	/** - Assign the partition header to local variable */
	const XilPdi_PrtnHdr * PrtnHdr =
			&(PdiPtr->MetaHdr->PrtnHdr[PrtnNum]);

	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);
	DstnCluster = XilPdi_GetDstnCluster(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) &&
	    (DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_ASU) &&
	    (DstnCluster <= XIH_PH_ATTRB_DSTN_CLUSTER_4)) {
		CpuNo = PdiPtr->NoOfHandoffCpus;
		if (XLoader_CheckHandoffCpu(PdiPtr, DstnCpu, DstnCluster) ==
			XST_SUCCESS) {
			if (CpuNo >= XLOADER_MAX_HANDOFF_CPUS) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_NUM_HANDOFF_CPUS, 0);
				goto END;
			}
			/** - Update the CPU settings */
			PdiPtr->HandoffParam[CpuNo].CpuSettings =
				XilPdi_GetDstnCpu(PrtnHdr) |
				XilPdi_GetA78ExecState(PrtnHdr) |
				XilPdi_GetDstnCluster(PrtnHdr) |
				XilPdi_GetClusterLockstep(PrtnHdr) |
				XilPdi_GetVecLocation(PrtnHdr);
			PdiPtr->HandoffParam[CpuNo].HandoffAddr =
				PrtnHdr->DstnExecutionAddr;
			PdiPtr->NoOfHandoffCpus += 1U;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function prints DDRMC register details.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_DumpDdrmcRegisters(void)
{
	int Status = XST_FAILURE;
	u32 PcsrCtrl;
	u32 DevId;
	u8 Ub = 0U;
	u8 LoopCount;
	u32 BaseAddr;
	u32 DevState;

	XPlmi_Printf(DEBUG_PRINT_ALWAYS,"====DDRMC Register Dump Start======\n\r");

	Status = XLoader_DdrInit(XLOADER_PDI_SRC_DDR);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Error  0x%0x in requesting DDR.\n\r", Status);
		goto END;
	}

	for (DevId = PM_DEV_DDRMC_0; DevId <= PM_DEV_DDRMC_7; DevId++) {

		if(DevId > PM_DEV_DDRMC_3 && DevId < PM_DEV_DDRMC_4){
			continue;
		}

		DevState = (u32)XPM_DEVSTATE_UNUSED;
		/** Get DDRMC UB Base address */
		Status = XPm_PmcGetDeviceState(DevId, &DevState);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (DevState != XPM_DEVSTATE_RUNNING) {
			XPlmi_Printf(DEBUG_GENERAL, "DDRMC_%u is not enabled,"
					" Skipping its dump...\n\r", Ub);
			++Ub;
			continue;
		}
		Status = XPm_GetDeviceBaseAddr(DevId, &BaseAddr);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Error 0x%0x in getting DDRMC_%u addr\n",
				Status, Ub);
			goto END;
		}

		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"DDRMC_%u (UB 0x%08x)\n\r", Ub, BaseAddr);

		/** Read PCSR Control */
		PcsrCtrl = XPlmi_In32(BaseAddr + DDRMC_PCSR_CONTROL_OFFSET);

		/** Skip DDRMC dump if PComplete is zero */
		if (0U == (PcsrCtrl & DDRMC_PCSR_CONTROL_PCOMPLETE_MASK)) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "PComplete not set\n\r");
			++Ub;
			continue;
		}

		Xloader_DdrmcRegisters DumpRegisters[DDRMC_ARRAY_SIZE] = {
			{"PCSR Status", DDRMC_PCSR_STATUS_OFFSET},
			{"PCSR Control", DDRMC_PCSR_CONTROL_OFFSET},
			{"Calibration Pointer", DDRMC_OFFSET_CAL_POINTER},
			{"Calibration Error Sub Stage", DDRMC_OFFSET_CAL_ERROR_SUB_STAGE},
			{"Calibration Error Rank", DDRMC_OFFSET_CAL_ERROR_RANK},
			{"Calibration Error", DDRMC_OFFSET_CAL_ERROR},
			{"CalibErrOctadData_8_0", DDRMC_OFFSET_CAL_ERROR_DATA_OCTAD_8_0},
			{"CalibErrOctadData_11_9", DDRMC_OFFSET_CAL_ERROR_DATA_OCTAD_11_9},
			{"CalibErrOctadData_8_0", DDRMC_OFFSET_CAL_ERROR_PHY_OCTAD_8_0},
			{"CalibErrOctadData_11_9", DDRMC_OFFSET_CAL_ERROR_PHY_OCTAD_11_9},
		};

		for (LoopCount=0U ; LoopCount<DDRMC_ARRAY_SIZE ; LoopCount++) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,"%s : 0x%x\n\r", DumpRegisters[LoopCount].RegStr,
			XPlmi_In32(BaseAddr + DumpRegisters[LoopCount].Offset));
		}

		for(LoopCount=0U; LoopCount<=9U; LoopCount++){
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,"CalibErrorDataLoc%d: 0x%0x\n\r",LoopCount+1U,
			XPlmi_In32(BaseAddr + DDRMC_OFFSET_CAL_ERROR_DATA_LOC_1 + (LoopCount * 0x4U)));
		}
		for(LoopCount=0U; LoopCount<=11U; LoopCount++){
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,"CalibErrorPhyLoc%d: 0x%0x\n\r",LoopCount+1U,
			XPlmi_In32(BaseAddr + DDRMC_OFFSET_CAL_ERROR_PHY_LOC_1 + (LoopCount * 0x4U)));
		}
	++Ub;
	}

	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "====DDRMC Register Dump End======\n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks if MJTAG workaround is required
 *
 * @return
 * 			- XLOADER_ERR_DEFERRED_CDO_PROCESS on error while processing CDO but
 * error is deferred till whole CDO processing is completed.
 *
 *****************************************************************************/
int XLoader_ProcessDeferredError(void)
{
	int Status = XST_FAILURE;

	Status = XLoader_DumpDdrmcRegisters();
	Status = XPlmi_UpdateStatus(XLOADER_ERR_DEFERRED_CDO_PROCESS, Status);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides update handler for xilloader
 *
 * @param	Op is the module operation variable
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_UpdateHandler(XPlmi_ModuleOp Op)
{
	volatile int Status = XST_FAILURE;
	static u8 LoaderHandlerState = XPLMI_MODULE_NORMAL_STATE;
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_AUTH_JTAG_EXCLUDE
	static u8 AuthJtagTaskRemoved = (u8)FALSE;
#endif
	static u8 DeviceStateTaskRemoved = (u8)FALSE;
#endif

	if (Op.Mode == XPLMI_MODULE_SHUTDOWN_INITIATE) {
		if (LoaderHandlerState == XPLMI_MODULE_NORMAL_STATE) {
			LoaderHandlerState = XPLMI_MODULE_SHUTDOWN_INITIATED_STATE;

			/** - Remove Scheduler tasks if they already exist. */
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_AUTH_JTAG_EXCLUDE
			Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_LOADER_ID,
				XLoader_CheckAuthJtagIntStatus,
				XLOADER_AUTH_JTAG_INT_STATUS_POLL_INTERVAL, NULL);
			if (Status == XST_SUCCESS) {
				AuthJtagTaskRemoved = (u8)TRUE;
			}
#endif

			Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_LOADER_ID,
					XLoader_CheckDeviceStateChange,
					XLOADER_DEVICE_STATE_POLL_INTERVAL, NULL);
			if (Status == XST_SUCCESS) {
				DeviceStateTaskRemoved = (u8)TRUE;
			}
#endif
			/* Ignore if the tasks to be removed are not present */
			Status = XST_SUCCESS;

			/** - Disable SBI Interrupt */
			XPlmi_GicIntrDisable(XPLMI_SBI_GICP_INDEX, XPLMI_SBI_GICPX_INDEX);
		}
	}
	else if (Op.Mode == XPLMI_MODULE_SHUTDOWN_COMPLETE) {
		if (LoaderHandlerState != XPLMI_MODULE_SHUTDOWN_INITIATED_STATE) {
			goto END;
		}
		else if (LoaderHandlerState == XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE) {
			Status = XST_SUCCESS;
			goto END;
		}

		LoaderHandlerState = XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE;
		Status = XST_SUCCESS;
	}
	else if (Op.Mode == XPLMI_MODULE_SHUTDOWN_ABORT) {
		if (LoaderHandlerState == XPLMI_MODULE_SHUTDOWN_INITIATED_STATE) {
			LoaderHandlerState = XPLMI_MODULE_NORMAL_STATE;

			/** - Add Scheduler tasks if they are removed during shutdown init */
#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_AUTH_JTAG_EXCLUDE
			if (AuthJtagTaskRemoved == (u8)TRUE) {
				Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_LOADER_ID,
					XLoader_CheckAuthJtagIntStatus, NULL,
					XLOADER_AUTH_JTAG_INT_STATUS_POLL_INTERVAL,
					XPLM_TASK_PRIORITY_1, NULL, XPLMI_PERIODIC_TASK);
				if (Status != XST_SUCCESS) {
					goto END;
				}
				AuthJtagTaskRemoved = (u8)FALSE;
			}
#endif
			if (DeviceStateTaskRemoved == (u8)TRUE) {
				Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_LOADER_ID,
					XLoader_CheckDeviceStateChange, NULL,
					XLOADER_DEVICE_STATE_POLL_INTERVAL,
					XPLM_TASK_PRIORITY_0, NULL, XPLMI_PERIODIC_TASK);
				if (Status != XST_SUCCESS) {
					goto END;
				}
				DeviceStateTaskRemoved = (u8)FALSE;
			}
#endif

			/** Enable SBI Interrupt */
			XPlmi_GicIntrEnable(XPLMI_SBI_GICP_INDEX, XPLMI_SBI_GICPX_INDEX);
			Status = XST_SUCCESS;
		}
	}

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function initializes the loader with platform specific
 * 			initializations.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_PlatInit(void)
{
	int Status = XST_FAILURE;

	Status = XLoader_InitTrngInstance();

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the PDI's meta header data by calculating
 *			the hash using SHA3.
 *
 * @param	PdiPtr is the pointer to PDI instance
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_HDR_MEASUREMENT if error in meta header measurement.
 *
 *****************************************************************************/
int XLoader_HdrMeasurement(XilPdi* PdiPtr)
{
	int Status = XLOADER_ERR_HDR_MEASUREMENT;
#ifdef PLM_OCP
	XLoader_ImageMeasureInfo ImageInfo = {0U};
	XilPdi_MetaHdr * MetaHdrPtr = PdiPtr->MetaHdr;

	/**
	 * - Start the hash calculation for the meta header.
	 */
	ImageInfo.SubsystemID = 0U;
	ImageInfo.PcrInfo = XOCP_PCR_INVALID_VALUE;
	ImageInfo.Flags = XLOADER_MEASURE_START;
	Status = XLoader_DataMeasurement(&ImageInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Calculate the hash for Image header table.
	 */
	ImageInfo.DataAddr = (u64)(UINTPTR)&(MetaHdrPtr->ImgHdrTbl);
	ImageInfo.DataSize = sizeof(XilPdi_ImgHdrTbl);
	ImageInfo.Flags = XLOADER_MEASURE_UPDATE;
	Status = XLoader_DataMeasurement(&ImageInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Calculate the hash for Image header.
	 */
	ImageInfo.DataAddr = (u64)(UINTPTR)(MetaHdrPtr->ImgHdr);
	ImageInfo.DataSize = (MetaHdrPtr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN);
	Status = XLoader_DataMeasurement(&ImageInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Calculate the hash for Partition header.
	 */
	ImageInfo.DataAddr = (u64)(UINTPTR)(MetaHdrPtr->PrtnHdr);
	ImageInfo.DataSize = (MetaHdrPtr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN);
	Status = XLoader_DataMeasurement(&ImageInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Stop/finish the hash calculation.
	 */
	ImageInfo.Flags = XLOADER_MEASURE_FINISH;
	Status = XLoader_DataMeasurement(&ImageInfo);

	XPlmi_Printf(DEBUG_INFO, "INFO: Measurement may not be accurate when"
		" CDO is enabled with key hole write\n\r");
END:
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_HDR_MEASUREMENT, Status);
	}
#else
	(void)PdiPtr;
	Status = XST_SUCCESS;
#endif

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the data by calculating SHA3 hash.
 *
 * @param	ImageInfo Pointer to the XLoader_ImageMeasureInfo structure.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_DATA_MEASUREMENT if error in data measurement.
 *
 *****************************************************************************/
int XLoader_DataMeasurement(XLoader_ImageMeasureInfo *ImageInfo)
{
	volatile int Status = (int)XLOADER_ERR_DATA_MEASUREMENT;

#ifdef PLM_OCP
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XSecure_Sha3Hash Sha3Hash;
	u32 PcrNo;

	switch(ImageInfo->Flags) {
	case XLOADER_MEASURE_START:
		Status = XSecure_ShaStart(Sha3InstPtr, XSECURE_SHA3_384);
		break;
	case XLOADER_MEASURE_UPDATE:
		Status = XSecure_ShaUpdate(Sha3InstPtr,
				ImageInfo->DataAddr, ImageInfo->DataSize);
		break;
	case XLOADER_MEASURE_LAST:
		Status = XSecure_ShaLastUpdate(Sha3InstPtr);
		if (Status != XST_SUCCESS) {
			break;
		}
		Status = XSecure_ShaUpdate(Sha3InstPtr,
				ImageInfo->DataAddr, ImageInfo->DataSize);
		break;
	case XLOADER_MEASURE_FINISH:
		Status = XSecure_ShaFinish(Sha3InstPtr, (UINTPTR)&Sha3Hash, XLOADER_SHA3_LEN);
		break;
	default:
		break;
	}
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_DATA_MEASUREMENT, Status);
		goto END;
	}

	if (ImageInfo->Flags == XLOADER_MEASURE_FINISH) {
#ifdef PLM_OCP_ASUFW_KEY_MGMT
	/* Store subsystem digest for corresponding subsystem ID. */
	XOcp_StoreSubsysDigest(ImageInfo->SubsystemID, (u64)(UINTPTR)Sha3Hash.Hash);
#endif

		if (ImageInfo->PcrInfo != XOCP_PCR_INVALID_VALUE) {
			PcrNo = (u32)ImageInfo->PcrInfo & XOCP_PCR_NUMBER_MASK;
			/* Extend HW PCR */
			Status = XOcp_ExtendHwPcr(PcrNo,(u64)(UINTPTR)&Sha3Hash.Hash,
				XLOADER_SHA3_LEN);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			/* Extend SW PCR */
			Status = XOcp_ExtendSwPcr(PcrNo, *(u32 *)(ImageInfo->DigestIndex),
				(u64)(UINTPTR)Sha3Hash.Hash, XLOADER_SHA3_LEN,
				ImageInfo->OverWrite);
		}
	}

END:
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_DATA_MEASUREMENT, Status);
	}
#else
	(void)ImageInfo;
	Status = XST_SUCCESS;
#endif

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the Secure Configuration that is
 * 		SPK, SPK ID and Encryption Revoke ID and extends to the
 * 		specified PCR
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams instance.
 * @param	PcrInfo 	Provides the PCR number and Measurement Index
 * 				to be extended.
 * @param	DigestIndex	Pointer to the DigestIndex across the PCR
 * @param	OverWrite 	TRUE or FALSE to overwrite the extended digest or not
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_ERR_SECURE_CONFIG_MEASUREMENT if error in Secure config
 * 		measurement.
 *
 *****************************************************************************/
int XLoader_SecureConfigMeasurement(XLoader_SecureParams* SecurePtr, u32 PcrInfo, u32 *DigestIndex, u32 OverWrite)
{
	volatile int Status = (int)XLOADER_ERR_SECURE_CONFIG_MEASUREMENT;
#if (!defined(PLM_SECURE_EXCLUDE)) && (defined(PLM_OCP))
	volatile u32 IsAuthenticated = SecurePtr->IsAuthenticated;
	volatile u32 IsAuthenticatedTmp = SecurePtr->IsAuthenticated;
	volatile u32 IsEncrypted = SecurePtr->IsEncrypted;
	volatile u32 IsEncryptedTmp = SecurePtr->IsEncrypted;
	u32 MeasureIdx = (PcrInfo & XOCP_PCR_MEASUREMENT_INDEX_MASK) >> 16U;
	u32 PcrNo = PcrInfo & XOCP_PCR_NUMBER_MASK;
	XSecure_Sha3Hash Sha3Hash = {0U};
	volatile u32 ExtendAuthKeys = (XPlmi_In32(XLOADER_EFUSE_ROM_RSVD_CACHE_ADDRESS) &
			XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK) >>
			XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_SHIFT;;
	volatile u32 ExtendAuthKeysTmp = (XPlmi_In32(XLOADER_EFUSE_ROM_RSVD_CACHE_ADDRESS) &
			XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK) >>
			XLOADER_EFUSE_ROM_RSVD_AUTH_KEYS_TO_HASH_SHIFT;;

	if (PcrInfo == XOCP_PCR_INVALID_VALUE) {
                Status = XST_SUCCESS;
                goto END;
        }

	if (((ExtendAuthKeys != 0U) || (ExtendAuthKeysTmp != 0U)) &&
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
		MeasureIdx = MeasureIdx + 1U;

	}
	if ((IsEncrypted == (u8)TRUE) || (IsEncryptedTmp == (u8)TRUE)) {
		Status = XLoader_EncRevokeIdMeasurement(SecurePtr, &Sha3Hash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XLoader_ExtendEncRevokeId(&Sha3Hash, PcrNo, MeasureIdx, OverWrite);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		MeasureIdx = MeasureIdx + 1U;
	}

	*DigestIndex = MeasureIdx;

	Status = XST_SUCCESS;
END:
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SECURE_CONFIG_MEASUREMENT, Status);
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

/*****************************************************************************/
/**
 * @brief	This function returns the KEK IV Address in Boot Header.
 *
 * @return
 * 		- Address of KEK IV.
 *
 *****************************************************************************/
u32 XLoader_GetBootHeaderIvAddr(void)
{
	XilPdi* PdiPtr = XLoader_GetPdiInstance();

	return (u32)PdiPtr->MetaHdr->BootHdrPtr->KekIv;
}

#if (!defined(PLM_SECURE_EXCLUDE)) && (defined(PLM_OCP))
/*****************************************************************************/
/**
 * @brief	This function measures the SPK by calculating SHA3 hash.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash	Pointer to the XSecure_Sha3Hash.
 *
 * @return	XST_SUCCESS 	On Success
 * 		ErrorCode	On failure
 *
 *****************************************************************************/
static int XLoader_SpkMeasurement(XLoader_SecureParams* SecurePtr,
	XSecure_Sha3Hash* Sha3Hash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_ShaDigest(ShaInstPtr, XSECURE_SHA3_384,
			(UINTPTR)&SecurePtr->AcPtr->Spk,
			SecurePtr->AcPtr->SpkHeader.SPKSize,
			(u64)(UINTPTR)Sha3Hash, XLOADER_SHA3_LEN);
}

/*****************************************************************************/
/**
 * @brief	This function extends the SPK Hash into specified PCR.
 *
 * @param	SpkHash		SPK key hash measurement
 * @param	PcrInfo 	Provides the PCR number and Measurement Index
 * 				to be extended.
 * @param	DigestIndex	Digest index in PCR log, applicable to SW PCR only
 * @param       OverWrite 	TRUE or FALSE for overwriting the PCR measurement
 * 				for that measurement index
 *
 * @return	XST_SUCCESS	On Success
 * 		ErrorCode 	On Failure
 *
 *****************************************************************************/
static int XLoader_ExtendSpkHash(XSecure_Sha3Hash* SpkHash , u32 PcrNo, u32 DigestIndex, u32 OverWrite)
{
	volatile int Status = XST_FAILURE;

	Status = XOcp_ExtendHwPcr(PcrNo, (u64)(UINTPTR)&SpkHash->Hash,
			XLOADER_SHA3_LEN);
	if (Status != XST_SUCCESS) {
                goto END;
        }

	Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex,
			(u64)(UINTPTR)&SpkHash->Hash, XLOADER_SHA3_LEN, OverWrite);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the SPK ID by calculating SHA3 hash.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash	Pointer to the XSecure_Sha3Hash.
 *
 * @return	XST_SUCCESS	On Success
 * 		ErrorCode 	On failure
 *
 *****************************************************************************/
static int XLoader_SpkIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_ShaDigest(ShaInstPtr, XSECURE_SHA3_384, (UINTPTR)&SecurePtr->AcPtr->SpkId,
			sizeof(SecurePtr->AcPtr->SpkId), (u64)(UINTPTR)Sha3Hash, XLOADER_SHA3_LEN);
}

/*****************************************************************************/
/**
 * @brief	This function extends the Partition AC SPK ID into specified
 * 		PCR.
 *
 * @param	SpkIdHash	Partition AC SPK ID Hash
 * @param	PcrInfo		Provides the PCR number and Measurement Index
 * 				to be extended
 * @param	DigestIndex 	Digest index in PCR log, applicable for SW PCR only
 * @param       OverWrite       TRUE or FALSE for overwriting the PCR measurement
 * 				for that measurement index
 *
 * @return	XST_SUCCESS	On Success
 * 		ErrorCode	On Failure
 *
 *****************************************************************************/
static int XLoader_ExtendSpkId(XSecure_Sha3Hash* SpkIdHash, u32 PcrNo, u32 DigestIndex, u32 OverWrite)
{
	volatile int Status = XST_FAILURE;

	Status = XOcp_ExtendHwPcr(PcrNo, (u64)(UINTPTR)&SpkIdHash->Hash, XLOADER_SHA3_LEN);
	if (Status != XST_SUCCESS) {
                goto END;
        }

        Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex,
				(u64)(UINTPTR)&SpkIdHash->Hash, XLOADER_SHA3_LEN, OverWrite);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the Encryption Revoke ID by calculating SHA3 hash.
 *
 * @param	SecurePtr	Pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash	Pointer to the XSecure_Sha3Hash.
 *
 * @return	XST_SUCCESS	On success
 * 		ErrorCode 	On failure
 *
 *****************************************************************************/
static int XLoader_EncRevokeIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetShaInstance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_ShaDigest(ShaInstPtr, XSECURE_SHA3_384, (UINTPTR)&SecurePtr->PrtnHdr->EncRevokeID,
			sizeof(SecurePtr->PrtnHdr->EncRevokeID), (u64)(UINTPTR)Sha3Hash, XLOADER_SHA3_LEN);
}

/*****************************************************************************/
/**
 * @brief	This function extends the Partition Header Revoke ID into
 * 		specified PCR.
 *
 * @param	RevokeIdHash	Partition Header Revocation ID Hash
 * @param	PcrInfo		Provides the PCR number and Measurement Index
 * 				to be extended
 * @param	DigestIndex	Digest index in PCR log, applicable to SW PCR only
 * @param       OverWrite       TRUE or FALSE for overwriting the PCR measurement
 *
 * @return	XST_SUCCESS	On success
 * 		ErrorCode 	On failure
 *
 *****************************************************************************/
static int XLoader_ExtendEncRevokeId(XSecure_Sha3Hash* RevokeIdHash, u32 PcrNo, u32 DigestIndex, u32 OverWrite)
{
	volatile int Status = XST_FAILURE;

	Status = XOcp_ExtendHwPcr(PcrNo, (u64)(UINTPTR)&RevokeIdHash->Hash, XLOADER_SHA3_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex,
			(u64)(UINTPTR)&RevokeIdHash->Hash, XLOADER_SHA3_LEN, OverWrite);

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function enables or disable Jtag Access
 * 			Command payload parameters are
 *				- Flag (enable / disable)
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_INVALID_JTAG_OPERATION
 *
*****************************************************************************/
int XLoader_ConfigureJtagState(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
#ifndef PLM_SECURE_EXCLUDE
	u32 Flag = (u32)(Cmd->Payload[XLOADER_CMD_CONFIG_JTAG_STATE_FLAG_INDEX]
			& XLOADER_CMD_CONFIG_JTAG_STATE_FLAG_MASK);

	if (((Flag != XLOADER_CONFIG_JTAG_STATE_FLAG_ENABLE) &&
			(Flag != XLOADER_CONFIG_JTAG_STATE_FLAG_DISABLE))) {
		/** Invalid JTAG Operation request */
		Status = XLOADER_ERR_INVALID_JTAG_OPERATION;
		goto END;
	}

	if (Flag == XLOADER_CONFIG_JTAG_STATE_FLAG_ENABLE) {
		Status = XLoader_EnableJtag(XLOADER_CONFIG_DAP_STATE_NONSECURE_DBG);
	} else {
		Status = XLoader_DisableJtag();
	}

END:
#else
	Status = XLOADER_ERR_INVALID_JTAG_OPERATION;
	(void)Cmd;
#endif
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the Trng instance.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_TRNG_INIT_FAIL if TRNG initialization fails.
 *
 *****************************************************************************/
static int XLoader_InitTrngInstance(void)
{
	int Status = XST_FAILURE;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();

	XTrngpsx_Config *CfgPtr = NULL;

	CfgPtr = XTrngpsx_LookupConfig(XLOADER_TRNG_DEVICE_ID);
	if (CfgPtr == NULL) {
		Status = XLOADER_TRNG_INIT_FAIL;
		goto END;
	}

	Status = XTrngpsx_CfgInitialize(TrngInstance, CfgPtr, CfgPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks and updates the secure state configuration
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure
 *
*****************************************************************************/
int XLoader_CheckAndUpdateSecureState(void)
{
	int Status = XST_FAILURE;

#if (!defined(PLM_SECURE_EXCLUDE)) && (defined(PLM_OCP))
	Status = XOcp_CheckAndExtendSecureState();
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_STATE_CONFIG_MEASUREMENT_ERROR, Status);
	}
#else
	Status = XST_SUCCESS;
#endif

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This Function does the following:
 *		- It resets the SHA instance 1.
 *		- It puts the SHA instance 1 into initialized state.
 *
*****************************************************************************/
void XLoader_ShaInstance1Reset(void)
{
	 /** Not Applicable for Versal_2Ve_2Vm */
}
