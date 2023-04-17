/******************************************************************************
* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_exception.h
*
* This header file contains ARM Cortex A53,A9,R5 specific exception related APIs.
* For exception related functions that can be used across all Xilinx supported
* processors, please use xil_exception.h.
*
* @addtogroup arm_exception_apis ARM Processor Exception Handling
* @{
* ARM processors specific exception related APIs for cortex A53,A9 and R5 can
* utilized for enabling/disabling IRQ, registering/removing handler for
* exceptions or initializing exception vector table with null handler.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 5.2	pkp  	 28/05/15 First release
* 6.0   mus      27/07/16 Consolidated file for a53,a9 and r5 processors
* 6.7   mna      26/04/18 Add API Xil_GetExceptionRegisterHandler.
* 6.7   asa      18/05/18 Update signature of API Xil_GetExceptionRegisterHandler.
* 7.0   mus      01/03/19 Tweak Xil_ExceptionEnableMask and
*                         Xil_ExceptionDisableMask macros to support legacy
*                         examples for Cortexa72 EL3 exception level.
* 7.3   mus      04/15/20 Added Xil_EnableNestedInterrupts and
*                         Xil_DisableNestedInterrupts macros for ARMv8.
*                         For Cortexa72, these macro's would not be supported
*                         at EL3, as Cortexa72 is using GIC-500(GICv3),  which
*                         triggeres only FIQ at EL3. Fix for CR#1062506
* 7.6   mus      09/17/21 Updated flag checking to fix warning reported with
*                         -Wundef compiler option CR#1110261
* 7.7   mus      01/31/22 Few of the #defines in xil_exception.h in are treated
*                         in different way based on "versal" flag. In existing
*                         flow, this flag is defined only in xparameters.h and
*                         BSP compiler flags, it is not defined in application
*                         compiler flags. So, including xil_exception.h in
*                         application source file, without including
*                         xparameters.h results  in incorrect behavior.
*                         Including xparameters.h in xil_exception.h to avoid
*                         such issues. It fixes CR#1120498.
* 8.0  mus       02/24/22 Updated few macros to support legacy driver examples
*                         for CortexR52. This is needed, as by default scugic
*                         driver configures interrupts as group0 and CortexR52
*                         GIC triggers FIQ for group0 interrupts.
* 8.0  sk	 03/02/22 Define XExc_VectorTableEntry structure to fix
* 			  misra_c_2012_rule_5_6 violation.
* 8.0  sk	 03/02/22 Add XExc_VectorTable as extern to fix misra_c_2012_
* 			  rule_8_4 violation.
* 8.1  asa       02/12/23 Updated data abort and prefetch abort fault
*                         status reporting for ARMv7.
*						  Updated Sync and SError fault status reporting
*						  for ARMv8.
* </pre>
*
******************************************************************************/

/**
 *@cond nocomments
 */

#ifndef XIL_EXCEPTION_H /* prevent circular inclusions */
#define XIL_EXCEPTION_H /* by using protection macros */

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xpseudo_asm.h"
#include "bspconfig.h"
#ifndef SDT
#include "xparameters.h"
#include "xdebug.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions ****************************/

#define XIL_EXCEPTION_FIQ	XREG_CPSR_FIQ_ENABLE
#define XIL_EXCEPTION_IRQ	XREG_CPSR_IRQ_ENABLE
#define XIL_EXCEPTION_ALL	(XREG_CPSR_FIQ_ENABLE | XREG_CPSR_IRQ_ENABLE)

#define XIL_EXCEPTION_ID_FIRST					0U
#if defined (__aarch64__)
#define XIL_EXCEPTION_ID_SYNC_INT				1U
#define XIL_EXCEPTION_ID_IRQ_INT				2U
#define XIL_EXCEPTION_ID_FIQ_INT				3U
#define XIL_EXCEPTION_ID_SERROR_ABORT_INT		4U
#define XIL_EXCEPTION_ID_LAST					5U
#else
#define XIL_EXCEPTION_ID_RESET					0U
#define XIL_EXCEPTION_ID_UNDEFINED_INT			1U
#define XIL_EXCEPTION_ID_SWI_INT				2U
#define XIL_EXCEPTION_ID_PREFETCH_ABORT_INT		3U
#define XIL_EXCEPTION_ID_DATA_ABORT_INT			4U
#define XIL_EXCEPTION_ID_IRQ_INT				5U
#define XIL_EXCEPTION_ID_FIQ_INT				6U
#define XIL_EXCEPTION_ID_LAST					6U
#endif

#ifdef DEBUG

#if defined (__aarch64__)
#define ARMV8_SYNC_ERROR						0x01U
#define ARMV8_SERROR							0x02U

#define ARMV8_FAR_VALUE_VALID 					0x01U

#define ARMV8_ESR_EC_SHIFT						26U
#define ARMV8_ESR_EC_MASK				((0x3FU) << ARMV8_ESR_EC_SHIFT)
#define ARMV8_EXTRACT_ESR_EC(Val) 	\
					(((Val) & ARMV8_ESR_EC_MASK) >> ARMV8_ESR_EC_SHIFT)

#define ARMV8_ESR_IL_SHIFT						(25U)
#define ARMV8_ESR_IL_MASK				((1U) << ARMV8_ESR_IL_SHIFT)
#define ARMV8_EXTRACT_ESR_IL(Val) 	\
					(((Val) & ARMV8_ESR_IL_MASK) >> ARMV8_ESR_IL_SHIFT)

#define ARMV8_ESR_ISS_MASK				(ARMV8_ESR_IL_MASK - 1U)
#define ARMV8_EXTRACT_ESR_ISS(Val)		((Val) & ARMV8_ESR_ISS_MASK)


#define ARMV8_ESR_EC_UNKNOWN_ERR				0x00U
#define ARMV8_ESR_EC_FP_ASIMD					0x07U
#define ARMV8_ESR_ILL_EXECUTION_STATE			0x0EU
#define ARMV8_ESR_EC_DATA_ABORT_LOWER			0x24U
#define ARMV8_ESR_EC_DATA_ABORT					0x25U
#define ARMV8_ESR_EC_INS_ABORT_LOWER			0x20U
#define ARMV8_ESR_EC_INS_ABORT					0x21U
#define ARMV8_ESR_EC_PC_ALIGNMENT_FAULT			0x22U
#define ARMV8_ESR_EC_SP_ALIGNMENT_FAULT			0x26U
#define ARMV8_ESR_EC_SERROR						0x2FU


/* ISS field definitions shared by different classes */
#define ARMV8_ESR_ISS_WNR_SHIFT					(6U)
#define ARMV8_ESR_ISS_WNR_MASK			((1U) << ARMV8_ESR_ISS_WNR_SHIFT)

/* Asynchronous Error Type */

#define ARMV8_ESR_ISS_AET_SHIFT					(10U)
#define ARMV8_ESR_ISS_AET_MASK			((0x7U) << ARMV8_ESR_ISS_AET_SHIFT)
#define ARMV8_ESR_ISS_AET_UC			((0U) << ARMV8_ESR_ISS_AET_SHIFT)
#define ARMV8_ESR_ISS_AET_UEU			((1U) << ARMV8_ESR_ISS_AET_SHIFT)
#define ARMV8_ESR_ISS_AET_UEO			((2U) << ARMV8_ESR_ISS_AET_SHIFT)
#define ARMV8_ESR_ISS_AET_UER			((3U) << ARMV8_ESR_ISS_AET_SHIFT)
#define ARMV8_ESR_ISS_AET_CE			((6U) << ARMV8_ESR_ISS_AET_SHIFT)


/* Shared ISS fault status code(IFSC/DFSC) for Data/Instruction aborts */
#define ARMV8_ESR_ISS_FSC						(0x3FU)
#define ARMV8_ESR_ISS_CM_SHIFT					(8U)
#define ARMV8_ESR_ISS_CM_MASK 			((1U) << ARMV8_ESR_ISS_CM_SHIFT)

#define ARMV8_LEVEL_0_ADDR_FAULT				0x00U
#define ARMV8_LEVEL_1_ADDR_FAULT				0x01U
#define ARMV8_LEVEL_2_ADDR_FAULT				0x02U
#define ARMV8_LEVEL_3_ADDR_FAULT				0x03U
#define ARMV8_LEVEL_0_TRANS_FAULT				0x04U
#define ARMV8_LEVEL_1_TRANS_FAULT				0x05U
#define ARMV8_LEVEL_2_TRANS_FAULT				0x06U
#define ARMV8_LEVEL_3_TRANS_FAULT				0x07U
#define ARMV8_LEVEL_0_ACCS_FLAG_FAULT			0x08U
#define ARMV8_LEVEL_1_ACCS_FLAG_FAULT			0x09U
#define ARMV8_LEVEL_2_ACCS_FLAG_FAULT			0x0AU
#define ARMV8_LEVEL_3_ACCS_FLAG_FAULT			0x0BU
#define ARMV8_LEVEL_0_PERMISSION_FAULT			0x0CU
#define ARMV8_LEVEL_1_PERMISSION_FAULT			0x0DU
#define ARMV8_LEVEL_2_PERMISSION_FAULT			0x0EU
#define ARMV8_LEVEL_3_PERMISSION_FAULT			0x0FU

#define ARMV8_SYNC_EXT_ABORT_NOT_ON_TTW			0x10U
#define ARMV8_LEVEL_0_SYNC_EXT_ABORT			0x14U
#define ARMV8_LEVEL_1_SYNC_EXT_ABORT			0x15U
#define ARMV8_LEVEL_2_SYNC_EXT_ABORT			0x16U
#define ARMV8_LEVEL_3_SYNC_EXT_ABORT			0x17U

#define ARMV8_SYNC_PAR_OR_ECC_ERROR_NOT_ON_TTW	0x18U
#define ARMV8_LEVEL_0_SYNC_PAR_OR_ECC_ERROR		0x1CU
#define ARMV8_LEVEL_1_SYNC_PAR_OR_ECC_ERROR		0x1DU
#define ARMV8_LEVEL_2_SYNC_PAR_OR_ECC_ERROR		0x1EU
#define ARMV8_LEVEL_3_SYNC_PAR_OR_ECC_ERROR		0x1FU

#define ARMV8_ALIGNMENT_FAULT					0x21U
#define ARMV8_TLB_CONFLICT_ABORT				0x30U

#define ARMV8_ASYNC_SEEROR_INTERRUPT			0x11U

#else /* #if defined (__aarch64__) */

#define DATA_ABORT								0x00U
#define INS_PREFETCH_ABORT						0x01U
#define ARMV7_FAULT_STATUS_MASK_BIT3_0 			0x0FU
#define ARMV7_DFAR_VALUE_VALID 					0x01U
#define ARMV7_FAULT_STATUS_BIT10_MASK 			0x400U

#define ARMV7_BACKGROUND_FAULT 					0x00U
#define ARMV7_ALIGNMENT_FAULT 					0x01U
#define ARMV7_DEBUG_EVENT						0x02U
#define ARMV7_ACCESS_FLAG_FAULT 				0x03U
#define ARMV7_ICACHE_MAINTENANCE_FAULT  		0x04U
#define ARMV7_TRANSLATION_FAULT					0x05U
#define ARMV7_SYNC_EXT_ABORT					0x08U
#define ARMV7_DOMAIN_FAULT						0x09U
#define ARMV7_SYNC_ABORT_TRANSTAB_WALK			0x0CU
#define ARMV7_PERMISSION_FAULT 					0x0DU
#define ARMV7_TLB_CONFLICT_ABORT 				0x10U
#define ARMV7_ASYNC_EXT_ABORT					0x16U
#define ARMV7_MEM_ACS_ASYNC_PAR_ERR				0x18U
#define ARMV7_MEM_ACS_SYNC_PAR_ERR				0x19U
#define ARMV7_SYNCPAR_ERR_TRANSTAB_WALK			0x1CU

#define EXTRACT_BITS_10_AND_3_TO_0(Val) \
		((Val & (ARMV7_FAULT_STATUS_BIT10_MASK)) >> 6) | \
				(Val & ARMV7_FAULT_STATUS_MASK_BIT3_0)

#endif /* #if defined (__aarch64__) */
#endif /* #ifdef DEBUG */

/*
 * XIL_EXCEPTION_ID_INT is defined for all Xilinx processors.
 */
#if (defined (versal) && !defined(ARMR5) && EL3)
#define XIL_EXCEPTION_ID_INT    XIL_EXCEPTION_ID_FIQ_INT
#else
#define XIL_EXCEPTION_ID_INT	XIL_EXCEPTION_ID_IRQ_INT
#endif

/**************************** Type Definitions ******************************/

/**
 * This typedef is the exception handler function.
 */
typedef void (*Xil_ExceptionHandler)(void *data);
typedef void (*Xil_InterruptHandler)(void *data);

typedef struct {
        Xil_ExceptionHandler Handler;
        void *Data;
} XExc_VectorTableEntry;

extern XExc_VectorTableEntry XExc_VectorTable[];

/**
*@endcond
*/

/***************** Macros (Inline Functions) Definitions ********************/

/****************************************************************************/
/**
* @brief	Enable Exceptions.
*
* @param	Mask: Value for enabling the exceptions.
*
* @return	None.
*
* @note		If bit is 0, exception is enabled.
*			C-Style signature: void Xil_ExceptionEnableMask(Mask)
*
******************************************************************************/
#if (defined (versal) && !defined(ARMR5) && EL3)
/*
 * Cortexa72 processor in versal is coupled with GIC-500, and GIC-500 supports
 * only FIQ at EL3. Hence, tweaking this macro to always enable FIQ
 * ignoring argument passed by user.
 */
#define Xil_ExceptionEnableMask(Mask)	\
		(void)Mask; \
		mtcpsr(mfcpsr() & ~ ((XIL_EXCEPTION_FIQ) & XIL_EXCEPTION_ALL))
#elif defined (__GNUC__) || defined (__ICCARM__)
#define Xil_ExceptionEnableMask(Mask)	\
		mtcpsr(mfcpsr() & ~ ((Mask) & XIL_EXCEPTION_ALL))
#else
#define Xil_ExceptionEnableMask(Mask)	\
		{								\
		  register u32 Reg __asm("cpsr"); \
		  mtcpsr((Reg) & (~((Mask) & XIL_EXCEPTION_ALL))); \
		}
#endif
/****************************************************************************/
/**
* @brief	Enable the IRQ exception.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
#if (defined (versal) && !defined(ARMR5) && EL3)
#define Xil_ExceptionEnable() \
                Xil_ExceptionEnableMask(XIL_EXCEPTION_FIQ)
#else
#define Xil_ExceptionEnable() \
		Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ)
#endif

/****************************************************************************/
/**
* @brief	Disable Exceptions.
*
* @param	Mask: Value for disabling the exceptions.
*
* @return	None.
*
* @note		If bit is 1, exception is disabled.
*			C-Style signature: Xil_ExceptionDisableMask(Mask)
*
******************************************************************************/
#if (defined (versal) && !defined(ARMR5) && EL3)
/*
 * Cortexa72 processor in versal is coupled with GIC-500, and GIC-500 supports
 * only FIQ at EL3. Hence, tweaking this macro to always disable FIQ
 * ignoring argument passed by user.
 */
#define Xil_ExceptionDisableMask(Mask)	\
		mtcpsr(mfcpsr() | ((XIL_EXCEPTION_FIQ) & XIL_EXCEPTION_ALL))
#elif defined (__GNUC__) || defined (__ICCARM__)
#define Xil_ExceptionDisableMask(Mask)	\
		mtcpsr(mfcpsr() | ((Mask) & XIL_EXCEPTION_ALL))
#else
#define Xil_ExceptionDisableMask(Mask)	\
		{									\
		  register u32 Reg __asm("cpsr"); \
		  mtcpsr((Reg) | ((Mask) & XIL_EXCEPTION_ALL)); \
		}
#endif
/****************************************************************************/
/**
* Disable the IRQ exception.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
#define Xil_ExceptionDisable() \
		Xil_ExceptionDisableMask(XIL_EXCEPTION_IRQ)

#if ( defined (PLATFORM_ZYNQMP) && defined (EL3) && (EL3==1) )
/****************************************************************************/
/**
* @brief	Enable nested interrupts by clearing the I bit in DAIF.This
*			macro is defined for Cortex-A53 64 bit mode BSP configured to run
*			at EL3.. However,it is not defined for Versal Cortex-A72 BSP
*			configured to run at EL3. Reason is, Cortex-A72 is coupled
*			with GIC-500(GICv3 specifications) and it triggers only FIQ at EL3.
*
* @return   None.
*
* @note     This macro is supposed to be used from interrupt handlers. In the
*			interrupt handler the interrupts are disabled by default (I bit
*			is set as 1). To allow nesting of interrupts, this macro should be
*			used. It clears the I bit. Once that bit is cleared and provided the
*			preemption of interrupt conditions are met in the GIC, nesting of
*			interrupts will start happening.
*			Caution: This macro must be used with caution. Before calling this
*			macro, the user must ensure that the source of the current IRQ
*			is appropriately cleared. Otherwise, as soon as we clear the I
*			bit, there can be an infinite loop of interrupts with an
*			eventual crash (all the stack space getting consumed).
******************************************************************************/
#define Xil_EnableNestedInterrupts() \
                __asm__ __volatile__ ("mrs    X1, ELR_EL3"); \
                __asm__ __volatile__ ("mrs    X2, SPSR_EL3");  \
                __asm__ __volatile__ ("stp    X1,X2, [sp,#-0x10]!"); \
                __asm__ __volatile__ ("mrs    X1, DAIF");  \
                __asm__ __volatile__ ("bic    X1,X1,#(0x1<<7)");  \
                __asm__ __volatile__ ("msr    DAIF, X1");  \

/****************************************************************************/
/**
* @brief	Disable the nested interrupts by setting the I bit in DAIF. This
*			macro is defined for Cortex-A53 64 bit mode BSP configured to run
*			at EL3.
*
* @return   None.
*
* @note     This macro is meant to be called in the interrupt service routines.
*			This macro cannot be used independently. It can only be used when
*			nesting of interrupts have been enabled by using the macro
*			Xil_EnableNestedInterrupts(). In a typical flow, the user first
*			calls the Xil_EnableNestedInterrupts in the ISR at the appropriate
*			point. The user then must call this macro before exiting the interrupt
*			service routine. This macro puts the ARM back in IRQ mode and
*			hence sets back the I bit.
******************************************************************************/
#define Xil_DisableNestedInterrupts() \
                __asm__ __volatile__ ("ldp    X1,X2, [sp,#0x10]!"); \
                __asm__ __volatile__ ("msr    ELR_EL3, X1"); \
                __asm__ __volatile__ ("msr    SPSR_EL3, X2"); \
                __asm__ __volatile__ ("mrs    X1, DAIF");  \
                __asm__ __volatile__ ("orr    X1, X1, #(0x1<<7)"); \
                __asm__ __volatile__ ("msr    DAIF, X1");  \

#elif (defined (EL1_NONSECURE) && (EL1_NONSECURE==1))
/****************************************************************************/
/**
* @brief	Enable nested interrupts by clearing the I bit in DAIF.This
*			macro is defined for Cortex-A53 64 bit mode and Cortex-A72 64 bit
*			BSP configured to run at EL1 NON SECURE
*
* @return   None.
*
* @note     This macro is supposed to be used from interrupt handlers. In the
*			interrupt handler the interrupts are disabled by default (I bit
*			is set as 1). To allow nesting of interrupts, this macro should be
*			used. It clears the I bit. Once that bit is cleared and provided the
*			preemption of interrupt conditions are met in the GIC, nesting of
*			interrupts will start happening.
*			Caution: This macro must be used with caution. Before calling this
*			macro, the user must ensure that the source of the current IRQ
*			is appropriately cleared. Otherwise, as soon as we clear the I
*			bit, there can be an infinite loop of interrupts with an
*			eventual crash (all the stack space getting consumed).
******************************************************************************/
#define Xil_EnableNestedInterrupts() \
                __asm__ __volatile__ ("mrs    X1, ELR_EL1"); \
                __asm__ __volatile__ ("mrs    X2, SPSR_EL1");  \
                __asm__ __volatile__ ("stp    X1,X2, [sp,#-0x10]!"); \
                __asm__ __volatile__ ("mrs    X1, DAIF");  \
                __asm__ __volatile__ ("bic    X1,X1,#(0x1<<7)");  \
                __asm__ __volatile__ ("msr    DAIF, X1");  \

/****************************************************************************/
/**
* @brief	Disable the nested interrupts by setting the I bit in DAIF. This
*			macro is defined for Cortex-A53 64 bit mode and Cortex-A72 64 bit
*			BSP configured to run at EL1 NON SECURE
*
* @return   None.
*
* @note     This macro is meant to be called in the interrupt service routines.
*			This macro cannot be used independently. It can only be used when
*			nesting of interrupts have been enabled by using the macro
*			Xil_EnableNestedInterrupts(). In a typical flow, the user first
*			calls the Xil_EnableNestedInterrupts in the ISR at the appropriate
*			point. The user then must call this macro before exiting the interrupt
*			service routine. This macro puts the ARM back in IRQ mode and
*			hence sets back the I bit.
******************************************************************************/
#define Xil_DisableNestedInterrupts() \
                __asm__ __volatile__ ("ldp    X1,X2, [sp,#0x10]!"); \
                __asm__ __volatile__ ("msr    ELR_EL1, X1"); \
                __asm__ __volatile__ ("msr    SPSR_EL1, X2"); \
                __asm__ __volatile__ ("mrs    X1, DAIF");  \
                __asm__ __volatile__ ("orr    X1, X1, #(0x1<<7)"); \
                __asm__ __volatile__ ("msr    DAIF, X1");  \

#elif (!defined (__aarch64__) && !defined (ARMA53_32))
/****************************************************************************/
/**
* @brief	Enable nested interrupts by clearing the I and F bits in CPSR. This
* 			API is defined for cortex-a9 and cortex-r5.
*
* @return   None.
*
* @note     This macro is supposed to be used from interrupt handlers. In the
*			interrupt handler the interrupts are disabled by default (I and F
*			are 1). To allow nesting of interrupts, this macro should be
*			used. It clears the I and F bits by changing the ARM mode to
*			system mode. Once these bits are cleared and provided the
*			preemption of interrupt conditions are met in the GIC, nesting of
*			interrupts will start happening.
*			Caution: This macro must be used with caution. Before calling this
*			macro, the user must ensure that the source of the current IRQ
*			is appropriately cleared. Otherwise, as soon as we clear the I and
*			F bits, there can be an infinite loop of interrupts with an
*			eventual crash (all the stack space getting consumed).
******************************************************************************/
#define Xil_EnableNestedInterrupts() \
		__asm__ __volatile__ ("stmfd   sp!, {lr}"); \
		__asm__ __volatile__ ("mrs     lr, spsr");  \
		__asm__ __volatile__ ("stmfd   sp!, {lr}"); \
		__asm__ __volatile__ ("msr     cpsr_c, #0x1F"); \
		__asm__ __volatile__ ("stmfd   sp!, {lr}");
/****************************************************************************/
/**
* @brief	Disable the nested interrupts by setting the I and F bits. This API
*			is defined for cortex-a9 and cortex-r5.
*
* @return   None.
*
* @note     This macro is meant to be called in the interrupt service routines.
*			This macro cannot be used independently. It can only be used when
*			nesting of interrupts have been enabled by using the macro
*			Xil_EnableNestedInterrupts(). In a typical flow, the user first
*			calls the Xil_EnableNestedInterrupts in the ISR at the appropriate
*			point. The user then must call this macro before exiting the interrupt
*			service routine. This macro puts the ARM back in IRQ/FIQ mode and
*			hence sets back the I and F bits.
******************************************************************************/
#define Xil_DisableNestedInterrupts() \
		__asm__ __volatile__ ("ldmfd   sp!, {lr}");   \
		__asm__ __volatile__ ("msr     cpsr_c, #0x92"); \
		__asm__ __volatile__ ("ldmfd   sp!, {lr}"); \
		__asm__ __volatile__ ("msr     spsr_cxsf, lr"); \
		__asm__ __volatile__ ("ldmfd   sp!, {lr}"); \

#endif
/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

extern void Xil_ExceptionRegisterHandler(u32 Exception_id,
					 Xil_ExceptionHandler Handler,
					 void *Data);

extern void Xil_ExceptionRemoveHandler(u32 Exception_id);
extern void Xil_GetExceptionRegisterHandler(u32 Exception_id,
					Xil_ExceptionHandler *Handler, void **Data);

extern void Xil_ExceptionInit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIL_EXCEPTION_H */
/**
* @} End of "addtogroup arm_exception_apis".
*/
