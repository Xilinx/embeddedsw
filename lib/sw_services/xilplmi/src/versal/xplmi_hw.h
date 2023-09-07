/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xplmi_hw.h
*
* This is the header file which contains definitions for the versal hardware
* registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
* 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
*       bsv  04/18/2019 Added support for NPI readback and CFI readback
*       kc   04/27/2019 Added SPP check for UART frequency
*       kc   05/21/2019 Updated IPI error code to response buffer
*       kc   07/16/2019 Added logic to determine the IRO frequency
*       bsv  08/29/2019 Added Multiboot and Fallback support in PLM
*       scs  08/31/2019 Added support for Extended IDCODE checks
* 1.02  ma   02/05/2020 Removed SRST error action for PSM errors
*       ma   02/18/2020 Added event logging code
*       ma   02/28/2020 Code related to handling PSM errors from PLM
*       ma   03/02/2020 Added support for logging trace events
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  07/10/2020 Added PMC_IOU_SLCR register related macros
*       kc   07/28/2020 Added PMC PS GPIO related macros
*       kc   08/04/2020 Added CRP NPLL related macros
*       bm   08/19/2020 Added ImageInfo Table related macros
*       bm   09/08/2020 Added PMC RAM Usage for RunTime Configuration registers
*       bsv  09/21/2020 Set clock source to IRO before SRST for ES1 silicon
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP
*                       and PCIE boot modes
*       td   10/30/2020 Fixed MISRA C Rule 2.5 and added PMC_GLOBAL macros
* 1.04  bm   11/10/2020 Added ROM_VALIDATION_DIGEST macro
*       bsv  01/29/2021 Added APIs for checking and clearing NPI errors
* 1.05  pj   03/24/2021 Added Macros for PSM_CR MASK and trigger
*       skd  03/25/2021 Macros re-definitions compilation warning fixes
*       ma   05/03/2021 Added macros for FW_CR and FW_ERR NCR_FLAG masks and
*                       removed PSM_CR mask macro which is unused
*       gm   05/11/2021 Added macros for DDRMC UB related registers
*       bsv  05/15/2021 Remove warning for AXI_WRSTRB NPI error
*       ma   05/17/2021 Added macro related to FW_ERR register
* 1.06  bsv  06/09/2021 Add warning in case IPI-0 interrupt is disabled
*       bsv  06/17/2021 Update warning in case some IPIs are disabled
*       ma   06/23/2021 Added readback support for SSIT Slave SLRs
*       ma   06/28/2021 Added macros related to CPM registers
*       td   07/08/2021 Fix doxygen warnings
*       bm   07/12/2021 Removed obsolete EFUSE_CACHE defines and added sysmon
*                       related defines
*       bsv  07/16/2021 Fix doxygen and compilation warnings
*       kc   07/22/2021 Added VP1802 idcode and external POR macros
*       rb   07/28/2021 Added Efuse DNA and VP1502 idcode macros
*       rb   07/29/2021 Added macros for persistent general storage register
*                       and reset reason masks
*       rb   08/11/2021 Fix compilation warnings
*       ma   08/30/2021 Added SSIT related define
*       gm   09/17/2021 Added MJTAG workaround related define
* 1.07  skd  11/23/2021 Fix compilation warnings
*       ma   01/17/2022 Add HW registers related to SLVERR
*       ma   01/24/2022 Add missing EAM registers
*       ma   01/31/2022 Fix DMA Keyhole command issue where the command
*                       starts at the 32K boundary
*       skd  03/09/2022 Compilation warning fix
*       is   03/22/2022 Combined old and new XPPU/XMPU macros
* 1.08  bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/08/2022 Add support for storing procs to PMC RAM based on ID
*       ma   07/08/2022 Added support for secure lockdown
*       ma   07/13/2022 Added RTC_CONTROL_SLVERR_EN_MASK macro
*       ma   07/20/2022 Move PMC_PSM_ERR_REG_OFFSET to xplmi_error_common.h
*       bm   07/20/2022 Added compatibility check for In-Place PLM Update
*       ma   07/25/2022 Enhancements to secure lockdown code
* 1.09  ng   11/11/2022 Fixed doxygen file name error
*       bm   01/03/2023 Handle SSIT Events from PPU1 IRQ directly
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
* 1.10  bm   05/22/2023 Update current CDO command offset in GSW Error Status
*       nb   06/28/2023 Add ERR2 trigger and status macros for SMON9
*       bm   07/06/2023 Refactored Proc logic to more generic logic
*       am   07/11/2023 Reduced the trace event buffer length to accomodate
*                       IHT OP data store address
*       ng   07/13/2023 Added support for system device-tree flow
*       bm   09/07/2023 Allow loading of ELFs into XRAM
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_HW_H
#define XPLMI_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xil_hw.h"
#include "xplmi_util.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*
 * PMC_GLOBAL Base Address
 */
#ifndef PMC_GLOBAL_BASEADDR
#define PMC_GLOBAL_BASEADDR     (0XF1110000U)
#endif

/*
 * Register: PMC_GLOBAL_GLOBAL_CNTRL
 */
#define PMC_GLOBAL_GLOBAL_CNTRL    (PMC_GLOBAL_BASEADDR + 0X00000000U)
#define PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK   (0X00000010U)
#define PMC_GLOBAL_GLOBAL_CNTRL_SLVERR_ENABLE_MASK		(0X00000002U)

/*
 * Register: PMC_GLOBAL_PMC_MULTI_BOOT
 */
#define PMC_GLOBAL_PMC_MULTI_BOOT    (PMC_GLOBAL_BASEADDR + 0X00000004U)

/*
 * Register: PMC_GLOBAL_GLOBAL_GEN_STORAGE0
 */
#define PMC_GLOBAL_GLOBAL_GEN_STORAGE0    (PMC_GLOBAL_BASEADDR + 0X00000030U)

/*
 * Register: PMC_GLOBAL_GLOBAL_GEN_STORAGE1
 */
#define PMC_GLOBAL_GLOBAL_GEN_STORAGE1    (PMC_GLOBAL_BASEADDR + 0X00000034U)

/*
 * Register: PMC_GLOBAL_GLOBAL_GEN_STORAGE2
 */
#define PMC_GLOBAL_GLOBAL_GEN_STORAGE2    (PMC_GLOBAL_BASEADDR + 0X00000038U)

/*
 * Register: PMC_GLOBAL_GLOBAL_GEN_STORAGE4
 */
#define PMC_GLOBAL_GLOBAL_GEN_STORAGE4    (PMC_GLOBAL_BASEADDR + 0X00000040U)

/*
 * Register: PMC_GLOBAL_PERS_GLOB_GEN_STORAGE1
 */
#define PMC_GLOBAL_PERS_GLOB_GEN_STORAGE1	(PMC_GLOBAL_BASEADDR + 0X00000054U)

/* Defines for the bit in PGGS1 register which indicates whether to Log CDO offset */
#define PMC_GLOBAL_LOG_CDO_OFFSET_SHIFT		(0x3U)
#define PMC_GLOBAL_LOG_CDO_OFFSET_MASK		XPLMI_BIT(PMC_GLOBAL_LOG_CDO_OFFSET_SHIFT)

/*
 * Register: PMC_GLOBAL_PERS_GEN_STORAGE2
 */
#define PMC_GLOBAL_PERS_GEN_STORAGE2	(PMC_GLOBAL_BASEADDR + 0X00000058U)
#define PERS_GEN_STORAGE2_ACC_RR_MASK	(0xFFFF0000U)

/*
 * Register: PMC_GLOBAL_PMC_GSW_ERR
 */
#define PMC_GLOBAL_PMC_GSW_ERR    (PMC_GLOBAL_BASEADDR + 0X00000064U)

/*
 * Register: PMC_GLOBAL_PWR_STATUS
 */
#define PMC_GLOBAL_PWR_STATUS    (PMC_GLOBAL_BASEADDR + 0X00000100U)

/*
 * Register: PMC_GLOBAL_PMC_SSS_CFG
 */
#define PMC_GLOBAL_PMC_SSS_CFG    (PMC_GLOBAL_BASEADDR + 0X00000500U)

/*
 * Register: PMC_GLOBAL_PRAM_ZEROIZE_SIZE
 */
#define PMC_GLOBAL_PRAM_ZEROIZE_SIZE    (PMC_GLOBAL_BASEADDR + 0X00000518U)

/*
 * Register: PMC_GLOBAL_PPU_1_RST_MODE
 */
#define PMC_GLOBAL_PPU_1_RST_MODE    (PMC_GLOBAL_BASEADDR + 0X00000624U)

#define PMC_GLOBAL_PPU_1_RST_MODE_WAKEUP_MASK   (0X00000010U)

/*
 * Register: PMC_GLOBAL_DONE
 */
#define PMC_GLOBAL_DONE    (PMC_GLOBAL_BASEADDR + 0X00000884U)

/*
 * Register: PMC_GLOBAL_SSIT_ERR
 */
#define PMC_GLOBAL_SSIT_ERR    (PMC_GLOBAL_BASEADDR + 0X00000958U)

/*
 * Register: PMC_GLOBAL_PMC_FW_ERR
 */
#define PMC_GLOBAL_PMC_FW_ERR    (PMC_GLOBAL_BASEADDR + 0X00010100U)
#define PMC_GLOBAL_PMC_FW_ERR_NCR_FLAG_MASK		(0x80000000U)
#define PMC_GLOBAL_PMC_FW_ERR_DATA_MASK			(0x3FFFFFFFU)

/*
 * Register: PMC_GLOBAL_PMC_ERR1_STATUS
 */
#define PMC_GLOBAL_PMC_ERR1_STATUS    (PMC_GLOBAL_BASEADDR + 0X00020000U)
#define PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK   (0X00000080U)
#define PMC_GLOBAL_PMC_ERR1_STATUS_CFU_MASK   (0X00000040U)
#define PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERRX_MASK   (0xE0000000U)

/*
 * Register: PMC_GLOBAL_PMC_ERR2_STATUS
 */
#define PMC_GLOBAL_PMC_ERR2_STATUS    (PMC_GLOBAL_BASEADDR + 0X00020004U)
#define PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK   (0X80000000U)
#define PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK   (0X40000000U)
#define PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_SHIFT  (29U)
#define PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK   (0X20000000U)
#define PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERRX_MASK   (0xE0000000U)
#define PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK   (0X00020000U)
#define PMC_GLOBAL_PMC_ERR2_STATUS_PMC_SMON9_MASK   (0X00010000U)

/*
 * Register: PMC_GLOBAL_PMC_ERR1_TRIG
 */
#define PMC_GLOBAL_PMC_ERR1_TRIG	(PMC_GLOBAL_BASEADDR + 0X00020010U)
#define PMC_GLOBAL_PMC_ERR1_TRIG_FW_CR_MASK			(0x00000004U)

/*
 * Register: PMC_GLOBAL_PMC_ERR2_TRIG
 */
#define PMC_GLOBAL_PMC_ERR2_TRIG	(PMC_GLOBAL_BASEADDR + 0X00020014U)

/*
 * Register: PMC_GLOBAL_PMC_ERR_OUT1_MASK
 */
#define PMC_GLOBAL_PMC_ERR_OUT1_MASK    (PMC_GLOBAL_BASEADDR + 0X00020020U)

/*
 * Register: PMC_GLOBAL_PMC_ERR_OUT1_EN
 */
#define PMC_GLOBAL_PMC_ERR_OUT1_EN    (PMC_GLOBAL_BASEADDR + 0X00020024U)

/*
 * Register: PMC_GLOBAL_PMC_ERR_OUT1_DIS
 */
#define PMC_GLOBAL_PMC_ERR_OUT1_DIS    (PMC_GLOBAL_BASEADDR + 0X00020028U)

/*
 * Register: PMC_GLOBAL_PMC_ERR_OUT2_MASK
 */
#define PMC_GLOBAL_PMC_ERR_OUT2_MASK    (PMC_GLOBAL_BASEADDR + 0X00020030U)

/*
 * Register: PMC_GLOBAL_PMC_ERR_OUT2_EN
 */
#define PMC_GLOBAL_PMC_ERR_OUT2_EN    (PMC_GLOBAL_BASEADDR + 0X00020034U)

/*
 * Register: PMC_GLOBAL_PMC_ERR_OUT2_DIS
 */
#define PMC_GLOBAL_PMC_ERR_OUT2_DIS    (PMC_GLOBAL_BASEADDR + 0X00020038U)

/*
 * Register: PMC_GLOBAL_PMC_POR1_MASK
 */
#define PMC_GLOBAL_PMC_POR1_MASK    (PMC_GLOBAL_BASEADDR + 0X00020040U)

/*
 * Register: PMC_GLOBAL_PMC_POR1_EN
 */
#define PMC_GLOBAL_PMC_POR1_EN    (PMC_GLOBAL_BASEADDR + 0X00020044U)

/*
 * Register: PMC_GLOBAL_PMC_POR1_DIS
 */
#define PMC_GLOBAL_PMC_POR1_DIS    (PMC_GLOBAL_BASEADDR + 0X00020048U)

/*
 * Register: PMC_GLOBAL_PMC_POR2_MASK
 */
#define PMC_GLOBAL_PMC_POR2_MASK    (PMC_GLOBAL_BASEADDR + 0X00020050U)

/*
 * Register: PMC_GLOBAL_PMC_POR2_EN
 */
#define PMC_GLOBAL_PMC_POR2_EN    (PMC_GLOBAL_BASEADDR + 0X00020054U)

/*
 * Register: PMC_GLOBAL_PMC_POR2_DIS
 */
#define PMC_GLOBAL_PMC_POR2_DIS    (PMC_GLOBAL_BASEADDR + 0X00020058U)

/*
 * Register: PMC_GLOBAL_PMC_IRQ1_MASK
 */
#define PMC_GLOBAL_PMC_IRQ1_MASK    (PMC_GLOBAL_BASEADDR + 0X00020060U)

/*
 * Register: PMC_GLOBAL_PMC_IRQ1_EN
 */
#define PMC_GLOBAL_PMC_IRQ1_EN    (PMC_GLOBAL_BASEADDR + 0X00020064U)

/*
 * Register: PMC_GLOBAL_PMC_IRQ1_DIS
 */
#define PMC_GLOBAL_PMC_IRQ1_DIS    (PMC_GLOBAL_BASEADDR + 0X00020068U)

/*
 * Register: PMC_GLOBAL_PMC_IRQ2_MASK
 */
#define PMC_GLOBAL_PMC_IRQ2_MASK    (PMC_GLOBAL_BASEADDR + 0X00020070U)

/*
 * Register: PMC_GLOBAL_PMC_IRQ2_EN
 */
#define PMC_GLOBAL_PMC_IRQ2_EN    (PMC_GLOBAL_BASEADDR + 0X00020074U)

/*
 * Register: PMC_GLOBAL_PMC_IRQ2_DIS
 */
#define PMC_GLOBAL_PMC_IRQ2_DIS    (PMC_GLOBAL_BASEADDR + 0X00020078U)

/*
 * Register: PMC_GLOBAL_PMC_SRST1_MASK
 */
#define PMC_GLOBAL_PMC_SRST1_MASK    (PMC_GLOBAL_BASEADDR + 0X00020080U)

/*
 * Register: PMC_GLOBAL_PMC_SRST1_EN
 */
#define PMC_GLOBAL_PMC_SRST1_EN    (PMC_GLOBAL_BASEADDR + 0X00020084U)

/*
 * Register: PMC_GLOBAL_PMC_SRST1_DIS
 */
#define PMC_GLOBAL_PMC_SRST1_DIS    (PMC_GLOBAL_BASEADDR + 0X00020088U)

/*
 * Register: PMC_GLOBAL_PMC_SRST2_MASK
 */
#define PMC_GLOBAL_PMC_SRST2_MASK    (PMC_GLOBAL_BASEADDR + 0X00020090U)

/*
 * Register: PMC_GLOBAL_PMC_SRST2_EN
 */
#define PMC_GLOBAL_PMC_SRST2_EN    (PMC_GLOBAL_BASEADDR + 0X00020094U)

/*
 * Register: PMC_GLOBAL_PMC_SRST2_DIS
 */
#define PMC_GLOBAL_PMC_SRST2_DIS    (PMC_GLOBAL_BASEADDR + 0X00020098U)

/*
 * Register: PMC_GLOBAL_GICP0_IRQ_STATUS
 */
#define PMC_GLOBAL_GICP0_IRQ_STATUS    (PMC_GLOBAL_BASEADDR + 0X00030000U)

/*
 * Register: PMC_GLOBAL_GICP0_IRQ_MASK
 */
#define PMC_GLOBAL_GICP0_IRQ_MASK    (PMC_GLOBAL_BASEADDR + 0X00030004U)

/*
 * Register: PMC_GLOBAL_GICP0_IRQ_ENABLE
 */
#define PMC_GLOBAL_GICP0_IRQ_ENABLE    (PMC_GLOBAL_BASEADDR + 0X00030008U)

/*
 * Register: PMC_GLOBAL_GICP0_IRQ_DISABLE
 */
#define PMC_GLOBAL_GICP0_IRQ_DISABLE    (PMC_GLOBAL_BASEADDR + 0X0003000CU)

/*
 * Register: PMC_GLOBAL_GICP1_IRQ_STATUS
 */
#define PMC_GLOBAL_GICP1_IRQ_STATUS    (PMC_GLOBAL_BASEADDR + 0X00030014U)

/*
 * Register: PMC_GLOBAL_GICP2_IRQ_STATUS
 */
#define PMC_GLOBAL_GICP2_IRQ_STATUS    (PMC_GLOBAL_BASEADDR + 0X00030028U)

/*
 * Register: PMC_GLOBAL_GICP3_IRQ_STATUS
 */
#define PMC_GLOBAL_GICP3_IRQ_STATUS    (PMC_GLOBAL_BASEADDR + 0X0003003CU)

/*
 * Register: PMC_GLOBAL_GICP4_IRQ_STATUS
 */
#define PMC_GLOBAL_GICP4_IRQ_STATUS    (PMC_GLOBAL_BASEADDR + 0X00030050U)

/*
 * Register: PMC_GLOBAL_GICP_PMC_IRQ_STATUS
 */
#define PMC_GLOBAL_GICP_PMC_IRQ_STATUS    (PMC_GLOBAL_BASEADDR + 0X000300A0U)

/*
 * Register: PMC_GLOBAL_GICP_PMC_IRQ_ENABLE
 */
#define PMC_GLOBAL_GICP_PMC_IRQ_ENABLE    (PMC_GLOBAL_BASEADDR + 0X000300A8U)

#define PMC_PSM_EN_REG_OFFSET			(0x4U)
#define PMC_PSM_DIS_REG_OFFSET			(0x8U)

/*
 * Register: PMC_GLOBAL_IER
 */
#define PMC_GLOBAL_IER				(PMC_GLOBAL_BASEADDR + 0X00000018U)

/*
 * Register: PMC_GLOBAL_ISR
 */
#define PMC_GLOBAL_ISR				(PMC_GLOBAL_BASEADDR + 0X00000010U)

/*
 * TAMPER_INT mask value
 */
#define PMC_GLOBAL_TAMPER_INT_MASK	(0x00000008U)

/*
 * Register: PMC_GLOBAL_TAMPER_RESP_0
 */
#define PMC_GLOBAL_TAMPER_RESP_0	(PMC_GLOBAL_BASEADDR + 0X00000530U)

/*
 * Register: PMC_GLOBAL_TAMPER_TRIG
 */
#define PMC_GLOBAL_TAMPER_TRIG		(PMC_GLOBAL_BASEADDR + 0X00000570U)
#define PMC_GLOBAL_TAMPER_TRIG_VAL	(1U)

/*
 * Register: NPI_NIR
 */
#define NPI_NIR_BASEADDR	(0XF6000000U)
#define NPI_NIR_REG_PCSR_LOCK	(NPI_NIR_BASEADDR + 0XCU)
#define NPI_NIR_REG_PCSR_UNLOCK_VAL	(0XF9E8D7C6U)
#define NPI_NIR_REG_ISR		(NPI_NIR_BASEADDR + 0X44U)
#define NPI_NIR_REG_ISR_ERR_MASK	(0X7FU)
#define NPI_NIR_ERR_TYPE	(NPI_NIR_BASEADDR + 0X204U)
#define NPI_NIR_ERR_TYPE_ERR_MASK	(0X7U)
#define NPI_NIR_ERR_LOG_P0_INFO_0	(NPI_NIR_BASEADDR + 0X208U)
#define NPI_NIR_ERR_LOG_P0_INFO_1	(NPI_NIR_BASEADDR + 0X20CU)

/*
 * Register: PS7_IPI_PMC_IMR
 */
#define PS7_IPI_PMC_IMR		(0xFF320014U)

/* IPI Aperture TZ register base address */
#define IPI_APER_TZ_000_ADDR			(0xFF3000BCU)
#define IPI_APER_TZ_PMC_REQ_BUF_MASK	(0x4U)

/*
 * Register: CPM5_SLCR_PS_UNCORR_IR_STATUS
 */
#define CPM5_SLCR_PS_UNCORR_IR_STATUS					(0xFCDD0320U)
#define CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE0_MASK		(0x2U)
#define CPM5_SLCR_PS_UNCORR_IR_STATUS_PCIE1_MASK		(0x4U)

#define CPM5_SLCR_PS_UNCORR_IR_MASK						(0xFCDD0324U)

#define CPM5_SLCR_PCIE0_IR_STATUS 						(0xFCDD02A0U)
#define CPM5_SLCR_PCIE1_IR_STATUS 						(0xFCDD02B4U)
#define CPM5_SLCR_PCIE_IR_STATUS_PCIE_LOCAL_ERR_MASK	(0x1U)
/*
 * Register: CPM5_DMAn_CSR_INT_DEC
 */
#define CPM5_DMA0_CSR_INT_DEC							(0xFCE20E10U)
#define CPM5_DMA1_CSR_INT_DEC							(0xFCEA0E10U)
#define CPM5_DMA_CSR_LINK_DOWN_MASK						(0x1U)

/*****************************************************************************/
/**
 * @brief        This function reads a 32-bit register
 *
 * @param        Addr is the address of the register
 *
 * @return       32-bit register value
 *
 ******************************************************************************/
static inline u32 XPlmi_In32(UINTPTR Addr)
{
	return Xil_In32(Addr);
}

/*****************************************************************************/
/**
 * @brief	This function writes 32-bit value to 32-bit register
 *
 * @param	Addr is the address of the register
 * @param	Value is the value to store in register
 *
 * @return	None
 *
******************************************************************************/
static inline void XPlmi_Out32(UINTPTR Addr, u32 Value)
{
	Xil_Out32(Addr, Value);
}

/*****************************************************************************/
/**
 * @brief        This function reads a 64-bit register
 *
 * @param        Addr is the address of the register
 *
 * @return       32-bit value from 64-bit register
 *
 ******************************************************************************/
static inline u32 XPlmi_In64(u64 Addr)
{
	return lwea(Addr);
}

/*****************************************************************************/
/**
 * @brief        This function reads an 8-bit value from a 64-bit register
 *
 * @param        Addr is the address of the register
 *
 * @return       8-bit value from 64-bit register
 *
 ******************************************************************************/
static inline u8 XPlmi_InByte64(u64 Addr)
{
	return (u8)lbuea(Addr);
}

/*****************************************************************************/
/**
 * @brief       This function disables waking up of PPU1 processor
 *
 * @return      None
 *
 *****************************************************************************/
static inline void XPlmi_PpuWakeUpDis(void)
{
	XPlmi_Out32(PMC_GLOBAL_PPU_1_RST_MODE,
		XPlmi_In32(PMC_GLOBAL_PPU_1_RST_MODE) &
		(~PMC_GLOBAL_PPU_1_RST_MODE_WAKEUP_MASK));
}

/*****************************************************************************/
/**
 * @brief	This function writes 32-bit value to 64-bit register
 *
 * @param	Addr is the address of the register
 * @param	Data is the value to store in register
 *
 * @return	None
 *
******************************************************************************/
static inline void XPlmi_Out64(u64 Addr, u32 Data)
{
	swea(Addr, Data);
}

/*****************************************************************************/
/**
 * @brief	This function writes 8-bit value to 64-bit register
 *
 * @param	Addr is the address of the register
 * @param	Data is the value to store in register
 *
 * @return	None
 *
******************************************************************************/
static inline void XPlmi_OutByte64(u64 Addr, u8 Data)
{
	sbea(Addr, Data);
}

/*
 * Register: PMC Global PLM Error
 */
#define PMC_GLOBAL_PLM_ERR		(PMC_GLOBAL_BASEADDR + 0X00010100U)

/*
 * Definitions required from pmc_tap.h
 */
#ifndef PMC_TAP_BASEADDR
#define PMC_TAP_BASEADDR		(0XF11A0000U)
#endif
#ifndef PMC_TAP_IDCODE
#define PMC_TAP_IDCODE		(PMC_TAP_BASEADDR + 0X00000000U)
#endif
#define PMC_TAP_IDCODE_SI_REV_MASK	(0xF0000000U)
#define PMC_TAP_IDCODE_SBFMLY_MASK	(0x001C0000U)
#define PMC_TAP_IDCODE_DEV_MASK		(0x0003F000U)
#define PMC_TAP_IDCODE_SIREV_DVCD_MASK	(PMC_TAP_IDCODE_SI_REV_MASK | \
		PMC_TAP_IDCODE_SBFMLY_MASK | PMC_TAP_IDCODE_DEV_MASK)

#define PMC_TAP_IDCODE_SI_REV_1		(0x00000000U)
#define PMC_TAP_IDCODE_SBFMLY_S		(0x00080000U)
#define PMC_TAP_IDCODE_SBFMLY_H		(0x00100000U)
#define PMC_TAP_IDCODE_DEV_80		(0x00028000U)
#define PMC_TAP_IDCODE_DEV_14		(0x00014000U)
#define PMC_TAP_IDCODE_DEV_8		(0x00008000U)
#define PMC_TAP_IDCODE_ES1_VC1902	(PMC_TAP_IDCODE_SI_REV_1 | \
	PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_80)
#define PMC_TAP_IDCODE_ES1_VP1802	(PMC_TAP_IDCODE_SI_REV_1 | \
	PMC_TAP_IDCODE_SBFMLY_H | PMC_TAP_IDCODE_DEV_14)
#define PMC_TAP_IDCODE_ES1_VP1502	(PMC_TAP_IDCODE_SI_REV_1 | \
	PMC_TAP_IDCODE_SBFMLY_H | PMC_TAP_IDCODE_DEV_8)

#ifndef PMC_TAP_VERSION
#define PMC_TAP_VERSION		(PMC_TAP_BASEADDR + 0X00000004U)
#endif
#define PMC_TAP_VERSION_PLATFORM_SHIFT		(24U)
#define PMC_TAP_VERSION_PS_VERSION_SHIFT		(8U)
#define PMC_TAP_VERSION_PS_VERSION_MASK		(0X0000FF00U)
#define PMC_TAP_VERSION_PMC_VERSION_SHIFT		(0U)
#ifndef PMC_TAP_VERSION_PMC_VERSION_MASK
#define PMC_TAP_VERSION_PMC_VERSION_MASK		(0X000000FFU)
#endif

#define PMC_TAP_VERSION_SILICON			(0x0U)
#define PMC_TAP_VERSION_SPP			(0x1U)
#define PMC_TAP_VERSION_QEMU			(0x3U)
#define XPLMI_PLATFORM_MASK			(0x07000000U)
#define XPLMI_PLATFORM		((XPlmi_In32(PMC_TAP_VERSION) & \
					XPLMI_PLATFORM_MASK) >> \
					PMC_TAP_VERSION_PLATFORM_SHIFT)

#define PMC_TAP_SLVERR_CTRL		(PMC_TAP_BASEADDR + 0X0000001CU)
#define XPLMI_SILICON_ES1_VAL	(0x10U)

/*
 * PMC RAM Memory usage:
 * 0xF2000000U to 0xF201011FU - Used by XilLoader to process CDO
 * 0xF2014000U to 0xF2014FFFU - Used for PLM Runtime Configuration Registers
 * 0xF2015000U to 0xF2015FFFU - Used for SSIT PLM to PLM communication
 * 0xF2016000U to 0xF2016BFFU - Used for storing secure lockdown CDO proc data
 * 0xF2019000U to 0xF201CFFFU - Used by XilPlmi to store PLM prints
 * 0xF201D000U to 0xF201DFFFU - Used by XilPlmi to store PLM Trace Events
 * 0xF201E000U to 0xF2020000U - Used by XilPdi to get boot Header copied by ROM
 */
#define XPLMI_PMCRAM_BASEADDR			(0xF2000000U)
#define XPLMI_PMCRAM_LEN			(0x20000U)

/* Loader chunk memory */
#define XPLMI_PMCRAM_CHUNK_MEMORY		(XPLMI_PMCRAM_BASEADDR + 0x20U)
#define XPLMI_PMCRAM_CHUNK_MEMORY_1		(XPLMI_PMCRAM_BASEADDR + 0x8120U)

/* Log Buffer default address and length */
#define XPLMI_DEBUG_LOG_BUFFER_ADDR	(XPLMI_PMCRAM_BASEADDR + 0x19000U)
#define XPLMI_DEBUG_LOG_BUFFER_LEN	(0x4000U) /* 16KB */

/* Trace Buffer default address and length */
#define XPLMI_TRACE_LOG_BUFFER_ADDR	(XPLMI_PMCRAM_BASEADDR + 0x1D000U)
#define XPLMI_TRACE_LOG_BUFFER_LEN	(0x200U)	/* 512B */

/* Image Info Table related macros */
#define XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR	(XPLMI_PMCRAM_BASEADDR + 0x1DD00U)
#define XPLMI_IMAGE_INFO_TBL_BUFFER_LEN		(0x300U)	/* 768B */

/* PMC RAM secure lockdown reserved memory macros */
#define XPLMI_PMCRAM_BUFFER_MEMORY			(XPLMI_PMCRAM_BASEADDR + 0x16000U)
#define XPLMI_PMCRAM_BUFFER_MEMORY_LENGTH		(0xC00U)

/*
 * Definitions required from Efuse
 */
#define EFUSE_CACHE_BASEADDR		(0xF1250000U)
#define EFUSE_CTRL_BASEADDR		(0xF1240000U)
#define EFUSE_CTRL_WR_LOCK		(EFUSE_CTRL_BASEADDR + 0x0U)
#define XPLMI_EFUSE_CTRL_UNLOCK_VAL	(0xDF0DU)
#define XPLMI_EFUSE_CTRL_LOCK_VAL	(1U)

#define EFUSE_CTRL_CFG			(EFUSE_CTRL_BASEADDR + 0x4U)
#define EFUSE_CTRL_CFG_SLVERR_ENABLE_MASK	(0x20U)
#define EFUSE_CTRL_ANLG_OSC_SW_1LP	(EFUSE_CTRL_BASEADDR + 0x60U)

#define EFUSE_CACHE_MISC_CTRL	(EFUSE_CACHE_BASEADDR + 0xA0U)
#define EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_MASK	(0x600000U)

#define XPLMI_IPI_BASEADDR		(0xFF320000U)

#ifndef SDT
	#if defined(XPAR_XIPIPSU_0_DEVICE_ID) && (XPAR_XIPIPSU_0_BASE_ADDRESS == XPLMI_IPI_BASEADDR)
		#define XPLMI_IPI_DEVICE_ID		XPAR_XIPIPSU_0_DEVICE_ID
	#elif defined(XPAR_XIPIPSU_1_DEVICE_ID) && (XPAR_XIPIPSU_1_BASE_ADDRESS == XPLMI_IPI_BASEADDR)
		#define XPLMI_IPI_DEVICE_ID		XPAR_XIPIPSU_1_DEVICE_ID
	#endif
#else
	#if defined(XPAR_XIPIPSU_0_BASEADDR) && (XPAR_XIPIPSU_0_BASEADDR == XPLMI_IPI_BASEADDR)
		#define XPLMI_IPI_DEVICE_ID		XPAR_XIPIPSU_0_BASEADDR
	#elif defined(XPAR_XIPIPSU_1_BASEADDR) && (XPAR_XIPIPSU_1_BASEADDR == XPLMI_IPI_BASEADDR)
		#define XPLMI_IPI_DEVICE_ID		XPAR_XIPIPSU_1_BASEADDR
	#endif
#endif

/*
 * Definition for QSPI to be included
 */
#if (!defined(PLM_QSPI_EXCLUDE) && defined(XPAR_XQSPIPSU_0_BASEADDR))
#define XLOADER_QSPI
#endif

/*
 * Definition for OSPI to be included
 */
#ifndef SDT
#if (!defined(PLM_OSPI_EXCLUDE) && defined(XPAR_XOSPIPSV_0_DEVICE_ID))
#define XLOADER_OSPI
#define XLOADER_OSPI_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID
#define XLOADER_OSPI_BASEADDR		XPAR_XOSPIPSV_0_BASEADDR
#endif
#else
#if (!defined(PLM_OSPI_EXCLUDE) && defined(XPAR_XOSPIPSV_0_BASEADDR))
#define XLOADER_OSPI
#define XLOADER_OSPI_DEVICE_ID		XPAR_XOSPIPSV_0_BASEADDR
#define XLOADER_OSPI_BASEADDR		XPAR_XOSPIPSV_0_BASEADDR
#endif
#endif

/*
 * Definitions for SD to be included
 */
#if (!defined(PLM_SD_EXCLUDE) && defined(XPAR_XSDPS_0_BASEADDR) &&\
		(XPAR_XSDPS_0_BASEADDR == 0xF1040000U))
#define XLOADER_SD_0
#endif

#if ((!defined(PLM_SD_EXCLUDE)) &&\
		((defined(XPAR_XSDPS_1_BASEADDR) &&\
		  (XPAR_XSDPS_1_BASEADDR == 0xF1050000U)) ||\
		 (defined(XPAR_XSDPS_0_BASEADDR) &&\
		  (XPAR_XSDPS_0_BASEADDR == 0xF1050000U))))
#define XLOADER_SD_1
#endif

/*
 * Definition for SBI to be included
 */
#if !defined(PLM_SBI_EXCLUDE)
#define XLOADER_SBI
#endif

#if (!defined(PLM_USB_EXCLUDE) && defined(XPAR_XUSBPSU_0_BASEADDR) &&\
		(XPAR_XUSBPSU_0_BASEADDR == 0xFE200000U))
#define XLOADER_USB
#endif

/*
 * Definition for SEM to be included
 */
#if !defined(PLM_SEM_EXCLUDE) && (defined(XSEM_CFRSCAN_EN) ||\
		defined(XSEM_NPISCAN_EN))
#define XPLM_SEM
#endif

/*
 * Definition for UART
 */
#if defined(XPAR_XUARTPSV_NUM_INSTANCES)
#define XPLMI_UART_NUM_INSTANCES	XPAR_XUARTPSV_NUM_INSTANCES
#endif

#if defined(XPAR_XUARTPSV_0_BASEADDR)
#define XPLMI_UART_0_BASEADDR	XPAR_XUARTPSV_0_BASEADDR
#endif

#if defined(XPAR_XUARTPSV_1_BASEADDR)
#define XPLMI_UART_1_BASEADDR	XPAR_XUARTPSV_1_BASEADDR
#endif

/*
 * Definitions required from crp.h
 */
#define CRP_BASEADDR		(0XF1260000U)
#define CRP_BOOT_MODE_USER		(CRP_BASEADDR + 0X00000200U)
#define CRP_BOOT_MODE_USER_BOOT_MODE_MASK		(0X0000000FU)
#define CRP_BOOT_MODE_POR		(CRP_BASEADDR + 0X00000204U)
#ifndef CRP_RESET_REASON
#define CRP_RESET_REASON		(CRP_BASEADDR + 0X00000220U)
#endif
#define CRP_RESET_REASON_MASK		(0x0000FFFFU)
#define CRP_RESET_REASON_SHIFT		(16U)
#define CRP_RESET_REASON_EXT_POR_MASK		(0x1U)
#define CRP_RST_SBI				(CRP_BASEADDR + 0X00000324U)
#define CRP_RST_SBI_RESET_MASK			(0X00000001U)
#define CRP_RST_PDMA				(CRP_BASEADDR + 0X00000328U)
#define CRP_RST_PDMA_RESET1_MASK		(0X00000002U)
#ifndef CRP_RST_NONPS
#define CRP_RST_NONPS		(CRP_BASEADDR + 0X00000320U)
#endif
#define CRP_RST_NONPS_NPI_RESET_SHIFT		(0X4U)

/*
 * Register: CRP_RST_PS
 */
#ifndef CRP_RST_PS
#define CRP_RST_PS		(CRP_BASEADDR + 0x0000031CU)
#endif
#define CRP_RST_PS_PMC_SRST_MASK		(0X00000008U)
#define CRP_RST_PS_PMC_POR_MASK		(0X00000080U)

/*
 * Register: CRP_NOCPLL_CTRL
 */
#define CRP_NOCPLL_CTRL		((CRP_BASEADDR) + 0X00000050U)
#define CRP_NOCPLL_CTRL_BYPASS_MASK	(0X00000008U)
#define CRP_NOCPLL_CTRL_RESET_MASK	(0X00000001U)

/*
 * Register: CRP_NOCPLL_CFG
 */
#define CRP_NOCPLL_CFG		((CRP_BASEADDR) + 0X00000054U)

/*
 * Register: CRP_PLL_STATUS
 */
#define CRP_PLL_STATUS		((CRP_BASEADDR) + 0X00000060U)
#define CRP_PLL_STATUS_NOCPLL_LOCK_MASK		(0X00000002U)

/*
 * Register: CRP_SYSMON_REF_CTRL
 */
#define CRP_SYSMON_REF_CTRL    ((CRP_BASEADDR) + 0X00000138U)
#define CRP_SYSMON_REF_CTRL_SRCSEL_MASK    (0X00000004U)

/*
 * Definitions required from slave_boot.h
 */
#define SLAVE_BOOT_BASEADDR				(0XF1220000U)
#define SLAVE_BOOT_SBI_MODE		(SLAVE_BOOT_BASEADDR + 0X00000000U)
#define SLAVE_BOOT_SBI_MODE_JTAG_MASK			(0X00000002U)
#define SLAVE_BOOT_SBI_MODE_SELECT_MASK			(0X00000001U)

#define SLAVE_BOOT_SBI_CTRL		(SLAVE_BOOT_BASEADDR + 0X00000004U)
#define SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK		(0X0000001CU)
#define SLAVE_BOOT_SBI_CTRL_ENABLE_MASK		(0X00000001U)
#define SLAVE_BOOT_SBI_CTRL_APB_ERR_RES_MASK	(0X00000020U)

/*
 * Register: SLAVE_BOOT_SBI_STATUS
 */
#define SLAVE_BOOT_SBI_STATUS		(SLAVE_BOOT_BASEADDR + 0X0000000CU)
#define SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK		(0XF0000000U)
#define SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK		(0X00F00000U)
#define SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_MASK		(0X00001FF8U)
#define SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_VAL			(0X1000U)
#define SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_VAL		(0x800000U)
#define SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_VAL		(0x80000000U)

#define SLAVE_BOOT_SBI_IRQ_STATUS	(SLAVE_BOOT_BASEADDR + 0X00000300U)
#define SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK		(0X00000004U)

#define SLAVE_BOOT_SBI_IRQ_ENABLE	(SLAVE_BOOT_BASEADDR + 0X00000308U)
#define SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK		(0X00000004U)

#define XPLMI_SBI_CTRL_INTERFACE_SMAP			(0x0U)
#define XPLMI_SBI_CTRL_INTERFACE_JTAG			(0x4U)

/*
 * Definitions required from psm_global_reg
 */
#define PSM_GLOBAL_REG_BASEADDR			(0XFFC90000U)
#define PSM_GLOBAL_REG_PSM_CR_ERR1_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001028U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR1_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001048U)
#define PSM_GLOBAL_REG_PSM_IRQ1_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001068U)
#define PSM_GLOBAL_REG_PSM_CR_ERR2_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001038U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR2_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001058U)
#define PSM_GLOBAL_REG_PSM_IRQ2_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001078U)
#define PSM_GLOBAL_REG_PSM_CR_ERR1_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001024U)
#define PSM_GLOBAL_REG_PSM_CR_ERR2_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001034U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR1_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001044U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR2_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001054U)
#define PSM_GLOBAL_REG_PSM_ERR1_STATUS	(PSM_GLOBAL_REG_BASEADDR + 0X00001000U)
#define PSM_GLOBAL_REG_PSM_ERR2_STATUS	(PSM_GLOBAL_REG_BASEADDR + 0X00001004U)
#define PSM_GLOBAL_REG_PSM_CR_ERR1_MASK	(PSM_GLOBAL_REG_BASEADDR + 0X00001020U)
#define PSM_GLOBAL_REG_PSM_CR_ERR2_MASK	(PSM_GLOBAL_REG_BASEADDR + 0X00001030U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR1_MASK	(PSM_GLOBAL_REG_BASEADDR + 0X00001040U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR2_MASK	(PSM_GLOBAL_REG_BASEADDR + 0X00001050U)
#define PSM_GLOBAL_REG_PSM_IRQ1_MASK	(PSM_GLOBAL_REG_BASEADDR + 0X00001060U)
#define PSM_GLOBAL_REG_PSM_IRQ2_MASK	(PSM_GLOBAL_REG_BASEADDR + 0X00001070U)

#define PMC_GLOBAL_PSM_ERR_ACTION_OFFSET	(0x10U)

#define PSM_GLOBAL_REG_PSM_ERR_OFFSET	(4U)
#define PMC_GLOBAL_REG_PMC_ERR_OFFSET	(4U)

#define XPLMI_PSM_MAX_ERR_CNT	(0x2U)
#define XPLMI_PMC_MAX_ERR_CNT	(0x2U)

/*
 * Register: EFUSE_CACHE_IP_DISABLE_0
 */
#define EFUSE_CACHE_IP_DISABLE_0	(EFUSE_CACHE_BASEADDR + 0x00000018U)
#define EFUSE_CACHE_IP_DISABLE_0_EID_MASK		(0x07FFC000U)
#define EFUSE_CACHE_IP_DISABLE_0_EID_SEL_MASK		(0x04000000U)
#define EFUSE_CACHE_IP_DISABLE_0_EID1_MASK		(0x000FC000U)
#define EFUSE_CACHE_IP_DISABLE_0_EID1_SHIFT		(14U)
#define EFUSE_CACHE_IP_DISABLE_0_EID2_MASK		(0x03F00000U)
#define EFUSE_CACHE_IP_DISABLE_0_EID2_SHIFT		(20U)

/*
 * Register: EFUSE_CACHE_DNA_1
 */
#define EFUSE_CACHE_DNA_1		(EFUSE_CACHE_BASEADDR + 0x00000024U)
#define EFUSE_CACHE_DNA_1_BIT25_MASK	(0x02000000U)
/*
 * Register: PMC_TAP_SLR_TYPE
 */
#define PMC_TAP_SLR_TYPE		(PMC_TAP_BASEADDR + 0X00000024U)
#define PMC_TAP_SLR_TYPE_VAL_MASK		(0X00000007U)

/*
 * Register: PMC_TAP_JTAG_TEST
 */
#define PMC_TAP_JTAG_TEST		(PMC_TAP_BASEADDR + 0X00010014U)

/*
 * Register: PMC_IOU_SLCR
 */
#ifndef PMC_IOU_SLCR_BASEADDR
#define PMC_IOU_SLCR_BASEADDR      (0XF1060000U)
#endif

/*
 * Register: PMC_IOU_SLCR_SD0_CDN_CTRL
 */
#define PMC_IOU_SLCR_SD0_CDN_CTRL    (PMC_IOU_SLCR_BASEADDR + 0X0000044CU)
#define PMC_IOU_SLCR_SD0_CDN_CTRL_SD0_CDN_CTRL_MASK    (0X00000001U)

/*
 * Register: PMC_IOU_SLCR_SD1_CDN_CTRL
 */
#define PMC_IOU_SLCR_SD1_CDN_CTRL    (PMC_IOU_SLCR_BASEADDR + 0X000004CCU)
#define PMC_IOU_SLCR_SD1_CDN_CTRL_SD1_CDN_CTRL_MASK    (0X00000001U)

/*
 * Register: PMC_IOU_SLCR_CTRL
 */
#define PMC_IOU_SLCR_CTRL			(PMC_IOU_SLCR_BASEADDR + 0X00000700U)

/*
 * Register: PMC_IOU_SECURE_SLCR
 */
#define PMC_IOU_SECURE_SLCR_BASEADDR	(0XF1070000U)

/*
 * Register: PMC_IOU_SECURE_SLCR_CTRL
 */
#define PMC_IOU_SECURE_SLCR_CTRL	(PMC_IOU_SECURE_SLCR_BASEADDR + 0X00000040U)

/*
 * Register: PMC_RAM_CFG
 */
#define PMC_RAM_CFG_BASEADDR	(0XF1100000U)

/*
 * Register: PMC_ANALOG
 */
#define PMC_ANALOG_BASEADDR	(0XF1160000U)

/*
 * Register: PMC_ANALOG_SLVERR_CTRL
 */
#define PMC_ANALOG_SLVERR_CTRL	(PMC_ANALOG_BASEADDR + 0X00000050U)

/*
 * Register: AES
 */
#define AES_BASEADDR	(0XF11E0000U)

/*
 * Register: AES_SLV_ERR_CTRL
 */
#define AES_SLV_ERR_CTRL	(AES_BASEADDR + 0X00000210U)

/*
 * Register: BBRAM_CTRL
 */
#define BBRAM_CTRL_BASEADDR		(0XF11F0000U)

/*
 * Register: BBRAM_CTRL_SLV_ERR_CTRL
 */
#define BBRAM_CTRL_SLV_ERR_CTRL		(BBRAM_CTRL_BASEADDR + 0X00000034U)

/*
 * Register: ECDSA_RSA
 */
#define ECDSA_RSA_BASEADDR		(0XF1200000U)

/*
 * Register: ECDSA_RSA_APB_SLV_ERR_CTRL
 */
#define ECDSA_RSA_APB_SLV_ERR_CTRL		(ECDSA_RSA_BASEADDR + 0X00000044U)

/*
 * Register: SHA3
 */
#define SHA3_BASEADDR		(0XF1210000U)

/*
 * Register: SHA3_SHA_SLV_ERR_CTRL
 */
#define SHA3_SHA_SLV_ERR_CTRL		(SHA3_BASEADDR + 0X00000040U)

/*
 * Register: RTC
 */
#define RTC_BASEADDR		(0XF12A0000U)

/*
 * Register: RTC_CONTROL
 */
#define RTC_CONTROL		(RTC_BASEADDR + 0X00000040U)
#define RTC_CONTROL_SLVERR_EN_MASK	(0X80000001U)

/*
 * XMPUs
 */
#define PMC_XMPU_BASEADDR		(0XF12F0000U)
#define LPD_XMPU_BASEADDR		(0xFF980000U)
#define FPD_XMPU_BASEADDR		(0xFD390000U)

/*
 * XMPU Offsets
 */
#define XMPU_ERR_STATUS1_LO		(0x00000004U)
#define XMPU_ERR_STATUS1_HI		(0x00000008U)
#define XMPU_ERR_STATUS2		(0x0000000CU)
#define XMPU_ISR			(0x00000010U)
#define XMPU_IEN			(0x00000018U)

/*
 * XPPUs
 */
#define PMC_XPPU_BASEADDR		(0XF1310000U)
#define PMC_XPPU_NPI_BASEADDR		(0XF1300000U)
#define LPD_XPPU_BASEADDR		(0xFF990000U)

/*
 * XPPU Offsets
 */
#define XPPU_ERR_STATUS1		(0x00000004U)
#define XPPU_ERR_STATUS2		(0x00000008U)
#define XPPU_ISR			(0x00000010U)
#define XPPU_IEN			(0x00000018U)

/*
 * Register: INTPMC_CONFIG
 */
#define INTPMC_CONFIG_BASEADDR		(0XF1330000U)

/*
 * Register: INTPMC_CONFIG_IR_ENABLE
 */
#define INTPMC_CONFIG_IR_ENABLE		(INTPMC_CONFIG_BASEADDR + 0X00000008U)

/*
 * Register: PMC_GLOBAL_ROM_VALIDATION_DIGEST_0
 */
#define PMC_GLOBAL_ROM_VALIDATION_DIGEST_0    (PMC_GLOBAL_BASEADDR + 0X00000704U)

/*
 * Definitions required for PMC, PS GPIO
 */
#define PMC_GPIO_DATA_0_OFFSET		(0xF1020040U)
#define PMC_GPIO_DATA_1_OFFSET		(0xF1020044U)
#define PS_GPIO_DATA_0_OFFSET		(0xFF0B0040U)

/*
 * Definitions required for DDRMC dump
 */
#define DDRMC_PCSR_CONTROL_OFFSET	(0x00000004U)
#define DDRMC_PCSR_CONTROL_PCOMPLETE_MASK	(0x00000001U)
#define DDRMC_PCSR_STATUS_OFFSET	(0x00000008U)

/* PMC master and Slave SLR base addresses */
#define XPLMI_PMC_LOCAL_BASEADDR		(0xF0000000U)
#define XPLMI_PMC_LOCAL_ADDR_LEN		(0x8000000U)
#define XPLMI_PMC_ALIAS1_BASEADDR		(0x108000000UL)
#define XPLMI_PMC_ALIAS2_BASEADDR		(0x110000000UL)
#define XPLMI_PMC_ALIAS3_BASEADDR		(0x118000000UL)
#define XPLMI_PMC_ALIAS_MAX_ADDR		(XPLMI_PMC_ALIAS3_BASEADDR + XPLMI_PMC_LOCAL_ADDR_LEN)

/* Sysmon supply 0 address */
#define XPLMI_SYSMON_SUPPLY0_ADDR		(0xF1271040U)
#define XPLMI_SYSMON_SUPPLYX_MASK		(0x0000FFFFU)
#define XPLMI_VCC_PMC_MP_MIN			(0.775f)

/* Slave error enable mask */
#define XPLMI_SLAVE_ERROR_ENABLE_MASK	(0x1U)

/* Uart base address */
#define XPLMI_HW_UART0_BASEADDR		(0xFF000000U)
#define XPLMI_HW_UART1_BASEADDR		(0xFF010000U)
#define XPLMI_HW_PPU1_MDM_BASEADDR	(0xF0310000U)

/*
 * PMC IOmodule interrupts
 */
#define XPLMI_IOMODULE_PMC_GIC_IRQ		(16U)
#define XPLMI_IOMODULE_PPU1_MB_RAM		(17U)
#define XPLMI_IOMODULE_ERR_IRQ			(18U)
#define XPLMI_IOMODULE_CFRAME_SEU		(20U)
#define XPLMI_IOMODULE_PMC_GPI			(22U)
#define XPLMI_IOMODULE_SSIT_ERR2		(24U)
#define XPLMI_IOMODULE_SSIT_ERR1		(25U)
#define XPLMI_IOMODULE_SSIT_ERR0		(26U)

#define PMC_PMC_MB_IO_IRQ_ACK			(0xF028003CU)
#define PMC_PMC_MB_IO_IRQ_ISR			(0xF0280030U)
#define PMC_PMC_MB_IO_SSIT_IRQ_MASK		(0x7000000U)

#define XPLMI_PSM_RAM_BASE_ADDR		(0xFFC00000U)

/* Memory map used for validating addresses */
#define XPLMI_PSM_RAM_BASE_ADDR		(0xFFC00000U)
#define XPLMI_PSM_RAM_HIGH_ADDR		(0xFFC3FFFFU)
#define XPLMI_TCM0_BASE_ADDR		(0xFFE00000U)
#define XPLMI_TCM0_HIGH_ADDR		(0xFFE3FFFFU)
#define XPLMI_TCM1_BASE_ADDR		(0xFFE90000U)
#define XPLMI_TCM1_HIGH_ADDR		(0xFFEBFFFFU)
#define XPLMI_RSVD_BASE_ADDR		(0xA0000000U)
#define XPLMI_RSVD_HIGH_ADDR		(0xA3FFFFFFU)
#define XPLMI_M_AXI_FPD_MEM_HIGH_ADDR	(0xBFFFFFFFU)
#define XPLMI_OCM_BASE_ADDR		(0xFFFC0000U)
#define XPLMI_OCM_HIGH_ADDR		(0xFFFFFFFFU)
#define XPLMI_2GB_END_ADDR		(0x7FFFFFFFU)
#define XPLMI_4GB_END_ADDR		(0xFFFFFFFFU)
#define XPLMI_XRAM_BASE_ADDR		(0xFE800000U)
#define XPLMI_XRAM_HIGH_ADDR		(0xFEBFFFFFU)

#ifdef SDT
#define XCFRAME_DEVICE  (XPAR_XCFRAME_0_BASEADDR)
#define XLOADER_QSPI_DEVICE		(XPAR_XQSPIPSU_0_BASEADDR)
#define XLOADER_USB_DEVICE		(XPAR_XUSBPSU_0_BASEADDR)
#define XILPLMI_IOMODULE_INTC_MAX_INTR_SIZE (XPAR_XIOMODULE_0_MAX_INTR_SIZE)
#define IOMODULE_DEVICE (XPAR_XIOMODULE_0_BASEADDR)
#define XLOADER_QSPI_CONNECTION_MODE (XPAR_XQSPIPSU_0_CONNECTION_MODE)
#define XLOADER_QSPI_BUS_WIDTH (XPAR_XQSPIPSU_0_BUS_WIDTH)
#else
#define XCFRAME_DEVICE  (XPAR_XCFRAME_0_DEVICE_ID)
#define XLOADER_QSPI_DEVICE		(XPAR_XQSPIPSU_0_DEVICE_ID)
#define XLOADER_USB_DEVICE		(XPAR_XUSBPSU_0_DEVICE_ID)
#define XILPLMI_IOMODULE_INTC_MAX_INTR_SIZE (XPAR_IOMODULE_INTC_MAX_INTR_SIZE)
#define IOMODULE_DEVICE (XPAR_IOMODULE_0_DEVICE_ID)
#define XLOADER_QSPI_CONNECTION_MODE (XPAR_XQSPIPSU_0_QSPI_MODE)
#define XLOADER_QSPI_BUS_WIDTH (XPAR_XQSPIPSU_0_QSPI_BUS_WIDTH)
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_HW_H */
