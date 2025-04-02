/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file PLL_Conf.h
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI    07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#ifndef PLL_CONF_H_
#define PLL_CONF_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "LMK04906.h"


#define INIT_CMD_NUM 34
int PLL_init_Seting(XSpi *SPI_LMK04906);

void CLK81MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num);
void CLK135MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num);
void CLK162MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num);
void CLK270MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num);
void PLL_RegWrite (XSpi *SPI_LMK04906 , u32 RegData , u32 RegNum);
#ifdef __cplusplus
}
#endif
#endif /* PLL_CONF_H_ */
