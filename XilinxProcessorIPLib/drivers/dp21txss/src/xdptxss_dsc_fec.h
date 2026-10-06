/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdptxss_dsc_fec.h
 * @brief DP TX Subsystem DSC/FEC Support Header
 *
 * Declares the Display Stream Compression and Forward Error Correction entry
 * points of the DisplayPort 2.1 TX Subsystem. The whole interface is compiled
 * only for designs that build the DSC encoder in. The subsystem instance is
 * left unchanged, so the state is held in this translation unit.
 */

#ifndef XDPTXSS_DSC_FEC_H
#define XDPTXSS_DSC_FEC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xdptxss.h"
#include "xdp21_dsc.h"
#include "xil_types.h"
#include "xstatus.h"

#ifdef XDPTXSS_USE_DSC

/** SST TU size (bytes per transfer unit). */
#define XDPTXSS_DSC_TU_SIZE_BYTES  64U

/** Link-bandwidth factor: line_rate_code * 27 MB/s per lane (8b/10b). */
#define XDPTXSS_DSC_LINK_BW_FACTOR 27U

/** DSC byte stream on the link is always 24 bpp (YUV444 8 bpc equivalent). */
#define XDPTXSS_DSC_LINK_BPP       24U

/************************** Function Prototypes ******************************/

/**
 * @brief Initialize DSC/FEC support for TX Subsystem
 * @param InstancePtr Pointer to XDpTxSs instance
 * @return XST_SUCCESS if successful
 */
int XDpTxSs_DscFecInit(XDpTxSs *InstancePtr);

/**
 * @brief Enable/Disable FEC on TX
 * @param InstancePtr Pointer to XDpTxSs instance
 * @param Enable 1 to enable, 0 to disable
 * @return XST_SUCCESS if successful
 */
int XDpTxSs_SetFecEnable(XDpTxSs *InstancePtr, u8 Enable);

/**
 * @brief Negotiate DSC capabilities with sink
 * @param InstancePtr Pointer to XDpTxSs instance
 * @return XST_SUCCESS if successful
 */
int XDpTxSs_DscNegotiateCapabilities(XDpTxSs *InstancePtr);

/**
 * @brief Enable DSC pass-through on TX
 * @param InstancePtr Pointer to XDpTxSs instance
 * @param Enable 1 to enable, 0 to disable
 * @return XST_SUCCESS if successful
 */
int XDpTxSs_EnableDscPassthrough(XDpTxSs *InstancePtr, u8 Enable);

/**
 * @brief Write PPS data to TX hardware
 * @param InstancePtr Pointer to XDpTxSs instance
 * @param PpsData Pointer to 128-byte PPS data
 * @return XST_SUCCESS if successful
 */
int XDpTxSs_WriteDscPps(XDpTxSs *InstancePtr, u8 *PpsData);

/**
 * @brief Enable/Disable DSC in strict sequence
 *        Enable order: sink DPCD DSC_ENABLE -> TX AXI-Lite DSC_ENABLE
 * @param InstancePtr Pointer to XDpTxSs instance
 * @param Enable 1 to enable, 0 to disable
 * @return XST_SUCCESS if successful
 */
int XDpTxSs_SetDscEnable(XDpTxSs *InstancePtr, u8 Enable);

/**
 * @brief Tell the DP TX driver to use DSC TU formulas in MSA recalculate.
 *
 * When Enable=1, PpsData must be a valid 128-byte PPS packet.  The BSP
 * XDp_TxCfgMsaRecalculate() path then computes USER_DATA_COUNT and TU fields
 * from PPS (chunk + dummy + EOC) instead of legacy uncompressed HActive.
 *
 * @param InstancePtr TX subsystem instance.
 * @param Enable 1 = DSC stream, 0 = legacy uncompressed.
 * @param PpsData 128-byte PPS (required when Enable=1), else NULL.
 * @return XST_SUCCESS or error.
 */
int XDpTxSs_SetDscStreamConfig(XDpTxSs *InstancePtr, u8 Enable, const u8 *PpsData);

/**
 * @brief Re-run MSA recalculate and commit registers (after link train / HPD).
 *
 * Uses DscStreamEnable flag set by XDpTxSs_SetDscStreamConfig().
 */
int XDpTxSs_RefreshDscMsaTu(XDpTxSs *InstancePtr, u8 Stream);

/**
 * @brief DSC SST TU / USER_DATA_COUNT register values (DSC_TX IP SPEC).
 *
 * Via XDp_TxCalcDscMsaTu(): compressed bytes per line = (chunk + dummy +
 * EOC) × num_slices; USER_DATA_COUNT from words-per-line − lane_count;
 * average bytes per TU uses 24 bpp on link + mandatory FEC overhead ×1024/1000
 * on video bandwidth (Guidelines / DP 2.1 §3.5.1.5); MIN/FRAC via BSP scaling.
 */
typedef struct {
	u32 UserDataCountPerLane; /**< Register 0x1BC */
	u32 MinBytesPerTu;        /**< Register 0x1C4 */
	u32 FracBytesPerTu;       /**< Register 0x1C8 (1/1024 units) */
	u32 InitWait;             /**< Register 0x1CC */
	u32 CompBytesPerLine;     /**< (chunk + dummy + EOC) * slices */
	u32 AvgBytesPerTu;        /**< MsaConfig format: average * 1000 */
} XDpTxSs_DscTuValues;

/**
 * @brief Calculate DSC TU registers from PPS and link configuration.
 *
 * @param Pps Parsed PPS (pic_width, slice_width, chunk_size, bpc, native_420/422).
 * @param UncompPixelClockHz Uncompressed source pixel clock in Hz.
 * @param LinkRate DP link rate code (e.g. XDP_TX_LINK_BW_SET_270GBPS).
 * @param LaneCount Active lane count (1, 2, or 4).
 * @param Out Output register values.
 * @return XST_SUCCESS or XST_INVALID_PARAM / XST_FAILURE.
 */
int XDpTxSs_CalcDscTuValues(const XDp21_DscPps *Pps,
			    u64 UncompPixelClockHz,
			    u8 LinkRate,
			    u8 LaneCount,
			    XDpTxSs_DscTuValues *Out);

/**
 * @brief Write DSC TU registers, update MsaConfig, and set MISC1 compressed flag.
 *
 * Programs 0x1BC/0x1C4/0x1C8/0x1CC and copies DataPerLane / AvgBytesPerTU /
 * InitWait into MsaConfig so a later BSP XDp_TxSetMsaValues() does not restore
 * the uncompressed HActive-based USER_DATA_COUNT (e.g. 3328).
 *
 * @param InstancePtr TX subsystem instance.
 * @param Values Values from XDpTxSs_CalcDscTuValues().
 * @param Stream Stream index (1..4).
 * @return XST_SUCCESS or error.
 */
int XDpTxSs_ApplyDscTuValues(XDpTxSs *InstancePtr,
			     const XDpTxSs_DscTuValues *Values,
			     u8 Stream);

#endif /* XDPTXSS_USE_DSC */

#ifdef __cplusplus
}
#endif

#endif /* XDPTXSS_DSC_FEC_H */
