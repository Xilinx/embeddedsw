/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtmrctr_stats.c
* @addtogroup tmrctr_v4_7
* @{
*
* Contains function to get and clear statistics for the XTmrCtr component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  02/06/02 First release.
* 1.10b mta  03/21/07 Updated for new coding style.
* 2.00a ktn  10/30/09 Updated to use HAL API's.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtmrctr.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Get a copy of the XTmrCtrStats structure, which contains the current
* statistics for this driver.
*
* @param	InstancePtr is a pointer to the XTmrCtr instance.
* @param	StatsPtr is a pointer to a XTmrCtrStats structure which will get
*		a copy of current statistics.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTmrCtr_GetStats(XTmrCtr * InstancePtr, XTmrCtrStats * StatsPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(StatsPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	StatsPtr->Interrupts = InstancePtr->Stats.Interrupts;
}

/*****************************************************************************/
/**
*
* Clear the XTmrCtrStats structure for this driver.
*
* @param	InstancePtr is a pointer to the XTmrCtr instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTmrCtr_ClearStats(XTmrCtr * InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Stats.Interrupts = 0;
}
/** @} */
