/******************************************************************************
*
* Copyright (C) 2008 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* 4.2   ms   01/23/17   Modified xil_printf statement in main function to
*                       ensure that "Successfully ran" and "Failed" strings
*                       are available in all examples. This is a fix for
*                       CR-965028.
*
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
		printf ("MutexExample :\tMutex tapp Example Failed.\r\n");
		printf ("MutexExample :\tEnds.\r\n");
		return XST_FAILURE;
	}

	printf ("MutexExample :\tSuccessfully ran Mutex tapp Example\r\n");
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
