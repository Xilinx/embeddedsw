/******************************************************************************
* Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xloader_sd.c
*
* This is the file which contains sd related code for XilLoader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   09/21/2017 Initial release
* 1.01  bsv  06/26/2019 Added secondary boot support
*       bsv  02/12/2020 Added support for SD/eMMC raw boot mode
*       bsv  02/23/2020 Added multi partition support for SD/eMMC FS boot modes
*       bsv  03/14/2020 Added eMMC0 FS and raw boot mode support
*       bsv  03/17/2020 Changes relatd to multiple partitions in SD/eMMC boot
*       bsv  02/04/2020 Reset file system instance in init functions for LPD off
*						suspend and resume to work
* 1.02  bsv  04/09/2020 Code clean up
*       bsv  04/28/2020 Changed SD drive number to 5 when both SD0 and SD1 are
*						in design
* 1.03  bsv  07/01/2020 Unmount file system after loading PDIs
*       skd  07/14/2020 Added 64bit support for SD copy destination address
*       bsv  07/16/2020 Force Cdn bit to 1 to improve performance
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bsv  09/04/2020 Added error checks for XPlmi_Strcat function calls
*       bsv  10/13/2020 Code clean up
*       td	 10/19/2020	MISRA C Fixes
* 1.04  bsv  08/31/2021 Code clean up
* 1.05  bsv  10/01/2021 Addressed code review comments
*       bsv  10/26/2021 Code clean up
* 1.06  kpt  12/13/2021 Replaced Xil_Strcat with Xil_SStrcat
* 1.07  skg  06/20/2022 Fixed MISRA C Rule 10.3 violation
*       skg  06/20/2022 Fixed MISRA C Rule 7.4 violation
*       bm   07/06/2022 Refactor versal and versal_net code
* 1.08  ng   11/11/2022 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       ng   08/16/2023 Fixed status overwrite in SdRelease
*       dd	 09/11/2023 MISRA-C violation Rule 10.3 fixed
*       dd   09/11/2023 MISRA-C violation Rule 17.8 fixed
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader_sd.h"
#include "xloader.h"
#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
#include "xparameters.h"
#include "ff.h"
#include "xplmi_generic.h"
#include "xil_util.h"
#include "xpm_api.h"
#include "xpm_nodeid.h"
#include "xplmi.h"
#include "xloader_plat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_SD_SRC_FILENAME_SIZE1		(9U)
#define XLOADER_SD_SRC_FILENAME_SIZE2		(13U)

/************************** Function Prototypes ******************************/
static int XLoader_MakeSdFileName(u32 MultiBootOffsetVal);
static u8 XLoader_GetDrvNumSD(u8 DeviceFlags);

/************************** Variable Definitions *****************************/
static FIL FFil;		/* File object */
static XSdPs SdInstance;
static char BootFile[XLOADER_BASE_FILE_NAME_LEN_SD + 1U] = {'\0'};
static u32 SdCdnVal = 0U;
static u32 SdCdnReg = 0U;
static u32 SdDeviceNode;

/*****************************************************************************/
/**
 * @brief	This function creates the Boot image name for file system devices
 * 			based on the multiboot register.
 *
 * @param	MultiBootOffsetVal is the value of the multiboot register that
 *			would be suffixed to the filename
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_SD_MAX_BOOT_FILES_LIMIT if search for BOOT.BIN
 * 			crosses max limit.
 *
 ******************************************************************************/
static int XLoader_MakeSdFileName(u32 MultiBootOffsetVal)
{
	int Status = XST_FAILURE;
	u8 Index = 10U;
	u8 Value;
	u32 MultiBootOffset = MultiBootOffsetVal;
    /**
     * - Verify that multiboot offset is within the files limits.
     */
	if (MultiBootOffset >= XLOADER_SD_MAX_BOOT_FILES_LIMIT) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_MAX_BOOT_FILES_LIMIT, 0);
		goto END;
	}

	if (0x0U == MultiBootOffset) {
		Status = Xil_SStrCat((u8*)BootFile, XLOADER_BASE_FILE_NAME_LEN_SD,
			(const u8*)"BOOT.BIN", XLOADER_SD_SRC_FILENAME_SIZE1);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = Xil_SStrCat((u8*)BootFile, XLOADER_BASE_FILE_NAME_LEN_SD,
			(const u8*)"BOOT0000.BIN", XLOADER_SD_SRC_FILENAME_SIZE2);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		while (MultiBootOffset > 0U) {
			Value = (u8)(MultiBootOffset % 10U);
			MultiBootOffset /= 10U;
			BootFile[Index] += (char)Value;
			Index--;
		}
	}

	XLoader_Printf(DEBUG_INFO, "File name is %s\r\n", BootFile);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to obtain drive number based on design
 * and boot mode.
 *
 * @param       DeviceFlags contain boot mode information
 *
 * @return      Drive number
 *
 *****************************************************************************/
static u8 XLoader_GetDrvNumSD(u8 DeviceFlags)
{
	u8 DrvNum;
	/**
	 * - If design has both SD0 and SD1, select drive number based on bootmode
	 * - If design has only SD0 or ONLY SD1, drive number should be "0".
	 */
#ifdef XPAR_XSDPS_1_DEVICE_ID
	if ((XLoader_IsPdiSrcSD0(DeviceFlags) == (u8)TRUE) ||
		(DeviceFlags == XLOADER_PDI_SRC_EMMC0)) {
		DrvNum = XLOADER_SD_DRV_NUM_0;
	}
	else {
		/**
		 * - For XLOADER_SD1_BOOT_MODE or XLOADER_SD1_LS_BOOT_MODE
		 * or XLOADER_EMMC_BOOT_MODE, drive should be "5".
		 */
		DrvNum = XLOADER_SD_DRV_NUM_5;
	}
#else
	(void)DeviceFlags;
	DrvNum = XLOADER_SD_DRV_NUM_0;
#endif

	return DrvNum;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the sd controller and driver.
 *
 * @param	DeviceFlagsVal have the bootmode information.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_MEMSET_SD_BOOT_FILE if SD bootfile instance creation
 * 			fails.
 * 			- XLOADER_ERR_PM_DEV_SDIO_0 if device request for SDIO_0 fails.
 * 			- XLOADER_ERR_PM_DEV_SDIO_1 if device request for SDIO_1 fails.
 * 			- XLOADER_ERR_SD_F_OPEN if file is not present or read fails.
 *
 *****************************************************************************/
int XLoader_SdInit(u32 DeviceFlagsVal)
{
	int Status = XST_FAILURE;
	FRESULT Rc;
	u32 MultiBootOffset;
	u32 DeviceFlags = DeviceFlagsVal;
	u8 PdiSrc = (u8)(DeviceFlags & XLOADER_PDISRC_FLAGS_MASK);
	u8 DrvNum = XLoader_GetDrvNumSD(PdiSrc);
	static FATFS FatFileSystem;
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;
	u32 ErrorCode;

	Status = XPlmi_MemSetBytes(BootFile, sizeof(BootFile), 0U, sizeof(BootFile));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET,
			(int)XLOADER_ERR_MEMSET_SD_BOOT_FILE);
		goto END;
	}

	if ((XLoader_IsPdiSrcSD0(PdiSrc) == (u8)TRUE) ||
		(PdiSrc == XLOADER_PDI_SRC_EMMC0)) {
		SdDeviceNode = PM_DEV_SDIO_0;
		SdCdnReg = PMC_IOU_SLCR_SD0_CDN_CTRL;
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_SDIO_0;
	}
	else {
		SdDeviceNode = PM_DEV_SDIO_1;
		SdCdnReg = PMC_IOU_SLCR_SD1_CDN_CTRL;
		ErrorCode =  (u32)XLOADER_ERR_PM_DEV_SDIO_1;
	}

	Status = XPm_RequestDevice(PM_SUBSYS_PMC, SdDeviceNode,
		CapSecureAccess, XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, Status);
		goto END;
	}
	SdCdnVal = XPlmi_In32(SdCdnReg);
	XPlmi_Out32(SdCdnReg, PMC_IOU_SLCR_SD0_CDN_CTRL_SD0_CDN_CTRL_MASK);

	/**
	 * - Filesystem boot modes require the filename extension as well as
	 * the logical drive in which the secondary pdi file is present.
	 * To meet these requirements and to reuse the same code for primary
	 * and secondary boot modes, bits 0 to 3 in DeviceFlags denote the
	 * PdiSrc. Bits 4 to 19 denote the file name extension.
	 * Example if the offset is 4, file name should be BOOT0004.BIN, these
	 * bits are analogous to multiboot offset in case of primary boot mode.
	 * Bits 20, 21, 22 and 23 denote the logical drive number of the
	 * secondary device. Please note that bits 20, 21, 22 and 23 in device
	 * flags actually map to bits 16, 17, 18 and 19 in secondary device
	 * address specified in bif file.
	 */
	DeviceFlags >>= XLOADER_SD_ADDR_SHIFT;
	MultiBootOffset = (DeviceFlags & XLOADER_SD_ADDR_MASK);
	DeviceFlags >>= XLOADER_LOGICAL_DRV_SHIFT;
	DrvNum += (u8)(DeviceFlags & XLOADER_LOGICAL_DRV_MASK);

	/** - Set logical drive number. */
	/** - Register volume work area, initialize device. */
	BootFile[0U] = (char)DrvNum + 48;
	BootFile[1U] = ':';
	BootFile[2U] = '/';
	Rc = f_mount(&FatFileSystem, BootFile, 0U);

	XLoader_Printf(DEBUG_INFO,"SD: rc= %.8x\n\r", Rc);

	if (Rc != FR_OK) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_INIT, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_SD_INIT\n\r");
		XPlmi_Out32(SdCdnReg, SdCdnVal);
		goto END;
	}

	/** - Create boot image name. */
	Status = XLoader_MakeSdFileName(MultiBootOffset);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Rc = f_open(&FFil, BootFile, (BYTE)FA_READ);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "SD: Unable to open file %s: %d\n",
				BootFile, Rc);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_F_OPEN, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_SD_F_OPEN\n\r");
		XPlmi_Out32(SdCdnReg, SdCdnVal);
		(void)f_unmount(BootFile);
		goto END;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from SD/eMMC to
 * destination address.
 *
 * @param	SrcAddr is the address of the SD flash where copy should
 *			start from
 * @param 	DestAddr is the address of the destination where it
 * 			should copy to
 * @param	Len of the bytes to be copied
 * @param	Flags are unused and only passed to maintain compatibility without
 *			the copy functions of other boot modes
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_SC_F_LSEEK on f_seek fail.
 * 			- XLOADER_ERR_SD_F_READ if reading from SD card fails.
 * 			- XLOADER_ERR_DMA_XFER if DMA transfer fails.
 *
 *****************************************************************************/
int XLoader_SdCopy(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;
	FRESULT Rc; /* Result code */
	UINT Br = 0U;
	u32 TrfLen;
	u64 DestOffset = 0U;
	u32 Length = Len;

	if (Flags == XPLMI_DEVICE_COPY_STATE_INITIATE) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** - Verify whether you can access the source address. */
	Rc = f_lseek(&FFil, (FSIZE_t)SrcAddr);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "SD: Unable to seek to 0x%0x%08x\n",
				(SrcAddr >> 32U), (u32)SrcAddr);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_F_LSEEK, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_LSEEK\n\r");
		goto END;
	}

	/**
	 * - Verify if destination address is 32bit or 64bit.
	 *   - if destination address is 32bit, copy data directly to destination
	 *     address.
	 *   - if destination address is 64bit, copy data indirectly using PMCRAM
	 *     address to destination address.
	 */
	if ((DestAddr >> 32U) == 0U) {
		Rc = f_read(&FFil, (void*)(UINTPTR)DestAddr, Length, &Br);
		if (Rc != FR_OK) {
			XLoader_Printf(DEBUG_GENERAL, "SD: f_read returned %d\r\n", Rc);
			Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_F_READ, (int)Rc);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_SD_F_READ\n\r");
			goto END;
		}
	}
	else {
		while(Length > 0U) {
			if(Length > XLOADER_CHUNK_SIZE) {
				TrfLen = XLOADER_CHUNK_SIZE;
			}
			else {
				TrfLen = Length;
			}

			Rc = f_read(&FFil, (void*)(UINTPTR)XPLMI_PMCRAM_BASEADDR, TrfLen, &Br);
			if (Rc != FR_OK) {
				XLoader_Printf(DEBUG_GENERAL, "SD: f_read returned %d\r\n", Rc);
				Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_F_READ, (int)Rc);
				XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_SD_F_READ\n\r");
				goto END;
			}
			Status = XPlmi_DmaXfr((u64)XPLMI_PMCRAM_BASEADDR,
					(DestAddr + DestOffset), (TrfLen / XPLMI_WORD_LEN),
					XPLMI_PMCDMA_0);
            if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_DMA_XFER, Status);
                 XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_SD_F_READ\n\r");
                 goto END;
            }

			Length -= TrfLen;
			DestOffset += TrfLen;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to close the boot file and unmount the
 * 			file system.
 *
 * @param	None
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_SD_F_CLOSE if failed to close file in SD filesystem.
 * 			- XLOADER_ERR_SD_UNMOUNT if unmounting filesystem fails.
 *
 *****************************************************************************/
 int XLoader_SdRelease(void)
 {
	int Status = XST_FAILURE;
	int PmStatus = XST_FAILURE;
	FRESULT Rc;

	/** - Close PDI file. */
	Rc = f_close(&FFil);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "SD: Unable to close file\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_F_CLOSE, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_CLOSE\n\r");
		goto END;
	}

	/** - Unmount file system. */
	Rc = f_unmount(BootFile);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "SD: Unable to unmount filesystem\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_UMOUNT, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_UMOUNT\n\r");
		goto END;
	}

END:
	/** - Release the device and restore the value of IOU_SLCR_CDN register. */
	PmStatus = XPm_ReleaseDevice(PM_SUBSYS_PMC, SdDeviceNode,
		XPLMI_CMD_SECURE);
	if (Rc == FR_OK) {
		Status = PmStatus;
	}
	XPlmi_Out32(SdCdnReg, SdCdnVal);
	return Status;
 }

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the sd controller and driver.
 * 			It is only called in raw boot mode.
 *
 * @param	DeviceFlags contains the boot mode information
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_MEMSET_SD_INSTANCE if SD instance creation fails.
 * 			- XLOADER_ERR_PM_DEV_SDIO_0 if device request for SDIO_0 fails.
 * 			- XLOADER_ERR_PM_DEV_SDIO_1 if device request for SDIO_1 fails.
 * 			- XLOADER_ERR_SD_LOOKUP if SD lookup fails.
 * 			- XLOADER_ERR_SD_CFG if SD config fails.
 * 			- XLOADER_ERR_SD_CARD_INIT if SD card initialization fails.
 * 			- XLOADER_ERR_MMC_PART_CONFIG if MMC part configuration fails.
 *
 *****************************************************************************/
int XLoader_RawInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	u8 PdiSrc = (u8)(DeviceFlags & XLOADER_PDISRC_FLAGS_MASK);
	u8 DrvNum = XLoader_GetDrvNumSD(PdiSrc);
	XSdPs_Config *SdConfig;
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;
	u32 ErrorCode;
	u32 SdRawBootVal = DeviceFlags & XLOADER_SD_RAWBOOT_MASK;
	u32 MmcConfig;

	/** - Initialise SD instance. */
	Status = XPlmi_MemSetBytes(&SdInstance, sizeof(SdInstance),
				0U, sizeof(SdInstance));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET,
			(int)XLOADER_ERR_MEMSET_SD_INSTANCE);
		goto END;
	}

	/** - Set the device node and CDn register address based on the bootmode. */
	if ((XLoader_IsPdiSrcSD0(PdiSrc) == (u8)TRUE) ||
		(PdiSrc == XLOADER_PDI_SRC_EMMC0)) {
		SdDeviceNode = PM_DEV_SDIO_0;
		SdCdnReg = PMC_IOU_SLCR_SD0_CDN_CTRL;
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_SDIO_0;
	}
	else {
		SdDeviceNode = PM_DEV_SDIO_1;
		SdCdnReg = PMC_IOU_SLCR_SD1_CDN_CTRL;
		ErrorCode =  (u32)XLOADER_ERR_PM_DEV_SDIO_1;
	}

	/** - Request the usage of SD device. */
	Status = XPm_RequestDevice(PM_SUBSYS_PMC, SdDeviceNode,
		CapSecureAccess, XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, Status);
		goto END;
	}
	SdCdnVal = XPlmi_In32(SdCdnReg);
	XPlmi_Out32(SdCdnReg, PMC_IOU_SLCR_SD0_CDN_CTRL_SD0_CDN_CTRL_MASK);

	if (DrvNum != XLOADER_SD_DRV_NUM_0) {
		DrvNum = XLOADER_SD_DRV_NUM_1;
	}

	/** - Get the device configuration. */
	SdConfig = XSdPs_LookupConfig(DrvNum);
	if (NULL == SdConfig) {
		XLoader_Printf(DEBUG_GENERAL,"RAW Lookup config failed\r\n");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_LOOKUP, Status);
		XPlmi_Out32(SdCdnReg, SdCdnVal);
		goto END;
	}

	/** - Configure the device. */
	Status = XSdPs_CfgInitialize(&SdInstance, SdConfig,
				SdConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		XLoader_Printf(DEBUG_GENERAL, "RAW Config init failed\r\n");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_CFG, Status);
		XPlmi_Out32(SdCdnReg, SdCdnVal);
		goto END;
	}

	/** - Initialise the device. */
	Status = XSdPs_CardInitialize(&SdInstance);
	if (Status != XST_SUCCESS) {
		XLoader_Printf(DEBUG_GENERAL, "RAW SD Card init failed\r\n");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SD_CARD_INIT, Status);
		XPlmi_Out32(SdCdnReg, SdCdnVal);
		goto END;
	}

	if ((PdiSrc != XLOADER_PDI_SRC_EMMC1) &&
		(PdiSrc != XLOADER_PDI_SRC_EMMC0)) {
		goto END1;
	}

	if (SdRawBootVal == XLOADER_SD_RAWBOOT_VAL) {
		MmcConfig = XSDPS_MMC_PART_CFG_0_ARG;
	}
	else if (SdRawBootVal == XLOADER_EMMC_BP1_RAW_VAL) {
		MmcConfig = XSDPS_MMC_PART_CFG_1_ARG;
	}
	else if (SdRawBootVal == XLOADER_EMMC_BP2_RAW_VAL)
	{
		MmcConfig = XSDPS_MMC_PART_CFG_2_ARG;
	}
	else {
		goto END1;
	}
	/** - Configure MMC settings in case of EMMC or EMMC0 boot modes. */
	Status = XSdPs_Set_Mmc_ExtCsd(&SdInstance, MmcConfig);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(
					XLOADER_ERR_MMC_PART_CONFIG, Status);
		XPlmi_Out32(SdCdnReg, SdCdnVal);
		goto END;
	}

END1:
	XLoader_Printf(DEBUG_INFO,"Raw init completed\n\r");
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from SD/eMMC to
 * 			destination address in raw boot mode only.
 *
 * @param	SrcAddress is the address of the SD flash where copy should
 * 			start from
 * @param	DestAddress is the address of the destination where it
 * 			should copy to
 * @param	Len of the bytes to be copied
 * @param	Flags param is unused and is only included for compliance with
 * 			other device boot modes
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_RawCopy(u64 SrcAddress, u64 DestAddress, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;
	u64 SrcAddr = SrcAddress;
	u64 DestAddr = DestAddress;
	u32 Length = Len;
	u8 ReadBuffer[XLOADER_SD_RAW_BLK_SIZE];
	u8* ReadBuffPtr;
	u16 DestOffset = (u16)(SrcAddr % XLOADER_SD_RAW_BLK_SIZE);
	u64 StartBlock = SrcAddr / XLOADER_SD_RAW_BLK_SIZE;
	u32 TrfLen;
	u32 NumBlocks;

	if (Flags == XPLMI_DEVICE_COPY_STATE_INITIATE) {
		Status = XST_SUCCESS;
		goto END;
	}

	XLoader_Printf(DEBUG_INFO, "SD Raw Reading Src 0x%0x%08x,"
		"Dest 0x%0x%08x, Length 0x%0x, Flags 0x%0x\r\n",
		(u32)(SrcAddr >> 32U), (u32)(SrcAddr), (u32)(DestAddr >> 32U),
		(u32)DestAddr, Length, Flags);
	SdInstance.Dma64BitAddr = 0U;
	/** - Copy the first chunk (512 bytes) to local buffer. */
	Status = XSdPs_ReadPolled(&SdInstance, (u32)StartBlock, 1U, ReadBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Calculate the remaining length. */
	TrfLen = (u32)(XLOADER_SD_RAW_BLK_SIZE - DestOffset);
	if (Length < TrfLen) {
		TrfLen = Length;
	}

	/** - Transfer the local buffer to destination address. */
	Status = XPlmi_DmaXfr((u64)(UINTPTR)&ReadBuffer[DestOffset], DestAddr,
		(TrfLen >> XPLMI_WORD_LEN_SHIFT), XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	DestAddr += TrfLen;
	Length -= TrfLen;
	++StartBlock;

	/**
	 * - Transfer the remaining data by copying the subsequent segment and
	 *   transmitting it to the destination address.
	 */
	while (Length > XLOADER_SD_CHUNK_SIZE) {
		if((DestAddr >> 32U) == 0U) {
			ReadBuffPtr = (u8 *)(UINTPTR)(DestAddr);
		}
		else {
			SdInstance.Dma64BitAddr = DestAddr;
			ReadBuffPtr = NULL;
		}
		Status  = XSdPs_ReadPolled(&SdInstance, (u32)StartBlock,
			XLOADER_NUM_SECTORS, ReadBuffPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		SrcAddr += XLOADER_SD_CHUNK_SIZE;
		DestAddr += XLOADER_SD_CHUNK_SIZE;
		Length -= XLOADER_SD_CHUNK_SIZE;
		StartBlock += XLOADER_NUM_SECTORS;
	}
	if (Length == 0U) {
		goto END;
	}

	if((DestAddr >> 32U) == 0U) {
		ReadBuffPtr = (u8 *)(UINTPTR)(DestAddr);
	}
	else {
		SdInstance.Dma64BitAddr = DestAddr;
		ReadBuffPtr = NULL;
	}
	NumBlocks = Length / XLOADER_SD_RAW_BLK_SIZE;
	if (NumBlocks != 0U) {
		Status = XSdPs_ReadPolled(&SdInstance, (u32)StartBlock,
			NumBlocks, ReadBuffPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		TrfLen = NumBlocks * XLOADER_SD_RAW_BLK_SIZE;
		DestAddr += TrfLen;
		Length -= TrfLen;
		StartBlock += NumBlocks;
	}
	if (Length == 0U) {
		goto END;
	}
	SdInstance.Dma64BitAddr = 0U;
	Status  = XSdPs_ReadPolled(&SdInstance, (u32)StartBlock, 1U, ReadBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XPlmi_DmaXfr((u64)(UINTPTR)&ReadBuffer[0U], DestAddr,
		(Length >> XPLMI_WORD_LEN_SHIFT), XPLMI_PMCDMA_0);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to restore the card detect value to
 * 			PMC_IOU_SLCR registers.
 *
 * @param	None
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure.
 *
 ****************************************************************************/
int XLoader_RawRelease(void)
{
	int Status = XST_FAILURE;

	/** - Release the device. */
	XPlmi_Out32(SdCdnReg, SdCdnVal);
	Status = XPm_ReleaseDevice(PM_SUBSYS_PMC, SdDeviceNode,
		XPLMI_CMD_SECURE);

	return Status;
}
#endif /* end of XLOADER_SD_0 */
