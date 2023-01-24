/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
 * 4.2   adk     08/03/18  Initial Release.
 * 4.2	 adk	 08/23/18  Added bitstream size define.
 * 5.0   Nava	 02/06/19  Updated the example to sync with 5.0 version API's
 * 5.2   Nava    02/14/20  Removed unwanted header file inclusion.
 * 5.2   Nava    02/27/20  Added support for Versal Platform.
 * 5.3   Nava    06/16/20  Modified the date format from dd/mm to mm/dd.
 * 6.0   Nava    12/14/20  In XFpga_PL_BitStream_Load() API the argument
 *                         AddrPtr_Size is being used for multiple purposes.
 *                         Use of the same variable for multiple purposes can
 *                         make it more difficult for a person to read (or)
 *                         understand the code and also it leads to a safety
 *                         violation. fixes this  issue by adding a separate
 *                         function arguments to read KeyAddr and
 *                         Size(Bitstream size).
 * 6.1   Nava    09/13/21  Fixed compilation warning.
 *
 * </pre>
 *
 ******************************************************************************/

#include "xilfpga.h"
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
 * @note: This example supports only Zynq UltraScale+ MPSoC and Versal platform.
 */

/* For Versal platform Passing the below definition is Optional */
#define BITSTREAM_SIZE	0x1000000U /* Bin or bit or PDI image size */
#ifdef versal
#define PDI_LOAD        0U
#endif
/*****************************************************************************/
int main(void)
{
	u64 addr = XFPGA_BASE_ADDRESS;
	XFpga XFpgaInstance = {0U};
	UINTPTR KeyAddr = (UINTPTR)NULL;
	s32 Status;

	xil_printf("Loading Partial Reconfiguration Bitstream from DDR location :0x%x\n\r",
		   XFPGA_BASE_ADDRESS);
	xil_printf("Trying to configure Partial Reconfiguration Bitstream into the PL ......\n\r");

	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XST_SUCCESS) {
		goto done;
	}
#ifdef versal
	Status = XFpga_BitStream_Load(&XFpgaInstance, addr, KeyAddr,
				      BITSTREAM_SIZE, PDI_LOAD);
#else
	Status = XFpga_BitStream_Load(&XFpgaInstance, addr, KeyAddr,
				      BITSTREAM_SIZE, XFPGA_PARTIAL_EN);
#endif

 done:
	if (Status == XFPGA_SUCCESS)
		xil_printf("Partial Reconfiguration Bitstream loaded into the PL successfully");
	else
		xil_printf("Partial Reconfiguration Bitstream loading into the PL failed\n\r");

	return 0;
}
