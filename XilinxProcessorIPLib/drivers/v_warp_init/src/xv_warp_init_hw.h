// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
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
// 0x30 : Data signal of ip_status_reg
//        bit 31~0 - ip_status_reg[31:0] (Read/Write)
// 0x34 : reserved
// 0x38 : Data signal of maxi_read_write
//        bit 31~0 - maxi_read_write[31:0] (Read/Write)
// 0x3c : Data signal of maxi_read_write
//        bit 31~0 - maxi_read_write[63:32] (Read/Write)
// 0x40 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XV_WARP_INIT_CTRL_ADDR_AP_CTRL              0x00
#define XV_WARP_INIT_CTRL_ADDR_GIE                  0x04
#define XV_WARP_INIT_CTRL_ADDR_IER                  0x08
#define XV_WARP_INIT_CTRL_ADDR_ISR                  0x0c
#define XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA       0x10
#define XV_WARP_INIT_CTRL_BITS_DESC_ADDR_DATA       64
#define XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA 0x20
#define XV_WARP_INIT_CTRL_BITS_MAXI_READ_WRITE_DATA 64
#define XV_WARP_INIT_CTRL_ADDR_IP_STATUS_REG_DATA   0x30
#define XV_WARP_INIT_CTRL_BITS_IP_STATUS_REG_DATA   32


/* Bit masks for AP_CTRL*/
#define AP_CTRL_START_MASK		0x01
#define AP_CTRL_DONE_MASK		0x02
#define AP_CTRL_IDLE_MASK		0x04
#define AP_CTRL_READY_MASK		0x08
#define AP_CTRL_AUTO_RST_MASK	0x80
#define AP_CTRL_FLUSH_BIT_POS			(5)
#define AP_CTRL_FLUSH_BIT				(1 << AP_CTRL_FLUSH_BIT_POS)
#define AP_CTRL_FLUSH_STATUSBIT_POS		(6)
#define AP_CTRL_FLUSH_STATUSBIT			(1 << AP_CTRL_FLUSH_STATUSBIT_POS)

/* Bit masks for IER, ISR, GIE*/
#define XV_WARP_INIT_INTR_AP_DONE_MASK		0x01
#define XV_WARP_INIT_INTR_AP_READY_MASK		0x02
