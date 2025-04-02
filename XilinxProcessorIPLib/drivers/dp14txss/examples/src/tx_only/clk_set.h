/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 *
 * @file clk_set.h
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   Nishant 20/012/19 Added suppport for vck190
 *
*******************************************************************************/

#ifndef SRC_CLK_SET_H_
#define SRC_CLK_SET_H_
#ifdef __cplusplus
extern "C" {
#endif

u32 clk_set(u8 i2c_mux_addr, u8 i2c_dev_addr, double set_freq);

#ifdef __cplusplus
}
#endif
#endif /* SRC_CLK_SET_H_ */
