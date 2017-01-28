/******************************************************************************
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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

#include "xpfw_default.h"
#include "xpfw_xpu.h"

#define    XMPU_DDR_0_BASE_ADDR    0xFD000000U
#define    XMPU_DDR_1_BASE_ADDR    0xFD010000U
#define    XMPU_DDR_2_BASE_ADDR    0xFD020000U
#define    XMPU_DDR_3_BASE_ADDR    0xFD030000U
#define    XMPU_DDR_4_BASE_ADDR    0xFD040000U
#define    XMPU_DDR_5_BASE_ADDR    0xFD050000U
#define    XMPU_FPD_BASE_ADDR    0xFD5D0000U
#define    XMPU_OCM_BASE_ADDR    0xFFA70000U
#define    XPPU_BASE_ADDR    0xFF980000U

#define XPU_ISR_OFFSET	0x10U
#define XPU_IER_OFFSET	0x18U

struct XpuReg {
	u32 BaseAddress;
	u32 MaskAll;
};

struct XpuReg XpuRegList[] =
{
	{
		.BaseAddress = XMPU_DDR_0_BASE_ADDR,
		.MaskAll = 0xFU,
	},
	{
		.BaseAddress = XMPU_DDR_1_BASE_ADDR,
		.MaskAll = 0xFU,
	},
	{
		.BaseAddress = XMPU_DDR_2_BASE_ADDR,
		.MaskAll = 0xFU,
	},
	{
		.BaseAddress = XMPU_DDR_3_BASE_ADDR,
		.MaskAll = 0xFU,
	},
	{
		.BaseAddress = XMPU_DDR_4_BASE_ADDR,
		.MaskAll = 0xFU,
	},
	{
		.BaseAddress = XMPU_DDR_5_BASE_ADDR,
		.MaskAll = 0xFU,
	},
	{
		.BaseAddress = XMPU_FPD_BASE_ADDR,
		.MaskAll = 0xFU,
	},
	{
		.BaseAddress = XMPU_OCM_BASE_ADDR,
		.MaskAll = 0xFU,
	},
	{
		.BaseAddress = XPPU_BASE_ADDR,
		.MaskAll = 0xEFU,
	},
};


/**
 * Enable interrupts for all XMPU/XPPU Instances
 */
void XPfw_XpuIntrInit(void)
{
	u32 Idx;
	fw_printf("EM: Enabling XMPU/XPPU interrupts\r\n");
	for(Idx=0; Idx < ARRAYSIZE(XpuRegList);Idx++) {
		/* Enable all the Interrupts for this XMPU/XPPU Instance */
		XPfw_Write32(XpuRegList[Idx].BaseAddress + XPU_IER_OFFSET,
						XpuRegList[Idx].MaskAll);
	}
}

/**
 * Ack interrupts for all XMPU/XPPU Instances so that any new
 * interrupts occurring later can trigger an Error interrupt to PMU
 */
void XPfw_XpuIntrAck(void)
{
	u32 Idx;
	for(Idx=0; Idx < ARRAYSIZE(XpuRegList);Idx++) {
		/* Ack the Interrupts */
		XPfw_Write32(XpuRegList[Idx].BaseAddress + XPU_ISR_OFFSET,
						XpuRegList[Idx].MaskAll);
	}
}

/**
 * A placeholder for handling XPPU/XMPU errors
 * This routine is called when ever a XMPU/XPPU error occurs
 * It prints a message if debug is enabled and Acks the errors
 *
 * @param ErrorId is the error identifier passed by Error Manager
 */
void XPfw_XpuIntrHandler(u8 ErrorId)
{
	fw_printf("EM: XMPU/XPPU violation occured (ErrorId: %d)\r\n", ErrorId);
	XPfw_XpuIntrAck();
}
