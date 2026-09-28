/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xtmr_manager_stats.c
* @addtogroup Overview
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
* 1.10  ml   25/09/26 Fixed doxygen 1.15.0 and compliance checker warnings.
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
* Returns a snapshot of the current statistics in the structure specified.
*
* @param[in] InstancePtr is a pointer to the XTMR_Manager instance.
* @param[out] StatsPtr is a pointer to a XTMR_ManagerStats structure to where the
*		statistics are to be copied.
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

	/**< Copy the stats from the instance to the specified stats */

	StatsPtr->InterruptCount = InstancePtr->Stats.InterruptCount;
	StatsPtr->RecoveryCount = InstancePtr->Stats.RecoveryCount;
}

/****************************************************************************/
/**
* This function zeros the statistics for the given instance.
*
* @param[in] InstancePtr is a pointer to the XTMR_Manager instance.
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
