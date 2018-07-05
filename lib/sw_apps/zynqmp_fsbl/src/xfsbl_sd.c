/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_sd.c
*
* This is the file which contains sd related code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   04/21/14 Initial release
* 2.0   bv   12/02/16 Made compliance to MISRAC 2012 guidelines
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xfsbl_main.h"

#if (defined(XFSBL_SD_0) || defined(XFSBL_SD_1))

#include "xparameters.h"
#include "ff.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
extern void XFsbl_MakeSdFileName(char *XFsbl_SdEmmcFileName,
		u32 MultibootReg, u32 DrvNum);

extern u32 XFsbl_GetDrvNumSD(u32 DeviceFlags);

/************************** Variable Definitions *****************************/

static FIL fil;		/* File object */

/*****************************************************************************/
/**
 * This function is used to initialize the qspi controller and driver
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
u32 XFsbl_SdInit(u32 DeviceFlags)
{
	static FATFS fatfs;
	u32 Status;
	FRESULT rc;
	char buffer[32];
	char *boot_file = buffer;
	u32 MultiBootOffset;
	u32 DrvNum;

	DrvNum = XFsbl_GetDrvNumSD(DeviceFlags);

	/* Set logical drive number */
	/* Register volume work area, initialize device */
	if (DrvNum == XFSBL_SD_DRV_NUM_0) {
		rc=f_mount(&fatfs, "0:/", 0);
	}
	else {
		rc=f_mount(&fatfs, "1:/", 0);
	}

	XFsbl_Printf(DEBUG_INFO,"SD: rc= %.8x\n\r", rc);

	if (rc != FR_OK) {
		Status = XFSBL_ERROR_SD_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SD_INIT\n\r");
		goto END;
	}

	/**
         * Read the Multiboot Register
         */
        MultiBootOffset = XFsbl_In32(CSU_CSU_MULTI_BOOT);

	/**
	 * Create boot image name
	 */
	XFsbl_MakeSdFileName(boot_file, MultiBootOffset, DrvNum);

	if(boot_file!=NULL) {
		rc = f_open(&fil, boot_file, (BYTE)FA_READ);
		if (rc!=FR_OK) {
		XFsbl_Printf(DEBUG_INFO,
			"SD: Unable to open file %s: %d\n", boot_file, rc);
		Status = XFSBL_ERROR_SD_F_OPEN;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SD_F_OPEN\n\r");
		goto END;
		}
	}
	else
	{
		Status = XFSBL_ERROR_SD_F_OPEN;
	}

	Status = XFSBL_SUCCESS;
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
 * 		- XFSBL_SUCCESS for successful copy
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
u32 XFsbl_SdCopy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length)
{
	u32 Status;

	FRESULT rc;	 /* Result code */
	UINT br=0U;

	rc = f_lseek(&fil, SrcAddress);
	if (rc != FR_OK) {
		XFsbl_Printf(DEBUG_INFO,
			"SD: Unable to seek to %x\n", SrcAddress);
		Status = XFSBL_ERROR_SD_F_LSEEK;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SD_F_LSEEK\n\r");
		goto END;
	}

	rc = f_read(&fil, (void*)DestAddress, Length, &br);

	if (rc != FR_OK) {
		XFsbl_Printf(DEBUG_GENERAL,
			"SD: f_read returned %d\r\n", rc);
		Status = XFSBL_ERROR_SD_F_READ;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SD_F_READ\n\r");
		goto END;
	}

	Status = XFSBL_SUCCESS;
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
u32 XFsbl_SdRelease(void )
{
	(void )f_close(&fil);

	return XFSBL_SUCCESS;
}

#endif /* end of XFSBL_SD */
