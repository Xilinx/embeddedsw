/******************************************************************************
*
* Copyright (C) 2008 - 2017 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#include "ff.h"
#include "xil_printf.h"

int platform_init_fs()
{
	static FATFS fatfs;
	static FIL fil;		/* File object */
	FRESULT Res;
	TCHAR *Path = "0:/";

	/*
	 * Register volume work area, initialize device
	 */
	Res = f_mount(&fatfs, Path, 1);
	if (Res != FR_OK) {
		xil_printf("Failed to mount FAT FS. Formatting...\r\n");
		Res = f_mkfs(Path, 1, 0);
		if (Res != FR_OK) {
			xil_printf("Failed to format FAT FS\r\n");
			return -1;
		}

		Res = f_mount(&fatfs, Path, 1);
		if (Res != FR_OK) {
			xil_printf("Failed to mount FAT FS after format\r\n");
			return -1;
		}
	}

	Res = f_open(&fil, "index.htm", FA_READ);
	if (Res) {
		xil_printf("%s: ERROR: unable to locate index.htm in FS\r\n",
			   __func__);
	}

	/* Closing the file */
	f_close(&fil);
	return 0;
}
