// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_CSC_HW_H_  /* prevent circular inclusions */
#define XV_CSC_HW_H_  /* by using protection macros  */

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
// 0x010 : Data signal of HwReg_InVideoFormat
//         bit 7~0 - HwReg_InVideoFormat[7:0] (Read/Write)
//         others  - reserved
// 0x014 : reserved
// 0x018 : Data signal of HwReg_OutVideoFormat
//         bit 7~0 - HwReg_OutVideoFormat[7:0] (Read/Write)
//         others  - reserved
// 0x01c : reserved
// 0x020 : Data signal of HwReg_width
//         bit 15~0 - HwReg_width[15:0] (Read/Write)
//         others   - reserved
// 0x024 : reserved
// 0x028 : Data signal of HwReg_height
//         bit 15~0 - HwReg_height[15:0] (Read/Write)
//         others   - reserved
// 0x02c : reserved
// 0x030 : Data signal of HwReg_ColStart
//         bit 15~0 - HwReg_ColStart[15:0] (Read/Write)
//         others   - reserved
// 0x034 : reserved
// 0x038 : Data signal of HwReg_ColEnd
//         bit 15~0 - HwReg_ColEnd[15:0] (Read/Write)
//         others   - reserved
// 0x03c : reserved
// 0x040 : Data signal of HwReg_RowStart
//         bit 15~0 - HwReg_RowStart[15:0] (Read/Write)
//         others   - reserved
// 0x044 : reserved
// 0x048 : Data signal of HwReg_RowEnd
//         bit 15~0 - HwReg_RowEnd[15:0] (Read/Write)
//         others   - reserved
// 0x04c : reserved
// 0x050 : Data signal of HwReg_K11
//         bit 15~0 - HwReg_K11[15:0] (Read/Write)
//         others   - reserved
// 0x054 : reserved
// 0x058 : Data signal of HwReg_K12
//         bit 15~0 - HwReg_K12[15:0] (Read/Write)
//         others   - reserved
// 0x05c : reserved
// 0x060 : Data signal of HwReg_K13
//         bit 15~0 - HwReg_K13[15:0] (Read/Write)
//         others   - reserved
// 0x064 : reserved
// 0x068 : Data signal of HwReg_K21
//         bit 15~0 - HwReg_K21[15:0] (Read/Write)
//         others   - reserved
// 0x06c : reserved
// 0x070 : Data signal of HwReg_K22
//         bit 15~0 - HwReg_K22[15:0] (Read/Write)
//         others   - reserved
// 0x074 : reserved
// 0x078 : Data signal of HwReg_K23
//         bit 15~0 - HwReg_K23[15:0] (Read/Write)
//         others   - reserved
// 0x07c : reserved
// 0x080 : Data signal of HwReg_K31
//         bit 15~0 - HwReg_K31[15:0] (Read/Write)
//         others   - reserved
// 0x084 : reserved
// 0x088 : Data signal of HwReg_K32
//         bit 15~0 - HwReg_K32[15:0] (Read/Write)
//         others   - reserved
// 0x08c : reserved
// 0x090 : Data signal of HwReg_K33
//         bit 15~0 - HwReg_K33[15:0] (Read/Write)
//         others   - reserved
// 0x094 : reserved
// 0x098 : Data signal of HwReg_ROffset_V
//         bit 11~0 - HwReg_ROffset_V[11:0] (Read/Write)
//         others   - reserved
// 0x09c : reserved
// 0x0a0 : Data signal of HwReg_GOffset_V
//         bit 11~0 - HwReg_GOffset_V[11:0] (Read/Write)
//         others   - reserved
// 0x0a4 : reserved
// 0x0a8 : Data signal of HwReg_BOffset_V
//         bit 11~0 - HwReg_BOffset_V[11:0] (Read/Write)
//         others   - reserved
// 0x0ac : reserved
// 0x0b0 : Data signal of HwReg_ClampMin_V
//         bit 9~0 - HwReg_ClampMin_V[9:0] (Read/Write)
//         others  - reserved
// 0x0b4 : reserved
// 0x0b8 : Data signal of HwReg_ClipMax_V
//         bit 9~0 - HwReg_ClipMax_V[9:0] (Read/Write)
//         others  - reserved
// 0x0bc : reserved
// 0x0c0 : Data signal of HwReg_K11_2
//         bit 15~0 - HwReg_K11_2[15:0] (Read/Write)
//         others   - reserved
// 0x0c4 : reserved
// 0x0c8 : Data signal of HwReg_K12_2
//         bit 15~0 - HwReg_K12_2[15:0] (Read/Write)
//         others   - reserved
// 0x0cc : reserved
// 0x0d0 : Data signal of HwReg_K13_2
//         bit 15~0 - HwReg_K13_2[15:0] (Read/Write)
//         others   - reserved
// 0x0d4 : reserved
// 0x0d8 : Data signal of HwReg_K21_2
//         bit 15~0 - HwReg_K21_2[15:0] (Read/Write)
//         others   - reserved
// 0x0dc : reserved
// 0x0e0 : Data signal of HwReg_K22_2
//         bit 15~0 - HwReg_K22_2[15:0] (Read/Write)
//         others   - reserved
// 0x0e4 : reserved
// 0x0e8 : Data signal of HwReg_K23_2
//         bit 15~0 - HwReg_K23_2[15:0] (Read/Write)
//         others   - reserved
// 0x0ec : reserved
// 0x0f0 : Data signal of HwReg_K31_2
//         bit 15~0 - HwReg_K31_2[15:0] (Read/Write)
//         others   - reserved
// 0x0f4 : reserved
// 0x0f8 : Data signal of HwReg_K32_2
//         bit 15~0 - HwReg_K32_2[15:0] (Read/Write)
//         others   - reserved
// 0x0fc : reserved
// 0x100 : Data signal of HwReg_K33_2
//         bit 15~0 - HwReg_K33_2[15:0] (Read/Write)
//         others   - reserved
// 0x104 : reserved
// 0x108 : Data signal of HwReg_ROffset_2_V
//         bit 11~0 - HwReg_ROffset_2_V[11:0] (Read/Write)
//         others   - reserved
// 0x10c : reserved
// 0x110 : Data signal of HwReg_GOffset_2_V
//         bit 11~0 - HwReg_GOffset_2_V[11:0] (Read/Write)
//         others   - reserved
// 0x114 : reserved
// 0x118 : Data signal of HwReg_BOffset_2_V
//         bit 11~0 - HwReg_BOffset_2_V[11:0] (Read/Write)
//         others   - reserved
// 0x11c : reserved
// 0x120 : Data signal of HwReg_ClampMin_2_V
//         bit 9~0 - HwReg_ClampMin_2_V[9:0] (Read/Write)
//         others  - reserved
// 0x124 : reserved
// 0x128 : Data signal of HwReg_ClipMax_2_V
//         bit 9~0 - HwReg_ClipMax_2_V[9:0] (Read/Write)
//         others  - reserved
// 0x12c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XV_CSC_CTRL_ADDR_AP_CTRL                   0x000
#define XV_CSC_CTRL_ADDR_GIE                       0x004
#define XV_CSC_CTRL_ADDR_IER                       0x008
#define XV_CSC_CTRL_ADDR_ISR                       0x00c
#define XV_CSC_CTRL_ADDR_HWREG_INVIDEOFORMAT_DATA  0x010
#define XV_CSC_CTRL_BITS_HWREG_INVIDEOFORMAT_DATA  8
#define XV_CSC_CTRL_ADDR_HWREG_OUTVIDEOFORMAT_DATA 0x018
#define XV_CSC_CTRL_BITS_HWREG_OUTVIDEOFORMAT_DATA 8
#define XV_CSC_CTRL_ADDR_HWREG_WIDTH_DATA          0x020
#define XV_CSC_CTRL_BITS_HWREG_WIDTH_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_HEIGHT_DATA         0x028
#define XV_CSC_CTRL_BITS_HWREG_HEIGHT_DATA         16
#define XV_CSC_CTRL_ADDR_HWREG_COLSTART_DATA       0x030
#define XV_CSC_CTRL_BITS_HWREG_COLSTART_DATA       16
#define XV_CSC_CTRL_ADDR_HWREG_COLEND_DATA         0x038
#define XV_CSC_CTRL_BITS_HWREG_COLEND_DATA         16
#define XV_CSC_CTRL_ADDR_HWREG_ROWSTART_DATA       0x040
#define XV_CSC_CTRL_BITS_HWREG_ROWSTART_DATA       16
#define XV_CSC_CTRL_ADDR_HWREG_ROWEND_DATA         0x048
#define XV_CSC_CTRL_BITS_HWREG_ROWEND_DATA         16
#define XV_CSC_CTRL_ADDR_HWREG_K11_DATA            0x050
#define XV_CSC_CTRL_BITS_HWREG_K11_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_K12_DATA            0x058
#define XV_CSC_CTRL_BITS_HWREG_K12_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_K13_DATA            0x060
#define XV_CSC_CTRL_BITS_HWREG_K13_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_K21_DATA            0x068
#define XV_CSC_CTRL_BITS_HWREG_K21_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_K22_DATA            0x070
#define XV_CSC_CTRL_BITS_HWREG_K22_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_K23_DATA            0x078
#define XV_CSC_CTRL_BITS_HWREG_K23_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_K31_DATA            0x080
#define XV_CSC_CTRL_BITS_HWREG_K31_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_K32_DATA            0x088
#define XV_CSC_CTRL_BITS_HWREG_K32_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_K33_DATA            0x090
#define XV_CSC_CTRL_BITS_HWREG_K33_DATA            16
#define XV_CSC_CTRL_ADDR_HWREG_ROFFSET_V_DATA      0x098
#define XV_CSC_CTRL_BITS_HWREG_ROFFSET_V_DATA      12
#define XV_CSC_CTRL_ADDR_HWREG_GOFFSET_V_DATA      0x0a0
#define XV_CSC_CTRL_BITS_HWREG_GOFFSET_V_DATA      12
#define XV_CSC_CTRL_ADDR_HWREG_BOFFSET_V_DATA      0x0a8
#define XV_CSC_CTRL_BITS_HWREG_BOFFSET_V_DATA      12
#define XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_V_DATA     0x0b0
#define XV_CSC_CTRL_BITS_HWREG_CLAMPMIN_V_DATA     10
#define XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_V_DATA      0x0b8
#define XV_CSC_CTRL_BITS_HWREG_CLIPMAX_V_DATA      10
#define XV_CSC_CTRL_ADDR_HWREG_K11_2_DATA          0x0c0
#define XV_CSC_CTRL_BITS_HWREG_K11_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_K12_2_DATA          0x0c8
#define XV_CSC_CTRL_BITS_HWREG_K12_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_K13_2_DATA          0x0d0
#define XV_CSC_CTRL_BITS_HWREG_K13_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_K21_2_DATA          0x0d8
#define XV_CSC_CTRL_BITS_HWREG_K21_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_K22_2_DATA          0x0e0
#define XV_CSC_CTRL_BITS_HWREG_K22_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_K23_2_DATA          0x0e8
#define XV_CSC_CTRL_BITS_HWREG_K23_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_K31_2_DATA          0x0f0
#define XV_CSC_CTRL_BITS_HWREG_K31_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_K32_2_DATA          0x0f8
#define XV_CSC_CTRL_BITS_HWREG_K32_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_K33_2_DATA          0x100
#define XV_CSC_CTRL_BITS_HWREG_K33_2_DATA          16
#define XV_CSC_CTRL_ADDR_HWREG_ROFFSET_2_V_DATA    0x108
#define XV_CSC_CTRL_BITS_HWREG_ROFFSET_2_V_DATA    12
#define XV_CSC_CTRL_ADDR_HWREG_GOFFSET_2_V_DATA    0x110
#define XV_CSC_CTRL_BITS_HWREG_GOFFSET_2_V_DATA    12
#define XV_CSC_CTRL_ADDR_HWREG_BOFFSET_2_V_DATA    0x118
#define XV_CSC_CTRL_BITS_HWREG_BOFFSET_2_V_DATA    12
#define XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_2_V_DATA   0x120
#define XV_CSC_CTRL_BITS_HWREG_CLAMPMIN_2_V_DATA   10
#define XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_2_V_DATA    0x128
#define XV_CSC_CTRL_BITS_HWREG_CLIPMAX_2_V_DATA    10

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
