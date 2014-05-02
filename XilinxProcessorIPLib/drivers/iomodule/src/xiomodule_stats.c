/******************************************************************************
*
* (c) Copyright 2011-2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xiomodule_stats.c
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
