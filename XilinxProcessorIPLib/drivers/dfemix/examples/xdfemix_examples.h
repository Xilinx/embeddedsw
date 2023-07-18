/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix_examples.h
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.1   dc     07/21/21 Add and reorganise examples
* 1.6   cog    07/18/23 Modify example for SDT flow
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
#include "xdfemix.h"
#include "xdfemix_hw.h"
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
#define XDFEMIX_NODE_NAME XPAR_XDFEMIX_0_DEV_NAME
#define XDFEMIX_NODE_BASE XPAR_XDFEMIX_0_S_AXI_BASEADDR
#else
#define XDFEMIX_NODE_NAME "a7c40000.xdfe_cc_mixer"
#define XDFEMIX_NODE_BASE 0xa7c40000
#endif
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFEMIX_NODE_NAME "a7c40000.xdfe_cc_mixer"
#endif

/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/
