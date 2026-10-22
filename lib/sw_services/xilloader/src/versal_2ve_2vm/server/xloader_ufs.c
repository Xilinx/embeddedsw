/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xloader_ufs.c
*
* This is the file which contains ufs related code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  sk  09/23/2023 Initial release
* 1.01  sk  03/28/2025 Updated UFS init and release calls using PM dev nodes
* 1.02 	abh	07/21/2025 Fixed GCC warnings
*       sk  09/26/2025 Added UFS speed change config
* 2.4   gnr 02/09/2026 Moved versal_2ve_2vm server files to dedicated server folder
*       ias  03/26/2026 Handle XPM_PMC_BOOT_DEV_RETAINED in UFS release
*       pre 09/04/2026 Added XLoader_UfsReadDdrCalibData for calib data restore
*       pre 09/26/2026 Fail UFS copy when f_read returns fewer bytes than requested
*       pre 10/22/2026 Added UFS calib data restore under
*                      PLM_ENABLE_DDR_CALIB_RESTORE, disabled by default
*
* </pre>
*
*
******************************************************************************/
/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader_ufs.h"
#include "xplmi_proc.h"
#include "xloader.h"
#include "xplmi.h"
#include "xparameters.h"	/* SDK generated parameters */
#include "xplmi_status.h"	/* PLMI error codes */

#ifdef XLOADER_UFS
#include "xufspsxc.h"		/* UFS device driver */
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
/************************** Function Prototypes ******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_UFS_SRC_FILENAME_SIZE1		(9U)
#define XLOADER_UFS_SRC_FILENAME_SIZE2		(13U)

/************************** Function Prototypes ******************************/
static int XLoader_MakeUFSFileName(u32 MultiBootOffsetVal, u32 BootFileIndex);
static u8 XLoader_GetDrvNumUFS(u8 DeviceFlags);
/************************** Variable Definitions *****************************/
static FIL FFil;		/* File object */
static char BootFile[XLOADER_BASE_FILE_NAME_LEN_UFS + 1U] = {'\0'};
#ifdef PLM_ENABLE_DDR_CALIB_RESTORE
static u8 CalibUfsMounted = (u8)FALSE;	/* The copies registered the calib volume */
static char CalibUfsPath[XLOADER_BASE_FILE_NAME_LEN_UFS + 1U] = {'\0'};
#endif

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the ufs controller and driver.
 *
 * @param	DeviceFlags have the bootmode information.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_MEMSET_UFS_BOOT_FILE if UFS bootfile instance creation
 * 			fails.
 * 			- XLOADER_ERR_PM_DEV_UFS if device request for UFS Dev fails.
 * 			- XLOADER_ERR_UFS_INIT if UFS init fails
 * 			- XLOADER_ERR_UFS_F_OPEN if file is not present or read fails.
 * 			- XLOADER_ERR_UFS_SPEED_CHANGE if speed change config fails
 *
 *****************************************************************************/
int XLoader_UfsInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	FRESULT Rc;
	u32 MultiBootOffset;
	u8 PdiSrc = (u8)(DeviceFlags & XLOADER_PDISRC_FLAGS_MASK);
	u8 DrvNum = XLoader_GetDrvNumUFS(PdiSrc);
	u8 DrvIndex = 0U;
	u8 Index = 0U;
	u8 Value;
	char DrvTmpBuff[XLOADER_BASE_FILE_NAME_LEN_UFS + 1U] = {'\0'};
	static FATFS FatFileSystem;
	u32 PowerMode;

	Status = XPlmi_MemSetBytes(BootFile, sizeof(BootFile), 0U, sizeof(BootFile));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET,
			(int)XLOADER_ERR_MEMSET_UFS_BOOT_FILE);
		goto END;
	}

	/* Clock, Power, Reset & MIO are configuried via PMC CDO */
	Status = XPm_PmcRequestDevice(PM_DEV_UFS);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_UFS, Status);
		goto END;
	}

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
	DeviceFlags >>= XLOADER_UFS_ADDR_SHIFT;
	MultiBootOffset = (DeviceFlags & XLOADER_UFS_ADDR_MASK);
	DeviceFlags >>= XLOADER_LOGICAL_DRV_SHIFT;
	DrvNum += (u8)(DeviceFlags & XLOADER_LOGICAL_DRV_MASK);

	/** - Set logical drive number. */
	/** - Register volume work area, initialize device. */
	/* Logic to extract each digit and convert to char */
	while (DrvNum > 0U) {
		Value = (u8)(DrvNum % XLOADER_NUMERIC_TEN);
		DrvNum /= XLOADER_NUMERIC_TEN;
		DrvTmpBuff[Index] = (char)(Value + XLOADER_ASCII_ZERO_ENCODING);
		Index++;
	}

	/* Update the Drive number in proper order to form mount point eg: 10:/ */
	while (Index > 0U ) {
		BootFile[DrvIndex++] = DrvTmpBuff[Index - 1];
		Index--;
	}

	BootFile[DrvIndex++] = ':';
	BootFile[DrvIndex] = '/';
	Rc = f_mount(&FatFileSystem, BootFile, 0U);

	XLoader_Printf(DEBUG_INFO,"UFS: rc= %.8x\n\r", Rc);

	if (Rc != FR_OK) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_INIT, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UFS_INIT\n\r");
		goto END;
	}


	/** - Create boot image name. */
	Status = XLoader_MakeUFSFileName(MultiBootOffset, DrvIndex);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Rc = f_open(&FFil, BootFile, (BYTE)FA_READ);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_GENERAL, "UFS: Unable to open file %s %d\n",
				BootFile, Rc);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_OPEN, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UFS_F_OPEN\n\r");
		(void)f_unmount(BootFile);
		goto END;
	}

	/** - Configure speed mode */
	/** - Fallback will only work on Uncalibrated Boards */
	PowerMode = XUFSPSXC_HS_G4_B;
	Rc = f_ioctl(BootFile, XUFSPSXC_SPEED_CHANGE, &PowerMode);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_GENERAL, "UFS Fallback to PWM G1\n\r");
		PowerMode = XUFSPSXC_PWM_G1;
		Rc = f_ioctl(BootFile, XUFSPSXC_SPEED_CHANGE, &PowerMode);
		if (Rc != FR_OK) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_SPEED_CHANGE, (int)Rc);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UFS_SPEED_CHANGE\n\r");
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from UFS to
 * destination address.
 *
 * @param	SrcAddr is the address of the UFS flash where copy should
 *			start from
 * @param 	DestAddr is the address of the destination where it
 * 			should copy to
 * @param	Length of the bytes to be copied
 * @param	Flags are unused and only passed to maintain compatibility without
 *			the copy functions of other boot modes
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_SC_F_LSEEK on f_seek fail.
 * 			- XLOADER_ERR_UFS_F_READ if reading from UFS fails or fewer
 * 			  bytes are read than requested.
 * 			- XLOADER_ERR_DMA_XFER if DMA transfer fails.
 *
 *****************************************************************************/
int XLoader_UfsCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	FRESULT Rc; /* Result code */
	UINT Br = 0U;
	u32 TrfLen;
	u64 DestOffset = 0U;

	if (Flags == XPLMI_DEVICE_COPY_STATE_INITIATE) {
		Status = XST_SUCCESS;
		goto END;
	}

	/**
	 * - Seek to the source offset. FatFs clips an offset past end of file
	 * to the file size and still returns FR_OK, so the bytes read are
	 * checked after each f_read.
	 */
	Rc = f_lseek(&FFil, (FSIZE_t)SrcAddr);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "UFS: Unable to seek to 0x%0x%08x\n",
				(SrcAddr >> 32U), (u32)SrcAddr);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_LSEEK, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_UFS_F_LSEEK\n\r");
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
			XLoader_Printf(DEBUG_GENERAL, "UFS: f_read returned %d\r\n", Rc);
			Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_READ, (int)Rc);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UFS_F_READ\n\r");
			goto END;
		}
		/**
		 * f_read returns FR_OK at end of file and reports a short transfer
		 * in Br. Fail so the unread tail is not left as success.
		 */
		if ((u32)Br != Length) {
			XLoader_Printf(DEBUG_GENERAL,
					"UFS: f_read short, requested 0x%x got 0x%x\r\n",
					Length, (u32)Br);
			Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_READ,
					(int)FR_DENIED);
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
				XLoader_Printf(DEBUG_GENERAL, "UFS: f_read returned %d\r\n", Rc);
				Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_READ, (int)Rc);
				XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UFS_F_READ\n\r");
				goto END;
			}
			/**
			 * f_read returns FR_OK at end of file and reports a short
			 * transfer in Br. Fail before DMA so stale PMCRAM is not copied.
			 */
			if ((u32)Br != TrfLen) {
				XLoader_Printf(DEBUG_GENERAL,
						"UFS: f_read short, requested 0x%x got 0x%x\r\n",
						TrfLen, (u32)Br);
				Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_READ,
						(int)FR_DENIED);
				goto END;
			}
			Status = XPlmi_DmaXfr((u64)XPLMI_PMCRAM_BASEADDR,
					(DestAddr + DestOffset), (TrfLen / XPLMI_WORD_LEN),
					XPLMI_PMCDMA_0);
            if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_DMA_XFER, Status);
                 XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UFS_F_READ\n\r");
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


#ifdef PLM_ENABLE_DDR_CALIB_RESTORE
/*****************************************************************************/
/**
 * @brief	This function releases UFS once the calibration data of all the
 *		DDR MCs has been copied.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Error code from XPm_PmcReleaseDevice on failure.
 *
 *****************************************************************************/
int XLoader_UfsCalibDataRelease(void)
{
	int Status = XST_FAILURE;
	FRESULT Rc;

	/**
	 * Unmount only a volume this copy registered. The primary boot file may
	 * already be open on the same drive.
	 */
	if (CalibUfsMounted == (u8)TRUE) {
		CalibUfsMounted = (u8)FALSE;
		Rc = f_unmount(CalibUfsPath);
		if (Rc != FR_OK) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_UMOUNT, (int)Rc);
		}
		else {
			Status = XST_SUCCESS;
		}
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function is used to close the boot file and unmount the
 * 			file system.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_UFS_F_CLOSE if failed to close file in UFS filesystem.
 * 			- XLOADER_ERR_UFS_UNMOUNT if unmounting filesystem fails.
 *
 *****************************************************************************/
 int XLoader_UfsRelease(void)
 {
	int Status = XST_FAILURE;
	int PmStatus = XST_FAILURE;
	FRESULT Rc;

	/** - Close PDI file. */
	Rc = f_close(&FFil);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "UFS: Unable to close file\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_CLOSE, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_UFS_F_CLOSE\n\r");
		goto END;
	}

	/** - Unmount file system. */
	Rc = f_unmount(BootFile);
	if (Rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO, "UFS: Unable to unmount filesystem\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_UMOUNT, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_UFS_UMOUNT\n\r");
		goto END;
	}
END:
	/** - Release the device */
	PmStatus = XPm_PmcReleaseDevice(PM_DEV_UFS);
	/**
	 * Boot device is not released and retained by PMC,
	 * this is intended behavior, return success.
	 */
	if (PmStatus == (int)XPM_PMC_BOOT_DEV_RETAINED) {
		PmStatus = XST_SUCCESS;
	}
	if (Rc == FR_OK) {
		Status = PmStatus;
	}
	return Status;
 }


#ifdef PLM_ENABLE_DDR_CALIB_RESTORE
/*****************************************************************************/
/**
 * @brief	This function copies calibration data from a file on the UFS
 *		filesystem to the destination address.
 *
 * @param	FileName is the calibration data file name on the UFS filesystem
 * @param	DestAddr is the 32-bit destination address to copy the data to
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XST_INVALID_PARAM if FileName or PDI instance is NULL.
 * 		- Error code on failure to mount, open, seek, read or close.
 *
 *****************************************************************************/
int XLoader_UfsReadDdrCalibData(const char *FileName, u32 DestAddr)
{
	int Status = XST_FAILURE;
	const XilPdi *PdiPtr = XLoader_GetPdiInstance();
	u32 DeviceFlags = 0U;
	u8 PdiSrc = 0U;
	u8 DrvNum;
	u8 DrvIndex = 0U;
	u8 Index = 0U;
	u8 Value;
	char FilePath[XLOADER_BASE_FILE_NAME_LEN_UFS + 1U] = {'\0'};
	char DrvTmpBuff[XLOADER_BASE_FILE_NAME_LEN_UFS + 1U] = {'\0'};
	static FATFS CalibFatFs;
	FIL CalibFil;
	u8 FileOpened = (u8)FALSE;
	FRESULT Rc;
	UINT Br = 0U;

	/** Verify input parameters */
	if ((FileName == NULL) || (PdiPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Use the drive XLoader_UfsInit mounted. PdiSrc bits [3:0] select the
	 * controller. Bits [23:20] are the logical drive. For primary boot those
	 * bits are multiboot [19:16], which selects the FAT partition.
	 */
	DeviceFlags = PdiPtr->PdiSrc;
	PdiSrc = (u8)(DeviceFlags & XLOADER_PDISRC_FLAGS_MASK);
	DrvNum = XLoader_GetDrvNumUFS(PdiSrc);
	DeviceFlags >>= XLOADER_UFS_ADDR_SHIFT;
	DeviceFlags >>= XLOADER_LOGICAL_DRV_SHIFT;
	DrvNum += (u8)(DeviceFlags & XLOADER_LOGICAL_DRV_MASK);

	/** Build mount path from drive number (e.g. "2:/" or "10:/") */
	while (DrvNum > 0U) {
		Value = (u8)(DrvNum % XLOADER_NUMERIC_TEN);
		DrvNum /= XLOADER_NUMERIC_TEN;
		DrvTmpBuff[Index] = (char)(Value + XLOADER_ASCII_ZERO_ENCODING);
		Index++;
	}
	while (Index > 0U) {
		CalibUfsPath[DrvIndex++] = DrvTmpBuff[Index - 1U];
		Index--;
	}
	CalibUfsPath[DrvIndex++] = ':';
	CalibUfsPath[DrvIndex] = '/';
	CalibUfsPath[DrvIndex + 1U] = '\0';

	/**
	 * A path without a volume prefix is resolved against drive 0, so prefix the
	 * file name with the drive number.
	 */
	Status = Xil_SStrCat((u8 *)FilePath, (u32)sizeof(FilePath),
			(const u8 *)CalibUfsPath, (u32)sizeof(CalibUfsPath));
	if (Status == XST_SUCCESS) {
		Status = Xil_SStrCat((u8 *)FilePath, (u32)sizeof(FilePath),
				(const u8 *)FileName, (u32)XLOADER_BASE_FILE_NAME_LEN_UFS);
	}
	if (Status != XST_SUCCESS) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * XLoader_UfsInit already registered this volume and keeps the boot PDI file open on
	 * it. Registering the volume again clears fs_type in the filesystem object that the
	 * boot file handle belongs to, which makes every later access to that handle fail
	 * with FR_INVALID_OBJECT. So mount only when the volume has no work area yet, which
	 * also keeps the mount to the first of the per DDR MC copies.
	 */
	Rc = f_open(&CalibFil, FilePath, (BYTE)FA_READ);
	if (Rc == FR_NOT_ENABLED) {
		Rc = f_mount(&CalibFatFs, CalibUfsPath, 0U);
		if (Rc != FR_OK) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_INIT, (int)Rc);
			XLoader_Printf(DEBUG_INFO, "UFS calib data copy: f_mount failed %d\r\n", Rc);
			goto END;
		}
		CalibUfsMounted = (u8)TRUE;
		Rc = f_open(&CalibFil, FilePath, (BYTE)FA_READ);
	}
	if (Rc != FR_OK) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_OPEN, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL, "UFS calib data copy: Unable to open file %s: %d\n",
				FilePath, Rc);
		goto END;
	}
	FileOpened = (u8)TRUE;

	/** Seek to beginning of file */
	Rc = f_lseek(&CalibFil, 0U);
	if (Rc != FR_OK) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_LSEEK, (int)Rc);
		XLoader_Printf(DEBUG_INFO, "UFS calib data copy: f_lseek failed %d\r\n", Rc);
		goto END;
	}

	/** Read calibration data from file into 32-bit destination address */
	Rc = f_read(&CalibFil, (void *)(UINTPTR)DestAddr, XLOADER_CALIBDATA_SIZE_BYTES, &Br);
	if (Rc != FR_OK) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_READ, (int)Rc);
		XLoader_Printf(DEBUG_INFO, "UFS calib data copy: f_read failed %d\r\n", Rc);
		goto END;
	}
	if ((u32)Br != XLOADER_CALIBDATA_SIZE_BYTES) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_READ, (int)FR_DENIED);
		XLoader_Printf(DEBUG_INFO,
				"UFS calib data copy: f_read short, requested 0x%x got 0x%x\r\n",
				XLOADER_CALIBDATA_SIZE_BYTES, (u32)Br);
		goto END;
	}
	Status = XST_SUCCESS;

END:
	if (FileOpened == (u8)TRUE) {
		Rc = f_close(&CalibFil);
		if ((Status == XST_SUCCESS) && (Rc != FR_OK)) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_F_CLOSE, (int)Rc);
		}
	}

	/**
	 * UFS remains owned by the primary boot flow across all calibration copies.
	 * Do not call XLoader_UfsRelease here because it closes the boot PDI file.
	 */
	if (Status != XST_SUCCESS) {
		XLoader_Printf(DEBUG_GENERAL, "UFS calib data copy failed: 0x%x\n\r",
				Status);
	}

	return Status;
}
#endif


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
static u8 XLoader_GetDrvNumUFS(u8 DeviceFlags)
{
	u8 DrvNum;

	(void)DeviceFlags;
#ifdef XLOADER_FS_MULTI_PART
	DrvNum = XLOADER_UFS_DRV_NUM_10;
#else
	DrvNum = XLOADER_UFS_DRV_NUM_2;
#endif
	return DrvNum;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Boot image name for file system devices
 * 			based on the multiboot register.
 *
 * @param	MultiBootOffsetVal is the value of the multiboot register that
 *			would be suffixed to the filename
 * @param	BootFileIndex is the index into the search sequence used to
 *			construct the suffixed BOOT.BIN filename
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_UFS_MAX_BOOT_FILES_LIMIT if search for BOOT.BIN
 * 			crosses max limit.
 *
 ******************************************************************************/
static int XLoader_MakeUFSFileName(u32 MultiBootOffsetVal, u32 BootFileIndex)
{
	int Status = XST_FAILURE;
	u8 Index = (u8)(XLOADER_MULTIBOOT_FILENAME_SIZE + BootFileIndex);
	u8 Value;
	u32 MultiBootOffset = MultiBootOffsetVal;
    /**
     * - Verify that multiboot offset is within the files limits.
     */
	if (MultiBootOffset >= XLOADER_UFS_MAX_BOOT_FILES_LIMIT) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_MAX_BOOT_FILES_LIMIT, XIL_SIGNED_ZERO);
		goto END;
	}

	if (0x0U == MultiBootOffset) {
		Status = Xil_SStrCat((u8*)BootFile, XLOADER_BASE_FILE_NAME_LEN_UFS,
			(const u8*)"BOOT.BIN", XLOADER_UFS_SRC_FILENAME_SIZE1);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = Xil_SStrCat((u8*)BootFile, XLOADER_BASE_FILE_NAME_LEN_UFS,
			(const u8*)"BOOT0000.BIN", XLOADER_UFS_SRC_FILENAME_SIZE2);
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


#endif /* End Of XLOADER_UFS */

/** @} end of xloader_server_apis group */
