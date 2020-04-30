// ==============================================================
// Copyright (c) 1986-2020 Xilinx, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
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
// 0x10 : Data signal of width
//        bit 31~0 - width[31:0] (Read/Write)
// 0x14 : reserved
// 0x18 : Data signal of height
//        bit 31~0 - height[31:0] (Read/Write)
// 0x1c : reserved
// 0x20 : Data signal of read_fb_V
//        bit 31~0 - read_fb_V[31:0] (Read/Write)
// 0x24 : Data signal of read_fb_V
//        bit 31~0 - read_fb_V[63:32] (Read/Write)
// 0x28 : reserved
// 0x30 : Data signal of colorFormat
//        bit 7~0 - colorFormat[7:0] (Read/Write)
//        others  - reserved
// 0x34 : reserved
// 0x38 : Data signal of algo
//        bit 7~0 - algo[7:0] (Read/Write)
//        others  - reserved
// 0x3c : reserved
// 0x40 : Data signal of invert_field_id
//        bit 0  - invert_field_id[0] (Read/Write)
//        others - reserved
// 0x44 : reserved
// 0x50 : Data signal of write_fb_V
//        bit 31~0 - write_fb_V[31:0] (Read/Write)
// 0x54 : Data signal of write_fb_V
//        bit 31~0 - write_fb_V[63:32] (Read/Write)
// 0x58 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XV_DEINTERLACER_CTRL_ADDR_AP_CTRL              0x00
#define XV_DEINTERLACER_CTRL_ADDR_GIE                  0x04
#define XV_DEINTERLACER_CTRL_ADDR_IER                  0x08
#define XV_DEINTERLACER_CTRL_ADDR_ISR                  0x0c
#define XV_DEINTERLACER_CTRL_ADDR_WIDTH_DATA           0x10
#define XV_DEINTERLACER_CTRL_BITS_WIDTH_DATA           32
#define XV_DEINTERLACER_CTRL_ADDR_HEIGHT_DATA          0x18
#define XV_DEINTERLACER_CTRL_BITS_HEIGHT_DATA          32
#define XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA       0x20
#define XV_DEINTERLACER_CTRL_BITS_READ_FB_V_DATA       64
#define XV_DEINTERLACER_CTRL_ADDR_COLORFORMAT_DATA     0x30
#define XV_DEINTERLACER_CTRL_BITS_COLORFORMAT_DATA     8
#define XV_DEINTERLACER_CTRL_ADDR_ALGO_DATA            0x38
#define XV_DEINTERLACER_CTRL_BITS_ALGO_DATA            8
#define XV_DEINTERLACER_CTRL_ADDR_INVERT_FIELD_ID_DATA 0x40
#define XV_DEINTERLACER_CTRL_BITS_INVERT_FIELD_ID_DATA 1
#define XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA      0x50
#define XV_DEINTERLACER_CTRL_BITS_WRITE_FB_V_DATA      64

