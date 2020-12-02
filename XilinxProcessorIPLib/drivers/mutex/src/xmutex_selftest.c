/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmutex_selftest.c
* @addtogroup mutex_v4_6
* @{
*
* Contains XMutex driver selftest code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va            First release
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 3.00a hbm  10/15/09 Migrated to HAL phase 1 to use xil_io, xil_types,
*			and xil_assert.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xmutex.h"
#include "xparameters.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Selftest a particular Mutex hardware core.
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
*
* @return
*		- XST_SUCCESS if test was successful.
*		- XST_FAILURE if test was not successful.
*
* @note
*
* This test is destructive. It will fail if the Mutex is currently being used.
* This is also a blocking call, if there is another process which has the
* Mutex, the first _lock will hand the test until the other process releases
* it.
*
******************************************************************************/
int XMutex_SelfTest(XMutex *InstancePtr)
{
	int Status;
	u32 Locked;
	u32 Owner;
	u32 Index;

	for (Index = 0; Index < InstancePtr->Config.NumMutex; Index++) {

		/* Lock Mutex blocking call*/
		XMutex_Lock(InstancePtr, Index);

		/* Get Status and verify if Status matches */
		XMutex_GetStatus(InstancePtr, Index, &Locked, &Owner);
		if (Owner != XPAR_CPU_ID) {
			return XST_FAILURE;
		}

		if (Locked != LOCKED_BIT) {
			return XST_FAILURE;
		}

		/* Verify that the Mutex is locked */
		if (XMutex_IsLocked(InstancePtr, Index) != TRUE) {
			return XST_FAILURE;
		}

		/* Unlock Mutex */
		Status = XMutex_Unlock(InstancePtr, Index);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Get Status and verify if Status matches */
		XMutex_GetStatus(InstancePtr, Index, &Locked, &Owner);
		if (Owner != 0) {
			return XST_FAILURE;
		}

		if (Locked != 0) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
/** @} */
