/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdp21_dsc_fec_vbid_pps_extraction.c
 * @brief VB-ID[6] checking and PPS data extraction from hardware registers for
 *        DP 2.1 DSC FEC pass-through
 */

#include "xdp21_dsc_fec_vbid_pps_extraction.h"
#include "xdp.h"

/*
 * The receive register definitions this file reads exist only when the design
 * instantiates a receive subsystem, so a transmit-only design has to drop the
 * whole file rather than just the compression support. xdp.h is included above
 * the test because it supplies the instance count the test reads.
 */
#if defined(XDP_USE_DSC) && XPAR_XDPRXSS_NUM_INSTANCES

#include "xdp21_dsc.h"
#include "xdp_hw.h"
#include "xdebug.h"
#include "sleep.h"
#include <string.h>

/**************************** Function Implementations ***********************/

/**
 * @brief Check VB-ID[6] compression flag for DSC stream detection
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @param StreamId Stream ID (0-3 for multi-stream, 0 for SST)
 * @param VbIdValue Pointer to store complete VB-ID value
 * @param CompressionFlag Pointer to store DSC compression flag state
 *
 * @return
 *   - XST_SUCCESS if VB-ID is read successfully
 *   - XST_INVALID_PARAM if parameters are invalid
 *
 * @note This function implements the exact VB-ID[6] checking mechanism
 *       shown in the DSC Discovery flow diagram. VB-ID[6] = 1 indicates
 *       compressed DSC stream, VB-ID[6] = 0 indicates uncompressed stream.
 */
int XDp21_CheckVbIdCompressionFlag(u32 BaseAddr, u8 StreamId,
                                   u8 *VbIdValue, u8 *CompressionFlag)
{
    u32 VbIdRegOffset;
    u32 RegValue;

    /* Verify arguments */
    if (StreamId > 3 || VbIdValue == NULL || CompressionFlag == NULL) {
        return XST_INVALID_PARAM;
    }

    /* Select VB-ID register based on stream ID */
    switch (StreamId) {
        case 0: /* SST or MST Stream 1 */
            VbIdRegOffset = XDP_RX_MSA_VBID;
            break;
        case 1: /* MST Stream 2 */
            VbIdRegOffset = XDP_RX_MSA_VBID + XDP_RX_STREAM2_MSA_START_OFFSET;
            break;
        case 2: /* MST Stream 3 */
            VbIdRegOffset = XDP_RX_MSA_VBID + XDP_RX_STREAM3_MSA_START_OFFSET;
            break;
        case 3: /* MST Stream 4 */
            VbIdRegOffset = XDP_RX_MSA_VBID + XDP_RX_STREAM4_MSA_START_OFFSET;
            break;
        default:
            return XST_INVALID_PARAM;
    }

    /*
     * Read VB-ID from hardware, or DSC_CAPS_1[0] (kCFG_DSC_ENABLE) as fallback.
     * kCFG_DSC_ENABLE is ro and indicates compressed stream when set.
     */
    RegValue = XDp_ReadReg(BaseAddr, VbIdRegOffset);
    *VbIdValue = (u8)(RegValue & 0xFF);  /* VB-ID is 8 bits */
    *CompressionFlag = (*VbIdValue & XDP_RX_MSA_VBID_DSC_MASK) ? 1 : 0;

    /* Fallback: use DSC_CAPS_1 kCFG_DSC_ENABLE [0] if VB-ID not conclusive */
    if (*CompressionFlag == 0) {
        u32 DscCaps1 = XDp_ReadReg(BaseAddr, XDP_RX_DSC_CAPS_1);
        if (DscCaps1 & XDP_RX_DSC_CAPS_1_ENABLE_MASK) {
            *CompressionFlag = 1;
        }
    }

    return XST_SUCCESS;
}

/**
 * @brief Monitor VB-ID[6] for dynamic DSC enable/disable detection
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @param StreamId Stream ID to monitor
 * @param PreviousCompressionState Previous compression state
 * @param CurrentCompressionState Current compression state
 *
 * @return
 *   - 1 if compression state changed
 *   - 0 if compression state unchanged
 *   - -1 if error occurred
 *
 * @note This function continuously monitors VB-ID[6] to detect when the
 *       source dynamically enables or disables DSC compression. This
 *       triggers the "Set or clear VB-ID[6] = 1 or 0" action in the diagram.
 */
int XDp21_MonitorVbIdCompressionChange(u32 BaseAddr, u8 StreamId,
                                       u8 PreviousCompressionState,
                                       u8 *CurrentCompressionState)
{
    u8 VbIdValue;
    int Status;

    /* Read current VB-ID compression flag */
    Status = XDp21_CheckVbIdCompressionFlag(BaseAddr, StreamId,
                                            &VbIdValue, CurrentCompressionState);
    if (Status != XST_SUCCESS) {
        return -1;
    }

    if (PreviousCompressionState != *CurrentCompressionState) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "DSC RX: compression state changed %u->%u\r\n",
                    PreviousCompressionState, *CurrentCompressionState);
        return 1; /* State changed */
    }

    return 0; /* State unchanged */
}

/**
 * @brief Extract PPS data from hardware capture registers
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @param StreamId Stream ID to extract PPS from
 * @param PpsData Buffer to store extracted PPS data (128 bytes)
 * @param PpsValid Pointer to store PPS validity flag
 *
 * @return
 *   - XST_SUCCESS if PPS is extracted successfully
 *   - XST_INVALID_PARAM if parameters are invalid
 *   - XST_NO_DATA if no PPS data is available
 *   - XST_FAILURE if PPS data is invalid
 *
 * @note This function extracts the complete 128-byte PPS packet from
 *       hardware registers where the DP RX core has captured it from
 *       the incoming DSC stream. This is the "transmit the first PPS SDP"
 *       action from the flow diagram.
 */
int XDp21_ExtractPpsFromHardware(u32 BaseAddr, u8 StreamId,
                                 u8 *PpsData, u8 *PpsValid)
{
    u32 RegValue;
    u32 FirstWord;
    int PollTry;
    int i;
    u32 PpsOffset;

    (void)StreamId; /* SST uses single PPS block at 0x0C00 */

    if (PpsData == NULL || PpsValid == NULL) {
        return XST_INVALID_PARAM;
    }
    *PpsValid = 0;

    /*
     * Caller verified VB-ID[6]=1 (DSC stream). Read PPS directly.
     * DSC_CAPS_1[0] may not be set by IP; do not gate on it.
     */

    /*
     * PPS capture can lag VB-ID[6] assertion by a few milliseconds.
     * Poll first PPS word briefly before declaring failure.
     */
    FirstWord = 0U;
    for (PollTry = 0; PollTry < 50; PollTry++) {
        FirstWord = XDp_ReadReg(BaseAddr, XDP_RX_DSC_PPS_DATA);
        if (FirstWord != 0U) {
            break;
        }
        usleep(2000U); /* 2ms x 50 = up to 100ms wait */
    }

    if (FirstWord == 0U) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "PPS capture not ready yet: PPS[0]=0x00000000 at 0x%08X\r\n",
                    XDP_RX_DSC_PPS_DATA);
        return XST_NO_DATA;
    }

    /*
     * Extract PPS from DSC_PPS_DATA[0:127] at 0x0C00-0x0C7C
     * Per RAL: BYTE_0 [7:0], BYTE_1 [15:8], BYTE_2 [23:16], BYTE_3 [31:24]
     */
    for (i = 0; i < XDP21_DSC_PPS_SIZE; i += 4) {
        PpsOffset = XDP_RX_DSC_PPS_DATA + i;
        RegValue = XDp_ReadReg(BaseAddr, PpsOffset);
        PpsData[i + 0] = (u8)(RegValue & 0xFF);
        PpsData[i + 1] = (u8)((RegValue >> 8) & 0xFF);
        PpsData[i + 2] = (u8)((RegValue >> 16) & 0xFF);
        PpsData[i + 3] = (u8)((RegValue >> 24) & 0xFF);
    }

    /*
     * Validate PPS header. VESA DSC: 0x00 0x10 or 0x12 0x00 (IP byte order).
     * Observed: 0x44A04C00=0x89000012 -> bytes 0x12 0x00 0x00 0x89.
     * Accept: (0x00, 0x10-0x12) or (0x10-0x12, 0x00).
     */
    if (!((PpsData[0] == 0x00 && PpsData[1] >= 0x10 && PpsData[1] <= 0x12) ||
          (PpsData[0] >= 0x10 && PpsData[0] <= 0x12 && PpsData[1] == 0x00))) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "PPS header invalid: [%02X %02X], PPS[0]=0x%08X\r\n",
                    PpsData[0], PpsData[1],
                    (unsigned int)XDp_ReadReg(BaseAddr, XDP_RX_DSC_PPS_DATA));
        return XST_FAILURE;
    }

    *PpsValid = 1;
    return XST_SUCCESS;
}

/**
 * @brief Parse extracted PPS data into structured configuration
 *
 * @param PpsData Raw PPS data (128 bytes) extracted from hardware
 * @param PpsConfigPtr Structured PPS configuration to populate
 *
 * @return
 *   - XST_SUCCESS if parsing is successful
 *   - XST_INVALID_PARAM if parameters are invalid
 *   - XST_FAILURE if parsing fails
 *
 * @note This function parses the raw 128-byte PPS packet into a structured
 *       format for easy software manipulation. This implements the detailed
 *       bit-field extraction according to VESA DSC specification.
 *
 *       This is the STANDARD API name used throughout the driver.
 *       Previously named XDp21_ParseExtractedPpsData() - now renamed for consistency.
 */
int XDp21_DscParsePpsPacket(const u8 *PpsData, XDp21_DscPpsConfig *PpsConfigPtr)
{
    /* Verify arguments */
    if (PpsData == NULL || PpsConfigPtr == NULL) {
        return XST_INVALID_PARAM;
    }

    memset(PpsConfigPtr, 0, sizeof(XDp21_DscPpsConfig));

    /* Byte 0: DSC Version, Byte 1: PPS Identifier */
    PpsConfigPtr->DscVersionMajor = (PpsData[0] >> 4) & 0x0F;
    PpsConfigPtr->DscVersionMinor = PpsData[0] & 0x0F;
    PpsConfigPtr->PpsIdentifier = PpsData[1];

    /* Bytes 6-9: Picture Height and Width */
    PpsConfigPtr->PicHeight = (u16)(PpsData[6] << 8) | PpsData[7];
    PpsConfigPtr->PicWidth = (u16)(PpsData[8] << 8) | PpsData[9];

    /* Bytes 10-13: Slice Height and Width */
    PpsConfigPtr->SliceHeight = (u16)(PpsData[10] << 8) | PpsData[11];
    PpsConfigPtr->SliceWidth = (u16)(PpsData[12] << 8) | PpsData[13];

    /* Bytes 14-15: Chunk Size */
    PpsConfigPtr->ChunkSize = (u16)(PpsData[14] << 8) | PpsData[15];

    /* Bits per Pixel (1/16 bpp units): byte4[1:0]:byte5 */
    PpsConfigPtr->BitsPerPixel = (u16)(((u16)(PpsData[4] & 0x03) << 8) | PpsData[5]);

    /* Byte 3: Bits per Component and line buffer depth */
    PpsConfigPtr->BitsPerComponent = (PpsData[3] >> 4) & 0x0F;
    PpsConfigPtr->LinebufDepth = PpsData[3] & 0x0F;

    /* Byte 4: DSC flags */
    PpsConfigPtr->BlockPredEnable = (PpsData[4] >> 5) & 0x01;
    PpsConfigPtr->ConvertRgb = (PpsData[4] >> 4) & 0x01;
    PpsConfigPtr->ColorFormat = (PpsData[4] >> 1) & 0x03;

    /* Bytes 38-39: RC Model Size */
    PpsConfigPtr->RcModelSize = (u16)(PpsData[38] << 8) | PpsData[39];

#ifdef DEBUG
    /*
     * Guideline calculations for TX DSC 1.4 link-rates, SST mode. These values
     * are derived only to be reported, so they are scoped to the diagnostic
     * build and none of them feed PpsConfigPtr.
     */
    {
    u32 NumSlices = 0U;
    u32 NumSliceRows = 0U;
    u32 CompressedBitsPerLine;
    u32 CompressedBytesPerLine;
    u32 CompressedHactive;
    u32 Numerator;
    u32 MaxDivFactorFromCond1;
    u32 CompressedVactive;

    if (PpsConfigPtr->SliceWidth != 0U) {
        NumSlices = (u32)PpsConfigPtr->PicWidth / (u32)PpsConfigPtr->SliceWidth;
    }
    if (PpsConfigPtr->SliceHeight != 0U) {
        NumSliceRows = (u32)PpsConfigPtr->PicHeight / (u32)PpsConfigPtr->SliceHeight;
    }
    CompressedVactive = (u32)PpsConfigPtr->PicHeight; /* Vertical remains uncompressed */

    CompressedBytesPerLine = (u32)PpsConfigPtr->ChunkSize * NumSlices;
    CompressedBitsPerLine = CompressedBytesPerLine * 8U;

    if ((CompressedBitsPerLine != 0U) && (PpsConfigPtr->PicWidth != 0U)) {
        MaxDivFactorFromCond1 =
            ((u32)PpsConfigPtr->PicWidth * 24U) / CompressedBitsPerLine;
    } else {
        MaxDivFactorFromCond1 = 1U;
    }
    if (MaxDivFactorFromCond1 == 0U) {
        MaxDivFactorFromCond1 = 1U;
    }
    Numerator = (u32)PpsConfigPtr->ChunkSize * NumSlices;
    CompressedHactive = (Numerator % 3U) ? 0U : (Numerator / 3U); /* PPC=1 case */

    xil_printf("DSC RX PPS: valid=1 pic=%ux%u slice=%ux%u slices=%u rows=%u chunk=%u bpp=%u.%u bpc=%u fmt=%u hact_c=%u vact_c=%u div=%u\r\n",
               (u32)PpsConfigPtr->PicWidth, (u32)PpsConfigPtr->PicHeight,
               (u32)PpsConfigPtr->SliceWidth, (u32)PpsConfigPtr->SliceHeight,
               NumSlices, NumSliceRows,
               (u32)PpsConfigPtr->ChunkSize,
               (u32)(PpsConfigPtr->BitsPerPixel / 16U),
               (u32)((PpsConfigPtr->BitsPerPixel % 16U) * 10U / 16U),
               (u32)PpsConfigPtr->BitsPerComponent,
               (u32)PpsConfigPtr->ColorFormat,
               CompressedHactive, CompressedVactive, MaxDivFactorFromCond1);
    }
#endif

    memcpy(PpsConfigPtr->RawPpsData, PpsData, XDP21_DSC_PPS_SIZE);
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Expose DSC and FEC capability to upstream TX via DPCD-mapped registers
 *
 * Programs FEC_CAPS (0x0100) and DSC_CRC_1 kCFG_DSC_SUPPORT (0x0798 bit 3)
 * so the source can discover capability when probing DPCD via AUX.
 * Call at RX init or before link training.
 */
int XDp21_ExposeDscFecCapability(u32 BaseAddr)
{
    u32 RegVal;

    /* FEC_CAPS: maps to DPCD 0x090 - declare FEC capability */
    XDp_WriteReg(BaseAddr, XDP_RX_FEC_CAPS, 0x01);

    /*
     * DSC_CRC_1: set kCFG_DSC_SUPPORT [3] - maps to DPCD 0x060 bit[0].
     * This write was previously commented out, so the source never saw
     * DSC advertised as supported at all (independent of link bandwidth) -
     * it had no reason to ever choose a DSC-compressed encoding.
     */
    RegVal = XDp_ReadReg(BaseAddr, XDP_RX_DSC_CRC_1);
    RegVal |= XDP_RX_DSC_CRC_1_SUPPORT_MASK;

    XDp_WriteReg(BaseAddr, XDP_RX_DSC_CRC_1, RegVal);

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Complete VB-ID[6] monitoring and PPS extraction flow
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @param StreamId Stream ID to monitor (0 for SST)
 * @param PpsConfig Pointer to store extracted and parsed PPS configuration
 *
 * @return
 *   - XST_SUCCESS if DSC stream detected and PPS extracted
 *   - XST_NO_DATA if no DSC stream detected
 *   - XST_FAILURE if error occurred
 *
 * @note This function implements the complete flow from Figure 2-122:
 *       1. Check VB-ID[6] for DSC compression flag
 *       2. If VB-ID[6] = 1, enable DSC decompression
 *       3. Extract PPS packet from hardware
 *       4. Parse PPS into structured format
 *       5. Apply new PPS on upcoming frame
 */
int XDp21_CompleteDscStreamDetectionFlow(u32 BaseAddr, u8 StreamId,
                                         XDp21_DscPpsConfig *PpsConfig)
{
    u8 VbIdValue, CompressionFlag, PpsValid;
    u8 PpsData[XDP21_DSC_PPS_SIZE];
    int Status;

    Status = XDp21_CheckVbIdCompressionFlag(BaseAddr, StreamId,
                                            &VbIdValue, &CompressionFlag);
    if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL, "DSC RX: VB-ID read failed\r\n");
        return Status;
    }

    if (!CompressionFlag) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "DSC RX: uncompressed stream (VBID=0x%02X)\r\n", VbIdValue);
        return XST_NO_DATA; /* No DSC stream */
    }
    Status = XDp21_ExtractPpsFromHardware(BaseAddr, StreamId, PpsData, &PpsValid);
    if (Status != XST_SUCCESS) {
        if (Status != XST_NO_DATA) {
            xdbg_printf(XDBG_DEBUG_GENERAL,
                        "DSC RX: PPS extract failed\r\n");
        }
        return Status;
    }

    if (!PpsValid) {
        xdbg_printf(XDBG_DEBUG_GENERAL, "DSC RX: PPS invalid\r\n");
        return XST_FAILURE;
    }
    Status = XDp21_DscParsePpsPacket(PpsData, PpsConfig);
    if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL, "DSC RX: PPS parse failed\r\n");
        return Status;
    }
    if (PpsConfig->PicWidth == 0 || PpsConfig->PicHeight == 0 ||
        PpsConfig->BitsPerPixel == 0 || PpsConfig->BitsPerComponent == 0) {
        xdbg_printf(XDBG_DEBUG_GENERAL, "DSC RX: PPS fields invalid\r\n");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

#endif /* XDP_USE_DSC && XPAR_XDPRXSS_NUM_INSTANCES */
