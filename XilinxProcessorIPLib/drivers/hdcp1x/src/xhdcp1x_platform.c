/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp1x_platform.c
*
* This file provides the implementation for the hdcp platform integration
* module
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdcp1x.h"
#include "xhdcp1x_platform.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Extern Declarations ******************************/
extern XHdcp1x_KsvRevokeCheck  XHdcp1xKsvRevokeCheck;
extern XHdcp1x_TimerStart  XHdcp1xTimerStart;
extern XHdcp1x_TimerStop  XHdcp1xTimerStop;
extern XHdcp1x_TimerDelay  XHdcp1xTimerDelay;

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This function checks a KSV value to determine if it has been revoked or not
*
* @param InstancePtr  the hdcp interface
* @param Ksv  the KSV to check
*
* @return
*   Truth value indicating the KSV is revoked (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PlatformIsKsvRevoked(const XHdcp1x *InstancePtr, u64 Ksv)
{
	int IsRevoked = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Sanity Check */
	if (XHdcp1xKsvRevokeCheck != NULL) {
		IsRevoked = (*XHdcp1xKsvRevokeCheck)(InstancePtr, Ksv);
	}

	return (IsRevoked);
}

/*****************************************************************************/
/**
*
* This function starts a timer on behalf of an hdcp interface
*
* @param InstancePtr  the hdcp interface
* @param TimeoutInMs  the duration of the timer in milliseconds
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PlatformTimerStart(XHdcp1x *InstancePtr, u16 TimeoutInMs)
{
	int Status = XST_SUCCESS;

	/* Sanity Check */
	if (XHdcp1xTimerStart != NULL) {
		Status = (*XHdcp1xTimerStart)(InstancePtr, TimeoutInMs);
	}
	else {
		Status = XST_FAILURE;
	}

	return (Status);
}


/*****************************************************************************/
/**
*
* This function stop a timer on behalf of an hdcp interface
*
* @param InstancePtr  the hdcp interface
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PlatformTimerStop(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Sanity Check */
	if (XHdcp1xTimerStop != NULL) {
		Status = (*XHdcp1xTimerStop)(InstancePtr);
	}
	else {
		Status = XST_FAILURE;
	}

	return (Status);
}


/*****************************************************************************/
/**
*
* This function busy waits on a timer for a number of milliseconds
*
* @param InstancePtr  the hdcp interface
* @param DelayInMs  the delay time in milliseconds
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PlatformTimerBusy(XHdcp1x *InstancePtr, u16 DelayInMs)
{
	int Status = XST_SUCCESS;

	/* Sanity Check */
	if (XHdcp1xTimerDelay != NULL) {
		Status = (*XHdcp1xTimerDelay)(InstancePtr, DelayInMs);
	}
	else {
		Status = XST_FAILURE;
	}

	return (Status);
}
