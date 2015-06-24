/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
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
/*****************************************************************************/
/**
* @file mpu.c
*
* This file contains initial configuration of the MPU.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00 	pkp  02/20/14 First release
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

#include "xil_mpu.h"
#include "xil_types.h"
#include "xreg_cortexr5.h"
#include "xparameters.h"

void Init_MPU(void);

void Init_MPU(void)
{

	u32 addr, reg_size, attrib, reg;
	s32 reg_num;

	addr = 0xFFFF0000U;
	reg_size = REGION_64K;
	reg_num = 0;
	attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW;
	Xil_SetAttribute(addr,reg_size,reg_num, attrib);

	addr = 0xFFFC0000U;
	reg_size = REGION_128K;
	reg_num = 1;
	attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW  ;
	Xil_SetAttribute(addr,reg_size,reg_num, attrib);

	addr = 0xFFFE0000U;
	reg_size = REGION_64K;
	reg_num = 2;
	attrib = NORM_NSHARED_WB_WA| PRIV_RW_USER_RW  ;
	Xil_SetAttribute(addr,reg_size,reg_num, attrib);

	addr = 0xFD000000U;
	reg_size = REGION_4M;
	reg_num = 3;
	attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(addr,reg_size,reg_num, attrib);

	addr = 0xFEC00000U;
	reg_size = REGION_4M;
	reg_num = 4;
	attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(addr,reg_size,reg_num, attrib);


	addr = 0xFF000000U;
	reg_size = REGION_4M;
	reg_num = 5;
	attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(addr,reg_size,reg_num, attrib);


	addr = 0xFF400000U;
	reg_size = REGION_4M;
	reg_num = 6;
	attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(addr,reg_size,reg_num, attrib);


	addr = 0xFF800000U;
	reg_size = REGION_4M;
	reg_num = 7;
	attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(addr,reg_size,reg_num, attrib);

	Xil_EnableBackgroundRegion();
	Xil_EnableMPU();

}
