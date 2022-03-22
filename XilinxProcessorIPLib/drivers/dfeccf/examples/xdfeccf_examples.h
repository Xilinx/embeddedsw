/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#include "xparameters.h"
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
#define XDFECCF_NODE1_NAME XPAR_XDFECCF_0_DEV_NAME
#define XDFECCF_NODE2_NAME XPAR_XDFECCF_1_DEV_NAME
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFECCF_NODE1_NAME "a7c00000.xdfe_cc_filter"
#define XDFECCF_NODE2_NAME "a7d00000.xdfe_cc_filter"
#endif

/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/
