/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 *
 * @file clk_set.h
 *
 * MODIFICATION HISTORY:
 *
 * Ver  Who      Date      Changes
 * ---- ---      --------  --------------------------------------------------.
 * 1.00  ND      18/10/22  Common DP 2.1 tx only application for zcu102 and
 * 						  vcu118
 * 1.01	ND		26/02/24  Added support for 13.5 and 20G
 *
*******************************************************************************/

#ifndef SRC_CLK_SET_H_
#define SRC_CLK_SET_H_

u32 clk_set(u8 i2c_mux_addr, u8 i2c_dev_addr, double set_freq);

#endif /* SRC_CLK_SET_H_ */
