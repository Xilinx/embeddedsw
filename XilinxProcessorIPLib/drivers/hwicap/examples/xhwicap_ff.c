/******************************************************************************
*
* Copyright (C) 2003 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xhwicap_ff.c
*
* This example demonstrates how to set and reset FF values. It sets and
* resets the FFX in SLICE_X0Y0. Make sure this FF is not being used in
* the design. This can be done by adding the following line in the UCF
* file.
*
* CONFIG PROHIBIT = SLICE_X0Y0;
*
* This example assumes that there is a UART Device or STDIO Device in the
* hardware system.
*
* @note
*
* This example should run on any or a Virtex4 device.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  12/02/03 First release
* 1.00a bjs  03/08/04 Updated for EDK6.2.1
* 1.00a sv   07/18/05 Minor changes to comply to Doxygen and coding guidelines
* 4.00a hvm  12/1/09  Updated with HAL phase 1 changes
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <xparameters.h>
#include <xstatus.h>
#include <xhwicap.h>
#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex 4 */
#include <xhwicap_clb_ff.h>
#include <xhwicap_clb_srinv.h>
#endif
#include <stdio.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define HWICAP_DEVICEID		XPAR_HWICAP_0_DEVICE_ID


/*
 * Row and column to test
 */
#define HWICAP_EXAMPLE_TEST_COL		0
#define HWICAP_EXAMPLE_TEST_ROW		0

#define printf      xil_printf     /* A smaller footprint print */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int HwIcapBramFfExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

static XHwIcap HwIcap;

/*****************************************************************************/
/**
* Main function to call the HwIcap BRAM-FF example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex 4 */
int main(void)
{
	int Status;

	/*
	 * Run the HwIcap BRAM-FF example, specify the Device ID generated in
	 * xparameters.h
	 */
	Status = HwIcapBramFfExample(HWICAP_DEVICEID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a test on the BRAM FF as an  example.
*
* @param	DeviceId is the XPAR_<HWICAP_INSTANCE>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note		None
*
****************************************************************************/
int HwIcapBramFfExample(u16 DeviceId)
{
	u32 SrMode=0;
	int Status;
	u32 Row;
	u32 Col;
	u32 Slice;
	u32 Loop;
	u32 Value;
	u8 Contents[1];
	XHwIcap_Config *CfgPtr;

	/*
	 * Initialize the HwIcap instance.
	 */
	CfgPtr = XHwIcap_LookupConfig(DeviceId);
	if (CfgPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XHwIcap_CfgInitialize(&HwIcap, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Identify the FF to change: FFX in SLICE_X0Y0.
	 */
	Col = XHwIcap_SliceX2Col(HWICAP_EXAMPLE_TEST_COL);
	Row = XHwIcap_SliceY2Row(&HwIcap, HWICAP_EXAMPLE_TEST_ROW);
	Slice = XHwIcap_SliceXY2Slice(HWICAP_EXAMPLE_TEST_COL,
						HWICAP_EXAMPLE_TEST_ROW);

	printf("Setting the SRMODE -> SRHIGH. \r\n");
	Status = XHwIcap_SetClbBits(&HwIcap, Row, Col,
		XHI_CLB_FF.SRMODE[Slice][XHI_CLB_XQ], XHI_CLB_FF.SRHIGH, 1);
	if (Status != XST_SUCCESS) {
		printf("Failed to Set SRMODE->SRHIGH: %d \r\n", Status);
		return XST_FAILURE;
	}

	printf("Setting the SRMODE -> SRLOW. \r\n");
	Status = XHwIcap_SetClbBits(&HwIcap, Row, Col,
	XHI_CLB_FF.SRMODE[Slice][XHI_CLB_XQ], XHI_CLB_FF.SRLOW, 1);
	if (Status != XST_SUCCESS) {
		printf("Failed to Set SRMODE->SRLOW: %d \r\n", Status);
		return XST_FAILURE;
	}


	printf("Set SRINV to SR \r\n");
	Status = XHwIcap_SetClbBits(&HwIcap, Row, Col,
		 XHI_CLB_SRINV.RES[Slice], XHI_CLB_SRINV.SR, 1);
	if (Status != XST_SUCCESS) {
		printf("Failed to Set SRINV->SR: %d \r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * Set it back
	 */
	printf("Set SRINV to SR_B \r\n");
	Status = XHwIcap_SetClbBits(&HwIcap, Row, Col,
		 XHI_CLB_SRINV.RES[Slice], XHI_CLB_SRINV.SR_B, 1);
	if (Status != XST_SUCCESS) {
		printf("Failed to Set SRINV->SR_B: %d \r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * Capture the FF states. If the CAPTURE block is instantiated in
	 * the design then the XHwIcap_CommandCapture() is not necessary.
	 */
	printf("Capture the FF state. \r\n");
	Status = XHwIcap_CommandCapture(&HwIcap);
	if (Status != XST_SUCCESS) {
		printf("Failed to capture FF states: %d \r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * Read the FF Contents
	 */
	Status = XHwIcap_GetClbBits(&HwIcap, Row, Col,
					XHI_CLB_FF.CONTENTS[Slice][XHI_CLB_XQ],
					Contents, 1);
	if (Status != XST_SUCCESS) {
		printf("Failed to Get FF Contents: %d \r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * The readback value of the FF is inverted from its true value.
	 */
	Value = ~Contents[0] & 0x1;
	printf("FF Contents: %d \r\n", Value);


	printf("HwicapBramFFExample Passed Successfully ... \r\n\r\n");
	return XST_SUCCESS;
}
#else
#error Unsupported FPGA Family
#endif

