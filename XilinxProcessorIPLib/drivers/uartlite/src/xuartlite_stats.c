/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartlite_stats.c
* @addtogroup uartlite_v3_7
* @{
*
* This file contains the statistics functions for the UART Lite component
* (XUartLite).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/31/01 First release
* 1.00b jhl  02/21/02 Repartitioned the driver for smaller files
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs.
*		      XUartLite_mClearStats macro is removed.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xuartlite.h"
#include "xuartlite_i.h"

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
* @param	InstancePtr is a pointer to the XUartLite instance.
* @param	StatsPtr is a pointer to a XUartLiteStats structure to where the
*		statistics are to be copied.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUartLite_GetStats(XUartLite *InstancePtr, XUartLite_Stats *StatsPtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(StatsPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Copy the stats from the instance to the specified stats */

	StatsPtr->TransmitInterrupts = InstancePtr->Stats.TransmitInterrupts;
	StatsPtr->ReceiveInterrupts = InstancePtr->Stats.ReceiveInterrupts;
	StatsPtr->CharactersTransmitted =
			InstancePtr->Stats.CharactersTransmitted;
	StatsPtr->CharactersReceived = InstancePtr->Stats.CharactersReceived;
	StatsPtr->ReceiveOverrunErrors =
			InstancePtr->Stats.ReceiveOverrunErrors;
	StatsPtr->ReceiveFramingErrors =
			InstancePtr->Stats.ReceiveFramingErrors;
	StatsPtr->ReceiveParityErrors = InstancePtr->Stats.ReceiveParityErrors;
}

/****************************************************************************/
/**
*
* This function zeros the statistics for the given instance.
*
* @param	InstancePtr is a pointer to the XUartLite instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUartLite_ClearStats(XUartLite *InstancePtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Stats.TransmitInterrupts = 0;
	InstancePtr->Stats.ReceiveInterrupts = 0;
	InstancePtr->Stats.CharactersTransmitted = 0;
	InstancePtr->Stats.CharactersReceived = 0;
	InstancePtr->Stats.ReceiveOverrunErrors = 0;
	InstancePtr->Stats.ReceiveFramingErrors = 0;
	InstancePtr->Stats.ReceiveParityErrors = 0;

}

/** @} */
