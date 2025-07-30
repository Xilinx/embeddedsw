/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file translation_table.s
*
* @addtogroup Versal_2VE_2VM_apu_boot_code
* @{
* <h2> translation_table.S </h2>
* The translation_table.S contains a static page table required by MMU for
* cortex-A78. This translation table is flat mapped (input address = output
* address) with default memory attributes defined for Versal_2VE_2VM
* architecture. It utilizes translation granual size of 4 KB with 2 MB section
* size for initial 4 GB memory and 1GB section size for memory after 4 GB.
* The overview of translation table memory attributes is described below.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 9.2  mus  07/18/24 Initial version
* 9.3  mus  01/07/25 Update translation table to mark all DDR regions as
*                    normal memory.
* 9.3  adk  04/11/25 Updated the translation table to reflect DDR regions
*		     in accordance with the design configuration.
* 9.4  ml   06/12/25 Removed irrelevent comment
******************************************************************************/
#include "bspconfig.h"

#ifndef SDT
#include "xparameters.h"
#else
#include "xmem_config.h"
#endif

	.globl  MMUTableL0
	.globl  MMUTableL1
	.globl  MMUTableL2

	.set UNDEF_START, 0

	/* Base addresses for all DDR regions in Versal_2VE_2VM SoC TODO: revisit */
	.set DDR_LOW_0_REGION_START_ADDR, 0x0000000000		/* 0x0000_0000_0000 */
	.set DDR_LOW_1_REGION_START_ADDR, 0x800000000		/* 0x0008_0000_0000 */
	.set DDR_LOW_2_REGION_START_ADDR, 0xC000000000		/* 0x00C0_0000_0000 */
	.set DDR_LOW_3_REGION_START_ADDR, 0x10000000000		/* 0x0100_0000_0000 */
	.set DDR_CH_1_REGION_START_ADDR, 0x50000000000		/* 0x0500_0000_0000 */
	.set DDR_CH_2_REGION_START_ADDR, 0x60000000000		/* 0x0600_0000_0000 */
	.set DDR_CH_3_REGION_START_ADDR, 0x70000000000		/* 0x0700_0000_0000 */

	.set reserved,	0x0 					/* Fault*/
	.set Memory_NonCacheable,	0x401 		/* normal non-cacheable */
	#if EL1_NONSECURE
	.set Memory,	0x405 | (2 << 8) | (0x0)		/* normal writeback write allocate outer shared read write */
	#else
	.set Memory,	0x405 | (3 << 8) | (0x0)		/* normal writeback write allocate inner shared read write */
	#endif
	.set Device,	0x409 | (1 << 53)| (1 << 54) |(0x0)	/* strongly ordered read write non executable*/

	.section .mmu_tbl0,"a"

MMUTableL0:

/*
 * 96 sections each with 512 GB size covers 0 - 47 TB.
 * 48TB - 256TB is reserved in Versal_2VE_2VM address map,
 * so not defining sections for that region.
 */

.set SECT, MMUTableL1		/* 0x0000_0000 -  0x7F_FFFF_FFFF */
.8byte	SECT + 0x3

#if ! defined  (OCM_ELF) && ! defined (ENABLE_MINIMAL_XLAT_TBL)
.rept	0x5f
.set SECT, SECT + 0x1000	/* 0x80_0000_0000 - 0xFFF_FFFF_FFFF */
.8byte	SECT + 0x3
.endr
#else
.rept	0x7
.set SECT, SECT + 0x1000        /* 0x80_0000_0000 - 0xFFF_FFFF_FFFF */
.8byte  SECT + 0x3
.endr
#endif

	.section .mmu_tbl1,"a"

MMUTableL1:

/*
 * For first 4GB of address space provide granularity
 * of 2MB.
 */

.set SECT, MMUTableL2		/* 0x0000_0000 - 0x3FFF_FFFF */
.8byte	SECT + 0x3		/* 1GB DDR */

.rept	0x3			/* 0x4000_0000 - 0x1_3FFF_FFFF */
.set SECT, SECT + 0x1000	/* DDR, PS_LPD_AFI_FS, PCIE_REGION0, PS_FPD_AFI_FS, OCM, xSPI, FPD_STM_CORESIGHT,*/
.8byte	SECT + 0x3              /* FPD GIC, PS_FPD_CMN, PMC, CPM, FPD slaves, LPD slaves, PMC_ALIAS_REGION0-3, FPD_PKI */
.endr                           /* HNIC, Multimedia tiles */

.set SECT,0x100000000

.rept	0x1C			/* 0x1_0000_0000 - 0x7_FFFF_FFFF */
.8byte	SECT + reserved		/* 28GB Reserved */
.set SECT, SECT + 0x40000000
.endr

#if defined(XPAR_AXI_NOC2_DDR_LOW_1_BASEADDR)
.set DDR_1_START, XPAR_AXI_NOC2_DDR_LOW_1_BASEADDR
.set DDR_1_END, XPAR_AXI_NOC2_DDR_LOW_1_HIGHADDR

.if DDR_1_START > DDR_LOW_1_REGION_START_ADDR
.set UNDEF_START, (DDR_1_START - DDR_LOW_1_REGION_START_ADDR)
.set UNDEF_START, (UNDEF_START)/0x40000000
.endif

.set DDR_1_SIZE, (DDR_1_END - DDR_1_START)+1
.if DDR_1_SIZE > 0x800000000
/* If DDR size is larger than 32GB, truncate to 32GB */
.set DDR_1_REG, 0x20
.else
.set DDR_1_REG, DDR_1_SIZE/0x40000000
.endif
#else
.set DDR_1_REG, 0
#warning "There's no DDR_1 in the HW design. MMU translation table marks 32 GB DDR address space as undefined"
#endif

.set UNDEF_1_REG, (0x20 - DDR_1_REG - UNDEF_START)

.rept   UNDEF_START                             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept   DDR_1_REG                       /* DDR based on size in hdf*/
.8byte  SECT + Memory
.set    SECT, SECT+0x40000000
.endr

.rept   UNDEF_1_REG             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept	0x40			/* 0x10_0000_0000 - 0x1F_FFFF_FFFF */
.8byte	SECT + reserved		/* 64GB Reserved */
.set SECT, SECT + 0x40000000
.endr

.rept	0x80			/* 0x20_0000_0000 - 0x3F_FFFF_FFFF */
.8byte	SECT + Device		/* 128GB C2C_MED */
.set SECT, SECT + 0x40000000
.endr

/* TODO: Mark HBM regions as normal memory based on the HW design */
.rept	0x100			/* 0x40_0000_0000 - 0x7F_FFFF_FFFF */
.8byte	SECT + reserved		/* 256GB device */
.set SECT, SECT + 0x40000000
.endr

.rept	0x100			/* 0x80_0000_0000 - 0xBF_FFFF_FFFF */
.8byte	SECT + reserved		/* 256GB reserved */
.set SECT, SECT + 0x40000000
.endr

.set UNDEF_START, 0
#ifdef XPAR_AXI_NOC2_DDR_LOW_2_BASEADDR
.set DDR_START, XPAR_AXI_NOC2_DDR_LOW_2_BASEADDR
.set DDR_END, XPAR_AXI_NOC2_DDR_LOW_2_HIGHADDR

.if DDR_START > DDR_LOW_2_REGION_START_ADDR
.set UNDEF_START, (DDR_START - DDR_LOW_2_REGION_START_ADDR)
.set UNDEF_START, (UNDEF_START)/0x40000000
.endif

.set DDR_SIZE, (DDR_END - DDR_START)+1
.if DDR_SIZE > 0x4000000000
/* If DDR size is larger than 256GB, truncate to 256GB */
.set DDR_REG, 0x100
.else
.set DDR_REG, DDR_SIZE/0x40000000
.endif
#else
.set DDR_REG, 0
#warning "There's no DDRMC0_REGION2 in the HW design. MMU translation table marks 256GB DDR address space as undefined"
#endif

.set UNDEF_REG, (0x100 - DDR_REG - UNDEF_START)

.rept   UNDEF_START                             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept   DDR_REG         /*  DDR based on size in xsa 0xC0_0000_0000 - 0xFF_FFFF_FFFF */
.8byte  SECT + Memory           /* Maximum DDR region size - 256GB */
.set SECT, SECT + 0x40000000
.endr

.rept   UNDEF_REG               /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.set UNDEF_START, 0
#ifdef XPAR_AXI_NOC2_DDR_LOW_3_BASEADDR
.set DDR_START, XPAR_AXI_NOC2_DDR_LOW_3_BASEADDR
.set DDR_END, XPAR_AXI_NOC2_DDR_LOW_3_HIGHADDR

.if DDR_START > DDR_LOW_3_REGION_START_ADDR
.set UNDEF_START, (DDR_START - DDR_LOW_3_REGION_START_ADDR)
.set UNDEF_START, (UNDEF_START)/0x40000000
.endif

.set DDR_SIZE, (DDR_END - DDR_START)+1
.if DDR_SIZE > 0xB780000000
/* If DDR size is larger than 734GB, truncate to 734GB */
.set DDR_REG, 0x2DE
.else
.set DDR_REG, DDR_SIZE/0x40000000
.endif
#else
.set DDR_REG, 0
#warning "There's no DDRMC0_REGION3 in the HW design. MMU translation table marks 734GB DDR address space as undefined"
#endif

.set UNDEF_REG, (0x2DE - DDR_REG - UNDEF_START)

.rept   UNDEF_START                             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept   DDR_REG         /*  DDR based on size in xsa 0x100_0000_0000 - 0x1B&_7FFF_FFFF */
.8byte  SECT + Memory           /* Maximum DDR region size - 734GB */
.set SECT, SECT + 0x40000000
.endr

.rept   UNDEF_REG               /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept   0x122                   /* 0x1B8_0000_0000 - 0x1FF_FFFF_FFFF */
.8byte  SECT + reserved         /* 290GB DDR */
.set SECT, SECT + 0x40000000
.endr


.rept	0x4			/* 0x200_0000_0000 - 0x200_FFFF_FFFF */
.8byte	SECT + Device		/* 4GB ME Programming */
.set SECT, SECT + 0x40000000
.endr

.rept   0x4                     /* 0x201_0000_0000 - 0x201_FFFF_FFFF */
.8byte  SECT + reserved         /* 4GB reserved */
.set SECT, SECT + 0x40000000
.endr

.rept   0x7                     /* 0x202_0000_0000 - 0x203_BFFF_FFFF */
.8byte  SECT + Device           /* 2GB PS_FPD_AFI_FS_CONFIG + 2GB C2C_CONFIG + 2GB PL_VIA_NOC_CONFIG + 512MB PMC_ALIAS0-3 */
.set SECT, SECT + 0x40000000
.endr

.8byte  SECT + reserved         /* 1GB reserved  0x203_C000_0000 - 0x203_FFFF_FFFF */
.set SECT, SECT + 0x40000000

.8byte  SECT + Device           /* 8MB PKI, 1016MB reserved from next section  0x204_0000_0000 - 0x204_3FFF_FFFF */
.set SECT, SECT + 0x40000000

.rept	0x7EF			/* 0x204_4000_0000 - 0x3ffffffffff */
.8byte	SECT + Device		/* 2031GB reserved */
.set SECT, SECT + 0x40000000
.endr

#if ! defined (OCM_ELF) && ! defined (ENABLE_MINIMAL_XLAT_TBL)
.rept	0x400			/* 0x400_0000_0000 - 0x4FF_FFFF_FFFF */
.8byte	SECT + Device		/* 1TB PS_FPD_AFI_FS_HIGH */
.set SECT, SECT + 0x40000000
.endr

.set UNDEF_START, 0
#ifdef XPAR_AXI_NOC2_DDR_CH_1_BASEADDR
.set DDR_START, XPAR_AXI_NOC2_DDR_CH_1_BASEADDR
.set DDR_END, XPAR_AXI_NOC2_DDR_CH_1_HIGHADDR

.if DDR_START > DDR_CH_1_REGION_START_ADDR
.set UNDEF_START, (DDR_START - DDR_CH_1_REGION_START_ADDR)
.set UNDEF_START, (UNDEF_START)/0x40000000
.endif

.set DDR_SIZE, (DDR_END - DDR_START)+1
.if DDR_SIZE > 0x10000000000
/* If DDR size is larger than 1TB, truncate to 1TB */
.set DDR_REG, 0x400
.else
.set DDR_REG, DDR_SIZE/0x40000000
.endif
#else
.set DDR_REG, 0
#warning "There's no DDRMC1_mem in the HW design. MMU translation table marks 1TB DDR address space as undefined"
#endif

.set UNDEF_REG, (0x400 - DDR_REG - UNDEF_START)

.rept   UNDEF_START                             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept   DDR_REG         /*  DDR based on size in xsa 0x500_0000_0000 - 0x5FF_FFFF_FFFF */
.8byte  SECT + Memory           /* Maximum DDR region size - 1TB */
.set SECT, SECT + 0x40000000
.endr

.rept   UNDEF_REG               /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.set UNDEF_START, 0
#ifdef XPAR_AXI_NOC2_DDR_CH_1A_BASEADDR
.set DDR_START, XPAR_AXI_NOC2_DDR_CH_1A_BASEADDR
.set DDR_END, XPAR_AXI_NOC2_DDR_CH_1A_HIGHADDR

.if DDR_START > DDR_CH_2_REGION_START_ADDR
.set UNDEF_START, (DDR_START - DDR_CH_2_REGION_START_ADDR)
.set UNDEF_START, (UNDEF_START)/0x40000000
.endif

.set DDR_SIZE, (DDR_END - DDR_START)+1
.if DDR_SIZE > 0x10000000000
/* If DDR size is larger than 1TB, truncate to 1TB */
.set DDR_REG, 0x400
.else
.set DDR_REG, DDR_SIZE/0x40000000
.endif
#else
.set DDR_REG, 0
#warning "There's no DDRMC2_mem in the HW design. MMU translation table marks 1TB DDR address space as undefined"
#endif

.set UNDEF_REG, (0x400 - DDR_REG - UNDEF_START)

.rept   UNDEF_START                             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept   DDR_REG         /*  DDR based on size in xsa 0x600_0000_0000 - 0x6FF_FFFF_FFFF */
.8byte  SECT + Memory           /* Maximum DDR region size - 1TB */
.set SECT, SECT + 0x40000000
.endr

.rept   UNDEF_REG               /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.set UNDEF_START, 0
#ifdef XPAR_AXI_NOC2_DDR_CH_2_BASEADDR
.set DDR_START, XPAR_AXI_NOC2_DDR_CH_2_BASEADDR
.set DDR_END, XPAR_AXI_NOC2_DDR_CH_2_HIGHADDR

.if DDR_START > DDR_CH_3_REGION_START_ADDR
.set UNDEF_START, (DDR_START - DDR_CH_3_REGION_START_ADDR)
.set UNDEF_START, (UNDEF_START)/0x40000000
.endif

.set DDR_SIZE, (DDR_END - DDR_START)+1
.if DDR_SIZE > 0x10000000000
/* If DDR size is larger than 1TB, truncate to 1TB */
.set DDR_REG, 0x400
.else
.set DDR_REG, DDR_SIZE/0x40000000
.endif
#else
.set DDR_REG, 0
#warning "There's no DDRMC3_mem in the HW design. MMU translation table marks 1TB DDR address space as undefined"
#endif

.set UNDEF_REG, (0x400 - DDR_REG - UNDEF_START)

.rept   UNDEF_START                             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept   DDR_REG         /*  DDR based on size in xsa 0x600_0000_0000 - 0x6FF_FFFF_FFFF */
.8byte  SECT + Memory           /* Maximum DDR region size - 1TB */
.set SECT, SECT + 0x40000000
.endr

.rept   UNDEF_REG               /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x40000000
.endr

.rept	0x2000			/* 0x800_0000_0000 - 0xFFF_FFFF_FFFF */
.8byte	SECT + Device		/* 8TB PL_VIA_NOC_HIGH*/
.set SECT, SECT + 0x40000000
.endr

.rept	0x2000			/* 0x1000_0000_0000 - 0x17FF_FFFF_FFFF */
.8byte	SECT + Device		/* 8TB CPM_CDX_DPU_HIGH */
.set SECT, SECT + 0x40000000
.endr

/* TODO: Mark this region as normal memory based on the HW design */
.rept	0x1800			/* 0x1800_0000_0000 - 0x1DFF_FFFF_FFFF */
.8byte	SECT + Memory		/* 6TB DDR: 12 DDR regions (DDR_CH4 to DDR_CH15) each of size 512GB */
.set SECT, SECT + 0x40000000
.endr

.rept	0x800			/* 0x1E00_0000_0000 - 0x1FFF_FFFF_FFFF */
.8byte	SECT + reserved		/* 2TB reserved */
.set SECT, SECT + 0x40000000
.endr

.rept	0x4000			/* 0x2000_0000_0000 - 0x2FFF_FFFF_FFFF */
.8byte	SECT + Device		/* 16TB C2C_HIGH */
.set SECT, SECT + 0x40000000
.endr
#endif

.section .mmu_tbl2,"a"

MMUTableL2:

.set SECT, 0
.set UNDEF_START, 0
#if defined(XPAR_AXI_NOC2_DDR_LOW_0_BASEADDR)
.set DDR_0_START, XPAR_AXI_NOC2_DDR_LOW_0_BASEADDR
.set DDR_0_END, XPAR_AXI_NOC2_DDR_LOW_0_HIGHADDR

.if DDR_0_START > DDR_LOW_0_REGION_START_ADDR
.set UNDEF_START, (DDR_0_START - DDR_LOW_0_REGION_START_ADDR)
.set UNDEF_START, (UNDEF_START)/0x200000
.endif

.set DDR_0_SIZE, (DDR_0_END - DDR_0_START)+1
.if DDR_0_SIZE > 0x80000000
/* If DDR size is larger than 2GB, truncate to 2GB */
.set DDR_0_REG, 0x400
.else
.set DDR_0_REG, DDR_0_SIZE/0x200000
.set TEMP, DDR_0_SIZE%0x200000
.if TEMP > 0
.set DDR_0_REG, DDR_0_REG + 1
.endif
.endif
#else
.set DDR_0_REG, 0
#warning "There's no DDR_0 in the HW design. MMU translation table marks 2 GB DDR address space as undefined"
#endif

.set UNDEF_0_REG, (0x400 - DDR_0_REG - UNDEF_START)

.rept   UNDEF_START                             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x200000
.endr

.rept   DDR_0_REG                       /* DDR based on size in hdf*/
.8byte  SECT + Memory
.set    SECT, SECT+0x200000
.endr

.rept   UNDEF_0_REG             /* reserved for region where ddr is absent */
.8byte  SECT + reserved
.set    SECT, SECT+0x200000
.endr

.rept	0x0100			/* 0x8000_0000 - 0x9FFF_FFFF */
.8byte	SECT + Device		/* 512MB PS_LPD_AFI_FS */
.set	SECT, SECT+0x200000
.endr

.rept	0x080			/* 0xA000_0000 - 0xAFFF_FFFF */
.8byte	SECT + Device		/* 256MB PCIE_REGION */
.set	SECT, SECT+0x200000
.endr


.rept	0x040			   /* 0xB00_0000 - 0xB7FF_FFFF */
.8byte	SECT + Device              /* 128MB PS_FPD_AFI_FS */
.set	SECT, SECT+0x200000
.endr

.rept   0x01F                      /* 0xB800_0000 - 0xBBDF_FFFF */
.8byte  SECT + reserved		   /* 62MB reserved */
.set    SECT, SECT+0x200000
.endr

.rept	0x01			   /* 0xBBE0_0000 - 0xBBFF_FFFF */
.8byte	SECT + Memory_NonCacheable /* 2 MB OCM 0xBBE0_0000 - 0xBBFF_FFFF*/
.set	SECT, SECT+0x200000
.endr

.rept	0x020			/* 0xBC00_0000 - 0xBFFF_FFFF */
.8byte	SECT + Device
.set	SECT, SECT+0x200000
.endr

.rept	0x100			/* 0xE000_0000 - 0xEFFF_FFFF */
.8byte	SECT + Device		/* 512MB xSPI */
.set	SECT, SECT+0x200000
.endr

.rept	0x08			/* 0xE000_0000 - 0xEFFF_FFFF */
.8byte	SECT + Device		/* 16MB FPD_STM_CORESIGHT */
.set	SECT, SECT+0x200000
.endr

.rept   0x08                    /* 0xe1000000 - 0xE1FF_FFFF */
.8byte  SECT + reserved         /* 16MB reserved */
.set    SECT, SECT+0x200000
.endr

.rept   0x02                    /* 0xE200_0000 - 0xE23F_FFFF */
.8byte  SECT + Device           /* 4MB GIC */
.set    SECT, SECT+0x200000
.endr

.rept	0xE			/* 0xE240_0000 - 0xE3FF_FFFF */
.8byte	SECT + reserved		/* 28MB reserved */
.set	SECT, SECT+0x200000
.endr

.rept	0x08			/* 0xE400_0000 - 0xE4FF_FFFF_ */
.8byte  SECT + Device           /* 16MB CPM */
.set	SECT, SECT+0x200000
.endr

.rept	0x8			/* 0xE500_0000 - 0xE5FF_FFFF */
.8byte	SECT + reserved		/* 16MB reserved */
.set	SECT, SECT+0x200000
.endr

.rept	0x4			/* 0xE600_0000 - 0xE67F_FFFF */
.8byte	SECT + Device		/* 8MB HNIC */
.set	SECT, SECT+0x200000
.endr

.rept   0xC                     /* 0xE680_0000 - 0xE7FF_FFFF */
.8byte  SECT + reserved         /* 24MB reserved */
.set    SECT, SECT+0x200000
.endr

.rept   0x4                     /* 0xE800_0000 - 0xE87F_FFFF */
.8byte  SECT + Device           /* 8MB Multimedia tiles */
.set    SECT, SECT+0x200000
.endr

.rept   0xC                     /* 0xE880_0000 - 0xE9FF_FFFF */
.8byte  SECT + reserved         /* 24MB reserved */
.set    SECT, SECT+0x200000
.endr

.rept	0x18			/* 0xEA00_0000 - 0xECFF_FFFF */
.8byte	SECT + Device		/* 32MB LPD_SLAVES 16MB FPD_SLAVES */
.set	SECT, SECT+0x200000
.endr

.rept   0x8                     /* 0xED00_0000 - 0xEDFF_FFFF */
.8byte  SECT + Device           /* 16MB MMI */
.set    SECT, SECT+0x200000
.endr

.rept   0x10                     /* 0xEE00_0000 - 0xEFFF_FFFF */
.8byte  SECT + reserved          /* 32MB reserved */
.set    SECT, SECT+0x200000
.endr

.rept   0x40                    /* 0xF000_0000 - 0xF7FF_FFFF */
.8byte  SECT + Device           /* 128MB PMC */
.set    SECT, SECT+0x200000
.endr

.rept   0x20                    /* 0xF800_0000 - 0xFBFF_FFFF */
.8byte  SECT + Device           /* 64MB PS_FPD_CMN */
.set    SECT, SECT+0x200000
.endr

.rept   0x20                     /* 0xFC00_0000 - 0xFFFF_FFFF */
.8byte  SECT + reserved         /* 64MB reserved */
.set    SECT, SECT+0x200000
.endr

.end

/**
* @} End of "addtogroup Versal_2VE_2VM_apu_boot_code".
*/
