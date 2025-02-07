/******************************************************************************
* Copyright (C) 2003 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
*
* @file xhwicap_low_level_example.c
*
* Contains a design example of how to use the low-level macros and functions
* of the XHwIcap driver.
*
* This example reads back the value stored in the IDCODE register.
*
* This example assumes that there is a UART Device or STDIO Device in the
* hardware system.
*
*
* @note
*
* This example can be run on a 7 series device, Zynq device, Ultrascale and
* ZynqMP Ultrascale FPGAs.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/21/03 First release
* 1.00a sv   07/18/05 Minor changes to comply to Doxygen and coding guidelines
* 1.01a sv   04/10/07 Changes to support V4
* 2.00a sv   10/10/07 Changes to support V5
* 4.00a hvm  11/20/09 Added support for V6 and updated with HAL phase 1
*		      modifications
* 5.00a hvm  02/08/10 Added support for S6
* 5.00a hvm  04/28/10 Added Check for the control bit clearance in the CR
*			register for regsiter read and write operations.
* 5.00a hvm  05/04/10 Updated the example to read Id twice so as to igonore
*			the first read.
* 5.00a hvm  05/21/10 Updated the ID sequence to be consistent across V4/V5/V6
*			devices. Added an extra NOP before Type1 Read device ID
*			and removed  extra NOP after the device ID. This
*			change removed the need of reading ID twice.
*			Removed the extra read ID.
* 5.01a hvm  07/29/10 Code to check whether read/write bit in control register
*			is cleared after the initiation of respective transfer
*			is added for all devices. This check was earlier done
*			only for S6 devices
* 6.00a hvm  08/05/11 Added support for K7 family
* 10.0  bss  6/24/14  Removed support for families older than 7 series
* 11.0  ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 11.6 Nava 06/28/23  Added support for system device-tree flow.
* 11.7 Nava 02/06/25  Updated HWICAP_BASEADDR to use XPAR_HWICAP_0_BASEADDR
*                     instead of XPAR_XHWICAP_0_BASEADDR to align with YAML
*                     changes.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <xparameters.h>
#include <xstatus.h>
#include <xil_types.h>
#include <xil_assert.h>
#include <xhwicap_i.h>
#include <xhwicap_l.h>
#include <stdio.h>
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define HWICAP_BASEADDR		XPAR_HWICAP_0_BASEADDR

/*
 * Number of words to Read for getting Id code.
 */
#define HWICAP_IDCODE_SIZE		    1

/*
 * Bitstream that reads back ID Code Register
 */
#define HWICAP_EXAMPLE_BITSTREAM_LENGTH     6

static u32 ReadId[HWICAP_EXAMPLE_BITSTREAM_LENGTH] = {
	XHI_DUMMY_PACKET, /* Dummy Word */
	XHI_SYNC_PACKET, /* Sync Word*/
	XHI_NOOP_PACKET, /* Type 1 NO OP */
	XHI_NOOP_PACKET, /* Type 1 NO OP */
	XHI_DEVICE_ID_READ, /* Read Product ID Code Register */
	XHI_NOOP_PACKET, /* Type 1 NO OP */
};

#define printf  xil_printf           /* A smaller footprint printf */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 HwIcapLowLevelExample(u32 BaseAddress, u32 *IdCode);


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* Main function to call the HWICAP Low Level example.
*
* @param    None
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note     None
*
******************************************************************************/
int main(void)
{
	int Status;
	u32 IdCode;

	/*
	 * Run the HwIcap Low Level example, specify the Base Address
	 * generated in xparameters.h.
	 */
	Status = HwIcapLowLevelExample(HWICAP_BASEADDR, &IdCode);
	if (Status != XST_SUCCESS) {
		xil_printf("Hwicap lowlevel Example Failed\r\n");
		return XST_FAILURE;
	}

	printf("The IDCODE is %x \r\n", IdCode);
	printf("\r\nSuccessfully ran HwIcapLowLevel Example\r\n\r\n");

	return XST_SUCCESS;

}


/*****************************************************************************/
/**
*
* This function returns the IDCODE of the target device.
*
* @param	BaseAddress is the base address of the HwIcap instance.
* @param	IdCode is the IDCODE of the part this code is running on.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note		None
*
******************************************************************************/
u32 HwIcapLowLevelExample(u32 BaseAddress, u32 *IdCode)
{

	u32 Index;
	u32 Retries;

	/*
	 * Write command sequence to the FIFO
	 */
	for (Index = 0; Index < HWICAP_EXAMPLE_BITSTREAM_LENGTH; Index++) {
		XHwIcap_WriteReg(BaseAddress, XHI_WF_OFFSET, ReadId[Index]);
	}

	/*
	 * Start the transfer of the data from the FIFO to the ICAP device.
	 */
	XHwIcap_WriteReg(BaseAddress, XHI_CR_OFFSET, XHI_CR_WRITE_MASK);

	/*
	 * Poll for done, which indicates end of transfer
	 */
	Retries = 0;
	while ((XHwIcap_ReadReg(BaseAddress, XHI_SR_OFFSET) &
		XHI_SR_DONE_MASK) != XHI_SR_DONE_MASK) {
		Retries++;
		if (Retries > XHI_MAX_RETRIES) {

			/*
			 * Waited to long. Exit with error.
			 */
			printf("\r\nHwIcapLowLevelExample failed- retries  \
			failure. \r\n\r\n");

			return XST_FAILURE;
		}
	}

	/*
	 * Wait till the Write bit is cleared in the CR register.
	 */
	while ((XHwIcap_ReadReg(BaseAddress, XHI_CR_OFFSET)) &
	       XHI_CR_WRITE_MASK);
	/*
	 * Write to the SIZE register. We want to readback one word.
	 */
	XHwIcap_WriteReg(BaseAddress, XHI_SZ_OFFSET, HWICAP_IDCODE_SIZE);


	/*
	 * Start the transfer of the data from ICAP to the FIFO.
	 */
	XHwIcap_WriteReg(BaseAddress, XHI_CR_OFFSET, XHI_CR_READ_MASK);

	/*
	 * Poll for done, which indicates end of transfer
	 */
	Retries = 0;
	while ((XHwIcap_ReadReg(BaseAddress, XHI_SR_OFFSET) &
		XHI_SR_DONE_MASK) != XHI_SR_DONE_MASK) {
		Retries++;
		if (Retries > XHI_MAX_RETRIES) {

			/*
			 * Waited to long. Exit with error.
			 */

			return XST_FAILURE;
		}
	}

	/*
	 * Wait till the Read bit is cleared in the CR register.
	 */
	while ((XHwIcap_ReadReg(BaseAddress, XHI_CR_OFFSET)) &
	       XHI_CR_READ_MASK);
	/*
	 * Return the IDCODE value
	 */
	*IdCode =  XHwIcap_ReadReg(BaseAddress, XHI_RF_OFFSET);

	return XST_SUCCESS;
}

