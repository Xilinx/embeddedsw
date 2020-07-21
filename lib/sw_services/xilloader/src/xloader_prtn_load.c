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
*
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

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_SUCCESS_NOT_PRTN_OWNER	(0x100U)

/************************** Function Prototypes ******************************/
static int XLoader_PrtnHdrValidation(XilPdi* PdiPtr, u32 PrtnNum);
static int XLoader_ProcessPrtn(XilPdi* PdiPtr, u32 PrtnNum);
static int XLoader_PrtnCopy(XilPdi* PdiPtr, u32 PrtnNum);
static int XLoader_CheckHandoffCpu (XilPdi* PdiPtr, u32 DstnCpu);
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function loads the partition.
 *
 * @param	PdiPtr is pointer to XilPdi Instance
 * @param	ImgNum is the image number to be loaded
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_LoadImagePrtns(XilPdi* PdiPtr, u32 ImgNum, u32 PrtnNum)
{
	int Status = XST_FAILURE;
	u32 PrtnIndex;
	u64 PrtnLoadTime;
	XPlmi_PerfTime PerfTime = {0U};

	/* Validate and load the image partitions */
	for (PrtnIndex = 0U; PrtnIndex < PdiPtr->MetaHdr.ImgHdr[ImgNum].NoOfPrtns;
		PrtnIndex++) {

		XPlmi_Printf(DEBUG_GENERAL, "-------Loading Prtn No: 0x%0x\r\n",
			     PrtnNum);

		PrtnLoadTime = XPlmi_GetTimerValue();
		/* Prtn Hdr Validation */
		Status = XLoader_PrtnHdrValidation(PdiPtr, PrtnNum);

		/* PLM is not partition owner and skip this partition */
		if (Status == XLOADER_SUCCESS_NOT_PRTN_OWNER) {
			Status = XST_SUCCESS;
			goto END;
		}
		else if (XST_SUCCESS != Status) {
			goto END;
		}
		else {
			/* For MISRA C compliance */
		}

		/* Process Partition */
		Status = XLoader_ProcessPrtn(PdiPtr, PrtnNum);
		if (XST_SUCCESS != Status) {
			goto END;
		}
		XPlmi_MeasurePerfTime(PrtnLoadTime, &PerfTime);
		XPlmi_Printf(DEBUG_PRINT_PERF,
			" %u.%u ms for PrtnNum: %u, Size: %u Bytes\n\r",
			(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac, PrtnNum,
			(PdiPtr->MetaHdr.PrtnHdr[PrtnNum].TotalDataWordLen) *
			XPLMI_WORD_LEN);
		PrtnNum++;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function validates the partition header.
 *
 * @param	PdiPtr is pointer to XilPdi Instance
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_PrtnHdrValidation(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status = XST_FAILURE;
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Check if partition belongs to PLM */
	if (XilPdi_GetPrtnOwner(PrtnHdr) != XIH_PH_ATTRB_PRTN_OWNER_PLM) {
		/* If the partition doesn't belong to PLM, skip the partition */
		XPlmi_Printf(DEBUG_GENERAL, "Skipping the Prtn 0x%08x\n\r",
			PrtnNum);
		Status = XLOADER_SUCCESS_NOT_PRTN_OWNER;
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
 * @brief	This function copies the partition to specified destination.
 *
 * @param	PdiPtr is pointer to XilPdi Instance
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_PrtnCopy(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status = XST_FAILURE;
	u64 SrcAddr;
	u64 DestAddr;
	u32 DstnCpu;
	u32 Len;
	XilPdi_PrtnHdr * PrtnHdr;
	XLoader_SecureParms SecureParams = {0U};
	u32 Mode = 0U;
	u32 PrtnType;
	u32 TempVal;

	Status = XLoader_SecureInit(&SecureParams, PdiPtr, PrtnNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	SrcAddr = PdiPtr->MetaHdr.FlashOfstAddr +
		((PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	DestAddr = PrtnHdr->DstnLoadAddr;

	/* For Non-secure image */
	Len = (PrtnHdr->UnEncDataWordLen) * XIH_PRTN_WORD_LEN;
	/* Make Length 16byte aligned
	 * TODO remove this after partition len is made
	 * 16byte aligned by bootgen*/
	TempVal = Len % XLOADER_DMA_LEN_ALIGN;
	if (TempVal != 0U) {
		Len = Len + XLOADER_DMA_LEN_ALIGN - TempVal;
	}

	if (PdiPtr->CopyToMem == TRUE) {
		Status = PdiPtr->DeviceCopy(SrcAddr, SrcAddr +
					XLOADER_DDR_COPYIMAGE_BASEADDR, Len, 0U);
		goto END;
	}

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
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);
	if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_PSM) {
		XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_PSM_PROC,
			PM_CAP_ACCESS, XPM_DEF_QOS, 0U);
	}

	/* Check if R5 App memory is TCM, Copy to global TCM memory MAP */
	if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) ||
		(DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) ||
		(DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L)) {
		if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
			XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1,
					IOCTL_SET_RPU_OPER_MODE,
					XPM_RPU_MODE_SPLIT, 0U, &Mode);
			XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_1_A,
					PM_CAP_ACCESS | PM_CAP_CONTEXT,
					XPM_DEF_QOS,0U);
			XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_1_B,
					PM_CAP_ACCESS | PM_CAP_CONTEXT,
					XPM_DEF_QOS, 0U);
		}
		else if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
			XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0,
				IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_SPLIT, 0U, &Mode);
			XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_0_A,
					PM_CAP_ACCESS | PM_CAP_CONTEXT,
					XPM_DEF_QOS, 0U);
			XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_0_B,
					PM_CAP_ACCESS | PM_CAP_CONTEXT,
					XPM_DEF_QOS, 0U);
		}
		else {
			XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0,
				IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_LOCKSTEP, 0U, &Mode);
			XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1,
				IOCTL_SET_RPU_OPER_MODE,
				XPM_RPU_MODE_LOCKSTEP, 0U, &Mode);
			XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_0_A,
					PM_CAP_ACCESS | PM_CAP_CONTEXT,
					XPM_DEF_QOS, 0U);
			XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_0_B,
					PM_CAP_ACCESS | PM_CAP_CONTEXT,
					XPM_DEF_QOS, 0U);
			XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_1_A,
					PM_CAP_ACCESS | PM_CAP_CONTEXT,
					XPM_DEF_QOS, 0U);
			XPm_RequestDevice(PM_SUBSYS_PMC,PM_DEV_TCM_1_B,
					PM_CAP_ACCESS | PM_CAP_CONTEXT,
					XPM_DEF_QOS, 0U);
		}

		Status = XLoader_GetLoadAddr(DstnCpu, &DestAddr, Len);
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}

	if (SecureParams.SecureEn != TRUE) {
		Status = PdiPtr->MetaHdr.DeviceCopy(SrcAddr, DestAddr, Len, 0x0U);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL, "Device Copy Failed \n\r");
			goto END;
		}
	}
	else {
		Status = XLoader_SecureCopy(&SecureParams, DestAddr, Len);
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}

	PrtnType = XilPdi_GetPrtnType(PrtnHdr);

	if ((PrtnType == XIH_PH_ATTRB_PRTN_TYPE_ELF) &&
		(((DstnCpu >= XIH_PH_ATTRB_DSTN_CPU_A72_0) &&
		(DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_A72_1))||
		(DstnCpu == XIH_PH_ATTRB_DSTN_CPU_NONE))) {
		/*
		 *  Populate handoff parameters to ATF
		 *  These correspond to the partition of application
		 *  which ATF will be loading
		 */
		XLoader_SetATFHandoffParameters(PrtnHdr);
	}

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function is used to update the handoff parameters.
 *
 * @param	PdiPtr is pointer to XilPdi Instance
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status = XST_FAILURE;
	u32 DstnCpu;
	u32 CpuNo;
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) &&
	    (DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_PSM)) {
		CpuNo = PdiPtr->NoOfHandoffCpus;
		if (XLoader_CheckHandoffCpu(PdiPtr, DstnCpu) == XST_SUCCESS) {
			if (CpuNo == XLOADER_MAX_HANDOFF_CPUS) {
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
 * @param	PdiPtr is pointer to XilPdi Instance
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
 * @param	PdiPtr is pointer to XilPdi Instance
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ProcessCdo (XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status = XST_FAILURE;
	u32 SrcAddr;
	u32 Len;
	u32 ChunkLen;
	XPlmiCdo Cdo = {0U};
	XilPdi_PrtnHdr * PrtnHdr;
	u32 LastChunk = FALSE;
	u32 ChunkAddr = XPLMI_LOADER_CHUNK_MEMORY;
	u32 IsNextChunkCopyStarted = FALSE;
	XLoader_SecureParms SecureParams = {0U};
	u32 TempVal;

	XPlmi_Printf(DEBUG_INFO, "Processing CDO partition \n\r");

	Status = XLoader_SecureInit(&SecureParams, PdiPtr, PrtnNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	SrcAddr = PdiPtr->MetaHdr.FlashOfstAddr +
			(((u64)PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	Len = (PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN);

	/**
	 * Make Length 16byte aligned.
	 * TODO remove this after partition len is made
	 * 16byte aligned by bootgen
	 */
	TempVal = Len % XLOADER_DMA_LEN_ALIGN;
	if (TempVal != 0U) {
		Len = Len + XLOADER_DMA_LEN_ALIGN - TempVal;
	}

	if (PdiPtr->CopyToMem == TRUE) {
		Status = PdiPtr->DeviceCopy(SrcAddr, SrcAddr +
					XLOADER_DDR_COPYIMAGE_BASEADDR, Len, 0U);
		goto END;
	}

	/*
	 * Initialize the Cdo Pointer and
	 * check CDO header contents
	 */
	XPlmi_InitCdo(&Cdo);
	Cdo.ImgId = PdiPtr->CurImgId;
	Cdo.PrtnId = PdiPtr->CurPrtnId;
	Cdo.IpiMask = PdiPtr->IpiMask;

	/*
	 * Process CDO in chunks.
	 * Chunk size is based on the available PRAM size.
	 */
	if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_DDR) &&
		(SecureParams.SecureEn != TRUE)) {
		ChunkLen = XLOADER_CHUNK_SIZE / 2U;
	}
	else {
		ChunkLen = XLOADER_CHUNK_SIZE;
	}

	SecureParams.IsCdo = TRUE;
	while (Len > 0U) {
		/* Update the len for last chunk */
		if (Len <= ChunkLen) {
			LastChunk = TRUE;
			ChunkLen = Len;
		}

		if (SecureParams.SecureEn != TRUE) {
			if (IsNextChunkCopyStarted == TRUE) {
				IsNextChunkCopyStarted = FALSE;
				/* Wait for copy to get completed */
				PdiPtr->DeviceCopy(SrcAddr, ChunkAddr, ChunkLen,
					XLOADER_DEVICE_COPY_STATE_WAIT_DONE);
			}
			else {
				/* Copy the data to PRAM buffer */
				PdiPtr->DeviceCopy(SrcAddr, ChunkAddr, ChunkLen,
					XLOADER_DEVICE_COPY_STATE_BLK);
			}
			/* Update variables for next chunk */
			Cdo.BufPtr = (u32 *)ChunkAddr;
			Cdo.BufLen = ChunkLen/XIH_PRTN_WORD_LEN;
			SrcAddr += ChunkLen;
			Len -= ChunkLen;

			if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI24) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_OSPI) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SMAP) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_JTAG) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SBI)) {
				Cdo.Cmd.KeyHoleParams.PdiSrc = PdiPtr->PdiSrc;
				Cdo.Cmd.KeyHoleParams.SrcAddr = SrcAddr;
				Cdo.Cmd.KeyHoleParams.Func = PdiPtr->DeviceCopy;
			}
			else if(PdiPtr->PdiSrc == XLOADER_PDI_SRC_DDR) {
				Cdo.Cmd.KeyHoleParams.PdiSrc = PdiPtr->PdiSrc;
				Cdo.Cmd.KeyHoleParams.SrcAddr = SrcAddr;
			}
			else {
				/** MISRA-C compliance */
			}

			if((PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI24) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32) ||
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_OSPI) ||
				(PdiPtr->SlrType == XLOADER_SSIT_MASTER_SLR)) {
				Cdo.Cmd.KeyHoleParams.InChunkCopy = TRUE;
			}
			/*
			 * For DDR case, start the copy of the
			 * next chunk for increasing performance
			 */
			if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_DDR)
			    && (LastChunk != TRUE)) {
				/* Update the next chunk address to other part */
				if (ChunkAddr == XPLMI_LOADER_CHUNK_MEMORY) {
					ChunkAddr = XPLMI_LOADER_CHUNK_MEMORY_1;
				}
				else {
					ChunkAddr = XPLMI_LOADER_CHUNK_MEMORY;
				}

				/* Update the len for last chunk */
				if (Len <= ChunkLen) {
					LastChunk = TRUE;
					ChunkLen = Len;
				}
				IsNextChunkCopyStarted = TRUE;

				/* Initiate the data copy */
				PdiPtr->DeviceCopy(SrcAddr, ChunkAddr, ChunkLen,
					XLOADER_DEVICE_COPY_STATE_INITIATE);
			}
		}
		else {
			Status = XLoader_ProcessSecurePrtn(&SecureParams,
				SecureParams.SecureData, ChunkLen, LastChunk);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Cdo.BufPtr = (u32 *)SecureParams.SecureData;
			Cdo.BufLen = SecureParams.SecureDataLen / XIH_PRTN_WORD_LEN;
			SrcAddr += ChunkLen;
			Len -= ChunkLen;
		}

		/* Process the chunk */
		Status = XPlmi_ProcessCdo(&Cdo);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (Cdo.Cmd.KeyHoleParams.ExtraWords != 0x0U) {
			Cdo.Cmd.KeyHoleParams.ExtraWords *= XPLMI_WORD_LEN;
			Len = Len - Cdo.Cmd.KeyHoleParams.ExtraWords;
			SrcAddr += Cdo.Cmd.KeyHoleParams.ExtraWords;
			IsNextChunkCopyStarted = FALSE;
			Cdo.Cmd.KeyHoleParams.ExtraWords = 0x0U;
		}
	}
	/* If deferred error, flagging it after CDO process complete */
	if (Cdo.DeferredError == TRUE) {
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
 * @brief	This function is used to process the partition. It copies and validates if
 * security is enabled.
 *
 * @param	PdiPtr is pointer to XilPdi Instance
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ProcessPrtn(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status = XST_FAILURE;
	XilPdi_PrtnHdr * PrtnHdr;
	u32 PrtnType;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Update current Processing partition ID */
	PdiPtr->CurPrtnId = PrtnHdr->PrtnId;

	/* Read Partition Type */
	PrtnType = XilPdi_GetPrtnType(PrtnHdr);
	if (PrtnType == XIH_PH_ATTRB_PRTN_TYPE_CDO) {
		Status = XLoader_ProcessCdo(PdiPtr, PrtnNum);
	}
	else {
		XPlmi_Printf(DEBUG_INFO, "Copying elf/data partition \n\r");
		/* Partition Copy */
		Status = XLoader_PrtnCopy(PdiPtr, PrtnNum);
	}

	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((PdiPtr->CopyToMem == FALSE) &&
		(PdiPtr->DelayHandoff == FALSE)) {
		/* Update the handoff values */
		Status = XLoader_UpdateHandoffParam(PdiPtr, PrtnNum);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
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
	u64 Address;

	Address = *LoadAddrPtr;

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
