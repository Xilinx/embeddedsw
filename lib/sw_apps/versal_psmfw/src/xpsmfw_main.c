/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xpsmfw_main.c
*
* This is the main file for PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#include "xil_printf.h"
#include "xil_io.h"
#include "mb_interface.h"
#include "xpsmfw_main.h"
#include "xpsmfw_debug.h"
#include "xpsmfw_init.h"
#include "xpsmfw_iomodule.h"

int main(void)
{
	int Status = XST_FAILURE;

	XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "PSM Firmware version: %s "
			"[Build: %s %s ] \r\n",	versal_PSMFW_VERSION, __DATE__, __TIME__);

	Status = XPsmFw_Init();
	/* Init IOModule and connect interrupts */
	XPsmFw_IoModuleInit(XPAR_PSU_PSM_IOMODULE_0_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: Error! PSM Initialization failed\r\n", __func__);
	}

	/* Put Microblaze to Sleep in an infinite loop */
	do{
		mb_sleep();
	} while(1);
}
