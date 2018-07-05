/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp1x_debug.h
* @addtogroup hdcp1x_v4_2
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
