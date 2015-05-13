/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx_example_common.c
 *
 * Contains a design example using the XDptx driver. It performs a self test on
 * the DisplayPort TX core by training the main link at the maximum common
 * capabilities between the TX and RX and checking the lane status.
 *
 * @note	The DisplayPort TX core does not work alone - video/audio
 *		sources need to be set up in the system correctly, as well as
 *		setting up the output path (for example, configuring the
 *		hardware system with the DisplayPort TX core output to an FMC
 *		card with DisplayPort output capabilities. Some platform
 *		initialization will need to happen prior to calling XDptx driver
 *		functions. See XAPP1178 as a reference.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  06/17/14 Initial creation.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx_example_common.h"
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static void Dptx_StartVideoStream(XDptx *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function will configure and establish a link with the receiver device,
 * afterwards, a video stream will start to be sent over the main link.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LaneCount is the number of lanes to use over the main link.
 * @param	LinkRate is the link rate to use over the main link.
 *
 * @return
 *		- XST_SUCCESS if main link was successfully established.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dptx_Run(XDptx *InstancePtr)
{
	u32 Status;

	/* Configure and establish a link. */
	Status = Dptx_StartLink(InstancePtr);
	if (Status == XST_SUCCESS) {
		/* Start the video stream. */
		Dptx_StartVideoStream(InstancePtr);
	} else {
		xil_printf("<-- Failed to establish/train the link.\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will setup and initialize the DisplayPort TX core. The core's
 * configuration parameters will be retrieved based on the configuration
 * to the DisplayPort TX core instance with the specified device ID.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 *
 * @return
 *		- XST_SUCCESS if the device configuration was found and obtained
 *		  and if the main link was successfully established.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dptx_SetupExample(XDptx *InstancePtr, u16 DeviceId)
{
	XDptx_Config *ConfigPtr;
	u32 Status;

	/* Obtain the device configuration for the DisplayPort TX core. */
	ConfigPtr = XDptx_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the InstancePtr's Config
	 * structure. */
	XDptx_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddr);

	/* Initialize the DisplayPort TX core. */
	Status = XDptx_InitializeTx(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will configure and establish a link with the receiver device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS the if main link was successfully established.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dptx_StartLink(XDptx *InstancePtr)
{
	u32 VsLevelTx;
	u32 PeLevelTx;
	u32 Status;
	u8 LaneCount;
	u8 LinkRate;

	/* Obtain the capabilities of the RX device by reading the monitor's
	 * DPCD. */
	Status = XDptx_GetRxCapabilities(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if (TRAIN_USE_MAX_LINK == 1)
	LaneCount = InstancePtr->LinkConfig.MaxLaneCount;
	LinkRate = InstancePtr->LinkConfig.MaxLinkRate;
#else
	LaneCount = TRAIN_USE_LANE_COUNT;
	LinkRate = TRAIN_USE_LINK_RATE;
#endif

	/* Check if the link is already trained  */
	Status = XDptx_CheckLinkStatus(InstancePtr, LaneCount);
	if (Status == XST_SUCCESS) {
		xil_printf("-> Link is already trained on %d lanes.\n",
								LaneCount);
		if (XDptx_ReadReg(InstancePtr->Config.BaseAddr,
					XDPTX_LINK_BW_SET) == LinkRate) {
			xil_printf("-> Link needs to be re-trained %d Mbps.\n",
							(270 * LinkRate));
		}
		else {
			xil_printf("-> Link is already trained at %d Mbps.\n",
							(270 * LinkRate));
			return XST_SUCCESS;
		}
	}
	else if (Status == XST_FAILURE) {
		xil_printf("-> Needs training.\n");
	}
	else {
		/* Either a connection does not exist or the supplied lane count
		 * is invalid. */
		xil_printf("-> Error checking link status.\n");
		return XST_FAILURE;
	}

	XDptx_SetEnhancedFrameMode(InstancePtr, 1);
	XDptx_SetDownspread(InstancePtr, 0);

#if (TRAIN_USE_MAX_LINK == 1)
	/* Configure the main link based on the maximum common capabilities of
	 * the DisplayPort TX core and the receiver device. */
	Status = XDptx_CfgMainLinkMax(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#else
	XDptx_SetLinkRate(InstancePtr, LinkRate);
	XDptx_SetLaneCount(InstancePtr, LaneCount);
#endif

	/* Train the link. */
	xil_printf("******************************************\n");
	Status = XDptx_EstablishLink(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("!!! Training failed !!!\n");
		xil_printf("******************************************\n");
		return XST_FAILURE;
	}

	VsLevelTx = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
						XDPTX_PHY_VOLTAGE_DIFF_LANE_0);
	PeLevelTx = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
						XDPTX_PHY_POSTCURSOR_LANE_0);
	xil_printf("!!! Training passed at LR:0x%02lx LC:%d !!!\n",
					InstancePtr->LinkConfig.LinkRate,
					InstancePtr->LinkConfig.LaneCount);
	xil_printf("VS:%d (TX:%d) PE:%d (TX:%d)\n",
				InstancePtr->LinkConfig.VsLevel, VsLevelTx,
				InstancePtr->LinkConfig.PeLevel, PeLevelTx);
	xil_printf("******************************************\n");

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will start sending a video stream over the main link. The
 * settings to be used are as follows:
 *	- 8 bits per color.
 *	- Video timing and screen resolution used:
 *	- The connected monitor's preferred timing is used to determine the
 *	  video resolution (and associated timings) for the stream.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return	None.
 *
 * @note	The Dptx_StreamSrc* are intentionally left for the user to
 *		implement since configuration of the stream source is
 *		application-specific.
 * @note	The Extended Display Identification Data (EDID) is read in order
 *		to obtain the video resolution and timings. If this read fails,
 *		a resolution of 640x480 is used at a refresh rate of 60Hz.
 *
*******************************************************************************/
static void Dptx_StartVideoStream(XDptx *InstancePtr)
{
	u32 Status;
	u8 Edid[XDPTX_EDID_BLOCK_SIZE];

	/* Set the bits per color. If not set, the default is 6. */
	XDptx_CfgMsaSetBpc(InstancePtr, XDPTX_STREAM_ID1, 8);

	/* Set synchronous clock mode. */
	XDptx_CfgMsaEnSynchClkMode(InstancePtr, XDPTX_STREAM_ID1, 1);

	XDptx_ClearMsaValues(InstancePtr, XDPTX_STREAM_ID1);
	XDptx_ClearMsaValues(InstancePtr, XDPTX_STREAM_ID2);
	XDptx_ClearMsaValues(InstancePtr, XDPTX_STREAM_ID3);
	XDptx_ClearMsaValues(InstancePtr, XDPTX_STREAM_ID4);

/* Choose a method for selecting the video mode. There are 3 ways to do this:
 * 1) Use the preferred timing from the monitor's EDID:
 *	u8 Edid[XDPTX_EDID_BLOCK_SIZE];
 *	XDptx_GetEdid(InstancePtr, Edid);
 *	XDptx_CfgMsaUseEdidPreferredTiming(InstancePtr, XDPTX_STREAM_ID1, Edid);
 *
 * 2) Use a standard video timing mode (see mode_table.h):
 *	XDptx_CfgMsaUseStandardVideoMode(InstancePtr, XDPTX_STREAM_ID1,
							XVIDC_VM_640x480_60_P);
 *
 * 3) Use a custom configuration for the main stream attributes (MSA):
 *	XDptx_MainStreamAttributes MsaConfigCustom;
 *	MsaConfigCustom.Dmt.HResolution = 1280;
 *	MsaConfigCustom.Dmt.VResolution = 1024;
 *	MsaConfigCustom.Dmt.PixelClkKhz = 108000;
 *	MsaConfigCustom.Dmt.HSyncPolarity = 0;
 *	MsaConfigCustom.Dmt.VSyncPolarity = 0;
 *	MsaConfigCustom.Dmt.HFrontPorch = 48;
 *	MsaConfigCustom.Dmt.HSyncPulseWidth = 112;
 *	MsaConfigCustom.Dmt.HBackPorch = 248;
 *	MsaConfigCustom.Dmt.VFrontPorch = 1;
 *	MsaConfigCustom.Dmt.VSyncPulseWidth = 3;
 *	MsaConfigCustom.Dmt.VBackPorch = 38;
 *	XDptx_CfgMsaUseCustom(InstancePtr, XDPTX_STREAM_ID1,
 *							&MsaConfigCustom, 1);
 *
 * To override the user pixel width:
 *	InstancePtr->MsaConfig[_STREAM#_].OverrideUserPixelWidth = 1;
 *	InstancePtr->MsaConfig[_STREAM#_].UserPixelWidth = _DESIRED_VALUE_;
 *	Then, use one of the methods above to calculate the rest of the MSA.
 */
	Status = XDptx_GetEdid(InstancePtr, Edid);
	if (Status == XST_SUCCESS) {
		XDptx_CfgMsaUseEdidPreferredTiming(InstancePtr,
							XDPTX_STREAM_ID1, Edid);
	}
	else {
		XDptx_CfgMsaUseStandardVideoMode(InstancePtr, XDPTX_STREAM_ID1,
							XVIDC_VM_640x480_60_P);
	}

	/* Disable MST for this example. */
	XDptx_MstDisable(InstancePtr);

	/* Disable main stream to force sending of IDLE patterns. */
	XDptx_DisableMainLink(InstancePtr);

	/* Reset the transmitter. */
	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_SOFT_RESET,
					XDPTX_SOFT_RESET_VIDEO_STREAM_ALL_MASK);
	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_SOFT_RESET, 0x0);

	/* Set the DisplayPort TX video mode. */
	XDptx_SetVideoMode(InstancePtr, XDPTX_STREAM_ID1);

	/* Configure video stream source or generator here. These function need
	 * to be implemented in order for video to be displayed and is hardware
	 * system specific. It is up to the user to implement these
	 * functions. */
	Dptx_StreamSrcSetup(InstancePtr);
	Dptx_StreamSrcConfigure(InstancePtr);
	Dptx_StreamSrcSync(InstancePtr);
	/*********************************/

	XDptx_EnableMainLink(InstancePtr);
}
