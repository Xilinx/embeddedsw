/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef SRC_CLK_SET_H_
#define SRC_CLK_SET_H_

//int PLL_init_Seting(XSpi *SPI_LMK04906, u32 div_value);
u32 clk_set(u8 i2c_mux_addr, u8 i2c_dev_addr, double set_freq);

#endif /* SRC_CLK_SET_H_ */
