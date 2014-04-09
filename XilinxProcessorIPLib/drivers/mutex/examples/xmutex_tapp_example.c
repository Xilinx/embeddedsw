/* $Id: xmutex_tapp_example.c,v 1.1.2.2 2011/05/12 06:42:39 sadanan Exp $ */
/******************************************************************************
*
* (c) Copyright 2008-2010 Xilinx, Inc. All rights reserved.
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
/******************************************************************************/
/**
* @file xmutex_tapp_example.c
*
* This file contains a design example for using the Mutex hardware and driver
* XMutex
*
* The example assumes there are two processors availabile in the system that
* are expected to inter-communicate.
*
* This example attempts to lock the Mutex from the processor identified as 0
* (XPAR_CPU_ID=0) to prevent the other processor from getting the lock.
* Since the application is running on two seperate processors, the
* initiator declares success when the Mutex locks the other processor
* declares success when the Mutex is locked from its perspective. There is
* no feedback to the initiator so a terminal is required for each processor
* to verify that the test passed for both sides.
*
* This example has been tested on ML505 Hardware Evaluation board.
*
* @note
*
* These code fragments will illustrate how the XMutex driver can be used to:
*   - Initialize the Mutex core
*   - Lock, Unlock and query the status of the hardware
*
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va		First release
* 1.00a ecm  06/04/07   Cleanup, new coding standard, check into XCS
* 1.00a ecm  08/28/08   Converted to testapp example
* 3.01a sdm  05/04/10   Removed printfs from the MutexExample

*</pre>
*******************************************************************************/

/***************************** Include Files **********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xmutex.h"
#include <stdio.h>
#include <stdlib.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define MUTEX_DEVICE_ID		XPAR_MUTEX_0_DEVICE_ID

#define MUTEX_NUM 		0 /**< The Mutex number on which this
				   *   example is run
				   */

#define LOCK_TIMEOUT		2000000	/* Number of attempts before error */

#define printf xil_printf 	/* Small foot-print printf function */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

XMutex Mutex;	/* Mutex instance */

/************************** Function Prototypes ******************************/

int MutexExample (u16 MutexDeviceID);

/*****************************************************************************/
/**
*
* This function is the main function for the Mutex example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	printf ("MutexExample :\tStarts.\r\n");

	if (MutexExample (MUTEX_DEVICE_ID) != XST_SUCCESS) {
		printf ("MutexExample :\tFailed.\r\n");
		printf ("MutexExample :\tEnds.\r\n");
		return XST_FAILURE;
	}

	printf ("MutexExample :\tSucceeded.\r\n");
	printf ("MutexExample :\tEnds.\r\n");

	return XST_SUCCESS;
}

#endif

/*****************************************************************************/
/**
* This function contains the actual functionality for the XMutex TestApp
* example. he idea is to have this application running on two processors which
* share the same bus and have access to the Mutex hardware. CPU 0 is considered
* the primary CPU since it is the one which is expected to initially lock the
* Mutex.
*
* @param	MutexDeviceID is the device id to be initialized and used.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
int MutexExample(u16 MutexDeviceID)
{
	XMutex_Config *ConfigPtr;
	XStatus Status;
	u32 TimeoutCount = 0;

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver instance.
	 */
	ConfigPtr = XMutex_LookupConfig(MutexDeviceID);
	if (ConfigPtr == (XMutex_Config *)NULL) {
		return XST_FAILURE;
	}

	/*
	 * Perform the rest of the initialization.
	 */
	Status = XMutex_CfgInitialize(&Mutex, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Depending on which CPU this is running on, either lock the
	 * Mutex or test the Mutex to see if it is locked.
	 */
	if (XPAR_CPU_ID == 0) {

		XMutex_Lock(&Mutex, MUTEX_NUM);	/* Acquire lock */
		XMutex_Unlock(&Mutex, MUTEX_NUM);	/* Release lock */
	} else {

		/*
		 * Try to acquire lock, other CPU should have it so it
		 * should fail.
		 */
		while ((XMutex_Trylock(&Mutex, MUTEX_NUM)) != XST_SUCCESS) {
			if (TimeoutCount++ > LOCK_TIMEOUT) {
				return XST_FAILURE;
			}
		}

		XMutex_Unlock(&Mutex, MUTEX_NUM);	/* Release lock */
	}

	return XST_SUCCESS;
}
