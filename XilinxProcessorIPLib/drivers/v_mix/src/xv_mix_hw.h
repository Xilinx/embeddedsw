// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_mix_hw.h
* @addtogroup v_mix Overview
*/

#ifndef XV_MIX_HW_H_     /* prevent circular inclusions */
#define XV_MIX_HW_H_     /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

// CTRL
// 0x00000 : Control signals
//           bit 0  - ap_start (Read/Write/COH)
//           bit 1  - ap_done (Read/COR)
//           bit 2  - ap_idle (Read)
//           bit 3  - ap_ready (Read)
//        	 bit 5  - Flush pending AXI transactions
//        	 bit 6  - Flush done
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
// 0x00010 : Data signal of HwReg_width
//           bit 15~0 - HwReg_width[15:0] (Read/Write)
//           others   - reserved
// 0x00014 : reserved
// 0x00018 : Data signal of HwReg_height
//           bit 15~0 - HwReg_height[15:0] (Read/Write)
//           others   - reserved
// 0x0001c : reserved
// 0x00020 : Data signal of HwReg_video_format
//           bit 15~0 - HwReg_video_format[15:0] (Read/Write)
//           others   - reserved
// 0x00024 : reserved
// 0x00028 : Data signal of HwReg_background_Y_R
//           bit 15~0 - HwReg_background_Y_R[15:0] (Read/Write)
//           others   - reserved
// 0x0002c : reserved
// 0x00030 : Data signal of HwReg_background_U_G
//           bit 15~0 - HwReg_background_U_G[15:0] (Read/Write)
//           others   - reserved
// 0x00034 : reserved
// 0x00038 : Data signal of HwReg_background_V_B
//           bit 15~0 - HwReg_background_V_B[15:0] (Read/Write)
//           others   - reserved
// 0x0003c : reserved
// 0x00040 : Data signal of HwReg_layerEnable
//           bit 15~0 - HwReg_layerEnable[15:0] (Read/Write)
//           others   - reserved
// 0x00044 : reserved
// 0x00100 : Data signal of HwReg_layerAlpha_0
//           bit 15~0 - HwReg_layerAlpha_0[15:0] (Read/Write)
//           others   - reserved
// 0x00104 : reserved
// 0x00108 : Data signal of HwReg_layerStartX_0
//           bit 15~0 - HwReg_layerStartX_0[15:0] (Read/Write)
//           others   - reserved
// 0x0010c : reserved
// 0x00110 : Data signal of HwReg_layerStartY_0
//           bit 15~0 - HwReg_layerStartY_0[15:0] (Read/Write)
//           others   - reserved
// 0x00114 : reserved
// 0x00118 : Data signal of HwReg_layerWidth_0
//           bit 15~0 - HwReg_layerWidth_0[15:0] (Read/Write)
//           others   - reserved
// 0x0011c : reserved
// 0x00120 : Data signal of HwReg_layerStride_0
//           bit 15~0 - HwReg_layerStride_0[15:0] (Read/Write)
//           others   - reserved
// 0x00124 : reserved
// 0x00128 : Data signal of HwReg_layerHeight_0
//           bit 15~0 - HwReg_layerHeight_0[15:0] (Read/Write)
//           others   - reserved
// 0x0012c : reserved
// 0x00130 : Data signal of HwReg_layerScaleFactor_0
//           bit 7~0 - HwReg_layerScaleFactor_0[7:0] (Read/Write)
//           others  - reserved
// 0x00134 : reserved
// 0x00138 : Data signal of HwReg_layerVideoFormat_0
//           bit 7~0 - HwReg_layerVideoFormat_0[7:0] (Read/Write)
//           others  - reserved
// 0x0013c : reserved
// 0x00200 : Data signal of HwReg_layerAlpha_1
//           bit 15~0 - HwReg_layerAlpha_1[15:0] (Read/Write)
//           others   - reserved
// 0x00204 : reserved
// 0x00208 : Data signal of HwReg_layerStartX_1
//           bit 15~0 - HwReg_layerStartX_1[15:0] (Read/Write)
//           others   - reserved
// 0x0020c : reserved
// 0x00210 : Data signal of HwReg_layerStartY_1
//           bit 15~0 - HwReg_layerStartY_1[15:0] (Read/Write)
//           others   - reserved
// 0x00214 : reserved
// 0x00218 : Data signal of HwReg_layerWidth_1
//           bit 15~0 - HwReg_layerWidth_1[15:0] (Read/Write)
//           others   - reserved
// 0x0021c : reserved
// 0x00220 : Data signal of HwReg_layerStride_1
//           bit 15~0 - HwReg_layerStride_1[15:0] (Read/Write)
//           others   - reserved
// 0x00224 : reserved
// 0x00228 : Data signal of HwReg_layerHeight_1
//           bit 15~0 - HwReg_layerHeight_1[15:0] (Read/Write)
//           others   - reserved
// 0x0022c : reserved
// 0x00230 : Data signal of HwReg_layerScaleFactor_1
//           bit 7~0 - HwReg_layerScaleFactor_1[7:0] (Read/Write)
//           others  - reserved
// 0x00234 : reserved
// 0x00238 : Data signal of HwReg_layerVideoFormat_1
//           bit 7~0 - HwReg_layerVideoFormat_1[7:0] (Read/Write)
//           others  - reserved
// 0x0023c : reserved
// 0x00240 : Data signal of HwReg_layer1_buf1_V
//           bit 31~0 - HwReg_layer1_buf1_V[31:0] (Read/Write)
// 0x00244 : Data signal of HwReg_layer1_buf1_V
//           bit 31~0 - HwReg_layer1_buf1_V[63:32] (Read/Write)
// 0x00248 : reserved
// 0x0024c : Data signal of HwReg_layer1_buf2_V
//           bit 31~0 - HwReg_layer1_buf2_V[31:0] (Read/Write)
// 0x00250 : Data signal of HwReg_layer1_buf2_V
//           bit 31~0 - HwReg_layer1_buf2_V[63:32] (Read/Write)
// 0x00254 : reserved
// 0x00300 : Data signal of HwReg_layerAlpha_2
//           bit 15~0 - HwReg_layerAlpha_2[15:0] (Read/Write)
//           others   - reserved
// 0x00304 : reserved
// 0x00308 : Data signal of HwReg_layerStartX_2
//           bit 15~0 - HwReg_layerStartX_2[15:0] (Read/Write)
//           others   - reserved
// 0x0030c : reserved
// 0x00310 : Data signal of HwReg_layerStartY_2
//           bit 15~0 - HwReg_layerStartY_2[15:0] (Read/Write)
//           others   - reserved
// 0x00314 : reserved
// 0x00318 : Data signal of HwReg_layerWidth_2
//           bit 15~0 - HwReg_layerWidth_2[15:0] (Read/Write)
//           others   - reserved
// 0x0031c : reserved
// 0x00320 : Data signal of HwReg_layerStride_2
//           bit 15~0 - HwReg_layerStride_2[15:0] (Read/Write)
//           others   - reserved
// 0x00324 : reserved
// 0x00328 : Data signal of HwReg_layerHeight_2
//           bit 15~0 - HwReg_layerHeight_2[15:0] (Read/Write)
//           others   - reserved
// 0x0032c : reserved
// 0x00330 : Data signal of HwReg_layerScaleFactor_2
//           bit 7~0 - HwReg_layerScaleFactor_2[7:0] (Read/Write)
//           others  - reserved
// 0x00334 : reserved
// 0x00338 : Data signal of HwReg_layerVideoFormat_2
//           bit 7~0 - HwReg_layerVideoFormat_2[7:0] (Read/Write)
//           others  - reserved
// 0x0033c : reserved
// 0x00340 : Data signal of HwReg_layer2_buf1_V
//           bit 31~0 - HwReg_layer2_buf1_V[31:0] (Read/Write)
// 0x00344 : Data signal of HwReg_layer2_buf1_V
//           bit 31~0 - HwReg_layer2_buf1_V[63:32] (Read/Write)
// 0x00348 : reserved
// 0x0034c : Data signal of HwReg_layer2_buf2_V
//           bit 31~0 - HwReg_layer2_buf2_V[31:0] (Read/Write)
// 0x00350 : Data signal of HwReg_layer2_buf2_V
//           bit 31~0 - HwReg_layer2_buf2_V[63:32] (Read/Write)
// 0x00354 : reserved
// 0x00400 : Data signal of HwReg_layerAlpha_3
//           bit 15~0 - HwReg_layerAlpha_3[15:0] (Read/Write)
//           others   - reserved
// 0x00404 : reserved
// 0x00408 : Data signal of HwReg_layerStartX_3
//           bit 15~0 - HwReg_layerStartX_3[15:0] (Read/Write)
//           others   - reserved
// 0x0040c : reserved
// 0x00410 : Data signal of HwReg_layerStartY_3
//           bit 15~0 - HwReg_layerStartY_3[15:0] (Read/Write)
//           others   - reserved
// 0x00414 : reserved
// 0x00418 : Data signal of HwReg_layerWidth_3
//           bit 15~0 - HwReg_layerWidth_3[15:0] (Read/Write)
//           others   - reserved
// 0x0041c : reserved
// 0x00420 : Data signal of HwReg_layerStride_3
//           bit 15~0 - HwReg_layerStride_3[15:0] (Read/Write)
//           others   - reserved
// 0x00424 : reserved
// 0x00428 : Data signal of HwReg_layerHeight_3
//           bit 15~0 - HwReg_layerHeight_3[15:0] (Read/Write)
//           others   - reserved
// 0x0042c : reserved
// 0x00430 : Data signal of HwReg_layerScaleFactor_3
//           bit 7~0 - HwReg_layerScaleFactor_3[7:0] (Read/Write)
//           others  - reserved
// 0x00434 : reserved
// 0x00438 : Data signal of HwReg_layerVideoFormat_3
//           bit 7~0 - HwReg_layerVideoFormat_3[7:0] (Read/Write)
//           others  - reserved
// 0x0043c : reserved
// 0x00440 : Data signal of HwReg_layer3_buf1_V
//           bit 31~0 - HwReg_layer3_buf1_V[31:0] (Read/Write)
// 0x00444 : Data signal of HwReg_layer3_buf1_V
//           bit 31~0 - HwReg_layer3_buf1_V[63:32] (Read/Write)
// 0x00448 : reserved
// 0x0044c : Data signal of HwReg_layer3_buf2_V
//           bit 31~0 - HwReg_layer3_buf2_V[31:0] (Read/Write)
// 0x00450 : Data signal of HwReg_layer3_buf2_V
//           bit 31~0 - HwReg_layer3_buf2_V[63:32] (Read/Write)
// 0x00454 : reserved
// 0x00500 : Data signal of HwReg_layerAlpha_4
//           bit 15~0 - HwReg_layerAlpha_4[15:0] (Read/Write)
//           others   - reserved
// 0x00504 : reserved
// 0x00508 : Data signal of HwReg_layerStartX_4
//           bit 15~0 - HwReg_layerStartX_4[15:0] (Read/Write)
//           others   - reserved
// 0x0050c : reserved
// 0x00510 : Data signal of HwReg_layerStartY_4
//           bit 15~0 - HwReg_layerStartY_4[15:0] (Read/Write)
//           others   - reserved
// 0x00514 : reserved
// 0x00518 : Data signal of HwReg_layerWidth_4
//           bit 15~0 - HwReg_layerWidth_4[15:0] (Read/Write)
//           others   - reserved
// 0x0051c : reserved
// 0x00520 : Data signal of HwReg_layerStride_4
//           bit 15~0 - HwReg_layerStride_4[15:0] (Read/Write)
//           others   - reserved
// 0x00524 : reserved
// 0x00528 : Data signal of HwReg_layerHeight_4
//           bit 15~0 - HwReg_layerHeight_4[15:0] (Read/Write)
//           others   - reserved
// 0x0052c : reserved
// 0x00530 : Data signal of HwReg_layerScaleFactor_4
//           bit 7~0 - HwReg_layerScaleFactor_4[7:0] (Read/Write)
//           others  - reserved
// 0x00534 : reserved
// 0x00538 : Data signal of HwReg_layerVideoFormat_4
//           bit 7~0 - HwReg_layerVideoFormat_4[7:0] (Read/Write)
//           others  - reserved
// 0x0053c : reserved
// 0x00540 : Data signal of HwReg_layer4_buf1_V
//           bit 31~0 - HwReg_layer4_buf1_V[31:0] (Read/Write)
// 0x00544 : Data signal of HwReg_layer4_buf1_V
//           bit 31~0 - HwReg_layer4_buf1_V[63:32] (Read/Write)
// 0x00548 : reserved
// 0x0054c : Data signal of HwReg_layer4_buf2_V
//           bit 31~0 - HwReg_layer4_buf2_V[31:0] (Read/Write)
// 0x00550 : Data signal of HwReg_layer4_buf2_V
//           bit 31~0 - HwReg_layer4_buf2_V[63:32] (Read/Write)
// 0x00554 : reserved
// 0x00600 : Data signal of HwReg_layerAlpha_5
//           bit 15~0 - HwReg_layerAlpha_5[15:0] (Read/Write)
//           others   - reserved
// 0x00604 : reserved
// 0x00608 : Data signal of HwReg_layerStartX_5
//           bit 15~0 - HwReg_layerStartX_5[15:0] (Read/Write)
//           others   - reserved
// 0x0060c : reserved
// 0x00610 : Data signal of HwReg_layerStartY_5
//           bit 15~0 - HwReg_layerStartY_5[15:0] (Read/Write)
//           others   - reserved
// 0x00614 : reserved
// 0x00618 : Data signal of HwReg_layerWidth_5
//           bit 15~0 - HwReg_layerWidth_5[15:0] (Read/Write)
//           others   - reserved
// 0x0061c : reserved
// 0x00620 : Data signal of HwReg_layerStride_5
//           bit 15~0 - HwReg_layerStride_5[15:0] (Read/Write)
//           others   - reserved
// 0x00624 : reserved
// 0x00628 : Data signal of HwReg_layerHeight_5
//           bit 15~0 - HwReg_layerHeight_5[15:0] (Read/Write)
//           others   - reserved
// 0x0062c : reserved
// 0x00630 : Data signal of HwReg_layerScaleFactor_5
//           bit 7~0 - HwReg_layerScaleFactor_5[7:0] (Read/Write)
//           others  - reserved
// 0x00634 : reserved
// 0x00638 : Data signal of HwReg_layerVideoFormat_5
//           bit 7~0 - HwReg_layerVideoFormat_5[7:0] (Read/Write)
//           others  - reserved
// 0x0063c : reserved
// 0x00640 : Data signal of HwReg_layer5_buf1_V
//           bit 31~0 - HwReg_layer5_buf1_V[31:0] (Read/Write)
// 0x00644 : Data signal of HwReg_layer5_buf1_V
//           bit 31~0 - HwReg_layer5_buf1_V[63:32] (Read/Write)
// 0x00648 : reserved
// 0x0064c : Data signal of HwReg_layer5_buf2_V
//           bit 31~0 - HwReg_layer5_buf2_V[31:0] (Read/Write)
// 0x00650 : Data signal of HwReg_layer5_buf2_V
//           bit 31~0 - HwReg_layer5_buf2_V[63:32] (Read/Write)
// 0x00654 : reserved
// 0x00700 : Data signal of HwReg_layerAlpha_6
//           bit 15~0 - HwReg_layerAlpha_6[15:0] (Read/Write)
//           others   - reserved
// 0x00704 : reserved
// 0x00708 : Data signal of HwReg_layerStartX_6
//           bit 15~0 - HwReg_layerStartX_6[15:0] (Read/Write)
//           others   - reserved
// 0x0070c : reserved
// 0x00710 : Data signal of HwReg_layerStartY_6
//           bit 15~0 - HwReg_layerStartY_6[15:0] (Read/Write)
//           others   - reserved
// 0x00714 : reserved
// 0x00718 : Data signal of HwReg_layerWidth_6
//           bit 15~0 - HwReg_layerWidth_6[15:0] (Read/Write)
//           others   - reserved
// 0x0071c : reserved
// 0x00720 : Data signal of HwReg_layerStride_6
//           bit 15~0 - HwReg_layerStride_6[15:0] (Read/Write)
//           others   - reserved
// 0x00724 : reserved
// 0x00728 : Data signal of HwReg_layerHeight_6
//           bit 15~0 - HwReg_layerHeight_6[15:0] (Read/Write)
//           others   - reserved
// 0x0072c : reserved
// 0x00730 : Data signal of HwReg_layerScaleFactor_6
//           bit 7~0 - HwReg_layerScaleFactor_6[7:0] (Read/Write)
//           others  - reserved
// 0x00734 : reserved
// 0x00738 : Data signal of HwReg_layerVideoFormat_6
//           bit 7~0 - HwReg_layerVideoFormat_6[7:0] (Read/Write)
//           others  - reserved
// 0x0073c : reserved
// 0x00740 : Data signal of HwReg_layer6_buf1_V
//           bit 31~0 - HwReg_layer6_buf1_V[31:0] (Read/Write)
// 0x00744 : Data signal of HwReg_layer6_buf1_V
//           bit 31~0 - HwReg_layer6_buf1_V[63:32] (Read/Write)
// 0x00748 : reserved
// 0x0074c : Data signal of HwReg_layer6_buf2_V
//           bit 31~0 - HwReg_layer6_buf2_V[31:0] (Read/Write)
// 0x00750 : Data signal of HwReg_layer6_buf2_V
//           bit 31~0 - HwReg_layer6_buf2_V[63:32] (Read/Write)
// 0x00754 : reserved
// 0x00800 : Data signal of HwReg_layerAlpha_7
//           bit 15~0 - HwReg_layerAlpha_7[15:0] (Read/Write)
//           others   - reserved
// 0x00804 : reserved
// 0x00808 : Data signal of HwReg_layerStartX_7
//           bit 15~0 - HwReg_layerStartX_7[15:0] (Read/Write)
//           others   - reserved
// 0x0080c : reserved
// 0x00810 : Data signal of HwReg_layerStartY_7
//           bit 15~0 - HwReg_layerStartY_7[15:0] (Read/Write)
//           others   - reserved
// 0x00814 : reserved
// 0x00818 : Data signal of HwReg_layerWidth_7
//           bit 15~0 - HwReg_layerWidth_7[15:0] (Read/Write)
//           others   - reserved
// 0x0081c : reserved
// 0x00820 : Data signal of HwReg_layerStride_7
//           bit 15~0 - HwReg_layerStride_7[15:0] (Read/Write)
//           others   - reserved
// 0x00824 : reserved
// 0x00828 : Data signal of HwReg_layerHeight_7
//           bit 15~0 - HwReg_layerHeight_7[15:0] (Read/Write)
//           others   - reserved
// 0x0082c : reserved
// 0x00830 : Data signal of HwReg_layerScaleFactor_7
//           bit 7~0 - HwReg_layerScaleFactor_7[7:0] (Read/Write)
//           others  - reserved
// 0x00834 : reserved
// 0x00838 : Data signal of HwReg_layerVideoFormat_7
//           bit 7~0 - HwReg_layerVideoFormat_7[7:0] (Read/Write)
//           others  - reserved
// 0x0083c : reserved
// 0x00840 : Data signal of HwReg_layer7_buf1_V
//           bit 31~0 - HwReg_layer7_buf1_V[31:0] (Read/Write)
// 0x00844 : Data signal of HwReg_layer7_buf1_V
//           bit 31~0 - HwReg_layer7_buf1_V[63:32] (Read/Write)
// 0x00848 : reserved
// 0x0084c : Data signal of HwReg_layer7_buf2_V
//           bit 31~0 - HwReg_layer7_buf2_V[31:0] (Read/Write)
// 0x00850 : Data signal of HwReg_layer7_buf2_V
//           bit 31~0 - HwReg_layer7_buf2_V[63:32] (Read/Write)
// 0x00854 : reserved
// 0x00900 : Data signal of HwReg_layerAlpha_8
//           bit 15~0 - HwReg_layerAlpha_8[15:0] (Read/Write)
//           others   - reserved
// 0x00904 : reserved
// 0x00908 : Data signal of HwReg_layerStartX_8
//           bit 15~0 - HwReg_layerStartX_8[15:0] (Read/Write)
//           others   - reserved
// 0x0090c : reserved
// 0x00910 : Data signal of HwReg_layerStartY_8
//           bit 15~0 - HwReg_layerStartY_8[15:0] (Read/Write)
//           others   - reserved
// 0x00914 : reserved
// 0x00918 : Data signal of HwReg_layerWidth_8
//           bit 15~0 - HwReg_layerWidth_8[15:0] (Read/Write)
//           others   - reserved
// 0x0091c : reserved
// 0x00920 : Data signal of HwReg_layerStride_8
//           bit 15~0 - HwReg_layerStride_8[15:0] (Read/Write)
//           others   - reserved
// 0x00924 : reserved
// 0x00928 : Data signal of HwReg_layerHeight_8
//           bit 15~0 - HwReg_layerHeight_8[15:0] (Read/Write)
//           others   - reserved
// 0x0092c : reserved
// 0x00930 : Data signal of HwReg_layerScaleFactor_8
//           bit 7~0 - HwReg_layerScaleFactor_8[7:0] (Read/Write)
//           others  - reserved
// 0x00934 : reserved
// 0x00938 : Data signal of HwReg_layerVideoFormat_8
//           bit 7~0 - HwReg_layerVideoFormat_8[7:0] (Read/Write)
//           others  - reserved
// 0x0093c : reserved
// 0x00940 : Data signal of HwReg_layer8_buf1_V
//           bit 31~0 - HwReg_layer8_buf1_V[31:0] (Read/Write)
// 0x00944 : Data signal of HwReg_layer8_buf1_V
//           bit 31~0 - HwReg_layer8_buf1_V[63:32] (Read/Write)
// 0x00948 : reserved
// 0x0094c : Data signal of HwReg_layer8_buf2_V
//           bit 31~0 - HwReg_layer8_buf2_V[31:0] (Read/Write)
// 0x00950 : Data signal of HwReg_layer8_buf2_V
//           bit 31~0 - HwReg_layer8_buf2_V[63:32] (Read/Write)
// 0x00954 : reserved
//
// 0x00A00 : Same set of  registers for layer 9
// 0x00B00 : Same set of  registers for layer 10
// 0x00C00 : Same set of  registers for layer 11
// 0x00D00 : Same set of  registers for layer 12
// 0x00E00 : Same set of  registers for layer 13
// 0x00F00 : Same set of  registers for layer 14
// 0x01000 : Same set of  registers for layer 15
// 0x02000 : Same set of  registers for layer 16
//
// 0x00ff0 : Data signal of HwReg_reserve
//           bit 15~0 - HwReg_reserve[15:0] (Read/Write)
//           others   - reserved
// 0x00ff4 : reserved
// 0x02000 : Data signal of HwReg_logoStartX
//           bit 15~0 - HwReg_logoStartX[15:0] (Read/Write)
//           others   - reserved
// 0x02004 : reserved
// 0x02008 : Data signal of HwReg_logoStartY
//           bit 15~0 - HwReg_logoStartY[15:0] (Read/Write)
//           others   - reserved
// 0x0200c : reserved
// 0x02010 : Data signal of HwReg_logoWidth
//           bit 15~0 - HwReg_logoWidth[15:0] (Read/Write)
//           others   - reserved
// 0x02014 : reserved
// 0x02018 : Data signal of HwReg_logoHeight
//           bit 15~0 - HwReg_logoHeight[15:0] (Read/Write)
//           others   - reserved
// 0x0201c : reserved
// 0x02020 : Data signal of HwReg_logoScaleFactor
//           bit 15~0 - HwReg_logoScaleFactor[15:0] (Read/Write)
//           others   - reserved
// 0x02024 : reserved
// 0x02028 : Data signal of HwReg_logoAlpha
//           bit 15~0 - HwReg_logoAlpha[15:0] (Read/Write)
//           others   - reserved
// 0x0202c : reserved
// 0x02030 : Data signal of HwReg_logoClrKeyMin_R
//           bit 7~0 - HwReg_logoClrKeyMin_R[7:0] (Read/Write)
//           others  - reserved
// 0x02034 : reserved
// 0x02038 : Data signal of HwReg_logoClrKeyMin_G
//           bit 7~0 - HwReg_logoClrKeyMin_G[7:0] (Read/Write)
//           others  - reserved
// 0x0203c : reserved
// 0x02040 : Data signal of HwReg_logoClrKeyMin_B
//           bit 7~0 - HwReg_logoClrKeyMin_B[7:0] (Read/Write)
//           others  - reserved
// 0x02044 : reserved
// 0x02048 : Data signal of HwReg_logoClrKeyMax_R
//           bit 7~0 - HwReg_logoClrKeyMax_R[7:0] (Read/Write)
//           others  - reserved
// 0x0204c : reserved
// 0x02050 : Data signal of HwReg_logoClrKeyMax_G
//           bit 7~0 - HwReg_logoClrKeyMax_G[7:0] (Read/Write)
//           others  - reserved
// 0x02054 : reserved
// 0x02058 : Data signal of HwReg_logoClrKeyMax_B
//           bit 7~0 - HwReg_logoClrKeyMax_B[7:0] (Read/Write)
//           others  - reserved
// 0x0205c : reserved
// 0x10000 ~
// 0x10fff : Memory 'HwReg_logoR_V' (4096 * 8b)
//           Word n : bit [ 7: 0] - HwReg_logoR_V[4n]
//                    bit [15: 8] - HwReg_logoR_V[4n+1]
//                    bit [23:16] - HwReg_logoR_V[4n+2]
//                    bit [31:24] - HwReg_logoR_V[4n+3]
// 0x20000 ~
// 0x20fff : Memory 'HwReg_logoG_V' (4096 * 8b)
//           Word n : bit [ 7: 0] - HwReg_logoG_V[4n]
//                    bit [15: 8] - HwReg_logoG_V[4n+1]
//                    bit [23:16] - HwReg_logoG_V[4n+2]
//                    bit [31:24] - HwReg_logoG_V[4n+3]
// 0x30000 ~
// 0x30fff : Memory 'HwReg_logoB_V' (4096 * 8b)
//           Word n : bit [ 7: 0] - HwReg_logoB_V[4n]
//                    bit [15: 8] - HwReg_logoB_V[4n+1]
//                    bit [23:16] - HwReg_logoB_V[4n+2]
//                    bit [31:24] - HwReg_logoB_V[4n+3]
// 0x40000 ~
// 0x40fff : Memory 'HwReg_logoA_V' (4096 * 8b)
//           Word n : bit [ 7: 0] - HwReg_logoA_V[4n]
//                    bit [15: 8] - HwReg_logoA_V[4n+1]
//                    bit [23:16] - HwReg_logoA_V[4n+2]
//                    bit [31:24] - HwReg_logoA_V[4n+3]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

/** Control register base address */
#define XV_MIX_CTRL_ADDR_AP_CTRL                       0x00000
/** Bit position for flush control bit */
#define XV_MIX_CTRL_BITS_FLUSH_BIT_POS				   (5)
/** Flush control bit mask */
#define XV_MIX_CTRL_BITS_FLUSH_BIT					   (1 << XV_MIX_CTRL_BITS_FLUSH_BIT_POS)
/** Bit position for flush status bit */
#define XV_MIX_CTRL_BITS_FLUSH_STATUSBIT_POS		   (6)
/** Flush status bit mask */
#define XV_MIX_CTRL_BITS_FLUSH_STATUSBIT			   (1 << XV_MIX_CTRL_BITS_FLUSH_STATUSBIT_POS)
/** Global Interrupt Enable register address */
#define XV_MIX_CTRL_ADDR_GIE                           0x00004
/** Interrupt Enable register address */
#define XV_MIX_CTRL_ADDR_IER                           0x00008
/** Interrupt Status register address */
#define XV_MIX_CTRL_ADDR_ISR                           0x0000c
/** Register address for frame width */
#define XV_MIX_CTRL_ADDR_HWREG_WIDTH_DATA              0x00010
/** Bit width for frame width data */
#define XV_MIX_CTRL_BITS_HWREG_WIDTH_DATA              16
/** Register address for frame height */
#define XV_MIX_CTRL_ADDR_HWREG_HEIGHT_DATA             0x00018
/** Bit width for frame height data */
#define XV_MIX_CTRL_BITS_HWREG_HEIGHT_DATA             16
/** Register address for video format */
#define XV_MIX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA       0x00020
/** Bit width for video format data */
#define XV_MIX_CTRL_BITS_HWREG_VIDEO_FORMAT_DATA       16
/** Register address for background Y (luma) or R (red) component */
#define XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_Y_R_DATA     0x00028
/** Bit width for background Y/R data */
#define XV_MIX_CTRL_BITS_HWREG_BACKGROUND_Y_R_DATA     16
/** Register address for background U (chroma) or G (green) component */
#define XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_U_G_DATA     0x00030
/** Bit width for background U/G data */
#define XV_MIX_CTRL_BITS_HWREG_BACKGROUND_U_G_DATA     16
/** Register address for background V (chroma) or B (blue) component */
#define XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_V_B_DATA     0x00038
/** Bit width for background V/B data */
#define XV_MIX_CTRL_BITS_HWREG_BACKGROUND_V_B_DATA     16
/** Register address for layer enable control */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERENABLE_DATA        0x00040
/** Bit width for layer enable data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERENABLE_DATA        16
/** Register address for color conversion matrix coefficient K11 */
#define XV_MIX_CTRL_ADDR_HWREG_K11_DATA        0x00048
/** Bit width for K11 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K11_DATA        16
/** Register address for color conversion matrix coefficient K12 */
#define XV_MIX_CTRL_ADDR_HWREG_K12_DATA        0x00050
/** Bit width for K12 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K12_DATA        16
/** Register address for color conversion matrix coefficient K13 */
#define XV_MIX_CTRL_ADDR_HWREG_K13_DATA        0x00058
/** Bit width for K13 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K13_DATA        16
/** Register address for color conversion matrix coefficient K21 */
#define XV_MIX_CTRL_ADDR_HWREG_K21_DATA        0x00060
/** Bit width for K21 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K21_DATA        16
/** Register address for color conversion matrix coefficient K22 */
#define XV_MIX_CTRL_ADDR_HWREG_K22_DATA        0x00068
/** Bit width for K22 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K22_DATA        16
/** Register address for color conversion matrix coefficient K23 */
#define XV_MIX_CTRL_ADDR_HWREG_K23_DATA        0x00070
/** Bit width for K23 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K23_DATA        16
/** Register address for color conversion matrix coefficient K31 */
#define XV_MIX_CTRL_ADDR_HWREG_K31_DATA        0x00078
/** Bit width for K31 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K31_DATA        16
/** Register address for color conversion matrix coefficient K32 */
#define XV_MIX_CTRL_ADDR_HWREG_K32_DATA        0x00080
/** Bit width for K32 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K32_DATA        16
/** Register address for color conversion matrix coefficient K33 */
#define XV_MIX_CTRL_ADDR_HWREG_K33_DATA        0x00088
/** Bit width for K33 coefficient data */
#define XV_MIX_CTRL_BITS_HWREG_K33_DATA        16
/** Register address for color offset R component */
#define XV_MIX_CTRL_ADDR_HWREG_R_DATA        0x00090
/** Bit width for R offset data */
#define XV_MIX_CTRL_BITS_HWREG_R_DATA        16
/** Register address for color offset G component */
#define XV_MIX_CTRL_ADDR_HWREG_G_DATA        0x00098
/** Bit width for G offset data */
#define XV_MIX_CTRL_BITS_HWREG_G_DATA        16
/** Register address for color offset B component */
#define XV_MIX_CTRL_ADDR_HWREG_B_DATA        0x000A0
/** Bit width for B offset data */
#define XV_MIX_CTRL_BITS_HWREG_B_DATA        16
/** Register address for layer 0 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA       0x00100
/** Bit width for layer 0 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_0_DATA       16
/** Register address for layer 0 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA      0x00108
/** Bit width for layer 0 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_0_DATA      16
/** Register address for layer 0 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA      0x00110
/** Bit width for layer 0 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_0_DATA      16
/** Register address for layer 0 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA       0x00118
/** Bit width for layer 0 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_0_DATA       16
/** Register address for layer 0 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_0_DATA      0x00120
/** Bit width for layer 0 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_0_DATA      16
/** Register address for layer 0 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA      0x00128
/** Bit width for layer 0 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_0_DATA      16
/** Register address for layer 0 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA 0x00130
/** Bit width for layer 0 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_0_DATA 8
/** Register address for layer 0 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_0_DATA 0x00138
/** Bit width for layer 0 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_0_DATA 8
/** Register address for layer 0 color conversion matrix coefficient K11 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K11_2_DATA        0x00140
/** Bit width for layer 0 K11 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K11_2_DATA        16
/** Register address for layer 0 color conversion matrix coefficient K12 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K12_2_DATA        0x00148
/** Bit width for layer 0 K12 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K12_2_DATA        16
/** Register address for layer 0 color conversion matrix coefficient K13 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K13_2_DATA        0x00150
/** Bit width for layer 0 K13 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K13_2_DATA        16
/** Register address for layer 0 color conversion matrix coefficient K21 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K21_2_DATA        0x00158
/** Bit width for layer 0 K21 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K21_2_DATA        16
/** Register address for layer 0 color conversion matrix coefficient K22 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K22_2_DATA        0x00160
/** Bit width for layer 0 K22 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K22_2_DATA        16
/** Register address for layer 0 color conversion matrix coefficient K23 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K23_2_DATA        0x00168
/** Bit width for layer 0 K23 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K23_2_DATA        16
/** Register address for layer 0 color conversion matrix coefficient K31 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K31_2_DATA        0x00170
/** Bit width for layer 0 K31 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K31_2_DATA        16
/** Register address for layer 0 color conversion matrix coefficient K32 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K32_2_DATA        0x00178
/** Bit width for layer 0 K32 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K32_2_DATA        16
/** Register address for layer 0 color conversion matrix coefficient K33 (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_K33_2_DATA        0x00180
/** Bit width for layer 0 K33 coefficient data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_K33_2_DATA        16
/** Register address for layer 0 color offset R component (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_R_2_DATA        0x00188
/** Bit width for layer 0 R offset data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_R_2_DATA        16
/** Register address for layer 0 color offset G component (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_G_2_DATA        0x00190
/** Bit width for layer 0 G offset data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_G_2_DATA        16
/** Register address for layer 0 color offset B component (second set) */
#define XV_MIX_CTRL_ADDR_HWREG_B_2_DATA        0x00198
/** Bit width for layer 0 B offset data (second set) */
#define XV_MIX_CTRL_BITS_HWREG_B_2_DATA        16
/** Register address for layer 1 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_1_DATA       0x00200
/** Bit width for layer 1 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_1_DATA       16
/** Register address for layer 1 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_1_DATA      0x00208
/** Bit width for layer 1 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_1_DATA      16
/** Register address for layer 1 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_1_DATA      0x00210
/** Bit width for layer 1 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_1_DATA      16
/** Register address for layer 1 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_1_DATA       0x00218
/** Bit width for layer 1 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_1_DATA       16
/** Register address for layer 1 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_1_DATA      0x00220
/** Bit width for layer 1 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_1_DATA      16
/** Register address for layer 1 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_1_DATA      0x00228
/** Bit width for layer 1 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_1_DATA      16
/** Register address for layer 1 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_1_DATA 0x00230
/** Bit width for layer 1 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_1_DATA 8
/** Register address for layer 1 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_1_DATA 0x00238
/** Bit width for layer 1 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_1_DATA 8
/** Register address for layer 1 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA      0x00240
/** Bit width for layer 1 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER1_BUF1_V_DATA      64
/** Register address for layer 1 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA      0x0024c
/** Bit width for layer 1 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER1_BUF2_V_DATA      64
/** Register address for layer 1 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA      0x00258
/** Bit width for layer 1 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER1_BUF3_V_DATA      64

/** Register address for layer 2 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_2_DATA       0x00300
/** Bit width for layer 2 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_2_DATA       16
/** Register address for layer 2 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_2_DATA      0x00308
/** Bit width for layer 2 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_2_DATA      16
/** Register address for layer 2 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_2_DATA      0x00310
/** Bit width for layer 2 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_2_DATA      16
/** Register address for layer 2 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_2_DATA       0x00318
/** Bit width for layer 2 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_2_DATA       16
/** Register address for layer 2 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_2_DATA      0x00320
/** Bit width for layer 2 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_2_DATA      16
/** Register address for layer 2 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_2_DATA      0x00328
/** Bit width for layer 2 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_2_DATA      16
/** Register address for layer 2 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_2_DATA 0x00330
/** Bit width for layer 2 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_2_DATA 8
/** Register address for layer 2 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_2_DATA 0x00338
/** Bit width for layer 2 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_2_DATA 8
/** Register address for layer 2 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA      0x00340
/** Bit width for layer 2 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER2_BUF1_V_DATA      64
/** Register address for layer 2 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA      0x0034c
/** Bit width for layer 2 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER2_BUF2_V_DATA      64
/** Register address for layer 2 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA      0x00358
/** Bit width for layer 2 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER2_BUF3_V_DATA      64

/** Register address for layer 3 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_3_DATA       0x00400
/** Bit width for layer 3 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_3_DATA       16
/** Register address for layer 3 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_3_DATA      0x00408
/** Bit width for layer 3 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_3_DATA      16
/** Register address for layer 3 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_3_DATA      0x00410
/** Bit width for layer 3 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_3_DATA      16
/** Register address for layer 3 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_3_DATA       0x00418
/** Bit width for layer 3 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_3_DATA       16
/** Register address for layer 3 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_3_DATA      0x00420
/** Bit width for layer 3 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_3_DATA      16
/** Register address for layer 3 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_3_DATA      0x00428
/** Bit width for layer 3 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_3_DATA      16
/** Register address for layer 3 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_3_DATA 0x00430
/** Bit width for layer 3 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_3_DATA 8
/** Register address for layer 3 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_3_DATA 0x00438
/** Bit width for layer 3 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_3_DATA 8
/** Register address for layer 3 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA      0x00440
/** Bit width for layer 3 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER3_BUF1_V_DATA      64
/** Register address for layer 3 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA      0x0044c
/** Bit width for layer 3 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER3_BUF2_V_DATA      64
/** Register address for layer 3 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA      0x00458
/** Bit width for layer 3 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER3_BUF3_V_DATA      64

/** Register address for layer 4 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_4_DATA       0x00500
/** Bit width for layer 4 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_4_DATA       16
/** Register address for layer 4 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_4_DATA      0x00508
/** Bit width for layer 4 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_4_DATA      16
/** Register address for layer 4 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_4_DATA      0x00510
/** Bit width for layer 4 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_4_DATA      16
/** Register address for layer 4 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_4_DATA       0x00518
/** Bit width for layer 4 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_4_DATA       16
/** Register address for layer 4 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_4_DATA      0x00520
/** Bit width for layer 4 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_4_DATA      16
/** Register address for layer 4 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_4_DATA      0x00528
/** Bit width for layer 4 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_4_DATA      16
/** Register address for layer 4 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_4_DATA 0x00530
/** Bit width for layer 4 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_4_DATA 8
/** Register address for layer 4 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_4_DATA 0x00538
/** Bit width for layer 4 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_4_DATA 8
/** Register address for layer 4 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA      0x00540
/** Bit width for layer 4 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER4_BUF1_V_DATA      64
/** Register address for layer 4 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA      0x0054c
/** Bit width for layer 4 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER4_BUF2_V_DATA      64
/** Register address for layer 4 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA      0x00558
/** Bit width for layer 4 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER4_BUF3_V_DATA      64

/** Register address for layer 5 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_5_DATA       0x00600
/** Bit width for layer 5 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_5_DATA       16
/** Register address for layer 5 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_5_DATA      0x00608
/** Bit width for layer 5 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_5_DATA      16
/** Register address for layer 5 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_5_DATA      0x00610
/** Bit width for layer 5 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_5_DATA      16
/** Register address for layer 5 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_5_DATA       0x00618
/** Bit width for layer 5 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_5_DATA       16
/** Register address for layer 5 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_5_DATA      0x00620
/** Bit width for layer 5 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_5_DATA      16
/** Register address for layer 5 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_5_DATA      0x00628
/** Bit width for layer 5 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_5_DATA      16
/** Register address for layer 5 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_5_DATA 0x00630
/** Bit width for layer 5 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_5_DATA 8
/** Register address for layer 5 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_5_DATA 0x00638
/** Bit width for layer 5 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_5_DATA 8
/** Register address for layer 5 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA      0x00640
/** Bit width for layer 5 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER5_BUF1_V_DATA      64
/** Register address for layer 5 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA      0x0064c
/** Bit width for layer 5 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER5_BUF2_V_DATA      64
/** Register address for layer 5 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA      0x00658
/** Bit width for layer 5 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER5_BUF3_V_DATA      64

/** Register address for layer 6 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_6_DATA       0x00700
/** Bit width for layer 6 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_6_DATA       16
/** Register address for layer 6 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_6_DATA      0x00708
/** Bit width for layer 6 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_6_DATA      16
/** Register address for layer 6 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_6_DATA      0x00710
/** Bit width for layer 6 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_6_DATA      16
/** Register address for layer 6 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_6_DATA       0x00718
/** Bit width for layer 6 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_6_DATA       16
/** Register address for layer 6 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_6_DATA      0x00720
/** Bit width for layer 6 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_6_DATA      16
/** Register address for layer 6 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_6_DATA      0x00728
/** Bit width for layer 6 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_6_DATA      16
/** Register address for layer 6 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_6_DATA 0x00730
/** Bit width for layer 6 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_6_DATA 8
/** Register address for layer 6 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_6_DATA 0x00738
/** Bit width for layer 6 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_6_DATA 8
/** Register address for layer 6 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA      0x00740
/** Bit width for layer 6 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER6_BUF1_V_DATA      64
/** Register address for layer 6 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA      0x0074c
/** Bit width for layer 6 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER6_BUF2_V_DATA      64
/** Register address for layer 6 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA      0x00758
/** Bit width for layer 6 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER6_BUF3_V_DATA      64

/** Register address for layer 7 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_7_DATA       0x00800
/** Bit width for layer 7 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_7_DATA       16
/** Register address for layer 7 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_7_DATA      0x00808
/** Bit width for layer 7 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_7_DATA      16
/** Register address for layer 7 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_7_DATA      0x00810
/** Bit width for layer 7 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_7_DATA      16
/** Register address for layer 7 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_7_DATA       0x00818
/** Bit width for layer 7 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_7_DATA       16
/** Register address for layer 7 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_7_DATA      0x00820
/** Bit width for layer 7 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_7_DATA      16
/** Register address for layer 7 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_7_DATA      0x00828
/** Bit width for layer 7 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_7_DATA      16
/** Register address for layer 7 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_7_DATA 0x00830
/** Bit width for layer 7 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_7_DATA 8
/** Register address for layer 7 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_7_DATA 0x00838
/** Bit width for layer 7 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_7_DATA 8
/** Register address for layer 7 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA      0x00840
/** Bit width for layer 7 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER7_BUF1_V_DATA      64
/** Register address for layer 7 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA      0x0084c
/** Bit width for layer 7 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER7_BUF2_V_DATA      64
/** Register address for layer 7 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA      0x00858
/** Bit width for layer 7 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER7_BUF3_V_DATA      64

/** Register address for layer 8 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_8_DATA       0x00900
/** Bit width for layer 8 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_8_DATA       16
/** Register address for layer 8 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_8_DATA      0x00908
/** Bit width for layer 8 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_8_DATA      16
/** Register address for layer 8 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_8_DATA      0x00910
/** Bit width for layer 8 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_8_DATA      16
/** Register address for layer 8 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_8_DATA       0x00918
/** Bit width for layer 8 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_8_DATA       16
/** Register address for layer 8 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_8_DATA      0x00920
/** Bit width for layer 8 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_8_DATA      16
/** Register address for layer 8 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_8_DATA      0x00928
/** Bit width for layer 8 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_8_DATA      16
/** Register address for layer 8 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_8_DATA 0x00930
/** Bit width for layer 8 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_8_DATA 8
/** Register address for layer 8 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_8_DATA 0x00938
/** Bit width for layer 8 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_8_DATA 8
/** Register address for layer 8 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA      0x00940
/** Bit width for layer 8 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER8_BUF1_V_DATA      64
/** Register address for layer 8 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA      0x0094c
/** Bit width for layer 8 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER8_BUF2_V_DATA      64
/** Register address for layer 8 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA      0x00958
/** Bit width for layer 8 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER8_BUF3_V_DATA      64

/** Register address for layer 9 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_9_DATA       0x00A00
/** Bit width for layer 9 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_9_DATA       16
/** Register address for layer 9 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_9_DATA      0x00A08
/** Bit width for layer 9 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_9_DATA      16
/** Register address for layer 9 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_9_DATA      0x00A10
/** Bit width for layer 9 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_9_DATA      16
/** Register address for layer 9 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_9_DATA       0x00A18
/** Bit width for layer 9 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_9_DATA       16
/** Register address for layer 9 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_9_DATA      0x00A20
/** Bit width for layer 9 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_9_DATA      16
/** Register address for layer 9 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_9_DATA      0x00A28
/** Bit width for layer 9 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_9_DATA      16
/** Register address for layer 9 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_9_DATA 0x00A30
/** Bit width for layer 9 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_9_DATA 8
/** Register address for layer 9 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_9_DATA 0x00A38
/** Bit width for layer 9 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_9_DATA 8
/** Register address for layer 9 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA      0x00A40
/** Bit width for layer 9 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER9_BUF1_V_DATA      64
/** Register address for layer 9 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA      0x00A4c
/** Bit width for layer 9 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER9_BUF2_V_DATA      64
/** Register address for layer 9 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA      0x00A58
/** Bit width for layer 9 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER9_BUF3_V_DATA      64

/** Register address for layer 10 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_10_DATA       0x00B00
/** Bit width for layer 10 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_10_DATA       16
/** Register address for layer 10 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_10_DATA      0x00B08
/** Bit width for layer 10 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_10_DATA      16
/** Register address for layer 10 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_10_DATA      0x00B10
/** Bit width for layer 10 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_10_DATA      16
/** Register address for layer 10 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_10_DATA       0x00B18
/** Bit width for layer 10 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_10_DATA       16
/** Register address for layer 10 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_10_DATA      0x00B20
/** Bit width for layer 10 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_10_DATA      16
/** Register address for layer 10 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_10_DATA      0x00B28
/** Bit width for layer 10 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_10_DATA      16
/** Register address for layer 10 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_10_DATA 0x00B30
/** Bit width for layer 10 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_10_DATA 8
/** Register address for layer 10 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_10_DATA 0x00B38
/** Bit width for layer 10 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_10_DATA 8
/** Register address for layer 10 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA     0x00B40
/** Bit width for layer 10 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER10_BUF1_V_DATA     64
/** Register address for layer 10 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA     0x00B4c
/** Bit width for layer 10 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER10_BUF2_V_DATA     64
/** Register address for layer 10 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA      0x00B58
/** Bit width for layer 10 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER10_BUF3_V_DATA      64

/** Register address for layer 11 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_11_DATA       0x00C00
/** Bit width for layer 11 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_11_DATA       16
/** Register address for layer 11 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_11_DATA      0x00C08
/** Bit width for layer 11 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_11_DATA      16
/** Register address for layer 11 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_11_DATA      0x00C10
/** Bit width for layer 11 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_11_DATA      16
/** Register address for layer 11 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_11_DATA       0x00C18
/** Bit width for layer 11 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_11_DATA       16
/** Register address for layer 11 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_11_DATA      0x00C20
/** Bit width for layer 11 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_11_DATA      16
/** Register address for layer 11 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_11_DATA      0x00C28
/** Bit width for layer 11 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_11_DATA      16
/** Register address for layer 11 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_11_DATA 0x00C30
/** Bit width for layer 11 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_11_DATA 8
/** Register address for layer 11 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_11_DATA 0x00C38
/** Bit width for layer 11 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_11_DATA 8
/** Register address for layer 11 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA     0x00C40
/** Bit width for layer 11 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER11_BUF1_V_DATA     64
/** Register address for layer 11 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA     0x00C4c
/** Bit width for layer 11 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER11_BUF2_V_DATA     64
/** Register address for layer 11 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA      0x00C58
/** Bit width for layer 11 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER11_BUF3_V_DATA      64

/** Register address for layer 12 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_12_DATA       0x00D00
/** Bit width for layer 12 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_12_DATA       16
/** Register address for layer 12 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_12_DATA      0x00D08
/** Bit width for layer 12 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_12_DATA      16
/** Register address for layer 12 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_12_DATA      0x00D10
/** Bit width for layer 12 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_12_DATA      16
/** Register address for layer 12 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_12_DATA       0x00D18
/** Bit width for layer 12 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_12_DATA       16
/** Register address for layer 12 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_12_DATA      0x00D20
/** Bit width for layer 12 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_12_DATA      16
/** Register address for layer 12 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_12_DATA      0x00D28
/** Bit width for layer 12 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_12_DATA      16
/** Register address for layer 12 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_12_DATA 0x00D30
/** Bit width for layer 12 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_12_DATA 8
/** Register address for layer 12 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_12_DATA 0x00D38
/** Bit width for layer 12 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_12_DATA 8
/** Register address for layer 12 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA     0x00D40
/** Bit width for layer 12 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER12_BUF1_V_DATA     64
/** Register address for layer 12 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA     0x00D4c
/** Bit width for layer 12 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER12_BUF2_V_DATA     64
/** Register address for layer 12 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA      0x00D58
/** Bit width for layer 12 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER12_BUF3_V_DATA      64

/** Register address for layer 13 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_13_DATA       0x00E00
/** Bit width for layer 13 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_13_DATA       16
/** Register address for layer 13 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_13_DATA      0x00E08
/** Bit width for layer 13 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_13_DATA      16
/** Register address for layer 13 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_13_DATA      0x00E10
/** Bit width for layer 13 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_13_DATA      16
/** Register address for layer 13 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_13_DATA       0x00E18
/** Bit width for layer 13 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_13_DATA       16
/** Register address for layer 13 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_13_DATA      0x00E20
/** Bit width for layer 13 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_13_DATA      16
/** Register address for layer 13 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_13_DATA      0x00E28
/** Bit width for layer 13 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_13_DATA      16
/** Register address for layer 13 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_13_DATA 0x00E30
/** Bit width for layer 13 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_13_DATA 8
/** Register address for layer 13 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_13_DATA 0x00E38
/** Bit width for layer 13 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_13_DATA 8
/** Register address for layer 13 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA     0x00E40
/** Bit width for layer 13 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER13_BUF1_V_DATA     64
/** Register address for layer 13 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA     0x00E4c
/** Bit width for layer 13 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER13_BUF2_V_DATA     64
/** Register address for layer 13 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA      0x00E58
/** Bit width for layer 13 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER13_BUF3_V_DATA      64

/** Register address for layer 14 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_14_DATA       0x00F00
/** Bit width for layer 14 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_14_DATA       16
/** Register address for layer 14 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_14_DATA      0x00F08
/** Bit width for layer 14 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_14_DATA      16
/** Register address for layer 14 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_14_DATA      0x00F10
/** Bit width for layer 14 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_14_DATA      16
/** Register address for layer 14 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_14_DATA       0x00F18
/** Bit width for layer 14 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_14_DATA       16
/** Register address for layer 14 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_14_DATA      0x00F20
/** Bit width for layer 14 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_14_DATA      16
/** Register address for layer 14 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_14_DATA      0x00F28
/** Bit width for layer 14 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_14_DATA      16
/** Register address for layer 14 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_14_DATA 0x00F30
/** Bit width for layer 14 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_14_DATA 8
/** Register address for layer 14 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_14_DATA 0x00F38
/** Bit width for layer 14 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_14_DATA 8
/** Register address for layer 14 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA     0x00F40
/** Bit width for layer 14 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER14_BUF1_V_DATA     64
/** Register address for layer 14 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA     0x00F4c
/** Bit width for layer 14 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER14_BUF2_V_DATA     64
/** Register address for layer 14 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA      0x00F58
/** Bit width for layer 14 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER14_BUF3_V_DATA      64

/** Register address for layer 15 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_15_DATA       0x001000
/** Bit width for layer 15 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_15_DATA       16
/** Register address for layer 15 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_15_DATA      0x001008
/** Bit width for layer 15 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_15_DATA      16
/** Register address for layer 15 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_15_DATA      0x001010
/** Bit width for layer 15 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_15_DATA      16
/** Register address for layer 15 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_15_DATA       0x001018
/** Bit width for layer 15 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_15_DATA       16
/** Register address for layer 15 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_15_DATA      0x001020
/** Bit width for layer 15 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_15_DATA      16
/** Register address for layer 15 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_15_DATA      0x001028
/** Bit width for layer 15 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_15_DATA      16
/** Register address for layer 15 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_15_DATA 0x001030
/** Bit width for layer 15 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_15_DATA 8
/** Register address for layer 15 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_15_DATA 0x001038
/** Bit width for layer 15 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_15_DATA 8
/** Register address for layer 15 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA     0x001040
/** Bit width for layer 15 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER15_BUF1_V_DATA     64
/** Register address for layer 15 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA     0x00104c
/** Bit width for layer 15 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER15_BUF2_V_DATA     64
/** Register address for layer 15 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA      0x001058
/** Bit width for layer 15 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER15_BUF3_V_DATA      64

/** Register address for layer 16 alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_16_DATA       0x001100
/** Bit width for layer 16 alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERALPHA_16_DATA       16
/** Register address for layer 16 horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_16_DATA      0x001108
/** Bit width for layer 16 start X data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTX_16_DATA      16
/** Register address for layer 16 vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_16_DATA      0x001110
/** Bit width for layer 16 start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTARTY_16_DATA      16
/** Register address for layer 16 width */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_16_DATA       0x001118
/** Bit width for layer 16 width data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERWIDTH_16_DATA       16
/** Register address for layer 16 stride (bytes per line) */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_16_DATA      0x001120
/** Bit width for layer 16 stride data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSTRIDE_16_DATA      16
/** Register address for layer 16 height */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_16_DATA      0x001128
/** Bit width for layer 16 height data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERHEIGHT_16_DATA      16
/** Register address for layer 16 scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_16_DATA 0x001130
/** Bit width for layer 16 scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERSCALEFACTOR_16_DATA 8
/** Register address for layer 16 video format */
#define XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_16_DATA 0x001138
/** Bit width for layer 16 video format data */
#define XV_MIX_CTRL_BITS_HWREG_LAYERVIDEOFORMAT_16_DATA 8
/** Register address for layer 16 buffer 1 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA     0x001140
/** Bit width for layer 16 buffer 1 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER16_BUF1_V_DATA     64
/** Register address for layer 16 buffer 2 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA     0x00114c
/** Bit width for layer 16 buffer 2 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER16_BUF2_V_DATA     64
/** Register address for layer 16 buffer 3 base address */
#define XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA      0x001158
/** Bit width for layer 16 buffer 3 address data */
#define XV_MIX_CTRL_BITS_HWREG_LAYER16_BUF3_V_DATA      64


/** Register address for hardware reserve register */
#define XV_MIX_CTRL_ADDR_HWREG_RESERVE_DATA            0x00ff0
/** Bit width for reserve register data */
#define XV_MIX_CTRL_BITS_HWREG_RESERVE_DATA            16

/** Register address for logo horizontal start position */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTX_DATA         0x02000
/** Bit width for logo start X data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOSTARTX_DATA         16
/** Register address for logo vertical start position */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTY_DATA         0x02008
/** Bit width for logo start Y data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOSTARTY_DATA         16
/** Register address for logo width */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOWIDTH_DATA          0x02010
/** Bit width for logo width data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOWIDTH_DATA          16
/** Register address for logo height */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOHEIGHT_DATA         0x02018
/** Bit width for logo height data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOHEIGHT_DATA         16
/** Register address for logo scale factor */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOSCALEFACTOR_DATA    0x02020
/** Bit width for logo scale factor data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOSCALEFACTOR_DATA    16
/** Register address for logo alpha (transparency) value */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOALPHA_DATA          0x02028
/** Bit width for logo alpha data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOALPHA_DATA          16
/** Register address for logo color key minimum red component */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_R_DATA    0x02030
/** Bit width for logo color key minimum red data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOCLRKEYMIN_R_DATA    8
/** Register address for logo color key minimum green component */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_G_DATA    0x02038
/** Bit width for logo color key minimum green data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOCLRKEYMIN_G_DATA    8
/** Register address for logo color key minimum blue component */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_B_DATA    0x02040
/** Bit width for logo color key minimum blue data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOCLRKEYMIN_B_DATA    8
/** Register address for logo color key maximum red component */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_R_DATA    0x02048
/** Bit width for logo color key maximum red data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOCLRKEYMAX_R_DATA    8
/** Register address for logo color key maximum green component */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_G_DATA    0x02050
/** Bit width for logo color key maximum green data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOCLRKEYMAX_G_DATA    8
/** Register address for logo color key maximum blue component */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_B_DATA    0x02058
/** Bit width for logo color key maximum blue data */
#define XV_MIX_CTRL_BITS_HWREG_LOGOCLRKEYMAX_B_DATA    8

/** Base address for logo red component memory region */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE            0x10000
/** High address for logo red component memory region */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH            0x10fff
/** Data width for logo red component memory */
#define XV_MIX_CTRL_WIDTH_HWREG_LOGOR_V                8
/** Memory depth for logo red component (number of elements) */
#define XV_MIX_CTRL_DEPTH_HWREG_LOGOR_V                4096
/** Base address for logo green component memory region */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE            0x20000
/** High address for logo green component memory region */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH            0x20fff
/** Data width for logo green component memory */
#define XV_MIX_CTRL_WIDTH_HWREG_LOGOG_V                8
/** Memory depth for logo green component (number of elements) */
#define XV_MIX_CTRL_DEPTH_HWREG_LOGOG_V                4096
/** Base address for logo blue component memory region */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE            0x30000
/** High address for logo blue component memory region */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH            0x30fff
/** Data width for logo blue component memory */
#define XV_MIX_CTRL_WIDTH_HWREG_LOGOB_V                8
/** Memory depth for logo blue component (number of elements) */
#define XV_MIX_CTRL_DEPTH_HWREG_LOGOB_V                4096
/** Base address for logo alpha component memory region */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE            0x40000
/** High address for logo alpha component memory region */
#define XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH            0x40fff
/** Data width for logo alpha component memory */
#define XV_MIX_CTRL_WIDTH_HWREG_LOGOA_V                8
/** Memory depth for logo alpha component (number of elements) */
#define XV_MIX_CTRL_DEPTH_HWREG_LOGOA_V                4096

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
