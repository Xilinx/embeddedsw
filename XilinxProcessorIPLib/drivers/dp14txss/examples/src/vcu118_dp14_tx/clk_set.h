/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
 * 1.0   Kei  08/09/17 Initial release.
 *
*******************************************************************************/

#ifndef SRC_CLK_SET_H_
#define SRC_CLK_SET_H_

u32 clk_set(u8 i2c_mux_addr, u8 i2c_dev_addr, double set_freq);

#endif /* SRC_CLK_SET_H_ */
