/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCLOCK_HW_H /* prevent circular inclusions */
#define XCLOCK_HW_H

/****************************************************************************/
/**
*
* @file xclockps_hw.h
* @brief Defines CLOCKPS register offsets and addresses.
*
* This file contains the hardware details for the System Clock controller.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    02/09/18 First release
* 1.6   sd     10/30/23 Fix the redefinition of the PSS_REF
* 1.8   tb    09/17/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup clockps_api CLOCKPS APIs
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* CRL APB register defines */
#ifndef XPAR_PSU_PSS_REF_CLK_FREQ_HZ
/** PSS reference clock frequency in Hz. */
#define XPAR_PSU_PSS_REF_CLK_FREQ_HZ		33330000U
#endif
#ifndef XPAR_PSU_CRL_APB_S_AXI_BASEADDR
/** CRL APB controller base address. */
#define XPAR_PSU_CRL_APB_S_AXI_BASEADDR		0xFF5E0000
#endif
#ifndef XPAR_PSU_IOUSLCR_0_S_AXI_BASEADDR
/** IOU SLCR controller base address. */
#define XPAR_PSU_IOUSLCR_0_S_AXI_BASEADDR	0xFF180000
#endif
#ifndef XPAR_PSU_CRF_APB_S_AXI_BASEADDR
/** CRF APB controller base address. */
#define XPAR_PSU_CRF_APB_S_AXI_BASEADDR		0xFD1A0000
#endif
/** CRL APB base address. */
#define CRL_APB_BASE_ADDR    XPAR_PSU_CRL_APB_S_AXI_BASEADDR
/** IOPLL control register address. */
#define IOPLL_CTRL           (u32)(CRL_APB_BASE_ADDR + 0x20)
/** RPLL control register address. */
#define RPLL_CTRL            (u32)(CRL_APB_BASE_ADDR + 0x30)
/** IOPLL-to-FPD control register address. */
#define IOPLL_TO_FPD_CTRL    (u32)(CRL_APB_BASE_ADDR + 0x44)
/** RPLL-to-FPD control register address. */
#define RPLL_TO_FPD_CTRL     (u32)(CRL_APB_BASE_ADDR + 0x48)
/** USB3 dual ref control register address. */
#define USB3_DUAL_REF_CTRL   (u32)(CRL_APB_BASE_ADDR + 0x4C)
/** GEM0 ref control register address. */
#define GEM0_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x50)
/** GEM1 ref control register address. */
#define GEM1_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x54)
/** GEM2 ref control register address. */
#define GEM2_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x58)
/** GEM3 ref control register address. */
#define GEM3_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x5C)
/** USB0 bus ref control register address. */
#define USB0_BUS_REF_CTRL    (u32)(CRL_APB_BASE_ADDR + 0x60)
/** USB1 bus ref control register address. */
#define USB1_BUS_REF_CTRL    (u32)(CRL_APB_BASE_ADDR + 0x64)
/** QSPI ref control register address. */
#define QSPI_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x68)
/** SDIO0 ref control register address. */
#define SDIO0_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x6C)
/** SDIO1 ref control register address. */
#define SDIO1_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x70)
/** UART0 ref control register address. */
#define UART0_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x74)
/** UART1 ref control register address. */
#define UART1_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x78)
/** SPI0 ref control register address. */
#define SPI0_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x7C)
/** SPI1 ref control register address. */
#define SPI1_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x80)
/** CAN0 ref control register address. */
#define CAN0_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x84)
/** CAN1 ref control register address. */
#define CAN1_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x88)
/** CPU R5 control register address. */
#define CPU_R5_CTRL          (u32)(CRL_APB_BASE_ADDR + 0x90)
/** IOU switch control register address. */
#define IOU_SWITCH_CTRL      (u32)(CRL_APB_BASE_ADDR + 0x9C)
/** CSU PLL control register address. */
#define CSU_PLL_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xA0)
/** PCAP control register address. */
#define PCAP_CTRL            (u32)(CRL_APB_BASE_ADDR + 0xA4)
/** LPD switch control register address. */
#define LPD_SWITCH_CTRL      (u32)(CRL_APB_BASE_ADDR + 0xA8)
/** LPD LBUS control register address. */
#define LPD_LBUS_CTRL        (u32)(CRL_APB_BASE_ADDR + 0xAC)
/** DBG LPD control register address. */
#define DBG_LPD_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xB0)
/** NAND ref control register address. */
#define NAND_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0xB4)
/** LPD DMA ref control register address. */
#define LPDDMA_REF_CTRL      (u32)(CRL_APB_BASE_ADDR + 0xB8)
/** PL0 ref control register address. */
#define PL0_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xC0)
/** PL1 ref control register address. */
#define PL1_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xC4)
/** PL2 ref control register address. */
#define PL2_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xC8)
/** PL3 ref control register address. */
#define PL3_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xCC)
/** GEM TSU ref control register address. */
#define GEM_TSU_REF_CTRL     (u32)(CRL_APB_BASE_ADDR + 0x100)
/** DLL ref control register address. */
#define DLL_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0x104)
/** PS SYSMON ref control register address. */
#define PSSYSMON_REF_CTRL    (u32)(CRL_APB_BASE_ADDR + 0x108)
/** I2C0 ref control register address. */
#define I2C0_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x120)
/** I2C1 ref control register address. */
#define I2C1_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x124)
/** Timestamp ref control register address. */
#define TSTMP_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x128)

/* CRF APB register defines */
/** CRF APB base address. */
#define CRF_APB_BASE_ADDR    XPAR_PSU_CRF_APB_S_AXI_BASEADDR
/** APLL control register address. */
#define APLL_CTRL            (u32)(CRF_APB_BASE_ADDR + 0x20)
/** DPLL control register address. */
#define DPLL_CTRL            (u32)(CRF_APB_BASE_ADDR + 0x2C)
/** VPLL control register address. */
#define VPLL_CTRL            (u32)(CRF_APB_BASE_ADDR + 0x38)
/** APLL-to-LPD control register address. */
#define APLL_TO_LPD_CTRL     (u32)(CRF_APB_BASE_ADDR + 0x48)
/** PLL status register address. */
#define PLL_STATUS           (u32)(CRF_APB_BASE_ADDR + 0x44)
/** DPLL-to-LPD control register address. */
#define DPLL_TO_LPD_CTRL     (u32)(CRF_APB_BASE_ADDR + 0x4C)
/** VPLL-to-LPD control register address. */
#define VPLL_TO_LPD_CTRL     (u32)(CRF_APB_BASE_ADDR + 0x50)
/** ACPU control register address. */
#define ACPU_CTRL            (u32)(CRF_APB_BASE_ADDR + 0x60)
/** DBG trace control register address. */
#define DBG_TRACE_CTRL       (u32)(CRF_APB_BASE_ADDR + 0x64)
/** DBG FPD control register address. */
#define DBG_FPD_CTRL         (u32)(CRF_APB_BASE_ADDR + 0x68)
/** DP video ref control register address. */
#define DP_VIDEO_REF_CTRL    (u32)(CRF_APB_BASE_ADDR + 0x70)
/** DP audio ref control register address. */
#define DP_AUDIO_REF_CTRL    (u32)(CRF_APB_BASE_ADDR + 0x74)
/** DP STC ref control register address. */
#define DP_STC_REF_CTRL      (u32)(CRF_APB_BASE_ADDR + 0x7C)
/** DDR control register address. */
#define DDR_CTRL             (u32)(CRF_APB_BASE_ADDR + 0x80)
/** GPU ref control register address. */
#define GPU_REF_CTRL         (u32)(CRF_APB_BASE_ADDR + 0x84)
/** SATA ref control register address. */
#define SATA_REF_CTRL        (u32)(CRF_APB_BASE_ADDR + 0xA0)
/** PCIe ref control register address. */
#define PCIE_REF_CTRL        (u32)(CRF_APB_BASE_ADDR + 0xB4)
/** FPD DMA ref control register address. */
#define FPDDMA_REF_CTRL      (u32)(CRF_APB_BASE_ADDR + 0xB8)
/** DP DMA ref control register address. */
#define DPDMA_REF_CTRL       (u32)(CRF_APB_BASE_ADDR + 0xBC)
/** TOPSW main control register address. */
#define TOPSW_MAIN_CTRL      (u32)(CRF_APB_BASE_ADDR + 0xC0)
/** TOPSW LSBUS control register address. */
#define TOPSW_LSBUS_CTRL     (u32)(CRF_APB_BASE_ADDR + 0xC4)
/** GTGREF0 ref control register address. */
#define GTGREF0_REF_CTRL     (u32)(CRF_APB_BASE_ADDR + 0xC8)
/** DBG timestamp control register address. */
#define DBG_TSTMP_CTRL       (u32)(CRF_APB_BASE_ADDR + 0xF8)

/* IOU SLCR defines */
/** IOU SLCR base address. */
#define IOU_SLCR_BASE_ADDR   XPAR_PSU_IOUSLCR_0_S_AXI_BASEADDR
/** Watchdog-timer clock-select register address. */
#define WDT_CLK_SEL          (u32)(IOU_SLCR_BASE_ADDR + 0x300)
/** CAN clock control register address. */
#define CAN_CLK_CTRL         (u32)(IOU_SLCR_BASE_ADDR + 0x304)
/** GEM clock control register address. */
#define GEM_CLK_CTRL         (u32)(IOU_SLCR_BASE_ADDR + 0x308)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

/** @} */
