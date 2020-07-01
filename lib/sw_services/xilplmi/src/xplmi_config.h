/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.01  kc   01/07/2020 Added MACRO to get performance number for keyhole
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
 * Enable the below defines as per the requirement.
 * POLL prints the time taken for any poll for MASK_POLL command.
 * DMA prints the time taken for PMC DMA, QSPI, OSPI.
 * CDO_PROCESS will print the time taken to process CDO file.
 * KEYHOLE will print the time taken to process keyhole command.
 * Keyhole command is used for Cframe and slave slr image loading.
 * Make sure to enable PLM_PRINT_PERF to see prints.
 */
//#define PLM_PRINT_PERF_POLL
//#define PLM_PRINT_PERF_DMA
//#define PLM_PRINT_PERF_CDO_PROCESS
//#define PLM_PRINT_PERF_KEYHOLE

/**
 * @name PLM code include options
 *
 *  PLM by default all the code is included.
 *  Unwanted code can be excluded from the elf by defining here
 *  Below blocks can be excluded from the code.
 *		- PLM_QSPI_EXCLUDE QSPI code will be excluded
 *		- PLM_SD_EXCLUDE SD code will be excluded
 *		- PLM_SEM_EXCLUDE SEM code will be excluded
 */
//#define PLM_QSPI_EXCLUDE
//#define PLM_SD_EXCLUDE
//#define PLM_OSPI_EXCLUDE
//#define PLM_USB_EXCLUDE
//#define PLM_SEM_EXCLUDE
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
/**
 * @name PLM DEBUG MODE options
 *
 * By default, PLM would get built in release mode, which implies any
 * error during boot pdi load would result in SRST. User has the options
 * to enable the below macro to enable debug mode, which would make the system hang
 * in case of any error for the user to debug further.
 *
 */
//#define PLM_DEBUG_MODE
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_CONFIG_H */
