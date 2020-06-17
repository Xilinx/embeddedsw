/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
 * 1.0   Nava    06/08/16  First release
 * 4.0   Nava	 02/21/18  Updated the example relevant to src code changes.
 * 4.2   Nava    05/30/18  Refactor the xilfpga library to support
 *                         different PL programming Interfaces.
 * 4.2	 adk	 08/23/18  Added bitstream size define.
 * 5.0   Nava	 02/06/19  Updated the example to sync with 5.0 version API's
 * 5.0	 Nava 	 03/16/19  Typical bitstram size of zcu102 board is 26MB.So
 *			   updated the bitstream size macro value for the same.
 * 5.2   Nava	 02/14/20  Removed unwanted header file inclusion.
 * 5.3   Nava    06/16/20  Modified the date format from dd/mm to mm/dd.
 * </pre>
 *
 ******************************************************************************/

#include "xilfpga.h"

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
#define BITSTREAM_SIZE	0x1A00000
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
