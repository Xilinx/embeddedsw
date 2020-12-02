/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_config.h
*
* This is the header file which contains FSBL configuration
* for users.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   vns  03/24/17 Removed READ_BUFFER_SIZE from configuration
*                     Added FSBL_PL_CLEAR_EXCLUDE_VAL, FSBL_USB_EXCLUDE_VAL,
*                     FSBL_PROT_BYPASS_EXCLUDE_VAL configurations
* 3.0   vns  03/07/18 Added FSBL_FORCE_ENC_EXCLUDE_VAL configuration
*
*</pre>
*
* @note
*
******************************************************************************/
#ifndef XFSBL_CONFIG_H
#define XFSBL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* This is the address in DDR where bitstream will be copied temporarily */
#define XFSBL_DDR_TEMP_ADDRESS			(0x100000U)

/* This is the address in DDR where boot.bin will be copied in USB boot mode */
#define XFSBL_DDR_TEMP_BUFFER_ADDRESS			(0x4000000U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name FSBL Debug options
 *
 *  FSBL supports an unconditional print
 *     - FSBL_PRINT Used to print FSBL header and any mandatory prints
 *       Hence FSBL_PRINT_VAL should always be 1
 *  Further FSBL by default doesn't have any debug prints enabled. If user
 *  want to enable the debug prints, they can define the following
 *  options
 *  FSBL supports three types of debug levels.
 *     - FSBL_DEBUG Defining this will print basic information and
 *       error prints if any
 *     - FSBL_DEBUG_INFO Defining this will have prints enabled with format
 *       specifiers in addition to the basic information
 *     - FSBL_DEBUG_DETAILED Defining this will print information with
 *       all data exchanged.
 */
#define FSBL_PRINT_VAL              (1U)
#define FSBL_DEBUG_VAL              (0U)
#define FSBL_DEBUG_INFO_VAL         (0U)
#define FSBL_DEBUG_DETAILED_VAL     (0U)

/**
 * FSBL Debug options
 */

#if FSBL_PRINT_VAL
#define FSBL_PRINT
#endif

#if FSBL_DEBUG_VAL
#define FSBL_DEBUG
#endif

#if FSBL_DEBUG_INFO_VAL
#define FSBL_DEBUG_INFO
#endif

#if FSBL_DEBUG_DETAILED_VAL
#define FSBL_DEBUG_DETAILED
#endif

/**
 * @name FSBL code include options
 *
 *  FSBL by default all the code is included.
 *  Unwanted code can be excluded from the elf by defining here
 *  Below blocks can be excluded from the code.
 *     - FSBL_NAND_EXCLUDE NAND code will be excluded
 *     - FSBL_QSPI_EXCLUDE QSPI code will be excluded
 *     - FSBL_SD_EXCLUDE SD code will be excluded
 *     - FSBL_SECURE_EXCLUDE Secure features
 *     			(authentication, decryption, checksum) will be excluded
 *     - FSBL_BS_EXCLUDE PL bitstream code will be excluded
 *     - FSBL_EARLY_HANDOFF_EXCLUDE Early handoff related code will be excluded
 *     - FSBL_WDT_EXCLUDE WDT code will be excluded
 *     - FSBL_PERF_EXCLUDE_VAL Performance prints are excluded
 *     - FSBL_A53_TCM_ECC_EXCLUDE_VAL TCM ECC Init will be excluded for A53
 *     - FSBL_PL_CLEAR_EXCLUDE_VAL PL clear will be excluded unless boot.bin
 *     	 contains bitstream
 *     - FSBL_FORCE_ENC_EXCLUDE_VAL Forcing encryption for every partition
 *       when ENC only bit is blown will be excluded.
 */
#define FSBL_NAND_EXCLUDE_VAL			(0U)
#define FSBL_QSPI_EXCLUDE_VAL			(0U)
#define FSBL_SD_EXCLUDE_VAL			(0U)
#define FSBL_SECURE_EXCLUDE_VAL			(0U)
#define FSBL_BS_EXCLUDE_VAL				(0U)
#define FSBL_EARLY_HANDOFF_EXCLUDE_VAL	(1U)
#define FSBL_WDT_EXCLUDE_VAL			(0U)
#define FSBL_PERF_EXCLUDE_VAL			(1U)
#define FSBL_A53_TCM_ECC_EXCLUDE_VAL	(1U)
#define FSBL_PL_CLEAR_EXCLUDE_VAL		(1U)
#define FSBL_USB_EXCLUDE_VAL			(1U)
#define FSBL_PROT_BYPASS_EXCLUDE_VAL	(1U)
#define FSBL_PARTITION_LOAD_EXCLUDE_VAL (0U)
#define FSBL_FORCE_ENC_EXCLUDE_VAL		(0U)
#define FSBL_DDR_SR_EXCLUDE_VAL			(1U)

#if FSBL_NAND_EXCLUDE_VAL
#define FSBL_NAND_EXCLUDE
#endif

#if FSBL_QSPI_EXCLUDE_VAL
#define FSBL_QSPI_EXCLUDE
#endif

#if FSBL_SD_EXCLUDE_VAL
#define FSBL_SD_EXCLUDE
#endif

#if FSBL_SECURE_EXCLUDE_VAL
#define FSBL_SECURE_EXCLUDE
#endif

#if FSBL_BS_EXCLUDE_VAL
#define FSBL_BS_EXCLUDE
#endif

#if FSBL_EARLY_HANDOFF_EXCLUDE_VAL
#define FSBL_EARLY_HANDOFF_EXCLUDE
#endif

#if FSBL_WDT_EXCLUDE_VAL
#define FSBL_WDT_EXCLUDE
#endif

#if FSBL_PERF_EXCLUDE_VAL
#define FSBL_PERF_EXCLUDE
#endif

#if FSBL_A53_TCM_ECC_EXCLUDE_VAL
#define FSBL_A53_TCM_ECC_EXCLUDE
#endif

#if FSBL_PL_CLEAR_EXCLUDE_VAL
#define FSBL_PL_CLEAR_EXCLUDE
#endif

#if FSBL_USB_EXCLUDE_VAL
#define FSBL_USB_EXCLUDE
#endif

#if FSBL_PROT_BYPASS_EXCLUDE_VAL
#define FSBL_PROT_BYPASS_EXCLUDE
#endif

#if FSBL_PARTITION_LOAD_EXCLUDE_VAL
#define FSBL_PARTITION_LOAD_EXCLUDE
#endif

#if FSBL_FORCE_ENC_EXCLUDE_VAL
#define FSBL_FORCE_ENC_EXCLUDE
#endif

#if (FSBL_DDR_SR_EXCLUDE_VAL == 0U)
#define XFSBL_ENABLE_DDR_SR
#endif
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_CONFIG_H */
