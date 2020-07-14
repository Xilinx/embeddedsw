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
*       kc   12/02/2019 Added peformance time stamps
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader.h"
#include "xpm_api.h"
#include "xpm_nodeid.h"
#include "xloader_secure.h"
#ifdef XPLM_SEM
#include "xilsem.h"
#endif
#include "xplmi_err.h"
#include "xplmi_event_logging.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XLoader_ReadAndValidateHdrs(XilPdi* PdiPtr, u32 RegVal);
static int XLoader_LoadAndStartSubSystemImages(XilPdi *PdiPtr);
static int XLoader_LoadAndStartSubSystemPdi(XilPdi *PdiPtr);
static void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi);
static int XLoader_IdCodeCheck(XilPdi_ImgHdrTbl * ImgHdrTbl);
static int XLoader_LoadAndStartSecPdi(XilPdi* PdiPtr);

/************************** Variable Definitions *****************************/
XilSubsystem SubSystemInfo = {0U};
XilPdi SubsystemPdiIns = {0U};
XilPdi_ATFHandoffParams ATFHandoffParams = {0U};
XilPdi* BootPdiPtr = NULL;

/*****************************************************************************/
#define XLOADER_DEVICEOPS_INIT(DevSrc, DevInit, DevCopy, DevRelease)\
	{ \
		.Name = DevSrc, \
		.DeviceBaseAddr = 0U, \
		.Init = DevInit, \
		.Copy = DevCopy, \
		.Release = DevRelease, \
	}

const XLoader_DeviceOps DeviceOps[] =
{
	XLOADER_DEVICEOPS_INIT("JTAG", XLoader_SbiInit, XLoader_SbiCopy, NULL),  /* JTAG - 0U */
#ifdef XLOADER_QSPI
	XLOADER_DEVICEOPS_INIT("QSPI24", XLoader_QspiInit, XLoader_QspiCopy, NULL), /* QSPI24 - 1U */
	XLOADER_DEVICEOPS_INIT("QSPI32", XLoader_QspiInit, XLoader_QspiCopy, NULL), /* QSPI32- 2U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_0
	XLOADER_DEVICEOPS_INIT("SD0", XLoader_SdInit, XLoader_SdCopy, XLoader_SdRelease), /* SD0 - 3U*/
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),  /* 4U */
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT("SD1", XLoader_SdInit, XLoader_SdCopy, XLoader_SdRelease), /* SD1 - 5U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT("EMMC", XLoader_SdInit, XLoader_SdCopy, XLoader_SdRelease), /* EMMC - 6U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_USB
	XLOADER_DEVICEOPS_INIT("USB", XLoader_UsbInit, XLoader_UsbCopy, NULL), /* USB - 7U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_OSPI
	XLOADER_DEVICEOPS_INIT("OSPI", XLoader_OspiInit, XLoader_OspiCopy, NULL), /* OSPI - 8U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL), /* 9U */
#ifdef XLOADER_SBI
	XLOADER_DEVICEOPS_INIT("SMAP", XLoader_SbiInit, XLoader_SbiCopy, NULL), /* SMAP - 0xA */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL), /* 0xBU */
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL), /* 0xCU */
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL), /* 0xDU */
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT("SD1_LS", XLoader_SdInit, XLoader_SdCopy, XLoader_SdRelease), /* SD1 LS - 0xEU */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT("DDR", XLoader_DdrInit, XLoader_DdrCopy, NULL), /* DDR - 0xF */
#ifdef XLOADER_SBI
	XLOADER_DEVICEOPS_INIT("SBI", XLoader_SbiInit, XLoader_SbiCopy, NULL), /* SBI - 0x10 */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL), /* PCIE - 0x11U */
#ifdef XLOADER_SD_0
	XLOADER_DEVICEOPS_INIT("SD0_RAW", XLoader_RawInit, XLoader_RawCopy, XLoader_SdRelease), /* SD0_RAW - 0x12U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT("SD1_RAW", XLoader_RawInit, XLoader_RawCopy, NULL), /* SD1_RAW - 0x13U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT("EMMC_RAW", XLoader_RawInit, XLoader_RawCopy, NULL), /* EMMC_RAW - 0x14U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT("SD1_LS_RAW", XLoader_RawInit, XLoader_RawCopy, NULL), /* SD1_LS_RAW - 0x15U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT("EMMC_RAW_BP1", XLoader_RawInit, XLoader_RawCopy, NULL), /* EMMC_RAW - 0x16U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT("EMMC_RAW_BP2", XLoader_RawInit, XLoader_RawCopy, NULL), /* EMMC_RAW - 0x17U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_0
	XLOADER_DEVICEOPS_INIT("EMMC0", XLoader_SdInit, XLoader_SdCopy, XLoader_SdRelease), /* EMMC0 - 0x18U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
#ifdef XLOADER_SD_0
	XLOADER_DEVICEOPS_INIT("EMMC0_RAW", XLoader_RawInit, XLoader_RawCopy, NULL), /* EMMC0_RAW - 0x19U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL, NULL, NULL),
#endif
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
int XLoader_Init()
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
int XLoader_PdiInit(XilPdi* PdiPtr, PdiSrc_t PdiSrc, u64 PdiAddr)
{
	int Status = XST_FAILURE;
	u32 RegVal = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);
	u64 PdiInitTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
	u32 DeviceFlags = PdiSrc & XLOADER_PDISRC_FLAGS_MASK;
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
		if ((DeviceFlags == XLOADER_PDI_SRC_SD0)
		|| (DeviceFlags == XLOADER_PDI_SRC_SD1)
		|| (DeviceFlags == XLOADER_PDI_SRC_SD1_LS)
		|| (DeviceFlags == XLOADER_PDI_SRC_EMMC)) {
			SdRawBootVal = RegVal & XLOADER_SD_RAWBOOT_MASK;
			if (SdRawBootVal == XLOADER_SD_RAWBOOT_VAL) {
				if (DeviceFlags == XLOADER_PDI_SRC_SD0) {
					PdiSrc = XLOADER_PDI_SRC_SD0_RAW;
				}
				else if (DeviceFlags == XLOADER_PDI_SRC_SD1) {
					PdiSrc = XLOADER_PDI_SRC_SD1_RAW;
				}
				else if (DeviceFlags == XLOADER_PDI_SRC_SD1_LS) {
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
			DeviceFlags = PdiSrc & XLOADER_PDISRC_FLAGS_MASK;
		}
		RegVal &= ~(XLOADER_SD_RAWBOOT_MASK);
	}

	if (DeviceOps[DeviceFlags].Init == NULL) {
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

	Status = XLoader_ReadAndValidateHdrs(PdiPtr, RegVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) {
		BootPdiPtr = PdiPtr;
	}
	XPlmi_MeasurePerfTime(PdiInitTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
		"%u.%u ms: PDI initialization time\n\r",
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
	int Status = XST_FAILURE;
	XLoader_SecureParms SecureParam = {0U};

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
				(RegVal * XLOADER_IMAGE_SEARCH_OFFSET);
			if ((PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI24) || \
				(PdiPtr->PdiSrc == XLOADER_PDI_SRC_QSPI32)) {
#ifdef XLOADER_QSPI
				Status = XLoader_QspiGetBusWidth(
					PdiPtr->MetaHdr.FlashOfstAddr);
				if (Status != XST_SUCCESS) {
					goto END;
				}
#endif
			}
		}
		else {
			PdiPtr->MetaHdr.FlashOfstAddr = PdiPtr->PdiAddr;
		}
		/* Update KEK red key availablity status */
		XLoader_UpdateKekRdKeyStatus(PdiPtr);
	}
	else {
		PdiPtr->ImageNum = 0U;
		PdiPtr->PrtnNum = 0U;
		PdiPtr->MetaHdr.FlashOfstAddr = PdiPtr->PdiAddr;
		/* Read Boot header */
		XilPdi_ReadBootHdr(&PdiPtr->MetaHdr);
		memset(&(PdiPtr->MetaHdr.BootHdr.BootHdrFwRsvd.MetaHdrOfst),
			0U, sizeof(XilPdi_BootHdrFwRsvd));
		PdiPtr->PlmKatStatus |= BootPdiPtr->PlmKatStatus;
		PdiPtr->KekStatus |= BootPdiPtr->KekStatus;
	}

	/* Read image header */
	Status = XilPdi_ReadImgHdrTbl(&PdiPtr->MetaHdr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR_TBL, Status);
		goto END;
	}

	SecureParam.PdiPtr = PdiPtr;
	if (XLoader_IsAuthEnabled(PdiPtr) == TRUE) {
		SecureParam.IsAuthenticated = TRUE;
		SecureParam.SecureEn = TRUE;
	}

	if (XLoader_IsEncEnabled(PdiPtr) == TRUE) {
		SecureParam.IsEncrypted = TRUE;
		SecureParam.SecureEn = TRUE;
	}

	/* Validates if authentication/encryption is compulsory */
	Status = XLoader_SecureValidations(&SecureParam);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO,"Failed at secure validations\n\r");
		goto END;
	}

	/* Authentication of IHT */
	if (SecureParam.IsAuthenticated == TRUE) {
		Status = XLoader_ImgHdrTblAuth(&SecureParam);
		if (Status != XST_SUCCESS) {
			goto END;
		}
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
	if (SecureParam.SecureEn != TRUE) {
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
		Status = XLoader_ReadAndVerifySecureHdrs(&SecureParam,
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
	u32 NoOfDelayedHandoffCpus = 0U;
	u32 DelayHandoffImageNum[XLOADER_MAX_HANDOFF_CPUS] = {0U};
	u32 DelayHandoffPrtnNum[XLOADER_MAX_HANDOFF_CPUS] = {0U};
	u32 Index = 0U;
	u32 ImageNum;
	u32 PrtnNum;
	u32 PrtnIndex;

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
	for ( ;PdiPtr->ImageNum < PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs;
			++PdiPtr->ImageNum)
	{
		if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) {
			PdiPtr->CopyToMem = XilPdi_GetCopyToMemory
				(&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum]) >>
				XILPDI_IH_ATTRIB_COPY_MEMORY_SHIFT;
		}
		else {
			PdiPtr->CopyToMem = FALSE;
		}
		PdiPtr->DelayHandoff = XilPdi_GetDelayHandoff(
			&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum]) >>
			XILPDI_IH_ATTRIB_DELAY_HANDOFF_SHIFT;
		if ((PdiPtr->CopyToMem == TRUE) &&
				(PdiPtr->DelayHandoff == TRUE)) {
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_DELAY_ATTRB, 0U);
			goto END;
		}

		if (PdiPtr->DelayHandoff == TRUE) {
			if (NoOfDelayedHandoffCpus == XLOADER_MAX_HANDOFF_CPUS) {
				Status = XPlmi_UpdateStatus(
						XLOADER_ERR_NUM_HANDOFF_CPUS, 0U);
				goto END;
			}
			DelayHandoffImageNum[NoOfDelayedHandoffCpus] =
					PdiPtr->ImageNum;
			DelayHandoffPrtnNum[NoOfDelayedHandoffCpus] =
					PdiPtr->PrtnNum;
			NoOfDelayedHandoffCpus += 1U;
		}

		Status = XLoader_LoadImage(PdiPtr, 0xFFFFFFFFU);
		if (Status != XST_SUCCESS) {
			/* Check for Cfi errors */
			if (NODESUBCLASS(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID)
					== XPM_NODESUBCL_DEV_PL) {
				XLoader_CframeErrorHandler();
			}
			goto END;
		}

		if ((PdiPtr->CopyToMem == TRUE) ||
			(PdiPtr->DelayHandoff == TRUE)) {
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

		for (PrtnIndex = 0U; PrtnIndex <
			PdiPtr->MetaHdr.ImgHdr[ImageNum].NoOfPrtns; PrtnIndex++) {
			Status = XLoader_UpdateHandoffParam(PdiPtr,
					PrtnNum + PrtnIndex);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		Status = XLoader_StartImage(PdiPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (DeviceOps[PdiPtr->PdiSrc].Release != NULL) {
		Status = DeviceOps[PdiPtr->PdiSrc].Release();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
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
	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) {
		SubSystemInfo.PdiPtr = PdiPtr;
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

#if defined(XPLM_SEM) && defined(XSEM_CFRSCAN_EN)
	/* Stop the SEM scan before PDI load */
	if (PdiPtr->PdiType != XLOADER_PDI_TYPE_FULL) {
		Status = XSem_CfrStopScan();
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_SEM_STOP_SCAN, Status);
			goto END;
		}
	}
#endif
	Status = XLoader_PdiInit(PdiPtr, PdiSrc, PdiAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
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

#if defined(XPLM_SEM) && defined(XSEM_CFRSCAN_EN)
	/* Restart the SEM SCAN */
	if (PdiPtr->PdiType != XLOADER_PDI_TYPE_FULL) {
		Status = XSem_CfrInit();
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_SEM_CFR_INIT, Status);
			goto END1;
		}
	}
END1:
#endif
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
int XLoader_StartImage(XilPdi *PdiPtr)
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
				break;
		}
	}

	/*
	 * Make Number of handoff CPUs to zero
	 */
	PdiPtr->NoOfHandoffCpus = 0x0U;
	Status = XST_SUCCESS;

END:
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
 * @brief	This function is used load a image in PDI. PDI can have multiple
 * images present in it. This can be used to load a single image like PL, APU, RPU.
 * This will load all the partitions that are present in that image.
 *
 * @param	PdiPtr is Pdi instance pointer
 * @param	ImageId is Id of the image present in PDI
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_LoadImage(XilPdi *PdiPtr, u32 ImageId)
{
	int Status = XST_FAILURE;
	u32 Index;

	if (0xFFFFFFFFU != ImageId) {
		/*
		 * Get subsystem information from the info stored during boot
		 */
		for (Index = 0U; Index < SubSystemInfo.Count; Index++) {
			if (ImageId ==
				SubSystemInfo.SubsystemLut[Index].SubsystemId) {
				PdiPtr->ImageNum =
					SubSystemInfo.SubsystemLut[Index].ImageNum;
				PdiPtr->PrtnNum =
					SubSystemInfo.SubsystemLut[Index].PrtnNum;
				break;
			}
		}
		if (Index == SubSystemInfo.Count) {
			Status = XLOADER_ERR_IMG_ID_NOT_FOUND;
			goto END;
		}
	}
	else {
		/*
		 * Update subsystem info only for FULL PDI type and subsystem count is
		 * less than max subsystems supported.
		 */
		if ((PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) &&
			(SubSystemInfo.Count < XLOADER_MAX_SUBSYSTEMS)) {
			SubSystemInfo.SubsystemLut[SubSystemInfo.Count].SubsystemId =
				PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID;
			SubSystemInfo.SubsystemLut[SubSystemInfo.Count].ImageNum =
				PdiPtr->ImageNum;
			SubSystemInfo.SubsystemLut[SubSystemInfo.Count++].PrtnNum =
				PdiPtr->PrtnNum;
		}
	}
	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_RESTORE) {
		for (Index = 0U; Index < PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs;
			++Index) {
			if (PdiPtr->MetaHdr.ImgHdr[Index].ImgID == ImageId) {
				PdiPtr->ImageNum = Index;
				break;
			}
		}

		if (Index == PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_IMG_ID_NOT_FOUND,
						0);
			goto END;
		}
	}

	PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName[3] = 0U;
	XPlmi_Printf(DEBUG_INFO, "------------------------------------\r\n");
	XPlmi_Printf(DEBUG_GENERAL,
			"+++++++Loading Image No: 0x%0x, Name: %s, Id: 0x%08x\n\r",
			PdiPtr->ImageNum,
			(char *)PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName,
			PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);

	PdiPtr->CurImgId = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID;
	/* Update current subsystem ID for EM */
	EmSubsystemId = PdiPtr->CurImgId;
	Status = XLoader_LoadImagePrtns(PdiPtr, PdiPtr->ImageNum,
				PdiPtr->PrtnNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	PdiPtr->PrtnNum += PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].NoOfPrtns;

	/* Log the image load to the Trace Log buffer */
	XPlmi_TraceLog3(XPLMI_TRACE_LOG_LOAD_IMAGE, PdiPtr->CurImgId);

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to restart the image in PDI. This function will take
 * ImageId as an input and based on the subsystem info available, it will read
 * the image partitions, loads them and hand-off to the required CPUs as part
 * of the image load.
 *
 * @param ImageId Id of the image present in PDI
 *
 * @return	returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XLoader_RestartImage(u32 ImageId)
{
	int Status = XST_FAILURE;

#if defined(XPLM_SEM) && defined(XSEM_CFRSCAN_EN)
	/** Stop the SEM scan before Image load */
	Status = XSem_CfrStopScan();
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SEM_STOP_SCAN, Status);
		goto END;
	}
#endif
	Status = XLoader_ReloadImage(ImageId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_StartImage(SubSystemInfo.PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#if defined(XPLM_SEM) && defined(XSEM_CFRSCAN_EN)
	/* Restart the SEM SCAN */
	Status = XSem_CfrInit();
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SEM_CFR_INIT, Status);
		goto END;
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to reload the image only in PDI. This function will
 * take ImageId as an input and based on the subsystem info available, it will
 * read the image partitions and loads them.
 *
 * @param ImageId Id of the image present in PDI
 *
 * @return      returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XLoader_ReloadImage(u32 ImageId)
{
	int Status = XST_FAILURE;
	u32 DeviceFlags = SubSystemInfo.PdiPtr->PdiSrc &
				XLOADER_PDISRC_FLAGS_MASK;

	/*
	 * This is for libpm to do the clock settings reqired for boot device
	 * to resume post suspension.
	 */
	switch(DeviceFlags)
	{
		case XLOADER_PDI_SRC_QSPI24:
		case XLOADER_PDI_SRC_QSPI32:
			XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_QSPI,
				PM_CAP_ACCESS, XPM_DEF_QOS, 0U);
			break;
		case XLOADER_PDI_SRC_SD0:
			XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_SDIO_0,
				PM_CAP_ACCESS, XPM_DEF_QOS, 0U);
			break;
		case XLOADER_PDI_SRC_SD1:
		case XLOADER_PDI_SRC_EMMC:
		case XLOADER_PDI_SRC_SD1_LS:
			XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_SDIO_1,
				PM_CAP_ACCESS, XPM_DEF_QOS, 0U);
			break;
		case XLOADER_PDI_SRC_USB:
			XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_USB_0,
				PM_CAP_ACCESS, XPM_DEF_QOS, 0U);
			break;
		case XLOADER_PDI_SRC_OSPI:
			XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_OSPI,
				PM_CAP_ACCESS, XPM_DEF_QOS, 0U);
			break;
		default:
			break;
	}

	if (DeviceOps[DeviceFlags].Init != NULL) {
		Status = DeviceOps[DeviceFlags].Init(
				SubSystemInfo.PdiPtr->PdiSrc);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XLoader_LoadImage(SubSystemInfo.PdiPtr, ImageId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (DeviceOps[DeviceFlags].Release != NULL) {
		Status = DeviceOps[DeviceFlags].Release();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	switch(DeviceFlags)
	{
		case XLOADER_PDI_SRC_QSPI24:
		case XLOADER_PDI_SRC_QSPI32:
			XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_QSPI);
			break;
		case XLOADER_PDI_SRC_SD0:
			XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_SDIO_0);
			break;
		case XLOADER_PDI_SRC_SD1:
		case XLOADER_PDI_SRC_EMMC:
		case XLOADER_PDI_SRC_SD1_LS:
			XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_SDIO_1);
			break;
		case XLOADER_PDI_SRC_USB:
			XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_USB_0);
			break;
		case XLOADER_PDI_SRC_OSPI:
			XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_OSPI);
			break;
		default:
			break;
	}

END:
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
static int XLoader_IdCodeCheck(XilPdi_ImgHdrTbl * ImgHdrTbl)
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
		IdCodeInfo.IsExtIdCodeZero = TRUE;
	}
	else {
		IdCodeInfo.IsExtIdCodeZero = FALSE;

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
		IdCodeInfo.IsVC1902Es1 = TRUE;
	}
	else {
		IdCodeInfo.IsVC1902Es1 = FALSE;
	}

	/* Check if a subset of checks to be bypassed */
	if (0x1U == (ImgHdrTbl->Attr & XIH_IHT_ATTR_BYPS_MASK)) {
		IdCodeInfo.BypassChkIHT = TRUE;
	}
	else {
		IdCodeInfo.BypassChkIHT = FALSE;
	}

	/*
	*  EXT_IDCODE
	*  [26:14]is0?  VC1902-ES1?  BYPASS?  Checks done
	*  --------------------------------------------------------------------
	*  Y            Y            Y        Check IDCODE[27:0] (skip Si Rev chk)
	*  Y		Y            N        Check IDCODE[31:0]
	*  Y		N            X        Invalid combination (Error out)
	*  N		X            Y        Check IDCODE[27:0] (skip Si Rev chk), check ext_idcode
	*  N		X            N        Check IDCODE[31:0], check ext_idcode
	*  --------------------------------------------------------------------
	*/

	/*
	 * Error out for the invalid combination of Extended IDCODE - Device.
	 * Assumption is that only VC1902-ES1 device can have Extended IDCODE value 0
	 */
	if ((IdCodeInfo.IsExtIdCodeZero == TRUE) &&
			(IdCodeInfo.IsVC1902Es1 == FALSE)) {
		Status = XLOADER_ERR_EXT_ID_SI;
		goto END;
	}
	else {
		/* Do not check Si revision if bypass configured */
		if (TRUE == IdCodeInfo.BypassChkIHT) {
			IdCodeInfo.IdCodeIHT &= ~PMC_TAP_IDCODE_SI_REV_MASK;
			IdCodeInfo.IdCodeRd &= ~PMC_TAP_IDCODE_SI_REV_MASK;
		}

		/* Do the actual IDCODE check */
		if (IdCodeInfo.IdCodeIHT != IdCodeInfo.IdCodeRd) {
			Status = XLOADER_ERR_IDCODE;
			goto END;
		}

		/* Do the actual Extended IDCODE check */
		if (FALSE == IdCodeInfo.IsExtIdCodeZero) {
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
	u32 PdiSrc;
	u32 PdiAddr;
	u32 SecBootMode = XilPdi_GetSBD(&(PdiPtr->MetaHdr.ImgHdrTbl)) >>
				XIH_IHT_ATTR_SBD_SHIFT;

	if ((SecBootMode == XIH_IHT_ATTR_SBD_SAME) ||
		((PdiPtr->SlrType != XLOADER_SSIT_MASTER_SLR) &&
		(PdiPtr->SlrType != XLOADER_SSIT_MONOLITIC))) {
		/* Do nothing */
		Status = XST_SUCCESS;
	} else {
		XPlmi_Printf(DEBUG_INFO, "+++Configuring Secondary Boot "
				"Device\n\r");
		if (SecBootMode == XIH_IHT_ATTR_SBD_PCIE) {
			Status = XLoader_SbiInit(XLOADER_PDI_SRC_PCIE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		} else {
			switch(SecBootMode)
			{
				case XIH_IHT_ATTR_SBD_QSPI32:
					PdiSrc = XLOADER_PDI_SRC_QSPI32;
					PdiAddr =
					PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					break;
				case XIH_IHT_ATTR_SBD_QSPI24:
					PdiSrc = XLOADER_PDI_SRC_QSPI24;
					PdiAddr =
					PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					break;
				case XIH_IHT_ATTR_SBD_SD_0:
				#ifdef XLOADER_SD_0
					PdiSrc = XLOADER_PDI_SRC_SD0 |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_SBD_ADDR_SHIFT);
				#else
					PdiSrc = XLOADER_PDI_SRC_SD0;
				#endif
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_SD_1:
				#ifdef XLOADER_SD_1
					PdiSrc = XLOADER_PDI_SRC_SD1 |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_SBD_ADDR_SHIFT);
				#else
					PdiSrc = XLOADER_PDI_SRC_SD1;
				#endif
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_SD_LS:
				#ifdef XLOADER_SD_1
					PdiSrc = XLOADER_PDI_SRC_SD1_LS |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_SBD_ADDR_SHIFT);
				#else
					PdiSrc = XLOADER_PDI_SRC_SD1_LS;
				#endif
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_EMMC:
				#ifdef XLOADER_SD_1
					PdiSrc = XLOADER_PDI_SRC_EMMC |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_SBD_ADDR_SHIFT);
				#else
					PdiSrc = XLOADER_PDI_SRC_EMMC;
				#endif
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_OSPI:
					PdiSrc = XLOADER_PDI_SRC_OSPI;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					break;
				case XIH_IHT_ATTR_SBD_USB:
					PdiSrc = XLOADER_PDI_SRC_USB;
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_SMAP:
					PdiSrc = XLOADER_PDI_SRC_SMAP;
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_SD_0_RAW:
					PdiSrc = XLOADER_PDI_SRC_SD0_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					break;
				case XIH_IHT_ATTR_SBD_SD_1_RAW:
					PdiSrc = XLOADER_PDI_SRC_SD1_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_RAW:
					PdiSrc = XLOADER_PDI_SRC_EMMC_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					break;
				case XIH_IHT_ATTR_SBD_SD_LS_RAW:
					PdiSrc = XLOADER_PDI_SRC_SD1_LS_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_0:
				#ifdef XLOADER_SD_0
					PdiSrc = XLOADER_PDI_SRC_EMMC0 |
						XLOADER_SD_SBD_ADDR_SET_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						 << XLOADER_SD_SBD_ADDR_SHIFT);
				#else
					PdiSrc = XLOADER_PDI_SRC_EMMC0;
				#endif
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_0_RAW:
					PdiSrc = XLOADER_PDI_SRC_EMMC0_RAW;
					PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
					break;
				default:
					Status =
					XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE;
					break;
			}

			if (Status == XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE) {
				goto END;
			}

			memset(PdiPtr, 0U, sizeof(XilPdi));
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
