/******************************************************************************
* Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpm_counter.c
*
* This file contains APIs for configuring and controlling the Performance
* Monitor Events for ARM based processors supported by standalone BSP.
* For more information about the event counters, see xpm_counter.h.
* The file contains APIs to setup an event, return the event counter value,
* disable event(s), enable events, reset event counters.
* It also provides a helper function: Xpm_SleepPerfCounter that is used to
* implement sleep routines in non-OS environment.
* It also contains two APIs which are being deprecated. Users are advised
* not to use them.
* On usage of these APIs, please refer to xpm_conter_example.c file which can
* be found at standalone example folder.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 8.00  mus  07/07/22 Initial version
* 8.00  mus  07/14/22 Existing PMU APIs dont have support for CortexA53 32
*                     bit processor, added check to skip PMU APIs
*                     compilation in case of CortexA53 32 bit BSP.
* 8.1   adk  03/13/23 Include xstatus.h when xiltimer is enabled.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#if !defined(ARMA53_32)
#include "xpm_counter.h"
#ifndef XPAR_XILTIMER_ENABLED
#include "xil_sleeptimer.h"
#include "xtime_l.h"
#else
#include "xstatus.h"
#endif

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/
typedef const u32 PmcrEventCfg32[XPM_CTRCOUNT];

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions *****************************/


/******************************************************************************/

/****************************************************************************/
/**
*
* @brief    This function disables the event counters.
*
* @return	None.
*
*****************************************************************************/
void Xpm_DisableEventCounters(void)
{
    u32 RegVal;

#if defined(__aarch64__)
    RegVal = mfcp(PMCNTENCLR_EL0);
#else
    /* Disable the event counters */
#ifdef __GNUC__
    RegVal = mfcp(XREG_CP15_COUNT_ENABLE_CLR);
#elif defined (__ICCARM__)
    mfcp(XREG_CP15_COUNT_ENABLE_CLR, RegVal);
#else
    { register u32 C15Reg __asm(XREG_CP15_COUNT_ENABLE_CLR);
      RegVal = C15Reg; }
#endif
#endif

    RegVal &= 0x7FFFFFFF;
    RegVal |= (XPM_EVENT_CNTRS_MASK & 0x7FFFFFFF);
#if defined(__aarch64__)
    mtcp(PMCNTENCLR_EL0, RegVal);
#else
    mtcp(XREG_CP15_COUNT_ENABLE_CLR, RegVal);
#endif
}

/****************************************************************************/
/**
*
* @brief    This function enables the event counters.
*
* @return	None.
*
*****************************************************************************/
void Xpm_EnableEventCounters(void)
{
    u32 RegVal;

#if defined(__aarch64__)
    RegVal = mfcp(PMCNTENSET_EL0);
#else
    /* Enable the event counters */
#ifdef __GNUC__
    RegVal = mfcp(XREG_CP15_COUNT_ENABLE_SET);
#elif defined (__ICCARM__)
    mfcp(XREG_CP15_COUNT_ENABLE_SET, RegVal);
#else
    { register u32 C15Reg __asm(XREG_CP15_COUNT_ENABLE_SET);
      RegVal = C15Reg; }
#endif
#endif
    RegVal |= XPM_EVENT_CNTRS_MASK;
#if defined(__aarch64__)
    mtcp(PMCNTENSET_EL0, RegVal);
#if defined(VERSAL_NET)
	RegVal = mfcp(MDCR_EL3);
	mtcp(MDCR_EL3, RegVal | XPM_MDCR_EL3_SPME_MASK);
#endif
#else
    mtcp(XREG_CP15_COUNT_ENABLE_SET, RegVal);
#endif
}

/****************************************************************************/
/**
*
* @brief    This function resets the event counters.
*
* @return	None.
*
*****************************************************************************/
void Xpm_ResetEventCounters(void)
{
    u32 Reg;

#if defined(__aarch64__)
    Reg = mfcp(PMCR_EL0);
#else
#ifdef __GNUC__
    Reg = mfcp(XREG_CP15_PERF_MONITOR_CTRL);
#elif defined (__ICCARM__)
    mfcp(XREG_CP15_PERF_MONITOR_CTRL, Reg);
#else
	{ register u32 C15Reg __asm(XREG_CP15_PERF_MONITOR_CTRL);
	  Reg = C15Reg; }
#endif
#endif

    Reg |= (1U << 1U); /* reset event counters */

#if defined(__aarch64__)
    mtcp(PMCR_EL0, Reg);
#else
    mtcp(XREG_CP15_PERF_MONITOR_CTRL, Reg);
#endif
}

/*****************************************************************************/
/**
 *
 * Disables the requested event counter.
 *
 *
 * @param	EventCntrId: Event Counter ID. The counter ID is the same that
 *          was earlier returned through a call to Xpm_SetUpAnEvent.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if the passed Counter ID is invalid
 *
 ******************************************************************************/
u32 Xpm_DisableEvent(u32 EventCntrId)
{
    u32 Counters;
    u32 CntrMask = 0x1U;

    if (EventCntrId > XPM_MAX_EVENTHANDLER_ID) {
        xil_printf("Invalid EventHandlerID\r\n");
        return XST_FAILURE;
    } else {
        CntrMask = CntrMask << EventCntrId;
#if defined(__aarch64__)
        Counters = mfcp(PMCNTENCLR_EL0);
#else
#ifdef __GNUC__
        Counters = mfcp(XREG_CP15_COUNT_ENABLE_CLR);
#elif defined (__ICCARM__)
        mfcp(XREG_CP15_COUNT_ENABLE_CLR, Counters);
#else
        { register u32 C15Reg __asm(XREG_CP15_COUNT_ENABLE_CLR);
		Counters = C15Reg; }
#endif
#endif
		Counters &= ~CntrMask;
#if defined(__aarch64__)
        mtcp(PMCNTENCLR_EL0, Counters);
#else
        mtcp(XREG_CP15_COUNT_ENABLE_CLR, Counters);
#endif
        return XST_SUCCESS;
    }
}

/*****************************************************************************/
/**
 *
 * Sets up one of the event counters to count events based on the Event ID
 * passed. For supported Event IDs please refer xpm_counter.h.
 * Upon invoked, the API searches for an available counter. After finding
 * one, it sets up the counter to count events for the requested event.
 *
 *
 * @param	EventID: For valid values, please refer xpm_counter.h.
 *
 * @return
 *		- Counter Number if successful. For Cortex-R5, valid return values are
 *        0, 1, or 2, and for others valid values are 0 to 5
 *		- XPM_NO_COUNTERS_AVAILABLE (0xFF) if all counters are being used
 *
 ******************************************************************************/
u32 Xpm_SetUpAnEvent(u32 EventID)
{
    u32 Counters;
    u32 OriginalCounters;
    u32 Index;

#if defined(__aarch64__)
    OriginalCounters = mfcp(PMCNTENSET_EL0);
#else
#ifdef __GNUC__
    OriginalCounters = mfcp(XREG_CP15_COUNT_ENABLE_SET);
#elif defined (__ICCARM__)
    mfcp(XREG_CP15_COUNT_ENABLE_SET, OriginalCounters);
#else
    { register u32 C15Reg __asm(XREG_CP15_COUNT_ENABLE_SET);
	OriginalCounters = C15Reg; }
#endif
#endif

    OriginalCounters &= XPM_EVENT_CNTRS_BIT_MASK;
    Counters = OriginalCounters;
    if (Counters == XPM_ALL_EVENT_CNTRS_IN_USE) {
        xil_printf("No counters available\r\n");
        return XPM_NO_COUNTERS_AVAILABLE;
	} else {
        for(Index = 0U; Index < XPM_CTRCOUNT; Index++) {
	     if ((Counters & 0x1U) == 0x0U) {
			    break;
            } else {
                Counters = Counters >> 0x1U;
            }
        }
    }

#if defined(__aarch64__)
   /* Select event counter */
    mtcp(PMSELR_EL0, Index);
    /* Set the event */
    mtcp( PMXEVTYPER_EL0, EventID);
    /* Enable event counter */
    mtcp(PMCNTENSET_EL0, OriginalCounters | (1U << Index));
#else
    /* Select event counter */
    mtcp(XREG_CP15_EVENT_CNTR_SEL, Index);
    /* Set the event */
    mtcp(XREG_CP15_EVENT_TYPE_SEL, EventID);
    /* Enable event counter */
    mtcp(XREG_CP15_COUNT_ENABLE_SET, OriginalCounters | ((u32)1U << Index));
#endif
    return Index;
}

/*****************************************************************************/
/**
 *
 * Reads the counter value for the requested counter ID. This is used to read
 * the number of events that has been counted for the requsted event ID.
 * This can only be called after a call to Xpm_SetUpAnEvent.
 *
 *
 * @param	EventCntrId: The counter ID is the same that was earlier
 *          returned through a call to Xpm_SetUpAnEvent.
 *          Cortex-R5 supports only 3 counters. The valid values are 0, 1,
 *          or 2 for other processors valid values are 0 to 5.
 * @param	CntVal: Pointer to a 32 bit unsigned int type. This is used to return
 *          the event counter value.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if the passed Counter ID is invalid
 *
 ******************************************************************************/
u32 Xpm_GetEventCounter(u32 EventCntrId, u32 *CntVal)
{
    if (EventCntrId > XPM_MAX_EVENTHANDLER_ID) {
        xil_printf("Invalid Event Handler ID\r\n");
        return XST_FAILURE;
    } else {
#if defined(__aarch64__)
	mtcp(PMSELR_EL0, EventCntrId);
        *CntVal = mfcp(PMXEVCNTR_EL0);
#else
        mtcp(XREG_CP15_EVENT_CNTR_SEL, EventCntrId);
#ifdef __GNUC__
        *CntVal = mfcp(XREG_CP15_PERF_MONITOR_COUNT);
#elif defined (__ICCARM__)
        mfcp(XREG_CP15_PERF_MONITOR_COUNT, (*CntVal));
#else
        { register u32 C15Reg __asm(XREG_CP15_PERF_MONITOR_COUNT);
	    *CntVal = C15Reg; }
#endif
#endif
        return XST_SUCCESS;
    }
}

/****************************************************************************/
/**
*
* @brief    This is helper function used by sleep/usleep APIs to generate
*           delay in sec/usec
*
* @param    delay - delay time in sec/usec
* @param    frequency - Number of countes in second/micro second
*
* @return       None.
*
*****************************************************************************/
#ifndef XPAR_XILTIMER_ENABLED
void Xpm_SleepPerfCounter(u32 delay, u64 frequency)
{
    u64 tEnd = 0U;
    u64 tCur = 0U;
    XCntrVal TimeHighVal = 0U;
    XCntrVal TimeLowVal1 = 0U;
    XCntrVal TimeLowVal2 = 0U;

#if defined (__GNUC__)
    TimeLowVal1 = Xpm_ReadCycleCounterVal();
#elif defined (__ICCARM__)
    Xpm_ReadCycleCounterVal(TimeLowVal1);
#endif
    tEnd = (u64)TimeLowVal1 + ((u64)(delay) * frequency);
    do {
#if defined (__GNUC__)
        TimeLowVal2 = Xpm_ReadCycleCounterVal();
#elif defined (__ICCARM__)
        Xpm_ReadCycleCounterVal(TimeLowVal2);
#endif
        if (TimeLowVal2 < TimeLowVal1) {
		    TimeHighVal++;
        }
        TimeLowVal1 = TimeLowVal2;
        tCur = (((u64) TimeHighVal) << XSLEEP_TIMER_REG_SHIFT) |
		                                           (u64)TimeLowVal2;
        }while (tCur < tEnd);
}
#endif
#if !defined(__aarch64__)
/****************************************************************************/
/**
*
* @brief    This function configures the event counters controller,
*           with the event codes, in a configuration selected by the user and
*           enables the counters.
*
* @param	PmcrCfg: Configuration value based on which the event counters
*		    are configured.XPM_CNTRCFG* values defined in xpm_counter.h can
*		    be utilized for setting configuration
*
* @return	None.
*
*****************************************************************************/
void Xpm_SetEvents(s32 PmcrCfg)
{
	u32 Counter;
	static PmcrEventCfg32 PmcrEvents[] = {

#if defined(ARMR5)
		{
			XPM_EVENT_SOFTINCR,
			XPM_EVENT_INSTRCACHEMISS,
			XPM_EVENT_DATACACHEMISS
		},
		{
			XPM_EVENT_DATACACHEACCESS,
			XPM_EVENT_DATAREAD,
			XPM_EVENT_DATAWRITE
		},
		{
			XPM_EVENT_INSTR,
			XPM_EVENT_DUALINSTR,
			XPM_EVENT_EXCEPTION
		},
		{
			XPM_EVENT_EXCEPTIONRET,
			XPM_EVENT_CHANGETOCONTEXID,
			XPM_EVENT_SWCHANGE
		},
		{
			XPM_EVENT_IMMEDIATEINSTR,
			XPM_EVENT_PROCEDURERET,
			XPM_EVENT_UNALIGNACCESS
		},
		{
			XPM_EVENT_BRANCHMISPREDICT,
			XPM_EVENT_CLOCKCYCLES,
			XPM_EVENT_BRANCHPREDICT
		},
		{
			XPM_EVENT_INSTRSTALL,
			XPM_EVENT_DATASTALL,
			XPM_EVENT_DATACACHEWRITE
		},
		{
			XPM_EVENT_EXTERNALMEMREQ,
			XPM_EVENT_LSUSTALL,
			XPM_EVENT_FORCEDRAINSTORE
		},
		{
			XPM_EVENT_INSTRTAGPARITY,
			XPM_EVENT_INSTRDATAPARITY,
			XPM_EVENT_DATATAGPARITY
		},
		{
			XPM_EVENT_DATADATAPARITY,
			XPM_EVENT_TCMERRORPREFETCH,
			XPM_EVENT_TCMERRORSTORE
		},
		{
			XPM_EVENT_INSTRCACHEACCESS,
			XPM_EVENT_DUALISSUEA,
			XPM_EVENT_DUALISSUEB
		},
		{
			XPM_EVENT_DUALISSUEOTHER,
			XPM_EVENT_FPA,
			XPM_EVENT_DATACACHEDATAERROR
		},
		{
			XPM_EVENT_DATACACHETAGERROR,
			XPM_EVENT_PROCESSORLIVELOCK,
			XPM_EVENT_ATCMMULTIBITERROR
		},
		{
			XPM_EVENT_B0TCMMULTIBITERROR,
			XPM_EVENT_B1TCMMULTIBITERROR,
			XPM_EVENT_ATCMSINGLEBITERROR
		},
		{
			XPM_EVENT_B0TCMSINGLEBITERROR,
			XPM_EVENT_B1TCMSINGLEBITERROR,
			XPM_EVENT_TCMERRORLSU
		},
		{
			XPM_EVENT_TCMERRORPFU,
			XPM_EVENT_TCMFATALERRORAXI,
			XPM_EVENT_TCMERRORAXI
		},
#else
	{
		XPM_EVENT_SOFTINCR,
		XPM_EVENT_INSRFETCH_CACHEREFILL,
		XPM_EVENT_INSTRFECT_TLBREFILL,
		XPM_EVENT_DATA_CACHEREFILL,
		XPM_EVENT_DATA_CACHEACCESS,
		XPM_EVENT_DATA_TLBREFILL
	},
	{
		XPM_EVENT_DATA_READS,
		XPM_EVENT_DATA_WRITE,
		XPM_EVENT_EXCEPTION,
		XPM_EVENT_EXCEPRETURN,
		XPM_EVENT_CHANGECONTEXT,
		XPM_EVENT_SW_CHANGEPC
	},
	{
		XPM_EVENT_IMMEDBRANCH,
		XPM_EVENT_UNALIGNEDACCESS,
		XPM_EVENT_BRANCHMISS,
		XPM_EVENT_CLOCKCYCLES,
		XPM_EVENT_BRANCHPREDICT,
		XPM_EVENT_JAVABYTECODE
	},
	{
		XPM_EVENT_SWJAVABYTECODE,
		XPM_EVENT_JAVABACKBRANCH,
		XPM_EVENT_COHERLINEMISS,
		XPM_EVENT_COHERLINEHIT,
		XPM_EVENT_INSTRSTALL,
		XPM_EVENT_DATASTALL
	},
	{
		XPM_EVENT_MAINTLBSTALL,
		XPM_EVENT_STREXPASS,
		XPM_EVENT_STREXFAIL,
		XPM_EVENT_DATAEVICT,
		XPM_EVENT_NODISPATCH,
		XPM_EVENT_ISSUEEMPTY
	},
	{
		XPM_EVENT_INSTRRENAME,
		XPM_EVENT_PREDICTFUNCRET,
		XPM_EVENT_MAINEXEC,
		XPM_EVENT_SECEXEC,
		XPM_EVENT_LDRSTR,
		XPM_EVENT_FLOATRENAME
	},
	{
		XPM_EVENT_NEONRENAME,
		XPM_EVENT_PLDSTALL,
		XPM_EVENT_WRITESTALL,
		XPM_EVENT_INSTRTLBSTALL,
		XPM_EVENT_DATATLBSTALL,
		XPM_EVENT_INSTR_uTLBSTALL
	},
	{
		XPM_EVENT_DATA_uTLBSTALL,
		XPM_EVENT_DMB_STALL,
		XPM_EVENT_INT_CLKEN,
		XPM_EVENT_DE_CLKEN,
		XPM_EVENT_INSTRISB,
		XPM_EVENT_INSTRDSB
	},
	{
		XPM_EVENT_INSTRDMB,
		XPM_EVENT_EXTINT,
		XPM_EVENT_PLE_LRC,
		XPM_EVENT_PLE_LRS,
		XPM_EVENT_PLE_FLUSH,
		XPM_EVENT_PLE_CMPL
	},
	{
		XPM_EVENT_PLE_OVFL,
		XPM_EVENT_PLE_PROG,
		XPM_EVENT_PLE_LRC,
		XPM_EVENT_PLE_LRS,
		XPM_EVENT_PLE_FLUSH,
		XPM_EVENT_PLE_CMPL
	},
	{
		XPM_EVENT_DATASTALL,
		XPM_EVENT_INSRFETCH_CACHEREFILL,
		XPM_EVENT_INSTRFECT_TLBREFILL,
		XPM_EVENT_DATA_CACHEREFILL,
		XPM_EVENT_DATA_CACHEACCESS,
		XPM_EVENT_DATA_TLBREFILL
	},
#endif
	};
	const u32 *ptr = PmcrEvents[PmcrCfg];

	Xpm_DisableEventCounters();

	for(Counter = 0U; Counter < XPM_CTRCOUNT; Counter++) {

		/* Select event counter */
		mtcp(XREG_CP15_EVENT_CNTR_SEL, Counter);

		/* Set the event */
		mtcp(XREG_CP15_EVENT_TYPE_SEL, ptr[Counter]);
	}

	Xpm_ResetEventCounters();
	Xpm_EnableEventCounters();
}

/****************************************************************************/
/**
*
* @brief    This function disables the event counters and returns the counter
*           values.
*
* @param	PmCtrValue: Pointer to an array of type u32 PmCtrValue[6].
*		    It is an output parameter which is used to return the PM
*		    counter values.
*
* @return	None.
*
*****************************************************************************/
void Xpm_GetEventCounters(u32 *PmCtrValue)
{
	u32 Counter;

	Xpm_DisableEventCounters();

	for(Counter = 0U; Counter < XPM_CTRCOUNT; Counter++) {

		mtcp(XREG_CP15_EVENT_CNTR_SEL, Counter);

#ifdef __GNUC__
		PmCtrValue[Counter] = mfcp(XREG_CP15_PERF_MONITOR_COUNT);
#elif defined (__ICCARM__)
        mfcp(XREG_CP15_PERF_MONITOR_COUNT, PmCtrValue[Counter]);
#else
		{ register u32 Cp15Reg __asm(XREG_CP15_PERF_MONITOR_COUNT);
		  PmCtrValue[Counter] = Cp15Reg; }
#endif
	}
}
#endif
#endif
