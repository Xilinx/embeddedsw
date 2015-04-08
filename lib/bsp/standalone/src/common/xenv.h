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
/*****************************************************************************/
/**
*
* @file xenv.h
*
* Defines common services that are typically found in a host operating.
* environment. This include file simply includes an OS specific file based
* on the compile-time constant BUILD_ENV_*, where * is the name of the target
* environment.
*
* All services are defined as macros.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b ch   10/24/02 Added XENV_LINUX
* 1.00a rmm  04/17/02 First release
* </pre>
*
******************************************************************************/

#ifndef XENV_H /* prevent circular inclusions */
#define XENV_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Select which target environment we are operating under
 */

/* VxWorks target environment */
#if defined XENV_VXWORKS
#include "xenv_vxworks.h"

/* Linux target environment */
#elif defined XENV_LINUX
#include "xenv_linux.h"

/* Unit test environment */
#elif defined XENV_UNITTEST
#include "ut_xenv.h"

/* Integration test environment */
#elif defined XENV_INTTEST
#include "int_xenv.h"

/* Standalone environment selected */
#else
#include "xenv_standalone.h"
#endif


/*
 * The following comments specify the types and macro wrappers that are
 * expected to be defined by the target specific header files
 */

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
 *
 * XENV_TIME_STAMP
 *
 * A structure that contains a time stamp used by other time stamp macros
 * defined below. This structure is processor dependent.
 */


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * XENV_MEM_COPY(void *DestPtr, void *SrcPtr, unsigned Bytes)
 *
 * Copies a non-overlapping block of memory.
 *
 * @param   DestPtr is the destination address to copy data to.
 * @param   SrcPtr is the source address to copy data from.
 * @param   Bytes is the number of bytes to copy.
 *
 * @return  None
 */

/*****************************************************************************/
/**
 *
 * XENV_MEM_FILL(void *DestPtr, char Data, unsigned Bytes)
 *
 * Fills an area of memory with constant data.
 *
 * @param   DestPtr is the destination address to set.
 * @param   Data contains the value to set.
 * @param   Bytes is the number of bytes to set.
 *
 * @return  None
 */
/*****************************************************************************/
/**
 *
 * XENV_TIME_STAMP_GET(XTIME_STAMP *StampPtr)
 *
 * Samples the processor's or external timer's time base counter.
 *
 * @param   StampPtr is the storage for the retrieved time stamp.
 *
 * @return  None
 */

/*****************************************************************************/
/**
 *
 * XENV_TIME_STAMP_DELTA_US(XTIME_STAMP *Stamp1Ptr, XTIME_STAMP* Stamp2Ptr)
 *
 * Computes the delta between the two time stamps.
 *
 * @param   Stamp1Ptr - First sampled time stamp.
 * @param   Stamp1Ptr - Sedond sampled time stamp.
 *
 * @return  An unsigned int value with units of microseconds.
 */

/*****************************************************************************/
/**
 *
 * XENV_TIME_STAMP_DELTA_MS(XTIME_STAMP *Stamp1Ptr, XTIME_STAMP* Stamp2Ptr)
 *
 * Computes the delta between the two time stamps.
 *
 * @param   Stamp1Ptr - First sampled time stamp.
 * @param   Stamp1Ptr - Sedond sampled time stamp.
 *
 * @return  An unsigned int value with units of milliseconds.
 */

/*****************************************************************************//**
 *
 * XENV_USLEEP(unsigned delay)
 *
 * Delay the specified number of microseconds.
 *
 * @param   delay is the number of microseconds to delay.
 *
 * @return  None
 */

#ifdef __cplusplus
}
#endif

#endif            /* end of protection macro */

