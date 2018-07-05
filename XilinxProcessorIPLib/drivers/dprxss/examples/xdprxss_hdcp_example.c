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
* @file xdprxss_hdcp_example.c
*
* This file contains a design example using the XDpRxSs driver in single stream
* (SST) transport or multi-stream transport (MST) mode and enables HDCP.
*
* @note		For this example to display output, the user need to implement
*		initialization of the system (DpRxSs_PlatformInit), Video Phy
*		(DpRxSs_VideoPhyInit), start DisplayPort RX subsystem
*		(XDpRxSs_Start) and DisplayPort RX Subsystem setup
*		(DpRxSs_Setup).
*		The input to the Subsystem is from RX (GT).
*		The functions DpRxSs_PlatformInit, DpRxSs_VideoPhyInit and
*		DpRxSs_Setup are declared and are left up to the user to
*		implement.
*		In addition to the setups, this example requires to implement
*		required interrupts to view the output.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 10/05/15 Initial release.
* 4.00 ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#include "string.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the DisplayPort Receiver Subsystem HIP instance
 * to be used
 */
#define XDPRXSS_DEVICE_ID		XPAR_DPRXSS_0_DEVICE_ID

/* Example will run either in MST or SST mode based upon config parameters.
 * In MST mode, this example exposes maximum number of input and output ports
 * of DP RX that will be included while replying to sideband messages from TX.
 */
#define DPRXSS_LINK_RATE		XDPRXSS_LINK_BW_SET_540GBPS
#define DPRXSS_LANE_COUNT		XDPRXSS_LANE_COUNT_SET_4

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DpRxSs_HdcpExample(u16 DeviceId);
u32 DpRxSs_PlatformInit(void);
u32 DpRxSs_VideoPhyInit(void);
u32 DpRxSs_Setup(void);

/************************** Variable Definitions *****************************/

XDpRxSs DpRxSsInst;	/* The DPRX Subsystem instance.*/

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs SST/MST example.
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
	xil_printf("DisplayPort RX Subsystem HDCP example\n\r");
	xil_printf("(c) 2015 by Xilinx\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

	Status = DpRxSs_HdcpExample(XDPRXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort RX Subsystem HDCP example failed."
				"\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DisplayPort RX Subsystem HDCP example\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the HDCP example using the
* XDpRxSs driver. This function will enable HDCP during DisplayPort RX
* Subsystem set up to work in MST/SST mode.
*
* @param	DeviceId is the unique device ID of the DisplayPort RX
*		Subsystem core.
*
* @return
*		- XST_SUCCESS if DisplayPort RX Subsystem HDCP enabled
*		successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_HdcpExample(u16 DeviceId)
{
	u32 Status;
	XDpRxSs_Config *ConfigPtr;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpRxSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
	ConfigPtr = XDpRxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the DpRxSsInst's Config
	 * structure. */
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPRXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpRxSsInst.UsrOpt.MstSupport) {
		xil_printf("\n\rINFO:DPRXSS is MST enabled. DPRXSS can be "
			"switched to SST/MST\n\r\n\r");
	}
	else {
		xil_printf("\n\rINFO:DPRXSS is  SST enabled. DPRXSS works "
			"only in SST mode.\n\r\n\r");
	}


	/* Set Link rate and lane count to maximum */
	XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	/* Enable HDCP */
	Status = XDpRxSs_HdcpEnable(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS HDCP enable failed\n\r");
		return XST_FAILURE;
	}

	/* Set PHY status down */
	Status = XDpRxSs_SetPhysicalState(&DpRxSsInst, 0);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS HDCP PHY failed\n\r");
		return XST_FAILURE;
	}

	/* Execute HDCP RX state machine */
	Status = XDpRxSs_Poll(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS HDCP poll failed\n\r");
		return XST_FAILURE;
	}
#endif

	/* Start DPRXSS */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return XST_FAILURE;
	}

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	/* Set PHY status up */
	Status = XDpRxSs_SetPhysicalState(&DpRxSsInst, 1);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS HDCP PHY failed\n\r");
		return XST_FAILURE;
	}

	/* Set authenticate */
	Status = XDpRxSs_Authenticate(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS HDCP AUTH failed\n\r");
		return XST_FAILURE;
	}

	/* Execute HDCP RX state machine */
	Status = XDpRxSs_Poll(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS HDCP poll failed\n\r");
		return XST_FAILURE;
	}
#endif

	/* Setup Video Phy, left to the user for implementation */
	DpRxSs_VideoPhyInit();

	/* Setup DPRX SS, left to the user for implementation */
	DpRxSs_Setup();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required platform specific peripherals.
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
u32 DpRxSs_PlatformInit(void)
{
	/* User is responsible to setup platform specific initialization */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures Video Phy.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if Video Phy configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_VideoPhyInit(void)
{
	/* User is responsible to setup Video Phy */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures DisplayPort RX Subsystem.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if DPRX Subsystem configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 DpRxSs_Setup(void)
{
	/* User is responsible to setup Video Phy */

	return XST_SUCCESS;
}
