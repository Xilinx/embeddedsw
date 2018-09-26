/******************************************************************************
*
* Copyright (C) 2011 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xiomodule_i.h
* @addtogroup iomodule_v2_6
* @{
*
* This file contains data which is shared between files and internal to the
* XIOModule component. It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a sa   07/15/11 First release
* 1.02a sa   07/25/12 Added UART prototypes
* </pre>
*
******************************************************************************/

#ifndef XIOMODULE_I_H		/* prevent circular inclusions */
#define XIOMODULE_I_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xiomodule.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************
*
* Update the statistics of the instance.
*
* @param	InstancePtr is a pointer to the XIOMOdule instance.
* @param	StatusRegister contains the contents of the UART status
*		register to update the statistics with.
*
* @return	None.
*
* @note
*
* Signature: void XIOModule_UpdateStats(XIOModule *InstancePtr,
*						u32 StatusRegister)
*
*****************************************************************************/
#define XIOModule_UpdateStats(InstancePtr, StatusRegister)		\
{									\
	if ((StatusRegister) & XUL_SR_OVERRUN_ERROR)			\
	{								\
		(InstancePtr)->Uart_Stats.ReceiveOverrunErrors++;	\
	}								\
	if ((StatusRegister) & XUL_SR_PARITY_ERROR)			\
	{								\
		(InstancePtr)->Uart_Stats.ReceiveParityErrors++;	\
	}								\
	if ((StatusRegister) & XUL_SR_FRAMING_ERROR)			\
	{								\
		(InstancePtr)->Uart_Stats.ReceiveFramingErrors++;	\
	}								\
}

/************************** Function Prototypes ******************************/

unsigned int XIOModule_SendBuffer(XIOModule *InstancePtr);
unsigned int XIOModule_ReceiveBuffer(XIOModule *InstancePtr);


/************************** Variable Definitions *****************************/

extern u32 XIOModule_BitPosMask[];

extern XIOModule_Config XIOModule_ConfigTable[];

#ifdef __cplusplus
}
#endif

#endif
/** @} */
