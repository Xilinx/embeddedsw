/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeprach_examples.h
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.1   dc     07/21/21 Add and reorganise examples
* 1.7   dc     11/29/23 Add continuous scheduling
*       cog    19/02/24 SDT Support
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#ifndef SDT
#include "xparameters.h"
#endif
#include <metal/device.h>
#endif
#include "xdfeprach.h"
#include "xdfeprach_hw.h"
#include <math.h>
#include <stdio.h>
#include <metal/log.h>
#include <metal/sleep.h>

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#ifndef SDT
#define XDFEPRACH_NODE_NAME XPAR_XDFEPRACH_0_DEV_NAME
#define XDFEPRACH_NODE_BASE XPAR_XDFEPRACH_0_BASEADDR
#else
#define XDFEPRACH_NODE_NAME "a7e00000.xdfe_nr_prach"
#define XDFEPRACH_NODE_BASE 0xa7e00000
#endif
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFEPRACH_NODE_NAME "a7d40000.xdfe_nr_prach"
#endif

/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/
