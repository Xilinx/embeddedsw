/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiic_stats.c
* @brief Implements IIC statistics collection.
*
* Contains statistics functions for the XIic component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- --- ------- -----------------------------------------------
* 1.01b jhl 3/26/02 repartioned the driver
* 1.01c ecm 12/05/02 new rev
* 1.13a wgr 03/22/07 Converted to new coding style.
* 2.00a ktn 10/22/09 Converted all register accesses to 32 bit access.
*		     Updated to use the HAL APIs/macros.
*		     XIic_ClearStats function is updated as the
*		     macro XIIC_CLEAR_STATS has been removed.
* 3.13  bkv 02/20/25 Fixed run time error from invalid memory access.
* 3.16  vn  07/30/26 Fix Doxygen warnings
* 3.16  tb  09/22/26 Align Doxygen comments with AMD documentation standards
*
* </pre>
*
****************************************************************************/

/**
* @addtogroup iic_api IIC APIs
* @{
*/

/***************************** Include Files *******************************/

#include "xiic.h"
#include "xiic_i.h"

/************************** Constant Definitions ***************************/

/**************************** Type Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

/************************** Variable Definitions **************************/

/*****************************************************************************/
/**
*
* @brief Gets a copy of the statistics for an IIC device.
*
* Gets a copy of the statistics for an IIC device.
*
* @param[in] InstancePtr Pointer to the XIic instance to be worked on.
* @param[out] StatsPtr Pointer to a XIicStats structure which gets a
*             copy of current statistics.
*
****************************************************************************/
void XIic_GetStats(XIic *InstancePtr, XIicStats * StatsPtr)
{
	u8 NumBytes;
	u8 *SrcPtr;
	u8 *DestPtr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(StatsPtr != NULL);

	/*
	 * Setup pointers to copy the stats structure
	 */
	SrcPtr = (u8 *) &InstancePtr->Stats;
	DestPtr = (u8 *) StatsPtr;

	/*
	 * Copy the current statistics to the structure passed in
	 */
	for (NumBytes = 0; NumBytes < sizeof(XIicStats); NumBytes++) {
		*DestPtr++ = *SrcPtr++;
	}
}

/*****************************************************************************/
/**
*
* @brief Clears the statistics for the IIC device by zeroing all counts.
*
* Clears the statistics for the IIC device by zeroing all counts.
*
* @param[in] InstancePtr Pointer to the XIic instance to be worked on.
*
****************************************************************************/
void XIic_ClearStats(XIic *InstancePtr)
{
	u8 NumBytes;
	u8 *DestPtr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	DestPtr = (u8 *)&InstancePtr->Stats;
	for (NumBytes = 0; NumBytes < sizeof(XIicStats); NumBytes++) {
		*DestPtr++ = 0;
	}

}
/** @} */
