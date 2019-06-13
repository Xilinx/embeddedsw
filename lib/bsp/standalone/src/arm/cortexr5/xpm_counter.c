/******************************************************************************
*
* Copyright (C) 2014 - 2019 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpm_counter.c
*
* This file contains APIs for configuring and controlling the Cortex-R5
* Performance Monitor Events. For more information about the event counters,
* see xpm_counter.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 5.00  pkp  02/10/14 Initial version
* 6.2   mus  01/27/17 Updated to support IAR compiler
* 7.1   aru  04/15/19 Updated the events correctly
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xpm_counter.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

typedef const u32 PmcrEventCfg32[XPM_CTRCOUNT];

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions *****************************/



/************************** Function Prototypes ******************************/

void Xpm_DisableEventCounters(void);
void Xpm_EnableEventCounters (void);
void Xpm_ResetEventCounters (void);

/******************************************************************************/

/****************************************************************************/
/**
*
* @brief    This function disables the Cortex R5 event counters.
*
* @param	None.
*
* @return	None.
*
*****************************************************************************/
void Xpm_DisableEventCounters(void)
{
	/* Disable the event counters */
	mtcp(XREG_CP15_COUNT_ENABLE_CLR, 0x3f);
}

/****************************************************************************/
/**
*
* @brief    This function enables the Cortex R5 event counters.
*
* @param	None.
*
* @return	None.
*
*****************************************************************************/
void Xpm_EnableEventCounters(void)
{
	/* Enable the event counters */
	mtcp(XREG_CP15_COUNT_ENABLE_SET, 0x3f);
}

/****************************************************************************/
/**
*
* @brief    This function resets the Cortex R5 event counters.
*
* @param	None.
*
* @return	None.
*
*****************************************************************************/
void Xpm_ResetEventCounters(void)
{
	u32 Reg;

#ifdef __GNUC__
	Reg = mfcp(XREG_CP15_PERF_MONITOR_CTRL);
#elif defined (__ICCARM__)
    mfcp(XREG_CP15_PERF_MONITOR_CTRL, Reg);
#else
	{ register u32 C15Reg __asm(XREG_CP15_PERF_MONITOR_CTRL);
	  Reg = C15Reg; }
#endif
	Reg |= (1U << 2U); /* reset event counters */
	mtcp(XREG_CP15_PERF_MONITOR_CTRL, Reg);
}

/****************************************************************************/
/**
*
* @brief    This function configures the Cortex R5 event counters controller,
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
