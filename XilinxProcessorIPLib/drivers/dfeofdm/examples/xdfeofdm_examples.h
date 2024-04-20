/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeofdm_examples.h
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   dc     11/21/22 Initial version
* 1.1   cog    07/18/23 Modify example for SDT flow
* 1.2   cog    04/20/24 Configure si570 in Linux examples
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
#include "xdfeofdm.h"
#include "xdfeofdm_hw.h"
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
#define XDFEOFDM_NODE1_NAME XPAR_XDFEOFDM_0_DEV_NAME
#define XDFEOFDM_NODE1_BASE XPAR_XDFEOFDM_0_BASEADDR
#else
#define XDFEOFDM_NODE1_NAME "a7e40000.xdfe_ofdm"
#define XDFEOFDM_NODE1_BASE 0xa7e40000
#endif
#else
#define XDFEOFDM_NODE1_NAME "a7e40000.xdfe_ofdm"
#endif
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88

/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/
