/******************************************************************************
*
* Copyright (C) 2002 - 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
/****************************************************************************/
/**
*
* @file xuartns550_stats.c
* @addtogroup uartns550_v3_3
* @{
*
* This file contains the statistics functions for the 16450/16550 UART driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  03/11/02 Repartitioned driver for smaller files.
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a ktn  10/20/09 Updated to use HAL processor APIs. XUartNs550_mClearStats
*		      macro is removed.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xuartns550.h"
#include "xuartns550_i.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
*
* This functions returns a snapshot of the current statistics in the area
* provided.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
* @param	StatsPtr is a pointer to a XUartNs550Stats structure to where
*		the statistics are to be copied to.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XUartNs550_GetStats(XUartNs550 *InstancePtr, XUartNs550Stats *StatsPtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(StatsPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	StatsPtr->TransmitInterrupts = InstancePtr->Stats.TransmitInterrupts;
	StatsPtr->ReceiveInterrupts = InstancePtr->Stats.ReceiveInterrupts;
	StatsPtr->StatusInterrupts = InstancePtr->Stats.StatusInterrupts;
	StatsPtr->ModemInterrupts = InstancePtr->Stats.ModemInterrupts;
	StatsPtr->CharactersTransmitted =
				InstancePtr->Stats.CharactersTransmitted;
	StatsPtr->CharactersReceived = InstancePtr->Stats.CharactersReceived;
	StatsPtr->ReceiveOverrunErrors =
				InstancePtr->Stats.ReceiveOverrunErrors;
	StatsPtr->ReceiveFramingErrors =
				InstancePtr->Stats.ReceiveFramingErrors;
	StatsPtr->ReceiveParityErrors = InstancePtr->Stats.ReceiveParityErrors;
	StatsPtr->ReceiveBreakDetected =
				InstancePtr->Stats.ReceiveBreakDetected;
}

/****************************************************************************/
/**
*
* This function zeros the statistics for the given instance.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUartNs550_ClearStats(XUartNs550 *InstancePtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Stats.TransmitInterrupts = 0;
	InstancePtr->Stats.ReceiveInterrupts = 0;
	InstancePtr->Stats.StatusInterrupts = 0;
	InstancePtr->Stats.ModemInterrupts = 0;
	InstancePtr->Stats.CharactersTransmitted = 0;
	InstancePtr->Stats.CharactersReceived = 0;
	InstancePtr->Stats.ReceiveOverrunErrors = 0;
	InstancePtr->Stats.ReceiveFramingErrors = 0;
	InstancePtr->Stats.ReceiveParityErrors = 0;
	InstancePtr->Stats.ReceiveBreakDetected = 0;
}
/** @} */
