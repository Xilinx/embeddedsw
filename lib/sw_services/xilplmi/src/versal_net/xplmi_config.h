/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xplmi_config.h
*
* This is the header file which contains versal_net PLM configuration for users.
* All key configuration options are now made available through Vitis GUI and
* xsct command line. Hence, the corresponding defines now appear in
* xparameters.h file. However, references for those configurations are retained
* here in comments, for users who prefer configuring by updating sources.
* Please refer xparameters.h file (under comment "PLM/XilPLMI configuration")
* for the Vitis/tool generated definitions, while building, before attempting
* to edit definitions in this file
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       dc   07/17/2022 Added PLM_OCP configuration
* 1.01  ng   11/11/2022 Fixed doxygen file name error
* 1.02  ng   06/21/2023 Added support for system device-tree flow
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
#include "xparameters.h"

#ifdef SDT
#include "xplmi_bsp_config.h"
#endif

/**@cond xplmi_internal
 * @{
 */

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
 * Please note that below are defined in xparameters.h based on the
 * xilplmi library configuration, hence commented out here.
 * PLM_DEBUG is defined by default.
 */
//#define PLM_PRINT
//#define PLM_DEBUG
//#define PLM_DEBUG_INFO
//#define PLM_DEBUG_DETAILED

/**
 * Enable the below define to disable prints from UART.
 * Prints to memory are still enabled as defined by PLM DEBUG macros
 * Please note that below is defined in xparameters.h based on the
 * xilplmi library configuration, hence commented out here.
 * This definition is disabled by default (i.e. not defined).
 */
//#define PLM_PRINT_NO_UART

/**
 * Enabling the PLM_PRINT_PERF prints the time taken for loading partitions,
 * images and tasks. This define can be enabled with any of the above
 * PLM debug options to print the timings.
 * Please note that below is defined in xparameters.h based on the
 * xilplmi library configuration, hence commented out here.
 * This definition is enabled by default.
 */
//#define PLM_PRINT_PERF

/**
 * @name PLM code include options
 *
 *  PLM by default includes all the code except USB code and NVM code.
 *  Unwanted code can be excluded from the elf by defining here
 *  Below blocks can be excluded from the code.
 *	- PLM_QSPI_EXCLUDE QSPI code will be excluded (included by default)
 *	- PLM_SD_EXCLUDE SD code will be excluded (included by default)
 *	- PLM_OSPI_EXCLUDE OSPI code will be excluded (included by default)
 *	- PLM_USB_EXCLUDE USB code will be excluded (excluded by default)
 *	- PLM_SEM_EXCLUDE SEM code will be excluded (included by default)
 *	- PLM_SECURE_EXCLUDE secure code will be excluded (included by default)
 *	- PLM_NVM_EXCLUDE NVM handlers will be excluded (excluded by default)
 *	- PLM_PUF_EXCLUDE PUF handlers will be excluded (excluded by default)
 *
 * Please note that below are defined in xparameters.h based on the
 * xilplmi library configuration, hence all the below are commented out here.
 */
//#define PLM_QSPI_EXCLUDE
//#define PLM_SD_EXCLUDE
//#define PLM_OSPI_EXCLUDE
//#define PLM_USB_EXCLUDE
//#define PLM_SEM_EXCLUDE
//#define PLM_SECURE_EXCLUDE
//#define PLM_NVM_EXCLUDE
//#define PLM_PUF_EXCLUDE

#if (!defined(PLM_NVM_EXCLUDE)) && (!defined(PLM_NVM))
#define PLM_NVM
#endif

#if (!defined(PLM_PUF_EXCLUDE)) && (!defined(PLM_PUF))
#define PLM_PUF
#endif

#if (!defined(PLM_OCP_EXCLUDE)) && (!defined(PLM_OCP))
#define PLM_OCP
#endif

/**
 * @name PLM DEBUG MODE options
 *
 * By default, PLM would get built in release mode, which implies any
 * error during boot pdi load would result in SRST. User has the options
 * to enable the below macro to enable debug mode, which would make the system hang
 * in case of any error for the user to debug further.
 * Please note that below is defined in xparameters.h based on the
 * xilplmi library configuration, hence commented out here.
 * This definition is disabled by default (i.e. not defined).
 */
//#define PLM_DEBUG_MODE

/**
 * NOTE: ALL the configurations below this line can only be done in this file
 *       and NOT through xilplmi library configuration through xparameters.h
 */

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

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/**
 * @}
 * @endcond
 */


#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_CONFIG_H */
