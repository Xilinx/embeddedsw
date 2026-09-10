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
#ifdef XPAR_CALIBDATA_STORAGEDEVICE
#define XLOADER_CALIBDATA_IDCODE_INDEX      (0U)
        /**< Index of the IDCODE word in a calibration data record */
#define XLOADER_CALIBDATA_CHECKSUM_START_INDEX  (1U)
        /**< Index of the first word of a calibration data record covered by the checksum */
#define XLOADER_CALIBDATA_METADATA_SIZE     (2U)
        /**< Number of words of a calibration data record taken by the IDCODE word and the
             checksum word */

/************************** Function Prototypes ******************************/
static void *XLoader_GetCalibDataNvStorageAddr(u32 DDRMCNumber, u8 StorageIndex);
static int XLoader_GetCalibDataStorageIndex(u8 *StorageIndex);
static u32 XLoader_GetWarmBootEnabled(u32 DDRMCNumber);
static int XLoader_FetchCalibDataFromNvStorage(u8 StorageIndex, void *CalibDataSrc,
		u32 *DestBuffer);
static int XLoader_ReleaseCalibDataStorage(u8 StorageIndex);
static int XLoader_CheckCalibDataIntegrity(const u32 *CalibData);
static int XLoader_WriteCalibDataToXRam(u32 XRamAddr, const u32 *CalibData);
static int XLoader_SetDdrMcCalibSkipFlag(u32 DDRMCNumber, u8 Enable);
#endif

/************************** Variable Definitions *****************************/
static u8 DdrRequested = (u8)FALSE;
#ifdef XPAR_CALIBDATA_STORAGEDEVICE
static u32 CalibDataTempBuffer[XLOADER_CALIBDATA_SIZE];
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

#ifdef XPAR_CALIBDATA_STORAGEDEVICE
/*************************************************************************************************/
/**
 * @brief	This function returns the source address or filename of the calibration data
 *		based on the DDR MC number and storage index.
 *
 * @param	DDRMCNumber is the DDR MC number
 * @param	StorageIndex is the storage index
 *
 * @return
 * 		- Source address or filename pointer.
 *
 **************************************************************************************************/
static void *XLoader_GetCalibDataNvStorageAddr(u32 DDRMCNumber, u8 StorageIndex)
{
	static void *const CalibDataSrcAddr[MAX_DEV_DDRMC] = {
		XLOADER_DDRMC_LIST(XLOADER_CALIBDATA_SRC_ADDR)
	};
	static void *const CalibDataSrcFilename[MAX_DEV_DDRMC] = {
		XLOADER_DDRMC_LIST(XLOADER_CALIBDATA_SRC_FILENAME)
	};
	void *CalibDataSrc = (void *)(UINTPTR)XLOADER_INVALID_ADDR;

	if (DDRMCNumber < (u32)MAX_DEV_DDRMC) {
		/** If QSPI/OSPI is used for calibration data storage */
		if ((StorageIndex == XLOADER_QSPI_INDEX) || (StorageIndex == XLOADER_OSPI_INDEX)) {
			CalibDataSrc = CalibDataSrcAddr[DDRMCNumber];
		}
		/** If SD/UFS is used for calibration data storage */
		else {
			CalibDataSrc = CalibDataSrcFilename[DDRMCNumber];
		}
	}

	return CalibDataSrc;
}

/*************************************************************************************************/
/**
 * @brief	This function returns whether warm boot is enabled for the given DDR MC.
 *
 * @param	DDRMCNumber is the DDR MC number
 *
 * @return
 * 		- TRUE if warm boot is enabled for the DDR MC.
 * 		- FALSE otherwise.
 *
 **************************************************************************************************/
static u32 XLoader_GetWarmBootEnabled(u32 DDRMCNumber)
{
	static u32 const WarmBootEnabled[MAX_DEV_DDRMC] = {
		XLOADER_DDRMC_LIST(XLOADER_WARM_BOOT_ENABLED)
	};
	u32 IsEnabled = (u32)FALSE;

	if (DDRMCNumber < (u32)MAX_DEV_DDRMC) {
		IsEnabled = WarmBootEnabled[DDRMCNumber];
	}

	return IsEnabled;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the base address of the DDRMC
 *		based on the DDR MC number.
 *
 * @param	DDRMCNumber is the DDR MC number
 *
 * @return
 * 		- Base address pointer.
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
			Status = XLoader_QspiReadDdrCalibData(CalibDataSrc,
					(u32)(UINTPTR)DestBuffer);
			break;
#endif
#ifdef XLOADER_OSPI
		case XLOADER_OSPI_INDEX:
			Status = XLoader_OspiReadDdrCalibData(CalibDataSrc,
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
 * @brief	This function releases the storage device that the calibration data was fetched
 *		from. The copies keep the device requested and initialized so that all the DDR
 *		MCs are served without repeating that work, so this is what undoes it, once the
 *		restore is over.
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
#ifdef XLOADER_QSPI
		case XLOADER_QSPI_INDEX:
			Status = XLoader_QspiCalibDataRelease();
			break;
#endif
#ifdef XLOADER_OSPI
		case XLOADER_OSPI_INDEX:
			Status = XLoader_OspiCalibDataRelease();
			break;
#endif
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
 * @brief	This function returns the storage index for calibration data based on
 *		the primary boot mode and calibration data storage mode configuration.
 *
 * @param	StorageIndex is the pointer to store the calibration data storage index
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_ERR_INVALID_CALIBDATA_STORAGE_MODE if boot mode or storage index is invalid.
 * 		- XST_FAILURE on failure.
 *
 **************************************************************************************************/
static int XLoader_GetCalibDataStorageIndex(u8 *StorageIndex)
{
	int Status = XST_FAILURE;
	PdiSrc_t PrimaryBootMode = XLoader_GetBootMode();
	u8 DeviceFlags;
	const PdiSrcMap PdiSourceMap[] = XLOADER_GET_PDISRC_INFO();

	/** Check compatibility of primary boot mode and calibration data storage mode */
	if (PrimaryBootMode != XPAR_CALIBDATA_STORAGEDEVICE) {
		goto END;
	}

	/** Fetch device flags */
	DeviceFlags = (u8)(XPAR_CALIBDATA_STORAGEDEVICE & XLOADER_PDISRC_FLAGS_MASK);
	if (DeviceFlags >= XPLMI_ARRAY_SIZE(PdiSourceMap)) {
		goto END;
	}

	/** Fetch storage index */
	*StorageIndex = PdiSourceMap[DeviceFlags].Index;

	/** Proceed only if QSPI/OSPI/SD/UFS is used for calibration data storage */
	if ((*StorageIndex == XLOADER_QSPI_INDEX) || (*StorageIndex == XLOADER_OSPI_INDEX) ||
		(*StorageIndex == XLOADER_SD_INDEX)
#ifdef VERSAL_2VE_2VM
		|| (*StorageIndex == XLOADER_UFS_INDEX)
#endif
	) {
		Status = XST_SUCCESS;
	}
END:
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
#endif /* XPAR_CALIBDATA_STORAGEDEVICE */

/*****************************************************************************/
/**
 * @brief	This function restores the calibration data of all the DDR memory
 *		controllers of the device which are requested for warm boot in the
 *		XPLMI_RTCFG_DDRMC_WARM_BOOT_MASK_ADDR RTCA register and for which warm
 *		boot is enabled in the design.
 *
 *  This command takes no payload parameters.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error Code in case of failure
 *
 *****************************************************************************/
int XLoader_CalibDataRestore(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
#ifdef XPAR_CALIBDATA_STORAGEDEVICE
	u8 StorageIndex = XLOADER_INVALID_INDEX;
	void *CalibDataSrc = (void *)(UINTPTR)XLOADER_INVALID_ADDR;
	u32 XRamAddr = (u32)XLOADER_INVALID_ADDR;
	u32 DDRMCNumber;
	u32 DDRMCWarmBootMask;
	int RelStatus = XST_FAILURE;
	int ClrStatus = XST_FAILURE;

	(void)Cmd;

	/** Fetch storage index for calibration data */
	Status = XLoader_GetCalibDataStorageIndex(&StorageIndex);
	if (Status != XST_SUCCESS) {
		Status = XLOADER_ERR_INVALID_CALIBDATA_STORAGE_MODE;
		goto END;
	}

	/** Fetch the bit mask of the DDR MCs to be warm booted from RTCA */
	DDRMCWarmBootMask = XPlmi_In32(XPLMI_RTCFG_DDRMC_WARM_BOOT_MASK_ADDR);

	for (DDRMCNumber = 0U; DDRMCNumber < (u32)MAX_DEV_DDRMC; DDRMCNumber++) {
		/**
		 * Skip the DDR MC if it is not requested for warm boot in RTCA or if
		 * warm boot is not enabled for it in the design
		 */
		if (((DDRMCWarmBootMask & ((u32)1U << DDRMCNumber)) == 0U) ||
			(XLoader_GetWarmBootEnabled(DDRMCNumber) != (u32)TRUE)) {
			continue;
		}

		/** Fetch Non-volatile storage address or filename */
		CalibDataSrc = XLoader_GetCalibDataNvStorageAddr(DDRMCNumber, StorageIndex);
		if (CalibDataSrc == (void *)(UINTPTR)XLOADER_INVALID_ADDR) {
			Status = XLOADER_ERR_INVALID_NVSTORAGE_ADDR;
			goto END;
		}

		/** Fetch XRAM address */
		XRamAddr = XLoader_GetDdrMcBaseAddr(DDRMCNumber);
		if (XRamAddr == (u32)XLOADER_INVALID_ADDR) {
			Status = XLOADER_ERR_INVALID_XRAM_ADDR;
			goto END;
		}
		XRamAddr += XLOADER_DDRMC_CALIB_DATA_OFFSET;

		/** Fetch calibration data from Non-volatile storage to Temporary buffer */
		Status = XLoader_FetchCalibDataFromNvStorage(StorageIndex,
				CalibDataSrc, CalibDataTempBuffer);
		if (Status != XST_SUCCESS) {
			Status = XLOADER_ERR_CALIBDATA_FETCH;
			goto END;
		}

		/** Check integrity of the calibration data */
		Status = XLoader_CheckCalibDataIntegrity(CalibDataTempBuffer);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "Integrity check failed for calibration data "
				"record for the DDR MC %d in storage device\n", DDRMCNumber);
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
#else
	(void)Cmd;
	Status = XST_SUCCESS;
#endif
	return Status;
}

/** @} end of xloader_server_apis group */
