/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xdptxss_hdcp_example.c
*
* This file contains a design example using the XDpTxSs driver in single stream
* (SST) transport or multi-stream transport (MST) mode and enables HDCP.
*
* @note		For this example to display output, the user need to implement
*		initialization of the system (DpTxSs_PlatformInit) and after
*		DisplayPort TX subsystem start (XDpTxSs_Start) is complete,
*		implement configuration of the video stream source in order to
*		provide the DisplayPort TX Subsystem HIP input.
*		The functions DpTxSs_PlatformInit and DpTxSs_StreamSrc are
*		declared and are left up to the user implement.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 2.00 sha 09/28/15 Initial release.
* 4.1  ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include "string.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the DisplayPort Transmitter Subsystem HIP instance
 * to be used
 */
#define XDPTXSS_DEVICE_ID		XPAR_DPTXSS_0_DEVICE_ID

/* If set to 1, example will run in MST mode. Otherwise, in SST mode.
 * In MST mode, this example reads the EDID of RX devices if connected in
 * daisy-chain.
 */
#define DPTXSS_MST			1
#define DPTXSS_LINK_RATE		XDPTXSS_LINK_BW_SET_540GBPS
#define DPTXSS_LANE_COUNT		XDPTXSS_LANE_COUNT_SET_4

/* The video resolution from the display mode timings (DMT) table to use for
 * DisplayPort TX Subsystem. It can be set to use preferred video mode for
 * EDID of RX device.
 */
#define DPTXSS_VID_MODE			XVIDC_VM_USE_EDID_PREFERRED

/* The color depth (bits per color component) to use DisplayPort TX
 * Subsystem.
 */
#define DPTXSS_BPC			8

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DpTxSs_HdcpExample(u16 DeviceId);
u32 DpTxSs_PlatformInit(void);
u32 DpTxSs_StreamSrc(u8 VerticalSplit);

/************************** Variable Definitions *****************************/

XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This is the main function for XDpTxSs SST/MST example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the MST/SST example passed.
*		- XST_FAILURE if the MST/SST example was unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main()
{
	u32 Status;

	xil_printf("-------------------------------------------\n\r");
	xil_printf("DisplayPort TX Subsystem HDCP example\n\r");
	xil_printf("(c) 2015 by Xilinx\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

	Status = DpTxSs_HdcpExample(XDPTXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort TX Subsystem HDCP example failed."
				"\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DisplayPort TX Subsystem HDCP example\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the HDCP example using the
* XDpTxSs driver. This function will enable HDCP during DisplayPort TX
* Subsystem set up to work in MST/SST mode.
*
* @param	DeviceId is the unique device ID of the DisplayPort TX
*		Subsystem core.
*
* @return
*		- XST_SUCCESS if DisplayPort TX Subsystem HDCP enabled
*		successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_HdcpExample(u16 DeviceId)
{
	u32 Status;
	u8 VSplitMode = 0;
	XDpTxSs_Config *ConfigPtr;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpTxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

	/* Obtain the device configuration for the DisplayPort TX Subsystem */
	ConfigPtr = XDpTxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the DpTxSsInst's Config
	 * structure. */
	Status = XDpTxSs_CfgInitialize(&DpTxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPTXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpTxSsInst.UsrOpt.MstSupport) {
		xil_printf("\n\rINFO:DPTXSS is MST enabled. DPTXSS can be "
			"switched to SST/MST\n\r\n\r");
	}
	else {
		xil_printf("\n\rINFO:DPTXSS is  SST enabled. DPTXSS works "
			"only in SST mode.\n\r\n\r");
	}

	/* Read capabilities of RX device */
	Status = XDpTxSs_GetRxCapabilities(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: RX device is not connected.\r\n");

		/* Enable interrupts. */
		Xil_ExceptionEnable();

		return XST_FAILURE;
	}

	/* Set Link rate and lane count to maximum */
	XDpTxSs_SetLinkRate(&DpTxSsInst, DPTXSS_LINK_RATE);
	XDpTxSs_SetLaneCount(&DpTxSsInst, DPTXSS_LANE_COUNT);

	/* Set video mode */
	XDpTxSs_SetVidMode(&DpTxSsInst, DPTXSS_VID_MODE);

	/* Set BPC */
	XDpTxSs_SetBpc(&DpTxSsInst, DPTXSS_BPC);

	/* Check whether DPTXSS and RX device is in MST */
	Status = XDpTxSs_IsMstCapable(&DpTxSsInst);
	if (DpTxSsInst.UsrOpt.MstSupport && DPTXSS_MST &&
		(Status == XST_SUCCESS)) {
		/* Set DPTXSS in MST mode */
		XDpTxSs_SetTransportMode(&DpTxSsInst, 1);
	}
	else {
		/* Set DPTXSS in SST mode */
		XDpTxSs_SetTransportMode(&DpTxSsInst, 0);
	}

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	/* Enable HDCP */
	Status = XDpTxSs_HdcpEnable(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPTX SS HDCP enable failed\n\r");
		return XST_FAILURE;
	}

	/* Execute HDCP TX state machine */
	Status = XDpTxSs_Poll(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPTX SS HDCP poll failed\n\r");
		return XST_FAILURE;
	}
#endif

	/* Start DPTXSS parameters set */
	Status = XDpTxSs_Start(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPTX SS start failed\n\r");

		/* Enable interrupts. */
		Xil_ExceptionEnable();

		return XST_FAILURE;
	}

	if ((DpTxSsInst.UsrOpt.VmId == (XVIDC_VM_UHD2_60_P)) &&
				(DpTxSsInst.UsrOpt.MstSupport)) {
		VSplitMode = 1;
	}
	else {
		VSplitMode = 0;
	}

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	/* Set authenticate */
	Status = XDpTxSs_Authenticate(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPTX SS HDCP AUTH failed\n\r");
		return XST_FAILURE;
	}

	/* Enable encryption for stream 1 */
	XDpTxSs_EnableEncryption(&DpTxSsInst, 0);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPTX SS HDCP encrypt failed\n\r");
		return XST_FAILURE;
	}

	/* Execute HDCP TX state machine */
	Status = XDpTxSs_Poll(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPTX SS HDCP poll failed\n\r");
		return XST_FAILURE;
	}
#endif

	/* Do stream setup in this function. It is up to the user to implement
	* this function.
	 */
	DpTxSs_StreamSrc(VSplitMode);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required platform-specifc peripherals.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if required peripherals are initialized and
*		configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_PlatformInit(void)
{
	/* User is responsible to setup platform specific initialization */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function setup stream source to input DisplayPort TX Subsystem.
*
* @param	VerticalSplit specifies whether to split video frame
*		vertically into two different vertical halves.
*		- 1 = Vertically split input frame
*		- 0 = No vertically split input frame.
*
* @return
*		- XST_SUCCESS if stream source is configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpTxSs_StreamSrc(u8 VerticalSplit)
{
	/* User is responsible to setup stream source to input DPTXSS */

	return XST_SUCCESS;
}
