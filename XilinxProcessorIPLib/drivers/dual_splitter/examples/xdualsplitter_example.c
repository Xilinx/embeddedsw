/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdualsplitter_example.c
*
* This file contains an example of how to use the Dual Splitter driver.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 08/05/14 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xdualsplitter.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/


/**
* The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#define XDUALSPLITTER_DEVICE_ID		XPAR_DUALSPLITTER_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XDualSplitterExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XDualSplitter DualSplitterInst;	/**< Instance of the Dual Splitter core */

/*****************************************************************************/
/**
*
* This is the main function for Dual Splitter example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if Dual Splitter example ran successfully.
*		- XST_FAILURE if Dual Splitter example failed.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run Dual Splitter example */
	Status = XDualSplitterExample(XDUALSPLITTER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Dual Splitter driver example failed.\r\n");
		return XST_FAILURE;
	}

	xil_printf("Dual Splitter driver example ran successfully.\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function demonstrates the use of Dual Splitter driver functions.
*
* @param	DeviceId is the unique device ID of the Dual Splitter core.
*
* @return
*		- XST_SUCCESS if Dual Splitter example ran successfully.
*		- XST_FAILURE if Dual Splitter example failed.
*
* @note		None.
*
******************************************************************************/
int XDualSplitterExample(u16 DeviceId)
{
	int Status;
	u16 Height;
	u16 Width;
	u8 InputSamples ;
	u8 OutputSamples;
	u8 ImageSegments;
	u8 Overlap = 0;
	XDualSplitter_Config *Config;

	/* Initialize the Dual Splitter driver so that it's ready to use
	 * look up the configuration in the config table, then initialize it.
	 */
	Config = XDualSplitter_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XDualSplitter_CfgInitialize(&DualSplitterInst, Config,
						Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the Dual Splitter core */
	XDualSplitter_Enable(&DualSplitterInst);

	/* Disable the register update, program all other registers and at
	 * the end of programming enable register update.
	 */
	XDualSplitter_RegUpdateDisable(&DualSplitterInst);

	/* Set image size width x height */
	Width = 1920;
	Height = 2160;
	XDualSplitter_SetImageSize(&DualSplitterInst, Height, Width);

	/* UHD2@60:
	 *	MST - 4 Streams: InputSamples per clock = 4
	 *			 OutputSamples per clock = 4
	 *			 Number of segments = 4
	 *	MST - 2 Streams: InputSamples per clock = 4
	 *			 OutPutSamples per clock = 4
	 *			 Number of segments = 2
	 * UHD@60:
	 *	SST :	InputSamples per clock = 4
	 *		OutputSamples per clock = 4
	 *		Number of segments = 1
	 * UHD@30:
	 *	SST :	InputSamples per clock = 2
	 *		OutputSamples per clock = 2
	 *		Number of segments = 1
	 */

	/* UHD2@60 in MST mode with 2 streams */
	InputSamples = 4;
	OutputSamples = 4;
	ImageSegments = 2;
	XDualSplitter_SetImgParam(&DualSplitterInst, InputSamples,
			OutputSamples, ImageSegments, Overlap);

	/* Enable the register update */
	XDualSplitter_RegUpdateEnable(&DualSplitterInst);

	return XST_SUCCESS;
}
