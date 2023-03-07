/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xpm_counter_arm_example.c
*
* Implements example that demonstrates usage of PMU counters and the
* available APIs provided through xpm_counter.c. This example can be used all
* AMD Xilinx supported ARM platforms except CortexA53/A72/A78 32 bit.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 8.2   asa  2/23/23  First version of ARM PMU example based out of the
*                     older PMU example. The older PMU example is replaced
*                     with this new one.
* </pre>
******************************************************************************/
#if !defined (__MICROBLAZE__) && !defined(ARMA53_32)
#include "xil_printf.h"
#include "xpm_counter.h"
#include "xstatus.h"
#include "xparameters.h"

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
#if defined(VERSAL_NET) && !defined (ARMR52)
    u32 RegVal;
#endif

    xil_printf("Start of PMU Example\n\r");

	/* Disable and reset all counters to start from a known state */
	Xpm_DisableEventCounters();
    Xpm_ResetEventCounters();

#if defined(VERSAL_NET) && !defined (ARMR52)
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
#elif defined (ARMR5) || defined (ARMR52)
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
#elif defined (ARMR5) || defined (ARMR52)
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
    isb();

	/* Create some instruction read and instruction executed events */
	DummyWrite = 1;
    DummyWrite1 = 2;
	DummyWrite2 = DummyWrite + DummyWrite1;

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


	for (Count = 3; Count < XPM_CTRCOUNT; Count++) {
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


	/*
	 * Read the instructions executed event one more time.
	 * Since all three counters are being used by now, it should
	 * return an error.
	 */
#if defined (__aarch64__)
	CntrId3 = Xpm_SetUpAnEvent(XPM_EVENT_INSTR_RETIRED);
#elif defined (ARMR5) || defined (ARMR52)
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
#endif /* #if !defined (__MICROBLAZE__) && !defined(ARMA53_32) */
