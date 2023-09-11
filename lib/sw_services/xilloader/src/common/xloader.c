/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
*                       partial PDI load
*       bsv  06/11/2019 Added TCM power up code to Xilloader to fix issue in
*                       R5-1 split mode functionality
*       bsv  06/17/2019 Added support for CFI and CFU error handling
*       bsv  06/26/2019 Added secondary boot support
*       kc   07/16/2019 Added code to print execution time
*       rv   07/29/2019 Added code to request boot source before subsystem restart
*       vnsl 07/30/2019 Added APIs to load secure headers
*       scs  08/29/2019 Added API to validate extended ID Code
*       bsv  08/30/2019 Added fallback and multiboot support in PLM
*       kc   09/05/2019 Added code to use PMCDMA0 and PMCDMA1 in parallel
*       kc   09/13/2019 SBI reset is removed for SMAP boot mode to ensure smap
*                       bus width value remains unchanged
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
* 1.02  ana  06/04/2020 Updated PlmkatStatus and Kekstatus variables from
*                       initial boot pdi to partial pdi structure variables
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
*       kpt  10/19/2020 Renamed XLoader_UpdateKekRdKeyStatus to
*                       XLoader_UpdateKekSrc
*       td   10/19/2020 MISRA C Fixes
* 1.03  td   11/23/2020 Coverity Warning Fixes
*       bsv  12/02/2020 Replace SemCfr APIs with generic Sem APIs
*       bm   12/16/2020 Added PLM_SECURE_EXCLUDE macro
*       kpt  01/06/2021 Added redundancy check for
*                       XLoader_ReadAndVerifySecureHdrs
*       ma   01/18/2021 Added function for PMC state clear
*       bsv  01/28/2021 Initialize ParentImgID to invalid value
*       skd  02/01/2021 Added EL_3 check for partition in ATF HandoffParams
*       bm   02/12/2021 Updated logic to use BootHdr directly from PMC RAM
*       kpt  02/16/2021 Updated error code when secure validations are failed
*       rb   03/09/2021 Updated Sem Scan Init API call
*       bm   03/16/2021 Added Image Upgrade support
*       har  03/17/2021 Added API call to set the secure state
*       ma   03/24/2021 Redirect XilPdi prints to XilLoader
*       ma   03/24/2021 Minor updates to prints in XilLoader
*       har  03/31/2021 Added code to update PDI ID in RTC area of PMC RAM
*       bl   04/01/2021 Add extra arg for calls to XPm_RequestWakeUp
*       bm   04/14/2021 Update UID logic for AIE Image IDs
*       rp   04/20/2021 Add extra arg for calls to XPm_RequestDevice and
*			XPm_ReleaseDevice
*       bm   05/05/2021 Added USR_ACCESS support for PLD0 image
*       bm   05/10/2021 Updated chunking logic for hashes
*       ma   05/18/2021 Minor code cleanup
* 1.04  td   07/08/2021 Fix doxygen warnings
*       td   07/15/2021 Fix doxygen warnings
*       bm   07/16/2021 Updated XLoader_PdiInit prototype
*       bsv  07/18/2021 Debug enhancements
*       bsv  07/19/2021 Disable UART prints when invalid header is encountered
*                       in slave boot modes
*       bsv  07/24/2021 Clear RTC area at the beginning of PLM
*       ma   07/27/2021 Added temporal check for XLoader_SetSecureState
*       ma   07/27/2021 Added temporal check for XilPdi_ValidateImgHdrTbl
*       bm   08/09/2021 Removed obsolete XLoader_PMCStateClear API
*       bsv  08/17/2021 Code clean up
*       rb   08/11/2021 Fix compilation warnings
*       bm   08/24/2021 Added Extract Metaheader support
*       bm   08/26/2021 Updated XLOADER_PDI_LOAD_STARTED register write
*       bsv  08/31/2021 Code clean up
*       kpt  09/06/2021 Fixed SW-BP-ZEROIZE issue in
*                       XLoader_LoadAndStartSubSystemImages
*       gm   09/17/2021 Support added for MJTAG workaround
*       bsv  09/24/2021 Fix secondary Pdi load issue
* 1.05  kpt  10/20/2021 Modified temporal checks to use temporal variables from
*                       data section
*       bsv  10/26/2021 Code clean up
*       bsv  12/04/2021 Clear complete PMCRAM in XLoader_ReloadImage
*       kpt  12/13/2021 Replace Xil_SecureMemCpy with Xil_SMemCpy
*       bsv  12/22/2021 Added zeroization of ImageHeaderTable instance in
*                       case of invalid header
*       bm   01/20/2022 Fix compilation warnings in Xil_SMemCpy
*       bsv  01/29/2022 Added redundancy to Status variable in XLoader_LoadImage
* 1.06  kpt  02/01/2022 Updated XilPdi_ReadBootHdr prototype
*       skg  06/20/2022 Fixed MISRA C Rule 10.3 violation
*       ma   06/21/2022 Add support for Get Handoff Parameters IPI command
*       bsv  07/06/2022 Added support to read Optional Data in slave boot modes
*       bm   07/06/2022 Refactor versal and versal_net code
*       kpt  07/05/22 Added support to update KAT status
*       bsv  07/08/2022 Code changes related to Optional data in Image header
*       dc   07/12/2022 Added device stage to scheduler
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
*       bm   07/13/2022 Retain critical data structures after In-Place PLM Update
*       dc   07/20/2022 Added Data measurement, supports only for Versal Net
*       dc   07/27/2022 Added goto END in error case for header failures
*       ma   08/08/2022 Check EAM errors between each image load
*       ng   08/18/2022 Modified DelayedHandoffCpus condition to handle all possible values
* 1.07  sk   11/22/2022 Added Subsystems ValidHeader member variable init to
*                       XLoader_Init function to handle in-place update scenerio
*       ng   11/23/2022 Updated doxygen comments
*       ng   01/02/2023 Check to bypass entire ID Code Check
*       kpt  01/04/2023 Added check to update FIPS state
*       sk   01/11/2023 Added Image Store support as secondary boot media
*       bm   01/04/2023 Switch to SSIT Events as soon as basic Noc path is
*                       configured
*       bm   01/14/2023 Remove bypassing of PLM Set Alive during boot
*       sk   02/08/2023 Renamed XLoader_UpdateKatStatus to XLoader_ClearKatOnPPDI
*       sk   03/17/2023 Renamed Kekstatus to DecKeySrc in xilpdi structure
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       sk   04/28/2023 Added function to retrieve PDI Address from Image Store
*                       based on PDI ID
*       sk   05/18/2023 Deprecate copy to memory feature,Added function to save
*                       BootPDI info, Definition for XLoader_GetPdiInstance
*       am   07/07/2023 Added error code for Read IHT optional data
*       sk   07/31/2023 Updated error codes in XLoader_IsPdiAddrLookup
*       sk   08/18/2023 Fixed security review comments
*       sk   08/28/2023 Added redundant call for XLoader_GetKekSrc
*       rama 08/30/2023 Changed PDI source type print to DEBUG_ALWAYS for
*                       debug level_0 option
*       dd   09/11/2023 MISRA-C violation Rule 10.3 fixed
*       dd   09/11/2023 MISRA-C violation Rule 17.8 fixed
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
#include "xloader_secure.h"
#include "xpm_device.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xpm_nodeid.h"
#include "xloader_auth_enc.h"
#ifdef XPLM_SEM
#include "xilsem.h"
#endif
#include "xplmi.h"
#include "xplmi_err_common.h"
#include "xplmi_event_logging.h"
#include "xplmi_wdt.h"
#include "xloader_plat.h"
#include "xplmi_err.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_IMAGE_INFO_TBL_MAX_NUM	(XPLMI_IMAGE_INFO_TBL_BUFFER_LEN / \
		sizeof(XLoader_ImageInfo)) /**< Maximum number of image info
					     tables in the available buffer */

/************************** Function Prototypes ******************************/
static int XLoader_ReadAndValidateHdrs(XilPdi* PdiPtr, u32 RegVal, u64 PdiAddr);
static int XLoader_LoadAndStartSubSystemImages(XilPdi *PdiPtr);
static int XLoader_LoadAndStartSubSystemPdi(XilPdi *PdiPtr);
static int XLoader_LoadAndStartSecPdi(XilPdi* PdiPtr);
static int XLoader_VerifyImgInfo(const XLoader_ImageInfo *ImageInfo);
static int XLoader_GetChildRelation(u32 ChildImgID, u32 ParentImgID, u32 *IsChild);
static int XLoader_InvalidateChildImgInfo(u32 ParentImgID, u32 *ChangeCount);
static int XLoader_LoadImage(XilPdi *PdiPtr);
static int XLoader_ReloadImage(XilPdi *PdiPtr, u32 ImageId, const u32 *FuncID);
static int XLoader_StoreImageInfo(const XLoader_ImageInfo *ImageInfo);

/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
 * @{
 * @cond xloader_internal
 */
static const XLoader_DeviceOps DeviceOps[] =
{
	{XLoader_SbiInit, XLoader_SbiCopy, XLoader_SbiRecovery}, /* JTAG, SBI, SMAP, PCIE */
#ifdef XLOADER_QSPI
	{XLoader_QspiInit, XLoader_QspiCopy, XLoader_QspiRelease}, /* QSPI24, QSPI32 */
#else
	{NULL, NULL, NULL},
#endif
#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
	{XLoader_SdInit, XLoader_SdCopy, XLoader_SdRelease},
	/* For Versal, SD0, SD1, SD1_LS, EMMC1, EMMC0 */
	/* For VersalNet, SDLS_B0, SD_B1, SDLS_B1, EMMC1, EMMC0 */
	{XLoader_RawInit, XLoader_RawCopy, XLoader_RawRelease},
	/* For Versal, SD0_RAW, SD1_RAW, SD1_LS_RAW, EMMC_RAW, EMMC_RAW_BP1, EMMC_RAW_BP2, EMMC0_RAW */
	/* For VersalNet, SDLS_B0_RAW, SD_B1_RAW, SDLS_B1_RAW, EMMC_RAW, EMMC_RAW_BP1, EMMC_RAW_BP2, EMMC0_RAW */
#else
	{NULL, NULL, NULL},
	{NULL, NULL, NULL},
#endif
	{XLoader_DdrInit, XLoader_DdrCopy, XLoader_DdrRelease}, /* DDR */
#ifdef XLOADER_OSPI
	{XLoader_OspiInit, XLoader_OspiCopy, XLoader_OspiRelease}, /* OSPI */
#else
	{NULL, NULL, NULL},
#endif
#ifdef XLOADER_USB
	{XLoader_UsbInit, XLoader_UsbCopy, XLoader_UsbRelease}, /* USB */
#else
	{NULL, NULL, NULL},
#endif
};

/**
 * @}
 * @endcond
 */

/*****************************************************************************/
/**
 * @brief	This function initializes the loader instance and registers loader
 * 			commands with PLM.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_Init(void)
{
	volatile int Status = XST_FAILURE;
	XilPdi *PdiPtr = XLoader_GetPdiInstance();

	/** - Initialize the Uart Logs Config */
	PdiPtr->DiscardUartLogs = (u8)FALSE;

	/** - Initialize the loader commands */
	XLoader_CmdsInit();

	/** - Initialize the loader interrupts */
	Status = XLoader_IntrInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef PLM_DEBUG_MODE
	Status = XLoader_CframeInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	/** - Set the secure state of boot in registers and global variables */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_SetSecureState);

#ifndef PLM_SECURE_EXCLUDE
	/** - Add task to the scheduler to handle Authenticated JTAG message */
	Status = XLoader_AddAuthJtagToScheduler();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Add DAP status check task to the scheduler, this is
	 * applicable only for VersalNet.
	 */
	Status = XLoader_AddDeviceStateChangeToScheduler();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif
	Status = XLoader_PlatInit();

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the PDI instance with required details
 * 			and read the meta header.
 *
 * @param	PdiPtr is instance pointer pointing to PDI details
 * @param	PdiSource is source of PDI. It can be any boot Device or DDR
 * @param	PdiAddr is the address at which PDI is located in the PDI source
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_UNSUPPORTED_BOOT_MODE on unsupported boot mode.
 *****************************************************************************/
int XLoader_PdiInit(XilPdi* PdiPtr, PdiSrc_t PdiSource, u64 PdiAddr)
{
	PdiSrc_t PdiSrc = PdiSource;
	volatile int Status = XST_FAILURE;
	u32 RegVal = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT);
	u64 PdiInitTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime;
	u8 DeviceFlags = (u8)(PdiSrc & XLOADER_PDISRC_FLAGS_MASK);
	u32 SdRawBootVal = RegVal & XLOADER_SD_RAWBOOT_MASK;
	const char *RawString = "";
	const PdiSrcMap PdiSourceMap[] = XLOADER_GET_PDISRC_INFO();

	/**
	 * - Mark PDI loading is started.
	 */
	XPlmi_Out32(PMC_GLOBAL_DONE, XLOADER_PDI_LOAD_STARTED);

	/**
	 * - Store address of the structure in PMC_GLOBAL.GLOBAL_GEN_STORAGE4.
	 */
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE4,
		(u32)((UINTPTR)XLoader_GetATFHandoffParamsAddr()));

	if (DeviceFlags >= XPLMI_ARRAY_SIZE(PdiSourceMap)) {
		goto END1;
	}

	/**
	 * - Read the PDI source.
	*/
	PdiPtr->PdiIndex = PdiSourceMap[DeviceFlags].Index;
	if (PdiPtr->PdiIndex == XLOADER_SD_INDEX) {
		/**
		 * - Get the PDI source type.
		*/
		if (PdiPtr->PdiType != XLOADER_PDI_TYPE_FULL) {
			SdRawBootVal = PdiSrc & XLOADER_SD_RAWBOOT_MASK;
		}
		else {
			if ((SdRawBootVal == XLOADER_SD_RAWBOOT_MASK) ||
				(SdRawBootVal == 0U)) {
				/* 0 is for QEMU */
				PdiSrc |= (RegVal << XLOADER_PDISRC_FLAGS_SHIFT);
			}
			else {
				PdiSrc |= SdRawBootVal;
			}
		}
		if ((SdRawBootVal != 0U) &&
			(SdRawBootVal != XLOADER_SD_RAWBOOT_MASK)) {
			PdiPtr->PdiIndex = XLOADER_SD_RAW_INDEX;
			if (SdRawBootVal == XLOADER_EMMC_BP1_RAW_VAL) {
				RawString = "_RAW_BP1";
			}
			else if (SdRawBootVal == XLOADER_EMMC_BP2_RAW_VAL) {
				RawString = "_RAW_BP2";
			}
			else {
				RawString = "_RAW";
			}
		}
	}
	if ((PdiPtr->PdiIndex >= XLOADER_INVALID_INDEX) ||
		(DeviceOps[PdiPtr->PdiIndex].Init == NULL)) {
		goto END1;
	}
	/**
	 * - Print the PDI source type.
	*/
	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Loading PDI from %s%s\n\r",
		PdiSourceMap[DeviceFlags].Name, RawString);

	/*
	 * Update the PDI instance with the PDI device source and the PDI address
	 * received
	 */
	PdiPtr->PdiSrc = PdiSrc;
	PdiPtr->SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) &
		PMC_TAP_SLR_TYPE_VAL_MASK);
	if ((PdiPtr->SlrType == XLOADER_SSIT_MASTER_SLR) ||
		(PdiPtr->SlrType == XLOADER_SSIT_MONOLITIC)) {
		XPlmi_Printf(DEBUG_GENERAL, "Monolithic/Master Device\n\r");

		Status = DeviceOps[PdiPtr->PdiIndex].Init(PdiSrc);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (PdiSrc == XLOADER_PDI_SRC_USB) {
			PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;
		}
	}

	/**
	 * - Get the device copy function for the given boot mode.
	 */
	PdiPtr->MetaHdr.DeviceCopy = DeviceOps[PdiPtr->PdiIndex].Copy;

	/**
	 * - Read and validate all the headers in the PDI.
	*/
	Status = XST_FAILURE;
	Status = XLoader_ReadAndValidateHdrs(PdiPtr, RegVal, PdiAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	goto END;

END1:
	XPlmi_Printf(DEBUG_GENERAL, "Unsupported Boot Mode: Source:0x%x\n\r",
		PdiPtr->PdiSrc);
	Status = XPlmi_UpdateStatus(XLOADER_UNSUPPORTED_BOOT_MODE, 0);
END:
	XPlmi_MeasurePerfTime(PdiInitTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
		"%u.%03u ms: PDI initialization time\n\r",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads bootheader and metaheader and validates them.
 *
 * @param	PdiPtr is instance pointer pointing to PDI details
 * @param	RegValue is the value of the Multiboot register
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_MEMSET_BOOT_HDR_FW_RSVD if failed to create instance
 * 			for boot header reserved fields.
 * 			- XLOADER_ERR_IMGHDR_TBL if PLM is unable to read image header table.
 * 			- XLOADER_ERR_MEMSET_SECURE_PTR if failed to create instance for
 * 			secure pointer.
 * 			- XLOADER_ERR_UNSUPPORTED_PDI_VER on PDI version used in secure
 * 			operations is unsupported.
 * 			- XLOADER_ERR_SECURE_NOT_ENABLED on attempt to secure boot in an
 * 			excluded secure critical code.
 * 			- XLOADER_ERR_GEN_IDCODE on ID code mismatch.
 * 			- XLOADER_ERR_IMGHDR if image header checksum fails.
 * 			- XLOADER_ERR_PRTNHDR if partition header checksum fails.
 * 			- XLOADER_ERR_SECURE_METAHDR if secure metaheader validation fails.
 *
 *****************************************************************************/
static int XLoader_ReadAndValidateHdrs(XilPdi* PdiPtr, u32 RegValue, u64 PdiAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTemp =  XST_FAILURE;
	XLoader_SecureParams SecureParams = {0U};
	volatile u32 DecKeySrcTmp;
	u32 RegVal = RegValue;
#ifdef PLM_SECURE_EXCLUDE
	u8 IsEncrypted;
	u8 IsAuthenticated;
#else
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();
#endif
	XilBootPdiInfo *BootPdiInfo = XLoader_GetBootPdiInfo();

	SecureParams.PdiPtr = PdiPtr;
	/** Read Boot header */
	XilPdi_ReadBootHdr(&PdiPtr->MetaHdr.BootHdrPtr);
	XPlmi_Printf(DEBUG_INFO, "Boot Header Attributes: 0x%x\n\r",
		PdiPtr->MetaHdr.BootHdrPtr->ImgAttrb);

	if (XPlmi_IsLoadBootPdiDone() == TRUE) {
		PdiPtr->MetaHdr.MetaHdrOfst = BootPdiInfo->MetaHdrOfst;
	} else {
		PdiPtr->MetaHdr.MetaHdrOfst = PdiPtr->MetaHdr.BootHdrPtr->BootHdrFwRsvd.MetaHdrOfst;
	}
	/**
	 * Read meta header from PDI source
	 */
	if ((PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) ||
			(PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL_METAHEADER)) {

		PdiPtr->ImageNum = 1U;
		PdiPtr->PrtnNum = 1U;
		/**
		 * Update the flash offset address in PDI instance if the boot mode is
		 * QSPI, OSPI or SD_RAW.
		 */
		if ((PdiPtr->PdiIndex == XLOADER_QSPI_INDEX) ||
			(PdiPtr->PdiIndex == XLOADER_OSPI_INDEX) ||
			(PdiPtr->PdiIndex == XLOADER_SD_RAW_INDEX)) {
			RegVal &= ~(XLOADER_SD_RAWBOOT_MASK);
			PdiPtr->MetaHdr.FlashOfstAddr = PdiAddr + \
				((u64)RegVal * XLOADER_IMAGE_SEARCH_OFFSET);
#ifdef XLOADER_QSPI
			/**
			 * - If boot mode is QSPI, then get the QSPI bus width.
			*/
			if (PdiPtr->PdiIndex == XLOADER_QSPI_INDEX) {
				Status = XLoader_QspiGetBusWidth(
					PdiPtr->MetaHdr.FlashOfstAddr);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
#endif
		}
		else {
			PdiPtr->MetaHdr.FlashOfstAddr = PdiAddr;
		}

		if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL_METAHEADER) {
			PdiPtr->MetaHdr.MetaHdrOfst = XPlmi_In64(PdiAddr + XIH_BH_META_HDR_OFFSET);
		}

		/* Print FW Rsvd/Requested fields Details */
		XPlmi_Printf(DEBUG_INFO, "Meta Header Offset: 0x%x\n\r",
			PdiPtr->MetaHdr.MetaHdrOfst);
	}
	else {
		PdiPtr->ImageNum = 0U;
		PdiPtr->PrtnNum = 0U;
		PdiPtr->MetaHdr.FlashOfstAddr = PdiAddr;
		/* Partial PDI/Header set offset to 0 */
		PdiPtr->MetaHdr.MetaHdrOfst = 0U;
	}

	/** Read image header table from Meta Header */
	Status = XilPdi_ReadImgHdrTbl(&PdiPtr->MetaHdr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR_TBL, Status);
		goto END;
	}

	/**
	 * Check the validity of Img Hdr Table fields
	 */
	XSECURE_TEMPORAL_IMPL(Status, StatusTemp,
			XilPdi_ValidateImgHdrTbl, &(PdiPtr->MetaHdr.ImgHdrTbl));

	/**
	 * Update the PDI ID in the RTC area of PMC RAM
	 */
	XPlmi_Out32(XPLMI_RTCFG_PDI_ID_ADDR, PdiPtr->MetaHdr.ImgHdrTbl.PdiId);

	/*
	 * Print the Img header table details
	 * Print the Bootgen version
	 */
	XPlmi_Printf(DEBUG_INFO, "--------Image Header Table Details--------\n\r");
	XPlmi_Printf(DEBUG_INFO, "Boot Gen Version: 0x%x\n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.Version);
	XPlmi_Printf(DEBUG_INFO, "No of Images: 0x%x\n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs);
	XPlmi_Printf(DEBUG_INFO, "Image Header Address: 0x%x\n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.ImgHdrAddr);
	XPlmi_Printf(DEBUG_INFO, "No of Partitions: 0x%x\n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.NoOfPrtns);
	XPlmi_Printf(DEBUG_INFO, "Partition Header Address: 0x%x\n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.PrtnHdrAddr);
	XPlmi_Printf(DEBUG_INFO, "Secondary Boot Device Address: 0x%x\n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr);
	XPlmi_Printf(DEBUG_INFO, "IDCODE: 0x%x \n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.Idcode);
	XPlmi_Printf(DEBUG_INFO, "Attributes: 0x%x\n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.Attr);
	if ((Status != XST_SUCCESS) || (StatusTemp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_GENERAL, "Image Header Table Validation "
					"failed\n\r");
		PdiPtr->DiscardUartLogs = (u8)TRUE;
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR_TBL, Status);
		StatusTemp = Xil_SecureZeroize((u8 *)&(PdiPtr->MetaHdr.ImgHdrTbl),
			sizeof(PdiPtr->MetaHdr.ImgHdrTbl));
		goto END;
	}
	if (PdiPtr->DiscardUartLogs == (u8)TRUE) {
		PdiPtr->DiscardUartLogs = (u8)FALSE;
		DebugLog->LogLevel |= (DebugLog->LogLevel >> XPLMI_LOG_LEVEL_SHIFT);
	}
#ifndef PLM_SECURE_EXCLUDE
	Status = XPlmi_MemSetBytes(SecureTempParams, sizeof(XLoader_SecureTempParams),
				0U, sizeof(XLoader_SecureTempParams));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET,
				(int)XLOADER_ERR_MEMSET_SECURE_PTR);
		goto END;
	}

	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_FULL) {
		/**
		 * Update KEK red key availability status if PLM is encrypted with
		 * Black key
		 */
		XSECURE_REDUNDANT_CALL(PdiPtr->DecKeySrc, DecKeySrcTmp, XLoader_GetKekSrc);
		if (PdiPtr->DecKeySrc != DecKeySrcTmp) {
			Status = XST_GLITCH_ERROR;
			goto END;
		}
	}
	else {
		PdiPtr->PlmKatStatus |= BootPdiInfo->PlmKatStatus;
		PdiPtr->DecKeySrc |= BootPdiInfo->DecKeySrc;
		/* Update KAT status */
		XLoader_ClearKatOnPPDI(PdiPtr, 0U);
	}

	/**
	 * Read Image Header Table to know if authentication is enabled for
	 * Meta header
	 */
	SecureParams.IsAuthenticated =
		XilPdi_IsAuthEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	SecureTempParams->IsAuthenticated =
		XilPdi_IsAuthEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	SecureParams.IsEncrypted =
		XilPdi_IsEncEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	SecureTempParams->IsEncrypted =
		XilPdi_IsEncEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	if ((SecureParams.IsEncrypted == (u8)TRUE) ||
		(SecureTempParams->IsEncrypted == (u8)TRUE) ||
		(SecureParams.IsAuthenticated == (u8)TRUE) ||
		(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
		SecureParams.SecureEn = (u8)TRUE;
		SecureTempParams->SecureEn = (u8)TRUE;
		Status = XilPdi_ReadIhtAndOptionalData(&PdiPtr->MetaHdr);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_READ_IHT_OPTIONAL_DATA, Status);
			goto END;
		}
		Status = XilPdi_StoreDigestTable(&PdiPtr->MetaHdr);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_STORE_DIGEST_TABLE, Status);
			goto END;
		}
	}

	/* Secure flow is not supported for PDI versions older than v4 */
	if ((SecureParams.SecureEn == (u8)TRUE) &&
	   ((PdiPtr->MetaHdr.ImgHdrTbl.Version == XLOADER_PDI_VERSION_1) ||
	   (PdiPtr->MetaHdr.ImgHdrTbl.Version < XLOADER_PDI_VERSION_4))) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UNSUPPORTED_PDI_VER, 0);
		goto END;
	}

	/**
	 * Validate if authentication/encryption is compulsory
	 */
	Status = XST_FAILURE;
	Status = XLoader_SecureValidations(&SecureParams);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_INFO, "Error: Failed at secure validations\n\r");
		goto END;
	}

	/**
	 * Authenticate Image Header Table
	 */
	if ((SecureParams.IsAuthenticated == (u8)TRUE) ||
		(SecureTempParams->IsAuthenticated == (u8)TRUE)) {
		XSECURE_TEMPORAL_CHECK(END, Status, XLoader_ImgHdrTblAuth,
			&SecureParams);
	}
#else
	IsAuthenticated = XilPdi_IsAuthEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);
	IsEncrypted = XilPdi_IsEncEnabled(&PdiPtr->MetaHdr.ImgHdrTbl);

	if ((IsEncrypted == (u8)TRUE) || (IsAuthenticated == (u8)TRUE)) {
		XPlmi_Printf(DEBUG_GENERAL, "Secure Code is excluded\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SECURE_NOT_ENABLED, 0U);
		goto END;
	}
#endif

#ifndef VERSAL_IMAGE_SEL
	/**
	 * Perform IDCODE and Extended IDCODE checks
	 */
	if(XPLMI_PLATFORM == PMC_TAP_VERSION_SILICON) {
		Status = XLoader_IdCodeCheck(&(PdiPtr->MetaHdr.ImgHdrTbl));
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL, "IDCODE Checks failed\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_GEN_IDCODE,
						Status);
			goto END;
		}
	}
#endif

	/**
	 * Read and verify image headers and partition headers
	 */
	if (SecureParams.SecureEn != (u8)TRUE) {
		if (PdiPtr->PdiIndex == XLOADER_SBI_INDEX) {
			/* Source address does not matter in slave boot mode and hence
			 * making it zero
			 */
			Status = PdiPtr->MetaHdr.DeviceCopy(0U, XPLMI_PMCRAM_CHUNK_MEMORY,
				(PdiPtr->MetaHdr.ImgHdrTbl.OptionalDataLen <<
				XPLMI_WORD_LEN_SHIFT), 0U);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		/* Read IHT and PHT to structures and verify checksum */
		XPlmi_Printf(DEBUG_INFO, "Reading 0x%x Image Headers\n\r",
				PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs);
		Status = XilPdi_ReadImgHdrs(&PdiPtr->MetaHdr);
		if (XST_SUCCESS != Status) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR, Status);
			goto END;
		}
		Status = XilPdi_VerifyImgHdrs(&PdiPtr->MetaHdr);
		if (XST_SUCCESS != Status) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_IMGHDR, Status);
			goto END;
		}

		XPlmi_Printf(DEBUG_INFO, "Reading 0x%x Partition Headers\n\r",
			PdiPtr->MetaHdr.ImgHdrTbl.NoOfPrtns);
		Status = XilPdi_ReadPrtnHdrs(&PdiPtr->MetaHdr);
		if (XST_SUCCESS != Status) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTNHDR, Status);
			goto END;
		}
		Status = XilPdi_VerifyPrtnHdrs(&PdiPtr->MetaHdr);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTNHDR, Status);
			goto END;
		}
	}
#ifndef PLM_SECURE_EXCLUDE
	else {
		XSECURE_TEMPORAL_IMPL(Status, StatusTemp,
			XLoader_ReadAndVerifySecureHdrs, &SecureParams,
			&(PdiPtr->MetaHdr));
		if ((Status != XST_SUCCESS) || (StatusTemp != XST_SUCCESS)) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_SECURE_METAHDR,
						Status);
			goto END;
		}
	}
#endif
	/* This is applicable only for VersalNet */
	Status = XLoader_HdrMeasurement(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to load and start images. It reads
 * 			meta header, loads the images as present in the PDI and starts images based
 * 			on hand-off information present in PDI.
 *
 * @param	PdiPtr is Pdi instance pointer
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_DELAY_ATTRB if both delay handoff and delay load are
 * 			set for the same image.
 * 			- XLOADER_ERR_NUM_HANDOFF_CPUS if number of CPUs exceed max count.
 *
 *****************************************************************************/
static int XLoader_LoadAndStartSubSystemImages(XilPdi *PdiPtr)
{
	int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	u8 NoOfDelayedHandoffCpus = 0U;
	u8 DelayHandoffImageNum[XLOADER_MAX_HANDOFF_CPUS];
	u8 DelayHandoffPrtnNum[XLOADER_MAX_HANDOFF_CPUS];
	u8 Index;
	u8 ImageNum;
	u8 PrtnNum;
	u8 PrtnIndex;

	/**
	 * From the meta header present in PDI pointer, read the subsystem
	 * image details and load, start all the images
	 *
	 * For every image,
	 *   1. Read the CDO file if present
	 *   2. Send the CDO file to cdo parser which directs
	 *      CDO commands to Xilpm, and other components
	 *   3. Load partitions to respective memories
	 */
	for ( ; PdiPtr->ImageNum < (u8)PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs;
			++PdiPtr->ImageNum) {

		PdiPtr->DelayHandoff = (u8)(XilPdi_GetDelayHandoff(
			&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum]) >>
			XILPDI_IH_ATTRIB_DELAY_HANDOFF_SHIFT);
		PdiPtr->DelayLoad = (u8)(XilPdi_GetDelayLoad(
			&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum]) >>
			XILPDI_IH_ATTRIB_DELAY_LOAD_SHIFT);

		if (PdiPtr->DelayHandoff == (u8)TRUE) {
			if (PdiPtr->DelayLoad == (u8)TRUE) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_DELAY_ATTRB, 0);
				goto END;
			}

			if (NoOfDelayedHandoffCpus >= XLOADER_MAX_HANDOFF_CPUS) {
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
			if (NODESUBCLASS(
				PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID)
					== (u32)XPM_NODESUBCL_DEV_PL) {
				/*
				 * To preserve the PDI error, ignoring the
				 * Error returned by XLoader_CframeErrorHandler
				 */
				XLoader_CframeErrorHandler(
					PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
			}
			goto END;
		}

		/** Enable Ssit Events once the basic NoC has been configured in SSIT devices */
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
		if ((PdiPtr->SlrType != XLOADER_SSIT_MONOLITIC) &&
			(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID == PM_DEV_PLD_0)) {
			XPlmi_EnableSsitErrors();
		}
#endif

		if ((PdiPtr->DelayLoad == (u8)TRUE) ||
			(PdiPtr->DelayHandoff == (u8)TRUE)) {
			continue;
		}

		Status = XLoader_StartImage(PdiPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Check for EAM errors */
		(void)XPlmi_ErrorTaskHandler(NULL);
	}

	/** Execute Delay Handoff after all images are loaded. */
	for (Index = 0U; Index < NoOfDelayedHandoffCpus; ++Index) {
		ImageNum = DelayHandoffImageNum[Index];
		PrtnNum = DelayHandoffPrtnNum[Index];
		PdiPtr->PrtnNum = PrtnNum;
		for (PrtnIndex = 0U;
			PrtnIndex < (u8)PdiPtr->MetaHdr.ImgHdr[ImageNum].NoOfPrtns;
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
	if (DeviceOps[PdiPtr->PdiIndex].Release != NULL) {
		SStatus = DeviceOps[PdiPtr->PdiIndex].Release();
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}

#ifndef PLM_DEBUG_MODE
	SStatus = XPlmi_MemSet(XPLMI_PMCRAM_CHUNK_MEMORY, XPLMI_DATA_INIT_PZM,
			XLOADER_TOTAL_CHUNK_SIZE / XPLMI_WORD_LEN);
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
 * @return
 * 			- XST_SUCCESS on success and error code on failure
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
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_LoadPdi(XilPdi* PdiPtr, PdiSrc_t PdiSrc, u64 PdiAddr)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PdiInit, PdiPtr,
			PdiSrc, PdiAddr);
	Status = XLoader_LoadAndStartSubSystemPdi(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks if the given ImgID is a child of Parent ImgID
 *
 * @param	ChildImgID is the current ImgID whose relation is to be checked
 * @param	ParentImgID is the ImgID with which the hierarchical ImgIDs are
 * 			compared in order to get the correct parent-child relationship
 * @param	IsChild is the pointer to the Relation that has to be
 *			returned, TRUE if it is a Child, else FALSE
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_PARENT_QUERY_RELATION_CHECK if parent query fails while
 * 			checking child relation.
 *
 *****************************************************************************/
static int XLoader_GetChildRelation(u32 ChildImgID, u32 ParentImgID, u32 *IsChild)
{
	int Status = XST_FAILURE;
	u32 TempParentImgID = ChildImgID;

	while (TempParentImgID != 0U) {
		Status = XPm_Query((u32)XPM_QID_PLD_GET_PARENT, TempParentImgID, 0U, 0U,
				&TempParentImgID);
		if (Status != XST_SUCCESS) {
			Status = (int)XLOADER_ERR_PARENT_QUERY_RELATION_CHECK;
			goto END;
		}

		if (TempParentImgID == ParentImgID) {
			*IsChild = (u32)TRUE;
			break;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function invalidates the child image info entries
 * 			corresponding to the parent image id in the ImageInfo Table
 *
 * @param	ParentImgID whose corresponding child image info entries are
 * 			invalidated
 * @param	ChangeCount points to ChangeCount that has to be modified
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_InvalidateChildImgInfo(u32 ParentImgID, u32 *ChangeCount)
{
	int Status = XST_FAILURE;
	u32 IsChild;
	u32 Index;
	u32 NodeId;
	XLoader_ImageInfoTbl *ImageInfoTbl = XLoader_GetImageInfoTbl();
	XLoader_ImageInfo *ImageInfoTblPtr = (XLoader_ImageInfo *)
		XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR;

	for (Index = 0U; Index < ImageInfoTbl->Count; Index++) {
		NodeId = NODESUBCLASS(ImageInfoTblPtr[Index].ImgID);
		/*
		 * Only PL and AIE images are supporting hierarchical DFX,
		 * so only those image entries need to be invalidated
		 */
		if (XLoader_IsDFxApplicable(NodeId) != (u8)TRUE) {
			continue;
		}
		IsChild = (u32)FALSE;
		Status = XLoader_GetChildRelation(ImageInfoTblPtr[Index].ImgID,
			ParentImgID, &IsChild);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (IsChild == (u32)TRUE) {
			ImageInfoTbl->Count--;
			Status = Xil_SMemCpy(&ImageInfoTblPtr[Index],
				sizeof(XLoader_ImageInfo),
				&ImageInfoTblPtr[ImageInfoTbl->Count],
				sizeof(XLoader_ImageInfo),
				sizeof(XLoader_ImageInfo));
			if (Status != XST_SUCCESS) {
				goto END;
			}
			(*ChangeCount)++;
			ImageInfoTbl->IsBufferFull = (u8)FALSE;
			--Index;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns the ImageInfoEntry by checking if an entry
 * 			exists for that particular ImgId in the ImgInfoTbl
 *
 * @param	ImgID of the the entry that has to be stored
 *
 * @return	Address of ImageInfo Entry in the table
 *
 *****************************************************************************/
XLoader_ImageInfo* XLoader_GetImageInfoEntry(u32 ImgID)
{
	XLoader_ImageInfo *ImageEntry = NULL;
	u32 Index;
	XLoader_ImageInfoTbl *ImageInfoTbl = XLoader_GetImageInfoTbl();
	XLoader_ImageInfo *ImageInfoTblPtr = (XLoader_ImageInfo *)
		XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR;

	/**
	 * Check for a existing valid image entry matching given ImgID
	 */
	for (Index = 0U; Index < ImageInfoTbl->Count; Index++) {
		if (ImageInfoTblPtr[Index].ImgID == ImgID) {
			ImageEntry = &ImageInfoTblPtr[Index];
			goto END;
		}
	}
	if (ImageInfoTbl->Count < XLOADER_IMAGE_INFO_TBL_MAX_NUM) {
		ImageEntry = &ImageInfoTblPtr[ImageInfoTbl->Count];
		ImageEntry->ImgID = XLOADER_INVALID_IMG_ID;
	}

END:
	return ImageEntry;
}

/*****************************************************************************/
/**
 * @brief	This function stores the ImageInfo to Image Info Table
 *
 * @param	ImageInfo is a Pointer to ImageInfo that has to be written.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_INVALIDATE_CHILD_IMG when invalidating child image entry.
 * 			- XLOADER_ERR_IMAGE_INFO_TBL_OVERFLOW when image info table overflows.
 * 			- XPLMI_ERR_MEMCPY_IMAGE_INFO if failed to copy image info to memory.
 *
 *****************************************************************************/
static int XLoader_StoreImageInfo(const XLoader_ImageInfo *ImageInfo)
{
	int Status = XST_FAILURE;
	XLoader_ImageInfo *ImageEntry;
	u32 ChangeCount;
	u32 RtCfgLen;
	u32 NodeId = NODESUBCLASS(ImageInfo->ImgID);
	XLoader_ImageInfoTbl *ImageInfoTbl = XLoader_GetImageInfoTbl();

	if (ImageInfo->ImgID == PM_DEV_PLD_0) {
		XPlmi_Out32(XPLMI_RTCFG_USR_ACCESS_ADDR, ImageInfo->FuncID);
	}
	/* Read ChangeCount */
	ChangeCount = ((XPlmi_In32(XPLMI_RTCFG_IMGINFOTBL_LEN_ADDR) &
			XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_MASK)
			>> XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_SHIFT);

	if ((ImageInfo->UID != XLOADER_INVALID_UID) &&
		(XLoader_IsDFxApplicable(NodeId) == (u8)TRUE)) {
		/**
		 * Check if there is a valid image Entry which is matching
		 * with the ImgID
		 */
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
		ImageInfoTbl->IsBufferFull = (u8)TRUE;
		Status = XPlmi_UpdateStatus(XLOADER_ERR_IMAGE_INFO_TBL_OVERFLOW, 0);
		goto END;
	}

	if (ImageEntry->ImgID == XLOADER_INVALID_IMG_ID) {
		ImageInfoTbl->Count++;
	}
	ChangeCount++;
	/**
	 * Store the Image Info if the provided Image is valid
	 */
	Status = Xil_SMemCpy(ImageEntry, sizeof(XLoader_ImageInfo), ImageInfo,
			sizeof(XLoader_ImageInfo), sizeof(XLoader_ImageInfo));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_MEMCPY_IMAGE_INFO, Status);
		goto END;
	}

	/* Update ChangeCount and number of entries in the RunTime config register */
	RtCfgLen = (ImageInfoTbl->Count & XPLMI_RTCFG_IMGINFOTBL_NUM_ENTRIES_MASK);
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
 * @param	DestAddr is the 64 bit Destination Address
 * @param	MaxSize is the max size of Buffer present at Destination Address
 * @param	NumEntries that are loaded from the Image Info Table
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_INVALID_DEST_IMGINFOTBL_SIZE if destination buffer size
 * 			is less than the current length of the image info table.
 * 			- XLOADER_ERR_IMAGE_INFO_TBL_FULL if image info table is full.
 *
 *****************************************************************************/
int XLoader_LoadImageInfoTbl(u64 DestAddr, u32 MaxSize, u32 *NumEntries)
{
	int Status = XST_FAILURE;
	u32 MaxLen = MaxSize / sizeof(XLoader_ImageInfo);
	u32 Len;
	u32 SrcAddr = XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR;
	XLoader_ImageInfoTbl *ImageInfoTbl = XLoader_GetImageInfoTbl();

	/**
	 * Validate the available length of ImageInfoTable is in the range of
	 * Maximum buffer size available
	 */
	if (ImageInfoTbl->Count > MaxLen) {
		Status = (int)XLOADER_ERR_INVALID_DEST_IMGINFOTBL_SIZE;
		goto END;
	}

	/**
	 * Copy the ImageInfoTable for the given length of table
	*/
	Len = ImageInfoTbl->Count * sizeof(XLoader_ImageInfo);
	Status = XPlmi_DmaXfr((u64)SrcAddr, DestAddr, (Len >> XPLMI_WORD_LEN_SHIFT),
		XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (ImageInfoTbl->IsBufferFull == (u8)TRUE) {
		Status = (int)XLOADER_ERR_IMAGE_INFO_TBL_FULL;
		XPlmi_Printf(DEBUG_INFO, "Image Info Table Overflowed\r\n");
	}
	*NumEntries = ImageInfoTbl->Count;

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function validates the UIDs in Image Header
 *
 * @param	ImageInfo is pointer to the image info in image
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_DEV_NOT_DEFINED if Device ID of the image to be loaded
 * 			is not defined.
 * 			- XLOADER_ERR_PARENT_QUERY_VERIFY if Failed to Query Parent ID of
 * 			an image while verifying its Image UIDs.
 * 			- XLOADER_ERR_INVALID_PARENT_IMG_ID when Invalid ParentImgID is
 * 			obtained when queried for parent ImgID.
 * 			- XLOADER_ERR_NO_VALID_PARENT_IMG_ENTRY if No Valid Parent Image
 * 			Entry is found in the ImageInfo Table.
 * 			- XLOADER_ERR_INCOMPATIBLE_CHILD_IMAGE if child image is not
 * 			compatible with parent image.
 *
 *****************************************************************************/
static int XLoader_VerifyImgInfo(const XLoader_ImageInfo *ImageInfo)
{
	int Status = XST_FAILURE;
	XPm_DeviceStatus DeviceStatus;
	const XLoader_ImageInfo *ParentImageInfo;
	u32 ParentImgID = XLOADER_INVALID_IMG_ID;
	u32 NodeId = NODESUBCLASS(ImageInfo->ImgID);

	/**
	 * Validate both the Image ID and UID are Non-Zero and perform the
	 * compatibility check
	*/
	if ((ImageInfo->ImgID != XLOADER_INVALID_IMG_ID) &&
		(ImageInfo->UID != XLOADER_INVALID_UID) &&
		(XLoader_IsDFxApplicable(NodeId) == (u8)TRUE)) {
		Status = XPmDevice_GetStatus(PM_SUBSYS_PMC, ImageInfo->ImgID,
				&DeviceStatus);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_DEV_NOT_DEFINED,
					Status);
			goto END;
		}

		if (ImageInfo->PUID != XLOADER_INVALID_UID) {
			/**
			 * Query the parent Image ID for compatibility check using XilPM API
			 */
			Status = XPm_Query((u32)XPM_QID_PLD_GET_PARENT,
				ImageInfo->ImgID, 0U, 0U, &ParentImgID);
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

			/**
			 * Get the ImageEntry stored in the ImageInfoTable using
			 * parent Image ID
			*/
			ParentImageInfo = XLoader_GetImageInfoEntry(ParentImgID);
			if (ParentImageInfo == NULL) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_NO_VALID_PARENT_IMG_ENTRY, 0);
				goto END;
			}

			/**
			 * Validate the Parent Image UID with the current Image PUID
			*/
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
 * 			images present in it. This can be used to load a single image
 * 			like PL, APU, RPU. This will load all the partitions that are
 * 			present in that image.
 *
 * @param	PdiPtr is Pdi instance pointer
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_SEM_STOP_SCAN if unable to stop SEM scan.
 * 			- XLOADER_ERR_CONFIG_SUBSYSTEM if subsystem configuration fails.
 * 			- XLOADER_ERR_SEM_INIT if unable to start the SEM scan.
 *
 *****************************************************************************/
static int XLoader_LoadImage(XilPdi *PdiPtr)
{
	volatile int Status = XST_FAILURE;
	u32 NodeId = NODESUBCLASS(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
	XLoader_ImageMeasureInfo ImageMeasureInfo = {0U};
	u32 PcrInfo = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].PcrInfo;

#ifdef XPLM_SEM
	/* Stop the SEM scan before PL load */
	if ((PdiPtr->PdiType != XLOADER_PDI_TYPE_FULL) &&
		(NodeId == (u32)XPM_NODESUBCL_DEV_PL)) {
		Status = XSem_StopScan();
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_SEM_STOP_SCAN, Status);
			goto END;
		}
	}
#endif
	Status = XLoader_VerifyImgInfo((const XLoader_ImageInfo *)
		&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Configure preallocs for subsystem */
	if (NODECLASS(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID)
			== XPM_NODECLASS_SUBSYSTEM) {
		Status = XPmSubsystem_Configure(
			PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_CONFIG_SUBSYSTEM, Status);
			goto END;
		}
	}

	PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgName[XILPDI_IMG_NAME_ARRAY_SIZE - 1U] = 0;
	/* Update current subsystem ID for EM */
	XPlmi_SetEmSubsystemId(&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);

	ImageMeasureInfo.PcrInfo = PcrInfo;
	ImageMeasureInfo.Flags = XLOADER_MEASURE_START;
	ImageMeasureInfo.SubsystemID = PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID;
	ImageMeasureInfo.DigestIndex = &PdiPtr->DigestIndex;
	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_PARTIAL) {
		ImageMeasureInfo.OverWrite = TRUE;
	}
	else {
		ImageMeasureInfo.OverWrite = FALSE;
	}
	/* This is applicable only for Versal Net */
	Status = XLoader_DataMeasurement(&ImageMeasureInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_LoadImagePrtns(PdiPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ImageMeasureInfo.Flags = XLOADER_MEASURE_FINISH;
	/* This is applicable only for Versal Net */
	Status = XLoader_DataMeasurement(&ImageMeasureInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* This is applicable only for VersalNet */
	Status = XPlmi_CheckAndUpdateFipsState();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((PdiPtr->DelayLoad == (u8)FALSE) &&
		(PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID !=
		XLOADER_INVALID_IMG_ID)) {
		Status = XLoader_StoreImageInfo((const XLoader_ImageInfo *)
		&PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	/* Log the image load to the Trace Log buffer */
	XPlmi_TraceLog3(XPLMI_TRACE_LOG_LOAD_IMAGE,
		PdiPtr->MetaHdr.ImgHdr[PdiPtr->ImageNum].ImgID);
	/* Apply MJTAG Work-around after first PL loading */
	if (NodeId == (u32)XPM_NODESUBCL_DEV_PL) {
		/* Apply MJTAG workaround only if bootmode is other than JTAG */
		if (PdiPtr->PdiSrc != XLOADER_PDI_SRC_JTAG) {
			XLoader_SetJtagTapToReset();
		}
	}

#ifdef XPLM_SEM
	/* Resume the SEM scan after PL load */
	if ((PdiPtr->PdiType != XLOADER_PDI_TYPE_FULL) &&
		(NodeId == (u32)XPM_NODESUBCL_DEV_PL)) {
		Status = XST_FAILURE;
		Status = XSem_InitScan();
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_SEM_INIT, Status);
		}
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to restart the image in PDI. This function
 * 			will take ImageId as an input and based on the subsystem info
 * 			available, it will read the image partitions, loads them and
 * 			hand-off to the required CPUs as part of the image load.
 *
 * @param	ImageId Id of the image present in PDI
 * @param	FuncID is verified with the FuncID present in PDI
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_RestartImage(u32 ImageId, u32 *FuncID)
{
	int Status = XST_FAILURE;
	XilPdi *PdiPtr = XLoader_GetPdiInstance();
	const XLoader_ImageStore *PdiList = XLoader_GetPdiList();
	int Index;
	u64 PdiAddr;
	XilBootPdiInfo *BootPdiInfo = XLoader_GetBootPdiInfo();


	/**
	 * - Scan through PdiList for the given ImageId and restart image from
	 * that respective PDI if ImageId is found
	 */
	 PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;
	 PdiPtr->PdiSrc = XLOADER_PDI_SRC_DDR;
	for (Index = (int)PdiList->Count - 1; Index >= 0; Index--) {
		PdiAddr = PdiList->ImgList[Index].PdiAddr;
		Status = XLoader_PdiInit(PdiPtr, XLOADER_PDI_SRC_DDR, PdiAddr);
		if (Status != XST_SUCCESS) {
			goto END1;
		}

		XPlmi_Printf(DEBUG_GENERAL, "Loading from PdiAddr: "
			"0x%0x%08x\n\r", (u32)(PdiAddr >> 32U),
			(u32)PdiAddr);
		Status = XLoader_ReloadImage(PdiPtr, ImageId,
				FuncID);
		if (Status != XST_SUCCESS) {
			goto END1;
		}

		Status = XLoader_StartImage(PdiPtr);
END1:
		if (Status != XST_SUCCESS) {
			if ((Status != (int)XLOADER_ERR_IMG_ID_NOT_FOUND) &&
				(Status != (int)XLOADER_ERR_FUNCTION_ID_MISMATCH)) {
				XPlmi_Printf(DEBUG_GENERAL, "Restart Image "
					"Failed with PLM Error: 0x%0x\n\r",
					Status);
				XPlmi_Printf(DEBUG_GENERAL, "Fallback to next"
					" PDI started...\n\r");
			}
		}
		else {
			goto END;
		}
	}

	/**
	 * - Load image from BootPdi if the image is not found or loading is
	 * unsuccessful from the PdiList
	 */
	Status = XST_FAILURE;
	if ((BootPdiInfo->PdiSrc == XLOADER_PDI_SRC_JTAG) ||
			(BootPdiInfo->PdiSrc == XLOADER_PDI_SRC_SMAP)) {
		XPlmi_Printf(DEBUG_GENERAL, "Fallback failed invalid BootSrc JTAG/SMAP\n\r");
		goto END;
	}
	PdiPtr->PdiType = XLOADER_PDI_TYPE_FULL;
	PdiPtr->DiscardUartLogs = (u8)FALSE;
	XPlmi_Printf(DEBUG_GENERAL, "Loading from BootPdi\n\r");
	Status = XLoader_PdiInit(PdiPtr, BootPdiInfo->PdiSrc , 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XLoader_ReloadImage(PdiPtr, ImageId, FuncID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_StartImage(PdiPtr);

END:
	if (Status == XST_SUCCESS) {
		XPlmi_Out32(PMC_GLOBAL_DONE, XLOADER_PDI_LOAD_COMPLETE);
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to reload the image only in PDI. This
 * 			function will take ImageId as an input and based on the subsystem info
 * 			available, it will read the image partitions and loads them.
 *
 * @param	PdiPtr is Pdi instance pointer
 * @param	ImageId Id of the image present in PDI
 * @param	FuncID is verified with the FuncID present in PDI
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_IMG_ID_NOT_FOUND if image id is not found in subsystem
 * 			when reloading image.
 * 			- XLOADER_ERR_FUNCTION_ID_MISMATCH when Function ID given while
 * 			loading Image from DDR is not matching with the ID stored in Image Header.
 *
 *****************************************************************************/
static int XLoader_ReloadImage(XilPdi *PdiPtr, u32 ImageId, const u32 *FuncID)
{
	int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	u32 PdiSrc = PdiPtr->PdiSrc;
	u8 PdiIndex = PdiPtr->PdiIndex;
	u8 PdiType = PdiPtr->PdiType;
	u8 PrtnNum = 0U;
	u8 Index;

	for (Index = 0U; Index < (u8)PdiPtr->MetaHdr.ImgHdrTbl.NoOfImgs;
		++Index) {
		if (PdiPtr->MetaHdr.ImgHdr[Index].ImgID == ImageId) {
			PdiPtr->ImageNum = Index;
			PdiPtr->PrtnNum = PrtnNum;
			break;
		}
		PrtnNum += (u8)PdiPtr->MetaHdr.ImgHdr[Index].NoOfPrtns;
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

	PdiPtr->DelayHandoff = (u8)FALSE;
	PdiPtr->DelayLoad = (u8)FALSE;

	Status = XLoader_LoadImage(PdiPtr);

END:
	if ((PdiPtr->PdiIndex != XLOADER_DDR_INDEX) &&
		(DeviceOps[PdiPtr->PdiIndex].Release != NULL)) {
		SStatus = DeviceOps[PdiPtr->PdiIndex].Release();
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}
	PdiPtr->PdiSrc = PdiSrc;
	PdiPtr->PdiIndex = PdiIndex;
	PdiPtr->MetaHdr.DeviceCopy = DeviceOps[PdiPtr->PdiIndex].Copy;
	PdiPtr->PdiType = PdiType;
#ifndef PLM_DEBUG_MODE
	SStatus = XPlmi_MemSet(XPLMI_PMCRAM_CHUNK_MEMORY, XPLMI_DATA_INIT_PZM,
			XLOADER_TOTAL_CHUNK_SIZE / XPLMI_WORD_LEN);
	if ((Status == XST_SUCCESS) && (SStatus != XST_SUCCESS)) {
		Status = SStatus;
	}
#endif
	return Status;
}

/****************************************************************************/
/**
*  @brief	This function performs the checks of IDCODE and EXTENDED IDCODE.
*  			It also supports bypass of subset of these checks.
*
* @param	ImgHdrTbl pointer to the image header table.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_EXT_ID_SI if extended ID code is mismatched with device.
* 			- XLOADER_ERR_IDCODE on ID code mismatch.
* 			- XLOADER_ERR_EXT_IDCODE on extended ID code mismatch.
*
*****************************************************************************/
int XLoader_IdCodeCheck(const XilPdi_ImgHdrTbl * ImgHdrTbl)
{
	int Status = XST_FAILURE;
	u32 IdCodeIHT; /**< IdCode as read from IHT */
	u32 ExtIdCodeIHT; /**< Extended IdCode as read from IHT */
	u32 IdCodeRd; /**< IdCode as read from Device */
	u32 ExtIdCodeRd; /**< Extended IdCode as read from Device */
	u8 BypassChkIHT; /**< Flag to bypass checks */
	u8 IsVC1902Es1; /**< Flag to indicate IsVC1902-ES1 device */
	u8 IsExtIdCodeZero; /**< Flag to indicate Extended IdCode is valid */

	/** - check to bypass entire ID Code Check */
	if ((ImgHdrTbl->Attr & XIH_IHT_ATTR_BYPS_ID_CODE_MASK)
					== XIH_IHT_ATTR_BYPS_ID_CODE_MASK) {
		Status = XST_SUCCESS;
		goto END;
	}

	/**
	 * - Read IdCode and extended Id Code from the image header table and
	 * from PMC_TAP register
	*/
	IdCodeIHT = ImgHdrTbl->Idcode;
	IdCodeRd = XPlmi_In32(PMC_TAP_IDCODE);
	ExtIdCodeIHT = ImgHdrTbl->ExtIdCode &
				XIH_IHT_EXT_IDCODE_MASK;
	ExtIdCodeRd = Xil_In32(EFUSE_CACHE_IP_DISABLE_0)
			& EFUSE_CACHE_IP_DISABLE_0_EID_MASK;

	/** - Determine and fetch the Extended IDCODE (out of two) for checks */
	if (0U == ExtIdCodeRd) {
		IsExtIdCodeZero = (u8)TRUE;
	}
	else {
		IsExtIdCodeZero = (u8)FALSE;

		if ((ExtIdCodeRd &
			EFUSE_CACHE_IP_DISABLE_0_EID_SEL_MASK) == 0U) {
			ExtIdCodeRd = (ExtIdCodeRd &
				EFUSE_CACHE_IP_DISABLE_0_EID1_MASK)
				>> EFUSE_CACHE_IP_DISABLE_0_EID1_SHIFT;
		}
		else {
			ExtIdCodeRd = (ExtIdCodeRd &
					EFUSE_CACHE_IP_DISABLE_0_EID2_MASK)
					>> EFUSE_CACHE_IP_DISABLE_0_EID2_SHIFT;
		}
	}

	/** - Check if the IdCode read is VC1902 ES1 */
	if ((IdCodeRd & PMC_TAP_IDCODE_SIREV_DVCD_MASK) ==
			PMC_TAP_IDCODE_ES1_VC1902) {
		IsVC1902Es1 = (u8)TRUE;
	}
	else {
		IsVC1902Es1 = (u8)FALSE;
	}

	/** - Check if a subset of checks to be bypassed */
	if (0x1U == (ImgHdrTbl->Attr & XIH_IHT_ATTR_BYPS_MASK)) {
		BypassChkIHT = (u8)TRUE;
	}
	else {
		BypassChkIHT = (u8)FALSE;
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

	/**
	 * - Error out for the invalid combination of Extended IDCODE - Device.
	 * Assumption is that only VC1902-ES1 device can have Extended IDCODE value 0
	 */
	if ((IsExtIdCodeZero == (u8)TRUE) &&
			(IsVC1902Es1 == (u8)FALSE)) {
		Status = (int)XLOADER_ERR_EXT_ID_SI;
		goto END;
	}
	else {
		/* Do not check Si revision if bypass configured */
		if ((u8)TRUE == BypassChkIHT) {
			IdCodeIHT &= ~PMC_TAP_IDCODE_SI_REV_MASK;
			IdCodeRd &= ~PMC_TAP_IDCODE_SI_REV_MASK;
		}

		/* Do the actual IDCODE check */
		if (IdCodeIHT != IdCodeRd) {
			Status = (int)XLOADER_ERR_IDCODE;
			goto END;
		}

		/* Do the actual Extended IDCODE check */
		if ((u8)FALSE == IsExtIdCodeZero) {
			if (ExtIdCodeIHT != ExtIdCodeRd) {
				Status = (int)XLOADER_ERR_EXT_IDCODE;
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
 * @brief	This function is used to load the primary and secondary PDIs.
 *
 * @param	PdiPtr is Pdi instance pointer
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE on unsupported secondary boot
 * 			mode.
 * 			- XLOADER_ERR_MEMSET_PDIPTR if failed to create instance for PdiPtr.
 *
 *****************************************************************************/
static int XLoader_LoadAndStartSecPdi(XilPdi* PdiPtr)
{
	int Status = XST_FAILURE;
	u32 PdiSrc;
	u64 PdiAddr = PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr;
	u32 SecBootMode = XilPdi_GetSBD(&(PdiPtr->MetaHdr.ImgHdrTbl)) >>
		XIH_IHT_ATTR_SBD_SHIFT;
	u32 PdiId;

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
		} else {
			switch(SecBootMode)
			{
				case XIH_IHT_ATTR_SBD_QSPI32:
					PdiSrc = XLOADER_PDI_SRC_QSPI32;
					break;
				case XIH_IHT_ATTR_SBD_QSPI24:
					PdiSrc = XLOADER_PDI_SRC_QSPI24;
					break;
				case XIH_IHT_ATTR_SBD_EMMC:
				#ifdef XLOADER_SD_1
					PdiSrc = XLOADER_PDI_SRC_EMMC1 |
						XLOADER_SD_RAWBOOT_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						<< XLOADER_SD_ADDR_SHIFT);
				#else
					PdiSrc = XLOADER_PDI_SRC_EMMC1;
				#endif
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_OSPI:
					PdiSrc = XLOADER_PDI_SRC_OSPI;
					break;
				case XIH_IHT_ATTR_SBD_USB:
					PdiSrc = XLOADER_PDI_SRC_USB;
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_SMAP:
					PdiSrc = XLOADER_PDI_SRC_SMAP;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_RAW:
					PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_EMMC1;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_0:
				#ifdef XLOADER_SD_0
					PdiSrc = XLOADER_PDI_SRC_EMMC0 |
						XLOADER_SD_RAWBOOT_MASK |
						(PdiPtr->MetaHdr.ImgHdrTbl.SBDAddr
						 << XLOADER_SD_ADDR_SHIFT);
				#else
					PdiSrc = XLOADER_PDI_SRC_EMMC0;
				#endif
					PdiAddr = 0U;
					break;
				case XIH_IHT_ATTR_SBD_EMMC_0_RAW:
					PdiSrc = XLOADER_SD_RAWBOOT_VAL | XLOADER_PDI_SRC_EMMC0;
					break;
				case XIH_IHT_ATTR_IMAGE_STORE:
					PdiId = (u32)PdiAddr;
					Status = XLoader_IsPdiAddrLookup(PdiId, (u64*)&PdiAddr);
					if (Status != XST_SUCCESS) {
						goto END;
					}
					PdiSrc = XLOADER_PDI_SRC_DDR;
					break;
				default:
					Status = XLoader_GetSDPdiSrcNAddr(SecBootMode,
							PdiPtr, &PdiSrc, &PdiAddr);
					break;
			}

			if (Status == (int)XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE) {
				Status = XPlmi_UpdateStatus((XPlmiStatus_t)Status, 0);
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
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to read the Image Store DDR Memory Addr
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_ReadImageStoreCfg(void)
{
	int Status = XST_FAILURE;
	XLoader_ImageStore *PdiList = XLoader_GetPdiList();
	u32 Unaligned;

	/**
	 * - Read & Populate Image Store base address,size from RTCA area.
	 */
	PdiList->PdiImgStrAddr = XPlmi_In32(XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH);
	PdiList->PdiImgStrAddr = XPlmi_In32(XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW) | (PdiList->PdiImgStrAddr << 32U);
	PdiList->PdiImgStrSize = XPlmi_In32(XPLMI_RTCFG_IMG_STORE_SIZE);

	/**
	 * - Validate if address is unaligned.
	 */
	Unaligned = (u32)(PdiList->PdiImgStrAddr % XPLMI_WORD_LEN);

	if((PdiList->PdiImgStrAddr == XLOADER_IMG_STORE_INVALID_ADDR) ||
			(PdiList->PdiImgStrSize == XLOADER_IMG_STORE_INVALID_SIZE) || Unaligned) {
		XPlmi_Printf(DEBUG_INFO,"Image Store Configuration not Set\n\r");
		PdiList->PdiImgStrSize = XLOADER_IMG_STORE_INVALID_SIZE;
		goto END;
	} else {
		Status = XST_SUCCESS;
	}

	/**
	 * - Update the PDI image store address in the image list.
	 */
	PdiList->ImgList[0U].PdiAddr = PdiList->PdiImgStrAddr;

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function is used to read the Address based-on PDI Id from
 *              Image Store
 *
 * @return
 *		- XST_SUCCESS on success
 *		- XLOADER_ERR_PDI_LIST_EMPTY if PDI list is empty
 *		- XLOADER_ERR_PDI_ADDR_NOT_FOUND if PDI ID not found in list
 *
 *****************************************************************************/
int XLoader_IsPdiAddrLookup(u32 PdiId, u64 *PdiAddr)
{
	int Status = XST_FAILURE;
	int Index;
	const XLoader_ImageStore *PdiList = XLoader_GetPdiList();

	if (PdiAddr ==  NULL){
		goto END;
	}

	if (PdiList->Count == 0U) {
		Status = (int)XLOADER_ERR_PDI_LIST_EMPTY;
		goto END;
	}

	for (Index = (int)PdiList->Count - 1; Index >= 0; Index--) {
		if (PdiList->ImgList[Index].PdiId  == PdiId) {
			*PdiAddr = PdiList->ImgList[Index].PdiAddr;
			break;
		}
	}

	if (Index < 0) {
		Status = (int)XLOADER_ERR_PDI_ADDR_NOT_FOUND;
		XPlmi_Printf(DEBUG_GENERAL, "Image Store PdiId:0x%x Not Found\n\r",PdiId);
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to save the BootPDI info
 *
 *****************************************************************************/
void Xloader_SaveBootPdiInfo(XilPdi *BootPdiPtr)
{
	XilBootPdiInfo *BootPdiInfo = XLoader_GetBootPdiInfo();

	if ((NULL == BootPdiPtr) || (NULL == BootPdiInfo)) {
		goto END;
	}

#ifndef PLM_SECURE_EXCLUDE
	BootPdiInfo->DecKeySrc = BootPdiPtr->DecKeySrc;
	BootPdiInfo->PlmKatStatus = BootPdiPtr->PlmKatStatus;
#endif
	BootPdiInfo->MetaHdrOfst = BootPdiPtr->MetaHdr.BootHdrPtr->BootHdrFwRsvd.MetaHdrOfst;
	BootPdiInfo->PdiSrc = BootPdiPtr->PdiSrc;

END:
	return;
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

	return &PdiInstance;
}
