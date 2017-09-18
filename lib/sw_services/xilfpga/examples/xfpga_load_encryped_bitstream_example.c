/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xfpga_load_bitstream_example.c
*
* This file contains the example using Xilfpga library to transfer the user provided Bit-stream
* into zynqmp pl region.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   Nava    08/06/16  First release
*
* </pre>
*
******************************************************************************/

#include "xil_printf.h"
#include "xilfpga_pcap.h"
#include "xfpga_config.h"

#define ENCRPTED_BITSTREAM 0x8
char key[65] = "0D00C023E238AC9039EA984D49AA8C819456A98C124AE890ACEF002100128932";
char iv[25] = "F7F8FDE08674A28DC6ED8E37";

int main()
{
	u32 count = XFPGA_NUMBYTES/4;
	u64 addr = XFPGA_BASE_ADDRESS;
	s32 Status;

	xil_printf("Loading Encrypted Bit-stream for DDR location :0x%x and Length :%d\n\r",
				XFPGA_BASE_ADDRESS, XFPGA_NUMBYTES);
	xil_printf("Trying to configure the PL ......\n\r");
	memcpy(addr + XFPGA_NUMBYTES, key, 64);
	memcpy(addr + XFPGA_NUMBYTES + 64, iv, 24);


	Status = XFpga_PL_BitSream_Load((u32)(addr >> 32),
					(u32)addr, count, ENCRPTED_BITSTREAM);

	if(Status == XFPGA_SUCCESS)
		xil_printf("PL Configuration done successfully");
	else
		xil_printf("PL configuration failed\n\r");

	return 0;
}
