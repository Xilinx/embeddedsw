/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_platform.h
*
* This file contains platform specific API's declaration for System Board.
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
#define DHCP_TIMEOUT		24U
#define DHCP_TIMER_COUNT	120U

/* DHCP timeout function states */
enum {
	INIT,
	GET,
	DEC
};
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int Xbir_Platform_Init (void);
void Xbir_Platform_Cleanup (void);
int Xbir_Platform_SetupTimer (void);
void Xbir_Platform_EnableInterrupts (void);
void Xbir_Platform_ClearInterrupt (void);
int Xbir_dhcp_timoutcntr(int state);

#ifdef __cplusplus
}
#endif

#endif
