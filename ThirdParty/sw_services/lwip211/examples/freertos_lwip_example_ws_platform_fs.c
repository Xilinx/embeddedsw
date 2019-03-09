/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include "ff.h"
#include "xil_printf.h"

int platform_init_fs()
{
	static FATFS fatfs;
	static FIL fil;		/* File object */
	FRESULT Res;
	TCHAR *Path = "0:/";
	BYTE work[FF_MAX_SS];

	/*
	 * Register volume work area, initialize device
	 */
	Res = f_mount(&fatfs, Path, 1);
	if (Res != FR_OK) {
		xil_printf("Failed to mount FAT FS. Formatting...\r\n");
		Res = f_mkfs(Path, FM_SFD, 0, work, sizeof work);
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
