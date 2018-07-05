/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xmutex_selftest.c
* @addtogroup mutex_v4_3
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
