/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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

extern XIic_Config *fzetta_fmc_Iic_ConfigPtr;	/* Pointer to configuration data */
extern XIic fzetta_fmc_Iic; /* The driver instance for IIC Device */

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
