/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file LMK04906.h
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
#ifndef LMK04906_H_
#define LMK04906_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "xspi.h"
#include "xparameters.h"
#ifndef SDT
#include "microblaze_sleep.h"
#endif

#ifndef SDT
#define  LMK04906_DEVICE_ID  XPAR_SPI_0_DEVICE_ID
#endif
#ifndef SDT
#define  LMK04906_DEVICE_BASEADDR  XPAR_SPI_0_BASEADDR
#else
#define  LMK04906_DEVICE_BASEADDR  XPAR_XSPI_0_BASEADDR
#endif

typedef struct {
	u32 SPI_BaseAddr;
} LMK04906_SPI_Info;

//void LMK04906_init(XSpi_Config *SPI_LMK04906_Conf ,XSpi *SPI_LMK04906);
void LMK04906_init(XSpi *SPI_LMK04906);
int  IF_LoopBack_Test();
int  LMK04906_RegWrite(XSpi *SPI_LMK04906 , u32 RegData ,u32 RegAddr);

void Soft_Reset();
void Set_Option(u32 Option);
u32  Get_Option();
u32  Get_Status();
u32  Rx_Data_Read();
void Tx_Data_Write(u32 WriteData);

#ifdef __cplusplus
}
#endif
#endif /* LMK04906_H_ */
