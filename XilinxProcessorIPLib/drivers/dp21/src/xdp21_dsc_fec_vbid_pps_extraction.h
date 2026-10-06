/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdp21_dsc_fec_vbid_pps_extraction.h
 * @brief Header file for DSC VB-ID and PPS extraction APIs
 *
 * This header defines the public APIs for DSC stream detection,
 * VB-ID[6] monitoring, and PPS packet extraction/parsing.
 */

#ifndef XDP21_DSC_FEC_VBID_PPS_EXTRACTION_H_
#define XDP21_DSC_FEC_VBID_PPS_EXTRACTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xdp21_dsc.h"
#include "xil_types.h"
#include "xstatus.h"

#ifdef XDP_USE_DSC

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * Extended PPS configuration structure for VB-ID/PPS extraction APIs.
 * Contains parsed PPS fields plus raw data for pass-through.
 */
typedef struct {
    u8 DscVersionMajor;
    u8 DscVersionMinor;
    u8 PpsIdentifier;
    u16 PicWidth;
    u16 PicHeight;
    u16 SliceWidth;
    u16 SliceHeight;
    u16 ChunkSize;
    u16 BitsPerPixel;
    u8 BitsPerComponent;
    u8 LinebufDepth;
    u8 BlockPredEnable;
    u8 ConvertRgb;
    u8 ColorFormat;
    u16 RcModelSize;
    u8 RawPpsData[XDP21_DSC_PPS_SIZE];
} XDp21_DscPpsConfig;

/************************** Function Prototypes ******************************/

/**
 * @name VB-ID[6] Monitoring APIs
 * @{
 */

/**
 * Check VB-ID[6] DSC compression flag for a specific stream
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @param StreamId Stream ID (0-3 for MST, 0 for SST)
 * @param VbIdValue Pointer to store complete VB-ID value
 * @param CompressionFlag Pointer to store DSC compression flag (0 or 1)
 *
 * @return XST_SUCCESS if successful, XST_INVALID_PARAM if parameters invalid
 */
int XDp21_CheckVbIdCompressionFlag(u32 BaseAddr, u8 StreamId,
                                   u8 *VbIdValue, u8 *CompressionFlag);

/**
 * Monitor VB-ID[6] for dynamic DSC enable/disable changes
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @param StreamId Stream ID to monitor
 * @param PreviousCompressionState Previous compression state
 * @param CurrentCompressionState Pointer to store current compression state
 *
 * @return 1 if state changed, 0 if unchanged, -1 if error
 */
int XDp21_MonitorVbIdCompressionChange(u32 BaseAddr, u8 StreamId,
                                       u8 PreviousCompressionState,
                                       u8 *CurrentCompressionState);

/** @} */

/**
 * @name PPS Packet Extraction and Parsing APIs
 * @{
 */

/**
 * Extract PPS packet from hardware capture registers
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @param StreamId Stream ID to extract PPS from
 * @param PpsData Buffer to store extracted PPS data (must be 128 bytes)
 * @param PpsValid Pointer to store PPS validity flag
 *
 * @return XST_SUCCESS if extracted, XST_NO_DATA if not available,
 *         XST_FAILURE if invalid
 */
int XDp21_ExtractPpsFromHardware(u32 BaseAddr, u8 StreamId,
                                 u8 *PpsData, u8 *PpsValid);

/**
 * Parse raw PPS packet into structured configuration
 *
 * This is the STANDARD API name used throughout the DSC/FEC driver.
 *
 * @param PpsData Raw PPS data (128 bytes) extracted from hardware
 * @param PpsConfigPtr Pointer to XDp21_DscPpsConfig structure to populate
 *
 * @return XST_SUCCESS if parsed successfully, XST_INVALID_PARAM if invalid,
 *         XST_FAILURE if parsing fails
 *
 * @note This API name follows the driver naming convention:
 *       XDp21_Dsc<Action><Object>
 *       Example: XDp21_DscParsePpsPacket, XDp21_DscEncodePpsPacket
 */
int XDp21_DscParsePpsPacket(const u8 *PpsData, XDp21_DscPpsConfig *PpsConfigPtr);

/** @} */

/**
 * @name Complete DSC Detection Flow APIs
 * @{
 */

/**
 * Complete VB-ID[6] monitoring and PPS extraction flow
 *
 * This function implements the complete DSC detection flow:
 * 1. Check VB-ID[6] for DSC compression flag
 * 2. Extract PPS packet from hardware if DSC is enabled
 * 3. Parse PPS into structured format
 * 4. Validate parsed parameters
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @param StreamId Stream ID to monitor (0 for SST)
 * @param PpsConfig Pointer to store extracted and parsed PPS configuration
 *
 * @return XST_SUCCESS if DSC stream detected and PPS extracted,
 *         XST_NO_DATA if no DSC stream detected,
 *         XST_FAILURE if error occurred
 */
int XDp21_CompleteDscStreamDetectionFlow(u32 BaseAddr, u8 StreamId,
                                         XDp21_DscPpsConfig *PpsConfig);

/**
 * Expose DSC and FEC capability to upstream TX via DPCD-mapped registers.
 * Programs FEC_CAPS (0x0100) and DSC_CRC_1 kCFG_DSC_SUPPORT (0x0798 bit 3)
 * so the source can discover capability when probing DPCD via AUX.
 *
 * @param BaseAddr Base address of DP RX subsystem
 * @return XST_SUCCESS if successful
 */
int XDp21_ExposeDscFecCapability(u32 BaseAddr);

/** @} */

/************************** API Naming Convention ****************************/

/**
 * STANDARD DSC/FEC API NAMING CONVENTION
 * =======================================
 *
 * All DSC/FEC APIs follow this naming pattern:
 *
 * Format: XDp21[Subsystem]_[Component][Action][Object]
 *
 * Examples:
 * ---------
 * VB-ID APIs:
 *   - XDp21_CheckVbIdCompressionFlag()
 *   - XDp21_MonitorVbIdCompressionChange()
 *
 * PPS Parsing APIs (STANDARD NAMES):
 *   - XDp21_DscParsePpsPacket()   ← Use this (hardware → software)
 *   - XDp21_DscEncodePpsPacket()  ← Use this (software → hardware)
 *
 * PPS Extraction APIs:
 *   - XDp21_ExtractPpsFromHardware()
 *
 * Complete Flow APIs:
 *   - XDp21_CompleteDscStreamDetectionFlow()
 *
 * DEPRECATED/ALTERNATE NAMES (DO NOT USE):
 * -----------------------------------------
 *   - XDp21_ParseExtractedPpsData()  ← OLD NAME, use XDp21_DscParsePpsPacket()
 *
 * RATIONALE:
 * ----------
 * The "XDp21_Dsc" prefix clearly indicates these are DSC-specific operations.
 * The "Parse" and "Encode" verbs indicate direction of conversion:
 *   - Parse:  Raw bytes → Structured data (hardware → software)
 *   - Encode: Structured data → Raw bytes (software → hardware)
 *
 * This naming is consistent with other Xilinx driver APIs and makes the
 * function purpose immediately clear from the name.
 */

#endif /* XDP_USE_DSC */

#ifdef __cplusplus
}
#endif

#endif /* XDP21_DSC_FEC_VBID_PPS_EXTRACTION_H_ */
