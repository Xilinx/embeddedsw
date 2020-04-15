/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

// CTRL
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 5  - flush (Read/Write)
//        bit 6  - flush_done (Read)
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
// 0x10 : Data signal of HwStructIn_status
//        bit 31~0 - HwStructIn_status[31:0] (Read)
// 0x14 : Data signal of HwStructIn_status
//        bit 31~0 - HwStructIn_status[63:32] (Read)
// 0x18 : reserved
// 0x20 : Data signal of HwStructIn_mux_context
//        bit 31~0 - HwStructIn_mux_context[31:0] (Read/Write)
// 0x24 : Data signal of HwStructIn_mux_context
//        bit 31~0 - HwStructIn_mux_context[63:32] (Read/Write)
// 0x28 : reserved
// 0x30 : Data signal of HwStructIn_stream_context
//        bit 31~0 - HwStructIn_stream_context[31:0] (Read/Write)
// 0x34 : Data signal of HwStructIn_stream_context
//        bit 31~0 - HwStructIn_stream_context[63:32] (Read/Write)
// 0x38 : reserved
// 0x40 : Data signal of HwStructIn_data_in
//        bit 31~0 - HwStructIn_data_in[31:0] (Read/Write)
// 0x44 : reserved
// 0x60 : Data signal of HwStructIn_data_out_byte_inf
//        bit 31~0 - HwStructIn_data_out_byte_inf[31:0] (Read/Write)
// 0x64 : reserved
// 0x70 : Data signal of HwStructIn_num_desc
//        bit 7~0 - HwStructIn_num_desc[7:0] (Read/Write)
//        others  - reserved
// 0x74 : reserved
// 0x78 : Data signal of HwStructIn_stream_id_table
//        bit 31~0 - HwStructIn_stream_id_table[31:0] (Read/Write)
// 0x7c : Data signal of HwStructIn_stream_id_table
//        bit 31~0 - HwStructIn_stream_id_table[63:32] (Read/Write)
// 0x80 : reserved
// 0x48 : Data signal of HwStructIn_num_streams_table
//        bit 31~0 - HwStructIn_num_streams_table[31:0] (Read/Write)
// 0x4c : Data signal of HwStructIn_num_streams_table
//        bit 31~0 - HwStructIn_num_streams_table[63:32] (Read/Write)
// 0x50 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XMPEGTSMUX_CTRL_ADDR_AP_CTRL                           0x00
#define XMPEGTSMUX_CTRL_ADDR_GIE                               0x04
#define XMPEGTSMUX_CTRL_ADDR_IER                               0x08
#define XMPEGTSMUX_CTRL_ADDR_ISR                               0x0c
#define XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STATUS_DATA            0x10
#define XMPEGTSMUX_CTRL_BITS_HWSTRUCTIN_STATUS_DATA            64
#define XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_MUX_CONTEXT_DATA       0x20
#define XMPEGTSMUX_CTRL_BITS_HWSTRUCTIN_MUX_CONTEXT_DATA       64
#define XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_CONTEXT_DATA    0x30
#define XMPEGTSMUX_CTRL_BITS_HWSTRUCTIN_STREAM_CONTEXT_DATA    64
#define XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_DATA_IN_DATA           0x40
#define XMPEGTSMUX_CTRL_BITS_HWSTRUCTIN_DATA_IN_DATA           32
#define XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_DATA_OUT_BYTE_INF_DATA 0x60
#define XMPEGTSMUX_CTRL_BITS_HWSTRUCTIN_DATA_OUT_BYTE_INF_DATA 32
#define XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_NUM_DESC_DATA          0x70
#define XMPEGTSMUX_CTRL_BITS_HWSTRUCTIN_NUM_DESC_DATA          8
#define XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_ID_TABLE_DATA   0x78
#define XMPEGTSMUX_CTRL_BITS_HWSTRUCTIN_STREAM_ID_TABLE_DATA   64
#define XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_NUM_STREAMS_TABLE_DATA 0x48
#define XMPEGTSMUX_CTRL_BITS_HWSTRUCTIN_NUM_STREAMS_TABLE_DATA 64

#define XMPEGTSMUX_CTRL_EN_MASK	                               0x1
#define XMPEGTSMUX_CTRL_DIS_MASK	                       0
#define XMPEGTSMUX_CTRL_IS_DONE_MASK	                       0x1
#define XMPEGTSMUX_CTRL_IS_DONE_SHIFT   	               1
#define XMPEGTSMUX_CTRL_IS_IDLE_MASK	                       0x1
#define XMPEGTSMUX_CTRL_IS_IDLE_SHIFT   	               2
#define XMPEGTSMUX_CTRL_AUTO_RESTART_MASK	               0x80
