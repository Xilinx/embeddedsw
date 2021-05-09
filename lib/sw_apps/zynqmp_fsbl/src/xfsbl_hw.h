/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_hw.h
*
* This is the header file which contains definitions for the hardware
* registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
* 3.0   vns  09/08/17 Added eFUSE secure control register masks for PPK revoke
* 4.0   vns  02/02/18 Added warning message to notify SHA2 support
*                     deprecation in future releases.
*       vns  03/07/18 Added ENC_ONLY mask
* 4.0   vns  03/14/19 Added AES reset offset and Mask values.
* 5.0   bsv  04/01/21 Added TPM support
*       bsv  05/03/21 Add provision to load bitstream from OCM with DDR
*                     present in design
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_HW_H
#define XFSBL_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xparameters.h"
#include "xil_types.h"
#include "sleep.h"

#include "xfsbl_config.h"
#include "xfsbl_debug.h"
#include "xfsbl_error.h"
#include "xfsbl_hooks.h"
#include "xfsbl_misc.h"

/************************** Constant Definitions *****************************/

/* csu */

/**
 * CSU Base Address
 */
#define CSU_BASEADDR      0XFFCA0000U

/**
 * Register: CSU_CSU_SSS_CFG
 */
#define CSU_CSU_SSS_CFG    ( ( CSU_BASEADDR ) + 0X00000008U )
#define CSU_CSU_SSS_CFG_PCAP_SSS_MASK    0X0000000FU
#define CSU_CSU_SSS_CFG_PCAP_SSS_SHIFT   0U
#define CSU_CSU_SSS_CFG_DMA_SSS_SHIFT	 1U
#define CSU_CSU_SSS_CFG_SHA_SSS_DMA_VAL		(0x5000U)

/**
 * Register: CSU DMA RESET
 */
#define CSU_DMA_RESET   ( ( CSU_BASEADDR ) + 0X0000000CU )
#define CSU_DMA_RESET_RESET_MASK        0X00000001U

/**
 * Register: CSU AES RESET
 */
#define CSU_AES_RESET	( ( CSU_BASEADDR ) + 0X00001010U )
#define CSU_AES_RESET_RESET_MASK	0X00000001U

/**
 * Register: CSU_PCAP_STATUS
 */
#define CSU_PCAP_STATUS    ( ( CSU_BASEADDR ) + 0X00003010U )
#define CSU_PCAP_STATUS_PL_INIT_SHIFT   2U
#define CSU_PCAP_STATUS_PL_INIT_MASK    0X00000004U
#define CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK    0X00000001U
#define CSU_PCAP_STATUS_PL_DONE_MASK    0X00000008U
#define CSU_PCAP_STATUS_PL_CFG_RST_MASK	0x00000040U

/**
 * Register: CSU_PCAP_RDWR
 */
#define CSU_PCAP_RDWR    ( ( CSU_BASEADDR ) + 0X00003004U )
#define CSU_PCAP_RDWR_PCAP_RDWR_B_SHIFT   0U

/* Register: CSU_PCAP_PROG */
#define CSU_PCAP_PROG    ( ( CSU_BASEADDR ) + 0X00003000U )
#define CSU_PCAP_PROG_PCFG_PROG_B_MASK    0X00000001U
#define CSU_PCAP_PROG_PCFG_PROG_B_SHIFT   0U

/**
 * Register: CSU_VERSION
 */
#define CSU_VERSION    ( ( CSU_BASEADDR ) + 0X00000044U )

#define CSU_VERSION_PLATFORM_MASK    0X0000F000U

/**
 * Register: CSU_CSU_MULTI_BOOT
 */
#define CSU_CSU_MULTI_BOOT    ( ( CSU_BASEADDR ) + 0X00000010U )

/**
 * Register: CSU_ROM_DIGEST_0
 */
#define CSU_ROM_DIGEST_ADDR_0	( ( CSU_BASEADDR ) + 0X00000050U )
/**
 * Register: CSU_ROM_DIGEST_11
 */
#define CSU_ROM_DIGEST_ADDR_11	( ( CSU_BASEADDR ) + 0X0000007CU )

/**
 * Register: CSU_SHA_RESET
 */
#define CSU_SHA_RESET    ( ( CSU_BASEADDR ) + 0X00002004U )
#define CSU_SHA_RESET_RESET_MASK    0X00000001U

/**
 * Register: CSU_SHA_START
 */
#define CSU_SHA_START    ( ( CSU_BASEADDR ) + 0X00002000U )
#define CSU_SHA_START_START_MSG_MASK    0X00000001U

/**
 * Register: CSU_SHA_DONE
 */
#define CSU_SHA_DONE    ( ( CSU_BASEADDR ) + 0X00002008U )
#define CSU_SHA_DONE_SHA_DONE_MASK    0X00000001U

/**
 * Register: CSU_SHA_DIGEST_0
 */
#define CSU_SHA_DIGEST_0    ( ( CSU_BASEADDR ) + 0X00002010U )

/* Register: CSU_PCAP_RESET */
#define CSU_PCAP_RESET    ( ( CSU_BASEADDR ) + 0X0000300CU )
#define CSU_PCAP_RESET_RESET_MASK    0X00000001U

/* Register: CSU_PCAP_CTRL */
#define CSU_PCAP_CTRL    ( ( CSU_BASEADDR ) + 0X00003008U )
#define CSU_PCAP_CTRL_PCAP_PR_MASK    0X00000001U

/* Register: CSU_IDCODE */
#define CSU_IDCODE    ( ( CSU_BASEADDR ) + 0X00000040U )

#define CSU_IDCODE_SVD_SHIFT		12U
#define CSU_IDCODE_SVD_MASK		(0x7U << CSU_IDCODE_SVD_SHIFT)
#define CSU_IDCODE_DEVICE_CODE_SHIFT	15U
#define CSU_IDCODE_DEVICE_CODE_MASK	(0xFU << CSU_IDCODE_DEVICE_CODE_SHIFT)

/* efuse */

/**
 * EFUSE Base Address
 */
#define EFUSE_BASEADDR      0XFFCC0000U

/**
 * Register: EFUSE_SEC_CTRL
 */
#define EFUSE_SEC_CTRL    ( ( EFUSE_BASEADDR ) + 0X00001058U )
#define EFUSE_SEC_CTRL_ENC_ONLY_MASK  0x00000004U
#define EFUSE_SEC_CTRL_RSA_EN_MASK    0x03FFF800U
#define EFUSE_SEC_CTRL_PPK0_RVK_MASK  0x18000000U
#define EFUSE_SEC_CTRL_PPK1_RVK_MASK  0xC0000000U

/* Register: EFUSE_IPDISABLE */
#define EFUSE_IPDISABLE    ( ( EFUSE_BASEADDR ) + 0X00001018U )

#define EFUSE_IPDISABLE_GPU_DIS_MASK    0X00000020U
#define EFUSE_IPDISABLE_APU3_DIS_MASK    0X00000008U
#define EFUSE_IPDISABLE_APU2_DIS_MASK    0X00000004U
#define EFUSE_IPDISABLE_VCU_DIS_MASK	 0x00000100U

/* Register PPK0_0 */
#define EFUSE_PPK0	(EFUSE_BASEADDR + 0x000010A0U)

/* Register PPK1_0 */
#define EFUSE_PPK1	(EFUSE_BASEADDR + 0x000010D0U)

/* Register SPK ID */
#define EFUSE_SPKID	(EFUSE_BASEADDR + 0x0000105CU)

/* csudma */

/**
 * CSUDMA Base Address
 */
#define CSUDMA_BASEADDR      0XFFC80000U

/* crf_apb */

/**
 * CRF_APB Base Address
 */
#define CRF_APB_BASEADDR      0XFD1A0000U

/**
 * Register: CRF_APB_RST_FPD_APU
 */
#define CRF_APB_RST_FPD_APU    ( ( CRF_APB_BASEADDR ) + 0X00000104U )
#define CRF_APB_RST_FPD_APU_ACPU0_RESET_MASK    (u32)0X00000001U
#define CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK    (u32)0X00000100U

/**
 * Register: CRF_APB_ACPU_CTRL
 */
#define CRF_APB_ACPU_CTRL    ( ( CRF_APB_BASEADDR ) + 0X00000060U )
#define CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK    0X01000000U
#define CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK    0X02000000U
#define CRF_APB_RST_FPD_APU_ACPU1_RESET_MASK    (u32)0X00000002U
#define CRF_APB_RST_FPD_APU_ACPU2_RESET_MASK    (u32)0X00000004U
#define CRF_APB_RST_FPD_APU_ACPU3_RESET_MASK   (u32) 0X00000008U

#define CRF_APB_RST_FPD_APU_ACPU3_PWRON_RESET_MASK    (u32)0X00002000U
#define CRF_APB_RST_FPD_APU_ACPU2_PWRON_RESET_MASK    (u32)0X00001000U
#define CRF_APB_RST_FPD_APU_ACPU1_PWRON_RESET_MASK    (u32)0X00000800U
#define CRF_APB_RST_FPD_APU_ACPU0_PWRON_RESET_MASK    (u32)0X00000400U

/* crl_apb */

/**
 * CRL_APB Base Address
 */
#define CRL_APB_BASEADDR      0XFF5E0000U

/**
 * Register: CRL_APB_CPU_R5_CTRL
 */
#define CRL_APB_CPU_R5_CTRL    ( ( CRL_APB_BASEADDR ) + 0X00000090U )
#define CRL_APB_CPU_R5_CTRL_CLKACT_MASK    0X01000000U

/**
 * Register: CRL_APB_RST_LPD_TOP
 */
#define CRL_APB_RST_LPD_TOP    ( ( CRL_APB_BASEADDR ) + 0X0000023CU )
#define CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK    (u32)0X00000001U
#define CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK    (u32)0X00000004U
#define CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK    (u32)0X00000002U

/**
 * Register: CRL_APB_BOOT_MODE_USER
 */
#define CRL_APB_BOOT_MODE_USER    ( ( CRL_APB_BASEADDR ) + 0X00000200U )
#define CRL_APB_BOOT_MODE_USER_BOOT_MODE_MASK    0X0000000FU

/**
 * Register: CRL_APB_RESET_CTRL
 */
#define CRL_APB_RESET_CTRL    ( ( CRL_APB_BASEADDR ) + 0X00000218U )
#define CRL_APB_RESET_CTRL_SOFT_RESET_MASK    0X00000010U

/* Register: CRL_APB_PCAP_CTRL */
#define CRL_APB_PCAP_CTRL    ( ( CRL_APB_BASEADDR ) + 0X000000A4U )
#define CRL_APB_PCAP_CTRL_DIVISOR0_SHIFT   8U
#define CRL_APB_PCAP_CTRL_DIVISOR0_MASK    0X00003F00U
#define CRL_APB_PCAP_CTRL_CLKACT_MASK    0X01000000U

/* Register: CRL_APB_RESET_REASON */
#define CRL_APB_RESET_REASON    ( ( CRL_APB_BASEADDR ) + 0X00000220U )
#define CRL_APB_RESET_REASON_PMU_SYS_RESET_MASK    0X00000004U
#define CRL_APB_RESET_REASON_PSONLY_RESET_REQ_MASK 0x00000008U

/**
 * Register: CRL_APB_RPLL_CTRL
 */
#define CRL_APB_RPLL_CTRL    ( ( CRL_APB_BASEADDR ) + 0X00000030U )
#define CRL_APB_RPLL_CTRL_BYPASS_MASK    0X00000008U


/* apu */

/**
 * APU Base Address
 */
#define APU_BASEADDR      0XFD5C0000U

/**
 * Register: APU_CONFIG_0
 */
#define APU_CONFIG_0    ( ( APU_BASEADDR ) + 0X00000020U )
#define APU_CONFIG_0_VINITHI_MASK  0x000000F0

/**
 * Register: APU_RVBARADDR0L
 */
#define APU_RVBARADDR0L    ( ( APU_BASEADDR ) + 0X00000040U )

/**
 * Register: APU_RVBARADDR0H
 */
#define APU_RVBARADDR0H    ( ( APU_BASEADDR ) + 0X00000044U )

/**
 * Register: APU_RVBARADDR1L
 */
#define APU_RVBARADDR1L    ( ( APU_BASEADDR ) + 0X00000048U )

/**
 * Register: APU_RVBARADDR1H
 */
#define APU_RVBARADDR1H    ( ( APU_BASEADDR ) + 0X0000004CU )

/**
 * Register: APU_RVBARADDR2L
 */
#define APU_RVBARADDR2L    ( ( APU_BASEADDR ) + 0X00000050U )

/**
 * Register: APU_RVBARADDR2H
 */
#define APU_RVBARADDR2H    ( ( APU_BASEADDR ) + 0X00000054U )

/**
 * Register: APU_RVBARADDR3L
 */
#define APU_RVBARADDR3L    ( ( APU_BASEADDR ) + 0X00000058U )

/**
 * Register: APU_RVBARADDR3H
 */
#define APU_RVBARADDR3H    ( ( APU_BASEADDR ) + 0X0000005CU )

/* pmu_global */
#define PMU_GLOBAL_GLOB_GEN_STORAGE6    ( ( PMU_GLOBAL_BASEADDR ) + 0X48U )
#define PMU_GLOBAL_GLOB_GEN_STORAGE5	( ( PMU_GLOBAL_BASEADDR ) + 0x44U )
#define PMU_GLOBAL_GLOB_GEN_STORAGE4 	( ( PMU_GLOBAL_BASEADDR ) + 0X40U )
#define PMU_GLOBAL_GLOB_GEN_STORAGE1    ( ( PMU_GLOBAL_BASEADDR ) + 0X34U )
#define PMU_GLOBAL_GLOB_GEN_STORAGE2    ( ( PMU_GLOBAL_BASEADDR ) + 0X38U )

/**
 * Register: PMU_GLOBAL_PERS_GLOB_GEN_STORAGE4
 */
#define PMU_GLOBAL_PERS_GLOB_GEN_STORAGE4    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000060U )

/**
 * Register: PMU_GLOBAL_PERS_GLOB_GEN_STORAGE5
 */
#define PMU_GLOBAL_PERS_GLOB_GEN_STORAGE5    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000064U )

/*
 * Register: PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7
 */
#define PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7    ( ( PMU_GLOBAL_BASEADDR ) + 0X0000006CU )

/**
 * PMU_GLOBAL Base Address
 */
#define PMU_GLOBAL_BASEADDR      0XFFD80000U

#define PMU_GLOBAL_GLOBAL_CNTRL    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000000U )
#define PMU_GLOBAL_GLOBAL_CNTRL_MB_SLEEP_MASK    0X00010000U
#define PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK    0X00000010U

#define PMU_GLOBAL_PS_CNTRL            ( (PMU_GLOBAL_BASEADDR ) + 0x4U)
#define PMU_GLOBAL_PS_CNTRL_PROG_ENABLE_MASK    0x2U
#define PMU_GLOBAL_PS_CNTRL_PROG_GATE_MASK      0x1U

#define PMU_GLOBAL_AIB_CNTRL           ( ( PMU_GLOBAL_BASEADDR ) + 0X600U )
#define PMU_GLOBAL_AIB_STATUS          ( ( PMU_GLOBAL_BASEADDR ) + 0X604U )

/* Register: PMU_GLOBAL_REQ_PWRUP_INT_EN */
#define PMU_GLOBAL_REQ_PWRUP_INT_EN    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000118U )
#define PMU_GLOBAL_REQ_PWRUP_INT_EN_PL_MASK    0X00800000U

/* Register: PMU_GLOBAL_REQ_PWRUP_TRIG */
#define PMU_GLOBAL_REQ_PWRUP_TRIG    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000120U )
#define PMU_GLOBAL_REQ_PWRUP_TRIG_PL_MASK    0X00800000U

/* Register: PMU_GLOBAL_REQ_PWRUP_STATUS */
#define PMU_GLOBAL_REQ_PWRUP_STATUS    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000110U )
#define PMU_GLOBAL_REQ_PWRUP_STATUS_PL_SHIFT   23U
#define PMU_GLOBAL_REQ_PWRUP_STATUS_PL_MASK    0X00800000U

/* Register: PMU_GLOBAL_PWR_STATE */
#define PMU_GLOBAL_PWR_STATE    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000100U )
#define PMU_GLOBAL_PWR_STATE_PL_MASK    0X00800000U
#define PMU_GLOBAL_PWR_STATE_FP_MASK    		0X00400000U
#define PMU_GLOBAL_PWR_STATE_USB1_MASK    		0X00200000U
#define PMU_GLOBAL_PWR_STATE_USB0_MASK    		0X00100000U
#define PMU_GLOBAL_PWR_STATE_OCM_BANK3_MASK    	0X00080000U
#define PMU_GLOBAL_PWR_STATE_OCM_BANK2_MASK    	0X00040000U
#define PMU_GLOBAL_PWR_STATE_OCM_BANK1_MASK    	0X00020000U
#define PMU_GLOBAL_PWR_STATE_OCM_BANK0_MASK    	0X00010000U
#define PMU_GLOBAL_PWR_STATE_TCM1B_MASK    		(u32)0X00008000U
#define PMU_GLOBAL_PWR_STATE_TCM1A_MASK    		(u32)0X00004000U
#define PMU_GLOBAL_PWR_STATE_TCM0B_MASK    		(u32)0X00002000U
#define PMU_GLOBAL_PWR_STATE_TCM0A_MASK    		(u32)0X00001000U
#define PMU_GLOBAL_PWR_STATE_R5_1_MASK    		(u32)0X00000800U
#define PMU_GLOBAL_PWR_STATE_R5_0_MASK    		(u32)0X00000400U
#define PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK    	0X00000080U
#define PMU_GLOBAL_PWR_STATE_PP1_MASK    		0X00000020U
#define PMU_GLOBAL_PWR_STATE_PP0_MASK    		0X00000010U
#define PMU_GLOBAL_PWR_STATE_ACPU3_MASK    		0X00000008U
#define PMU_GLOBAL_PWR_STATE_ACPU2_MASK    		0X00000004U
#define PMU_GLOBAL_PWR_STATE_ACPU1_MASK    		0X00000002U
#define PMU_GLOBAL_PWR_STATE_ACPU0_MASK    		0X00000001U

/* Register: PMU_GLOBAL_ERROR_STATUS_1 */
#define PMU_GLOBAL_ERROR_STATUS_1    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000530U )
#define PMU_GLOBAL_ERROR_STATUS_1_LPD_SWDT_MASK    0X00001000U

/* Register: PMU_GLOBAL_ERROR_SRST_EN_1 */
#define PMU_GLOBAL_ERROR_SRST_EN_1    ( ( PMU_GLOBAL_BASEADDR ) + 0X0000056CU )
#define PMU_GLOBAL_ERROR_SRST_EN_1_LPD_SWDT_MASK    0X00001000U
#define PMU_GLOBAL_ERROR_SRST_EN_1_FPD_SWDT_MASK    0X00002000U

/* Register: PMU_GLOBAL_ERROR_SRST_DIS_1 */
#define PMU_GLOBAL_ERROR_SRST_DIS_1    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000570U )
#define PMU_GLOBAL_ERROR_SRST_DIS_1_LPD_SWDT_MASK    0X00001000U
#define PMU_GLOBAL_ERROR_SRST_DIS_1_FPD_SWDT_MASK    0X00002000U

/* Register: PMU_GLOBAL_ERROR_EN_1 */
#define PMU_GLOBAL_ERROR_EN_1    ( ( PMU_GLOBAL_BASEADDR ) + 0X000005A0U )
#define PMU_GLOBAL_ERROR_EN_1_LPD_SWDT_MASK    0X00001000U

/* Register: PMU_GLOBAL_REQ_ISO_INT_EN */
#define PMU_GLOBAL_REQ_ISO_INT_EN    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000318U )
#define PMU_GLOBAL_REQ_ISO_INT_EN_PL_NONPCAP_MASK    0X00000004U

/* Register: PMU_GLOBAL_REQ_ISO_TRIG */
#define PMU_GLOBAL_REQ_ISO_TRIG    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000320U )

/* Register: PMU_GLOBAL_REQ_ISO_STATUS */
#define PMU_GLOBAL_REQ_ISO_STATUS    ( ( PMU_GLOBAL_BASEADDR ) + 0X00000310U )

/* rpu */

/**
 * RPU Base Address
 */
#define RPU_BASEADDR      0XFF9A0000U

/**
 * Register: RPU_RPU_GLBL_CNTL
 */
#define RPU_RPU_GLBL_CNTL    ( ( RPU_BASEADDR ) + 0X00000000U )
#define RPU_RPU_GLBL_CNTL_SLSPLIT_MASK    0X00000008U
#define RPU_RPU_GLBL_CNTL_TCM_COMB_MASK    0X00000040U
#define RPU_RPU_GLBL_CNTL_SLCLAMP_MASK    0X00000010U

/**
 * Register: RPU_RPU_0_CFG
 */
#define RPU_RPU_0_CFG    ( ( RPU_BASEADDR ) + 0X00000100U )
#define RPU_RPU_0_CFG_VINITHI_SHIFT		2U
#define RPU_RPU_0_CFG_VINITHI_MASK     0x00000004U
#define RPU_RPU_0_CFG_NCPUHALT_MASK    0X00000001U

/**
 * Register: RPU_RPU_1_CFG
 */
#define RPU_RPU_1_CFG    ( ( RPU_BASEADDR ) + 0X00000200U )
#define RPU_RPU_1_CFG_VINITHI_SHIFT		2U
#define RPU_RPU_1_CFG_VINITHI_MASK     0x00000004U
#define RPU_RPU_1_CFG_NCPUHALT_MASK    0X00000001U

/* rsa */

/**
 * RSA Base Address
 */
#define RSA_BASEADDR      0XFFCE002CU

/**
 * Register: RSA_WR_DATA_0
 */
#define RSA_WR_DATA_0    ( ( RSA_BASEADDR ) + 0X00000000U )

/**
 * Register: RSA_WR_ADDR
 */
#define RSA_WR_ADDR    ( ( RSA_BASEADDR ) + 0X00000018U )

/**
 * Register: RSA_RD_ADDR
 */
#define RSA_RD_ADDR    ( ( RSA_BASEADDR ) + 0X00000034U )

/**
 * Register: RSA_RD_DATA_0
 */
#define RSA_RD_DATA_0    ( ( RSA_BASEADDR ) + 0X0000001CU )

/* rsa_core */

/**
 * RSA_CORE Base Address
 */
#define RSA_CORE_BASEADDR      0XFFCE0000U

/**
 * Register: RSA_CORE_MINV0
 */
#define RSA_CORE_MINV0    ( ( RSA_CORE_BASEADDR ) + 0X00000018U )

/**
 * Register: RSA_CORE_MINV1
 */
#define RSA_CORE_MINV1    ( ( RSA_CORE_BASEADDR ) + 0X0000001CU )

/**
 * Register: RSA_CORE_MINV2
 */
#define RSA_CORE_MINV2    ( ( RSA_CORE_BASEADDR ) + 0X00000020U )

/**
 * Register: RSA_CORE_MINV3
 */
#define RSA_CORE_MINV3    ( ( RSA_CORE_BASEADDR ) + 0X00000024U )

/**
 * Register: RSA_CORE_STATUS
 */
#define RSA_CORE_STATUS    ( ( RSA_CORE_BASEADDR ) + 0X00000014U )
#define RSA_CORE_STATUS_ERROR_MASK    0X00000004U
#define RSA_CORE_STATUS_DONE_MASK    0X00000001U

/**
 * Register: RSA_CORE_CTRL
 */
#define RSA_CORE_CTRL    ( ( RSA_CORE_BASEADDR ) + 0X00000010U )

/* LPD_SLCR Base Address */
#define LPD_SLCR_BASEADDR      0XFF410000U

/* Register: LPD_SLCR_PERSISTENT4 */
#define LPD_SLCR_PERSISTENT4    ( ( LPD_SLCR_BASEADDR ) + 0X00000030U )

/* Register: LPD_SLCR_PERSISTENT5 */
#define LPD_SLCR_PERSISTENT5    ( ( LPD_SLCR_BASEADDR ) + 0X00000034U )

/* IPI Base Address */
#define IPI_BASEADDR      0XFF300000U

/* Register: IPI_PMU_0_TRIG */
#define IPI_PMU_0_TRIG    ( ( IPI_BASEADDR ) + 0X00030000U )
#define IPI_PMU_0_TRIG_PMU_0_MASK    0X00010000U

#define IPI_PMU_0_IER    ( ( IPI_BASEADDR ) + 0X00030018U )
#define IPI_PMU_0_IER_PMU_0_MASK    0X00010000U

/* serdes */
/* SERDES Base Address */
#define SERDES_BASEADDR      0XFD400000U

#define SERDES_ICM_CFG0    ( ( SERDES_BASEADDR ) + 0X00010010U )
#define SERDES_ICM_CFG0_L1_ICM_CFG_SHIFT   4U
#define SERDES_ICM_CFG0_L1_ICM_CFG_MASK    0X00000070U
#define SERDES_ICM_CFG0_L0_ICM_CFG_MASK    0X00000007U

#define SERDES_ICM_CFG1    ( ( SERDES_BASEADDR ) + 0X00010014U )
#define SERDES_ICM_CFG1_L3_ICM_CFG_SHIFT   4U
#define SERDES_ICM_CFG1_L3_ICM_CFG_MASK    0X00000070U
#define SERDES_ICM_CFG1_L2_ICM_CFG_MASK    0X00000007U

/* gpio */
/* GPIO Base Address */
#define GPIO_BASEADDR      0XFF0A0000U

#define GPIO_DATA_1    ( ( GPIO_BASEADDR ) + 0X00000044U )
#define GPIO_DIRM_1    ( ( GPIO_BASEADDR ) + 0X00000244U )
#define GPIO_OEN_1     ( ( GPIO_BASEADDR ) + 0X00000248U )

/* IOU_SLCR Base Address */
#define IOU_SLCR_BASEADDR      0XFF180000U

/* Register: IOU_SLCR_SD_CDN_CTRL */
#define IOU_SLCR_SD_CDN_CTRL    ( ( IOU_SLCR_BASEADDR ) + 0X0000035CU )

#define IOU_SLCR_SD_CDN_CTRL_SD1_CDN_CTRL_MASK    0X00010000U
#define IOU_SLCR_SD_CDN_CTRL_SD0_CDN_CTRL_MASK    0X00000001U


/* Register: ADMA_CH0 Base Address */
#define ADMA_CH0_BASEADDR      0XFFA80000U

/* Register: ADMA_CH0_ZDMA_CH_STATUS */
#define ADMA_CH0_ZDMA_CH_STATUS    ( ( ADMA_CH0_BASEADDR ) + 0X0000011CU )
#define ADMA_CH0_ZDMA_CH_STATUS_STATE_MASK    0X00000003U
#define ADMA_CH0_ZDMA_CH_STATUS_STATE_DONE    0X00000000U
#define ADMA_CH0_ZDMA_CH_STATUS_STATE_ERR     0X00000003U

/* Register: ADMA_CH0_ZDMA_CH_CTRL0 */
#define ADMA_CH0_ZDMA_CH_CTRL0    ( ( ADMA_CH0_BASEADDR ) + 0X00000110U )
#define ADMA_CH0_ZDMA_CH_CTRL0_POINT_TYPE_MASK    (u32)0X00000040U
#define ADMA_CH0_ZDMA_CH_CTRL0_POINT_TYPE_NORMAL  (u32)0X00000000U
#define ADMA_CH0_ZDMA_CH_CTRL0_MODE_MASK    (u32)0X00000030U
#define ADMA_CH0_ZDMA_CH_CTRL0_MODE_WR_ONLY (u32)0X00000010U
#define ADMA_CH0_ZDMA_CH_CTRL0_TOTAL_BYTE_COUNT ( ( ADMA_CH0_BASEADDR ) + 0X00000188U )

/* Register: ADMA_CH0_ZDMA_CH_WR_ONLY_WORD0 */
#define ADMA_CH0_ZDMA_CH_WR_ONLY_WORD0    ( ( ADMA_CH0_BASEADDR ) + 0X00000148U )

/* Register: ADMA_CH0_ZDMA_CH_WR_ONLY_WORD1 */
#define ADMA_CH0_ZDMA_CH_WR_ONLY_WORD1    ( ( ADMA_CH0_BASEADDR ) + 0X0000014CU )

/* Register: ADMA_CH0_ZDMA_CH_WR_ONLY_WORD2 */
#define ADMA_CH0_ZDMA_CH_WR_ONLY_WORD2    ( ( ADMA_CH0_BASEADDR ) + 0X00000150U )

/* Register: ADMA_CH0_ZDMA_CH_WR_ONLY_WORD3 */
#define ADMA_CH0_ZDMA_CH_WR_ONLY_WORD3    ( ( ADMA_CH0_BASEADDR ) + 0X00000154U )

/* Register: ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0 */
#define ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0    ( ( ADMA_CH0_BASEADDR ) + 0X00000138U )
#define ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0_LSB_MASK    0XFFFFFFFFU

/* Register: ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1 */
#define ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1    ( ( ADMA_CH0_BASEADDR ) + 0X0000013CU )
#define ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1_MSB_MASK    0X0001FFFFU

/* Register: ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD2 */
#define ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD2    ( ( ADMA_CH0_BASEADDR ) + 0X00000130U )

/* Register: ADMA_CH0_ZDMA_CH_DST_DSCR_WORD2 */
#define ADMA_CH0_ZDMA_CH_DST_DSCR_WORD2    ( ( ADMA_CH0_BASEADDR ) + 0X00000140U )

/* Register: ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD1 */
#define ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD1    ( ( ADMA_CH0_BASEADDR ) + 0X0000012CU )

/* Register: ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD0 */
#define ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD0    ( ( ADMA_CH0_BASEADDR ) + 0X00000128U )

/* Register: ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD3 */
#define ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD3    ( ( ADMA_CH0_BASEADDR ) + 0X00000134U )

/* Register: ADMA_CH0_ZDMA_CH_DST_DSCR_WORD3 */
#define ADMA_CH0_ZDMA_CH_DST_DSCR_WORD3    ( ( ADMA_CH0_BASEADDR ) + 0X00000144U )

/* Register: ADMA_CH0_ZDMA_CH_CTRL2 */
#define ADMA_CH0_ZDMA_CH_CTRL2    ( ( ADMA_CH0_BASEADDR ) + 0X00000200U )
#define ADMA_CH0_ZDMA_CH_CTRL2_EN_MASK    0X00000001U

/* Register: ADMA_CH0_ZDMA_CH_ISR */
#define ADMA_CH0_ZDMA_CH_ISR    ( ( ADMA_CH0_BASEADDR ) + 0X00000100U )
#define ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK    0X00000400U

/* AMS_PS_SYSMON Base Address */
#define AMS_PS_SYSMON_BASEADDR      0XFFA50800U
#define XFSBL_PS_SYSMON_CONFIGREG1    0XFFA50904U
#define XFSBL_PS_SYSMON_CONFIGREG3    0XFFA5090CU
#define XFSBL_PS_SYSMON_CFGREG1_ALRM_DISBL_MASK  0X0F0EU
#define XFSBL_PS_SYSMON_CFGREG3_ALRM_DISBL_MASK  0X3FU


/* Register: AMS_PS_SYSMON_ANALOG_BUS */
#define AMS_PS_SYSMON_ANALOG_BUS    ( ( AMS_PS_SYSMON_BASEADDR ) + 0X00000114U )

/* Register: ACPU_GIC_GICD_ISPENDR0 */
#define ACPU_GIC_BASEADDR			0xF9000000U

/* Register: ACPU_GIC_GICD_ICENBLR0 */
#define ACPU_GIC_GICD_ICENBLR0    ( ( ACPU_GIC_BASEADDR ) + 0X00010180U )

/* Register: ACPU_GIC_GICD_ICENBLR1 */
#define ACPU_GIC_GICD_ICENBLR1    ( ( ACPU_GIC_BASEADDR ) + 0X00010184U )

/* Register: ACPU_GIC_GICD_ICENBLR2 */
#define ACPU_GIC_GICD_ICENBLR2    ( ( ACPU_GIC_BASEADDR ) + 0X00010188U )

/* Register: ACPU_GIC_GICD_ICENBLR3 */
#define ACPU_GIC_GICD_ICENBLR3    ( ( ACPU_GIC_BASEADDR ) + 0X0001018CU )

/* Register: ACPU_GIC_GICD_ICENBLR4 */
#define ACPU_GIC_GICD_ICENBLR4    ( ( ACPU_GIC_BASEADDR ) + 0X00010190U )

/* Register: ACPU_GIC_GICD_ICENBLR5 */
#define ACPU_GIC_GICD_ICENBLR5    ( ( ACPU_GIC_BASEADDR ) + 0X00010194U )

/* Register: ACPU_GIC_GICD_ICPENDR0 */
#define ACPU_GIC_GICD_ICPENDR0    ( ( ACPU_GIC_BASEADDR ) + 0X00010280U )

/* Register: ACPU_GIC_GICD_ICPENDR1 */
#define ACPU_GIC_GICD_ICPENDR1    ( ( ACPU_GIC_BASEADDR ) + 0X00010284U )

/* Register: ACPU_GIC_GICD_ICPENDR2 */
#define ACPU_GIC_GICD_ICPENDR2    ( ( ACPU_GIC_BASEADDR ) + 0X00010288U )

/* Register: ACPU_GIC_GICD_ICPENDR3 */
#define ACPU_GIC_GICD_ICPENDR3    ( ( ACPU_GIC_BASEADDR ) + 0X0001028CU )

/* Register: ACPU_GIC_GICD_ICPENDR4 */
#define ACPU_GIC_GICD_ICPENDR4    ( ( ACPU_GIC_BASEADDR ) + 0X00010290U )

/* Register: ACPU_GIC_GICD_ICPENDR5 */
#define ACPU_GIC_GICD_ICPENDR5    ( ( ACPU_GIC_BASEADDR ) + 0X00010294U )

/* Register: ACPU_GIC_GICD_ICACTIVER0 */
#define ACPU_GIC_GICD_ICACTIVER0    ( ( ACPU_GIC_BASEADDR ) + 0X00010380U )

/* Register: ACPU_GIC_GICD_ICACTIVER1 */
#define ACPU_GIC_GICD_ICACTIVER1    ( ( ACPU_GIC_BASEADDR ) + 0X00010384U )

/* Register: ACPU_GIC_GICD_ICACTIVER2 */
#define ACPU_GIC_GICD_ICACTIVER2    ( ( ACPU_GIC_BASEADDR ) + 0X00010388U )

/* Register: ACPU_GIC_GICD_ICACTIVER3 */
#define ACPU_GIC_GICD_ICACTIVER3    ( ( ACPU_GIC_BASEADDR ) + 0X0001038CU )

/* Register: ACPU_GIC_GICD_ICACTIVER4 */
#define ACPU_GIC_GICD_ICACTIVER4    ( ( ACPU_GIC_BASEADDR ) + 0X00010390U )

/* Register: ACPU_GIC_GICD_ICACTIVER5 */
#define ACPU_GIC_GICD_ICACTIVER5    ( ( ACPU_GIC_BASEADDR ) + 0X00010394U )

/* Register: ACPU_GIC_GICD_CPENDSGIR0 */
#define ACPU_GIC_GICD_CPENDSGIR0    ( ( ACPU_GIC_BASEADDR ) + 0X00010F10U )

/* Register: ACPU_GIC_GICD_CPENDSGIR1 */
#define ACPU_GIC_GICD_CPENDSGIR1    ( ( ACPU_GIC_BASEADDR ) + 0X00010F14U )

/* Register: ACPU_GIC_GICD_CPENDSGIR2 */
#define ACPU_GIC_GICD_CPENDSGIR2    ( ( ACPU_GIC_BASEADDR ) + 0X00010F18U )

/* Register: ACPU_GIC_GICD_CPENDSGIR3 */
#define ACPU_GIC_GICD_CPENDSGIR3    ( ( ACPU_GIC_BASEADDR ) + 0X00010F1CU )

/* Register: ACPU_GIC_GICD_INTR_ACK_REG */
#define ACPU_GIC_GICD_INTR_ACK_REG    ( ( ACPU_GIC_BASEADDR ) + 0X0002000CU )

/* Register: ACPU_GIC_GICD_END_INTR_REG */
#define ACPU_GIC_GICD_END_INTR_REG    ( ( ACPU_GIC_BASEADDR ) + 0X00020010U )

/* Register: ACPU_GIC_GICD_CACTIVESGIR0 */
#define ACPU_GIC_GICD_CACTIVESGIR0    ( ( ACPU_GIC_BASEADDR ) + 0X00010F10U )

/* Register: ACPU_GIC_GICD_CACTIVESGIR1 */
#define ACPU_GIC_GICD_CACTIVESGIR1    ( ( ACPU_GIC_BASEADDR ) + 0X00010F14U )

/* Register: ACPU_GIC_GICD_CACTIVESGIR2 */
#define ACPU_GIC_GICD_CACTIVESGIR2    ( ( ACPU_GIC_BASEADDR ) + 0X00010F18U )

/* Register: ACPU_GIC_GICD_CACTIVESGIR3 */
#define ACPU_GIC_GICD_CACTIVESGIR3    ( ( ACPU_GIC_BASEADDR ) + 0X00010F1CU )


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/



/**
 * For error reporting PMU_GLOBAL_PERS_GLOB_GEN_STORAGE4 is used
 */
#define XFSBL_ERROR_STATUS_REGISTER_OFFSET	(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE4)


/* PMU RAM address for PMU FW */
#define XFSBL_PMU_RAM_START_ADDRESS		(0xFFDC0000U)
#define XFSBL_PMU_RAM_END_ADDRESS		(0xFFDDFFFFU)

/**
 * If address where bitstream need to be copied is not configured, below will
 * be the default address to indicate it is invalid address. In such cases,
 * XFSBL_DDR_TEMP_ADDRESS is used as load address.
 */
#define XFSBL_DUMMY_PL_ADDR				(0xFFFFFFFFU)

/**
 * ARM Processor defines
 */
#define XFSBL_CLUSTER_ID_MASK			(0x00000F00U)
#define XFSBL_A53_PROCESSOR			(0x00000000U)
#define XFSBL_R5_PROCESSOR			(0x00000100U)


/**
 * Other FSBL defines
 * this can defined in xfsbl_main.h
 */
#define XFSBL_R5_0				(0x1U)
#define XFSBL_R5_L				(0x2U)

/**
 * To indicate usage of RPU cores to PMU, PMU_GLOBAL_GLOB_GEN_STORAGE4 is used
 */
#define XFSBL_R5_USAGE_STATUS_REG		(PMU_GLOBAL_GLOB_GEN_STORAGE4)
/* Bit 1 of rpu uasge status register is used for R50 status */
#define XFSBL_R5_0_STATUS_MASK			(1U << 1)
/* Bit 2 of rpu usage status register is used for R51 status */
#define XFSBL_R5_1_STATUS_MASK			(1U << 2)

/**
 * TCM address for R5
 */
#define XFSBL_R5_TCM_START_ADDRESS		(u32)(0x0U)
#define XFSBL_R5_BTCM_START_ADDRESS		(0x20000U)

#define XFSBL_R50_HIGH_ATCM_START_ADDRESS	(0xFFE00000U)
#define XFSBL_R50_HIGH_BTCM_START_ADDRESS	(0xFFE20000U)
#define XFSBL_R51_HIGH_ATCM_START_ADDRESS	(0xFFE90000U)
#define XFSBL_R51_HIGH_BTCM_START_ADDRESS	(0xFFEB0000U)

#define XFSBL_R5_TCM_BANK_LENGTH			(0x10000U)

/**
 * defines for the FSBL peripherals present
 */

/**
 * Definition for WDT to be included
 */
#if (!defined(FSBL_WDT_EXCLUDE) && defined(XPAR_PSU_WDT_0_DEVICE_ID))
#define XFSBL_WDT_PRESENT
#define XFSBL_WDT_DEVICE_ID	XPAR_PSU_WDT_0_DEVICE_ID
#define XFSBL_WDT_MASK		PMU_GLOBAL_ERROR_SRST_EN_1_LPD_SWDT_MASK
#elif (!defined(FSBL_WDT_EXCLUDE) && defined(XPAR_PSU_WDT_1_DEVICE_ID))
#define XFSBL_WDT_PRESENT
#define XFSBL_WDT_DEVICE_ID	XPAR_PSU_WDT_1_DEVICE_ID
#define XFSBL_WDT_MASK		PMU_GLOBAL_ERROR_SRST_EN_1_FPD_SWDT_MASK
#endif

/**
 * Definitions for SD to be included
 */
#if (!defined(FSBL_SD_EXCLUDE) && (XPAR_XSDPS_0_BASEADDR == 0xFF160000))
#define XFSBL_SD_0
#endif

#if (!defined(FSBL_SD_EXCLUDE) && (XPAR_XSDPS_0_BASEADDR == 0xFF170000) ||\
		(XPAR_XSDPS_1_BASEADDR == 0xFF170000))
#define XFSBL_SD_1
#endif

/**
 * Definition for QSPI to be included
 */
#if (!defined(FSBL_QSPI_EXCLUDE) && defined(XPAR_XQSPIPSU_0_DEVICE_ID))
#define XFSBL_QSPI
#define XFSBL_QSPI_BASEADDRESS	XPAR_XQSPIPS_0_BASEADDR
#define XFSBL_QSPI_BUSWIDTH_ONE			0U
#define XFSBL_QSPI_BUSWIDTH_TWO			1U
#define XFSBL_QSPI_BUSWIDTH_FOUR		2U
#endif

/**
 * Definition for NAND to be included
 */
#if (!defined(FSBL_NAND_EXCLUDE) && defined(XPAR_XNANDPSU_0_DEVICE_ID))
#define XFSBL_NAND
#endif

/**
 * Definition for SECURE to be included
 */
#if !defined(FSBL_SECURE_EXCLUDE)
#define XFSBL_SECURE
#endif

/**
 * Definition for PL bitsream feature to be included
 */
#if !defined(FSBL_BS_EXCLUDE)
#define XFSBL_BS
#endif


/**
 * Definition for early handoff feature to be included
 */
#if !defined(FSBL_EARLY_HANDOFF_EXCLUDE)
#define XFSBL_EARLY_HANDOFF
#endif

#if !defined(FSBL_PERF_EXCLUDE) && (!defined(ARMR5) || (defined(ARMR5) && defined(SLEEP_TIMER_BASEADDR)))
#define XFSBL_PERF
#endif

/* Definition for TCM ECC Enable for A53 to be included */
#if !defined(FSBL_A53_TCM_ECC_EXCLUDE)
#define XFSBL_A53_TCM_ECC
#endif

/* Definition for PL clear include irrespective of boot image has bitstream or not */
#if !defined(FSBL_PL_CLEAR_EXCLUDE)
#define XFSBL_PL_CLEAR
#endif

/*
 * Definition for forcing encryption for each partition
 * when ENC_ONLY FUSE bit is blown
 */
#if !defined(FSBL_FORCE_ENC_EXCLUDE)
#define XFSBL_FORCE_ENC
#endif

#define XFSBL_QSPI_LINEAR_BASE_ADDRESS_START		(0xC0000000U)
#define XFSBL_QSPI_LINEAR_BASE_ADDRESS_END		(0xDFFFFFFFU)

#if defined(XPAR_PSU_DDR_0_S_AXI_BASEADDR) 	\
		|| defined(XPAR_PSU_R5_DDR_0_S_AXI_BASEADDR)
#define XFSBL_PS_DDR
#endif

#define XFSBL_PS_DDR_START_ADDRESS		(0x0U)
#define XFSBL_PS_DDR_START_ADDRESS_R5	(0x100000U)

#if ((!defined(FSBL_PL_LOAD_FROM_OCM_EXCLUDE)) || (!defined(XFSBL_PS_DDR)))
#define XFSBL_PL_LOAD_FROM_OCM
#endif

#if (!defined(FSBL_USB_EXCLUDE) && defined(XPAR_XUSBPSU_0_DEVICE_ID) && (XPAR_XUSBPSU_0_BASEADDR == 0xFE200000) && defined(XFSBL_PS_DDR))
#define XFSBL_USB
#endif

#if (!defined(FSBL_TPM_EXCLUDE) && defined(XPAR_XSPIPS_0_DEVICE_ID)\
	&& defined(XFSBL_PS_DDR) && (!defined(XFSBL_TPM)))
#define XFSBL_SPI_DEVICE_ID	XPAR_XSPIPS_0_DEVICE_ID
#define XFSBL_TPM
#endif

#if !defined(FSBL_PROT_BYPASS_EXCLUDE)
#define XFSBL_PROT_BYPASS
#endif

#ifdef ARMR5
#define XFSBL_PS_DDR_INIT_START_ADDRESS	XFSBL_PS_DDR_START_ADDRESS_R5
#if defined(XPAR_PSU_R5_DDR_1_S_AXI_BASEADDR)
#define XFSBL_PS_HI_DDR_START_ADDRESS	XPAR_PSU_R5_DDR_1_S_AXI_BASEADDR
#define XFSBL_PS_HI_DDR_END_ADDRESS XPAR_PSU_R5_DDR_1_S_AXI_HIGHADDR
#endif
#else
#define XFSBL_PS_DDR_INIT_START_ADDRESS	XFSBL_PS_DDR_START_ADDRESS
#if defined(XPAR_PSU_DDR_1_S_AXI_BASEADDR)
#define XFSBL_PS_HI_DDR_START_ADDRESS	XPAR_PSU_DDR_1_S_AXI_BASEADDR
#define XFSBL_PS_HI_DDR_END_ADDRESS XPAR_PSU_DDR_1_S_AXI_HIGHADDR
#endif
#endif

#ifdef XFSBL_PS_DDR
#ifdef ARMR5
#define XFSBL_PS_DDR_END_ADDRESS		(XPAR_PSU_R5_DDR_0_S_AXI_HIGHADDR)
#else
#define XFSBL_PS_DDR_END_ADDRESS		(XPAR_PSU_DDR_0_S_AXI_HIGHADDR)
#endif
#endif

#ifdef XFSBL_ENABLE_DDR_SR
/*
 * For DDR status PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7 is used
 */
#define XFSBL_DDR_STATUS_REGISTER_OFFSET	(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7)
/*
 * DDR controller initialization flag mask
 *
 * This flag signals whether DDR controller have been initialized or not. It is
 * used by FSBL to inform PMU that DDR controller is initialized. When booting
 * with DDR in self refresh mode, PMU must wait until DDR controller have been
 * initialized by the FSBL before it can bring the DDR out of self refresh mode.
 */
#define DDRC_INIT_FLAG_MASK			(1U << 4)
/*
 * DDR self refresh mode indication flag mask
 *
 * This flag indicates whether DDR is in self refresh mode or not. It is used
 * by PMU to signal FSBL in order to skip over DDR phy and ECC initialization
 * at boot time.
 */
#define DDR_STATUS_FLAG_MASK			(1U << 3)
#endif

/* The number of bytes transferred per cycle of DMA should be
 * 64 bit aligned to avoid any ECC errors. Hence, even though the maximum
 * number of bytes that can be sent per cycle is 1GB -1, only 1GB -8 bytes
 * are sent to preserve the 64 bit alignment.
 */
#define ZDMA_TRANSFER_MAX_LEN (0x3FFFFFFFU - 7U)

#define EFUSE_IPDISABLE_CG_MASK ((EFUSE_IPDISABLE_GPU_DIS_MASK) | \
		(EFUSE_IPDISABLE_APU2_DIS_MASK) | (EFUSE_IPDISABLE_APU3_DIS_MASK))

#define XFSBL_OCM
#define XFSBL_OCM_START_ADDRESS			(0xFFFEA000U)
#define XFSBL_OCM_END_ADDRESS			(0xFFFFFFFFU)

/* Different Memory types */
#define XFSBL_R5_0_TCM		(0x1U)
#define XFSBL_R5_1_TCM		(0x2U)
#define XFSBL_R5_L_TCM		(0x3U)

#define PTRSIZE		UINTPTR

/* Reset Reason */
#define XFSBL_SYSTEM_RESET		0U
#define XFSBL_PS_ONLY_RESET		1U
#define XFSBL_MASTER_ONLY_RESET	2U

/* AMS PS Sysmon ANALOG_BUS value */
#define PS_SYSMON_ANALOG_BUS_VAL 0X00003210U

/****************************************************************************/
/**
*
* Read the given register.
*
* @param	BaseAddr is the base address of the device
* @param	RegOffset is the register offset to be read
*
* @return	The 32-bit value of the register
*
* @note		C-style signature:
*		u32 XFsbl_ReadReg(u32 BaseAddr, u32 RegOffset)
*
*****************************************************************************/
#define XFsbl_ReadReg(BaseAddr, RegOffset)		\
	Xil_In32((BaseAddr) + (RegOffset))

#define XFsbl_In32(Addr)		Xil_In32(Addr)

#define XFsbl_In64(Addr)		Xil_In64(Addr)

/****************************************************************************/
/**
*
* Write to the given register.
*
* @param	BaseAddr is the base address of the device
* @param	RegOffset is the register offset to be written
* @param	Data is the 32-bit value to write to the register
*
* @return	None.
*
* @note		C-style signature:
*		void XFsbl_WriteReg(u32 BaseAddr, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XFsbl_WriteReg(BaseAddr, RegOffset, Data)	\
	Xil_Out32((BaseAddr) + (RegOffset), (Data))

#define XFsbl_Out32(Addr, Data)		Xil_Out32(Addr, Data)

#define XFsbl_Out64(Addr, Data)		Xil_Out64(Addr, Data)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_HW_H */
