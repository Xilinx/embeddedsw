/******************************************************************************
* Copyright (C) 2020-2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp_example.h
*
* This file provides the interface of the HDCP example
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XHDCP1X_EXAMPLE_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/
#include "xparameters.h"
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#include "xhdcp1x.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/****************************** Local Globals ********************************/


/************************** Function Prototypes ******************************/

int XHdcp1xExample_Init(void);
int XHdcp1xExample_Enable(void);
void XHdcp1xExample_Poll(void);
int XHdcp1xExample_TxEncrypt(void);
int XHdcp1xExample_TxAuthenticate(void);
int XHdcp1xExample_TxIsCapable(void);
int XHdcp1xExample_TxEncryptIfRxIsUp(void);
int XHdcp1xExample_TxIsAuthenticated(void);

XHdcp1x* XHdcp1xExample_Get(u16 DeviceId);

#ifdef __cplusplus
}
#endif

#endif

#endif  /* XHDCP1X_EXAMPLE_H */
