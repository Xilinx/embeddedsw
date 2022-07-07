/******************************************************************************
* Copyright (c) 2020 - 2022  Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xpm_counter_r5_example.c
*
* Implements example that demonstrates usage of PMU counters and the
* available APIs provided through xpm_counter.c. This example is to be used
* on ARM based platforms.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 7.2   asa  3/18/10  First release of R5 based performance counter example
* 8.0   mus  01/19/22 Added support for CortexR52 processor.
* 8.0   mus  07/07/22 Added support for Cortex-A9 and ARMv8 based processors.
* </pre>
******************************************************************************/
#if !defined (__MICROBLAZE__)
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
    u32 Count;
    u32 RegVal;

    xil_printf("Start of PMU Example\n\r");

	/* Disable and reset all counters to start from a known state */
	Xpm_DisableEventCounters();
    Xpm_ResetEventCounters();

#if defined (__aarch64__) && defined(VERSAL_NET)
        RegVal = mfcp(MDCR_EL3);
        mtcp(MDCR_EL3, RegVal | XPM_MDCR_EL3_SPME_MASK);
#endif

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
        xil_printf("PMU example has FAILED\r\n");
        goto END;
    }
	/* Setup the instructions executed event. */
#if defined (__aarch64__)
    CntrId1 = Xpm_SetUpAnEvent(XPM_EVENT_INSTR_RETIRED);
#elif defined (ARMR5)
    CntrId1 = Xpm_SetUpAnEvent(XPM_EVENT_INSTR);
#else
    CntrId1 = Xpm_SetUpAnEvent(XPM_EVENT_SW_CHANGEPC);
#endif
    if(XPM_NO_COUNTERS_AVAILABLE == CntrId1) {
	    xil_printf("No free counter available1\r\n");
	    xil_printf("PMU example has FAILED\r\n");
	    goto END;
    }
#if defined (__aarch64__)
	CntrId2 = Xpm_SetUpAnEvent(XPM_EVENT_CPU_CYCLES);
#elif defined (ARMR5)
    /* Setup data read event counter */
    CntrId2 = Xpm_SetUpAnEvent(XPM_EVENT_DATAREAD);
#else
    CntrId2 = Xpm_SetUpAnEvent(XPM_EVENT_CLOCKCYCLES);
#endif
        if(XPM_NO_COUNTERS_AVAILABLE == CntrId2) {
        xil_printf("PMU example has FAILED\r\n");
        goto END;
	}

	/* Increment the SW increment register event by 3 */
#if defined(__aarch64__)
    mtcp(PMSWINC_EL0, 0x1);
    mtcp(PMSWINC_EL0, 0x1);
    mtcp(PMSWINC_EL0, 0x1);
#else
    mtcp(XREG_CP15_SW_INC, (0x1U << CntrId));
    mtcp(XREG_CP15_SW_INC, (0x1U << CntrId));
    mtcp(XREG_CP15_SW_INC, (0x1U << CntrId));
#endif

	/* Create some instruction read and instruction executed events */
	DummyWrite = 1;
    DummyWrite1 = 2;
	DummyWrite2 = DummyWrite + DummyWrite1;

#if defined (__aarch64__) && defined(VERSAL_NET)
	RegVal = mfcp(PMCR_EL0);
	RegVal &= 0xFFFFFFFC;
	mtcp(PMCR_EL0, RegVal);
#endif

    /* Read the sw increment event counter */
	if (XST_FAILURE == Xpm_GetEventCounter(CntrId, &CntVal)) {
        xil_printf("PMU example has FAILED\r\n");
        goto END;
    }

    /* Read the instructions executed event counter */
    if (XST_FAILURE == Xpm_GetEventCounter(CntrId1, &CntVal1)) {
		xil_printf("PMU example has FAILED\r\n");
		goto END;
	}

	/* Read the data read event counter */
	if (XST_FAILURE == Xpm_GetEventCounter(CntrId2, &CntVal2)) {
        xil_printf("PMU example has FAILED\r\n");
        goto END;
	}

#if !defined(ARMR5)
        for (Count = 3; Count < XPM_CTRCOUNT; Count++)
        {
                /*
                * CortA53/CortexA72/CortexA78/CortexA53/CortexA9 processors
                * have 6 event counters, so use 3 more event counters
                * to use all event counters.
                */
                CntrId = Xpm_SetUpAnEvent(XPM_EVENT_SOFTINCR);
                if(XPM_NO_COUNTERS_AVAILABLE == CntrId) {
                        xil_printf("No free counter available1\r\n");
                        xil_printf("PMU example has FAILED\r\n");
                        goto END;
                }
        }

#endif

	/*
	 * Read the instructions executed event one more time.
	 * Since all three counters are being used by now, it should
	 * return an error.
	 */
#if defined (__aarch64__)
	CntrId3 = Xpm_SetUpAnEvent(XPM_EVENT_INSTR_RETIRED);
#elif defined (ARMR5)
    CntrId3 = Xpm_SetUpAnEvent(XPM_EVENT_INSTR);
#else
    CntrId3 = Xpm_SetUpAnEvent(XPM_EVENT_SW_CHANGEPC);
#endif
    if(XPM_NO_COUNTERS_AVAILABLE != CntrId3) {
		xil_printf("PMU example has FAILED\r\n");
		goto END;
	}
	/* Confirm that the SW increment event counter is 3 */
    if (3 != CntVal) {
		xil_printf("PMU example has FAILED. CntVal = %d\r\n",CntVal);
		goto END;
	}

	if ((CntVal1 == 0 || CntVal2 == 0)) {
		xil_printf("PMU example has FAILED\r\n");
		goto END;
	}
	/* Print the counter values  */
	xil_printf("Instruction Executed Event Cntr = %d\r\n", CntVal1);
	xil_printf("Data Read Event Cntr = %d\r\n", CntVal2);

	xil_printf("PMU example has PASSED\r\n");

END:
	/* Disable and reset all counters for reuse */
    Xpm_ResetEventCounters();
    Xpm_DisableEventCounters();

    return 0;
}
#endif
