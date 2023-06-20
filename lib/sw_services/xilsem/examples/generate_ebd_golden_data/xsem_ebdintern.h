/******************************************************************************
* (c) Copyright 2022 Xilinx, Inc.  All rights reserved.
* (c) Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xsem_ebdintern.h
*
* This file contains required declarations and definitions
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date         Changes
* ----  ---  ----------   --------------------------------------------------
* 0.1   hv   05/17/2022   Initial creation
* 0.2   anv  05/10/2023	  Replaced file name from xbasic_types.h to xilsem_types.h
*                         and Updated copyright information
* </pre>
*
*/
/*****************************************************************************/
#ifndef XSEM_EBDINTERN_H		/* prevent circular inclusions */
#define XSEM_EBDINTERN_H		/**< by using protection macros */
/***************************** Include Files *********************************/
#include "xilsem_types.h"

/************************** Macro Definitions ********************************/
#define XSEM_BT0ROW0_FRAMES	34111
#define XSEM_BT0ROW1_FRAMES	38498
#define XSEM_BT0ROW2_FRAMES	38498
#define XSEM_BT0ROW3_FRAMES	38498
#define XSEM_BT3ROW0_FRAMES	11
#define XSEM_BT3ROW1_FRAMES	12
#define XSEM_BT3ROW2_FRAMES	12
#define XSEM_BT3ROW3_FRAMES	12
#define XSEM_BT4ROW0_FRAMES	5
#define XSEM_BT4ROW1_FRAMES	6
#define XSEM_BT4ROW2_FRAMES	6
#define XSEM_BT4ROW3_FRAMES	6
#define XSEM_BT5ROW0_FRAMES	1
#define XSEM_BT5ROW1_FRAMES	2
#define XSEM_BT5ROW2_FRAMES	2
#define XSEM_BT5ROW3_FRAMES	2

/**
 * XSem_EbdSet - EBD set data structure
 */
typedef struct {
	u32 Frame_Addr;
	u32 words[25][4];
} XSem_EbdSet;

#endif
