/*******************************************************************************
* Copyright (C) 2023-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef XDP21_DSC_TU_H_
#define XDP21_DSC_TU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xdp.h"
#include "xdp21_dsc.h"
#include "xil_types.h"

#ifdef XDP_USE_DSC

/** SST transfer-unit size (bytes). */
#define XDP21_DSC_TU_SIZE_BYTES   64U
/** Link BW factor: line_rate_code * 27 MB/s per lane (8b/10b only). */
#define XDP21_DSC_LINK_BW_FACTOR  27U
/** DSC byte stream on link is 24 bpp (YUV444 8 bpc equivalent). */
#define XDP21_DSC_LINK_BPP        24U

/**
 * FEC overhead factor on DSC video bandwidth for TU sizing (~2.4%), per
 * DSC_TX IP SPEC / Guidelines (DP 2.1 SST DSC): multiply uncompressed-video-
 * derived bandwidth by 1024/1000 before (Video_BW / Link_BW) × TU_SIZE.
 */
#define XDP21_DSC_FEC_BW_NUM      1024U
#define XDP21_DSC_FEC_BW_DEN      1000U

/**
 * Compute DSC SST MSA fields (USER_DATA_COUNT, AvgBytesPerTU, InitWait).
 *
 * Matches DSC_TX__IP_SPEC.doc / Guidelines:
 *   - Compressed bytes per line = (chunk + dummy + EOC) * num_slices;
 *     USER_DATA_COUNT from words-per-line - lane_count (PG420-style).
 *   - TU average uses 24 bpp on link + mandatory FEC overhead x1024/1000
 *     on video bandwidth.
 *   - Over-subscription guard: returns XST_BUFFER_TOO_SMALL if
 *     AvgBytesPerTU/1000 >= 64 (would underflow INIT_WAIT).
 *   - Non-integer (chunk x slices) / (3 x PPC) ("unaligned with PPC") is
 *     accepted, not rejected: CompBytesPerLine's NumDummy/NumEoc per-lane
 *     padding already covers the remainder.
 */
int XDp_TxCalcDscMsaTu(XDp_TxMainStreamAttributes *MsaConfig,
			const XDp21_DscPps *Pps,
			u8 LinkRate,
			u8 LaneCount);

/**
 * Store the DSC-enable flag in the XDp_Tx instance.
 * When Enable=0 also clears DscPpsValid.
 */
void XDp_TxSetDscStreamEnable(XDp *InstancePtr, u8 Enable);

/**
 * Parse and validate a 128-byte PPS, then store it for use by
 * XDp_TxCalcDscMsaTu().  Reads USER_PIXEL_WIDTH (0x1B8) for PPC.
 * A non-integer (chunk x slices)/(3 x PPC) is logged but no longer
 * rejected -- see XDp_TxValidateDscPps() in xdp21_dsc_tu.c.
 */
int XDp_TxSetDscStreamPps(XDp *InstancePtr, const u8 *PpsData);

#endif /* XDP_USE_DSC */

#ifdef __cplusplus
}
#endif

#endif /* XDP21_DSC_TU_H_ */
