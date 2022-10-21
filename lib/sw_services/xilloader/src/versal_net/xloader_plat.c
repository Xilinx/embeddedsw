/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_plat.c
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
#include "xloader_plat.h"
#include "xplmi_update.h"
#include "xplmi.h"
#include "xilpdi.h"
#include "xplmi_gic_interrupts.h"
#ifdef PLM_OCP
#include "xsecure_sha.h"
#endif
#include "xplmi_scheduler.h"
#include "xplmi_plat.h"

/************************** Constant Definitions *****************************/
#define XLOADER_IMAGE_INFO_VERSION	(1U)
#define XLOADER_IMAGE_INFO_LCVERSION	(1U)
#define XLOADER_PDI_INST_VERSION 	(1U)
#define XLOADER_PDI_INST_LCVERSION 	(1U)
#define XLOADER_PDI_LIST_VERSION 	(1U)
#define XLOADER_PDI_LIST_LCVERSION 	(1U)
#define XLOADER_ATF_HANDOFF_PARAMS_VERSION 	(1U)
#define XLOADER_ATF_HANDOFF_PARAMS_LCVERSION 	(1U)
#define XLOADER_TCM_A_0 (0U)
#define XLOADER_TCM_A_1 (1U)
#define XLOADER_TCM_B_0 (2U)
#define XLOADER_TCM_B_1 (3U)
#define XLOADER_RPU_CLUSTER_A (0U)
#define XLOADER_RPU_CLUSTER_B (1U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu,
	const u32 DstnCluster);
static int XLoader_GetLoadAddr(u32 DstnCpu, u32 DstnCluster, u64 *LoadAddrPtr,
	u32 Len);
static int XLoader_InitSha1Instance(void);

/************************** Variable Definitions *****************************/
#ifdef PLM_OCP
static XSecure_Sha3 Sha1Instance;
#endif

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

	return &ImageInfoTbl;
}

/*****************************************************************************/
/**
 * @brief	This function provides PdiInstance pointer
 *
 * @return	Pointer to PdiInstance
 *
 *****************************************************************************/
XilPdi *XLoader_GetPdiInstance(void)
{
	static XilPdi PdiInstance __attribute__ ((aligned(4U))) = {0U};

	EXPORT_LOADER_DS(ImageInfoTbl, XLOADER_PDI_INST_DS_ID,
		XLOADER_PDI_INST_VERSION, XLOADER_PDI_INST_LCVERSION,
		sizeof(PdiInstance), (u32)(UINTPTR)&PdiInstance);

	return &PdiInstance;
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
 * @brief	This function is used to start the subsystems in the PDI.
 *
 * @param	PdiPtr Pdi instance pointer
 *
 * @return	XST_SUCCESS on success and error code on failure
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

	/* Handoff to the cpus */
	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;
		ClusterId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CLUSTER_MASK;
		ClusterId >>= XIH_PH_ATTRB_DSTN_CLUSTER_SHIFT;
		HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;
		Status = XST_FAILURE;
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
	/*
	 * Make Number of handoff CPUs to zero
	 */
	PdiPtr->NoOfHandoffCpus = 0x0U;
	return Status;
}

/****************************************************************************/
/**
* @brief	This function sets the handoff parameters to the ARM Trusted
* Firmware(ATF). Some of the inputs for this are taken from image partition
* header. A pointer to the structure containing these parameters is stored in
* the PMC_GLOBAL.GLOBAL_GEN_STORAGE4 register, which ATF reads.
*
* @param	PrtnHdr is pointer to Partition header details
*
* @return	None
*
*****************************************************************************/
void XLoader_SetATFHandoffParameters(const XilPdi_PrtnHdr *PrtnHdr)
{
	u32 PrtnAttrbs;
	u32 PrtnFlags;
	u8 LoopCount = 0U;
	XilPdi_ATFHandoffParams *ATFHandoffParams = XLoader_GetATFHandoffParamsAddr();

	PrtnAttrbs = PrtnHdr->PrtnAttrb;

	PrtnFlags =
		(((PrtnAttrbs & XIH_PH_ATTRB_A78_EXEC_ST_MASK)
				>> XIH_ATTRB_A78_EXEC_ST_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_ENDIAN_MASK)
				>> XIH_ATTRB_ENDIAN_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_TZ_SECURE_MASK)
				<< XIH_ATTRB_TR_SECURE_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_TARGET_EL_MASK)
				<< XIH_ATTRB_TARGET_EL_SHIFT_DIFF));

	PrtnAttrbs &= XIH_PH_ATTRB_DSTN_CPU_MASK;
	/* Update CPU number based on destination CPU */
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
		for (; LoopCount < (u8)ATFHandoffParams->NumEntries;
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
 * 		SD boot modes

 * @param	SecBootMode is the secondary boot mode value
 * @param	PdiSrc is pointer to the source of PDI
 * @param	PdiAddr is the pointer to the address of the Pdi

 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_GetSDPdiSrcNAddr(u32 SecBootMode, XilPdi *PdiPtr, u32 *PdiSrc,
		u32 *PdiAddr)
{
	int Status = XST_FAILURE;

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
 * @brief	This function requests TCM
 * depending upon input param and R52-0 and R52-1 cores as required for TCMs.
 *
 * @param	TcmId denotes TCM_A or TCM_B or TCM_C
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_RequestTCM(u8 TcmId)
{
	int Status = XST_FAILURE;
	u32 CapAccess = (u32)PM_CAP_ACCESS;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
	u32 ErrorCode;

	if (XLOADER_TCM_A_0 == TcmId) {

		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_A_0A,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_0_A;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_A_0B,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_0_A;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_A_0C,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_0_A;
			goto END;
		}
	}else if (XLOADER_TCM_A_1 == TcmId) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_A_1A,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_1_A;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_A_1B,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_1_A;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_A_1C,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_1_A;
			goto END;
		}
	}else if (XLOADER_TCM_B_0 == TcmId) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_B_0A,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_0_B;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_B_0B,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_0_B;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_B_0C,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_0_B;
			goto END;
		}
	}else if (XLOADER_TCM_B_1 == TcmId) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_B_1A,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_1_B;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_B_1B,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_1_B;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_B_1C,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (XST_SUCCESS != Status) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_1_B;
			goto END;
		}
	}

END:
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, 0);
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
 * @return	XST_SUCCESS on success and error code on failure
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

	/*
	 * Requirements:
	 *
	 * PSM:
	 * For PSM, PSM should be taken out of reset before loading
	 * PSM RAM should be ECC initialized
	 *
	 * OCM:
	 * OCM RAM should be ECC initialized
	 *
	 * R5:
	 * R5 should be taken out of reset before loading
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

	if ((XIH_PH_ATTRB_DSTN_CPU_A78_3 < PrtnParams->DstnCpu) &&
		(XIH_PH_ATTRB_DSTN_CPU_PSM > PrtnParams->DstnCpu)) {
		if (((XIH_PH_ATTRB_DSTN_CPU_R52_0 == PrtnParams->DstnCpu) ||
			(XPM_RPU_MODE_LOCKSTEP == Mode)) && (XLOADER_RPU_CLUSTER_A == DstnCluster)) {
			Status = XLoader_RequestTCM(XLOADER_TCM_A_0);
		}else if ((XIH_PH_ATTRB_DSTN_CPU_R52_1 == PrtnParams->DstnCpu) &&
			(XLOADER_RPU_CLUSTER_A == DstnCluster)) {
			Status = XLoader_RequestTCM(XLOADER_TCM_A_1);
		}else if (((XIH_PH_ATTRB_DSTN_CPU_R52_0 == PrtnParams->DstnCpu) ||
			(XPM_RPU_MODE_LOCKSTEP == Mode)) && (XLOADER_RPU_CLUSTER_B == DstnCluster)) {
			Status = XLoader_RequestTCM(XLOADER_TCM_B_0);
		}else if ((XIH_PH_ATTRB_DSTN_CPU_R52_1 == PrtnParams->DstnCpu) &&
			(XLOADER_RPU_CLUSTER_B == DstnCluster)) {
			Status = XLoader_RequestTCM(XLOADER_TCM_B_1);
		}
	}
	if (XST_SUCCESS != Status) {
		goto END;
	}

	Status = XLoader_GetLoadAddr(PrtnParams->DstnCpu, DstnCluster,
			&PrtnParams->DeviceCopy.DestAddr,
			(PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN));
	if (XST_SUCCESS != Status) {
		goto END;
	}
	Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams->DeviceCopy, SecureParams);
	if (XST_SUCCESS != Status) {
			goto END;
	}

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A78_1)) {
		/*
		 *  Populate handoff parameters to ATF
		 *  These correspond to the partitions of application
		 *  which ATF will be loading
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

/****************************************************************************/
/**
 * @brief	This function is used to check whether cpu has handoff address
 * stored in the handoff structure.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	DstnCpu is the cpu which needs to be checked
 * @param	DstnCluster is the cluster which needs to be checked
 *
 * @return	XST_SUCCESS if the DstnCpu is successfully added to Handoff list
 *          XST_FAILURE if the DstnCpu is already added to Handoff list
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
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	u32 DstnCpu = XIH_PH_ATTRB_DSTN_CPU_NONE;
	u32 DstnCluster = XIH_PH_ATTRB_DSTN_CLUSTER_0;
	u32 CpuNo = XLOADER_MAX_HANDOFF_CPUS;
	u32 PrtnNum = PdiPtr->PrtnNum;
	/* Assign the partition header to local variable */
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
			/* Update the CPU settings */
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
 * @brief	This function updates the load address based on the
 * destination CPU.
 *
 * @param	DstnCpu is destination CPU
 * @param	DstnCluster is destination Cluste
 * @param	LoadAddrPtr is the destination load address pointer
 * @param	Len is the length of the partition
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_GetLoadAddr(u32 DstnCpu, u32 DstnCluster, u64 *LoadAddrPtr, u32 Len)
{
	int Status = XST_FAILURE;
	u64 Address = *LoadAddrPtr;

	if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_0) &&
			((Address < (XLOADER_R52_TCMA_LOAD_ADDRESS +
			XLOADER_R52_TCM_TOTAL_LENGTH)))) {
		if (((Address % XLOADER_R52_TCM_TOTAL_LENGTH) + Len) >
			XLOADER_R52_TCM_TOTAL_LENGTH) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
			goto END;
		}

		Address += XLOADER_R52_0A_TCMA_BASE_ADDR +
				(DstnCluster * XLOADER_R52_TCM_CLUSTER_OFFSET);
	}
	else if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_1) &&
			((Address < (XLOADER_R52_TCMA_LOAD_ADDRESS +
			XLOADER_R52_TCM_TOTAL_LENGTH)))) {
		if (((Address % XLOADER_R52_TCM_TOTAL_LENGTH) + Len) >
			XLOADER_R52_TCM_TOTAL_LENGTH) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
			goto END;
		}

		Address += XLOADER_R52_1A_TCMA_BASE_ADDR +
				(DstnCluster * XLOADER_R52_TCM_CLUSTER_OFFSET);
	}
	else {
		/* Do nothing */
	}

	/*
	 * Update the load address
	 */
	*LoadAddrPtr = Address;
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
	XPlmi_Printf(DEBUG_GENERAL, "Deferred Error Occurred during CDO"
		"processing\n\r");

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
	int Status = XST_FAILURE;
	static u8 LoaderHandlerState = XPLMI_MODULE_NORMAL_STATE;
#ifndef PLM_SECURE_EXCLUDE
	static u8 AuthJtagTaskRemoved = (u8)FALSE;
	static u8 DeviceStateTaskRemoved = (u8)FALSE;
#endif

	if (Op.Mode == XPLMI_MODULE_SHUTDOWN_INITIATE) {
		if (LoaderHandlerState == XPLMI_MODULE_NORMAL_STATE) {
			LoaderHandlerState = XPLMI_MODULE_SHUTDOWN_INITIATED_STATE;

			/* Remove Scheduler tasks if they are existing */
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

			/* Disable SBI Interrupt */
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

			/* Add Scheduler tasks if they are removed during shutdown init */
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

			/* Enable SBI Interrupt */
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

	Status = XLoader_InitSha1Instance();

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function measures the PDI's meta header data by calculating
 *		the hash using SHA3.
 *
 * @param	PdiPtr is the pointer to PDI instance
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_HdrMeasurement(XilPdi* PdiPtr)
{
	int Status = XLOADER_ERR_HDR_MEASUREMENT;
#ifdef PLM_OCP
	XilPdi_MetaHdr * MetaHdrPtr = &PdiPtr->MetaHdr;

	Status = XLoader_DataMeasurement(0U, 0U, 0U, XLOADER_MEASURE_START);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XLoader_DataMeasurement((u64)(UINTPTR)&(MetaHdrPtr->ImgHdrTbl),
			sizeof(XilPdi_ImgHdrTbl), 0U, XLOADER_MEASURE_UPDATE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XLoader_DataMeasurement((u64)(UINTPTR)(MetaHdrPtr->ImgHdr),
			(MetaHdrPtr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN), 0U,
			 XLOADER_MEASURE_UPDATE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XLoader_DataMeasurement((u64)(UINTPTR)(MetaHdrPtr->PrtnHdr),
			(MetaHdrPtr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN), 0U,
			 XLOADER_MEASURE_UPDATE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XLoader_DataMeasurement(0U, 0U, 0U, XLOADER_MEASURE_FINISH);
	XPlmi_Printf(DEBUG_GENERAL, "INFO: Measurement may not be accurate when"
		" CDO is enabled with key whole write\n\r");
END:
	if (Status != XST_SUCCESS) {
		Status |= XLOADER_ERR_HDR_MEASUREMENT;
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
 * @param	DataAddr is the address of the data to be measured.
 * @param	DataSize is the size of the data to be measured.
 * @param	PcrInfo provides the PCR number to be extended.
 * @param	Flags - The hash calculation flags
 * 			- XLOADER_MEASURE_START : Sha3 start
 * 			- XLOADER_MEASURE_UPDATE: Sha3 update
 * 			- XLOADER_MEASURE_FINISH: Sha3Finish
 * 			- Any other option will be an error.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_DataMeasurement(u64 DataAddr, u32 DataSize, u32 PcrInfo, u8 Flags)
{
	int Status = XLOADER_ERR_DATA_MEASUREMENT;
#ifdef PLM_OCP
	XSecure_Sha3Hash Sha3Hash;
	(void)PcrInfo;

	switch(Flags) {
	case XLOADER_MEASURE_START:
		Status = XSecure_Sha3Start(&Sha1Instance);
		break;
	case XLOADER_MEASURE_UPDATE:
		Status = XSecure_Sha3Update64Bit(&Sha1Instance,
				DataAddr, DataSize);
		break;
	case XLOADER_MEASURE_FINISH:
		Status = XSecure_Sha3Finish(&Sha1Instance, &Sha3Hash);
		break;
	default:
		break;
	}
	if (Status != XST_SUCCESS) {
		Status |= XLOADER_ERR_DATA_MEASUREMENT;
	}
#else
	(void)DataAddr;
	(void)DataSize;
	(void)PcrInfo;
	(void)Flags;
	Status = XST_SUCCESS;
#endif

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the SHA1 instance.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_InitSha1Instance(void)
{
	int Status = XLOADER_ERR_SHA1_INIT;
#ifdef PLM_OCP
	XPmcDma *PmcDmaPtr = XPlmi_GetDmaInstance(0U);

	Status = XSecure_Sha3LookupConfig(&Sha1Instance, XLOADER_SHA1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Initialize(&Sha1Instance, PmcDmaPtr);
END:
	if (Status != XST_SUCCESS) {
		Status = XLOADER_ERR_SHA1_INIT;
	}
#else
	Status = XST_SUCCESS;
#endif
	return Status;
}
