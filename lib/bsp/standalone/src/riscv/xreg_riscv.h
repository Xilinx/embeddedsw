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
* 9.0   sa       07/20/20 Initial version
* 9.0   sa       07/20/23 Added CSR definitions for  Hardware Performance
*                         Counters and Physical Memory Protection.
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
#define XREG_HPMCOUNTER3			hpmcounter3    /* 0xC03      */
#define XREG_HPMCOUNTER4			hpmcounter4    /* 0xC04      */
#define XREG_HPMCOUNTER5			hpmcounter5    /* 0xC05      */
#define XREG_HPMCOUNTER6			hpmcounter6    /* 0xC06      */
#define XREG_HPMCOUNTER7			hpmcounter7    /* 0xC07      */
#define XREG_HPMCOUNTER8			hpmcounter8    /* 0xC08      */
#define XREG_HPMCOUNTER9			hpmcounter9    /* 0xC09      */
#define XREG_HPMCOUNTER10			hpmcounter10   /* 0xC0A      */
#define XREG_HPMCOUNTER11			hpmcounter11   /* 0xC0B      */
#define XREG_HPMCOUNTER12			hpmcounter12   /* 0xC0C      */
#define XREG_HPMCOUNTER13			hpmcounter13   /* 0xC0D      */
#define XREG_HPMCOUNTER14			hpmcounter14   /* 0xC0E      */
#define XREG_HPMCOUNTER15			hpmcounter15   /* 0xC0F      */
#define XREG_HPMCOUNTER16			hpmcounter16   /* 0xC10      */
#define XREG_HPMCOUNTER17			hpmcounter17   /* 0xC11      */
#define XREG_HPMCOUNTER18			hpmcounter18   /* 0xC12      */
#define XREG_HPMCOUNTER19			hpmcounter19   /* 0xC13      */
#define XREG_HPMCOUNTER20			hpmcounter20   /* 0xC14      */
#define XREG_HPMCOUNTER21			hpmcounter21   /* 0xC15      */
#define XREG_HPMCOUNTER22			hpmcounter22   /* 0xC16      */
#define XREG_HPMCOUNTER23			hpmcounter23   /* 0xC17      */
#define XREG_HPMCOUNTER24			hpmcounter24   /* 0xC18      */
#define XREG_HPMCOUNTER25			hpmcounter25   /* 0xC19      */
#define XREG_HPMCOUNTER26			hpmcounter26   /* 0xC1A      */
#define XREG_HPMCOUNTER27			hpmcounter27   /* 0xC1B      */
#define XREG_HPMCOUNTER28			hpmcounter28   /* 0xC1C      */
#define XREG_HPMCOUNTER29			hpmcounter29   /* 0xC1D      */
#define XREG_HPMCOUNTER30			hpmcounter30   /* 0xC1E      */
#define XREG_HPMCOUNTER31			hpmcounter31   /* 0xC1F      */
#define XREG_CYCLEH                             cycleh      /* 0xC80      */
#define XREG_TIMEH                              timeh       /* 0xC81      */
#define XREG_INSTRETH                           instreth    /* 0xC82      */
#define XREG_HPMCOUNTER3H			hpmcounter3h   /* 0xC83      */
#define XREG_HPMCOUNTER4H			hpmcounter4h   /* 0xC84      */
#define XREG_HPMCOUNTER5H			hpmcounter5h   /* 0xC85      */
#define XREG_HPMCOUNTER6H			hpmcounter6h   /* 0xC86      */
#define XREG_HPMCOUNTER7H			hpmcounter7h   /* 0xC87      */
#define XREG_HPMCOUNTER8H			hpmcounter8h   /* 0xC88      */
#define XREG_HPMCOUNTER9H			hpmcounter9h   /* 0xC89      */
#define XREG_HPMCOUNTER10H			hpmcounter10h  /* 0xC8A      */
#define XREG_HPMCOUNTER11H			hpmcounter11h  /* 0xC8B      */
#define XREG_HPMCOUNTER12H			hpmcounter12h  /* 0xC8C      */
#define XREG_HPMCOUNTER13H			hpmcounter13h  /* 0xC8D      */
#define XREG_HPMCOUNTER14H			hpmcounter14h  /* 0xC8E      */
#define XREG_HPMCOUNTER15H			hpmcounter15h  /* 0xC8F      */
#define XREG_HPMCOUNTER16H			hpmcounter16h  /* 0xC90      */
#define XREG_HPMCOUNTER17H			hpmcounter17h  /* 0xC91      */
#define XREG_HPMCOUNTER18H			hpmcounter18h  /* 0xC92      */
#define XREG_HPMCOUNTER19H			hpmcounter19h  /* 0xC93      */
#define XREG_HPMCOUNTER20H			hpmcounter20h  /* 0xC94      */
#define XREG_HPMCOUNTER21H			hpmcounter21h  /* 0xC95      */
#define XREG_HPMCOUNTER22H			hpmcounter22h  /* 0xC96      */
#define XREG_HPMCOUNTER23H			hpmcounter23h  /* 0xC97      */
#define XREG_HPMCOUNTER24H			hpmcounter24h  /* 0xC98      */
#define XREG_HPMCOUNTER25H			hpmcounter25h  /* 0xC99      */
#define XREG_HPMCOUNTER26H			hpmcounter26h  /* 0xC9A      */
#define XREG_HPMCOUNTER27H			hpmcounter27h  /* 0xC9B      */
#define XREG_HPMCOUNTER28H			hpmcounter28h  /* 0xC9C      */
#define XREG_HPMCOUNTER29H			hpmcounter29h  /* 0xC9D      */
#define XREG_HPMCOUNTER30H			hpmcounter30h  /* 0xC9E      */
#define XREG_HPMCOUNTER31H			hpmcounter31h  /* 0xC9F      */

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
#define XREG_PMPCFG0				pmpcfg0        /* 0x3A0      */
#define XREG_PMPCFG1				pmpcfg1        /* 0x3A1      */
#define XREG_PMPCFG2				pmpcfg2        /* 0x3A2      */
#define XREG_PMPCFG3				pmpcfg3        /* 0x3A3      */
#define XREG_PMPCFG4				pmpcfg4        /* 0x3A4      */
#define XREG_PMPCFG5				pmpcfg5        /* 0x3A5      */
#define XREG_PMPCFG6				pmpcfg6        /* 0x3A6      */
#define XREG_PMPCFG7				pmpcfg7        /* 0x3A7      */
#define XREG_PMPCFG8				pmpcfg8        /* 0x3A8      */
#define XREG_PMPCFG9				pmpcfg9        /* 0x3A9      */
#define XREG_PMPCFG10				pmpcfg10       /* 0x3AA      */
#define XREG_PMPCFG11				pmpcfg11       /* 0x3AB      */
#define XREG_PMPCFG12				pmpcfg12       /* 0x3AC      */
#define XREG_PMPCFG13				pmpcfg13       /* 0x3AD      */
#define XREG_PMPCFG14				pmpcfg14       /* 0x3AE      */
#define XREG_PMPCFG15				pmpcfg15       /* 0x3AF      */
#define XREG_PMPADDR0				pmpaddr0       /* 0x3B0      */
#define XREG_PMPADDR1				pmpaddr1       /* 0x3B1      */
#define XREG_PMPADDR2				pmpaddr2       /* 0x3B2      */
#define XREG_PMPADDR3				pmpaddr3       /* 0x3B3      */
#define XREG_PMPADDR4				pmpaddr4       /* 0x3B4      */
#define XREG_PMPADDR5				pmpaddr5       /* 0x3B5      */
#define XREG_PMPADDR6				pmpaddr6       /* 0x3B6      */
#define XREG_PMPADDR7				pmpaddr7       /* 0x3B7      */
#define XREG_PMPADDR8				pmpaddr8       /* 0x3B8      */
#define XREG_PMPADDR9				pmpaddr9       /* 0x3B9      */
#define XREG_PMPADDR10				pmpaddr10      /* 0x3BA      */
#define XREG_PMPADDR11				pmpaddr11      /* 0x3BB      */
#define XREG_PMPADDR12				pmpaddr12      /* 0x3BC      */
#define XREG_PMPADDR13				pmpaddr13      /* 0x3BD      */
#define XREG_PMPADDR14				pmpaddr14      /* 0x3BE      */
#define XREG_PMPADDR15				pmpaddr15      /* 0x3BF      */
#define XREG_PMPADDR16				pmpaddr16      /* 0x3C0      */
#define XREG_PMPADDR17				pmpaddr17      /* 0x3C1      */
#define XREG_PMPADDR18				pmpaddr18      /* 0x3C2      */
#define XREG_PMPADDR19				pmpaddr19      /* 0x3C3      */
#define XREG_PMPADDR20				pmpaddr20      /* 0x3C4      */
#define XREG_PMPADDR21				pmpaddr21      /* 0x3C5      */
#define XREG_PMPADDR22				pmpaddr22      /* 0x3C6      */
#define XREG_PMPADDR23				pmpaddr23      /* 0x3C7      */
#define XREG_PMPADDR24				pmpaddr24      /* 0x3C8      */
#define XREG_PMPADDR25				pmpaddr25      /* 0x3C9      */
#define XREG_PMPADDR26				pmpaddr26      /* 0x3CA      */
#define XREG_PMPADDR27				pmpaddr27      /* 0x3CB      */
#define XREG_PMPADDR28				pmpaddr28      /* 0x3CC      */
#define XREG_PMPADDR29				pmpaddr29      /* 0x3CD      */
#define XREG_PMPADDR30				pmpaddr30      /* 0x3CE      */
#define XREG_PMPADDR31				pmpaddr31      /* 0x3CF      */
#define XREG_PMPADDR32				pmpaddr32      /* 0x3D0      */
#define XREG_PMPADDR33				pmpaddr33      /* 0x3D1      */
#define XREG_PMPADDR34				pmpaddr34      /* 0x3D2      */
#define XREG_PMPADDR35				pmpaddr35      /* 0x3D3      */
#define XREG_PMPADDR36				pmpaddr36      /* 0x3D4      */
#define XREG_PMPADDR37				pmpaddr37      /* 0x3D5      */
#define XREG_PMPADDR38				pmpaddr38      /* 0x3D6      */
#define XREG_PMPADDR39				pmpaddr39      /* 0x3D7      */
#define XREG_PMPADDR40				pmpaddr40      /* 0x3D8      */
#define XREG_PMPADDR41				pmpaddr41      /* 0x3D9      */
#define XREG_PMPADDR42				pmpaddr42      /* 0x3DA      */
#define XREG_PMPADDR43				pmpaddr43      /* 0x3DB      */
#define XREG_PMPADDR44				pmpaddr44      /* 0x3DC      */
#define XREG_PMPADDR45				pmpaddr45      /* 0x3DD      */
#define XREG_PMPADDR46				pmpaddr46      /* 0x3DE      */
#define XREG_PMPADDR47				pmpaddr47      /* 0x3DF      */
#define XREG_PMPADDR48				pmpaddr48      /* 0x3E0      */
#define XREG_PMPADDR49				pmpaddr49      /* 0x3E1      */
#define XREG_PMPADDR50				pmpaddr50      /* 0x3E2      */
#define XREG_PMPADDR51				pmpaddr51      /* 0x3E3      */
#define XREG_PMPADDR52				pmpaddr52      /* 0x3E4      */
#define XREG_PMPADDR53				pmpaddr53      /* 0x3E5      */
#define XREG_PMPADDR54				pmpaddr54      /* 0x3E6      */
#define XREG_PMPADDR55				pmpaddr55      /* 0x3E7      */
#define XREG_PMPADDR56				pmpaddr56      /* 0x3E8      */
#define XREG_PMPADDR57				pmpaddr57      /* 0x3E9      */
#define XREG_PMPADDR58				pmpaddr58      /* 0x3EA      */
#define XREG_PMPADDR59				pmpaddr59      /* 0x3EB      */
#define XREG_PMPADDR60				pmpaddr60      /* 0x3EC      */
#define XREG_PMPADDR61				pmpaddr61      /* 0x3ED      */
#define XREG_PMPADDR62				pmpaddr62      /* 0x3EE      */
#define XREG_PMPADDR63				pmpaddr63      /* 0x3EF      */

#define XREG_MCYCLE				mcycle         /* 0xB00      */
#define XREG_MINSTRET				minstret       /* 0xB02      */
#define XREG_MHPMCOUNTER3			mhpmcounter3   /* 0xB03      */
#define XREG_MHPMCOUNTER4			mhpmcounter4   /* 0xB04      */
#define XREG_MHPMCOUNTER5			mhpmcounter5   /* 0xB05      */
#define XREG_MHPMCOUNTER6			mhpmcounter6   /* 0xB06      */
#define XREG_MHPMCOUNTER7			mhpmcounter7   /* 0xB07      */
#define XREG_MHPMCOUNTER8			mhpmcounter8   /* 0xB08      */
#define XREG_MHPMCOUNTER9			mhpmcounter9   /* 0xB09      */
#define XREG_MHPMCOUNTER10			mhpmcounter10  /* 0xB0A      */
#define XREG_MHPMCOUNTER11			mhpmcounter11  /* 0xB0B      */
#define XREG_MHPMCOUNTER12			mhpmcounter12  /* 0xB0C      */
#define XREG_MHPMCOUNTER13			mhpmcounter13  /* 0xB0D      */
#define XREG_MHPMCOUNTER14			mhpmcounter14  /* 0xB0E      */
#define XREG_MHPMCOUNTER15			mhpmcounter15  /* 0xB0F      */
#define XREG_MHPMCOUNTER16			mhpmcounter16  /* 0xB10      */
#define XREG_MHPMCOUNTER17			mhpmcounter17  /* 0xB11      */
#define XREG_MHPMCOUNTER18			mhpmcounter18  /* 0xB12      */
#define XREG_MHPMCOUNTER19			mhpmcounter19  /* 0xB13      */
#define XREG_MHPMCOUNTER20			mhpmcounter20  /* 0xB14      */
#define XREG_MHPMCOUNTER21			mhpmcounter21  /* 0xB15      */
#define XREG_MHPMCOUNTER22			mhpmcounter22  /* 0xB16      */
#define XREG_MHPMCOUNTER23			mhpmcounter23  /* 0xB17      */
#define XREG_MHPMCOUNTER24			mhpmcounter24  /* 0xB18      */
#define XREG_MHPMCOUNTER25			mhpmcounter25  /* 0xB19      */
#define XREG_MHPMCOUNTER26			mhpmcounter26  /* 0xB1A      */
#define XREG_MHPMCOUNTER27			mhpmcounter27  /* 0xB1B      */
#define XREG_MHPMCOUNTER28			mhpmcounter28  /* 0xB1C      */
#define XREG_MHPMCOUNTER29			mhpmcounter29  /* 0xB1D      */
#define XREG_MHPMCOUNTER30			mhpmcounter30  /* 0xB1E      */
#define XREG_MHPMCOUNTER31			mhpmcounter31  /* 0xB1F      */
#define XREG_MCYCLEH				mcycleh        /* 0xB80      */
#define XREG_MINSTRETH				minstreth      /* 0xB82      */
#define XREG_MHPMCOUNTER3H			mhpmcounter3h  /* 0xB83      */
#define XREG_MHPMCOUNTER4H			mhpmcounter4h  /* 0xB84      */
#define XREG_MHPMCOUNTER5H			mhpmcounter5h  /* 0xB85      */
#define XREG_MHPMCOUNTER6H			mhpmcounter6h  /* 0xB86      */
#define XREG_MHPMCOUNTER7H			mhpmcounter7h  /* 0xB87      */
#define XREG_MHPMCOUNTER8H			mhpmcounter8h  /* 0xB88      */
#define XREG_MHPMCOUNTER9H			mhpmcounter9h  /* 0xB89      */
#define XREG_MHPMCOUNTER10H			mhpmcounter10h /* 0xB8A      */
#define XREG_MHPMCOUNTER11H			mhpmcounter11h /* 0xB8B      */
#define XREG_MHPMCOUNTER12H			mhpmcounter12h /* 0xB8C      */
#define XREG_MHPMCOUNTER13H			mhpmcounter13h /* 0xB8D      */
#define XREG_MHPMCOUNTER14H			mhpmcounter14h /* 0xB8E      */
#define XREG_MHPMCOUNTER15H			mhpmcounter15h /* 0xB8F      */
#define XREG_MHPMCOUNTER16H			mhpmcounter16h /* 0xB90      */
#define XREG_MHPMCOUNTER17H			mhpmcounter17h /* 0xB91      */
#define XREG_MHPMCOUNTER18H			mhpmcounter18h /* 0xB92      */
#define XREG_MHPMCOUNTER19H			mhpmcounter19h /* 0xB93      */
#define XREG_MHPMCOUNTER20H			mhpmcounter20h /* 0xB94      */
#define XREG_MHPMCOUNTER21H			mhpmcounter21h /* 0xB95      */
#define XREG_MHPMCOUNTER22H			mhpmcounter22h /* 0xB96      */
#define XREG_MHPMCOUNTER23H			mhpmcounter23h /* 0xB97      */
#define XREG_MHPMCOUNTER24H			mhpmcounter24h /* 0xB98      */
#define XREG_MHPMCOUNTER25H			mhpmcounter25h /* 0xB99      */
#define XREG_MHPMCOUNTER26H			mhpmcounter26h /* 0xB9A      */
#define XREG_MHPMCOUNTER27H			mhpmcounter27h /* 0xB9B      */
#define XREG_MHPMCOUNTER28H			mhpmcounter28h /* 0xB9C      */
#define XREG_MHPMCOUNTER29H			mhpmcounter29h /* 0xB9D      */
#define XREG_MHPMCOUNTER30H			mhpmcounter30h /* 0xB9E      */
#define XREG_MHPMCOUNTER31H			mhpmcounter31h /* 0xB9F      */

#define XREG_MCOUNTINHIBIT			mcountinhibit  /* 0x320      */
#define XREG_MHPMEVENT3				mhpmevent3    /* 0x323      */
#define XREG_MHPMEVENT4				mhpmevent4    /* 0x324      */
#define XREG_MHPMEVENT5				mhpmevent5    /* 0x325      */
#define XREG_MHPMEVENT6				mhpmevent6    /* 0x326      */
#define XREG_MHPMEVENT7				mhpmevent7    /* 0x327      */
#define XREG_MHPMEVENT8				mhpmevent8    /* 0x328      */
#define XREG_MHPMEVENT9				mhpmevent9    /* 0x329      */
#define XREG_MHPMEVENT10			mhpmevent10   /* 0x32A      */
#define XREG_MHPMEVENT11			mhpmevent11   /* 0x32B      */
#define XREG_MHPMEVENT12			mhpmevent12   /* 0x32C      */
#define XREG_MHPMEVENT13			mhpmevent13   /* 0x32D      */
#define XREG_MHPMEVENT14			mhpmevent14   /* 0x32E      */
#define XREG_MHPMEVENT15			mhpmevent15   /* 0x32F      */
#define XREG_MHPMEVENT16			mhpmevent16   /* 0x330      */
#define XREG_MHPMEVENT17			mhpmevent17   /* 0x331      */
#define XREG_MHPMEVENT18			mhpmevent18   /* 0x332      */
#define XREG_MHPMEVENT19			mhpmevent19   /* 0x333      */
#define XREG_MHPMEVENT20			mhpmevent20   /* 0x334      */
#define XREG_MHPMEVENT21			mhpmevent21   /* 0x335      */
#define XREG_MHPMEVENT22			mhpmevent22   /* 0x336      */
#define XREG_MHPMEVENT23			mhpmevent23   /* 0x337      */
#define XREG_MHPMEVENT24			mhpmevent24   /* 0x338      */
#define XREG_MHPMEVENT25			mhpmevent25   /* 0x339      */
#define XREG_MHPMEVENT26			mhpmevent26   /* 0x33A      */
#define XREG_MHPMEVENT27			mhpmevent27   /* 0x33B      */
#define XREG_MHPMEVENT28			mhpmevent28   /* 0x33C      */
#define XREG_MHPMEVENT29			mhpmevent29   /* 0x33D      */
#define XREG_MHPMEVENT30			mhpmevent30   /* 0x33E      */
#define XREG_MHPMEVENT31			mhpmevent31   /* 0x33F      */

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
