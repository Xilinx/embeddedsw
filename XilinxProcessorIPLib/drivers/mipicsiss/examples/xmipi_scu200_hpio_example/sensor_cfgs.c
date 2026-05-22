/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file sensor_cfgs.c
 *
 * This file contains the Omnivision IMX219 CSI2 Camera sensor configurations for
 *
 * SONY IMX219
 * - 1920x1080@60fps	(quad lane)
 *
 * The structure names are sensor_<lane_count>L_<resolution>_<fps>fps_regs[]
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.00  pg    12/07/17 Initial release.
 * </pre>
 *
 ******************************************************************************/

#include "sensor_cfgs.h"
#include "xparameters.h"

struct regval_list IMX219_config_1080p_60fps_regs[] = {
{0x30EB, 0x05},
{0x30EB, 0x0C},
{0x300A, 0xFF},
{0x300B, 0xFF},
{0x30EB, 0x05},
{0x30EB, 0x09},
{0x0114, 0x03}, // 2-wire csi
{0x0128, 0x00}, // auto MIPI global timing
{0x012A, 0x18}, // INCK freq: 24.0Mhz
{0x012B, 0x00},
{0x0160, 0x04}, // frame length lines = 1113
{0x0161, 0x59},
{0x0162, 0x0D}, // line length pixels = 3448
{0x0163, 0x78},
{0x0164, 0x02}, // x-start address = 680
{0x0165, 0xA8},
{0x0166, 0x0A}, // x-end address = 2599
{0x0167, 0x27},
{0x0168, 0x02}, // y-start address = 692
{0x0169, 0xB4},
{0x016A, 0x06}, // y-end address = 1771
{0x016B, 0xEB},
{0x016C, 0x07}, // x-output size = 1920
{0x016D, 0x80},
{0x016E, 0x04}, // y-output size = 1080
{0x016F, 0x38},
{0x0170, 0x01}, //

// {0x015A, 0x03}, // Coarse Integration Time MSB
// {0x015B, 0xE8}, // Coarse Integration Time LSB  } = 1000 lines exposure
// {0x0157, 0x80}, // Analogue Gain = 2x (128/256 code)
// {0x0158, 0x01}, // Digital Gain MSB = 1x (no digital gain)
// {0x0159, 0x00}, // Digital Gain LSB

{0x015A, 0x04}, // Coarse Integration Time MSB
{0x015B, 0x50}, // Coarse Integration Time LSB  } = 1104 lines (near max)
{0x0157, 0xC0}, // Analogue Gain = 4x
{0x0158, 0x02}, // Digital Gain = 2x
{0x0159, 0x00},


{0x0171, 0x01},
{0x0174, 0x00},
{0x0175, 0x00},
{0x018C, 0x0A},
{0x018D, 0x0A},
{0x0301, 0x05}, // video timing pixel clock divider value = 5
{0x0303, 0x01}, // video timing system clock divider value = 1
{0x0304, 0x03}, // external clock 24-27MHz
{0x0305, 0x03}, // external clock 24-27MHz
{0x0306, 0x00}, // PLL Video Timing system multiplier value = 57
{0x0307, 0x57},
{0x0309, 0x0A}, // output pixel clock divider value = 10
{0x030B, 0x01}, // output system clock divider value = 1
{0x030C, 0x00}, // PLL output system multiplier value = 114
{0x030D, 0x72},
{0x455E, 0x00},
{0x471E, 0x4B},
{0x4767, 0x0F},
{0x4750, 0x14},
{0x4540, 0x00},
{0x47B4, 0x14},
{0x4713, 0x30},
{0x478B, 0x10},
{0x478F, 0x10},
{0x4793, 0x10},
{0x4797, 0x0E},
{0x479B, 0x0E},
{0x0100, 0x01}
};


#define	ARRAY_SIZE(x)	sizeof((x))/sizeof((x)[0])


const int length_IMX219_config_1080p_60fps_regs =
		ARRAY_SIZE(IMX219_config_1080p_60fps_regs);
