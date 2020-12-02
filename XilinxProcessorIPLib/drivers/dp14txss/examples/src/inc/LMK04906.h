/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
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
#include <stdlib.h>
#include "xspi.h"
#include "xparameters.h"
#include "microblaze_sleep.h"

#define  LMK04906_DEVICE_ID  XPAR_SPI_0_DEVICE_ID
#define  LMK04906_DEVICE_BASEADDR  XPAR_SPI_0_BASEADDR

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
#endif /* LMK04906_H_ */
