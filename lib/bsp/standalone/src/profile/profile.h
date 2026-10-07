/******************************************************************************
* Copyright (c) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file profile.h
* @brief Profiling support interface for standalone BSP.
*/



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

/** @brief Initialise the profiling system. */
void _system_init( void ) ;
/** @brief Clean up profiling resources. */
void _system_clean( void ) ;
/**
 * @brief Call-graph profiling hook called on every function entry.
 * @param[in] frompc Program counter of the calling (parent) function.
 * @param[in] selfpc Program counter of the called (child) function.
 */
void mcount(u32 frompc, u32 selfpc);
/** @brief Profiling interrupt handler; samples program counter for histogram. */
void profile_intr_handler( void ) ;
/** @brief Initialize the profiling subsystem and allocate profiling buffers. */
void _profile_init( void );



/****************************************************************************
 * Profiling on hardware - Hash table maintained on hardware and data sent
 * to xmd for gmon.out generation.
 ****************************************************************************/
/*
 * histogram counters are unsigned shorts (according to the kernel).
 */
#define	HISTCOUNTER	u16 /**< Histogram counter type (16-bit unsigned integer). */

/** @brief Call-graph arc destination structure. */
struct tostruct {
	u32  selfpc; /**< PC value of the callee function. */
	s32	 count; /**< Number of times this arc was traversed. */
	s16  link;  /**< Index to next tostruct in hash chain. */
	u16	 pad;   /**< Padding for alignment. */
};

/** @brief Call-graph arc source structure. */
struct fromstruct {
	u32 frompc ; /**< PC value of the caller function. */
	s16 link ;   /**< Index to next fromstruct in hash chain. */
	u16 pad ;    /**< Padding for alignment. */
} ;

/*
 * general rounding functions.
 */
#define ROUNDDOWN(x,y)	(((x)/(y))*(y)) /**< Round x down to the nearest multiple of y. */
#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y)) /**< Round x up to the nearest multiple of y. */

/**
 * @brief Profiling data structure housing all gmon parameters.
 */
struct gmonparam {
	s32		state; /**< Current profiling state (GMON_PROF_ON/OFF/etc.). */

	/* Histogram Information */
	u16		*kcount;    /**< Histogram bin counter array. */
	u32		kcountsize; /**< Number of histogram samples (size of kcount). */

	/* Call-graph Information */
	struct fromstruct	*froms;   /**< Array of call-graph source arcs. */
	u32		fromssize;            /**< Size of froms array in bytes. */
	struct tostruct		*tos;     /**< Array of call-graph destination arcs. */
	u32		tossize;              /**< Size of tos array in bytes. */

	/* Initialization I/Ps */
	u32    	lowpc;    /**< Low end of the text region being profiled. */
	u32		highpc;   /**< High end of the text region being profiled. */
	u32		textsize; /**< Size of the text region being profiled. */
	/* u32 		cg_froms, */
	/* u32 		cg_tos, */
};
extern struct gmonparam *_gmonparam; /**< Pointer to gmon profiling parameters. */
extern s32 n_gmon_sections;          /**< Number of gmon sections being profiled. */

/*
 * Possible states of profiling.
 */
#define	GMON_PROF_ON	0 /**< Profiling is active. */
#define	GMON_PROF_BUSY	1 /**< Profiling is busy (cannot be interrupted). */
#define	GMON_PROF_ERROR	2 /**< Profiling encountered an error. */
#define	GMON_PROF_OFF	3 /**< Profiling is disabled. */

/*
 * Sysctl definitions for extracting profiling information from the kernel.
 */
#define	GPROF_STATE	0	/**< Sysctl index: profiling enabling variable. */
#define	GPROF_COUNT	1	/**< Sysctl index: profile tick count buffer. */
#define	GPROF_FROMS	2	/**< Sysctl index: from location hash bucket. */
#define	GPROF_TOS	3	/**< Sysctl index: destination/count structure. */
#define	GPROF_GMONPARAM	4	/**< Sysctl index: profiling parameters. */

#ifdef __cplusplus
}
#endif

#endif 		/* PROFILE_H */
