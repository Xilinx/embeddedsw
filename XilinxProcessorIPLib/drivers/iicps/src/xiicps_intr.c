/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps_intr.c
* @brief Implements the interrupt functions of the XIicPs driver.
*
* The xiicps_intr.c file contains functions of the XIicPs driver for
* interrupt-driven transfers.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------
* 1.00a drg/jz  01/30/10 First release
* 3.00	sk		01/31/15 Modified the code according to MISRAC 2012 Compliant.
* 3.24 vn  08/26/26 Fix Doxygen warnings
* 3.24 tb  09/30/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup iicps_api IICPS APIs
 * @{
 */

/***************************** Include Files *********************************/

#include "xiicps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************* Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Sets the status callback function.
*
* The driver calls the status handler when it encounters conditions that
* should be reported to the higher layer software. The handler executes in an
* interrupt context, so the amount of processing should be minimized.
*
* Refer to the xiicps.h file for a list of the Callback events. The events are
* defined to start with XIICPS_EVENT_*.
*
* @param[in,out] InstancePtr Pointer to the XIicPs instance.
* @param[in]     CallBackRef Upper layer callback reference passed back
*                when the callback function is invoked.
* @param[in]     FunctionPtr Pointer to the callback function.
*
* @note
* The handler is called within interrupt context, so it should finish its
* work quickly.
*
******************************************************************************/
void XIicPs_SetStatusHandler(XIicPs *InstancePtr, void *CallBackRef,
				  XIicPs_IntrHandler FunctionPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FunctionPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	InstancePtr->StatusHandler = FunctionPtr;
	InstancePtr->CallBackRef = CallBackRef;
}
/** @} */
