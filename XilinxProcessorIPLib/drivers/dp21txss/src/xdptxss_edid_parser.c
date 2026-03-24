/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xdptxss_edid_parser.c
 * @addtogroup dptxss Overview
 * @{
 *
 * This file contains EDID timing parsing functions that extract video
 * timings from EDID/DisplayID structures and populate it in priority order.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- --------------------------------------------------
 * 1.0   amd  03/18/26 Initial release
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "stdio.h"
#include "string.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xdptxss.h"
#include "xvidc_edid_ext.h"
#include "xvidc_edid.h"
#include "xdptxss_edid_parser.h"

/**
 * @def EDID_DEBUG
 * @brief Enable verbose EDID parsing debug output (0=disabled, 1=enabled)
 */
#define EDID_DEBUG 0

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function reads the full EDID (base block + all extensions) from
 * the connected monitor, validates header and checksum, and stores it in
 * the AVCaps structure's embedded buffer.
 *
 * @param	InstancePtr is a pointer to the XDpTxSs instance.
 * @param	AVCaps is a pointer to XDpTxSs_Sink_AV_Capabilities structure
 *		where EDID will be stored in AVCaps->EdidBuffer and extension
 *		count in AVCaps->extensions_count.
 *
 * @return	XST_SUCCESS if all EDID blocks read and Base block validated successfully.
 *		XST_FAILURE if instance/structure pointer invalid, monitor not
 *		connected, EDID read fails, or EDID header/checksum invalid.
 *
 * @note	This function validates base EDID header and checksum.
 *		Extension count is limited to MAX_EXTENSIONS_SUPPORTED.
 *		Each EDID block is 128 bytes.
 *
 ******************************************************************************/
int XDpTxSs_get_full_edid(XDpTxSs *InstancePtr, XDpTxSs_Sink_AV_Capabilities *AVCaps)
{
	u8 ExtensionCount;
	u32 Status;
	u8 BlockIndex;
	u8 *BufferPtr;
	u8 i;
	static const u8 EDID_HEADER[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};

	if (InstancePtr == NULL) {
		xil_printf("XDpTxSs_get_full_edid: Invalid instance pointer\r\n");
		return XST_FAILURE;
	}

	if (AVCaps == NULL) {
		xil_printf("XDpTxSs_get_full_edid: Invalid AVCaps pointer\r\n");
		return XST_FAILURE;
	}

	if (!XDpTxSs_IsConnected(InstancePtr)) {
		xil_printf("XDpTxSs_get_full_edid: Monitor not connected\r\n");
		return XST_FAILURE;
	}

	Status = XDp_TxGetEdidBlock(InstancePtr->DpPtr, AVCaps->EdidBuffer, 0);
	if (Status != XST_SUCCESS) {
		xil_printf("XDpTxSs_get_full_edid: Failed to read base EDID block\r\n");
		return XST_FAILURE;
	}

	for (i = 0; i < 8; i++) {
		if (AVCaps->EdidBuffer[i] != EDID_HEADER[i]) {
			xil_printf("XDpTxSs_get_full_edid: Invalid EDID header\r\n");
			return XST_FAILURE;
		}
	}

	if (!xvidc_edid_verify_checksum(AVCaps->EdidBuffer)) {
		xil_printf("XDpTxSs_get_full_edid: Base EDID block checksum failed\r\n");
		return XST_FAILURE;
	}

	ExtensionCount = AVCaps->EdidBuffer[126];
	if (ExtensionCount >= MAX_EXTENSIONS_SUPPORTED) {
		xil_printf("XDpTxSs_get_full_edid: EDID has %d extensions, limiting to %d\r\n",
				ExtensionCount, MAX_EXTENSIONS_SUPPORTED - 1);
		ExtensionCount = MAX_EXTENSIONS_SUPPORTED - 1;
	}
#if EDID_DEBUG
	xil_printf("Reading EDID: Base block + %d extension(s)\r\n", ExtensionCount);
#endif
	for (BlockIndex = 1; BlockIndex <= ExtensionCount; BlockIndex++) {
		BufferPtr = AVCaps->EdidBuffer + (BlockIndex * 128);
		Status = XDp_TxGetEdidBlock(InstancePtr->DpPtr, BufferPtr, BlockIndex);
		if (Status != XST_SUCCESS) {
			xil_printf("XDpTxSs_get_full_edid: Failed to read extension block %d\r\n",
					BlockIndex);
			return XST_FAILURE;
		}
		xil_printf(".");
	}

	AVCaps->extensions_count = ExtensionCount;

	xil_printf("\r\nEDID read complete: %d bytes (%d extension blocks)\r\n",
			(1 + ExtensionCount) * 128, ExtensionCount);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function parses complete Audio/Video capabilities and timings from
 * embedded EDID buffer by aggregating data from base EDID, CTA-861, and
 * DisplayID extension blocks.
 *
 * @param	AVCaps is a pointer to XDpTxSs_Sink_AV_Capabilities structure
 *		with EDID already loaded in AVCaps->EdidBuffer.
 *		On return, AVCaps->checksum_failures will contain the count of
 *		extension blocks with checksum errors (0 if all blocks valid).
 *
 * @return	XST_SUCCESS if all blocks are parsed successfully
 *		XST_FAILURE if any error is encountered
 *
 * @note	EDID must be loaded via XDpTxSs_get_full_edid() before calling.
 *		Maximum SAD_count is limited to MAX_SAD_COUNT.
 *		Maximum timing count is limited to EDID_MAX_TIMINGS.
 *		CTA-861 blocks are optional (success returned if none found).
 *		DisplayID blocks are optional (success returned if none found).
 *		Speaker allocation is OR'd across all CTA-861 blocks.
 *		If arrays fill up, parsing continues but additional entries ignored.
 *		Video timings are parsed by calling XDpTxSs_GetVideoTimingsFromEdid()
 *		internally and stored in AVCaps->TimingList[] in priority order.
 *		Parse statistics from both CTA-861 and DisplayID blocks are
 *		accumulated and reported in the summary.
 *
 ******************************************************************************/
int XDpTxSs_get_sink_AV_Capabilities(XDpTxSs_Sink_AV_Capabilities *AVCaps)
{
	u8 ExtensionCount;
	u8 BlockIndex;
	u8 *BlockPtr;
	u8 ExtensionTag;
	u8 DtdOffset;
	u8 DataBlockOffset;
	u8 TagLength;
	u8 TagCode;
	u8 BlockLength;
	u8 TotalSadIndex = 0;
	u8 SadIndex;
	u8 *SadPtr;
	u8 Byte0, Byte1, Byte2;
	u8 bpc_code;
	u8 feature_byte;
	u8 cta_byte3;
	u8 SpeakerAlloc;
	XDpTxSs_AudioSAD *SadArray;
	u8 *EdidBuffer;
	u8 ValidBlocks;

	XDpTxSs_ParseResult CtaResult = {0, 0};
	XDpTxSs_ParseResult TimingResult;
	XDpTxSs_ParseResult TotalResult = {0, 0};

	if (AVCaps == NULL) {
		xil_printf("XDpTxSs_get_sink_AV_Capabilities: Invalid structure pointer\r\n");
		return XST_FAILURE;
	}

	EdidBuffer = AVCaps->EdidBuffer;

	AVCaps->checksum_failures = 0;
	SadArray = AVCaps->SinkAudioCfg.SAD_array;
	AVCaps->SinkAudioCfg.Speaker_channel_allocation = 0;
	AVCaps->SinkAudioCfg.SAD_count = 0;
	AVCaps->MaxBpc = 0;
	AVCaps->SupportedColorFormats = 0x01;  /* RGB 4:4:4 is always supported */


	bpc_code = (EdidBuffer[20] >> 4) & 0x07;
	switch (bpc_code) {
		case 1: AVCaps->MaxBpc = 6;  break;
		case 2: AVCaps->MaxBpc = 8;  break;
		case 3: AVCaps->MaxBpc = 10; break;
		case 4: AVCaps->MaxBpc = 12; break;
		case 5: AVCaps->MaxBpc = 14; break;
		case 6: AVCaps->MaxBpc = 16; break;
		default:
			xil_printf("Unsupported BPC in EDID\r\n");
			AVCaps->MaxBpc = 0;
			break;
	}

	feature_byte = EdidBuffer[24];
	if (feature_byte & 0x10) {  /* Bit 4 */
		AVCaps->SupportedColorFormats |= 0x04;  /* YCbCr 4:2:2 */
	}
	if (feature_byte & 0x08) {  /* Bit 3 */
		AVCaps->SupportedColorFormats |= 0x02;  /* YCbCr 4:4:4 */
	}

	ExtensionCount = AVCaps->extensions_count;

	if (ExtensionCount == 0) {
		xil_printf("No EDID extension blocks found \r\n");
		return XST_SUCCESS;  /* Success: Extension blocks are optional */
	}


	for (BlockIndex = 1; BlockIndex <= ExtensionCount; BlockIndex++) {
		BlockPtr = EdidBuffer + (BlockIndex * 128);
		ExtensionTag = BlockPtr[0];

		if (ExtensionTag != CTA861_EXTENSION_TAG) {
			continue;
		}

		CtaResult.TotalBlockCount++;
		xil_printf("\r\nFound CTA-861 block %d (EDID block %d)\r\n", CtaResult.TotalBlockCount, BlockIndex);

		if (!xvidc_edid_verify_checksum(BlockPtr)) {
			xil_printf("  Checksum FAILED - skipping block\r\n");
			CtaResult.InvalidBlockCount++;
			continue;
		}

		cta_byte3 = BlockPtr[3];
		if (cta_byte3 & 0x20) {  /* Bit 5 */
			AVCaps->SupportedColorFormats |= 0x02;  /* YCbCr 4:4:4 */
		}
		if (cta_byte3 & 0x10) {  /* Bit 4 */
			AVCaps->SupportedColorFormats |= 0x04;  /* YCbCr 4:2:2 */
		}

		DtdOffset = BlockPtr[2];
		DataBlockOffset = 4;
#if EDID_DEBUG
		xil_printf("  Parsing data blocks (offset 4 to %d)...\r\n", DtdOffset - 1);
#endif
		while (DataBlockOffset < DtdOffset && DataBlockOffset < 127) {
			TagLength = BlockPtr[DataBlockOffset];
			TagCode = (TagLength >> 5) & 0x07;
			BlockLength = TagLength & 0x1F;

			if (DataBlockOffset + 1 + BlockLength > 127) {
				xil_printf("  Warning: Data block exceeds boundary, stopping parse\r\n");
				break;
			}

			if (TagCode == CTA861_AUDIO_DATA_BLOCK) {
				u8 SadCount = BlockLength / 3;
				xil_printf("  Audio data block found: %d bytes, %d SAD(s)\r\n",
						BlockLength, SadCount);

				for (SadIndex = 0; SadIndex < SadCount; SadIndex++) {
					if (TotalSadIndex >= MAX_SAD_COUNT) {
						xil_printf("  Warning: SAD array full (%d), additional SADs ignored\r\n",
								MAX_SAD_COUNT);
						break;
					}

					SadPtr = &BlockPtr[DataBlockOffset + 1 + (SadIndex * 3)];

					Byte0 = SadPtr[0];
					Byte1 = SadPtr[1];
					Byte2 = SadPtr[2];

					SadArray[TotalSadIndex].AudioFormatCode =
						(Byte0 >> SAD_AUDIO_FORMAT_SHIFT) & SAD_AUDIO_FORMAT_MASK;

					SadArray[TotalSadIndex].MaxChannels =
						(Byte0 & SAD_MAX_CHANNELS_MASK) + 1;

					SadArray[TotalSadIndex].SamplingFreqFlags =
						Byte1 & SAD_SAMPLING_FREQ_MASK;

					if (SadArray[TotalSadIndex].AudioFormatCode == 1) {
						SadArray[TotalSadIndex].FormatDependent.BitDepthFlags =
							Byte2 & SAD_BIT_DEPTH_MASK;
					} else {
						SadArray[TotalSadIndex].FormatDependent.MaxBitrate =
							Byte2 & SAD_MAX_BITRATE_MASK;
					}
#if EDID_DEBUG
					xil_printf("    SAD[%d]: Format=%d, Channels=%d, SampFreq=0x%02X, Dependent=0x%02X\r\n",
							TotalSadIndex,
							SadArray[TotalSadIndex].AudioFormatCode,
							SadArray[TotalSadIndex].MaxChannels,
							SadArray[TotalSadIndex].SamplingFreqFlags,
							(SadArray[TotalSadIndex].AudioFormatCode == 1) ?
							SadArray[TotalSadIndex].FormatDependent.BitDepthFlags :
							SadArray[TotalSadIndex].FormatDependent.MaxBitrate);
#endif
					TotalSadIndex++;
				}
			}
			else if (TagCode == CTA861_SPKR_ALLOC_DATA_BLOCK && BlockLength > 0) {
				SpeakerAlloc = BlockPtr[DataBlockOffset + 1];
				AVCaps->SinkAudioCfg.Speaker_channel_allocation |= SpeakerAlloc;
				xil_printf("  Speaker Allocation Data Block found: ChannelAlloc=0x%02X (cumulative=0x%02X)\r\n",
						SpeakerAlloc, AVCaps->SinkAudioCfg.Speaker_channel_allocation);
			}

			DataBlockOffset += (BlockLength + 1);
		}
	}

	AVCaps->SinkAudioCfg.SAD_count = TotalSadIndex;

	TimingResult = XDpTxSs_GetVideoTimingsFromEdid(AVCaps);

	/* Accumulate: Base + CTA + DisplayID */
	TotalResult.TotalBlockCount = 1 +                           /* Base block (always valid) */
	                              CtaResult.TotalBlockCount +    /* CTA blocks */
	                              TimingResult.TotalBlockCount;  /* DisplayID blocks */

	TotalResult.InvalidBlockCount = CtaResult.InvalidBlockCount +    /* CTA invalid */
	                                TimingResult.InvalidBlockCount;  /* DisplayID invalid */

	ValidBlocks = TotalResult.TotalBlockCount - TotalResult.InvalidBlockCount;

	AVCaps->checksum_failures = TotalResult.InvalidBlockCount;

	xil_printf("\r\n=== A/V Capabilities Parse Summary ===\r\n");
	xil_printf("  Total Blocks: %d (%d valid, %d invalid)\r\n",
		TotalResult.TotalBlockCount, ValidBlocks, TotalResult.InvalidBlockCount);
	xil_printf("  CTA-861 Blocks: %d\r\n", CtaResult.TotalBlockCount);
	xil_printf("  DisplayID Blocks: %d\r\n", TimingResult.TotalBlockCount);
	xil_printf("  SADs Found: %d\r\n", AVCaps->SinkAudioCfg.SAD_count);
	xil_printf("  Speaker Allocation: 0x%02X\r\n", AVCaps->SinkAudioCfg.Speaker_channel_allocation);
	xil_printf("  Max BPC: %d\r\n", AVCaps->MaxBpc);
	xil_printf("  Color Formats: 0x%02X\r\n", AVCaps->SupportedColorFormats);
	xil_printf("  Video Timings: %d\r\n", AVCaps->TimingCount);
	xil_printf("======================================\r\n\r\n");

	if (TotalResult.InvalidBlockCount > 0) {
		return XST_FAILURE;  /* Any invalid blocks = failure */
	}

	return XST_SUCCESS;  /* All blocks valid */
}


/*****************************************************************************/
/**
 *
 * Helper function to fill established timing with known values and zero-fill
 * unavailable parameters.
 *
 * @param    Timing is the XDpTxSs_VideoTimingParam structure to populate.
 * @param    HActive is the horizontal active pixels.
 * @param    VActive is the vertical active lines.
 * @param    RefreshRate is the refresh rate in Hz.
 *
 ******************************************************************************/
static void XDpTxSs_FillEstablishedTiming(XDpTxSs_VideoTimingParam *Timing,
		u16 HActive, u16 VActive, u8 RefreshRate)
{
	/* Fill known values from macro name */
	Timing->HActive = HActive;
	Timing->VActive = VActive;
	Timing->RefreshRate = RefreshRate;

	/* Fill metadata */
	Timing->TimingSource = TIMING_SOURCE_ESTABLISHED;

	/* Zero-fill all blanking parameters (not in EDID) */
	Timing->AspectRatio = 0;
	Timing->HFrontPorch = 0;
	Timing->HSyncWidth = 0;
	Timing->HBackPorch = 0;
	Timing->HTotal = 0;
	Timing->F0PVFrontPorch = 0;
	Timing->F0PVSyncWidth = 0;
	Timing->F0PVBackPorch = 0;
	Timing->F0PVTotal = 0;
	Timing->HSyncPolarity = 0;
	Timing->VSyncPolarity = 0;
	Timing->F1VFrontPorch = 0;
	Timing->F1VSyncWidth = 0;
	Timing->F1VBackPorch = 0;
	Timing->F1VTotal = 0;
	Timing->PixelClockHz = 0;
}

/*****************************************************************************/
/**
 * Helper function to fill standard timing with known values and zero-fill
 * unavailable parameters.
 *
 * @param    Timing is the XDpTxSs_VideoTimingParam structure to populate.
 * @param    HActive is the horizontal active pixels.
 * @param    VActive is the vertical active lines.
 * @param    AspectRatio is the aspect ratio code (0-3).
 * @param    RefreshRate is the refresh rate in Hz.
 *
 ******************************************************************************/
static void XDpTxSs_FillStandardTiming(XDpTxSs_VideoTimingParam *Timing,
		u16 HActive, u16 VActive,
		u8 AspectRatio, u8 RefreshRate)
{
	/* Fill known values from EDID */
	Timing->HActive = HActive;
	Timing->VActive = VActive;
	Timing->AspectRatio = AspectRatio;
	Timing->RefreshRate = RefreshRate;

	/* Fill metadata */
	Timing->TimingSource = TIMING_SOURCE_STANDARD;

	/* Zero-fill all blanking parameters (not in EDID) */
	Timing->HFrontPorch = 0;
	Timing->HSyncWidth = 0;
	Timing->HBackPorch = 0;
	Timing->HTotal = 0;
	Timing->F0PVFrontPorch = 0;
	Timing->F0PVSyncWidth = 0;
	Timing->F0PVBackPorch = 0;
	Timing->F0PVTotal = 0;
	Timing->HSyncPolarity = 0;
	Timing->VSyncPolarity = 0;
	Timing->F1VFrontPorch = 0;
	Timing->F1VSyncWidth = 0;
	Timing->F1VBackPorch = 0;
	Timing->F1VTotal = 0;
	Timing->PixelClockHz = 0;
}

/*****************************************************************************/
/**
 *
 * This function converts XV_VidC_TimingParam to XDpTxSs_VideoTimingParam format.
 *
 * @param    TimingParam is the source timing parameter structure.
 * @param    VideoTiming is the destination XDpTxSs_VideoTimingParam structure.
 *
 ******************************************************************************/
static void XDpTxSs_ConvertTimingParamToVideoTiming(
		const XV_VidC_TimingParam *TimingParam,
		XDpTxSs_VideoTimingParam *VideoTiming)
{
	/* Horizontal timing */
	VideoTiming->HActive = TimingParam->hres;
	VideoTiming->HFrontPorch = TimingParam->hfp;
	VideoTiming->HSyncWidth = TimingParam->hsync_width;
	VideoTiming->HBackPorch = TimingParam->htotal - TimingParam->hres -
		TimingParam->hfp - TimingParam->hsync_width;
	VideoTiming->HTotal = TimingParam->htotal;
	VideoTiming->HSyncPolarity = TimingParam->hsync_polarity;

	/* Vertical timing - progressive */
	VideoTiming->VActive = TimingParam->vres;
	VideoTiming->F0PVFrontPorch = TimingParam->vfp;
	VideoTiming->F0PVSyncWidth = TimingParam->vsync_width;
	VideoTiming->F0PVBackPorch = TimingParam->vtotal - TimingParam->vres -
		TimingParam->vfp - TimingParam->vsync_width;
	VideoTiming->F0PVTotal = TimingParam->vtotal;
	VideoTiming->VSyncPolarity = TimingParam->vsync_polarity;
	VideoTiming->PixelClockHz = TimingParam->pixclk;

	/* Interlaced handling */
	if (TimingParam->vidfrmt == XVIDC_VF_INTERLACED) {
		VideoTiming->VActive = VideoTiming->VActive * 2;
		VideoTiming->F1VFrontPorch = VideoTiming->F0PVFrontPorch;
		VideoTiming->F1VSyncWidth  = VideoTiming->F0PVSyncWidth;
		VideoTiming->F1VBackPorch  = VideoTiming->F0PVBackPorch;
		VideoTiming->F1VTotal      = VideoTiming->F0PVTotal;

	} else {
		VideoTiming->F1VFrontPorch = 0;
		VideoTiming->F1VSyncWidth = 0;
		VideoTiming->F1VBackPorch = 0;
		VideoTiming->F1VTotal = 0;
	}
	if (VideoTiming->HTotal && VideoTiming->F0PVTotal) {
		VideoTiming->RefreshRate = (u32)(VideoTiming->PixelClockHz /
				((u64)VideoTiming->HTotal * VideoTiming->F0PVTotal));
	}

	VideoTiming->TimingSource = TIMING_SOURCE_DTD;
	VideoTiming->AspectRatio = 0;
}

/*****************************************************************************/
/**
 *
 * This function validates all DisplayID 2.0 extension blocks by checking
 * checksums. Called once to count total and invalid DisplayID blocks.
 *
 * @param    AVCaps is a pointer to the AV capabilities structure with EDID
 *           in AVCaps->EdidBuffer.
 *
 * @return   XDpTxSs_ParseResult with TotalBlockCount and InvalidBlockCount.
 *
 ******************************************************************************/
static XDpTxSs_ParseResult XDpTxSs_ValidateDisplayId2_0Blocks(
		XDpTxSs_Sink_AV_Capabilities *AVCaps)
{
	XDpTxSs_ParseResult Result = {0, 0};
	u8 ExtensionCount = AVCaps->extensions_count;
	u8 BlockIndex;
	const u8 *EdidRaw = AVCaps->EdidBuffer;

	for (BlockIndex = 1; BlockIndex <= ExtensionCount; BlockIndex++) {
		const u8 *BlockPtr = EdidRaw + (BlockIndex * 128);

		if (BlockPtr[0] != DISPLAYID_2_0_EXTENSION_TAG)
			continue;

		Result.TotalBlockCount++;

		if (!xvidc_edid_verify_checksum(BlockPtr)) {
			Result.InvalidBlockCount++;
		}
	}

	return Result;
}

/*****************************************************************************/
/**
 *
 * This function parses DisplayID 2.0 extension blocks and extracts Type VII
 * detailed timings based on the specified category filter (Preferred/Native/Remaining).
 * This function assumes that blocks have been pre-validated.
 *
 * @param    AVCaps is a pointer to the AV capabilities structure with EDID
 *           in AVCaps->EdidBuffer. Timings will be stored in AVCaps->TimingList[]
 *           and AVCaps->TimingCount is updated with the number of timings added.
 * @param    Category specifies which timing category to extract:
 *           - DISPLAYID_TYPE7_PREFERRED: Timings with Preferred flag (Options bit 7)
 *           - DISPLAYID_TYPE7_NATIVE: Timings with Native flag (Options bit 6, Preferred clear)
 *           - DISPLAYID_TYPE7_REMAINING: All other timings (both flags clear)
 *
 * @return   XST_SUCCESS if parsing completed.
 *           XST_FAILURE if AVCaps pointer is invalid.
 *
 * @note     This function does NOT validate checksums - caller must validate first.
 *           DisplayID 2.0 Type VII Timing Descriptor Structure (20 bytes)
 ******************************************************************************/
static int XDpTxSs_ParseDisplayIdType7(
		XDpTxSs_Sink_AV_Capabilities *AVCaps,
		XDpTxSs_DisplayIdType7Category Category)
{
	u8 ExtensionCount;
	u8 BlockIndex;
	const u8 *EdidRaw;

	if (AVCaps == NULL) {
		return XST_FAILURE;
	}

	ExtensionCount = AVCaps->extensions_count;
	EdidRaw = AVCaps->EdidBuffer;

	for (BlockIndex = 1; BlockIndex <= ExtensionCount &&
			AVCaps->TimingCount < EDID_MAX_TIMINGS; BlockIndex++) {

		const u8 *BlockPtr = EdidRaw + (BlockIndex * 128);

		if (BlockPtr[0] != DISPLAYID_2_0_EXTENSION_TAG)
			continue;

		u8 SectionLength = BlockPtr[2];
		u8 DataOffset = 5;

		while (DataOffset < (SectionLength + 5) &&
				DataOffset < 124 &&
				AVCaps->TimingCount < EDID_MAX_TIMINGS) {

			u8 BlockTag = BlockPtr[DataOffset];
			u8 PayloadLength = BlockPtr[DataOffset + 2];

			if (DataOffset + 3 + PayloadLength > 127)
				break;

			if (BlockTag == DISPLAYID_TYPE_VII_TAG) {
				u8 NumTimings = PayloadLength / 20;  /* Type VII is 20 bytes per timing */
				u8 TimingOffset = DataOffset + 3;
				u8 t;

				for (t = 0; t < NumTimings &&
						AVCaps->TimingCount < EDID_MAX_TIMINGS; t++) {

					const u8 *TimingPtr = &BlockPtr[TimingOffset + (t * 20)];
					u8 Options = TimingPtr[3];
					u8 IsPreferred = (Options & 0x80) ? 1 : 0;
					u8 IsNative = (Options & 0x40) ? 1 : 0;

					/* Filter based on requested category */
					switch (Category) {
						case DISPLAYID_TYPE7_PREFERRED:
							if (!IsPreferred)
								continue;
							break;
						case DISPLAYID_TYPE7_NATIVE:
							if (!IsNative)
								continue;
							break;
						case DISPLAYID_TYPE7_REMAINING:
							if (IsPreferred || IsNative)
								continue;
							break;
						default:
							continue;
					}

					u32 PixClk1kHz = TimingPtr[0] | (TimingPtr[1] << 8) | (TimingPtr[2] << 16);
					u16 HActive = (TimingPtr[4] | (TimingPtr[5] << 8)) + 1;
					u16 HBlank = (TimingPtr[6] | (TimingPtr[7] << 8)) + 1;
					u16 HFrontPorchRaw = TimingPtr[8] | (TimingPtr[9] << 8);
					u16 HSyncWidth = (TimingPtr[10] | (TimingPtr[11] << 8)) + 1;
					u16 VActive = (TimingPtr[12] | (TimingPtr[13] << 8)) + 1;
					u16 VBlank = (TimingPtr[14] | (TimingPtr[15] << 8)) + 1;
					u16 VFrontPorchRaw = TimingPtr[16] | (TimingPtr[17] << 8);
					u16 VSyncWidth = TimingPtr[18] + 1;
					u8 SyncFlags = TimingPtr[19];

					XDpTxSs_VideoTimingParam *Timing = &AVCaps->TimingList[AVCaps->TimingCount];

					Timing->HActive = HActive;
					Timing->HFrontPorch = (HFrontPorchRaw & 0x7FFF) + 1;
					Timing->HSyncWidth = HSyncWidth;
					Timing->HBackPorch = HBlank - Timing->HFrontPorch - HSyncWidth;
					Timing->HTotal = HActive + HBlank;
					Timing->HSyncPolarity = (HFrontPorchRaw & 0x8000) ? 1 : 0;

					Timing->VActive = VActive;
					Timing->F0PVFrontPorch = (VFrontPorchRaw & 0x7FFF) + 1;
					Timing->F0PVSyncWidth = VSyncWidth;
					Timing->F0PVBackPorch = VBlank - Timing->F0PVFrontPorch - VSyncWidth;
					Timing->F0PVTotal = VActive + VBlank;
					Timing->VSyncPolarity = (VFrontPorchRaw & 0x8000) ? 1 : 0;

					Timing->PixelClockHz = (u64)PixClk1kHz * 1000ULL;

					if (SyncFlags & 0x10) {
						Timing->F1VFrontPorch = Timing->F0PVFrontPorch;
						Timing->F1VSyncWidth = Timing->F0PVSyncWidth;
						Timing->F1VBackPorch = Timing->F0PVBackPorch;

						Timing->F1VTotal = Timing->F0PVTotal / 2;
						Timing->F0PVTotal = Timing->F0PVTotal / 2;
					} else {

						Timing->F1VFrontPorch = 0;
						Timing->F1VSyncWidth = 0;
						Timing->F1VBackPorch = 0;
						Timing->F1VTotal = 0;
					}
					if (Timing->HTotal && Timing->F0PVTotal) {
						Timing->RefreshRate = (u32)(Timing->PixelClockHz /
								((u64)Timing->HTotal * Timing->F0PVTotal));
					}

					Timing->TimingSource = TIMING_SOURCE_DTD;
					AVCaps->TimingCount++;
				}
			}

			DataOffset += (3 + PayloadLength);
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function extracts video timings from an EDID structure and populates
 * the timing list within an AV capabilities structure in priority order.
 * Index 0 contains the highest priority timing (preferred timing) from the sink.
 *
 * Priority order (EdidTimingPriority enum):
 *   1. PRIORITY_DISPLAYID_PREFERRED     - DisplayID 2.0 Type VII Preferred
 *   2. PRIORITY_BASE_DTD1_PREFERRED     - Base EDID DTD #1 (EDID spec mandated preferred)
 *   3. PRIORITY_DISPLAYID_NATIVE        - DisplayID 2.0 Type VII Native
 *   4. PRIORITY_DISPLAYID_DTD_OTHER     - DisplayID 2.0 Type VII (remaining DTDs)
 *   5. PRIORITY_BASE_DTD_OTHER          - Base EDID DTD #2-4
 *   6. PRIORITY_STANDARD_TIMINGS        - Base EDID Standard Timings
 *   7. PRIORITY_ESTABLISHED_TIMINGS     - Base EDID Established Timings
 *
 * @param    AVCaps is a pointer to the XDpTxSs_Sink_AV_Capabilities structure
 *           with EDID loaded in AVCaps->EdidBuffer. Timings are stored in
 *           AVCaps->TimingList[] (max EDID_MAX_TIMINGS) and count in
 *           AVCaps->TimingCount.
 *
 * @return   XDpTxSs_ParseResult structure with parse statistic
 *
 * @note     This function is typically called internally by
 *           XDpTxSs_get_sink_AV_Capabilities(). The caller must ensure the base
 *           EDID header and checksum have been validated before calling.
 *           The AVCaps structure must be allocated by the caller.
 *
 ******************************************************************************/
XDpTxSs_ParseResult XDpTxSs_GetVideoTimingsFromEdid(XDpTxSs_Sink_AV_Capabilities *AVCaps)
{
	XDpTxSs_ParseResult Result = {0, 0};
	XV_VidC_TimingParam TimingParam;
	u8 ValidDisplayIdBlocks;
	u8 i;
	u16 HActive, VActive;
	u8 RefreshRate;
	const u8 *EdidRaw;

	if (AVCaps == NULL) {
		return Result;
	}

	EdidRaw = AVCaps->EdidBuffer;
	AVCaps->TimingCount = 0;

	Result = XDpTxSs_ValidateDisplayId2_0Blocks(AVCaps);
	ValidDisplayIdBlocks = Result.TotalBlockCount - Result.InvalidBlockCount;

	/*************************************************************************
	 * Priority 1: DisplayID 2.0 Type VII Preferred Timing
	 *************************************************************************/
	if (ValidDisplayIdBlocks > 0) {
		XDpTxSs_ParseDisplayIdType7(AVCaps, DISPLAYID_TYPE7_PREFERRED);
	}

	/*************************************************************************
	 * Priority 2: Base EDID DTD #1 (Preferred Timing)
	 *************************************************************************/
	if (EdidRaw[XVIDC_EDID_PTM + XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_LSB] != 0 ||
			EdidRaw[XVIDC_EDID_PTM + XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_MSB] != 0) {
		struct xvidc_edid_detailed_timing_descriptor dtd;

		const u8 *dtd_bytes = &EdidRaw[XVIDC_EDID_PTM];
		(void)memcpy(&dtd, dtd_bytes, sizeof(dtd));

		TimingParam = XV_VidC_timing(&dtd);

		XDpTxSs_ConvertTimingParamToVideoTiming(&TimingParam,
				&AVCaps->TimingList[AVCaps->TimingCount++]);
	}

	/*************************************************************************
	 * Priority 3: DisplayID 2.0 Type VII Native Timing
	 *************************************************************************/
	if (ValidDisplayIdBlocks > 0) {
		XDpTxSs_ParseDisplayIdType7(AVCaps, DISPLAYID_TYPE7_NATIVE);
	}

	/*************************************************************************
	 * Priority 4: DisplayID 2.0 Type VII Remaining DTDs
	 *************************************************************************/
	if (ValidDisplayIdBlocks > 0) {
		XDpTxSs_ParseDisplayIdType7(AVCaps, DISPLAYID_TYPE7_REMAINING);
	}

	/*************************************************************************
	 * Priority 5: Additional Base EDID DTDs (DTD #2, #3, #4)
	 *************************************************************************/
	for (i = 1; i < 4 && AVCaps->TimingCount < EDID_MAX_TIMINGS; i++) {
		u16 DtdOffset = XVIDC_EDID_PTM + (i * EDID_DTD_SIZE);

		if (EdidRaw[DtdOffset + XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_LSB] != 0 ||
				EdidRaw[DtdOffset + XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_MSB] != 0) {
			struct xvidc_edid_detailed_timing_descriptor dtd;
			const u8 *dtd_bytes = &EdidRaw[DtdOffset];
			(void)memcpy(&dtd, dtd_bytes, sizeof(dtd));

			TimingParam = XV_VidC_timing(&dtd);

			XDpTxSs_ConvertTimingParamToVideoTiming(&TimingParam,
					&AVCaps->TimingList[AVCaps->TimingCount++]);
		}
	}

	/*************************************************************************
	 * Priority 6: Standard Timings (8 entries)
	 *************************************************************************/
	for (i = 1; i <= 8 && AVCaps->TimingCount < EDID_MAX_TIMINGS; i++) {
		u8 offset = XVIDC_EDID_STD_TIMINGS_H(i);
		if (EdidRaw[offset] == 0x01 && EdidRaw[offset + 1] == 0x01) {
			continue; /* Skip unused entries */
		}

		HActive = XVidC_EdidGetStdTimingsH(EdidRaw, i);
		VActive = XVidC_EdidGetStdTimingsV(EdidRaw, i);
		RefreshRate = XVidC_EdidGetStdTimingsFrr(EdidRaw, i);
		u8 AspectRatio = XVidC_EdidGetStdTimingsAr(EdidRaw, i);

		if (HActive == 0 || VActive == 0 || RefreshRate == 0) {
			continue;
		}

		XDpTxSs_FillStandardTiming(&AVCaps->TimingList[AVCaps->TimingCount++],
				HActive, VActive, AspectRatio, RefreshRate);
	}

	/*************************************************************************
	 * Priority 7: Established Timings (17 total)
	 *************************************************************************/

	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings720x400_70(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 720, 400, 70);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings720x400_88(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 720, 400, 88);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings640x480_60(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 640, 480, 60);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings640x480_67(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 640, 480, 67);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings640x480_72(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 640, 480, 72);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings640x480_75(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 640, 480, 75);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings800x600_56(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 800, 600, 56);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings800x600_60(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 800, 600, 60);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings800x600_72(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 800, 600, 72);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings800x600_75(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 800, 600, 75);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings832x624_75(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 832, 624, 75);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings1024x768_87(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 1024, 768, 87);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings1024x768_60(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 1024, 768, 60);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings1024x768_70(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 1024, 768, 70);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings1024x768_75(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 1024, 768, 75);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings1280x1024_75(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 1280, 1024, 75);
	}
	if (AVCaps->TimingCount < EDID_MAX_TIMINGS && XVidC_EdidSuppEstTimings1152x870_75(EdidRaw)) {
		XDpTxSs_FillEstablishedTiming(&AVCaps->TimingList[AVCaps->TimingCount++], 1152, 870, 75);
	}


	xil_printf("EDID Parse Summary: %d timings from %d DisplayID blocks (%d valid, %d invalid)\r\n",
			AVCaps->TimingCount, Result.TotalBlockCount,
			ValidDisplayIdBlocks, Result.InvalidBlockCount);

	return Result;
}

/*****************************************************************************/
/**
 *
 * This function prints debug information about a video timing.
 *
 * @param    Timing is the video timing structure to print.
 * @param    Index is the index number to display.
 *
 ******************************************************************************/
void XDpTxSs_PrintVideoTiming(const XDpTxSs_VideoTimingParam *Timing, u8 Index)
{
	xil_printf("Timing[%d]: %dx%d ", Index,
			Timing->HActive, Timing->VActive);
	xil_printf("PixClk=%llu Hz ", Timing->PixelClockHz);
	xil_printf("HTotal=%d VTotal=%d ", Timing->HTotal, Timing->F0PVTotal);
	xil_printf("HFP=%d HSW=%d HBP=%d ",
			Timing->HFrontPorch, Timing->HSyncWidth, Timing->HBackPorch);
	xil_printf("VFP=%d VSW=%d VBP=%d ",
			Timing->F0PVFrontPorch, Timing->F0PVSyncWidth,
			Timing->F0PVBackPorch);
	xil_printf("HPol=%d VPol=%d ",
			Timing->HSyncPolarity, Timing->VSyncPolarity);
	xil_printf("Src=%d AR=%d RR=%d\r\n",
			Timing->TimingSource, Timing->AspectRatio, Timing->RefreshRate);
}
/** @} */
