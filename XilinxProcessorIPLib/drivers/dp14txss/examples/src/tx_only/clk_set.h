/******************************************************************************
* Copyright (C) 2010 - 20209Xilinx, Inc.  All rights reserved.
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

u32 clk_set(u8 i2c_mux_addr, u8 i2c_dev_addr, double set_freq);

#endif /* SRC_CLK_SET_H_ */
