/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_REPAIR_REGS_H_
#define XPM_REPAIR_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* CPM5M register definitions */
#define CPM5N_SLCR_BASEADDR (0xE4A10000U)
#define CPM5N_SLCR_WPROTP ((CPM5N_SLCR_BASEADDR) + 0x00000004U)
#define CPM5N_SLCR_WPROTP_DEFVAL (0x1U)
#define CPM5N_SLCR_BISR_CACHE_CTRL ((CPM5N_SLCR_BASEADDR) + 0x00000504U)
#define CPM5N_SLCR_BISR_CACHE_CTRL_TRIGGER_GLOBAL_MASK (0x00000004U)
#define CPM5N_SLCR_BISR_CACHE_CTRL_TRIGGER_DPU_MASK (0x00000002U)
#define CPM5N_SLCR_BISR_CACHE_CTRL_TRIGGER_PCIE_CDX_INTWRAP_MASK 0x00000001U
#define CPM5N_SLCR_BISR_CACHE_DATA_0 ((CPM5N_SLCR_BASEADDR) + 0x00000508U)
#define CPM5N_SLCR_BISR_CACHE_CTRL ((CPM5N_SLCR_BASEADDR) + 0x00000504U)
#define CPM5N_SLCR_BISR_CACHE_CTRL_CLR_MASK (0x00000010U)
#define CPM5N_SLCR_BISR_CACHE_STATUS ((CPM5N_SLCR_BASEADDR) + 0x00000500U)
#define CPM5N_SLCR_BISR_CACHE_STATUS_PASS_GLOBAL_MASK (0x00000020U)
#define CPM5N_SLCR_BISR_CACHE_STATUS_DONE_GLOBAL_MASK (0x00000010U)
#define CPM5N_SLCR_BISR_CACHE_STATUS_PASS_DPU_MASK (0x00000008U)
#define CPM5N_SLCR_BISR_CACHE_STATUS_DONE_DPU_MASK (0x00000004U)
#define CPM5N_SLCR_BISR_CACHE_STATUS_PASS_PCIE_CDX_INTWRAP_MASK (0x00000002U)
#define CPM5N_SLCR_BISR_CACHE_STATUS_DONE_PCIE_CDX_INTWRAP_MASK (0x00000001U)

/* LPX register definitions */
#define LPD_SLCR_BASEADDR				(0xEB410000U)
#define LPD_SLCR_BISR_CACHE_DATA_0			(LPD_SLCR_BASEADDR + 0x0000010CU)
#define LPD_SLCR_BISR_CACHE_CTRL_0			(LPD_SLCR_BASEADDR + 0x00000100U)
#define LPD_SLCR_BISR_CACHE_CTRL_1			(LPD_SLCR_BASEADDR + 0x00000104U)
#define LPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK		(0x00000010U)
#define LPD_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK		(0x00000001U)
#define LPD_SLCR_BISR_CACHE_CTRL_1_FULLMASK		(0x0000001FU)
#define LPD_SLCR_BISR_CACHE_STATUS			(LPD_SLCR_BASEADDR + 0x00000108U)
#define LPD_SLCR_BISR_CACHE_STATUS_DONE_GLOBAL_MASK	(0x40000000U)
#define LPD_SLCR_BISR_CACHE_STATUS_DONE_0_MASK		(0x00000001U)
#define LPD_SLCR_BISR_CACHE_STATUS_DONE_1_MASK		(0x00000004U)
#define LPD_SLCR_BISR_CACHE_STATUS_DONE_2_MASK		(0x00000010U)
#define LPD_SLCR_BISR_CACHE_STATUS_DONE_3_MASK		(0x00000040U)
#define LPD_SLCR_BISR_CACHE_STATUS_DONE_4_MASK		(0x00000100U)
#define LPD_SLCR_BISR_CACHE_STATUS_PASS_GLOBAL_MASK	(0x80000000U)
#define LPD_SLCR_BISR_CACHE_STATUS_PASS_0_MASK		(0x00000002U)
#define LPD_SLCR_BISR_CACHE_STATUS_PASS_1_MASK		(0x00000008U)
#define LPD_SLCR_BISR_CACHE_STATUS_PASS_2_MASK		(0x00000020U)
#define LPD_SLCR_BISR_CACHE_STATUS_PASS_3_MASK		(0x00000080U)
#define LPD_SLCR_BISR_CACHE_STATUS_PASS_4_MASK		(0x00000200U)

/* FPX register definitions */
#define FPD_SLCR_BASEADDR                               (0xEC8C0000U)
#define FPD_SLCR_BISR_CACHE_DATA_0                      (FPD_SLCR_BASEADDR + 0x00000410U)
#define FPD_SLCR_BISR_CACHE_DATA_16                     (FPD_SLCR_BASEADDR + 0x00000500U)
#define FPD_SLCR_BISR_CACHE_DATA_32                     (FPD_SLCR_BASEADDR + 0x00000580U)
#define FPD_SLCR_BISR_CACHE_DATA_48                     (FPD_SLCR_BASEADDR + 0x00000600U)
#define FPD_SLCR_BISR_CACHE_DATA_64                     (FPD_SLCR_BASEADDR + 0x00000680U)
#define FPD_SLCR_BISR_CACHE_CTRL_0                      (FPD_SLCR_BASEADDR + 0x00000400U)
#define FPD_SLCR_BISR_CACHE_CTRL_0_CLR_MASK             (0x00000010U)
#define FPD_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK         (0x00000001U)
#define FPD_SLCR_BISR_CACHE_CTRL_1                      (FPD_SLCR_BASEADDR + 0x00000404U)
#define FPD_SLCR_BISR_CACHE_CTRL_1_FULLMASK             (0x001fffffU)
#define FPD_SLCR_BISR_CACHE_STATUS1                     (FPD_SLCR_BASEADDR + 0x0000040CU)
#define FPD_SLCR_BISR_CACHE_STATUS1_PASS_15_MASK        (0x00000002U)
#define FPD_SLCR_BISR_CACHE_STATUS1_PASS_16_MASK        (0x00000008U)
#define FPD_SLCR_BISR_CACHE_STATUS1_PASS_17_MASK        (0x00000020U)
#define FPD_SLCR_BISR_CACHE_STATUS1_PASS_18_MASK        (0x00000080U)
#define FPD_SLCR_BISR_CACHE_STATUS1_PASS_19_MASK        (0x00000200U)
#define FPD_SLCR_BISR_CACHE_STATUS1_PASS_20_MASK        (0x00000800U)
#define FPD_SLCR_BISR_CACHE_STATUS0                     (FPD_SLCR_BASEADDR + 0x00000408U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_MASK           (0x00000002U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_0_MASK         (0x00000008U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_1_MASK         (0x00000020U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_2_MASK         (0x00000080U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_3_MASK         (0x00000200U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_4_MASK         (0x00000800U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_5_MASK         (0x00002000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_6_MASK         (0x00008000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_7_MASK         (0x00020000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_8_MASK         (0x00080000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_9_MASK         (0x00200000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_10_MASK        (0x00800000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_11_MASK        (0x02000000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_12_MASK        (0x08000000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_13_MASK        (0x20000000U)
#define FPD_SLCR_BISR_CACHE_STATUS0_PASS_14_MASK        (0x80000000U)

/* HNICX register definitions */
#define HNICX_NPI_0_BASEADDR                            (0xF6AF0000U)
#define HNICX_NPI_0_BISR_CACHE_DATA0                    (HNICX_NPI_0_BASEADDR + 0x00000420U)
#define HNICX_NPI_0_BISR_CACHE_CNTRL                    (HNICX_NPI_0_BASEADDR + 0x00000400U)
#define HNICX_NPI_0_BISR_CACHE_STATUS                   (HNICX_NPI_0_BASEADDR + 0x00000404U)
#define HNICX_NPI_0_NPI_CSR_INST                        (HNICX_NPI_0_BASEADDR + 0x00000200U)
#define HNICX_NPI_0_NPI_CSR_WDATA                       (HNICX_NPI_0_BASEADDR + 0x00000204U)
#define HNICX_NPI_0_NPI_CSR_WR_STATUS                   (HNICX_NPI_0_BASEADDR + 0x00000208U)
#define HNICX_NPI_0_NPI_CSR_WR_STATUS_BVALID_MASK       (0x00000001U)
#define HNICX_NPI_0_BISR_CACHE_CNTRL_BISR_TRIGGER_NTHUB_MASK    (0x00000002U)
#define HNICX_NPI_0_BISR_CACHE_STATUS_BISR_DONE_NTHUB_MASK      (0x00000002U)
#define HNICX_NPI_0_BISR_CACHE_STATUS_BISR_PASS_NTHUB_MASK      (0x00000001U)
#define HNICX_NPI_0_BISR_CACHE_CNTRL_BISR_TRIGGER_DPU_MASK      (0x00000020U)
#define HNICX_NPI_0_BISR_CACHE_STATUS_BISR_PASS_DPU_MASK        (0x00000010U)
#define HNICX_NPI_0_BISR_CACHE_STATUS_BISR_DONE_DPU_MASK        (0x00000020U)
#define HNICX_NPI_0_BISR_CACHE_CNTRL_BISR_TRIGGER_LCS_MASK (0x00000008U)
#define HNICX_NPI_0_BISR_CACHE_STATUS_BISR_PASS_LCS_MASK (0x00000004U)
#define HNICX_NPI_0_BISR_CACHE_STATUS_BISR_DONE_LCS_MASK (0x00000008U)


#ifdef __cplusplus
}
#endif

#endif /* XPM_REPAIR_REGS_H_ */
