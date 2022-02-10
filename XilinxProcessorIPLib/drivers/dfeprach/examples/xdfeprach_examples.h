/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved.
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
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#include "xparameters.h"
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
#define XDFEPRACH_NODE_NAME XPAR_XDFEPRACH_0_DEV_NAME
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFEPRACH_NODE_NAME "a7d40000.xdfe_nr_prach"
#endif

/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/
