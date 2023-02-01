/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_config.h
*
* This file contains declaration of API's specific to platform configuration.
*
******************************************************************************/
#ifndef __XBIR_CONFIG_H_
#define __XBIR_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
/**
 * @name Image Recovery Debug options
 *
 *  IR supports an unconditional print
 *     - IR_PRINT_VAL Used to print any mandatory prints
 *       Hence IR_PRINT_VAL should always be 1
 *  Further IR by default doesn't have any debug prints enabled. If user
 *  want to enable the debug prints, they can define the following
 *  options
 *  IR supports two types of debug levels.
 *     - IR_DEBUG_VAL Defining this will print basic information and
 *       error prints if any
 *     - IR_DEBUG_INFO_VAL Defining this will have prints enabled with format
 *       specifiers in addition to the basic information
 */
#ifndef IR_PRINT_VAL
#define IR_PRINT_VAL           (1U)
#endif

#ifndef IR_DEBUG_VAL
#define IR_DEBUG_VAL           (0U)
#endif

#ifndef IR_DEBUG_INFO_VAL
#define IR_DEBUG_INFO_VAL      (0U)
#endif

/**
 * IR Debug options
 */

#if (IR_PRINT_VAL) && (!defined(IR_PRINT))
#define IR_PRINT
#endif

#if (IR_DEBUG_VAL) && (!defined(IR_DEBUG))
#define IR_DEBUG
#endif

#if (FSBL_DEBUG_INFO_VAL) && (!defined(IR_DEBUG_INFO))
#define IR_DEBUG_INFO
#endif

#define DEBUG_PRINT_ALWAYS    (0x00000001U)    /* unconditional messages */
#define DEBUG_GENERAL         (0x00000002U)    /* general debug  messages */
#define DEBUG_INFO            (0x00000004U)    /* More debug information */

#if defined (IR_DEBUG_INFO)
#define XbirDbgCurrentTypes ((DEBUG_INFO) | (DEBUG_GENERAL) | \
         (DEBUG_PRINT_ALWAYS))
#elif defined (IR_DEBUG)
#define XbirDbgCurrentTypes ((DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (IR_PRINT)
#define XbirDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XbirDbgCurrentTypes (0U)
#endif


#if defined(XPAR_XUARTPS_NUM_INSTANCES)
#define Xbir_Printf(DebugType,...) \
        if(((DebugType) & XbirDbgCurrentTypes)!=XST_SUCCESS) {xil_printf (__VA_ARGS__); }
#else
#define Xbir_Printf(Str, ...)
#endif

#if (XPAR_XSDPS_0_BASEADDR == 0xFF160000)
#define XBIR_SD_0
#endif

#if ((XPAR_XSDPS_0_BASEADDR == 0xFF170000) ||\
        (XPAR_XSDPS_1_BASEADDR == 0xFF170000))
#define XBIR_SD_1
#endif

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif
