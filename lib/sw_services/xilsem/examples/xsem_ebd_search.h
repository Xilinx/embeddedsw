/******************************************************************************
* (c) Copyright 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xsem_ebd_search.h
*
* This file contains required declarations and definitions
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date         Changes
* ----  ---  ----------   --------------------------------------------------
* 0.1   hv   05/17/2022   Initial creation
* </pre>
*
*/
/*****************************************************************************/
#ifndef XSEM_EBDSEARCH_H		/* prevent circular inclusions */
#define XSEM_EBDSEARCH_H		/**< by using protection macros */
/***************************** Include Files *********************************/
#include "xbasic_types.h"

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

/***************************** Function prototypes ***************************/
u32 XSem_EbdLookUp(u8 BtIndex, u8 RowIndex,\
		u32 FAddr, u32 QwordIndex, u32 BitIndex);
#endif
