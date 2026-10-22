/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xloader_ddr.c
*
* This is the file which contains DDR init and copy functions
* related code for the platform loader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/12/2019 Initial release
* 1.01  kc   09/04/2019 Added support to use non blocking DMA in
*						DdrCopy function
* 1.02  bsv  04/09/2020 Code clean up of Xilloader
* 1.03  skd  07/14/2020 Added 64bit support for DDR source address
*       bsv  07/29/2020 Added provision to use PMCDMA0 for Ddr Copy
*       skd  07/29/2020 Updated device copy macros
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP and PCIE
*                       boot modes
*       bsv  10/13/2020 Code clean up
* 1.04  bsv  08/31/2021 Code clean up
* 1.05  bsv  10/26/2021 Code clean up
*       bm   07/06/2022 Refactor versal and versal_net code
*       is   09/12/2022 Remove PM_CAP_SECURE capability when requesting DDR_0
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       dd   09/11/2023 MISRA-C violation Rule 17.8 fixed
*       pre  08/17/2026 Added DDR calibration data restore and status check APIs
*       pre  10/22/2026 Added DDR calibration restore for primary boot PDI
*                       loading using DDRMC physical address and offset pairs,
*                       the primary boot mode as the storage device, and no RTCA
*                       mask. Accepts a short CDO chunk until the full payload
*                       arrives, and is disabled by default under PLM_ENABLE_DDR_CALIB_RESTORE
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_util.h"
#include "xloader_ddr.h"
#include "xplmi.h"
#include "xplmi_dma.h"
#include "xpm_api.h"
#include "xpm_nodeid.h"
#include "xloader_plat.h"
#include "xloader_qspi.h"
#include "xloader_ospi.h"
#include "xloader_sd.h"
#ifdef VERSAL_2VE_2VM
#include "xloader_ufs.h"
#endif
#include "xilpdi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef PLM_ENABLE_DDR_CALIB_RESTORE
#define XLOADER_CALIBDATA_IDCODE_INDEX      (0U)
        /**< Index of the IDCODE word in a calibration data record */
#define XLOADER_CALIBDATA_CHECKSUM_START_INDEX  (1U)
        /**< Index of the first word of a calibration data record covered by the checksum */
#define XLOADER_CALIBDATA_METADATA_SIZE     (2U)
        /**< Number of words of a calibration data record taken by the IDCODE word and the
             checksum word */
#define XLOADER_CALIBDATA_CMD_WORDS_PER_MC	(2U)
		/**< Payload words per warm-boot DDR MC: physical address, then offset */
#define XLOADER_CALIBDATA_PHYS_ADDR_INDEX	(0U)
		/**< Index of the DDR MC physical address within one payload pair */
#define XLOADER_CALIBDATA_OFFSET_INDEX		(1U)
		/**< Index of the DDR MC storage offset within one payload pair */
#define XLOADER_CALIBDATA_FILE_NAME		"calib00.bin"
		/**< SD/UFS calibration data filename template */
#define XLOADER_CALIBDATA_FILE_TENS_INDEX	(5U)
		/**< Index of the DDR MC number tens digit in the filename */
#define XLOADER_CALIBDATA_FILE_UNITS_INDEX	(6U)
		/**< Index of the DDR MC number units digit in the filename */
#define XLOADER_CALIBDATA_FILE_RADIX		(10U)
		/**< Radix used to encode the DDR MC number in the filename */
#endif

/************************** Function Prototypes ******************************/
#ifdef PLM_ENABLE_DDR_CALIB_RESTORE
static void *XLoader_GetCalibDataSrc(u8 StorageIndex, u32 DDRMCNumber, u32 Offset);
static int XLoader_GetCalibDataStorageIndex(u8 *StorageIndex);
static int XLoader_GetDdrMcNumber(u32 PhysAddr, u32 *DDRMCNumber);
static int XLoader_FetchCalibDataFromNvStorage(u8 StorageIndex, void *CalibDataSrc,
		u32 *DestBuffer);
static int XLoader_ReleaseCalibDataStorage(u8 StorageIndex);
static int XLoader_CheckCalibDataIntegrity(const u32 *CalibData);
static int XLoader_WriteCalibDataToXRam(u32 XRamAddr, const u32 *CalibData);
static int XLoader_SetDdrMcCalibSkipFlag(u32 DDRMCNumber, u8 Enable);
#endif

/************************** Variable Definitions *****************************/
static u8 DdrRequested = (u8)FALSE;
#ifdef PLM_ENABLE_DDR_CALIB_RESTORE
static u32 CalibDataTempBuffer[XLOADER_CALIBDATA_SIZE];
/** SD/UFS calibration filename. The two digits are updated for each DDR MC. */
static char CalibDataFileName[] = XLOADER_CALIBDATA_FILE_NAME;
#endif

/*****************************************************************************/
/**
 * @brief	This function is used to initialize for DDR init.
 *
 * @param	DeviceFlags Loader init prototype requires flags
 *
 * @return
 * 			- XST_SUCCESS on success
 * 			- XLOADER_ERR_PM_DEV_DDR_0 on device request to DDR fail.
 *
 *****************************************************************************/
int XLoader_DdrInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
#ifndef VERSAL_2VE_2VM
	u32 CapAccess = (u32)PM_CAP_ACCESS;
	u32 CapContext = (u32)PM_CAP_CONTEXT;
#endif

	if (DdrRequested == (u8)FALSE) {
		/**
		 * - Initialize the device request for DDR_0.
		 * - Otherwise return XLOADER_ERR_PM_DEV_DDR_0.
		*/
#ifdef VERSAL_2VE_2VM
		Status = XPm_PmcRequestDevice(PM_DEV_DDR_0);
#else
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_DDR_0,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
#endif
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_DDR_0, XIL_SIGNED_ZERO);
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
	}
	DdrRequested = (u8)DeviceFlags;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from DDR to destination
 * 			address.
 *
 * @param	SrcAddr of DDR
 * @param	DestAddr is the address of the destination where the data needs
 *			to be copied.
 * @param	Length of the bytes to be copied
 * @param	FlagsVal that denote blocking / non-blocking dma
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_DDR_COPY_UNSUPPORTED_PARAMS on invalid params passed.
 *
 *****************************************************************************/
int XLoader_DdrCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 FlagsVal)
{
	int Status = XST_FAILURE;
	u32 DmaFlags;
	u32 Flags = FlagsVal;

	/**
	 * - Validate the source address, destination address and length of the
	 * bytes. Otherwise return XLOADER_DDR_COPY_UNSUPPORTED_PARAMS.
	*/
	if (((SrcAddr & XPLMI_WORD_LEN_MASK) != 0U) ||
		((DestAddr & XPLMI_WORD_LEN_MASK) != 0U) ||
		((Length & XPLMI_WORD_LEN_MASK) != 0U)) {
		Status = XPlmi_UpdateStatus(XLOADER_DDR_COPY_UNSUPPORTED_PARAMS, XIL_SIGNED_ZERO);
		goto END;
	}
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		DmaFlags = XPLMI_PMCDMA_0;
	}
	else {
		DmaFlags = XPLMI_PMCDMA_1;
	}
	DmaFlags |= (Flags & (~(XPLMI_DEVICE_COPY_STATE_MASK)));
	Flags &= XPLMI_DEVICE_COPY_STATE_MASK;
	/**
	 * - If the flag is XPLMI_DEVICE_COPY_STATE_WAIT_DONE, then wait till the
	 * data is copied and DMA returns done.
	*/
	if (Flags == XPLMI_DEVICE_COPY_STATE_WAIT_DONE) {
		Status = XPlmi_WaitForNonBlkDma(DmaFlags);
		goto END;
	}

	/**
	 * - If the flag is XPLMI_DEVICE_COPY_STATE_INITIATE, then start the dma
	 * transfer and don't wait for the done bit.
	*/
	if (Flags == XPLMI_DEVICE_COPY_STATE_INITIATE) {
		DmaFlags |= XPLMI_DMA_SRC_NONBLK;
	}
	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Length >> (XPLMI_WORD_LEN_SHIFT),
		DmaFlags);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function releases control of DDR.
 *
 * @return
 * 			- XST_SUCCESS on success
 * 			- XLOADER_ERR_RELEASE_PM_DEV_DDR_0 on device release fail.
 *
 *****************************************************************************/
int XLoader_DdrRelease(void)
{
	int Status = XST_FAILURE;

	if (DdrRequested == XLOADER_PDI_SRC_DDR) {
		/**
		 * - Initialize the DDR_0 device release request.
		 * - Otherwise return XLOADER_ERR_RELEASE_PM_DEV_DDR_0.
		*/
#ifdef VERSAL_2VE_2VM
		Status = XPm_PmcReleaseDevice(PM_DEV_DDR_0);
#else
		Status = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_DDR_0,
			XPLMI_CMD_SECURE);
#endif
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_RELEASE_PM_DEV_DDR_0, XIL_SIGNED_ZERO);
			goto END;
		}
		DdrRequested = (u8)FALSE;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

#ifdef PLM_ENABLE_DDR_CALIB_RESTORE
/*************************************************************************************************/
/**
 * @brief	This function returns the calibration data source for one DDR MC.
 *		For QSPI/OSPI the source is the flash offset from the command. For
 *		SD/UFS the source is calibNN.bin, where NN is the DDR MC number.
 *
 * @param	StorageIndex is the storage index
 * @param	DDRMCNumber is the DDR MC number matched from the physical address
 * @param	Offset is the flash offset from the command, used for QSPI/OSPI
 *
 * @return
 * 		- Source address or filename pointer.
 * 		- XLOADER_INVALID_ADDR when the source cannot be determined.
 *
 **************************************************************************************************/
static void *XLoader_GetCalibDataSrc(u8 StorageIndex, u32 DDRMCNumber, u32 Offset)
{
	void *CalibDataSrc = (void *)(UINTPTR)XLOADER_INVALID_ADDR;

	/** QSPI/OSPI: the command offset is the flash address of the record */
	if ((StorageIndex == XLOADER_QSPI_INDEX) || (StorageIndex == XLOADER_OSPI_INDEX)) {
		if (Offset != (u32)XLOADER_INVALID_ADDR) {
			CalibDataSrc = (void *)(UINTPTR)Offset;
		}
	}
	/** SD/UFS: file name follows the DDR MC whose base address matched */
	else if (DDRMCNumber < (u32)MAX_DEV_DDRMC) {
		CalibDataFileName[XLOADER_CALIBDATA_FILE_TENS_INDEX] =
			(char)('0' + (DDRMCNumber / XLOADER_CALIBDATA_FILE_RADIX));
		CalibDataFileName[XLOADER_CALIBDATA_FILE_UNITS_INDEX] =
			(char)('0' + (DDRMCNumber % XLOADER_CALIBDATA_FILE_RADIX));
		CalibDataSrc = (void *)CalibDataFileName;
	}
	else {
		/* Required by MISRA */
	}

	return CalibDataSrc;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the base address of the DDRMC
 *		based on the DDR MC number.
 *
 * @param	DDRMCNumber is the DDR MC number
 *
 * @return
 * 		- Base address of the DDR MC.
 * 		- XLOADER_INVALID_ADDR when the DDR MC number or node is not valid.
 *
 **************************************************************************************************/
static u32 XLoader_GetDdrMcBaseAddr(u32 DDRMCNumber)
{
	u32 BaseAddr = (u32)XLOADER_INVALID_ADDR;
	XStatus Status = XST_FAILURE;
	static const u32 DdrMcDeviceId[MAX_DEV_DDRMC] = {
		XLOADER_DDRMC_LIST(XLOADER_DDRMC_DEVICE_ID)
	};

	/** Verify DDR MC number */
	if (DDRMCNumber < (u32)MAX_DEV_DDRMC) {
		/** Map the DDR MC number to its device ID and fetch the base address */
		Status = XPm_GetDeviceBaseAddr(DdrMcDeviceId[DDRMCNumber], &BaseAddr);
		if (Status != XST_SUCCESS) {
			BaseAddr = (u32)XLOADER_INVALID_ADDR;
		}
	}

	return BaseAddr;
}

/*************************************************************************************************/
/**
 * @brief	This function maps a DDR MC physical address to its DDR MC number
 *		by comparing it with the base address of each XLOADER_DDRMC_n.
 *
 * @param	PhysAddr is the DDR MC physical address from the command
 * @param	DDRMCNumber is the pointer in which the matched DDR MC number is stored.
 *
 * @return
 * 		- XST_SUCCESS if the physical address matches a DDR MC base address.
 * 		- XST_INVALID_PARAM if the output pointer or physical address is not valid.
 * 		- XST_FAILURE if the physical address does not match any DDR MC.
 *
 **************************************************************************************************/
static int XLoader_GetDdrMcNumber(u32 PhysAddr, u32 *DDRMCNumber)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 BaseAddr;

	if (DDRMCNumber == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** No match leaves the DDR MC number outside the valid range. */
	*DDRMCNumber = (u32)MAX_DEV_DDRMC;

	if (PhysAddr == (u32)XLOADER_INVALID_ADDR) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	for (Index = 0U; Index < (u32)MAX_DEV_DDRMC; Index++) {
		BaseAddr = XLoader_GetDdrMcBaseAddr(Index);
		if (BaseAddr == PhysAddr) {
			*DDRMCNumber = Index;
			Status = XST_SUCCESS;
			break;
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function fetches the calibration data of a DDR MC from the storage device
 *		into the given buffer, using the copy interface of that storage device.
 *
 * @param	StorageIndex is the PDI source index of the calibration data storage device
 * @param	CalibDataSrc is the source address in the storage device in case of QSPI/OSPI,
 *		or the source filename in case of SD/UFS
 * @param	DestBuffer is the buffer to which the calibration data is fetched
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XST_INVALID_PARAM if the source, destination or storage device is not valid.
 * 		- Error code returned by the storage device copy in case of failure.
 *
 **************************************************************************************************/
static int XLoader_FetchCalibDataFromNvStorage(u8 StorageIndex, void *CalibDataSrc,
		u32 *DestBuffer)
{
	int Status = XST_FAILURE;

	/** Verify the source and destination buffer */
	if ((CalibDataSrc == (void *)(UINTPTR)XLOADER_INVALID_ADDR) || (DestBuffer == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Fetch the calibration data using the copy interface of the storage device */
	switch (StorageIndex) {
#ifdef XLOADER_QSPI
		case XLOADER_QSPI_INDEX:
			Status = XLoader_QspiReadDdrCalibData((u32)(UINTPTR)CalibDataSrc,
					(u32)(UINTPTR)DestBuffer);
			break;
#endif
#ifdef XLOADER_OSPI
		case XLOADER_OSPI_INDEX:
			Status = XLoader_OspiReadDdrCalibData((u32)(UINTPTR)CalibDataSrc,
					(u32)(UINTPTR)DestBuffer);
			break;
#endif
#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
		case XLOADER_SD_INDEX:
			Status = XLoader_SdReadDdrCalibData((const char *)CalibDataSrc,
					(u32)(UINTPTR)DestBuffer);
			break;
#endif
#ifdef XLOADER_UFS
		case XLOADER_UFS_INDEX:
			Status = XLoader_UfsReadDdrCalibData((const char *)CalibDataSrc,
					(u32)(UINTPTR)DestBuffer);
			break;
#endif
		default:
			Status = XLOADER_ERR_INVALID_CALIBDATA_STORAGE_MODE;
			break;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function releases the calibration storage used by the copies.
 *		QSPI and OSPI stay initialized as the primary boot device. SD and UFS
 *		unmount a volume only when these copies mounted it.
 *
 * @param	StorageIndex is the index of the storage device holding the calibration data
 *
 * @return
 * 		- XST_SUCCESS on success, and when there is nothing to release.
 * 		- Error code returned by the storage device release in case of failure.
 *
 **************************************************************************************************/
static int XLoader_ReleaseCalibDataStorage(u8 StorageIndex)
{
	int Status = XST_FAILURE;

	switch (StorageIndex) {
#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
		case XLOADER_SD_INDEX:
			Status = XLoader_SdCalibDataRelease();
			break;
#endif
#ifdef XLOADER_UFS
		case XLOADER_UFS_INDEX:
			Status = XLoader_UfsCalibDataRelease();
			break;
#endif
		default:
			/* Nothing was requested, so there is nothing to release */
			Status = XST_SUCCESS;
			break;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks the integrity of a calibration data record that is fetched
 *		from the storage device. The first word of the record holds the IDCODE of the
 *		device on which the record was saved and the last word holds the checksum of all
 *		the words in between.
 *
 * @param	CalibData is the buffer holding the calibration data record
 *
 * @return
 * 		- XST_SUCCESS if both the IDCODE and the checksum of the record are valid.
 * 		- XST_INVALID_PARAM if the record buffer is NULL.
 * 		- XST_FAILURE if the IDCODE or the checksum of the record does not match.
 *
 **************************************************************************************************/
static int XLoader_CheckCalibDataIntegrity(const u32 *CalibData)
{
	int Status = XST_FAILURE;
	u32 IdCode;

	/** Verify the record buffer */
	if (CalibData == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Verify the IDCODE of the record against the IDCODE of the device */
	IdCode = XPlmi_In32(PMC_TAP_IDCODE);
	if (CalibData[XLOADER_CALIBDATA_IDCODE_INDEX] != IdCode) {
		XPlmi_Printf(DEBUG_INFO, "Calibration data IDCODE 0x%08x does not match device "
			"IDCODE 0x%08x\n", CalibData[XLOADER_CALIBDATA_IDCODE_INDEX], IdCode);
		goto END;
	}

	/**
	 * Verify the checksum held in the last word of the record against the words that
	 * follow the IDCODE
	 */
	Status = XilPdi_ValidateChecksum(&CalibData[XLOADER_CALIBDATA_CHECKSUM_START_INDEX],
			((XLOADER_CALIBDATA_SIZE - XLOADER_CALIBDATA_CHECKSUM_START_INDEX) * sizeof(u32)));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function writes the calibration data of a DDR MC to the XRAM address.
 *		Only the calibration data words of the record are written, the IDCODE word and
 *		the checksum word are left out.
 *
 * @param	XRamAddr is the XRAM address where the calibration data is written
 * @param	CalibData is the buffer holding the calibration data record
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XST_INVALID_PARAM if the record buffer is NULL.
 * 		- Error code returned by the DMA transfer in case of failure.
 *
 **************************************************************************************************/
static int XLoader_WriteCalibDataToXRam(u32 XRamAddr, const u32 *CalibData)
{
	int Status = XST_FAILURE;

	/** Verify the record buffer */
	if (CalibData == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Write the calibration data words that follow the IDCODE, excluding the checksum word */
	Status = XPlmi_DmaXfr((u64)(UINTPTR)&CalibData[XLOADER_CALIBDATA_CHECKSUM_START_INDEX],
			(u64)XRamAddr, (XLOADER_CALIBDATA_SIZE - XLOADER_CALIBDATA_METADATA_SIZE), XPLMI_PMCDMA_0);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sets or clears the calibration skip flag of a DDR MC.
 *		When set, the DDR MC warm boots with restored calibration data instead
 *		of calibrating again. When cleared, the DDR MC performs a cold boot.
 *
 * @param	DDRMCNumber is the DDR MC number
 * @param	Enable is 1 to set the calibration skip flag, 0 to clear it
 *
 * @return
 * 		- XST_SUCCESS if the calibration skip flag is updated as requested.
 * 		- XST_INVALID_PARAM if the DDR MC number is not valid.
 * 		- XST_FAILURE if the calibration skip flag could not be updated.
 *
 **************************************************************************************************/
static int XLoader_SetDdrMcCalibSkipFlag(u32 DDRMCNumber, u8 Enable)
{
	int Status = XST_FAILURE;
	u32 BaseAddr;
	u32 RegAddr;
	u32 Value;

	/** Fetch the base address of the DDR MC */
	BaseAddr = XLoader_GetDdrMcBaseAddr(DDRMCNumber);
	if (BaseAddr == (u32)XLOADER_INVALID_ADDR) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	RegAddr = BaseAddr + XLOADER_DDRMC_CALIB_SKIP_OFFSET;
	Value = ((Enable != (u8)0U) ? XLOADER_DDRMC_CALIB_SKIP_MASK : 0U);

	/** Set or clear the calibration skip flag of the DDR MC */
	XPlmi_UtilRMW(RegAddr, XLOADER_DDRMC_CALIB_SKIP_MASK, Value);

	/** Verify the calibration skip flag matches the requested value */
	if ((XPlmi_In32(RegAddr) & XLOADER_DDRMC_CALIB_SKIP_MASK) == Value) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the calibration skip flag of a DDR MC.
 *
 * @param	BaseAddr is the base address of the DDR MC
 *
 * @return
 * 		- TRUE if the calibration skip flag is set.
 * 		- FALSE otherwise.
 *
 **************************************************************************************************/
u32 XLoader_IsWarmBootFlagSet(u32 BaseAddr)
{
	u32 RegVal = XPlmi_In32(BaseAddr + XLOADER_DDRMC_CALIB_SKIP_OFFSET);

	return (((RegVal & XLOADER_DDRMC_CALIB_SKIP_MASK) != 0U) ? (u32)TRUE : (u32)FALSE);
}

/*************************************************************************************************/
/**
 * @brief	This function returns the storage index for calibration data from
 *		the PDI source selected by XLoader_PdiInit. That index distinguishes
 *		SD/eMMC filesystem boot from raw boot. The boot-mode map does not.
 *
 * @param	StorageIndex is the pointer to store the calibration data storage index
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XST_FAILURE when the PDI source is not a supported calibration data
 *		  storage device, including JTAG, SMAP/SBI, PCIe and SD/eMMC raw
 *		  boot. The caller skips the restore in that case.
 *
 **************************************************************************************************/
static int XLoader_GetCalibDataStorageIndex(u8 *StorageIndex)
{
	int Status = XST_FAILURE;
	u8 Index = XLoader_GetPdiInstance()->PdiIndex;

	/**
	 * Use the source opened for this PDI. SD/eMMC raw boot shares the
	 * filesystem boot-mode value, and XLoader_PdiInit selects
	 * XLOADER_SD_RAW_INDEX only after reading the multiboot register.
	 */
	*StorageIndex = Index;

	/** Proceed only if QSPI/OSPI/SD filesystem/UFS holds the calibration data */
	if ((Index == XLOADER_QSPI_INDEX) || (Index == XLOADER_OSPI_INDEX) ||
		(Index == XLOADER_SD_INDEX)
#ifdef VERSAL_2VE_2VM
		|| (Index == XLOADER_UFS_INDEX)
#endif
	) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks DDR MC calibration status from the PCSR Status
 *		register. Calibration is considered successful when CALDONE is set and
 *		CALERROR and INCAL are cleared.
 *
 * @param	BaseAddr is the DDR MC base address
 *
 * @return
 * 		- XST_SUCCESS if calibration completed successfully.
 * 		- XST_FAILURE if calibration is incomplete or failed.
 *
 **************************************************************************************************/
int XLoader_CheckDdrMcCalibStatus(u32 BaseAddr)
{
	int Status = XST_FAILURE;
	u32 PcsrStatus;

	PcsrStatus = XPlmi_In32(BaseAddr + DDRMC_PCSR_STATUS_OFFSET);

	if (((PcsrStatus & DDRMC_PCSR_STATUS_CALDONE_MASK) == DDRMC_PCSR_STATUS_CALDONE_MASK) &&
	    ((PcsrStatus & DDRMC_PCSR_STATUS_CALERROR_MASK) == 0U) &&
	    ((PcsrStatus & DDRMC_PCSR_STATUS_INCAL_MASK) == 0U)) {
			Status = XST_SUCCESS;
	}

	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function restores the calibration data of the warm-boot DDR
 *		memory controllers carried in the command. The storage device is
 *		the primary boot device. JTAG, SMAP/SBI, PCIe and SD/eMMC raw boot
 *		skip the restore and return success. A fetch or integrity failure
 *		for one DDR MC clears its calibration skip flag so that MC
 *		cold-calibrates, and the command continues.
 *
 * @param	Cmd is pointer to the command structure. Cmd->Len is the full
 *		payload, one pair per warm-boot DDR MC, and may be empty when no
 *		DDR MC requested warm boot:
 *		- Physical address of the DDR MC.
 *		- Storage offset. For QSPI/OSPI this is the flash address of the
 *		  record. A CDO chunk can deliver fewer words than Cmd->Len. That
 *		  chunk is accepted, and the restore runs only on the chunk that
 *		  finishes the command.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error Code in case of failure
 *
 *****************************************************************************/
int XLoader_CalibDataRestore(XPlmi_Cmd *Cmd)
{
#ifdef PLM_ENABLE_DDR_CALIB_RESTORE
	int Status = XST_FAILURE;
	u8 StorageIndex = XLOADER_INVALID_INDEX;
	void *CalibDataSrc = (void *)(UINTPTR)XLOADER_INVALID_ADDR;
	u32 XRamAddr = (u32)XLOADER_INVALID_ADDR;
	u32 DDRMCNumber = (u32)MAX_DEV_DDRMC;
	u32 PayloadIndex;
	u32 ChunkIndex;
	u32 PhysAddr;
	u32 Offset;
	const u32 *CmdPayload = NULL;
	int RelStatus = XST_FAILURE;
	int ClrStatus = XST_FAILURE;
	const XilPdi *PdiPtr = XLoader_GetPdiInstance();
	/**
	 * Payload words kept across CDO chunks. The chunk that finishes the
	 * command is the one that runs the restore.
	 */
	static u32 CalibCmdPayload[MAX_DEV_DDRMC * XLOADER_CALIBDATA_CMD_WORDS_PER_MC];

	/**
	 * Cmd->Len is the full payload, in address/offset pairs. PayloadLen is
	 * only the words in this CDO chunk. With 16 words left in the chunk the
	 * loader executes the command instead of moving it, so a 16-word payload
	 * arrives here as 15 words and the rest follows on resume.
	 * A zero length means no DDR MC requested warm boot.
	 */
	if (Cmd == NULL) {
		Status = XLOADER_ERR_INVALID_PARAM;
		goto END;
	}
	/**
	 * Restore only while the primary boot PDI is loading. The boot device is
	 * still open then. A secondary or later partial PDI has no supported way
	 * back to that device.
	 */
	if (PdiPtr->PdiType != XLOADER_PDI_TYPE_FULL) {
		if (Cmd->ProcessedLen == 0U) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"DDRMC calibration data restore is skipped\r\n");
		}
		Status = XST_SUCCESS;
		goto END_CHUNK;
	}
	if ((Cmd->PayloadLen == 0U) && (Cmd->ProcessedLen == 0U)) {
		Status = XST_SUCCESS;
		goto END;
	}
	/**
	 * JTAG, SMAP/SBI, PCIe and SD/eMMC raw have no calibration data to
	 * restore. Skip the command so those boots continue.
	 */
	Status = XLoader_GetCalibDataStorageIndex(&StorageIndex);
	if (Status != XST_SUCCESS) {
		if (Cmd->ProcessedLen == 0U) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"DDRMC calibration data restore is skipped: boot device "
				"cannot store calibration data\n");
		}
		Status = XST_SUCCESS;
		goto END_CHUNK;
	}
	if (((Cmd->Len % XLOADER_CALIBDATA_CMD_WORDS_PER_MC) != 0U) ||
		(Cmd->Len > ((u32)MAX_DEV_DDRMC * XLOADER_CALIBDATA_CMD_WORDS_PER_MC)) ||
		(Cmd->Len == 0U) || (Cmd->Payload == NULL) ||
		(Cmd->ProcessedLen > Cmd->Len) ||
		(Cmd->PayloadLen > (Cmd->Len - Cmd->ProcessedLen))) {
		Status = XLOADER_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 * This chunk does not finish the command. Keep its words and accept it.
	 * The restore runs only from the chunk that delivers the last word.
	 */
	if ((Cmd->ProcessedLen + Cmd->PayloadLen) < Cmd->Len) {
		for (ChunkIndex = 0U; ChunkIndex < Cmd->PayloadLen; ChunkIndex++) {
			CalibCmdPayload[Cmd->ProcessedLen + ChunkIndex] =
					Cmd->Payload[ChunkIndex];
		}
		Status = XST_SUCCESS;
		goto END_CHUNK;
	}

	if (Cmd->ProcessedLen == 0U) {
		CmdPayload = Cmd->Payload;
	}
	else {
		for (ChunkIndex = 0U; ChunkIndex < Cmd->PayloadLen; ChunkIndex++) {
			CalibCmdPayload[Cmd->ProcessedLen + ChunkIndex] =
					Cmd->Payload[ChunkIndex];
		}
		CmdPayload = CalibCmdPayload;
	}

	for (PayloadIndex = 0U; PayloadIndex < Cmd->Len;
			PayloadIndex += XLOADER_CALIBDATA_CMD_WORDS_PER_MC) {
		PhysAddr = CmdPayload[PayloadIndex + XLOADER_CALIBDATA_PHYS_ADDR_INDEX];
		Offset = CmdPayload[PayloadIndex + XLOADER_CALIBDATA_OFFSET_INDEX];

		/**
		 * Map the physical address to a DDR MC. The SD/UFS file name is
		 * selected from that DDR MC number.
		 */
		Status = XLoader_GetDdrMcNumber(PhysAddr, &DDRMCNumber);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "Calibration data physical address 0x%08x does not "
				"match a DDR MC base address\n", PhysAddr);
			Status = XLOADER_ERR_INVALID_PHYS_ADDR;
			goto END;
		}

		/** Fetch Non-volatile storage address or filename */
		CalibDataSrc = XLoader_GetCalibDataSrc(StorageIndex, DDRMCNumber, Offset);
		if (CalibDataSrc == (void *)(UINTPTR)XLOADER_INVALID_ADDR) {
			Status = XLOADER_ERR_INVALID_NVSTORAGE_ADDR;
			goto END;
		}

		/** Reject a destination that would wrap past the end of the address space */
		if (PhysAddr > ((u32)XLOADER_INVALID_ADDR - XLOADER_DDRMC_CALIB_DATA_OFFSET)) {
			Status = XLOADER_ERR_INVALID_XRAM_ADDR;
			goto END;
		}
		XRamAddr = PhysAddr + XLOADER_DDRMC_CALIB_DATA_OFFSET;

		/** Fetch calibration data from Non-volatile storage to Temporary buffer */
		Status = XLoader_FetchCalibDataFromNvStorage(StorageIndex,
				CalibDataSrc, CalibDataTempBuffer);
		if (Status != XST_SUCCESS) {
			/**
			 * A missing record is normal on the first boot. Fall back to
			 * cold calibration instead of failing the PDI load.
			 */
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Calibration data fetch failed for the DDR MC %u "
				"with error code 0x%08x\n", DDRMCNumber, (u32)Status);
		}
		else {
			/** Check integrity of the calibration data */
			Status = XLoader_CheckCalibDataIntegrity(CalibDataTempBuffer);
			if (Status != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Integrity check failed for calibration data "
					"record for the DDR MC %d in storage device\n", DDRMCNumber);
			}
		}
		if (Status != XST_SUCCESS) {
			/** Clear the calibration skip flag for the DDR MC to get cold boot */
			Status = XLoader_SetDdrMcCalibSkipFlag(DDRMCNumber, (u8)0U);
			if (Status != XST_SUCCESS) {
				Status = XLOADER_ERR_SET_CALIB_SKIP_FLAG;
				goto END;
			}
			/** Clear the temporary buffer after use */
			Status = Xil_SecureZeroize((u8 *)CalibDataTempBuffer,
					XLOADER_CALIBDATA_SIZE_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			continue;
		}

		/** Write calibration data to XRAM */
		Status = XLoader_WriteCalibDataToXRam(XRamAddr, CalibDataTempBuffer);
		if (Status != XST_SUCCESS) {
			Status = XLOADER_ERR_CALIBDATA_WRITE;
			goto END;
		}

		/** Set DDR MC calibration skip flag to warm boot the DDR MC */
		Status = XLoader_SetDdrMcCalibSkipFlag(DDRMCNumber, (u8)1U);
		if (Status != XST_SUCCESS) {
			Status = XLOADER_ERR_SET_CALIB_SKIP_FLAG;
			goto END;
		}

		/** Clear the temporary buffer after use */
		Status = Xil_SecureZeroize((u8 *)CalibDataTempBuffer,
				XLOADER_CALIBDATA_SIZE_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	/** Clear leftover calibration data on error paths that skip the per-MC clear. */
	ClrStatus = Xil_SecureZeroize((u8 *)CalibDataTempBuffer, XLOADER_CALIBDATA_SIZE_BYTES);
	/**
	 * The copies keep the storage device up across the DDR MCs, so it is
	 * released here, once, on both paths.
	 */
	RelStatus = XLoader_ReleaseCalibDataStorage(StorageIndex);
	if (Status == XST_SUCCESS) {
		if (ClrStatus != XST_SUCCESS) {
			Status = ClrStatus;
		}
		else {
			Status = RelStatus;
		}
	}

END_CHUNK:
	return Status;
#else
	(void)Cmd;
	return XST_SUCCESS;
#endif
}

/** @} end of xloader_server_apis group */
