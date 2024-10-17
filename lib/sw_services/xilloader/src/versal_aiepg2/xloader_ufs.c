/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
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
static int XLoader_MakeUFSFileName(u32 MultiBootOffsetVal);
static u8 XLoader_GetDrvNumUFS(u8 DeviceFlags);
/************************** Variable Definitions *****************************/
static FIL FFil;		/* File object */
static char BootFile[XLOADER_BASE_FILE_NAME_LEN_UFS + 1U] = {'\0'};

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the ufs controller and driver.
 *
 * @param	DeviceFlags have the bootmode information.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_MEMSET_UFS_BOOT_FILE if SD bootfile instance creation
 * 			fails.
 * 			- XLOADER_ERR_PM_DEV_SDIO_0 if device request for SDIO_0 fails.
 * 			- XLOADER_ERR_PM_DEV_SDIO_1 if device request for SDIO_1 fails.
 * 			- XLOADER_ERR_SD_F_OPEN if file is not present or read fails.
 *
 *****************************************************************************/
int XLoader_UfsInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	FRESULT Rc;
	u32 MultiBootOffset;
	u8 PdiSrc = (u8)(DeviceFlags & XLOADER_PDISRC_FLAGS_MASK);
	u8 DrvNum = XLoader_GetDrvNumUFS(PdiSrc);
	static FATFS FatFileSystem;
//	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;

	Status = XPlmi_MemSetBytes(BootFile, sizeof(BootFile), 0U, sizeof(BootFile));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET,
			(int)XLOADER_ERR_MEMSET_UFS_BOOT_FILE);
		goto END;
	}

	/* Clock, Power, Reset & MIO are configuried via PMC CDO */
#if 0 /* Node details to be updated for UFS */
	if ((XLoader_IsPdiSrcSD0(PdiSrc) == (u8)TRUE) ||
		(PdiSrc == XLOADER_PDI_SRC_EMMC0)) {
		SdDeviceNode = PM_DEV_SDIO_0;
		SdCdnReg = PMC_IOU_SLCR_SD0_CDN_CTRL;
		ErrorCode = (u32)XLOADER_ERR_PM_DEV_SDIO_0;
	}
	else {
		SdDeviceNode = PM_DEV_SDIO_1;
		SdCdnReg = PMC_IOU_SLCR_SD1_CDN_CTRL;
		ErrorCode = XLOADER_ERR_PM_DEV_SDIO_1;
	}
#endif

#if 0 /* Update for UFS */
	Status = XPm_RequestDevice(PM_SUBSYS_PMC, SdDeviceNode,
		CapSecureAccess, XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus((XPlmiStatus_t)ErrorCode, Status);
		goto END;
	}
	SdCdnVal = XPlmi_In32(SdCdnReg);
	XPlmi_Out32(SdCdnReg, PMC_IOU_SLCR_SD0_CDN_CTRL_SD0_CDN_CTRL_MASK);
#endif
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
	BootFile[0U] = (char)DrvNum + 48;
	BootFile[1U] = ':';
	BootFile[2U] = '/';
	Rc = f_mount(&FatFileSystem, BootFile, 0U);

	XLoader_Printf(DEBUG_INFO,"UFS: rc= %.8x\n\r", Rc);

	if (Rc != FR_OK) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_INIT, (int)Rc);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UFS_INIT\n\r");
		goto END;
	}

	/** - Create boot image name. */
	Status = XLoader_MakeUFSFileName(MultiBootOffset);
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
 * 			- XLOADER_ERR_UFS_F_READ if reading from UFS card fails.
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

	/** - Verify whether you can access the source address. */
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


/*****************************************************************************/
/**
 * @brief	This function is used to close the boot file and unmount the
 * 			file system.
 *
 * @param	None
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
	Status = XST_SUCCESS;
END:
#if 0 /* TODO to be updated for UFS */
	/** - Release the device and restore the value of IOU_SLCR_CDN register. */
	Status = XPm_ReleaseDevice(PM_SUBSYS_PMC, SdDeviceNode,
		XPLMI_CMD_SECURE);
	XPlmi_Out32(SdCdnReg, SdCdnVal);
	return Status;
#endif
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
static u8 XLoader_GetDrvNumUFS(u8 DeviceFlags)
{
	u8 DrvNum;
	/**
	 * - If design has UFS
	 */
#if (XPAR_XUFSPSXC_NUM_INSTANCES >= 1)
	if (DeviceFlags == XLOADER_PDI_SRC_UFS) {
		DrvNum = XLOADER_UFS_DRV_NUM_2;
		goto END;
	}
#else
	(void)DeviceFlags;
	DrvNum = XLOADER_UFS_DRV_NUM_0;
#endif

END:
	return DrvNum;
}

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
 * 			- XLOADER_ERR_UFS_MAX_BOOT_FILES_LIMIT if search for BOOT.BIN
 * 			crosses max limit.
 *
 ******************************************************************************/
static int XLoader_MakeUFSFileName(u32 MultiBootOffsetVal)
{
	int Status = XST_FAILURE;
	u8 Index = 10U;
	u8 Value;
	u32 MultiBootOffset = MultiBootOffsetVal;
    /**
     * - Verify that multiboot offset is within the files limits.
     */
	if (MultiBootOffset >= XLOADER_UFS_MAX_BOOT_FILES_LIMIT) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UFS_MAX_BOOT_FILES_LIMIT, 0);
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
