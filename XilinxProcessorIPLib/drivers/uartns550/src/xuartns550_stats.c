/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550_stats.c
* @addtogroup uartns550 Overview
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
