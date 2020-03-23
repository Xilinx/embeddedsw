/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xil_clocking.c
*
* The xil_clocking.c file contains clocking related functions and macros.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 7.2 sd  02/06/20 First release of clocking
* 7.2 sd  03/20/20 Added checking for isolation case
* </pre>
*
******************************************************************************/

#include "xil_clocking.h"
/************************** Variable Definitions *****************************/

#if defined  (XPAR_XCRPSU_0_DEVICE_ID) && defined (XCLOCKING)

XClock ClockInstance;		/* Instance of clock Controller */
XClockPs_Config *ConfigPtr;


XStatus Xil_ClockInit(void)
{
	XStatus Status = XST_FAILURE;

	/* Lookup clock configurations */
	ConfigPtr = XClock_LookupConfig(XPAR_XCLOCKPS_DEVICE_ID);

	/* Initialize the Clock controller driver */
	Status = XClock_CfgInitialize(&ClockInstance, ConfigPtr);
	return Status;
}

XStatus Xil_ClockEnable(XClock_OutputClks ClockId)
{

	XStatus Status = XST_FAILURE;

	Status = XClock_EnableClock(ClockId);
	return Status;
}

XStatus Xil_ClockDisable(XClock_OutputClks ClockId)
{
	XStatus Status = XST_FAILURE;

	Status = XClock_DisableClock(ClockId);
	return Status;
}

XStatus Xil_ClockGetRate(XClock_OutputClks ClockId, XClockRate *Rate)
{
	XStatus Status = XST_FAILURE;

	Xil_AssertNonvoid(Rate != NULL);

	Status = XClock_GetRate(ClockId, Rate);
	if (XST_SUCCESS == Status) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "Operating rate =  %lx\n",*Rate);
	} else {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed: Fetching rate\r\n");
	}
	return Status;
}

XStatus Xil_ClockSetRate(XClock_OutputClks ClockId, XClockRate Rate,
							XClockRate *SetRate)
{
	XStatus Status = XST_FAILURE;

	Xil_AssertNonvoid(SetRate != NULL);

	if (Rate == 0) {
		return XST_FAILURE;
	}
	Status = XClock_SetRate(ClockId, Rate, SetRate);
	if (XST_SUCCESS != Status) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed Setting rate\n");
	}
	return Status;
}

#else
XStatus Xil_ClockGetRate(XClock_OutputClks ClockId, XClockRate *Rate)
{
	(void) ClockId;
	(void) Rate;
	return XST_FAILURE;
}

XStatus Xil_ClockSetRate(XClock_OutputClks ClockId, XClockRate Rate,
							XClockRate *SetRate) {
	(void) ClockId;
	(void) Rate;
	(void) SetRate;
	return XST_FAILURE;
}

XStatus Xil_ClockInit(void)
{
	return XST_SUCCESS;
}

XStatus Xil_ClockEnable(XClock_OutputClks ClockId)
{
	(void) ClockId;
	return XST_SUCCESS;
}

XStatus Xil_ClockDisable(XClock_OutputClks ClockId)
{
	(void) ClockId;
	return XST_SUCCESS;
}

#endif /* XCLOCKING */
