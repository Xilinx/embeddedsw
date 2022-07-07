/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpm_counter.h
*
* @addtogroup armv8_event_counter_apis ARMv8-A Event Counters Functions
*
* ARMv8 event counter functions can be utilized to configure and control
* the ARMv8-A based processor's performance monitor events.
* ARMv8-A Performance Monitor has 6 event counters which can be used to
* count a variety of events described in ARMv8-A TRM. The xpm_counter.h file
* defines configurations XPM_CNTRCFGx which can be used to program the event
* counters to count a set of events.
*
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 8.0   mus  06/13/22 Initial version
* </pre>
*
******************************************************************************/

/**
 *@cond nocomments
 */

#ifndef XPMCOUNTER_H /* prevent circular inclusions */
#define XPMCOUNTER_H /* by using protection macros */

/***************************** Include Files ********************************/

#include <stdint.h>
#include "xpseudo_asm.h"
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************************** Constant Definitions ****************************/

/* Number of performance counters */
#define XPM_CTRCOUNT 6U

/* The following constants define the ARMv8-A Performance Monitor Events */

#define XPM_EVENT_SOFTINCR		0x00
#define XPM_EVENT_L1INSTRCACHE_REFILL	0x01
#define XPM_EVENT_L1INSTRTLB_REFILL	0x02
#define XPM_EVENT_L1DATACACHE_REFILL	0x03
#define XPM_EVENT_DATACACHEACCESS 	0x04
#define XPM_EVENT_L1DATATLB_REFILL	0x05
#define XPM_EVENT_LOAD_RETIRED		0x06
#define XPM_EVENT_STR_RETIRED		0x07
#define XPM_EVENT_INSTR_RETIRED		0x08
#define XPM_EVENT_EXCEPTION_TAKEN	0x09
#define XPM_EVENT_EXCEPTION_RETURN	0x0A
#define XPM_EVENT_CONTEXTIDR_WRITE_RETIRED	0x0B
#define XPM_EVENT_PC_WRITE_RETIRED	0x0C
#define XPM_EVENT_BRANCH_IMMEDIATE_RETIRED	0x0D
#define XPM_EVENT_BRANCH_RETURN_RETIRED	0x0E
#define XPM_EVENT_UNALIGNED_LDRSTR_RETIRED	0x0F
#define XPM_EVENT_BRANCHMISS_PREDICTED	0x10
#define XPM_EVENT_CPU_CYCLES		0x11
#define XPM_EVENT_BRANCH_PREDICTED	0x12
#define XPM_EVENT_DATAMEM_ACCESS	0x13
#define XPM_EVENT_L1DATACACHE_ACCESS	0x14
#define XPM_EVENT_L1DATACACHE_WB	0x15
#define XPM_EVENT_L2DATACACHE_ACCESS	0x16
#define XPM_EVENT_L2DATACACHE_REFILL	0x17
#define XPM_EVENT_L2DATACACHE_WB	0x18
#define XPM_EVENT_BUS_ACCESS		0x19
#define XPM_EVENT_MEM_ERROR		0x1A
#define XPM_EVENT_INSTR_SPECULATED	0x1B
#define XPM_EVENT_TTBR_WRITE_RETIRED	0x1C
#define XPM_EVENT_BUS_CYCLES		0x1D
#define XPM_EVENT_CHAINED_EVENTS	0x1E
#define XPM_EVENT_L1DATACACHE_ALLOCATE	0x1F
#define XPM_EVENT_L2DATACACHE_ALLOCATE	0x20
#define XPM_EVENT_BR_RETIRED		0x21
#define XPM_EVENT_BR_MISPRED_RETIRED	0x22
#define XPM_EVENT_STALL_FRONTEND	0x23
#define XPM_EVENT_STALL_BACKEND		0x24
#define XPM_EVENT_L1DATATLB_ACCESS	0x25
#define XPM_EVENT_L1INSTRTLB_ACCESS	0x26
#define XPM_EVENT_L2INSTRCACHE_ACCESS	0x27
#define XPM_EVENT_L2INSTRCACHE_REFILL	0x28
#define XPM_EVENT_L3DATACACHE_ALLOCATE	0x29
#define XPM_EVENT_L3DATACACHE_REFILL	0x2A
#define XPM_EVENT_L3DATACACHE_ACCESS	0x2B
#define XPM_EVENT_L3DATACACHE_WB	0x2C
#define XPM_EVENT_L2DATATLB_REFILL	0x2D
#define XPM_EVENT_L2INSTRTLB_REFILL	0x2E
#define XPM_EVENT_L2DATATLB_ACCESS	0x2F
#define XPM_EVENT_L2INSTRTLB_ACCESS	0x30
#define XPM_EVENT_REMOTE_ACCESS		0x31
#define XPM_EVENT_LASTLEVELCACHE_ACCESS		0x32
#define XPM_EVENT_LASTLEVELCACHE_MISS		0x33
#define XPM_EVENT_DATATLB_WALK		0x34
#define XPM_EVENT_INSTRTLB_WALK		0x35
#define XPM_EVENT_LASTLEVELCACHE_RD	0x36
#define XPM_EVENT_LASTLEVELCACHE_MISS_RD	0x37
#define XPM_EVENT_REMOTE_ACCESS_RD		0x38

#define XPM_NO_COUNTERS_AVAILABLE 	0xFFU
#define XPM_MAX_EVENTHANDLER_ID		0x5U
#define XPM_EVENT_CNTRS_BIT_MASK	0x3FU
#define XPM_ALL_EVENT_CNTRS_IN_USE	0x3FU
#define XPM_EVENT_CNTRS_MASK		0x3FU

#if defined(VERSAL_NET)
#define XPM_MDCR_EL3_SPME_MASK		0x20000U
#endif

/**************************** Type Definitions ******************************/
/**
 *@endcond
 */
/***************** Macros (Inline Functions) Definitions ********************/
#define Xpm_ReadCycleCounterVal()	mfcp(PMCCNTR_EL0)
/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

/* Interface functions to access performance counters from abstraction layer */
u32 Xpm_DisableEvent(u32 EventCntrId);
u32 Xpm_SetUpAnEvent(u32 EventID);
u32 Xpm_GetEventCounter(u32 EventCntrId, u32 *CntVal);
void Xpm_DisableEventCounters(void);
void Xpm_EnableEventCounters (void);
void Xpm_ResetEventCounters (void);

/* This is helper function for sleep/usleep APIs */
void Xpm_SleepPerfCounter(u32 delay, u64 frequency);

#ifdef __cplusplus
}
#endif

#endif

/**
* @} End of "addtogroup armv8_event_counter_apis".
*/
