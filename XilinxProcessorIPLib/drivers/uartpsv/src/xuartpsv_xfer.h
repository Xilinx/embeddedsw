/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv_xfer.h
* @addtogroup uartpsv_v1_4
* @{
*
* This header file contains the prototypes of objects used internally.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.3  rna  05/18/20  First release
* </pre>
*
******************************************************************************/
#ifndef XUARTPSV_XFER_H		/* prevent circular inclusions */
#define XUARTPSV_XFER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xuartpsv.h"

/************************** Function Prototypes ******************************/


u32  XUartPsv_SendBuffer(XUartPsv *InstancePtr);

u32  XUartPsv_ReceiveBuffer(XUartPsv *InstancePtr);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
