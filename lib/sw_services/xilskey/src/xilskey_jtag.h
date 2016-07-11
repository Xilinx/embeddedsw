/******************************************************************************
*
* Copyright (C) 2013 - 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef XILSKEY_JTAG_H
#define XILSKEY_JTAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xilskey_jscmd.h"

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

#ifdef __cplusplus
}
#endif

#endif /*XILSKEY_JTAG_H*/
