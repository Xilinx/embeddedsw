/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaietile_error.h
* @{
*
*  Header file for error handling
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   01/09/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_ERROR_H
#define XAIETILE_ERROR_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
/***************************** Macro Definitions *****************************/

#define XAIETILE_ERROR_BROADCAST	0U /**< Broadcast signal to broadcast errors */
#define XAIETILE_ERROR_SHIM_INTEVENT	0x10U /**< SHIM Internal event for errors */
#define XAIETILE_ERROR_ALL		0x0U /**< All errors except those are
						  set as poll only (no logging) */
/***************************** Type Definitions ******************************/

/************************** Function Prototypes  *****************************/
int XAieTile_ErrorRegisterNotification(XAieGbl *AieInst, u8 Module, u8 Error, XAieTile_ErrorCallBack Cb, void *Arg);
void XAieTile_ErrorUnregisterNotification(XAieGbl *AieInst, u8 Module, u8 Error, u8 Logging);
int XAieTile_ErrorsHandlingInitialize(XAieGbl *AieInst);
#endif		/* end of protection macro */

/** @} */
