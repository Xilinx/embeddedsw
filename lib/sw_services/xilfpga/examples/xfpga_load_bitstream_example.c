/******************************************************************************
 *
 * Copyright (C) 2016-2018 Xilinx, Inc.  All rights reserved.
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
 * @file xfpga_load_bitstream_example.c
 *
 * This file contains the example using Xilfpga library to transfer the user
 * provided Bit-stream into zynqmp pl region.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------  -------- ------------------------------------------------------
 * 1.0   Nava    08/06/16  First release
 * 4.0   Nava	21/02/18  Updated the example relevant to src code changes.
 * 4.2   Nava    30/05/18  Refactor the xilfpga library to support
 *                         different PL programming Interfaces.
 * </pre>
 *
 ******************************************************************************/

#include "xil_printf.h"
#include "xilfpga.h"
#include "xfpga_config.h"

int main(void)
{
	u64 addr = XFPGA_BASE_ADDRESS;
	s32 Status;

	xil_printf("Loading Bit-stream for DDR location :0x%x\n\r",
				XFPGA_BASE_ADDRESS);
	xil_printf("Trying to configure the PL ......\n\r");

	Status = XFpga_PL_BitStream_Load(addr, 0, 0);

	if (Status == XFPGA_SUCCESS)
		xil_printf("PL Configuration done successfully");
	else
		xil_printf("PL configuration failed\n\r");

	return 0;
}
