/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_exception.h
*
* @addtogroup riscv_exception_apis RISC-V Exception APIs
*
* The xil_exception.h file contains RISC-V specific exception related APIs
* and macros. Application programs can use these APIs/Macros for various
* exception related operations (i.e. enable exception, disable exception,
* register exception handler etc.)
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sa   08/27/22 Initial release
* 9.3   ml   02/19/25 Add support for RISC-V Interrupt Handling
* 9.4   vmt  29/07/25 Add support for RISC-V Exception and Interrupt
*                     Handling in User and Supervisor modes.
*
* </pre>
*
* @note
*  To use exception related functions, the xil_exception.h file must be added
*  in source code
*
******************************************************************************/

/**
 *@cond nocomments
 */
#ifndef XIL_EXCEPTION_H /* prevent circular inclusions */
#define XIL_EXCEPTION_H /* by using protection macros */

#include "xil_types.h"

#include "riscv_exceptions_g.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/*
 * These constants are specific to RISC-V processors.
 *
 * See Table 3.6 in "The RISC-V Instruction Set Manual, Volume II".
 */

#define XIL_EXCEPTION_ID_FIRST                0U
#define XIL_EXCEPTION_ID_IADDR_MISALIGN       0U
#define XIL_EXCEPTION_ID_IACCESS_FAULT        1U
#define XIL_EXCEPTION_ID_ILLEGAL_INSTR        2U
#define XIL_EXCEPTION_ID_BREAKPOINT           3U
#define XIL_EXCEPTION_ID_LADDR_MISALIGN       4U
#define XIL_EXCEPTION_ID_LACCESS_FAULT        5U
#define XIL_EXCEPTION_ID_SADDR_MISALIGN       6U
#define XIL_EXCEPTION_ID_SACCESS_FAULT        7U
#define XIL_EXCEPTION_ID_ENVCALL_UMODE        8U
#define XIL_EXCEPTION_ID_ENVCALL_SMODE        9U
#define XIL_EXCEPTION_ID_ENVCALL_MMODE        11U
#define XIL_EXCEPTION_ID_IPAGE_FAULT          12U
#define XIL_EXCEPTION_ID_LPAGE_FAULT          13U
#define XIL_EXCEPTION_ID_SPAGE_FAULT          15U
#ifdef RISCV_FSL_EXCEPTION
#define XIL_EXCEPTION_ID_FSL_ERROR            24U
#define XIL_EXCEPTION_ID_LAST                 XIL_EXCEPTION_ID_FSL_ERROR
#else
#define XIL_EXCEPTION_ID_LAST                 XIL_EXCEPTION_ID_SPAGE_FAULT
#endif

#define XIL_INTERRUPT_ID_FIRST                0x8000000U
#define XIL_INTERRUPT_ID_NON_MASKABLE         0x8000000U
#define XIL_INTERRUPT_ID_MACHINE_SOFTWARE     0x8000003U
#define XIL_INTERRUPT_ID_MACHINE_TIMER        0x8000007U
#define XIL_INTERRUPT_ID_MACHINE_EXTERNAL     0x800000BU
#define XIL_INTERRUPT_ID_PLATFORM_BREAK       0x8000010U
#define XIL_INTERRUPT_ID_SUPERVISOR_SOFTWARE  0x80000001U
#define XIL_INTERRUPT_ID_SUPERVISOR_TIMER     0x80000005U
#define XIL_INTERRUPT_ID_SUPERVISOR_EXTERNAL  0x80000009U
#define XIL_INTERRUPT_ID_LAST                 XIL_INTERRUPT_ID_PLATFORM_BREAK

/*
 * XIL_EXCEPTION_ID_INT is defined for all processors, but with different value.
 *
 * For RISC-V it is set to Machine External Interrupt.
 */
#define XIL_EXCEPTION_ID_INT                  XIL_INTERRUPT_ID_MACHINE_EXTERNAL

/**************************** Type Definitions *******************************/

/**
 * This typedef is the exception handler function.
 */
typedef void (*Xil_ExceptionHandler)(void *Data);

/**
 * This data type defines an interrupt handler for a device.
 * The argument points to the instance of the component
 */
typedef void (*XInterruptHandler) (void *InstancePtr);

typedef struct {
   Xil_ExceptionHandler Handler;
   void *CallBackRef;
} RISCV_ExceptionVectorTableEntry;

/************************** Variable Definitions *****************************/
extern RISCV_ExceptionVectorTableEntry RISCV_ExceptionVectorTable[(XIL_EXCEPTION_ID_LAST - XIL_EXCEPTION_ID_FIRST) + 1U];
extern RISCV_ExceptionVectorTableEntry RISCV_InterruptVectorTable[(XIL_INTERRUPT_ID_LAST - XIL_INTERRUPT_ID_FIRST) + 1U];

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

extern void Xil_ExceptionRegisterHandler(u32 Id,
                                         Xil_ExceptionHandler Handler,
                                         void *Data);

extern void Xil_ExceptionRemoveHandler(u32 Id);

extern void Xil_ExceptionInit(void);
extern void Xil_ExceptionEnable(void);
extern void Xil_ExceptionDisable(void);

#ifdef __cplusplus
}
#endif

#endif

/**
 *@endcond
 */

/**
* @} End of "addtogroup riscv_exception_apis".
*/
