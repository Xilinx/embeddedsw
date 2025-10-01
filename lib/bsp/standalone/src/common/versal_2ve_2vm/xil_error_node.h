/******************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_error_node.h
*
* This is the file which contains node IDs information for Versal 2VE and 2VM
* devices error events.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who 	 Date        Changes
* ----- -------- -------- -----------------------------------------------------
* 1.0   sk       02/20/2025 Initial release
*       sk       02/21/2025 Added nodes for Versal 2VE and 2VM devices EAM
*                           register in LPD SLCR
*       sk       04/07/2025 Updated error id encoding for UFSFE
* 1.1   ng       09/19/2025 Fixed LPD SLCR error descriptions
* 1.2   sk       09/23/2025 Added Additional HBMON Error ID's
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XIL_ERROR_NODE_H
#define XIL_ERROR_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/
/**@name Versal_2VE_2VM Event Node IDs
 * @defgroup xileventnodes Event Node IDs
 * @ingroup xilnodeids
 * @{
 */

#define XIL_NODETYPE_EVENT_ERROR_ID_ENCODING	(0x28100000U)

 /**
 * Error Event Node Ids
 */
#define XIL_NODETYPE_EVENT_ERROR_PMC_ERR1	(0x28100000U)
#define XIL_NODETYPE_EVENT_ERROR_PMC_ERR2	(0x28104000U)
#define XIL_NODETYPE_EVENT_ERROR_PMC_ERR3	(0x28108000U)
#define XIL_NODETYPE_EVENT_ERROR_LPD_SLCR_ERR1	(0x2810C000U)
#define XIL_NODETYPE_EVENT_ERROR_LPD_SLCR_ERR2	(0x28110000U)
#define XIL_NODETYPE_EVENT_ERROR_LPD_SLCR_ERR3	(0x28114000U)
#define XIL_NODETYPE_EVENT_ERROR_LPD_SLCR_ERR4	(0x28118000U)
#define XIL_NODETYPE_EVENT_ERROR_SW_ERR		(0x2811C000U)
/**
 * @}
 */

/**@name Versal_2VE_2VM Error event Mask
 * @defgroup xilerroreventmask Error Event Mask
 * @ingroup xilnodeids
 * @{
 * @defgroup pmcerr1 Error Event Mask for PMC ERR1
 * @ingroup xilerroreventmask
 * @{
 * @brief Error Events belong to PMC ERR1 Node
 */

/** Error event mask for PMC Boot Correctable Error.
 * Set by ROM code during ROM execution during Boot. */
#define XIL_EVENT_ERROR_MASK_BOOT_CR		(0x00000001U)

/** Error event mask for PMC Boot Non-Correctable Error.
 * Set by ROM code during ROM execution during Boot. */
#define XIL_EVENT_ERROR_MASK_BOOT_NCR		(0x00000002U)

/** Error event mask for PMC Firmware Boot Correctable Error.
 * Set by PLM during firmware execution during Boot. */
#define XIL_EVENT_ERROR_MASK_FW_CR		(0x00000004U)

/** Error event mask for PMC Firmware Boot Non-Correctable Error.
 * Set by PLM during firmware execution during Boot. */
#define XIL_EVENT_ERROR_MASK_FW_NCR		(0x00000008U)

/** Error event mask for General Software Correctable Error.
 * Set by any processors after Boot. */
#define XIL_EVENT_ERROR_MASK_GSW_CR		(0x00000010U)

/** Error event mask for General Software Non-Correctable Error.
 * Set by any processors after Boot. */
#define XIL_EVENT_ERROR_MASK_GSW_NCR		(0x00000020U)

/** Error event mask for CFU Error. */
#define XIL_EVENT_ERROR_MASK_CFU		(0x00000040U)

/** Error event mask for CFRAME Error. */
#define XIL_EVENT_ERROR_MASK_CFRAME		(0x00000080U)

/** Reserved */
#define XIL_EVENT_ERROR_MASK_RESERVED_1		(0x00000100U)

/** Reserved */
#define XIL_EVENT_ERROR_MASK_RESERVED_2		(0x00000200U)

/** Error event mask for DDRMC MB Correctable ECC Error. */
#define XIL_EVENT_ERROR_MASK_DDRMB_CR		(0x00000400U)

/** Error event mask for DDRMC MB Non-Correctable ECC Error. */
#define XIL_EVENT_ERROR_MASK_DDRMB_NCR		(0x00000800U)

/** Error event mask for NoC Type1 Correctable Error. */
#define XIL_EVENT_ERROR_MASK_NOCTYPE1_CR	(0x00001000U)

/** Error event mask for NoC Type1 Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_NOCTYPE1_NCR	(0x00002000U)

/** Error event mask for NoC User Error. */
#define XIL_EVENT_ERROR_MASK_NOCUSER		(0x00004000U)

/** Error event mask for MMCM Lock Error. */
#define XIL_EVENT_ERROR_MASK_MMCM		(0x00008000U)

/** Error event mask for ME Correctable Error. */
#define XIL_EVENT_ERROR_MASK_AIE_CR		(0x00010000U)

/** Error event mask for ME Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_AIE_NCR		(0x00020000U)

/** Error event mask for DDRMC MC Correctable ECC Error. */
#define XIL_EVENT_ERROR_MASK_DDRMC_CR		(0x00040000U)

/** Error event mask for DDRMC MC Non-Correctable ECC Error. */
#define XIL_EVENT_ERROR_MASK_DDRMC_NCR		(0x00080000U)

/** Error event mask for GT Correctable Error. */
#define XIL_EVENT_ERROR_MASK_GT_CR		(0x00100000U)

/** Error event mask for GT Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_GT_NCR		(0x00200000U)

/** Error event mask for PL Sysmon Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PLSMON_CR		(0x00400000U)

/** Error event mask for PL Sysmon Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PLSMON_NCR		(0x00800000U)

/** Error event mask for User defined PL generic error. */
#define XIL_EVENT_ERROR_MASK_PL0		(0x01000000U)

/** Error event mask for User defined PL generic error. */
#define XIL_EVENT_ERROR_MASK_PL1		(0x02000000U)

/** Error event mask for User defined PL generic error. */
#define XIL_EVENT_ERROR_MASK_PL2		(0x04000000U)

/** Error event mask for User defined PL generic error. */
#define XIL_EVENT_ERROR_MASK_PL3		(0x08000000U)

/** Error event mask for NPI Root Error. */
#define XIL_EVENT_ERROR_MASK_NPIROOT		(0x10000000U)

/** Error event mask for SSIT Error from Slave SLR1,
 * Only used in Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT3		(0x20000000U)

/** Error event mask for SSIT Error from Slave SLR2,
 * Only used in Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT4		(0x40000000U)

/** Error event mask for SSIT Error from Slave SLR3,
 * Only used in Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT5		(0x80000000U)
/**
 * @}
 */

/**
 * @defgroup pmcerr2 Error Event Mask for PMC ERR2
 * @ingroup xilpmerroreventmask
 * @{
 * @brief Error Events belong to PMC ERR2 Node
 */
/** Error event mask for General purpose PMC error,
 * can be triggered by any of the following peripherals:,
 * - PMC Global Registers,- PMC Clock & Reset (CRP),- PMC IOU Secure SLCR,
 * - PMC IOU SLCR,- BBRAM Controller,- PMC Analog Control Registers,
 * - RTC Control Registers. */
#define XIL_EVENT_ERROR_MASK_PMCAPB		(0x00000001U)

/** Error event mask for PMC ROM Validation Error. */
#define XIL_EVENT_ERROR_MASK_PMCROM		(0x00000002U)

/** Error event mask for PMC PPU0 MB TMR Fatal Error. */
#define XIL_EVENT_ERROR_MASK_MB_FATAL0		(0x00000004U)

/** Error event mask for PMC PPU1 MB TMR Fatal Error. */
#define XIL_EVENT_ERROR_MASK_MB_FATAL1		(0x00000008U)

/** Error event mask for PMC Correctable Errors:,
 * PPU0 RAM correctable error.,PPU1 instruction RAM correctable error.,
 * PPU1 data RAM correctable error. */
#define XIL_EVENT_ERROR_MASK_PMC_CR		(0x00000020U)

/** Error event mask for PMC Non-Correctable Errors:,
 * PPU0 RAM non-correctable error.,PPU1 instruction RAM non-correctable error.,
 * PPU1 data RAM non-correctable error.,PRAM non-correctable error. */
#define XIL_EVENT_ERROR_MASK_PMC_NCR		(0x00000040U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[0].
 * Indicates an alarm condition on any of SUPPLY0 to SUPPLY31. */
#define XIL_EVENT_ERROR_MASK_PMCSMON0		(0x00000080U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[1].
 * Indicates an alarm condition on any of SUPPLY32 to SUPPLY63. */
#define XIL_EVENT_ERROR_MASK_PMCSMON1		(0x00000100U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[2].
 * Indicates an alarm condition on any of SUPPLY64 to SUPPLY95. */
#define XIL_EVENT_ERROR_MASK_PMCSMON2		(0x00000200U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[3].
 * Indicates an alarm condition on any of SUPPLY96 to SUPPLY127. */
#define XIL_EVENT_ERROR_MASK_PMCSMON3		(0x00000400U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[4].
 * Indicates an alarm condition on any of SUPPLY128 to SUPPLY159. */
#define XIL_EVENT_ERROR_MASK_PMCSMON4		(0x00000800U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[8].
 * Indicates an over-temperature alarm. */
#define XIL_EVENT_ERROR_MASK_PMCSMON8		(0x00008000U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[9].
 * Indicates a device temperature alarm. */
#define XIL_EVENT_ERROR_MASK_PMCSMON9		(0x00010000U)

/** Error event mask for CFI Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_CFI		(0x00020000U)

/** Error event mask for CFRAME SEU CRC Error. */
#define XIL_EVENT_ERROR_MASK_SEUCRC		(0x00040000U)

/** Error event mask for CFRAME SEU ECC Error. */
#define XIL_EVENT_ERROR_MASK_SEUECC		(0x00080000U)

/** Error event mask for PMX WDT Error. */
#define XIL_EVENT_ERROR_MASK_PMX_WWDT		(0x00100000U)

/** Error event mask for RTC Alarm Error. */
#define XIL_EVENT_ERROR_MASK_RTCALARM		(0x00400000U)

/** Error event mask for PMC NPLL Lock Error,
 * This error can be unmasked after the NPLL is locked to alert when the
 * NPLL loses lock. */
#define XIL_EVENT_ERROR_MASK_NPLL		(0x00800000U)

/** Error event mask for PMC PPLL Lock Error,
 * This error can be unmasked after the PPLL is locked to alert when the
 * PPLL loses lock. */
#define XIL_EVENT_ERROR_MASK_PPLL		(0x01000000U)

/** Error event mask for Clock Monitor Errors.,
 * Collected from CRP's CLKMON_STATUS register. */
#define XIL_EVENT_ERROR_MASK_CLKMON		(0x02000000U)

/** Error event mask for PMC interconnect correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_PMX_CORR_ERR		(0x08000000U)

/** Error event mask for PMC interconnect uncorrectable error. */
#define XIL_EVENT_ERROR_MASK_INT_PMX_UNCORR_ERR		(0x10000000U)

/** Error event mask for For Master SLR: SSIT Error from Slave SLR1.,
 * For Slave SLRs: SSIT Error0 from Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT0		(0x20000000U)

/** Error event mask for For Master SLR: SSIT Error from Slave SLR2.,
 * For Slave SLRs: SSIT Error1 from Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT1		(0x40000000U)

/** Error event mask for For Master SLR: SSIT Error from Slave SLR3.,
 * For Slave SLRs: SSIT Error2 from Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT2		(0x80000000U)
/**
 * @}
 */

/**
 * @defgroup pmcerr3 Error Event Mask for PMC ERR3
 * @ingroup xilpmerroreventmask
 * @{
 * @brief Error Events belong to PMC ERR3 Node
 */
/** Error event mask for PMC IOU correctable error. */
#define XIL_EVENT_ERROR_MASK_IOU_CR		(0x00000001U)

/** Error event mask for PMC IOU uncorrectable error. */
#define XIL_EVENT_ERROR_MASK_IOU_NCR		(0x00000002U)

/** Error event mask for DFX unexpected activation. */
#define XIL_EVENT_ERROR_MASK_DFX_UXPT_ACT		(0x00000004U)

/** Error event mask for DICE CDI SEED parity. */
#define XIL_EVENT_ERROR_MASK_DICE_CDI_PAR		(0x00000008U)

/** Error event mask for Device identity private key parity. */
#define XIL_EVENT_ERROR_MASK_DEVIK_PRIV		(0x00000010U)

/** Error event mask for Next SW CDI SEED parity. */
#define XIL_EVENT_ERROR_MASK_NXTSW_CDI_PAR		(0x00000020U)

/** Error event mask for Device attestation private key parity. */
#define XIL_EVENT_ERROR_MASK_DEVAK_PRIV		(0x00000040U)

/** Error event mask for DME public key X component's parity. */
#define XIL_EVENT_ERROR_MASK_DME_PUB_X		(0x00000080U)

/** Error event mask for DME public key Y component's parity. */
#define XIL_EVENT_ERROR_MASK_DME_PUB_Y		(0x00000100U)

/** Error event mask for DEVAK public key X component's parity. */
#define XIL_EVENT_ERROR_MASK_DEVAK_PUB_X		(0x00000200U)

/** Error event mask for DEVAK public key Y component's parity. */
#define XIL_EVENT_ERROR_MASK_DEVAK_PUB_Y		(0x00000400U)

/** Error event mask for DEVIK public key X component's parity. */
#define XIL_EVENT_ERROR_MASK_DEVIK_PUB_X		(0x00000800U)

/** Error event mask for DEVIK public key Y component's parity. */
#define XIL_EVENT_ERROR_MASK_DEVIK_PUB_Y		(0x00001000U)

/** Error event mask for PCR parity. */
#define XIL_EVENT_ERROR_MASK_PCR_PAR		(0x00002000U)

/** Error event mask for PSX_EAM_E0. */
#define XIL_EVENT_ERROR_MASK_PSX_EAM_E0		(0x00004000U)

/** Error event mask for PSX_EAM_E1. */
#define XIL_EVENT_ERROR_MASK_PSX_EAM_E1		(0x00008000U)

/** Error event mask for PSX_EAM_E2. */
#define XIL_EVENT_ERROR_MASK_PSX_EAM_E2		(0x00010000U)

/** Error event mask for PSX_EAM_E3. */
#define XIL_EVENT_ERROR_MASK_PSX_EAM_E3		(0x00020000U)

/** Error event mask for ASU_EAM_GD. */
#define XIL_EVENT_ERROR_MASK_ASU_EAM_GD		(0x00040000U)

/** Error event mask for PMC_EAM_GD. */
#define XIL_EVENT_ERROR_MASK_PMC_EAM_GD		(0x00080000U)

/** Error event mask for PMC_EAM_SMIRQ0 */
#define XIL_EVENT_ERROR_MASK_PMC_EAM_SMIRQ0	(0x00100000U)

/** Error event mask for PMC_EAM_SMIRQ1 */
#define XIL_EVENT_ERROR_MASK_PMC_EAM_SMIRQ1	(0x00200000U)

/** Error event mask for PMC_EAM_PRAM */
#define XIL_EVENT_ERROR_MASK_PMC_EAM_PRAM	(0x00400000U)

/** Error event mask for PMC_EAM_AGERR */
#define XIL_EVENT_ERROR_MASK_PMC_EAM_AGERR	(0x00800000U)

/** Error event mask for PMC_EAM_UFSFE */
#define XIL_EVENT_ERROR_MASK_PMC_EAM_UFSFE	(0x01000000U)

/**
 * @}
 */

/**
 * @defgroup LPD SLCR err0 Error Event Mask for LPD SLCR ERR0
 * @ingroup xilpmerroreventmask
 * @{
 * @brief Error Events belong to LPD SLCR ERR0 Node
 */
/** Error event mask for PS Software can write to trigger register to
 * generate this Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PS_SW_CR		(0x00000001U)

/** Error event mask for PS Software can write to trigger register to
 * generate this Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PS_SW_NCR		(0x00000002U)

/** Error event mask for Aggregated LPX USB errors. */
#define XIL_EVENT_ERROR_MASK_USB_ERR		(0x00000004U)

/** Error event mask for Aggregated LPX DFX controllers
 * unexpected activation errors. */
#define XIL_EVENT_ERROR_MASK_LPX_DFX		(0x00000008U)

/** Error event mask for UFSHC_FE_IRQ Error-Unused. */
#define XIL_EVENT_ERROR_MASK_UFSHC_FE_IRQ		(0x00000010U)

/** Error event mask for APLL1 lock error. */
#define XIL_EVENT_ERROR_MASK_APLL1_LOCK		(0x00000020U)

/** Error event mask for APLL2 lock error. */
#define XIL_EVENT_ERROR_MASK_APLL2_LOCK		(0x00000040U)

/** Error event mask for RPLL Lock Errors. The error can be unmasked
 * after the PLL is locked to alert when the PLL loses lock. */
 #define XIL_EVENT_ERROR_MASK_RPLL_LOCK		(0x00000080U)

/** Error event mask for FLXPLL Lock Errors. The error can be unmasked
 * after the PLL is locked to alert when the PLL loses lock. */
#define XIL_EVENT_ERROR_MASK_FLXPLL_LOCK	(0x00000100U)

/** Error event mask for Aggregated LPX ASIL B correctable
 * errors from OCMASILB, LPXASILB, IOU */
#define XIL_EVENT_ERROR_MASK_INT_LPXASILB_CR		(0x00000200U)

/** Error event mask for Aggregated LPX ASIL B uncorrectable
 * errors from OCMASILB, LPXASILB, IOU */
#define XIL_EVENT_ERROR_MASK_INT_LPXASILB_NCR		(0x00000400U)

/** Error event mask for Aggregated LPX ASIL D correctable
 * errors from OCMASILD, LPXASILD */
#define XIL_EVENT_ERROR_MASK_INT_LPXASILD_CR		(0x00000800U)

/** Error event mask for Aggregated LPX ASIL D uncorrectable
 * errors from OCMASILD, LPXASILD */
#define XIL_EVENT_ERROR_MASK_INT_LPXASILD_NCR		(0x00001000U)

/** Error event mask for Aggregated FPX ASIL D correctable
 * errors. */
#define XIL_EVENT_ERROR_MASK_INT_FPXASILD_CR		(0x00002000U)

/** Error event mask for Aggregated FPX ASIL D uncorrectable
 * errors. */
#define XIL_EVENT_ERROR_MASK_INT_FPXASILD_NCR		(0x00004000U)

/** Error event mask for Aggregated FPX ASIL B correctable
 * errors. */
#define XIL_EVENT_ERROR_MASK_INT_FPXASILB_CR	(0x00008000U)

/** Error event mask for Aggregated FPX ASIL B uncorrectable
 * errors. */
#define XIL_EVENT_ERROR_MASK_INT_FPXASILB_NCR	(0x00010000U)

/** Error event mask for Splitter interconnect correctable error */
#define XIL_EVENT_ERROR_MASK_INT_SPLIT_CR		(0x00020000U)

/** Error event mask for Splitter interconnect uncorrectable error */
#define XIL_EVENT_ERROR_MASK_INT_SPLIT_NCR		(0x00040000U)

/** Error event mask for Firewall write errors from NOC NMUs*/
#define XIL_EVENT_ERROR_MASK_NOC_NMU_FIREWALL_WR_ERR	(0x00800000U)

/** Error event mask for Firewall read error from NOC NMU */
#define XIL_EVENT_ERROR_MASK_NOC_NMU_FIREWALL_RD_ERR	(0x01000000U)

/** Error event mask for Firewall error from NOC NSU. */
#define XIL_EVENT_ERROR_MASK_NOC_NSU_FIREWALL_ERR	(0x02000000U)

/** Error event mask for GIC_FMU_ERR, ARM suggested to have
 * separated from GIC_ERR. */
#define XIL_EVENT_ERROR_MASK_GIC_FMU_ERR	(0x04000000U)

/** Error event mask for GIC_FMU_FAULT, ARM suggested to have
 * separated from GIC_FAULT */
#define XIL_EVENT_ERROR_MASK_GIC_FMU_FAULT		(0x08000000U)

/** Error event mask for Aggregated IPI error (see IPI_ISR reg) */
#define XIL_EVENT_ERROR_MASK_IPI_ERR		(0x40000000U)

/** Error event mask for Aggregated CPI error (see CPI_IRQ reg) */
#define XIL_EVENT_ERROR_MASK_FPD_CPI		(0x80000000U)
/**
 * @}
 */


/**
 * @defgroup LPD SLCR 1 Error Event Mask for LPD SLCR EAM ERR1
 * @ingroup xilpmerroreventmask
 * @brief Error Events belong to LPD SLCR ERR1 Node
 * @{
 */
/** Error event mask for FPD WDT0 error. */
#define XIL_EVENT_ERROR_MASK_FPD_WDT0		(0x00000001U)

/** Error event mask for FPD WDT1 error. */
#define XIL_EVENT_ERROR_MASK_FPD_WDT1		(0x00000002U)

/** Error event mask for FPD WDT2 error. */
#define XIL_EVENT_ERROR_MASK_FPD_WDT2		(0x00000004U)

/** Error event mask for FPD WDT3 error. */
#define XIL_EVENT_ERROR_MASK_FPD_WDT3		(0x00000008U)

/** Error event mask for Memory Errors for Splitter0. */
#define XIL_EVENT_ERROR_MASK_PSXC_SPLITTER0_NON_FATAL_ERR	(0x00000010U)

/** Error event mask for Memory Errors for Splitter1. */
#define XIL_EVENT_ERROR_MASK_PSXC_SPLITTER1_NON_FATAL_ERR	(0x00000020U)

/** Error event mask for Memory Errors for Splitter2. */
#define XIL_EVENT_ERROR_MASK_PSXC_SPLITTER2_NON_FATAL_ERR	(0x00000040U)

/** Error event mask for Memory Errors for Splitter3. */
#define XIL_EVENT_ERROR_MASK_PSXC_SPLITTER3_NON_FATAL_ERR	(0x00000080U)

/** Error event mask for aggregated Fatal Error For Splitter0-3. */
#define XIL_EVENT_ERROR_MASK_PSXC_SPLITTER_FATAL_ERR	(0x00000100U)

/** Error event mask for Aggregated GIC_error */
#define XIL_EVENT_ERROR_MASK_GIC_ERR	(0x00000200U)

/** Error event mask for Aggregated GIC_fault */
#define XIL_EVENT_ERROR_MASK_GIC_FAULT   	(0x00000400U)

/** Error event mask for aggregated CMN faults from all
* PD domains and FMU*/
#define XIL_EVENT_ERROR_MASK_CMN_FAULT	(0x00000800U)

/** Error event mask for aggregated CMN errors from
* all PD domains and FMU */
#define XIL_EVENT_ERROR_MASK_CMN_ERR 	(0x00001000U)

/** Error event mask for aggregated errors from
 * ACP0 + ACP1 */
#define XIL_EVENT_ERROR_MASK_ACP_ERR	(0x00002000U)

/** Error event mask for APU Cluster 0 fatal error */
#define XIL_EVENT_ERROR_MASK_FPD_APU0_ERI	(0x00004000U)

/** Error event mask for APU Cluster 0 non-fatal/fatal error */
#define XIL_EVENT_ERROR_MASK_FPD_APU0_FHI	(0x00008000U)

/** Error event mask forAPU Cluster 1 error. */
#define XIL_EVENT_ERROR_MASK_FPD_APU1_ERI		(0x00010000U)

/** Error event mask for APU Cluster 1 non-fatal/fatal error */
#define XIL_EVENT_ERROR_MASK_FPD_APU1_FHI		(0x00020000U)

/** Error event mask for APU Cluster 2 error */
#define XIL_EVENT_ERROR_MASK_FPD_APU2_ERI	(0x00040000U)

/** Error event mask for APU Cluster 2 non-fatal/fatal error */
#define XIL_EVENT_ERROR_MASK_FPD_APU2_FHI		(0x00080000U)

/** Error event mask for APU Cluster 3 error */
#define XIL_EVENT_ERROR_MASK_FPD_APU3_ERI	(0x00100000U)

/** Error event mask for APU Cluster 3 non-fatal/fatal error */
#define XIL_EVENT_ERROR_MASK_FPD_APU3_FHI	(0x00200000U)

/** Error event mask for aggregated MMU error. */
#define XIL_EVENT_ERROR_MASK_FPD_MMU_ERR	(0x00400000U)

/** Error event mask for aggregated MMU fault. */
#define XIL_EVENT_ERROR_MASK_FPD_MMU_FAULT	(0x00800000U)

/** Error event mask for SLCR errors */
#define XIL_EVENT_ERROR_MASK_FPD_SLCR_ERR		(0x01000000U)

/** Error event mask for SLCR SECURE errors. */
#define XIL_EVENT_ERROR_MASK_FPD_SLCR_SECURE_ERR	(0x02000000U)

/** Error event mask for Non Fatal Error from AFI FM0 in FPX. */
#define XIL_EVENT_ERROR_MASK_FPX_AFIFM0_NONFATAL_ERR	(0x04000000U)

/** Error event mask for Non Fatal Error from AFI FM1 in FPX. */
#define XIL_EVENT_ERROR_MASK_FPX_AFIFM1_NONFATAL_ERR	(0x08000000U)

/** Error event mask for Non Fatal Error from AFI FM2 in FPX */
#define XIL_EVENT_ERROR_MASK_FPX_AFIFM2_NONFATAL_ERR	(0x10000000U)

/** Error event mask for Non Fatal Error from AFI FM3 in FPX */
#define XIL_EVENT_ERROR_MASK_FPX_AFIFM3_NONFATAL_ERR	(0x20000000U)

/** Error event mask for FPXAFIFS Corr Error */
#define XIL_EVENT_ERROR_MASK_FPX_AFIFS_CORR_ERR	(0x40000000U)

/** Error event mask for FPXAFIFS UnCorr Error */
#define XIL_EVENT_ERROR_MASK_FPX_AFIFS_UNCORR_ERR	(0x80000000U)
/**
 * @}
 */

/**
 * @defgroup LPDSLCRERR@ Error Event Mask for LPDSLCR ERR2
 * @ingroup xilpmerroreventmask
 * @brief Error Events belong to LPDSLCR ERR2 Node
 * @{
 */
/** Error event mask for Aggregated RPU Cluster A
 * cluster+ core Fatal Error */
#define XIL_EVENT_ERROR_MASK_RPUA_CORE_CLUSTER_FATAL		(0x00000001U)

/** Error event mask for RPUA Core0 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_RPUA_CORE0_NON_FATAL	(0x00000002U)

/** Error event mask for RPUA Core1 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_RPUA_CORE1_NON_FATAL		(0x00000004U)

/** Error event mask for Aggregated RPU Cluster B
 * cluster+ core Fatal Error */
#define XIL_EVENT_ERROR_MASK_RPUB_CORE_CLUSTER_FATAL		(0x00000008U)

/** Error event mask for RPUB Core0 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_RPUB_CORE0_NON_FATAL		(0x00000010U)

/** Error event mask for RPUB Core1 NonFatal Error. */
#define XIL_EVENT_ERROR_MASK_RPUB_CORE1_NON_FATAL		(0x00000020U)

/** Error event mask for Aggregated RPU Cluster C
 * cluster+ core Fatal Error */
#define XIL_EVENT_ERROR_MASK_RPUC_CORE_CLUSTER_FATAL	(0x00000040U)

/** Error event mask for RPUC Core0 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_RPUC_CORE0_NON_FATAL		(0x00000080U)

/** Error event mask for RPUC Core1 NonFatal Error*/
#define XIL_EVENT_ERROR_MASK_RPUC_CORE1_NON_FATAL		(0x00000100U)

/** Error event mask for Aggregated RPU Cluster D
 * cluster+ core Fatal Error */
#define XIL_EVENT_ERROR_MASK_RPUD_CORE_CLUSTER_FATAL	(0x00000200U)

/** Error event mask for RPUD Core0 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_RPUD_CORE0_NON_FATAL    	(0x00000400U)

/** Error event mask for RPUD Core1 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_RPUD_CORE1_NON_FATAL	(0x00000800U)

/** Error event mask for Aggregated RPU Cluster E
* cluster+ core Fatal Error */
#define XIL_EVENT_ERROR_MASK_RPUE_CORE_CLUSTER_FATAL	(0x00001000U)

/** Error event mask for RPUE Core0 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_RPUE_CORE0_NON_FATAL		(0x00002000U)

/** Error event mask for RPUE Core1 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_RPUE_CORE1_NON_FATAL	(0x00004000U)

/** Error event mask for PCIL ERR FOR RPU Clusters */
#define XIL_EVENT_ERROR_MASK_RPU_PCIL_ERR    	(0x00008000U)

/** Error event mask for OCM Bank0 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_OCM0_NONFATAL_ERR	(0x00010000U)

/** Error event mask for OCM Bank0 Fatal Error */
#define XIL_EVENT_ERROR_MASK_OCM0_FATAL_ERR		(0x00020000U)

/** Error event mask for OCM Bank1 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_OCM1_NONFATAL_ERR	(0x00040000U)

/** Error event mask for OCM Bank1 Fatal Error */
#define XIL_EVENT_ERROR_MASK_OCM1_FATAL_ERR	(0x00080000U)

/** Error event mask for OCM Bank2 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_OCM2_NONFATAL_ERR    	(0x00100000U)

/** Error event mask for OCM Bank2 Fatal Error */
#define XIL_EVENT_ERROR_MASK_OCM2_FATAL_ERR	(0x00200000U)

/** Error event mask for OCM Bank3 NonFatal Error*/
#define XIL_EVENT_ERROR_MASK_OCM3_NONFATAL_ERR		(0x00400000U)

/** Error event mask for PCIL ERR FOR RPU Clusters. */
#define XIL_EVENT_ERROR_MASK_OCM3_FATAL_ERR		(0x00800000U)

/** Error event mask for LPX WDT0 Errors*/
#define XIL_EVENT_ERROR_MASK_LPX_WWDT0		(0x01000000U)

/** Error event mask for LPX WDT1 Errors */
#define XIL_EVENT_ERROR_MASK_LPX_WWDT1	(0x02000000U)

/** Error event mask for LPX WDT2 Errors */
#define XIL_EVENT_ERROR_MASK_LPX_WWDT2		(0x04000000U)

/** Error event mask for LPX WDT3 Errors */
#define XIL_EVENT_ERROR_MASK_LPX_WWDT3		(0x08000000U)

/** Error event mask for LPX WDT4 Errors */
#define XIL_EVENT_ERROR_MASK_LPX_WWDT4		(0x10000000U)

/** Error event mask for ADMA LS Error */
#define XIL_EVENT_ERROR_MASK_ADMA_LS_ERR		(0x20000000U)

/** Error event mask for LPX Glitch Detector0 glitch detected. */
#define XIL_EVENT_ERROR_MASK_LPX_GLITCH_DET0	(0x40000000U)

/** Error event mask for LPX Glitch Detector1 glitch detected. */
#define XIL_EVENT_ERROR_MASK_LPX_GLITCH_DET1	(0x80000000U)

/**
 * @}
 */

/**
 * @defgroup lpdslcr3 Error Event Mask for LPDSLCR ERR3
 * @ingroup xilpmerroreventmask
 * @brief Error Events belong to LPDSLCR ERR3 Node
 * @{
 */
/** Error event mask for FPD Reset Monitor ERROR*/
#define XIL_EVENT_ERROR_MASK_FPD_CRF		(0x00000001U)

/** Error event mask for LPD reset and Clock Monitor Error. */
#define XIL_EVENT_ERROR_MASK_LPD_MON_ERR	(0x00000002U)

/** Error event mask for Fatal Error from all AFI FM. */
#define XIL_EVENT_ERROR_MASK_FATAL_AFI_FM	    (0x00000004U)

/** Error event mask for Non Fatal Error from AFI FM in LPX. */
#define XIL_EVENT_ERROR_MASK_NFATAL_AFI_FM_LPX      (0x00000008U)

/** Error event mask for aggregated ASU + ASU_PL Fatal error */
#define XIL_EVENT_ERROR_MASK_LPD_ASU_FATAL	    (0x00000010U)

/** Error event mask for aggregated ASU + ASU_PL NonFatal error. */
#define XIL_EVENT_ERROR_MASK_LPD_ASU_NON_FATAL	    (0x00000020U)

/** Error event mask for LPX AFI FS Non Fatal Error. */
#define XIL_EVENT_ERROR_MASK_LPX_AFIFS_CORR_ERR    (0x00000040U)

/** Error event mask for LPX AFI FS Fatal Error */
#define XIL_EVENT_ERROR_MASK_LPX_AFIFS_UNCORR_ERR	    (0x00000080U)

/** Error event mask for MMI top level correctable error */
#define XIL_EVENT_ERROR_MASK_MMI_CORR_EVENT	    (0x00000100U)

/** Error event mask for MMI top level uncorrectable error */
#define XIL_EVENT_ERROR_MASK_MMI_UNCORR_EVENT	    (0x00000200U)

/** Error event mask for MMI gpu correctable error */
#define XIL_EVENT_ERROR_MASK_MMI_GPU_COR_EVENT    	(0x00000400U)

/** Error event mask for MMI pcie0 correctable error*/
#define XIL_EVENT_ERROR_MASK_MMI_PCIE0_COR_EVENT	(0x00000800U)

/** Error event mask MMI pcie1 correctable error */
#define XIL_EVENT_ERROR_MASK_MMI_PCIE1_COR_EVENT	(0x00001000U)

/** Error event mask for RPUE Core0 NonFatal Error */
#define XIL_EVENT_ERROR_MASK_MMI_GEM_COR_EVENT	(0x00002000U)

/** Error event mask for MMI dc correctable error */
#define XIL_EVENT_ERROR_MASK_MMI_DC_COR_EVENT	(0x00004000U)

/** Error event mask for MMI udh correctable error */
#define XIL_EVENT_ERROR_MASK_MMI_UDH_COR_EVENT	   	(0x00008000U)

/** Error event mask for ADMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_ADMA_ERR1	(0x00010000U)

/** Error event mask for ADMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_ADMA_ERR2	(0x00020000U)

/** Error event mask for ADMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_ADMA_ERR3	(0x00040000U)

/** Error event mask for ADMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_ADMA_ERR4	(0x00080000U)

/** Error event mask for ADMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_ADMA_ERR5    	(0x00100000U)

/** Error event mask for ADMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_ADMA_ERR6	(0x00200000U)

/** Error event mask for ADMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_ADMA_ERR7	(0x00400000U)

/** Error event mask for ADMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_ADMA_ERR8		(0x00800000U)

/** Error event mask for SDMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_SDMA_ERR1	(0x01000000U)

/** Error event mask for SDMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_SDMA_ERR2	(0x02000000U)

/** Error event mask for SDMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_SDMA_ERR3		(0x04000000U)

/** Error event mask for SDMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_SDMA_ERR4		(0x08000000U)

/** Error event mask for SDMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_SDMA_ERR5		(0x10000000U)

/** Error event mask for SDMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_SDMA_ERR6		(0x20000000U)

/** Error event mask for SDMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_SDMA_ERR7	(0x40000000U)

/** Error event mask for SDMA perchannel error*/
#define XIL_EVENT_ERROR_MASK_SDMA_ERR8	(0x80000000U)


/**
 * @}
 */

/**
 * @defgroup swerr Error Event Mask for Software error events
 * @ingroup xilpmerroreventmask
 * @{
 * @brief Error Events belong to SW ERR Node
 */
/** Error event mask for Software error events */
/** Health Boot Monitoring errors */
#define XIL_EVENT_ERROR_MASK_HB_MON_0		(0x00000001U)
#define XIL_EVENT_ERROR_MASK_HB_MON_1		(0x00000002U)
#define XIL_EVENT_ERROR_MASK_HB_MON_2		(0x00000004U)
#define XIL_EVENT_ERROR_MASK_HB_MON_3		(0x00000008U)
#define XIL_EVENT_ERROR_MASK_PLM_EXCEPTION	(0x00000010U)
#define XIL_EVENT_ERROR_MASK_DEV_STATE_CHANGE 	(0x00000020U)
#define XIL_EVENT_ERROR_PCR_LOG_UPDATE		(0x00000040U)
/** XilSem errors */
#define XIL_EVENT_ERROR_MASK_XSEM_CRAM_CE	(0x00000080U)
#define XIL_EVENT_ERROR_MASK_XSEM_CRAM_UE	(0x00000100U)
#define XIL_EVENT_ERROR_MASK_XSEM_NPI_UE	(0x00000200U)
#define XIL_EVENT_ERROR_MASK_HB_MON_4		(0x00000400U)
#define XIL_EVENT_ERROR_MASK_HB_MON_5		(0x00000800U)
#define XIL_EVENT_ERROR_MASK_HB_MON_6		(0x00001000U)
#define XIL_EVENT_ERROR_MASK_HB_MON_7		(0x00002000U)
/**
 * @}
 */

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif /* XIL_ERROR_NODE_H */
