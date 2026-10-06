/*******************************************************************************
* Copyright (C) 2023-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xdp21_dsc_tu.h"

/*
 * The transmit register definitions this file reads exist only when the design
 * instantiates a transmit subsystem, so a receive-only design has to drop the
 * whole file rather than just the compression support.
 */
#if defined(XDP_USE_DSC) && XPAR_XDPTXSS_NUM_INSTANCES

#include "xdp_hw.h"
#include "xdebug.h"

/*****************************************************************************/
/**
 * Per AMD DSC SST guideline (1.4 link rates) and DP 2.1 §3.5.1.5, a
 * (chunk_size * num_of_slices) / (3 * PPC) that isn't a whole number
 * ("unaligned with PPC") used to be rejected outright here. That is no
 * longer necessary: XDp_TxCalcDscMsaTu() below already carries a
 * NumDummy/NumEoc per-lane padding scheme -- CompBytesPerLine =
 * (ChunkSize + NumDummy + NumEoc) * NumSlices -- which accommodates a
 * non-integer chunk/PPC ratio in the TU/bandwidth math. Rejecting the PPS
 * here pre-empted a case the real link-layer math already tolerates, so
 * this is now informational only; only a structurally invalid PPS
 * (missing dimensions or a non-whole slice count) is still fatal.
 *
 * Returns XST_SUCCESS if PPS is supported, else XST_NO_FEATURE.
 */
static int XDp_TxValidateDscPps(const XDp21_DscPps *Pps, u8 Ppc)
{
	u32 NumSlices;
	u32 Numerator;
	u32 Denominator;

	if ((Pps->slice_width == 0U) || (Pps->chunk_size == 0U) ||
	    (Pps->bits_per_component == 0U) || (Pps->pic_width == 0U)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "DSC PPS: invalid PPS dimensions "
			    "(pic_w=%u slice_w=%u chunk=%u bpc=%u)\r\n",
			    (unsigned int)Pps->pic_width,
			    (unsigned int)Pps->slice_width,
			    (unsigned int)Pps->chunk_size,
			    (unsigned int)Pps->bits_per_component);
		return XST_FAILURE;
	}

	if ((Pps->pic_width % Pps->slice_width) != 0U) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "DSC PPS unsupported: pic_width(%u) "
			    "not divisible by slice_width(%u)\r\n",
			    (unsigned int)Pps->pic_width,
			    (unsigned int)Pps->slice_width);
		return XST_NO_FEATURE;
	}

	if (Ppc == 0U) {
		Ppc = 1U;
	}

	NumSlices   = (u32)Pps->pic_width / (u32)Pps->slice_width;
	Numerator   = (u32)Pps->chunk_size * NumSlices;
	Denominator = 3U * (u32)Ppc;

	if ((Denominator == 0U) || ((Numerator % Denominator) != 0U)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "DSC PPS: (chunk_size*num_slices)/(3*PPC) is not "
			    "integer (chunk=%u slices=%u PPC=%u) -- accepted, "
			    "TU dummy/EOC padding covers the remainder\r\n",
			    (unsigned int)Pps->chunk_size,
			    (unsigned int)NumSlices, (unsigned int)Ppc);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
void XDp_TxSetDscStreamEnable(XDp *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	if (Enable == 0U) {
		InstancePtr->TxInstance.DscStreamEnable = 0U;
		InstancePtr->TxInstance.DscPpsValid = 0U;
	} else {
		InstancePtr->TxInstance.DscStreamEnable = 1U;
	}
}

/*****************************************************************************/
int XDp_TxSetDscStreamPps(XDp *InstancePtr, const u8 *PpsData)
{
	int Status;
	u8 Ppc;

	if ((InstancePtr == NULL) || (PpsData == NULL)) {
		return XST_INVALID_PARAM;
	}
	if (XDp_GetCoreType(InstancePtr) != XDP_TX) {
		return XST_FAILURE;
	}

	Status = XDp21_DscParsePps(PpsData, &InstancePtr->TxInstance.DscPps);
	if (Status != XST_SUCCESS) {
		InstancePtr->TxInstance.DscPpsValid = 0U;
		return Status;
	}

	Ppc = (u8)XDp_ReadReg(InstancePtr->Config.BaseAddr,
			      XDP_TX_USER_PIXEL_WIDTH);
	if ((Ppc != 1U) && (Ppc != 2U) && (Ppc != 4U) && (Ppc != 8U)) {
		Ppc = 1U;
	}

	Status = XDp_TxValidateDscPps(&InstancePtr->TxInstance.DscPps, Ppc);
	if (Status != XST_SUCCESS) {
		InstancePtr->TxInstance.DscPpsValid = 0U;
		return Status;
	}

	InstancePtr->TxInstance.DscPpsValid = 1U;
	return XST_SUCCESS;
}

/*****************************************************************************/
int XDp_TxCalcDscMsaTu(XDp_TxMainStreamAttributes *MsaConfig,
		       const XDp21_DscPps *Pps,
		       u8 LinkRate,
		       u8 LaneCount)
{
	u32 NumSlices;
	u32 ChunkSize;
	u32 LaneCount_u;
	u32 NumDummy;
	u32 NumEoc;
	u32 CompBytesPerLine;
	u32 BitsPerPixel;
	u32 UncompBytesPerLine;
	u32 WplBits;
	u32 WordsPerLine;
	u32 UncompPixClkKHz;
	u32 CompPixClkKHz;
	u32 VideoBw;
	u32 VideoBwFec;
	u32 LinkBw;
	u32 AvgBytesPerTu;
	u32 MinBytes;

	if ((MsaConfig == NULL) || (Pps == NULL) || (LaneCount == 0U) ||
	    (MsaConfig->PixelClockHz == 0U)) {
		return XST_INVALID_PARAM;
	}

	if ((Pps->slice_width == 0U) || (Pps->chunk_size == 0U) ||
	    (Pps->bits_per_component == 0U) || (Pps->pic_width == 0U)) {
		return XST_FAILURE;
	}

	LaneCount_u = (u32)LaneCount;
	NumSlices = ((u32)Pps->pic_width + (u32)Pps->slice_width - 1U) /
		    (u32)Pps->slice_width;
	ChunkSize = (u32)Pps->chunk_size;

	if ((ChunkSize % LaneCount_u) == 0U) {
		NumDummy = 0U;
	} else {
		NumDummy = LaneCount_u - (ChunkSize % LaneCount_u);
	}
	NumEoc = LaneCount_u;

	CompBytesPerLine = (ChunkSize + NumDummy + NumEoc) * NumSlices;

	if (Pps->native_422) {
		BitsPerPixel = (u32)Pps->bits_per_component * 2U;
	} else if (Pps->native_420) {
		BitsPerPixel = ((u32)Pps->bits_per_component * 15U) / 10U;
	} else {
		BitsPerPixel = (u32)Pps->bits_per_component * 3U;
	}
	UncompBytesPerLine = ((u32)Pps->pic_width * BitsPerPixel) / 8U;
	if (UncompBytesPerLine == 0U) {
		return XST_FAILURE;
	}

	WplBits = CompBytesPerLine * 8U;
	if ((WplBits % 16U) == 0U) {
		WordsPerLine = WplBits / 16U;
	} else {
		WordsPerLine = (WplBits / 16U) + 1U;
	}

	MsaConfig->DataPerLane = (WordsPerLine > LaneCount_u) ?
				 (WordsPerLine - LaneCount_u) : 0U;

	UncompPixClkKHz = (u32)(MsaConfig->PixelClockHz / 1000U);
	CompPixClkKHz = (u32)((u64)UncompPixClkKHz * (u64)CompBytesPerLine /
			      (u64)UncompBytesPerLine);

	/*
	 * TU sizing per DSC_TX IP SPEC / Guidelines:
	 * Video_BW from compressed pixel clock × 24 bpp / 8 (DSC byte stream).
	 * FEC with DSC is mandatory — apply ~2.4 % overhead as ×1024/1000 on
	 * video bandwidth (Guidelines + DP 2.1 §3.5.1.5), matching legacy
	 * XDp_TxCfgMsaRecalculate SST TU convention for scaled numerator.
	 */
	VideoBw = (CompPixClkKHz * XDP21_DSC_LINK_BPP) / 8U;
	VideoBwFec = (VideoBw * XDP21_DSC_FEC_BW_NUM) / XDP21_DSC_FEC_BW_DEN;
	LinkBw = LaneCount_u * (u32)LinkRate * XDP21_DSC_LINK_BW_FACTOR;
	if (LinkBw == 0U) {
		return XST_FAILURE;
	}

	AvgBytesPerTu = (VideoBwFec * XDP21_DSC_TU_SIZE_BYTES) / LinkBw;

	/*
	 * Guard: compressed stream must fit within one TU (64 bytes).
	 * AvgBytesPerTu is in 1/1000 units, so threshold is 64*1000.
	 * Over-subscription means wrong chunk/link config — reject.
	 */
	if (AvgBytesPerTu >= (XDP21_DSC_TU_SIZE_BYTES * 1000U)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "DSC TU: link over-subscribed "
			    "(AvgBTU_x1000=%u, max=%u). "
			    "Check chunk_size and link rate.\r\n",
			    (unsigned int)AvgBytesPerTu,
			    (unsigned int)(XDP21_DSC_TU_SIZE_BYTES * 1000U));
		return XST_BUFFER_TOO_SMALL;
	}

	MsaConfig->AvgBytesPerTU = AvgBytesPerTu;
	MsaConfig->TransferUnitSize = XDP_TX_MAX_NUM_OF_TIMESLOTS;

	MinBytes = AvgBytesPerTu / 1000U;
	if (MinBytes <= 4U) {
		MsaConfig->InitWait = XDP_TX_MAX_NUM_OF_TIMESLOTS;
	} else {
		MsaConfig->InitWait = XDP_TX_MAX_NUM_OF_TIMESLOTS - MinBytes;
	}

	/* Compressed stream marker (DP 2.1). */
//	MsaConfig->Misc1 |= (1U << 6); //default
	MsaConfig->Misc1 = 0;		//temp hack

	return XST_SUCCESS;
}

#endif /* XDP_USE_DSC && XPAR_XDPTXSS_NUM_INSTANCES */
