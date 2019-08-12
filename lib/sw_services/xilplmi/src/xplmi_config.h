/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_config.h
*
* This is the header file which contains PLM configuration
* for users.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_CONFIG_H
#define XPLM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
//#undef STDOUT_BASEADDRESS
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name PLM Debug options
 *
 *  PLM supports an unconditional print
 *     - PLM_PRINT Used to print PLM header and any mandatory prints
 *       Hence PLM_PRINT_VAL should always be 1
 *  Further PLM by default doesn't have any debug prints enabled. If user
 *  want to enable the debug prints, they can define the following
 *  options
 *  PLM supports three types of debug levels.
 *     - PLM_DEBUG Defining this will print basic information and
 *       error prints if any
 *     - PLM_DEBUG_INFO Defining this will have prints enabled with format
 *       specifiers in addition to the basic information
 *     - PLM_DEBUG_DETAILED Defining this will print information with
 *       all data exchanged.
 */
/**
 * PLM Debug options
 */

//#define PLM_PRINT
#define PLM_DEBUG
//#define PLM_DEBUG_INFO
//#define PLM_DEBUG_DETAILED

/**
 * Enabling the PLM_PRINT_PERF prints the time taken for loading partitions,
 * images and tasks. This define can be enabled with any of the above
 * debug defines to print the timings.
 */
#define PLM_PRINT_PERF

/**
 * @name PLM code include options
 *
 *  PLM by default all the code is included.
 *  Unwanted code can be excluded from the elf by defining here
 *  Below blocks can be excluded from the code.
 *     - PLM_QSPI_EXCLUDE QSPI code will be excluded
 *     - PLM_SD_EXCLUDE SD code will be excluded
 */
//#define PLM_QSPI_EXCLUDE
//#define PLM_SD_EXCLUDE

/**
 * @name PLM Error management options
 *
 *  PLM by default for any error during full PDI loading, does a fallback.
 *  Users will be given an option to disable fallback so that they can debug
 *  the error.
 *  	- PLM_ERR_FALLBACK (default)
 *  	- PLM_ERR_DISABLE_FALLBACK
 *
 *  TODO Check if we can classify critical and non critical errors at boot
 *
 *  Post Boot:
 */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_CONFIG_H */
