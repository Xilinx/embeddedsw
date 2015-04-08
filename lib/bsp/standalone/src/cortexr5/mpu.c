/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
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
