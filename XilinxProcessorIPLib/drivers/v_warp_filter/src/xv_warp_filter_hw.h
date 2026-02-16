// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_warp_filter_hw.h
 * @addtogroup v_warp_filter Overview
 */

// control
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
//        bit 0  - enable ap_done interrupt (Read/Write)
//        bit 1  - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - ap_done (COR/TOW)
//        bit 1  - ap_ready (COR/TOW)
//        others - reserved
// 0x10 : Data signal of desc_addr
//        bit 31~0 - desc_addr[31:0] (Read/Write)
// 0x14 : Data signal of desc_addr
//        bit 31~0 - desc_addr[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of maxi_read
//        bit 31~0 - maxi_read[31:0] (Read/Write)
// 0x20 : Data signal of maxi_read
//        bit 31~0 - maxi_read[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of maxi_reads
//        bit 31~0 - maxi_reads[31:0] (Read/Write)
// 0x2c : Data signal of maxi_reads
//        bit 31~0 - maxi_reads[63:32] (Read/Write)
// 0x30 : reserved
// 0x34 : Data signal of maxi_write
//        bit 31~0 - maxi_write[31:0] (Read/Write)
// 0x38 : Data signal of maxi_write
//        bit 31~0 - maxi_write[63:32] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of maxi_read1
//        bit 31~0 - maxi_read1[31:0] (Read/Write)
// 0x44 : Data signal of maxi_read1
//        bit 31~0 - maxi_read1[63:32] (Read/Write)
// 0x48 : reserved
// 0x4c : Data signal of maxi_read1s
//        bit 31~0 - maxi_read1s[31:0] (Read/Write)
// 0x50 : Data signal of maxi_read1s
//        bit 31~0 - maxi_read1s[63:32] (Read/Write)
// 0x54 : reserved
// 0x58 : Data signal of maxi_write1
//        bit 31~0 - maxi_write1[31:0] (Read/Write)
// 0x5c : Data signal of maxi_write1
//        bit 31~0 - maxi_write1[63:32] (Read/Write)
// 0x60 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

/***************** Macros (Inline Functions) Definitions *********************/

/** Control register address offset */
#define XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL          0x00
/** Global Interrupt Enable register address offset */
#define XV_WARP_FILTER_CONTROL_ADDR_GIE              0x04
/** Interrupt Enable Register address offset */
#define XV_WARP_FILTER_CONTROL_ADDR_IER              0x08
/** Interrupt Status Register address offset */
#define XV_WARP_FILTER_CONTROL_ADDR_ISR              0x0c
/** Descriptor address register offset */
#define XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA   0x10
/** Descriptor address data width in bits */
#define XV_WARP_FILTER_CONTROL_BITS_DESC_ADDR_DATA   64
/** AXI master read address register offset */
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA   0x1c
/** AXI master read address data width in bits */
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_READ_DATA   64
/** AXI master reads address register offset */
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA  0x28
/** AXI master reads address data width in bits */
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_READS_DATA  64
/** AXI master write address register offset */
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA  0x34
/** AXI master write address data width in bits */
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_WRITE_DATA  64
/** AXI master read1 address register offset */
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA  0x40
/** AXI master read1 address data width in bits */
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_READ1_DATA  64
/** AXI master read1s address register offset */
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA 0x4c
/** AXI master read1s address data width in bits */
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_READ1S_DATA 64
/** AXI master write1 address register offset */
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA 0x58
/** AXI master write1 address data width in bits */
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_WRITE1_DATA 64

/** Start bit mask in AP_CTRL register */
#define AP_CTRL_START_MASK		0x01
/** Done bit mask in AP_CTRL register */
#define AP_CTRL_DONE_MASK		0x02
/** Idle bit mask in AP_CTRL register */
#define AP_CTRL_IDLE_MASK		0x04
/** Ready bit mask in AP_CTRL register */
#define AP_CTRL_READY_MASK		0x08
/** Auto-restart bit mask in AP_CTRL register */
#define AP_CTRL_AUTO_RST_MASK	0x80
/** Flush bit position in AP_CTRL register */
#define AP_CTRL_BITS_FLUSH_BIT_POS		       (5)
/** Flush bit mask in AP_CTRL register */
#define AP_CTRL_BITS_FLUSH_BIT			       (1 << AP_CTRL_BITS_FLUSH_BIT_POS)
/** Flush status bit position in AP_CTRL register */
#define AP_CTRL_BITS_FLUSH_STATUSBIT_POS	   (6)
/** Flush status bit mask in AP_CTRL register */
#define AP_CTRL_BITS_FLUSH_STATUSBIT		   (1 << AP_CTRL_BITS_FLUSH_STATUSBIT_POS)

/** AP_DONE interrupt mask for IER, ISR, and GIE registers */
#define XV_WARP_FILTER_INTR_AP_DONE_MASK		0x01
/** AP_READY interrupt mask for IER, ISR, and GIE registers */
#define XV_WARP_FILTER_INTR_AP_READY_MASK		0x02
