/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XILSKEY_JTAG_H
#define XILSKEY_JTAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xilskey_jscmd.h"

/**
 * @name Jtag API declarations
 * @{
 */
/**< Prototype declarations for Jtag API's */
void JtagInitGpio(XilSKey_Register Register);
void GpioConfig(volatile unsigned long *addr, unsigned long mask, unsigned long val);
int JtagServerInit(XilSKey_EPl *PlInstancePtr);
int JtagValidateMioPins(XilSKey_Register Register);
void JtagWrite(unsigned char row, unsigned char bit);
void JtagRead(unsigned char row, unsigned int * row_data, unsigned char marginOption);
int JtagWrite_Ultrascale(u8 Row, u8 Bit, u8 Page, u8 Redundant);
void JtagRead_Ultrascale(u8 Row, u32 *RowData, u8 MarginOption,
			u8 Page, u8 Redundant);
void JtagRead_Status_Ultrascale(u32 *Rowdata);
void Jtag_Read_Sysmon(u8 Row, u32 *Row_Data);
u32 JtagAES_Check_Ultrascale(u32 *Crc, u8 MarginOption);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /*XILSKEY_JTAG_H*/
