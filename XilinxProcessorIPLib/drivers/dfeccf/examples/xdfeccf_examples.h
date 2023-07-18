/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_examples.h
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.1   dc     07/21/21 Add and reorganise examples
* 1.2   dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
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
#include "xdfeccf.h"
#include "xdfeccf_hw.h"
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
#define XDFECCF_NODE1_NAME XPAR_XDFECCF_0_DEV_NAME
#define XDFECCF_NODE2_NAME XPAR_XDFECCF_1_DEV_NAME
#define XDFECCF_NODE1_BASE XPAR_XDFECCF_0_BASEADDR
#define XDFECCF_NODE2_BASE XPAR_XDFECCF_1_BASEADDR
#else
#define XDFECCF_NODE1_NAME "a7c00000.xdfe_cc_filter"
#define XDFECCF_NODE2_NAME "a7d00000.xdfe_cc_filter"
#define XDFECCF_NODE1_BASE 0xa7c00000
#define XDFECCF_NODE2_BASE 0xa7d00000
#endif
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFECCF_NODE1_NAME "a7c00000.xdfe_cc_filter"
#define XDFECCF_NODE2_NAME "a7d00000.xdfe_cc_filter"
#endif

/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/
