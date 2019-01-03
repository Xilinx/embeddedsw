/******************************************************************************
* Copyright (C) 2017-2018 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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
#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
#include "xplmi_util.h"
#include "xparameters.h"
#include "ff.h"
#include "xloader.h"
#include "xplmi_generic.h"
/************************** Constant Definitions *****************************/
#define XLOADER_BASE_FILE_NAME_LEN_SD_0 	8
#define XLOADER_BASE_FILE_NAME_LEN_SD_1 	11
#define XLOADER_NUM_DIGITS_IN_FILE_NAME 	4
#define XLOADER_SD_DRV_NUM_0				0
#define XLOADER_SD_DRV_NUM_1				1
#define XLOADER_MULTIBOOT_OFFSET_MASK    	(0x001FFFFF)

/**
 * PMC_GLOBAL Base Address
 */
#define PMC_GLOBAL_BASEADDR      0XF1110000

#define PMC_GLOBAL_PMC_MULTI_BOOT    ( ( PMC_GLOBAL_BASEADDR ) + 0X00000004 )

#define PMC_GLOBAL_PMC_MULTI_BOOT_VALUE_SHIFT   0
#define PMC_GLOBAL_PMC_MULTI_BOOT_VALUE_WIDTH   32
#define PMC_GLOBAL_PMC_MULTI_BOOT_VALUE_MASK    0XFFFFFFFF

 /*Error Codes */
#define XLOADER_ERR_SD_INIT                              (0x1B00)
#define XLOADER_ERR_SD_F_OPEN                            (0x1C00)
#define XLOADER_ERR_SD_F_LSEEK                           (0x1D00)
#define XLOADER_ERR_SD_F_READ                            (0x1E00)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XLoader_MakeSdFileName(char *SdEmmcFileName,
		u32 MultibootReg, u32 DrvNum);
static u32 XLoader_GetDrvNumSD(u32 DeviceFlags);

/************************** Variable Definitions *****************************/

static FIL fil;		/* File object */
static FATFS fatfs;

/*****************************************************************************/
/**
 * This function creates the Boot image name for file system devices based on
 * the multiboot register.
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************/
static void XLoader_MakeSdFileName(char *SdEmmcFileName,
		u32 MultibootReg, u32 DrvNum)
{

	u32 Index;
	u32 Value;
	u32 FileNameLen;
	u32 MultiBootNum = MultibootReg;

	if (0x0U == MultiBootNum)
	{
		/* SD file name is BOOT.BIN when Multiboot register value is 0 */
		if (DrvNum == XLOADER_SD_DRV_NUM_0) {
			(void)XPlmi_Strcpy(SdEmmcFileName, "BOOT.BIN");
		}
		else {
			/* For second SD instance, include drive number 1 as well */
			(void)XPlmi_Strcpy(SdEmmcFileName, "1:/BOOT.BIN");
		}
	}
	else
	{
		/* set default SD file name as BOOT0000.BIN */
		if (DrvNum == XLOADER_SD_DRV_NUM_0) {
			(void)XPlmi_Strcpy(SdEmmcFileName, "BOOT0000.BIN");
			FileNameLen = XLOADER_BASE_FILE_NAME_LEN_SD_0;
		}
		else {
			/* For second SD instance, include drive number 1 as well */
			(void)XPlmi_Strcpy(SdEmmcFileName, "1:/BOOT0000.BIN");
			FileNameLen = XLOADER_BASE_FILE_NAME_LEN_SD_1;
		}

		/* Update file name (to BOOTXXXX.BIN) based on Multiboot register value */
		for(Index = FileNameLen - 1U;
				Index >= (FileNameLen - XLOADER_NUM_DIGITS_IN_FILE_NAME);
				Index--)
		{
			Value = MultiBootNum % 10U;
			MultiBootNum = MultiBootNum / 10U;
			SdEmmcFileName[Index] += (char)Value;
			if (MultiBootNum == 0U)
			{
				break;
			}
		}
	}

	XLoader_Printf(DEBUG_INFO, "File name is %s\r\n", SdEmmcFileName);
}

/*****************************************************************************/
/**
 * This function is used to obtain drive number based on design and boot mode
 *
 * @param       DeviceFlags contain boot mode information
 *
 * @return      Drive number (0 or 1)
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
		(DeviceFlags == XLOADER_PDI_SRC_EMMC))
	{
		DeviceFlags = XLOADER_SD_DRV_NUM_0;
	} else {
		/* For XLOADER_SD1_BOOT_MODE or XLOADER_SD1_LS_BOOT_MODE */
		DeviceFlags = XLOADER_SD_DRV_NUM_1;
	}
#else
	DeviceFlags = XLOADER_SD_DRV_NUM_0;
#endif

	return DeviceFlags;
}

/*****************************************************************************/
/**
 * This function is used to initialize the sd controller and driver
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XLoader_SdInit(u32 DeviceFlags)
{
	int Status;
	FRESULT rc;
	char buffer[32];
	char *boot_file = buffer;
	u32 MultiBootOffset;
	u32 DrvNum;

	DrvNum = XLoader_GetDrvNumSD(DeviceFlags);

	/* Set logical drive number */
	/* Register volume work area, initialize device */
	if (DrvNum == XLOADER_SD_DRV_NUM_0) {
		rc=f_mount(&fatfs, "0:/", 0);
	}
	else {
		rc=f_mount(&fatfs, "1:/", 0);
	}

	XLoader_Printf(DEBUG_INFO,"SD: rc= %.8x\n\r", rc);

	if (rc != FR_OK) {
		Status = XLOADER_ERR_SD_INIT;
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_INIT\n\r");
		goto END;
	}

	/**
	 * Read the Multiboot Register
	 */
	MultiBootOffset = Xil_In32(PMC_GLOBAL_PMC_MULTI_BOOT) & 
						XLOADER_MULTIBOOT_OFFSET_MASK;

	/**
	 * Create boot image name
	 */
	XLoader_MakeSdFileName(boot_file, MultiBootOffset, DrvNum);

	if(boot_file!=NULL) {
		rc = f_open(&fil, boot_file, (BYTE)FA_READ);
		if (rc!=FR_OK) {
			XLoader_Printf(DEBUG_INFO,
					"SD: Unable to open file %s: %d\n", boot_file, rc);
			Status = XLOADER_ERR_SD_F_OPEN;
			XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_OPEN\n\r");
			goto END;
		}
	}
	else
	{
		Status = XLOADER_ERR_SD_F_OPEN;
	}

	Status = XLOADER_SUCCESS;
	END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from SD/eMMC to destination
 * address
 *
 * @param SrcAddress is the address of the SD flash where copy should
 * start from
 *
 * @param DestAddress is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 * 		- XLOADER_SUCCESS for successful copy
 * 		- errors as mentioned in xloader_error.h
 *
 *****************************************************************************/
XStatus XLoader_SdCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status;

	FRESULT rc;	 /* Result code */
	UINT br=Flags;

	rc = f_lseek(&fil, SrcAddress);
	if (rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO,
				"SD: Unable to seek to %x\n", SrcAddress);
		Status = XLOADER_ERR_SD_F_LSEEK;
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_LSEEK\n\r");
		goto END;
	}

	rc = f_read(&fil, (void*)DestAddress, Length, &br);

	if (rc != FR_OK) {
		XLoader_Printf(DEBUG_GENERAL,
				"SD: f_read returned %d\r\n", rc);
		Status = XLOADER_ERR_SD_F_READ;
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_READ\n\r");
		goto END;
	}

	Status = XLOADER_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the sd settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XLoader_SdRelease(void )
{
	(void )f_close(&fil);

	return XLOADER_SUCCESS;
}

#endif /* end of XLOADER_SD_0 */
