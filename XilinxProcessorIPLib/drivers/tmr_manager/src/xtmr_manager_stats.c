/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xtmr_manager_stats.c
* @addtogroup tmr_manager_v1_0
* @{
*
* This file contains the statistics functions for the TMR Manager component
* (XTMR_Manager).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xtmr_manager.h"
#include "xtmr_manager_i.h"

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
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
* @param	StatsPtr is a pointer to a XTMR_ManagerStats structure to where the
*		statistics are to be copied.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XTMR_Manager_GetStats(XTMR_Manager *InstancePtr, XTMR_Manager_Stats *StatsPtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(StatsPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Copy the stats from the instance to the specified stats */

	StatsPtr->InterruptCount = InstancePtr->Stats.InterruptCount;
	StatsPtr->RecoveryCount = InstancePtr->Stats.RecoveryCount;
}

/****************************************************************************/
/**
*
* This function zeros the statistics for the given instance.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XTMR_Manager_ClearStats(XTMR_Manager *InstancePtr)
{
	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Stats.InterruptCount = 0;
	InstancePtr->Stats.RecoveryCount = 0;
}

/** @} */
