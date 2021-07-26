/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu_common_example.c
 *
 * Contains a design example using the XDpPsu driver. It performs a self test on
 * the DisplayPort TX core by training the main link at the maximum common
 * capabilities between the TX and RX and checking the lane status.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  09/17/17 Initial creation.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdppsu_common_example.h"
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

static void DpPsu_StartVideoStream(XDpPsu *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function will configure and establish a link with the receiver device,
 * afterwards, a video stream will start to be sent over the main link.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS if main link was successfully established.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 DpPsu_Run(XDpPsu *InstancePtr)
{
	u32 Status;

	/* Configure and establish a link. */
	Status = DpPsu_StartLink(InstancePtr);
	if (Status == XST_SUCCESS) {
		/* Start the video stream. */
		DpPsu_StartVideoStream(InstancePtr);
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
 * @param	InstancePtr is a pointer to the XDpPsu instance.
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
u32 DpPsu_SetupExample(XDpPsu *InstancePtr, u16 DeviceId)
{
	XDpPsu_Config *ConfigPtr;
	u32 Status;

	/* Obtain the device configuration for the DisplayPort TX core. */
	ConfigPtr = XDpPsu_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the InstancePtr's Config
	 * structure. */
	XDpPsu_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddr);

	/* Initialize the DisplayPort TX core. */
	Status = XDpPsu_InitializeTx(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will configure and establish a link with the receiver device.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return
 *		- XST_SUCCESS the if main link was successfully established.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 DpPsu_StartLink(XDpPsu *InstancePtr)
{
	u32 VsLevelTx;
	u32 PeLevelTx;
	u32 Status;
	u8 LaneCount;
	u8 LinkRate;

	/* Obtain the capabilities of the RX device by reading the monitor's
	 * DPCD. */
	Status = XDpPsu_GetRxCapabilities(InstancePtr);
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
	Status = XDpPsu_CheckLinkStatus(InstancePtr, LaneCount);
	if (Status == XST_SUCCESS) {
		xil_printf("-> Link is already trained on %d lanes.\n",
								LaneCount);
		if (XDpPsu_ReadReg(InstancePtr->Config.BaseAddr,
					XDPPSU_LINK_BW_SET) == LinkRate) {
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

	XDpPsu_SetEnhancedFrameMode(InstancePtr, 1);
	XDpPsu_SetDownspread(InstancePtr, 0);

#if (TRAIN_USE_MAX_LINK == 1)
	/* Configure the main link based on the maximum common capabilities of
	 * the DisplayPort TX core and the receiver device. */
	Status = XDpPsu_CfgMainLinkMax(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#else
	XDpPsu_SetLinkRate(InstancePtr, LinkRate);
	XDpPsu_SetLaneCount(InstancePtr, LaneCount);
#endif

	/* Train the link. */
	xil_printf("******************************************\n");
	Status = XDpPsu_EstablishLink(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("!!! Training failed !!!\n");
		xil_printf("******************************************\n");
		return XST_FAILURE;
	}

	xil_printf("!!! Training passed at LR:0x%02lx LC:%d !!!\n",
					InstancePtr->LinkConfig.LinkRate,
					InstancePtr->LinkConfig.LaneCount);
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
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	The DpPsu_StreamSrc* are intentionally left for the user to
 *		implement since configuration of the stream source is
 *		application-specific.
 * @note	The Extended Display Identification Data (EDID) is read in order
 *		to obtain the video resolution and timings. If this read fails,
 *		a resolution of 640x480 is used at a refresh rate of 60Hz.
 *
*******************************************************************************/
static void DpPsu_StartVideoStream(XDpPsu *InstancePtr)
{
	u32 Status;
	u8 Edid[XDPPSU_EDID_BLOCK_SIZE];

	/* Set the bits per color. If not set, the default is 6. */
	XDpPsu_CfgMsaSetBpc(InstancePtr, 8);

	/* Set synchronous clock mode. */
	XDpPsu_CfgMsaEnSynchClkMode(InstancePtr, 1);

/* Choose a method for selecting the video mode. There are 3 ways to do this:
 * 1) Use the preferred timing from the monitor's EDID:
 *	u8 Edid[XDPPSU_EDID_BLOCK_SIZE];
 *	XDpPsu_GetEdid(InstancePtr, Edid);
 *	XDpPsu_CfgMsaUseEdidPreferredTiming(InstancePtr, Edid);
 *
 * 2) Use a standard video timing mode (see mode_table.h):
 *	XDpPsu_CfgMsaUseStandardVideoMode(InstancePtr, XVIDC_VM_640x480_60_P);
 *
 * 3) Use a custom configuration for the main stream attributes (MSA):
 *	XDpPsu_MainStreamAttributes MsaConfigCustom;
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
 *	XDpPsu_CfgMsaUseCustom(InstancePtr, XDPPSU_STREAM_ID1,
 *							&MsaConfigCustom, 1);
 *
 *	Then, use one of the methods above to calculate the rest of the MSA.
 */
	Status = XDpPsu_GetEdid(InstancePtr, Edid);
	if (Status == XST_SUCCESS) {
		XDpPsu_CfgMsaUseEdidPreferredTiming(InstancePtr, Edid);
	}
	else {
		XDpPsu_CfgMsaUseStandardVideoMode(InstancePtr, XVIDC_VM_640x480_60_P);
	}

	/* Disable main stream to force sending of IDLE patterns. */
	XDpPsu_EnableMainLink(InstancePtr, 0);

	/* Reset the transmitter. */
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_SOFT_RESET, 0x1);
	XDpPsu_WriteReg(InstancePtr->Config.BaseAddr, XDPPSU_SOFT_RESET, 0x0);

	/* Set the DisplayPort TX video mode. */
	XDpPsu_SetVideoMode(InstancePtr);

	/* If VPLL is being used for Video Pixel clock, use the APIs from
	 * AVBUF driver to set the clock.
	 */

	/* Configure video stream source or generator here. These function need
	 * to be implemented in order for video to be displayed and is hardware
	 * system specific. It is up to the user to implement these
	 * functions. These functions are available in AVBuf driver */
	DpPsu_StreamSrcSetup(InstancePtr);
	DpPsu_StreamSrcConfigure(InstancePtr);
	DpPsu_StreamSrcSync(InstancePtr);
	/*********************************/

	XDpPsu_EnableMainLink(InstancePtr, 1);
}
