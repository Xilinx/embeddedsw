/******************************************************************************
* Copyright (c) 2020 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xreg_riscv.h
*
* This header file contains definitions for using inline assembler code. It is
* written specifically for the GNU compiler.
*
* All of the RISC-V GPRs, CSRs, and Debug Registers are defined along
* with the positions of the bits within the registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 9.0   sa       07/20/20 Initial version
* 9.0   sa       07/20/23 Added CSR definitions for  Hardware Performance
*                         Counters and Physical Memory Protection.
* 9.0   ss       04/03/26 Added user custom CSR to access AXI STREAM (FSL)
*                         from user mode.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/
#ifndef XREG_RISCV_H
#define XREG_RISCV_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* GPRs */
#define XREG_GPR0				x0 /**< General purpose register x0. */
#define XREG_GPR1				x1 /**< General purpose register x1. */
#define XREG_GPR2				x2 /**< General purpose register x2. */
#define XREG_GPR3				x3 /**< General purpose register x3. */
#define XREG_GPR4				x4 /**< General purpose register x4. */
#define XREG_GPR5				x5 /**< General purpose register x5. */
#define XREG_GPR6				x6 /**< General purpose register x6. */
#define XREG_GPR7				x7 /**< General purpose register x7. */
#define XREG_GPR8				x8 /**< General purpose register x8. */
#define XREG_GPR9				x9 /**< General purpose register x9. */
#define XREG_GPR10				x10 /**< General purpose register x10. */
#define XREG_GPR11				x11 /**< General purpose register x11. */
#define XREG_GPR12				x12 /**< General purpose register x12. */
#define XREG_GPR13				x13 /**< General purpose register x13. */
#define XREG_GPR14				x14 /**< General purpose register x14. */
#define XREG_GPR15				x15 /**< General purpose register x15. */
#define XREG_GPR16				x16 /**< General purpose register x16. */
#define XREG_GPR17				x17 /**< General purpose register x17. */
#define XREG_GPR18				x18 /**< General purpose register x18. */
#define XREG_GPR19				x19 /**< General purpose register x19. */
#define XREG_GPR20				x20 /**< General purpose register x20. */
#define XREG_GPR21				x21 /**< General purpose register x21. */
#define XREG_GPR22				x22 /**< General purpose register x22. */
#define XREG_GPR23				x23 /**< General purpose register x23. */
#define XREG_GPR24				x24 /**< General purpose register x24. */
#define XREG_GPR25				x25 /**< General purpose register x25. */
#define XREG_GPR26				x26 /**< General purpose register x26. */
#define XREG_GPR27				x27 /**< General purpose register x27. */
#define XREG_GPR28				x28 /**< General purpose register x28. */
#define XREG_GPR29				x29 /**< General purpose register x29. */
#define XREG_GPR30				x30 /**< General purpose register x30. */
#define XREG_GPR31				x31 /**< General purpose register x31. */

#define XREG_ZERO                               XREG_GPR0 /**< Hard-wired zero register alias. */
#define XREG_RA                                 XREG_GPR1 /**< Return address register alias. */
#define XREG_SP                                 XREG_GPR2 /**< Stack pointer register alias. */
#define XREG_GP                                 XREG_GPR3 /**< Global pointer register alias. */
#define XREG_TP                                 XREG_GPR4 /**< Thread pointer register alias. */
#define XREG_T0                                 XREG_GPR5 /**< Temporary/alternate link register alias. */
#define XREG_T1                                 XREG_GPR6 /**< Temporary register alias. */
#define XREG_T2                                 XREG_GPR7 /**< Temporary register alias. */
#define XREG_S0                                 XREG_GPR8 /**< Saved register alias. */
#define XREG_FP                                 XREG_GPR8 /**< Frame pointer register alias. */
#define XREG_S1                                 XREG_GPR9 /**< Saved register alias. */
#define XREG_A0                                 XREG_GPR10 /**< Function argument/return value register alias. */
#define XREG_A1                                 XREG_GPR11 /**< Function argument register alias. */
#define XREG_A2                                 XREG_GPR12 /**< Function argument register alias. */
#define XREG_A3                                 XREG_GPR13 /**< Function argument register alias. */
#define XREG_A4                                 XREG_GPR14 /**< Function argument register alias. */
#define XREG_A5                                 XREG_GPR15 /**< Function argument register alias. */
#define XREG_A6                                 XREG_GPR16 /**< Function argument register alias. */
#define XREG_A7                                 XREG_GPR17 /**< Function argument register alias. */
#define XREG_S2                                 XREG_GPR18 /**< Saved register alias. */
#define XREG_S3                                 XREG_GPR19 /**< Saved register alias. */
#define XREG_S4                                 XREG_GPR20 /**< Saved register alias. */
#define XREG_S5                                 XREG_GPR21 /**< Saved register alias. */
#define XREG_S6                                 XREG_GPR22 /**< Saved register alias. */
#define XREG_S7                                 XREG_GPR23 /**< Saved register alias. */
#define XREG_S8                                 XREG_GPR24 /**< Saved register alias. */
#define XREG_S9                                 XREG_GPR25 /**< Saved register alias. */
#define XREG_S10                                XREG_GPR26 /**< Saved register alias. */
#define XREG_S11                                XREG_GPR27 /**< Saved register alias. */
#define XREG_T3                                 XREG_GPR28 /**< Temporary register alias. */
#define XREG_T4                                 XREG_GPR29 /**< Temporary register alias. */
#define XREG_T5                                 XREG_GPR30 /**< Temporary register alias. */
#define XREG_T6                                 XREG_GPR31 /**< Temporary register alias. */

/* FPs */
#define XREG_FP0                                f0 /**< Floating-point register f0. */
#define XREG_FP1                                f1 /**< Floating-point register f1. */
#define XREG_FP2                                f2 /**< Floating-point register f2. */
#define XREG_FP3                                f3 /**< Floating-point register f3. */
#define XREG_FP4                                f4 /**< Floating-point register f4. */
#define XREG_FP5                                f5 /**< Floating-point register f5. */
#define XREG_FP6                                f6 /**< Floating-point register f6. */
#define XREG_FP7                                f7 /**< Floating-point register f7. */
#define XREG_FP8                                f8 /**< Floating-point register f8. */
#define XREG_FP9                                f9 /**< Floating-point register f9. */
#define XREG_FP10                               f10 /**< Floating-point register f10. */
#define XREG_FP11                               f11 /**< Floating-point register f11. */
#define XREG_FP12                               f12 /**< Floating-point register f12. */
#define XREG_FP13                               f13 /**< Floating-point register f13. */
#define XREG_FP14                               f14 /**< Floating-point register f14. */
#define XREG_FP15                               f15 /**< Floating-point register f15. */
#define XREG_FP16                               f16 /**< Floating-point register f16. */
#define XREG_FP17                               f17 /**< Floating-point register f17. */
#define XREG_FP18                               f18 /**< Floating-point register f18. */
#define XREG_FP19                               f19 /**< Floating-point register f19. */
#define XREG_FP20                               f20 /**< Floating-point register f20. */
#define XREG_FP21                               f21 /**< Floating-point register f21. */
#define XREG_FP22                               f22 /**< Floating-point register f22. */
#define XREG_FP23                               f23 /**< Floating-point register f23. */
#define XREG_FP24                               f24 /**< Floating-point register f24. */
#define XREG_FP25                               f25 /**< Floating-point register f25. */
#define XREG_FP26                               f26 /**< Floating-point register f26. */
#define XREG_FP27                               f27 /**< Floating-point register f27. */
#define XREG_FP28                               f28 /**< Floating-point register f28. */
#define XREG_FP29                               f29 /**< Floating-point register f29. */
#define XREG_FP30                               f30 /**< Floating-point register f30. */
#define XREG_FP31                               f31 /**< Floating-point register f31. */

#define XREG_FT0                                XREG_FP0 /**< Floating-point temporary register alias. */
#define XREG_FT1                                XREG_FP1 /**< Floating-point temporary register alias. */
#define XREG_FT2                                XREG_FP2 /**< Floating-point temporary register alias. */
#define XREG_FT3                                XREG_FP3 /**< Floating-point temporary register alias. */
#define XREG_FT4                                XREG_FP4 /**< Floating-point temporary register alias. */
#define XREG_FT5                                XREG_FP5 /**< Floating-point temporary register alias. */
#define XREG_FT6                                XREG_FP6 /**< Floating-point temporary register alias. */
#define XREG_FT7                                XREG_FP7 /**< Floating-point temporary register alias. */
#define XREG_FS0                                XREG_FP8 /**< Floating-point saved register alias. */
#define XREG_FS1                                XREG_FP9 /**< Floating-point saved register alias. */
#define XREG_FA0                                XREG_FP10 /**< Floating-point argument/return value register alias. */
#define XREG_FA1                                XREG_FP11 /**< Floating-point argument register alias. */
#define XREG_FA2                                XREG_FP12 /**< Floating-point argument register alias. */
#define XREG_FA3                                XREG_FP13 /**< Floating-point argument register alias. */
#define XREG_FA4                                XREG_FP14 /**< Floating-point argument register alias. */
#define XREG_FA5                                XREG_FP15 /**< Floating-point argument register alias. */
#define XREG_FA6                                XREG_FP16 /**< Floating-point argument register alias. */
#define XREG_FA7                                XREG_FP17 /**< Floating-point argument register alias. */
#define XREG_FS2                                XREG_FP18 /**< Floating-point saved register alias. */
#define XREG_FS3                                XREG_FP19 /**< Floating-point saved register alias. */
#define XREG_FS4                                XREG_FP20 /**< Floating-point saved register alias. */
#define XREG_FS5                                XREG_FP21 /**< Floating-point saved register alias. */
#define XREG_FS6                                XREG_FP22 /**< Floating-point saved register alias. */
#define XREG_FS7                                XREG_FP23 /**< Floating-point saved register alias. */
#define XREG_FS8                                XREG_FP24 /**< Floating-point saved register alias. */
#define XREG_FS9                                XREG_FP25 /**< Floating-point saved register alias. */
#define XREG_FS10                               XREG_FP26 /**< Floating-point saved register alias. */
#define XREG_FS11                               XREG_FP27 /**< Floating-point saved register alias. */
#define XREG_FT8                                XREG_FP28 /**< Floating-point temporary register alias. */
#define XREG_FT9                                XREG_FP29 /**< Floating-point temporary register alias. */
#define XREG_FT10                               XREG_FP30 /**< Floating-point temporary register alias. */
#define XREG_FT11                               XREG_FP31 /**< Floating-point temporary register alias. */

/* CSRs */

#define XREG_FFLAGS                             fflags /**< Floating-point Accrued Exceptions CSR (0x001). */
#define XREG_FRM                                frm /**< Floating-point Dynamic Rounding Mode CSR (0x002). */
#define XREG_FCSR                               fcsr /**< Floating-point Control and Status Register CSR (0x003). */

#define XREG_CYCLE                              cycle /**< Cycle counter CSR (0xC00). */
#define XREG_TIME                               time /**< Real-time clock CSR (0xC01). */
#define XREG_INSTRET                            instret /**< Instructions-retired counter CSR (0xC02). */
#define XREG_HPMCOUNTER3			hpmcounter3 /**< Hardware Performance Monitor Counter 3 CSR. */
#define XREG_HPMCOUNTER4			hpmcounter4 /**< Hardware Performance Monitor Counter 4 CSR. */
#define XREG_HPMCOUNTER5			hpmcounter5 /**< Hardware Performance Monitor Counter 5 CSR. */
#define XREG_HPMCOUNTER6			hpmcounter6 /**< Hardware Performance Monitor Counter 6 CSR. */
#define XREG_HPMCOUNTER7			hpmcounter7 /**< Hardware Performance Monitor Counter 7 CSR. */
#define XREG_HPMCOUNTER8			hpmcounter8 /**< Hardware Performance Monitor Counter 8 CSR. */
#define XREG_HPMCOUNTER9			hpmcounter9 /**< Hardware Performance Monitor Counter 9 CSR. */
#define XREG_HPMCOUNTER10			hpmcounter10 /**< Hardware Performance Monitor Counter 10 CSR. */
#define XREG_HPMCOUNTER11			hpmcounter11 /**< Hardware Performance Monitor Counter 11 CSR. */
#define XREG_HPMCOUNTER12			hpmcounter12 /**< Hardware Performance Monitor Counter 12 CSR. */
#define XREG_HPMCOUNTER13			hpmcounter13 /**< Hardware Performance Monitor Counter 13 CSR. */
#define XREG_HPMCOUNTER14			hpmcounter14 /**< Hardware Performance Monitor Counter 14 CSR. */
#define XREG_HPMCOUNTER15			hpmcounter15 /**< Hardware Performance Monitor Counter 15 CSR. */
#define XREG_HPMCOUNTER16			hpmcounter16 /**< Hardware Performance Monitor Counter 16 CSR. */
#define XREG_HPMCOUNTER17			hpmcounter17 /**< Hardware Performance Monitor Counter 17 CSR. */
#define XREG_HPMCOUNTER18			hpmcounter18 /**< Hardware Performance Monitor Counter 18 CSR. */
#define XREG_HPMCOUNTER19			hpmcounter19 /**< Hardware Performance Monitor Counter 19 CSR. */
#define XREG_HPMCOUNTER20			hpmcounter20 /**< Hardware Performance Monitor Counter 20 CSR. */
#define XREG_HPMCOUNTER21			hpmcounter21 /**< Hardware Performance Monitor Counter 21 CSR. */
#define XREG_HPMCOUNTER22			hpmcounter22 /**< Hardware Performance Monitor Counter 22 CSR. */
#define XREG_HPMCOUNTER23			hpmcounter23 /**< Hardware Performance Monitor Counter 23 CSR. */
#define XREG_HPMCOUNTER24			hpmcounter24 /**< Hardware Performance Monitor Counter 24 CSR. */
#define XREG_HPMCOUNTER25			hpmcounter25 /**< Hardware Performance Monitor Counter 25 CSR. */
#define XREG_HPMCOUNTER26			hpmcounter26 /**< Hardware Performance Monitor Counter 26 CSR. */
#define XREG_HPMCOUNTER27			hpmcounter27 /**< Hardware Performance Monitor Counter 27 CSR. */
#define XREG_HPMCOUNTER28			hpmcounter28 /**< Hardware Performance Monitor Counter 28 CSR. */
#define XREG_HPMCOUNTER29			hpmcounter29 /**< Hardware Performance Monitor Counter 29 CSR. */
#define XREG_HPMCOUNTER30			hpmcounter30 /**< Hardware Performance Monitor Counter 30 CSR. */
#define XREG_HPMCOUNTER31			hpmcounter31 /**< Hardware Performance Monitor Counter 31 CSR. */
#define XREG_CYCLEH                             cycleh /**< Upper 32 bits of CYCLE CSR (0xC80). */
#define XREG_TIMEH                              timeh /**< Upper 32 bits of TIME CSR (0xC81). */
#define XREG_INSTRETH                           instreth /**< Upper 32 bits of INSTRET CSR (0xC82). */
#define XREG_HPMCOUNTER3H			hpmcounter3h /**< Hardware Performance Monitor Counter 3H CSR. */
#define XREG_HPMCOUNTER4H			hpmcounter4h /**< Hardware Performance Monitor Counter 4H CSR. */
#define XREG_HPMCOUNTER5H			hpmcounter5h /**< Hardware Performance Monitor Counter 5H CSR. */
#define XREG_HPMCOUNTER6H			hpmcounter6h /**< Hardware Performance Monitor Counter 6H CSR. */
#define XREG_HPMCOUNTER7H			hpmcounter7h /**< Hardware Performance Monitor Counter 7H CSR. */
#define XREG_HPMCOUNTER8H			hpmcounter8h /**< Hardware Performance Monitor Counter 8H CSR. */
#define XREG_HPMCOUNTER9H			hpmcounter9h /**< Hardware Performance Monitor Counter 9H CSR. */
#define XREG_HPMCOUNTER10H			hpmcounter10h /**< Hardware Performance Monitor Counter 10H CSR. */
#define XREG_HPMCOUNTER11H			hpmcounter11h /**< Hardware Performance Monitor Counter 11H CSR. */
#define XREG_HPMCOUNTER12H			hpmcounter12h /**< Hardware Performance Monitor Counter 12H CSR. */
#define XREG_HPMCOUNTER13H			hpmcounter13h /**< Hardware Performance Monitor Counter 13H CSR. */
#define XREG_HPMCOUNTER14H			hpmcounter14h /**< Hardware Performance Monitor Counter 14H CSR. */
#define XREG_HPMCOUNTER15H			hpmcounter15h /**< Hardware Performance Monitor Counter 15H CSR. */
#define XREG_HPMCOUNTER16H			hpmcounter16h /**< Hardware Performance Monitor Counter 16H CSR. */
#define XREG_HPMCOUNTER17H			hpmcounter17h /**< Hardware Performance Monitor Counter 17H CSR. */
#define XREG_HPMCOUNTER18H			hpmcounter18h /**< Hardware Performance Monitor Counter 18H CSR. */
#define XREG_HPMCOUNTER19H			hpmcounter19h /**< Hardware Performance Monitor Counter 19H CSR. */
#define XREG_HPMCOUNTER20H			hpmcounter20h /**< Hardware Performance Monitor Counter 20H CSR. */
#define XREG_HPMCOUNTER21H			hpmcounter21h /**< Hardware Performance Monitor Counter 21H CSR. */
#define XREG_HPMCOUNTER22H			hpmcounter22h /**< Hardware Performance Monitor Counter 22H CSR. */
#define XREG_HPMCOUNTER23H			hpmcounter23h /**< Hardware Performance Monitor Counter 23H CSR. */
#define XREG_HPMCOUNTER24H			hpmcounter24h /**< Hardware Performance Monitor Counter 24H CSR. */
#define XREG_HPMCOUNTER25H			hpmcounter25h /**< Hardware Performance Monitor Counter 25H CSR. */
#define XREG_HPMCOUNTER26H			hpmcounter26h /**< Hardware Performance Monitor Counter 26H CSR. */
#define XREG_HPMCOUNTER27H			hpmcounter27h /**< Hardware Performance Monitor Counter 27H CSR. */
#define XREG_HPMCOUNTER28H			hpmcounter28h /**< Hardware Performance Monitor Counter 28H CSR. */
#define XREG_HPMCOUNTER29H			hpmcounter29h /**< Hardware Performance Monitor Counter 29H CSR. */
#define XREG_HPMCOUNTER30H			hpmcounter30h /**< Hardware Performance Monitor Counter 30H CSR. */
#define XREG_HPMCOUNTER31H			hpmcounter31h /**< Hardware Performance Monitor Counter 31H CSR. */

#define XREG_MVENDORID                          mvendorid /**< Machine Vendor ID CSR (0xF11). */
#define XREG_MARCHID                            marchid /**< Machine Architecture ID CSR (0xF12). */
#define XREG_MIMPID                             mimpid /**< Machine Implementation ID CSR (0xF13). */
#define XREG_MHARTID                            mhartid /**< Machine Hardware Thread ID CSR (0xF14). */
#define XREG_MSTATUS                            mstatus /**< Machine Status CSR (0x300). */
#define XREG_MISA                               misa /**< Machine ISA CSR (0x301). */
#define XREG_MEDELEG                            medeleg /**< Machine Exception Delegation CSR (0x302). */
#define XREG_MIDELEG                            mideleg /**< Machine Interrupt Delegation CSR (0x303). */
#define XREG_MIE                                mie /**< Machine Interrupt Enable CSR (0x304). */
#define XREG_MTVEC                              mtvec /**< Machine Trap Vector Base Address CSR (0x305). */
#define XREG_MCOUNTEREN                         mcounteren /**< Machine Counter Enable CSR (0x306). */
#define XREG_MSCRATCH                           mscratch /**< Machine Scratch CSR (0x340). */
#define XREG_MEPC                               mepc /**< Machine Exception Program Counter CSR (0x341). */
#define XREG_MCAUSE                             mcause /**< Machine Trap Cause CSR (0x342). */
#define XREG_MTVAL                              mtval /**< Machine Trap Value CSR (0x343). */
#define XREG_MIP                                mip /**< Machine Interrupt Pending CSR (0x344). */
#define XREG_PMPCFG0				pmpcfg0 /**< Physical Memory Protection Configuration 0 CSR. */
#define XREG_PMPCFG1				pmpcfg1 /**< Physical Memory Protection Configuration 1 CSR. */
#define XREG_PMPCFG2				pmpcfg2 /**< Physical Memory Protection Configuration 2 CSR. */
#define XREG_PMPCFG3				pmpcfg3 /**< Physical Memory Protection Configuration 3 CSR. */
#define XREG_PMPCFG4				pmpcfg4 /**< Physical Memory Protection Configuration 4 CSR. */
#define XREG_PMPCFG5				pmpcfg5 /**< Physical Memory Protection Configuration 5 CSR. */
#define XREG_PMPCFG6				pmpcfg6 /**< Physical Memory Protection Configuration 6 CSR. */
#define XREG_PMPCFG7				pmpcfg7 /**< Physical Memory Protection Configuration 7 CSR. */
#define XREG_PMPCFG8				pmpcfg8 /**< Physical Memory Protection Configuration 8 CSR. */
#define XREG_PMPCFG9				pmpcfg9 /**< Physical Memory Protection Configuration 9 CSR. */
#define XREG_PMPCFG10				pmpcfg10 /**< Physical Memory Protection Configuration 10 CSR. */
#define XREG_PMPCFG11				pmpcfg11 /**< Physical Memory Protection Configuration 11 CSR. */
#define XREG_PMPCFG12				pmpcfg12 /**< Physical Memory Protection Configuration 12 CSR. */
#define XREG_PMPCFG13				pmpcfg13 /**< Physical Memory Protection Configuration 13 CSR. */
#define XREG_PMPCFG14				pmpcfg14 /**< Physical Memory Protection Configuration 14 CSR. */
#define XREG_PMPCFG15				pmpcfg15 /**< Physical Memory Protection Configuration 15 CSR. */
#define XREG_PMPADDR0				pmpaddr0 /**< Physical Memory Protection Address 0 CSR. */
#define XREG_PMPADDR1				pmpaddr1 /**< Physical Memory Protection Address 1 CSR. */
#define XREG_PMPADDR2				pmpaddr2 /**< Physical Memory Protection Address 2 CSR. */
#define XREG_PMPADDR3				pmpaddr3 /**< Physical Memory Protection Address 3 CSR. */
#define XREG_PMPADDR4				pmpaddr4 /**< Physical Memory Protection Address 4 CSR. */
#define XREG_PMPADDR5				pmpaddr5 /**< Physical Memory Protection Address 5 CSR. */
#define XREG_PMPADDR6				pmpaddr6 /**< Physical Memory Protection Address 6 CSR. */
#define XREG_PMPADDR7				pmpaddr7 /**< Physical Memory Protection Address 7 CSR. */
#define XREG_PMPADDR8				pmpaddr8 /**< Physical Memory Protection Address 8 CSR. */
#define XREG_PMPADDR9				pmpaddr9 /**< Physical Memory Protection Address 9 CSR. */
#define XREG_PMPADDR10				pmpaddr10 /**< Physical Memory Protection Address 10 CSR. */
#define XREG_PMPADDR11				pmpaddr11 /**< Physical Memory Protection Address 11 CSR. */
#define XREG_PMPADDR12				pmpaddr12 /**< Physical Memory Protection Address 12 CSR. */
#define XREG_PMPADDR13				pmpaddr13 /**< Physical Memory Protection Address 13 CSR. */
#define XREG_PMPADDR14				pmpaddr14 /**< Physical Memory Protection Address 14 CSR. */
#define XREG_PMPADDR15				pmpaddr15 /**< Physical Memory Protection Address 15 CSR. */
#define XREG_PMPADDR16				pmpaddr16 /**< Physical Memory Protection Address 16 CSR. */
#define XREG_PMPADDR17				pmpaddr17 /**< Physical Memory Protection Address 17 CSR. */
#define XREG_PMPADDR18				pmpaddr18 /**< Physical Memory Protection Address 18 CSR. */
#define XREG_PMPADDR19				pmpaddr19 /**< Physical Memory Protection Address 19 CSR. */
#define XREG_PMPADDR20				pmpaddr20 /**< Physical Memory Protection Address 20 CSR. */
#define XREG_PMPADDR21				pmpaddr21 /**< Physical Memory Protection Address 21 CSR. */
#define XREG_PMPADDR22				pmpaddr22 /**< Physical Memory Protection Address 22 CSR. */
#define XREG_PMPADDR23				pmpaddr23 /**< Physical Memory Protection Address 23 CSR. */
#define XREG_PMPADDR24				pmpaddr24 /**< Physical Memory Protection Address 24 CSR. */
#define XREG_PMPADDR25				pmpaddr25 /**< Physical Memory Protection Address 25 CSR. */
#define XREG_PMPADDR26				pmpaddr26 /**< Physical Memory Protection Address 26 CSR. */
#define XREG_PMPADDR27				pmpaddr27 /**< Physical Memory Protection Address 27 CSR. */
#define XREG_PMPADDR28				pmpaddr28 /**< Physical Memory Protection Address 28 CSR. */
#define XREG_PMPADDR29				pmpaddr29 /**< Physical Memory Protection Address 29 CSR. */
#define XREG_PMPADDR30				pmpaddr30 /**< Physical Memory Protection Address 30 CSR. */
#define XREG_PMPADDR31				pmpaddr31 /**< Physical Memory Protection Address 31 CSR. */
#define XREG_PMPADDR32				pmpaddr32 /**< Physical Memory Protection Address 32 CSR. */
#define XREG_PMPADDR33				pmpaddr33 /**< Physical Memory Protection Address 33 CSR. */
#define XREG_PMPADDR34				pmpaddr34 /**< Physical Memory Protection Address 34 CSR. */
#define XREG_PMPADDR35				pmpaddr35 /**< Physical Memory Protection Address 35 CSR. */
#define XREG_PMPADDR36				pmpaddr36 /**< Physical Memory Protection Address 36 CSR. */
#define XREG_PMPADDR37				pmpaddr37 /**< Physical Memory Protection Address 37 CSR. */
#define XREG_PMPADDR38				pmpaddr38 /**< Physical Memory Protection Address 38 CSR. */
#define XREG_PMPADDR39				pmpaddr39 /**< Physical Memory Protection Address 39 CSR. */
#define XREG_PMPADDR40				pmpaddr40 /**< Physical Memory Protection Address 40 CSR. */
#define XREG_PMPADDR41				pmpaddr41 /**< Physical Memory Protection Address 41 CSR. */
#define XREG_PMPADDR42				pmpaddr42 /**< Physical Memory Protection Address 42 CSR. */
#define XREG_PMPADDR43				pmpaddr43 /**< Physical Memory Protection Address 43 CSR. */
#define XREG_PMPADDR44				pmpaddr44 /**< Physical Memory Protection Address 44 CSR. */
#define XREG_PMPADDR45				pmpaddr45 /**< Physical Memory Protection Address 45 CSR. */
#define XREG_PMPADDR46				pmpaddr46 /**< Physical Memory Protection Address 46 CSR. */
#define XREG_PMPADDR47				pmpaddr47 /**< Physical Memory Protection Address 47 CSR. */
#define XREG_PMPADDR48				pmpaddr48 /**< Physical Memory Protection Address 48 CSR. */
#define XREG_PMPADDR49				pmpaddr49 /**< Physical Memory Protection Address 49 CSR. */
#define XREG_PMPADDR50				pmpaddr50 /**< Physical Memory Protection Address 50 CSR. */
#define XREG_PMPADDR51				pmpaddr51 /**< Physical Memory Protection Address 51 CSR. */
#define XREG_PMPADDR52				pmpaddr52 /**< Physical Memory Protection Address 52 CSR. */
#define XREG_PMPADDR53				pmpaddr53 /**< Physical Memory Protection Address 53 CSR. */
#define XREG_PMPADDR54				pmpaddr54 /**< Physical Memory Protection Address 54 CSR. */
#define XREG_PMPADDR55				pmpaddr55 /**< Physical Memory Protection Address 55 CSR. */
#define XREG_PMPADDR56				pmpaddr56 /**< Physical Memory Protection Address 56 CSR. */
#define XREG_PMPADDR57				pmpaddr57 /**< Physical Memory Protection Address 57 CSR. */
#define XREG_PMPADDR58				pmpaddr58 /**< Physical Memory Protection Address 58 CSR. */
#define XREG_PMPADDR59				pmpaddr59 /**< Physical Memory Protection Address 59 CSR. */
#define XREG_PMPADDR60				pmpaddr60 /**< Physical Memory Protection Address 60 CSR. */
#define XREG_PMPADDR61				pmpaddr61 /**< Physical Memory Protection Address 61 CSR. */
#define XREG_PMPADDR62				pmpaddr62 /**< Physical Memory Protection Address 62 CSR. */
#define XREG_PMPADDR63				pmpaddr63 /**< Physical Memory Protection Address 63 CSR. */

#define XREG_MCYCLE				mcycle /**< Machine Cycle Counter CSR (0xB00). */
#define XREG_MINSTRET				minstret /**< Machine Instructions-Retired Counter CSR (0xB02). */
#define XREG_MHPMCOUNTER3			mhpmcounter3 /**< Machine Hardware Performance Monitor Counter 3 CSR. */
#define XREG_MHPMCOUNTER4			mhpmcounter4 /**< Machine Hardware Performance Monitor Counter 4 CSR. */
#define XREG_MHPMCOUNTER5			mhpmcounter5 /**< Machine Hardware Performance Monitor Counter 5 CSR. */
#define XREG_MHPMCOUNTER6			mhpmcounter6 /**< Machine Hardware Performance Monitor Counter 6 CSR. */
#define XREG_MHPMCOUNTER7			mhpmcounter7 /**< Machine Hardware Performance Monitor Counter 7 CSR. */
#define XREG_MHPMCOUNTER8			mhpmcounter8 /**< Machine Hardware Performance Monitor Counter 8 CSR. */
#define XREG_MHPMCOUNTER9			mhpmcounter9 /**< Machine Hardware Performance Monitor Counter 9 CSR. */
#define XREG_MHPMCOUNTER10			mhpmcounter10 /**< Machine Hardware Performance Monitor Counter 10 CSR. */
#define XREG_MHPMCOUNTER11			mhpmcounter11 /**< Machine Hardware Performance Monitor Counter 11 CSR. */
#define XREG_MHPMCOUNTER12			mhpmcounter12 /**< Machine Hardware Performance Monitor Counter 12 CSR. */
#define XREG_MHPMCOUNTER13			mhpmcounter13 /**< Machine Hardware Performance Monitor Counter 13 CSR. */
#define XREG_MHPMCOUNTER14			mhpmcounter14 /**< Machine Hardware Performance Monitor Counter 14 CSR. */
#define XREG_MHPMCOUNTER15			mhpmcounter15 /**< Machine Hardware Performance Monitor Counter 15 CSR. */
#define XREG_MHPMCOUNTER16			mhpmcounter16 /**< Machine Hardware Performance Monitor Counter 16 CSR. */
#define XREG_MHPMCOUNTER17			mhpmcounter17 /**< Machine Hardware Performance Monitor Counter 17 CSR. */
#define XREG_MHPMCOUNTER18			mhpmcounter18 /**< Machine Hardware Performance Monitor Counter 18 CSR. */
#define XREG_MHPMCOUNTER19			mhpmcounter19 /**< Machine Hardware Performance Monitor Counter 19 CSR. */
#define XREG_MHPMCOUNTER20			mhpmcounter20 /**< Machine Hardware Performance Monitor Counter 20 CSR. */
#define XREG_MHPMCOUNTER21			mhpmcounter21 /**< Machine Hardware Performance Monitor Counter 21 CSR. */
#define XREG_MHPMCOUNTER22			mhpmcounter22 /**< Machine Hardware Performance Monitor Counter 22 CSR. */
#define XREG_MHPMCOUNTER23			mhpmcounter23 /**< Machine Hardware Performance Monitor Counter 23 CSR. */
#define XREG_MHPMCOUNTER24			mhpmcounter24 /**< Machine Hardware Performance Monitor Counter 24 CSR. */
#define XREG_MHPMCOUNTER25			mhpmcounter25 /**< Machine Hardware Performance Monitor Counter 25 CSR. */
#define XREG_MHPMCOUNTER26			mhpmcounter26 /**< Machine Hardware Performance Monitor Counter 26 CSR. */
#define XREG_MHPMCOUNTER27			mhpmcounter27 /**< Machine Hardware Performance Monitor Counter 27 CSR. */
#define XREG_MHPMCOUNTER28			mhpmcounter28 /**< Machine Hardware Performance Monitor Counter 28 CSR. */
#define XREG_MHPMCOUNTER29			mhpmcounter29 /**< Machine Hardware Performance Monitor Counter 29 CSR. */
#define XREG_MHPMCOUNTER30			mhpmcounter30 /**< Machine Hardware Performance Monitor Counter 30 CSR. */
#define XREG_MHPMCOUNTER31			mhpmcounter31 /**< Machine Hardware Performance Monitor Counter 31 CSR. */
#define XREG_MCYCLEH				mcycleh /**< Upper 32 bits of MCYCLE CSR (0xB80). */
#define XREG_MINSTRETH				minstreth /**< Upper 32 bits of MINSTRET CSR (0xB82). */
#define XREG_MHPMCOUNTER3H			mhpmcounter3h /**< Machine Hardware Performance Monitor Counter 3H CSR. */
#define XREG_MHPMCOUNTER4H			mhpmcounter4h /**< Machine Hardware Performance Monitor Counter 4H CSR. */
#define XREG_MHPMCOUNTER5H			mhpmcounter5h /**< Machine Hardware Performance Monitor Counter 5H CSR. */
#define XREG_MHPMCOUNTER6H			mhpmcounter6h /**< Machine Hardware Performance Monitor Counter 6H CSR. */
#define XREG_MHPMCOUNTER7H			mhpmcounter7h /**< Machine Hardware Performance Monitor Counter 7H CSR. */
#define XREG_MHPMCOUNTER8H			mhpmcounter8h /**< Machine Hardware Performance Monitor Counter 8H CSR. */
#define XREG_MHPMCOUNTER9H			mhpmcounter9h /**< Machine Hardware Performance Monitor Counter 9H CSR. */
#define XREG_MHPMCOUNTER10H			mhpmcounter10h /**< Machine Hardware Performance Monitor Counter 10H CSR. */
#define XREG_MHPMCOUNTER11H			mhpmcounter11h /**< Machine Hardware Performance Monitor Counter 11H CSR. */
#define XREG_MHPMCOUNTER12H			mhpmcounter12h /**< Machine Hardware Performance Monitor Counter 12H CSR. */
#define XREG_MHPMCOUNTER13H			mhpmcounter13h /**< Machine Hardware Performance Monitor Counter 13H CSR. */
#define XREG_MHPMCOUNTER14H			mhpmcounter14h /**< Machine Hardware Performance Monitor Counter 14H CSR. */
#define XREG_MHPMCOUNTER15H			mhpmcounter15h /**< Machine Hardware Performance Monitor Counter 15H CSR. */
#define XREG_MHPMCOUNTER16H			mhpmcounter16h /**< Machine Hardware Performance Monitor Counter 16H CSR. */
#define XREG_MHPMCOUNTER17H			mhpmcounter17h /**< Machine Hardware Performance Monitor Counter 17H CSR. */
#define XREG_MHPMCOUNTER18H			mhpmcounter18h /**< Machine Hardware Performance Monitor Counter 18H CSR. */
#define XREG_MHPMCOUNTER19H			mhpmcounter19h /**< Machine Hardware Performance Monitor Counter 19H CSR. */
#define XREG_MHPMCOUNTER20H			mhpmcounter20h /**< Machine Hardware Performance Monitor Counter 20H CSR. */
#define XREG_MHPMCOUNTER21H			mhpmcounter21h /**< Machine Hardware Performance Monitor Counter 21H CSR. */
#define XREG_MHPMCOUNTER22H			mhpmcounter22h /**< Machine Hardware Performance Monitor Counter 22H CSR. */
#define XREG_MHPMCOUNTER23H			mhpmcounter23h /**< Machine Hardware Performance Monitor Counter 23H CSR. */
#define XREG_MHPMCOUNTER24H			mhpmcounter24h /**< Machine Hardware Performance Monitor Counter 24H CSR. */
#define XREG_MHPMCOUNTER25H			mhpmcounter25h /**< Machine Hardware Performance Monitor Counter 25H CSR. */
#define XREG_MHPMCOUNTER26H			mhpmcounter26h /**< Machine Hardware Performance Monitor Counter 26H CSR. */
#define XREG_MHPMCOUNTER27H			mhpmcounter27h /**< Machine Hardware Performance Monitor Counter 27H CSR. */
#define XREG_MHPMCOUNTER28H			mhpmcounter28h /**< Machine Hardware Performance Monitor Counter 28H CSR. */
#define XREG_MHPMCOUNTER29H			mhpmcounter29h /**< Machine Hardware Performance Monitor Counter 29H CSR. */
#define XREG_MHPMCOUNTER30H			mhpmcounter30h /**< Machine Hardware Performance Monitor Counter 30H CSR. */
#define XREG_MHPMCOUNTER31H			mhpmcounter31h /**< Machine Hardware Performance Monitor Counter 31H CSR. */

#define XREG_MCOUNTINHIBIT			mcountinhibit /**< Machine Counter Inhibit CSR (0x320). */
#define XREG_MHPMEVENT3				mhpmevent3 /**< Machine Hardware Performance Monitor Event Selector 3 CSR. */
#define XREG_MHPMEVENT4				mhpmevent4 /**< Machine Hardware Performance Monitor Event Selector 4 CSR. */
#define XREG_MHPMEVENT5				mhpmevent5 /**< Machine Hardware Performance Monitor Event Selector 5 CSR. */
#define XREG_MHPMEVENT6				mhpmevent6 /**< Machine Hardware Performance Monitor Event Selector 6 CSR. */
#define XREG_MHPMEVENT7				mhpmevent7 /**< Machine Hardware Performance Monitor Event Selector 7 CSR. */
#define XREG_MHPMEVENT8				mhpmevent8 /**< Machine Hardware Performance Monitor Event Selector 8 CSR. */
#define XREG_MHPMEVENT9				mhpmevent9 /**< Machine Hardware Performance Monitor Event Selector 9 CSR. */
#define XREG_MHPMEVENT10			mhpmevent10 /**< Machine Hardware Performance Monitor Event Selector 10 CSR. */
#define XREG_MHPMEVENT11			mhpmevent11 /**< Machine Hardware Performance Monitor Event Selector 11 CSR. */
#define XREG_MHPMEVENT12			mhpmevent12 /**< Machine Hardware Performance Monitor Event Selector 12 CSR. */
#define XREG_MHPMEVENT13			mhpmevent13 /**< Machine Hardware Performance Monitor Event Selector 13 CSR. */
#define XREG_MHPMEVENT14			mhpmevent14 /**< Machine Hardware Performance Monitor Event Selector 14 CSR. */
#define XREG_MHPMEVENT15			mhpmevent15 /**< Machine Hardware Performance Monitor Event Selector 15 CSR. */
#define XREG_MHPMEVENT16			mhpmevent16 /**< Machine Hardware Performance Monitor Event Selector 16 CSR. */
#define XREG_MHPMEVENT17			mhpmevent17 /**< Machine Hardware Performance Monitor Event Selector 17 CSR. */
#define XREG_MHPMEVENT18			mhpmevent18 /**< Machine Hardware Performance Monitor Event Selector 18 CSR. */
#define XREG_MHPMEVENT19			mhpmevent19 /**< Machine Hardware Performance Monitor Event Selector 19 CSR. */
#define XREG_MHPMEVENT20			mhpmevent20 /**< Machine Hardware Performance Monitor Event Selector 20 CSR. */
#define XREG_MHPMEVENT21			mhpmevent21 /**< Machine Hardware Performance Monitor Event Selector 21 CSR. */
#define XREG_MHPMEVENT22			mhpmevent22 /**< Machine Hardware Performance Monitor Event Selector 22 CSR. */
#define XREG_MHPMEVENT23			mhpmevent23 /**< Machine Hardware Performance Monitor Event Selector 23 CSR. */
#define XREG_MHPMEVENT24			mhpmevent24 /**< Machine Hardware Performance Monitor Event Selector 24 CSR. */
#define XREG_MHPMEVENT25			mhpmevent25 /**< Machine Hardware Performance Monitor Event Selector 25 CSR. */
#define XREG_MHPMEVENT26			mhpmevent26 /**< Machine Hardware Performance Monitor Event Selector 26 CSR. */
#define XREG_MHPMEVENT27			mhpmevent27 /**< Machine Hardware Performance Monitor Event Selector 27 CSR. */
#define XREG_MHPMEVENT28			mhpmevent28 /**< Machine Hardware Performance Monitor Event Selector 28 CSR. */
#define XREG_MHPMEVENT29			mhpmevent29 /**< Machine Hardware Performance Monitor Event Selector 29 CSR. */
#define XREG_MHPMEVENT30			mhpmevent30 /**< Machine Hardware Performance Monitor Event Selector 30 CSR. */
#define XREG_MHPMEVENT31			mhpmevent31 /**< Machine Hardware Performance Monitor Event Selector 31 CSR. */

/* Custom CSR Defines */
#define XREG_MSTREAM                           0x7C0 /**< Custom CSR: AXI stream access from user mode (0x7C0). */
#define XREG_MWFI                              0x7C4 /**< Custom CSR: machine WFI control (0x7C4). */
#define XREG_STREAM                            0x8C0 /**< Custom CSR: user-mode AXI stream access (0x8C0). */

/** @name MSTATUS MPP field
 *
 * MPP (bits 12:11) encodes the privilege mode mret restores control to.
 * @{
 */
#define XREG_MSTATUS_MPP_SHIFT                  (11U)         /**< Bit position of the MPP field. */
#define XREG_MSTATUS_MPP_MASK                   (0x00001800U) /**< Mask for the MPP field. */
#define XREG_MSTATUS_MPP_U                      (0x00000000U) /**< MPP value: mret restores U-mode. */
#define XREG_MSTATUS_MPP_M                      (0x00001800U) /**< MPP value: mret restores M-mode. */
/** @} */
#define XREG_MSTATUS_MSS_MASK                   (0x00001800U) /**< Mask for the MPP field in MSTATUS. */
#define XREG_MSTATUS_MPIE_BIT                   (7U) /**< Bit position of MPIE in MSTATUS. */
#define XREG_MSTATUS_MPIE_MASK                  (0x00000001U << XREG_MSTATUS_MPIE_BIT) /**< Mask for the MPIE bit in MSTATUS. */
#define XREG_MSTATUS_MIE_BIT                    (3U) /**< Bit position of MIE in MSTATUS. */
#define XREG_MSTATUS_MIE_MASK                   (0x00000001U << XREG_MSTATUS_MIE_BIT) /**< Mask for the MIE bit in MSTATUS. */

/** @name PMPCFG bits
 *
 * Per-entry PMP configuration byte: permission bits and the address-matching
 * mode (A) field.
 * @{
 */
#define XREG_PMPCFG_R_MASK                      (0x00000001U) /**< Read permission. */
#define XREG_PMPCFG_W_MASK                      (0x00000002U) /**< Write permission. */
#define XREG_PMPCFG_X_MASK                      (0x00000004U) /**< Execute permission. */
#define XREG_PMPCFG_A_BIT                       (3U)          /**< Bit position of the A field. */
#define XREG_PMPCFG_A_MASK                      (0x00000003U << XREG_PMPCFG_A_BIT) /**< Mask for the A field. */
#define XREG_PMPCFG_A_TOR                       (0x00000001U << XREG_PMPCFG_A_BIT) /**< A field: top-of-range. */
#define XREG_PMPCFG_A_NA4                       (0x00000002U << XREG_PMPCFG_A_BIT) /**< A field: naturally-aligned 4-byte region. */
#define XREG_PMPCFG_A_NAPOT                     (0x00000003U << XREG_PMPCFG_A_BIT) /**< A field: naturally-aligned power-of-two region. */
#define XREG_PMPCFG_L_MASK                      (0x00000080U) /**< Lock bit: entry stays locked until reset. */
/** @} */

/* MIE bits */
#define XREG_MIE_MEBE_BIT                       (16U) /**< Bit position of MEBE in MIE. */
#define XREG_MIE_MEBE_MASK                      (0x00000001U << XREG_MIE_MEBE_BIT) /**< Mask for the MEBE bit in MIE. */
#define XREG_MIE_MEIE_BIT                       (11U) /**< Bit position of MEIE in MIE. */
#define XREG_MIE_MEIE_MASK                      (0x00000001U << XREG_MIE_MEIE_BIT) /**< Mask for the MEIE bit in MIE. */
#define XREG_MIE_MTIE_BIT                       (7U) /**< Bit position of MTIE in MIE. */
#define XREG_MIE_MTIE_MASK                      (0x00000001U << XREG_MIE_MTIE_BIT) /**< Mask for the MTIE bit in MIE. */
#define XREG_MIE_MSIE_BIT                       (3U) /**< Bit position of MSIE in MIE. */
#define XREG_MIE_MSIE_MASK                      (0x00000001U << XREG_MIE_MSIE_BIT) /**< Mask for the MSIE bit in MIE. */

/* MWFI bits */
#define XREG_MWFI_SUSPEND_BIT                   (2U) /**< Bit position of SUSPEND in MWFI. */
#define XREG_MWFI_SUSPEND_MASK                  (0x00000001U << XREG_MWFI_SUSPEND_BIT) /**< Mask for the SUSPEND bit in MWFI. */
#define XREG_MWFI_HIBERNATE_BIT                 (1U) /**< Bit position of HIBERNATE in MWFI. */
#define XREG_MWFI_HIBERNATE_MASK                (0x00000001U << XREG_MWFI_HIBERNATE_BIT) /**< Mask for the HIBERNATE bit in MWFI. */
#define XREG_MWFI_SLEEP_BIT                     (0U) /**< Bit position of SLEEP in MWFI. */
#define XREG_MWFI_SLEEP_MASK                    (0x00000001U << XREG_MWFI_SLEEP_BIT) /**< Mask for the SLEEP bit in MWFI. */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XREG_RISCV_H */
