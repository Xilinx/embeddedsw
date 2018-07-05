/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xiomodule_stats.c
* @addtogroup iomodule_v2_8
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
