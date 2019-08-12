/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*
******************************************************************************/

/*****************************************************************************/
/**
* @file keymgmt_debug.h
*
* This file contains the debug related definitions of the key management
* software.
*
* @note
* The contents of this file adjust the definition of the PRINTF and DEBUG_LOG
* macros based on the presence/absence of a "FIDUS_BLAZY" definition from
* within xparameters.h.  It is assumed that is such a definition is present,
* then the console infrastructure will be part of the build.  Otherwise, the
* raw Xilinx BSP functions are called directly.
*
******************************************************************************/


#ifndef KEYMGMT_DEBUG_H
#define KEYMGMT_DEBUG_H


/* Include Files */
#include "xparameters.h"


#if defined(XPAR_FIDUS_BLAZY_MICROBLAZE_0_COMPONENT_NAME)
  #include "console.h"
#else
  #include "xil_printf.h"
#endif


/******************************************************************************
 *
 * KEYMGMT_CONSOLE_PRINTF
 *
 * @note
 * This macro is defined to allow for the debug console functions of the key
 * management module to be directed to the correct output
 *
 ******************************************************************************/
#if defined(CONSOLE_H)
  #define KEYMGMT_CONSOLE_PRINTF       CONSOLE_Printf
#else
  #define KEYMGMT_CONSOLE_PRINTF       xil_printf
#endif


/******************************************************************************
 *
 * KEYMGMT_DEBUG_LOG
 *
 * @note
 * This macro is defined to allow for the debug log messages of the key
 * management module to be directed to the correct output
 *
 ******************************************************************************/
#define KEYMGMT_DEBUG_LOG



#endif /* KEYMGMT_DEBUG_H */
