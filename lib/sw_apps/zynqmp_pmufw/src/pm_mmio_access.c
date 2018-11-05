/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */
#include "xpfw_config.h"
#ifdef ENABLE_PM

#include "pm_master.h"
#include "pm_mmio_access.h"
#include "crl_apb.h"
#include "crf_apb.h"
#include "pmu_iomodule.h"
#include "afi.h"

#define PM_MMIO_IOU_SLCR_BASE  0xFF180000
#define PM_MMIO_CSU_BASE       0xFFCA0000

#define WRITE_PERM_SHIFT	16
#define MMIO_ACCESS_RO(m)	(m)
#define MMIO_ACCESS_RW(m)	((m) | ((m) << WRITE_PERM_SHIFT))

enum mmio_access_type {
	MMIO_ACCESS_TYPE_READ,
	MMIO_ACCESS_TYPE_WRITE,
};

/**
 * PmAccessRegion - Structure containing information about memory access
                    permissions
 * @startAddr   Starting address of the memory region
 * @endAddr     Ending address of the memory region
 * @access      Access control bitmask (1 bit per master, see 'pmAllMasters')
 */
typedef struct PmAccessRegion {
	const u32 startAddr;
	const u32 endAddr;
	const u32 access;
} PmAccessRegion;

static const PmAccessRegion pmAccessTable[] = {
	/* Module clock controller full power domain (CRF_APB) */
	{
		.startAddr = CRF_APB_BASEADDR + 0x20,
		.endAddr = CRF_APB_BASEADDR + 0x63,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	{
		.startAddr = CRF_APB_BASEADDR + 0x70,
		.endAddr = CRF_APB_BASEADDR + 0x7b,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	{
		.startAddr = CRF_APB_BASEADDR + 0x84,
		.endAddr = CRF_APB_BASEADDR + 0xbf,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* Module clock controller low power domain (CRL_APB) */
	{
		.startAddr = CRL_APB_BASEADDR + 0x20,
		.endAddr = CRL_APB_BASEADDR + 0x73,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

#if !((STDOUT_BASEADDRESS == XPAR_PSU_UART_0_BASEADDR) && defined(DEBUG_MODE))
	{
		.startAddr = CRL_APB_BASEADDR + 0x74,
		.endAddr = CRL_APB_BASEADDR + 0x77,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},
#endif

#if !((STDOUT_BASEADDRESS == XPAR_PSU_UART_1_BASEADDR) && defined(DEBUG_MODE))
	{
		.startAddr = CRL_APB_BASEADDR + 0x78,
		.endAddr = CRL_APB_BASEADDR + 0x7B,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},
#endif
	{
		.startAddr = CRL_APB_BASEADDR + 0x7C,
		.endAddr = CRL_APB_BASEADDR + 0x8C,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	{
		.startAddr = CRL_APB_BASEADDR + 0xa4,
		.endAddr = CRL_APB_BASEADDR + 0xa7,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	{
		.startAddr = CRL_APB_BASEADDR + 0xb4,
		.endAddr = CRL_APB_BASEADDR + 0x12b,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU's global Power Status register*/
	{
		.startAddr = PMU_GLOBAL_PWR_STATE,
		.endAddr = PMU_GLOBAL_PWR_STATE,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* PMU's global gen storage */
	{
		.startAddr = PMU_GLOBAL_GLOBAL_GEN_STORAGE0,
		.endAddr = PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* IOU SLCR Registers required for Linux */
	{
		.startAddr = PM_MMIO_IOU_SLCR_BASE,
		.endAddr = PM_MMIO_IOU_SLCR_BASE + 0x524,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* CSU Device IDCODE and Version Registers */
	{
		.startAddr = PM_MMIO_CSU_BASE + 0x40,
		.endAddr = PM_MMIO_CSU_BASE + 0x44,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RO access to CRL_APB required for Linux CCF */
	{
		.startAddr = CRL_APB_BASEADDR,
		.endAddr = CRL_APB_BASEADDR + 0x288,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RO access to CRF_APB required for Linux CCF */
	{
		.startAddr = CRF_APB_BASEADDR,
		.endAddr = CRF_APB_BASEADDR + 0x108,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* Boot pin control register */
	{
		.startAddr = CRL_APB_BASEADDR + 0x250,
		.endAddr = CRL_APB_BASEADDR + 0x250,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* FPD Lock status register */
	{
		.startAddr = PMU_LOCAL_DOMAIN_ISO_CNTRL,
		.endAddr = PMU_LOCAL_DOMAIN_ISO_CNTRL,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

#ifdef XPAR_VCU_0_BASEADDR
	/* VCU SLCR register */
	{
		.startAddr = XPAR_VCU_0_BASEADDR + 0x40024,
		.endAddr = XPAR_VCU_0_BASEADDR + 0x40060,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},
#endif

	/* Software controlled FPD resets register */
	{
		.startAddr = CRF_APB_BASEADDR + 0x100,
		.endAddr = CRF_APB_BASEADDR + 0x100,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* Software controlled LPD resets register */
	{
		.startAddr = CRL_APB_BASEADDR + 0x23c,
		.endAddr = CRL_APB_BASEADDR +0x23c,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* FPD_SLCR AFI_FS Register */
	{
		.startAddr = FPD_SLCR_AFI_FS_REG,
		.endAddr = FPD_SLCR_AFI_FS_REG,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* LPD SLCR AFI_FS Register */
	{
		.startAddr = LPD_SLCR_AFI_FS,
		.endAddr = LPD_SLCR_AFI_FS,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* AFI FM 0 Registers */
	{
		.startAddr = AFI_FM0_BASEADDR,
		.endAddr = AFI_FM0_BASEADDR + 0xF0CU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* AFI FM 1 Registers */
	{
		.startAddr = AFI_FM1_BASEADDR,
		.endAddr = AFI_FM1_BASEADDR + 0xF0CU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* AFI FM 2 Registers */
	{
		.startAddr = AFI_FM2_BASEADDR,
		.endAddr = AFI_FM2_BASEADDR + 0xF0CU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* AFI FM 3 Registers */
	{
		.startAddr = AFI_FM3_BASEADDR,
		.endAddr = AFI_FM3_BASEADDR + 0xF0CU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* AFI FM 4 Registers */
	{
		.startAddr = AFI_FM4_BASEADDR,
		.endAddr = AFI_FM4_BASEADDR + 0xF0CU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* AFI FM 5 Registers */
	{
		.startAddr = AFI_FM5_BASEADDR,
		.endAddr = AFI_FM5_BASEADDR + 0xF0CU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* AFI FM 6 Registers */
	{
		.startAddr = AFI_FM6_BASEADDR,
		.endAddr = AFI_FM6_BASEADDR + 0xF0CU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},
};

/**
 * PmGetMmioAccess() - Retrieve access info for a particular address
 * @master     Master who requests access permission
 * @address    Address to write/read
 * @type       Type of access (read or write)
 *
 * @return     Return true if master's IPI bit was present in the access region
 *             table
 */
static bool PmGetMmioAccess(const PmMaster *const master, const u32 address,
			    enum mmio_access_type type)
{
	u32 i;
	bool permission = false;

	if (NULL == master) {
		goto done;
	}

	for (i = 0U; i < ARRAY_SIZE(pmAccessTable); i++) {
		if ((address >= pmAccessTable[i].startAddr) &&
		    (address <= pmAccessTable[i].endAddr)) {

			u32 mask = master->ipiMask;

			if (MMIO_ACCESS_TYPE_WRITE == type) {
				mask <<= WRITE_PERM_SHIFT;
			}

			permission = !!(pmAccessTable[i].access & mask);

			if (permission)
				break;
		}
	}

done:
	return permission;
}

bool PmGetMmioAccessRead(const PmMaster *const master, const u32 address)
{
	return PmGetMmioAccess(master, address, MMIO_ACCESS_TYPE_READ);
}

bool PmGetMmioAccessWrite(const PmMaster *const master, const u32 address)
{
	return PmGetMmioAccess(master, address, MMIO_ACCESS_TYPE_WRITE);
}

#endif
