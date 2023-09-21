/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xloader_plat.c
*
* This file contains the versal_net specific code related to PDI image loading.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_update.h"
#include "xloader.h"
#include "xpm_device.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_nodeid.h"
#include "xpm_rpucore_plat.h"
#include "xloader_plat.h"
#include "xplmi_update.h"
#include "xplmi.h"
#include "xilpdi.h"
#include "xplmi_gic_interrupts.h"
#ifdef PLM_OCP
#include "xocp.h"
#include "xocp_keymgmt.h"
#include "xsecure_sha.h"
#endif
#include "xsecure_init.h"
#include "xplmi_scheduler.h"
#include "xplmi_plat.h"
#include "xplmi_hw.h"
#include "xtrngpsx.h"

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
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu,
	const u32 DstnCluster);
static int XLoader_InitSha3Instance1(void);
static int XLoader_InitTrngInstance(void);
#if (!defined(PLM_SECURE_EXCLUDE)) && (defined(PLM_OCP))
static int XLoader_SpkMeasurement(XLoader_SecureParams* SecureParams,
	XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendSpkHash(XSecure_Sha3Hash* SpkHash , u32 PcrNo, u32 DigestIndex, u32 PdiType);
static int XLoader_SpkIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendSpkId(XSecure_Sha3Hash* SpkIdHash, u32 PcrInfo, u32 DigestIndex, u32 PdiType);
static int XLoader_EncRevokeIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash);
static int XLoader_ExtendEncRevokeId(XSecure_Sha3Hash* RevokeIdHash, u32 PcrInfo, u32 DigestIndex, u32 PdiType);
#endif
#ifdef PLM_OCP
static int XLoader_GenSubSysDevAk(u32 SubsystemID, u64 InHash);
#endif
#ifndef PLM_SECURE_EXCLUDE
static int XLoader_RunSha3Engine1Kat(XilPdi* PdiPtr);
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
	static XilPdi_ATFHandoffParams ATFHandoffParams
		__attribute__ ((aligned(4U))) = {0}; /**< Instance containing
							 ATF handoff params */

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
 * 			- XLOADER_ERR_WAKEUP_R52_0 if waking up the R52_1 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A78_0 if waking up the A78_0 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A78_1 if waking up the A78_1 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A78_2 if waking up the A78_2 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_A78_3 if waking up the A78_3 failed during
 * 			handoff.
 * 			- XLOADER_ERR_WAKEUP_PSM if waking up the PSM failed during handoff.
 *
 *****************************************************************************/
int XLoader_StartImage(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;
	u32 DeviceId;
	u32 ClusterId;
	u64 HandoffAddr;
	u8 SetAddress = 1U;
	u32 ErrorCode;

	/** - Start Handoff to the cpus */
	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;
		ClusterId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CLUSTER_MASK;
		ClusterId >>= XIH_PH_ATTRB_DSTN_CLUSTER_SHIFT;
		HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;
		Status = XST_FAILURE;
		/** - Wake up each processor */
		switch (CpuId)
		{
			case XIH_PH_ATTRB_DSTN_CPU_R52_0:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_1) {
					Status = XLOADER_ERR_WAKEUP_R52_0;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_R52_0;
				DeviceId = PM_DEV_RPU_A_0 + (ClusterId*2) +
						XLOADER_RPU_CORE0;
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d"
						" R52_0 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_R52_1:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_1) {
					Status = XLOADER_ERR_WAKEUP_R52_0;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_R52_0;
				DeviceId = PM_DEV_RPU_A_0 + (ClusterId*2) +
						XLOADER_RPU_CORE1;
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d"
						" R52_1 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_A78_0:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_3) {
					Status = XLOADER_ERR_WAKEUP_A78_0;
				}
				ErrorCode = XLOADER_ERR_WAKEUP_A78_0;
				DeviceId = PM_DEV_ACPU_0_0 + (ClusterId*4);
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d "
						" A78_0 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_A78_1:
				if (ClusterId > XIH_ATTRB_DSTN_CLUSTER_3) {
					Status = XLOADER_ERR_WAKEUP_A78_1;
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
				}
				ErrorCode = XLOADER_ERR_WAKEUP_A78_3;
				DeviceId = PM_DEV_ACPU_0_0 + (ClusterId*4) +
						XLOADER_APU_CORE3;
				XLoader_Printf(DEBUG_INFO, "Request Cluster %d "
						" A78_3 wakeup\r\n", ClusterId);
				break;

			case XIH_PH_ATTRB_DSTN_CPU_PSM:
				DeviceId = PM_DEV_PSM_PROC;
				ErrorCode = XLOADER_ERR_WAKEUP_PSM;
				SetAddress = 0U;
				XLoader_Printf(DEBUG_INFO, "Request PSM wakeup\r\n");
				break;

			default:
				Status = XST_SUCCESS;
				break;
		}
		if (Status == XST_FAILURE) {
			Status = XPm_RequestWakeUp(PM_SUBSYS_PMC, DeviceId,
				SetAddress, HandoffAddr, 0U, XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, Status);
				goto END;
			}
		}
		else if (Status != XST_SUCCESS) {
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
* @return
* 			- None
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
							<< XIH_PRTN_FLAGS_DSTN_CLUSTER_SHIFT_DIFF;

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
				ATFHandoffParams->Entry[LoopCount].EntryPoint =
					PrtnHdr->DstnExecutionAddr;
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

	/**
	 * - Get the PDI source address for the secondary boot device.
	 */
	switch(SecBootMode)
	{
		case XIH_IHT_ATTR_SBD_SDLS_B0:
		#ifdef XLOADER_SD_0
			*PdiSrc = XLOADER_PDI_SRC_SDLS_B0 |
				XLOADER_SD_RAWBOOT_MASK |
				(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
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
				(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
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
				(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
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
 * 			- XLOADER_ERR_PM_DEV_PSM_PROC if device request for PSM is failed.
 * 			- XLOADER_ERR_INVALID_R52_CLUSTER if invalid R52 cluster is
 * 			selected.
 *
 *****************************************************************************/
int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams)
{
	int Status = XST_FAILURE;
	u32 CapAccess = (u32)PM_CAP_ACCESS;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
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

	/**
	 * - Verify the load address.
	 */
	Status = XPlmi_VerifyAddrRange(PrtnParams->DeviceCopy.DestAddr, EndAddr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_ELF_LOAD_ADDR,
				Status);
		goto END;
	}
	PrtnParams->DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	DstnCluster = XilPdi_GetDstnCluster(PrtnHdr) >>
			XIH_PH_ATTRB_DSTN_CLUSTER_SHIFT;
	ClusterLockstep = XilPdi_GetClusterLockstep(PrtnHdr);

	/**
	 *
	 * - For PSM, PSM should be taken out of reset before loading.
	 * PSM RAM should be ECC initialized
	 *
	 * - For OCM, RAM should be ECC initialized
	 *
	 * - R5 should be taken out of reset before loading.
	 * R5 TCM should be ECC initialized
	 */
	if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_PSM) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_PSM_PROC,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_PSM_PROC, 0);
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
			if (DstnCluster > XIH_ATTRB_DSTN_CLUSTER_1) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_R52_CLUSTER, 0U);
				goto END;
			}
			DeviceId = PM_DEV_RPU_A_0 + (DstnCluster * 2) + XLOADER_RPU_CORE0;
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId, IOCTL_SET_RPU_OPER_MODE,
					      Mode, 0U, 0U, NULL, XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			XPmRpuCore_SetTcmBoot(DeviceId, (u8)TcmBootFlag);
			break;
		case XIH_PH_ATTRB_DSTN_CPU_R52_1:
			if (DstnCluster > XIH_ATTRB_DSTN_CLUSTER_1) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_R52_CLUSTER, 0U);
				goto END;
			}
			DeviceId = PM_DEV_RPU_A_0 + (DstnCluster * 2) + XLOADER_RPU_CORE1;
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId, IOCTL_SET_RPU_OPER_MODE,
					      Mode, 0U, 0U, NULL, XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			XPmRpuCore_SetTcmBoot(DeviceId, (u8)TcmBootFlag);
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_0:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster * 4);
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId, IOCTL_SET_APU_OPER_MODE,
					      Mode, 0U, 0U, NULL, XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_1:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE1;
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId, IOCTL_SET_APU_OPER_MODE,
					      Mode, 0U, 0U, NULL, XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_2:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE2;
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId, IOCTL_SET_APU_OPER_MODE,
					      Mode, 0U, 0U, NULL, XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_3:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE3;
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId, IOCTL_SET_APU_OPER_MODE,
					      Mode, 0U, 0U, NULL, XPLMI_CMD_SECURE);
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
	Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams->DeviceCopy, SecureParams);
	if (XST_SUCCESS != Status) {
			goto END;
	}

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_1) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_2) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_3)
		) {
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
		Status = (int)XLOADER_ERR_DDR_DEVICE_ID;
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
		Status = (int)XLOADER_ERR_PCOMPLETE_NOT_DONE;
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
			&(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);
	DstnCluster = XilPdi_GetDstnCluster(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) &&
	    (DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_PSM) &&
	    (DstnCluster <= XIH_PH_ATTRB_DSTN_CLUSTER_3)) {
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
 * @brief	This function checks if MJTAG workaround is required
 *
 * @return	Error Code of Deferred Error
 *
 *****************************************************************************/
int XLoader_ProcessDeferredError(void)
{
	/* TODO Add DDR5 dump if it is available */
	return XPlmi_UpdateStatus(XLOADER_ERR_DEFERRED_CDO_PROCESS, 0U);
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
	static u8 AuthJtagTaskRemoved = (u8)FALSE;
	static u8 DeviceStateTaskRemoved = (u8)FALSE;
#endif

	if (Op.Mode == XPLMI_MODULE_SHUTDOWN_INITIATE) {
		if (LoaderHandlerState == XPLMI_MODULE_NORMAL_STATE) {
			LoaderHandlerState = XPLMI_MODULE_SHUTDOWN_INITIATED_STATE;

			/** - Remove Scheduler tasks if they already exist. */
#ifndef PLM_SECURE_EXCLUDE
			Status = XPlmi_SchedulerRemoveTask(XPLMI_MODULE_LOADER_ID,
				XLoader_CheckAuthJtagIntStatus,
				XLOADER_AUTH_JTAG_INT_STATUS_POLL_INTERVAL, NULL);
			if (Status == XST_SUCCESS) {
				AuthJtagTaskRemoved = (u8)TRUE;
			}
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

	Status = XLoader_InitSha3Instance1();
	if(Status != XST_SUCCESS){
		goto END;
	}
	Status = XLoader_InitTrngInstance();

END:
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
	XilPdi_MetaHdr * MetaHdrPtr = &PdiPtr->MetaHdr;

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
		XPlmi_UpdateStatus(XLOADER_ERR_HDR_MEASUREMENT, Status);
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
	int Status = XLOADER_ERR_DATA_MEASUREMENT;

	XSecure_Sha3 *Sha3Instance = XLoader_GetSha3Engine1Instance();
	XSecure_Sha3Hash Sha3Hash;
#ifdef PLM_OCP
	u32 PcrNo;
	u32 DevAkIndex;
#endif
#ifndef PLM_SECURE_EXCLUDE
	XilPdi* PdiPtr = XLoader_GetPdiInstance();
#endif

#ifndef PLM_SECURE_EXCLUDE
	Status = XLoader_RunSha3Engine1Kat(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	switch(ImageInfo->Flags) {
	case XLOADER_MEASURE_START:
		Status = XSecure_Sha3Start(Sha3Instance);
		break;
	case XLOADER_MEASURE_UPDATE:
		Status = XSecure_Sha3Update64Bit(Sha3Instance,
				ImageInfo->DataAddr, ImageInfo->DataSize);
		break;
	case XLOADER_MEASURE_FINISH:
		Status = XSecure_Sha3Finish(Sha3Instance, &Sha3Hash);
		break;
	default:
		break;
	}
	if (Status != XST_SUCCESS) {
		XPlmi_UpdateStatus(XLOADER_ERR_DATA_MEASUREMENT, Status);
		goto END;
	}

#ifdef PLM_OCP
	DevAkIndex = XOcp_GetSubSysReqDevAkIndex(ImageInfo->SubsystemID);

	if ((ImageInfo->PcrInfo == XOCP_PCR_INVALID_VALUE) &&
			(DevAkIndex == XOCP_INVALID_DEVAK_INDEX)) {
		Status = XST_SUCCESS;
		goto END;
	}

	if (ImageInfo->Flags == XLOADER_MEASURE_FINISH) {
		if (DevAkIndex != XOCP_INVALID_DEVAK_INDEX) {
			/* Generate DEVAK */
			Status = XLoader_GenSubSysDevAk(ImageInfo->SubsystemID,
						(u64)(UINTPTR)Sha3Hash.Hash);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (ImageInfo->PcrInfo != XOCP_PCR_INVALID_VALUE) {
			PcrNo = ImageInfo->PcrInfo & XOCP_PCR_NUMBER_MASK;

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
#endif

END:
	if (Status != XST_SUCCESS) {
		XPlmi_UpdateStatus(XLOADER_ERR_DATA_MEASUREMENT, Status);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the SHA1 instance.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_SHA1_INIT if SHA1 initialization fails.
 *
 *****************************************************************************/
static int XLoader_InitSha3Instance1(void)
{
	int Status = XLOADER_ERR_SHA3_1_INIT;

	XPmcDma *PmcDmaPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	XSecure_Sha3 *Sha3Instance = XLoader_GetSha3Engine1Instance();

	Status = XSecure_Sha3LookupConfig(Sha3Instance, XLOADER_SHA3_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Initialize(Sha3Instance, PmcDmaPtr);
END:
	if (Status != XST_SUCCESS) {
		Status = XLOADER_ERR_SHA3_1_INIT;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the Secure Configuration that is
 * 		SPK, SPK ID and Encryption Revoke ID and extends to the
 * 		specified PCR
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	PcrInfo provides the PCR number and Measurement Index
 * 			to be extended.
 * @param	DigestIndex is pointer to the DigestIndex across the PCR
 * @param	OverWrite TRUE or FALSE to overwrite the extended digest or not
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_ERR_SECURE_CONFIG_MEASUREMENT if error in Secure config
 * 		measurement.
 *
 *****************************************************************************/
int XLoader_SecureConfigMeasurement(XLoader_SecureParams* SecurePtr, u32 PcrInfo, u32 *DigestIndex, u32 OverWrite)
{
	int Status = XLOADER_ERR_SECURE_CONFIG_MEASUREMENT;
#if (!defined(PLM_SECURE_EXCLUDE)) && (defined(PLM_OCP))
	u32 IsAuthenticated = SecurePtr->IsAuthenticated;
	u32 IsEncrypted = SecurePtr->IsEncrypted;
	u32 MeasureIdx = (PcrInfo & XOCP_PCR_MEASUREMENT_INDEX_MASK) >> 16U;
	u32 PcrNo = PcrInfo & XOCP_PCR_NUMBER_MASK;
	XSecure_Sha3Hash Sha3Hash = {0U};

	if (PcrInfo == XOCP_PCR_INVALID_VALUE) {
                Status = XST_SUCCESS;
                goto END;
        }

	if(IsAuthenticated == (u8)TRUE) {
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
	if (IsEncrypted == (u8)TRUE) {
		if (IsAuthenticated != (u8)TRUE) {
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

#if (!defined(PLM_SECURE_EXCLUDE)) && (defined(PLM_OCP))
/*****************************************************************************/
/**
 * @brief	This function measures the SPK by calculating SHA3 hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash  is pointer to the XSecure_Sha3Hash.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_SpkMeasurement(XLoader_SecureParams* SecurePtr,
	XSecure_Sha3Hash* Sha3Hash)
{
	int Status = XST_FAILURE;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	u32 AuthType;
	u32 SpkLen = 0U;

	AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);

	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
		SpkLen = XLOADER_SPK_SIZE - XOCP_WORD_LEN;
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
		SpkLen = (XLOADER_ECDSA_P384_KEYSIZE + XLOADER_ECDSA_P384_KEYSIZE);
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521) {
		SpkLen = (XLOADER_ECDSA_P521_KEYSIZE + XLOADER_ECDSA_P521_KEYSIZE);
	}

	Status = XSecure_Sha3Digest(Sha3InstPtr, (UINTPTR)&SecurePtr->AcPtr->Spk,
			SpkLen, Sha3Hash);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function extends the SPK Hash into specified PCR.
 *
 * @param	SpkHash SPK key hash measured
 * @param	PcrInfo provides the PCR number and Measurement Index
 * 		to be extended.
 * @param	DigestIndex Digest index in PCR log, applicable to SW PCR only
 * @param       PdiType Full or Partial or Restore PDI
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ExtendSpkHash(XSecure_Sha3Hash* SpkHash , u32 PcrNo, u32 DigestIndex, u32 PdiType)
{
	int Status = XST_FAILURE;

	Status = XOcp_ExtendHwPcr(PcrNo, (u64)(UINTPTR)&SpkHash->Hash,
			XLOADER_SHA3_LEN);
	if (Status != XST_SUCCESS) {
                goto END;
        }

	Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex,
			(u64)(UINTPTR)&SpkHash->Hash, XLOADER_SHA3_LEN, PdiType);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the SPK ID by calculating SHA3 hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash  is pointer to the XSecure_Sha3Hash.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_SpkIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash)
{
	int Status = XST_FAILURE;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();

	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Digest(Sha3InstPtr, (UINTPTR)&SecurePtr->AcPtr->SpkId,
			sizeof(SecurePtr->AcPtr->SpkId), Sha3Hash);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function extends the Partition AC SPK ID into specified
 * 		PCR.
 *
 * @param	SpkIdHash Partition AC SPK ID Hash
 * @param	PcrInfo provides the PCR number and Measurement Index
 * 		to be extended.
 * @param	DigestIndex Digest index in PCR log, applicable to SW PCR only
 * @param       PdiType Full or Partial or Restore PDI
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ExtendSpkId(XSecure_Sha3Hash* SpkIdHash, u32 PcrNo, u32 DigestIndex, u32 PdiType)
{
	int Status = XST_FAILURE;

	Status = XOcp_ExtendHwPcr(PcrNo, (u64)(UINTPTR)&SpkIdHash->Hash, XLOADER_SHA3_LEN);
	if (Status != XST_SUCCESS) {
                goto END;
        }

        Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex,
				(u64)(UINTPTR)&SpkIdHash->Hash, XLOADER_SHA3_LEN, PdiType);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the Encryption Revoke ID by calculating SHA3 hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	Sha3Hash  is pointer to the XSecure_Sha3Hash.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_EncRevokeIdMeasurement(XLoader_SecureParams* SecurePtr, XSecure_Sha3Hash* Sha3Hash)
{
	int Status = XST_FAILURE;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();

	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Digest(Sha3InstPtr, (UINTPTR)&SecurePtr->PrtnHdr->EncRevokeID,
			sizeof(SecurePtr->PrtnHdr->EncRevokeID), Sha3Hash);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function extends the Partition Header Revoke ID into
 * 		specified PCR.
 *
 * @param	RevokeIdHash Partition Header Revocation ID Hash
 * @param	PcrInfo provides the PCR number and Measurement Index
 * 		to be extended.
 * @param	DigestIndex Digest index in PCR log, applicable to SW PCR only
 * @param       PdiType Full or Partial or Restore PDI
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ExtendEncRevokeId(XSecure_Sha3Hash* RevokeIdHash, u32 PcrNo, u32 DigestIndex, u32 PdiType)
{
	int Status = XST_FAILURE;

	Status = XOcp_ExtendHwPcr(PcrNo, (u64)(UINTPTR)&RevokeIdHash->Hash, XLOADER_SHA3_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XOcp_ExtendSwPcr(PcrNo, DigestIndex,
			(u64)(UINTPTR)&RevokeIdHash->Hash, XLOADER_SHA3_LEN, PdiType);

END:
	return Status;
}
#endif

#ifdef PLM_OCP
/*****************************************************************************/
/**
 * @brief	This function generates the DEVAK for requested subsystem by user.
 *
 * @param	SubsystemID is the ID of image.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_GenSubSysDevAk(u32 SubsystemID, u64 InHash)
{
	int Status = XST_FAILURE;
	u32 DevAkIndex = XOcp_GetSubSysReqDevAkIndex(SubsystemID);
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();

	if (DevAkIndex != XOCP_INVALID_DEVAK_INDEX)  {
		DevAkData = DevAkData + DevAkIndex;
		Status = XPlmi_MemCpy64((u64)(UINTPTR)DevAkData->SubSysHash,
					InHash, XLOADER_SHA3_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XOcp_GenerateDevAk(SubsystemID);
		XPlmi_Printf(DEBUG_DETAILED, "DEV AK of subsystem is generated %x\n\r",
					SubsystemID);
	}
	else {
		XPlmi_Printf(DEBUG_DETAILED, "DEV AK of subsystem is not generated \n\r");
		Status = XST_SUCCESS;
	}
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
		XLoader_EnableJtag(XLOADER_CONFIG_DAP_STATE_NONSECURE_DBG);
	} else {
		XLoader_DisableJtag();
	}

	Status = XST_SUCCESS;

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
	XTrngpsx_Instance  *TrngInstance = XSecure_GetTrngInstance();
	XTrngpsx_Config *CfgPtr = NULL;

	CfgPtr = XTrngpsx_LookupConfig(0);
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

#ifndef PLM_SECURE_EXCLUDE
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
 * 		- Error code on failure
 *
*****************************************************************************/
static int XLoader_RunSha3Engine1Kat(XilPdi* PdiPtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha3 *Sha3Instance = XLoader_GetSha3Engine1Instance();

	XLoader_ClearKatOnPPDI(PdiPtr, XPLMI_SECURE_SHA3_1_KAT_MASK);

	if (XPlmi_IsKatRan(XPLMI_SECURE_SHA3_1_KAT_MASK) != (u8)TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_Sha3Kat, Sha3Instance);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}

		PdiPtr->PlmKatStatus |= XPLMI_SECURE_SHA3_1_KAT_MASK;

		/* Update KAT status */
		XPlmi_UpdateKatStatus(PdiPtr->PlmKatStatus);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
#endif
