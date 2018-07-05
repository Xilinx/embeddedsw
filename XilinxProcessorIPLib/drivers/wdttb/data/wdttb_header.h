/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file wdttb_header.h
*
* This header file contains the include files used to run the TestApp of
* the Watchdog Timer.
*
* @note
*
* None
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 2.00a ktn  10/22/09 Updated to use the HAL processor APIs/macros.
* 4.5   nsk  08/07/19 Fix warnings while generating testapp
*
* </pre>
*
******************************************************************************/
#ifndef WDDTB_HEADER_H		/* prevent circular inclusions */
#define WDDTB_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int WdtTbSelfTestExample(u16 DeviceId);
int WdtTbExample(u16 DeviceId);
int GWdtTbExample(u16 DeviceId);
int WinWdtTbExample(u16 DeviceId);
#endif

