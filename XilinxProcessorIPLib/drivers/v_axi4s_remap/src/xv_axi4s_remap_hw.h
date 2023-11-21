// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
#ifndef XV_AXI4S_REMAP_HW_H_  /* prevent circular inclusions */
#define XV_AXI4S_REMAP_HW_H_  /* by using protection macros  */

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
// 0x10 : Data signal of height
//        bit 15~0 - height[15:0] (Read/Write)
//        others   - reserved
// 0x14 : reserved
// 0x18 : Data signal of width
//        bit 15~0 - width[15:0] (Read/Write)
//        others   - reserved
// 0x1c : reserved
// 0x20 : Data signal of ColorFormat
//        bit 7~0 - ColorFormat[7:0] (Read/Write)
//        others  - reserved
// 0x24 : reserved
// 0x28 : Data signal of inPixClk
//        bit 7~0 - inPixClk[7:0] (Read/Write)
//        others  - reserved
// 0x2c : reserved
// 0x30 : Data signal of outPixClk
//        bit 7~0 - outPixClk[7:0] (Read/Write)
//        others  - reserved
// 0x34 : reserved
// 0x38 : Data signal of inHDMI420
//        bit 7~0 - inHDMI420[7:0] (Read/Write)
//        others  - reserved
// 0x3c : reserved
// 0x40 : Data signal of outHDMI420
//        bit 7~0 - outHDMI420[7:0] (Read/Write)
//        others  - reserved
// 0x44 : reserved
// 0x48 : Data signal of inPixDrop
//        bit 7~0 - inPixDrop[7:0] (Read/Write)
//        others  - reserved
// 0x4c : reserved
// 0x50 : Data signal of outPixRepeat
//        bit 7~0 - outPixRepeat[7:0] (Read/Write)
//        others  - reserved
// 0x54 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL           0x00
#define XV_AXI4S_REMAP_CTRL_ADDR_GIE               0x04
#define XV_AXI4S_REMAP_CTRL_ADDR_IER               0x08
#define XV_AXI4S_REMAP_CTRL_ADDR_ISR               0x0c
#define XV_AXI4S_REMAP_CTRL_ADDR_HEIGHT_DATA       0x10
#define XV_AXI4S_REMAP_CTRL_BITS_HEIGHT_DATA       16
#define XV_AXI4S_REMAP_CTRL_ADDR_WIDTH_DATA        0x18
#define XV_AXI4S_REMAP_CTRL_BITS_WIDTH_DATA        16
#define XV_AXI4S_REMAP_CTRL_ADDR_COLORFORMAT_DATA  0x20
#define XV_AXI4S_REMAP_CTRL_BITS_COLORFORMAT_DATA  8
#define XV_AXI4S_REMAP_CTRL_ADDR_INPIXCLK_DATA     0x28
#define XV_AXI4S_REMAP_CTRL_BITS_INPIXCLK_DATA     8
#define XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXCLK_DATA    0x30
#define XV_AXI4S_REMAP_CTRL_BITS_OUTPIXCLK_DATA    8
#define XV_AXI4S_REMAP_CTRL_ADDR_INHDMI420_DATA    0x38
#define XV_AXI4S_REMAP_CTRL_BITS_INHDMI420_DATA    8
#define XV_AXI4S_REMAP_CTRL_ADDR_OUTHDMI420_DATA   0x40
#define XV_AXI4S_REMAP_CTRL_BITS_OUTHDMI420_DATA   8
#define XV_AXI4S_REMAP_CTRL_ADDR_INPIXDROP_DATA    0x48
#define XV_AXI4S_REMAP_CTRL_BITS_INPIXDROP_DATA    8
#define XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXREPEAT_DATA 0x50
#define XV_AXI4S_REMAP_CTRL_BITS_OUTPIXREPEAT_DATA 8

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
