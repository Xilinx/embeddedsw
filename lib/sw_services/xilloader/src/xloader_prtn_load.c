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
#include "xplmi_err.h"
#include "xpm_nodeid.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_SUCCESS_NOT_PRTN_OWNER	(0x100U) /**< Indicates that PLM is not the partition owner */
#define XLOADER_TCM_0		(0U)
#define XLOADER_TCM_1		(1U)
#define XLOADER_RPU_GLBL_CNTL	(0xFF9A0000U)
#define XLOADER_TCMCOMB_MASK		(0x40U)
#define XLOADER_TCMCOMB_SHIFT		(6U)

/**
 * @{
 * @cond DDR calibration errors
 */
#define DDRMC_OFFSET_CALIB_ERR		(0x840CU)
#define DDRMC_OFFSET_CALIB_ERR_NIBBLE_1	(0x8420U)
#define DDRMC_OFFSET_CALIB_ERR_NIBBLE_2	(0x841CU)
#define DDRMC_OFFSET_CALIB_ERR_NIBBLE_3	(0x8418U)
#define DDRMC_OFFSET_CALIB_STAGE_PTR	(0x8400U)
/**
 * @}
 * @endcond
 */

/************************** Function Prototypes ******************************/
static int XLoader_PrtnHdrValidation(const XilPdi_PrtnHdr* PrtnHdr, u32 PrtnNum);
static int XLoader_ProcessPrtn(XilPdi* PdiPtr);
static int XLoader_PrtnCopy(const XilPdi* PdiPtr, const XLoader_DeviceCopy* DeviceCopy,
	XLoader_SecureParams* SecureParams);
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu);
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len);
static int XLoader_ProcessCdo (const XilPdi* PdiPtr, XLoader_DeviceCopy* DeviceCopy,
	XLoader_SecureParams* SecureParams);
static int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr* PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams);
static int XLoader_DumpDdrmcRegisters(void);
static int XLoader_RequestTCM(u8 TcmId);

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
	XPlmi_PerfTime PerfTime;

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
			Status = XPlmi_ClearNpiErrors();
			if (XST_SUCCESS != Status) {
				goto END;
			}
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
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();

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
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
	u32 Len = PrtnHdr->UnEncDataWordLen << XPLMI_WORD_LEN_SHIFT;
	u64 EndAddr = PrtnParams->DeviceCopy.DestAddr + Len - 1U;
	u32 ErrorCode;
	u32 Mode = 0U;
	u8 TcmComb;

	Status = XPlmi_VerifyAddrRange(PrtnParams->DeviceCopy.DestAddr, EndAddr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INVALID_ELF_LOAD_ADDR,
				Status);
		goto END;
	}
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
			(CapSecureAccess | CapContext), XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_PSM_PROC, 0);
			goto END;
		}
		goto END1;
	}

	if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0,
			IOCTL_SET_RPU_OPER_MODE, XPM_RPU_MODE_SPLIT, 0U, 0U, &Mode,
			XPLMI_CMD_SECURE);
		ErrorCode = XLOADER_ERR_PM_DEV_IOCTL_RPU0_SPLIT;
	}
	else if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1,
			IOCTL_SET_RPU_OPER_MODE, XPM_RPU_MODE_SPLIT, 0U, 0U, &Mode,
			XPLMI_CMD_SECURE);
		ErrorCode = XLOADER_ERR_PM_DEV_IOCTL_RPU1_SPLIT;
	}
	else if (PrtnParams->DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) {
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_0,
			IOCTL_SET_RPU_OPER_MODE, XPM_RPU_MODE_LOCKSTEP, 0U, 0U,
			&Mode, XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_PM_DEV_IOCTL_RPU0_LOCKSTEP, 0);
			goto END;
		}
		Status = XPm_DevIoctl(PM_SUBSYS_PMC, PM_DEV_RPU0_1,
			IOCTL_SET_RPU_OPER_MODE, XPM_RPU_MODE_LOCKSTEP, 0U, 0U,
			&Mode, XPLMI_CMD_SECURE);
		ErrorCode = XLOADER_ERR_PM_DEV_IOCTL_RPU1_LOCKSTEP;
	}
	else {
		/* MISRA-C compliance */
	}
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(ErrorCode, 0);
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

	Status = XLoader_GetLoadAddr(PrtnParams->DstnCpu,
		&PrtnParams->DeviceCopy.DestAddr, Len);
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
		TcmComb = (u8)((XPlmi_In32(XLOADER_RPU_GLBL_CNTL) &
			XLOADER_TCMCOMB_MASK) >> XLOADER_TCMCOMB_SHIFT);
		if (TcmComb == (u8)FALSE) {
			Status = XLoader_RequestTCM(XLOADER_TCM_1);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INVALID_TCM_ADDR, 0);
		}
	}
	else if ((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_0_TCM_A_BASE_ADDR)
		&& (EndAddr <= XLOADER_R5_0_TCM_A_END_ADDR)) {
		/* TCM 0 A is in use */
		Status = XLoader_RequestTCM(XLOADER_TCM_0);
	}
	else if ((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_0_TCM_B_BASE_ADDR)
		&& (EndAddr <= XLOADER_R5_0_TCM_B_END_ADDR)) {
		/* TCM 0 B is in use */
		TcmComb = (u8)((XPlmi_In32(XLOADER_RPU_GLBL_CNTL) &
			XLOADER_TCMCOMB_MASK) >> XLOADER_TCMCOMB_SHIFT);
		if (TcmComb == (u8)FALSE) {
			Status = XLoader_RequestTCM(XLOADER_TCM_0);
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INVALID_TCM_ADDR, 0);
		}
	}
	else if ((PrtnParams->DeviceCopy.DestAddr >= XLOADER_R5_0_TCM_A_BASE_ADDR)
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
		}
		else {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INVALID_TCM_ADDR, 0);
		}
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

END1:
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
	u32 CpuNo = XLOADER_MAX_HANDOFF_CPUS;
	u32 PrtnNum = PdiPtr->PrtnNum;
	/* Assign the partition header to local variable */
	const XilPdi_PrtnHdr * PrtnHdr =
			&(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

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
 * @return	XST_SUCCESS if the DstnCpu is successfully added to Handoff list
 *          XST_FAILURE if the DstnCpu is already added to Handoff list
 *
 *****************************************************************************/
static int XLoader_CheckHandoffCpu(const XilPdi* PdiPtr, const u32 DstnCpu)
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
	Cdo.NextChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
	Cdo.SubsystemId = XPm_GetSubsystemId(
		PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
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
		/* Update the len for last chunk */
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
			/* Update variables for next chunk */
			Cdo.BufPtr = (u32 *)ChunkAddr;
			Cdo.BufLen = ChunkLen >> XPLMI_WORD_LEN_SHIFT;
			DeviceCopy->SrcAddr += ChunkLen;
			DeviceCopy->Len -= ChunkLen;
			Cdo.Cmd.KeyHoleParams.SrcAddr = DeviceCopy->SrcAddr;
			/*
			 * Start the copy of the next chunk for increasing performance
			 */
			if (LastChunk != (u8)TRUE) {
				/* Update the next chunk address to other part */
				ChunkAddr += ChunkLen;
				if (ChunkAddr > XPLMI_PMCRAM_CHUNK_MEMORY_1) {
					ChunkAddr =
						XPLMI_PMCRAM_CHUNK_MEMORY;
				}
				else {
					ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY_1;
				}
				/* Update the len for last chunk */
				if (DeviceCopy->Len <= ChunkLen) {
					LastChunk = (u8)TRUE;
					ChunkLen = DeviceCopy->Len;
				}
				Cdo.Cmd.KeyHoleParams.IsNextChunkCopyStarted = (u8)TRUE;
				Cdo.NextChunkAddr = ChunkAddr;
				/* Initiate the data copy */
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
			DeviceCopy->SrcAddr += SecureParams->ProcessedLen;
			DeviceCopy->Len -= SecureParams->ProcessedLen;
		}
#ifdef PLM_PRINT_PERF_CDO_PROCESS
		CdoProcessTimeStart = XPlmi_GetTimerValue();
#endif
		/* Process the chunk */
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
				/*
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
	}

	/* If deferred error, flagging it after CDO process complete */
	if (Cdo.DeferredError == (u8)TRUE) {
		Status = XLoader_DumpDdrmcRegisters();
		Status = XPlmi_UpdateStatus(
			XLOADER_ERR_DEFERRED_CDO_PROCESS, Status);
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
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ProcessPrtn(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	u32 PdiSrc = PdiPtr->PdiSrc;
	int (*DevCopy) (u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags) = NULL;
	XLoader_SecureParams SecureParams;
	XLoader_PrtnParams PrtnParams;
	u32 PrtnType;
	u64 OfstAddr = 0U;
	u32 TrfLen;
	u8 TempVal;
	u32 PrtnNum = PdiPtr->PrtnNum;
	u8 ToStoreInDdr = (u8)FALSE;
	u8 PdiType;
	u64 CopyToMemAddr = PdiPtr->CopyToMemAddr;
	/* Assign the partition header to local variable */
	const XilPdi_PrtnHdr * PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	u32 RstReason;

	/* Read Partition Type */
	PrtnType = XilPdi_GetPrtnType(PrtnHdr);

	PrtnParams.DeviceCopy.DestAddr = PrtnHdr->DstnLoadAddr;
	PrtnParams.DeviceCopy.Len = (PrtnHdr->TotalDataWordLen <<
		XPLMI_WORD_LEN_SHIFT);
	PrtnParams.DeviceCopy.Flags = 0U;

	if (PdiPtr->PdiType != XLOADER_PDI_TYPE_RESTORE) {
		PrtnParams.DeviceCopy.SrcAddr = PdiPtr->MetaHdr.FlashOfstAddr +
			((u64)PrtnHdr->DataWordOfst << XPLMI_WORD_LEN_SHIFT);
	}

	if (PdiPtr->CopyToMem == (u8)TRUE) {
		Status = XLoader_SecureInit(&SecureParams, PdiPtr, PrtnNum);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = PdiPtr->MetaHdr.DeviceCopy(PrtnParams.DeviceCopy.SrcAddr,
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
		DevCopy = PdiPtr->MetaHdr.DeviceCopy;
		PdiPtr->MetaHdr.DeviceCopy = XLoader_DdrCopy;
		OfstAddr = PdiPtr->MetaHdr.FlashOfstAddr;
		PrtnParams.DeviceCopy.Flags = XPLMI_PMCDMA_0;
		ToStoreInDdr = (u8)TRUE;
		PdiPtr->CopyToMemAddr = CopyToMemAddr;
		PdiPtr->CopyToMem = (u8)FALSE;
		PdiType = PdiPtr->PdiType;
		PdiPtr->PdiType = XLOADER_PDI_TYPE_RESTORE;
	}
	else if (PdiPtr->DelayLoad == (u8)TRUE) {
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

	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_RESTORE) {
		PrtnParams.DeviceCopy.SrcAddr = PdiPtr->CopyToMemAddr;
		OfstAddr = PdiPtr->MetaHdr.FlashOfstAddr;
		PdiPtr->MetaHdr.FlashOfstAddr = PdiPtr->CopyToMemAddr -
				((u64)PrtnHdr->DataWordOfst * XIH_PRTN_WORD_LEN);
		PdiPtr->CopyToMemAddr += ((u64)PrtnParams.DeviceCopy.Len - SecureParams.SecureHdrLen);
	}

	/*
	 * ProcessCdo, ProcessElf and PrtnCopy APIs expected unencrypted
	 * length that is 16 byte aligned
	 */
	/*
	 * Make unencrypted length 16 byte aligned.
	 */
	TempVal = (u8)(XLOADER_DMA_LEN_ALIGN -
		(u8)((PrtnParams.DeviceCopy.Len & XLOADER_DMA_LEN_ALIGN_MASK)));
	PrtnParams.DeviceCopy.Len += TempVal & XLOADER_DMA_LEN_ALIGN_MASK;

	/* To make sure total data length passed is without authentication
	 * certificate size when authentication is enabled.
	 */
	PrtnParams.DeviceCopy.Len -= SecureParams.ProcessedLen;

	/* MJTAG workaround partition */
	if (PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID ==
			PM_MISC_MJTAG_WA_IMG) {
		RstReason = XPlmi_In32(PMC_GLOBAL_PERS_GEN_STORAGE2);
		/*
		 * Skip MJTAG WA2 partitions if boot mode is JTAG and
		 * Reset Reason is not external POR
		 */
		if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_JTAG) ||
			(((RstReason & PERS_GEN_STORAGE2_ACC_RR_MASK) >>
					CRP_RESET_REASON_SHIFT) !=
				CRP_RESET_REASON_EXT_POR_MASK)) {
			/* Just copy the partitions to PMC RAM and skip processing */
			PrtnParams.DeviceCopy.DestAddr = XPLMI_PMCRAM_BASEADDR;
			Status = XLoader_PrtnCopy(PdiPtr, &PrtnParams.DeviceCopy, &SecureParams);
			XPlmi_Printf(DEBUG_GENERAL, "Skipping MJTAG partition\n\r");
			goto END;
		}
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

END:
	if (ToStoreInDdr == (u8)TRUE) {
		PdiPtr->PdiSrc = PdiSrc;
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
	u32 Offset = 0U;

	if (((Address < (XLOADER_R5_TCMA_LOAD_ADDRESS +
			XLOADER_R5_TCM_BANK_LENGTH)) ||
			((Address >= XLOADER_R5_TCMB_LOAD_ADDRESS) &&
			(Address < (XLOADER_R5_TCMB_LOAD_ADDRESS +
				XLOADER_R5_TCM_BANK_LENGTH))))) {
		if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
			Offset = XLOADER_R5_0_TCMA_BASE_ADDR;
		}
		else if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
			Offset = XLOADER_R5_1_TCMA_BASE_ADDR;
		}
		else {
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

	if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) &&
		(Address < XLOADER_R5_TCM_TOTAL_LENGTH)) {
		if (((Address % XLOADER_R5_TCM_TOTAL_LENGTH) + Len) >
			XLOADER_R5_TCM_TOTAL_LENGTH) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, 0);
			goto END;
		}
		Offset = XLOADER_R5_0_TCMA_BASE_ADDR;
	}

	/*
	 * Update the load address
	 */
	Address += Offset;
	*LoadAddrPtr = Address;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function prints DDRMC register details.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_DumpDdrmcRegisters(void)
{
	int Status = XST_FAILURE;
	u32 PcsrStatus;
	u32 CalibErr;
	u32 CalibErrNibble1;
	u32 CalibErrNibble2;
	u32 CalibErrNibble3;
	u32 CalibStage;
	u32 PcsrCtrl;
	u32 DevId;
	u8 Ub = 0U;
	u32 BaseAddr;

	XPlmi_Printf(DEBUG_GENERAL,"====DDRMC Register Dump Start======\n\r");

	Status = XLoader_DdrInit(XLOADER_PDI_SRC_DDR);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL,
				"Error  0x%0x in requesting DDR.\n\r", Status);
		goto END;
	}

	for (DevId = PM_DEV_DDRMC_0; DevId <= PM_DEV_DDRMC_3; DevId++) {
		/* Get DDRMC UB Base address */
		Status = XPm_GetDeviceBaseAddr(DevId, &BaseAddr);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Error 0x%0x in getting DDRMC_%u addr\n",
				Status, Ub);
			goto END;
		}

		XPlmi_Printf(DEBUG_GENERAL,
				"DDRMC_%u (UB 0x%08x)\n\r", Ub, BaseAddr);

		/* Read PCSR Control */
		PcsrCtrl = XPlmi_In32(BaseAddr + DDRMC_PCSR_CONTROL_OFFSET);

		/* Skip DDRMC dump if PComplete is zero */
		if (0U == (PcsrCtrl & DDRMC_PCSR_CONTROL_PCOMPLETE_MASK)) {
			XPlmi_Printf(DEBUG_GENERAL, "PComplete not set\n\r");
			++Ub;
			continue;
		}

		/* Read PCSR Status */
		PcsrStatus = XPlmi_In32(BaseAddr + DDRMC_PCSR_STATUS_OFFSET);
		/* Read Calibration Error */
		CalibErr = XPlmi_In32(BaseAddr + DDRMC_OFFSET_CALIB_ERR);
		/* Read Error Nibble 1 */
		CalibErrNibble1 = XPlmi_In32(BaseAddr +
				DDRMC_OFFSET_CALIB_ERR_NIBBLE_1);
		/* Read Error Nibble 2 */
		CalibErrNibble2 = XPlmi_In32(BaseAddr +
				DDRMC_OFFSET_CALIB_ERR_NIBBLE_2);
		/* Read Error Nibble 3 */
		CalibErrNibble3 = XPlmi_In32(BaseAddr +
				DDRMC_OFFSET_CALIB_ERR_NIBBLE_3);
		/* Read calibration stage */
		CalibStage = XPlmi_In32(BaseAddr +
				DDRMC_OFFSET_CALIB_STAGE_PTR);

		XPlmi_Printf(DEBUG_GENERAL,
				"PCSR Control: 0x%0x\n\r", PcsrCtrl);
		XPlmi_Printf(DEBUG_GENERAL,
				"PCSR Status: 0x%0x\n\r", PcsrStatus);
		XPlmi_Printf(DEBUG_GENERAL,
				"Calibration Error: 0x%0x\n\r", CalibErr);
		XPlmi_Printf(DEBUG_GENERAL,
				"Nibble Location 1: 0x%0x\n\r",
				CalibErrNibble1);
		XPlmi_Printf(DEBUG_GENERAL,
				"Nibble Location 2: 0x%0x\n\r",
				CalibErrNibble2);
		XPlmi_Printf(DEBUG_GENERAL,
				"Nibble Location 3: 0x%0x\n\r",
				CalibErrNibble3);
		XPlmi_Printf(DEBUG_GENERAL,
				"Calibration Stage: 0x%0x\n\r", CalibStage);
		++Ub;
	}
	XPlmi_Printf(DEBUG_GENERAL, "PMC Interrupt Status : 0x%0x\n\r",
			XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "====DDRMC Register Dump End======\n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function requests TCM_0_A, TCM_0_B, TCM_1_A and TCM_1_B
 * depending upon input param and R5-0 and R5-1 cores as required for TCMs.
 *
 * @param	TcmId denotes TCM_0 or TCM_1
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_RequestTCM(u8 TcmId)
{
	int Status = XST_FAILURE;
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
	u32 ErrorCode;

	if (TcmId == XLOADER_TCM_0) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_0_A,
			(CapSecureAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_0_A;
			goto END;
		}
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_0_B,
			(CapSecureAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		ErrorCode = XLOADER_ERR_PM_DEV_TCM_0_B;
	}
	else if (TcmId == XLOADER_TCM_1) {
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_1_A,
			(CapSecureAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			ErrorCode = XLOADER_ERR_PM_DEV_TCM_1_A;
			goto END;
		}

		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_TCM_1_B,
			(CapSecureAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		ErrorCode = XLOADER_ERR_PM_DEV_TCM_1_B;
	}

END:
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(ErrorCode, 0);
	}
	return Status;
}