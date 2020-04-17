/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *  @file xaxicdma_i.h
* @addtogroup axicdma_v4_7
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

#ifndef XAXICDMA_I_H_    /* prevent circular inclusions */
#define XAXICDMA_I_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xaxicdma.h"

/************************** Constant Definitions *****************************/
#define XAXICDMA_RESET_LOOP_LIMIT	5

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

int XAxiCdma_IsSimpleMode(XAxiCdma *InstancePtr);
int XAxiCdma_SwitchMode(XAxiCdma *InstancePtr, int Mode);
int XAxiCdma_BdRingStartTransfer(XAxiCdma *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif    /* prevent circular inclusions */
/** @} */
