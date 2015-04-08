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

#include "profile.h"
#include "_profile_timer_hw.h"

#ifdef PROC_MICROBLAZE
#include "mblaze_nt_types.h"
#endif

#ifdef PROC_PPC
#include "xpseudo_asm.h"
#define SPR_SRR0 0x01A
#endif

#include "xil_types.h"

extern u32 binsize ;
u32 prof_pc ;

void profile_intr_handler( void )
{

	s32 j;

#ifdef PROC_MICROBLAZE
	asm( "swi r14, r0, prof_pc" ) ;
#elif defined PROC_PPC
	prof_pc = mfspr(SPR_SRR0);
#else
	/* for cortexa9, lr is saved in asm interrupt handler */
#endif
	/* print("PC: "), putnum(prof_pc), print("\r\n"), */
	for(j = 0; j < n_gmon_sections; j++ ){
		if((prof_pc >= ((u32)_gmonparam[j].lowpc)) && (prof_pc < ((u32)_gmonparam[j].highpc))) {
			_gmonparam[j].kcount[(prof_pc-_gmonparam[j].lowpc)/((u32)4 * binsize)]++;
			break;
		}
	}
	/* Ack the Timer Interrupt */
	timer_ack();
}

