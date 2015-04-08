/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc. All rights reserved.
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

	.globl _mcount

	#define _MCOUNT_STACK_FRAME 48
	.section .text
	.align 2
	.type _mcount@function


_mcount:
	stwu 1,	-_MCOUNT_STACK_FRAME(1)
	stw 3, 8(1)
	stw 4, 12(1)
	stw 5, 16(1)
	stw 6, 20(1)
	stw 7, 24(1)
	stw 8, 28(1)
	stw 9, 32(1)
	stw 10, 36(1)
	stw 11, 40(1)
	stw 12, 44(1)
	mflr 4
	stw 4, (_MCOUNT_STACK_FRAME+4)(1)
	lwz 3, (_MCOUNT_STACK_FRAME)(1)
	lwz 3, 4(3)
	bl mcount
	lwz 4, (_MCOUNT_STACK_FRAME+4)(1)
	mtlr 4
	lwz 12, 44(1)
	lwz 11, 40(1)
	lwz 10, 36(1)
	lwz 9, 32(1)
	lwz 8, 28(1)
	lwz 7, 24(1)
	lwz 6, 20(1)
	lwz 5, 16(1)
	lwz 4, 12(1)
	lwz 3, 8(1)
	addi 1,1, _MCOUNT_STACK_FRAME
	blr
