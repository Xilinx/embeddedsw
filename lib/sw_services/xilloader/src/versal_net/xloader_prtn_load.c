/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_prtn_load.c
*
* This is the file which contains partition load code for the Platform
* loader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
* 1.01  vnsl 04/28/2019 Added security support
*       kc   05/09/2019 Added code for PSM RAM ECC initialization
*       bsv  06/11/2019 Added TCM power up code to Xilloader to fix issue in
*						R5-1 split mode functionality
*       bsv  06/24/2019 Moved ECC initialization code from Xilloader to Xilpm
*       js   06/27/2019 Updated PSM arguments
*       vnsl 07/09/2019 Added PPK and SPK integrity checks
*       vnsl 07/09/2019 Added authentication + encryption support
*       kc   09/05/2019 Added code to use PMCDMA0 and PMCDMA1 in parallel
* 1.02  ma   12/12/2019 Added support for passing hand off parameters to ATF
*       kc   12/17/2019 Added support for deferred error mechanism for mask poll
*       bsv  01/12/2020 Changes related to bitstream loading
*       bsv  01/30/2002 Enabled direct DMA from boot devices to CFI
*       ma   02/03/2020 Change XPlmi_MeasurePerfTime to retrieve Performance
*                       time and print
*       bsv  02/28/2020 Added support for delay handoff
*       ma   03/02/2020 Added support for logging trace events
*       bsv  04/09/2020 Code clean up of Xilloader
* 1.03  kc   06/12/2020 Added IPI mask to PDI CDO commands to get subsystem info
*       kal  07/20/2020 Added double buffering support for secure CDOs
*       bsv  07/29/2020 Added delay load support
*       skd  07/29/2020 Added parallel DMA support for Qspi and Ospi
*       bsv  08/06/2020 Code clean up
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       kal  08/23/2020 Added parallel DMA support for Qspi and Ospi for secure
*       kpt  09/07/2020 Fixed key rolling issue for secure cases
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP and PCIE
*                       boot modes
*       bsv  10/13/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  bsv  01/28/2021 Initialize variables to invalid values
*       bsv  01/29/2021 Added check for NPI errors after loading every partition
* 1.05  bm   03/04/2021 Added address range check before loading elfs
*       ma   03/24/2021 Redirect XilPdi prints to XilLoader
*       ma   03/24/2021 Minor updates to prints in XilLoader
*       bl   04/01/2021 Add secure IPI arg to XPm_DevIoctl and
*                       XPm_RequestWakeUp
*       bsv  04/13/2021 Added support for variable Keyhole sizes in
*                       DmaWriteKeyHole command
*       bsv  04/16/2021 Add provision to store Subsystem Id in XilPlmi
*       rp   04/20/2021 Add extra arg for calls to XPm_RequestDevice and
*			XPm_ReleaseDevice
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_dma.h"
#include "xplmi_debug.h"
#include "xplmi_cdo.h"
#include "xplmi_util.h"
#include "xloader_secure.h"
#include "xloader_ddr.h"
#include "xplmi.h"
#include "xil_util.h"
#include "xplmi_err.h"
#ifndef PLM_PM_EXCLUDE
#include "xpm_nodeid.h"
#include "xpm_api.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_SUCCESS_NOT_PRTN_OWNER	(0x100U)

#define XLOADER_R52_0_TCMA_ECC_DONE	(0x00000001U)
#define XLOADER_R52_1_TCMA_ECC_DONE	(0x00000002U)

#define XLOADER_A78_CLUSTER_CONFIGURED 		(1U)
#define XLOADER_R52_CLUSTER_CONFIGURED 		(1U)
#define XLOADER_APU_CLUSTER_LOCKSTEP_DISABLE	(0U)
#define XLOADER_APU_CLUSTER_LOCKSTEP_ENABLE	(1U)
#define XLOADER_RPU_CLUSTER_LOCKSTEP_DISABLE	(1U)
#define XLOADER_RPU_CLUSTER_LOCKSTEP_ENABLE	(0U)

#define GetRpuRstMask(Mask, ClusterNum, CoreNum)  (Mask << ((2U * ClusterNum)\
						+ CoreNum))

/************************** Function Prototypes ******************************/
static int XLoader_PrtnHdrValidation(const XilPdi_PrtnHdr* PrtnHdr, u32 PrtnNum);
static int XLoader_ProcessPrtn(XilPdi* PdiPtr);
static int XLoader_PrtnCopy(const XilPdi* PdiPtr, const XLoader_DeviceCopy* DeviceCopy,
	XLoader_SecureParams* SecureParams);
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu,
	const u32 DstnCluster);
static int XLoader_GetLoadAddr(u32 DstnCpu, u32 DstnCluster, u64 *LoadAddrPtr, u32 Len);
static int XLoader_ProcessCdo (const XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
	XLoader_SecureParams* SecureParams);
static int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr* PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams);
static void XLoader_R52Config(u8 ClusterNum, u8 CoreNum, u32 LockstepVal,
		u64 HandoffAddr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function loads the partition.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_LoadImagePrtns(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	u32 PrtnIndex;
	u64 PrtnLoadTime;
	XPlmi_PerfTime PerfTime = {0U};

	if ((PdiPtr->CopyToMem == (u8)FALSE) && (PdiPtr->DelayLoad == (u8)FALSE)) {
		XPlmi_Printf(DEBUG_GENERAL,
			"+++Loading Image#: 0x%0x, Name: %s, Id: 0x%08x\n\r",
			PdiPtr->ImageNum,
			(char *)PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName,
			PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
	}
	else {
		if (PdiPtr->DelayLoad == (u8)TRUE) {
			XPlmi_Printf(DEBUG_GENERAL,
				"+++Skipping Image#: 0x%0x, Name: %s, Id: 0x%08x\n\r",
				PdiPtr->ImageNum,
				(char *)PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName,
				PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
		}
		if (PdiPtr->CopyToMem == (u8)TRUE) {
			XPlmi_Printf(DEBUG_GENERAL,
				"+++Copying Image#: 0x%0x, Name: %s, Id: 0x%08x\n\r",
				PdiPtr->ImageNum,
				(char *)PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName,
				PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
		}
	}

	XPlmi_Printf(DEBUG_INFO, "------------------------------------\r\n");
	/* Validate and load the image partitions */
	for (PrtnIndex = 0U;
		PrtnIndex < PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].NoOfPrtns;
		++PrtnIndex) {
		/* Clear NPI errors before loading each partition */
		if (XPlmi_NpiOutOfReset() == (u8)TRUE) {
			XPlmi_ClearNpiErrors();
		}

		if ((PdiPtr->CopyToMem == (u8)FALSE) && (PdiPtr->DelayLoad == (u8)FALSE)) {
			XPlmi_Printf(DEBUG_GENERAL, "---Loading Partition#: 0x%0x, "
					"Id: 0x%0x\r\n", PdiPtr->PrtnNum,
					PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum].PrtnId);
		}
		else {
			if (PdiPtr->DelayLoad == (u8)TRUE) {
				XPlmi_Printf(DEBUG_GENERAL, "---Skipping Partition#: 0x%0x, "
						"Id: 0x%0x\r\n", PdiPtr->PrtnNum,
						PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum].PrtnId);
			}
			if (PdiPtr->CopyToMem == (u8)TRUE) {
				XPlmi_Printf(DEBUG_GENERAL, "---Copying Partition#: 0x%0x, "
						"Id: 0x%0x\r\n", PdiPtr->PrtnNum,
						PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum].PrtnId);
			}
		}

		PrtnLoadTime = XPlmi_GetTimerValue();
		/* Prtn Hdr Validation */
		Status = XLoader_PrtnHdrValidation(
				&(PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum]), PdiPtr->PrtnNum);
		/* PLM is not partition owner and skip this partition */
		if (Status == (int)XLOADER_SUCCESS_NOT_PRTN_OWNER) {
			Status = XST_SUCCESS;
			continue;
		}
		if (XST_SUCCESS != Status) {
			goto END;
		}

		/* Process Partition */
		Status = XLoader_ProcessPrtn(PdiPtr);
		if (XST_SUCCESS != Status) {
			goto END;
		}
		XPlmi_MeasurePerfTime(PrtnLoadTime, &PerfTime);
		XPlmi_Printf(DEBUG_PRINT_PERF,
			" %u.%03u ms for Partition#: 0x%0x, Size: %u Bytes\n\r",
			(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac, PdiPtr->PrtnNum,
			(PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum].TotalDataWordLen) *
			XPLMI_WORD_LEN);

		++PdiPtr->PrtnNum;
		if (XPlmi_NpiOutOfReset() == (u8)TRUE) {
			Status = XPlmi_CheckNpiErrors();
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XPLMI_NPI_ERR, Status);
				goto END1;
			}
		}
	}
	Status = XST_SUCCESS;

END:
	if (Status != XST_SUCCESS) {
		if (XPlmi_NpiOutOfReset() == (u8)TRUE) {
			(void)XPlmi_CheckNpiErrors();
		}
	}
END1:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function validates the partition header.
 *
 * @param	PrtnHdr is pointer to partition header
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	XLOADER_SUCCESS_NOT_PRTN_OWNER if partition is not owned by PLM,
 *			else XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_PrtnHdrValidation(const XilPdi_PrtnHdr * PrtnHdr, u32 PrtnNum)
{
	int Status = (int)XLOADER_SUCCESS_NOT_PRTN_OWNER;

	/* Check if partition belongs to PLM */
	if (XilPdi_GetPrtnOwner(PrtnHdr) != XIH_PH_ATTRB_PRTN_OWNER_PLM) {
		/* If the partition doesn't belong to PLM, skip the partition */
		XPlmi_Printf(DEBUG_GENERAL, "Not owned by PLM,"
				"skipping the Partition# 0x%08x\n\r", PrtnNum);
		goto END;
	}

	/* Validate the fields of partition */
	Status = XilPdi_ValidatePrtnHdr(PrtnHdr);
	if (XST_SUCCESS != Status) {
		goto END;
	}

END:
	/*
	 * Print Prtn Hdr Details
	 */
	XPlmi_Printf(DEBUG_INFO, "UnEncrypted data length: 0x%x\n\r",
				PrtnHdr->UnEncDataWordLen);
	XPlmi_Printf(DEBUG_INFO, "Encrypted data length: 0x%x\n\r",
				PrtnHdr->EncDataWordLen);
	XPlmi_Printf(DEBUG_INFO, "Total data word length: 0x%x\n\r",
				PrtnHdr->TotalDataWordLen);
	XPlmi_Printf(DEBUG_INFO, "Destination load address: 0x%lx\n\r",
				PrtnHdr->DstnLoadAddr);
	XPlmi_Printf(DEBUG_INFO, "Execution address: 0x%lx\n\r",
				PrtnHdr->DstnExecutionAddr);
	XPlmi_Printf(DEBUG_INFO, "Data word offset: 0x%x\n\r",
				PrtnHdr->DataWordOfst);
	XPlmi_Printf(DEBUG_INFO, "Partition attributes: 0x%x\n\r",
				PrtnHdr->PrtnAttrb);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copies partition data to respective target memories.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	DeviceCopy is pointer to the structure variable with parameters
 *			required for copying
 * @param	SecureParams is pointer to the instance containing security related
 *			params
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_PrtnCopy(const XilPdi* PdiPtr, const XLoader_DeviceCopy* DeviceCopy,
		XLoader_SecureParams* SecureParams)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;

	if ((SecureParams->SecureEn == (u8)FALSE) &&
			(SecureParams->SecureEnTmp == (u8)FALSE)) {
		Status = PdiPtr->MetaHdr.DeviceCopy(DeviceCopy->SrcAddr,
			DeviceCopy->DestAddr,DeviceCopy->Len, DeviceCopy->Flags);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "Device Copy Failed \n\r");
			goto END;
		}
	}
	else {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SecureCopy,
					SecureParams, DeviceCopy->DestAddr,
					DeviceCopy->Len);
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
			Status |= StatusTmp;
			XPlmi_Printf(DEBUG_GENERAL, "Device Copy Failed \n\r");
			goto END;
		}
	}

END:
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
static int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams)
{
	int Status = XST_FAILURE;
#ifndef PLM_PM_EXCLUDE
	u32 Mode = 0U;
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
#endif
	u64 Addr = PrtnParams->DeviceCopy.DestAddr;
	u32 Len = PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN;
	u32 DstnCluster = 0U;
	u32 ClusterLockstep = 0U;
	static u8 EccInitDone[2] = {0U};
	u32 DeviceId;

	Status = XPlmi_VerifyAddrRange(Addr, Addr + Len - 1U);
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
#ifndef PLM_PM_EXCLUDE
	if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_PSM) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_PSM_PROC,
			(CapSecureAccess | CapContext), XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_PSM_PROC, 0);
			goto END;
		}
	}
	switch (PrtnParams->DstnCpu)
	{
		case XIH_PH_ATTRB_DSTN_CPU_R52_0:
			if (DstnCluster > XIH_ATTRB_DSTN_CLUSTER_1) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_R52_CLUSTER, 0U);
				goto END;
			}
			DeviceId = PM_DEV_RPU_A_0 + (DstnCluster*2) + XLOADER_RPU_CORE0;
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId,
						IOCTL_SET_RPU_OPER_MODE,
						ClusterLockstep, DstnCluster, NULL,
						XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC,DeviceId,
							(CapSecureAccess | CapContext),XPM_DEF_QOS,0,XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_R52_1:
			if (DstnCluster > XIH_ATTRB_DSTN_CLUSTER_1) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_R52_CLUSTER, 0U);
				goto END;
			}
			DeviceId = PM_DEV_RPU_A_0 + (DstnCluster*2) + XLOADER_RPU_CORE1;
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId,
						IOCTL_SET_RPU_OPER_MODE,
						ClusterLockstep, DstnCluster, NULL,
						XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC,DeviceId,
						(CapSecureAccess | CapContext),XPM_DEF_QOS,0,XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_0:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4);
			Status =  XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId,
								IOCTL_SET_RPU_OPER_MODE,
								ClusterLockstep, DstnCluster, NULL,
								XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_1:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE1;
			Status =  XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId,
							IOCTL_SET_RPU_OPER_MODE,
							ClusterLockstep, DstnCluster, NULL,
								XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_2:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE2;
			Status =  XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId,
							IOCTL_SET_RPU_OPER_MODE,
							ClusterLockstep, DstnCluster, NULL,
							XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A78_3:
			DeviceId = PM_DEV_ACPU_0_0 + (DstnCluster*4) + XLOADER_APU_CORE3;
			Status =  XPm_DevIoctl(PM_SUBSYS_PMC, DeviceId,
							IOCTL_SET_RPU_OPER_MODE,
							ClusterLockstep, DstnCluster, NULL,
							XPLMI_CMD_SECURE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		default:
			Status=XST_SUCCESS;
			break;
	}

	Status = XLoader_GetLoadAddr(PrtnParams->DstnCpu, DstnCluster,
			&PrtnParams->DeviceCopy.DestAddr,
			(PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN));
	if (XST_SUCCESS != Status) {
		goto END;
	}
#else

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_1)) {

		if (DstnCluster > XIH_PH_ATTRB_DSTN_CLUSTER_1) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_R52_CLUSTER, 0U);
			goto END;
		}
		if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_0) {
			XLoader_R52Config(DstnCluster, XLOADER_RPU_CORE0,
				ClusterLockstep, PrtnParams->DeviceCopy.DestAddr);
			if ((EccInitDone[DstnCluster] & XLOADER_R52_0_TCMA_ECC_DONE) !=
				XLOADER_R52_0_TCMA_ECC_DONE) {
				XPlmi_EccInit(XLOADER_R52_0A_TCMA_BASE_ADDR +
					(DstnCluster * XLOADER_R52_TCM_CLUSTER_OFFSET),
					XLOADER_R52_TCM_TOTAL_LENGTH);
				EccInitDone[DstnCluster] |= XLOADER_R52_0_TCMA_ECC_DONE;
			}
		}
		else if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R52_1) {
			XLoader_R52Config(DstnCluster, XLOADER_RPU_CORE1,
				ClusterLockstep, PrtnParams->DeviceCopy.DestAddr);
			if ((EccInitDone[DstnCluster] & XLOADER_R52_1_TCMA_ECC_DONE) !=
				XLOADER_R52_1_TCMA_ECC_DONE) {
				XPlmi_EccInit(XLOADER_R52_1A_TCMA_BASE_ADDR +
					(DstnCluster * XLOADER_R52_TCM_CLUSTER_OFFSET),
					XLOADER_R52_TCM_TOTAL_LENGTH);
				EccInitDone[DstnCluster] |= XLOADER_R52_1_TCMA_ECC_DONE;
			}
		}
		else {
			/* For Misra-C */
		}
		Status = XLoader_GetLoadAddr(PrtnParams->DstnCpu, DstnCluster,
				&PrtnParams->DeviceCopy.DestAddr,
				(PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN));
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}
#endif

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

/****************************************************************************/
/**
 * @brief	This function is used to check whether cpu has handoff address
 * stored in the handoff structure.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	DstnCpu is the cpu which needs to be checked
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
 * @brief	This function is used to process the CDO partition. It copies and
 * validates if security is enabled.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	DeviceCopy is pointer to the structure variable with parameters
 *			required for copying
 * @param	SecureParams is pointer to the instance containing security related
 *			params
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ProcessCdo(const XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
		XLoader_SecureParams* SecureParams)
{
	int Status = XST_FAILURE;
	u32 ChunkLen;
	u32 ChunkLenTemp;
	XPlmiCdo Cdo = {0U};
	u32 PdiVer;
	u32 ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
	u32 ChunkAddrTemp;
	u8 LastChunk = (u8)FALSE;
	u8 IsNextChunkCopyStarted = (u8)FALSE;

	XPlmi_Printf(DEBUG_INFO, "Processing CDO partition \n\r");
	/*
	 * Initialize the Cdo Pointer and
	 * check CDO header contents
	 */
	Status = XPlmi_InitCdo(&Cdo);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INIT_CDO, Status);
		goto END;
	}
#ifndef PLM_PM_EXCLUDE
	Cdo.SubsystemId = XPm_GetSubsystemId(
		PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
#else
	Cdo.SubsystemId = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID;
#endif
	SecureParams->IsCdo = (u8)TRUE;
	PdiVer = PdiPtr->MetaHdr.ImgHdrTbl.Version;

	/*
	 * Process CDO in chunks.
	 * Chunk size is based on the available PRAM size.
	 */
	if (SecureParams->SecureEn == (u8)FALSE) {
		if (DeviceCopy->IsDoubleBuffering == (u8)TRUE) {
			ChunkLen = XLOADER_CHUNK_SIZE / 2U;
		}
		else {
			ChunkLen = XLOADER_CHUNK_SIZE;
		}
	}
	else {
		if ((PdiVer != XLOADER_PDI_VERSION_1) &&
			(PdiVer != XLOADER_PDI_VERSION_2)) {
			ChunkLen = XLOADER_SECURE_CHUNK_SIZE;
		}
		else {
			ChunkLen = XLOADER_CHUNK_SIZE;
		}
	}

	/*
	 * Double buffering for secure cases is possible only
	 * when available PRAM Size >= ChunkLen * 2
	 */
	if ((SecureParams->IsDoubleBuffering == (u8)TRUE) &&
		((ChunkLen * 2U) > XLOADER_CHUNK_SIZE)) {
		/*
		 * Blocking DMA will be used in case
		 * DoubleBuffering is FALSE.
		 */
		SecureParams->IsDoubleBuffering = (u8)FALSE;
	}

	while (DeviceCopy->Len > 0U) {
		/* Update the len for last chunk */
		if (DeviceCopy->Len <= ChunkLen) {
			LastChunk = (u8)TRUE;
			ChunkLen = DeviceCopy->Len;
		}

		if ((SecureParams->SecureEn == (u8)FALSE) &&
			(SecureParams->SecureEnTmp == (u8)FALSE)) {
			if (IsNextChunkCopyStarted == (u8)TRUE) {
				IsNextChunkCopyStarted = (u8)FALSE;
				/* Wait for copy to get completed */
				Status = PdiPtr->DeviceCopy(DeviceCopy->SrcAddr, ChunkAddr, ChunkLen,
					DeviceCopy->Flags | XPLMI_DEVICE_COPY_STATE_WAIT_DONE);
			}
			else {
				/* Copy the data to PRAM buffer */
				Status = PdiPtr->DeviceCopy(DeviceCopy->SrcAddr, ChunkAddr, ChunkLen,
					DeviceCopy->Flags | XPLMI_DEVICE_COPY_STATE_BLK);
			}
			if (Status != XST_SUCCESS) {
					goto END;
			}
			/* Update variables for next chunk */
			Cdo.BufPtr = (u32 *)ChunkAddr;
			Cdo.BufLen = ChunkLen / XIH_PRTN_WORD_LEN;
			DeviceCopy->SrcAddr += ChunkLen;
			DeviceCopy->Len -= ChunkLen;
			if (DeviceCopy->IsDoubleBuffering == (u8)TRUE) {
				Cdo.Cmd.KeyHoleParams.Func = PdiPtr->DeviceCopy;
				Cdo.Cmd.KeyHoleParams.SrcAddr = DeviceCopy->SrcAddr;
			}
			/*
			 * For DDR case, start the copy of the
			 * next chunk for increasing performance
			 */
			if ((DeviceCopy->IsDoubleBuffering == (u8)TRUE)
			    && (LastChunk != (u8)TRUE)) {
				/* Update the next chunk address to other part */
				if (ChunkAddr == XPLMI_PMCRAM_CHUNK_MEMORY) {
					ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY_1;
				}
				else {
					ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
				}

				/* Update the len for last chunk */
				if (DeviceCopy->Len <= ChunkLen) {
					LastChunk = (u8)TRUE;
					ChunkLen = DeviceCopy->Len;
				}
				IsNextChunkCopyStarted = (u8)TRUE;

				/* Initiate the data copy */
				Status = PdiPtr->DeviceCopy(DeviceCopy->SrcAddr, ChunkAddr, ChunkLen,
					DeviceCopy->Flags | XPLMI_DEVICE_COPY_STATE_INITIATE);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
		}
		else {
			SecureParams->RemainingDataLen = DeviceCopy->Len;

			Status = XLoader_ProcessSecurePrtn(SecureParams,
					SecureParams->SecureData, ChunkLen, LastChunk);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			if (SecureParams->IsDoubleBuffering == (u8)TRUE) {
				SecureParams->ChunkAddr = SecureParams->NextChunkAddr;
			}
			Cdo.BufPtr = (u32 *)SecureParams->SecureData;
			Cdo.BufLen = SecureParams->SecureDataLen / XIH_PRTN_WORD_LEN;
			DeviceCopy->SrcAddr += SecureParams->ProcessedLen;
			DeviceCopy->Len -= SecureParams->ProcessedLen;
		}
		/* Process the chunk */
		Status = XPlmi_ProcessCdo(&Cdo);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (Cdo.Cmd.KeyHoleParams.ExtraWords != 0x0U) {
			Cdo.Cmd.KeyHoleParams.ExtraWords *= XPLMI_WORD_LEN;
			DeviceCopy->Len -= Cdo.Cmd.KeyHoleParams.ExtraWords;
			DeviceCopy->SrcAddr += Cdo.Cmd.KeyHoleParams.ExtraWords;
			if ((IsNextChunkCopyStarted == (u8)TRUE) &&
					(Cdo.Cmd.KeyHoleParams.ExtraWords < ChunkLen)) {
				/*
				 * There are some CDO commands to be processed in
				 * memory pointed to by ChunkAddr
				 */
				ChunkAddrTemp = (ChunkAddr + Cdo.Cmd.KeyHoleParams.ExtraWords);
				ChunkLenTemp = (ChunkLen - Cdo.Cmd.KeyHoleParams.ExtraWords);
				if (ChunkAddr == XPLMI_PMCRAM_CHUNK_MEMORY) {
					ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY_1;
				}
				else {
					ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
				}
				Status = XPlmi_DmaXfr(ChunkAddrTemp, ChunkAddr,
						(ChunkLenTemp / XPLMI_WORD_LEN), XPLMI_PMCDMA_0);
				if (Status != XST_SUCCESS) {
					goto END;
				}

				if (Cdo.Cmd.KeyHoleParams.ExtraWords < (DeviceCopy->Len - ChunkLenTemp)) {
					Status = PdiPtr->DeviceCopy(DeviceCopy->SrcAddr + ChunkLenTemp,
							ChunkAddr + ChunkLenTemp, Cdo.Cmd.KeyHoleParams.ExtraWords,
							DeviceCopy->Flags | XPLMI_DEVICE_COPY_STATE_INITIATE);
				}
				else {
					Status = PdiPtr->DeviceCopy(DeviceCopy->SrcAddr + ChunkLenTemp,
							ChunkAddr + ChunkLenTemp, (DeviceCopy->Len - ChunkLenTemp),
							DeviceCopy->Flags | XPLMI_DEVICE_COPY_STATE_INITIATE);
				}
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
			else {
				IsNextChunkCopyStarted = (u8)FALSE;
				SecureParams->IsNextChunkCopyStarted = (u8)FALSE;
			}
			Cdo.Cmd.KeyHoleParams.ExtraWords = 0x0U;
		}
	}
	/* If deferred error, flagging it after CDO process complete */
	if (Cdo.DeferredError == (u8)TRUE) {
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_DEFERRED_CDO_PROCESS, 0);
		goto END;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function is used to process the partition.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ProcessPrtn(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	PdiSrc_t PdiSrc = PdiPtr->PdiSrc;
	int (*DevCopy) (u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags) = NULL;
	XLoader_SecureParams SecureParams = {0U};
	XLoader_PrtnParams PrtnParams = {0U};
	u32 PrtnType;
	u64 OfstAddr = 0U;
	u32 TrfLen;
	u32 TempVal;
	u32 PrtnNum = PdiPtr->PrtnNum;
	u8 ToStoreInDdr = (u8)FALSE;
	u8 PdiType;
	/* Assign the partition header to local variable */
	const XilPdi_PrtnHdr * PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Update current Processing partition ID */
	PdiPtr->CurPrtnId = PrtnHdr->PrtnId;
	/* Read Partition Type */
	PrtnType = XilPdi_GetPrtnType(PrtnHdr);

	PrtnParams.DeviceCopy.DestAddr = PrtnHdr->DstnLoadAddr;
	PrtnParams.DeviceCopy.Len = (PrtnHdr->TotalDataWordLen * XIH_PRTN_WORD_LEN);

	if (PdiPtr->PdiType != XLOADER_PDI_TYPE_RESTORE) {
		PrtnParams.DeviceCopy.SrcAddr = PdiPtr->MetaHdr.FlashOfstAddr +
			((u64)PrtnHdr->DataWordOfst * XIH_PRTN_WORD_LEN);
	}

	if (PdiPtr->CopyToMem == (u8)TRUE) {
		Status = XLoader_SecureInit(&SecureParams, PdiPtr, PrtnNum);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = PdiPtr->DeviceCopy(PrtnParams.DeviceCopy.SrcAddr,
					PdiPtr->CopyToMemAddr, PrtnParams.DeviceCopy.Len -
					SecureParams.SecureHdrLen, PrtnParams.DeviceCopy.Flags);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_COPY_TO_MEM, 0);
			goto END;
		}

		if (PdiPtr->DelayLoad == (u8)TRUE) {
			PdiPtr->CopyToMemAddr +=
					((u64)(PrtnParams.DeviceCopy.Len) - SecureParams.SecureHdrLen);
			goto END;
		}

		PdiSrc = PdiPtr->PdiSrc;
		PdiPtr->PdiSrc = XLOADER_PDI_SRC_DDR;
		DevCopy = PdiPtr->DeviceCopy;
		PdiPtr->DeviceCopy = XLoader_DdrCopy;
		PdiPtr->MetaHdr.DeviceCopy = XLoader_DdrCopy;
		OfstAddr = PdiPtr->MetaHdr.FlashOfstAddr;
		PrtnParams.DeviceCopy.Flags = XPLMI_PMCDMA_0;
		ToStoreInDdr = (u8)TRUE;
		PdiPtr->CopyToMemAddr -= SecureParams.SecureHdrLen;
		PdiPtr->CopyToMem = (u8)FALSE;
		PdiType = PdiPtr->PdiType;
		PdiPtr->PdiType = XLOADER_PDI_TYPE_RESTORE;
	}
	else if (PdiPtr->DelayLoad == (u8)TRUE) {
		if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_JTAG) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SBI) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SMAP) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_PCIE)) {
			while (PrtnParams.DeviceCopy.Len > 0U) {
				if (PrtnParams.DeviceCopy.Len > XLOADER_CHUNK_SIZE) {
					TrfLen = XLOADER_CHUNK_SIZE;
				}
				else {
					TrfLen = PrtnParams.DeviceCopy.Len;
				}
				Status = PdiPtr->DeviceCopy(PrtnParams.DeviceCopy.SrcAddr,
					XPLMI_PMCRAM_CHUNK_MEMORY, TrfLen, 0U);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(XLOADER_ERR_DELAY_LOAD, Status);
					goto END;
				}
				PrtnParams.DeviceCopy.Len = PrtnParams.DeviceCopy.Len - TrfLen;
				PrtnParams.DeviceCopy.SrcAddr = PrtnParams.DeviceCopy.SrcAddr + TrfLen;
			}
		}
		Status = XST_SUCCESS;
		goto END;
	}
	else {
		/*
		 * MISRA-C compliance
		 */
	}

	Status = XLoader_SecureInit(&SecureParams, PdiPtr, PrtnNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_RESTORE) {
		PrtnParams.DeviceCopy.SrcAddr = PdiPtr->CopyToMemAddr;
		OfstAddr = PdiPtr->MetaHdr.FlashOfstAddr;
		PdiPtr->MetaHdr.FlashOfstAddr = PdiPtr->CopyToMemAddr -
				((u64)PrtnHdr->DataWordOfst * XIH_PRTN_WORD_LEN);
		PdiPtr->CopyToMemAddr += ((u64)PrtnParams.DeviceCopy.Len - SecureParams.SecureHdrLen);
	}

	if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_DDR) ||
		(PdiPtr->PdiSrc == XLOADER_PDI_SRC_OSPI) ||
		(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI24) ||
		(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32) ||
		(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SBI) ||
		(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SMAP) ||
		(PdiPtr->PdiSrc == XLOADER_PDI_SRC_JTAG) ||
		(PdiPtr->PdiSrc == XLOADER_PDI_SRC_PCIE)) {
		PrtnParams.DeviceCopy.IsDoubleBuffering = (u8)TRUE;
	}

	SecureParams.IsDoubleBuffering = PrtnParams.DeviceCopy.IsDoubleBuffering;

	/*
	 * ProcessCdo, ProcessElf and PrtnCopy APIs expected unencrypted
	 * length that is 16 byte aligned
	 */
	/*
	 * Make unencrypted length 16 byte aligned.
	 */
	TempVal = PrtnParams.DeviceCopy.Len % XLOADER_DMA_LEN_ALIGN;
	if (TempVal != 0U) {
		PrtnParams.DeviceCopy.Len += (XLOADER_DMA_LEN_ALIGN - TempVal);
	}

	/* To make sure total data length passed is without authentication
	 * certificate size when authentication is enabled.
	 */
	PrtnParams.DeviceCopy.Len -= SecureParams.ProcessedLen;

	if (PrtnType == XIH_PH_ATTRB_PRTN_TYPE_CDO) {
		Status = XLoader_ProcessCdo(PdiPtr, &PrtnParams.DeviceCopy, &SecureParams);
	}
	else if (PrtnType == XIH_PH_ATTRB_PRTN_TYPE_ELF) {
		XPlmi_Printf(DEBUG_INFO, "Copying elf partitions\n\r");
		Status = XLoader_ProcessElf(PdiPtr, PrtnHdr, &PrtnParams, &SecureParams);
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "Copying data partition\n\r");
		/* Partition Copy */
		Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams.DeviceCopy, &SecureParams);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	if (ToStoreInDdr == (u8)TRUE) {
		PdiPtr->PdiSrc = PdiSrc;
		PdiPtr->DeviceCopy = DevCopy;
		PdiPtr->MetaHdr.DeviceCopy = DevCopy;
		PdiPtr->MetaHdr.FlashOfstAddr = OfstAddr;
		PdiPtr->CopyToMem = (u8)TRUE;
		PdiPtr->PdiType = PdiType;
	}
	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_RESTORE) {
		PdiPtr->MetaHdr.FlashOfstAddr = OfstAddr;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the load address based on the
 * destination CPU.
 *
 * @param	DstnCpu is destination CPU
 * @param	LoadAddrPtr is the destination load address pointer
 * @param	Len is the length of the partition
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_GetLoadAddr(u32 DstnCpu, u32 DstnCluster, u64 *LoadAddrPtr,
		u32 Len)
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
 * @brief	This function initializes and configures the APU Cluster
 *
 * @param	ClusterNum is the APU Cluster number
 * @param	LockstepVal is the Lockstep value for given APU Cluster
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_A78Config(u8 ClusterNum, u8 CoreNum, u64 HandoffAddr,
		u32 LockstepVal)
{
	int Status = XST_FAILURE;
	static u8 ApuClusterState[4U] = {0U};
	u8 LockstepEn = XLOADER_APU_CLUSTER_LOCKSTEP_DISABLE;
	u32 LowAddress = (u32)(HandoffAddr & 0xFFFFFFFCUL);
	u32 HighAddress = (u32)(HandoffAddr >> 32UL);
	u32 PcliCoreNum = (ClusterNum * 4U) + CoreNum;
	u32 RstApuOffset = PSX_CRF_RST_APU0 + (ClusterNum * RST_APU_REG_OFFSET);
	u32 ApuCoreOffset = (CoreNum * 8U);

	/* Skip cluster configuration if cluster is already configured */
	if (ApuClusterState[ClusterNum] == XLOADER_A78_CLUSTER_CONFIGURED) {
		Status = XST_SUCCESS;
		goto END1;
	}
	if (LockstepVal != XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED) {
		LockstepEn = XLOADER_APU_CLUSTER_LOCKSTEP_ENABLE;
	}
	/* Programming cluster id aff2, aff3 */
	XPlmi_Out32(GET_APU_CLUSTER_REG(ClusterNum, APU_CLUSTER_CONFIG0_OFFSET),
			ClusterNum);
	/* APU PSTATE, PREQ configuration */
	XPlmi_UtilRMW(GET_APU_PCLI_CLUSTER_REG(ClusterNum,
		APU_PCLI_CLUSTER_PSTATE_OFFSET),
		APU_PCLI_CLUSTER_PSTATE_PSTATE_MASK,
		APU_CLUSTER_PSTATE_FULL_ON_VAL);
	XPlmi_UtilRMW(GET_APU_PCLI_CLUSTER_REG(ClusterNum,
		APU_PCLI_CLUSTER_PREQ_OFFSET), APU_PCLI_CLUSTER_PREQ_PREQ_MASK,
		APU_PCLI_CLUSTER_PREQ_PREQ_MASK);
	/* Configure lockstep value for cluster */
	XPlmi_UtilRMW(FPX_SLCR_APU_CTRL, (u32)(1U << ClusterNum),
			(u32)(LockstepEn << ClusterNum));
	/* ACPU clock config */
	XPlmi_UtilRMW(PSX_CRF_ACPU0_CLK_CTRL +
			(ClusterNum * ACPU_CLK_CTRL_REG_OFFSET),
			ACPU0_CLK_CTRL_CLKACT_MASK, ACPU0_CLK_CTRL_CLKACT_VAL);
	/* APU cluster release cold & warm reset */
	XPlmi_UtilRMW(RstApuOffset, (APU_CLUSTER_WARM_RESET_MASK |
		APU_CLUSTER_COLD_RESET_MASK), 0U);

	Status = XPlmi_UtilPollForMask(GET_APU_PCLI_CLUSTER_REG(ClusterNum,
		APU_PCLI_CLUSTER_PACTIVE_OFFSET),
		APU_PCLI_CLUSTER_PACTIVE_PACCEPT_MASK, 1000U);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "A78 Cluster PACCEPT timeout..\n");
		goto END;
	}
	ApuClusterState[ClusterNum] = XLOADER_A78_CLUSTER_CONFIGURED;

END1:
	XPlmi_Out32(GET_APU_CLUSTER_REG(ClusterNum,
		APU_CLUSTER_RVBARADDR0L_OFFSET) + ApuCoreOffset, LowAddress);
	XPlmi_Out32(GET_APU_CLUSTER_REG(ClusterNum,
		APU_CLUSTER_RVBARADDR0H_OFFSET) + ApuCoreOffset, HighAddress);

	XPlmi_UtilRMW(GET_APU_PCLI_CORE_REG(PcliCoreNum, APU_PCLI_CORE_PSTATE_OFFSET),
		APU_PCLI_CORE_PSTATE_PSTATE_MASK, APU_CORE_PSTATE_FULL_ON_VAL);
	XPlmi_UtilRMW(GET_APU_PCLI_CORE_REG(PcliCoreNum, APU_PCLI_CORE_PREQ_OFFSET),
		APU_PCLI_CORE_PREQ_PREQ_MASK, APU_PCLI_CORE_PREQ_PREQ_MASK);

	/* APU core release warm reset */
	XPlmi_UtilRMW(RstApuOffset,
		(u32)(1U << (CoreNum + APU_CORE_WARM_RESET_SHIFT)), 0U);
	Status = XPlmi_UtilPollForMask(GET_APU_PCLI_CORE_REG(PcliCoreNum,
			APU_PCLI_CORE_PACTIVE_OFFSET),
			APU_PCLI_CORE_PACTIVE_PACCEPT_MASK, 1000U);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "A78 core PACCEPT timeout..\n\r");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes and configures the RPU Core
 *
 * @param	ClusterNum is the RPU Cluster number
 * @param	CoreNum is the RPU Core number
 * @param	HandoffAddr is the reset vector base address for the core
 * @param	RstRpuMask is the pointer to the mask value used for RST_RPU
 *
 *****************************************************************************/
static void XLoader_RpuCoreConfig(u8 ClusterNum, u8 CoreNum, u64 HandoffAddr,
		u32 *RstRpuMask)
{
	u32 RpuCfg0Addr = GET_RPU_CLUSTER_CORE_REG(ClusterNum, CoreNum,
				RPU_CLUSTER_CORE_CFG0_OFFSET);
	u32 VecTableAddr = GET_RPU_CLUSTER_CORE_REG(ClusterNum, CoreNum,
				RPU_CLUSTER_CORE_VECTABLE_OFFSET);
	u32 Address = (u32)(HandoffAddr & 0xFFFFFFE0U);

	/* Disable address remap */
	XPlmi_UtilRMW(RpuCfg0Addr, RPU_CLUSTER_CORE_CFG0_REMAP_MASK, 0U);
	XPlmi_UtilRMW(RpuCfg0Addr, RPU_CLUSTER_CORE_CFG0_CPU_HALT_MASK, 1U);
	/* Configure Vector Table Address */
	XPlmi_UtilRMW(VecTableAddr, RPU_CLUSTER_CORE_VECTABLE_MASK,
				Address);
	*RstRpuMask |= GetRpuRstMask(RPU_CORE0A_POR_MASK, ClusterNum, CoreNum) |
			GetRpuRstMask(RPU_CORE0A_RESET_MASK, ClusterNum, CoreNum);
}

/*****************************************************************************/
/**
 * @brief	This function initializes and configures the RPU Core
 *
 * @param	ClusterNum is the RPU Cluster number
 * @param	CoreNum is the RPU Core number
 * @param	LockstepVal is value of lockstep field in PrtnHdr
 * @param	HandoffAddr is the reset vector base address for the core
 *
 *****************************************************************************/
static void XLoader_R52Config(u8 ClusterNum, u8 CoreNum, u32 LockstepVal,
		u64 HandoffAddr)
{
	u32 RstRpuMask = 0U;
	static u8 RpuClusterState[2U] = {0U};
	u32 LockstepEn = XLOADER_RPU_CLUSTER_LOCKSTEP_DISABLE;

	/* Skip cluster configuration if cluster is already configured */
	if (RpuClusterState[ClusterNum] == XLOADER_R52_CLUSTER_CONFIGURED) {
		goto END;
	}
	RstRpuMask = (RPU_A_TOPRESET_MASK | RPU_A_DBGRST_MASK) << ClusterNum;
	if (LockstepVal != XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED) {
		RstRpuMask |= RPU_A_DCLS_TOPRESET_MASK << ClusterNum;
		LockstepEn = XLOADER_RPU_CLUSTER_LOCKSTEP_ENABLE;
	}
	/* Lockstep configuration */
	XPlmi_UtilRMW(GET_RPU_CLUSTER_REG(ClusterNum, RPU_CLUSTER_CFG_OFFSET),
		RPU_CLUSTER_CFG_SLSPLIT_MASK, LockstepEn);
	XPlmi_UtilRMW(CRL_RST_RPU_ADDR, RstRpuMask, 0U);
	RpuClusterState[ClusterNum] = XLOADER_R52_CLUSTER_CONFIGURED;

END:
	/* Configure Lockstep and bring cluster, cores out of reset */
	if (LockstepVal == XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED) {
		XLoader_RpuCoreConfig(ClusterNum, CoreNum, HandoffAddr, &RstRpuMask);
	}
	else {
		XLoader_RpuCoreConfig(ClusterNum, XLOADER_RPU_CORE0, HandoffAddr,
				&RstRpuMask);
		XLoader_RpuCoreConfig(ClusterNum, XLOADER_RPU_CORE1, HandoffAddr,
				&RstRpuMask);
	}
	XPlmi_UtilRMW(CRL_RST_RPU_ADDR, RstRpuMask, 0U);
}
