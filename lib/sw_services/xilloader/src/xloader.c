/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader.c
*
* This file contains the code related to PDI image loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/25/2018 Initial release
*       tp   04/05/2019 Added API to reload a particular image
*       kc   04/09/2019 Added support for PCIe secondary boot mode and
*						partial PDI load
*       bsv  06/11/2019 Added TCM power up code to Xilloader to fix issue in
*						R5-1 split mode functionality
*       bsv  06/17/2019 Added support for CFI and CFU error handling
*       bsv  06/26/2019 Added secondary boot support
*       kc   07/16/2019 Added code to print execution time
*       rv   07/29/2019 Added code to request boot source before subsystem restart
*       vnsl 07/30/2019 Added APIs to load secure headers
*       scs  08/29/2019 Added API to validate extended ID Code
*       bsv  08/30/2019 Added fallback and multiboot support in PLM
*       kc   09/05/2019 Added code to use PMCDMA0 and PMCDMA1 in parallel
*       kc   09/13/2019 SBI reset is removed for SMAP boot mode to ensure smap
*						bus width value remains unchanged
* 1.01  bsv  10/31/2019 Added USB secondary boot mode support
*       kc   12/02/2019 Added performance time stamps
*       ma   12/12/2019 Added support for passing hand off parameters to ATF
*       bsv  12/30/2019 Added SMAP secondary boot mode support
*       ma   02/03/2020 Change XPlmi_MeasurePerfTime to retrieve Performance
*                       time and print
*       bsv  02/12/2020 Added support for SD/eMMC raw boot mode
*       bsv  02/23/2020 Added multi partition support for SD/eMMC FS boot modes
*       bsv  02/25/2020 Added macros to handle u32 return values from drivers
*       vnsl 02/26/2020 Added boot header reading call during partial PDI
*       kc   02/27/2020 Added SEM support for partial reconfiguration
*       bsv  02/28/2020 Added support for delay handoff
*       har  02/28/2020 Removed code to return error codes for security related
*                       errors
*       vnsl 03/01/2020 Added PUF KEK decrypt support
*       bsv  03/14/2020 Added eMMC0 FS and raw boot mode support
*       bsv  04/09/2020 Code clean up
* 1.02	ana  06/04/2020 Updated PlmkatStatus and Kekstatus variables from
*						initial boot pdi to partial pdi structure variables
*       bsv  06/22/2020 Cfi error handler should only be called for PL image
*       bsv  07/01/2020 Added DevRelease to DevOps
*       kc   07/28/2020 PLM mode is set to configuration during PDI load
*       bsv  07/29/2020 Added support for delay load attribute
*       bsv  08/06/2020 Code clean up
*       bsv  08/10/2020 Added subsystem restart support from DDR
*       har  08/11/2020 Added task to scheduler for checking Authenticated JTAG
*                       interrupt status
*       bsv  08/16/2020 Reinitialized Status variable to XST_FAILURE for
*                       security reasons
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bm   08/19/2020 Added logic to store ImageInfo
*       bsv  08/21/2020 Added XSECURE_TEMPORAL_CHECK macro to add
*                       redundancy in security critical functions
*       bm   09/07/2020 Clear PMC RAM chunk after loading PDIs and reloading images
*       bm   09/21/2020 Added ImageInfo related code and added compatibility
*                       check required for DFx
*       bm   09/24/2020 Added FuncID to RestartImage
*       bsv  09/30/2020 Enable parallel DMA for SBI related boot modes
*       bsv  10/09/2020 Add subsystem restart support for SD raw boot modes
*       bsv  10/13/2020 Code clean up
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"
#include "xloader_qspi.h"
#include "xloader_sbi.h"
#include "xloader_sd.h"
#include "xloader_usb.h"
#include "xloader_ddr.h"
#include "xloader_ospi.h"
#include "xpm_device.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_nodeid.h"
#include "xloader_secure.h"
#ifdef XPLM_SEM
#include "xilsem.h"
#endif
#include "xplmi_err.h"
#include "xplmi_event_logging.h"
#include "xplmi_wdt.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_IMAGE_INFO_TBL_MAX_NUM	(XPLMI_IMAGE_INFO_TBL_BUFFER_LEN / \
						sizeof(XLoader_ImageInfo))

/************************** Function Prototypes ******************************/
static int XLoader_PdiInit(XilPdi* PdiPtr, PdiSrc_t PdiSrc, u64 PdiAddr);
static int XLoader_ReadAndValidateHdrs(XilPdi* PdiPtr, u32 RegVal);
static int XLoader_LoadAndStartSubSystemImages(XilPdi *PdiPtr);
static int XLoader_LoadAndStartSubSystemPdi(XilPdi *PdiPtr);
static void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi);
static int XLoader_IdCodeCheck(const XilPdi_ImgHdrTbl * ImgHdrTbl);
static int XLoader_LoadAndStartSecPdi(XilPdi* PdiPtr);
static int XLoader_VerifyImgInfo(const XLoader_ImageInfo *ImageInfo);
static int XLoader_GetChildRelation(u32 ChildImgID, u32 ParentImgID, u32 *IsChild);
static int XLoader_InvalidateChildImgInfo(u32 ParentImgID, u32 *ChangeCount);
static int XLoader_LoadImage(XilPdi *PdiPtr);
static int XLoader_ReloadImage(u32 ImageId, u32 *FuncID);
static int XLoader_StartImage(XilPdi *PdiPtr);

/************************** Variable Definitions *****************************/
XilPdi SubsystemPdiIns = {0U};
static XilPdi_ATFHandoffParams ATFHandoffParams = {0};
XilPdi* BootPdiPtr = NULL;

/*****************************************************************************/
static const XLoader_DeviceOps DeviceOps[] =
{
	{"JTAG", 0U, XLoader_SbiInit, XLoader_SbiCopy, NULL},  /* JTAG - 0U */
#ifdef XLOADER_QSPI
	{"QSPI24", 0U, XLoader_QspiInit, XLoader_QspiCopy, NULL}, /* QSPI24 - 1U */
	{"QSPI32", 0U, XLoader_QspiInit, XLoader_QspiCopy, NULL}, /* QSPI32- 2U */
#else
	{NULL, 0U, NULL, NULL, NULL},
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_0
	{"SD0", 0U, XLoader_SdInit, XLoader_SdCopy,
		XLoader_SdRelease}, /* SD0 - 3U*/
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
	{NULL, 0U, NULL, NULL, NULL},  /* 4U */
#ifdef XLOADER_SD_1
	{"SD1", 0U, XLoader_SdInit, XLoader_SdCopy,
		XLoader_SdRelease}, /* SD1 - 5U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_1
	{"EMMC", 0U, XLoader_SdInit, XLoader_SdCopy,
		XLoader_SdRelease}, /* EMMC - 6U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_USB
	{"USB", 0U, XLoader_UsbInit, XLoader_UsbCopy, NULL}, /* USB - 7U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_OSPI
	{"OSPI", 0U, XLoader_OspiInit, XLoader_OspiCopy, NULL}, /* OSPI - 8U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
	{NULL, 0U, NULL, NULL, NULL}, /* 9U */
#ifdef XLOADER_SBI
	{"SMAP", 0U, XLoader_SbiInit, XLoader_SbiCopy, NULL}, /* SMAP - 0xA */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
	{NULL, 0U, NULL, NULL, NULL}, /* 0xBU */
	{NULL, 0U, NULL, NULL, NULL}, /* 0xCU */
	{NULL, 0U, NULL, NULL, NULL}, /* 0xDU */
#ifdef XLOADER_SD_1
	{"SD1_LS", 0U, XLoader_SdInit, XLoader_SdCopy,
		XLoader_SdRelease}, /* SD1 LS - 0xEU */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
	{"DDR", 0U, XLoader_DdrInit, XLoader_DdrCopy, NULL}, /* DDR - 0xF */
#ifdef XLOADER_SBI
	{"SBI", 0U, XLoader_SbiInit, XLoader_SbiCopy, NULL}, /* SBI - 0x10 */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
	{NULL, 0U, NULL, NULL, NULL}, /* PCIE - 0x11U */
#ifdef XLOADER_SD_0
	{"SD0_RAW", 0U, XLoader_RawInit, XLoader_RawCopy,
		XLoader_RawRelease}, /* SD0_RAW - 0x12U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_1
	{"SD1_RAW", 0U, XLoader_RawInit, XLoader_RawCopy,
		XLoader_RawRelease}, /* SD1_RAW - 0x13U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_1
	{"EMMC_RAW", 0U, XLoader_RawInit, XLoader_RawCopy,
		XLoader_RawRelease}, /* EMMC_RAW - 0x14U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_1
	{"SD1_LS_RAW", 0U, XLoader_RawInit, XLoader_RawCopy,
		XLoader_RawRelease}, /* SD1_LS_RAW - 0x15U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_1
	{"EMMC_RAW_BP1", 0U, XLoader_RawInit, XLoader_RawCopy,
		XLoader_RawRelease}, /* EMMC_RAW_BP1 - 0x16U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_1
	{"EMMC_RAW_BP2", 0U, XLoader_RawInit, XLoader_RawCopy,
		XLoader_RawRelease}, /* EMMC_RAW_BP2 - 0x17U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_0
	{"EMMC0", 0U, XLoader_SdInit, XLoader_SdCopy,
		XLoader_SdRelease}, /* EMMC0 - 0x18U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
#ifdef XLOADER_SD_0
	{"EMMC0_RAW", 0U, XLoader_RawInit, XLoader_RawCopy,
		XLoader_RawRelease}, /* EMMC0_RAW - 0x19U */
#else
	{NULL, 0U, NULL, NULL, NULL},
#endif
};

/* Image Info Table */
static XLoader_ImageInfoTbl ImageInfoTbl = {
	.TblPtr = (XLoader_ImageInfo *)XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR,
	.Count = 0U,
	.IsBufferFull = FALSE,
};

/*****************************************************************************/
/**
 * @brief	This function initializes the loader instance and registers loader
 * commands with PLM.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_Init(void)
{
	int Status = XST_FAILURE;

	/* Initializes the DMA pointers */
	Status = XPlmi_DmaInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Initialize the loader commands */
	XLoader_CmdsInit();

	/* Initialize the loader interrupts */
	Status = XLoader_IntrInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_CframeInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Adding task to the scheduler to handle Authenticated JTAG message */
	Status = XLoader_AddAuthJtagToScheduler();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the PDI instance with required details
 * and read the meta header.
 *
 * @param	PdiPtr is instance pointer pointing to PDI details
 * @param	PdiSrc is source of PDI. It can be any boot Device or DDR
 * @param	PdiAddr is the address at which PDI is located in the PDI source
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_PdiInit(XilPdi* PdiPtr, PdiSrc_t PdiSrc, u64 PdiAddr)
{
	volatile int Status = XST_FAILURE;
	u32 RegVal = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);
	u64 PdiInitTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
	u32 UPdiSrc = (u32)PdiSrc;
	u32 DeviceFlags = UPdiSrc & XLOADER_PDISRC_FLAGS_MASK;
	u32 SdRawBootVal;

	/*
	 * Update PDI Ptr with source, addr, meta header
	 */
	PdiPtr->PdiSrc = PdiSrc;
	PdiPtr->PdiAddr = PdiAddr;

	/*
	 * Mark PDI loading is started.
	 */
	XPlmi_Out32(PMC_GLOBAL_DONE, XLOADER_PDI_LOAD_STARTED);

	/*
	 * Store address of the structure in PMC_GLOBAL.GLOBAL_GEN_STORAGE4.
	 */
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE4,
		(u32)((UINTPTR) &ATFHandoffParams));

	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) {
		if (((PdiSrc_t)DeviceFlags == XLOADER_PDI_SRC_SD0)
		|| ((PdiSrc_t)DeviceFlags == XLOADER_PDI_SRC_SD1)
		|| ((PdiSrc_t)DeviceFlags == XLOADER_PDI_SRC_SD1_LS)
		|| ((PdiSrc_t)DeviceFlags == XLOADER_PDI_SRC_EMMC)) {
			SdRawBootVal = RegVal & XLOADER_SD_RAWBOOT_MASK;
			if (SdRawBootVal == XLOADER_SD_RAWBOOT_VAL) {
				if ((PdiSrc_t)DeviceFlags == XLOADER_PDI_SRC_SD0) {
					PdiSrc = XLOADER_PDI_SRC_SD0_RAW;
				}
				else if ((PdiSrc_t)DeviceFlags == XLOADER_PDI_SRC_SD1) {
					PdiSrc = XLOADER_PDI_SRC_SD1_RAW;
				}
				else if ((PdiSrc_t)DeviceFlags == XLOADER_PDI_SRC_SD1_LS) {
					PdiSrc = XLOADER_PDI_SRC_SD1_LS_RAW;
				}
				else {
					PdiSrc = XLOADER_PDI_SRC_EMMC_RAW;
				}
			}
			else if (SdRawBootVal == XLOADER_EMMC_BP1_RAW_VAL) {
				PdiSrc = XLOADER_PDI_SRC_EMMC_RAW_BP1;
			}
			else if (SdRawBootVal == XLOADER_EMMC_BP2_RAW_VAL) {
				PdiSrc = XLOADER_PDI_SRC_EMMC_RAW_BP2;
			}
			else {
				/* For MISRA-C compliance */
			}
			PdiPtr->PdiSrc = PdiSrc;
			UPdiSrc = (u32)PdiSrc;
			DeviceFlags = UPdiSrc & XLOADER_PDISRC_FLAGS_MASK;
		}
		RegVal &= ~(XLOADER_SD_RAWBOOT_MASK);
	}

	if ((DeviceFlags >= XPLMI_ARRAY_SIZE(DeviceOps)) ||
		(DeviceOps[DeviceFlags].Init == NULL)) {
		XPlmi_Printf(DEBUG_GENERAL, "Unsupported Boot Mode:"
					"Source:0x%x\n\r", DeviceFlags);
		Status = XPlmi_UpdateStatus(XLOADER_UNSUPPORTED_BOOT_MODE,
					0);
		goto END;
	}

	XPlmi_Printf(DEBUG_GENERAL, "Loading PDI from %s\n\r",
				DeviceOps[DeviceFlags].Name);

	PdiPtr->SlrType = XPlmi_In32(PMC_TAP_SLR_TYPE) &
				PMC_TAP_SLR_TYPE_VAL_MASK;
	if ((PdiPtr->SlrType == XLOADER_SSIT_MASTER_SLR) ||
		(PdiPtr->SlrType == XLOADER_SSIT_MONOLITIC)) {
		XPlmi_Printf(DEBUG_GENERAL, "Monolithic/Master Device\n\r");
		Status = DeviceOps[DeviceFlags].Init(PdiSrc);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (PdiSrc == XLOADER_PDI_SRC_USB) {
			PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;
		}
	}

	PdiPtr->DeviceCopy = DeviceOps[DeviceFlags].Copy;
	PdiPtr->MetaHdr.DeviceCopy = PdiPtr->DeviceCopy;

	Status = XST_FAILURE;
	Status = XLoader_ReadAndValidateHdrs(PdiPtr, RegVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) {
		BootPdiPtr = PdiPtr;
	}

END:
	XPlmi_MeasurePerfTime(PdiInitTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
		"%u.%06u ms: PDI initialization time\n\r",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads bootheader and metaheader and validates them.
 *
 * @param	PdiPtr is instance pointer pointing to PDI details
 * @param	RegVal is the value of the Multiboot register
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ReadAndValidateHdrs(XilPdi* PdiPtr, u32 RegVal)
{
	volatile int Status = XST_FAILURE;
	XLoader_SecureParams SecureParams = {0U};

	/*
	 * Read meta header from PDI source
	 */
	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) {
		XilPdi_ReadBootHdr(&PdiPtr->MetaHdr);
		PdiPtr->ImageNum = 1U;
		PdiPtr->PrtnNum = 1U;
		if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI24) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_OSPI) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SD0_RAW) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SD1_RAW) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_SD1_LS_RAW) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_EMMC_RAW) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_EMMC_RAW_BP1) ||
			(PdiPtr->PdiSrc == XLOADER_PDI_SRC_EMMC_RAW_BP2)) {

			PdiPtr->MetaHdr.FlashOfstAddr = PdiPtr->PdiAddr + \
				((u64)RegVal * XLOADER_IMAGE_SEARCH_OFFSET);
#ifdef XLOADER_QSPI
			if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI24) || \
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32)) {
				Status = XLoader_QspiGetBusWidth(
					PdiPtr->MetaHdr.FlashOfstAddr);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
#endif
		}
		else {
			PdiPtr->MetaHdr.FlashOfstAddr = PdiPtr->PdiAddr;
		}
		/* Update KEK red key availability status */
		XLoader_UpdateKekRdKeyStatus(PdiPtr);
	}
	else {
		PdiPtr->ImageNum = 0U;
		PdiPtr->PrtnNum = 0U;
		PdiPtr->MetaHdr.FlashOfstAddr = PdiPtr->PdiAddr;
		/* Read Boot header */
		XilPdi_ReadBootHdr(&PdiPtr->MetaHdr);
		Status = XPlmi_MemSetBytes(&(PdiPtr->MetaHdr.BootHdr.BootHdrFwRsvd.MetaHdrOfst),
			sizeof(XilPdi_BootHdrFwRsvd), 0U, sizeof(XilPdi_BootHdrFwRsvd));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET, (int)XLOADER_ERR_MEMSET_BOOT_HDR_FW_RSVD);
			goto END;
		}
		PdiPtr->PlmKatStatus |= BootPdiPtr->PlmKatStatus;
		PdiPtr->KekStatus |= BootPdiPtr->KekStatus;
	}

	/* Read image header */
	Status = XilPdi_ReadImgHdrTbl(&PdiPtr->MetaHdr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR_TBL, Status);
		goto END;
	}

	SecureParams.PdiPtr = PdiPtr;
	SecureParams.IsAuthenticated =
		XilPdi_IsAuthEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	SecureParams.IsAuthenticatedTmp =
		XilPdi_IsAuthEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	SecureParams.IsEncrypted =
		XilPdi_IsEncEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	SecureParams.IsEncryptedTmp =
		XilPdi_IsEncEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	if ((SecureParams.IsEncrypted == (u8)TRUE) ||
		(SecureParams.IsEncryptedTmp == (u8)TRUE) ||
		(SecureParams.IsAuthenticated == (u8)TRUE) ||
		(SecureParams.IsAuthenticatedTmp == (u8)TRUE)) {
		SecureParams.SecureEn = (u8)TRUE;
		SecureParams.SecureEnTmp = (u8)TRUE;
	}

	/* Validates if authentication/encryption is compulsory */
	Status = XST_FAILURE;
	Status = XLoader_SecureValidations(&SecureParams);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,"Failed at secure validations\n\r");
		goto END;
	}

	/* Authentication of IHT */
	if ((SecureParams.IsAuthenticated == (u8)TRUE) ||
		(SecureParams.IsAuthenticatedTmp == (u8)TRUE)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_ImgHdrTblAuth,
			&SecureParams);
	}

	/*
	 * Check the validity of Img Hdr Table fields
	 */
	Status = XilPdi_ValidateImgHdrTbl(&(PdiPtr->MetaHdr.ImgHdrTbl));
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "Image Header Table Validation "
					"failed\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR_TBL, Status);
		goto END;
	}

	/* Perform IDCODE and Extended IDCODE checks */
	if(XPLMI_PLATFORM == PMC_TAP_VERSION_SILICON) {
		Status = XLoader_IdCodeCheck(&(PdiPtr->MetaHdr.ImgHdrTbl));
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL, "IDCODE Checks failed\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GEN_IDCODE,
						Status);
			goto END;
		}
	}

	/*
	 * Read and verify image headers and partition headers
	 */
	if (SecureParams.SecureEn != (u8)TRUE) {
		PdiPtr->MetaHdr.Flag = XILPDI_METAHDR_RD_HDRS_FROM_DEVICE;
		Status = XilPdi_ReadAndVerifyImgHdr(&(PdiPtr->MetaHdr));
		if (XST_SUCCESS != Status) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR, Status);
			goto END;
		}

		Status = XilPdi_ReadAndVerifyPrtnHdr(&PdiPtr->MetaHdr);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTNHDR, Status);
			goto END;
		}
	}
	else {
		Status = XLoader_ReadAndVerifySecureHdrs(&SecureParams,
					&(PdiPtr->MetaHdr));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_SECURE_METAHDR,
						Status);
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to load and start images. It reads
 * meta header, loads the images as present in the PDI and starts images based
 * on hand-off information present in PDI.
 *
 * @param	PdiPtr is Pdi instance pointer
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadAndStartSubSystemImages(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	u32 NoOfDelayedHandoffCpus = 0U;
	u32 DelayHandoffImageNum[XLOADER_MAX_HANDOFF_CPUS] = {0U};
	u32 DelayHandoffPrtnNum[XLOADER_MAX_HANDOFF_CPUS] = {0U};
	u32 Index = 0U;
	u32 ImageNum;
	u32 PrtnNum;
	u32 PrtnIndex;
	u32 UPdiSrc = (u32)(PdiPtr->PdiSrc);
	u32 DeviceFlags = UPdiSrc & XLOADER_PDISRC_FLAGS_MASK;
	u8 DdrRequested = (u8)FALSE;
	u32 Pm_CapAccess = (u32)PM_CAP_ACCESS;
	u32 Pm_CapContext = (u32)PM_CAP_CONTEXT;

	/*
	 * From the meta header present in PDI pointer, read the subsystem
	 * image details and load, start all the images
	 *
	 * For every image,
	 *   1. Read the CDO file if present
	 *   2. Send the CDO file to cdo parser which directs
	 *      CDO commands to Xilpm, and other components
	 *   3. Load partitions to respective memories
	 */
	for ( ; PdiPtr->ImageNum < PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs;
			++PdiPtr->ImageNum) {
		if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) {
			PdiPtr->CopyToMem = XilPdi_GetCopyToMemory(
				&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum]) >>
				XILPDI_IH_ATTRIB_COPY_MEMORY_SHIFT;

			if (PdiPtr->CopyToMem == (u8)TRUE) {
				if (DdrRequested == (u8)FALSE) {
					Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_DDR_0,
						Pm_CapAccess | Pm_CapContext, XPM_DEF_QOS, 0U);
					if (Status != XST_SUCCESS) {
						Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_DDR_0, 0);
						goto END;
					}
					DdrRequested = (u8)TRUE;
				}
				PdiPtr->CopyToMemAddr =
						PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].CopyToMemoryAddr;
			}
		}
		else {
			PdiPtr->CopyToMem = (u8)FALSE;
		}

		PdiPtr->DelayHandoff = XilPdi_GetDelayHandoff(
			&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum]) >>
			XILPDI_IH_ATTRIB_DELAY_HANDOFF_SHIFT;
		PdiPtr->DelayLoad = XilPdi_GetDelayLoad(
			&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum]) >>
			XILPDI_IH_ATTRIB_DELAY_LOAD_SHIFT;

		if (PdiPtr->DelayHandoff == (u8)TRUE) {
			if (PdiPtr->DelayLoad == (u8)TRUE) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_DELAY_ATTRB, 0);
				goto END;
			}

			if (NoOfDelayedHandoffCpus == XLOADER_MAX_HANDOFF_CPUS) {
				Status = XPlmi_UpdateStatus(
						XLOADER_ERR_NUM_HANDOFF_CPUS, 0);
				goto END;
			}
			DelayHandoffImageNum[NoOfDelayedHandoffCpus] =
					PdiPtr->ImageNum;
			DelayHandoffPrtnNum[NoOfDelayedHandoffCpus] =
					PdiPtr->PrtnNum;
			NoOfDelayedHandoffCpus += 1U;
		}

		Status = XLoader_LoadImage(PdiPtr);
		if (Status != XST_SUCCESS) {
			/* Check for Cfi errors */
			if (NODESUBCLASS(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID)
					== (u32)XPM_NODESUBCL_DEV_PL) {
				/*
				 * To preserve the PDI error, ignoring the
				 * Error returned by XLoader_CframeErrorHandler
				 */
				(void)XLoader_CframeErrorHandler(
				PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
			}
			goto END;
		}

		if ((PdiPtr->DelayLoad == (u8)TRUE) ||
			(PdiPtr->DelayHandoff == (u8)TRUE)) {
			continue;
		}

		Status = XLoader_StartImage(PdiPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/* Delay Handoff starts here */
	for ( ; Index < NoOfDelayedHandoffCpus; ++Index) {
		ImageNum = DelayHandoffImageNum[Index];
		PrtnNum = DelayHandoffPrtnNum[Index];
		PdiPtr->PrtnNum = PrtnNum;
		for (PrtnIndex = 0U;
			PrtnIndex < PdiPtr->MetaHdr.ImgHdr[ImageNum].NoOfPrtns;
			PrtnIndex++) {
			Status = XLoader_UpdateHandoffParam(PdiPtr);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			PdiPtr->PrtnNum++;
		}

		Status = XLoader_StartImage(PdiPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	if (DeviceOps[DeviceFlags].Release != NULL) {
		SStatus = DeviceOps[DeviceFlags].Release();
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}

#ifndef PLM_DEBUG_MODE
	SStatus = XPlmi_MemSet(XPLMI_PMCRAM_CHUNK_MEMORY, XPLMI_DATA_INIT_PZM,
			XLOADER_CHUNK_SIZE / XPLMI_WORD_LEN);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
#endif

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to load the primary and secondary PDIs.
 *
 * @param	PdiPtr is Pdi instance pointer
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadAndStartSubSystemPdi(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;

	Status = XLoader_LoadAndStartSubSystemImages(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_LoadAndStartSecPdi(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Mark PDI loading is completed */
	XPlmi_Out32(PMC_GLOBAL_DONE, XLOADER_PDI_LOAD_COMPLETE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides loading PDI.
 *
 * @param	PdiPtr is the instance pointer that points to PDI details
 * @param	PdiSrc is source of PDI. It can be in Boot Device or DDR.
 * @param	PdiAddr is the address at PDI is located in the PDI source
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_LoadPdi(XilPdi* PdiPtr, PdiSrc_t PdiSrc, u64 PdiAddr)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	XPlmi_SetPlmMode(XPLMI_MODE_CONFIGURATION);

	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PdiInit, PdiPtr,
			PdiSrc, PdiAddr);

	PdiSrc = PdiPtr->PdiSrc;
	Status = XLoader_LoadAndStartSubSystemPdi(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	/* Reset the SBI/DMA to clear the buffers */
	if ((PdiSrc == XLOADER_PDI_SRC_JTAG) ||
			(PdiSrc == XLOADER_PDI_SRC_SBI)) {
		XLoader_SbiRecovery();
	}
	XPlmi_SetPlmMode(XPLMI_MODE_OPERATIONAL);
	return Status;
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
static int XLoader_StartImage(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 CpuId;
	u64 HandoffAddr;
	u32 ExecState;
	u32 VInitHi;

	/* Handoff to the cpus */
	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++) {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;

		HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;
		ExecState = PdiPtr->HandoffParam[Index].CpuSettings &
				XIH_PH_ATTRB_A72_EXEC_ST_MASK;
		VInitHi = PdiPtr->HandoffParam[Index].CpuSettings &
				XIH_PH_ATTRB_HIVEC_MASK;

		switch (CpuId)
		{
			case XIH_PH_ATTRB_DSTN_CPU_A72_0:
				/* APU Core configuration */
				XLoader_A72Config(CpuId, ExecState, VInitHi);
				XLoader_Printf(DEBUG_INFO, "Request APU0 "
						"wakeup\r\n");
				Status = XPm_RequestWakeUp(PM_SUBSYS_PMC,
					PM_DEV_ACPU_0, 1U, HandoffAddr, 0U);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_WAKEUP_A72_0, Status);
					goto END;
				}
				break;
			case XIH_PH_ATTRB_DSTN_CPU_A72_1:
				/* APU Core configuration */
				XLoader_A72Config(CpuId, ExecState, VInitHi);
				XLoader_Printf(DEBUG_INFO, "Request APU1"
						"wakeup\r\n");
				Status = XPm_RequestWakeUp(PM_SUBSYS_PMC,
					PM_DEV_ACPU_1, 1U, HandoffAddr, 0U);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_WAKEUP_A72_1, Status);
					goto END;
				}
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_0:
				XLoader_Printf(DEBUG_INFO, "Request RPU 0 "
						"wakeup\r\n");
				Status = XPm_RequestWakeUp(PM_SUBSYS_PMC,
					PM_DEV_RPU0_0, 1U, HandoffAddr, 0U);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_WAKEUP_R5_0, Status);
					goto END;
				}
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_1:
				XLoader_Printf(DEBUG_INFO, "Request RPU 1 "
						"wakeup\r\n");
				Status = XPm_RequestWakeUp(PM_SUBSYS_PMC,
					PM_DEV_RPU0_1, 1U, HandoffAddr, 0U);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_WAKEUP_R5_1, Status);
					goto END;
				}
				break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_L:
				XLoader_Printf(DEBUG_INFO, "Request RPU "
						"wakeup\r\n");
				Status = XPm_RequestWakeUp(PM_SUBSYS_PMC,
					PM_DEV_RPU0_0, 1U, HandoffAddr, 0U);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_WAKEUP_R5_L, Status);
					goto END;
				}
				break;
			case XIH_PH_ATTRB_DSTN_CPU_PSM:
				XLoader_Printf(DEBUG_INFO, "Request PSM wakeup\r\n");
				Status = XPm_RequestWakeUp(PM_SUBSYS_PMC,
						PM_DEV_PSM_PROC, 0U, 0U, 0U);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_WAKEUP_PSM, Status);
					goto END;
				}
				break;

			default:
				Status = XST_SUCCESS;
				break;
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

/*****************************************************************************/
/**
 * @brief	This function sets Aarch state and vector location for APU.
 *
 * @param	CpuId CPU ID
 * @param	ExecState CPU execution state
 * @param	VinitHi resembles highvec configuration for CPU
 *
 * @return	None
 *
 *****************************************************************************/
static void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi)
{
	u32 RegVal;

	RegVal = Xil_In32(XLOADER_FPD_APU_CONFIG_0);

	switch(CpuId)
	{
		case XIH_PH_ATTRB_DSTN_CPU_A72_0:
			/* Set Aarch state 64 Vs 32 bit and vection location for 32 bit */
			if (ExecState == XIH_PH_ATTRB_A72_EXEC_ST_AA64) {
				RegVal |=
				XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU0;
			}
			else {
				RegVal &=
				~(XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU0);

				if (VInitHi == XIH_PH_ATTRB_HIVEC_MASK) {
					RegVal |=
					XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU0;
				} else {
					RegVal &=
					~(XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU0);
				}
			}
			break;
		case XIH_PH_ATTRB_DSTN_CPU_A72_1:
			/* Set Aarch state 64 Vs 32 bit and vection location for 32 bit */
			if (ExecState == XIH_PH_ATTRB_A72_EXEC_ST_AA64) {
				RegVal |=
				XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU1;
			}
			else {
				RegVal &=
				~(XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU1);

				if (VInitHi == XIH_PH_ATTRB_HIVEC_MASK) {
					RegVal |=
					XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU1;
				}
				else {
					RegVal &=
					~(XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU1);
				}
			}
			break;
		default:
			break;
	}

	/* Update the APU configuration */
	XPlmi_Out32(XLOADER_FPD_APU_CONFIG_0, RegVal);
}

/*****************************************************************************/
/**
 * @brief	This function checks if the given ImgID is a child of Parent ImgID
 *
 * @param	ChildImgID is the current ImgID whose relation is to be checked
 * @param	ParentImgID is the ImgID with which the hierarchical ImgIDs are
 * compared in order to get the correct parent-child relationship
 * @param	IsChild is the pointer to the Relation that has to be
 *		returned, TRUE if it is a Child, else FALSE
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_GetChildRelation(u32 ChildImgID, u32 ParentImgID, u32 *IsChild)
{
	int Status = XST_FAILURE;
	u32 TempImgID;
	u32 DummyArg = 0U;
	u32 TempParentImgID;

	TempImgID = ChildImgID;
	while (1U) {
		Status = XPm_Query((u32)XPM_QID_PLD_GET_PARENT,
				TempImgID, DummyArg, DummyArg,
				&TempParentImgID);
		if (Status != XST_SUCCESS) {
			Status = XLOADER_ERR_PARENT_QUERY_RELATION_CHECK;
			goto END;
		}

		if (TempParentImgID == 0U) {
			*IsChild = FALSE;
			break;
		}

		if (TempParentImgID == ParentImgID) {
			*IsChild = TRUE;
			break;
		}
		TempImgID = TempParentImgID;
	}

	Status = XST_SUCCESS;

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function invalidates the child image info entries
 * corresponding to the parent image id in the ImageInfo Table
 *
 * @param	ParentImgID whose corresponding child image info entries are
 * invalidated
 * @param	Pointer to ChangeCount that has to be modified
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_InvalidateChildImgInfo(u32 ParentImgID, u32 *ChangeCount)
{
	int Status = XST_FAILURE;
	u32 TempCount = 0;
	u32 IsChild;
	u32 Index;

	for (Index = 0U; Index < XLOADER_IMAGE_INFO_TBL_MAX_NUM; Index++) {
		if (TempCount >= ImageInfoTbl.Count) {
			break;
		}
		if (NODESUBCLASS(ImageInfoTbl.TblPtr[Index].ImgID) !=
			(u32)XPM_NODESUBCL_DEV_PL) {
			continue;
		}
		Status = XLoader_GetChildRelation(
				ImageInfoTbl.TblPtr[Index].ImgID,
				ParentImgID, &IsChild);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (IsChild == TRUE) {
			ImageInfoTbl.TblPtr[Index].ImgID = XLOADER_INVALID_IMG_ID;
			ImageInfoTbl.Count--;
			(*ChangeCount)++;
			if (ImageInfoTbl.IsBufferFull == TRUE) {
				ImageInfoTbl.IsBufferFull = FALSE;
			}
		}
		else if (ImageInfoTbl.TblPtr[Index].ImgID != XLOADER_INVALID_IMG_ID) {
			TempCount++;
		}
		else {
			/* For MISRA-C */
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns the ImageInfoEntry by checking if an entry
 * exists for that particular ImgId in the ImgInfoTbl
 *
 * @param	ImgId of the the entry that has to be stored
 *
 * @return	Address of ImageInfo Entry in the table
 *
 *****************************************************************************/
XLoader_ImageInfo* XLoader_GetImageInfoEntry(u32 ImgID)
{
	XLoader_ImageInfo *ImageEntry = NULL;
	u32 Index;
	u32 EmptyImgIndex = (u32)XLOADER_IMG_INDEX_NOT_FOUND;
	u32 TempCount = 0U;

	/* Check for a existing valid image entry matching given ImgID */
	for (Index = 0U; Index < XLOADER_IMAGE_INFO_TBL_MAX_NUM; Index++) {
		if (TempCount < ImageInfoTbl.Count) {
			TempCount++;
			if ((ImageInfoTbl.TblPtr[Index].ImgID == ImgID) &&
				(ImageInfoTbl.TblPtr[Index].ImgID !=
				XLOADER_INVALID_IMG_ID)) {
				ImageEntry = &ImageInfoTbl.TblPtr[Index];
				goto END;
			}
			else if ((ImageInfoTbl.TblPtr[Index].ImgID ==
				XLOADER_INVALID_IMG_ID) && (EmptyImgIndex ==
				(u32)XLOADER_IMG_INDEX_NOT_FOUND)) {
				EmptyImgIndex = Index;
			}
			else {
				/* For MISRA-C */
			}
		}
	}

	/* If no valid image entry is found above, return empty entry */
	if ((Index == XLOADER_IMAGE_INFO_TBL_MAX_NUM) && (ImageInfoTbl.Count <
		XLOADER_IMAGE_INFO_TBL_MAX_NUM)) {
		if (EmptyImgIndex == (u32)XLOADER_IMG_INDEX_NOT_FOUND) {
			EmptyImgIndex = ImageInfoTbl.Count;
		}
		ImageEntry = &ImageInfoTbl.TblPtr[EmptyImgIndex];
		ImageEntry->ImgID = XLOADER_INVALID_IMG_ID;
	}

END:
	return ImageEntry;
}

/*****************************************************************************/
/**
 * @brief	This function stores the ImageInfo to Image Info Table
 *
 * @param	Pointer to ImageInfo that has to be written.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_StoreImageInfo(XLoader_ImageInfo *ImageInfo)
{
	int Status = XST_FAILURE;
	XLoader_ImageInfo *ImageEntry;
	u32 ChangeCount;
	u32 RtCfgLen;

	/* Read ChangeCount */
	ChangeCount = ((XPlmi_In32(XPLMI_RTCFG_IMGINFOTBL_LEN_ADDR) &
			XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_MASK)
			>> XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_SHIFT);

	if (NODESUBCLASS(ImageInfo->ImgID) == (u32)XPM_NODESUBCL_DEV_PL) {
		Status = XLoader_InvalidateChildImgInfo(ImageInfo->ImgID,
				&ChangeCount);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_INVALIDATE_CHILD_IMG, Status);
			goto END;
		}
	}

	ImageEntry = XLoader_GetImageInfoEntry(ImageInfo->ImgID);
	if (ImageEntry == NULL) {
		ImageInfoTbl.IsBufferFull = TRUE;
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMAGE_INFO_TBL_OVERFLOW, 0);
		goto END;
	}

	if (ImageEntry->ImgID == XLOADER_INVALID_IMG_ID) {
		ImageInfoTbl.Count++;
	}
	ChangeCount++;
	(void)memcpy(ImageEntry, ImageInfo, sizeof(XLoader_ImageInfo));

	/* Update ChangeCount and number of entries in the RunTime config register */
	RtCfgLen = (ImageInfoTbl.Count & XPLMI_RTCFG_IMGINFOTBL_NUM_ENTRIES_MASK);
	RtCfgLen |= ((ChangeCount << XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_SHIFT) &
				XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_MASK);
	XPlmi_Out32(XPLMI_RTCFG_IMGINFOTBL_LEN_ADDR, RtCfgLen);

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function loads the ImageInfo table to the given memory address
 *
 * @param	64 bit Destination Address
 * @param	Max Size of Buffer present at Destination Address
 * @param	NumEntries that are loaded from the Image Info Table
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_LoadImageInfoTbl(u64 DestAddr, u32 MaxSize, u32 *NumEntries)
{
	int Status = XST_FAILURE;
	u32 Len = ImageInfoTbl.Count;
	u32 MaxLen = MaxSize / sizeof(XLoader_ImageInfo);
	u32 Count = 0U;
	u32 Index = 0U;
	u64 SrcAddr = (u64)XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR;
	XLoader_ImageInfo *ImageInfo;

	if (Len > MaxLen) {
		Len = MaxLen;
	}

	while (Count < Len) {
		if (Index >= XLOADER_IMAGE_INFO_TBL_MAX_NUM) {
			break;
		}
		ImageInfo = (XLoader_ImageInfo *) (u32)SrcAddr;
		if (ImageInfo->ImgID != XLOADER_INVALID_IMG_ID) {
			Status = XPlmi_DmaXfr(SrcAddr, DestAddr,
					sizeof(XLoader_ImageInfo) / XPLMI_WORD_LEN,
					XPLMI_PMCDMA_0);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Count++;
			DestAddr += sizeof(XLoader_ImageInfo);
		}
		Index++;
		SrcAddr += sizeof(XLoader_ImageInfo);
	}

	if (ImageInfoTbl.IsBufferFull == TRUE) {
		Status = XLOADER_ERR_IMAGE_INFO_TBL_FULL;
		XPlmi_Printf(DEBUG_INFO, "Image Info Table Overflowed\r\n");
	}
	*NumEntries = Len;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function validates the UIDs in Image Header
*
* @param	ImageInfo is pointer to the image info in image
*
* @return	XST_SUCCESS on success and error code on failure
*
*****************************************************************************/
static int XLoader_VerifyImgInfo(const XLoader_ImageInfo *ImageInfo)
{
	int Status = XST_FAILURE;
	XPm_DeviceStatus DeviceStatus;
	XLoader_ImageInfo *ParentImageInfo;
	u32 DummyArg = 0U;
	u32 ParentImgID;

	if ((ImageInfo->ImgID != XLOADER_INVALID_IMG_ID) &&
		(ImageInfo->UID != XLOADER_INVALID_UID) &&
		(NODESUBCLASS(ImageInfo->ImgID) == (u32)XPM_NODESUBCL_DEV_PL)) {
		Status = XPmDevice_GetStatus(PM_SUBSYS_PMC, ImageInfo->ImgID,
				&DeviceStatus);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_DEV_NOT_DEFINED,
					Status);
			goto END;
		}

		if (ImageInfo->PUID != XLOADER_INVALID_UID) {
			Status = XPm_Query((u32)XPM_QID_PLD_GET_PARENT,
					ImageInfo->ImgID, DummyArg, DummyArg,
					&ParentImgID);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_PARENT_QUERY_VERIFY, Status);
				goto END;
			}

			if (ParentImgID == XLOADER_INVALID_IMG_ID) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_INVALID_PARENT_IMG_ID, 0);
				goto END;
			}

			ParentImageInfo = XLoader_GetImageInfoEntry(ParentImgID);
			if (ParentImageInfo == NULL) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_NO_VALID_PARENT_IMG_ENTRY, 0);
				goto END;
			}

			if (ParentImageInfo->UID != ImageInfo->PUID) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_INCOMPATIBLE_CHILD_IMAGE, 0);
				XPlmi_Printf(DEBUG_GENERAL, "Image is not "
					"compatible with Parent Image\r\n");
				goto END;
			}
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used load a image in PDI. PDI can have multiple
 * images present in it. This can be used to load a single image like PL, APU, RPU.
 * This will load all the partitions that are present in that image.
 *
 * @param	PdiPtr is Pdi instance pointer
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadImage(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;
	XLoader_ImageInfo ImageInfo;

#if defined(XPLM_SEM) && defined(XSEM_CFRSCAN_EN)
	/* Stop the SEM scan before PL load */
	if ((PdiPtr->PdiType != XLOADER_PDI_TYPE_FULL) &&
		(NODESUBCLASS(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID)
		== (u32)XPM_NODESUBCL_DEV_PL)) {
		Status = XSem_CfrStopScan();
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_SEM_STOP_SCAN, Status);
			goto END;
		}
	}
#endif

	ImageInfo.ImgID = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID;
	ImageInfo.UID = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].UID;
	ImageInfo.PUID = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].PUID;
	ImageInfo.FuncID = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].FuncID;

	Status = XLoader_VerifyImgInfo(&ImageInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Configure preallocs for subsystem */
	if (NODECLASS(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID)
			== (u32)XPM_NODECLASS_SUBSYSTEM) {
		Status = XPmSubsystem_Configure(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_CONFIG_SUBSYSTEM, Status);
			goto END;
		}
	}

	PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName[3U] = 0U;
	PdiPtr->CurImgId = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID;
	/* Update current subsystem ID for EM */
	EmSubsystemId = PdiPtr->CurImgId;
	Status = XLoader_LoadImagePrtns(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((PdiPtr->DelayLoad == (u8)FALSE) &&
		(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID !=
		XLOADER_INVALID_IMG_ID)) {
		Status = XLoader_StoreImageInfo(&ImageInfo);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	/* Log the image load to the Trace Log buffer */
	XPlmi_TraceLog3(XPLMI_TRACE_LOG_LOAD_IMAGE, PdiPtr->CurImgId);

#if defined(XPLM_SEM) && defined(XSEM_CFRSCAN_EN)
	/* Resume the SEM scan after PL load */
	if ((PdiPtr->PdiType != XLOADER_PDI_TYPE_FULL) &&
		(NODESUBCLASS(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID)
		== (u32)XPM_NODESUBCL_DEV_PL)) {
		Status = XSem_CfrInit();
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_SEM_CFR_INIT, Status);
			goto END;
		}
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to restart the image in PDI. This function
 * will take ImageId as an input and based on the subsystem info available, it
 * will read the image partitions, loads them and hand-off to the required CPUs
 * as part of the image load.
 *
 * @param	ImageId Id of the image present in PDI
 * @param	FuncID is verified with the FuncID present in PDI
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_RestartImage(u32 ImageId, u32 *FuncID)
{
	int Status = XST_FAILURE;

	Status = XLoader_ReloadImage(ImageId, FuncID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_StartImage(BootPdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to reload the image only in PDI. This function will
 * take ImageId as an input and based on the subsystem info available, it will
 * read the image partitions and loads them.
 *
 * @param	ImageId Id of the image present in PDI
 * @param	FuncID is verified with the FuncID present in PDI
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_ReloadImage(u32 ImageId, u32 *FuncID)
{
	int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	XilPdi* PdiPtr = BootPdiPtr;
	PdiSrc_t PdiSrc = PdiPtr->PdiSrc;
	u32 UPdiSrc = (u32)(PdiPtr->PdiSrc);
	u32 DeviceFlags = UPdiSrc & XLOADER_PDISRC_FLAGS_MASK;
	u32 PrtnNum = 0U;
	u32 Index = 0U;
	u32 Pm_CapAccess = (u32)PM_CAP_ACCESS;

	for (Index = 0U; Index < PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs;
		++Index) {
		if (PdiPtr->MetaHdr.ImgHdr[Index].ImgID == ImageId) {
			PdiPtr->ImageNum = Index;
			PdiPtr->PrtnNum = PrtnNum;
			break;
		}
		PrtnNum += PdiPtr->MetaHdr.ImgHdr[Index].NoOfPrtns;
	}
	if (Index == PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMG_ID_NOT_FOUND, 0);
		goto END;
	}

	if (FuncID != NULL) {
		if (PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].FuncID != *FuncID) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_FUNCTION_ID_MISMATCH, 0);
			goto END;
		}
	}

	PdiPtr->CopyToMem = XilPdi_GetCopyToMemory(
		&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum]) >>
		XILPDI_IH_ATTRIB_COPY_MEMORY_SHIFT;
	if (PdiPtr->CopyToMem == (u8)TRUE) {
		PdiPtr->PdiSrc = XLOADER_PDI_SRC_DDR;
		UPdiSrc = (u32)(PdiPtr->PdiSrc);
		DeviceFlags = UPdiSrc & XLOADER_PDISRC_FLAGS_MASK;
		PdiPtr->PdiType = XLOADER_PDI_TYPE_RESTORE;
		PdiPtr->CopyToMem = (u8)FALSE;
		PdiPtr->CopyToMemAddr =
				PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].CopyToMemoryAddr;
	}
	PdiPtr->DelayHandoff = (u16)FALSE;
	PdiPtr->DelayLoad = (u8)FALSE;

	/*
	 * This is for libpm to do the clock settings reqired for boot device
	 * to resume post suspension.
	 */
	switch((PdiSrc_t)DeviceFlags)
	{
		case XLOADER_PDI_SRC_QSPI24:
		case XLOADER_PDI_SRC_QSPI32:
			Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_QSPI,
				Pm_CapAccess, XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_QSPI, 0);
				goto END;
			}
			break;
		case XLOADER_PDI_SRC_SD0:
		case XLOADER_PDI_SRC_SD0_RAW:
			Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_SDIO_0,
				Pm_CapAccess, XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_SDIO_0, 0);
				goto END;
			}
			break;
		case XLOADER_PDI_SRC_SD1:
		case XLOADER_PDI_SRC_EMMC:
		case XLOADER_PDI_SRC_SD1_LS:
		case XLOADER_PDI_SRC_SD1_RAW:
		case XLOADER_PDI_SRC_EMMC_RAW:
		case XLOADER_PDI_SRC_EMMC_RAW_BP1:
		case XLOADER_PDI_SRC_EMMC_RAW_BP2:
		case XLOADER_PDI_SRC_SD1_LS_RAW:
			Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_SDIO_1,
				Pm_CapAccess, XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_SDIO_1, 0);
				goto END;
			}
			break;
		case XLOADER_PDI_SRC_USB:
			Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_USB_0,
				Pm_CapAccess, XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_USB_0, 0);
				goto END;
			}
			break;
		case XLOADER_PDI_SRC_OSPI:
			Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_OSPI,
				Pm_CapAccess, XPM_DEF_QOS, 0U);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_OSPI, 0);
				goto END;
			}
			break;
		default:
			Status = XST_SUCCESS;
			break;
	}

	if (DeviceOps[DeviceFlags].Init != NULL) {
		Status = DeviceOps[DeviceFlags].Init(PdiPtr->PdiSrc);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	PdiPtr->DeviceCopy = DeviceOps[DeviceFlags].Copy;
	PdiPtr->MetaHdr.DeviceCopy = DeviceOps[DeviceFlags].Copy;

	Status = XLoader_LoadImage(PdiPtr);
	if (DeviceOps[DeviceFlags].Release != NULL) {
		SStatus = DeviceOps[DeviceFlags].Release();
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	switch((PdiSrc_t)DeviceFlags)
	{
		case XLOADER_PDI_SRC_QSPI24:
		case XLOADER_PDI_SRC_QSPI32:
			SStatus = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_QSPI);
			break;
		case XLOADER_PDI_SRC_SD0:
		case XLOADER_PDI_SRC_SD0_RAW:
			SStatus = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_SDIO_0);
			break;
		case XLOADER_PDI_SRC_SD1:
		case XLOADER_PDI_SRC_EMMC:
		case XLOADER_PDI_SRC_SD1_LS:
		case XLOADER_PDI_SRC_SD1_RAW:
		case XLOADER_PDI_SRC_EMMC_RAW:
		case XLOADER_PDI_SRC_EMMC_RAW_BP1:
		case XLOADER_PDI_SRC_EMMC_RAW_BP2:
		case XLOADER_PDI_SRC_SD1_LS_RAW:
			SStatus = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_SDIO_1);
			break;
		case XLOADER_PDI_SRC_USB:
			SStatus = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_USB_0);
			break;
		case XLOADER_PDI_SRC_OSPI:
			SStatus = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_OSPI);
			break;
		default:
			SStatus = XST_SUCCESS;
			break;
	}
	if ((Status == XST_SUCCESS) && (SStatus != XST_SUCCESS)) {
		Status = SStatus;
	}

	XPlmi_SetPlmMode(XPLMI_MODE_OPERATIONAL);
	PdiPtr->PdiSrc = PdiSrc;
	PdiPtr->PdiType = XLOADER_PDI_TYPE_FULL;

#ifndef PLM_DEBUG_MODE
	SStatus = XPlmi_MemSet(XPLMI_PMCRAM_CHUNK_MEMORY, XPLMI_DATA_INIT_PZM,
			XLOADER_CHUNK_SIZE / XPLMI_WORD_LEN);
	if ((Status == XST_SUCCESS) && (SStatus != XST_SUCCESS)) {
		Status = SStatus;
	}
#endif

	return Status;
}

/****************************************************************************/
/**
*  @brief	This function performs the checks of IDCODE and EXTENDED IDCODE.
*  It also supports bypass of subset of these checks.
*
* @param	ImgHdrTbl pointer to the image header table.
*
* @return	XST_SUCCESS on success and error code on failure
*
*****************************************************************************/
static int XLoader_IdCodeCheck(const XilPdi_ImgHdrTbl * ImgHdrTbl)
{
	int Status = XST_FAILURE;
	XLoader_IdCodeInfo IdCodeInfo;

	IdCodeInfo.IdCodeIHT = ImgHdrTbl->Idcode;
	IdCodeInfo.IdCodeRd = XPlmi_In32(PMC_TAP_IDCODE);
	IdCodeInfo.ExtIdCodeIHT = ImgHdrTbl->ExtIdCode &
				XIH_IHT_EXT_IDCODE_MASK;
	IdCodeInfo.ExtIdCodeRd = Xil_In32(EFUSE_CACHE_IP_DISABLE_0)
			& EFUSE_CACHE_IP_DISABLE_0_EID_MASK;

	/* Determine and fetch the Extended IDCODE (out of two) for checks */
	if (0U == IdCodeInfo.ExtIdCodeRd) {
		IdCodeInfo.IsExtIdCodeZero = (u8)TRUE;
	}
	else {
		IdCodeInfo.IsExtIdCodeZero = (u8)FALSE;

		if ((IdCodeInfo.ExtIdCodeRd &
			EFUSE_CACHE_IP_DISABLE_0_EID_SEL_MASK) == 0U) {
			IdCodeInfo.ExtIdCodeRd = (IdCodeInfo.ExtIdCodeRd &
				EFUSE_CACHE_IP_DISABLE_0_EID1_MASK)
				>> EFUSE_CACHE_IP_DISABLE_0_EID1_SHIFT;
		}
		else {
			IdCodeInfo.ExtIdCodeRd = (IdCodeInfo.ExtIdCodeRd &
					EFUSE_CACHE_IP_DISABLE_0_EID2_MASK)
					>> EFUSE_CACHE_IP_DISABLE_0_EID2_SHIFT;
		}
	}

	/* Check if VC1902 ES1 */
	if ((IdCodeInfo.IdCodeRd & PMC_TAP_IDCODE_SIREV_DVCD_MASK) ==
			PMC_TAP_IDCODE_ES1_VC1902) {
		IdCodeInfo.IsVC1902Es1 = (u8)TRUE;
	}
	else {
		IdCodeInfo.IsVC1902Es1 = (u8)FALSE;
	}

	/* Check if a subset of checks to be bypassed */
	if (0x1U == (ImgHdrTbl->Attr & XIH_IHT_ATTR_BYPS_MASK)) {
		IdCodeInfo.BypassChkIHT = (u8)TRUE;
	}
	else {
		IdCodeInfo.BypassChkIHT = (u8)FALSE;
	}

	/*
	 *  EXT_IDCODE
	 *  [26:14]is 0?  VC1902-ES1?  BYPASS?  Checks done
	 *  --------------------------------------------------------------------
	 *  		Y				Y		Y	Check IDCODE[27:0] (skip Si Rev chk)
	 *  		Y				Y		N	Check IDCODE[31:0]
	 *  		Y				N		X	Invalid combination (Error out)
	 *  		N				X		Y	Check IDCODE[27:0] (skip Si Rev chk),
	 *										check ext_idcode
	 *  		N				X		N	Check IDCODE[31:0], check ext_idcode
	 *  --------------------------------------------------------------------
	 */

	/*
	 * Error out for the invalid combination of Extended IDCODE - Device.
	 * Assumption is that only VC1902-ES1 device can have Extended IDCODE value 0
	 */
	if ((IdCodeInfo.IsExtIdCodeZero == (u8)TRUE) &&
			(IdCodeInfo.IsVC1902Es1 == (u8)FALSE)) {
		Status = XLOADER_ERR_EXT_ID_SI;
		goto END;
	}
	else {
		/* Do not check Si revision if bypass configured */
		if ((u8)TRUE == IdCodeInfo.BypassChkIHT) {
			IdCodeInfo.IdCodeIHT &= ~PMC_TAP_IDCODE_SI_REV_MASK;
			IdCodeInfo.IdCodeRd &= ~PMC_TAP_IDCODE_SI_REV_MASK;
		}

		/* Do the actual IDCODE check */
		if (IdCodeInfo.IdCodeIHT != IdCodeInfo.IdCodeRd) {
			Status = XLOADER_ERR_IDCODE;
			goto END;
		}

		/* Do the actual Extended IDCODE check */
		if ((u8)FALSE == IdCodeInfo.IsExtIdCodeZero) {
			if (IdCodeInfo.ExtIdCodeIHT != IdCodeInfo.ExtIdCodeRd) {
				Status = XLOADER_ERR_EXT_IDCODE;
				goto END;
			}
		}
	}

	Status = XST_SUCCESS;

END:
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
	u32 PrtnFlags = 0U;
	u32 LoopCount = 0U;

	PrtnAttrbs = PrtnHdr->PrtnAttrb;

	PrtnFlags =
		(((PrtnAttrbs & XIH_PH_ATTRB_A72_EXEC_ST_MASK)
				>> XIH_ATTRB_A72_EXEC_ST_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_ENDIAN_MASK)
				>> XIH_ATTRB_ENDIAN_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_TZ_SECURE_MASK)
				<< XIH_ATTRB_TR_SECURE_SHIFT_DIFF) |
		((PrtnAttrbs & XIH_PH_ATTRB_TARGET_EL_MASK)
				<< XIH_ATTRB_TARGET_EL_SHIFT_DIFF));

	/* Update CPU number based on destination CPU */
	if ((PrtnAttrbs & XIH_PH_ATTRB_DSTN_CPU_MASK)
			== XIH_PH_ATTRB_DSTN_CPU_A72_0) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A72_0;
	}
	else if ((PrtnAttrbs & XIH_PH_ATTRB_DSTN_CPU_MASK)
			== XIH_PH_ATTRB_DSTN_CPU_A72_1) {
		PrtnFlags |= XIH_PRTN_FLAGS_DSTN_CPU_A72_1;
	}
	else if ((PrtnAttrbs & XIH_PH_ATTRB_DSTN_CPU_MASK)
			== XIH_PH_ATTRB_DSTN_CPU_NONE) {
		/*
		 * This is required for u-boot handoff to work
		 * when BOOTGEN_SUBSYSTEM_PDI is set to 0 in bootgen
		 */
		PrtnFlags &= (~(XIH_ATTRB_EL_MASK) | XIH_PRTN_FLAGS_EL_2)
					| XIH_PRTN_FLAGS_DSTN_CPU_A72_0;
	}
	else {
		/* MISRA-C compliance */
	}

	if (ATFHandoffParams.NumEntries == 0U) {
		/* Insert magic string */
		ATFHandoffParams.MagicValue[0U] = 'X';
		ATFHandoffParams.MagicValue[1U] = 'L';
		ATFHandoffParams.MagicValue[2U] = 'N';
		ATFHandoffParams.MagicValue[3U] = 'X';
	}
	else {
		for (; LoopCount < ATFHandoffParams.NumEntries; LoopCount++) {
			if (ATFHandoffParams.Entry[LoopCount].PrtnFlags ==
					PrtnFlags) {
				ATFHandoffParams.Entry[LoopCount].EntryPoint =
					PrtnHdr->DstnExecutionAddr;
				break;
			}
		}
	}

	if ((ATFHandoffParams.NumEntries < XILPDI_MAX_ENTRIES_FOR_ATF) &&
		(ATFHandoffParams.NumEntries == LoopCount)) {
		ATFHandoffParams.NumEntries += 1U;
		ATFHandoffParams.Entry[LoopCount].EntryPoint =
				PrtnHdr->DstnExecutionAddr;
		ATFHandoffParams.Entry[LoopCount].PrtnFlags = PrtnFlags;
	}
}

/*****************************************************************************/
/**
 * @brief	This function is used to load the primary and secondary PDIs.
 *
 * @param	PdiPtr is Pdi instance pointer
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadAndStartSecPdi(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	PdiSrc_t PdiSrc;
	u32 PdiAddr;
	u32 SecBootMode = XilPdi_GetSBD(&(PdiPtr->MetaHdr.ImgHdrTbl)) >>
				XIH_IHT_ATTR_SBD_SHIFT;
#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
	u32 UPdiSrc;
#endif

	if ((SecBootMode == XIH_IHT_ATTR_SBD_SAME) ||
		((PdiPtr->SlrType != XLOADER_SSIT_MASTER_SLR) &&
		(PdiPtr->SlrType != XLOADER_SSIT_MONOLITIC))) {
		/* Do nothing */
		Status = XST_SUCCESS;
	} else {
		XPlmi_Printf(DEBUG_INFO, "+++Configuring Secondary Boot "
				"Device\n\r");
		if (SecBootMode == XIH_IHT_ATTR_SBD_PCIE) {
			Status = XLoader_SbiInit((u32)XLOADER_PDI_SRC_PCIE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		} else {
			switch(SecBootMode)
			{
				case XIH_IHT_ATTR_SBD_QSPI32:
					PdiSrc = XLOADER_PDI_SRC_QSPI32;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_QSPI24:
					PdiSrc = XLOADER_PDI_SRC_QSPI24;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_SD_0:
				#ifdef XLOADER_SD_0
					UPdiSrc = (u32)XLOADER_PDI_SRC_SD0 |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_SBD_ADDR_SHIFT);
					PdiSrc = (PdiSrc_t)UPdiSrc;
				#else
					PdiSrc = XLOADER_PDI_SRC_SD0;
				#endif
					PdiAddr = 0U;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_SD_1:
				#ifdef XLOADER_SD_1
					UPdiSrc = (u32)XLOADER_PDI_SRC_SD1 |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_SBD_ADDR_SHIFT);
					PdiSrc = (PdiSrc_t)UPdiSrc;
				#else
					PdiSrc = XLOADER_PDI_SRC_SD1;
				#endif
					PdiAddr = 0U;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_SD_LS:
				#ifdef XLOADER_SD_1
					UPdiSrc = (u32)XLOADER_PDI_SRC_SD1_LS |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_SBD_ADDR_SHIFT);
					PdiSrc = (PdiSrc_t)UPdiSrc;
				#else
					PdiSrc = XLOADER_PDI_SRC_SD1_LS;
				#endif
					PdiAddr = 0U;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_EMMC:
				#ifdef XLOADER_SD_1
					UPdiSrc = (u32)XLOADER_PDI_SRC_EMMC |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_SBD_ADDR_SHIFT);
					PdiSrc = (PdiSrc_t)UPdiSrc;
				#else
					PdiSrc = XLOADER_PDI_SRC_EMMC;
				#endif
					PdiAddr = 0U;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_OSPI:
					PdiSrc = XLOADER_PDI_SRC_OSPI;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_USB:
					PdiSrc = XLOADER_PDI_SRC_USB;
					PdiAddr = 0U;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_SMAP:
					PdiSrc = XLOADER_PDI_SRC_SMAP;
					PdiAddr = 0U;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_SD_0_RAW:
					PdiSrc = XLOADER_PDI_SRC_SD0_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_SD_1_RAW:
					PdiSrc = XLOADER_PDI_SRC_SD1_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_RAW:
					PdiSrc = XLOADER_PDI_SRC_EMMC_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_SD_LS_RAW:
					PdiSrc = XLOADER_PDI_SRC_SD1_LS_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_0:
				#ifdef XLOADER_SD_0
					UPdiSrc = (u32)XLOADER_PDI_SRC_EMMC0 |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						 << XLOADER_SD_SBD_ADDR_SHIFT);
					PdiSrc = (PdiSrc_t)UPdiSrc;
				#else
					PdiSrc = XLOADER_PDI_SRC_EMMC0;
				#endif
					PdiAddr = 0U;
					Status = XST_SUCCESS;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_0_RAW:
					PdiSrc = XLOADER_PDI_SRC_EMMC0_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					Status = XST_SUCCESS;
					break;
				default:
					Status = XST_FAILURE;
					break;
			}

			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE, 0);
				goto END;
			}

			Status = XPlmi_MemSetBytes(PdiPtr, sizeof(XilPdi), 0U, sizeof(XilPdi));
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET, (int)XLOADER_ERR_MEMSET_PDIPTR);
				goto END;
			}
			PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;
			Status = XLoader_PdiInit(PdiPtr, PdiSrc, PdiAddr);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			Status = XLoader_LoadAndStartSubSystemImages(PdiPtr);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}
END:
	return Status;
}
