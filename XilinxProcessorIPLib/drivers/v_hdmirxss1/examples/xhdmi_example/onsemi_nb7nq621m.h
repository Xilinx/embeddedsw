/******************************************************************************
*
* Copyright (C) 2018 – 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file onsemi_nb7nq621m.h
* @addtogroup ONSEMI_NB7NQ621M
* @{
* 
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  YH     18/12/24 Initial release.
* </pre>
*
******************************************************************************/

#ifndef ONSEMI_NB7NQ621M_H
#define ONSEMI_NB7NQ621M_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == 6) /*GTYE4*/
#define XPS_BOARD_VCU118
#else
/* Place-holder for other boards in future */
#endif

/************************** Constant Definitions *****************************/
#define REG04_BIT7_LANE_CTRL_GLOBAL		0
#define REG04_BIT7_LANE_CTRL_INDIVIDUAL	1

#define REG04_BIT6_LANE_SWAP_DISABLE	0
#define REG04_BIT6_LANE_SWAP_ENABLE		1

#define REG04_BIT53_MODE_TMDS_HIZ		0
#define REG04_BIT53_MODE_TMDS_300		1
#define REG04_BIT53_MODE_TMDS_200		2
#define REG04_BIT53_MODE_TMDS_100		3
#define REG04_BIT53_MODE_FRL_DC			4
#define REG04_BIT53_MODE_FRL_AC			6
#define REG04_BIT53_MODE_ML				7


 
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int ONSEMI_NB7NQ621M_Init(void *IicPtr, u8 I2CSlaveAddress, u8 IsTx);
int ONSEMI_NB7NQ621M_CheckDeviceID(void *IicPtr, u8 I2CSlaveAddress);
int ONSEMI_NB7NQ621M_LineRateReconfig(void *IicPtr, u8 I2CSlaveAddress,
		u8 IsFRL, u64 LineRate);
void ONSEMI_NB7NQ621M_RegisterDump(void *IicPtr, u8 I2CSlaveAddress);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* ONSEMI_NB7NQ621M_H */
/** @} */
