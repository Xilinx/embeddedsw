// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_letterbox_hw.h
 * @addtogroup xv_letterbox Overview
 */

#ifndef XV_LETTERBOX_HW_H_		 /* prevent circular inclusions */
#define XV_LETTERBOX_HW_H_		 /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

// CTRL
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
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
// 0x20 : Data signal of HwReg_video_format
//        bit 15~0 - HwReg_video_format[15:0] (Read/Write)
//        others   - reserved
// 0x24 : reserved
// 0x28 : Data signal of HwReg_col_start
//        bit 15~0 - HwReg_col_start[15:0] (Read/Write)
//        others   - reserved
// 0x2c : reserved
// 0x30 : Data signal of HwReg_col_end
//        bit 15~0 - HwReg_col_end[15:0] (Read/Write)
//        others   - reserved
// 0x34 : reserved
// 0x38 : Data signal of HwReg_row_start
//        bit 15~0 - HwReg_row_start[15:0] (Read/Write)
//        others   - reserved
// 0x3c : reserved
// 0x40 : Data signal of HwReg_row_end
//        bit 15~0 - HwReg_row_end[15:0] (Read/Write)
//        others   - reserved
// 0x44 : reserved
// 0x48 : Data signal of HwReg_Y_R_value
//        bit 15~0 - HwReg_Y_R_value[15:0] (Read/Write)
//        others   - reserved
// 0x4c : reserved
// 0x50 : Data signal of HwReg_Cb_G_value
//        bit 15~0 - HwReg_Cb_G_value[15:0] (Read/Write)
//        others   - reserved
// 0x54 : reserved
// 0x58 : Data signal of HwReg_Cr_B_value
//        bit 15~0 - HwReg_Cr_B_value[15:0] (Read/Write)
//        others   - reserved
// 0x5c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

/**
 * Register address and bit-width definitions for XV_LETTERBOX hardware control interface
 */
#define XV_LETTERBOX_CTRL_ADDR_AP_CTRL                 0x00  // Control signals register
#define XV_LETTERBOX_CTRL_ADDR_GIE                     0x04  // Global Interrupt Enable register
#define XV_LETTERBOX_CTRL_ADDR_IER                     0x08  // IP Interrupt Enable register
#define XV_LETTERBOX_CTRL_ADDR_ISR                     0x0c  // IP Interrupt Status register

#define XV_LETTERBOX_CTRL_ADDR_HWREG_WIDTH_DATA        0x10  // Width register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_WIDTH_DATA        16    // Width register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_HEIGHT_DATA       0x18  // Height register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_HEIGHT_DATA       16    // Height register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA 0x20  // Video format register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_VIDEO_FORMAT_DATA 16    // Video format register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_COL_START_DATA    0x28  // Column start register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_COL_START_DATA    16    // Column start register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_COL_END_DATA      0x30  // Column end register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_COL_END_DATA      16    // Column end register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_START_DATA    0x38  // Row start register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_ROW_START_DATA    16    // Row start register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_END_DATA      0x40  // Row end register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_ROW_END_DATA      16    // Row end register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_Y_R_VALUE_DATA    0x48  // Y/R value register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_Y_R_VALUE_DATA    16    // Y/R value register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_CB_G_VALUE_DATA   0x50  // Cb/G value register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_CB_G_VALUE_DATA   16    // Cb/G value register bit width

#define XV_LETTERBOX_CTRL_ADDR_HWREG_CR_B_VALUE_DATA   0x58  // Cr/B value register address
#define XV_LETTERBOX_CTRL_BITS_HWREG_CR_B_VALUE_DATA   16    // Cr/B value register bit width

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
