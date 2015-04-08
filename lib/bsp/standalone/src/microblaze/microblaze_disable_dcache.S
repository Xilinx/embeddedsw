/******************************************************************************
*
* Copyright (C) 2008 - 2014 Xilinx, Inc. All rights reserved.
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

	.text
	.globl	microblaze_disable_dcache
	.ent	microblaze_disable_dcache
	.align	2
microblaze_disable_dcache:
#if XPAR_MICROBLAZE_USE_MSR_INSTR == 1

#if XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0
    addik   r1, r1, -4
    swi     r15, r1, 0
    brlid   r15, microblaze_flush_dcache /* microblaze_flush_dcache does not use r1*/
    nop
    lwi     r15, r1, 0
    addi    r1, r1, 4
#endif /* XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0 */
	rtsd	r15, 8
	msrclr r0, 0x80

#else /* XPAR_MICROBLAZE_USE_MSR_INSTR == 1 */

    addik   r1, r1, -4

#if XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0
    swi     r15, r1, 0
    brlid   r15, microblaze_flush_dcache
    nop
#endif /* XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0 */
    mfs	r11, rmsr
	andi	r11, r11, ~(0x80)
	mts	rmsr, r11

#if XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0
    lwi     r15, r1, 0
#endif /* XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK != 0 */

	rtsd	r15, 8
	addi	r1, r1, 4

#endif /*XPAR_MICROBLAZE_USE_MSR_INSTR == 1*/
        .end	microblaze_disable_dcache
