/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

#include "pm_master.h"
#include "pm_mmio_access.h"
#include "crl_apb.h"
#include "crf_apb.h"
#include "pmu_iomodule.h"
#include "afi.h"
#include "pmu_global.h"
#include "csu.h"
#include "csudma.h"
#include "rsa.h"
#include "rsa_core.h"

#define WRITE_PERM_SHIFT	16U
#define MMIO_ACCESS_RO(m)	(m)
#define MMIO_ACCESS_RW(m)	((m) | ((m) << WRITE_PERM_SHIFT))
#define MMIO_ACCESS_WO(m)	((m) << WRITE_PERM_SHIFT)

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
		.startAddr = CRF_APB_BASEADDR + 0x20U,
		.endAddr = CRF_APB_BASEADDR + 0x63U,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	{
		.startAddr = CRF_APB_BASEADDR + 0x70U,
		.endAddr = CRF_APB_BASEADDR + 0x7bU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	{
		.startAddr = CRF_APB_BASEADDR + 0x84U,
		.endAddr = CRF_APB_BASEADDR + 0xbfU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* Module clock controller low power domain (CRL_APB) */
	{
		.startAddr = CRL_APB_BASEADDR + 0x20U,
		.endAddr = CRL_APB_BASEADDR + 0x73U,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},
	{
		.startAddr = CRL_APB_BASEADDR + 0x7CU,
		.endAddr = CRL_APB_BASEADDR + 0x8CU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	{
		.startAddr = CRL_APB_BASEADDR + 0xa4U,
		.endAddr = CRL_APB_BASEADDR + 0xa7U,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	{
		.startAddr = CRL_APB_BASEADDR + 0xb4U,
		.endAddr = CRL_APB_BASEADDR + 0x12bU,
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
		.startAddr = IOU_SLCR_BASE,
		.endAddr = IOU_SLCR_BASE + 0x2FFU,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	{
		.startAddr = IOU_SLCR_BASE + 0x304U,
		.endAddr = IOU_SLCR_BASE + 0x524U,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* RO access to CRL_APB required for Linux CCF */
	{
		.startAddr = CRL_APB_BASEADDR,
		.endAddr = CRL_APB_BASEADDR + 0x288U,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RO access to CRF_APB required for Linux CCF */
	{
		.startAddr = CRF_APB_BASEADDR,
		.endAddr = CRF_APB_BASEADDR + 0x108U,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* Boot pin control register */
	{
		.startAddr = CRL_APB_BASEADDR + 0x250U,
		.endAddr = CRL_APB_BASEADDR + 0x250U,
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
		.startAddr = XPAR_VCU_0_BASEADDR + 0x40024U,
		.endAddr = XPAR_VCU_0_BASEADDR + 0x40060U,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},
#endif

	/* Software controlled FPD resets register */
	{
		.startAddr = CRF_APB_BASEADDR + 0x100U,
		.endAddr = CRF_APB_BASEADDR + 0x100U,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK),
	},

	/* Software controlled LPD resets register */
	{
		.startAddr = CRL_APB_BASEADDR + 0x23cU,
		.endAddr = CRL_APB_BASEADDR +0x23cU,
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

	/* CSU Status register */
	{
		.startAddr = CSU_BASEADDR,
		.endAddr = CSU_BASEADDR,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU multi-boot register*/
	{
		.startAddr = CSU_MULTI_BOOT,
		.endAddr = CSU_MULTI_BOOT,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU tamper-trig register */
	{
		.startAddr = CSU_TAMPER_TRIG,
		.endAddr = CSU_TAMPER_TRIG,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
				IPI_PMU_0_IER_RPU_0_MASK |
				IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU ft-status register*/
	{
		.startAddr = CSU_FT_STATUS,
		.endAddr = CSU_FT_STATUS,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
				IPI_PMU_0_IER_RPU_0_MASK |
				IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU jtag-chain-status register*/
	{
		.startAddr = CSU_JTAG_CHAIN_STATUS,
		.endAddr = CSU_JTAG_CHAIN_STATUS,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
				IPI_PMU_0_IER_RPU_0_MASK |
				IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU Device IDCODE and Version Registers */
	{
		.startAddr = CSU_IDCODE,
		.endAddr = CSU_VERSION,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU rom-digest registers */
	{
		.startAddr = CSU_ROM_DIGEST_0,
		.endAddr = CSU_ROM_DIGEST_11,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU aes-status register */
	{
		.startAddr = CSU_AES_STATUS,
		.endAddr = CSU_AES_STATUS,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU pcap-status register */
	{
		.startAddr = CSU_PCAP_STATUS_REG,
		.endAddr = CSU_PCAP_STATUS_REG,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU_GLOBAL registers*/
	{
                .startAddr = PMU_GLOBAL_GLOBAL_CNTRL,
                .endAddr = PMU_GLOBAL_SAFETY_CHK,
                .access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
                                         IPI_PMU_0_IER_RPU_0_MASK |
                                         IPI_PMU_0_IER_RPU_1_MASK),
        },

	/* PMU Golbal_Req_Iso_Status register */
	{
		.startAddr = PMU_GLOBAL_REQ_ISO_STATUS,
		.endAddr = PMU_GLOBAL_REQ_ISO_STATUS,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Req_SwRst_Status register */
	{
		.startAddr = PMU_GLOBAL_REQ_SWRST_STATUS,
		.endAddr = PMU_GLOBAL_REQ_SWRST_STATUS,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Csu_br_error register */
	{
		.startAddr = PMU_GLOBAL_CSU_BR_ERROR,
		.endAddr = PMU_GLOBAL_CSU_BR_ERROR,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Safety_Chk register */
	{
		.startAddr = PMU_GLOBAL_SAFETY_CHK,
		.endAddr = PMU_GLOBAL_SAFETY_CHK,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* eFUSE IPDISABLE register */
	{
		.startAddr = EFUSE_IPDISABLE,
		.endAddr = EFUSE_IPDISABLE,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

#ifdef SECURE_ACCESS
	/*CSU ctrl, sss-cfg, dma-reset registers */
	{
		.startAddr = CSU_CTRL,
		.endAddr = CSU_DMA_RESET,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU isr register*/
	{
		.startAddr = CSU_ISR,
		.endAddr = CSU_ISR,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU imr register*/
	{
		.startAddr = CSU_IMR,
		.endAddr = CSU_IMR,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU ier register*/
	{
		.startAddr = CSU_IER,
		.endAddr = CSU_JTAG_CHAIN_CFG,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU idr register*/
	{
		.startAddr = CSU_IDR,
		.endAddr = CSU_IDR,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU jtag register*/
	{
		.startAddr = CSU_JTAG_SEC,
		.endAddr = CSU_JTAG_DAP_CFG,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU AES registers*/
	{
		.startAddr = CSU_AES_KEY_SRC,
		.endAddr = CSU_AES_KUP_WR,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU AES KUP registers*/
	{
		.startAddr = CSU_AES_KUP_0,
		.endAddr = CSU_AES_KUP_7,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU IV registers*/
	{
		.startAddr = CSU_AES_IV_0,
		.endAddr = CSU_AES_IV_3,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU  sha_start register*/
	{
		.startAddr = CSU_SHA_START,
		.endAddr = CSU_SHA_START,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU sha_reset register*/
	{
		.startAddr = CSU_SHA_RESET,
		.endAddr = CSU_SHA_RESET,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},
	/* CSU Sha registers */
	{
		.startAddr = CSU_SHA_DONE,
		.endAddr = CSU_SHA_DIGEST_11,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* CSU pcap registers*/
	{
		.startAddr = CSU_PCAP_PROG_REG,
		.endAddr = CSU_PCAP_RESET_REG,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/*CSU tamper-status register */
	{
		.startAddr = CSU_TAMPER_STATUS,
		.endAddr = CSU_TAMPER_STATUS,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/*CSU csu-tamper_0-12 registers */
	{
		.startAddr = CSU_TAMPER_0,
		.endAddr = CSU_TAMPER_12,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/*CSUDMA registers*/
	{
		.startAddr = CSUDMA_BASEADDR,
		.endAddr = CSUDMA_CSUDMA_FUTURE_ECO,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA wr_data registers */
	{
		.startAddr = RSA_BASEADDR,
		.endAddr = RSA_WR_ADDR,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA rd_data registers */
	{
		.startAddr = RSA_RD_DATA_0,
		.endAddr = RSA_RD_DATA_5,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA rd_address registers */
	{
		.startAddr = RSA_RD_ADDR,
		.endAddr = RSA_RD_ADDR,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA cfg and isr registers */
	{
		.startAddr = RSA_RSA_CFG,
		.endAddr = RSA_RSA_ISR,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA imr registers */
	{
		.startAddr = RSA_RSA_IMR,
		.endAddr = RSA_RSA_IMR,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA ier and idr registers */
	{
		.startAddr = RSA_RSA_IER,
		.endAddr = RSA_RSA_IDR,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA_CORE wr_data and wr_address registers */
	{
		.startAddr = RSA_CORE_BASEADDR,
		.endAddr = RSA_CORE_RSA_WR_ADDR,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA_CORE rd_data registers */
	{
		.startAddr = RSA_CORE_RSA_RD_DATA,
		.endAddr = RSA_CORE_RSA_RD_DATA,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA_CORE rd_addr and ctrl registers */
	{
		.startAddr = RSA_CORE_RSA_RD_ADDR,
		.endAddr = RSA_CORE_CTRL,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA_CORE status registers */
	{
		.startAddr = RSA_CORE_STATUS,
		.endAddr = RSA_CORE_STATUS,
		.access = MMIO_ACCESS_RO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* RSA_CORE minv registers */
	{
		.startAddr = RSA_CORE_MINV0,
		.endAddr = RSA_CORE_MINV3,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU global ctrl, ps-ctrl, apu_power_status_init,
	 * mem_ctrl registers and addr_error_status registers
	 */
	{
		.startAddr = PMU_GLOBAL_GLOBAL_CNTRL,
		.endAddr = PMU_GLOBAL_ADDR_ERROR_STATUS,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU addr_err_int_en and addr_err_int_dis registers */
	{
		.startAddr = PMU_GLOBAL_ADDR_ERROR_INT_EN,
		.endAddr = PMU_GLOBAL_ADDR_ERROR_INT_DIS,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU DDR_CTRL register */
	{
		.startAddr = PMU_GLOBAL_DDR_CNTRL,
		.endAddr = PMU_GLOBAL_DDR_CNTRL,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU RAM_RET_CTRL registers */
	{
		.startAddr = PMU_GLOBAL_RAM_RET_CNTRL,
		.endAddr = PMU_GLOBAL_RAM_RET_CNTRL,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Req_Pwrup_Status registers */
	{
		.startAddr = PMU_GLOBAL_REQ_PWRUP_STATUS,
		.endAddr = PMU_GLOBAL_REQ_PWRUP_STATUS,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Req_Pwrup_Int_En ,Req_Pwrup_Int_Dis and
	 * Req_Pwrup_Int_Trig registers*/
	{
		.startAddr = PMU_GLOBAL_REQ_PWRUP_INT_EN,
		.endAddr = PMU_GLOBAL_REQ_PWRUP_TRIG,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Req_Pwrdw_status registers*/
	{
		.startAddr = PMU_GLOBAL_REQ_PWRDWN_STATUS,
		.endAddr = PMU_GLOBAL_REQ_PWRDWN_STATUS,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Req_Pwrdwn_Int_En , Req_PwrDwn_Int_Dis and
	 * Req_PwrDwn_Int_Trig registers
	 */
	{
		.startAddr = PMU_GLOBAL_REQ_PWRDWN_INT_EN,
		.endAddr = PMU_GLOBAL_REQ_PWRDWN_TRIG,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Req_Iso_Int_En, Req_Iso_Int_Dis and
	 * Req_Iso_Int_Trig registers */
	{
		.startAddr = PMU_GLOBAL_REQ_ISO_INT_EN,
		.endAddr = PMU_GLOBAL_REQ_ISO_TRIG,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Req_SwRst_Int_En, Req_SwRst_Int_Dis and
	 * Req_SwRst_Int_Trig registers */
	{
		.startAddr = PMU_GLOBAL_REQ_SWRST_INT_EN,
		.endAddr = PMU_GLOBAL_REQ_SWRST_TRIG,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Status_1 register */
	{
		.startAddr = PMU_GLOBAL_ERROR_STATUS_1,
		.endAddr = PMU_GLOBAL_ERROR_STATUS_1 ,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Int_En_1 , Error_Int_Dis_1 and
	 * Error_Status_2 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_INT_EN_1,
		.endAddr = PMU_GLOBAL_ERROR_STATUS_2,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Int_En_2 and Error_Int_Dis_2 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_INT_EN_2,
		.endAddr = PMU_GLOBAL_ERROR_INT_DIS_2,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Por_En_1 and Error_Por_Dis_1 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_POR_EN_1,
		.endAddr = PMU_GLOBAL_ERROR_POR_DIS_1 ,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Por_En_2 and Error_Por_Dis_2 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_POR_EN_2,
		.endAddr = PMU_GLOBAL_ERROR_POR_DIS_2 ,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Srst_En_1 and Error_Srst_Dis_1 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_SRST_EN_1,
		.endAddr = PMU_GLOBAL_ERROR_SRST_DIS_1,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Srst_En_2 and Error_Srst_Dis_2 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_SRST_EN_2,
		.endAddr = PMU_GLOBAL_ERROR_SRST_DIS_2,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Sig_En_1 and Error_Sig_Dis_1 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_SIG_EN_1,
		.endAddr = PMU_GLOBAL_ERROR_SIG_DIS_1,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_Sig_En_2 and Error_Sig_Dis_2 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_SIG_EN_2,
		.endAddr = PMU_GLOBAL_ERROR_SIG_DIS_2,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Error_En_1 and Error_En_2 registers */
	{
		.startAddr = PMU_GLOBAL_ERROR_EN_1,
		.endAddr = PMU_GLOBAL_ERROR_EN_2,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Aib_Ctrl register */
	{
		.startAddr = PMU_GLOBAL_AIB_CNTRL,
		.endAddr = PMU_GLOBAL_AIB_CNTRL,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Global_Reset register */
	{
		.startAddr = PMU_GLOBAL_GLOBAL_RESET,
		.endAddr = PMU_GLOBAL_GLOBAL_RESET,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Safety_Gate register */
	{
		.startAddr = PMU_GLOBAL_SAFETY_GATE,
		.endAddr = PMU_GLOBAL_SAFETY_GATE,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},

	/* PMU Mbist_Reset, Mbist_Pg_En and Mbist_Setup registers */
	{
		.startAddr = PMU_GLOBAL_MBIST_RST,
		.endAddr = PMU_GLOBAL_MBIST_SETUP,
		.access = MMIO_ACCESS_WO(IPI_PMU_0_IER_APU_MASK |
					 IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},
#endif
#ifdef ENABLE_EM
	/* PMU Local FW error register */
	{
		.startAddr = PMU_LOCAL_PMU_SERV_ERR,
		.endAddr = PMU_LOCAL_PMU_SERV_ERR,
		.access = MMIO_ACCESS_RW(IPI_PMU_0_IER_RPU_0_MASK |
					 IPI_PMU_0_IER_RPU_1_MASK),
	},
#endif

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

			if (0U != (pmAccessTable[i].access & mask)) {
				permission = true;
				break;
			}
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
