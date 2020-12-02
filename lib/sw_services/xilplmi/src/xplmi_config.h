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
* 1.01  kc   07/16/2019 Make DEBUG_GENERAL as default to reduce print log
*       kc   07/16/2019 Added PERF macro to print task times
* 1.02  bsv  10/31/2019 Added macro to exclude USB
*       kc   02/26/2020 Added macro to exclude SEM
*       bsv  03/09/2020 Added DEBUG MODE to PLM
* 1.03  bsv  04/04/2020 Code clean up
* 1.04  kc   01/07/2020 Added MACRO to get performance number for keyhole
* 1.05  rama 08/12/2020 Added macro to exclude STL by default
*       bm   10/14/2020 Code clean up
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_CONFIG_H
#define XPLMI_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

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
/**
 * Enable the below define to disable prints from UART.
 * Prints to memory are still enabled as defined by PLM DEBUG macros
 */
//#define PLM_PRINT_NO_UART

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
 * PL prints the PL Power status and House clean status.
 * Make sure to enable PLM_PRINT_PERF to see prints.
 */
//#define PLM_PRINT_PERF_POLL
//#define PLM_PRINT_PERF_DMA
//#define PLM_PRINT_PERF_CDO_PROCESS
//#define PLM_PRINT_PERF_KEYHOLE
//#define PLM_PRINT_PERF_PL

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
 * @name PLM DEBUG MODE options
 *
 * By default, PLM would get built in release mode, which implies any
 * error during boot pdi load would result in SRST. User has the options
 * to enable the below macro to enable debug mode, which would make the system hang
 * in case of any error for the user to debug further.
 *
 */
//#define PLM_DEBUG_MODE
/**
 * @name PLM DEBUG MODE options
 *
 * By default, STL is not enabled in PLM
 * Users will be given an option to enable this
 *
 */
//#define PLM_ENABLE_STL
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_CONFIG_H */
