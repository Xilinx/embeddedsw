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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xiomodule_stats.c
* @addtogroup iomodule_v2_7
* @{
*
* This file contains the statistics functions for the UART module
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   07/15/11 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xiomodule.h"
#include "xiomodule_i.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
*
* Returns a snapshot of the current statistics in the structure specified.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	StatsPtr is a pointer to a XIOModule_Uart_Stats structure to
*		where the statistics are to be copied.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XIOModule_GetStats(XIOModule *InstancePtr, XIOModule_Uart_Stats *StatsPtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(StatsPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Copy the stats from the instance to the specified stats */

	StatsPtr->TransmitInterrupts =
		InstancePtr->Uart_Stats.TransmitInterrupts;
	StatsPtr->ReceiveInterrupts =
		InstancePtr->Uart_Stats.ReceiveInterrupts;
	StatsPtr->CharactersTransmitted =
		InstancePtr->Uart_Stats.CharactersTransmitted;
	StatsPtr->CharactersReceived =
		InstancePtr->Uart_Stats.CharactersReceived;
	StatsPtr->ReceiveOverrunErrors =
		InstancePtr->Uart_Stats.ReceiveOverrunErrors;
	StatsPtr->ReceiveFramingErrors =
		InstancePtr->Uart_Stats.ReceiveFramingErrors;
	StatsPtr->ReceiveParityErrors =
		InstancePtr->Uart_Stats.ReceiveParityErrors;
}

/****************************************************************************/
/**
*
* This function zeros the statistics for the given instance.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XIOModule_ClearStats(XIOModule *InstancePtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Uart_Stats.TransmitInterrupts = 0;
	InstancePtr->Uart_Stats.ReceiveInterrupts = 0;
	InstancePtr->Uart_Stats.CharactersTransmitted = 0;
	InstancePtr->Uart_Stats.CharactersReceived = 0;
	InstancePtr->Uart_Stats.ReceiveOverrunErrors = 0;
	InstancePtr->Uart_Stats.ReceiveParityErrors = 0;
	InstancePtr->Uart_Stats.ReceiveFramingErrors = 0;
}
/** @} */
