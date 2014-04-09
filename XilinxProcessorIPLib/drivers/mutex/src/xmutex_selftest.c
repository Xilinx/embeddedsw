/******************************************************************************
*
* (c) Copyright 2007-2013 Xilinx, Inc. All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xmutex_selftest.c
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
