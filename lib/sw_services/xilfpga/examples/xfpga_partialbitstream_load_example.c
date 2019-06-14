/******************************************************************************
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xfpga_partialbitstream_load_example.c
 *
 * Partial reconfiguration(PR) is the ability for a portion of an FPGA to be
 * reprogrammed while the remainder of the system stays unchanged. Dynamic PR
 * allows device reconfiguration during runtime while rest of the Device is
 * still functioning.
 *
 * This file contains the example using xilfpga library to transfer the user
 * provided Partial Reconfiguration Bitstream into ZynqMP PL region.
 * Before loading this example please make sure static Bitstream associated with
 * the PR design has been loaded into the PL.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------  -------- ------------------------------------------------------
 * 4.2   adk     03/08/18  Initial Release.
 * 4.2	 adk	 23/08/18  Added bitstream size define.
 * 5.0   Nava	 06/02/19  Updated the example to sync with 5.0 version API's
 * </pre>
 *
 ******************************************************************************/

#include "xil_printf.h"
#include "xilfpga.h"
#include "xfpga_config.h"

/**************************** Type Definitions *******************************/
/* Xilfpga library supports vivado generated Partial Bitstream(*.bit) and
 * bootgen generated Partial Bitstream(*.bin), Passing below definition is
 * mandatory for vivado generated Partial Bitstream, For bootgen generated
 * Partial Bitstream Xilfpga will take Bitstream size from the Bitstream Header.
 *
 * Below definition is for typical Partial Bitstream size of zcu102 board
 * User should replace the below definition value with the actual
 * Partial Bitstream size.
 *
 * @note: This example supports only Zynq UltraScale+ MPSoC platform.
 */
#define BITSTREAM_SIZE	0x1000000
/*****************************************************************************/
int main(void)
{
	u64 addr = XFPGA_BASE_ADDRESS;
	XFpga XFpgaInstance = {0U};
	s32 Status;

	xil_printf("Loading Partial Reconfiguration Bitstream from DDR location :0x%x\n\r",
		   XFPGA_BASE_ADDRESS);
	xil_printf("Trying to configure Partial Reconfiguration Bitstream into the PL ......\n\r");

	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XFpga_PL_BitStream_Load(&XFpgaInstance, addr,
					 BITSTREAM_SIZE, XFPGA_PARTIAL_EN);

 done:
	if (Status == XFPGA_SUCCESS)
		xil_printf("Partial Reconfiguration Bitstream loaded into the PL successfully");
	else
		xil_printf("Partial Reconfiguration Bitstream loading into the PL failed\n\r");

	return 0;
}
