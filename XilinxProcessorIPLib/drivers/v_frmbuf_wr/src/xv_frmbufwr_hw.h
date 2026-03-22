// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
#ifndef XV_FRMBUFWR_HW_H_  /* prevent circular inclusions */
#define XV_FRMBUFWR_HW_H_  /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * *
* @file xv_frmbufwr_hw.h
* @addtogroup v_frmbuf_wr Overview
*
**/
// CTRL
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 5  - Flush pending AXI transactions
//        bit 6  - Flush done
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x10 : Data signal of HwReg_width
//        bit 15~0 - HwReg_width[15:0] (Read/Write)
//        others   - reserved
// 0x14 : reserved
// 0x18 : Data signal of HwReg_height
//        bit 15~0 - HwReg_height[15:0] (Read/Write)
//        others   - reserved
// 0x1c : reserved
// 0x20 : Data signal of HwReg_stride
//        bit 15~0 - HwReg_stride[15:0] (Read/Write)
//        others   - reserved
// 0x24 : reserved
// 0x28 : Data signal of HwReg_video_format
//        bit 15~0 - HwReg_video_format[15:0] (Read/Write)
//        others   - reserved
// 0x2c : reserved
// 0x30 : Data signal of HwReg_frm_buffer_V
//        bit 31~0 - HwReg_frm_buffer_V[31:0] (Read/Write)
// 0x34 : Data signal of HwReg_frm_buffer_V
//        bit 31~0 - HwReg_frm_buffer_V[63:32] (Read/Write)
// 0x38 : reserved
// 0x3c : Data signal of HwReg_frm_buffer2_V
//        bit 31~0 - HwReg_frm_buffer2_V[31:0] (Read/Write)
// 0x40 : Data signal of HwReg_frm_buffer2_V
//        bit 31~0 - HwReg_frm_buffer2_V[63:32] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of HwReg_field_id
//        bit 0  - HwReg_field_id[0] (Read)
//        others - reserved
// 0x4c : reserved
// 0x54 : Data signal of HwReg_frm_buffer3_V
//        bit 31~0 - HwReg_frm_buffer3_V[31:0] (Read/Write)
// 0x58 : Data signal of HwReg_frm_buffer3_V
//        bit 31~0 - HwReg_frm_buffer3_V[63:32] (Read/Write)
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

/** Base address of the control register for AP (Accelerator Protocol) control signals */
#define XV_FRMBUFWR_CTRL_ADDR_AP_CTRL                  0x00

/** Bit position for the flush command in the control register */
#define XV_FRMBUFWR_CTRL_BITS_FLUSH_BIT_POS		       (5)

/** Bit mask for the flush command (bit 5) - initiates flush of pending AXI transactions */
#define XV_FRMBUFWR_CTRL_BITS_FLUSH_BIT			       (1 << XV_FRMBUFWR_CTRL_BITS_FLUSH_BIT_POS)

/** Bit position for the flush status in the control register */
#define XV_FRMBUFWR_CTRL_BITS_FLUSH_STATUSBIT_POS	   (6)

/** Bit mask for the flush status (bit 6) - indicates flush operation completion */
#define XV_FRMBUFWR_CTRL_BITS_FLUSH_STATUSBIT		   (1 << XV_FRMBUFWR_CTRL_BITS_FLUSH_STATUSBIT_POS)

/** Address of the Global Interrupt Enable Register */
#define XV_FRMBUFWR_CTRL_ADDR_GIE                      0x04

/** Address of the IP Interrupt Enable Register */
#define XV_FRMBUFWR_CTRL_ADDR_IER                      0x08

/** Address of the IP Interrupt Status Register */
#define XV_FRMBUFWR_CTRL_ADDR_ISR                      0x0c

/** Address of the hardware register for frame width configuration */
#define XV_FRMBUFWR_CTRL_ADDR_HWREG_WIDTH_DATA         0x10

/** Bit width of the frame width register (16 bits, supporting values 0-65535) */
#define XV_FRMBUFWR_CTRL_BITS_HWREG_WIDTH_DATA         16

/** Address of the hardware register for frame height configuration */
#define XV_FRMBUFWR_CTRL_ADDR_HWREG_HEIGHT_DATA        0x18

/** Bit width of the frame height register (16 bits, supporting values 0-65535) */
#define XV_FRMBUFWR_CTRL_BITS_HWREG_HEIGHT_DATA        16

/** Address of the hardware register for stride configuration (bytes per line) */
#define XV_FRMBUFWR_CTRL_ADDR_HWREG_STRIDE_DATA        0x20

/** Bit width of the stride register (16 bits, supporting values 0-65535) */
#define XV_FRMBUFWR_CTRL_BITS_HWREG_STRIDE_DATA        16

/** Address of the hardware register for video format configuration */
#define XV_FRMBUFWR_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA  0x28

/** Bit width of the video format register (16 bits) */
#define XV_FRMBUFWR_CTRL_BITS_HWREG_VIDEO_FORMAT_DATA  16

/** Base address of the hardware register for primary frame buffer pointer */
#define XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA  0x30

/** Bit width of the frame buffer address (64 bits for 64-bit address space) */
#define XV_FRMBUFWR_CTRL_BITS_HWREG_FRM_BUFFER_V_DATA  64

/** Base address of the hardware register for secondary frame buffer pointer (chroma plane) */
#define XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA 0x3c

/** Bit width of the secondary frame buffer address (64 bits) */
#define XV_FRMBUFWR_CTRL_BITS_HWREG_FRM_BUFFER2_V_DATA 64

/** Address of the hardware register for field ID (interlaced video field indicator) */
#define XV_FRMBUFWR_CTRL_ADDR_HWREG_FIELD_ID_DATA      0x48

/** Bit width of the field ID register (1 bit: 0 or 1) */
#define XV_FRMBUFWR_CTRL_BITS_HWREG_FIELD_ID_DATA      1

/** Base address of the hardware register for tertiary frame buffer pointer (second chroma plane) */
#define XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA 0x54

/** Bit width of the tertiary frame buffer address (64 bits) */
#define XV_FRMBUFWR_CTRL_BITS_HWREG_FRM_BUFFER3_V_DATA 64

/** Address of the hardware register for Forcing EOF */
#define XV_FRMBUFWR_CTRL_ADDR_FORCE_EOF_DATA  0x68

/** Bit width of the Forcing EOF register (1 bit: 0 or 1) */
#define XV_FRMBUFWR_CTRL_BITS_FORCE_EOF_DATA      1

/** Address of the hardware register for partial frame count */
#define XV_FRMBUFWR_CTRL_ADDR_PARTIAL_FRM_CNT_DATA  0x70

/** Bit width of the partial frame count register (16 bits, supporting values 0-65535) */
#define XV_FRMBUFWR_CTRL_BITS_PARTIAL_FRM_CNT_DATA      16

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
