// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

// CTRL
// 0x00000 : Control signals
//           bit 0  - ap_start (Read/Write/COH)
//           bit 1  - ap_done (Read/COR)
//           bit 2  - ap_idle (Read)
//           bit 3  - ap_ready (Read)
//           bit 7  - auto_restart (Read/Write)
//           others - reserved
// 0x00004 : Global Interrupt Enable Register
//           bit 0  - Global Interrupt Enable (Read/Write)
//           others - reserved
// 0x00008 : IP Interrupt Enable Register (Read/Write)
//           bit 0  - Channel 0 (ap_done)
//           bit 1  - Channel 1 (ap_ready)
//           others - reserved
// 0x0000c : IP Interrupt Status Register (Read/TOW)
//           bit 0  - Channel 0 (ap_done)
//           bit 1  - Channel 1 (ap_ready)
//           others - reserved
// 0x00010 : Data signal of HwReg_num_outs
//           bit 7~0 - HwReg_num_outs[7:0] (Read/Write)
//           others  - reserved
// 0x00014 : reserved
// 0x00100 : Data signal of HwReg_WidthIn_0
//           bit 15~0 - HwReg_WidthIn_0[15:0] (Read/Write)
//           others   - reserved
// 0x00104 : reserved
// 0x00108 : Data signal of HwReg_WidthOut_0
//           bit 15~0 - HwReg_WidthOut_0[15:0] (Read/Write)
//           others   - reserved
// 0x0010c : reserved
// 0x00110 : Data signal of HwReg_HeightIn_0
//           bit 15~0 - HwReg_HeightIn_0[15:0] (Read/Write)
//           others   - reserved
// 0x00114 : reserved
// 0x00118 : Data signal of HwReg_HeightOut_0
//           bit 15~0 - HwReg_HeightOut_0[15:0] (Read/Write)
//           others   - reserved
// 0x0011c : reserved
// 0x00120 : Data signal of HwReg_LineRate_0
//           bit 31~0 - HwReg_LineRate_0[31:0] (Read/Write)
// 0x00124 : reserved
// 0x00128 : Data signal of HwReg_PixelRate_0
//           bit 31~0 - HwReg_PixelRate_0[31:0] (Read/Write)
// 0x0012c : reserved
// 0x00130 : Data signal of HwReg_InPixelFmt_0
//           bit 7~0 - HwReg_InPixelFmt_0[7:0] (Read/Write)
//           others  - reserved
// 0x00134 : reserved
// 0x00138 : Data signal of HwReg_OutPixelFmt_0
//           bit 7~0 - HwReg_OutPixelFmt_0[7:0] (Read/Write)
//           others  - reserved
// 0x0013c : reserved
// 0x00150 : Data signal of HwReg_InStride_0
//           bit 15~0 - HwReg_InStride_0[15:0] (Read/Write)
//           others   - reserved
// 0x00154 : reserved
// 0x00158 : Data signal of HwReg_OutStride_0
//           bit 15~0 - HwReg_OutStride_0[15:0] (Read/Write)
//           others   - reserved
// 0x0015c : reserved
// 0x00160 : Data signal of HwReg_srcImgBuf0_0_V
//           bit 31~0 - HwReg_srcImgBuf0_0_V[31:0] (Read/Write)
// 0x00164 : reserved
// 0x00170 : Data signal of HwReg_srcImgBuf1_0_V
//           bit 31~0 - HwReg_srcImgBuf1_0_V[31:0] (Read/Write)
// 0x00174 : reserved
// 0x00190 : Data signal of HwReg_dstImgBuf0_0_V
//           bit 31~0 - HwReg_dstImgBuf0_0_V[31:0] (Read/Write)
// 0x00194 : reserved
// 0x00200 : Data signal of HwReg_dstImgBuf1_0_V
//           bit 31~0 - HwReg_dstImgBuf1_0_V[31:0] (Read/Write)
// 0x00204 : reserved
// 0x00300 : Data signal of HwReg_WidthIn_1
//           bit 15~0 - HwReg_WidthIn_1[15:0] (Read/Write)
//           others   - reserved
// 0x00304 : reserved
// 0x00308 : Data signal of HwReg_WidthOut_1
//           bit 15~0 - HwReg_WidthOut_1[15:0] (Read/Write)
//           others   - reserved
// 0x0030c : reserved
// 0x00310 : Data signal of HwReg_HeightIn_1
//           bit 15~0 - HwReg_HeightIn_1[15:0] (Read/Write)
//           others   - reserved
// 0x00314 : reserved
// 0x00318 : Data signal of HwReg_HeightOut_1
//           bit 15~0 - HwReg_HeightOut_1[15:0] (Read/Write)
//           others   - reserved
// 0x0031c : reserved
// 0x00320 : Data signal of HwReg_LineRate_1
//           bit 31~0 - HwReg_LineRate_1[31:0] (Read/Write)
// 0x00324 : reserved
// 0x00328 : Data signal of HwReg_PixelRate_1
//           bit 31~0 - HwReg_PixelRate_1[31:0] (Read/Write)
// 0x0032c : reserved
// 0x00330 : Data signal of HwReg_InPixelFmt_1
//           bit 7~0 - HwReg_InPixelFmt_1[7:0] (Read/Write)
//           others  - reserved
// 0x00334 : reserved
// 0x00338 : Data signal of HwReg_OutPixelFmt_1
//           bit 7~0 - HwReg_OutPixelFmt_1[7:0] (Read/Write)
//           others  - reserved
// 0x0033c : reserved
// 0x00350 : Data signal of HwReg_InStride_1
//           bit 15~0 - HwReg_InStride_1[15:0] (Read/Write)
//           others   - reserved
// 0x00354 : reserved
// 0x00358 : Data signal of HwReg_OutStride_1
//           bit 15~0 - HwReg_OutStride_1[15:0] (Read/Write)
//           others   - reserved
// 0x0035c : reserved
// 0x00360 : Data signal of HwReg_srcImgBuf0_1_V
//           bit 31~0 - HwReg_srcImgBuf0_1_V[31:0] (Read/Write)
// 0x00364 : reserved
// 0x00370 : Data signal of HwReg_srcImgBuf1_1_V
//           bit 31~0 - HwReg_srcImgBuf1_1_V[31:0] (Read/Write)
// 0x00374 : reserved
// 0x00390 : Data signal of HwReg_dstImgBuf0_1_V
//           bit 31~0 - HwReg_dstImgBuf0_1_V[31:0] (Read/Write)
// 0x00394 : reserved
// 0x00400 : Data signal of HwReg_dstImgBuf1_1_V
//           bit 31~0 - HwReg_dstImgBuf1_1_V[31:0] (Read/Write)
// 0x00404 : reserved
// 0x00500 : Data signal of HwReg_WidthIn_2
//           bit 15~0 - HwReg_WidthIn_2[15:0] (Read/Write)
//           others   - reserved
// 0x00504 : reserved
// 0x00508 : Data signal of HwReg_WidthOut_2
//           bit 15~0 - HwReg_WidthOut_2[15:0] (Read/Write)
//           others   - reserved
// 0x0050c : reserved
// 0x00510 : Data signal of HwReg_HeightIn_2
//           bit 15~0 - HwReg_HeightIn_2[15:0] (Read/Write)
//           others   - reserved
// 0x00514 : reserved
// 0x00518 : Data signal of HwReg_HeightOut_2
//           bit 15~0 - HwReg_HeightOut_2[15:0] (Read/Write)
//           others   - reserved
// 0x0051c : reserved
// 0x00520 : Data signal of HwReg_LineRate_2
//           bit 31~0 - HwReg_LineRate_2[31:0] (Read/Write)
// 0x00524 : reserved
// 0x00528 : Data signal of HwReg_PixelRate_2
//           bit 31~0 - HwReg_PixelRate_2[31:0] (Read/Write)
// 0x0052c : reserved
// 0x00530 : Data signal of HwReg_InPixelFmt_2
//           bit 7~0 - HwReg_InPixelFmt_2[7:0] (Read/Write)
//           others  - reserved
// 0x00534 : reserved
// 0x00538 : Data signal of HwReg_OutPixelFmt_2
//           bit 7~0 - HwReg_OutPixelFmt_2[7:0] (Read/Write)
//           others  - reserved
// 0x0053c : reserved
// 0x00550 : Data signal of HwReg_InStride_2
//           bit 15~0 - HwReg_InStride_2[15:0] (Read/Write)
//           others   - reserved
// 0x00554 : reserved
// 0x00558 : Data signal of HwReg_OutStride_2
//           bit 15~0 - HwReg_OutStride_2[15:0] (Read/Write)
//           others   - reserved
// 0x0055c : reserved
// 0x00560 : Data signal of HwReg_srcImgBuf0_2_V
//           bit 31~0 - HwReg_srcImgBuf0_2_V[31:0] (Read/Write)
// 0x00564 : reserved
// 0x00570 : Data signal of HwReg_srcImgBuf1_2_V
//           bit 31~0 - HwReg_srcImgBuf1_2_V[31:0] (Read/Write)
// 0x00574 : reserved
// 0x00590 : Data signal of HwReg_dstImgBuf0_2_V
//           bit 31~0 - HwReg_dstImgBuf0_2_V[31:0] (Read/Write)
// 0x00594 : reserved
// 0x00600 : Data signal of HwReg_dstImgBuf1_2_V
//           bit 31~0 - HwReg_dstImgBuf1_2_V[31:0] (Read/Write)
// 0x00604 : reserved
// 0x00700 : Data signal of HwReg_WidthIn_3
//           bit 15~0 - HwReg_WidthIn_3[15:0] (Read/Write)
//           others   - reserved
// 0x00704 : reserved
// 0x00708 : Data signal of HwReg_WidthOut_3
//           bit 15~0 - HwReg_WidthOut_3[15:0] (Read/Write)
//           others   - reserved
// 0x0070c : reserved
// 0x00710 : Data signal of HwReg_HeightIn_3
//           bit 15~0 - HwReg_HeightIn_3[15:0] (Read/Write)
//           others   - reserved
// 0x00714 : reserved
// 0x00718 : Data signal of HwReg_HeightOut_3
//           bit 15~0 - HwReg_HeightOut_3[15:0] (Read/Write)
//           others   - reserved
// 0x0071c : reserved
// 0x00720 : Data signal of HwReg_LineRate_3
//           bit 31~0 - HwReg_LineRate_3[31:0] (Read/Write)
// 0x00724 : reserved
// 0x00728 : Data signal of HwReg_PixelRate_3
//           bit 31~0 - HwReg_PixelRate_3[31:0] (Read/Write)
// 0x0072c : reserved
// 0x00730 : Data signal of HwReg_InPixelFmt_3
//           bit 7~0 - HwReg_InPixelFmt_3[7:0] (Read/Write)
//           others  - reserved
// 0x00734 : reserved
// 0x00738 : Data signal of HwReg_OutPixelFmt_3
//           bit 7~0 - HwReg_OutPixelFmt_3[7:0] (Read/Write)
//           others  - reserved
// 0x0073c : reserved
// 0x00750 : Data signal of HwReg_InStride_3
//           bit 15~0 - HwReg_InStride_3[15:0] (Read/Write)
//           others   - reserved
// 0x00754 : reserved
// 0x00758 : Data signal of HwReg_OutStride_3
//           bit 15~0 - HwReg_OutStride_3[15:0] (Read/Write)
//           others   - reserved
// 0x0075c : reserved
// 0x00760 : Data signal of HwReg_srcImgBuf0_3_V
//           bit 31~0 - HwReg_srcImgBuf0_3_V[31:0] (Read/Write)
// 0x00764 : reserved
// 0x00770 : Data signal of HwReg_srcImgBuf1_3_V
//           bit 31~0 - HwReg_srcImgBuf1_3_V[31:0] (Read/Write)
// 0x00774 : reserved
// 0x00790 : Data signal of HwReg_dstImgBuf0_3_V
//           bit 31~0 - HwReg_dstImgBuf0_3_V[31:0] (Read/Write)
// 0x00794 : reserved
// 0x00800 : Data signal of HwReg_dstImgBuf1_3_V
//           bit 31~0 - HwReg_dstImgBuf1_3_V[31:0] (Read/Write)
// 0x00804 : reserved
// 0x00900 : Data signal of HwReg_WidthIn_4
//           bit 15~0 - HwReg_WidthIn_4[15:0] (Read/Write)
//           others   - reserved
// 0x00904 : reserved
// 0x00908 : Data signal of HwReg_WidthOut_4
//           bit 15~0 - HwReg_WidthOut_4[15:0] (Read/Write)
//           others   - reserved
// 0x0090c : reserved
// 0x00910 : Data signal of HwReg_HeightIn_4
//           bit 15~0 - HwReg_HeightIn_4[15:0] (Read/Write)
//           others   - reserved
// 0x00914 : reserved
// 0x00918 : Data signal of HwReg_HeightOut_4
//           bit 15~0 - HwReg_HeightOut_4[15:0] (Read/Write)
//           others   - reserved
// 0x0091c : reserved
// 0x00920 : Data signal of HwReg_LineRate_4
//           bit 31~0 - HwReg_LineRate_4[31:0] (Read/Write)
// 0x00924 : reserved
// 0x00928 : Data signal of HwReg_PixelRate_4
//           bit 31~0 - HwReg_PixelRate_4[31:0] (Read/Write)
// 0x0092c : reserved
// 0x00930 : Data signal of HwReg_InPixelFmt_4
//           bit 7~0 - HwReg_InPixelFmt_4[7:0] (Read/Write)
//           others  - reserved
// 0x00934 : reserved
// 0x00938 : Data signal of HwReg_OutPixelFmt_4
//           bit 7~0 - HwReg_OutPixelFmt_4[7:0] (Read/Write)
//           others  - reserved
// 0x0093c : reserved
// 0x00950 : Data signal of HwReg_InStride_4
//           bit 15~0 - HwReg_InStride_4[15:0] (Read/Write)
//           others   - reserved
// 0x00954 : reserved
// 0x00958 : Data signal of HwReg_OutStride_4
//           bit 15~0 - HwReg_OutStride_4[15:0] (Read/Write)
//           others   - reserved
// 0x0095c : reserved
// 0x00960 : Data signal of HwReg_srcImgBuf0_4_V
//           bit 31~0 - HwReg_srcImgBuf0_4_V[31:0] (Read/Write)
// 0x00964 : reserved
// 0x00970 : Data signal of HwReg_srcImgBuf1_4_V
//           bit 31~0 - HwReg_srcImgBuf1_4_V[31:0] (Read/Write)
// 0x00974 : reserved
// 0x00990 : Data signal of HwReg_dstImgBuf0_4_V
//           bit 31~0 - HwReg_dstImgBuf0_4_V[31:0] (Read/Write)
// 0x00994 : reserved
// 0x01000 : Data signal of HwReg_dstImgBuf1_4_V
//           bit 31~0 - HwReg_dstImgBuf1_4_V[31:0] (Read/Write)
// 0x01004 : reserved
// 0x01100 : Data signal of HwReg_WidthIn_5
//           bit 15~0 - HwReg_WidthIn_5[15:0] (Read/Write)
//           others   - reserved
// 0x01104 : reserved
// 0x01108 : Data signal of HwReg_WidthOut_5
//           bit 15~0 - HwReg_WidthOut_5[15:0] (Read/Write)
//           others   - reserved
// 0x0110c : reserved
// 0x01110 : Data signal of HwReg_HeightIn_5
//           bit 15~0 - HwReg_HeightIn_5[15:0] (Read/Write)
//           others   - reserved
// 0x01114 : reserved
// 0x01118 : Data signal of HwReg_HeightOut_5
//           bit 15~0 - HwReg_HeightOut_5[15:0] (Read/Write)
//           others   - reserved
// 0x0111c : reserved
// 0x01120 : Data signal of HwReg_LineRate_5
//           bit 31~0 - HwReg_LineRate_5[31:0] (Read/Write)
// 0x01124 : reserved
// 0x01128 : Data signal of HwReg_PixelRate_5
//           bit 31~0 - HwReg_PixelRate_5[31:0] (Read/Write)
// 0x0112c : reserved
// 0x01130 : Data signal of HwReg_InPixelFmt_5
//           bit 7~0 - HwReg_InPixelFmt_5[7:0] (Read/Write)
//           others  - reserved
// 0x01134 : reserved
// 0x01138 : Data signal of HwReg_OutPixelFmt_5
//           bit 7~0 - HwReg_OutPixelFmt_5[7:0] (Read/Write)
//           others  - reserved
// 0x0113c : reserved
// 0x01150 : Data signal of HwReg_InStride_5
//           bit 15~0 - HwReg_InStride_5[15:0] (Read/Write)
//           others   - reserved
// 0x01154 : reserved
// 0x01158 : Data signal of HwReg_OutStride_5
//           bit 15~0 - HwReg_OutStride_5[15:0] (Read/Write)
//           others   - reserved
// 0x0115c : reserved
// 0x01160 : Data signal of HwReg_srcImgBuf0_5_V
//           bit 31~0 - HwReg_srcImgBuf0_5_V[31:0] (Read/Write)
// 0x01164 : reserved
// 0x01170 : Data signal of HwReg_srcImgBuf1_5_V
//           bit 31~0 - HwReg_srcImgBuf1_5_V[31:0] (Read/Write)
// 0x01174 : reserved
// 0x01190 : Data signal of HwReg_dstImgBuf0_5_V
//           bit 31~0 - HwReg_dstImgBuf0_5_V[31:0] (Read/Write)
// 0x01194 : reserved
// 0x01200 : Data signal of HwReg_dstImgBuf1_5_V
//           bit 31~0 - HwReg_dstImgBuf1_5_V[31:0] (Read/Write)
// 0x01204 : reserved
// 0x01300 : Data signal of HwReg_WidthIn_6
//           bit 15~0 - HwReg_WidthIn_6[15:0] (Read/Write)
//           others   - reserved
// 0x01304 : reserved
// 0x01308 : Data signal of HwReg_WidthOut_6
//           bit 15~0 - HwReg_WidthOut_6[15:0] (Read/Write)
//           others   - reserved
// 0x0130c : reserved
// 0x01310 : Data signal of HwReg_HeightIn_6
//           bit 15~0 - HwReg_HeightIn_6[15:0] (Read/Write)
//           others   - reserved
// 0x01314 : reserved
// 0x01318 : Data signal of HwReg_HeightOut_6
//           bit 15~0 - HwReg_HeightOut_6[15:0] (Read/Write)
//           others   - reserved
// 0x0131c : reserved
// 0x01320 : Data signal of HwReg_LineRate_6
//           bit 31~0 - HwReg_LineRate_6[31:0] (Read/Write)
// 0x01324 : reserved
// 0x01328 : Data signal of HwReg_PixelRate_6
//           bit 31~0 - HwReg_PixelRate_6[31:0] (Read/Write)
// 0x0132c : reserved
// 0x01330 : Data signal of HwReg_InPixelFmt_6
//           bit 7~0 - HwReg_InPixelFmt_6[7:0] (Read/Write)
//           others  - reserved
// 0x01334 : reserved
// 0x01338 : Data signal of HwReg_OutPixelFmt_6
//           bit 7~0 - HwReg_OutPixelFmt_6[7:0] (Read/Write)
//           others  - reserved
// 0x0133c : reserved
// 0x01350 : Data signal of HwReg_InStride_6
//           bit 15~0 - HwReg_InStride_6[15:0] (Read/Write)
//           others   - reserved
// 0x01354 : reserved
// 0x01358 : Data signal of HwReg_OutStride_6
//           bit 15~0 - HwReg_OutStride_6[15:0] (Read/Write)
//           others   - reserved
// 0x0135c : reserved
// 0x01360 : Data signal of HwReg_srcImgBuf0_6_V
//           bit 31~0 - HwReg_srcImgBuf0_6_V[31:0] (Read/Write)
// 0x01364 : reserved
// 0x01370 : Data signal of HwReg_srcImgBuf1_6_V
//           bit 31~0 - HwReg_srcImgBuf1_6_V[31:0] (Read/Write)
// 0x01374 : reserved
// 0x01390 : Data signal of HwReg_dstImgBuf0_6_V
//           bit 31~0 - HwReg_dstImgBuf0_6_V[31:0] (Read/Write)
// 0x01394 : reserved
// 0x01400 : Data signal of HwReg_dstImgBuf1_6_V
//           bit 31~0 - HwReg_dstImgBuf1_6_V[31:0] (Read/Write)
// 0x01404 : reserved
// 0x01500 : Data signal of HwReg_WidthIn_7
//           bit 15~0 - HwReg_WidthIn_7[15:0] (Read/Write)
//           others   - reserved
// 0x01504 : reserved
// 0x01508 : Data signal of HwReg_WidthOut_7
//           bit 15~0 - HwReg_WidthOut_7[15:0] (Read/Write)
//           others   - reserved
// 0x0150c : reserved
// 0x01510 : Data signal of HwReg_HeightIn_7
//           bit 15~0 - HwReg_HeightIn_7[15:0] (Read/Write)
//           others   - reserved
// 0x01514 : reserved
// 0x01518 : Data signal of HwReg_HeightOut_7
//           bit 15~0 - HwReg_HeightOut_7[15:0] (Read/Write)
//           others   - reserved
// 0x0151c : reserved
// 0x01520 : Data signal of HwReg_LineRate_7
//           bit 31~0 - HwReg_LineRate_7[31:0] (Read/Write)
// 0x01524 : reserved
// 0x01528 : Data signal of HwReg_PixelRate_7
//           bit 31~0 - HwReg_PixelRate_7[31:0] (Read/Write)
// 0x0152c : reserved
// 0x01530 : Data signal of HwReg_InPixelFmt_7
//           bit 7~0 - HwReg_InPixelFmt_7[7:0] (Read/Write)
//           others  - reserved
// 0x01534 : reserved
// 0x01538 : Data signal of HwReg_OutPixelFmt_7
//           bit 7~0 - HwReg_OutPixelFmt_7[7:0] (Read/Write)
//           others  - reserved
// 0x0153c : reserved
// 0x01550 : Data signal of HwReg_InStride_7
//           bit 15~0 - HwReg_InStride_7[15:0] (Read/Write)
//           others   - reserved
// 0x01554 : reserved
// 0x01558 : Data signal of HwReg_OutStride_7
//           bit 15~0 - HwReg_OutStride_7[15:0] (Read/Write)
//           others   - reserved
// 0x0155c : reserved
// 0x01560 : Data signal of HwReg_srcImgBuf0_7_V
//           bit 31~0 - HwReg_srcImgBuf0_7_V[31:0] (Read/Write)
// 0x01564 : reserved
// 0x01570 : Data signal of HwReg_srcImgBuf1_7_V
//           bit 31~0 - HwReg_srcImgBuf1_7_V[31:0] (Read/Write)
// 0x01574 : reserved
// 0x01590 : Data signal of HwReg_dstImgBuf0_7_V
//           bit 31~0 - HwReg_dstImgBuf0_7_V[31:0] (Read/Write)
// 0x01594 : reserved
// 0x01600 : Data signal of HwReg_dstImgBuf1_7_V
//           bit 31~0 - HwReg_dstImgBuf1_7_V[31:0] (Read/Write)
// 0x01604 : reserved
// 0x02000 ~
// 0x023ff : Memory 'HwReg_mm_vfltCoeff_0' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_vfltCoeff_0[2n]
//                    bit [31:16] - HwReg_mm_vfltCoeff_0[2n+1]
// 0x02800 ~
// 0x02bff : Memory 'HwReg_mm_hfltCoeff_0' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_hfltCoeff_0[2n]
//                    bit [31:16] - HwReg_mm_hfltCoeff_0[2n+1]
// 0x04000 ~
// 0x043ff : Memory 'HwReg_mm_vfltCoeff_1' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_vfltCoeff_1[2n]
//                    bit [31:16] - HwReg_mm_vfltCoeff_1[2n+1]
// 0x04800 ~
// 0x04bff : Memory 'HwReg_mm_hfltCoeff_1' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_hfltCoeff_1[2n]
//                    bit [31:16] - HwReg_mm_hfltCoeff_1[2n+1]
// 0x06000 ~
// 0x063ff : Memory 'HwReg_mm_vfltCoeff_2' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_vfltCoeff_2[2n]
//                    bit [31:16] - HwReg_mm_vfltCoeff_2[2n+1]
// 0x06800 ~
// 0x06bff : Memory 'HwReg_mm_hfltCoeff_2' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_hfltCoeff_2[2n]
//                    bit [31:16] - HwReg_mm_hfltCoeff_2[2n+1]
// 0x08000 ~
// 0x083ff : Memory 'HwReg_mm_vfltCoeff_3' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_vfltCoeff_3[2n]
//                    bit [31:16] - HwReg_mm_vfltCoeff_3[2n+1]
// 0x08800 ~
// 0x08bff : Memory 'HwReg_mm_hfltCoeff_3' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_hfltCoeff_3[2n]
//                    bit [31:16] - HwReg_mm_hfltCoeff_3[2n+1]
// 0x0a000 ~
// 0x0a3ff : Memory 'HwReg_mm_vfltCoeff_4' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_vfltCoeff_4[2n]
//                    bit [31:16] - HwReg_mm_vfltCoeff_4[2n+1]
// 0x0a800 ~
// 0x0abff : Memory 'HwReg_mm_hfltCoeff_4' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_hfltCoeff_4[2n]
//                    bit [31:16] - HwReg_mm_hfltCoeff_4[2n+1]
// 0x0c000 ~
// 0x0c3ff : Memory 'HwReg_mm_vfltCoeff_5' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_vfltCoeff_5[2n]
//                    bit [31:16] - HwReg_mm_vfltCoeff_5[2n+1]
// 0x0c800 ~
// 0x0cbff : Memory 'HwReg_mm_hfltCoeff_5' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_hfltCoeff_5[2n]
//                    bit [31:16] - HwReg_mm_hfltCoeff_5[2n+1]
// 0x0e000 ~
// 0x0e3ff : Memory 'HwReg_mm_vfltCoeff_6' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_vfltCoeff_6[2n]
//                    bit [31:16] - HwReg_mm_vfltCoeff_6[2n+1]
// 0x0e800 ~
// 0x0ebff : Memory 'HwReg_mm_hfltCoeff_6' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_hfltCoeff_6[2n]
//                    bit [31:16] - HwReg_mm_hfltCoeff_6[2n+1]
// 0x10000 ~
// 0x103ff : Memory 'HwReg_mm_vfltCoeff_7' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_vfltCoeff_7[2n]
//                    bit [31:16] - HwReg_mm_vfltCoeff_7[2n+1]
// 0x10800 ~
// 0x10bff : Memory 'HwReg_mm_hfltCoeff_7' (384 * 16b)
//           Word n : bit [15: 0] - HwReg_mm_hfltCoeff_7[2n]
//                    bit [31:16] - HwReg_mm_hfltCoeff_7[2n+1]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on
// Handshake)

#define XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL                   0x00000
#define XV_MULTI_SCALER_CTRL_ADDR_GIE                       0x00004
#define XV_MULTI_SCALER_CTRL_ADDR_IER                       0x00008
#define XV_MULTI_SCALER_CTRL_ADDR_ISR                       0x0000c
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_NUM_OUTS_DATA       0x00010
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_NUM_OUTS_DATA       8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_0_DATA      0x00100
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHIN_0_DATA      16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_0_DATA     0x00108
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHOUT_0_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_0_DATA     0x00110
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTIN_0_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_0_DATA    0x00118
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTOUT_0_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_0_DATA     0x00120
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_LINERATE_0_DATA     32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_0_DATA    0x00128
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_PIXELRATE_0_DATA    32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_0_DATA   0x00130
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INPIXELFMT_0_DATA   8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_0_DATA  0x00138
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTPIXELFMT_0_DATA  8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_0_DATA     0x00150
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INSTRIDE_0_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_0_DATA    0x00158
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTSTRIDE_0_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA 0x00160
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF0_0_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA 0x00170
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF1_0_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA 0x00190
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF0_0_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA 0x00200
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF1_0_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_1_DATA      0x00300
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHIN_1_DATA      16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_1_DATA     0x00308
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHOUT_1_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_1_DATA     0x00310
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTIN_1_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_1_DATA    0x00318
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTOUT_1_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_1_DATA     0x00320
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_LINERATE_1_DATA     32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_1_DATA    0x00328
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_PIXELRATE_1_DATA    32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_1_DATA   0x00330
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INPIXELFMT_1_DATA   8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_1_DATA  0x00338
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTPIXELFMT_1_DATA  8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_1_DATA     0x00350
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INSTRIDE_1_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_1_DATA    0x00358
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTSTRIDE_1_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA 0x00360
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF0_1_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA 0x00370
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF1_1_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA 0x00390
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF0_1_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA 0x00400
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF1_1_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_2_DATA      0x00500
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHIN_2_DATA      16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_2_DATA     0x00508
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHOUT_2_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_2_DATA     0x00510
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTIN_2_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_2_DATA    0x00518
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTOUT_2_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_2_DATA     0x00520
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_LINERATE_2_DATA     32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_2_DATA    0x00528
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_PIXELRATE_2_DATA    32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_2_DATA   0x00530
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INPIXELFMT_2_DATA   8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_2_DATA  0x00538
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTPIXELFMT_2_DATA  8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_2_DATA     0x00550
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INSTRIDE_2_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_2_DATA    0x00558
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTSTRIDE_2_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA 0x00560
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF0_2_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA 0x00570
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF1_2_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA 0x00590
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF0_2_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA 0x00600
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF1_2_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_3_DATA      0x00700
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHIN_3_DATA      16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_3_DATA     0x00708
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHOUT_3_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_3_DATA     0x00710
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTIN_3_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_3_DATA    0x00718
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTOUT_3_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_3_DATA     0x00720
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_LINERATE_3_DATA     32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_3_DATA    0x00728
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_PIXELRATE_3_DATA    32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_3_DATA   0x00730
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INPIXELFMT_3_DATA   8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_3_DATA  0x00738
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTPIXELFMT_3_DATA  8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_3_DATA     0x00750
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INSTRIDE_3_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_3_DATA    0x00758
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTSTRIDE_3_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA 0x00760
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF0_3_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA 0x00770
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF1_3_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA 0x00790
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF0_3_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA 0x00800
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF1_3_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_4_DATA      0x00900
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHIN_4_DATA      16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_4_DATA     0x00908
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHOUT_4_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_4_DATA     0x00910
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTIN_4_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_4_DATA    0x00918
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTOUT_4_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_4_DATA     0x00920
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_LINERATE_4_DATA     32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_4_DATA    0x00928
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_PIXELRATE_4_DATA    32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_4_DATA   0x00930
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INPIXELFMT_4_DATA   8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_4_DATA  0x00938
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTPIXELFMT_4_DATA  8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_4_DATA     0x00950
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INSTRIDE_4_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_4_DATA    0x00958
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTSTRIDE_4_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA 0x00960
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF0_4_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA 0x00970
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF1_4_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA 0x00990
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF0_4_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA 0x01000
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF1_4_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_5_DATA      0x01100
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHIN_5_DATA      16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_5_DATA     0x01108
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHOUT_5_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_5_DATA     0x01110
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTIN_5_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_5_DATA    0x01118
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTOUT_5_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_5_DATA     0x01120
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_LINERATE_5_DATA     32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_5_DATA    0x01128
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_PIXELRATE_5_DATA    32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_5_DATA   0x01130
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INPIXELFMT_5_DATA   8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_5_DATA  0x01138
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTPIXELFMT_5_DATA  8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_5_DATA     0x01150
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INSTRIDE_5_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_5_DATA    0x01158
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTSTRIDE_5_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA 0x01160
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF0_5_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA 0x01170
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF1_5_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA 0x01190
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF0_5_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA 0x01200
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF1_5_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_6_DATA      0x01300
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHIN_6_DATA      16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_6_DATA     0x01308
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHOUT_6_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_6_DATA     0x01310
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTIN_6_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_6_DATA    0x01318
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTOUT_6_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_6_DATA     0x01320
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_LINERATE_6_DATA     32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_6_DATA    0x01328
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_PIXELRATE_6_DATA    32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_6_DATA   0x01330
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INPIXELFMT_6_DATA   8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_6_DATA  0x01338
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTPIXELFMT_6_DATA  8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_6_DATA     0x01350
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INSTRIDE_6_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_6_DATA    0x01358
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTSTRIDE_6_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA 0x01360
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF0_6_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA 0x01370
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF1_6_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA 0x01390
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF0_6_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA 0x01400
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF1_6_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_7_DATA      0x01500
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHIN_7_DATA      16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_7_DATA     0x01508
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_WIDTHOUT_7_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_7_DATA     0x01510
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTIN_7_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_7_DATA    0x01518
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_HEIGHTOUT_7_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_7_DATA     0x01520
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_LINERATE_7_DATA     32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_7_DATA    0x01528
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_PIXELRATE_7_DATA    32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_7_DATA   0x01530
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INPIXELFMT_7_DATA   8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_7_DATA  0x01538
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTPIXELFMT_7_DATA  8
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_7_DATA     0x01550
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_INSTRIDE_7_DATA     16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_7_DATA    0x01558
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_OUTSTRIDE_7_DATA    16
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA 0x01560
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF0_7_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA 0x01570
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_SRCIMGBUF1_7_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA 0x01590
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF0_7_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA 0x01600
#define XV_MULTI_SCALER_CTRL_BITS_HWREG_DSTIMGBUF1_7_V_DATA 32
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE 0x02000
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH 0x023ff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_0     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_0     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE 0x02800
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH 0x02bff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_0     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_0     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE 0x04000
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH 0x043ff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_1     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_1     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE 0x04800
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH 0x04bff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_1     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_1     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE 0x06000
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH 0x063ff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_2     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_2     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE 0x06800
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH 0x06bff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_2     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_2     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE 0x08000
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH 0x083ff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_3     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_3     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE 0x08800
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH 0x08bff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_3     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_3     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE 0x0a000
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH 0x0a3ff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_4     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_4     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE 0x0a800
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH 0x0abff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_4     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_4     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE 0x0c000
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH 0x0c3ff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_5     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_5     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE 0x0c800
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH 0x0cbff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_5     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_5     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE 0x0e000
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH 0x0e3ff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_6     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_6     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE 0x0e800
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH 0x0ebff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_6     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_6     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE 0x10000
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH 0x103ff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_7     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_7     384
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE 0x10800
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH 0x10bff
#define XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_7     16
#define XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_7     384

