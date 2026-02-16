// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_demosaic_hw.h
 * @addtogroup v_demosaic Overview
 */

#ifndef XV_DEMOSAIC_HW_H_  /* prevent circular inclusions */
#define XV_DEMOSAIC_HW_H_  /* by using protection macros  */

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
// 0x28 : Data signal of HwReg_bayer_phase
//        bit 15~0 - HwReg_bayer_phase[15:0] (Read/Write)
//        others   - reserved
// 0x2c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

/** Control register address - AP control signals */
#define XV_DEMOSAIC_CTRL_ADDR_AP_CTRL                0x00
/** Global interrupt enable register address */
#define XV_DEMOSAIC_CTRL_ADDR_GIE                    0x04
/** Interrupt enable register address */
#define XV_DEMOSAIC_CTRL_ADDR_IER                    0x08
/** Interrupt status register address */
#define XV_DEMOSAIC_CTRL_ADDR_ISR                    0x0c
/** Hardware register width data address */
#define XV_DEMOSAIC_CTRL_ADDR_HWREG_WIDTH_DATA       0x10
/** Hardware register width data bit width */
#define XV_DEMOSAIC_CTRL_BITS_HWREG_WIDTH_DATA       16
/** Hardware register height data address */
#define XV_DEMOSAIC_CTRL_ADDR_HWREG_HEIGHT_DATA      0x18
/** Hardware register height data bit width */
#define XV_DEMOSAIC_CTRL_BITS_HWREG_HEIGHT_DATA      16
/** Hardware register Bayer phase data address */
#define XV_DEMOSAIC_CTRL_ADDR_HWREG_BAYER_PHASE_DATA 0x28
/** Hardware register Bayer phase data bit width */
#define XV_DEMOSAIC_CTRL_BITS_HWREG_BAYER_PHASE_DATA 16

#ifdef __cplusplus
}
#endif

#endif
