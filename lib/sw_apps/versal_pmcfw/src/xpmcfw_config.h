/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_config.h
*
* This is the header file which contains PMCFW configuration
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
#ifndef XPMCFW_CONFIG_H
#define XPMCFW_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* This is the address in DDR where bitstream will be copied temporarily */
#define XPMCFW_SC_BYPASS  /* Enable to bypass scan clear  */
/* Address where readback pdis are copied */
#ifdef RDBK_SRC_ADDR
#define XPMCFW_RDBK_SRC_ADDR	RDBK_SRC_ADDR
#else
#define XPMCFW_RDBK_SRC_ADDR	0x1000000U
#endif
#define XPMCFW_CDO_CHKSUM_BYPASS  /* Enable checksum bypass */
//#define XPMCFW_HOUSECLEAN_BYPASS  /* Bypass house cleaning */
//#define XPMCFW_STATIC_NPI_BYPASS /* Enable to bypass static NPI configuration */
//#define XPMCFW_ME  /* Enable for static ME NPI and NOC configuration */
//#define XPMCFW_CPM
//#define XPMCFW_HW50
//#define XPMCFW_HW60
//#define XPMCFW_HW70
//#define XPMCFW_HW80
//#define XPMCFW_HW90
#define XPMCFW_HW100
#define XPMCFW_DDR_TEMP_ADDR			(0x100000U)
#define XPMCFW_TEST_DDR_ADDR			(0x100000U)
#define XPMCFW_TEST_DDR_VAL			(0xDEADBEEFU)
#define XPMCFW_DDR64
//#undef STDOUT_BASEADDRESS

/* Slave PDI offset address */
#if SLAVE_PDI_QSPI_OFFSET
#define XPMCFW_SLAVE_PDI_QSPI_OFFSET	SLAVE_PDI_QSPI_OFFSET
#else
#define XPMCFW_SLAVE_PDI_QSPI_OFFSET	(0x60010U)
#endif

/* Slave PDI image length */
#if SLAVE_PDI_LENGTH
#define XPMCFW_SLAVE_PDI_LENGTH			SLAVE_PDI_LENGTH
#else
#define XPMCFW_SLAVE_PDI_LENGTH			(0x2F140U)
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name PMCFW Debug options
 *
 *  PMCFW supports an unconditional print
 *     - PMCFW_PRINT Used to print PMCFW header and any mandatory prints
 *       Hence PMCFW_PRINT_VAL should always be 1
 *  Further PMCFW by default doesn't have any debug prints enabled. If user
 *  want to enable the debug prints, they can define the following
 *  options
 *  PMCFW supports three types of debug levels.
 *     - PMCFW_DEBUG Defining this will print basic information and
 *       error prints if any
 *     - PMCFW_DEBUG_INFO Defining this will have prints enabled with format
 *       specifiers in addition to the basic information
 *     - PMCFW_DEBUG_DETAILED Defining this will print information with
 *       all data exchanged.
 */
/**
 * PMCFW Debug options
 */

//#define PMCFW_PRINT
//#define PMCFW_DEBUG
#define PMCFW_DEBUG_INFO
//#define PMCFW_DEBUG_DETAILED
//#define PMCFW_DEBUG_REG_WRITE
/**
 * @name PMCFW code include options
 *
 *  PMCFW by default all the code is included.
 *  Unwanted code can be excluded from the elf by defining here
 *  Below blocks can be excluded from the code.
 *     - PMCFW_QSPI_EXCLUDE QSPI code will be excluded
 *     - PMCFW_SD_EXCLUDE SD code will be excluded
 *     - PMCFW_SECURE_EXCLUDE Secure features
 *	 (authentication, decryption, checksum) will be excluded
 *     - PMCFW_WDT_EXCLUDE WDT code will be excluded
 *     - PMCFW_PERF_EXCLUDE_VAL Performance prints are excluded
 */
//#define PMCFW_QSPI_EXCLUDE
//#define PMCFW_SD_EXCLUDE
//#define PMCFW_SBI_EXCLUDE
//#define PMCFW_SECURE_EXCLUDE
#define PMCFW_SSIT_EXCLUDE

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPMCFW_CONFIG_H */
