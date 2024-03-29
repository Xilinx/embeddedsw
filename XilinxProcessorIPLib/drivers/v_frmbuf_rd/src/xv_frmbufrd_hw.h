// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
#ifndef XV_FRMBUFRD_HW_H_  /* prevent circular inclusions */
#define XV_FRMBUFRD_HW_H_  /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

// CTRL
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 5  - Flush pending AXI transactions
//        bit 6  - Flush done
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
// 0x20 : Data signal of HwReg_stride
//        bit 15~0 - HwReg_stride[15:0] (Read/Write)
//        others   - reserved
// 0x24 : reserved
// 0x28 : Data signal of HwReg_video_format
//        bit 15~0 - HwReg_video_format[15:0] (Read/Write)
//        others   - reserved
// 0x2c : reserved
// 0x30 : Data signal of HwReg_frm_buffer_V
//        bit 31~0 - HwReg_frm_buffer_V[31:0] (Read/Write)
// 0x34 : Data signal of HwReg_frm_buffer_V
//        bit 31~0 - HwReg_frm_buffer_V[63:32] (Read/Write)
// 0x38 : reserved
// 0x3c : Data signal of HwReg_frm_buffer2_V
//        bit 31~0 - HwReg_frm_buffer2_V[31:0] (Read/Write)
// 0x40 : Data signal of HwReg_frm_buffer2_V
//        bit 31~0 - HwReg_frm_buffer2_V[63:32] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of HwReg_field_id
//        bit 0  - HwReg_field_id[0] (Read/Write)
//        others - reserved
// 0x4c : reserved
// 0x50 : Data signal of fidOutMode
//        bit 31~0 - fidOutMode[31:0] (Read/Write)
// 0x54 : reserved
// 0x58 : Data signal of fid_error_i
//        bit 31~0 - fid_error_i[31:0] (Read/Write)
// 0x5c : reserved
// 0x60 : Data signal of fid_error_o
//        bit 31~0 - fid_error_o[31:0] (Read)
// 0x64 : reserved
// 0x68 : Data signal of HwReg_field_out
//        bit 0  - HwReg_field_out[0] (Read)
//        others - reserved
// 0x6c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XV_FRMBUFRD_CTRL_ADDR_AP_CTRL                  	0x00
#define XV_FRMBUFRD_CTRL_BITS_FLUSH_BIT_POS	      	   	(5)
#define XV_FRMBUFRD_CTRL_BITS_FLUSH_BIT		           	(1 << XV_FRMBUFRD_CTRL_BITS_FLUSH_BIT_POS)
#define XV_FRMBUFRD_CTRL_BITS_FLUSH_STATUSBIT_POS      	(6)
#define XV_FRMBUFRD_CTRL_BITS_FLUSH_STATUSBIT          	(1 << XV_FRMBUFRD_CTRL_BITS_FLUSH_STATUSBIT_POS)
#define XV_FRMBUFRD_CTRL_ADDR_GIE                      	0x04
#define XV_FRMBUFRD_CTRL_ADDR_IER                      	0x08
#define XV_FRMBUFRD_CTRL_ADDR_ISR                      	0x0c
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_WIDTH_DATA         	0x10
#define XV_FRMBUFRD_CTRL_BITS_HWREG_WIDTH_DATA         	16
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_HEIGHT_DATA        	0x18
#define XV_FRMBUFRD_CTRL_BITS_HWREG_HEIGHT_DATA        	16
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_STRIDE_DATA        	0x20
#define XV_FRMBUFRD_CTRL_BITS_HWREG_STRIDE_DATA        	16
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA  	0x28
#define XV_FRMBUFRD_CTRL_BITS_HWREG_VIDEO_FORMAT_DATA  	16
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA  	0x30
#define XV_FRMBUFRD_CTRL_BITS_HWREG_FRM_BUFFER_V_DATA  	64
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA 	0x3c
#define XV_FRMBUFRD_CTRL_BITS_HWREG_FRM_BUFFER2_V_DATA 	64
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_ID_DATA      	0x48
#define XV_FRMBUFRD_CTRL_BITS_HWREG_FIELD_ID_DATA      	0x1
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_FIDOUTMODE_DATA     0x50
#define XV_FRMBUFRD_CTRL_BITS_HWREG_FIDOUTMODE_DATA     0x3
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_FID_ERROR_DATA     	0x58
#define XV_FRMBUFRD_CTRL_BITS_HWREG_FID_ERROR_DATA     	0x1
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_OUT_DATA 	0x60
#define XV_FRMBUFRD_CTRL_BITS_HWREG_FIELD_OUT_DATA 	0x1
#define XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA  0x74
#define XV_FRMBUFRD_CTRL_BITS_HWREG_FRM_BUFFER3_V_DATA  64

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
