/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc. All rights reserved.
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
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xwdttb_config.h
* @addtogroup wdttb_v5_0
* @{
*
* Contains the required functions of the XWdtTb driver. See xwdttb.h for a
* description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 5.0   sne  11/19/19 First release
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#ifndef XWDTTB_CONFIG_H                /* prevent circular inclusions */
#define XWDTTB_CONFIG_H                /* by using protection macros */

#include "xwdttb.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Function Prototypes ******************************/

/*
 * Required functions in xwdttb_help.c
 */

s32 XWdtTb_DisableWinWdt(XWdtTb *InstancePtr);
void XWdtTb_EnableTimebaseWdt(XWdtTb *InstancePtr);
s32 XWdtTb_DisableTimebaseWdt(XWdtTb *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
