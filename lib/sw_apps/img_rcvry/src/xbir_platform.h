/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_platform.h
*
* This file contains platform specific API's declaration for SOM.
*
******************************************************************************/

#ifndef __XBIR_PLATFORM_H
#define __XBIR_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xttcps.h"

/************************** Constant Definitions *****************************/
#define XBIR_PLATFORM_EMAC_BASEADDR	XPAR_XEMACPS_0_BASEADDR

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int Xbir_Platform_Init (void);
void Xbir_Platform_Cleanup (void);
int Xbir_Platform_SetupTimer (void);
void Xbir_Platform_EnableInterrupts (void);
void Xbir_Platform_ClearInterrupt (void);

#ifdef __cplusplus
}
#endif

#endif