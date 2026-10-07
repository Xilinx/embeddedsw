/******************************************************************************
* Copyright (c) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file _profile_init.c
* @brief Profiling initialization for standalone BSP.
*/



#include "profile.h"

/* XMD Initializes the following Global Variables Value during Program
 *  Download with appropriate values. */

#ifdef PROC_MICROBLAZE

extern s32 microblaze_init(void);

#elif defined PROC_PPC

extern s32 powerpc405_init(void);

#else

/**
 * @brief Initialise the Cortex-A9 SCU timer and GIC for profiling.
 * @return 0 on success.
 */
extern s32 cortexa9_init(void);

#endif

s32 profile_version = 1;	/**< Version of S/W Intrusive Profiling library. */

u32 binsize = (u32)BINSIZE;    			/**< Histogram bin size. */
u32 cpu_clk_freq = (u32)CPU_FREQ_HZ ;		/**< CPU clock frequency in Hz. */
u32 sample_freq_hz = (u32)SAMPLE_FREQ_HZ ;	/**< Histogram sampling frequency in Hz. */
u32 timer_clk_ticks = (u32)TIMER_CLK_TICKS ;	/**< Timer clock ticks for the profiling timer. */

/* Structure for Storing the Profiling Data */
struct gmonparam *_gmonparam = (struct gmonparam *)(0xffffffffU); /**< Pointer to gmon profiling parameters. */
s32 n_gmon_sections = 1;	/**< Number of gmon sections being profiled. */

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
