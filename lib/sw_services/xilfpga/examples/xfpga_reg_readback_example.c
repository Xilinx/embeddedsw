/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
 * @file  xfpga_reg_readback_example.c
 *
 *
 * This example prints out the values of all the configuration registers in the
 * FPGA.
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
 * 2.1   Nava   10/05/17  First Release
 * 4.2   Nava   05/30/18  Refactor the xilfpga library to support
 *                        different PL programming Interfaces.
 * 5.0 	 Nava   02/06/19  Updated the example to sync with 5.0 version API's
 * 5.0   rama   04/03/19  Fixed IAR compiler warning.
 * 5.2   Nava   12/18/19  Fix for security violation in the readback path.
 * 5.2   Nava   02/14/20  Removed unwanted header file inclusion.
 * 5.3   Nava   06/16/20  Modified the date format from dd/mm to mm/dd.
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xilfpga.h"

/************************** Constant Definitions *****************************/

/*
 * Mask For IDCODE
 */
#define IDCODE_MASK   0x0FFFFFFF

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int Xfpga_RegReadExample(XFpga *InstancePtr);
/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 *
 * Main function to call the Xfpga Reg Read example.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if successful
 *		- XST_FAILURE if unsuccessful
 *
 * @note: This example supports only Zynq UltraScale+ MPSoC platform.
 *
 *****************************************************************************/
int main(void)
{
	int Status;
	XFpga XFpgaInstance = {0U};

	xil_printf("Register Read back example\r\n");

	/*
	 * Call the example , specify the device ID that is generated in
	 * xparameters.h.
	 */
	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = Xfpga_RegReadExample(&XFpgaInstance);

 done:
	if (Status != XST_SUCCESS) {
		xil_printf("Register Read back example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Register Read back example\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function reads the configuration registers inside the FPGA.
 *
 * @param	InstancePtr Pointer to the XFpga structure.
 *
 * @return
 *		- XST_SUCCESS if successful
 *		- XST_FAILURE if unsuccessful
 *
 * @note		None.
 *
 *****************************************************************************/
static int Xfpga_RegReadExample(XFpga *InstancePtr)
{
	u32 CfgReg;

	xil_printf("Value of the Configuration Registers. \r\n\n");

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, CRC) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CRC -> \t %x \t\r\n", CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, FAR1) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" FAR -> \t %x \t\r\n", CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, FDRI) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" FDRI -> \t %x \t\r\n", CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, FDRO) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" FDRO -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, CMD) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CMD -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, CTL0) !=	XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CTL0 -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, MASK) !=	XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" MASK -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, STAT) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" STAT -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, LOUT) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" LOUT -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, COR0) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" COR0 -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, MFWR) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" MFWR -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, CBC) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CBC -> \t %x \t\r\n",  CfgReg);


	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, IDCODE) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" IDCODE -> \t %x \t\r\n",  CfgReg & IDCODE_MASK);


	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, AXSS) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" AXSS -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, COR1) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" COR1 -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, WBSTAR) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" WBSTAR -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, TIMER) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" TIMER -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, BOOTSTS) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" BOOTSTS -> \t %x \t\r\n",  CfgReg);

	if (XFpga_GetPlConfigReg(InstancePtr, (UINTPTR)&CfgReg, CTL1) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf(" CTL1 -> \t %x \t\r\n",  CfgReg);

	return XST_SUCCESS;
}
