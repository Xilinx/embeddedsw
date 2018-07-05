/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp1x_debug.h
* @addtogroup hdcp1x_v4_4
* @{
*
* This file provides the interface of the HDCP debug commands
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
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

#include "xhdcp1x.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XHDCP1X_DEBUG_PRINTF if (XHdcp1xDebugPrintf != NULL) XHdcp1xDebugPrintf
 /**< Instance of the function interface used for debug print statements. */
#define XHDCP1X_DEBUG_LOGMSG if (XHdcp1xDebugLogMsg != NULL) XHdcp1xDebugLogMsg
 /**< Instance of the function interface used for debug log messages. */

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************* External Declarations *****************************/

extern XHdcp1x_Printf XHdcp1xDebugPrintf;	/**< Instance of function
						  *  interface used for debug
						  *  print statement */
extern XHdcp1x_LogMsg XHdcp1xDebugLogMsg;	/**< Instance of function
						  *  interface used for debug
						  *  log message statement */

#ifdef __cplusplus
}
#endif

#endif  /* XHDCP1X_DEBUG_H */
/** @} */
