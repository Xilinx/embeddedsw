/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
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
* 1.00  sa       07/20/20 Initial version
* </pre>
*
******************************************************************************/
#ifndef XREG_RISCV_H	/* prevent circular inclusions */
#define XREG_RISCV_H	/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* GPRs */
#define XREG_GPR0				x0
#define XREG_GPR1				x1
#define XREG_GPR2				x2
#define XREG_GPR3				x3
#define XREG_GPR4				x4
#define XREG_GPR5				x5
#define XREG_GPR6				x6
#define XREG_GPR7				x7
#define XREG_GPR8				x8
#define XREG_GPR9				x9
#define XREG_GPR10				x10
#define XREG_GPR11				x11
#define XREG_GPR12				x12
#define XREG_GPR13				x13
#define XREG_GPR14				x14
#define XREG_GPR15				x15
#define XREG_GPR16				x16
#define XREG_GPR17				x17
#define XREG_GPR18				x18
#define XREG_GPR19				x19
#define XREG_GPR20				x20
#define XREG_GPR21				x21
#define XREG_GPR22				x22
#define XREG_GPR23				x23
#define XREG_GPR24				x24
#define XREG_GPR25				x25
#define XREG_GPR26				x26
#define XREG_GPR27				x27
#define XREG_GPR28				x28
#define XREG_GPR29				x29
#define XREG_GPR30				x30
#define XREG_GPR31				x31

#define XREG_ZERO                               XREG_GPR0    /* Hard-wired zero */
#define XREG_RA                                 XREG_GPR1    /* Return address */
#define XREG_SP                                 XREG_GPR2    /* Stack pointer */
#define XREG_GP                                 XREG_GPR3    /* Global pointer */
#define XREG_TP                                 XREG_GPR4    /* Thread pointer */
#define XREG_T0                                 XREG_GPR5    /* Temporary/alternate link register */
#define XREG_T1                                 XREG_GPR6    /* Temporaries */
#define XREG_T2                                 XREG_GPR7
#define XREG_S0                                 XREG_GPR8    /* Saved register */
#define XREG_FP                                 XREG_GPR8    /* Frame pointer */
#define XREG_S1                                 XREG_GPR9    /* Saved register */
#define XREG_A0                                 XREG_GPR10   /* Function arguments/return values */
#define XREG_A1                                 XREG_GPR11   /* Function arguments */
#define XREG_A2                                 XREG_GPR12
#define XREG_A3                                 XREG_GPR13
#define XREG_A4                                 XREG_GPR14
#define XREG_A5                                 XREG_GPR15
#define XREG_A6                                 XREG_GPR16
#define XREG_A7                                 XREG_GPR17
#define XREG_S2                                 XREG_GPR18   /* Saved registers */
#define XREG_S3                                 XREG_GPR19
#define XREG_S4                                 XREG_GPR20
#define XREG_S5                                 XREG_GPR21
#define XREG_S6                                 XREG_GPR22
#define XREG_S7                                 XREG_GPR23
#define XREG_S8                                 XREG_GPR24
#define XREG_S9                                 XREG_GPR25
#define XREG_S10                                XREG_GPR26
#define XREG_S11                                XREG_GPR27
#define XREG_T3                                 XREG_GPR28  /* Temporaries */
#define XREG_T4                                 XREG_GPR29
#define XREG_T5                                 XREG_GPR30
#define XREG_T6                                 XREG_GPR31

/* FPs */
#define XREG_FP0                                f0
#define XREG_FP1                                f1
#define XREG_FP2                                f2
#define XREG_FP3                                f3
#define XREG_FP4                                f4
#define XREG_FP5                                f5
#define XREG_FP6                                f6
#define XREG_FP7                                f7
#define XREG_FP8                                f8
#define XREG_FP9                                f9
#define XREG_FP10                               f10
#define XREG_FP11                               f11
#define XREG_FP12                               f12
#define XREG_FP13                               f13
#define XREG_FP14                               f14
#define XREG_FP15                               f15
#define XREG_FP16                               f16
#define XREG_FP17                               f17
#define XREG_FP18                               f18
#define XREG_FP19                               f19
#define XREG_FP20                               f20
#define XREG_FP21                               f21
#define XREG_FP22                               f22
#define XREG_FP23                               f23
#define XREG_FP24                               f24
#define XREG_FP25                               f25
#define XREG_FP26                               f26
#define XREG_FP27                               f27
#define XREG_FP28                               f28
#define XREG_FP29                               f29
#define XREG_FP30                               f30
#define XREG_FP31                               f31

#define XREG_FT0                                XREG_FP0    /* FP temporaries */
#define XREG_FT1                                XREG_FP1
#define XREG_FT2                                XREG_FP2
#define XREG_FT3                                XREG_FP3
#define XREG_FT4                                XREG_FP4
#define XREG_FT5                                XREG_FP5
#define XREG_FT6                                XREG_FP6
#define XREG_FT7                                XREG_FP7
#define XREG_FS0                                XREG_FP8    /* FP saved registers */
#define XREG_FS1                                XREG_FP9
#define XREG_FA0                                XREG_FP10   /* FP arguments/return values */
#define XREG_FA1                                XREG_FP11
#define XREG_FA2                                XREG_FP12   /* FP arguments */
#define XREG_FA3                                XREG_FP13
#define XREG_FA4                                XREG_FP14
#define XREG_FA5                                XREG_FP15
#define XREG_FA6                                XREG_FP16
#define XREG_FA7                                XREG_FP17
#define XREG_FS2                                XREG_FP18   /* FP saved registers */
#define XREG_FS3                                XREG_FP19
#define XREG_FS4                                XREG_FP20
#define XREG_FS5                                XREG_FP21
#define XREG_FS6                                XREG_FP22
#define XREG_FS7                                XREG_FP23
#define XREG_FS8                                XREG_FP24
#define XREG_FS9                                XREG_FP25
#define XREG_FS10                               XREG_FP26
#define XREG_FS11                               XREG_FP27
#define XREG_FT8                                XREG_FP28   /* FP temporaries */
#define XREG_FT9                                XREG_FP29
#define XREG_FT10                               XREG_FP30
#define XREG_FT11                               XREG_FP31

/* CSRs */

#define XREG_FFLAGS                             fflags      /* 0x001      */
#define XREG_FRM                                frm         /* 0x002      */
#define XREG_FCSR                               fcsr        /* 0x003      */

#define XREG_CYCLE                              cycle       /* 0xC00      */
#define XREG_TIME                               time        /* 0xC01      */
#define XREG_INSTRET                            instret     /* 0xC02      */
#define XREG_CYCLEH                             cycleh      /* 0xC80      */
#define XREG_TIMEH                              timeh       /* 0xC81      */
#define XREG_INSTRETH                           instreth    /* 0xC82      */

#define XREG_MVENDORID                          mvendorid   /* 0xF11      */
#define XREG_MARCHID                            marchid     /* 0xF12      */
#define XREG_MIMPID                             mimpid      /* 0xF13      */
#define XREG_MHARTID                            mhartid     /* 0xF14      */
#define XREG_MSTATUS                            mstatus     /* 0x300      */
#define XREG_MISA                               misa        /* 0x301      */
#define XREG_MEDELEG                            medeleg     /* 0x302      */
#define XREG_MIDELEG                            mideleg     /* 0x303      */
#define XREG_MIE                                mie         /* 0x304      */
#define XREG_MTVEC                              mtvec       /* 0x305      */
#define XREG_MCOUNTEREN                         mcounteren  /* 0x306      */
#define XREG_MSCRATCH                           mscratch    /* 0x340      */
#define XREG_MEPC                               mepc        /* 0x341      */
#define XREG_MCAUSE                             mcause      /* 0x342      */
#define XREG_MTVAL                              mtval       /* 0x343      */
#define XREG_MIP                                mip         /* 0x344      */

#define XREG_MSTREAM                            mstream     /* 0x7C0      */
#define XREG_MWFI                               mwfi        /* 0x7C4      */

/* MSTATUS bits */
#define XREG_MSTATUS_MSS_MASK                   (0x00001800U)
#define XREG_MSTATUS_MPIE_BIT                   (7U)
#define XREG_MSTATUS_MPIE_MASK                  (0x00000001U << XREG_MSTATUS_MPIE_BIT)
#define XREG_MSTATUS_MIE_BIT                    (3U)
#define XREG_MSTATUS_MIE_MASK                   (0x00000001U << XREG_MSTATUS_MIE_BIT)

/* MIE bits */
#define XREG_MIE_MEBE_BIT                       (16U)
#define XREG_MIE_MEBE_MASK                      (0x00000001U << XREG_MIE_MEBE_BIT)
#define XREG_MIE_MEIE_BIT                       (11U)
#define XREG_MIE_MEIE_MASK                      (0x00000001U << XREG_MIE_MEIE_BIT)
#define XREG_MIE_MTIE_BIT                       (7U)
#define XREG_MIE_MTIE_MASK                      (0x00000001U << XREG_MIE_MTIE_BIT)
#define XREG_MIE_MSIE_BIT                       (3U)
#define XREG_MIE_MSIE_MASK                      (0x00000001U << XREG_MIE_MSIE_BIT)

/* MWFI bits */
#define XREG_MWFI_SUSPEND_BIT                   (2U)
#define XREG_MWFI_SUSPEND_MASK                  (0x00000001U << XREG_MWFI_SUSPEND_BIT)
#define XREG_MWFI_HIBERNATE_BIT                 (1U)
#define XREG_MWFI_HIBERNATE_MASK                (0x00000001U << XREG_MWFI_HIBERNATE_BIT)
#define XREG_MWFI_SLEEP_BIT                     (0U)
#define XREG_MWFI_SLEEP_MASK                    (0x00000001U << XREG_MWFI_SLEEP_BIT)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XREG_RISCV_H */
