/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XAXICDMA_I_H_    /* prevent circular inclusions */
#define XAXICDMA_I_H_

/*****************************************************************************/
/**
 *  @file xaxicdma_i.h
* @addtogroup axicdma Overview
* @{
 *
 * Header file for utility functions shared by driver files.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   04/16/10 First release
 * </pre>
 *
 *****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xaxicdma.h"

/************************** Constant Definitions *****************************/

/** Maximum number of reset completion polls. */
#define XAXICDMA_RESET_LOOP_LIMIT	5

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/** @cond INTERNAL */
int XAxiCdma_IsSimpleMode(XAxiCdma *InstancePtr);
int XAxiCdma_SwitchMode(XAxiCdma *InstancePtr, int Mode);
int XAxiCdma_BdRingStartTransfer(XAxiCdma *InstancePtr);
/** @endcond */

#ifdef __cplusplus
}
#endif

/** @} */
#endif    /* prevent circular inclusions */
