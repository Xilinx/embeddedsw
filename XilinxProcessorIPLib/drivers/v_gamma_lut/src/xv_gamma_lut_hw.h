// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_GAMMA_LUT_HW_H_  /* prevent circular inclusions */
#define XV_GAMMA_LUT_HW_H_  /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

// CTRL
// 0x0000 : Control signals
//          bit 0  - ap_start (Read/Write/COH)
//          bit 1  - ap_done (Read/COR)
//          bit 2  - ap_idle (Read)
//          bit 3  - ap_ready (Read)
//          bit 7  - auto_restart (Read/Write)
//          others - reserved
// 0x0004 : Global Interrupt Enable Register
//          bit 0  - Global Interrupt Enable (Read/Write)
//          others - reserved
// 0x0008 : IP Interrupt Enable Register (Read/Write)
//          bit 0  - Channel 0 (ap_done)
//          bit 1  - Channel 1 (ap_ready)
//          others - reserved
// 0x000c : IP Interrupt Status Register (Read/TOW)
//          bit 0  - Channel 0 (ap_done)
//          bit 1  - Channel 1 (ap_ready)
//          others - reserved
// 0x0010 : Data signal of HwReg_width
//          bit 15~0 - HwReg_width[15:0] (Read/Write)
//          others   - reserved
// 0x0014 : reserved
// 0x0018 : Data signal of HwReg_height
//          bit 15~0 - HwReg_height[15:0] (Read/Write)
//          others   - reserved
// 0x001c : reserved
// 0x0020 : Data signal of HwReg_video_format
//          bit 15~0 - HwReg_video_format[15:0] (Read/Write)
//          others   - reserved
// 0x0024 : reserved
// 0x0800 ~
// 0x0fff : Memory 'HwReg_gamma_lut_0' (1024 * 16b)
//          Word n : bit [15: 0] - HwReg_gamma_lut_0[2n]
//                   bit [31:16] - HwReg_gamma_lut_0[2n+1]
// 0x1000 ~
// 0x17ff : Memory 'HwReg_gamma_lut_1' (1024 * 16b)
//          Word n : bit [15: 0] - HwReg_gamma_lut_1[2n]
//                   bit [31:16] - HwReg_gamma_lut_1[2n+1]
// 0x1800 ~
// 0x1fff : Memory 'HwReg_gamma_lut_2' (1024 * 16b)
//          Word n : bit [15: 0] - HwReg_gamma_lut_2[2n]
//                   bit [31:16] - HwReg_gamma_lut_2[2n+1]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XV_GAMMA_LUT_CTRL_ADDR_AP_CTRL                 0x0000
#define XV_GAMMA_LUT_CTRL_ADDR_GIE                     0x0004
#define XV_GAMMA_LUT_CTRL_ADDR_IER                     0x0008
#define XV_GAMMA_LUT_CTRL_ADDR_ISR                     0x000c
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_WIDTH_DATA        0x0010
#define XV_GAMMA_LUT_CTRL_BITS_HWREG_WIDTH_DATA        16
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_HEIGHT_DATA       0x0018
#define XV_GAMMA_LUT_CTRL_BITS_HWREG_HEIGHT_DATA       16
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA 0x0020
#define XV_GAMMA_LUT_CTRL_BITS_HWREG_VIDEO_FORMAT_DATA 16
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE  0x0800
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_HIGH  0x0fff
#define XV_GAMMA_LUT_CTRL_WIDTH_HWREG_GAMMA_LUT_0      16
#define XV_GAMMA_LUT_CTRL_DEPTH_HWREG_GAMMA_LUT_0      1024
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE  0x1000
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_HIGH  0x17ff
#define XV_GAMMA_LUT_CTRL_WIDTH_HWREG_GAMMA_LUT_1      16
#define XV_GAMMA_LUT_CTRL_DEPTH_HWREG_GAMMA_LUT_1      1024
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE  0x1800
#define XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_HIGH  0x1fff
#define XV_GAMMA_LUT_CTRL_WIDTH_HWREG_GAMMA_LUT_2      16
#define XV_GAMMA_LUT_CTRL_DEPTH_HWREG_GAMMA_LUT_2      1024

#ifdef __cplusplus
}
#endif

#endif
