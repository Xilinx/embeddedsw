/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file  xfpga_readback_example.c
 *
 *
 * This example prints out the fpga configuration data.
 *
 * This example assumes that there is a UART Device or STDIO Device in the
 * hardware system.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 4.2   adk  07/11/18 First Release
 * 4.2   Nava 08/16/18 Modified the PL data handling Logic to support
 *                     different PL programming interfaces.
 * 5.0   Nava 02/06/19 Updated the example to sync with 5.0 version API's
 * 5.0   rama 04/03/19 Fixed IAR compiler warning
 * 5.2   Nava 12/18/19 Fix for security violation in the readback path.
 * 5.2   Nava 02/14/20 Removed unwanted header file inclusion.
 * 5.3   Nava    06/16/20  Modified the date format from dd/mm to mm/dd.
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xilfpga.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/* Below definitions are for zcu102 board
 * IDCODE -- 0x484A093 DeviceId -- XCKU9P.
 * For more details refer ug570 Table 1-4.
 */
#define FRAMES		71261
#define WORDS_PER_FRAME 93
#define PAD_FRAMES	25

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XFpga_ReadExample(XFpga *InstancePtr);
void PrintBitStream(u32 NumFrames);
/************************** Variable Definitions *****************************/
u32 readback_buffer[WORDS_PER_FRAME*FRAMES + PAD_FRAMES];

/*****************************************************************************/
/**
 *
 * Main function to call the XFpga Read example.
 * This example performs the read back of PL configuration data frames.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if successful
 *		- XST_FAILURE if unsuccessful
 *
 * @note: This example supports only Zynq UltraScale+ MPSoC platform.
 *****************************************************************************/
int main(void)
{
	int Status;
	XFpga XFpgaInstance = {0U};

	xil_printf("FPGA Configuration data Read back example\r\n");

	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XFpga_ReadExample(&XFpgaInstance);

 done:
	if (Status != XST_SUCCESS) {
		xil_printf("FPGA Configuration Read back example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran FPGA Configuration Read back example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function reads the fpga configuration data.
 *
 * @param	DeviceId is the unique device id of the device.
 *
 * @return
 *		- XST_SUCCESS if successful
 *		- XST_FAILURE if unsuccessful
 *
 * @note		None.
 *****************************************************************************/
static int XFpga_ReadExample(XFpga *InstancePtr)
{
	u32 Status = XST_SUCCESS;
	u32 NumFrames = WORDS_PER_FRAME * FRAMES + PAD_FRAMES;

	Status = XFpga_GetPlConfigData(InstancePtr,
				       (UINTPTR)readback_buffer, NumFrames);
	if (Status != XST_SUCCESS) {
		xil_printf("FPGA Configuration Read back Failed\r\n");
		return Status;
	}

	PrintBitStream(NumFrames);
	return Status;
}

void PrintBitStream(u32 NumFrames)
{
	u32 i;

	xil_printf("Bitstream contents are\r\n");

	for (i = 0; i < NumFrames; i+=4) {
		xil_printf("%04x %04x %04x %04x %04x %04x %04x %04x\n\r",
		            (readback_buffer[i] >> 16), (readback_buffer[i] & 0xFFFF),
					(readback_buffer[i+1] >> 16), (readback_buffer[i+1] & 0xFFFF),
		            (readback_buffer[i+2] >> 16), (readback_buffer[i+2] & 0xFFFF),
					(readback_buffer[i+3] >> 16), (readback_buffer[i+3] & 0xFFFF));
	}
}
