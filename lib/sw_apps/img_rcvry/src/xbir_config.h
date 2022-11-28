/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
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
#if defined(XPAR_XUARTPS_NUM_INSTANCES)
#define Xbir_Printf		xil_printf
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
