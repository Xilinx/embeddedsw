// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
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

#define XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL          0x00
#define XV_WARP_FILTER_CONTROL_ADDR_GIE              0x04
#define XV_WARP_FILTER_CONTROL_ADDR_IER              0x08
#define XV_WARP_FILTER_CONTROL_ADDR_ISR              0x0c
#define XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA   0x10
#define XV_WARP_FILTER_CONTROL_BITS_DESC_ADDR_DATA   64
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA   0x1c
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_READ_DATA   64
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA  0x28
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_READS_DATA  64
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA  0x34
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_WRITE_DATA  64
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA  0x40
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_READ1_DATA  64
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA 0x4c
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_READ1S_DATA 64
#define XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA 0x58
#define XV_WARP_FILTER_CONTROL_BITS_MAXI_WRITE1_DATA 64

/* Bit masks for AP_CTRL*/
#define AP_CTRL_START_MASK		0x01
#define AP_CTRL_DONE_MASK		0x02
#define AP_CTRL_IDLE_MASK		0x04
#define AP_CTRL_READY_MASK		0x08
#define AP_CTRL_AUTO_RST_MASK	0x80
#define AP_CTRL_BITS_FLUSH_BIT_POS		       (5)
#define AP_CTRL_BITS_FLUSH_BIT			       (1 << AP_CTRL_BITS_FLUSH_BIT_POS)
#define AP_CTRL_BITS_FLUSH_STATUSBIT_POS	   (6)
#define AP_CTRL_BITS_FLUSH_STATUSBIT		   (1 << AP_CTRL_BITS_FLUSH_STATUSBIT_POS)

/* Bit masks for IER, ISR, GIE*/
#define XV_WARP_FILTER_INTR_AP_DONE_MASK		0x01
#define XV_WARP_FILTER_INTR_AP_READY_MASK		0x02
