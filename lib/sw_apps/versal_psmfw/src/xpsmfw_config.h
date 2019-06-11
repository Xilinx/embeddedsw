/******************************************************************************
*
* Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xpsmfw_config.h
*
* This file contains user configuration for PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_CONFIG_H_
#define XPSMFW_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/************* User Configurable Options ***************/

/* PSMFW print levels */
#define XPSMFW_PRINT_ALWAYS_VAL		(1U)
#define XPSMFW_DEBUG_ERROR_VAL		(0U)
#define XPSMFW_DEBUG_DETAILED_VAL	(0U)

/**
 * PSMFW Debug options
 */

#if XPSMFW_PRINT_ALWAYS_VAL
#define XPSMFW_PRINT_ALWAYS
#endif

#if XPSMFW_DEBUG_ERROR_VAL
#define XPSMFW_DEBUG_ERROR
#endif

#if XPSMFW_DEBUG_DETAILED_VAL
#define XPSMFW_DEBUG_DETAILED
#endif

/*
 * PSM Firmware code include options
 *
 * PSM Firmware by default disables some functionality
 * Here is the list of all the build flags with the default options.
 * User can modify these flags to enable or disable any functionality
 */

/* TODO: List all PSM build flags here */

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_CONFIG_H_ */
