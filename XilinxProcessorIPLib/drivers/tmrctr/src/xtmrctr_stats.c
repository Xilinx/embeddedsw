/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtmrctr_stats.c
* @addtogroup Overview
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
* 4.16  ml   16/09/26 Fixed doxygen 1.15.0 and compliance checker warnings.
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
* @param[in]	InstancePtr is a pointer to the XTmrCtr instance.
* @param[out]	StatsPtr is a pointer to a XTmrCtrStats structure which will get
*		a copy of current statistics.
*
******************************************************************************/
void XTmrCtr_GetStats(XTmrCtr *InstancePtr, XTmrCtrStats *StatsPtr)
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
* @param[in]	InstancePtr is a pointer to the XTmrCtr instance.
*
******************************************************************************/
void XTmrCtr_ClearStats(XTmrCtr *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Stats.Interrupts = 0;
}
/** @} */
