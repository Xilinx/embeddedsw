/*******************************************************************************
 *
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
 * @file xdp_tx_example_common.c
 *
 * Contains a design example using the XDp driver (operating in TX mode). It
 * performs a self test on the DisplayPort TX core by training the main link at
 * the maximum common capabilities between the TX and RX and checking the lane
 * status.
 *
 * @note	The DisplayPort TX core does not work alone - video/audio
 *		sources need to be set up in the system correctly, as well as
 *		setting up the output path (for example, configuring the
 *		hardware system with the DisplayPort TX core output to an FMC
 *		card with DisplayPort output capabilities. Some platform
 *		initialization will need to happen prior to calling XDp driver
 *		functions. See XAPP1178 as a reference.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial creation.
 * 5.1   ms   01/23/17 Added xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp_tx_example_common.h"
#include "xil_printf.h"

/**************************** Function Prototypes *****************************/

static void Dptx_StartVideoStream(XDp *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function will configure and establish a link with the receiver device,
 * afterwards, a video stream will start to be sent over the main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 Dptx_Run(XDp *InstancePtr)
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

	xil_printf("Successfully ran dp_tx_common Example\r\n");
	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will setup and initialize the DisplayPort TX core. The core's
 * configuration parameters will be retrieved based on the configuration
 * to the DisplayPort TX core instance with the specified device ID.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 Dptx_SetupExample(XDp *InstancePtr, u16 DeviceId)
{
	XDp_Config *ConfigPtr;
	u32 Status;

	/* Obtain the device configuration for the DisplayPort TX core. */
	ConfigPtr = XDp_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the InstancePtr's Config
	 * structure. */
	XDp_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddr);

	/* Initialize the DisplayPort TX core. */
	Status = XDp_Initialize(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will configure and establish a link with the receiver device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS the if main link was successfully established.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dptx_StartLink(XDp *InstancePtr)
{
	u32 VsLevelTx;
	u32 PeLevelTx;
	u32 Status;
	u8 LaneCount;
	u8 LinkRate;

	/* Obtain the capabilities of the RX device by reading the monitor's
	 * DPCD. */
	Status = XDp_TxGetRxCapabilities(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if (TRAIN_USE_MAX_LINK == 1)
	LaneCount = InstancePtr->TxInstance.LinkConfig.MaxLaneCount;
	LinkRate = InstancePtr->TxInstance.LinkConfig.MaxLinkRate;
#else
	LaneCount = TRAIN_USE_LANE_COUNT;
	LinkRate = TRAIN_USE_LINK_RATE;
#endif

	/* Check if the link is already trained  */
	Status = XDp_TxCheckLinkStatus(InstancePtr, LaneCount);
	if (Status == XST_SUCCESS) {
		xil_printf("-> Link is already trained on %d lanes.\n",
								LaneCount);
		if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
					XDP_TX_LINK_BW_SET) == LinkRate) {
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

	XDp_TxSetEnhancedFrameMode(InstancePtr, 1);
	XDp_TxSetDownspread(InstancePtr, 0);

#if (TRAIN_USE_MAX_LINK == 1)
	/* Configure the main link based on the maximum common capabilities of
	 * the DisplayPort TX core and the receiver device. */
	Status = XDp_TxCfgMainLinkMax(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#else
	XDp_TxSetLinkRate(InstancePtr, LinkRate);
	XDp_TxSetLaneCount(InstancePtr, LaneCount);
#endif

	/* Train the link. */
	xil_printf("******************************************\n");
	Status = XDp_TxEstablishLink(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("!!! Training failed !!!\n");
		xil_printf("******************************************\n");
		return XST_FAILURE;
	}

	VsLevelTx = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_PHY_VOLTAGE_DIFF_LANE_0);
	PeLevelTx = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_PHY_POSTCURSOR_LANE_0);
	xil_printf("!!! Training passed at LR:0x%02lx LC:%d !!!\n",
				InstancePtr->TxInstance.LinkConfig.LinkRate,
				InstancePtr->TxInstance.LinkConfig.LaneCount);
	xil_printf("VS:%d (TX:%d) PE:%d (TX:%d)\n",
			InstancePtr->TxInstance.LinkConfig.VsLevel, VsLevelTx,
			InstancePtr->TxInstance.LinkConfig.PeLevel, PeLevelTx);
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
 * @param	InstancePtr is a pointer to the XDp instance.
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
static void Dptx_StartVideoStream(XDp *InstancePtr)
{
	u32 Status;
	u8 Edid[XDP_EDID_BLOCK_SIZE];

	/* Set the bits per color. If not set, the default is 6. */
	XDp_TxCfgMsaSetBpc(InstancePtr, XDP_TX_STREAM_ID1, 8);

	/* Set synchronous clock mode. */
	XDp_TxCfgMsaEnSynchClkMode(InstancePtr, XDP_TX_STREAM_ID1, 1);

	XDp_TxClearMsaValues(InstancePtr, XDP_TX_STREAM_ID1);
	XDp_TxClearMsaValues(InstancePtr, XDP_TX_STREAM_ID2);
	XDp_TxClearMsaValues(InstancePtr, XDP_TX_STREAM_ID3);
	XDp_TxClearMsaValues(InstancePtr, XDP_TX_STREAM_ID4);

/* Choose a method for selecting the video mode. There are 3 ways to do this:
 * 1) Use the preferred timing from the monitor's EDID:
 *	u8 Edid[XDP_EDID_BLOCK_SIZE];
 *	XDp_TxGetEdid(InstancePtr, Edid);
 *	XDp_TxCfgMsaUseEdidPreferredTiming(InstancePtr, XDP_TX_STREAM_ID1,
 *									Edid);
 *
 * 2) Use a standard video timing mode (see mode_table.h):
 *	XDp_TxCfgMsaUseStandardVideoMode(InstancePtr, XDP_TX_STREAM_ID1,
							XVIDC_VM_640x480_60_P);
 *
 * 3) Use a custom configuration for the main stream attributes (MSA):
 *	XDp_TxMainStreamAttributes MsaConfigCustom;
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
 *	XDp_TxCfgMsaUseCustom(InstancePtr, XDP_TX_STREAM_ID1,
 *							&MsaConfigCustom, 1);
 *
 * To override the user pixel width:
 *	InstancePtr->TxInstance.MsaConfig[_STREAM#_].OverrideUserPixelWidth = 1;
 *	InstancePtr->TxInstance.MsaConfig[_STREAM#_].UserPixelWidth =
 *								_DESIRED_VALUE_;
 *	Then, use one of the methods above to calculate the rest of the MSA.
 */
	Status = XDp_TxGetEdid(InstancePtr, Edid);
	if (Status == XST_SUCCESS) {
		XDp_TxCfgMsaUseEdidPreferredTiming(InstancePtr,
						XDP_TX_STREAM_ID1, Edid);
	}
	else {
		XDp_TxCfgMsaUseStandardVideoMode(InstancePtr, XDP_TX_STREAM_ID1,
							XVIDC_VM_640x480_60_P);
	}

	/* Disable MST for this example. */
	XDp_TxMstDisable(InstancePtr);

	/* Disable main stream to force sending of IDLE patterns. */
	XDp_TxDisableMainLink(InstancePtr);

	/* Reset the transmitter. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_SOFT_RESET,
				XDP_TX_SOFT_RESET_VIDEO_STREAM_ALL_MASK);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_SOFT_RESET, 0x0);

	/* Set the DisplayPort TX video mode. */
	XDp_TxSetVideoMode(InstancePtr, XDP_TX_STREAM_ID1);

	/* Configure video stream source or generator here. These function need
	 * to be implemented in order for video to be displayed and is hardware
	 * system specific. It is up to the user to implement these
	 * functions. */
	Dptx_StreamSrcSetup(InstancePtr);
	Dptx_StreamSrcConfigure(InstancePtr);
	Dptx_StreamSrcSync(InstancePtr);
	/*********************************/

	XDp_TxEnableMainLink(InstancePtr);
}
