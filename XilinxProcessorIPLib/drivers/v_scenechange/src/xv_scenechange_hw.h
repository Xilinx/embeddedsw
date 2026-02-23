// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_scenechange_hw.h
 * @addtogroup v_scenechange Overview
 */

// CTRL
// 0x000 : Control signals
//         bit 0  - ap_start (Read/Write/COH)
//         bit 1  - ap_done (Read/COR)
//         bit 2  - ap_idle (Read)
//         bit 3  - ap_ready (Read)
//         bit 7  - auto_restart (Read/Write)
//         others - reserved
// 0x004 : Global Interrupt Enable Register
//         bit 0  - Global Interrupt Enable (Read/Write)
//         others - reserved
// 0x008 : IP Interrupt Enable Register (Read/Write)
//         bit 0  - Channel 0 (ap_done)
//         bit 1  - Channel 1 (ap_ready)
//         others - reserved
// 0x00c : IP Interrupt Status Register (Read/TOW)
//         bit 0  - Channel 0 (ap_done)
//         bit 1  - Channel 1 (ap_ready)
//         others - reserved
// 0x010 : Data signal of HwReg_width_0
//         bit 15~0 - HwReg_width_0[15:0] (Read/Write)
//         others   - reserved
// 0x014 : reserved
// 0x018 : Data signal of HwReg_height_0
//         bit 15~0 - HwReg_height_0[15:0] (Read/Write)
//         others   - reserved
// 0x01c : reserved
// 0x020 : Data signal of HwReg_stride_0
//         bit 15~0 - HwReg_stride_0[15:0] (Read/Write)
//         others   - reserved
// 0x024 : reserved
// 0x028 : Data signal of HwReg_video_format_0
//         bit 15~0 - HwReg_video_format_0[15:0] (Read/Write)
//         others   - reserved
// 0x02c : reserved
// 0x030 : Data signal of HwReg_subsample_0
//         bit 15~0 - HwReg_subsample_0[15:0] (Read/Write)
//         others   - reserved
// 0x034 : reserved
// 0x038 : Data signal of HwReg_sad0
//         bit 31~0 - HwReg_sad0[31:0] (Read)
// 0x03c : Control signal of HwReg_sad0
//         bit 0  - HwReg_sad0_ap_vld (Read/COR)
//         others - reserved
// 0x040 : Data signal of HwReg_frm_buffer0_V
//         bit 31~0 - HwReg_frm_buffer0_V[31:0] (Read/Write)
// 0x044 : reserved
// 0x110 : Data signal of HwReg_width_1
//         bit 15~0 - HwReg_width_1[15:0] (Read/Write)
//         others   - reserved
// 0x114 : reserved
// 0x118 : Data signal of HwReg_height_1
//         bit 15~0 - HwReg_height_1[15:0] (Read/Write)
//         others   - reserved
// 0x11c : reserved
// 0x120 : Data signal of HwReg_stride_1
//         bit 15~0 - HwReg_stride_1[15:0] (Read/Write)
//         others   - reserved
// 0x124 : reserved
// 0x128 : Data signal of HwReg_video_format_1
//         bit 15~0 - HwReg_video_format_1[15:0] (Read/Write)
//         others   - reserved
// 0x12c : reserved
// 0x130 : Data signal of HwReg_subsample_1
//         bit 15~0 - HwReg_subsample_1[15:0] (Read/Write)
//         others   - reserved
// 0x134 : reserved
// 0x138 : Data signal of HwReg_sad1
//         bit 31~0 - HwReg_sad1[31:0] (Read)
// 0x13c : reserved
// 0x140 : Data signal of HwReg_frm_buffer1_V
//         bit 31~0 - HwReg_frm_buffer1_V[31:0] (Read/Write)
// 0x144 : reserved
// 0x210 : Data signal of HwReg_width_2
//         bit 15~0 - HwReg_width_2[15:0] (Read/Write)
//         others   - reserved
// 0x214 : reserved
// 0x218 : Data signal of HwReg_height_2
//         bit 15~0 - HwReg_height_2[15:0] (Read/Write)
//         others   - reserved
// 0x21c : reserved
// 0x220 : Data signal of HwReg_stride_2
//         bit 15~0 - HwReg_stride_2[15:0] (Read/Write)
//         others   - reserved
// 0x224 : reserved
// 0x228 : Data signal of HwReg_video_format_2
//         bit 15~0 - HwReg_video_format_2[15:0] (Read/Write)
//         others   - reserved
// 0x22c : reserved
// 0x230 : Data signal of HwReg_subsample_2
//         bit 15~0 - HwReg_subsample_2[15:0] (Read/Write)
//         others   - reserved
// 0x234 : reserved
// 0x238 : Data signal of HwReg_sad2
//         bit 31~0 - HwReg_sad2[31:0] (Read)
// 0x23c : reserved
// 0x240 : Data signal of HwReg_frm_buffer2_V
//         bit 31~0 - HwReg_frm_buffer2_V[31:0] (Read/Write)
// 0x244 : reserved
// 0x310 : Data signal of HwReg_width_3
//         bit 15~0 - HwReg_width_3[15:0] (Read/Write)
//         others   - reserved
// 0x314 : reserved
// 0x318 : Data signal of HwReg_height_3
//         bit 15~0 - HwReg_height_3[15:0] (Read/Write)
//         others   - reserved
// 0x31c : reserved
// 0x320 : Data signal of HwReg_stride_3
//         bit 15~0 - HwReg_stride_3[15:0] (Read/Write)
//         others   - reserved
// 0x324 : reserved
// 0x328 : Data signal of HwReg_video_format_3
//         bit 15~0 - HwReg_video_format_3[15:0] (Read/Write)
//         others   - reserved
// 0x32c : reserved
// 0x330 : Data signal of HwReg_subsample_3
//         bit 15~0 - HwReg_subsample_3[15:0] (Read/Write)
//         others   - reserved
// 0x334 : reserved
// 0x338 : Data signal of HwReg_sad3
//         bit 31~0 - HwReg_sad3[31:0] (Read)
// 0x33c : reserved
// 0x340 : Data signal of HwReg_frm_buffer3_V
//         bit 31~0 - HwReg_frm_buffer3_V[31:0] (Read/Write)
// 0x344 : reserved
// 0x410 : Data signal of HwReg_width_4
//         bit 15~0 - HwReg_width_4[15:0] (Read/Write)
//         others   - reserved
// 0x414 : reserved
// 0x418 : Data signal of HwReg_height_4
//         bit 15~0 - HwReg_height_4[15:0] (Read/Write)
//         others   - reserved
// 0x41c : reserved
// 0x420 : Data signal of HwReg_stride_4
//         bit 15~0 - HwReg_stride_4[15:0] (Read/Write)
//         others   - reserved
// 0x424 : reserved
// 0x428 : Data signal of HwReg_video_format_4
//         bit 15~0 - HwReg_video_format_4[15:0] (Read/Write)
//         others   - reserved
// 0x42c : reserved
// 0x430 : Data signal of HwReg_subsample_4
//         bit 15~0 - HwReg_subsample_4[15:0] (Read/Write)
//         others   - reserved
// 0x434 : reserved
// 0x438 : Data signal of HwReg_sad4
//         bit 31~0 - HwReg_sad4[31:0] (Read)
// 0x43c : reserved
// 0x440 : Data signal of HwReg_frm_buffer4_V
//         bit 31~0 - HwReg_frm_buffer4_V[31:0] (Read/Write)
// 0x444 : reserved
// 0x510 : Data signal of HwReg_width_5
//         bit 15~0 - HwReg_width_5[15:0] (Read/Write)
//         others   - reserved
// 0x514 : reserved
// 0x518 : Data signal of HwReg_height_5
//         bit 15~0 - HwReg_height_5[15:0] (Read/Write)
//         others   - reserved
// 0x51c : reserved
// 0x520 : Data signal of HwReg_stride_5
//         bit 15~0 - HwReg_stride_5[15:0] (Read/Write)
//         others   - reserved
// 0x524 : reserved
// 0x528 : Data signal of HwReg_video_format_5
//         bit 15~0 - HwReg_video_format_5[15:0] (Read/Write)
//         others   - reserved
// 0x52c : reserved
// 0x530 : Data signal of HwReg_subsample_5
//         bit 15~0 - HwReg_subsample_5[15:0] (Read/Write)
//         others   - reserved
// 0x534 : reserved
// 0x538 : Data signal of HwReg_sad5
//         bit 31~0 - HwReg_sad5[31:0] (Read)
// 0x53c : reserved
// 0x540 : Data signal of HwReg_frm_buffer5_V
//         bit 31~0 - HwReg_frm_buffer5_V[31:0] (Read/Write)
// 0x544 : reserved
// 0x610 : Data signal of HwReg_width_6
//         bit 15~0 - HwReg_width_6[15:0] (Read/Write)
//         others   - reserved
// 0x614 : reserved
// 0x618 : Data signal of HwReg_height_6
//         bit 15~0 - HwReg_height_6[15:0] (Read/Write)
//         others   - reserved
// 0x61c : reserved
// 0x620 : Data signal of HwReg_stride_6
//         bit 15~0 - HwReg_stride_6[15:0] (Read/Write)
//         others   - reserved
// 0x624 : reserved
// 0x628 : Data signal of HwReg_video_format_6
//         bit 15~0 - HwReg_video_format_6[15:0] (Read/Write)
//         others   - reserved
// 0x62c : reserved
// 0x630 : Data signal of HwReg_subsample_6
//         bit 15~0 - HwReg_subsample_6[15:0] (Read/Write)
//         others   - reserved
// 0x634 : reserved
// 0x638 : Data signal of HwReg_sad6
//         bit 31~0 - HwReg_sad6[31:0] (Read)
// 0x63c : reserved
// 0x640 : Data signal of HwReg_frm_buffer6_V
//         bit 31~0 - HwReg_frm_buffer6_V[31:0] (Read/Write)
// 0x644 : reserved
// 0x710 : Data signal of HwReg_width_7
//         bit 15~0 - HwReg_width_7[15:0] (Read/Write)
//         others   - reserved
// 0x714 : reserved
// 0x718 : Data signal of HwReg_height_7
//         bit 15~0 - HwReg_height_7[15:0] (Read/Write)
//         others   - reserved
// 0x71c : reserved
// 0x720 : Data signal of HwReg_stride_7
//         bit 15~0 - HwReg_stride_7[15:0] (Read/Write)
//         others   - reserved
// 0x724 : reserved
// 0x728 : Data signal of HwReg_video_format_7
//         bit 15~0 - HwReg_video_format_7[15:0] (Read/Write)
//         others   - reserved
// 0x72c : reserved
// 0x730 : Data signal of HwReg_subsample_7
//         bit 15~0 - HwReg_subsample_7[15:0] (Read/Write)
//         others   - reserved
// 0x734 : reserved
// 0x738 : Data signal of HwReg_sad7
//         bit 31~0 - HwReg_sad7[31:0] (Read)
// 0x73c : reserved
// 0x740 : Data signal of HwReg_frm_buffer7_V
//         bit 31~0 - HwReg_frm_buffer7_V[31:0] (Read/Write)
// 0x744 : reserved
// 0x780 : Data signal of HwReg_stream_enable
//         bit 7~0 - HwReg_stream_enable[7:0] (Read/Write)
//         others  - reserved
// 0x784 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)



/************************** Constant Definitions *****************************/
/* Control and Status Registers */
/** Control register address for AP control signals (start, done, idle, ready, auto_restart) */
#define XV_SCENECHANGE_CTRL_ADDR_AP_CTRL                   0x000
/** Global Interrupt Enable register address */
#define XV_SCENECHANGE_CTRL_ADDR_GIE                       0x004
/** IP Interrupt Enable register address */
#define XV_SCENECHANGE_CTRL_ADDR_IER                       0x008
/** IP Interrupt Status register address */
#define XV_SCENECHANGE_CTRL_ADDR_ISR                       0x00c
/** AP Done interrupt bit mask in ISR register */
#define XV_SCENECHANGE_CTRL_ADDR_ISR_AP_DONE		   0x001

/* Flush Control and Status Bits */
/** Bit position for flush done status in control register */
#define XV_SCDFLUSH_DONE_BIT				   5
/** Bit position for flush status in control register */
#define XV_SCDFLUSH_STS_BIT				   6
/** Bit mask for flush done status bit */
#define XV_SCD_CTRL_FLUSH_DONE_BIT_MASK			   (1<<XV_SCDFLUSH_DONE_BIT)
/** Bit mask for flush status bit */
#define XV_SCD_CTRL_FLUSH_STS_BIT_MASK			   (1<<XV_SCDFLUSH_STS_BIT)

/* Stream 0 Register Offsets and Bit Widths */
/** Stream 0 width register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_0_DATA        0x010
/** Stream 0 width register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_WIDTH_0_DATA        16
/** Stream 0 height register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_0_DATA       0x018
/** Stream 0 height register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_HEIGHT_0_DATA       16
/** Stream 0 stride register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_0_DATA       0x020
/** Stream 0 stride register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STRIDE_0_DATA       16
/** Stream 0 video format register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_0_DATA 0x028
/** Stream 0 video format register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_VIDEO_FORMAT_0_DATA 16
/** Stream 0 subsample register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_0_DATA    0x030
/** Stream 0 subsample register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SUBSAMPLE_0_DATA    16
/** Stream 0 SAD (Sum of Absolute Differences) data register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD0_DATA           0x038
/** Stream 0 SAD data register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SAD0_DATA           32
/** Stream 0 SAD control register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD0_CTRL           0x03c
/** Stream 0 frame buffer address register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER0_V_DATA  0x040
/** Stream 0 frame buffer address register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_FRM_BUFFER0_V_DATA  32

/* Stream 1 Register Offsets and Bit Widths */
/** Stream 1 width register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_1_DATA        0x110
/** Stream 1 width register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_WIDTH_1_DATA        16
/** Stream 1 height register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_1_DATA       0x118
/** Stream 1 height register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_HEIGHT_1_DATA       16
/** Stream 1 stride register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_1_DATA       0x120
/** Stream 1 stride register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STRIDE_1_DATA       16
/** Stream 1 video format register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_1_DATA 0x128
/** Stream 1 video format register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_VIDEO_FORMAT_1_DATA 16
/** Stream 1 subsample register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_1_DATA    0x130
/** Stream 1 subsample register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SUBSAMPLE_1_DATA    16
/** Stream 1 SAD (Sum of Absolute Differences) data register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD1_DATA           0x138
/** Stream 1 SAD data register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SAD1_DATA           32
/** Stream 1 frame buffer address register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER1_V_DATA  0x140
/** Stream 1 frame buffer address register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_FRM_BUFFER1_V_DATA  32

/* Stream 2 Register Offsets and Bit Widths */
/** Stream 2 width register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_2_DATA        0x210
/** Stream 2 width register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_WIDTH_2_DATA        16
/** Stream 2 height register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_2_DATA       0x218
/** Stream 2 height register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_HEIGHT_2_DATA       16
/** Stream 2 stride register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_2_DATA       0x220
/** Stream 2 stride register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STRIDE_2_DATA       16
/** Stream 2 video format register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_2_DATA 0x228
/** Stream 2 video format register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_VIDEO_FORMAT_2_DATA 16
/** Stream 2 subsample register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_2_DATA    0x230
/** Stream 2 subsample register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SUBSAMPLE_2_DATA    16
/** Stream 2 SAD (Sum of Absolute Differences) data register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD2_DATA           0x238
/** Stream 2 SAD data register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SAD2_DATA           32
/** Stream 2 frame buffer address register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA  0x240
/** Stream 2 frame buffer address register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_FRM_BUFFER2_V_DATA  32

/* Stream 3 Register Offsets and Bit Widths */
/** Stream 3 width register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_3_DATA        0x310
/** Stream 3 width register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_WIDTH_3_DATA        16
/** Stream 3 height register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_3_DATA       0x318
/** Stream 3 height register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_HEIGHT_3_DATA       16
/** Stream 3 stride register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_3_DATA       0x320
/** Stream 3 stride register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STRIDE_3_DATA       16
/** Stream 3 video format register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_3_DATA 0x328
/** Stream 3 video format register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_VIDEO_FORMAT_3_DATA 16
/** Stream 3 subsample register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_3_DATA    0x330
/** Stream 3 subsample register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SUBSAMPLE_3_DATA    16
/** Stream 3 SAD (Sum of Absolute Differences) data register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD3_DATA           0x338
/** Stream 3 SAD data register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SAD3_DATA           32
/** Stream 3 frame buffer address register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA  0x340
/** Stream 3 frame buffer address register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_FRM_BUFFER3_V_DATA  32

/* Stream 4 Register Offsets and Bit Widths */
/** Stream 4 width register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_4_DATA        0x410
/** Stream 4 width register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_WIDTH_4_DATA        16
/** Stream 4 height register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_4_DATA       0x418
/** Stream 4 height register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_HEIGHT_4_DATA       16
/** Stream 4 stride register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_4_DATA       0x420
/** Stream 4 stride register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STRIDE_4_DATA       16
/** Stream 4 video format register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_4_DATA 0x428
/** Stream 4 video format register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_VIDEO_FORMAT_4_DATA 16
/** Stream 4 subsample register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_4_DATA    0x430
/** Stream 4 subsample register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SUBSAMPLE_4_DATA    16
/** Stream 4 SAD (Sum of Absolute Differences) data register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD4_DATA           0x438
/** Stream 4 SAD data register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SAD4_DATA           32
/** Stream 4 frame buffer address register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER4_V_DATA  0x440
/** Stream 4 frame buffer address register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_FRM_BUFFER4_V_DATA  32

/* Stream 5 Register Offsets and Bit Widths */
/** Stream 5 width register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_5_DATA        0x510
/** Stream 5 width register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_WIDTH_5_DATA        16
/** Stream 5 height register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_5_DATA       0x518
/** Stream 5 height register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_HEIGHT_5_DATA       16
/** Stream 5 stride register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_5_DATA       0x520
/** Stream 5 stride register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STRIDE_5_DATA       16
/** Stream 5 video format register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_5_DATA 0x528
/** Stream 5 video format register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_VIDEO_FORMAT_5_DATA 16
/** Stream 5 subsample register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_5_DATA    0x530
/** Stream 5 subsample register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SUBSAMPLE_5_DATA    16
/** Stream 5 SAD (Sum of Absolute Differences) data register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD5_DATA           0x538
/** Stream 5 SAD data register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SAD5_DATA           32
/** Stream 5 frame buffer address register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER5_V_DATA  0x540
/** Stream 5 frame buffer address register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_FRM_BUFFER5_V_DATA  32

/* Stream 6 Register Offsets and Bit Widths */
/** Stream 6 width register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_6_DATA        0x610
/** Stream 6 width register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_WIDTH_6_DATA        16
/** Stream 6 height register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_6_DATA       0x618
/** Stream 6 height register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_HEIGHT_6_DATA       16
/** Stream 6 stride register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_6_DATA       0x620
/** Stream 6 stride register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STRIDE_6_DATA       16
/** Stream 6 video format register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_6_DATA 0x628
/** Stream 6 video format register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_VIDEO_FORMAT_6_DATA 16
/** Stream 6 subsample register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_6_DATA    0x630
/** Stream 6 subsample register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SUBSAMPLE_6_DATA    16
/** Stream 6 SAD (Sum of Absolute Differences) data register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD6_DATA           0x638
/** Stream 6 SAD data register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SAD6_DATA           32
/** Stream 6 frame buffer address register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER6_V_DATA  0x640
/** Stream 6 frame buffer address register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_FRM_BUFFER6_V_DATA  32

/* Stream 7 Register Offsets and Bit Widths */
/** Stream 7 width register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_7_DATA        0x710
/** Stream 7 width register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_WIDTH_7_DATA        16
/** Stream 7 height register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_7_DATA       0x718
/** Stream 7 height register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_HEIGHT_7_DATA       16
/** Stream 7 stride register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_7_DATA       0x720
/** Stream 7 stride register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STRIDE_7_DATA       16
/** Stream 7 video format register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_7_DATA 0x728
/** Stream 7 video format register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_VIDEO_FORMAT_7_DATA 16
/** Stream 7 subsample register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_7_DATA    0x730
/** Stream 7 subsample register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SUBSAMPLE_7_DATA    16
/** Stream 7 SAD (Sum of Absolute Differences) data register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD7_DATA           0x738
/** Stream 7 SAD data register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_SAD7_DATA           32
/** Stream 7 frame buffer address register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER7_V_DATA  0x740
/** Stream 7 frame buffer address register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_FRM_BUFFER7_V_DATA  32

/* Global Stream Enable Register */
/** Global stream enable register offset */
#define XV_SCENECHANGE_CTRL_ADDR_HWREG_STREAM_ENABLE_DATA  0x780
/** Global stream enable register bit width */
#define XV_SCENECHANGE_CTRL_BITS_HWREG_STREAM_ENABLE_DATA  8
