/*
 * Copyright (c) 2017 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include "ff.h"
#include "xil_printf.h"

int platform_init_fs()
{
	static FATFS fatfs;
	FRESULT Res;
	TCHAR *Path = "0:/";

	/* Try to mount FAT file system */
	Res = f_mount(&fatfs, Path, 1);
	if (Res != FR_OK) {
		xil_printf("Volume is not FAT formated; formating FAT\r\n");
		Res = f_mkfs(Path, 1, 0);
		if (Res != FR_OK) {
			xil_printf("Unable to format FATfs\r\n");
			return -1;
		}

		Res = f_mount(&fatfs, Path, 1);
		if (Res != FR_OK) {
			xil_printf("Unable to mount FATfs\r\n");
			return -1;
		}
	}
	xil_printf("File system initialization successful\r\n");

	return 0;
}
