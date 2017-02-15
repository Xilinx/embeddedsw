/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
*
* @file xtmr_manager_polled_example.c
*
* This file contains a design example using the TMR_Manager driver (XTMR_Manager)
* and hardware device using the polled mode.
*
* @note
*
* The user must provide a physical loopback such that data which is
* transmitted will be received.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xtmr_manager.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMR_MANAGER_DEVICE_ID	XPAR_TMR_MANAGER_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMR_ManagerPolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XTMR_Manager TMR_Manager;	/* Instance of the TMR_Manager Device */


/*****************************************************************************/
/**
*
* Main function to call the TMR Manager polled example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the TMR_Manager polled example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = TMR_ManagerPolledExample(TMR_MANAGER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}


/****************************************************************************/
/**
* This function does a minimal test on the TMR_Manager device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTMR_Manager component.
*
* This function polls the TMR_Manager and does not require the use of
* interrupts.
*
* @param	DeviceId is the Device ID of the TMR_Manager and is the
*		XPAR_<tmr_manager_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
*
* @note		None
*
****************************************************************************/
int TMR_ManagerPolledExample(u16 DeviceId)
{
	int Status;
	unsigned int SentCount;
	unsigned int ReceivedCount = 0;
	int Index;

	/*
	 * Initialize the TMR_Manager driver so that it is ready to use.
	 */
	Status = XTMR_Manager_Initialize(&TMR_Manager, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XTMR_Manager_SelfTest(&TMR_Manager);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
