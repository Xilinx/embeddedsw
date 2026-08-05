/******************************************************************************
 * @file dp_display_bridge.h
 * @brief ISP (VISP SS) -> DisplayPort live output bridge.
 *
 * Mirrors the role of vmix_hdmi_bridge for the DisplayPort datapath. It takes
 * the frames the ISP writes to DDR via the Frame Buffer Writer and streams
 * them out to a DisplayPort monitor through the design's live pipeline:
 *
 *   ISP -> frmbuf_wr -> DDR -> frmbuf_rd -> VTC -> DC (live input) -> DP TX
 *
 * The bridge programs the frame-buffer reader (frmbuf_rd), the video timing
 * controller (VTC), the live-input GPIO mux, the Display Controller (DC) live
 * input path (RGB passthrough) and the MMI DisplayPort transmitter (link
 * training happens via the DP HPD interrupt).
 *
 * Format note: only RGB888 is supported. The ISP / frmbuf_wr output must be
 * 8-bit RGB (XVIDC_CSF_MEM_RGB8); any other format is skipped by the bridge.
 *
 * Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 ******************************************************************************/

#ifndef DP_DISPLAY_BRIDGE_H
#define DP_DISPLAY_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xvidc.h"

/**
 * @brief Initialize the ISP -> DisplayPort live output bridge.
 *
 * Brings up the clock wizard, DC live input path, VTC, frmbuf_rd, live-input
 * GPIO and the DP transmitter. After this call the DP link trains on hotplug
 * and the DC continuously displays whatever frame frmbuf_rd points at.
 *
 * @param Width        Active width in pixels (e.g. 1920).
 * @param Height       Active height in pixels (e.g. 1080).
 * @param MemCfmt      Memory color format of the DDR frames written by the ISP
 *                     (must be XVIDC_CSF_MEM_RGB8; other formats are skipped).
 * @param Stride       Frame buffer stride in bytes (from the frmbuf_wr setup).
 * @param ChromaOffset Byte offset from luma base to chroma plane (unused for RGB).
 *
 * @return XST_SUCCESS on success, otherwise an error code.
 */
int DpDisplayBridge_Init(u32 Width, u32 Height, XVidC_ColorFormat MemCfmt,
			 u32 Stride, u32 ChromaOffset);

/**
 * @brief Point the DisplayPort datapath at the latest completed ISP frame.
 *
 * Call this from the frmbuf_wr done-callback with the address of the frame
 * that is ready for display (the read pointer buffer).
 *
 * @param LumaAddr   DDR address of the luma plane of the ready frame.
 * @param ChromaAddr DDR address of the chroma plane (LumaAddr + ChromaOffset).
 */
void DpDisplayBridge_UpdateFrame(u64 LumaAddr, u64 ChromaAddr);

/**
 * @brief Query whether the bridge finished initialization successfully.
 *
 * @return 1 if ready to accept frame updates, 0 otherwise.
 */
int DpDisplayBridge_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* DP_DISPLAY_BRIDGE_H */
