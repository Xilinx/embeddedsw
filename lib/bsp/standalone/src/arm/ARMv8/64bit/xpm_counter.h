/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
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
* 8.2   asa  02/23/23 Add PMU macros for A53, A72 and A78.
* 9.0   ml   03/03/23 Added description to fix doxygen warnings.
* </pre>
*
******************************************************************************/

/**
 *@cond nocomments
 */

#ifndef XPMCOUNTER_H /**< prevent circular inclusions */
#define XPMCOUNTER_H /**< by using protection macros */

/***************************** Include Files ********************************/

#include <stdint.h>
#include "xpseudo_asm.h"
#include "xil_types.h"
#ifndef SDT
#include "xparameters.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************************** Constant Definitions ****************************/

/* Number of performance counters */
#define XPM_CTRCOUNT 6U

/* The following constants define the ARMv8-A Performance Monitor Events */

#define XPM_EVENT_SOFTINCR						0x00U
#define XPM_EVENT_L1INSTRCACHE_REFILL			0x01U
#define XPM_EVENT_L1INSTRTLB_REFILL				0x02U
#define XPM_EVENT_L1DATACACHE_REFILL			0x03U
#define XPM_EVENT_DATACACHEACCESS 				0x04U
#define XPM_EVENT_L1DATATLB_REFILL				0x05U

#if defined (PLATFORM_ZYNQMP)
#define XPM_EVENT_LOAD_RETIRED					0x06U
#define XPM_EVENT_STR_RETIRED					0x07U
#endif

#define XPM_EVENT_INSTR_RETIRED					0x08U

#if defined (VERSAL_NET) || defined (PLATFORM_ZYNQMP)
#define XPM_EVENT_EXCEPTION_TAKEN				0x09U
#endif

#define XPM_EVENT_EXCEPTION_RETURN				0x0AU
#define XPM_EVENT_CONTEXTIDR_WRITE_RETIRED		0x0BU

#if defined (PLATFORM_ZYNQMP)
#define XPM_EVENT_PC_WRITE_RETIRED				0x0CU
#define XPM_EVENT_BRANCH_IMMEDIATE_RETIRED		0x0DU
#define XPM_EVENT_UNALIGNED_LDRSTR_RETIRED		0x0FU
#endif

#define XPM_EVENT_BRANCHMISS_PREDICTED			0x10U
#define XPM_EVENT_CPU_CYCLES					0x11U
#define XPM_EVENT_BRANCH_PREDICTED				0x12U
#define XPM_EVENT_DATAMEM_ACCESS				0x13U
#define XPM_EVENT_L1DATACACHE_ACCESS			0x14U
#define XPM_EVENT_L1DATACACHE_WB				0x15U
#define XPM_EVENT_L2DATACACHE_ACCESS			0x16U
#define XPM_EVENT_L2DATACACHE_REFILL			0x17U
#define XPM_EVENT_L2DATACACHE_WB				0x18U
#define XPM_EVENT_BUS_ACCESS					0x19U
#define XPM_EVENT_MEM_ERROR						0x1AU

#if defined (versal) || defined (VERSAL_NET)
#define XPM_EVENT_INSTR_SPECULATED				0x1BU
#define XPM_EVENT_TTBR_WRITE_RETIRED			0x1CU
#endif

#define XPM_EVENT_BUS_CYCLES					0x1DU
#define XPM_EVENT_CHAINED_EVENTS				0x1EU

#if defined (VERSAL_NET)
#define XPM_EVENT_L2DATACACHE_ALLOCATE			0x20U
#define XPM_EVENT_BR_RETIRED					0x21U
#define XPM_EVENT_BR_MISPRED_RETIRED			0x22U
#define XPM_EVENT_STALL_FRONTEND				0x23U
#define XPM_EVENT_STALL_BACKEND					0x24U
#define XPM_EVENT_L1DATATLB_ACCESS				0x25U
#define XPM_EVENT_L1INSTRTLB_ACCESS				0x26U
#define XPM_EVENT_L3DATACACHE_ALLOCATE			0x29U
#define XPM_EVENT_L3DATACACHE_REFILL			0x2AU
#define XPM_EVENT_L3DATACACHE_ACCESS			0x2BU
#define XPM_EVENT_L2DATATLB_REFILL				0x2DU
#define XPM_EVENT_L2DATATLB_ACCESS				0x2FU
#define XPM_EVENT_REMOTE_ACCESS					0x31U
#define XPM_EVENT_DATATLB_WALK					0x34U
#define XPM_EVENT_INSTRTLB_WALK					0x35U
#define XPM_EVENT_LASTLEVELCACHE_RD				0x36U
#define XPM_EVENT_LASTLEVELCACHE_MISS_RD		0x37U
#define XPM_EVENT_L1D_CACHE_LMISS_RD 			0x39U
#define XPM_EVENT_OP_RETIRED 					0x3AU
#define XPM_EVENT_OP_SPEC 						0x3BU
#define XPM_EVENT_STALL 						0x3CU
#define XPM_EVENT_STALL_SLOT_BACKEND 			0x3DU
#define XPM_EVENT_STALL_SLOT_FRONTEND 			0x3EU
#define XPM_EVENT_STALL_SLOT 					0x3FU
#endif

#if defined (versal) || defined (VERSAL_NET)
#define XPM_EVENT_L1_DCACHE_LD 					0x40U
#define XPM_EVENT_L1_DCACHE_ST 					0x41U
#define XPM_EVENT_L1_DCACHE_REFILL_LD 			0x42U
#define XPM_EVENT_L1_DCACHE_REFILL_ST 			0x43U
#endif

#if defined (VERSAL_NET)
#define XPM_EVENT_L1D_CACHE_REFILL_INNER 		0x44U
#define XPM_EVENT_L1D_CACHE_REFILL_OUTER 		0x45U
#endif

#if defined (versal) || defined (VERSAL_NET)
#define XPM_EVENT_L1D_CACHE_WB_VICTIM 			0x46U
#define XPM_EVENT_L1D_CACHE_WB_CLEAN 			0x47U
#define XPM_EVENT_L1D_CACHE_INVAL 				0x48U
#define XPM_EVENT_L1D_TLB_REFILL_RD 			0x4CU
#define XPM_EVENT_L1D_TLB_REFILL_WR 			0x4DU
#endif

#if defined (VERSAL_NET)
#define XPM_EVENT_L1D_TLB_RD 					0x4EU
#define XPM_EVENT_L1D_TLB_WR 					0x4FU
#endif

#if defined (versal) || defined (VERSAL_NET)
#define XPM_EVENT_L2D_CACHE_LD 					0x50U
#define XPM_EVENT_L2D_CACHE_ST 					0x51U
#define XPM_EVENT_L2D_CACHE_REFILL_LD 			0x52U
#define XPM_EVENT_L2D_CACHE_REFILL_ST 			0x53U
#define XPM_EVENT_CACHE_WRITEBACK_VICTIM 		0x56U
#define XPM_EVENT_CACHE_WRITEBACK_CLEAN_COH 	0x57U
#define XPM_EVENT_L2CACHE_INV 					0x58U
#endif

#if defined (VERSAL_NET)
#define XPM_EVENT_L2TLB_RD_REFILL 				0x5CU
#define XPM_EVENT_L2TLB_WR_REFILL 				0x5DU
#define XPM_EVENT_L2TLB_RD_REQ 					0x5EU
#define XPM_EVENT_L2TLB_WR_REQ 					0x5FU
#endif

#define XPM_EVENT_BUS_ACCESS_LD 				0x60U
#define XPM_EVENT_BUS_ACCESS_ST 				0x61U

#if defined (versal) || !defined (VERSAL_NET)
#define XPM_EVENT_BUS_ACCESS_SHARED 			0x62U
#define XPM_EVENT_BUS_ACCESS_NOT_SHARED 		0x63U
#define XPM_EVENT_BUS_ACCESS_NORMAL 			0x64U
#define XPM_EVENT_BUS_ACCESS_PERIPH 			0x65U
#endif

#if defined (versal) || defined (VERSAL_NET)
#define XPM_EVENT_MEM_ACCESS_LD 				0x66U
#define XPM_EVENT_MEM_ACCESS_ST 				0x67U
#define XPM_EVENT_UNALIGNED_LD_SPEC 			0x68U
#define XPM_EVENT_UNALIGNED_ST_SPEC 			0x69U
#define XPM_EVENT_UNALIGNED_LDST_SPEC 			0x6AU
#define XPM_EVENT_LDREX_SPEC 					0x6CU
#define XPM_EVENT_STREX_PASS_SPEC 				0x6DU
#define XPM_EVENT_STREX_FAIL_SPEC 				0x6EU
#define XPM_EVENT_STREX_SPEC 					0x6FU
#define XPM_EVENT_LD_SPEC 						0x70U
#define XPM_EVENT_ST_SPEC 						0x71U
#define XPM_EVENT_LDST_SPEC 					0x72U
#define XPM_EVENT_DP_SPEC 						0x73U
#define XPM_EVENT_ASE_SPEC 						0x74U
#define XPM_EVENT_VFP_SPEC 						0x75U
#define XPM_EVENT_PC_WRITE_SPEC 				0x76U
#define XPM_EVENT_CRYPTO_SPEC 					0x77U
#define XPM_EVENT_BR_IMMED_SPEC 				0x78U
#define XPM_EVENT_BR_RETURN_SPEC 				0x79U
#endif

#define XPM_EVENT_BR_INDIRECT_SPEC 				0x7AU

#if defined (versal) || defined (VERSAL_NET)
#define XPM_EVENT_ISB_SPEC 						0x7CU
#define XPM_EVENT_DSB_SPEC 						0x7DU
#define XPM_EVENT_DMB_SPEC 						0x7EU
#define XPM_EVENT_EXC_UNDEF 					0x81U
#define XPM_EVENT_EXC_SVC 						0x82U
#define XPM_EVENT_EXC_PABORT 					0x83U
#define XPM_EVENT_EXC_DABORT 					0x84U
#endif

#define XPM_EVENT_EXC_IRQ 						0x86U
#define XPM_EVENT_EXC_FIQ 						0x87U

#if defined (versal) || defined (VERSAL_NET)
#define XPM_EVENT_EXC_SMC 						0x88U
#define XPM_EVENT_EXC_HVC 						0x8AU
#define XPM_EVENT_EXC_TRAP_PABORT 				0x8BU
#define XPM_EVENT_EXC_TRAP_DABORT 				0x8CU
#define XPM_EVENT_EXC_TRAP_OTHER 				0x8DU
#define XPM_EVENT_EXC_TRAP_IRQ 					0x8EU
#define XPM_EVENT_EXC_TRAP_FIQ 					0x8FU
#define XPM_EVENT_RC_LD_SPEC 					0x90U
#define XPM_EVENT_RC_ST_SPEC 					0x91U
#endif

#if defined (VERSAL_NET)
#define XPM_EVENT_L3_CACHE_RD 					0xA0U
#define XPM_EVENT_CONST_FREQ_CYCLES 			0x4004U
#define XPM_EVENT_STALL_BACKEND_MEM 			0x4005U
#define XPM_EVENT_L1I_CACHE_LMISS 				0x4006U
#define XPM_EVENT_L2D_CACHE_LMISS_RD 			0x4009U
#define XPM_EVENT_L3D_CACHE_LMISS_RD 			0x400BU
#endif

#define XPM_NO_COUNTERS_AVAILABLE 				0xFFU
#define XPM_MAX_EVENTHANDLER_ID					0x5U
#define XPM_EVENT_CNTRS_BIT_MASK				0x3FU
#define XPM_ALL_EVENT_CNTRS_IN_USE				0x3FU
#define XPM_EVENT_CNTRS_MASK					0x3FU

#if defined(VERSAL_NET)
#define XPM_MDCR_EL3_SPME_MASK					(0x1U << 17U)
#endif

/**************************** Type Definitions ******************************/
/**
 *@endcond
 */
/***************** Macros (Inline Functions) Definitions ********************/

#define Xpm_ReadCycleCounterVal()	mfcp(PMCCNTR_EL0) /**< read PMU cycle
                                                            * counter value */
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
