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

#ifndef	PROFILE_H
#define	PROFILE_H	1

#include <stdio.h>
#include "xil_types.h"
#include "profile_config.h"

#ifdef PROC_MICROBLAZE
#include "mblaze_nt_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void _system_init( void ) ;
void _system_clean( void ) ;
void mcount(u32 frompc, u32 selfpc);
void profile_intr_handler( void ) ;
void _profile_init( void );



/****************************************************************************
 * Profiling on hardware - Hash table maintained on hardware and data sent
 * to xmd for gmon.out generation.
 ****************************************************************************/
/*
 * histogram counters are unsigned shorts (according to the kernel).
 */
#define	HISTCOUNTER	u16

struct tostruct {
	u32  selfpc;
	s32	 count;
	s16  link;
	u16	 pad;
};

struct fromstruct {
	u32 frompc ;
	s16 link ;
	u16 pad ;
} ;

/*
 * general rounding functions.
 */
#define ROUNDDOWN(x,y)	(((x)/(y))*(y))
#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y))

/*
 * The profiling data structures are housed in this structure.
 */
struct gmonparam {
	s32		state;

	/* Histogram Information */
	u16		*kcount;	/* No. of bins in histogram */
	u32		kcountsize;	/* Histogram samples */

	/* Call-graph Information */
	struct fromstruct	*froms;
	u32		fromssize;
	struct tostruct		*tos;
	u32		tossize;

	/* Initialization I/Ps */
	u32    	lowpc;
	u32		highpc;
	u32		textsize;
	/* u32 		cg_froms, */
	/* u32 		cg_tos, */
};
extern struct gmonparam *_gmonparam;
extern s32 n_gmon_sections;

/*
 * Possible states of profiling.
 */
#define	GMON_PROF_ON	0
#define	GMON_PROF_BUSY	1
#define	GMON_PROF_ERROR	2
#define	GMON_PROF_OFF	3

/*
 * Sysctl definitions for extracting profiling information from the kernel.
 */
#define	GPROF_STATE	0	/* int: profiling enabling variable */
#define	GPROF_COUNT	1	/* struct: profile tick count buffer */
#define	GPROF_FROMS	2	/* struct: from location hash bucket */
#define	GPROF_TOS	3	/* struct: destination/count structure */
#define	GPROF_GMONPARAM	4	/* struct: profiling parameters (see above) */

#ifdef __cplusplus
}
#endif

#endif 		/* PROFILE_H */

