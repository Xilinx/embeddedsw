/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
*       gm   05/10/2021 Added support to dump DDRMC registers in case of
*			deferred error
*       td   05/20/2021 Fixed blind write on locking NPI address space in
*                       XPlmi_ClearNpiErrors
* 1.06  td   07/15/2021 Fix doxygen warnings
*       bsv  08/17/2021 Code clean up
*       bsv  08/31/2021 Code clean up
*       bsv  09/01/2021 Added checks for zero length in XLoader_ProcessCdo
*       bsv  09/20/2021 Fixed logical error in processing Cdos
*       bm   09/23/2021 Fix R5 partition load issue
* 1.07  kpt  10/07/2021 Decoupled checksum functionality from secure code
*       is   10/12/2021 Updated XPm_DevIoctl to reflect additional command arg
*       kpt  10/20/2021 Modified temporal checks to use temporal variables from
*                       data section
*       bsv  10/26/2021 Code clean up
*       kpt  10/28/2021 Fixed checksum issue in case of copy to memory
* 1.08  skd  11/18/2021 Added time stamps in XLoader_ProcessCdo
*       ma   01/30/2022 Added support for skipping MJTAG image when bootmode is
*                       JTAG or Reset Reason is not EPOR
*       ma   01/31/2022 Fix DMA Keyhole command issue where the command
*                       starts at the 32K boundary
*       kpt  02/18/2022 Fix copy to memory issue for slave and non-slave boot
*                       modes
*       bsv  03/17/2022 Add support for A72 elfs to run from TCM
*       bsv  03/23/2022 Minor change in loading of A72 elfs to TCM
*       bsv  03/29/2022 Dump Ddrmc registers only when PLM DEBUG MODE is enabled
* 1.09  skg  06/20/2022 Fixed MISRA C Rule 10.3 violation
*       skg  06/20/2022 Fixed MISRA C Rule 4.1 violation
*       bm   07/06/2022 Refactor versal and versal_net code
*       dc   07/19/2022 Added support for data measurement in VersalNet
*       bm   07/24/2022 Set PlmLiveStatus during boot time
* 1.10  ng   11/11/2022 Updated doxygen comments
*       kal  01/05/2023 Added XLoader_SecureConfigMeasurement function for
*                       secure images measurement
*       bm   01/05/2023 Clear End Stack before processing a CDO partition
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       sk   05/18/2023 Deprecate copy to memory feature
*       bm   05/22/2023 Update current CDO command offset in GSW Error Status
*       bm   07/06/2023 Remove XPlmi_ClearEndStack call
*       rama 08/10/2023 Changed partition ID print to DEBUG_ALWAYS for
*                       debug level_0 option
*       dd   09/11/2023 MISRA-C violation Rule 10.3 fixed
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
#include "xloader_ddr.h"
#include "xplmi.h"
#include "xil_util.h"
#include "xplmi_err_common.h"
#include "xpm_nodeid.h"
#include "xplmi_plat.h"
#include "xloader_plat.h"
#include "xplmi_wdt.h"
#include "xplmi_tamper.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_SUCCESS_NOT_PRTN_OWNER	(0x100U) /**< Indicates that PLM is not the partition owner */

/************************** Function Prototypes ******************************/
static int XLoader_PrtnHdrValidation(const XilPdi_PrtnHdr* PrtnHdr, u32 PrtnNum);
static int XLoader_ProcessPrtn(XilPdi* PdiPtr, u32 PrtnIndex);
static int XLoader_ProcessCdo (const XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
	XLoader_SecureParams* SecureParams);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function loads the partition.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SLD_DETECTED_SKIP_PRTN_PROCESS on secure lockdown.
 * 			- XPLMI_NPI_ERR on NPI errors.
 *
 *****************************************************************************/
int XLoader_LoadImagePrtns(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	u32 PrtnIndex;
	u64 PrtnLoadTime;
	XPlmi_PerfTime PerfTime;

	if (PdiPtr->DelayLoad == (u8)FALSE) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"+++Loading Image#: 0x%0x, Name: %s, Id: 0x%08x\n\r",
			PdiPtr->ImageNum,
			(char *)PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName,
			PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
	}
	else {
		XPlmi_Printf(DEBUG_GENERAL,
			"+++Skipping Image#: 0x%0x, Name: %s, Id: 0x%08x\n\r",
			PdiPtr->ImageNum,
			(char *)PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName,
			PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
	}

	XPlmi_Printf(DEBUG_INFO, "------------------------------------\r\n");
	/**
	 * - Validate and load the image partitions.
	 */
	for (PrtnIndex = 0U;
		PrtnIndex < PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].NoOfPrtns;
		++PrtnIndex) {
		/**
		 * - Clear NPI errors before loading each partition.
		 */
		if (XPlmi_NpiOutOfReset() == (u8)TRUE) {
			Status = XPlmi_ClearNpiErrors();
			if (XST_SUCCESS != Status) {
				goto END;
			}
		}

		if (PdiPtr->DelayLoad == (u8)FALSE) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "---Loading Partition#: 0x%0x, "
					"Id: 0x%0x\r\n", PdiPtr->PrtnNum,
					PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum].PrtnId);
		}
		else {
			XPlmi_Printf(DEBUG_GENERAL, "---Skipping Partition#: 0x%0x, "
					"Id: 0x%0x\r\n", PdiPtr->PrtnNum,
					PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum].PrtnId);
		}

		PrtnLoadTime = XPlmi_GetTimerValue();
		/**
		 * - Validate the partition header.
		 */
		Status = XLoader_PrtnHdrValidation(
				&(PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum]), PdiPtr->PrtnNum);
		/**
		 * - If PLM is not partition owner then skip this partition.
		 */
		if (Status == (int)XLOADER_SUCCESS_NOT_PRTN_OWNER) {
			Status = XST_SUCCESS;
			continue;
		}
		if (XST_SUCCESS != Status) {
			goto END;
		}

		XPlmi_SetPlmLiveStatus();

		/**
		 * - Otherwise process the partition.
		 */
		Status = XLoader_ProcessPrtn(PdiPtr, PrtnIndex);
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

		/**
		 * - Skip processing rest of the partitions if secure lockdown
		 * is triggered.
		 */
		if (XPlmi_SldState() != XPLMI_SLD_NOT_TRIGGERED) {
			Status = XPlmi_UpdateStatus(XLOADER_SLD_DETECTED_SKIP_PRTN_PROCESS, 0);
			goto END1;
		}

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
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_SUCCESS_NOT_PRTN_OWNER if partition is not owned by PLM.
 *
 *****************************************************************************/
static int XLoader_PrtnHdrValidation(const XilPdi_PrtnHdr * PrtnHdr, u32 PrtnNum)
{
	int Status = (int)XLOADER_SUCCESS_NOT_PRTN_OWNER;

	/**
	 * Check if partition belongs to PLM
	 */
	if (XilPdi_GetPrtnOwner(PrtnHdr) != XIH_PH_ATTRB_PRTN_OWNER_PLM) {
		/**
		 * - If the partition doesn't belong to PLM, skip the partition
		 */
		XPlmi_Printf(DEBUG_GENERAL, "Not owned by PLM,"
				"skipping the Partition# 0x%08x\n\r", PrtnNum);
		goto END;
	}

	/**
	 * Validate the fields of partition
	 */
	Status = XilPdi_ValidatePrtnHdr(PrtnHdr);

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
 * @param	SecureParamsPtr is pointer to the instance containing security related
 *			params
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_PrtnCopy(const XilPdi* PdiPtr, const XLoader_DeviceCopy* DeviceCopy,
		void* SecureParamsPtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();
	XLoader_SecureParams *SecureParams = (XLoader_SecureParams *)SecureParamsPtr;
	u32 PrtnNum = PdiPtr->PrtnNum;
	const XilPdi_PrtnHdr * PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	u32 PcrInfo = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].PcrInfo;
	XLoader_ImageMeasureInfo ImageMeasureInfo = {0U};

	/**
	 * - Check if security is enabled and start the partition copy securely.
	 * Otherwise copy the partition in non-secure mode.
	 */
	if ((SecureParams->SecureEn == (u8)FALSE) &&
			(SecureTempParams->SecureEn == (u8)FALSE) &&
			(SecureParams->IsCheckSumEnabled == (u8)FALSE)) {
		Status = PdiPtr->MetaHdr.DeviceCopy(DeviceCopy->SrcAddr,
			DeviceCopy->DestAddr,DeviceCopy->Len, DeviceCopy->Flags);
	}
	else {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SecureCopy,
					SecureParams, DeviceCopy->DestAddr,
					DeviceCopy->Len);
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
			Status |= StatusTmp;
		}
	}
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "Device Copy Failed\n\r");
	}
	else {
		ImageMeasureInfo.DataAddr = DeviceCopy->DestAddr;
		ImageMeasureInfo.DataSize = PrtnHdr->UnEncDataWordLen << XPLMI_WORD_LEN_SHIFT;
		ImageMeasureInfo.PcrInfo = PcrInfo;
		ImageMeasureInfo.SubsystemID = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID;
		ImageMeasureInfo.Flags = XLOADER_MEASURE_UPDATE;
		/* Update the data for measurement, only VersalNet */
		Status = XLoader_DataMeasurement(&ImageMeasureInfo);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief	This function is used to process the CDO partition. It copies and
 * 			validates if security is enabled.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 * @param	DeviceCopy is pointer to the structure variable with parameters
 *			required for copying
 * @param	SecureParams is pointer to the instance containing security related
 *			params
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_INIT_CDO on CDO initialization fail.
 *
 *****************************************************************************/
static int XLoader_ProcessCdo(const XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
		XLoader_SecureParams* SecureParams)
{
	int Status = XST_FAILURE;
	u32 ChunkLen = XLOADER_SECURE_CHUNK_SIZE;
	u32 ChunkLenTemp;
	XPlmiCdo Cdo;
	u32 ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
	u32 ChunkAddrTemp;
	u8 LastChunk = (u8)FALSE;
	u8 Flags;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();
#ifdef PLM_PRINT_PERF_CDO_PROCESS
	u64 CdoProcessTimeStart;
	u64 CdoProcessTimeEnd;
	u64 CdoProcessTime = 0U;
	XPlmi_PerfTime PerfTime;
#endif
	u32 PcrInfo = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].PcrInfo;
	XLoader_ImageMeasureInfo ImageMeasureInfo = {0U};

	XPlmi_Printf(DEBUG_INFO, "Processing CDO partition \n\r");
	/**
	 * Initialize the Cdo Pointer and check CDO header contents
	 */
	Status = XPlmi_InitCdo(&Cdo);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INIT_CDO, Status);
		goto END;
	}
	Cdo.NextChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
	Cdo.SubsystemId = XPm_GetSubsystemId(
		PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
	Cdo.PartitionOffset = PdiPtr->MetaHdr.PrtnHdr[PdiPtr->PrtnNum].DataWordOfst;
	SecureParams->IsCdo = (u8)TRUE;
	if ((SecureParams->SecureEn == (u8)FALSE) &&
		(SecureTempParams->SecureEn == (u8)FALSE) &&
		(SecureParams->IsCheckSumEnabled == (u8)FALSE)) {
		if ((PdiPtr->PdiIndex == XLOADER_SD_INDEX) ||
			(PdiPtr->PdiIndex == XLOADER_SD_RAW_INDEX)) {
			ChunkLen = XLOADER_CHUNK_SIZE;
		}
		else {
			Cdo.Cmd.KeyHoleParams.Func = PdiPtr->MetaHdr.DeviceCopy;
		}
	}

	while (DeviceCopy->Len > 0U) {
		/** Update the len for last chunk */
		if (DeviceCopy->Len <= ChunkLen) {
			LastChunk = (u8)TRUE;
			ChunkLen = DeviceCopy->Len;
		}

		if ((SecureParams->SecureEn == (u8)FALSE) &&
			(SecureTempParams->SecureEn == (u8)FALSE) &&
			(SecureParams->IsCheckSumEnabled == FALSE)) {
			if (Cdo.Cmd.KeyHoleParams.IsNextChunkCopyStarted == (u8)TRUE) {
				Cdo.Cmd.KeyHoleParams.IsNextChunkCopyStarted = (u8)FALSE;
				Flags = XPLMI_DEVICE_COPY_STATE_WAIT_DONE;
			}
			else {
				Flags = XPLMI_DEVICE_COPY_STATE_BLK;
			}
			Status = PdiPtr->MetaHdr.DeviceCopy(DeviceCopy->SrcAddr,
				ChunkAddr, ChunkLen, (DeviceCopy->Flags | Flags));
			if (Status != XST_SUCCESS) {
					goto END;
			}
			/** Update variables for next chunk */
			Cdo.BufPtr = (u32 *)ChunkAddr;
			Cdo.BufLen = ChunkLen >> XPLMI_WORD_LEN_SHIFT;
			ChunkLenTemp = ChunkLen;
			DeviceCopy->SrcAddr += ChunkLen;
			DeviceCopy->Len -= ChunkLen;
			Cdo.Cmd.KeyHoleParams.SrcAddr = DeviceCopy->SrcAddr;
			/**
			 * Start the copy of the next chunk for increasing performance
			 */
			if (LastChunk != (u8)TRUE) {
				/** Update the next chunk address to other part */
				ChunkAddr += ChunkLen;
				if (ChunkAddr > XPLMI_PMCRAM_CHUNK_MEMORY_1) {
					ChunkAddr =
						XPLMI_PMCRAM_CHUNK_MEMORY;
				}
				else {
					ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY_1;
				}
				/** Update the len for last chunk */
				if (DeviceCopy->Len <= ChunkLen) {
					LastChunk = (u8)TRUE;
					ChunkLen = DeviceCopy->Len;
				}
				Cdo.Cmd.KeyHoleParams.IsNextChunkCopyStarted = (u8)TRUE;
				Cdo.NextChunkAddr = ChunkAddr;
				/** Initiate the data copy */
				Status = PdiPtr->MetaHdr.DeviceCopy(
					DeviceCopy->SrcAddr, ChunkAddr,
					ChunkLen, DeviceCopy->Flags |
					XPLMI_DEVICE_COPY_STATE_INITIATE);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
		}
		else {
			SecureParams->RemainingDataLen = DeviceCopy->Len;

			Status = SecureParams->ProcessPrtn(SecureParams,
					SecureParams->SecureData, ChunkLen, LastChunk);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			Cdo.NextChunkAddr = SecureParams->NextChunkAddr;
			SecureParams->ChunkAddr = SecureParams->NextChunkAddr;
			Cdo.BufPtr = (u32 *)SecureParams->SecureData;
			Cdo.BufLen = SecureParams->SecureDataLen >> XPLMI_WORD_LEN_SHIFT;
			ChunkLenTemp = SecureParams->SecureDataLen;
			DeviceCopy->SrcAddr += SecureParams->ProcessedLen;
			DeviceCopy->Len -= SecureParams->ProcessedLen;
		}
#ifdef PLM_PRINT_PERF_CDO_PROCESS
		CdoProcessTimeStart = XPlmi_GetTimerValue();
#endif
		/** Process the chunk */
		Status = XPlmi_ProcessCdo(&Cdo);
		if (Status != XST_SUCCESS) {
			goto END;
		}
#ifdef PLM_PRINT_PERF_CDO_PROCESS
		CdoProcessTimeEnd = XPlmi_GetTimerValue();
		CdoProcessTime += (CdoProcessTimeStart - CdoProcessTimeEnd);
#endif
		if (Cdo.Cmd.KeyHoleParams.ExtraWords != 0x0U) {
			Cdo.Cmd.KeyHoleParams.ExtraWords <<= XPLMI_WORD_LEN_SHIFT;
			if ((Cdo.Cmd.KeyHoleParams.IsNextChunkCopyStarted == (u8)TRUE) &&
					(Cdo.Cmd.KeyHoleParams.ExtraWords < ChunkLen)) {
				DeviceCopy->Len -= ChunkLen;
				DeviceCopy->SrcAddr += ChunkLen;
				/**
				 * There are some CDO commands to be processed in
				 * memory pointed to by ChunkAddr
				 */
				ChunkAddrTemp = (ChunkAddr + Cdo.Cmd.KeyHoleParams.ExtraWords);
				ChunkLenTemp = (ChunkLen - Cdo.Cmd.KeyHoleParams.ExtraWords);
				Cdo.BufPtr = (u32 *)ChunkAddrTemp;
				Cdo.BufLen = ChunkLenTemp >> XIH_PRTN_WORD_LEN_SHIFT;
				Cdo.Cmd.KeyHoleParams.ExtraWords = 0x0U;
				Cdo.Cmd.KeyHoleParams.SrcAddr = DeviceCopy->SrcAddr;
				Cdo.Cmd.KeyHoleParams.IsNextChunkCopyStarted = (u8)FALSE;
				Status = XPlmi_ProcessCdo(&Cdo);
				if (Status != XST_SUCCESS) {
					goto END;
				}

				if (Cdo.Cmd.KeyHoleParams.ExtraWords != 0x0U) {
					Cdo.Cmd.KeyHoleParams.ExtraWords <<= XPLMI_WORD_LEN_SHIFT;
					DeviceCopy->Len -= Cdo.Cmd.KeyHoleParams.ExtraWords;
					DeviceCopy->SrcAddr += Cdo.Cmd.KeyHoleParams.ExtraWords;
					Cdo.Cmd.KeyHoleParams.ExtraWords = 0x0U;
				}
			}
			else {
				DeviceCopy->Len -= Cdo.Cmd.KeyHoleParams.ExtraWords;
				DeviceCopy->SrcAddr += Cdo.Cmd.KeyHoleParams.ExtraWords;
				Cdo.Cmd.KeyHoleParams.ExtraWords = 0x0U;
				Cdo.Cmd.KeyHoleParams.IsNextChunkCopyStarted = (u8)FALSE;
			}
		}
		else {
			ImageMeasureInfo.DataAddr = (u64)(UINTPTR)Cdo.BufPtr;
			ImageMeasureInfo.DataSize = ChunkLenTemp;
			ImageMeasureInfo.PcrInfo = PcrInfo;
			ImageMeasureInfo.SubsystemID = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID;
			ImageMeasureInfo.Flags = XLOADER_MEASURE_UPDATE;
			/* Update the data for measurement, only VersalNet */
			Status = XLoader_DataMeasurement(&ImageMeasureInfo);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

	/** If deferred error, flagging it after CDO process complete */
	if (Cdo.DeferredError == (u8)TRUE) {
		Status = XLoader_ProcessDeferredError();
		goto END;
	}
	Status = XST_SUCCESS;

END:
#ifdef PLM_PRINT_PERF_CDO_PROCESS
	XPlmi_MeasurePerfTime((XPlmi_GetTimerValue() + CdoProcessTime),
				&PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
			"%u.%03u ms Cdo Processing time\n\r",
			(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
#endif
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function is used to process the partition.
 *
 * @param	PdiPtr is pointer to XilPdi instance
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_COPY_TO_MEM if failed to copy image to DDR.
 * 			- XLOADER_ERR_DELAY_LOAD on errors during copies from SMAP, SBI,
 * 			PCIE, or JTAG to PMC RAM.
 *
 *****************************************************************************/
static int XLoader_ProcessPrtn(XilPdi* PdiPtr, u32 PrtnIndex)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XLoader_SecureParams SecureParams;
	XLoader_PrtnParams PrtnParams;
	u32 PrtnType;
	u32 TrfLen;
	u8 TempVal;
	u32 PrtnNum = PdiPtr->PrtnNum;
	/* Assign the partition header to local variable */
	const XilPdi_PrtnHdr * PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	u32 PcrInfo = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].PcrInfo;

	/** Read Partition Type */
	PrtnType = XilPdi_GetPrtnType(PrtnHdr);

	PrtnParams.DeviceCopy.DestAddr = PrtnHdr->DstnLoadAddr;
	PrtnParams.DeviceCopy.Len = (PrtnHdr->TotalDataWordLen <<
		XPLMI_WORD_LEN_SHIFT);
	PrtnParams.DeviceCopy.Flags = 0U;

	if (PdiPtr->PdiType != XLOADER_PDI_TYPE_RESTORE) {
		PrtnParams.DeviceCopy.SrcAddr = PdiPtr->MetaHdr.FlashOfstAddr +
			((u64)PrtnHdr->DataWordOfst << XPLMI_WORD_LEN_SHIFT);
	}

	if (PdiPtr->DelayLoad == (u8)TRUE) {
		if (PdiPtr->PdiIndex == XLOADER_SBI_INDEX) {
			while (PrtnParams.DeviceCopy.Len > 0U) {
				if (PrtnParams.DeviceCopy.Len > XLOADER_CHUNK_SIZE) {
					TrfLen = XLOADER_CHUNK_SIZE;
				}
				else {
					TrfLen = PrtnParams.DeviceCopy.Len;
				}
				Status = PdiPtr->MetaHdr.DeviceCopy(PrtnParams.DeviceCopy.SrcAddr,
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

	/**
	 * ProcessCdo, ProcessElf and PrtnCopy APIs expected unencrypted
	 * length that is 16 byte aligned. Make unencrypted length 16 byte aligned.
	 */
	TempVal = (u8)(XLOADER_DMA_LEN_ALIGN -
		(u8)((PrtnParams.DeviceCopy.Len & XLOADER_DMA_LEN_ALIGN_MASK)));
	PrtnParams.DeviceCopy.Len += TempVal & XLOADER_DMA_LEN_ALIGN_MASK;

	/**
	 * To make sure total data length passed is without authentication
	 * certificate size when authentication is enabled.
	 */
	PrtnParams.DeviceCopy.Len -= SecureParams.ProcessedLen;

	/* Skip MJTAG workaround partition for versal */
	if (XLoader_SkipMJtagWorkAround(PdiPtr) == (u8)TRUE) {
		/* Just copy the partitions to PMC RAM and skip processing */
		PrtnParams.DeviceCopy.DestAddr = XPLMI_PMCRAM_BASEADDR;
		Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams.DeviceCopy, &SecureParams);
		XPlmi_Printf(DEBUG_GENERAL, "Skipping MJTAG partition\n\r");
		goto END;
	}

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
	/* In case of Authentication enabled, it is mandatory to use
	 * same SPK for all partitions of an image. Hence secure config extend is
	 * called only for the first partition of an image.
	 */
	if ((Status == XST_SUCCESS) && (PrtnIndex == 0x00U)) {
		if (PdiPtr->PdiType == XLOADER_PDI_TYPE_PARTIAL) {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SecureConfigMeasurement,
					&SecureParams, PcrInfo, &PdiPtr->DigestIndex, TRUE);
		} else {
			XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_SecureConfigMeasurement,
					&SecureParams, PcrInfo, &PdiPtr->DigestIndex, FALSE);
		}
		if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
			Status |= StatusTmp;
		}
	}

END:
	return Status;
}
