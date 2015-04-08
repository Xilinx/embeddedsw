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
* @file translation_table.s
*
* This file contains the initialization for the MMU table in RAM
* needed by the Cortex A53 processor
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00  pkp  05/21/14 Initial version
*
*
* @note
*
* None.
*
******************************************************************************/
	.globl  MMUTableL0
	.globl  MMUTableL1
	.globl  MMUTableL2

	.set reserved,	0x0 					/* Fault*/
	.set Memory,	0x405 | (3 << 8) | (0x0)		/* normal writeback write allocate inner shared read write */
	.set Device,	0x409 | (1 << 53)| (1 << 54) |(0x0)	/* strongly ordered read write non executable*/
	.section .mmu_tbl0,"a"

MMUTableL0:

.set SECT, MMUTableL1
.8byte	SECT + 0x3
.set SECT, MMUTableL1+0x1000
.8byte	SECT + 0x3

	.section .mmu_tbl1,"a"

MMUTableL1:

.set SECT, MMUTableL2			/*1GB DDR*/
.8byte	SECT + 0x3

.rept	0x3				/*1GB DDR, 1GB PL, 2GB other devices n memory*/
.set SECT, SECT + 0x1000
.8byte	SECT + 0x3
.endr

.set SECT,0x100000000
.rept	0xC
.8byte	SECT + reserved
.set SECT, SECT + 0x40000000	/*12GB Reserved*/
.endr

.rept	0x10
.8byte	SECT + Device
.set SECT, SECT + 0x40000000	/*8GB PL, 8GB PCIe*/

.endr

.rept	0x20
.8byte	SECT + Memory

.set SECT, SECT + 0x40000000	/*32GB DDR*/
.endr


.rept	0xC0
.8byte	SECT + Device
.set SECT, SECT + 0x40000000	/*192GB PL*/
.endr


.rept	0x100
.8byte	SECT + Device
.set SECT, SECT + 0x40000000	/*256GB PL/PCIe*/
.endr


.rept	0x200
.8byte	SECT + Device
.set SECT, SECT + 0x40000000	/*512GB PL/DDR*/
.endr


.section .mmu_tbl2,"a"

MMUTableL2:

.set SECT, 0

.rept	0x0400			/*2GB DDR */
.8byte	SECT + Memory
.set	SECT, SECT+0x200000
.endr

.rept	0x0200			/*1GB lower PL*/
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr
.rept	0x0100			/*512MB QSPI*/
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr
.rept	0x080			/*256MB lower PCIe*/
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr
.rept	0x040			/*128MB Reserved*/
.8byte	SECT + reserved
.set	SECT, SECT+0x200000
.endr
.rept	0x8			/*16MB coresight*/
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr
.rept	0x8			/*16MB RPU low latency port*/
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr

.rept	0x022			/*68MB Device*/
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr
.rept	0x8			/*8MB FPS*/
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr

.rept	0x4			/*16MB LPS*/
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr

.8byte	SECT + Device 		/*2MB PMU/CSU */
.set	SECT, SECT+0x200000
.8byte  SECT + Memory		/*2MB OCM/TCM*/
.end
