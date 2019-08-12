/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fzetta_fmc_iic.h
 *
 * FMC configuration file
 *
 * This file configures the FMC card for KCU116 SDI Tx to SDI Rx loopback design
 * board.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ---- ---- ---------- --------------------------------------------------
 * 1.0  jsr   03/07/2018 Initial version
 * </pre>
 *
 ******************************************************************************/
#ifndef FZETTA_FMC_IIC_H_
#define FZETTA_FMC_IIC_H_

#include <stdio.h>
#include "xiic.h"

XIic_Config *fzetta_fmc_Iic_ConfigPtr;	/* Pointer to configuration data */
XIic fzetta_fmc_Iic; /* The driver instance for IIC Device */

#define XBAR_IIC_WRITE_ADDR 0x50
/*****************************************************************************/
/**
 *
 * This function  Initializes IIC IP and its corresponding drivers and configptr instances.
 *
 * @param	Dev_ID  Device ID.
 *
 * @return	XST_SUCCESS if initialization is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_iic_init(u8 Dev_ID);
/*****************************************************************************/
/**
 *
 * This function  Fidus Zetta FMC Xbar Switch (DS10CP15A)
 * IIC Register Write Sequence:
 * 		START --> SMB ADDR + W --> ACK --> Xbar Reg ADDR \
 * 				         	   --> ACK --> Xbar Reg DATA --> ACK --> STOP
 *
 * @param	RegAddr  Register Address.
 * @param       RegData  Register Data
 *
 * @return	XST_SUCCESS if register write is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_iic_xbar_register_write(u8 RegAddr, u8 RegData);
/*****************************************************************************/
/**
 *
 * This function  Fidus Zetta FMC Xbar Switch DS10CP15A
 * Register Read IIC Sequence:
 * 		START --> SMB ADDR + W --> ACK --> Xbar Reg ADDR --> ACK \
 * 	--> START --> SMB ADDR + R --> ACK --> Xbar Reg DATA --> NACK -->STOP
 *	NOT WORKING YET
 *
 * @param	RegAddr  Register Address.
 *
 * @return	XST_SUCCESS if register read is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

u8 fzetta_fmc_iic_xbar_register_read(u8 RegAddr);

#endif /* FZETTA_FMC_IIC_H_ */
