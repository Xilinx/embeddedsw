// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_HSCALER_HW_H_        /* prevent circular inclusions */
#define XV_HSCALER_HW_H_        /* by using protection macros  */

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
// 0x0010 : Data signal of HwReg_Height
//          bit 15~0 - HwReg_Height[15:0] (Read/Write)
//          others   - reserved
// 0x0014 : reserved
// 0x0018 : Data signal of HwReg_WidthIn
//          bit 15~0 - HwReg_WidthIn[15:0] (Read/Write)
//          others   - reserved
// 0x001c : reserved
// 0x0020 : Data signal of HwReg_WidthOut
//          bit 15~0 - HwReg_WidthOut[15:0] (Read/Write)
//          others   - reserved
// 0x0024 : reserved
// 0x0028 : Data signal of HwReg_ColorMode
//          bit 7~0 - HwReg_ColorMode[7:0] (Read/Write)
//          others  - reserved
// 0x002c : reserved
// 0x0030 : Data signal of HwReg_PixelRate
//          bit 31~0 - HwReg_PixelRate[31:0] (Read/Write)
// 0x0034 : reserved
// 0x0038 : Data signal of HwReg_ColorModeOut
//          bit 7~0 - HwReg_ColorModeOut[7:0] (Read/Write)
//          others  - reserved
// 0x003c : reserved
// 0x0800 ~
// 0x0bff : Memory 'HwReg_hfltCoeff' (384 * 16b)
//          Word n : bit [15: 0] - HwReg_hfltCoeff[2n]
//                   bit [31:16] - HwReg_hfltCoeff[2n+1]
// 0x4000 ~
// 0x7fff : Memory 'HwReg_phasesH_V' (8192 * 9b)
//          Word n : bit [ 8: 0] - HwReg_phasesH_V[2n]
//                   bit [24:16] - HwReg_phasesH_V[2n+1]
//                   others      - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XV_HSCALER_CTRL_ADDR_AP_CTRL              0x0000
#define XV_HSCALER_CTRL_ADDR_GIE                  0x0004
#define XV_HSCALER_CTRL_ADDR_IER                  0x0008
#define XV_HSCALER_CTRL_ADDR_ISR                  0x000c
#define XV_HSCALER_CTRL_ADDR_HWREG_HEIGHT_DATA    0x0010
#define XV_HSCALER_CTRL_BITS_HWREG_HEIGHT_DATA    16
#define XV_HSCALER_CTRL_ADDR_HWREG_WIDTHIN_DATA   0x0018
#define XV_HSCALER_CTRL_BITS_HWREG_WIDTHIN_DATA   16
#define XV_HSCALER_CTRL_ADDR_HWREG_WIDTHOUT_DATA  0x0020
#define XV_HSCALER_CTRL_BITS_HWREG_WIDTHOUT_DATA  16
#define XV_HSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA 0x0028
#define XV_HSCALER_CTRL_BITS_HWREG_COLORMODE_DATA 8
#define XV_HSCALER_CTRL_ADDR_HWREG_PIXELRATE_DATA 0x0030
#define XV_HSCALER_CTRL_BITS_HWREG_PIXELRATE_DATA 32
#define XV_HSCALER_CTRL_ADDR_HWREG_COLORMODEOUT_DATA 0x0038
#define XV_HSCALER_CTRL_BITS_HWREG_COLORMODEOUT_DATA 8
#define XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE 0x0800
#define XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH 0x0bff
#define XV_HSCALER_CTRL_WIDTH_HWREG_HFLTCOEFF     16
#define XV_HSCALER_CTRL_DEPTH_HWREG_HFLTCOEFF     384
#define XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE 0x4000
#define XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH 0x7fff
#define XV_HSCALER_CTRL_WIDTH_HWREG_PHASESH_V     9
#define XV_HSCALER_CTRL_DEPTH_HWREG_PHASESH_V     8192

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
