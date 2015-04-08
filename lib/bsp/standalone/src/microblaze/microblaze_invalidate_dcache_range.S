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
*
*
* microblaze_invalidate_dcache_range (unsigned int cacheaddr, unsigned int len)
*
*    Invalidate a Dcache range
*
*    Parameters:
*       'cacheaddr' - address in the Dcache where invalidation begins
*	'len    '   - length (in bytes) worth of Dcache to be invalidated
*
*
*******************************************************************************/

#include "xparameters.h"

#define MICROBLAZE_MSR_DCACHE_ENABLE        0x00000080
#define MICROBLAZE_MSR_INTR_ENABLE          0x00000002

#ifndef XPAR_MICROBLAZE_DCACHE_LINE_LEN
#define XPAR_MICROBLAZE_DCACHE_LINE_LEN     1
#endif

#ifndef XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK
#define MB_VERSION_LT_v720
#define MB_HAS_WRITEBACK_SET 0
#else
#define MB_HAS_WRITEBACK_SET XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK
#endif

	.text
	.globl	microblaze_invalidate_dcache_range
	.ent	microblaze_invalidate_dcache_range
	.align	2

microblaze_invalidate_dcache_range:


#ifdef MB_VERSION_LT_v720                                       /* Disable Dcache and interrupts before invalidating */
	mfs	r9, rmsr
	andi	r10, r9, ~(MICROBLAZE_MSR_DCACHE_ENABLE | MICROBLAZE_MSR_INTR_ENABLE)
	mts	rmsr, r10
#endif

        beqi    r6, L_done                                      /* Skip loop if size is zero */

        add	r6, r5, r6                                      /* Compute end address */
        addik   r6, r6, -1

        andi    r6, r6, -(4 * XPAR_MICROBLAZE_DCACHE_LINE_LEN)  /* Align end down to cache line */
        andi    r5, r5, -(4 * XPAR_MICROBLAZE_DCACHE_LINE_LEN)  /* Align start down to cache line */

#if MB_HAS_WRITEBACK_SET == 0                                   /* Use a different scheme for MB version < v7.20 or when caches are write-through */

L_start:
	cmpu	r18, r5, r6                                     /* Are we at the end? */
	blti	r18, L_done

	wdc     r5, r0

	brid	L_start                                         /* Branch to the beginning of the loop */
	addik	r5, r5, (XPAR_MICROBLAZE_DCACHE_LINE_LEN * 4)   /* Increment the addrees by 4 (delay slot) */
#else

        rsubk   r6, r5, r6
                                                                /* r6 will now contain (count of bytes - (4 * XPAR_MICROBLAZE_DCACHE_LINE_LEN)) */
L_start:
        wdc.clear r5, r6                                        /* Invalidate the cache line only if the address matches */
        bneid   r6, L_start
        addik   r6, r6, -(XPAR_MICROBLAZE_DCACHE_LINE_LEN * 4)

#endif

L_done:
	rtsd	r15, 8
#ifdef MB_VERSION_LT_v720                                       /* restore MSR only for MB version < v7.20 */
	mts	rmsr, r9
#else
        nop
#endif
	.end	microblaze_invalidate_dcache_range

