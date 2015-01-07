/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* </pre>
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

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name FSBL Debug options
 *
 *  FSBL by default doesn't have any prints enabled. If user
 *  want to enable the prints, they can define the following
 *  options to have FSBL prints
 *  FSBL supports three types of debug levels.
 *     - FSBL_DEBUG Defining this will print the FSBL header and
 *       same basic information and error prints if any
 *     - FSBL_DEBUG_INFO Defining this will have prints enabled with
 *       in addition to the basic information
 *     - FSBL_DEBUG_DETAILED Defining this will print information with
 *       all data exchanged.
 */
#define FSBL_DEBUG_VAL				(0U)
#define FSBL_DEBUG_INFO_VAL			(1U)
#define FSBL_DEBUG_DETAILED_VAL			(0U)

/**
 * FSBL Debug options
 */
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
 *     - FSBL_RSA_EXCLUDE RSA code will be excluded
 */
#define FSBL_NAND_EXCLUDE_VAL			(0U)
#define FSBL_QSPI_EXCLUDE_VAL			(0U)
#define FSBL_SD_EXCLUDE_VAL			(0U)
#define FSBL_RSA_EXCLUDE_VAL			(0U)
#define FSBL_SHA2_EXCLUDE_VAL			(1U)

#if FSBL_NAND_EXCLUDE_VAL
#define FSBL_NAND_EXCLUDE
#endif

#if FSBL_QSPI_EXCLUDE_VAL
#define FSBL_QSPI_EXCLUDE
#endif

#if FSBL_SD_EXCLUDE_VAL
#define FSBL_SD_EXCLUDE
#endif

#if FSBL_RSA_EXCLUDE_VAL
#define FSBL_RSA_EXCLUDE
#endif

#if FSBL_SHA2_EXCLUDE_VAL
#define FSBL_SHA2_EXCLUDE
#endif


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_CONFIG_H */
