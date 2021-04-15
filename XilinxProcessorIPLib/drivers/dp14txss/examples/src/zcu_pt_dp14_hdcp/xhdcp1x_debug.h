/******************************************************************************
* Copyright (C) 2020-2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp_debug.h
*
* This file provides the interface of the HDCP debug commands
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
#ifndef XHDCP1X_DEBUG_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/****************************** Local Globals ********************************/


/************************** Function Prototypes ******************************/
int XHdcp1xDebug_ProcessKey(char CmdKey);


#ifdef __cplusplus
}
#endif


#endif  /* XHDCP1X_DEBUG_H */
