/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xparameters_ps.h
*
* This file contains the address definitions for the hard peripherals
* attached to the ARM Cortex R5 core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ---------------------------------------------------
* 5.00  pkp  	02/29/14 Initial version
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

#ifndef XPARAMETERS_PS_H_
#define XPARAMETERS_PS_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/*
 * This block contains constant declarations for the peripherals
 * within the hardblock
 */

/* Canonical definitions for DDR MEMORY */
#define XPAR_DDR_MEM_BASEADDR		0x00000000U
#define XPAR_DDR_MEM_HIGHADDR		0x3FFFFFFFU

/* Canonical definitions for Interrupts  */
#define XPAR_XUARTPS_0_INTR		XPS_UART0_INT_ID
#define XPAR_XUARTPS_1_INTR		XPS_UART1_INT_ID
#define XPAR_XUSBPS_0_INTR		XPS_USB0_INT_ID
#define XPAR_XUSBPS_1_INTR		XPS_USB1_INT_ID
#define XPAR_XIICPS_0_INTR		XPS_I2C0_INT_ID
#define XPAR_XIICPS_1_INTR		XPS_I2C1_INT_ID
#define XPAR_XSPIPS_0_INTR		XPS_SPI0_INT_ID
#define XPAR_XSPIPS_1_INTR		XPS_SPI1_INT_ID
#define XPAR_XCANPS_0_INTR		XPS_CAN0_INT_ID
#define XPAR_XCANPS_1_INTR		XPS_CAN1_INT_ID
#define XPAR_XGPIOPS_0_INTR		XPS_GPIO_INT_ID
#define XPAR_XEMACPS_0_INTR		XPS_GEM0_INT_ID
#define XPAR_XEMACPS_0_WAKE_INTR	XPS_GEM0_WAKE_INT_ID
#define XPAR_XEMACPS_1_INTR		XPS_GEM1_INT_ID
#define XPAR_XEMACPS_1_WAKE_INTR	XPS_GEM1_WAKE_INT_ID
#define XPAR_XEMACPS_2_INTR		XPS_GEM2_INT_ID
#define XPAR_XEMACPS_2_WAKE_INTR	XPS_GEM2_WAKE_INT_ID
#define XPAR_XEMACPS_3_INTR		XPS_GEM3_INT_ID
#define XPAR_XEMACPS_3_WAKE_INTR	XPS_GEM3_WAKE_INT_ID
#define XPAR_XSDIOPS_0_INTR		XPS_SDIO0_INT_ID
#define XPAR_XQSPIPS_0_INTR		XPS_QSPI_INT_ID
#define XPAR_XSDIOPS_1_INTR		XPS_SDIO1_INT_ID
#define XPAR_XWDTPS_0_INTR		XPS_WDT_INT_ID
#define XPAR_XDCFG_0_INTR		XPS_DVC_INT_ID
#define XPAR_SCUTIMER_INTR		XPS_SCU_TMR_INT_ID
#define XPAR_SCUWDT_INTR		XPS_SCU_WDT_INT_ID
#define XPAR_XTTCPS_0_INTR		XPS_TTC0_0_INT_ID
#define XPAR_XTTCPS_1_INTR		XPS_TTC0_1_INT_ID
#define XPAR_XTTCPS_2_INTR		XPS_TTC0_2_INT_ID
#define XPAR_XTTCPS_3_INTR		XPS_TTC1_0_INT_ID
#define XPAR_XTTCPS_4_INTR		XPS_TTC1_1_INT_ID
#define XPAR_XTTCPS_5_INTR		XPS_TTC1_2_INT_ID
#define XPAR_XTTCPS_6_INTR		XPS_TTC2_0_INT_ID
#define XPAR_XTTCPS_7_INTR		XPS_TTC2_1_INT_ID
#define XPAR_XTTCPS_8_INTR		XPS_TTC2_2_INT_ID
#define XPAR_XTTCPS_9_INTR		XPS_TTC3_0_INT_ID
#define XPAR_XTTCPS_10_INTR		XPS_TTC3_1_INT_ID
#define XPAR_XTTCPS_11_INTR		XPS_TTC3_2_INT_ID
#define XPAR_XDMAPS_0_FAULT_INTR	XPS_DMA0_ABORT_INT_ID
#define XPAR_XDMAPS_0_DONE_INTR_0	XPS_DMA0_INT_ID
#define XPAR_XDMAPS_0_DONE_INTR_1	XPS_DMA1_INT_ID
#define XPAR_XDMAPS_0_DONE_INTR_2	XPS_DMA2_INT_ID
#define XPAR_XDMAPS_0_DONE_INTR_3	XPS_DMA3_INT_ID
#define XPAR_XDMAPS_0_DONE_INTR_4	XPS_DMA4_INT_ID
#define XPAR_XDMAPS_0_DONE_INTR_5	XPS_DMA5_INT_ID
#define XPAR_XDMAPS_0_DONE_INTR_6	XPS_DMA6_INT_ID
#define XPAR_XDMAPS_0_DONE_INTR_7	XPS_DMA7_INT_ID
#define XPAR_XNANDPS8_0_INTR        XPS_NAND_INT_ID


#define XPAR_XQSPIPS_0_LINEAR_BASEADDR	XPS_QSPI_LINEAR_BASEADDR
#define XPAR_XPARPORTPS_CTRL_BASEADDR	XPS_PARPORT_CRTL_BASEADDR



/* Canonical definitions for DMAC */


/* Canonical definitions for WDT */

/* Canonical definitions for SLCR */
#define XPAR_XSLCR_NUM_INSTANCES	1U
#define XPAR_XSLCR_0_DEVICE_ID		0U
#define XPAR_XSLCR_0_BASEADDR		XPS_SYS_CTRL_BASEADDR

/* Canonical definitions for SCU GIC */
#define XPAR_SCUGIC_NUM_INSTANCES	1U
#define XPAR_SCUGIC_SINGLE_DEVICE_ID	0U
#define XPAR_SCUGIC_CPU_BASEADDR	(XPS_SCU_PERIPH_BASE + 0x00001000U)
#define XPAR_SCUGIC_DIST_BASEADDR	(XPS_SCU_PERIPH_BASE + 0x00002000U)
#define XPAR_SCUGIC_ACK_BEFORE		0U

/* Canonical definitions for Global Timer */
#define XPAR_GLOBAL_TMR_NUM_INSTANCES	1U
#define XPAR_GLOBAL_TMR_DEVICE_ID	0U
#define XPAR_GLOBAL_TMR_BASEADDR	(XPS_SCU_PERIPH_BASE + 0x00000200U)
#define XPAR_GLOBAL_TMR_INTR		XPS_GLOBAL_TMR_INT_ID


/* Xilinx Parallel Flash Library (XilFlash) User Settings */
#define XPAR_AXI_EMC


#define XPAR_CPU_CORTEXR5_CORE_CLOCK_FREQ_HZ	XPAR_CPU_CORTEXR5_0_CPU_CLK_FREQ_HZ


/*
 * This block contains constant declarations for the peripherals
 * within the hardblock. These have been put for bacwards compatibilty
 */

#define XPS_PERIPHERAL_BASEADDR		0xE0000000U
#define XPS_UART0_BASEADDR		0xFF000000U
#define XPS_UART1_BASEADDR		0xFF010000U
#define XPS_I2C0_BASEADDR		0xFF020000U
#define XPS_I2C1_BASEADDR		0xFF030000U
#define XPS_SPI0_BASEADDR		0xFF040000U
#define XPS_SPI1_BASEADDR		0xFF050000U
#define XPS_CAN0_BASEADDR		0xFF060000U
#define XPS_CAN1_BASEADDR		0xFF070000U
#define XPS_GPIO_BASEADDR		0xFF0A0000U
#define XPS_GEM0_BASEADDR		0xFF0B0000U
#define XPS_GEM1_BASEADDR		0xFF0C0000U
#define XPS_GEM2_BASEADDR		0xFF0D0000U
#define XPS_GEM3_BASEADDR		0xFF0E0000U
#define XPS_QSPI_BASEADDR		0xFF0F0000U
#define XPS_NAND_BASEADDR		0xFF100000U
#define XPS_TTC0_BASEADDR		0xFF110000U
#define XPS_TTC1_BASEADDR		0xFF120000U
#define XPS_TTC2_BASEADDR		0xFF130000U
#define XPS_TTC3_BASEADDR		0xFF140000U
#define XPS_WDT_BASEADDR		0xFF150000U
#define XPS_SDIO0_BASEADDR		0xFF160000U
#define XPS_SDIO1_BASEADDR		0xFF170000U
#define XPS_SYS_CTRL_BASEADDR	0xFF180000U
/*#define XPAR_XNANDPS8_0_BASEADDR 0xFF100000U */


#define XPS_PARPORT_CRTL_BASEADDR 0x0000000U
#define XPS_IOU_BUS_CFG_BASEADDR	0xE0200000U
#define XPS_PARPORT0_BASEADDR		0xE2000000U
#define XPS_PARPORT1_BASEADDR		0xE4000000U
#define XPS_QSPI_LINEAR_BASEADDR	0xF0000000U
#define XPS_DMAC0_NON_SEC_BASEADDR	0xFE507000U
#define XPS_DMAC0_SEC_BASEADDR		0xFE5F0000U
#define XPS_DDR_CTRL_BASEADDR		0xF8006000U
#define XPS_DEV_CFG_APB_BASEADDR	0xF8007000U
#define XPS_AFI0_BASEADDR		0xF8008000U
#define XPS_AFI1_BASEADDR		0xF8009000U
#define XPS_AFI2_BASEADDR		0xF800A000U
#define XPS_AFI3_BASEADDR		0xF800B000U
#define XPS_OCM_BASEADDR		0xF800C000U
#define XPS_EFUSE_BASEADDR		0xF800D000U
#define XPS_CORESIGHT_BASEADDR		0xF8800000U
#define XPS_TOP_BUS_CFG_BASEADDR	0xF8900000U
#define XPS_SCU_PERIPH_BASE		0xF9000000U
#define XPS_L2CC_BASEADDR		0xFD3FD000U
#define XPS_SAM_RAM_BASEADDR		0xFFFC0000U
#define XPS_FPGA_AXI_S0_BASEADDR	0x40000000U
#define XPS_FPGA_AXI_S1_BASEADDR	0x80000000U
#define XPS_IOU_S_SWITCH_BASEADDR	0xE0000000U
#define XPS_PERIPH_APB_BASEADDR		0xF8000000U
#define XPS_USB0_BASEADDR		0xE0002000U
#define XPS_USB1_BASEADDR		0xE0003000U


/* Shared Peripheral Interrupts (SPI) */
/* Shared Peripheral Interrupts (SPI) */

#define XPS_USB1_INT_ID			76U
#define XPS_USB0_INT_ID			53U

#define XPS_NAND_INT_ID        (32U + 32U)

#define XPS_FPGA1_INT_ID		62U
#define XPS_FPGA2_INT_ID		63U
#define XPS_FPGA3_INT_ID		64U
#define XPS_FPGA4_INT_ID		65U
#define XPS_FPGA5_INT_ID		66U
#define XPS_FPGA6_INT_ID		67U
#define XPS_FPGA7_INT_ID		68U
#define XPS_DMA4_INT_ID			72U
#define XPS_DMA5_INT_ID			73U
#define XPS_DMA6_INT_ID			74U
#define XPS_DMA7_INT_ID			75U
#define XPS_FPGA8_INT_ID		84U
#define XPS_FPGA9_INT_ID		85U
#define XPS_FPGA10_INT_ID		86U
#define XPS_FPGA11_INT_ID		87U
#define XPS_FPGA12_INT_ID		88U
#define XPS_FPGA13_INT_ID		89U
#define XPS_FPGA14_INT_ID		90U
#define XPS_FPGA15_INT_ID		91U


#define XPS_OCMINTR_INT_ID		(28U + 32U)
#define XPS_QSPI_INT_ID			(33U + 32U)
#define XPS_GPIO_INT_ID			(34U + 32U)
#define XPS_WDT_INT_ID			(106U + 32U)
#define XPS_LP_WDT_INT_ID			(69U + 32U)
#define XPS_TTC0_0_INT_ID		(53U + 32U)
#define XPS_TTC0_1_INT_ID		(54U + 32U)
#define XPS_TTC0_2_INT_ID 		(55U + 32U)
#define XPS_SDIO0_INT_ID		(65U + 32U)
#define XPS_I2C0_INT_ID			(35U + 32U)
#define XPS_SPI0_INT_ID			(37U + 32U)
#define XPS_UART0_INT_ID		(39U + 32U)
#define XPS_CAN0_INT_ID			(41U + 32U)

/* FIXME */
//#define XPS_FPGA0_INT_ID		100

#define XPS_TTC1_0_INT_ID		(56U + 32U)
#define XPS_TTC1_1_INT_ID		(57U + 32U)
#define XPS_TTC1_2_INT_ID		(58U + 32U)
#define XPS_TTC2_0_INT_ID		(59U + 32U)
#define XPS_TTC2_1_INT_ID		(60U + 32U)
#define XPS_TTC2_2_INT_ID		(61U + 32U)
#define XPS_TTC3_0_INT_ID		(62U + 32U)
#define XPS_TTC3_1_INT_ID		(63U + 32U)
#define XPS_TTC3_2_INT_ID		(64U + 32U)
#define XPS_SDIO1_INT_ID		(66U + 32U)
#define XPS_I2C1_INT_ID			(36U + 32U)
#define XPS_SPI1_INT_ID			(38U + 32U)
#define XPS_UART1_INT_ID		(40U + 32U)
#define XPS_CAN1_INT_ID			(42U + 32U)
#define XPS_GEM0_INT_ID			(73U + 32U)
#define XPS_GEM0_WAKE_INT_ID		(74U + 32U)
#define XPS_GEM1_INT_ID			(75U + 32U)
#define XPS_GEM1_WAKE_INT_ID		(76U + 32U)
#define XPS_GEM2_INT_ID			(77U + 32U)
#define XPS_GEM2_WAKE_INT_ID		(78U + 32U)
#define XPS_GEM3_INT_ID			(79U + 32U)
#define XPS_GEM3_WAKE_INT_ID		(80U + 32U)

/* Private Peripheral Interrupts (PPI) */
/*#define XPS_GLOBAL_TMR_INT_ID		27	 SCU Global Timer interrupt */
/*#define XPS_FIQ_INT_ID			28	 FIQ from FPGA fabric */
/*#define XPS_SCU_TMR_INT_ID		29	 SCU Private Timer interrupt */
/*#define XPS_SCU_WDT_INT_ID		30	 SCU Private WDT interrupt */
/*#define XPS_IRQ_INT_ID			31	 IRQ from FPGA fabric */


/* REDEFINES for TEST APP */
/* Definitions for UART */
#define XPAR_PS7_UART_0_INTR		XPS_UART0_INT_ID
#define XPAR_PS7_UART_1_INTR		XPS_UART1_INT_ID
#define XPAR_PS7_USB_0_INTR		XPS_USB0_INT_ID
#define XPAR_PS7_USB_1_INTR		XPS_USB1_INT_ID
#define XPAR_PS7_I2C_0_INTR		XPS_I2C0_INT_ID
#define XPAR_PS7_I2C_1_INTR		XPS_I2C1_INT_ID
#define XPAR_PS7_SPI_0_INTR		XPS_SPI0_INT_ID
#define XPAR_PS7_SPI_1_INTR		XPS_SPI1_INT_ID
#define XPAR_PS7_CAN_0_INTR		XPS_CAN0_INT_ID
#define XPAR_PS7_CAN_1_INTR		XPS_CAN1_INT_ID
#define XPAR_PS7_GPIO_0_INTR		XPS_GPIO_INT_ID
#define XPAR_PS7_ETHERNET_0_INTR	XPS_GEM0_INT_ID
#define XPAR_PS7_ETHERNET_0_WAKE_INTR	XPS_GEM0_WAKE_INT_ID
#define XPAR_PS7_ETHERNET_1_INTR	XPS_GEM1_INT_ID
#define XPAR_PS7_ETHERNET_1_WAKE_INTR	XPS_GEM1_WAKE_INT_ID
#define XPAR_PS7_ETHERNET_2_INTR	XPS_GEM2_INT_ID
#define XPAR_PS7_ETHERNET_2_WAKE_INTR	XPS_GEM2_WAKE_INT_ID
#define XPAR_PS7_ETHERNET_3_INTR	XPS_GEM3_INT_ID
#define XPAR_PS7_ETHERNET_3_WAKE_INTR	XPS_GEM3_WAKE_INT_ID

#define XPAR_PS7_QSPI_0_INTR		XPS_QSPI_INT_ID
#define XPAR_PS7_WDT_0_INTR		XPS_WDT_INT_ID
#define XPAR_PS7_SCUWDT_0_INTR		XPS_SCU_WDT_INT_ID
#define XPAR_PS7_SCUTIMER_0_INTR	XPS_SCU_TMR_INT_ID
#define XPAR_PS7_XADC_0_INTR		XPS_SYSMON_INT_ID

#define XPAR_XADCPS_NUM_INSTANCES 1U
#define XPAR_XADCPS_0_DEVICE_ID   0U
#define XPAR_XADCPS_0_BASEADDR	  (0xF8007000U)
#define XPAR_XADCPS_INT_ID		XPS_SYSMON_INT_ID

/* For backwards compatibilty */
#define XPAR_XUARTPS_0_CLOCK_HZ		XPAR_XUARTPS_0_UART_CLK_FREQ_HZ
#define XPAR_XUARTPS_1_CLOCK_HZ		XPAR_XUARTPS_1_UART_CLK_FREQ_HZ
#define XPAR_XTTCPS_0_CLOCK_HZ		XPAR_XTTCPS_0_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_1_CLOCK_HZ		XPAR_XTTCPS_1_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_2_CLOCK_HZ		XPAR_XTTCPS_2_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_3_CLOCK_HZ		XPAR_XTTCPS_3_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_4_CLOCK_HZ		XPAR_XTTCPS_4_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_5_CLOCK_HZ		XPAR_XTTCPS_5_TTC_CLK_FREQ_HZ
#define XPAR_XIICPS_0_CLOCK_HZ		XPAR_XIICPS_0_I2C_CLK_FREQ_HZ
#define XPAR_XIICPS_1_CLOCK_HZ		XPAR_XIICPS_1_I2C_CLK_FREQ_HZ

#define XPAR_XQSPIPS_0_CLOCK_HZ		XPAR_XQSPIPS_0_QSPI_CLK_FREQ_HZ

#ifdef XPAR_CPU_CORTEXR5_0_CPU_CLK_FREQ_HZ
#define XPAR_CPU_CORTEXR5_CORE_CLOCK_FREQ_HZ	XPAR_CPU_CORTEXR5_0_CPU_CLK_FREQ_HZ
#endif

#ifdef XPAR_CPU_CORTEXR5_1_CPU_CLK_FREQ_HZ
#define XPAR_CPU_CORTEXR5_CORE_CLOCK_FREQ_HZ	XPAR_CPU_CORTEXR5_1_CPU_CLK_FREQ_HZ
#endif

#define XPAR_SCUTIMER_DEVICE_ID		0U
#define XPAR_SCUWDT_DEVICE_ID		0U


#ifdef __cplusplus
}
#endif

#endif /* protection macro */
