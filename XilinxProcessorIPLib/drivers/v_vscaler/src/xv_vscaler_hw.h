// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_vscaler_hw.h
 * @addtogroup xv_vscaler Overview
 */

#ifndef XV_VSCALER_HW_H_ 	 /* prevent circular inclusions */
#define XV_VSCALER_HW_H_	 /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

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
// 0x010 : Data signal of HwReg_HeightIn
//         bit 15~0 - HwReg_HeightIn[15:0] (Read/Write)
//         others   - reserved
// 0x014 : reserved
// 0x018 : Data signal of HwReg_Width
//         bit 15~0 - HwReg_Width[15:0] (Read/Write)
//         others   - reserved
// 0x01c : reserved
// 0x020 : Data signal of HwReg_HeightOut
//         bit 15~0 - HwReg_HeightOut[15:0] (Read/Write)
//         others   - reserved
// 0x024 : reserved
// 0x028 : Data signal of HwReg_LineRate
//         bit 31~0 - HwReg_LineRate[31:0] (Read/Write)
// 0x02c : reserved
// 0x030 : Data signal of HwReg_ColorMode
//         bit 7~0 - HwReg_ColorMode[7:0] (Read/Write)
//         others  - reserved
// 0x034 : reserved
// 0x400 ~
// 0x7ff : Memory 'HwReg_vfltCoeff' (512 * 16b)
//         Word n : bit [15: 0] - HwReg_vfltCoeff[2n]
//                  bit [31:16] - HwReg_vfltCoeff[2n+1]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

/**
 * Register address and bit-width definitions for XV_VSCALER hardware module.
 *
 * XV_VSCALER_CTRL_ADDR_AP_CTRL:
 *   - Address: 0x000
 *   - Description: Control register for starting, stopping, and monitoring the hardware.
 *
 * XV_VSCALER_CTRL_ADDR_GIE:
 *   - Address: 0x004
 *   - Description: Global Interrupt Enable register.
 *
 * XV_VSCALER_CTRL_ADDR_IER:
 *   - Address: 0x008
 *   - Description: Interrupt Enable Register.
 *
 * XV_VSCALER_CTRL_ADDR_ISR:
 *   - Address: 0x00c
 *   - Description: Interrupt Status Register.
 *
 * XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTIN_DATA:
 *   - Address: 0x010
 *   - Bits: 16
 *   - Description: Input frame height register.
 *
 * XV_VSCALER_CTRL_ADDR_HWREG_WIDTH_DATA:
 *   - Address: 0x018
 *   - Bits: 16
 *   - Description: Input frame width register.
 *
 * XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTOUT_DATA:
 *   - Address: 0x020
 *   - Bits: 16
 *   - Description: Output frame height register.
 *
 * XV_VSCALER_CTRL_ADDR_HWREG_LINERATE_DATA:
 *   - Address: 0x028
 *   - Bits: 32
 *   - Description: Line rate configuration register.
 *
 * XV_VSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA:
 *   - Address: 0x030
 *   - Bits: 8
 *   - Description: Color mode configuration register.
 *
 * XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE:
 *   - Address: 0x800
 *   - Description: Base address for vertical filter coefficients.
 *
 * XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH:
 *   - Address: 0xbff
 *   - Description: High address for vertical filter coefficients.
 *
 * XV_VSCALER_CTRL_WIDTH_HWREG_VFLTCOEFF:
 *   - Width: 16
 *   - Description: Bit width of each vertical filter coefficient.
 *
 * XV_VSCALER_CTRL_DEPTH_HWREG_VFLTCOEFF:
 *   - Depth: 512
 *   - Description: Number of vertical filter coefficients.
 */
#define XV_VSCALER_CTRL_ADDR_AP_CTRL              0x000
#define XV_VSCALER_CTRL_ADDR_GIE                  0x004
#define XV_VSCALER_CTRL_ADDR_IER                  0x008
#define XV_VSCALER_CTRL_ADDR_ISR                  0x00c
#define XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTIN_DATA  0x010
#define XV_VSCALER_CTRL_BITS_HWREG_HEIGHTIN_DATA  16
#define XV_VSCALER_CTRL_ADDR_HWREG_WIDTH_DATA     0x018
#define XV_VSCALER_CTRL_BITS_HWREG_WIDTH_DATA     16
#define XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTOUT_DATA 0x020
#define XV_VSCALER_CTRL_BITS_HWREG_HEIGHTOUT_DATA 16
#define XV_VSCALER_CTRL_ADDR_HWREG_LINERATE_DATA  0x028
#define XV_VSCALER_CTRL_BITS_HWREG_LINERATE_DATA  32
#define XV_VSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA 0x030
#define XV_VSCALER_CTRL_BITS_HWREG_COLORMODE_DATA 8
#define XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE 0x800
#define XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH 0xbff
#define XV_VSCALER_CTRL_WIDTH_HWREG_VFLTCOEFF     16
#define XV_VSCALER_CTRL_DEPTH_HWREG_VFLTCOEFF     512

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
