/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
