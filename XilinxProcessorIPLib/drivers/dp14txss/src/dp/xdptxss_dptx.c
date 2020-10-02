/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_dptx.c
*
* This file contains a minimal set of functions for the DisplayPort core
* to configure in TX mode of operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ---------------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Renamed file name with prefix xdptxss_* and function
*                   names with prefix XDpTxSs_*.
* 2.00 sha 08/07/15 Added support for customized main stream attributes for
*                   Single Steam Transport and Multi-Stream Transport.
* 2.00 sha 09/28/15 Removed cross checking user set resolution with RX EDID.
* 4.0  aad 05/13/16 Use asynchronous clock mode by default.
* 5.0  tu  08/03/17 Enabled video packing for bpc > 10
* 5.0  aad 09/08/17 Case to handle HTotal > 4095, PPC = 1 in AXIStream Mode.
* 6.4  rg  09/26/20 Added support for YUV420 color format.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss_dptx.h"
#include "xvidc.h"
#include "xvidc_edid.h"
#include "xdebug.h"
#include "string.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static u32 Dp_GetTopology(XDp *InstancePtr);
static u32 Dp_CheckBandwidth(XDp *InstancePtr, u8 Bpc,
				XVidC_VideoMode VidMode);
static XVidC_VideoMode Dp_GetPreferredVm(u8 *EdidPtr);
static void Dp_ConfigVideoPackingClockControl(XDp *InstancePtr, u8 Bpc);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function configures DisplayPort sub-core with preferred resolution
* read from sink or user set resolution, bits per color in SST/MST mode.
* In MST mode, if sinks are more than two, it re-orders the sinks if belongs
* to same tiled display topology. It trains the link and allocates stream
* payloads for single stream (SST) or multi-stream transport mode (MST).
* In MST mode, discovers the topology and finds the actual number of sinks to
* which associates streams.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	TransportMode specifies whether multiple/single steam to be
*		sent over the main link.
*		- TransportMode = 1 (for Multi-Stream Transport)
*		- TransportMode = 0 (for Single Stream Transport)
* @param	Bpc is the new number of bits per color to use.
* @param	VidMode is one of the enumerated standard video modes
*		defined in xvidc.h file.
*
* @return
*		- XST_SUCCESS DisplayPort configured successfully.
*		- XST_FAILURE if DisplayPort configuration failed.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_DpTxStart(XDp *InstancePtr, u8 TransportMode, u8 Bpc,
			XVidC_VideoMode VidMode)
{
	u32 Status;
	u8 StreamIndex;
	u8 NumOfStreams;
	u8 Edid[128];
	XDp_TxTopologyNode *Sink1;
	int i;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((TransportMode == XDPTXSS_DPTX_MST) ||
			(TransportMode == XDPTXSS_DPTX_SST));
	Xil_AssertNonvoid((Bpc == XVIDC_BPC_6) || (Bpc == XVIDC_BPC_8) ||
			(Bpc == XVIDC_BPC_10) || (Bpc == XVIDC_BPC_12) ||
			(Bpc == XVIDC_BPC_16));

	/* Check for MST / SST mode */
	if (TransportMode) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"\n\rSS INFO:Starting "
			"MST config.\n\r");
		/* Enable MST mode in both the RX and TX. */
		Status = XDp_TxMstEnable(InstancePtr);
		if (Status != XST_SUCCESS) {
			/* If the immediate downstream RX device is an MST
			 * monitor and the DisplayPort Configuration Data
			 * (DPCD) does not indicate MST capability, it is
			 * likely that the MST or DisplayPort v1.2 option must
			 * be selected from the monitor's option menu.
			 * Likewise, the DisplayPort TX core must be configured
			 * to support MST mode. */
			if (Status == XST_DEVICE_NOT_FOUND) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:MST:"
					"No connection exists. Verify cable "
						"and/or monitor.\n\r");
			}
			else {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:MST:"
					"Verify DisplayPort MST capabilities "
					"in the TX and/or RX device MST "
					"status is %ld\n\r", Status);
			}

			return XST_FAILURE;
		}

		/* Set AUX and sideband delays in microseconds */
		InstancePtr->TxInstance.AuxDelayUs = 30000;
		InstancePtr->TxInstance.SbMsgDelayUs = 30000;

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Discovering "
				"topology.\n\r");
		/* Get list of sinks */
		Status = Dp_GetTopology(InstancePtr);
		if (Status)
			return Status;

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Topology "
				"discovery done, # of sinks found = %d.\n\r",
				InstancePtr->TxInstance.Topology.SinkTotal);

		/* Total number of streams equal to number of sinks found */
		NumOfStreams = InstancePtr->TxInstance.Topology.SinkTotal;

		/* Enable downshifting during link training */
		XDp_TxEnableTrainAdaptive(InstancePtr, 1);

		/* Disable main stream to force sending of IDLE patterns. */
		XDp_TxDisableMainLink(InstancePtr);

		/* Start link training with user set link rate and lane
		 * count. Training is required to discover topology so that
		 * prefer erred timing can be known.
		 */
		Status = XDpTxSs_DpTxStartLink(InstancePtr, FALSE);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Re- "
				"training with maximum RX capabilities\n\r");

			/* Train link with maximum RX capabilities */
			Status = XDpTxSs_DpTxStartLink(InstancePtr, TRUE);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:MST:"
					"Verify cable and/or monitor.\n\r");
				return Status;
			}
		}

		Status = XDp_TxCheckLinkStatus(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.LaneCount);
		if (Status == XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Link "
					"is up !\n\r\n\r");
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:Reading (MST) Sink "
			"EDID...\n\r");

		/* Read EDID of first sink */
		Sink1 = InstancePtr->TxInstance.Topology.SinkList[0];
		XDp_TxGetRemoteEdid(InstancePtr, Sink1->LinkCountTotal,
				Sink1->RelativeAddress, Edid);

		/* Check video mode for EDID preferred video mode */
		if (VidMode == XVIDC_VM_USE_EDID_PREFERRED) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Using "
				"preferred EDID resolution.\n\r");

			/* Get preferred video mode from EDID */
			VidMode = Dp_GetPreferredVm(Edid);

			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:VM "
				"from EDID:%s\n\r",
					XVidC_GetVideoModeStr(VidMode));

			/* Check whether Video mode exist in VTM table */
			if (XVIDC_VM_NOT_SUPPORTED == VidMode) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:"
					"Preferred video mode not present in "
					"VMT. Update video timing table.\n\r"
					"Setting to 640x480 video mode\n\r");
				VidMode = XVIDC_VM_640x480_60_P;
			}

			if ((InstancePtr->TxInstance.Topology.SinkTotal ==
				4) && (VidMode == XVIDC_VM_UHD2_60_P)) {
				VidMode = XVIDC_VM_1080_60_P;

				xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:"
					"MST:Re-ordering sinks if belongs to "
					"same TDT...\n\r");

				/* Order the sink belong to same TDT */
				XDp_TxTopologySortSinksByTiling(InstancePtr);
			}
			else if ((InstancePtr->TxInstance.Topology.SinkTotal ==
				2) && (VidMode <= XVIDC_VM_UHD2_60_P)){

				xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:"
					"MST:Re-ordering sinks if belongs to "
					"same TDT...\n\r");

				/* Order the sink belong to same TDT */
				XDp_TxTopologySortSinksByTiling(InstancePtr);
			}
		}
		else if (VidMode != XVIDC_VM_CUSTOM) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Using "
				"user set resolution.\n\r");

			if ((InstancePtr->TxInstance.Topology.SinkTotal ==
				4) && (VidMode == XVIDC_VM_UHD2_60_P)){
				VidMode = XVIDC_VM_1080_60_P;

				xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:"
					"Re-ordering sinks if belongs to same "
						"TDT...\n\r");

				/* Order sinks belongs to the same TDT */
				XDp_TxTopologySortSinksByTiling(InstancePtr);
			}
			else if ((InstancePtr->TxInstance.Topology.SinkTotal ==
				2) && (VidMode <= XVIDC_VM_UHD2_60_P)){

				xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:"
					"Re-ordering sinks if belongs to same "
						"TDT...\n\r");

				/* Order sinks belongs to the same TDT */
				XDp_TxTopologySortSinksByTiling(InstancePtr);
			}
		}
		else {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Using "
				"custom set resolution.\n\r");
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:calculating "
			"payload...\n\r");

		/* Check link and video bandwidth */
		Status = Dp_CheckBandwidth(InstancePtr, Bpc, VidMode);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Link is "
				"over-subscribed  for selected resolution, "
				"bpc, lane count and link rate value."
				"\n\rRe-training with maximum RX capabilities."
				"\n\r");

			/* Check for link training need and run training
			 * sequence.
			 */
			Status = XDpTxSs_DpTxStartLink(InstancePtr, TRUE);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:"
					"Re-training with max after payload "
						"failed.\n\r");
				return Status;
			}
		}

		/* Enable each stream(s) */
		for (StreamIndex = 0; StreamIndex < NumOfStreams;
							StreamIndex++) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:"
				"Enabling stream #%d\n", XDP_TX_STREAM_ID1 +
					StreamIndex);
			XDp_TxMstCfgStreamEnable(InstancePtr,
				XDP_TX_STREAM_ID1 + StreamIndex);
			XDp_TxSetStreamSelectFromSinkList(InstancePtr,
				XDP_TX_STREAM_ID1 + StreamIndex, StreamIndex);
		}

		/* Disable stream(s) */
		for (StreamIndex = NumOfStreams; StreamIndex < 4;
							StreamIndex++) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:"
				"Disabling stream #%d\n", XDP_TX_STREAM_ID1 +
					StreamIndex);
			XDp_TxMstCfgStreamDisable(InstancePtr,
					XDP_TX_STREAM_ID1 + StreamIndex);
		}

		/* Stream setup */
		for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
			if (XDp_TxMstStreamIsEnabled(InstancePtr,
					XDP_TX_STREAM_ID1 + StreamIndex)) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:"
					"Stream #%d... ",XDP_TX_STREAM_ID1 +
						StreamIndex);

				/* Set bits per color for each stream */
				XDp_TxCfgMsaSetBpc(InstancePtr,
					XDP_TX_STREAM_ID1 + StreamIndex, Bpc);

				/* Check for video mode */
				if (VidMode == XVIDC_VM_CUSTOM) {
					/* Enable sync clock mode for each
					 * stream
					 */
					XDp_TxCfgMsaEnSynchClkMode(InstancePtr,
						XDP_TX_STREAM_ID1 +
						StreamIndex,
					InstancePtr->TxInstance.MsaConfig[
					StreamIndex].SynchronousClockMode);

					/* Set user pixel width if video mode
					 * is 1920 x 2160
					 */
					if ((InstancePtr->TxInstance.MsaConfig[
						StreamIndex].Vtm.Timing.HActive
							== 1920) &&
					(InstancePtr->TxInstance.MsaConfig[
						StreamIndex].Vtm.Timing.VActive
							== 2160) &&
					(InstancePtr->TxInstance.MsaConfig[
					StreamIndex].OverrideUserPixelWidth
								== 0)) {
					InstancePtr->TxInstance.MsaConfig[
						StreamIndex].UserPixelWidth =
							4;
					}
				}
				else {
					/* Enable async clock mode for each
					 * stream
					 */
					XDp_TxCfgMsaEnSynchClkMode(InstancePtr,
						XDP_TX_STREAM_ID1 +
							StreamIndex, 0);

					/* Use standard video mode to calculate
					 * MSA
					 */
					XDp_TxCfgMsaUseStandardVideoMode(
						InstancePtr,
						XDP_TX_STREAM_ID1 +
						StreamIndex, VidMode);

					/* Set user pixel width if video mode
					 * is UHD2
					 */
					if ((InstancePtr->TxInstance.MsaConfig[
						StreamIndex].Vtm.VmId ==
							XVIDC_VM_UHD2_60_P) &&
					(InstancePtr->TxInstance.MsaConfig[
					StreamIndex].OverrideUserPixelWidth ==
									0)) {
					InstancePtr->TxInstance.MsaConfig[
						StreamIndex].UserPixelWidth =
									4;
					}
				}

				/* Apply to hardware */
				XDp_TxSetVideoMode(InstancePtr,
					XDP_TX_STREAM_ID1 + StreamIndex);

				xdbg_printf(XDBG_DEBUG_GENERAL,"configured."
					"\n\r");
			}
		}

		Status = XDp_TxCheckLinkStatus(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.LaneCount);
		if (Status == XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST: Link "
				"is up after streams are configured!\n\r\n\r");
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Allocating "
			"payload...\n\r");

		/* Clear virtual channel payload ID table in TX and all
		 * downstream RX devices
		*/
		Status = XDp_TxClearPayloadVcIdTable(InstancePtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:MST:"
				"Clearing virtual channel payload "
					"failed.\n\r");
			return XST_DATA_LOST;
		}

		/* Allocate payloads. */
		Status = XDp_TxAllocatePayloadStreams(InstancePtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:MST:"
				"Allocation failed. Check link "
				"over-subscription and bring down "
				"resolution or BPC.\n\r");
			return XST_DATA_LOST;
		}

		Status = XDp_TxCheckLinkStatus(InstancePtr,
				InstancePtr->TxInstance.LinkConfig.LaneCount);
		if (Status == XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Link "
				"is up after allocate payload!\n\r\n\r");
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:Config done!"
			"\n\r\n\r");
	}
	/* SST */
	else {
		xdbg_printf(XDBG_DEBUG_GENERAL,"\n\rSS INFO:Starting "
			"SST config.\n\r");

		/* Reset MST mode in both the RX and TX */
		Status = XDp_TxMstDisable(InstancePtr);
		if (Status != XST_SUCCESS) {
			InstancePtr->TxInstance.MstEnable = 0;
		}

		/* set AUX and sideband delays in microseconds */
		InstancePtr->TxInstance.AuxDelayUs = 0;
		InstancePtr->TxInstance.SbMsgDelayUs = 0;

		/* Enable downshifting during link training */
		XDp_TxEnableTrainAdaptive(InstancePtr, 1);

		/* Disable main stream to force sending of IDLE patterns. */
		XDp_TxDisableMainLink(InstancePtr);

		/* Start link training with user set link rate and lane
		 * count.
		 */
		Status = XDpTxSs_DpTxStartLink(InstancePtr, FALSE);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST:"
				"Re-training with maximum RX capabilities."
					"\n\r");

			/* Train link with maximum RX capabilities */
			Status = XDpTxSs_DpTxStartLink(InstancePtr, TRUE);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:SST:"
					"Verify cable and/or monitor.\n\r");
				return Status;
			}
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,"Reading (SST) Sink EDID..."
			"\n\r");

		/* Get EDID */
		XDp_TxGetEdid(InstancePtr, Edid);

		if (VidMode == XVIDC_VM_USE_EDID_PREFERRED) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST:Using "
				"preferred EDID resolution.\n\r");

			/* Get preferred video mode from EDID */
			VidMode = Dp_GetPreferredVm(Edid);

			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST: VM "
				"from EDID:%s\n\r",
					XVidC_GetVideoModeStr(VidMode));

			/* Check whether Video mode exist in VTM table */
			if (XVIDC_VM_NOT_SUPPORTED == VidMode) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"EDID "
					"preferred video mode not present. "
					"Update video timing table.\n\r"
					"Setting to 640x480 video mode\n\r");
				VidMode = XVIDC_VM_640x480_60_P;
			}
		}
		else if (VidMode != XVIDC_VM_CUSTOM) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST:Using "
				"user set resolution.\n\r");
		}
		else {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST:Using "
				"custom set resolution.\n\r");
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST:calculating "
			"payload...\n\r");

		/* Check link and video bandwidth */
		Status = Dp_CheckBandwidth(InstancePtr, Bpc, VidMode);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST:Link is "
				"over-subscribed for selected resolution, "
				"bpc, lane count and link rate value.\n\r"
				"Re-training with maximum RX capabilities."
				"\n\r");

			/* Check for link training need and run training
			 * sequence.
			 */
			Status = XDpTxSs_DpTxStartLink(InstancePtr, TRUE);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST:"
					"Re-training failed with max "
						"capabilities after payload "
						"failure.\n\r");
				return Status;
			}
		}

		/* Reset MSA values */
		XDp_TxClearMsaValues(InstancePtr, XDP_TX_STREAM_ID1);
		XDp_TxClearMsaValues(InstancePtr, XDP_TX_STREAM_ID2);
		XDp_TxClearMsaValues(InstancePtr, XDP_TX_STREAM_ID3);
		XDp_TxClearMsaValues(InstancePtr, XDP_TX_STREAM_ID4);

		if (VidMode == XVIDC_VM_CUSTOM) {
			XDp_TxCfgMsaEnSynchClkMode(InstancePtr,
				XDP_TX_STREAM_ID1,
					InstancePtr->TxInstance.MsaConfig[
						0].SynchronousClockMode);
		}
		else {
			InstancePtr->TxInstance.MsaConfig[
				0].DynamicRange = 0;
			InstancePtr->TxInstance.MsaConfig[
				0].YCbCrColorimetry = 0;
			/* Enable async clock mode */
			XDp_TxCfgMsaEnSynchClkMode(InstancePtr,
						XDP_TX_STREAM_ID1, 0);
		}

		/* Set user provided BPC to stream 1 */
		XDp_TxCfgMsaSetBpc(InstancePtr, XDP_TX_STREAM_ID1, Bpc);

		/* Set user standard video mode for stream 1 to populate
		 * MSA values
		 */
		if (VidMode != XVIDC_VM_CUSTOM) {
			XDp_TxCfgMsaUseStandardVideoMode(InstancePtr,
					XDP_TX_STREAM_ID1, VidMode);

			/* Set user pixel width if video mode is UHD2 */
			if ((InstancePtr->TxInstance.MsaConfig[0].Vtm.VmId ==
				XVIDC_VM_UHD2_60_P) &&
				(InstancePtr->TxInstance.MsaConfig[
					0].OverrideUserPixelWidth == 0)) {
					InstancePtr->TxInstance.MsaConfig[
					0].UserPixelWidth = 4;
			}
		}

		 if((InstancePtr->TxInstance.MsaConfig[0].PixelClockHz <=
		     75000000) &&
		    (InstancePtr->TxInstance.MsaConfig[0].Vtm.Timing.HTotal >
		     4095) &&
		    (InstancePtr->TxInstance.MsaConfig[0].UserPixelWidth==1)) {
			 InstancePtr->TxInstance.MsaConfig[
				 0].UserPixelWidth = 2;
		 }

		/* Set video mode */
		XDp_TxSetVideoMode(InstancePtr, XDP_TX_STREAM_ID1);

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST: Config done!"
			"\n\r\n\r");

		/* Reset the transmitter. */
		XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_SOFT_RESET,
				XDP_TX_SOFT_RESET_VIDEO_STREAM_ALL_MASK |
				XDP_TX_SOFT_RESET_HDCP_MASK);
		XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_SOFT_RESET,
				0x0);
	}

	Dp_ConfigVideoPackingClockControl(InstancePtr, Bpc);

	/* Enable the main link. */
	XDp_TxEnableMainLink(InstancePtr);

	Status = XDp_TxCheckLinkStatus(InstancePtr,
			InstancePtr->TxInstance.LinkConfig.LaneCount);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:Link "
			"is DOWN after main link enabled!\n\r\n\r");
	}
	else if (Status == XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:Link "
			"is UP after main link enabled!\n\r\n\r");
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:Enabled main link!"
		"\n\r\n\r");

	/* Program the VSC Extended Packet */
	if (InstancePtr->TxInstance.ColorimetryThroughVsc)
	{
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_TX_AUDIO_EXT_DATA(1), InstancePtr->TxInstance.VscPacket.Header);
		for (i = 0; i < XDPTXSS_EXT_DATA_2ND_TO_9TH_WORD; i++) {
			XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_EXT_DATA(i+2),
				InstancePtr->TxInstance.VscPacket.Payload[i]);
		}
	}

	/* Enable Audio*/
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x1);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks if the link needs training and runs the training
* sequence if training is required based on the flags, indicates to use maximum
* RX capabilities or user specified link rate, lane count during training.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	TrainMaxCap is a flag indicates whether maximum capabilities
*		to be used during link training.
*		- TRUE - Use maximum RX capabilities.
*		- FALSE - Use custom capabilities.
*
* @return
*		- XST_SUCCESS the if main link was successfully established.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_DpTxStartLink(XDp *InstancePtr, u8 TrainMaxCap)
{
	u32 Status;
	u32 IntrMask;
	u8 LinkRate;
	u8 LaneCount;

	/* Read interrupts */
	IntrMask = XDp_ReadReg(InstancePtr->Config.BaseAddr,
				XDP_TX_INTERRUPT_MASK);

	/* Disable HPD pulse interrupts during link training. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK,
		IntrMask | XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK);

	/* Obtain the capabilities of the RX device by reading the monitor's
	 * DPCD.
	 */
	Status = XDp_TxGetRxCapabilities(InstancePtr);
	if (Status != XST_SUCCESS) {
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, IntrMask);
		return XST_FAILURE;
	}

	/* Enable clock spreading for both DP TX and RX device */
	XDp_TxSetDownspread(InstancePtr, 1);

	/* Enable enhanced framing symbol sequence */
	XDp_TxSetEnhancedFrameMode(InstancePtr, 1);

	/* Configure link with max values of link rate and lane count
	 * for the first time from next onwards configure it with the
	 * user set values.
	 */
	if (TrainMaxCap) {
		/* Configure the main link based on the maximum common
		 * capabilities of the DisplayPort TX core and the
		 * receiver device.
		 */
		Status = XDp_TxCfgMainLinkMax(InstancePtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:RX device "
				"not connected.\n\r");
			XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_TX_INTERRUPT_MASK, IntrMask);
			return XST_FAILURE;
		}
	}

	LinkRate = InstancePtr->TxInstance.LinkConfig.LinkRate;
	LaneCount = InstancePtr->TxInstance.LinkConfig.LaneCount;

	/* Establish link after training process */
	Status = XDp_TxEstablishLink(InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:Training failed."
				"\n\r");
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_TX_INTERRUPT_MASK, IntrMask);

		return XST_FAILURE;
	}

	/* Check whether link rate downshifted */
	if (LinkRate != InstancePtr->TxInstance.LinkConfig.LinkRate) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS Warning! Link rate is "
			"downshifted.\n\r");
	}

	/* Check whether lane count downshifted */
	if (LaneCount != InstancePtr->TxInstance.LinkConfig.LaneCount) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS Warning! Lane count is "
			"downshifted.\n\r");
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:Training passed at link rate:"
		"0x%02x lane count:%d.\n\r",
			InstancePtr->TxInstance.LinkConfig.LinkRate,
				InstancePtr->TxInstance.LinkConfig.LaneCount);

	xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:VS:%d (TX:%ld) PE:%d (TX:%ld)"
		"\n\r", InstancePtr->TxInstance.LinkConfig.VsLevel,
			XDp_ReadReg(InstancePtr->Config.BaseAddr,
				XDP_TX_PHY_VOLTAGE_DIFF_LANE_0),
				InstancePtr->TxInstance.LinkConfig.PeLevel,
			XDp_ReadReg(InstancePtr->Config.BaseAddr,
				XDP_TX_PHY_POSTCURSOR_LANE_0));

	/* Enable HPD interrupts after link training. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK,
			IntrMask);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function calculates the video bandwidth and link bandwidth in single
* stream and millstream mode. It checks whether the link bandwidth is not
* over-subscribed for video bandwidth.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	Bpc is the new number of bits per color to use.
* @param	VidMode is one of the enumerated standard video modes
*		defined in xvidc.h file.
*
* @return
*		- XST_SUCCESS if link bandwidth is not over-subscribed.
*		- XST_FAILURE otherwise.
*
* @note		Check that the stream allocation will succeed based on
*		capabilities. Don't go through training and allocation sequence
*		if the pre-calculations indicate that it will fail
*
******************************************************************************/
static u32 Dp_CheckBandwidth(XDp *InstancePtr, u8 Bpc, XVidC_VideoMode VidMode)
{
	u32 MstCapable;
	u32 LinkBw;
	u8 BitsPerPixel;

	LinkBw = (InstancePtr->TxInstance.LinkConfig.LaneCount *
		  InstancePtr->TxInstance.LinkConfig.LinkRate * 27);
	if (InstancePtr->TxInstance.MsaConfig[0].ComponentFormat ==
	    XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
		/* YCbCr 4:2:2 color component format. */
		BitsPerPixel = Bpc * 2;
	} else if (InstancePtr->TxInstance.MsaConfig[0].ComponentFormat ==
			XDP_MAIN_VSC_SDP_COMPONENT_FORMAT_YCBCR420) {
		/* YCbCr 4:2:0 color component format. */
		BitsPerPixel = (Bpc * 15) / 10;
	} else {
		/* RGB or YCbCr 4:4:4 color component format. */
		BitsPerPixel = Bpc * 3;
	}

	/* Check for maximum link rate supported */
	if (InstancePtr->TxInstance.LinkConfig.MaxLinkRate <
				InstancePtr->TxInstance.LinkConfig.LinkRate) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS:INFO:Requested link rate "
			"exceeds maximum capabilities.\n\rMaximum link "
				"rate = ");

		/* Report maximum link rate supported */
		switch (InstancePtr->TxInstance.LinkConfig.MaxLinkRate) {
			case XDP_TX_LINK_BW_SET_540GBPS:
				xdbg_printf(XDBG_DEBUG_GENERAL,"5.40 Gbps."
					"\n\r");
				break;

			case XDP_TX_LINK_BW_SET_270GBPS:
				xdbg_printf(XDBG_DEBUG_GENERAL,"2.70 Gbps."
					"\n\r");
				break;

			case XDP_TX_LINK_BW_SET_162GBPS:
				xdbg_printf(XDBG_DEBUG_GENERAL,"1.62 Gbps."
					"\n\r");
				break;
		}

		return XST_FAILURE;
	}
	else if (InstancePtr->TxInstance.LinkConfig.MaxLaneCount <
				InstancePtr->TxInstance.LinkConfig.LaneCount) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:Requested lane count "
			"exceeds maximum capabilities.\n\tMaximum lane count "
				"= %d.\n\r",
			InstancePtr->TxInstance.LinkConfig.MaxLaneCount);
		return XST_BUFFER_TOO_SMALL;
	}

	/* Check MST mode */
	MstCapable = XDp_TxMstCapable(InstancePtr);

	/* This check is done so that this function check can be called from
	 * anywhere and it will precaculate the required total timeslots based
	 * on the number of sinks and MSA values.
	 * This works because if the example will always run in MST mode if
	 * the monitor is capable of it, otherwise in SST mode.
	 */
	if ((MstCapable != XST_SUCCESS) ||
				(InstancePtr->TxInstance.MstEnable == 0)) {
		u32 TransferUnitSize = 64;
		u32 VideoBw;

		/* Check video mode */
		if (VidMode != XVIDC_VM_CUSTOM){
			VideoBw = (XVidC_GetPixelClockHzByVmId(VidMode) /
					1000) * BitsPerPixel / 8;
		}
		else {
			VideoBw = InstancePtr->TxInstance.MsaConfig[
				0].PixelClockHz / 1000 * BitsPerPixel / 8;
		}

		u32 AvgBytesPerTU = (VideoBw * TransferUnitSize) / LinkBw;

		xdbg_printf(XDBG_DEBUG_GENERAL,"SS:INFO:Checking link "
			"bandwidth validity for SST.\n\r");
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: Link bandwidth = "
			"%ld Kbps and video bandwidth = %ld Kbps\n\r",
				(LinkBw * 1000), VideoBw);

		if (AvgBytesPerTU > (TransferUnitSize * 1000)) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:SST link is "
				"over-subscribed.\n\r");
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:Link "
				"bandwidth = %ld Kbps and video bandwidth = "
				"%ld Kbps\n\r", (LinkBw * 1000), VideoBw);

			return XST_BUFFER_TOO_SMALL;
		}
	}
	else if ((MstCapable == XST_SUCCESS) &&
				(InstancePtr->TxInstance.MstEnable == 1)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:Checking link "
			"bandwidth validity for MST\n\r");

		u8 StreamIndex;
		u32 TimeSlots;
		u32 TotalTimeSlots = 0;
		double PeakPixelBw;
		double Average_StreamSymbolTimeSlotsPerMTP;
		double Target_Average_StreamSymbolTimeSlotsPerMTP;
		double MaximumTarget_Average_StreamSymbolTimeSlotsPerMTP;
		u32 TsInt;
		u32 TsFrac;
		u16 Pbn;

		if (VidMode != XVIDC_VM_CUSTOM){
			PeakPixelBw =
				((double)XVidC_GetPixelClockHzByVmId(VidMode) /
					1000000) * ((double)BitsPerPixel / 8);
		}
		else {
			PeakPixelBw =
				((double)InstancePtr->TxInstance.MsaConfig[
					0].PixelClockHz / 1000000) *
					((double)BitsPerPixel / 8);
		}

		Pbn = 1.006 * PeakPixelBw * ((double)64 / 54);

		if ((double)(1.006 * PeakPixelBw * ((double)64 / 54)) >
							((double)Pbn)) {
			Pbn++;
		}

		Average_StreamSymbolTimeSlotsPerMTP = (64.0 * PeakPixelBw /
								LinkBw);
		MaximumTarget_Average_StreamSymbolTimeSlotsPerMTP = (54.0 *
						((double)Pbn / LinkBw));

		Target_Average_StreamSymbolTimeSlotsPerMTP =
				(u32)Average_StreamSymbolTimeSlotsPerMTP;
		Target_Average_StreamSymbolTimeSlotsPerMTP += ((1.0 / 8.0) *
				(u32)(8.0 *
			(MaximumTarget_Average_StreamSymbolTimeSlotsPerMTP -
				Target_Average_StreamSymbolTimeSlotsPerMTP)));

		TsInt = Target_Average_StreamSymbolTimeSlotsPerMTP;
		TsFrac = (((double)Target_Average_StreamSymbolTimeSlotsPerMTP *
				1000) - (TsInt * 1000));

		TimeSlots = TsInt;
		if (TsFrac != 0) {
			TimeSlots++;
		}
		if ((InstancePtr->Config.PayloadDataWidth == 4) &&
							(TimeSlots % 4) != 0) {
			TimeSlots += (4 - (TimeSlots % 4));
		}
		else if ((TimeSlots % 2) != 0) {
			TimeSlots++;
		}

		/* Add up all the timeslots. */
		for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
			if (XDp_TxMstStreamIsEnabled(InstancePtr,
							StreamIndex + 1)) {
				TotalTimeSlots += TimeSlots;
			}
		}

		if (TotalTimeSlots > 63) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: MST "
				"link over-subscribed.\n\rTotal time slots "
				"required: %ld for %ld streams.\n\rOnly 63 "
				"time slots are available.\n\r",
				TotalTimeSlots, TotalTimeSlots / TimeSlots);

			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function discovers the topology and finds the actual number of sinks.
* It enables streams corresponding to each sink found during topology
* discovery.
*
* @param	InstancePtr is a pointer to the XDp instance.
*
* @return
*		- XST_SUCCESS if topology discovered successfully.
*		- XST_FAILURE if topology discovery failed.
*
* @note		None.
*
******************************************************************************/
static u32 Dp_GetTopology(XDp *InstancePtr)
{
	u32 Status;
	u8 NumStreams;

	/* Clear node and sink */
	InstancePtr->TxInstance.Topology.NodeTotal = 0;
	InstancePtr->TxInstance.Topology.SinkTotal = 0;

	/* Discover topology and find total sinks */
	Status = XDp_TxDiscoverTopology(InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:MST:Topology failed:"
			"%ld.\n\r", Status);
		return XST_FAILURE;
	}

	/* Total number of streams equivalent to number of sinks found */
	NumStreams = InstancePtr->TxInstance.Topology.SinkTotal;
	xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:MST:No of streams based on "
		"topology discovery is = %d\n\r", NumStreams);

	if (NumStreams > InstancePtr->Config.NumMstStreams) {
		NumStreams = InstancePtr->Config.NumMstStreams;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function retrieves preferred timing mode information from the EDID
* to identify video mode or resolution.
*
* @param	EdidPtr is the supplied base EDID to retrieve timing values.
*
* @return	Id of a supported video mode.
*
* @note		None.
*
******************************************************************************/
static XVidC_VideoMode Dp_GetPreferredVm(u8 *EdidPtr)
{
	u8 *Ptm;
	u16 HBlank;
	u16 VBlank;
	u32 PixelClockHz;
	XVidC_FrameRate FrameRate;
	XVidC_VideoTiming Timing;
	XVidC_VideoMode VmId;

	(void)memset((void *)&Timing, 0, sizeof(XVidC_VideoTiming));

	Ptm = &EdidPtr[XDP_EDID_PTM];

	HBlank = ((Ptm[XDP_EDID_DTD_HRES_HBLANK_U4] &
			XDP_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
			Ptm[XDP_EDID_DTD_HBLANK_LSB];

	VBlank = ((Ptm[XDP_EDID_DTD_VRES_VBLANK_U4] &
			XDP_EDID_DTD_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
			Ptm[XDP_EDID_DTD_VBLANK_LSB];

	Timing.HActive = (((Ptm[XDP_EDID_DTD_HRES_HBLANK_U4] &
			XDP_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
			XDP_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			Ptm[XDP_EDID_DTD_HRES_LSB];

	Timing.VActive = (((Ptm[XDP_EDID_DTD_VRES_VBLANK_U4] &
			XDP_EDID_DTD_XRES_XBLANK_U4_XRES_MASK) >>
			XDP_EDID_DTD_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
			Ptm[XDP_EDID_DTD_VRES_LSB];

	PixelClockHz = (((Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_MSB] <<
		8) | Ptm[XDP_EDID_DTD_PIXEL_CLK_KHZ_LSB]) * 10) * 1000;

	Timing.HFrontPorch = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
			XDP_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_MASK) >>
			XDP_EDID_DTD_XFPORCH_XSPW_U2_HFPORCH_SHIFT) << 8) |
			Ptm[XDP_EDID_DTD_HFPORCH_LSB];

	Timing.HSyncWidth = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
			XDP_EDID_DTD_XFPORCH_XSPW_U2_HSPW_MASK) >>
			XDP_EDID_DTD_XFPORCH_XSPW_U2_HSPW_SHIFT) << 8) |
			Ptm[XDP_EDID_DTD_HSPW_LSB];

	Timing.F0PVFrontPorch = (((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
			XDP_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_MASK) >>
			XDP_EDID_DTD_XFPORCH_XSPW_U2_VFPORCH_SHIFT) << 8) |
			((Ptm[XDP_EDID_DTD_VFPORCH_VSPW_L4] &
			XDP_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_MASK) >>
			XDP_EDID_DTD_VFPORCH_VSPW_L4_VFPORCH_SHIFT);

	Timing.F0PVSyncWidth = ((Ptm[XDP_EDID_DTD_XFPORCH_XSPW_U2] &
			XDP_EDID_DTD_XFPORCH_XSPW_U2_VSPW_MASK) << 8) |
			(Ptm[XDP_EDID_DTD_VFPORCH_VSPW_L4] &
			XDP_EDID_DTD_VFPORCH_VSPW_L4_VSPW_MASK);

	/* Compute video mode timing values. */
	Timing.HBackPorch = HBlank - (Timing.HFrontPorch + Timing.HSyncWidth);
	Timing.F0PVBackPorch = VBlank - (Timing.F0PVFrontPorch +
				Timing.F0PVSyncWidth);
	Timing.HTotal = (Timing.HSyncWidth + Timing.HFrontPorch +
			Timing.HActive + Timing.HBackPorch);
	Timing.F0PVTotal = (Timing.F0PVSyncWidth + Timing.F0PVFrontPorch +
				Timing.VActive + Timing.F0PVBackPorch);
	FrameRate = PixelClockHz / (Timing.HTotal * Timing.F0PVTotal);

	xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:"
			"HAct:%d, VAct:%d, FR:%d\n\r", Timing.HActive,
				Timing.VActive, FrameRate);

	/* Few monitors returns 59 HZ. Hence, setting to 60. */
	if (FrameRate == 59) {
		FrameRate = 60;
	}

	/* Get video mode ID */
	VmId = XVidC_GetVideoModeId(Timing.HActive, Timing.VActive,
			FrameRate, XVidC_EdidIsDtdPtmInterlaced(EdidPtr));

	return VmId;
}

/*****************************************************************************/
/**
 *
 * This function configures DisplayPort video packaing clock control bit (
 * VIDEO_PACKING_CLOCK_CONTROL) if bpc is 12/16.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Bpc is the new number of bits per color to use.
 *
 * @note		None.
 *
 *****************************************************************************/
static void Dp_ConfigVideoPackingClockControl(XDp *InstancePtr, u8 Bpc)
{
	if (InstancePtr->Config.PayloadDataWidth == 4 &&
	    InstancePtr->TxInstance.MsaConfig[0].PixelClockHz != 0 &&
	    InstancePtr->TxInstance.MsaConfig[0].UserPixelWidth != 0 &&
	    Bpc > 10) {
		u32 PackingClk =
			(InstancePtr->TxInstance.MsaConfig[0].PixelClockHz /
			 InstancePtr->TxInstance.MsaConfig[0].UserPixelWidth);
		u32 LinkClk;
		long long DpLinkRateHz;

		switch (InstancePtr->TxInstance.LinkConfig.LinkRate) {
		case XDP_TX_LINK_BW_SET_540GBPS:
			DpLinkRateHz = DP_LINK_RATE_HZ_540GBPS;
			break;
		case XDP_TX_LINK_BW_SET_270GBPS:
			DpLinkRateHz = DP_LINK_RATE_HZ_270GBPS;
			break;
		default:
			DpLinkRateHz = DP_LINK_RATE_HZ_162GBPS;
			break;
		}
		/* link clock */
		LinkClk = DpLinkRateHz / InstancePtr->Config.PayloadDataWidth /
			  10;

		/* writing VIDEO_PACKING_CLOCK_CONTROL bit */
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
			     XDP_TX_VIDEO_PACKING_CLOCK_CONTROL,
			     PackingClk < LinkClk);
	}
}
