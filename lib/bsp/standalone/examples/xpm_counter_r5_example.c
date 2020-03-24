/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
*
*
******************************************************************************/
/**
*
* @file xpm_counter_r5_example.c
*
* Implements example that demonstrates usage of R5 PMU counters and the
* available APIs provided through xpm_counter.c. This example is to be used
* on Cortex-R5 based platforms.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 7.2   asa  3/18/10  First release of R5 based performance counter example
* </pre>
******************************************************************************/
#if defined (ARMR5) && defined (__GNUC__)
#include "xil_printf.h"
#include "xpm_counter.h"
#include "xstatus.h"

/* Variables defined that are used to create PM events */
volatile u32 DummyWrite;
volatile u32 DummyWrite1;
volatile u32 DummyWrite2;

int main()
{
    u32 CntrId;
    u32 CntrId1;
    u32 CntrId2;
    u32 CntrId3;
    u32 CntVal;
    u32 CntVal1;
    u32 CntVal2;

    xil_printf("Start of R5 PMU Example\n\r");

	/* Disable and reset all counters to start from a known state */
	Xpm_DisableEventCounters();
    Xpm_ResetEventCounters();

	/*
	 * Test the software increment register event
	 * The software increment event has an event ID of 0.
	 */
    CntrId = Xpm_SetUpAnEvent(XPM_EVENT_SOFTINCR);
    /*
     * Since counters are disabled just previously, this check is a dummy
     * check. But it also emphasizes that return value of the call to the API
     * should be checked. A return value of XPM_NO_COUNTERS_AVAILABLE tells
     * that all available counters are currently being used.
     */
    if(XPM_NO_COUNTERS_AVAILABLE == CntrId) {
        xil_printf("No free counter available1\r\n");
        xil_printf("R5 PMU example has FAILED\r\n");
        goto END;
    }
	/* Setup the instructions executed event. */
    CntrId1 = Xpm_SetUpAnEvent(XPM_EVENT_INSTR);
    if(XPM_NO_COUNTERS_AVAILABLE == CntrId1) {
	    xil_printf("No free counter available1\r\n");
	    xil_printf("R5 PMU example has FAILED\r\n");
	    goto END;
    }
    /* Setup data read event counter */
    CntrId2 = Xpm_SetUpAnEvent(XPM_EVENT_DATAREAD);
        if(XPM_NO_COUNTERS_AVAILABLE == CntrId) {
        xil_printf("R5 PMU example has FAILED\r\n");
        goto END;
	}

	/* Increment the SW increment register event by 3 */
    mtcp(XREG_CP15_SW_INC, (0x1U << CntrId));
    mtcp(XREG_CP15_SW_INC, (0x1U << CntrId));
    mtcp(XREG_CP15_SW_INC, (0x1U << CntrId));

	/* Create some instruction read and instruction executed events */
	DummyWrite = 1;
    DummyWrite1 = 2;
	DummyWrite2 = DummyWrite + DummyWrite1;

    /* Read the sw increment event counter */
	if (XST_FAILURE == Xpm_GetEventCounter(CntrId, &CntVal)) {
        xil_printf("R5 PMU example has FAILED\r\n");
        goto END;
    }

    /* Read the instructions executed event counter */
    if (XST_FAILURE == Xpm_GetEventCounter(CntrId1, &CntVal1)) {
		xil_printf("R5 PMU example has FAILED\r\n");
		goto END;
	}

	/* Read the data read event counter */
	if (XST_FAILURE == Xpm_GetEventCounter(CntrId2, &CntVal2)) {
        xil_printf("R5 PMU example has FAILED\r\n");
        goto END;
	}

	/*
	 * Read the instructions executed event one more time.
	 * Since all three counters are being used by now, it should
	 * return an error.
	 */
    CntrId3 = Xpm_SetUpAnEvent(XPM_EVENT_INSTR);
    if(XPM_NO_COUNTERS_AVAILABLE != CntrId3) {
		xil_printf("R5 PMU example has FAILED\r\n");
		goto END;
	}
	/* Confirm that the SW increment event counter is 3 */
    if (3 != CntVal) {
		xil_printf("R5 PMU example has FAILED. CntVal = %d\r\n",CntVal);
		goto END;
	}

	if ((CntVal1 == 0 || CntVal2 == 0)) {
		xil_printf("R5 PMU example has FAILED\r\n");
		goto END;
	}
	/* Print the counter values  */
	xil_printf("Instruction Executed Event Cntr = %d\r\n", CntVal1);
	xil_printf("Data Read Event Cntr = %d\r\n", CntVal2);

	xil_printf("R5 PMU example has PASSED\r\n");

END:
	/* Disable and reset all counters for reuse */
    Xpm_ResetEventCounters();
    Xpm_DisableEventCounters();

    return 0;
}
#endif
