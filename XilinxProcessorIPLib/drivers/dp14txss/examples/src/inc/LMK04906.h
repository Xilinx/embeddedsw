/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
