/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file vectors.h
*
* This file contains the C level vector prototypes for the ARM Cortex A9 core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a ecm  10/20/10 Initial version, moved over from bsp area
* 6.0   mus  07/27/16 Consolidated vectors for a9,a53 and r5 processors
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

#ifndef _VECTORS_H_
#define _VECTORS_H_

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"

#ifdef __cplusplus
extern "C" {
#endif
/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

void FIQInterrupt(void);
void IRQInterrupt(void);
#if !defined (__aarch64__)
void SWInterrupt(void);
void DataAbortInterrupt(void);
void PrefetchAbortInterrupt(void);
void UndefinedException(void);
#else
void SynchronousInterrupt(void);
void SErrorInterrupt(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* protection macro */
