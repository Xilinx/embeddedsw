/******************************************************************************
*
* Copyright (C) 2008 - 2018 Xilinx, Inc. All rights reserved.
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
/******************************************************************************
* File   : microblaze_disable_dcache.s
* Date   : 2002, March 20.
* Company: Xilinx
* Group  : Emerging Software Technologies
*
* Summary:
* Disable the L1 dcache on the microblaze.
*
*******************************************************************************/

#include "xparameters.h"
#include "microblaze_instructions.h"

	.text
	.globl	microblaze_disable_dcache
	.ent	microblaze_disable_dcache
	.align	2
microblaze_disable_dcache:
#if XPAR_MICROBLAZE_USE_MSR_INSTR == 1

#if XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0
    ADDIK   r1, r1, -8
    SI     r15, r1, 0
    BRLID   r15, microblaze_flush_dcache /* microblaze_flush_dcache does not use r1*/
    nop
    LI     r15, r1, 0
    ADDIK    r1, r1, 8
#endif /* XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0 */
	rtsd	r15, 8
	msrclr r0, 0x80

#else /* XPAR_MICROBLAZE_USE_MSR_INSTR == 1 */

    ADDIK   r1, r1, -8

#if XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0
    SI     r15, r1, 0
    BRLID   r15, microblaze_flush_dcache
    nop
#endif /* XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0 */
    mfs	r11, rmsr
	andi	r11, r11, ~(0x80)
	mts	rmsr, r11

#if XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0
    LI     r15, r1, 0
#endif /* XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0 */

	ADDIK	r1, r1, 8
	rtsd	r15, 8
	nop

#endif /*XPAR_MICROBLAZE_USE_MSR_INSTR == 1*/
        .end	microblaze_disable_dcache
