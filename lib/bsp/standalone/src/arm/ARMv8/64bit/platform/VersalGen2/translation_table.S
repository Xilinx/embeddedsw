/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file translation_table.s
*
* @addtogroup VersalGen2_apu_boot_code
* @{
* <h2> translation_table.S </h2>
* The translation_table.S contains a static page table required by MMU for
* cortex-A78. This translation table is flat mapped (input address = output
* address) with default memory attributes defined for Versal Gen2
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
*
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

	/* Base addresses for all DDR regions in Versal Gen2 SoC TODO: revisit */
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
 * 48TB - 256TB is reserved in Versal Gen2 address map,
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

.rept	0x2			/* 0x1_0000_0000 - 0x7_FFFF_FFFF */
.8byte	SECT + reserved		/* 28GB Reserved */
.set SECT, SECT + 0x40000000
.endr

/* TODO: Mark DDR_CH0_MED as normal memory based on HW design */
.rept	0x20			/* 0x8_0000_0000 - 0xF_FFFF_FFFF */
.8byte	SECT + reserved		/* 32GB DDR_CH0_MED marked as reserved for now */
.set SECT, SECT + 0x40000000
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

/* TODO: Mark this region as normal memory based on HW design */
.rept	0x100			/* 0xC0_0000_0000 - 0xFF_FFFF_FFFF */
.8byte	SECT + reserved		/* 256GB DDR (DDR_CH0_HIGH_0: 256GB + DDR_CH0_HIGH_1: 734GB) */
.set SECT, SECT + 0x40000000
.endr

.rept   0x2DE                   /* 0x100_0000_0000 - 0x1B7_7FFF_FFFF */
.8byte  SECT + reserved         /* 790GB DDR (DDR_CH0_HIGH_1) */
.set SECT, SECT + 0x40000000
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

/* TODO: Mark this region as normal memory based on the HW design */
.rept	0xc00			/* 0x500_0000_0000 - 0x7FF_FFFF_FFFF */
.8byte	SECT + reserved		/* 3TB DDR: DDR_CH1, DDr_CH2 and DDR_CH3 each with 1TB */
.set SECT, SECT + 0x40000000
.endr

.rept	0x2000			/* 0x800_0000_0000 - 0xFFF_FFFF_FFFF */
.8byte	SECT + Device		/* 8TB PL_VIA_NOC_HIGH*/
.set SECT, SECT + 0x40000000
.endr

.rept	0x2000			/* 0x800_0000_0000 - 0xFFF_FFFF_FFFF */
.8byte	SECT + Device		/* 8TB CPM_CDX_DPU_HIGH */
.set SECT, SECT + 0x40000000
.endr

/* TODO: Mark this region as normal memory based on the HW design */
.rept	0x1800			/* 0x400_0000_0000 - 0x4FF_FFFF_FFFF */
.8byte	SECT + reserved		/* 6TB DDR: 12 DDR regions (DDR_CH4 to DDR_CH15) each of size 512GB */
.set SECT, SECT + 0x40000000
.endr

.rept	0x800			/* 0x800_0000_0000 - 0xFFF_FFFF_FFFF */
.8byte	SECT + reserved		/* 2TB reserved */
.set SECT, SECT + 0x40000000
.endr

.rept	0x4000			/* 0x800_0000_0000 - 0xFFF_FFFF_FFFF */
.8byte	SECT + Device		/* 16TB C2C_HIGH */
.set SECT, SECT + 0x40000000
.endr
#endif

.section .mmu_tbl2,"a"

MMUTableL2:

.set SECT, 0

/* TODO: Configure this region as normal memory/reserved based on HW design */
.rept	0x400			/* 2GB DDR 0x0 - 0x80000000 */
.8byte	SECT + Memory
.set	SECT, SECT+0x200000
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
.8byte	SECT + Device		/* 64MB ATTICUS_C2C_LOW */
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
* @} End of "addtogroup VersalGen2_apu_boot_code".
*/
