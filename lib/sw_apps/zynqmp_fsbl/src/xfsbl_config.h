/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
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
* 4.0   ma   02/18/21 Added provision to define macros both using compiler
*                     build flags and this file without redefinition warning
*       bsv  04/01/21 Added TPM support
*       bsv  05/03/21 Add provision to load bitstream from OCM with DDR
*                     present in design
*       bsv  05/15/21 Support to ensure authenticated images boot as
*                     non-secure when RSA_EN is not programmed is disabled by
*                     default
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
#ifndef FSBL_PRINT_VAL
#define FSBL_PRINT_VAL              (1U)
#endif

#ifndef FSBL_DEBUG_VAL
#define FSBL_DEBUG_VAL              (0U)
#endif

#ifndef FSBL_DEBUG_INFO_VAL
#define FSBL_DEBUG_INFO_VAL         (0U)
#endif

#ifndef FSBL_DEBUG_DETAILED_VAL
#define FSBL_DEBUG_DETAILED_VAL     (0U)
#endif

/**
 * FSBL Debug options
 */

#if (FSBL_PRINT_VAL) && (!defined(FSBL_PRINT))
#define FSBL_PRINT
#endif

#if (FSBL_DEBUG_VAL) && (!defined(FSBL_DEBUG))
#define FSBL_DEBUG
#endif

#if (FSBL_DEBUG_INFO_VAL) && (!defined(FSBL_DEBUG_INFO))
#define FSBL_DEBUG_INFO
#endif

#if (FSBL_DEBUG_DETAILED_VAL) && (!defined(FSBL_DEBUG_DETAILED))
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
 *     - FSBL_USB_EXCLUDE_VAL USB boot mode related code is excluded
 *     - FSBL_PROT_BYPASS_EXCLUDE_VAL Isolation configurations are excluded
 *     - FSBL_PARTITION_LOAD_EXCLUDE_VAL Partition loading is excluded
 *     - FSBL_FORCE_ENC_EXCLUDE_VAL Forcing encryption for every partition
 *       when ENC only bit is blown will be excluded
 *     - FSBL_DDR_SR_EXCLUDE_VAL DDR self refresh code is excluded
 *     - FSBL_TPM_EXCLUDE_VAL TPM related code is excluded
 *     - FSBL_PL_LOAD_FROM_OCM_EXCLUDE_VAL Code to loading bitstream in chunks
 *       from OCM is excluded
 *     - FSBL_UNPROVISIONED_AUTH_SIGN_EXCLUDE_VAL Code to "load authenticated
 *       partitions as non secure when EFUSEs are not programmed and when boot
 *       header is not authenticated" is excluded
 */
#ifndef FSBL_NAND_EXCLUDE_VAL
#define FSBL_NAND_EXCLUDE_VAL			(0U)
#endif

#ifndef FSBL_QSPI_EXCLUDE_VAL
#define FSBL_QSPI_EXCLUDE_VAL			(0U)
#endif

#ifndef FSBL_SD_EXCLUDE_VAL
#define FSBL_SD_EXCLUDE_VAL				(0U)
#endif

#ifndef FSBL_SECURE_EXCLUDE_VAL
#define FSBL_SECURE_EXCLUDE_VAL			(0U)
#endif

#ifndef FSBL_BS_EXCLUDE_VAL
#define FSBL_BS_EXCLUDE_VAL				(0U)
#endif

#ifndef FSBL_EARLY_HANDOFF_EXCLUDE_VAL
#define FSBL_EARLY_HANDOFF_EXCLUDE_VAL	(1U)
#endif

#ifndef FSBL_WDT_EXCLUDE_VAL
#define FSBL_WDT_EXCLUDE_VAL			(0U)
#endif

#ifndef FSBL_PERF_EXCLUDE_VAL
#define FSBL_PERF_EXCLUDE_VAL			(1U)
#endif

#ifndef FSBL_A53_TCM_ECC_EXCLUDE_VAL
#define FSBL_A53_TCM_ECC_EXCLUDE_VAL	(1U)
#endif

#ifndef FSBL_PL_CLEAR_EXCLUDE_VAL
#define FSBL_PL_CLEAR_EXCLUDE_VAL		(1U)
#endif

#ifndef FSBL_USB_EXCLUDE_VAL
#define FSBL_USB_EXCLUDE_VAL			(1U)
#endif

#ifndef FSBL_PROT_BYPASS_EXCLUDE_VAL
#define FSBL_PROT_BYPASS_EXCLUDE_VAL	(1U)
#endif

#ifndef FSBL_PARTITION_LOAD_EXCLUDE_VAL
#define FSBL_PARTITION_LOAD_EXCLUDE_VAL (0U)
#endif

#ifndef FSBL_FORCE_ENC_EXCLUDE_VAL
#define FSBL_FORCE_ENC_EXCLUDE_VAL		(0U)
#endif

#ifndef FSBL_DDR_SR_EXCLUDE_VAL
#define FSBL_DDR_SR_EXCLUDE_VAL			(1U)
#endif

#ifndef FSBL_TPM_EXCLUDE_VAL
#define FSBL_TPM_EXCLUDE_VAL			(1U)
#endif

#ifndef FSBL_PL_LOAD_FROM_OCM_EXCLUDE_VAL
#define FSBL_PL_LOAD_FROM_OCM_EXCLUDE_VAL	(1U)
#endif

#ifndef FSBL_UNPROVISIONED_AUTH_SIGN_EXCLUDE_VAL
#define FSBL_UNPROVISIONED_AUTH_SIGN_EXCLUDE_VAL	(1U)
#endif

#if (FSBL_NAND_EXCLUDE_VAL) && (!defined(FSBL_NAND_EXCLUDE))
#define FSBL_NAND_EXCLUDE
#endif

#if (FSBL_QSPI_EXCLUDE_VAL) && (!defined(FSBL_QSPI_EXCLUDE))
#define FSBL_QSPI_EXCLUDE
#endif

#if (FSBL_SD_EXCLUDE_VAL) && (!defined(FSBL_SD_EXCLUDE))
#define FSBL_SD_EXCLUDE
#endif

#if (FSBL_SECURE_EXCLUDE_VAL) && (!defined(FSBL_SECURE_EXCLUDE))
#define FSBL_SECURE_EXCLUDE
#endif

#if (FSBL_BS_EXCLUDE_VAL) && (!defined(FSBL_BS_EXCLUDE))
#define FSBL_BS_EXCLUDE
#endif

#if (FSBL_EARLY_HANDOFF_EXCLUDE_VAL) && (!defined(FSBL_EARLY_HANDOFF_EXCLUDE))
#define FSBL_EARLY_HANDOFF_EXCLUDE
#endif

#if (FSBL_WDT_EXCLUDE_VAL) && (!defined(FSBL_WDT_EXCLUDE))
#define FSBL_WDT_EXCLUDE
#endif

#if (FSBL_PERF_EXCLUDE_VAL) && (!defined(FSBL_PERF_EXCLUDE))
#define FSBL_PERF_EXCLUDE
#endif

#if (FSBL_A53_TCM_ECC_EXCLUDE_VAL) && (!defined(FSBL_A53_TCM_ECC_EXCLUDE))
#define FSBL_A53_TCM_ECC_EXCLUDE
#endif

#if (FSBL_PL_CLEAR_EXCLUDE_VAL) && (!defined(FSBL_PL_CLEAR_EXCLUDE))
#define FSBL_PL_CLEAR_EXCLUDE
#endif

#if (FSBL_USB_EXCLUDE_VAL) && (!defined(FSBL_USB_EXCLUDE))
#define FSBL_USB_EXCLUDE
#endif

#if (FSBL_PROT_BYPASS_EXCLUDE_VAL) && (!defined(FSBL_PROT_BYPASS_EXCLUDE))
#define FSBL_PROT_BYPASS_EXCLUDE
#endif

#if (FSBL_PARTITION_LOAD_EXCLUDE_VAL) && \
	(!defined(FSBL_PARTITION_LOAD_EXCLUDE))
#define FSBL_PARTITION_LOAD_EXCLUDE
#endif

#if (FSBL_FORCE_ENC_EXCLUDE_VAL) && (!defined(FSBL_FORCE_ENC_EXCLUDE))
#define FSBL_FORCE_ENC_EXCLUDE
#endif

#if (FSBL_DDR_SR_EXCLUDE_VAL == 0U) && (!defined(XFSBL_ENABLE_DDR_SR))
#define XFSBL_ENABLE_DDR_SR
#endif

#if (FSBL_TPM_EXCLUDE_VAL == 1U) && (!defined(FSBL_TPM_EXCLUDE))
#define FSBL_TPM_EXCLUDE
#endif

#if (FSBL_PL_LOAD_FROM_OCM_EXCLUDE_VAL == 1U) && \
	(!defined(FSBL_PL_LOAD_FROM_OCM_EXCLUDE))
#define FSBL_PL_LOAD_FROM_OCM_EXCLUDE
#endif

#if (FSBL_UNPROVISIONED_AUTH_SIGN_EXCLUDE_VAL == 1U) && \
	(!defined(FSBL_UNPROVISIONED_AUTH_SIGN_EXCLUDE))
#define FSBL_UNPROVISIONED_AUTH_SIGN_EXCLUDE
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_CONFIG_H */
