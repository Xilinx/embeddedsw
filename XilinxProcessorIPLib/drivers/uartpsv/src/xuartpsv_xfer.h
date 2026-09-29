/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XUARTPSV_XFER_H
#define XUARTPSV_XFER_H

/*****************************************************************************/
/**
*
* @file xuartpsv_xfer.h
*
* @brief Declares internal transfer function prototypes for the XUartPsv driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.3  rna  05/18/20  First release
* 1.15  vn   09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartpsv Overview
 * @{
 */
/** @cond INTERNAL */

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

u32 XUartPsv_Send_NoWait(XUartPsv *InstancePtr, u8 *BufferPtr, u32 NumBytes);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

/** @endcond */
/** @} */
#endif
