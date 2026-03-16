/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 */


#ifndef PM_MMIO_ACCESS_H
#define PM_MMIO_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_MEM_RANGE

#ifdef ENABLE_MEM_RANGE_PM_SET_CONFIG
#define BASE_CONFIG_OBJ_START_ADDR	0xFFFD6740U
#define BASE_CONFIG_OBJ_LEN		0x1DBU

#define OVERLAY_CONFIG_OBJ_START_ADDR	0x812CB7C
#define OVERLAY_CONFIG_OBJ_LEN		0x20U
#endif

#ifdef ENABLE_MEM_RANGE_PM_SELF_SUSPEND
#define SELF_SUSPEND_DDR_START_ADDR	0x0U
#define SELF_SUSPEND_OCM_START_ADDR	0xFFFEA144U
#define SELF_SUSPEND_DDR_OCM_ADDR_LEN	0x4U
#endif

#ifdef ENABLE_MEM_RANGE_PM_REQUEST_WAKEUP
#define REQUEST_WAKEUP_OCM_START_ADDR	0xFFFEA144U
#define REQUEST_WAKEUP_OCM_ADDR_LEN	0x4U
#endif

/**
 * @enum XPm_MemRangeAccessType
 * @brief Defines the types of access permissions for a memory range.
 */
typedef enum {
        MEM_RANGE_READ_ACCESS = 1, /**< Accessible for read operations only. */
        MEM_RANGE_WRITE_ACCESS,	   /**< Accessible for write operations only. */
        MEM_RANGE_ANY_ACCESS,	   /**< Accessible for both read and write operations. */
}XPm_MemRangeAccessType;
#endif

/*********************************************************************
 * pmAccessTable[] Indices
 *
 * These indices correspond to entries in the pmAccessTable[] array
 * in pm_mmio_access.c. Use these named constants instead of hardcoded
 * numbers for better maintainability.
 *
 * WARNING: These indices are based on the default build configuration.
 * Conditional compilation (XPAR_VCU_0_BASEADDR, SECURE_ACCESS, etc.)
 * can change the actual indices. Use with caution.
 ********************************************************************/
typedef enum {
	PM_MMIO_IDX_CRF_APB_0 = 0,
	PM_MMIO_IDX_CRF_APB_1,
	PM_MMIO_IDX_CRF_APB_2,
	PM_MMIO_IDX_CRL_APB_0,
	PM_MMIO_IDX_CRL_APB_1,
	PM_MMIO_IDX_CRL_APB_2,
	PM_MMIO_IDX_CRL_APB_3,
	PM_MMIO_IDX_PMU_GLOBAL_PWR_STATE,
	PM_MMIO_IDX_PMU_GLOBAL_GEN_STORAGE,
	PM_MMIO_IDX_IOU_SLCR_0,
	PM_MMIO_IDX_IOU_SLCR_1,
	PM_MMIO_IDX_CRL_APB_RO,
	PM_MMIO_IDX_CRF_APB_RO,
	PM_MMIO_IDX_CRL_APB_BOOT_PIN_CTRL,
	PM_MMIO_IDX_PMU_LOCAL_DOM_ISO_CTRL,
#ifdef XPAR_VCU_0_BASEADDR
	PM_MMIO_IDX_VCU_SLCR,
#endif
	PM_MMIO_IDX_CRF_APB_FPD_RESETS,
	PM_MMIO_IDX_CRL_APB_LPD_RESETS,
	PM_MMIO_IDX_FPD_SLCR_AFI_FS,
	PM_MMIO_IDX_LPD_SLCR_AFI_FS,
	PM_MMIO_IDX_AFI_FM0,
	PM_MMIO_IDX_AFI_FM1,
	PM_MMIO_IDX_AFI_FM2,
	PM_MMIO_IDX_AFI_FM3,
	PM_MMIO_IDX_AFI_FM4,
	PM_MMIO_IDX_AFI_FM5,
	PM_MMIO_IDX_AFI_FM6,
	PM_MMIO_IDX_CSU_STATUS,
	PM_MMIO_IDX_CSU_MULTI_BOOT,
	PM_MMIO_IDX_CSU_TAMPER_TRIG,
	PM_MMIO_IDX_CSU_FT_STATUS,
	PM_MMIO_IDX_CSU_JTAG_CHAIN_STATUS,
	PM_MMIO_IDX_CSU_IDCODE,
	PM_MMIO_IDX_CSU_ROM_DIGEST,
	PM_MMIO_IDX_CSU_AES_STATUS,
	PM_MMIO_IDX_CSU_PCAP_STATUS,
	PM_MMIO_IDX_PMU_GLOBAL_CNTRL,
	PM_MMIO_IDX_PMU_GLOBAL_REQ_ISO_STATUS,
	PM_MMIO_IDX_PMU_GLOBAL_REQ_SWRST_STATUS,
	PM_MMIO_IDX_PMU_GLOBAL_CSU_BR_ERROR,
	PM_MMIO_IDX_PMU_GLOBAL_SAFETY_CHK,
	PM_MMIO_IDX_EFUSE_IPDISABLE,
#ifdef SECURE_ACCESS
	PM_MMIO_IDX_CSU_CTRL,
	PM_MMIO_IDX_CSU_ISR,
	PM_MMIO_IDX_CSU_IMR,
	PM_MMIO_IDX_CSU_IER,
	PM_MMIO_IDX_CSU_IDR,
	PM_MMIO_IDX_CSU_JTAG_SEC,
	PM_MMIO_IDX_CSU_AES_KEY_SRC,
	PM_MMIO_IDX_CSU_AES_KUP_0,
	PM_MMIO_IDX_CSU_AES_IV_0,
	PM_MMIO_IDX_CSU_SHA_START,
	PM_MMIO_IDX_CSU_SHA_RESET,
	PM_MMIO_IDX_CSU_SHA_DONE,
	PM_MMIO_IDX_CSU_PCAP_PROG,
	PM_MMIO_IDX_CSU_PCAP_RDWR,
	PM_MMIO_IDX_CSU_PCAP_CTRL,
	PM_MMIO_IDX_CSU_PCAP_RESET,
	PM_MMIO_IDX_CSU_TAMPER_STATUS,
	PM_MMIO_IDX_CSU_TAMPER_0,
	PM_MMIO_IDX_CSUDMA,
	PM_MMIO_IDX_RSA,
	PM_MMIO_IDX_RSA_RD_DATA_0,
	PM_MMIO_IDX_RSA_RD_ADDR,
	PM_MMIO_IDX_RSA_RSA_CFG,
	PM_MMIO_IDX_RSA_RSA_IMR,
	PM_MMIO_IDX_RSA_RSA_IER,
	PM_MMIO_IDX_RSA_CORE,
	PM_MMIO_IDX_RSA_CORE_RSA_RD_DATA,
	PM_MMIO_IDX_RSA_CORE_RSA_RD_ADDR,
	PM_MMIO_IDX_RSA_CORE_STATUS,
	PM_MMIO_IDX_RSA_CORE_MINV0,
	PM_MMIO_IDX_PMU_GLOBAL_GLOBAL_CNTRL,
	PM_MMIO_IDX_PMU_GLOBAL_ADDR_ERROR_INT_EN,
	PM_MMIO_IDX_PMU_GLOBAL_DDR_CNTRL,
	PM_MMIO_IDX_PMU_GLOBAL_RAM_RET_CNTRL,
	PM_MMIO_IDX_PMU_GLOBAL_REQ_PWRUP_STATUS,
	PM_MMIO_IDX_PMU_GLOBAL_REQ_PWRUP_INT_EN,
	PM_MMIO_IDX_PMU_GLOBAL_REQ_PWRDWN_STATUS,
	PM_MMIO_IDX_PMU_GLOBAL_REQ_PWRDWN_INT_EN,
	PM_MMIO_IDX_PMU_GLOBAL_REQ_ISO_INT_EN,
	PM_MMIO_IDX_PMU_GLOBAL_REQ_SWRST_INT_EN,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_STATUS_1,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_INT_EN_1,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_INT_EN_2,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_POR_EN_1,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_POR_EN_2,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_SRST_EN_1,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_SRST_EN_2,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_SIG_EN_1,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_SIG_EN_2,
	PM_MMIO_IDX_PMU_GLOBAL_ERROR_EN_1,
	PM_MMIO_IDX_PMU_GLOBAL_AIB_CNTRL,
	PM_MMIO_IDX_PMU_GLOBAL_GLOBAL_RESET,
	PM_MMIO_IDX_PMU_GLOBAL_SAFETY_GATE,
	PM_MMIO_IDX_PMU_GLOBAL_MBIST_RST,
#endif
#ifdef ENABLE_EM
	PM_MMIO_IDX_PMU_LOCAL_PMU_SERV_ERR,
#endif
} PmMmioAccessTableIndex;

#ifdef ENABLE_CSU_REG_ACCESS

/**
 * CSU Register IDs
 * These IDs are used to identify specific CSU registers
 */
typedef enum {
	CSU_REG_MULTIBOOT = 0U,    /**< CSU Multiboot register */
	CSU_REG_IDCODE,            /**< CSU IDCODE register */
	CSU_REG_PCAP_STATUS,       /**< CSU PCAP Status register */
#ifdef SECURE_ACCESS
	CSU_REG_PCAP_PROG,         /**< CSU PCAP Program register */
	CSU_REG_PCAP_CTRL,	   /**< CSU PCAP Control register */
#endif
	CSU_REG_MAX                /**< Maximum number of CSU registers */
} XPm_CsuRegId;

/* CSU Register function declarations */
u32 PmCsuRegGetCount(void);
s32 PmCsuRegGetName(const u32 regIdx, u32 *resp);
s32 PmCsuRegRead(const PmMaster* master, const u32 regId, u32* value);
s32 PmCsuRegWrite(const PmMaster* master, const u32 regId, const u32 mask, const u32 value);

#endif /* ENABLE_CSU_REG_ACCESS */

/*********************************************************************
 * Function declarations
 ********************************************************************/
bool PmGetMmioAccessRead(const PmMaster *const master, const u32 address);
bool PmGetMmioAccessWrite(const PmMaster *const master, const u32 address);
#ifdef ENABLE_MEM_RANGE
u32 PmIsValidAddressRange(const PmMaster *const master, const u64 address,
			  const u32 length, const u32 access);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PM_MMIO_ACCESS_H */
