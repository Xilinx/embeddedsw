/******************************************************************************
* Copyright (C) 2003 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhwicap_read_frame_polled_example.c
*
* The is example shows how to use the XHwIcap_DeviceReadFrame() to read a
* frame of data. This example simply reads one frame from the device using
* the polled mode.
*
* This example assumes that there is a UART Device or STDIO Device in the
* hardware system.
*
* @note
*
* This example should run on any Virtex4 or Virtex5 or Virtex6 or
* Spartan6 or Kintex 7 device.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/21/03 First release
* 1.00a sv   07/18/05 Minor changes to comply to Doxygen and coding guidelines
* 1.01a sv   04/10/07 Changes to support V4
* 2.00a sv   10/04/07 Changes to support FIFO mode
* 4.00a hvm  11/20/09 Updated to support V6
* 5.00a hvm  2/20/10  Updated to support S6.
* 6.00a hvm  08/05/11 Added support for K7 family
* 10.0  bss  6/24/14  Removed support for families older than 7 series
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <xparameters.h>
#include <xil_types.h>
#include <xil_assert.h>
#include <xhwicap.h>
#include <stdio.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define HWICAP_DEVICEID		XPAR_HWICAP_0_DEVICE_ID

#define HWICAP_EXAMPLE_BLOCK		0

/*
 * These are the parameters for reading a frame of data in
 * the slice SLICE_X0Y0
 */
#define HWICAP_EXAMPLE_TOP		0
#define HWICAP_EXAMPLE_HCLK		5
#define HWICAP_EXAMPLE_MAJOR		5
#define HWICAP_EXAMPLE_MINOR		10

#define printf  xil_printf	/* A smaller footprint printf */



/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int HwIcapReadFramePolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

static XHwIcap HwIcap;

/*****************************************************************************/
/**
*
* Main function to call the HwIcap Frame example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	printf("\r\nHwIcapReadFramePolledExample\r\n");
	/*
	 * Run the HwIcap example, specify the Device ID generated in
	 * xparameters.h
	 */
	Status = HwIcapReadFramePolledExample(HWICAP_DEVICEID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}


/*****************************************************************************/
/**
*
* This function reads a frame from the device as an example using polled mode.
*
* @param	DeviceId is the XPAR_<HWICAP_INSTANCE>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note		None
*
****************************************************************************/
int HwIcapReadFramePolledExample(u16 DeviceId)
{
	int Status;
	u32 Index;
	XHwIcap_Config *CfgPtr;
	u32  FrameData[XHI_NUM_WORDS_FRAME_INCL_NULL_FRAME];

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
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XHwIcap_SelfTest(&HwIcap);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read the Frame
	 */
	Status = XHwIcap_DeviceReadFrame(&HwIcap,
					 HWICAP_EXAMPLE_TOP,
					 HWICAP_EXAMPLE_BLOCK,
					 HWICAP_EXAMPLE_HCLK,
					 HWICAP_EXAMPLE_MAJOR,
					 HWICAP_EXAMPLE_MINOR,
					 (u32 *) &FrameData[0]);
	if (Status != XST_SUCCESS) {
		printf("Failed to Read Frame: %d \r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * Print Frame contents
	 */
	for (Index = HwIcap.WordsPerFrame;
		Index < (HwIcap.WordsPerFrame << 1) ; Index++) {

		printf("Frame Word %d -> \t %x \r\n",
			(Index - HwIcap.WordsPerFrame) , FrameData[Index]);
	}

	printf("\r\nHwIcapReadFramePolledExample Passed Successfully.\r\n\r\n");
	return  XST_SUCCESS;
}

