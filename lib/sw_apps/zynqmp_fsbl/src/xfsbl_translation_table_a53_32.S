/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
/*
* @file xfsbl_translation_table.S
*
* This file contains the initialization for the MMU table in RAM
* needed by the Cortex A53 processor (32-bit)
* This file is FSBL's local copy of the BSP's file
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.2	pkp  28/05/15 First release
* </pre>
*
* @note
*
* None.
*
******************************************************************************/
#include "xparameters.h"
	.globl  MMUTable

	.section .mmu_tbl,"a"

MMUTable:
	/* Each table entry occupies one 32-bit word and there are
	 * 4096 entries, so the entire table takes up 16KB.
	 * Each entry covers a 1MB section.
	 */

.set SECT, 0

#ifdef XPAR_PSU_DDR_0_S_AXI_BASEADDR
.set DDR_START, XPAR_PSU_DDR_0_S_AXI_BASEADDR
.set DDR_END, XPAR_PSU_DDR_0_S_AXI_HIGHADDR
.set DDR_SIZE, (DDR_END - DDR_START)+1
.if DDR_SIZE > 0x80000000
/* If DDR size is larger than 2GB, truncate to 2GB */
.set DDR_REG, 0x800
.else
.set DDR_REG, DDR_SIZE/0x100000
.endif
#else
.set DDR_REG, 0
#endif
.set UNDEF_REG, 0x800 - DDR_REG

.rept	DDR_REG			/* DDR reserved */
.word	SECT + 0x0		/* S=b0 TEX=b000 AP=b00, Domain=b0, C=b0, B=b0 */
.set	SECT, SECT+0x100000
.endr

.rept	UNDEF_REG		/* unassigned/reserved */
				/* Generates a translation fault if accessed */
.word	SECT + 0x0		/* S=b0 TEX=b000 AP=b00, Domain=b0, C=b0, B=b0 */
.set	SECT, SECT+0x100000
.endr

.rept	0x0200			/* 0x80000000 - 0x9fffffff (FPGA slave0) */
.word	SECT + 0xc02		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x0200			/* 0xA0000000 - 0xbfffffff (FPGA slave1) */
.word	SECT + 0xc02		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x0200			/* 0xc0000000 - 0xdfffffff (OSPI IOU)*/
.word	SECT + 0xc06		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x0100			/* 0xe0000000 - 0xefffffff (Lower PCIe)*/
.word	SECT + 0xc06		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x80			/* 0xf0000000 - 0xf7ffffff (unassigned/reserved).
				 * Generates a translation fault if accessed */
.word	SECT + 0x0		/* S=b0 TEX=b000 AP=b00, Domain=b0, C=b0, B=b0 */
.set	SECT, SECT+0x100000
.endr

.rept	0x10			/* 0xf8000000 - 0xf8ffffff (STM Coresight) */
.word	SECT + 0xc06		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x1			/* 0xf9000000 - 0xf90fffff (RPU_A53_GIC) */
.word	SECT + 0xc06		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x3f			/* 0xf9100000 - 0xfcffffff (reserved).*/
.word	SECT + 0x0		/* S=b0 TEX=b000 AP=b00, Domain=b0, C=b0, B=b0 */
.set	SECT, SECT+0x100000
.endr

.rept	0x10			/* 0xfd000000 - 0xfdffffff (FPS Slaves) */
.word	SECT + 0xc06		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x1C			/* 0xfe0000000 - 0xfeffffff (LPS Slaves) */
.word	SECT + 0xc06		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x2			/* 0xffc000000 - 0xffdfffff (CSU and PMU) */
.word	SECT + 0xc06		/* S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr

.rept	0x02			/* 0xffe00000 - 0xffffffff (TCM and OCM Cacheable) */
.word	SECT + 0x15de6		/* S=b1 TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1 */
.set	SECT, SECT+0x100000
.endr
.end
