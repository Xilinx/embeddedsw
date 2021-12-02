/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xclockps_hw.h
* @addtogroup xclockps_v1_4
* @{
*
* This file contains the hardware details for the System Clock controller.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    02/09/18 First release
* </pre>
*
******************************************************************************/
#ifndef XCLOCK_HW_H		/* prevent circular inclusions */
#define XCLOCK_HW_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* CRL APB register defines */
#define CRL_APB_BASE_ADDR    XPAR_PSU_CRL_APB_S_AXI_BASEADDR
#define IOPLL_CTRL           (u32)(CRL_APB_BASE_ADDR + 0x20)
#define RPLL_CTRL            (u32)(CRL_APB_BASE_ADDR + 0x30)
#define IOPLL_TO_FPD_CTRL    (u32)(CRL_APB_BASE_ADDR + 0x44)
#define RPLL_TO_FPD_CTRL     (u32)(CRL_APB_BASE_ADDR + 0x48)
#define USB3_DUAL_REF_CTRL   (u32)(CRL_APB_BASE_ADDR + 0x4C)
#define GEM0_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x50)
#define GEM1_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x54)
#define GEM2_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x58)
#define GEM3_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x5C)
#define USB0_BUS_REF_CTRL    (u32)(CRL_APB_BASE_ADDR + 0x60)
#define USB1_BUS_REF_CTRL    (u32)(CRL_APB_BASE_ADDR + 0x64)
#define QSPI_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x68)
#define SDIO0_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x6C)
#define SDIO1_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x70)
#define UART0_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x74)
#define UART1_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x78)
#define SPI0_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x7C)
#define SPI1_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x80)
#define CAN0_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x84)
#define CAN1_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x88)
#define CPU_R5_CTRL          (u32)(CRL_APB_BASE_ADDR + 0x90)
#define IOU_SWITCH_CTRL      (u32)(CRL_APB_BASE_ADDR + 0x9C)
#define CSU_PLL_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xA0)
#define PCAP_CTRL            (u32)(CRL_APB_BASE_ADDR + 0xA4)
#define LPD_SWITCH_CTRL      (u32)(CRL_APB_BASE_ADDR + 0xA8)
#define LPD_LBUS_CTRL        (u32)(CRL_APB_BASE_ADDR + 0xAC)
#define DBG_LPD_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xB0)
#define NAND_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0xB4)
#define LPDDMA_REF_CTRL      (u32)(CRL_APB_BASE_ADDR + 0xB8)
#define PL0_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xC0)
#define PL1_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xC4)
#define PL2_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xC8)
#define PL3_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0xCC)
#define GEM_TSU_REF_CTRL     (u32)(CRL_APB_BASE_ADDR + 0x100)
#define DLL_REF_CTRL         (u32)(CRL_APB_BASE_ADDR + 0x104)
#define PSSYSMON_REF_CTRL    (u32)(CRL_APB_BASE_ADDR + 0x108)
#define I2C0_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x120)
#define I2C1_REF_CTRL        (u32)(CRL_APB_BASE_ADDR + 0x124)
#define TSTMP_REF_CTRL       (u32)(CRL_APB_BASE_ADDR + 0x128)

/* CRF APB register defines */
#define CRF_APB_BASE_ADDR    XPAR_PSU_CRF_APB_S_AXI_BASEADDR
#define APLL_CTRL            (u32)(CRF_APB_BASE_ADDR + 0x20)
#define DPLL_CTRL            (u32)(CRF_APB_BASE_ADDR + 0x2C)
#define VPLL_CTRL            (u32)(CRF_APB_BASE_ADDR + 0x38)
#define APLL_TO_LPD_CTRL     (u32)(CRF_APB_BASE_ADDR + 0x48)
#define PLL_STATUS           (u32)(CRF_APB_BASE_ADDR + 0x44)
#define DPLL_TO_LPD_CTRL     (u32)(CRF_APB_BASE_ADDR + 0x4C)
#define VPLL_TO_LPD_CTRL     (u32)(CRF_APB_BASE_ADDR + 0x50)
#define ACPU_CTRL            (u32)(CRF_APB_BASE_ADDR + 0x60)
#define DBG_TRACE_CTRL       (u32)(CRF_APB_BASE_ADDR + 0x64)
#define DBG_FPD_CTRL         (u32)(CRF_APB_BASE_ADDR + 0x68)
#define DP_VIDEO_REF_CTRL    (u32)(CRF_APB_BASE_ADDR + 0x70)
#define DP_AUDIO_REF_CTRL    (u32)(CRF_APB_BASE_ADDR + 0x74)
#define DP_STC_REF_CTRL      (u32)(CRF_APB_BASE_ADDR + 0x7C)
#define DDR_CTRL             (u32)(CRF_APB_BASE_ADDR + 0x80)
#define GPU_REF_CTRL         (u32)(CRF_APB_BASE_ADDR + 0x84)
#define SATA_REF_CTRL        (u32)(CRF_APB_BASE_ADDR + 0xA0)
#define PCIE_REF_CTRL        (u32)(CRF_APB_BASE_ADDR + 0xB4)
#define FPDDMA_REF_CTRL      (u32)(CRF_APB_BASE_ADDR + 0xB8)
#define DPDMA_REF_CTRL       (u32)(CRF_APB_BASE_ADDR + 0xBC)
#define TOPSW_MAIN_CTRL      (u32)(CRF_APB_BASE_ADDR + 0xC0)
#define TOPSW_LSBUS_CTRL     (u32)(CRF_APB_BASE_ADDR + 0xC4)
#define GTGREF0_REF_CTRL     (u32)(CRF_APB_BASE_ADDR + 0xC8)
#define DBG_TSTMP_CTRL       (u32)(CRF_APB_BASE_ADDR + 0xF8)

/* IOU SLCR defines */
#define IOU_SLCR_BASE_ADDR   XPAR_PSU_IOUSLCR_0_S_AXI_BASEADDR
#define WDT_CLK_SEL          (u32)(IOU_SLCR_BASE_ADDR + 0x300)
#define CAN_CLK_CTRL         (u32)(IOU_SLCR_BASE_ADDR + 0x304)
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
