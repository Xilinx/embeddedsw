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
******************************************************************************
*
* _program_init.c:
*	Initialize the Profiling Structures.
*
******************************************************************************/

#include "profile.h"

/* XMD Initializes the following Global Variables Value during Program
 *  Download with appropriate values. */

#ifdef PROC_MICROBLAZE

extern s32 microblaze_init(void);

#elif defined PROC_PPC

extern s32 powerpc405_init(void);

#else

extern s32 cortexa9_init(void);

#endif

s32 profile_version = 1;	/* Version of S/W Intrusive Profiling library */

u32 binsize = (u32)BINSIZE;    			/* Histogram Bin Size */
u32 cpu_clk_freq = (u32)CPU_FREQ_HZ ;	/* CPU Clock Frequency */
u32 sample_freq_hz = (u32)SAMPLE_FREQ_HZ ;	/* Histogram Sampling Frequency */
u32 timer_clk_ticks = (u32)TIMER_CLK_TICKS ;/* Timer Clock Ticks for the Timer */

/* Structure for Storing the Profiling Data */
struct gmonparam *_gmonparam = (struct gmonparam *)(0xffffffffU);
s32 n_gmon_sections = 1;

/* This is the initialization code, which is called from the crtinit. */

void _profile_init( void )
{
/* 	print("Gmon Init called....\r\n")  */
/* 	putnum(n_gmon_sections) , print("\r\n")   */
/* 	if( _gmonparam == 0xffffffff ) */
/* 		printf("Gmonparam is NULL !!\r\n")  */
/* 	for( i = 0, i < n_gmon_sections, i++ )[ */
/* 		putnum( _gmonparam[i].lowpc) , print("\t")   */
/* 		putnum( _gmonparam[i].highpc) , print("\r\n")  */
/* 		putnum( _gmonparam[i].textsize ), print("\r\n")  */
/* 		putnum( _gmonparam[i].kcountsize * sizeof(unsigned short)), print("\r\n")  */
/* 	] */

#ifdef PROC_MICROBLAZE
	(void)microblaze_init();
#elif defined PROC_PPC
	powerpc405_init();
#else
	(void)cortexa9_init();
#endif
}

