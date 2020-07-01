/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xloader_sd.c
*
* This is the file which contains sd related code for the PMC FW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   09/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xloader_sd.h"
#include "xplmi_hw.h"
#include "xloader.h"
#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
#include "xplmi_util.h"
#include "xparameters.h"
#include "ff.h"
#include "xloader.h"
#include "xplmi_generic.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XLoader_MakeSdFileName(char *SdEmmcFileName, u32 MultibootReg);
static u32 XLoader_GetDrvNumSD(u32 DeviceFlags);

/************************** Variable Definitions *****************************/
static FIL FFil;		/* File object */
static FATFS fatfs;
static u32 XLoader_IsSDRaw;
static XSdPs SdInstance = {0U,};
static char BootFile[XLOADER_BASE_FILE_NAME_LEN_SD_1 + 1U] = {0U};

/*****************************************************************************/
/**
 * @brief	This function creates the Boot image name for file system devices
*  based on the multiboot register.
 *
 * @param       SdEmmcFileName is the pointer to the file name
 * @param	MultiBootOffset is the value of the multiboot register that
 *		would be suffixed to the filename
 *
 * @return      None
 *
 ******************************************************************************/
static void XLoader_MakeSdFileName(char* SdEmmcFileName, u32 MultiBootOffset)
{
	int Index;
	u32 Value;
	char BootNo[XLOADER_NUM_DIGITS_IN_FILE_NAME + 1U] = "0000";
	u32 FileNo = MultiBootOffset;

	if (0x0U == MultiBootOffset) {
		(void)XPlmi_Strcat(SdEmmcFileName, "BOOT.BIN");
	}
	else {
		for (Index = XLOADER_NUM_DIGITS_IN_FILE_NAME - 1U;
			(Index >= 0) && (FileNo > 0U); Index--) {
			Value = FileNo % 10U;
			FileNo /= 10U;
			BootNo[Index] += (char)Value;
		}
		(void)XPlmi_Strcat(SdEmmcFileName, "BOOT");
		(void)XPlmi_Strcat(SdEmmcFileName, BootNo);
		(void)XPlmi_Strcat(SdEmmcFileName, ".BIN");
	}

	XLoader_Printf(DEBUG_INFO, "File name is %s\r\n", SdEmmcFileName);
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
static u32 XLoader_GetDrvNumSD(u32 DeviceFlags)
{
	/*
	 * If design has both SD0 and SD1, select drive number based on bootmode
	 * If design has ONLY SD0 or ONLY SD1, drive number should be "0"
	 */
#ifdef XPAR_XSDPS_1_DEVICE_ID
	if ((DeviceFlags == XLOADER_PDI_SRC_SD0) ||
		(DeviceFlags == XLOADER_PDI_SRC_SD0_RAW) ||
		(DeviceFlags == XLOADER_PDI_SRC_EMMC0) ||
		(DeviceFlags == XLOADER_PDI_SRC_EMMC0_RAW)) {
		DeviceFlags = XLOADER_SD_DRV_NUM_0;
	}
	else {
		/*
		 * For XLOADER_SD1_BOOT_MODE or XLOADER_SD1_LS_BOOT_MODE
		 * or XLOADER_EMMC_BOOT_MODE or XLOADER_SD1_RAW_BOOT_MODE
		 * or XLOADER_SD1_LS_RAW_BOOT_MODE or XLOADER_EMMC_RAW_BOOT_MODE
		 */
		if ((DeviceFlags == XLOADER_PDI_SRC_SD1)
		|| (DeviceFlags == XLOADER_PDI_SRC_SD1_LS)
		|| (DeviceFlags == XLOADER_PDI_SRC_EMMC)) {
			DeviceFlags = XLOADER_SD_DRV_NUM_5;
		}
		else {
			DeviceFlags = XLOADER_SD_DRV_NUM_1;
		}
	}
#else
	DeviceFlags = XLOADER_SD_DRV_NUM_0;
#endif

	return DeviceFlags;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the sd controller and driver.
 *
 * @param	DeviceFlags have the bootmode information.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_SdInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	FRESULT Rc;
	u32 MultiBootOffset;
	u32 PdiSrc = DeviceFlags & XLOADER_PDISRC_FLAGS_MASK;
	u32 DrvNum = XLoader_GetDrvNumSD(PdiSrc);
	XLoader_IsSDRaw = FALSE;

	memset(BootFile, 0U, sizeof(BootFile));

	if ((DeviceFlags & XLOADER_SD_SBD_ADDR_SET_MASK) ==
		XLOADER_SD_SBD_ADDR_SET_MASK) {
		/*
		 * Filesystem boot modes require the filename extension as well as
		 * the logical drive in which the secondary pdi file is present.
		 * To meet these requirements and to reuse the same code for primary
		 * and secondary boot modes, bits 0 to 7 in DeviceFlags denote the
		 * PdiSrc. Bit 8, if set, denotes that filesystem boot is secondary.
		 * Bits 9 to 24 denote the file name extension. Example if the offset
		 * is 4, file name should be BOOT0004.BIN, these bits are analogous to
		 * multiboot offset in case of primary boot mode. Bits 25, 26, 27 and 28
		 * denote the logical drive number of the secondary device. Please note
		 * that bits 25, 26, 27 and 28 in device flags actually map to bits 16,
		 * 17, 18 and 19 in secondary device address specified in bif file.
		 */
		DeviceFlags = DeviceFlags >> XLOADER_SD_SBD_ADDR_SHIFT;
		/* Secondary Boot in FAT filesystem mode */
		MultiBootOffset = (DeviceFlags & XLOADER_SD_SBD_ADDR_MASK);
		DrvNum += ((DeviceFlags & XLOADER_LOGICAL_DRV_MASK) >>
				XLOADER_LOGICAL_DRV_SHIFT);
	}
	else {
		/* Primary Boot in FAT filesystem mode */
		MultiBootOffset = XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT) &
					XLOADER_MULTIBOOT_OFFSET_MASK;
		MultiBootOffset &= XLOADER_MULTIBOOT_OFFSET_MASK;
	}

	/* Set logical drive number */
	/* Register volume work area, initialize device */
	BootFile[0U] = DrvNum + 48U;
	BootFile[1U] = ':';
	BootFile[2U] = '/';
	Rc = f_mount(&fatfs, BootFile, 0U);

	XLoader_Printf(DEBUG_INFO,"SD: rc= %.8x\n\r", Rc);

	if (Rc != FR_OK) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_INIT, Rc);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_SD_INIT\n\r");
		goto END;
	}

	/*
	 * Create boot image name
	 */
	XLoader_MakeSdFileName(BootFile, MultiBootOffset);

	Rc = f_open(&FFil, BootFile, (BYTE)FA_READ);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "SD: Unable to open file %s: %d\n",
				BootFile, Rc);
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_F_OPEN, Rc);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_SD_F_OPEN\n\r");
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
 * @param	SrcAddress is the address of the SD flash where copy should
 * 		start from
 * @param 	DestAddress is the address of the destination where it
 * 		should copy to
 * @param	Length of the bytes to be copied
 * @param	Flags are unused and only passed to maintain compatibility without
 *		the copy functions of other boot modes
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_SdCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	FRESULT Rc; /* Result code */
	UINT Br = 0U;
	(void)Flags;

	Rc = f_lseek(&FFil, SrcAddress);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "SD: Unable to seek to %x\n",
				SrcAddress);
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_F_LSEEK, Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_LSEEK\n\r");
		goto END;
	}

	Rc = f_read(&FFil, (void*)(UINTPTR)DestAddress, Length, &Br);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_GENERAL, "SD: f_read returned %d\r\n", Rc);
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_F_READ, Rc);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_SD_F_READ\n\r");
		goto END;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to close the boot file and unmount the
 * file system.
 *
 * @param       None
 *
 * @return      XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
 int XLoader_SdRelease(void)
 {
	int Status = XST_FAILURE;
	FRESULT Rc;

	Rc = f_close(&FFil);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "SD: Unable to close file\n\r");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_F_CLOSE, Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_CLOSE\n\r");
		goto END;
	}

	Rc = f_unmount(BootFile);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "SD: Unable to unmount filesystem\n\r");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_UMOUNT, Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_UMOUNT\n\r");
		goto END;
	}
	Status = XST_SUCCESS;

END:
	return Status;
 }

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the sd controller and driver.
 * It is only called in raw boot mode.
 *
 * @param	DeviceFlags contains the boot mode information
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_RawInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	u32 PdiSrc = DeviceFlags & XLOADER_PDISRC_FLAGS_MASK;
	u32 DrvNum = XLoader_GetDrvNumSD(PdiSrc);
	XLoader_IsSDRaw = TRUE;
	XSdPs_Config *SdConfig;

	memset(&SdInstance, 0U, sizeof(SdInstance));

	/*
	 * Initialize the host controller
	 */
	SdConfig = XSdPs_LookupConfig(DrvNum);
	if (NULL == SdConfig) {
		XLoader_Printf(DEBUG_GENERAL,"RAW Lookup config failed\r\n");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_LOOKUP, Status);
		goto END;
	}

	Status = XSdPs_CfgInitialize(&SdInstance, SdConfig,
				SdConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		XLoader_Printf(DEBUG_GENERAL, "RAW Config init failed\r\n");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_CFG, Status);
		goto END;
	}

	Status = XSdPs_CardInitialize(&SdInstance);
	if (Status != XST_SUCCESS) {
		XLoader_Printf(DEBUG_GENERAL, "RAW SD Card init failed\r\n");
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_CARD_INIT, Status);
		goto END;
	}

	if (DeviceFlags == XLOADER_PDI_SRC_EMMC_RAW) {
		Status = XSdPs_Set_Mmc_ExtCsd(
					&SdInstance, XSDPS_MMC_PART_CFG_0_ARG);
	}
	else if (DeviceFlags == XLOADER_PDI_SRC_EMMC_RAW_BP1) {
		Status = XSdPs_Set_Mmc_ExtCsd(
					&SdInstance, XSDPS_MMC_PART_CFG_1_ARG);
	}
	else if (DeviceFlags == XLOADER_PDI_SRC_EMMC_RAW_BP2)
	{
		Status = XSdPs_Set_Mmc_ExtCsd(
					&SdInstance, XSDPS_MMC_PART_CFG_2_ARG);
	}
	else {
		/* MISRA-C compliance */
	}
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(
					XLOADER_ERR_MMC_PART_CONFIG, Status);
		goto END;
	}

	XLoader_Printf(DEBUG_INFO,"Raw init completed\n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from SD/eMMC to
 * destination address in raw boot mode only.
 *
 * @param	SrcAddress is the address of the SD flash where copy should
 * 		start from
 * @param	DestAddress is the address of the destination where it
 * 		should copy to
 * @param	Length of the bytes to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_RawCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	u32 BlockNumber;
	u32 DataOffset;
	u32 RemainingBytes;
	u8 ReadBuffer[1024U];
	u8* ReadBuffPtr;
	u32 SectorReadLen;
	u32 NoOfSectors;
	u32 Destination = DestAddress;
	(void) Flags;

	RemainingBytes = Length;
	BlockNumber = SrcAddress / XLOADER_SD_RAW_BLK_SIZE;
	DataOffset = SrcAddress % XLOADER_SD_RAW_BLK_SIZE;
	/*
	 * Setting the Read len for the first sector partial read
	 */
	SectorReadLen =  XLOADER_SD_RAW_BLK_SIZE - DataOffset;

	XLoader_Printf(DEBUG_INFO, "SD Raw Reading Src 0x%08x, Dest 0x%0x%08x, "
		       "Length 0x%0x, Flags 0x%0x\r\n", SrcAddress,
		       (u32)(DestAddress >> 32U), (u32)DestAddress,
		       Length, Flags);

	do
	{
		/*
		 * Make sure last sector data is read properly
		 */
		if (RemainingBytes < SectorReadLen) {
			SectorReadLen = RemainingBytes;
		}

		/*
		 * Read to temparory PRAM address if the length is not equal to 512 bytes
		 */
		if (SectorReadLen != XLOADER_SD_RAW_BLK_SIZE) {
			ReadBuffPtr = &ReadBuffer[0U];
			NoOfSectors = 1U;
		}
		else {
			ReadBuffPtr = (u8 *)Destination;
			NoOfSectors = RemainingBytes / XLOADER_SD_RAW_BLK_SIZE;
			if (NoOfSectors > XLOADER_SD_RAW_NUM_SECTORS) {
				NoOfSectors = XLOADER_SD_RAW_NUM_SECTORS;
			}
		}

		Status  = XSdPs_ReadPolled(&SdInstance, (u32)BlockNumber,
					NoOfSectors, (u8*)ReadBuffPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/*
		 * Copy the temporary read data to actual destination
		 */
		if (SectorReadLen != XLOADER_SD_RAW_BLK_SIZE) {
			(void)XPlmi_MemCpy((void *)Destination, (ReadBuffPtr
					+ DataOffset), SectorReadLen);
		}
		BlockNumber += NoOfSectors;
		Destination += (NoOfSectors * SectorReadLen);
		RemainingBytes -= (NoOfSectors * SectorReadLen);
		SectorReadLen = XLOADER_SD_RAW_BLK_SIZE;
		DataOffset = 0U;
	} while (RemainingBytes > 0U);

	Status = XST_SUCCESS;

END:
	return Status;
}
#endif /* end of XLOADER_SD_0 */
