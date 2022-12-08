/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
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
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#include "xparameters.h"
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
#define XDFEOFDM_NODE1_NAME XPAR_XDFEOFDM_0_DEV_NAME
#define XDFESI570_CURRENT_FREQUENCY 156.25
#define XDFESI570_NEW_FREQUENCY 122.88
#else
#define XDFEOFDM_NODE1_NAME "a7e40000.xdfe_ofdm"
#endif

/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/
