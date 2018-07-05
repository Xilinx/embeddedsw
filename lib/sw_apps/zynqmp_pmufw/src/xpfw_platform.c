/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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

#include "xpfw_platform.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xil_types.h"

#include "xpfw_default.h"


#define XPAR_CSU_BASEADDR   0xFFCA0000U
#define XPAR_CSU_VER_OFFSET 0x00000044U
#define CSU_VERSION_REG     (XPAR_CSU_BASEADDR + XPAR_CSU_VER_OFFSET)
#define VERSION_MASK     	(0xFU)

#define	SHIFT24				24U
#define	SHIFT16				16U
#define	SHIFT8				8U


u8 XPfw_PlatformGetPsVersion(void)
{
	return (u8)(XPfw_Read32(CSU_VERSION_REG) & VERSION_MASK);
}

void XPfw_PrintPBRVersion(u32 xpbr_version)
{
	u8 HW_Version = (u8)((xpbr_version >> SHIFT24) & VERSION_MASK);
	u8 Major_Num1 = (u8)((xpbr_version >> SHIFT16) & VERSION_MASK);
	u8 Major_Num2 = (u8)((xpbr_version >> SHIFT8) & VERSION_MASK);
	u8 Minor_Num = (u8)(xpbr_version & VERSION_MASK);

	XPfw_Printf(DEBUG_PRINT_ALWAYS, "PMU_ROM Version: xpbr-v%d.%d.%d-%d\r\n",
			HW_Version, Major_Num1, Major_Num2, Minor_Num);
}
