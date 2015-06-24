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
 * @file xdptx_mst_example.c
 *
 * Contains a design example using the XDptx driver in multi-stream transport
 * (MST) mode.
 *
 * @note	For this example to display output, the user will need to
 *		implement initialization of the system (Dptx_PlatformInit) and,
 *		after training is complete, implement configuration of the video
 *		stream source in order to provide the DisplayPort core with
 *		input. See XAPP1178 for reference.
 * @note	The functions Dptx_PlatformInit and Dptx_StreamSrc* are declared
 *		extern in xdptx_example_common.h and are left up to the user to
 *		implement.
 * @note	Some setups may require introduction of delays when sending
 *		sideband messages.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  08/07/14 Initial creation.
 * 2.0   als  09/23/14 Improved programming sequence for payload allocation.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx_example_common.h"

/**************************** Constant Definitions ****************************/

/* The maximum number of streams to enable. */
#define NUM_STREAMS 4

/* This enables topology discovery which will create a list of sinks in the
 * topology. If ALLOCATE_FROM_SINKLIST is defined, the streams will sent to
 * to the sinks with the corresponding index. See the function calls for
 * the XDptx_SetStreamSelectFromSinkList driver function below. */
#define ALLOCATE_FROM_SINKLIST
#ifdef ALLOCATE_FROM_SINKLIST
/* Define the mapping between sinks and streams. The sink numbers are in the
 * order that they are discovered by the XDptx_FindAccessibleDpDevices driver
 * function. */
#define STREAM1_USE_SINKNUM 0
#define STREAM2_USE_SINKNUM 1
#define STREAM3_USE_SINKNUM 2
#define STREAM4_USE_SINKNUM 3
#endif

/* The video resolution from the display mode timings (DMT) table to use for
 * each stream. */
#define USE_VIDEO_MODE XVIDC_VM_1920x1080_60_P

/* The color depth (bits per color component) to use for each stream. */
#define USE_BPC 8

/* Some MST configurations may require delays when sending sideband messages. By
 * default, disable these delays.*/
#undef USE_DELAYS_FOR_MST

/**************************** Function Prototypes *****************************/

u32 Dptx_MstExample(XDptx *InstancePtr, u16 DeviceId);
u32 Dptx_MstExampleRun(XDptx *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDptx multi-stream transport (MST)
 *  example.
 *
 * @param	None.
 *
 * @return
 *		- XST_FAILURE if the MST example was unsuccessful - system
 *		  setup failed.
 *
 * @note	Unless setup failed, main will never return since
 *		Dptx_MstExample is blocking.
 *
*******************************************************************************/
int main(void)
{
	/* Run the XDptx MST example. */
	Dptx_MstExample(&DptxInstance, DPTX_DEVICE_ID);

	return XST_FAILURE;
}

/******************************************************************************/
/**
 * The main entry point for the multi-stream transport (MST) example using the
 * XDptx driver. This function will either discover the topology and map streams
 * to the sinks in the sink list, or map streams to relative addresses.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 *
 * @return
 *		- XST_FAILURE if the system setup failed.
 *		- XST_SUCCESS should never return since this function, if setup
 *		  was successful, is blocking.
 *
 * @note	If system setup was successful, this function is blocking.
 *
*******************************************************************************/
u32 Dptx_MstExample(XDptx *InstancePtr, u16 DeviceId)
{
	u32 Status;

	/* Do platform initialization here. This is hardware system specific -
	 * it is up to the user to implement this function. */
	Dptx_PlatformInit();
	/******************/

	Status = Dptx_SetupExample(InstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XDptx_MstCapable(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* If the immediate downstream RX device is an MST monitor and
		 * the DisplayPort Configuration Data (DPCD) does not indicate
		 * MST capability, it is likely that the MST or DisplayPort v1.2
		 * option must be selected from the monitor's option menu.
		 * Likewise, the DisplayPort TX core must be configured to
		 * support MST mode. */
		xil_printf("!!! Verify DisplayPort MST capabilities in the TX "
							"and/or RX device.\n");
		return XST_FAILURE;
	}

	do {
		Status = Dptx_MstExampleRun(InstancePtr);
		if (Status == XST_DATA_LOST) {
			xil_printf("!!! Link lost... Need to re-train.\n");
		}
	} while (Status != XST_SUCCESS);

	/* Do not return. */
	xil_printf("MST example DONE.\n");
	while (1);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function trains the link and allocates stream payloads.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 *
 * @return
 *		- XST_SUCCESS if MST allocation was successful.
 *		- XST_ERROR_COUNT_MAX if the ACT trigger was lost.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dptx_MstExampleRun(XDptx *InstancePtr)
{
	u32 Status;
	u32 MaskVal;
	u8 StreamIndex;
	XVidC_VideoMode VideoMode = USE_VIDEO_MODE;
	u8 Bpc = USE_BPC;
	u8 NumStreams = NUM_STREAMS;

	/* Limit the number of streams to configure based on the configuration
	 * of the DisplayPort core. */
	if (NumStreams > InstancePtr->Config.NumMstStreams) {
		NumStreams = InstancePtr->Config.NumMstStreams;
	}

	XDptx_EnableTrainAdaptive(InstancePtr, TRAIN_ADAPTIVE);
	XDptx_SetHasRedriverInPath(InstancePtr, TRAIN_HAS_REDRIVER);

	/* A DisplayPort connection must exist at this point. See the interrupt
	 * and polling examples for waiting for connection events. */
	Status = Dptx_StartLink(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Link Training failed.\n");
		return XST_FAILURE;
	}

#ifdef USE_DELAYS_FOR_MST
	InstancePtr->AuxDelayUs = 30000;
	InstancePtr->SbMsgDelayUs = 100000;
#else
	InstancePtr->AuxDelayUs = 0;
	InstancePtr->SbMsgDelayUs = 0;
#endif

	XDptx_ClearMsaValues(InstancePtr, XDPTX_STREAM_ID1);
	XDptx_ClearMsaValues(InstancePtr, XDPTX_STREAM_ID2);
	XDptx_ClearMsaValues(InstancePtr, XDPTX_STREAM_ID3);
	XDptx_ClearMsaValues(InstancePtr, XDPTX_STREAM_ID4);

#ifdef ALLOCATE_FROM_SINKLIST
	/* Run topology discovery to determine what devices are accessible to
	 * the DisplayPort TX. */
	xil_printf("Find topology >>>\n");
	InstancePtr->Topology.NodeTotal = 0;
	InstancePtr->Topology.SinkTotal = 0;

	Status = XDptx_DiscoverTopology(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("!!! A LINK_ADDRESS response from a branch device "
			"in the MST topology was not successfully received.\n");
		return XST_FAILURE;
	}

	xil_printf("<<< Find topology DONE; # of sinks found = %d.\n",
					InstancePtr->Topology.SinkTotal);

	if (NumStreams > InstancePtr->Topology.SinkTotal) {
		NumStreams = InstancePtr->Topology.SinkTotal;
	}
#endif

	/* Enable multi-stream transport (MST) mode for this example. */
	XDptx_MstCfgModeEnable(InstancePtr);
	for (StreamIndex = 0; StreamIndex < NumStreams; StreamIndex++) {
		XDptx_MstCfgStreamEnable(InstancePtr, XDPTX_STREAM_ID1 +
								StreamIndex);
	}
	for (StreamIndex = NumStreams; StreamIndex < 4; StreamIndex++) {
		XDptx_MstCfgStreamDisable(InstancePtr, XDPTX_STREAM_ID1 +
								StreamIndex);
	}

	/* Specify the DisplayPort sink devices that each enabled stream will be
	 * directed towards. */
#ifndef ALLOCATE_FROM_SINKLIST
	/* If topology discovery is not used, specify the relative addresses of
	 * the DisplayPort sink devices. */
	u8 Lct;
	u8 Rad[15];

	if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID1)) {
		Lct = 2; Rad[0] = 8;
		XDptx_SetStreamSinkRad(InstancePtr, XDPTX_STREAM_ID1, Lct, Rad);
	}
	if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID2)) {
		Lct = 3; Rad[0] = 1; Rad[1] = 8;
		XDptx_SetStreamSinkRad(InstancePtr, XDPTX_STREAM_ID2, Lct, Rad);
	}
	if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID3)) {
		Lct = 4; Rad[0] = 1; Rad[1] = 1; Rad[2] = 8;
		XDptx_SetStreamSinkRad(InstancePtr, XDPTX_STREAM_ID3, Lct, Rad);
	}
	if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID4)) {
		Lct = 4; Rad[0] = 1; Rad[1] = 1; Rad[2] = 9;
		XDptx_SetStreamSinkRad(InstancePtr, XDPTX_STREAM_ID4, Lct, Rad);
	}
#else
	/* If topology discovery is used, associate a stream number with a sink
	 * number from the sink list obtained during topology discovery. The
	 * sinks are numbered in the order that they were found during topology
	 * discovery. */
	if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID1)) {
		XDptx_SetStreamSelectFromSinkList(InstancePtr, XDPTX_STREAM_ID1,
							STREAM1_USE_SINKNUM);
	}
	if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID2)) {
		XDptx_SetStreamSelectFromSinkList(InstancePtr, XDPTX_STREAM_ID2,
							STREAM2_USE_SINKNUM);
	}
	if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID3)) {
		XDptx_SetStreamSelectFromSinkList(InstancePtr, XDPTX_STREAM_ID3,
							STREAM3_USE_SINKNUM);
	}
	if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID4)) {
		XDptx_SetStreamSelectFromSinkList(InstancePtr, XDPTX_STREAM_ID4,
							STREAM4_USE_SINKNUM);
	}
#endif

	/* Reset MST mode in both the RX and TX. */
	XDptx_MstDisable(InstancePtr);
	XDptx_MstEnable(InstancePtr);

	/* Set the main stream attributes (MSA) for each enabled stream (each
	 * stream has an identical configuration). Then, set the configuration
	 * for that stream in the corresponding DisplayPort TX registers. */
	for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
		if (XDptx_MstStreamIsEnabled(InstancePtr, XDPTX_STREAM_ID1 +
								StreamIndex)) {
			XDptx_CfgMsaSetBpc(InstancePtr, XDPTX_STREAM_ID1 +
							StreamIndex, Bpc);
			XDptx_CfgMsaEnSynchClkMode(InstancePtr,
					XDPTX_STREAM_ID1 + StreamIndex, 1);

			XDptx_CfgMsaUseStandardVideoMode(InstancePtr,
				XDPTX_STREAM_ID1 + StreamIndex, VideoMode);
			XDptx_SetVideoMode(InstancePtr, XDPTX_STREAM_ID1 +
								StreamIndex);
		}
	}

	/* Configure video stream source or generator here. This function needs
	 * to be implemented in order for video to be displayed and is hardware
	 * system specific. It is up to the user to implement this function. */
	Dptx_StreamSrcSetup(InstancePtr);
	Dptx_StreamSrcConfigure(InstancePtr);
	Dptx_StreamSrcSync(InstancePtr);
	////////////////////////////////////

	/* Mask interrupts while allocating payloads. */
	MaskVal = XDptx_ReadReg(InstancePtr->Config.BaseAddr,
							XDPTX_INTERRUPT_MASK);
	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_INTERRUPT_MASK,
									0x3F);

	/* Clear the payload ID table first. */
	Status = XDptx_ClearPayloadVcIdTable(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_DATA_LOST;
	}

	/* Allocate payloads. */
	Status = XDptx_AllocatePayloadStreams(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_DATA_LOST;
	}

	/* Enable the main link. */
	XDptx_EnableMainLink(InstancePtr);

	/* Unmask interrupts. */
	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_INTERRUPT_MASK,
								MaskVal);

	/* Do a final check to verify that the link wasn't lost. */
	Status = XDptx_CheckLinkStatus(InstancePtr,
					InstancePtr->LinkConfig.LaneCount);
	if (Status != XST_SUCCESS) {
		XDptx_WaitUs(InstancePtr, 10000);
		return XST_DATA_LOST;
	}

	return XST_SUCCESS;
}
