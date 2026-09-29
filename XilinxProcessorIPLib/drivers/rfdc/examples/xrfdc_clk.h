/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef RFDC_CLK_H_
#define RFDC_CLK_H_

/****************************************************************************/
/**
*
* @file xrfdc_clk.h
* @brief Declares the clock helper interface for the RFdc examples.
*
******************************************************************************/

#ifdef XPS_BOARD_ZCU111

void LMX2594ClockConfig(int XIicBus, int XFrequency);
void LMK04208ClockConfig(int XIicBus, unsigned int LMK04208_CKin[1][26]);

#endif /* XPS_BOARD_ZCU111 */
#endif /* RFDC_CLK_H_ */
