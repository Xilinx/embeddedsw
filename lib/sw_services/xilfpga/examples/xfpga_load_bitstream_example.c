/******************************************************************************
 *
 * Copyright (C) 2016-2019 Xilinx, Inc.  All rights reserved.
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
 * provided Bitstream into zynqmp pl region.
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
 * 4.2	 adk	 23/08/18  Added bitstream size define.
 * 5.0   Nava	 06/02/19  Updated the example to sync with 5.0 version API's
 * </pre>
 *
 ******************************************************************************/

#include "xil_printf.h"
#include "xilfpga.h"
#include "xfpga_config.h"

/**************************** Type Definitions *******************************/
/* Xilfpga library supports vivado generated Bitstream(*.bit, *.bin) and bootgen
 * generated Bitstream(*.bin), Passing below definition is mandatory for vivado
 * generated Bitstream, For bootgen generated Bitstreams Xilfpga will take
 * Bitstream size from Bitstream Header.
 *
 * Below definition is for typical bitstream size of zcu102 board
 * User should replace the below definition value with the actual bitstream size.
 *
 * @note: This example supports only Zynq UltraScale+ MPSoC platform.
 */
#define BITSTREAM_SIZE	0x1A000000
/*****************************************************************************/
int main(void)
{
	u64 addr = XFPGA_BASE_ADDRESS;
	XFpga XFpgaInstance = {0U};
	s32 Status;

	xil_printf("Loading Bitstream for DDR location :0x%x\n\r",
				XFPGA_BASE_ADDRESS);
	xil_printf("Trying to configure the PL ......\n\r");

	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XFpga_PL_BitStream_Load(&XFpgaInstance, addr,
					 BITSTREAM_SIZE, XFPGA_FULLBIT_EN);

 done:
	if (Status == XFPGA_SUCCESS)
		xil_printf("PL Configuration done successfully");
	else
		xil_printf("PL configuration failed\n\r");

	return 0;
}
