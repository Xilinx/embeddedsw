// ==============================================================
// Copyright (c) 2015 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_tpg_hw.h
 * @addtogroup v_tpg Overview
 */

#ifndef XV_TPG_HW_H_  /* prevent circular inclusions */
#define XV_TPG_HW_H_  /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

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
// 0x20 : Data signal of bckgndId
//        bit 7~0 - bckgndId[7:0] (Read/Write)
//        others  - reserved
// 0x24 : reserved
// 0x28 : Data signal of ovrlayId
//        bit 7~0 - ovrlayId[7:0] (Read/Write)
//        others  - reserved
// 0x2c : reserved
// 0x30 : Data signal of maskId
//        bit 7~0 - maskId[7:0] (Read/Write)
//        others  - reserved
// 0x34 : reserved
// 0x38 : Data signal of motionSpeed
//        bit 7~0 - motionSpeed[7:0] (Read/Write)
//        others  - reserved
// 0x3c : reserved
// 0x40 : Data signal of colorFormat
//        bit 7~0 - colorFormat[7:0] (Read/Write)
//        others  - reserved
// 0x44 : reserved
// 0x48 : Data signal of crossHairX
//        bit 15~0 - crossHairX[15:0] (Read/Write)
//        others   - reserved
// 0x4c : reserved
// 0x50 : Data signal of crossHairY
//        bit 15~0 - crossHairY[15:0] (Read/Write)
//        others   - reserved
// 0x54 : reserved
// 0x58 : Data signal of ZplateHorContStart
//        bit 15~0 - ZplateHorContStart[15:0] (Read/Write)
//        others   - reserved
// 0x5c : reserved
// 0x60 : Data signal of ZplateHorContDelta
//        bit 15~0 - ZplateHorContDelta[15:0] (Read/Write)
//        others   - reserved
// 0x64 : reserved
// 0x68 : Data signal of ZplateVerContStart
//        bit 15~0 - ZplateVerContStart[15:0] (Read/Write)
//        others   - reserved
// 0x6c : reserved
// 0x70 : Data signal of ZplateVerContDelta
//        bit 15~0 - ZplateVerContDelta[15:0] (Read/Write)
//        others   - reserved
// 0x74 : reserved
// 0x78 : Data signal of boxSize
//        bit 15~0 - boxSize[15:0] (Read/Write)
//        others   - reserved
// 0x7c : reserved
// 0x80 : Data signal of boxColorR
//        bit 15~0 - boxColorR[15:0] (Read/Write)
//        others   - reserved
// 0x84 : reserved
// 0x88 : Data signal of boxColorG
//        bit 15~0 - boxColorG[15:0] (Read/Write)
//        others   - reserved
// 0x8c : reserved
// 0x90 : Data signal of boxColorB
//        bit 15~0 - boxColorB[15:0] (Read/Write)
//        others   - reserved
// 0x94 : reserved
// 0x98 : Data signal of enableInput
//        bit 7~0 - enableInput[7:0] (Read/Write)
//        others  - reserved
// 0x9c : reserved
// 0xa0 : Data signal of passthruStartX
//        bit 15~0 - passthruStartX[15:0] (Read/Write)
//        others   - reserved
// 0xa4 : reserved
// 0xa8 : Data signal of passthruStartY
//        bit 15~0 - passthruStartY[15:0] (Read/Write)
//        others   - reserved
// 0xac : reserved
// 0xb0 : Data signal of passthruEndX
//        bit 15~0 - passthruEndX[15:0] (Read/Write)
//        others   - reserved
// 0xb4 : reserved
// 0xb8 : Data signal of passthruEndY
//        bit 15~0 - passthruEndY[15:0] (Read/Write)
//        others   - reserved
// 0xbc : reserved
// 0xc0 : Data signal of dpDynamicRange
//        bit 7~0 - dpDynamicRange[7:0] (Read/Write)
//        others  - reserved
// 0xc4 : reserved
// 0xc8 : Data signal of dpYUVCoef
//        bit 7~0 - dpYUVCoef[7:0] (Read/Write)
//        others  - reserved
// 0xcc : reserved
// 0xd0 : Data signal of FieldId
//        bit 0  - progressive/interlaced (Read/Write)
//        bit 1  - polarity bit (Read/Write)
//        bit 2  - fid value passthrough enable (Read/Write)
//        others  - reserved
//        bit [2:0] -
//		0x0 - Progressive mode
//		0x1 - Interlaced mode
//		0x2 - Progressive mode with reverse polarity
//		0x3 - Interlaced mode with reverse polarity
//		0x4 - Passthrough mode
//		Others - Progressive mode
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

/**
 * Control register address for AP control signals (start, done, idle, ready, auto-restart)
 */
#define XV_TPG_CTRL_ADDR_AP_CTRL                 0x00

/**
 * Global Interrupt Enable register address
 */
#define XV_TPG_CTRL_ADDR_GIE                     0x04

/**
 * Interrupt Enable register address
 */
#define XV_TPG_CTRL_ADDR_IER                     0x08

/**
 * Interrupt Status register address
 */
#define XV_TPG_CTRL_ADDR_ISR                     0x0c

/**
 * Height data register address
 */
#define XV_TPG_CTRL_ADDR_HEIGHT_DATA             0x10

/**
 * Height data bit width
 */
#define XV_TPG_CTRL_BITS_HEIGHT_DATA             16

/**
 * Width data register address
 */
#define XV_TPG_CTRL_ADDR_WIDTH_DATA              0x18

/**
 * Width data bit width
 */
#define XV_TPG_CTRL_BITS_WIDTH_DATA              16

/**
 * Background pattern ID register address
 */
#define XV_TPG_CTRL_ADDR_BCKGNDID_DATA           0x20

/**
 * Background pattern ID bit width
 */
#define XV_TPG_CTRL_BITS_BCKGNDID_DATA           8

/**
 * Overlay pattern ID register address
 */
#define XV_TPG_CTRL_ADDR_OVRLAYID_DATA           0x28

/**
 * Overlay pattern ID bit width
 */
#define XV_TPG_CTRL_BITS_OVRLAYID_DATA           8

/**
 * Mask ID register address
 */
#define XV_TPG_CTRL_ADDR_MASKID_DATA             0x30

/**
 * Mask ID bit width
 */
#define XV_TPG_CTRL_BITS_MASKID_DATA             8

/**
 * Motion speed register address
 */
#define XV_TPG_CTRL_ADDR_MOTIONSPEED_DATA        0x38

/**
 * Motion speed bit width
 */
#define XV_TPG_CTRL_BITS_MOTIONSPEED_DATA        8

/**
 * Color format register address
 */
#define XV_TPG_CTRL_ADDR_COLORFORMAT_DATA        0x40

/**
 * Color format bit width
 */
#define XV_TPG_CTRL_BITS_COLORFORMAT_DATA        8

/**
 * Crosshair X coordinate register address
 */
#define XV_TPG_CTRL_ADDR_CROSSHAIRX_DATA         0x48

/**
 * Crosshair X coordinate bit width
 */
#define XV_TPG_CTRL_BITS_CROSSHAIRX_DATA         16

/**
 * Crosshair Y coordinate register address
 */
#define XV_TPG_CTRL_ADDR_CROSSHAIRY_DATA         0x50

/**
 * Crosshair Y coordinate bit width
 */
#define XV_TPG_CTRL_BITS_CROSSHAIRY_DATA         16

/**
 * Zone plate horizontal control start register address
 */
#define XV_TPG_CTRL_ADDR_ZPLATEHORCONTSTART_DATA 0x58

/**
 * Zone plate horizontal control start bit width
 */
#define XV_TPG_CTRL_BITS_ZPLATEHORCONTSTART_DATA 16

/**
 * Zone plate horizontal control delta register address
 */
#define XV_TPG_CTRL_ADDR_ZPLATEHORCONTDELTA_DATA 0x60

/**
 * Zone plate horizontal control delta bit width
 */
#define XV_TPG_CTRL_BITS_ZPLATEHORCONTDELTA_DATA 16

/**
 * Zone plate vertical control start register address
 */
#define XV_TPG_CTRL_ADDR_ZPLATEVERCONTSTART_DATA 0x68

/**
 * Zone plate vertical control start bit width
 */
#define XV_TPG_CTRL_BITS_ZPLATEVERCONTSTART_DATA 16

/**
 * Zone plate vertical control delta register address
 */
#define XV_TPG_CTRL_ADDR_ZPLATEVERCONTDELTA_DATA 0x70

/**
 * Zone plate vertical control delta bit width
 */
#define XV_TPG_CTRL_BITS_ZPLATEVERCONTDELTA_DATA 16

/**
 * Box size register address
 */
#define XV_TPG_CTRL_ADDR_BOXSIZE_DATA            0x78

/**
 * Box size bit width
 */
#define XV_TPG_CTRL_BITS_BOXSIZE_DATA            16

/**
 * Box color red component register address
 */
#define XV_TPG_CTRL_ADDR_BOXCOLORR_DATA          0x80

/**
 * Box color red component bit width
 */
#define XV_TPG_CTRL_BITS_BOXCOLORR_DATA          16

/**
 * Box color green component register address
 */
#define XV_TPG_CTRL_ADDR_BOXCOLORG_DATA          0x88

/**
 * Box color green component bit width
 */
#define XV_TPG_CTRL_BITS_BOXCOLORG_DATA          16

/**
 * Box color blue component register address
 */
#define XV_TPG_CTRL_ADDR_BOXCOLORB_DATA          0x90

/**
 * Box color blue component bit width
 */
#define XV_TPG_CTRL_BITS_BOXCOLORB_DATA          16

/**
 * Enable input register address
 */
#define XV_TPG_CTRL_ADDR_ENABLEINPUT_DATA        0x98

/**
 * Enable input bit width
 */
#define XV_TPG_CTRL_BITS_ENABLEINPUT_DATA        8

/**
 * Passthrough start X coordinate register address
 */
#define XV_TPG_CTRL_ADDR_PASSTHRUSTARTX_DATA     0xa0

/**
 * Passthrough start X coordinate bit width
 */
#define XV_TPG_CTRL_BITS_PASSTHRUSTARTX_DATA     16

/**
 * Passthrough start Y coordinate register address
 */
#define XV_TPG_CTRL_ADDR_PASSTHRUSTARTY_DATA     0xa8

/**
 * Passthrough start Y coordinate bit width
 */
#define XV_TPG_CTRL_BITS_PASSTHRUSTARTY_DATA     16

/**
 * Passthrough end X coordinate register address
 */
#define XV_TPG_CTRL_ADDR_PASSTHRUENDX_DATA       0xb0

/**
 * Passthrough end X coordinate bit width
 */
#define XV_TPG_CTRL_BITS_PASSTHRUENDX_DATA       16

/**
 * Passthrough end Y coordinate register address
 */
#define XV_TPG_CTRL_ADDR_PASSTHRUENDY_DATA       0xb8

/**
 * Passthrough end Y coordinate bit width
 */
#define XV_TPG_CTRL_BITS_PASSTHRUENDY_DATA       16

/**
 * DisplayPort dynamic range register address
 */
#define XV_TPG_CTRL_ADDR_DPDYNAMICRANGE_DATA     0xc0

/**
 * DisplayPort dynamic range bit width
 */
#define XV_TPG_CTRL_BITS_DPDYNAMICRANGE_DATA     8

/**
 * DisplayPort YUV coefficient register address
 */
#define XV_TPG_CTRL_ADDR_DPYUVCOEF_DATA          0xc8

/**
 * DisplayPort YUV coefficient bit width
 */
#define XV_TPG_CTRL_BITS_DPYUVCOEF_DATA          8

/**
 * Field ID register address for interlaced/progressive mode control
 */
#define XV_TPG_CTRL_ADDR_FIELDID_DATA            0xd0

/**
 * Field ID bit width
 */
#define XV_TPG_CTRL_BITS_FIELDID_DATA		 16

/**
 * Motion enable mask bit
 */
#define XV_TPG_CTRL_ADDR_MOTION_EN_MASK		(1<<0)

/**
 * Background motion enable register address
 */
#define XV_TPG_CTRL_ADDR_BCK_MOTION_EN_DATA      0xd8

/**
 * Background motion enable bit width
 */
#define XV_TPG_CTRL_BITS_BCK_MOTION_EN_DATA      16

/**
 * Field ID interlaced mode bit mask
 */
#define XV_TPG_CTRL_ADDR_FIELDID_INTERLACED_MASK (1<<0)

/**
 * Field ID interlaced mode bit shift
 */
#define XV_TPG_CTRL_ADDR_FIELDID_INTERLACED_SHIFT 0

/**
 * Field ID polarity bit mask
 */
#define XV_TPG_CTRL_ADDR_FIELDID_POLARITY_MASK	 (1<<1)

/**
 * Field ID polarity bit shift
 */
#define XV_TPG_CTRL_ADDR_FIELDID_POLARITY_SHIFT	 1

/**
 * Field ID passthrough mode bit mask
 */
#define XV_TPG_CTRL_ADDR_FIELDID_PASSTHR_MASK	 (1<<2)

/**
 * Field ID passthrough mode bit shift
 */
#define XV_TPG_CTRL_ADDR_FIELDID_PASSTHR_SHIFT	 2

#ifdef __cplusplus
}
#endif

#endif
