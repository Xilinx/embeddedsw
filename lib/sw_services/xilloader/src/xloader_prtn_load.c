/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_prtn_load.c
*
* This is the file which contains partition load code for the Platform
* loader..
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
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpm_nodeid.h"
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_dma.h"
#include "xplmi_debug.h"
#include "xplmi_cdo.h"
#include "xpm_api.h"
#include "xplmi_util.h"
#include "xloader_secure.h"
#include "xloader_ddr.h"
#include "xplmi.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_SUCCESS_NOT_PRTN_OWNER	(0x100U)

/************************** Function Prototypes ******************************/
static int XLoader_PrtnHdrValidation(XilPdi_PrtnHdr* PrtnHdr, u32 PrtnNum);
static int XLoader_ProcessPrtn(XilPdi* PdiPtr);
static int XLoader_PrtnCopy(XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
	XLoader_SecureParams* SecureParams);
static int XLoader_CheckHandoffCpu (XilPdi* PdiPtr, u32 DstnCpu);
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len);
static int XLoader_ProcessCdo (XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
	XLoader_SecureParams* SecureParams);
static int XLoader_ProcessElf(XilPdi* PdiPtr, XilPdi_PrtnHdr* PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams);

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
			"+++++++Loading Image No: 0x%0x, Name: %s, Id: 0x%08x\n\r",
			PdiPtr->ImageNum,
			(char *)PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName,
			PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
	}
	else {
		if (PdiPtr->DelayLoad == (u8)TRUE) {
			XPlmi_Printf(DEBUG_GENERAL,
				"+++++++Skipping Image No: 0x%0x, Name: %s, Id: 0x%08x\n\r",
				PdiPtr->ImageNum,
				(char *)PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName,
				PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
		}
		if (PdiPtr->CopyToMem == (u8)TRUE) {
			XPlmi_Printf(DEBUG_GENERAL,
				"+++++++Copying Image No: 0x%0x, Name: %s, Id: 0x%08x\n\r",
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
		if ((PdiPtr->CopyToMem == (u8)FALSE) && (PdiPtr->DelayLoad == (u8)FALSE)) {
			XPlmi_Printf(DEBUG_GENERAL, "-------Loading Prtn No: 0x%0x\r\n",
				PdiPtr->PrtnNum);
		}
		else {
			if (PdiPtr->DelayLoad == (u8)TRUE) {
				XPlmi_Printf(DEBUG_GENERAL, "-------Skipping Prtn No: 0x%0x\r\n",
					PdiPtr->PrtnNum);
			}
			if (PdiPtr->CopyToMem == (u8)TRUE) {
				XPlmi_Printf(DEBUG_GENERAL, "-------Copying Prtn No: 0x%0x\r\n",
					PdiPtr->PrtnNum);
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
		else if (XST_SUCCESS != Status) {
			goto END;
		}
		else {
			/* For MISRA C compliance */
		}

		/* Process Partition */
		Status = XLoader_ProcessPrtn(PdiPtr);
		if (XST_SUCCESS != Status) {
			goto END;
		}
		XPlmi_MeasurePerfTime(PrtnLoadTime, &PerfTime);
		XPlmi_Printf(DEBUG_PRINT_PERF,
			" %u.%06u ms for PrtnNum: %u, Size: %u Bytes\n\r",
			(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac, PdiPtr->PrtnNum,
			(PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum].TotalDataWordLen) *
			XPLMI_WORD_LEN);

		++PdiPtr->PrtnNum;
	}
	Status = XST_SUCCESS;

END:
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
static int XLoader_PrtnHdrValidation(XilPdi_PrtnHdr * PrtnHdr, u32 PrtnNum)
{
	int Status = (int)XLOADER_SUCCESS_NOT_PRTN_OWNER;

	/* Check if partition belongs to PLM */
	if (XilPdi_GetPrtnOwner(PrtnHdr) != XIH_PH_ATTRB_PRTN_OWNER_PLM) {
		/* If the partition doesn't belong to PLM, skip the partition */
		XPlmi_Printf(DEBUG_GENERAL, "Not owned by PLM,"
				"skipping the Prtn 0x%08x\n\r", PrtnNum);
		goto END;
	}

	/* Validate the fields of partition */
	Status = XilPdi_ValidatePrtnHdr(PrtnHdr);
	if (XST_SUCCESS != Status) {
		goto END;
	}

END:
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
static int XLoader_PrtnCopy(XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
		XLoader_SecureParams* SecureParams)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;

	if ((SecureParams->SecureEn == (u8)FALSE) &&
			(SecureParams->SecureEnTmp == (u8)FALSE)) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, PdiPtr->MetaHdr.DeviceCopy,
					DeviceCopy->SrcAddr, DeviceCopy->DestAddr,
					DeviceCopy->Len, DeviceCopy->Flags);
	}
	else {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SecureCopy,
					SecureParams, DeviceCopy->DestAddr,
					DeviceCopy->Len);
	}
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		Status = Status | StatusTmp;
		XPlmi_Printf(DEBUG_GENERAL, "Device Copy Failed \n\r");
		goto END;
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
static int XLoader_ProcessElf(XilPdi* PdiPtr, XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams)
{
	int Status = XST_FAILURE;
	u32 Mode = 0U;
	u32 Pm_CapAccess = (u32)PM_CAP_ACCESS;
	u32 Pm_CapContext = (u32)PM_CAP_CONTEXT;

	PrtnParams->DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

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
			Pm_CapAccess | Pm_CapContext, XPM_DEF_QOS, 0U);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_PSM_PROC, 0);
			goto END;
		}
	}

	/* Check if R5 App memory is TCM, Copy to global TCM memory MAP */
	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L)) {
		if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1,
					IOCTL_SET_RPU_OPER_MODE,
					XPM_RPU_MODE_SPLIT, 0U, &Mode);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_IOCTL_RPU1_SPLIT, 0);
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_1_A,
					Pm_CapAccess | Pm_CapContext,
					XPM_DEF_QOS,0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_TCM_1_A, 0);
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_1_B,
					Pm_CapAccess | Pm_CapContext,
					XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_TCM_1_B, 0);
				goto END;
			}
		}
		else if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0,
				IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_SPLIT, 0U, &Mode);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_IOCTL_RPU0_SPLIT, 0);
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_0_A,
					Pm_CapAccess | Pm_CapContext,
					XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_TCM_0_A, 0);
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_0_B,
					Pm_CapAccess | Pm_CapContext,
					XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_TCM_0_B, 0);
				goto END;
			}
		}
		else {
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0,
				IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_LOCKSTEP, 0U, &Mode);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_IOCTL_RPU0_LOCKSTEP, 0);
				goto END;
			}
			Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1,
				IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_LOCKSTEP, 0U, &Mode);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_IOCTL_RPU1_LOCKSTEP, 0);
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_0_A,
					Pm_CapAccess | Pm_CapContext,
					XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_TCM_0_A, 0);
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_0_B,
					Pm_CapAccess | Pm_CapContext,
					XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_TCM_0_B, 0);
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_1_A,
					Pm_CapAccess | Pm_CapContext,
					XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_TCM_1_A, 0);
				goto END;
			}
			Status = XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_1_B,
					Pm_CapAccess | Pm_CapContext,
					XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_TCM_1_B, 0);
				goto END;
			}
		}

		Status = XLoader_GetLoadAddr(PrtnParams->DstnCpu,
					&PrtnParams->DeviceCopy.DestAddr,
					(PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN));
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}

	Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams->DeviceCopy, SecureParams);
	if (XST_SUCCESS != Status) {
			goto END;
	}

	if ((PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A72_0) ||
		(PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_A72_1)) {
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
	u32 DstnCpu;
	u32 CpuNo;
	u32 PrtnNum = PdiPtr->PrtnNum;
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) &&
	    (DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_PSM)) {
		CpuNo = PdiPtr->NoOfHandoffCpus;
		if (XLoader_CheckHandoffCpu(PdiPtr, DstnCpu) == XST_SUCCESS) {
			if (CpuNo >= XLOADER_MAX_HANDOFF_CPUS) {
				Status = XPlmi_UpdateStatus(
							XLOADER_ERR_NUM_HANDOFF_CPUS, 0);
				goto END;
			}
			/* Update the CPU settings */
			PdiPtr->HandoffParam[CpuNo].CpuSettings =
				XilPdi_GetDstnCpu(PrtnHdr) |
				XilPdi_GetA72ExecState(PrtnHdr) |
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
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_CheckHandoffCpu (XilPdi* PdiPtr, u32 DstnCpu)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;

	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;
		if (CpuId == DstnCpu) {
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
static int XLoader_ProcessCdo(XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
		XLoader_SecureParams* SecureParams)
{
	int Status = XST_FAILURE;
	u32 ChunkLen;
	XPlmiCdo Cdo = {0U};
	u32 PdiVer;
	u32 ChunkAddr = XPLMI_LOADER_CHUNK_MEMORY;
	u8 LastChunk = (u8)FALSE;
	u8 IsNextChunkCopyStarted = (u8)FALSE;

	XPlmi_Printf(DEBUG_INFO, "Processing CDO partition \n\r");
	/*
	 * Initialize the Cdo Pointer and
	 * check CDO header contents
	 */
	XPlmi_InitCdo(&Cdo);
	Cdo.ImgId = PdiPtr->CurImgId;
	Cdo.PrtnId = PdiPtr->CurPrtnId;
	Cdo.IpiMask = PdiPtr->IpiMask;

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
		((ChunkLen * 2U) <= XLOADER_CHUNK_SIZE)) {
		/*
		 * Do nothing
		 */
	}
	else {
		/*
		 * Blocking DMA will be used in case
		 * DoubleBuffering is FALSE.
		 */
		SecureParams->IsDoubleBuffering = (u8)FALSE;
	}

	SecureParams->IsCdo = (u8)TRUE;
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
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
			else {
				/* Copy the data to PRAM buffer */
				Status = PdiPtr->DeviceCopy(DeviceCopy->SrcAddr, ChunkAddr, ChunkLen,
					DeviceCopy->Flags | XPLMI_DEVICE_COPY_STATE_BLK);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
			/* Update variables for next chunk */
			Cdo.BufPtr = (u32 *)ChunkAddr;
			Cdo.BufLen = ChunkLen / XIH_PRTN_WORD_LEN;
			DeviceCopy->SrcAddr += ChunkLen;
			DeviceCopy->Len -= ChunkLen;

			if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI24) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_OSPI) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SMAP) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_JTAG) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SBI)) {
				Cdo.Cmd.KeyHoleParams.PdiSrc = (u16)(PdiPtr->PdiSrc);
				Cdo.Cmd.KeyHoleParams.SrcAddr = DeviceCopy->SrcAddr;
				Cdo.Cmd.KeyHoleParams.Func = PdiPtr->DeviceCopy;
			}
			else if(PdiPtr->PdiSrc == XLOADER_PDI_SRC_DDR) {
				Cdo.Cmd.KeyHoleParams.PdiSrc = (u16)(PdiPtr->PdiSrc);
				Cdo.Cmd.KeyHoleParams.SrcAddr = DeviceCopy->SrcAddr;
			}
			else {
				/* MISRA-C compliance */
			}

			if((PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI24) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_OSPI) ||
				(PdiPtr->SlrType == XLOADER_SSIT_MASTER_SLR)) {
				Cdo.Cmd.KeyHoleParams.InChunkCopy = (u8)TRUE;
				Cdo.Cmd.KeyHoleParams.IsDoubleBuffering = DeviceCopy->IsDoubleBuffering;
			}
			/*
			 * For DDR case, start the copy of the
			 * next chunk for increasing performance
			 */
			if ((DeviceCopy->IsDoubleBuffering == (u8)TRUE)
			    && (LastChunk != (u8)TRUE)) {
				/* Update the next chunk address to other part */
				if (ChunkAddr == XPLMI_LOADER_CHUNK_MEMORY) {
					ChunkAddr = XPLMI_LOADER_CHUNK_MEMORY_1;
				}
				else {
					ChunkAddr = XPLMI_LOADER_CHUNK_MEMORY;
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
			Status = XLoader_ProcessSecurePrtn(SecureParams,
					SecureParams->SecureData, ChunkLen, LastChunk);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Cdo.BufPtr = (u32 *)SecureParams->SecureData;
			Cdo.BufLen = SecureParams->SecureDataLen / XIH_PRTN_WORD_LEN;
			DeviceCopy->SrcAddr += SecureParams->ProcessedLen;
			DeviceCopy->Len -= SecureParams->ProcessedLen;

			if ((SecureParams->IsDoubleBuffering == (u8)TRUE) &&
						(LastChunk != (u8)TRUE)) {
				Status = XLoader_StartNextChunkCopy(
						SecureParams, DeviceCopy->Len, ChunkLen);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
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
			IsNextChunkCopyStarted = (u8)FALSE;
			SecureParams->IsNextChunkCopyStarted = (u8)FALSE;
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
	XilPdi_PrtnHdr * PrtnHdr;
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
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
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
							XPLMI_LOADER_CHUNK_MEMORY, TrfLen, 0U);
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
		(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32)) {
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
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len)
{
	int Status = XST_FAILURE;
	u64 Address = *LoadAddrPtr;

	if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) &&
			((Address < (XLOADER_R5_TCMA_LOAD_ADDRESS +
					XLOADER_R5_TCM_BANK_LENGTH)) ||
			((Address >= XLOADER_R5_TCMB_LOAD_ADDRESS) &&
			(Address < (XLOADER_R5_TCMB_LOAD_ADDRESS +
					XLOADER_R5_TCM_BANK_LENGTH))))) {
		if (Len > XLOADER_R5_TCM_BANK_LENGTH) {
			Status = XPlmi_UpdateStatus(
						XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
			goto END;
		}

		Address += XLOADER_R5_0_TCMA_BASE_ADDR;
	}
	else if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) &&
			((Address < (XLOADER_R5_TCMA_LOAD_ADDRESS +
						XLOADER_R5_TCM_BANK_LENGTH)) ||
			((Address >= XLOADER_R5_TCMB_LOAD_ADDRESS) &&
			(Address < (XLOADER_R5_TCMB_LOAD_ADDRESS +
					XLOADER_R5_TCM_BANK_LENGTH))))) {
		if (Len > XLOADER_R5_TCM_BANK_LENGTH) {
			Status = XPlmi_UpdateStatus(
						XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
			goto END;
		}

		Address += XLOADER_R5_1_TCMA_BASE_ADDR;
	}
	else if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) &&
			(Address < (XLOADER_R5_TCM_BANK_LENGTH * 4U))) {
		if (Len > (XLOADER_R5_TCM_BANK_LENGTH * 4U)) {
			Status = XPlmi_UpdateStatus(
						XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
			goto END;
		}

		Address += XLOADER_R5_0_TCMA_BASE_ADDR;
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
